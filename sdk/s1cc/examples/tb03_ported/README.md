# tb03_ported — the Roland TB-03 voice on SYSTEM-S1CC

State: 2026-11-11. The voice of the Roland TB-03 (Boutique, firmware `BQ2_UPD.BIN` 1.07) as an s1cc kernel for the
SYSTEM-1 (`examples/system_s1cc`, PLUG-OUT `TB-03-S1`, one voice). Nothing in it is written by ear: the DSP part is
Roland's voice program translated operation by operation (`dsplift.py`, [Vol. V ch. 8](../../../../docs/vol5_toolchain_compiler_sdk/dsplift.md)),
the ARM part is the values Roland's own ARM code writes, recorded in an emulator. There is no TB-03 in the project:
the reference everything is compared with is the lifted program itself.

| file | what it is |
|---|---|
| `tb03.c` | the kernel: panel mapping, the voice's inputs (pitch, gate, accent, slide), DSP A / DSP B |
| `tb03_voice.h` | the TB-03 voice program as C (generated, §2; do not edit) |
| `tb03_names.txt` | what is known of the voice program: names of parameters, inputs, cells, values, sections (§2; hand-written) |
| `tb03_tables.h` | the ARM side as data: parameter init, knob tables (generated, §3; do not edit) |
| `tb03_import_ranges.txt` | the measured ranges of the values DSP A hands to DSP B (§2) |
| `tools/tb03_extract.py` | `BQ2_UPD.BIN` → decoded `BQ2:Appli` image and the DSP programs as hex (§1) |
| `tools/make_voice.sh` | the DSP program → `tb03_voice.h` (§2) |
| `tools/tb03_arm.py` | the ARM image → `tb03_tables.h` (§3) |

## 1. Source

`BQ2_UPD.BIN` = "Boutique2 System Program 107" (TB-03 1.07, 2021-01-21). Its module `BQ2:Appli` (0x40-byte header +
LZSS) is the ARM image (base 0x60000000); it carries the DSP programs as the SYSTEM-1 does, `{address, bytes,
pointer}` program sets, halfwords upper first ([Vol. V ch. 8 §5](../../../../docs/vol5_toolchain_compiler_sdk/dsplift.md)).

```sh
python3 sdk/s1cc/examples/tb03_ported/tools/tb03_extract.py BQ2_UPD.BIN        # -> work/tb03 (not in git)
```

| output | content |
|---|---|
| `work/tb03/BQ2_Appli.bin` | the decoded ARM image (SHA-256 checked) |
| `work/tb03/progs/v_0022.hex` | **the voice** (voice DSP set `0x6014e184`, program at byte `0x0022`, 751 instructions) |
| `work/tb03/progs/b_*.hex` | the second DSP (set `0x6015006c`): a list of calls, post-processing, effects, output |
| `work/tb03/progs/fx*_*.hex` | effect variants |

The voice program reads parameters `C000…C03E`, the per-note inputs `A003…A007` and `A104…A109` (from the second
DSP), and writes `A100…A103`; `A100` is the voice output.

## 2. The voice program (`tb03_voice.h`)

```sh
sh sdk/s1cc/examples/tb03_ported/tools/make_voice.sh [BQ2_UPD.BIN]
```

runs `dsplift.py --bq2 --kernel --split 415 --zero-cells 0bd,0bf --import-ranges tb03_import_ranges.txt
--names tb03_names.txt --consts …` ([Vol. V ch. 8 §6](../../../../docs/vol5_toolchain_compiler_sdk/dsplift.md)):

