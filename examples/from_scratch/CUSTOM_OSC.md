# Custom oscillator (own DSP microcode) — technical description

State 2026-10-15. First DSP program of our own: a naive sawtooth for the oscillator slots of the PLUG-OUT engine,
written from scratch with the measured instruction set (`DSP_ISA_SPEC.md`), assembled by `re/dspasm.py`, built into
the C-SAW PLUG-OUT and verified on the SYSTEM-8. Hardware/software context: `../../ARCHITECTURE.md`.

| item | value |
|---|---|
| source | `re/dspasm.py` (`saw()`), generated listings `re/dspexp/own/saw_dco1.txt`, `saw_dco2.txt` |
| size | 187 words (0x176 bytes) per slot program, both voices |
| runs on | slave BMCs, DSP2 and DSP3, oscillator slot DCO1 (`0x6000`) and/or DCO2 (`0xB000`); 2 voices per DSP |
| rate | 96 kHz, once per sample |
| result | in tune within ±1.2 cents from note 36 to 96, harmonic amplitudes 1/n (ideal sawtooth) |
| limitations | no anti-aliasing; PULSE (same program + flag) and the other slot inputs are ignored |

## 1. Where the program runs and how it gets there

* The engine (region 1, `engine/engine.c`, runs on the slave ARM) registers one program per DSP slot with the slave
  OS (`+0x28` slot, `+0x40` program, `+0x58` data descriptors) and starts / swaps slot programs with `+0x70`. DSP2
  and DSP3 each have an oscillator slot DCO1 at DSP address `0x6000` and DCO2 at `0xB000`; each DSP computes two
  voices (A and B), so the two slaves × 2 DSPs × 2 voices = 8 voices.
* The waveform parameters choose the program: DCO1 SAW / PULSE → `dco1_saw` (PULSE adds a flag), DCO1 SQR →
  `dco1_sqr`; DCO2 SAW / PULSE → `dco2_saw`, SQR → `dco2_sqr`, NOISE → `dco2_noise`.
* `engine/programs.py` copies Roland's programs from the JX-3P at build time; the environment variable
  `MC_TEST=prefix=file,...` replaces every program whose name starts with `prefix` by a text file of words. The
  registered size (`MC_*_BYTES`) follows the file, which matters: **a program longer than the registered one does not
  run completely** (seen when poking it live over a smaller test program), so a new program must be *installed*.

## 2. Slot interface (inputs, outputs, state)

Taken from Roland's SAW programs (disassembled with `re/dspisa.py dis`) and confirmed by the result:

| | DCO1 voice A | DCO1 voice B | DCO2 voice A | DCO2 voice B |
|---|---|---|---|---|
| pitch input | `[2201]` | `[220e]` | `[2207]` | `[2214]` |
| other inputs (not used by us) | `[2203]`–`[2206]` | `[2210]`–`[2213]` | `[2209]`–`[220c]` | `[2216]`–`[2219]` |
| outputs | `[221f]`, `[221b]` | `[2227]`, `[2223]` | `[2221]`, `[221d]` | `[2229]`, `[2225]` |
| our state (phase) | `0470` (read `0471`) | `0480` (read `0481`) | `0670` (read `0671`) | `0680` (read `0681`) |

* DCO2 extra inputs, measured by audio on the lab plugin (noise × input, DCO1 level 0) for the panel's
  **Cross Mod** button (OFF / SYNC / METAL), written by the main program (MAIN2/3: `[2209] <- r3`,
  `[220b] <- r1`, …):

  | input (voice A / B) | OFF | SYNC | METAL | meaning |
  |---|---|---|---|---|
  | `[2209]` / `[2216]` | 0 | 1 | 1 | sync enabled |
  | `[220a]` / `[2217]` | 0 | 0 | 1 | cross-mod (METAL) enabled |
  | `[220b]` / `[2218]` | 0 | pulse train | pulse train | sync pulses: one impulse per DCO1 cycle (flat harmonics at the note frequency) |
  | `[220c]` / `[2219]` | 0 | 0 | 0 | not seen used |

  An own oscillator can implement hard sync by resetting its phase when `[220b]` is non-zero (and `[2209]` = 1).
