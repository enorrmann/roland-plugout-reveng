# Volume II — Chapter 1: DSP Core Hardware Reference

Single document, without history, for whoever wants to **write an assembler, a compiler or an emulator** for the
DSP. Everything it says is measured on the device unless stated otherwise. Exhaustive detail (class table, examples,
original measurements): [Vol. II ch. 4](instruction_set_reference.md). Work diary: [NOTES.md](../vol7_cleanroom_evidence_archive/NOTES.md).

Certainty: **[exact]** = verified bit by bit or in all cases; **[measured]** = measured on the device,
representative cases; **[inferred]** = deduced from the corpus or from the ARM side, without direct measurement;
**[code]** = read in code; **[unknown]**.

---

## 1. Overview

| item | value | certainty |
|---|---|---|
| Chips | 3 BMC (master + 2 slaves of 4 voices each), identical in hardware | [inferred] |
| DSPs per BMC | 6 (DSP0..DSP5) + DSPM (not studied) | [inferred] |
| Sample rate | 96 kHz; the program of each DSP runs whole once per sample | [exact] |
| Cycles per sample | ≈ 925–945 (≈ 90 MHz) on every DSP of the SYSTEM-8. **SYSTEM-1, slave DSP A: between 703 and 826** (703 works, 826 hangs, also with pure filler); slave DSP B: 708 works, no limit searched ([examples/system_1_synth/README.md](../../examples/system_1_synth/README.md) §4) | [measured] |
| Cost | **1 cycle per instruction**, whatever its content (1 op, A/A/B, 1 or 2 MACs, up to 6 transfers, markers); taken jump +1 cycle; what a jump skips costs nothing | [measured] |
| Going over the budget | the end of the program is lost; on the slaves it leaves the device unstable until it is switched off | [measured] |
| Program memory | SYSTEM-8: up to 64 KiB per DSP (main at `0x0100`, slots at `0x6000` / `0x8000` / `0xB000`). SYSTEM-1: 32 KiB per DSP, in regions: main `0x0100`–`0x18FF` (**6 KiB**), sections 2–5 of ~6 KiB, closing at `0x7FF2` ([Vol. III ch. 3](../vol3_voice_allocation_scheduling/slot_program_model.md)) | [code] |
| Size of the main program (SYSTEM-1) | **6144 bytes**: a main program that reaches `0x1900` hangs the device even with fewer than 703 instructions (6534 bytes / 702 instr. and 6740 / 578 hung; 5854 / 703 ran, 2026-10-28) | [measured] |
| Size of an instruction | 1–31 16-bit words (header included, §4.2): 2–62 bytes; average 6–8 bytes in Roland's programs, ≈ 8–10 in ours (32-bit immediates) | [exact] / [measured] |
| Content of an instruction | up to 3 ops (A/A/B, like Roland) and 6 transfers; the SYSTEM-1 programs also respect the access rules R2–R8 (`asm/s1rules.py`: 1 access to bank 5, ≤ 2 loads from bank 0, …) and only the proven op groups (`asm/proven_forms.txt`) | [measured] |
| Common clock | all DSPs of a BMC share the sample counter (identical delay write position in DSP3 and DSP4) | [measured] |

Each DSP runs **several programs per sample**: a main program and "slots" that the ARM adds (oscillators, effects).
The programs of a DSP **share r0–r7, the flags and the hidden registers**: nothing is reset between programs or
between samples **[measured]**.

---

## 2. State visible to the programmer

| resource | description | certainty |
|---|---|---|
| r0–r7 | 8 32-bit registers in RAW format (§3) | [exact] |
| h4–h7 | hidden registers: h4 = delay base / sum of `A op0 sub4`; h5 = destination of delay taps / `sub5`; h6 = index of indexed transfers; h7 = no known reader. Loaded with `40TY` words (§5.6) | [measured] |
| flags | 32 condition bits = 8 groups × 4 (§6). Groups 1–5 writable; group 0 fixed `0111`; groups 6–7 fixed `1111` | [measured] |
| PC | program counter in 16-bit words; relative jumps (§6.3) | [measured] |
| ring pointer | advances one word per sample; addresses bank 0 (§7.2) | [measured] |
| delay position | global counter that goes down 1/16384 per sample (mod 512); readable with `f5c` (§5.3) | [measured] |

---

## 3. RAW number format **[exact]**

```
bit 31     sign (1 = negative)               value = (−1)^sign · M / 2^(32 − 4·s)
bits 30-29 exponent s (0..3)
bits 28-0  mantissa M (29 bits, unsigned)
```