| option | why |
|---|---|
| `--bq2` | the Boutique DSP: implicit flags, control words |
| `--kernel` | straight-line C for s1cc: registers as values, the bank-0 ring as delay chains `D_x_k`, conditions as 0 / 1 floats blended in (no compares) |
| `--split 415` | 751 instructions do not fit one slave DSP (703 / 708): instructions 0…414 become `tb03_voice_a` (DSP A), 415…750 `tb03_voice_b` (DSP B). the values B needs from A cross with `s1_export` / `s1_import` (`TB03_VOICE_SLOTS`, 9 with the TB-03 setup); a value that is a constant at the end of A (or measured constant, `--import-ranges`) is written into B instead |
| `--zero-cells 0bd,0bf` | two bank-0 cells part A reads that only part B writes (a value from B back to A, which SYSTEM-S1CC has no path for); both are 0 with the TB-03's parameters |
| `--import-ranges` | the range of every handed value, measured on the lift (a held constant stays a constant, the rest give s1cc its range) |
| `--names tb03_names.txt` | the names the C uses: the panel parameters (`knob`), the inputs, the output, the outputs `tb03.c` ignores (`unused`: not computed), the inputs that are constants in `tb03.c` (`const A003 1`, `A002`, `A104…A109` = 0), the bank-0 cells and temporaries by what they compute, the section headings |
| `--consts` | the parameters no knob moves, with the TB-03 init's values (`tb03_tables.h` `tb03_init`, made by `make_voice.sh`): the parts they switch off (a third oscillator, an FM / ring matrix, an LFO, the second input set `A104…A109`) fold away |

The result is every operation of Roland's program that reaches the output, in order, as named C expressions with a
section heading per block (glide, VCA envelope, filter envelope, cutoff map, saw oscillator, saw shaping, square
oscillator, mixer, filter, output); the number after each line is the instruction of Roland's program that makes the
value. Delay chains read as `NAME_zK` (cell NAME, K samples ago: the 20-tap decimation FIR is `saw_s0_z1 … saw_s3_z3`).
`tb03.c` supplies the memory through the macros `KP` (parameters, §3), `KIN` (inputs, §4), `KOUT` (only `A100`).

Names are read from the lift: a name says what the value is computed as (`svf_bp`, `fenv_coef`); where the musical
role is not measured it stays technical (`sq_leak`, `sq_slew`). To name more: add `cell` / `value tK` lines to
`tb03_names.txt` and run `make_voice.sh` again (the C changes only in names; a `value` that no longer exists is a
warning).

Checks [native, 2026-11-11] (the previous per-instruction `tb03_voice.h` against this one, clang, no contraction,
100 000 samples, knobs / notes / gate / accent / slide changing): the output `A100` is bit-identical in the TB-03
setup; without `--names` / `--consts` all five outputs are bit-identical also with every parameter and input random.

What the program is, read from the lift: pitch with glide (slide), two oscillators (saw, square: `C029` / `C02A`),
each computed at four sub-samples per sample with a rounded edge and decimated by the same 20-tap FIR, a
cutoff / resonance dependent shaping filter after each, a state-variable filter with its envelope (ENV MOD, DECAY,
accent sweep), a fixed two-pole output stage, the accent, the VCA envelope. A third oscillator, an FM matrix and an
LFO are in the program but off (their parameters are 0 after the init, no knob moves them): `--consts` removes them
from the C.

## 3. The ARM side (`tb03_tables.h`)

```sh
pip install unicorn
python3 sdk/s1cc/examples/tb03_ported/tools/tb03_arm.py work/tb03 -o sdk/s1cc/examples/tb03_ported/tb03_tables.h
```

The TB-03's ARM code is C++ with soft float; instead of reading it, its functions are run in an ARM emulator
(image at 0x60000000, ITCM copy `0x60163b4c` → `0x01000300` length `0xd710`, SRAM data `0x601712a8` → `0x20000000`
length `0x560`, as its start code does) and every write to the voice DSP's parameter window (`0x4016c000`, 16 bytes
per parameter, the value at +4) is recorded:

| table | from |
|---|---|
| `tb03_init[0x40]` | the voice object's constructor `0x6001da18` and init `0x6001e138`: every parameter's start value |
| `tb03_tune`, `tb03_cutoff`, `tb03_resonance`, `tb03_envmod`, `tb03_accent` (256), `tb03_decay` (128) | the knob handlers (dispatcher `0x6001d520`) called with every knob value: TUNE `0x6001e108` → `C010`, CUTOFF `0x6001defe` → `C019`, RESONANCE `0x6001ded0` → `C01A`, ENV MOD `0x6001de20` → `C01B`, DECAY `0x6001de86` → `C00D`, ACCENT `0x6001ddf2` → `C01C` |

