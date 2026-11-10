# Volume II — Chapter 9: What the Decoded Operations Are Good For

> **Scope:** both (same ISA); measured on the SYSTEM-8.

> The exact (bit-level) behaviour of the SFU operations is in [Vol. II ch. 1](core_architecture.md) §5.3 and
> [ch. 5](sfu_and_accelerators.md); this chapter is about what each operation is used for.

Practical guide for whoever writes their own microcode (or the future compiler / SDK). **The specification that rules
is [Vol. II ch. 4](instruction_set_reference.md)**; here it is only summarised what can be built with each thing, with minimal sequences as they
were measured. Each section says **where it was measured** (DSP and context). What is dated 2026-10-16 is verified by
reading memory with the peek (`re/dsptap.py`, `re/dsprdly.py`, `re/dspidx.py`, `re/dsphreg.py`, `re/dspidxring.py`,
`re/dspbus.py`, `re/dspbus2.py`); on 2026-10-17 the indexed delay (§2) and indexed bank 5 (§2b) were heard on the
slaves; the delay line of §1 has not been heard yet. History and raw evidence: [NOTES.md](../vol7_cleanroom_evidence_archive/NOTES.md) ("Tomas de retardo",
"Registros ocultos").

Rules that apply to everything (from [Vol. II ch. 4](instruction_set_reference.md)):
* ≈ 925–945 instructions per sample and per DSP (96 kHz); an A/A/B instruction costs one cycle. Going over can leave
  the device unstable until it is switched off.
* A load needs one instruction in between before an op uses the register.
* The immediates are integers in the `40TY` words (value × 16384) and RAW numbers in the A/B/MAC ops.

---

## 1. Delay line (bank-2 DMA) — *measured: master DSP4, slot 0xF7DC*

**What it does.** Every sample one sample is written into the delay memory and a read of another one from N samples
ago is requested (with a fractional part). The hardware returns two neighbouring samples and the fraction, ready to
interpolate.

**Uses.**
* Delay / echo (with feedback: add the scaled tap to the input before writing).
* Chorus / flanger: delay modulated by an LFO; the fraction gives sub-sample interpolation without "zipper".
* Feedback and feed-forward comb filters, Schroeder all-pass → reverb blocks.
* Karplus-Strong (plucked string): delay = period, feedback with the average of two samples (the two neighbouring
  samples the tap already delivers serve as a low-pass filter).
* Multi-tap: several taps per sample (Roland uses two: `[4301..]` and `[4304..]`). **Measured 2026-10-17: 24 taps per
  sample work** (each with its `0b03 4050 <dest>`, results in triplets `[4301+3k..]`; `re/dspmtap.py`), the limit did
  not show up.
* Idea: two-tap pitch shifter with crossed ramps.
* Long, sample-exact delays: measured up to 8.6 s (u = 50.648).

**Minimal sequence** (per sample; `W = 4300` is the window of this slot):

```text
8fe0 4300 ...        [4300] <- r0                  writes the input (any fe store; Roland: 1b06 c132 8fe0 4300 …)
1d06 4022 7fe0 c1fc 5100 0000                      h4 (base) = 0x510000/16384 = 324 (= write position → delay = u)
                                                   (the load r7 <- [c1fc] is Roland's; the 4022 skips those 2 words)
0903 3050 <imm16>    r3 = u                        delay in units of 16384 samples (u = samples/16384)
0a02 bf6c            r3 = (r3 + h4) mod 512        read address
0b03 4050 0301       h5 = 0x301                    the results will go to [4301..4303]  (mandatory!)
0c02 3a90            tap request with the address in r3   (or Roland's 1b04 c510 3a90 0008)
0c03 0fe0 4301 …     r0 <- [4301] (x[n-L-⌊k⌋]), r1 <- [4302] (one sample older), r2 <- [4303] (fraction)
0001                 load latency
0803 0c0a 9281       r0 = r0 + r2·(r1 − r0)        linear interpolation (MAC)
```