| s | range of \|v\| | resolution |
|---|---|---|
| 0 | < 1/8 | 2^−32 |
| 1 | < 2 | 2^−28 |
| 2 | < 32 | 2^−24 |
| 3 | < 512 | 2^−20 |

**Arithmetic (verified bit by bit on the master, `re/dspexact.py`)**:
* The ops compute the **exact** result (no intermediate rounding: `D + X·Y`, the MAC with two products plus W, the
  MAC D ≥ 8 = result/2) and encode it **only once**, **truncating toward zero** to the smallest exponent s in which
  the mantissa fits in 29 bits. 48/48 random cases in add, subtract, product, multiply-accumulate, MAC and MAC/2.
* **Overflow**: if |result| ≥ 512, the magnitude is taken **modulo 512** and the sign is kept (300 + 300 → 88;
  256 + 256 → 0 with flag V). Exact in 48/48.
* The logic ops (§5.2) work in fixed-point two's complement (23 fraction bits, range 512) and return the result
  **modulo 512** (a negative reads as 512 + x: words `7ffxxxxx`).
* The ARM sees the memory in three views: IEEE float, Q31 ("SD31") and RAW; the DSP only uses RAW.

---

## 4. Program and instructions

### 4.1 Program image **[exact]**
* Stream of 16-bit words. In program memory each 32-bit little-endian word carries two words of the stream, **the
  upper half first**.
* Slot program: starts with `0a02 ff70` and ends with `0a02 ff70 0b02 00f8 0001` (the final `00f8` is the slot's
  return). Main program of the master's DSP3: `0001 0001 … 0b03 0080 7fee 0001`. SYSTEM-1: starts with `0a02 ff71`,
  same end; program memory ends with a closing of 7 half-words at `0x7FF2` (`0000`, `ld r0,[e000]`, `st r0,[a0xx]`).
  Map of main/slots/sections of both devices: [Vol. III ch. 3](../vol3_voice_allocation_scheduling/slot_program_model.md).
* `0001` = one-word no-op.