## 4. Panel and inputs (`tb03.c`)

| TB-03 | SYSTEM-1 | parameter |
|---|---|---|
| CUTOFF | FILTER CUTOFF | `C019` = `tb03_cutoff[knob]` |
| RESONANCE | FILTER RESONANCE | `C01A` |
| ENV MOD | FILTER ENV AMOUNT (0…255, not bipolar) | `C01B` |
| DECAY | FILTER KEY TRACK (`0x186`) | `C00D` = `tb03_decay[knob >> 1]` |
| ACCENT | OSC1 COLOR with OSC1 COLOR MOD (`s1_color(0)`) | `C01C` (the accent's strength; the TB-03's table is knob / 255, so the color is the value) |
| TUNE | OSC2 FINE (128 = in tune, ±1 octave) | `C010` |
| WAVEFORM | OSC1 WAVE (position 0 saw, 1…5 square) | `C029` / `C02A` = 1 / 0 or 0 / 1 |
| accent of the notes (the TB-03's per-step accent) | OSC2 COLOR with OSC2 COLOR MOD (`s1_color(1)`, 0 none … 1 full) | input `A006` |

| input | value |
|---|---|
| `A003` | 1: selects the note inputs `A004…A007` (0 would select `A104…A107`, the same inputs written by another ARM path; the voice crossfades the two by `A003`, instructions 99–111) |
| `A004` | pitch in octaves, `s1_pitch()` ((note − 12) / 12) |
| `A005` gate | the gate word, 0 / 1 |
| `A006` accent | `s1_color(1)`, taken when a new gate starts (held for the note) |
| `A007` slide | a new note while the gate stayed on (legato, `MONO_LEGATO=1`: the host keeps the gate on and changes the pitch), until the gate closes (`0x6001e376`) |
| `A104…A109` | 0: with the TB-03's parameters they do not change the output (measured on the lift) |

**Accent from OSC2 COLOR, not velocity (2026-11-11).** The TB-03 sets a note's accent at velocity ≥ 100 (its ARM,
`0x6001d4ae`), and its voice program then also forces the filter envelope to a fixed short decay (`C00D` replaced by
0.1865, instructions 137–143): accented notes ignore DECAY, as on a TB-303. The SYSTEM-1 keyboard has no velocity
(it sends a fixed value above 100), so with the velocity rule every note was accented and DECAY did nothing
(measured on the device: DECAY at maximum, velocity 90 → the filter sweep lasts ~1 s; velocity 110 → closed in
100 ms). The accent is therefore the OSC2 COLOR knob, latched at each new gate (`GATE_ACCENT` is no longer used).

**COLOR MOD (2026-11-11).** Both are `s1_color` (examples/system_s1cc/README.md §3.2): color = knob × the source of
its COLOR MOD selector (0…1). OSC2 COLOR MOD = LFO gives accents that vary from note to note (the value at the note's
start); the envelopes are ~0 when a note starts, so for the note accent only MANUAL and LFO are useful. OSC1 COLOR
(ACCENT strength) is read every sample, so any source modulates it. Cost: the host's color machinery (LFO, pitch
envelope, sub oscillator, two ADSRs on DSP B, `MOD_DSP=auto` → B): DSP A 516, DSP B 462 instructions (budgets
703 / 708).

Gate, accent and slide are made from 0 / 1 arithmetic (`s1_sign`, `fminf`, `fmaxf`), not compares: a compare the
profiling never sees flip would be folded by s1cc.

## 5. Not ported

The second DSP (overdrive, delay, the output stage) and the sequencer. DSP B returns `A100 × TB03_GAIN` (12).