* Registers are shared by the programs of one DSP within a sample: the DCO2 slot sees what the DCO1 slot left in
  r0–r7 (measured: r0/r3/r4 carry DCO1 signal). Usable for own inter-slot communication; never rely on an
  initial value.
* `22nn` is the mailbox between the DSP's main program and its slot programs. Roland writes two different values to
  the two outputs; we write the same signal to both (it sounds right; their exact roles are not decoded).
* Pitch scale: `p = (note − 12) / 12` plus the modulations the main program adds (bend, range, detune, LFO / ENV
  mod). Roland's code starts with `x = p − 4.75`, so **f = 440 · 2^(p − 4.75)** (note 69 → p = 4.75 → 440 Hz). This
  was confirmed by the measured tuning.
* The 04xx (DCO1) and 06xx (DCO2) ranges are the slot programs' local memory (Roland's SAW uses `0600`–`0681`).

## 3. Machine facts the program depends on (all measured, see `DSP_ISA_SPEC.md`)

1. **96 kHz**: the program runs once per sample at 96 kHz.
2. **Ring memory**: data references are relative to a pointer that advances one word per sample. A value stored at
   reference S is read at **S + 1 in the next sample**. Persistent state = load from S + 1, store to S.
3. **Load latency**: an ALU op in the instruction right after a load sees the old register value; one instruction in
   between is enough.
4. **Registers do not persist** between samples (other programs on the DSP use them): constants are reloaded every
   sample.
5. All ops of one instruction read their operands before any write; a store in the same instruction as an op stores
   the old value.

## 4. Algorithm

Per voice, per sample:

```
x     = p − 4.75                              # octaves from A4
y     = x · ln2 / 16
e     = 1 + y·(1 + y·(1/2 + y·(1/6 + y/24)))  # e^y, Taylor degree 4, Horner
inc   = e^16 · (2 · 440 / 96000)              # four squarings: e^16 = 2^x ; phase moves 2 per period
phase = phase_prev + inc                       # phase in [−1, 1)
q     = max(phase − 1, 0)
q     = min(256·q, 1)   (4 times)              # 0 if phase < 1, exactly 1 if phase ≥ 1 (see below)
phase = phase − 2q                             # wrap
out   = phase
```

* **2^x accuracy**: |x| ≤ 6 covers every note; the Taylor polynomial on |y| ≤ 0.26 followed by 4 squarings has a
  worst-case error of 0.34 cents at x = −6 (0.1 cents at the lowest MIDI notes, < 0.002 cents within ±2 octaves of
  A4). The measured ±1.2 cents is the resolution of the FFT measurement.
* **Branch-free wrap**: the ISA has no compare/branch and the condition words (C) test static context bits, not
  data. With q₀ = max(phase − 1, 0): if phase < 1, q stays 0. If phase ≥ 1, q₀ ≥ 2⁻²⁸ (the RAW resolution for
  values in [1, 2)), and each `min(256·q, 1)` multiplies by 256 until it saturates: 2⁻²⁸ · 256⁴ = 16 → after four steps
  q = 1 exactly. Clamping at every step keeps every intermediate value ≤ 256 (the format overflows at 512). The only
  inexact case is phase = 1.0 exactly (q = 0, no wrap for one sample), harmless.
* **Range**: inc ≤ 0.26 (note 127), so phase + inc < 2 and one wrap per sample is always enough; phase stays in
  [−1, 1).
* Output level is ±1 (full scale), like the mailbox values Roland's programs write.

## 5. Register and memory use (per voice)

| resource | use |
|---|---|
| r0 | pitch p, then x |
| r1 | phase |
| r2 | y, then the wrap flag q |
| r3 | e^y, 2^x, increment |
| r5 | constant 0 (for `max`) |
| r6 | constant 1.0 (for `min`) |
| `[S+1]` / `[S]` | previous / new phase (S = 0670 … per table §2) |

Instructions used: unit A `move`/`add`/`sub`/`mul` and op 4 (`Y + X·D`, the Horner step) with 16/32-bit immediates;
unit B `max` (`D=8+d`, op 2) and `min` (op 0) with X in r0–r3 (the only measured unit-B forms); single transfers
(`0c03`, mode `fe`).