### 4.2 Instruction
```
header = class[15:5] | length[4:0]     length = words, header included (1..31)
body   = [A/B/M ops and C/J words] [T transfers] [slot area: N of the MACs, R references, immediates]
```
* The **class** (11 bits) identifies a **template** that says what each word of the body is; it is not a bit field.
  There are 139 known classes (all those of Roland's corpus), each measured or verified on the device
  (`re/dsp_templates.txt`, table in [Vol. II ch. 4](instruction_set_reference.md) §12). A class that is not in the table has to be measured
  (`re/dspslots.py`, `re/dspverify.py`) before using it. **[measured]**
* The ops of an instruction **all read before writing** (r4 = r3 + r1 and r3 = … in the same instruction see the
  old r3). A store in the same instruction stores the **old** value of the register. **[exact]**

### 4.3 Slot area and immediates **[measured, verified over 100 random instructions]**
* Slot area = the words after the last T (without T: the N and I words). First one N word per MAC; then references
  and immediates mixed.
* A transfer points to **slot word Y >> 1**. A jump takes its offset the same way.
* An **immediate** (mode 1 = 16 bits, 2 = 24, 3 = 32: the high bits of the RAW, the rest 0) of an A/B op starts
  **Y bytes** into the slot area (Y = the op's 4-bit Y field). Several ops can read the same immediate or different
  ones. In the MAC: Y at byte Y, W at byte W, the coefficient C **right after** Y's immediate (or at byte C if Y is a
  register). A read that passes from one word to the next **skips the words used as references**. If the immediate
  goes past the end of the instruction, the hardware keeps reading the following words of the program.

### 4.4 C (condition), J (jump) and `40TY` words **[measured]**
```
C:  bits 15-14 = 00 | cc[13:9] (condition 0..31) | neg[8] | skipT[7:4] | skipF[3:0]
    cond = flag[cc] XOR neg ; if cond: unit skipT does not execute; otherwise unit skipF
J:  0x009Y  = jump (one more unit, counted as an op)          40TY = hidden-register load (§5.6)
```
* **Units** = ops (A, B, M, J) in position order, then the transfers, numbered from 1.
* The chosen field only acts if **bit 3 of the other field is 0**.
* Field 0 or out of range: does nothing, **except** in classes with three A ops and a single C (0x2c8 AAAC, 0x570,
  0x240 MAAC), where it **cancels all the ops**. There are special numberings per class (0x190: field 2 is also the
  op). Roland hardly uses out-of-range fields (< 0.5 % of its instructions); a compiler should avoid them.

---

## 5. Compute units

Op word (A and B): `D[15:12] op[11:9] X[8:6] mY[5:4] Y[3:0]`. mY = 0: Y is a register (bit 3 = negate, bits 2–0 =
register); mY = 1/2/3: immediate of 16/24/32 bits (§4.3). "Nibble" = bits 11–8 = op·2 + X[2].

### 5.1 Unit A **[exact]**

| op (D < 8) | rD ← |
|---|---|
| 1 | D + X + Y (`add3`, exact 2026-10-28) |
| 2 | X · Y |
| 3 | D + X · Y |
| 4 | Y + X · D |
| 5 | Y − X · D |
| 6 | X + Y |
| 7 | Y − X |
| 0, sub X = 1 | Y (move / load immediate) |
| 0, sub 2 | Newton step: rD ← D·r(D+1); r(D+1) ← 2 − r(D+1)·rY; rY ← r(D+1)·rY (exact, results ready in the next instruction, 2026-10-28) |
| 0, sub 4 / 5 | D + Y + h4 / h5 |
| 0, sub 6 / 7 | D + Y |
| 0, sub 0 / 3 | nothing |
| D ≥ 8 | r(D−8) ← X + Y + r[op] (op indexes a third register; `sum8`, exact 2026-10-28) |

### 5.2 Unit B

| form | result | certainty |
|---|---|---|
| D < 8, nibble 0 / 2 / 4 / 6 / 8 / a | X AND Y / (NOT X) AND Y / X OR Y / (NOT X) OR Y / X XOR Y / NOT(X XOR Y) — two's complement, result mod 512 | [measured] |
| D < 8, nibble c / e | \|X\| + Y / Y − \|X\| | [exact] |
| D ≥ 8, nibble 0 / 2 / 4 / 6 | min(X, Y) / min(−X, Y) / max(X, Y) / max(−X, Y) | [exact] |
| D ≥ 8, nibble 8 / a | \|X\| / \|Y\| | [exact] |
| D ≥ 8, nibble c, mode 0 | frexp: X = m·2^e, m ∈ [0.5, 1): r(d) ← m, rY ← e | [measured] |
| D ≥ 8, nibble c, mode 1 / 2 / 3 | X·2^Y / X·2^−Y / X·2^−(16+Y) (Y = the number of the field, 0–15). **Two's complement with 32 fraction bits**: rounds toward −∞ at 2^−32 and wraps into [−512, 512) (2.65·2⁸ = 679 → −345); then the RAW encoding (§3) truncates toward 0 like any result. A negative result on the 2^−32 grid is 1 LSB below `mul`; on coarser grids it equals `mul` | [exact, 2026-10-29: 672 raw cases + 15 360 in `re/dspnative.py`] |
| D ≥ 8, nibble e, bits 7–6 = 00 | **CMP**: flags of r(d) + Y, writes nothing | [measured] |
| D ≥ 8, nibble e, bits 7–6 = 01 | r(d) ← Y | [measured] |
| D ≥ 8, nibble e, bits 7–6 = 10 | `divseed`, reciprocal seed with a numerator: \|Y\| = m·2^e, m ∈ [0.5, 1) → r(d) ← X·sign(Y)·2^−e (two's complement, 32 fraction bits, toward −∞), r(d+1) ← R[m], rY ← m; X even (r0, r2, r4, r6) | [exact, 2026-10-29, `re/dspnative.py raw_e10_*`; C reference `emu/dspcore.h`] |
| D ≥ 8, nibble e, bits 7–6 = 11 | by bits 5–4 and Y bit 3: 00 r(d) ± rY; 01 \|rY\| / clamp(rY, −1, 1); 10 wrap(rY) into [−1, 1) / triangle 1 − 2\|wrap(−rY − 1.5)\|; 11 (rY mod 2)/2 / −rY | [measured] |
| D ≥ 8, nibble f | SFU (§5.3) | |
| odd nibbles | the same op with X in r4–r7 | [exact] |

r(d) = r(D − 8). Flags of min/max: N = sign of the result, Z = operands equal.

### 5.3 SFU (unit B, D ≥ 8, nibble f)

Bits 7–6 = 00, function by bits 5–4 (mode) and Y bit 3:

| mode | Y bit 3 | function |
|---|---|---|
| 00 | 0 | reciprocal seed: \|Y\| = m·2^e, m ∈ [0.5, 1): r(d) ← sign·2^−e (**0 if 2^−e ≥ 512**), r(d+1) ← **R[k]**, **rY ← m** (overwrites the input). With `newton` r(d), r(d+1), rY three times: r(d) = 1/Y with error 10⁻⁸ (Goldschmidt) [exact, 2026-10-28] |
| 00 | 1 | atanh split: x = rY; (x+1)/(2·2^k) ∈ [0.5, 1): r(d) ← (x−1)/(2·2^k), rY ← (x+1)/(2·2^k), r(d+1) ← R[k] of rY → (x−1)/(x+1) = r(d)·r(d+1) after Newton (logarithms) |
| 01 | 0 | split for the root: \|Y\| = m·4^−k, m ∈ [1, 4): rY ← m, r(d+1) ← **Q[j]/512**, **r(d+3)** ← sign·2^−k. With rY = r(d+1) or r(d+3) the two writes mix bits (do not use) [exact, 2026-10-29: 384 raw cases + 3072 in `re/dspnative.py`] |
| 01 | 1 | r(d) ← e^⌊Y⌋ **from a ROM of its own** (n = −19…6, RAW words in `emu/dspcore.h EXP_ROM`; neither truncated nor rounded: up to 6·10⁻³ of error at n = −18, 10⁻⁸ near 0), 0 for n < −19, the largest RAW (511.999998) for n > 6; rY ← Y − ⌊Y⌋ [exact, 2026-10-28] |
| 10 | 0 | log2 split: Y = m·2^e, m ∈ [0.75, 1.5): rY ← m, r(d) ← e. Y < 0 is read in two's complement: r(d) ← e = ⌊log2((Y + 512)/0.75)⌋, rY ← Y/2^e rounded toward −∞ at 2^−32 (then RAW truncates); e = 9 for \|Y\| < 115 [exact, 2026-10-29: 120 raw + 3072] |
| 10 | 1 | r(d) ← sign(Y) (±1, 0) |
| 11 | 0 | r(d) ← ⌈Y⌉ |
| 11 | 1 | r(d) ← **⌊Y⌋** (floor) [exact, 2026-10-28] |

**ROM tables (bit by bit, `re/dspseed.py`)**:
* **R** (reciprocal, 64 entries): k = 6 bits of the mantissa after the leading 1 (steps of 1/128 in [0.5, 1));
  `R[k] = round(4096 / (0.5 + (2k + 1)/256)) / 4096`. Matches 64/64. **[exact]**
* **Q** (1/√, 48 entries, 9 bits): j = ⌊(m − 1)·16⌋ for m ∈ [1, 4); value = Q[j]/512 with
  `Q[j] = round(512 / √(1 + (j + 0.5)/16))`:
  504 490 476 464 452 442 432 422 414 406 398 391 384 377 371 365 359 354 349 344 339 334 330 326 322 318 314 311
  307 304 300 297 294 291 288 285 283 280 277 275 272 270 268 266 263 261 259 257. **[exact, 2026-10-29]**

Bits 7–6 = 01 (sub-op in bits 5–4): `f4Y` r(d) ← r(Y & 7); `f5c` r(d) ← delay write position; `f5Y` (Y ≠ c) reads
0 in our contexts; `f6Y` (Y = c–f) r(d) ← (r(d) + h(Y−8)) mod 512; `f7Y` = markers (§9). **[measured]**

### 5.4 MAC (two words: w1 among the ops, w2 = the N in the slot area) **[exact as far as measured]**
```
w1 = D op1 X mY Y        w2 = Z op2 C mW W
P1 = Y · rX              P2 = c · Z   (c = rC, or immediate if op2 >> 1 ≠ 0; Z = ±rZ)
rD ← (op1 & 1 ? −P1 : P1) + (op1 & 2 ? W : 0) + (op1 & 4 ? P2 : 0)        op1 ∈ {0, 1}: does not write
op2 = 000 and op1 ∈ {6, 7}: P1 = X (or X·Y if Y is immediate), P2 = Z  (sum mode, no products)
D ≥ 8: r(D − 8) ← result / 2, only with P2 (op1 ≥ 4); with op1 2 / 3 and D ≥ 8 nothing is written
immediate coefficient: at byte C of the slot area (+8 with bit 9 of w2); immediate Y at byte Y, W at W
```
An instruction can have two MACs (classes MM…): in class 0x080 (`MMNN`, the one Roland uses with one product each)
both read the registers from before the instruction (in parallel) and are exact; **if either of the two has a P2
term, neither writes** [exact, 2026-10-29]. Immediate coefficient with Y register or immediate, 16/24/32 bits, with
and without W, negated, /2: exact (20 forms × 1536 cases) [2026-10-29]. Exact result + truncation (§3). With the
coefficient in a register (op2 = 001: `mac_p1p2`, `mac_p1w`, `mac_p1p2w`, `mac_p1p2wp2`, also D ≥ 8 = /2): exact,
result ready in the next instruction [measured 2026-10-28, `re/dspnative.py`]. Classes 0x040 and 0x098 have the same
body (MN); the measurement is with 0x040 (header `0803`), the one the assembler picks.

### 5.5 Flags from the ops
`X9bY` (transfer of mode `9b`): group X − 8 (X = 9…d → groups 1…5) ← flags of the result of unit Y of the same
instruction. Per group: c0 = ≠ 0, c1 = ≥ 0, c2 = no overflow (\|x\| < 512), c3 = ≤ 0. They apply to A, B (min/max:
Z = operands equal), MAC, SFU and CMP. **Latency: one instruction.** They persist until rewritten. **[measured;
confirmed by audio on the slaves]**

### 5.6 Hidden registers: `40TY` in a C position **[measured]**
T = bits 7–6 → h4, h5, h6, h7; mode = bits 5–4 (0: h ← rY mod 512; 1/2: 16/24-bit immediate = value·16384, Y =
words to skip; 3: reads 0). h6 = index of `28` (bank 0) and of `da`/`de` (bank 5); **h7 = index of `db`/`df`**
(bank 5) **[measured 2026-10-17]**.

---

## 6. Control flow

* **Predication**: C words (§4.4) with the flags of §5.5. Selection of two ops in one instruction (`x12`: if true
  skip 1, otherwise 2).
* **Jumps**: word `009Y` in a C position, signed 16-bit offset = slot word Y >> 1, **in words from the end of the
  instruction**. It is a unit: a C makes it conditional. Backwards = loops (measured 3/8/16 exact iterations; on the
  slaves, a 4-iteration loop by audio).
* **Cost**: taken jump +1 cycle; what is skipped costs nothing.
* **Risk**: rewriting live a program with backward jumps can leave it "broken" halfway through the write and in an
  infinite loop (it hung the slaves). Always load an empty program first and the header last.

---

## 7. Memory

### 7.1 Banks (prefix = bits 15–13 of the reference)

| bank | references | content | measured behaviour |
|---|---|---|---|
| 0 | `0nnn` | DT0: data RAM, 0x800 words | **ring** (§7.2); 2 reads per instruction |
| 1 | `2nnn` | DT1: main ↔ slot mailbox (`22nn`) | used as input and output of the DCO slots. **2026-10-22:** private to each DSP except DSP0/DSP1 (ARM table) and, from the listings, a **ring** like bank 0 [measured with C-V4] ([S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6.7) |
| 2 | `4nnn` | DT2: window of the delay line | two pages that alternate per sample; delay DMA where the ARM connected it (§8.4) |
| 3 | `6nnn` | IOMD: I/O words (§8.3) | not memory: inputs and outputs with an owner |
| 4 | `8nnn` | MXM: bus between DSPs, zones M/O/Z/S of 0x200 words | §8.2 |
| 5 | `Annn` | DPC: plain persistent memory, 0x400 words (0x400–0x7FF: only within the sample; **2026-10-22:** in Roland's main programs it is the mailbox of the DSP pair; DSP5 → DSP4 measured with C-V4, [S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6.7) | 1 read per instruction; indexable with h6/h7 (§7.4); the ARM reads and writes it (best place for test results and **tables**) |
| 6 | `Cnnn` | parameters written by the ARM | read-only for the DSP |
| 7 | `Ennn` | ? (`e013` is read by the JX as a source) | [unknown] |

### 7.2 Bank-0 ring **[measured]**
Reference R addresses physical word (R + ptr) mod 0x800, and ptr advances 1 per sample. In the same sample what is
stored at R is read back at R; **in the next sample it appears at R + 1** (at R + k after k samples). State between
samples: load from S + 1, store to S. Variable delay: indexed transfers `X28Y` (bank0[⌊h6·16384⌋ + Y], also relative
to the ring).

### 7.3 Access rules **[measured]**
* Loads per instruction: bank 0 ≤ 2 (with more, only the last counts), bank 5 ≤ 1 (with more, none). **Stores to
  bank 5: also ≤ 1 per instruction** (with 2 or 6 none stays; 1 to bank 5 + 1 to bank 0 works) **[measured
  2026-10-17]**; stores to bank 0 do not count.
* Load latency: the op of the **next** instruction sees the loaded value; an op of the same instruction still sees
  the old one [measured 2026-10-29 on master DSP3, §7.4; SYSTEM-1 2026-10-23, [ch. 7](isa_overview.md) §9].
* An op and a load to the same register in the same instruction are **added**.
* Up to 6 transfers per instruction.
* **Bank 0: at most 2 stores per instruction; bank 6: at most 2 loads** [measured on the SYSTEM-1, 2026-10-23: 3 hung
  the device; in all of Roland's code, SYSTEM-8 and SYSTEM-1, there are never more than 2]. On the SYSTEM-1 the bank-5
  limits must also be met or the device hangs, and the slave DSPs only store to `A100`–`A1FF` [inferred from Roland's
  programs]. Detail and bisections: [examples/system_1_jx/README.md](../../examples/system_1_jx/README.md) §5, §10. The emulator checks these limits when
  it loads a program (`emu/chain.py`, `access_violations`).

### 7.4 Indexed transfers and tables **[measured 2026-10-17: master DSP3 exact, slaves by audio]**
| mode | address | memory |
|---|---|---|
| `X28Y` | ⌊h6·16384⌋ + Y | bank 0, relative to the ring (variable delay < 2048 samples; heard on the slaves) |
| `XdaY` / `XdeY` | ⌊h6·16384⌋ + Y | **bank 5**, fixed |
| `XdbY` / `XdfY` | ⌊h7·16384⌋ + Y | **bank 5**, fixed |
| `Xd0Y`…`Xd7Y` | N·16 + Y | bank 5, direct address 0x00–0x7F |

X 0–7 loads rX, 8–15 stores r(X−8); no reference word; truncated index; one instruction between `40TY` and the
transfer. Bank 5: index mod 0x400 (alias every 0x800); bit 10 (0x400–0x7FF) gives words that only last for the
sample (Roland's A400.. temporaries). **Fixed tables (wavetables, LUT) = bank 5**: a 16-point table read with
`0da0` in the DCO2 slot of the slaves, triangle and square exact by audio, persistent. Free in general (refs of
Roland's corpus): 0x048–0x1FF and 0x306–0x3FF. `d8`/`d9`/`dc`/`dd`: variable address [unknown]. In bank 0 a fixed
table only survives without any store to bank 0 in the DSP (h6 = T + sample counter). The ARM's RAW view of bank 0
is also relative to the ring: the ARM cannot leave a table there.

**Measured again 2026-10-29 (master DSP3, bit by bit, `re/dspverify.py --programs`):** `hset h6/h7, rX` (mode 0,
h = rX mod 512) followed by `da`/`db` (load and store) and by `X28Y`, exact; the load latency is the normal one (the
next instruction already sees the value). **SYSTEM-1, 2026-10-29: `hset` + `X28Y` run on the device** (SYSTEM-S1CC
MP-CSAW2-8, indexed COLOR MOD, 8 voices); Roland's programs for the S1 do not use indexed transfers.

**LUT: where and how much room** (a LUT = `value = table[index]` with the index computed in the DSP: phase, note,
knob):

| memory | good for | read | room |
|---|---|---|---|
| bank 5 | fixed tables (they persist; the ARM or the DSP write them once) | `da`/`db` (h6 / h7 + Y), write `XdaY` with X ≥ 8 | 1024 words per DSP. S8: free in general 0x048–0x1FF and 0x306–0x3FF (refs of the corpus); C-WTLIVE puts 438 points there. S1: one bank 5 for slave A and B; the ARM writes A000–A0FF, the slaves store only to A100–A1FF (`asm/s1rules.py` R7); SYSTEM-S1CC uses 42 + 90 of them: ≈ 200 (ARM) + ≈ 160 (DSP) remain |
| bank 0 | tables of the sample (written and read in the same sample) | `X28Y` (h6 + Y) | 2048 words per DSP, ring (a fixed table moves); SYSTEM-S1CC DSP B: 0x0500–0x05FF free (COLOR MOD uses 0x0510–0x055F) |
| EDM | samples and long delays | tap DMA `Xa9Y` (§8.4) | 983 040 words (10 s), only DSP4 of the S8 master (slot `f7dc`) |

COLOR MOD pattern (SYSTEM-S1CC, `make_s1cc.py colors_idx_all`): a row of 16 words per source, voice v at 2 v (4-bit
Y: up to 8 voices with ring words S / S + 1); per sample and color one `hset h6` with the chosen row, per voice
`ldidx` + `ymacd` + `st`. `asm/dspsched.py` packs `ldidx` as a bank-0 load (two instructions after the `hset`, after
the bank-0 stores) and `asm/s1rules.py` counts it in rule R3.

---

## 8. Communication

### 8.1 Inside one DSP (between the main program and the slots)
Shared registers (what one program leaves the next one sees), bank 1 (`22nn`, e.g. inputs of the DCO slots `2207`,
`2209`–`220c`), bank 5 (`a1fe`/`a2fe` in the effect slots of the master). **[measured / inferred]**
**2026-10-22** [measured with C-V4]: the main program runs before the slots; it writes the input (`[2207]`) and the
slot reads it in the same sample and address; the slot writes the output (`[2221]`) and the main program reads it at
`[2222]` in the next sample, because bank 1 is a ring ([S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6.3 D/F).

### 8.2 Between DSPs of the same BMC: MXM (bank 4) **[measured on the master, DSP3 ↔ DSP4]**
* They are ordinary loads and stores to the zones M (`80nn`/`81nn`), O (`82nn`/`83nn`), Z (`84nn`/`85nn`), S
  (`86nn`/`87nn`); the word nn is preserved.
* **Point-to-point channels with fixed wiring**: what DSP3 writes in Z or S is read by DSP4 in O; what DSP4 writes in
  O is read by DSP3 in S. What a DSP writes in M / O goes to the mix memory of its pair (ARM windows MXM2/3 for DSP3,
  MXM4/5 for DSP4).
* **Frozen once per sample** (double buffer): during its whole program the reader sees the same value, no matter when
  the other one wrote.
* **Exact latency: 1 sample** (measured with the common clock).
* **No summing**: two writes in the same sample → the last one wins. **No keeping**: if nobody writes in a sample, the
  reader gets 0.
* Topology of the voice programs (static, the same in all synthesizers): DSP0/1 → DSP2/3 → DSP4/5 → IOMD; input zones
  M ← Z of the even DSP of the previous pair, O ← Z of the odd DSP; S = return from the DSP two positions ahead (DSP2
  M → DSP0 S, DSP4 M → DSP2 S). **[inferred from the corpus; the DSP3/DSP4 part measured]**
  **2026-10-22:** on the slaves measured with C-V4: DSP0 → DSP2 M, DSP1 → DSP3 O, DSP2 Z `4nn` → DSP4 M `nn`, DSP3 →
  DSP5 O, with the same word number ([S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6.7).
* **Pair mailbox (bank 5 `A400`–`A7FF`)** [listing; DSP5 → DSP4 measured with C-V4]: another path between the two
  DSPs of a pair, besides MXM. DSP5 → DSP4 `A400`–`A403`, DSP4 → DSP5 `A404`–`A407`, DSP0 → DSP1 `A404`… (§7.1,
  [S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6.2).
* **Between slaves** [measured with C-V4]: slave 1 reaches the audio when DSP4 of slave 0 sums its IOMD inputs
  `6000`–`6003`. Which word of slave 1 goes to each input: open.

### 8.3 IOMD (bank 3): I/O of the BMC **[partly measured]**
* It is not shared memory: each word is an input or an output, with an owner DSP configured by the ARM. Writes of a
  DSP that is not the owner do not appear; words written by their owner **persist** and the ARM sees them (RAW view
  0x4113E000).
* In the voice programs of the slaves: inputs `00–06`, `10–17`, `e0–e3`; outputs `20–24`, `60–66`, `b0–b7` (towards
  the wave bus that carries the voices to the master). On the master, the block contains live data (for example
  `02`/`12` = 4.75, `04`/`14` alternating ±1, `c0`/`c1`, `e0`/`e1`, `f0`/`f1` signals).
* Which word goes to which external connection (wave bus, codec) **[unknown]**.

### 8.4 Delay line (bank-2 DMA) **[measured on the master, DSP4 slot 0xF7DC]**
Write `[4300] ← x`; base with `40TY` + `f6c`; result destination `0b03 4050 <a>` (without it the request is
ignored); request `Xa9Y`; in the same sample `[a]`, `[a+1]`, `[a+2]` = two neighbouring samples + fraction for
interpolating. Address unit = 16384 samples. **6-bit fraction (1/64), truncated; ≥ 24 requests per sample** (each
with its destination) **[measured 2026-10-17]**. Line of `f7dc` = EDM 0x110000–0x1FFFFF (983 040 words, 10.24 s);
the EDM has 0x200000 words per BMC, readable by the ARM through an indirect port (sys+0x40/+0x44, 0x41158000), ARM
write not achieved; the DMA writes `[4300]` every sample even if the program does not store (refresh it like a loop)
**[measured 2026-10-17]**. It only works where the ARM connected the window. Details: [Vol. II ch. 4](instruction_set_reference.md) §9.1.

### 8.5 ARM ↔ DSP
The ARM writes program memory (`0x41480000 + n·0x100000` + slot address, 32-bit stores), the parameters (bank 6) and
bank 5; it reads bank 0 (IEEE/SD31/RAW views at `0x41400000 + n·0x100000`), bank 5, MXM and IOMD. Control registers
per DSP at `0x41104000 + n·0x1000`. Full map of the windows: [Vol. II ch. 6](memory_banks_dma.md) §1.

---

## 9. Markers `ff70` / `ff71` / `ef75` / `ef74` **[measured]**
`0a02 ff70` = start and end of a program (SYSTEM-1: `ff71` at the start). `ef75` opens and `ef74` closes the section
of each voice in Roland's programs (also fused as a B op inside other instructions). **They have no effect** on
registers, flags, memory or audio; they cost 1 cycle; no ARM code looks for them. Optional.

---

## 10. Execution model for an emulator

```
for each sample n:
    freeze the MXM inputs (what was written in sample n−1) and IOMD; advance the ring ptr and the delay position
    for each DSP, for each program (main, slots) in order:
        pc = start
        while the return is not reached:
            ins = decode(pc) with the template of its class
            visible flags = those written up to two instructions back (latency 1)
            skipped units = C words (§4.4)
            read all operands (old registers; loads with latency 1; immediates §4.3)
            compute each op exactly and truncate to RAW (§3); MAC; SFU with tables R and Q
            write results; stores with old values; op + load to the same register are added
            if a jump is taken: pc = end of the instruction + offset (+1 cycle); else pc = end
    publish what was written to MXM for sample n+1
```
Reference: the C code (`emu/dspdec.h`, `emu/dspcore.h`, `emu/dspcore.c`; from Python `emu/dspref.py`: `Machine.run`
one instruction, `Machine.run_program` one program with flags and jumps; `re/dspasm.py simulate` with the ring). The
Python reference (`re/dspisa.py`) was archived on 2026-10-29 ([Vol. V ch. 5](../vol5_toolchain_compiler_sdk/emulator.md) §1.1). Validation: `re/dspverify.py` runs
a real instruction on the device and compares it (100/100 random Roland instructions match).

**What is already exact**: decoding of all the classes of the corpus, A, B, SFU (tables included), MAC, flags, jumps,
ring, port rules, MXM.

**What is missing to be faithful to Roland's engine**: integer arithmetic (today the simulator uses floats with the
truncation rule; for full bit-exactness it has to be implemented on 29-bit mantissas), the complete delay line
(length, other slots), IOMD/wave bus between BMCs, order and scheduling of the programs that the ARM sets, and the ARM
side (when and how parameters and voices are written). For real time it has to be ported to C/C++.

---

## 11. Pending (unused in Roland's code unless stated)

1. Instruction classes that are not in the corpus (the class numbering is not decoded).
2. C with out-of-range fields in multi-op classes (Roland: < 0.5 % of the instructions, measured cases).
3. `f5Y` with Y ≠ c (`cf54` of the JX on the slaves reads 0 in our contexts), `89ba`, `00cY`, transfer mode `9e` and
   the "other memory" ones (`e0`, `e1`, `8x`, `bx`, `cx`, `d8`/`d9`/`dc`/`dd`…; map in `re/dspmodescan.py`), zone
   0x400–0x7FF of bank 5.
4. Delay line: length, windows in other slots / DSPs, hearing it on the output.
5. IOMD: which word connects to what; wave bus between BMCs (slave 1 enters through `6000`–`6003` of slave 0, exact
   words not measured); DT1 shared only by DSP0/DSP1 according to the ARM table (not measured); MXM on the slaves
   (only static) and zones M/Z of the master beyond the DSP3/DSP4 pair.
6. DSPM (not studied).
7. Why a program that goes over the budget leaves the slaves unstable.

> **Note on paths:** in this chapter `emu/…`, `re/…` are relative to `examples/from_scratch/`.