Total delay = 16384·u + L samples, with L = 2 (own short program) or 3 (Roland's program): it depends on the position
inside the program. Base 324 ⇒ the delay is directly u.

**Limits / care.**
* Only slot `0xF7DC` of the master's DSP4 is known to be wired: using it **replaces Roland's global delay**. On DSP3
  none of this has any effect. It is not known yet how to get a window in another slot or on the slaves (where the
  choruses of the J60/JX run).
* The write of `[4300]` lands at position 324 (fixed, set by the ARM); a base smaller than 324 reads "the future"
  (zeros). Length of the line unknown: u = 100 no longer returns anything.
* **The fraction has 6 bits (1/64), truncated** (2026-10-17: 0.4 → 25/64, 0.8 → 51/64, 0.2 → 12/64): the linear
  interpolation of the tap has steps of 1/64 sample.
* Reading the whole WMEM (0x41300000) from the ARM with an own program in this slot hung the master once.

---

## 2. Indexed transfers `X28Y` + index h6 — *measured: master DSP3 (exact)*

**What it does.** `0b02 4080` copies r0 to the index register h6; `X28Y` loads (X 0–7: rX ← bank0[⌊h6·16384⌋ + Y]) or
stores (X 8–15: bank0[…] ← r(X−8)). Truncated index, modulo 0x800, without adding the R word. Several fit per
instruction (`1e04 0281 1282` → r0 = [h6+1], r1 = [h6+2]). **It is relative to the ring** like any bank-0 reference
(`re/dspidxring.py`: a counter with load `[h6+1]` / store `[h6]` counts 96 000/s; with `[h6]`/`[h6]` it does not
count).

**Main use: variable delay line inside bank 0, on any DSP (also the slaves, without DMA).** Since the ring advances one
word per sample, an indexed load of `[S + k]` returns what was stored at `[S]` k samples ago. Storing the input at
`[S]` every sample and choosing k with h6 gives a delay of 1 to < 2048 samples (< ~21 ms at 96 kHz), modulable sample
by sample:
* chorus / flanger / vibrato (k modulated by an LFO),
* short comb and all-pass filters, Karplus-Strong for high notes (period < 2048 samples ⇒ notes > ~47 Hz),
* linear interpolation with two neighbouring loads `[h6]`, `[h6+1]` and the fraction computed apart from the index
  (the index is truncated).
It is probably what Roland does in the S-100 (`r0 <- [a024]`, `0b02 4080`, `0c02 0280`, with h6 taken from a
parameter).

**Minimal sequence** (input in r1, k = delay in samples in r0 as k/16384, base S):

```text
0c03 9fe0 <S>        [S] <- r1                    writes the input (normal fe store)
<ops>                r0 = (S + k)/16384           index = reference of the sample of k ago
0b02 4080            h6 = r0
1e04 2280 3281       r2 <- [h6], r3 <- [h6+1]     samples of k and k+1 ago
0001                 latency
<lerp MAC>           out = r2 + frac·(r3 − r2)    (frac = fractional part of k, computed apart)
```

**Limits / care.**
* The range of references S..S+kmax of bank 0 **must be reserved**: no other code of that DSP may store there (Roland's
  programs share the bank and its ring).
* **Fixed tables: use bank 5 (§2b).** In bank 0 a fixed table only survives if **no** program of the DSP stores to
  bank 0: compensating the ring with a sample counter in bank 5 (h6 = T + n), 8 markers lasted 3 s on the master's
  DSP3 (idle); a single ordinary store erased them in < 10 ms (it sweeps the 2048 words). `re/dspb0fix.py`. It only
  works on a whole own DSP with the state in bank 5.
* **Verified by audio on the slaves** (2026-10-17, DCO2 slot, `re/dspb0dly.py`): SAW + SAW delayed 92 samples with
  indexed `[h6]` → note 72 comes out an octave higher (1045.9 Hz: the fundamental cancels); with k = 0 the SAW returns
  (523.6 Hz). h6 and mode `28` work the same on the slaves.

---

## 2b. Indexed bank 5: `da`/`de` (h6), `db`/`df` (h7), `d0`–`d7` (direct) — *measured 2026-10-17: master DSP3 exact, slaves by audio*

**What it does.** Transfer modes on **bank 5, plain memory (no ring)**, without a reference word:

| word | effect |
|---|---|
| `XdaY` / `XdeY` | X 0–7: rX ← bank5[⌊h6·16384⌋ + Y]; X 8–15: bank5[…] ← r(X−8) |
| `XdbY` / `XdfY` | the same with **h7** (`40c0` = h7 ← r0) |
| `Xd0Y` … `Xd7Y` | bank5[N·16 + Y] (direct address 0x00–0x7F in the word) |

* Truncated index (h6 = 0x10.75 → 0x10); one instruction between `4080` and the load is enough.
* **1024 persistent words**: index mod 0x400 (0x800, 0xC00, 0x1000… are aliases; `re/dspb5alias.py`). With bit 10
  (0x400–0x7FF) what is stored is read only in the same sample and does not stay: Roland uses A400–A427 as
  temporaries. The ARM sees 0x000–0x3FF (DPC*n* RAW) and can write it (on the master).
* Bank-5 limit: **1 load and 1 store per instruction** (with 2 or more stores none stays): two neighbouring reads for
  interpolating = two instructions; filling a table = `move` + `store` per point.
* **Tool ready: `re/wt5.py`** (WAV → table → live send to the slaves → player with interpolation and morph), see
  [Vol. VI ch. 6](../vol6_reference_designs_applications/custom_osc.md) "Real wavetable in bank 5".
* `d8`/`d9`/`dc`/`dd` read addresses that change with time: [unknown], do not use. Other modes (`80`–`8f`, `b0`–`bf`,
  `c0`–`cf`, `f0`–`ff`, `e0`–`ef`) read other things, `90`–`af` and `f5`–`f7` do not load (`re/dspmodescan.py`).

**Use in Roland** (`Annn` refs of the corpus): A000–A047, A200–A205, A300–A303, A4xx. **Free in general: 0x048–0x1FF
and 0x306–0x3BF** (on the master A3C0.. is used by our tests).

**Uses.**
* **True wavetables**: table in bank 5, index = phase · N + base → h6, one load (`0daY`), interpolation with a second
  load `[h6+1]` and the lerp MAC. Cost per voice ≈ 10 instructions, regardless of N (up to ~440 contiguous points),
  against 1 instruction per point in the version with immediates.
* Morph between frames (two reads with h6 and h7 + lerp), several tables per DSP, waveshaping LUTs, curves
  (envelopes, velocity, tuning), sequences.
* Persistent state without the ring: with the state in bank 5 a fixed table in bank 0 is also possible.

**Verified on the slaves by audio** (`re/dspb5osc.py`, DCO2 slot): a 16-point table at A100 → triangle (harmonics 1,
0, 0.111, 0, 0.042 = 1/n² odd) and square (1, 0, 0.315, 0, 0.180 = 1/n odd); with a program that only reads (without
writing the table) the sound stays the same > 10 s: the table persists and voices A and B share it.

**Minimal sequence** (phase r1 ∈ [−1, 1), table of N points at base B):

```text
r0 = r1 · (N/2)/16384                  (A mul imm)
r0 = r0 + (B + N/2)/16384              (A add imm)
0b02 4080                              h6 = r0
0001                                   (one instruction in between)
0c02 2da0                              r2 <- bank5[h6]
0001 0001                              latency
```

**Care.** Filling the table from the DSP (on the slaves the ARM does not reach it): N stores with immediates, once
(skip the block with a jump when it is already full) or spread over several samples. Bank 5 is shared by all the
programs of the DSP: respect Roland's addresses.

## 3. Hidden registers h4–h7 and `40TY` words — *measured: master DSP4 `f7dc` and DSP3*

**What it does.** A word in a C position with high byte `0x40` loads a hidden register: T (bits 7–6) = h4/h5/h6/h7,
mode (bits 5–4) = rY (mod 512) / imm16 / imm24, Y = register or words of the immediate stream to skip. A op 0 with
sub-code 4/5 adds h4/h5: **rD = D + Y + h4** (or h5).

**Uses.**
* h4/h5 as **constants that do not take a register** (nor an immediate on each use): load once per sample and add
  with A op 0 sub 4/5. Careful: h4 and h5 are also used by the delay engine.
* `40TY` in mode 0 copies a register to h4..h7 (with wrap mod 512).
* h6 = index of mode `28` (§2) and of `da`/`de` (§2b). **h7 = index of `db`/`df`** (§2b; `40c0` = h7 ← r0).

**Care.** The default values are left by another task (DSP4: h4 = 288, h5 = 0x105/16384); an own program must set
them before using them. Mode 3 (imm32) loads 0.

## 4. `{8+d}f6c`: add and wrap — *measured: master DSP4 / DSP3*

rd = (rd + h4) mod 512. Besides giving delay addresses, it is an **add-with-wrap** in one instruction (phase
accumulators with period 512, counters that do not saturate). Applied twice: 302.648 → 78.648.

## 5. Persistence of each bank — *measured: master DSP3 (counter load W / store W)*

| bank | behaviour | good for |
|---|---|---|
| 0 (`0nnn`) | ring: what is stored at S is read at S+1 in the next sample | state (load S+1, store S), free short delays (S+k), indexed (§2) |
| 2 (`4nnn`) | two pages alternating per sample (read back 2 samples later) | DMA window of the delay memory (§1) |
| 4 (`8nnn`, MXM) | does not persist between samples | audio/control bus between DSPs (§5b) |
| 5 (`Annn`) | plain persistent memory, the ARM reads it coherently (DPC*n* RAW) | persistent state, small tables, **debug outputs** read by the peek |
| 3 (`60nn`) | not read back; the ARM sees it | I/O (IOMD, §5b), output/debug channel |

Debugging trick: store test results in bank 5 (`A3C0..`, free on the master) and read them from the ARM; single reads
of bank 0 land on other references because of the ring.

## 5b. IOMD and MXM buses — *measured: master DSP3 → ARM / DSP4 (`re/dspbus.py`, `re/dspbus2.py`)*

* **IOMD**: a store `[60nn]` appears in word nn of the IOMD block (0x4113C000 IEEE / 0x4113E000 RAW), readable by the
  ARM. Uses: output / debug channel without going through the ring, and the I/O of the wave bus (where the voices of
  the slaves leave).
* **MXM**: what DSP3 stores in its zone Z (`85nn`) or S (`87nn`) is loaded by DSP4 from its zone O (`83nn`, same nn).
  The M/O zones of DSP3 do not reach DSP4. Uses: **own bus between DSPs**: split an effect over two DSPs (e.g. delay
  in one, reverb in another), send an LFO or a modulation signal to another DSP, sum voices.
* Careful: the latency of the bus is not measured; the map on the slaves and writing from the ARM are not either.

## 5c. Flags: `X9bY` + C — *measured: master DSP3 and DSP4 `efdc` (2026-10-17, `re/dspx9bflags.py`)*
* `X9bY` copies the flags of the result of unit Y of its instruction to group X−8 (9/a/b → cond 4–7 / 8–11 / 12–15):
  c0 = ≠0, c1 = ≥0, c2 = no overflow, c3 = ≤0. One instruction of latency; they persist until rewritten. The B op
  `{8+d}e{0Y}` (bits 7–6 = 00) **compares** (flags of r(d) + Y) without writing registers.
* Uses: **data-dependent selection** with a single instruction (`x12`: runs op 1 or op 2 according to the condition):
  if/else in the DSP without min/max, hysteresis (Schmitt trigger for sync or crossing detection), envelope
  comparators (different attack / release), sample & hold triggered by a crossing, counters with reset (dividers,
  sequencers), detection of a phase overflow (c2), choosing between two branches of a piecewise waveshaper.
* **Jumps and loops** (`009Y`, signed offset in words from the end of the instruction; the C makes it conditional):
  skip whole blocks (saves budget: a branch costs no time), `switch` by mode (like the JX), loops for variable
  filters/orders, summing N samples/voices, iterating Newton until convergence, polynomials of variable degree.
  Confirmed by audio on the slaves (identical SAW with a 4-iteration loop). Careful: load an empty program first
  (`dspown.load` already does it).
* `f5c` = write position of the delay line (counts 1/16384 per sample): delay addresses without depending on h4; it
  also serves as a free sample clock (87 s period).
* Minimal sequence: `cmp` (or any op in unit Y) + `X9bY` → any instruction → instruction with a C. Careful: the group
  stays written for the following programs of the same DSP (Roland rewrites it before using it).

## 6. Other operations, in one line

* **min / max / |x| (unit B)**: selection without jumps, clamps, phase wrap (`dspasm.wrap`), rectification, half-wave,
  threshold detection.
* **SFU**: `cf06` reciprocal seed + Goldschmidt → division; `8f19` exponential split → 2^x, e^x (pitch, exponential
  envelopes); `8f29` sign; `8f39` floor; `cf36` ceiling → quantisation, bitcrusher, integer indices.
* **MAC lerp** (`0803 0c0a 9281`): interpolation, crossfade, wet/dry mix, one-pole filters.
* **MAC with D ≥ 8 → r(D−8) = result/2** (`0803 8c0a 9281`): free half (averages, Karplus-Strong, 50 % mixes, avoiding
  saturation). **op2 = 010 → c = imm16** (`0804 0809 5480 3000` = r0·r1 + 1.0·r5): constant coefficient in 16 bits.
  **Do not** put an A op with an immediate inside a MAC class: it reads the N word.
* **SFU root and log**: `cf16` split for the root (rY ← m ∈ [1,4), r(d+1) = Q[j]/512 ≈ 1/√m, r(d+3) = sign·2^−k →
  √|x| = r(d+3)·m·r(d+1); refine r(d+1) with Newton) → RMS/envelope followers, compressors, equal-power pan/crossfade, quadrature magnitude. `cf26` log2
  split (rY ← m ∈ [0.75,1.5), r4 ← e; a negative Y is read in two's complement) → dB for meters/compressors, frequency → pitch, gain
  in the log domain; together with `8f19` (exp) it gives pow(x, y) and musical curves. `8f09` partial.
* **B ops (D ≥ 8)**: `b242` min(−X, Y) and `b642` max(−X, Y) → symmetric clamps (|x| ≤ a with one op per side
  without negating first), negative rectification; `bc42` **frexp** (X = m·2^e, m ∈ [0.5,1), rY ← e) → normalise
  before a polynomial, automatic level control by powers of 2, count bits / octave of a value.
* **A/A/B in one cycle** (class 0x160): three ops per instruction → wavetables of one point per instruction, chains
  of polynomials.
* **Shared immediates**: the A/B ops of an instruction read one immediate stream, each at its own byte
  ([Vol. II ch. 3](instruction_word_encoding.md) §4.4).
* **Condition flags** (`X9bY` + C, §5c): conditional ops and jumps; for a branch-free choice by data, min/max.

---

## Combined ideas

* **Own chorus/flanger** on the master's DSP4: LFO (phase with `f6c` or wrap) → modulated u → two taps (stereo) →
  lerp → mix with the MAC.
* **Delay with filtered feedback** (tape echo): tap → one-pole (MAC) → saturation (min/max or polynomial) → add to the
  input.
* **Small reverb**: 4 combs + 2 all-pass using several taps of the same line.
* **Karplus-Strong** on the delay line (polyphonic plucked string limited by the number of taps).
* **Chorus / vibrato on the slaves** with the indexed line of bank 0 (§2), without depending on the master's DMA.
* **Waveshaper / LUT and indexed wavetable with morph**: tables in bank 5 (§2b), h6 and h7 for two frames.
* **Compressor / limiter**: RMS with `cf16` → dB with `cf26` → curve → gain with `8f19`.
* **Effect split over two DSPs** through MXM (DSP3 Z/S → DSP4 O), or a common LFO sent over the bus.

## Open

1. §1 (delay line) **by audio**: it is measured exactly with the peek, it remains to hear it on the output.
2. How to obtain a delay window outside `0xF7DC` (ARM configuration) and the real length of the line.
3. MXM/IOMD on the slaves, ARM write into MXM; MAC op2 = 011; rest of `8f09`.
4. imm32 mode of `40TY` (loads 0), other low bytes of `40xx`; modes `d8`/`d9`/`dc`/`dd` of bank 5 (zone 0x400–0x7FF
   of bank 5 = mailbox of the DSP pair in Roland's main programs: [Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §4.4).
5. Bank 5 on the slaves: which addresses the programs of each DSP use (the corpus gives the union). The slave's ARM
   writes it (the DSPLIVE engine stores wavetables there: [Vol. VI ch. 6](../vol6_reference_designs_applications/custom_osc.md)).

> **Note on paths:** `re/…` paths are relative to `examples/from_scratch/`.