## 6. Listing (DCO2 slot; DCO1 differs only in the addresses of §2)

Generated by `python3 re/dspasm.py saw dco2`; "decoded" is `re/dspisa.py`'s reading of each instruction (the
start/end words are copied from Roland's programs). Constants are words 2–7, voice A words 8–94, voice B words 95–181 (same code, its own
addresses), end words 182–186.

| word | instruction | class | decoded (dspisa) |
|---|---|---|---|
| 0 | `0a02 ff70` | `0x050` B | slot program start (copied verbatim from Roland) |
| 2 | `0903 5050 0000` | `0x048` AI | r5 = #0 |
| 5 | `0903 6050 3000` | `0x048` AI | r6 = #1 |
| 8 | `0c03 1fe0 0671` | `0x060` TR | r1 <- [0671] |
| 11 | `0c03 0fe0 2207` | `0x060` TR | r0 <- [2207] |
| 14 | `0904 0c30 c4c0 0000` | `0x048` AII | r0 = r0 + #-4.75 |
| 18 | `0904 2430 0b17 217f` | `0x048` AII | r2 = r0 * #0.0433217 |
| 22 | `0904 3070 0aaa aaab` | `0x048` AII | r3 = #0.04166667 |
| 26 | `0904 38b0 22aa aaab` | `0x048` AII | r3 = #0.1666667 + r2 * r3 |
| 30 | `0904 38b0 2800 0000` | `0x048` AII | r3 = #0.5 + r2 * r3 |
| 34 | `0904 38b0 3000 0000` | `0x048` AII | r3 = #1 + r2 * r3 |
| 38 | `0904 38b0 3000 0000` | `0x048` AII | r3 = #1 + r2 * r3 |
| 42 | `0902 34c3` | `0x048` A | r3 = r3 * r3 |
| 44 | `0902 34c3` | `0x048` A | r3 = r3 * r3 |
| 46 | `0902 34c3` | `0x048` A | r3 = r3 * r3 |
| 48 | `0902 34c3` | `0x048` A | r3 = r3 * r3 |
| 50 | `0904 34f0 0258 bf26` | `0x048` AII | r3 = r3 * #0.009166667 |
| 54 | `0902 1cc1` | `0x048` A | r1 = r3 + r1 |
| 56 | `0903 2c50 b000` | `0x048` AI | r2 = r1 + #-1 |
| 59 | `0a02 a485` | `0x050` B | r2 = max(r2, r5) |
| 61 | `0903 2490 7000` | `0x048` AI | r2 = r2 * #256 |
| 64 | `0a02 a086` | `0x050` B | r2 = min(r2, r6) |
| 66 | `0903 2490 7000` | `0x048` AI | r2 = r2 * #256 |
| 69 | `0a02 a086` | `0x050` B | r2 = min(r2, r6) |
| 71 | `0903 2490 7000` | `0x048` AI | r2 = r2 * #256 |
| 74 | `0a02 a086` | `0x050` B | r2 = min(r2, r6) |
| 76 | `0903 2490 7000` | `0x048` AI | r2 = r2 * #256 |
| 79 | `0a02 a086` | `0x050` B | r2 = min(r2, r6) |
| 81 | `0903 2490 4200` | `0x048` AI | r2 = r2 * #2 |
| 84 | `0902 1e81` | `0x048` A | r1 = r1 - r2 |
| 86 | `0c03 9fe0 0670` | `0x060` TR | [0670] <- r1 |
| 89 | `0c03 9fe0 2221` | `0x060` TR | [2221] <- r1 |
| 92 | `0c03 9fe0 221d` | `0x060` TR | [221d] <- r1 |
| 95 | `0c03 1fe0 0681` | `0x060` TR | r1 <- [0681] |
| 98 | `0c03 0fe0 2214` | `0x060` TR | r0 <- [2214] |
| 101 | `0904 0c30 c4c0 0000` | `0x048` AII | r0 = r0 + #-4.75 |
| 105 | `0904 2430 0b17 217f` | `0x048` AII | r2 = r0 * #0.0433217 |
| 109 | `0904 3070 0aaa aaab` | `0x048` AII | r3 = #0.04166667 |
| 113 | `0904 38b0 22aa aaab` | `0x048` AII | r3 = #0.1666667 + r2 * r3 |
| 117 | `0904 38b0 2800 0000` | `0x048` AII | r3 = #0.5 + r2 * r3 |
| 121 | `0904 38b0 3000 0000` | `0x048` AII | r3 = #1 + r2 * r3 |
| 125 | `0904 38b0 3000 0000` | `0x048` AII | r3 = #1 + r2 * r3 |
| 129 | `0902 34c3` | `0x048` A | r3 = r3 * r3 |
| 131 | `0902 34c3` | `0x048` A | r3 = r3 * r3 |
| 133 | `0902 34c3` | `0x048` A | r3 = r3 * r3 |
| 135 | `0902 34c3` | `0x048` A | r3 = r3 * r3 |
| 137 | `0904 34f0 0258 bf26` | `0x048` AII | r3 = r3 * #0.009166667 |
| 141 | `0902 1cc1` | `0x048` A | r1 = r3 + r1 |
| 143 | `0903 2c50 b000` | `0x048` AI | r2 = r1 + #-1 |
| 146 | `0a02 a485` | `0x050` B | r2 = max(r2, r5) |
| 148 | `0903 2490 7000` | `0x048` AI | r2 = r2 * #256 |
| 151 | `0a02 a086` | `0x050` B | r2 = min(r2, r6) |
| 153 | `0903 2490 7000` | `0x048` AI | r2 = r2 * #256 |
| 156 | `0a02 a086` | `0x050` B | r2 = min(r2, r6) |
| 158 | `0903 2490 7000` | `0x048` AI | r2 = r2 * #256 |
| 161 | `0a02 a086` | `0x050` B | r2 = min(r2, r6) |
| 163 | `0903 2490 7000` | `0x048` AI | r2 = r2 * #256 |
| 166 | `0a02 a086` | `0x050` B | r2 = min(r2, r6) |
| 168 | `0903 2490 4200` | `0x048` AI | r2 = r2 * #2 |
| 171 | `0902 1e81` | `0x048` A | r1 = r1 - r2 |
| 173 | `0c03 9fe0 0680` | `0x060` TR | [0680] <- r1 |
| 176 | `0c03 9fe0 2229` | `0x060` TR | [2229] <- r1 |
| 179 | `0c03 9fe0 2225` | `0x060` TR | [2225] <- r1 |
| 182 | `0a02 ff70` | `0x050` B | end marker part (verbatim) |
| 184 | `0b02 00f8` | `0x058` C | end of program (verbatim) |
| 186 | `0001` | `0x000` ? | final no-op (required) |