## 6. Build, check, install

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/tb03_ported/tb03.c -o sdk/s1cc/build/tb03 --check
KERNEL=sdk/s1cc/build/tb03 VOICES=1 sh examples/system_s1cc/build.sh
python3 tools/plugout_tool.py install plugins/TB-03-S1 --slot 1 --force --no-patches -y
```

`S1_BUILD("NAME=TB-03-S1 MONO_LEGATO=1 MIX=0")`, `S1_COMPILE("--no-fused --regs 0,1,2,3,4,5,6,7")`.
The s1cc driver finds `dsplift_k.h` (`sdk/s1cc/tools`) by itself.

| check [sim, 2026-11-11] | result |
|---|---|
| `s1cc check` (note 57, knobs 128) | DSP A 479 instructions, DSP B 438 (budgets 703 / 708); B against the native C: 68.4 dB (first 10 ms), 48.7 dB (all), bands ±0.00 dB |
| `s1cc check`, the named `tb03_voice.h` (2026-11-11) | DSP A 468 instructions (497 with the per-instruction form), DSP B 410 (427); 9 values handed A → B (was 24); B against the native C 77.9 dB (10 ms), 65.4 dB (all), the same as the per-instruction form |
| port against the lifted program (`dsplift.py --bq2`, doubles), sustained notes | 76–80 dB |
| port against the lifted program, 16-step sequences with accent and slide | 34–48 dB, bands within ±0.1 dB; the rest is the oscillator edges (one sample earlier or later under float rounding) |
| exports | the values handed A → B arrive one sample late (the slave ring): a pure one-sample delay of part B |

On the device: earlier builds installed and played (slot 1); the build with the envelope fix (s1cc `mulLV`,
2026-11-11: DECAY and ENV MOD had no effect before) is not heard on the device yet.

## 7. Possible improvements (not done)

State 2026-11-11: TB-03-S1 runs with the voice in two parts (DSP A 468, DSP B 410 instructions) and both oscillators
computed every sample; WAVEFORM only sets the levels `C029` / `C02A` (0 / 1), as on the TB-03.

**The voice on one DSP, WAVEFORM by changing program.** The whole program on one DSP does not fit: 804
instructions (budget 703). One waveform alone fits [s1cc check, 2026-11-11: the lift without `--split`, `C029` /
`C02A` as constants]:

| voice on one DSP | instructions | budget | SNR against the native C |
|---|---|---|---|
| saw | 522 | 703 | 77.9 / 65.4 dB |
| square | 684 | 703 | 81.9 / 61.3 dB |
| both (as now) | 804 | 703 | 77.9 / 65.4 dB |

How Roland's PLUG-OUTs choose a waveform [code]: the JX-3P (SYSTEM-8) has one program per waveform and swaps the DCO
slot's program with the slave OS service `+0x70` when the knob moves ([Vol. III ch. 3](../../../../docs/vol3_voice_allocation_scheduling/slot_program_model.md)
§2.3, [Vol. VI ch. 6](../../../../docs/vol6_reference_designs_applications/custom_osc.md) §1); the stock SYSTEM-1
synth has 24 voice program variants placed in sections with `FC40` (waveform combinations: inferred); the SH-2
computes every shape it mixes (its panel mixes them, it does not choose). So: when a waveform is chosen and the time
is short, Roland loads another program.

Plan:
1. A probe on the device first: does `FC40` (re-place a section) work while the sound runs, from a parameter handler
   (so far it is only used in `load`)? A TEST PLUG-OUT that alternates two small programs on a knob.
2. If it does: either the whole voice per waveform in section 2 of DSP A (the host on DSP B; a waveform change
   restarts envelopes and filter, as a new note; the square program, ~5.7 KiB, is close to the 6 KiB of a section:
   6534 bytes hung, 5854 ran), or, as the JX-3P, only the oscillators on DSP A (saw or square program) and filter,
   envelopes, VCA on DSP B (a clean change: only the oscillator restarts; a different split than `--split 415`).
3. Build: two kernels (or one with two A programs); `make_s1cc.py` would need an option for an alternative DSP A
   section program, `engine.c` a `FC40` + `FC44` on the WAVE knob.

**Mixing the waveforms.** `C029` / `C02A` are continuous in Roland's program; `S1_PARAM(wave, …)` could cross-fade
saw and square (the TB-03 only writes 0 / 1). One line in `tb03.c`.

**More names.** `tb03_names.txt` names what was read from the lift; the square oscillator's inner values
(`sq_leak`, `sq_slew`, `sq_lim`, `sq_thr`) and the cutoff map's polynomials (instructions 200–271) are named by their
computation only; measuring them on the lift would give them musical names.