## 7. Build, install, test

```bash
cd examples/from_scratch
python3 re/dspasm.py saw dco1 -o re/dspexp/own/saw_dco1.txt
python3 re/dspasm.py saw dco2 -o re/dspexp/own/saw_dco2.txt
python3 re/dspasm.py sim re/dspexp/own/saw_dco2.txt --pitch 4.75 --n 1600      # offline (no ring model; see note)

R=$PWD/re/dspexp/own
# both oscillators ours (PEEK=1 keeps the master peek of region 0 for debugging):
PEEK=1 MC_TEST="dco1_saw=$R/saw_dco1.txt,dco2_saw=$R/saw_dco2.txt" engine/build.sh ../../plugins/JX-3P ../../plugins/C-SAW
# comparison build: DCO-1 ours, DCO-2 Roland's:
PEEK=1 MC_TEST="dco1_saw=$R/saw_dco1.txt" engine/build.sh ../../plugins/JX-3P ../../plugins/C-SAW
cd ../.. && python3 tools/plugout_tool.py install plugins/C-SAW --slot 3 --yes --force
# select PLG3 on the panel; INIT patch, DCO-1 (and/or DCO-2) SAW, cutoff up, Mod Sens down
cd examples/from_scratch/re && python3 dspown.py notes NAME --notes 36 48 60 69 72 81 96
```

* `dspown.py notes` plays one note per recording (USB audio, 96 kHz) and reports the spectral peak near the
  expected frequency and the harmonic ratios.
* The offline simulator (`dspasm.py sim`, `dspisa`) does not model the ring memory or the load latency: it reads the
  phase from S + 1 as a separate variable, so it is only a check of the arithmetic.
* Hardware check of the arithmetic on master DSP3 (exact read-back): the increment for note 69 is 0.0183333 at
  48 kHz / 0.0091667 at 96 kHz as computed, the wrap of 1.25 gives −0.75, and the ring trail of the running program
  shows the phase stepping by 0.0092 per sample.

### 7.1 Lab plugin: write programs live, no reinstall

A live poke only runs completely if the program is not longer than the one registered at install time. The lab
plugin registers a 2048-word program (`re/dspexp/own/lab_pad.txt`: noise to both voices, padded with no-ops) in the
DCO2 slot of DSP2 and DSP3, silences DCO1 and carries the master peek (`PEEK`) in region 0:

```bash
cd examples/from_scratch/re
O=$PWD/dspexp/own; A=$PWD/dspexp/audio
DSPLIVE=1 PEEK=1 python3 -c "import dspaudio as da; da.build('dco1_=$A/z_dco1.txt,t0=$O/lab_pad.txt,t1=$O/lab_pad.txt'); da.install()"
# (then reset work/dsplive/state.json to the lab program, see NOTES.md) and select PLG3 once
python3 dspown.py load dspexp/own/fold_dco2.txt      # any program <= 2048 words, in seconds
python3 dspown.py notes NAME --notes 48 69 81           # measure
python3 dspown.py restore                               # back to the lab program
```

Verified 2026-10-15: SAW (187 words) and FOLD (352 words) poked live into the lab plugin sound as installed
(SAW harmonics 1/n; FOLD brightness cycling every ~4 s with the internal LFO). Programs for the lab must use the
DCO2 addresses (`dspasm.py saw|fold dco2`).

## 8. Results (2026-10-15, C-SAW with both SAW programs ours)

| note | measured | equal temperament | error | harmonics 1…5 |
|---|---|---|---|---|
| 36 | 65.38 Hz | 65.41 Hz | −0.8 c | 1.00 0.50 0.33 0.25 0.20 |
| 48 | 130.75 Hz | 130.81 Hz | −0.8 c | 1.00 0.50 0.33 0.24 0.19 |
| 60 | 261.62 Hz | 261.63 Hz | 0.0 c | 1.00 0.50 0.33 0.25 0.20 |
| 69 | 440.00 Hz | 440.00 Hz | 0.0 c | 1.00 0.50 0.33 0.25 0.20 |
| 72 | 523.38 Hz | 523.25 Hz | +0.4 c | 1.00 0.50 0.33 0.25 0.20 |
| 81 | 880.25 Hz | 880.00 Hz | +0.5 c | 1.00 0.50 0.33 0.25 0.20 |
| 96 | 2094.5 Hz | 2093.0 Hz | +1.2 c | 1.00 0.49 0.32 0.23 0.18 |

Patch: INIT, DCO-1 SAW, cutoff open, **Mod Sens down** (with Mod Sens up the panel LFO modulates the pitch of every
oscillator, Roland's too — a patch setting, not a fault).

## 9. Pitfalls met on the way

| symptom | cause | fix |
|---|---|---|
| silence (INIT patch) | phase loaded from S and stored to S: with ring memory the load reads another word, the phase never advanced (DC, removed by the output filter) | load from S + 1 |
| phase stuck in tests | ALU op right after the load used the old register | one instruction between load and use |
| frequency off by 2 | the DSP runs at 96 kHz, not 48 | inc = 2f / 96000 |
| partial program / noise when poked live | program longer than the registered test program | install it (registered size follows the program) |
| whole instrument silent | earlier experiments wrote data memory of the master DSP4 global-effect task | power cycle; test only on master DSP3 |
| offline simulation exploded | the simulator skipped every `0a02` header, which is also the unit-B header | skip only the exact start/end words |

## 10. Next steps

1. Anti-aliasing (polyBLEP-style correction around the wrap, or Roland's 4-sub-sample scheme).
2. PULSE (read the flag Roland's SAW program uses), SQR, and the other slot inputs (`[2209]`… : sync, cross-mod,
   PWM).
3. Output scaling / the meaning of the second output word.
4. Own main programs (envelopes, LFO, filter, VCA) to remove the last copied Roland microcode.

## 11. Roland's oscillators compared (corpus, 2026-10-15)

Disassembled with `re/dspisa.py dis ../../work/corpus/<name>.bin` (corpus local, not in git). All models use the
same technique: exponential pitch (SFU `8f19` exp split + polynomial), reciprocal by seed + Newton/Goldschmidt
(`cf06`, `0902 4086`), the discontinuities smoothed with a sine-polynomial edge, 4 sub-samples per output sample.
They differ in how many edges per voice they compute:

| model / program | instructions | edges per voice | decoded |
|---|---|---|---|
| JX-3P SAW (DCO slot) | 325 | 1 | 291 / 325 (90 %) |
| SH-101 (oscillator inside main program `SH_0100_60a12bc0`) | 610 | 2 (saw + pulse) | ~90 % |
| Jupiter-4 DCO1 (`JP4_6000_60a1c450`) | 616 | 3 (no −4.75 constant) | ~90 % |
| Juno-106 / Juno-60 | — | 3 | 88–94 % |
| SH-1000 | — | 1 | 88–94 % |

The undecoded rest is the same everywhere: non-`fe` transfer modes (`X9bY`, `Xa9Y`: 5.1 % of the corpus), the
program start/end words and a few unit-B / SFU ops (`cf16`, `cf26`, `8f09`). None of them is needed for an own
oscillator (§4–§8 work without them).

## Wavetable from a single-cycle WAV (2026-10-16) — `re/wav2dco.py`, `re/dspasm.py wavetable()`, C-WAVE

* No indexed reads exist, so the N table points are hardcoded as immediates and every point is evaluated each
  sample. Linear interpolation written as a sum of ramps: `out = c + Σ s_k · max(u, k)`, `u = N·(phase + 1)/2`,
  `s_k` = change of slope at point k, `c = v0 − Σ s_k·k`. Per point and voice: unit B `max(r0, imm16 k)` (as
  Roland's `d550`) + unit A op 3 `r3 += r2 · imm32 s_k` → **2 instructions**. Phase/pitch code = the SAW's.
  Accumulator stays below ~4N + 3 (format limit 512) → N ≤ 120.
* `python3 re/wav2dco.py cycle.wav [-n 32] [--cycle 2048 --frame K] -o prog.txt [--check]`; PCM and float WAV.
  Cycle FFT-resampled to N points (band-limited), DC removed, peak = 0.9. Live: `python3 re/dspown.py load
  prog.txt` (lab, DCO2 slot). Check on the device: `python3 re/wtcheck.py NAME cycle.wav -n N` (scale, harmonics
  1–10 against the table).
* **C-WAVE PLUG-OUT** (`NAME=C-WAVE WAVETABLE=1 OWN_PROGS=dco1_w0=…,dco2_w5=… engine/build.sh`): six own programs
  per DCO, one per waveform position (`re/dspexp/own/wt/dco{1,2}_w{0..5}.txt`). DCO2: AKWF eorgan, epiano,
  clavinet, hvoice, fmsynth, vgame (32 points). DCO1: Echo Sound Works frames with 32/48/64/84/100/120 points.
* **Time budget, not memory**: the first version (one op per instruction, 5 per point) failed on **voice B** of
  every DSP (the second half of the program; alternate notes of a scale sound something else) — live in the lab
  above 401 instructions in DCO2 (DCO1 empty), installed already with DCO1 401 + DCO2 391. Roland's DSP2/3 load
  is main 189 + DCO1 325 + DCO2 357 instructions, but with several ops per instruction. The 2-instruction version
  measured on the device (C-WAVE, scale of 12 notes, all match within 0.02): DCO1 198 and 262 instructions with
  DCO2 198. Larger positions: see NOTES.md.
