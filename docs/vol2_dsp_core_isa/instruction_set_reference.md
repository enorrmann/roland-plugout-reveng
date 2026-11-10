# Volume II — Chapter 4: Instruction Set Reference (DSP ISA specification, part 2: §6–§14)

> **Scope:** both (same ISA); measured on the SYSTEM-8 unless stated.

> Part 2 of the ISA specification (§6–§14). Part 1 (§1–§5: machine model, RAW number format, program image,
> instruction encoding, word formats) is [Vol. II ch. 3](instruction_word_encoding.md); a reference "§n" with n ≤ 5
> points to chapter 3, n ≥ 6 to this chapter. Roland BMC DSP as used by the SYSTEM-8 (and its PLUG-OUTs). Status tags:
> **[exact]** measured, reproduced with random operands, error 0 · **[measured]** measured on specific cases ·
> **[inferred]** derived, not tested directly · **[unknown]** not decoded, do not emit.

## 6. ALU semantics

All ops of one instruction **read their operands before any of them writes** (§10).

### 6.1 Unit A **[exact]**

D < 8:

| op | rD ← |
|---|---|
| 0 | sub-code in X (below) |
| 1 | D + X + Y |
| 2 | X · Y |
| 3 | D + X · Y |
| 4 | Y + X · D |
| 5 | Y − X · D |
| 6 | X + Y |
| 7 | Y − X |

(D = old value of rD, X = rX, Y = operand Y.)

op 0 sub-codes (X field):

| X | effect | status |
|---|---|---|
| 0, 3 | no effect | [exact] |
| 1 | rD ← Y (move, load immediate) | [exact] |
| 2 | Newton step: rD ← rD·r(D+1); r(D+1) ← 2 − r(D+1)·rY; rY ← r(D+1)·rY (all with old values) | [exact] |
| 4 | rD ← D + Y + h4 (hidden, context dependent) | [exact formula, h unknown] |
| 5 | rD ← D + Y + h5 | [exact formula, h unknown] |
| 6, 7 | rD ← D + Y | [exact] |

D ≥ 8 (single A op, class 0x048): r(D−8) ← X + Y + r[op] (the op field indexes a third register) **[exact]**.
Measured 2026-10-20 in class 0x048 (one op, four values of op) and in class 0x0a8 (AA, all eight values of op):
r(D−8) ← X + Y + r[op] in every case. The listing prints the form as `sum8`. Other multi-op classes are not yet checked
on the device (DISCREPANCIES D5).

### 6.2 Division

There is no divide op. Roland computes reciprocals with repeated op-0 sub-2 Newton steps (`0902 4086` ×3 in the
SAW oscillator) after an initial estimate.

### 6.3 Unit B

Same word layout. **[exact unless noted]**

| word form | rD ← |
|---|---|
| `D 0 X Y` (D < 8) | **rX AND Y** — bit ops on two's-complement fixed point, result **mod 512** (a negative result reads as 512 + x) [measured 2026-10-17, `re/dspbits.py`] |
| `D 2 X Y` (D < 8) | (NOT rX) AND Y |
| `D 4 X Y` (D < 8) | rX OR Y |
| `D 6 X Y` (D < 8) | (NOT rX) OR Y |
| `D 8 X Y` (D < 8) | rX XOR Y |
| `D a X Y` (D < 8) | NOT (rX XOR Y) |
| `D c X Y` (op 6, D < 8) | \|rX\| + Y |
| `D e X Y` (op 7, D < 8) | Y − \|rX\| |
| D = 8+d, op 0 | min(rX, Y) |
| D = 8+d, op 2 | max(rX, Y) |
| D = 8+d, op 4 | \|rX\| |
| D = 8+d, op 5 | \|Y\| |
| D = 8+d, nibble e, bits 7–6 = 00 | **nothing written**: flags of r(d) + Y (CMP / TST, for `X9bY`; `8e09` = cmp r0 − r1, `ae10` = cmp r2 with imm) [measured 2026-10-17] |
| D = 8+d, nibble e, bits 7–6 = 01 | Y (r(d) ← ±rY) [fitted] |
| D = 8+d, nibble e, bits 7–6 = 11 | **bits 5–4 select the function, Y bit 3 the variant** (no immediate) [measured 2026-10-17, `re/dspshape.py`]: 00: r(d) ± rY; 01: \|rY\| / clamp(rY, −1, 1); 10: wrap(rY) into [−1, 1) / triangle 1 − 2\|wrap(−rY − 1.5)\|; 11: (rY mod 2)/2 (phase 0..1) / −rY |
| D = 8+d, nibble c, modes 1–3 | **shifts** [measured 2026-10-17]: mode 1 r(d) ← X·2^Y, mode 2 X·2^−Y, mode 3 X·2^−(16+Y) (Y = the 4-bit field, not a register); mode 0 = frexp |
| D = 8+d, nibble e, bits 7–6 = 10 | `divseed`, the reciprocal seed with a numerator (never used by Roland) [exact, 2026-10-29]: \|Y\| = m·2^e → r(d) ← X·sign(Y)·2^−e (two's complement, 32 fraction bits, toward −∞), r(d+1) ← R[m], rY ← m; X = r(2·mode), or rY with Y bit 3 (`re/dspnative.py raw_e10_*`; C reference `examples/from_scratch/emu/dspcore.h`, comment of `b_op`) |

X is 3 bits: the "B" nibble's low bit is X[2], so odd nibbles are the same ops with X in r4–r7 (measured
`d150` = min(r5, imm 1.0), `d550` = max(r5, 0), `e590` = max(r6, 0), `e190` = min(r6, 1.0): Roland's
`clamp(x, 0, 1)`). Y may be an immediate (mode bits). The ops with D < 8 and nibbles 0–a are bit operations (table above;
negative results read modulo 512) **[measured 2026-10-17]**. Some D ≥ 8 forms also change other registers.

Also measured (2026-10-16, D ≥ 8, random operands): nibble 2 = min(−X, Y), 6 = max(−X, Y) (bit 1 negates X);
nibble c = **frexp**: X = m·2^e, m ∈ [0.5, 1) → r(d) ← m, rY ← e (X = 0 → 0, −31).

### 6.4 Special-function unit (unit B, D = 8+d, nibble `f`, bits 7–6 = 00)

Selected by the mode bits (5–4) and Y bit 3; Y register = Y & 7. **[measured, random inputs]**

| mode | Y bit 3 | function | example |
|---|---|---|---|
| 00 | 0 | **reciprocal seed**: \|Y\| = 2^e·m (m in [0.5, 1)): r(d) ← sign(Y)·2^−e (0 when 2^−e ≥ 512), r(d+1) ← R[k] (seed ROM below), rY ← m (the input is overwritten) [exact, 2026-10-28] | `cf06`: r4, r5 from r6; refined by three `0902 4086` Goldschmidt steps |
| 01 | 1 | **exp split**: r(d) ← e^⌊Y⌋ from a ROM of its own (n = −19 … 6, `EXP_ROM` in `emu/dspcore.h`), 0 for n < −19, the largest RAW (511.999998) for n > 6; rY ← Y − ⌊Y⌋ [exact, 2026-10-28] | `8f19`: r0 = e^⌊r1⌋, r1 = frac(r1); Roland then evaluates e^frac by Horner |
| 10 | 1 | sign: r(d) ← ±1 | `8f29` |
| 11 | 1 | **floor**: r(d) ← ⌊Y⌋ [exact, 2026-10-28] | `8f39` |
| 11 | 0 | ceiling: r(d) ← ⌈Y⌉ | `cf36` |
| 01 | 0 | **sqrt split**: \|Y\| = m·4^−k, m ∈ [1, 4): rY ← m, r(d+1) ← Q[j]/512 (seed ROM below), r(d+3) ← sign(Y)·2^−k → √\|x\| = r(d+3)·m·r(d+1); with rY = r(d+1) or r(d+3) the two writes mix bits (do not use) [exact, 2026-10-29] | `cf16`: 9 → m 2.25, r5 0.662, r7 2 [measured 2026-10-16] |
| 10 | 0 | **log2 split**: Y = m·2^e, m ∈ [0.75, 1.5): rY ← m, r(d) ← e; Y < 0 is read in two's complement: r(d) ← ⌊log2((Y + 512)/0.75)⌋, rY ← Y/2^e rounded toward −∞ at 2^−32; e = 9 for \|Y\| < 115 [exact, 2026-10-29] | `cf26`: 9 → e 3, m 1.125; 0.375 → e −1, m 0.75 [measured] |
| 00 | 1 | **atanh split** (for logarithms): x = rY, 2^k such that (x+1)/(2·2^k) ∈ [0.5, 1): r(d) ← (x−1)/(2·2^k), rY ← (x+1)/(2·2^k), r(d+1) ≈ 1/rY (~1 %) → r(d)·r(d+1) refined by Newton = (x−1)/(x+1); ln x = 2·atanh of it | `9f0d` after `8f25` log2 split, then `0902 1085` Newton (base engine, JUPITER-8) [measured 2026-10-17]; `8f09` same op |

**Seed ROMs [exact, 2026-10-17, `re/dspseed.py`]**: reciprocal seed (`cf06`, also used by the atanh split
`9f0d`): 64 entries, k = the 6 mantissa bits after the leading 1 of m ∈ [0.5, 1), R[k] = round(4096 / (0.5 +
(2k+1)/256)) / 4096 (64/64). Reciprocal square root seed (`cf16`) [exact, 2026-10-29]: 48 entries of 9 bits,
j = ⌊(m − 1)·16⌋ for m ∈ [1, 4), value = Q[j]/512, `Q[j] = round(512 / √(1 + (j + 0.5)/16))` = 504 490 476 464 452
442 432 422 414 406 398 391 384 377 371 365 359 354 349 344 339 334 330 326 322 318 314 311 307 304 300 297 294 291 288
285 283 280 277 275 272 270 268 266 263 261 259 257.

**Nibble `f`, bits 7–6 = 01** (sub-op in bits 5–4) **[measured 2026-10-17, master DSP3/DSP4, `re/dsphops.py`,
`re/dspf5.py`]**:

| form | function |
|---|---|
| `{8+d}f4Y` | rd ← r(Y & 7) (sign bit of Y ignored). Roland also uses it as rd ← rd (`bf4b`, `df4d`, `ff4f`, with `89ba`): no other effect found on the master |
| `{8+d}f5c` | rd ← **delay-ring write position**: counts down 1/16384 per sample (96 000 samples/s), mod 512, own value per DSP; independent of rd and h4. Not used by Roland |
| `{8+d}f5Y`, Y ≠ c | reads 0 on the master (`9f5d`: a per-DSP constant, 0.0007 on DSP3, 1.8746 on DSP4) and on the slaves' DCO2 slot (`cf54`, `cf55`: SAW × value = silence, by audio). Roland uses `cf54` (JX DCO2 slot, as a weight): probably set up by Roland's own main program **[unknown]**. `cf5c` is non-zero on the slaves too |
| `{8+d}f6Y`, Y = c–f | **rd ← (rd + h(Y − 8)) mod 512**: `f6c` + h4 (delay base, §9.1), `f6d` + h5, `f6e` + h6, `f6f` + h7 (JX: `af6f` = index + h7, wrapped at 8 with cmp/C). Y < 8: no change |
| `{8+d}f7Y` | **measured 2026-10-17: no effect on registers, flags, memory addressing (banks 0/5) or audio (slaves: a SAW with Roland-style `ef75`/`ef74` voice sections sounds identical); each costs one cycle (`dspbudget.py marker`: same budget edge as single ops); no ARM code refers to them**. Section markers. `0a02 ff70` = program start and end (SYSTEM-1 programs start with `ff71`); **`ef75` opens and `ef74` closes a per-voice section** (layout of every two-voice program: input stores, `ef75`, voice A, `ef74 ef75`, voice B, `ef74`, output stores, end). Their hardware role (scheduling / sync) is not visible to the DSP |

## 7. MAC semantics **[exact on the measured cases]**

```
P1 = Y · rX
P2 = c · Z          (c = rC or immediate per op2; Z = ±rZ)
rD ← (op1 & 1 ? −P1 : P1) + (op1 & 2 ? W : 0) + (op1 & 4 ? P2 : 0)
special case: op2 = 000 and op1 ∈ {6,7}: no products → rD ← ±rX + W + Z (2026-10-17: with an immediate Y the
P1 product stays: ±imm·rX + W + Z; op1 ∈ {4,5} keeps both products with op2 = 000; op2 = 001 always multiplies:
op2 bit 9 enables the products in the W modes) [measured, `re/dspmacfit.py`; re-measured 2026-10-20: `mac_p1p2c0` with op1 = 6 gives X + W + Z]
op1 ∈ {0,1}: no write
D ≥ 8: r(D−8) ← result / 2                                   [measured 2026-10-16, random operands]
op2 = 010: c = imm16 (modes of c: 00 rC, 01 imm16, 10 imm24, 11 imm32); op2 = 011: unknown (constant result)
```
An A op with an immediate inside a MAC class (0x088) reads the MAC's N word as its immediate (measured): do not
give it a constant of its own.

Measured examples:

| instruction | computes |
|---|---|
| `0803 0c0a 9281` | r0 ← r1 + r2·(r0 − r1) (linear interpolation) |
| `0803 0809 5280` | r0 ← r0·r1 + r2·r5 |
| `0807 3cca 1cb6 002e 3bc7 01a0 1a01` | r3 ← r3·r2 + (2/2835)·r1 + 2/315 |
| `1107 7522 6504 0025 800d 0002 2222` | r7 ← r4·(−1/5040) + 1/120 ; r6 ← r4·r4 (A op) |
| `2b09 2c8f 6fe2 7fe4 5983 0407 0403 a2aa aa00` | r2 ← r2·r7 + r3 + (−1/6)·r5 ; r6 ← [0407] ; r7 ← [0403]. Y is not negated (measured 2026-10-20) |

## 8. Predication

A C word enables/disables an A/B op of its instruction according to one of 16 **condition bits** (§5.4). The
bits are **flags**, 4 groups of 4, written by `X9bY` transfers **[measured 2026-10-17, master DSP3 main and DSP4
`efdc`, `re/dspx9bflags.py`]**:

```
X9bY   (transfer word, mode byte 9b): group g = X − 8 (X = 9, a, b → g = 1, 2, 3), Y = unit (1-based, ops first)
       flags of group g ← flags of the result of unit Y of the SAME instruction
cond 4g+0 = !Z        (result ≠ 0)
cond 4g+1 = !N        (result ≥ 0)
cond 4g+2 = !V        (|result| < 512, no overflow)
cond 4g+3 = N or Z    (result ≤ 0)
```

* Z, N for an A op: of the written result (r0 = r2 + r2 with r2 = 256 → 512 wraps to 0 → Z and V). A results
  **wrap**: magnitude mod 512, sign kept (300 + 300 → 88, −300·2 → −88, 511·2 → 510) **[measured]**.
* Unit B min/max (measured `b4c0` = max(r3, r0)): N = sign of the result, but **Z = the two operands are equal**
  (max(−0.25, 0) = 0 gives "≠ 0"; max(−0.5, −0.5) gives `0011` = Z and N).
* Unit B nibble e with D ≥ 8 and bits 7–6 = 00 (`8e09`, `be0c`, `ae10`…) is a **compare**: flags of r(d) + Y,
  no register written (`be0c` = r3 − r4, `be04` = r3 + r4, `9e0c` = r1 − r4) — the usual partner of `X9bY`.
* Units: Y = Y-th op of the instruction in position order (A, B, M and jumps count): Y = 1, 2 (class 0x328
  `6505 7c0f be0c 99b1 b9b2`), 3 (`b48b 06c7 1505 ae0e b9b3 …`) and 4 (`904a 0cc0 0cca 6062 be0f 1a12 b9b4 …`,
  after a MAC and two A ops) measured.
* MAC results set flags like A results (`8f09 4448 5448 99b1 a9b2`: two MACs → groups 1 and 2); SFU too (`8f39`:
  flags of the written r(d)); min like max (Z = operands equal).
* **Confirmed on the slaves** by audio (DCO2 slot, 2026-10-17): a SAW whose wrap is `cmp r1 − 1` + `b9b1` + a C
  select (or a conditional jump) is identical to the min/max reference.
* **Latency: one instruction** (as loads): a C word in the instruction right after the `X9bY` still reads the old
  flags; from the second instruction on it reads the new ones. Roland always leaves one instruction in between.
* **Persistence**: flags are not reset between samples, programs or test sessions; a group keeps its value until
  the next `X9bY` on it. The "static context bits" measured before (DSP3 `0111111011101110`, DSP4 `efdc`
  `0111101111100111`, slaves DCO2 `0011011001100110`) were simply the last flags left by Roland's code.
* **Groups 4 and 5** (2026-10-17): the C condition field is 5 bits; **X = c → group 4 (conds 16–19), X = d →
  group 5 (conds 20–23)**, same 4-flag semantics (master DSP3, and slaves by audio: `c9b1` + cond 17, `d9b1` +
  cond 21 switch the level of a SAW like the `b9b1` + cond 13 control). Roland: `c9b2` → C `2310` (!cond 17),
  `d9b1` → C `2a10` (cond 21). So **5 writable groups (X = 9…d → groups 1…5)**.
* **Constant groups**: group 0 (conds 0–3) reads `0111` (the flags of a zero: cond 1/2/3 true, cond 0 false);
  conds 24–31 read `1111` (always true). X = 8, e, f: no effect. Usable as constants: cond 1 = always, !cond 1 =
  never; cond 24 = always.
* The "C words" with cc ≥ 24 that some measured templates show (`3000`, `3921`, `4826`, `c0xx`…) are immediates
  or references mislabelled by the template guess, not conditions.
* Old tests missed this because they used `1803 0040 b9bP` (unit 1 = `r0 = r0` with r0 = 0) and read groups that
  did not match X.

Consequences for a compiler: **data-dependent control is possible**: compute (or `cmp`) a value, `X9bY` it into a
group, one instruction of distance, then a C word picks which op of an instruction runs (e.g. `x12`: skip op 1 if
true, op 2 if false) or whether a **jump** (§5.4b) is taken: if/else, switch, and loops (backward jumps).

## 9. Transfers and memory

* Load: `rR ← mem[slot]`; store: `mem[slot] ← r(R−8)` with mode `fe` **[exact]**.
* Up to 6 transfers per instruction (classes 0x060…0x722) **[exact]**.
* **Read ports per bank [measured 2026-10-17]**: bank 0 serves **2 loads** per instruction (with 3 or more only
  the last one is done); bank 5 serves **1** (with 2 or more none is done); bank-0 stores are not limited by this
  (2 stores + 2 loads work). **Bank 5 also takes only ONE store per instruction** (2 or 6 → none stored; one to
  bank 5 + one to bank 0 works) [measured 2026-10-17, master DSP3]. Roland's code respects it (0x47a: 2 loads from bank 0 + 2 from bank 6).
* An op and a load writing the same register in one instruction: the values are **added** (A `r7 = 0.5` + load
  0.375 → 0.875; measured again 2026-10-17). Avoid.
* **Ring addressing [measured]**: a reference R addresses physical word (R + ptr) mod ring, and ptr moves by one
  word every sample. Within one sample, a value stored at R is read back at R. **A value stored at R is read at R + 1
  in the next sample** (R + k after k samples). This is how state and delay lines work: to keep a variable from one
  sample to the next, load it from S + 1 and store it to S. Loading from S reads the value stored at S − 1 one sample
  ago (on master DSP3, a counter loaded from S and stored to S did not accumulate; loaded from S + 1 it counted
  96 000 times per second). Verified on master DSP3 and by the oscillator on the slave DCO slots.
* Consequence for observation: the ARM sees physical words; a value stored every sample leaves a trail over the
  whole bank (2048 words in < 0.4 s).
* Persistence per bank (master DSP3, counter loaded from W and stored to W) **[measured]**: bank 0 ring (load S+1);
  **bank 2 (`4nnn`) = two pages alternating every sample** (a store is read back in the same sample and 2 samples
  later, not at W+1); bank 4 (`80nn`, M zone) does not keep values to the next sample; bank 5 (`Annn`) plain
  persistent memory; bank 3 (`60nn`) not read back. The ARM reads bank 5 coherently (DPC*n* RAW view), which makes
  it the best place for test outputs.
* Bank 0 can be read by the ARM (RAW view) but ARM writes to bank 0 do not stay; bank 5 is ARM-writable
  **[measured]**.

### 9.0 Hidden registers and indexed transfers **[measured, master DSP3 / DSP4, 2026-10-16]**

* **`40TY` in a C position** (classes 0x058, 0x0E8, …) loads a hidden register: T = bits 7–6 (0 → h4, 1 → h5,
  2 → h6, 3 → h7 (unread)), mode = bits 5–4 (0: rY, kept mod 512; 1: imm16; 2: imm24; 3: imm32 reads 0), Y = bits
  3–0 = register, or for immediates the byte of the instruction's immediate stream the value is read at (as for an A/B op, §4.4).
  Measured 2026-10-20 (`hset h5` with Y = 1 reads byte 1, not word 1); the example reads byte 2 for h4 = 324. Immediate
  values are integers = value·16384 (addresses): `0b03 4050 0301` → h5 = 0x301; `1d06 4022 7fe0 c1fc 5100 0000` →
  h4 = 0x510000/16384 = 324; `0b02 4080` → h6 = r0.
* h4 = delay base (`f6c`, §6.4) and the value A op 0 sub 4 adds; h5 = tap-result address (§9.1), added by A op 0
  sub 5. Defaults on master DSP4: h4 = 288, h5 = 0x105/16384.
* **Indexed transfer, mode byte `28`**: `X28Y`, X 0–7 → rX ← bank0[⌊h6·16384⌋ + Y]; X 8–15 → bank0[…] ← r(X−8).
  Truncated, mod 0x800, ring-relative like a normal reference; a reference word is not added; several per
  instruction (`1e04 0281 1282`). Usage (S-100): `r0 ← param`, `0b02 4080`, `0c02 0280`.
* **Bank-5 indexed transfers [measured 2026-10-17, master DSP3 exact (`re/dspmodescan.py`, `re/dspb5idx.py`,
  `re/dspb5map.py`, `re/dspb5alias.py`); slaves by audio (`re/dspb5osc.py`)]**: `XdaY`/`XdeY` = bank5[⌊h6·16384⌋ + Y],
  `XdbY`/`XdfY` = bank5[⌊h7·16384⌋ + Y] (h7 ← r0 with `40c0`), `Xd0Y`…`Xd7Y` = bank5[N·16 + Y]; X 0–7 load, 8–15 store.
  Index truncated; one instruction between `40TY` and the transfer is enough. Persistent words: index mod 0x400
  (0x800, 0xC00, 0x1000 alias 0x000); with bit 10 set (0x400–0x7FF) a store is seen in the same sample only and
  is not kept (Roland's A400–A427 temporaries). Bank 5 serves one load per instruction (§9). **This is the
  memory for fixed tables** (wavetables, LUTs): plain memory, shared by all programs of the DSP. Roland's refs:
  A000–A047, A200–A205, A300–A303, A4xx; free in general 0x048–0x1FF, 0x306–0x3FF.
* A fixed table in bank 0 is possible only with no bank-0 store on the whole DSP: h6 = T + n (n = sample count
  kept in bank 5) addresses a fixed physical word (kept 3 s on idle master DSP3, wiped < 10 ms by one plain store;
  `re/dspb0fix.py`). The ARM's RAW view of bank 0 is ring-relative too (consecutive ARM reads ≈ one sample apart
  return the same physical word), so the ARM cannot write a table there.

* Indexed addresses are **ring-relative** (a counter kept with load [h6+1] / store [h6] counts every sample), so
  `[S + k]` reads what was stored at S k samples ago: a variable delay of < 2048 samples on any DSP. A fixed table
  cannot live in bank 0.
* **MXM timing and semantics [measured 2026-10-17, `re/dspbussync.py`, `re/dspbusclk.py`, `re/dspmxm.py`,
  `re/dspmxsum.py`]**: latched once per sample (a reader sees one value for its whole program), latency exactly
  1 sample (checked with the common delay-position clock), last write in a sample wins (no summing), not kept
  (0 when nobody wrote that sample). DSP3 Z/S → DSP4 O; DSP4 O → DSP3 S. IOMD (`re/dspiomd.py`): I/O words with an
  owner DSP; non-owner writes vanish; owner writes persist; the master block holds live inputs. See
  [Vol. II ch. 1](core_architecture.md) §8.
* **Buses [measured, master]**: `[60nn]` = IOMD word nn (0x4113C000). MXM stores of DSP3: M → ARM window MXM2,
  O → MXM3, Z and S → MXM5; DSP4 loads DSP3's Z/S stores from its O zone (`83nn` ← DSP3 `85nn`/`87nn`, same nn).
  `X9bY` writes nothing visible in any of these windows.

### 9.1 Delay memory (bank-2 DMA) **[measured, master DSP4 slot 0xF7DC, 2026-10-16]**

Only where the ARM connected the task's bank-2 window to the delay memory (master DSP4 global-delay slot 0xF7DC,
window `4300`; nothing happens on master DSP3). Delay memory = a ring that advances one sample per sample;
addresses are in units of 16384 samples. Per sample:

| step | instruction | effect |
|---|---|---|
| write | `[4300] ← x` (any `fe` store) | x enters the delay ring at address 324 (fixed for this slot by the ARM) |
| base | `1d06 4022 7fe0 c1fc <imm32>` | f6c base ← raw imm >> 22 (`5100 0000` → 324). Other low bytes of `40xx`: unknown, avoid |
| address | `{8+d}f6c` | rd ← rd + base (§6.4) |
| destination | `0b03 4050 <a>` | the next request writes its results to bank 2 [4000 + a ..] (`0301` → `[4301..4303]`). **Without it the request is ignored** |
| request | `da90` (`0c02 3a90`, or `1b04 c510 3a90 0008`) | read at address rd |
| results | load `[a]`, `[a+1]`, `[a+2]` (same sample) | x[n − L − ⌊k⌋], x[n − L − ⌊k⌋ − 1], frac(k), with k = 16384·(rd − 324), L = 2–3 samples (program position) |

Interpolate with `0803 0c0a 9281`-style lerp: r0 + frac·(r1 − r0). **The fraction has 6 bits (1/64), truncated**
(2026-10-17: k frac .4 → 25/64, .8 → 51/64, .2 → 12/64). **At least 24 requests per sample** work, each with its
own `0b03 4050 <dest>` (`re/dspmtap.py`; limit not found). With base = 324 the delay is 16384·u + L for
u = r before f6c (measured u = 0, 0.5, 1, 10, 50.648 → 3, 8196, 16388, 163844, 829826 samples; u = 100 out of the
line). Roland's global delay: `m1_tcm_f7dc_01134238`, two taps (`[4301..]`, `[4304..]`). Tool: `re/dsptap.py`,
`re/dsprdly.py`. A full ARM read of WMEM (0x41300000) with a test program in this slot hung the master once.

**Base instruction is required for the write to reach the ring (2026-10-20, master DSP4 slot 0xF7DC).** A constant
input `0.25` stored with `[4300] ← r4` and read back by a tap of u = 25 gives `0x24000000` (= 0.25) only when the
program contains `1d06 4022 7fe0 c1fc 5100 0000` at its start; without it the same tap reads 0 (6 s after the load,
the tap already sees the ring). So the write `[4300] ← x` alone does not feed the ring: keep the base instruction in
every program that writes the ring. Verified tap age: `u = 25` returns exactly the sample written 409 602 samples
earlier (= 25·16384 + 2), measured on DSP4 with a counter difference stored to bank 5 (`counter − tap`, which is
independent of when the ARM reads it).

**Bank 5 as a sample queue (2026-10-20).** A program that loads `bank5[0]` (`0c03`-style load at `0xA000+k`, one load
per instruction plus 2 NOPs), stores it to `[4300]`, and rotates the queue (`bank5[k] ← bank5[k+1]`, `bank5[15] ←` old
`bank5[0]`) delivers the 16 values in order to the ring: 309 of 309 tap reads were members of the 16-value set. Indices
**2 and 3 of bank 5 are overwritten by values that change every sample** (not the program's own stores; it looks like
DSP-internal state mapped there), so queues must start at index 4 or above (used `0xA004..0xA013`). Sources: `re/dspedmloop.py`
(tap and base), `re/dspcopy.py` (the copier test).

## 10. Execution semantics **[measured, master DSP3]**

| case | result |
|---|---|
| two ops in one instruction | both read old values (`1503 3c01 4cc1`: r4 uses old r3), independent of word order |
| A + B / A + A + B in one instruction (classes 0x0B0, 0x160) | all three read old values [exact, 2026-10-16]: `1603 1c42 a445` (A `r1 = r1 + r2`, B `r2 = max(r1, r5)`) → r1 = old r1 + old r2, r2 = max(old r1, r5); `2c06 1c47 36b0 a445 2800 0000` (A `r1 += r7`, A `r3 += r2·0.5`, B `r2 = max(r1, r5)`) exact. Used for one-instruction-per-point wavetables ([Vol. VI ch. 6](../vol6_reference_designs_applications/custom_osc.md)) and verified by audio on the slaves |
| op + store in one instruction | the store writes the **old** register value |
| program → next program on the same DSP | registers carry over (no reset between the main and slot programs) |
| op + load in one instruction | the op reads the **old** value; the load result is visible to the next instruction |
| op and load writing the same register in one instruction | the register received the **sum** (seen once; avoid) |
| ALU op → next instruction | the result is visible (no latency) |
| `X9bY` → C word | **one instruction of latency** (like a load) [measured 2026-10-17] |
| A op overflow | magnitude wraps mod 512, sign kept; flag V set (§8) |
| **load → ALU op** | an op of the same instruction still sees the old register; the op of the next instruction sees the loaded value [measured: SYSTEM-1 2026-10-23, master DSP3 2026-10-29; [ch. 1](core_architecture.md) §7.3] |
| load → store of the loaded register | not characterised; keep one instruction in between |

## 11. Encoding examples

1. `r3 ← r0 + r1`: unit A, D=3, op=6, X=0, mY=0, Y=1 → `0011 110 000 00 0001` = `3c01`; template `A`
   (class 0x048), length 2 → header `0x048<<5 | 2` = `0902`. Instruction: `0902 3c01`.
2. `r1 ← r0 · 0.5`: D=1, op=2, X=0, mY=3 (imm32), Y=0 → `1430`; imm32 0.5 = `28000000` → 2 I words; length 4 →
   `0904 1430 2800 0000`.
3. `r1 ← 0.5` (imm16): op 0 sub 1: D=1, op=0, X=1, mY=1, Y=0 → `0001 000 001 01 0000` = `1050` → `0903 1050 2800`.
4. `r0 ← [0x0400]; [0x0401] ← r1`: template `TTRR` (0x0F0), length 5: T1 = `0fe0` (load r0, slot 0),
   T2 = `9fe2` (store r1, slot 1), R = `0400`, `0401` → `1e05 0fe0 9fe2 0400 0401`.
5. `r0 ← r1 + r2·(r0 − r1)`: MAC, template `MN` (0x040): w1 D=0 op1=6 X=0 mY=0 Y=r2 (corpus uses `0c0a`);
   w2 Z=−r1 (`9`), op2=001, C=r2, mW=0, W=r1 → `9281` → `0803 0c0a 9281`.
6. `r3 ← |r1| + r2` (unit B): template `B` (class 0x050), word D=3 op=6 X=1 Y=2 → `3c42` → `0a02 3c42`.
6b. Three ops in one instruction, template `AAB` (class 0x160): `r1 ← r1 + r7 | r3 ← r3 + r2·s | r2 ← max(r1, r5)`
   = A `1c47`, A op 3 imm32 `36b0`, B `a445`, one imm32 → `2c06 1c47 36b0 a445 <s hi> <s lo>` (`re/dspasm.py aab()`).

7. A complete program: `re/dspasm.py saw dco2` generates our own naive SAW for the DCO2 slot (pitch in `[2207]` /
   `[2214]`, `f = 440·2^(p − 4.75)`, fs 96 kHz, 2^x by Taylor + 4 squarings, phase kept in the ring (load S + 1,
   store S), wrap without branches: q = max(phase − 1, 0), four times q = min(256·q, 1), phase −= 2q). Installed
   in the C-SAW PLUG-OUT it is in tune within ±1.2 cents from note 36 to 96, harmonics 1/n. Full description:
   [Vol. VI ch. 6](../vol6_reference_designs_applications/custom_osc.md).

## 12. Template table (all classes)

Each class fixes the order of the body words: one letter per word, A/B ALU, M/N MAC, C condition, T transfer,
R reference; immediates (`I`) follow. The layouts were measured with `re/dspslots.py` and resolved with
`re/dsptemplates.py`. The table below is generated from `asm/isa.json` by `python3 asm/isadoc.py` (layout and status
from the file; lengths, uses and the most frequent instruction from the local corpus of Roland programs); do not edit
it by hand.

<!-- isadoc:classes -->
| class | header (most frequent length) | layout (isa.json) | status | lengths in corpus | uses | most frequent instruction |
|---|---|---|---|---|---|---|
| `0x0f0` | `1e05` | `TTRR` + I… | measured | 4, 5 | 3583 | `1e05 afe0 9fe2 0406 0407` |
| `0x048` | `0902` | `A` + I… | measured | 2, 3, 4 | 3530 | `0902 4086` |
| `0x040` | `0807` | `MN` + I… | measured | 3, 4, 5, 6, 7, 8, 9 | 3462 | `0807 3cca 1cb6 002e 3bc7 01a0 1a01` |
| `0x060` | `0c03` | `T` + I… | measured | 2, 3 | 3074 | `0c03 0fe0 a020` |
| `0x050` | `0a02` | `B` + I… | measured | 2, 3, 4 | 1982 | `0a02 cf06` |
| `0x088` | `1107` | `MAN` + I… | measured | 4, 5, 6, 7, 8 | 1164 | `1108 3472 20a1 0036 0001 ae64 0012 7e4f` |
| `0x0a8` | `1503` | `AA` + I… | measured | 3, 4, 5, 6, 7 | 1104 | `1503 2987 7505` |
| `0x270` | `4e0a` | `MATTNRR` + I… | measured | 8, 9, 10, 11, 12, 13 | 1101 | `4e08 640f 4046 afe2 2fe4 0006 041f 0427` |
| `0x0c0` | `1804` | `AT` + I… | measured | 3, 4, 5, 6 | 982 | `1805 0430 99b1 0258 bf25` |
| `0x0a0` | `1405` | `MTN` + I… | measured | 1, 4, 5, 6, 7, 8, 9, 10 | 795 | `1407 2c8f 3fe2 5903 041b a2aa aa00` |
| `0x0b0` | `1603` | `AB` + I… | measured | 3, 4, 5, 6, 7 | 651 | `1603 4086 beeb` |
| `0x158` | `2b07` | `MTTNRR` + I… | measured | 7, 8, 9, 10, 11, 13 | 595 | `2b0a 5d4c 0fe4 1fe6 3a12 3000 0408 040e 3000 0000` |
| `0x0d8` | `1b04` | `BTR` + I… | measured | 3, 4, 5, 6 | 491 | `1b04 ae10 b9b1 0000` |
| `0x0b8` | `1705` | `AC` + I… | measured | 3, 4, 5 | 441 | `1705 0070 0810 0000 0001` |
| `0x3c0` | `7809` | `TTTTRRRR` + I… | measured | 9 | 430 | `7809 8fe0 9fe2 2fe4 1fe6 0400 0401 0271 0204` |
| `0x1f0` | `3e07` | `TTTRR` + I… | measured | 7 | 402 | `3e07 8fe0 9fe2 1fe4 0404 0405 0205` |
| `0x1a0` | `3406` | `ATT` + I… | measured | 5, 6, 7, 8 | 401 | `3406 1c50 99b1 2fe2 0000 a02e` |
| `0x000` | `0001` | `(none)` + I… | exact | 1 | 394 | `0001` |
| `0x328` | `6507` | `ABTTRR` + I… | measured | 5, 6, 7, 8, 9, 10 | 389 | `6505 4f42 ae0e 99b1 b9b2` |
| `0x168` | `2d06` | `AAC` + I… | measured | 4, 5, 6, 7, 8 | 384 | `2d06 6050 6052 0b12 3000 b000` |
| `0x170` | `2e05` | `AAT` + I… | measured | 4, 5, 6, 7, 8, 9 | 350 | `2e04 3cc8 1445 99b1` |
| `0x2a0` | `5408` | `MBTTN` + I… | measured | 8, 9, 10, 11, 12 | 310 | `540c 1c12 af1b afe4 cfe6 7a2b 3000 0438 043d 2333 33c4 c008` |
| `0x188` | `3107` | `ABT` + I… | measured | 4, 5, 6, 7, 8 | 293 | `3107 24a2 ce10 b9b2 0000 4324 3f00` |
| `0x128` | `2509` | `MATN` + I… | measured | 6, 7, 8, 9, 10, 11, 12 | 273 | `2509 65f5 5c22 a9b2 0006 cc00 0042 6153 6200` |
| `0x4c0` | `9809` | `MABTTNRR` + I… | measured | 9, 10, 11, 12, 13, 14, 15 | 260 | `980c 34d2 6416 fe43 bfe8 1fea 0014 2800 2800 7000 041b 041c` |
| `0x140` | `280a` | `MBTN` + I… | measured | 5, 6, 7, 8, 9, 10, 11, 12 | 243 | `280a 1c12 af1b afe4 79a9 3000 0408 0000 00c4 c000` |
| `0x1d0` | `3a06` | `BTT` + I… | measured | 5, 6, 7, 8 | 243 | `3a06 8e10 b9b1 0fe2 0000 0210` |
| `0x090` | `1205` | `MBN` + I… | measured | 4, 5, 6, 7, 8, 9 | 224 | `1205 2512 eee6 0002 3000` |
| `0x348` | `6908` | `ATTTRR` + I… | measured | 7, 8, 9, 10 | 215 | `6908 2c86 afe0 1fe2 3fe4 0216 021a 0218` |
| `0x2c0` | `5809` | `MTTTNRRR` + I… | measured | 9, 11, 12, 13 | 205 | `580b 0812 8fe6 9fe8 2fea 1500 0400 0400 0600 0601 0605` |
| `0x2f8` | `5f09` | `AATT` + I… | measured | 7, 8, 9, 10, 11 | 196 | `5f08 2487 3ee2 afe0 a9b2 0215 0ccc cc00` |
| `0x2e0` | `5c07` | `AABTRC` + I… | measured | 6, 7, 8, 9, 10 | 171 | `5c08 44c4 3472 aec6 dfe0 043b 2b17 217f` |
| `0x5c0` | `b80b` | `AATTTRRR` + I… | measured | 7, 8, 9, 10, 11, 12 | 162 | `b80b 5902 7066 bfe0 1fe2 2fe4 042f 0417 0436 800a 7700` |
| `0x058` | `0b02` | `C` + I… | measured | 2, 3 | 160 | `0b02 00f8` |
| `0x4e0` | `9c0c` | `MATTTNRRR` + I… | measured | 10, 11, 12, 13, 14 | 157 | `9c0e 38d2 7cbc 9fe6 4fe8 5fea 4500 5000 5000 021a 0216 0217 0000 0002` |
| `0x118` | `2307` | `MABN` + I… | measured | 6, 7, 8, 9, 10 | 145 | `2307 5509 0507 fe62 0002 0000 0000` |
| `0x080` | `100b` | `MMNN` + I… | measured | 5, 7, 8, 9, 11 | 118 | `100b 0424 146a 0027 002d 4500 0000 0000 4500 0000 0000` |
| `0x238` | `470e` | `MMTTNNRR` + I… | measured | 9, 11, 12, 13, 14 | 113 | `470b 454c 3454 2fe8 9fea 0001 0016 2800 2800 0261 0251` |
| `0x258` | `4b0a` | `MABTNR` + I… | inferred | 8, 9, 10, 11 | 111 | `4b0a 65f5 75c5 8e22 b9b3 0006 cc00 00d3 d254 1f00` |
| `0x610` | `c20a` | `ABTTTRRR` + I… | measured | 7, 9, 10, 11 | 107 | `c20a 2510 dee6 9fe2 0fe4 1fe6 3000 0443 0409 0410` |
| `0x398` | `7308` | `BTTTRR` + I… | measured | 7, 8, 9 | 105 | `7307 8e09 8fe0 4fe2 b9b1 0250 024c` |
| `0x5a0` | `b40b` | `AABTTRR` + I… | measured | 6, 7, 9, 10, 11, 13 | 95 | `b406 7c0f 5c4d be0c 99b1 b9b3` |
| `0x378` | `6f07` | `BBTTR` + I… | measured | 6, 7, 8 | 88 | `6f07 aee2 be10 cfe2 b9b2 0000 040c` |
| `0x160` | `2c07` | `AAB` + I… | measured | 4, 6, 7, 8, 9, 10 | 87 | `2c07 1447 3490 ce72 7000 0000 0200` |
| `0x2c8` | `5907` | `AAAC` + I… | measured | 7, 9, 10, 11 | 81 | `5907 3482 1050 1052 1f23 0000 3000` |
| `0x198` | `3306` | `ACT` + I… | measured | 5, 6 | 76 | `3306 0420 4082 1280 2333 3300` |
| `0x4c2` | `984c` | `MABTTTNRRR` + I… | measured | 11, 12, 14 | 76 | `984e 3512 146a eee6 afe4 4fe6 2fe8 0003 3000 043a 0437 0438 1555 5500` |
| `0x45a` | `8b4f` | `MMBTTTNN` + I… | inferred | 12, 14, 15 | 72 | `8b4f 3454 2518 eee6 dfea 1fec 5fee 0016 0002 800d 0222 3000 040d 0442 043a` |
| `0x5b8` | `b708` | `AACTTRR` + I… | measured | 7, 8, 9, 10, 11 | 67 | `b707 2503 2510 1a12 99b1 99b2 3000` |
| `0x2f0` | `5e07` | `AACT` + I… | measured | 6, 7, 8 | 63 | `5e07 2040 4c60 0a10 99b2 8041 8900` |
| `0x5a2` | `b44a` | `AABTTTRRR` + I… | measured | 8, 10, 13, 14 | 60 | `b44e 54e6 34f9 ced1 afe0 bfe2 7fe4 0437 043a 043e 03af a600 962f c900` |
| `0x250` | `4a0b` | `MABCN` + I… | inferred | 10, 11 | 48 | `4a0b 04f4 54c2 9e52 1a30 0038 3000 8000 006b 0000 2e3b` |
| `0x340` | `6807` | `ACTT` + I… | measured | 6, 7, 8, 10 | 42 | `6808 1050 1210 6fe2 7fe4 0000 029d 02a7` |
| `0x0e8` | `1d04` | `CT` + I… | measured | 3, 4, 5, 6 | 40 | `1d03 4081 0280` |
| `0x2d8` | `5b07` | `AAB` + I… | measured | 6, 7, 8, 9 | 40 | `5b07 0041 3050 a492 0910 3000 3000` |
| `0x558` | `ab0b` | `MTTTTNRRRR` + I… | measured | 11, 13, 14 | 40 | `ab0b 2c0b bfe2 3fe4 7fe6 4fe8 a2c2 0217 021b c023 c025` |
| `0x190` | `3206` | `ACC` + I… | measured | 4, 5, 6 | 37 | `3206 2060 1a10 0e20 2800 0000` |
| `0x0c8` | `1904` | `BB` + I… | measured | 4 | 36 | `1904 9eef f590 0000` |
| `0x120` | `240a` | `MAN` + I… | measured | 7, 9, 10 | 36 | `240a 0422 0468 1b12 0025 3199 9999 9999 4300 0000` |
| `0x180` | `3006` | `ABC` + I… | measured | 4, 5, 6, 7 | 34 | `3006 0050 8012 1812 b000 0000` |
| `0x598` | `b307` | `AABCTR` + I… | measured | 7, 9, 11 | 33 | `b307 4d08 3ed0 8e41 0930 99b1 3000` |
| `0x568` | `ad08` | `AAACC` + I… | measured | 7, 8, 9 | 32 | `ad08 7040 7050 3052 0e12 1f30 0000 0000` |
| `0x638` | `c709` | `ATTTTRRRR` + I… | measured | 9, 10, 11, 12 | 32 | `c70a 1c47 4fe0 7fe2 bfe4 3fe6 0221 0222 022a a02a` |
| `0x320` | `6407` | `ABCTR` + I… | measured | 7, 8, 9 | 30 | `6407 2060 be08 1b10 b9b2 2b5c 2800` |
| `0x100` | `200a` | `MMBNN` + I… | measured | 10, 11 | 28 | `200a 44a6 54e9 e5d4 0004 0005 0000 2c4f 762f bd2f` |
| `0x130` | `2607` | `MBBN` + I… | measured | 7, 11 | 28 | `260b 1472 aeda 803a 0036 1999 9999 2e66 6666 03fa 0945` |
| `0x268` | `4d0c` | `MACTN` + I… | measured | 8, 12 | 28 | `4d08 54cd 1c52 1a20 2fe4 0004 4200 023d` |
| `0x2e8` | `5d07` | `AACC` + I… | measured | 5, 7 | 28 | `5d07 1050 3052 0f10 1f20 0000 0000` |
| `0x5c2` | `b84b` | `AATTTTRRRR` + I… | measured | 11, 13 | 28 | `b84b 06c7 1505 7fe0 4fe2 5fe4 bfe6 0240 022b c02b 023d` |
| `0x720` | `e40b` | `TTTTTRRRRR` + I… | measured | 11 | 28 | `e40b 8fe0 8fe2 9fe4 2fe6 0fe8 8400 a400 0201 8614 60e1` |
| `0x098` | `1307` | `MN` + I… | measured | 7 | 26 | `1307 65a2 1210 0025 afd7 0a2f eb85` |
| `0x530` | `a60a` | `MBTTTNRRRC` + I… | measured | 10, 12, 13 | 26 | `a60a 640f cf2e afe2 9fe4 2fe6 0006 0417 043d 041f` |
| `0x4c4` | `988d` | `MABTTTTNRRRR` + I… | inferred | 13 | 24 | `988d 640f 5902 ce46 bfe2 9fe4 1fe6 2fe8 0006 043c 043a 0440 0417` |
| `0x4e2` | `9c4e` | `MATTTTNRRRR` + I… | measured | 12, 14 | 24 | `9c4e 08cf 706a afe2 9fe4 3fe6 4fe8 43c0 0417 0409 040f 0416 038d 7f00` |
| `0x608` | `c108` | `ABCTTRR` + I… | inferred | 7, 8, 9, 10 | 22 | `c108 0041 aef2 1310 1fe0 3fe2 0609 0615` |
| `0x220` | `440b` | `MMBTNNR` + I… | inferred | 11, 12 | 20 | `440b 6566 054f fe69 dfe4 0003 0000 0411 0e70 3a00 0000` |
| `0x5a4` | `b48d` | `AABTTTTRR` + I… | measured | 11, 13 | 20 | `b48d 24b6 6dc0 be0d b9b3 4fe0 afe2 bfe4 0404 040e 040a 3000 0000` |
| `0x570` | `ae08` | `AAACT` + I… | measured | 8, 11 | 19 | `ae08 3cc1 4510 4052 1e23 99b1 4200 0000` |
| `0x63a` | `c74d` | `ATTTTTRRRRR` + I… | measured | 12, 13, 14 | 17 | `c74c 0c0a dfe0 9fe2 3fe4 4fe6 7fe8 0616 0615 060a 060b c3e1` |
| `0x512` | `a24d` | `MBBTTTNRRR` + I… | measured | 12, 13 | 16 | `a24d 79a6 ae12 8ee0 b9b2 c9b3 dfe4 7a40 0000 022c 3000 0000 0000` |
| `0x592` | `b24a` | `AABCCT` + I… | measured | 10 | 16 | `b24a 4050 5052 eee6 1e10 1e25 5fe4 3000 2800 0402` |
| `0x1e8` | `3d05` | `CTTRRC` + I… | measured | 5, 7 | 13 | `3d07 4050 0fe2 7fe4 0104 4101 0415` |
| `0x490` | `920c` | `MAACTN` + I… | inferred | 12 | 13 | `920c 75e6 5043 5052 1a23 ffe4 0029 3000 022f 4500 00c4 0000` |
| `0x0e0` | `1c04` | `CC` + I… | measured | 3, 4 | 12 | `1c04 0090 0810 0009` |
| `0x1b0` | `3604` | `BBC` + I… | inferred | 4 | 12 | `3604 8ee1 8ee0 0812` |
| `0x4aa` | `954c` | `MABBTTN` + I… | measured | 12 | 12 | `954c 2476 3cc0 ce12 eee6 b9b3 4fe4 0002 a800 040f 0000 0000` |
| `0x4da` | `9b4e` | `MACTTTN` + I… | inferred | 14 | 12 | `9b4e 253a 3052 1820 dfe4 1fe6 4fe8 0002 b000 0405 0411 0412 0000 0000` |
| `0x514` | `a290` | `MBBTTTTNRRR` + I… | inferred | 16 | 12 | `a290 2d78 bee3 9e0c 5fe2 6fe4 b9b3 9fe6 6f02 0411 0412 040a 0000 0000 0000 0000` |
| `0x56a` | `ad4b` | `AAACCTI` + I… | inferred | 11 | 12 | `ad4b 6d50 6f52 7054 0b12 1a35 7fe6 3000 3000 2800 0403` |
| `0x590` | `b208` | `AABC` + I… | measured | 8 | 12 | `b208 3c20 5044 9ee1 2620 1f40 2800 0000` |
| `0x59c` | `b38e` | `AABCTTTRRRCC` + I… | measured | 14 | 12 | `b38e 4537 3064 ee0f 1820 dfe0 b9b3 efe2 0402 040b b000 002f ae14 7a00` |
| `0x59e` | `b3d0` | `AABCTTTTRRRI` + I… | inferred | 16 | 12 | `b3d0 4539 3066 ee0f 1820 dfe0 5fe2 b9b3 efe4 0402 0438 040b 3000 0000 0000 0000` |
| `0x5b0` | `b609` | `AACCT` + I… | measured | 7, 9 | 12 | `b609 4050 5052 1e10 1e24 5fe4 3000 2c00 0402` |
| `0x5f2` | `be49` | `ABBTTTRR` + I… | measured | 9 | 12 | `be49 0c07 de0a eede b9b2 1fe0 8fe2 0236 0241` |
| `0x612` | `c24a` | `ABTTTTRRRR` + I… | measured | 10, 12 | 12 | `c24a 6d83 9ee9 9fe0 99b1 3fe2 4fe4 0614 0603 060c` |
| `0x632` | `c64b` | `ACTTTTRRRR` + I… | inferred | 11 | 12 | `c64b 3041 1a10 afe0 afe2 5fe4 4fe6 0411 0413 040b 0408` |
| `0x110` | `2208` | `MMTNN` + I… | measured | 7, 8, 10, 11 | 11 | `220b 14a6 55b9 2fe4 0001 0005 060f 2ffb 804a 4bbc 2900` |
| `0x5f0` | `be07` | `ABBTTRR` + I… | measured | 7 | 11 | `be07 3503 de0e 9f2b b9b2 dfe0 0252` |
| `0x55a` | `ab4d` | `MTTTTTNRRRRR` + I… | measured | 13, 15 | 10 | `ab4d 084e dfe2 1fe4 2fe6 6fe8 7fea 23c0 060c 0618 0619 c3e3 c3e4` |
| `0x59a` | `b349` | `AABCTTRRCC` + I… | measured | 9, 12 | 10 | `b34c 4076 4050 be12 1812 b9b3 7fe4 0000 d000 c035 0800 0000` |
| `0x478` | `8f0b` | `MMTTTNNRR` + I… | measured | 11, 14 | 9 | `8f0e 246a 366d cfe4 6fe6 7fe8 0002 0006 02a7 c032 c033 1a6d b442 8000` |
| `0x550` | `aa0c` | `MCTTTNRRR` + I… | inferred | 12, 15 | 8 | `aa0c 158f 4028 9fe2 7fe4 3fe6 0002 0409 0411 0412 0020 0000` |
| `0x572` | `ae4b` | `AAACTTRRRC` + I… | measured | 11 | 8 | `ae4b 4050 4052 05c6 0b12 1fe4 6fe6 3000 0000 02b9 c04c` |
| `0x5ee` | `bdcd` | `ABBCTTTI` + I… | inferred | 13 | 8 | `bdcd 5050 bee3 ee0f 1f14 5fe2 9fe4 b9b3 efe6 2800 0405 040f 040c` |
| `0x6c8` | `d908` | `BBTTTRR` + I… | measured | 8 | 8 | `d908 ae10 8ee0 b9b1 c9b2 dfe2 0000 022c` |
| `0x240` | `4809` | `MAACN` + I… | measured | 9 | 7 | `4809 094f 7de2 7de5 1a23 03c0 8200 0000 1000` |
| `0x630` | `c608` | `ACTTTRR` + I… | inferred | 8 | 7 | `c608 4d4c 0a20 2fe0 99b1 5fe2 02c8 02c4` |
| `0x108` | `2109` | `MMNN` + I… | measured | 9 | 6 | `2109 140b 140b 0b12 0024 0027 2780 00a7 8000` |
| `0x178` | `2f08` | `ABBCCC` + I… | measured | 4, 8 | 5 | `2f08 1470 aeda 8034 3000 0000 03fa 0945` |
| `0x1b8` | `3705` | `BBT` + I… | measured | 5, 6 | 5 | `3705 ce10 aed1 b9b1 a400` |
| `0x310` | `6206` | `ABBT` + I… | measured | 5, 6 | 5 | `6206 3503 de0e 9f2b dfe0 0102` |
| `0x722` | `e44d` | `TTTTTTRRRRRR` + I… | measured | 13 | 5 | `e44d afe0 8fe2 9fe4 0fe6 1fe8 2fea 8406 0204 0289 02dd 0206 8008` |
| `0x148` | `2907` | `MCCN` + I… | measured | 7 | 4 | `2907 0c0f 00c1 1e20 f494 3000 3000` |
| `0x248` | `490b` | `MABBN` + I… | inferred | 11 | 4 | `490b 1462 6d40 bee3 f1e8 0025 ae66 6600 0000 a999 9900` |
| `0x47a` | `8f4e` | `MMTTTTNNRR` + I… | inferred | 14 | 4 | `8f4e 140f 6d00 2fe6 5fe8 7fea 4fec 0002 b014 b000 0612 0613 c3e2 c3e4` |
| `0x586` | `b0cd` | `AABBCTTTIIRR` + I… | inferred | 13 | 4 | `b0cd 5050 5052 bee3 ee0f 1f12 9fe4 b9b4 efe6 2400 0000 040f 040c` |
| `0x4b8` | `970c` | `MABCTNR` + I… | measured | 12 | 2 | `970c 65e4 4077 be12 1820 b9b3 0006 c800 ac00 0010 0000 0000` |
| `0x5c4` | `b88d` | `AATTTTTRRRRR` + I… | measured | 13 | 2 | `b88d 0487 14c7 8fe0 9fe2 2fe4 3fe6 7fe8 060e 061f 060d 061e c1fd` |
| `0x60c` | `c18e` | `ABCTTTTRRRRC` + I… | measured | 14 | 2 | `c18e 4078 be10 1810 b9b2 afe2 5fe4 6fe6 c200 0242 023c c034 2400 0000` |
| `0x2d0` | `5a09` | `AABB` + I… | measured | 9 | 1 | `5a09 0422 7890 b525 ce45 3000 1eb8 5129 9999` |
| `0x318` | `6305` | `AB` + I… | measured | 5 | 1 | `6305 5044 9ee1 2310 1a30` |
| `0x510` | `a20b` | `MBBTTN` + I… | inferred | 11 | 1 | `a20b 79a4 ae12 8ee0 b9b2 c9b3 79c0 0000 3000 0000 0000` |
| `0x60a` | `c14d` | `ABCTTT` + I… | measured | 13 | 1 | `c14d 4078 be10 1810 afe2 5fe4 6fe6 c200 00f2 00ec c05b 2400 0000` |
| `0x1d8` | `3b06` | `CCCCC` + I… | measured |  | 0 |  |
| `0x208` | `4106` | `MMACN` + I… | inferred |  | 0 |  |
| `0x2b8` | `5708` | `MCTTNRR` + I… | measured |  | 0 |  |
| `0x388` | `7105` | `BCCT` + I… | measured |  | 0 |  |
| `0x3b8` | `7709` | `CTTTRRCC` + I… | measured |  | 0 |  |
| `0x458` | `8b0a` | `MMBTTNNRR` + I… | measured |  | 0 |  |
| `0x482` | `9048` | `MAAACTN` + I… | inferred |  | 0 |  |
| `0x4b4` | `9689` | `MAACCTTN` + I… | measured |  | 0 |  |
| `0x4ba` | `974d` | `MAACTTNRRCCC` + I… | measured |  | 0 |  |
| `0x566` | `accd` | `AAABCTTTRRRC` + I… | measured |  | 0 |  |
| `0x574` | `ae8b` | `AAACTTTRRR` + I… | measured |  | 0 |  |
| `0x584` | `b08b` | `AAAACTTRRC` + I… | measured |  | 0 |  |
| `0x588` | `b109` | `AAAATRCC` + I… | measured |  | 0 |  |
| `0x5a6` | `b4cc` | `AAATTTTTRRR` + I… | measured |  | 0 |  |
| `0x600` | `c006` | `AACCT` + I… | measured |  | 0 |  |
| `0x6e8` | `dd06` | `ACTTT` + I… | measured |  | 0 |  |

139 classes in isa.json.
<!-- /isadoc:classes -->

## 13. Method (how to verify anything here)

`re/dsppeek.py` (PLUG-OUT region 0 built with `-DPEEK`: MIDI pitch bend + CC1 in, SysEx out) lets the PC write
any master memory and read it back. `re/dspmaster.py` loads a test program into a context (`DSPCTX=dsp3`: the idle
master DSP3 main program, default; `efdc`: master DSP4 slot 0xEFDC, shared with the global effects — writing its
data memory once silenced the whole instrument until a power cycle), the original is saved and restored
(`dspmaster.py --restore` repairs an interrupted run). `re/dspprobe.py` sets r0..r7, runs instruction lines and
stores r0..r7 to bank 0 [0780..0787], which is read back (majority of 7 reads). Because bank 0 is a ring (§9),
the stored values leave a trail and the ARM read returns the value stored at some recent sample: fine for constant
results, but measuring a value that changes per sample needs a coherent 16-word read (the ring rotates ~10 µs per
word) or a read-back through memory the program itself maintains. `re/dspfit.py`, `re/dspclass.py`, `re/dspnlfit.py` fit exact formulas over random operands;
`re/dspslots.py` maps template positions; `re/dsptemplates.py` builds §12. Only one tool may use the MIDI port at a
time.

**Re-assembly check (2026-10-20).** In `asm/`, `disasm.program` assembles every decoded line back and keeps it
verbatim if the words differ. On the corpus (132 files) every decoded line re-assembles and every file comes back
identical (`python3 -m unittest discover -s tests`). That checks the word layout against itself; the meaning of each
field is what the probes above and DISCREPANCIES measure.

## 13b. All Roland DSP programs (2026-10-17)

`re/dsp_corpus_all.py` extracts every program (descriptor tables; signature scan as a fallback) from the
SYSTEM-8 base engine (`base-engine/up/p1y01_{p0s,pl1m,pl1s,pl2m}`), the OS (`m1_tcm`, `m1_sdram`, `s_data`), all
nine SYSTEM-8 PLUG-OUTs (JUNO-60/106, JUPITER-4/8, JX-3P, PROMARS, SH-101, SH-2, SYSTEM-100: region 1; region 0
holds no DSP code) → **212 unique programs** (84 = the old corpus, 116 base engine, 12 from JUPITER-8 / PROMARS /
SH-2); `work/corpus_all` holds the 128 new ones. `re/dsp_s1.py`: the SYSTEM-1 images of SH-2 / PROMARS use the
same ISA (4 programs, start marker `ff71`, no descriptor table) → `work/corpus_s1`. The base engine adds 20 classes
not seen before; their templates were inferred from the instances (fe slots, `b9bY` units, C words, +2 per extra
T between neighbouring classes) and are marked `inferred` in `re/dsp_templates.txt` (not measured on hardware).

**Templates measured (2026-10-17)**: every inferred template was run on master DSP3 with Roland's own instance
(`re/dspverify.py --inferred`: references moved to scratch words of the same bank and preloaded, random
registers and flags, registers and written memory compared with `dspisa.run`); `re/dspslots.py` position maps
for 0x240, 0x248, 0x250, 0x478, 0x47a, 0x4aa, 0x4da. 19 of 20 agree in every trial (0x56a: only its out-of-range
C field, §5.4); the hand-fixed templates 0x090, 0x098, 0x130, 0x178, 0x208, 0x240, 0x572, 0x592 agree too.
Random regression over the whole corpus (100 instructions without jumps / special transfer modes, 2 random cases each): **100 / 100 agree** with `dspisa.py`.

Coverage (measured with `archive/dspisa.py cov DIR`): first corpus (84 programs) 98.6 %, new programs 97.8 %, SYSTEM-1 98.6 %; **the only
words left are the section markers `ff70`/`ff71`/`ef75`/`ef74`** (no visible effect).

## 14. Open items (do not emit)

Open discrepancies, with the probe that would close each: [Vol. II ch. 8](isa_discrepancies.md)
(D1–D10). Open hardware items: [Vol. II ch. 1](core_architecture.md) §11. Everything decoded in the corpus of all Roland programs
(212 SYSTEM-8 + 4 SYSTEM-1) except the section markers, which have no effect (§6.4). Timing: every instruction
costs one cycle whatever it contains (1 or 2 MACs, up to 6 transfers; `re/dspbudget.py mac|mac2|xfer4|xfer6`),
a taken jump one more.

## 15. Appendix: assembler mnemonic index

Every variant the assembler (`asm/asm.py`) and the disassembler (`asm/disasm.py`) know, in the order of
`asm/isa.json`: form group, mnemonic, the field values that select it, result, status. Generated from `asm/isa.json`
by `python3 asm/isadoc.py`; do not edit it by hand. What each `mac_*` computes is its result column and §7.

<!-- isadoc:mnemonics -->
| group | mnemonic | selected by | result | status |
|---|---|---|---|---|
| `alu_a` | `add3` | op=1, D<8 | rD <- rD + rX + Y | exact |
| `alu_a` | `mul` | op=2, D<8 | rD <- rX * Y | exact |
| `alu_a` | `mad` | op=3, D<8 | rD <- rD + rX * Y (unit A op 3; see mac_* for the MAC unit) | exact |
| `alu_a` | `ymacd` | op=4, D<8 | rD <- Y + rX * rD | exact |
| `alu_a` | `ymsubd` | op=5, D<8 | rD <- Y - rX * rD | exact |
| `alu_a` | `addxy` | op=6, D<8 | rD <- rX + Y | exact |
| `alu_a` | `subxy` | op=7, D<8 | rD <- Y - rX | exact |
| `alu_a` | `nop` | op=0, X=0, mode=0x0, Y=0, D<8 | no effect (the one-word 0001 is class 0x000 instead) | exact |
| `alu_a` | `mov` | op=0, X=1, D<8 | rD <- Y (also loads an immediate) | exact |
| `alu_a` | `newton` | op=0, X=2, D<8 | rD <- rD*r(D+1); r(D+1) <- 2-r(D+1)*rY; rY <- r(D+1)*rY | exact |
| `alu_a` | `addh4` | op=0, X=4, D<8 | rD <- rD + Y + h4 | exact |
| `alu_a` | `addh5` | op=0, X=5, D<8 | rD <- rD + Y + h5 | exact |
| `alu_a` | `add` | op=0, X=6, D<8 | rD <- rD + Y | exact |
| `alu_a` | `sub` | op=0, X=7, D<8 | rD <- rD + Y (identical to sub-code 6; the spec keeps them apart) | exact |
| `alu_a` | `add3r` | op=0, D<8 | r(D-8) <- rX + Y + r[op]; only measured in a single-op class (spec 6.1) | exact |
| `alu_a` | `sum8` | D≥8 | r(D-8) <- X + Y + r[op] | measured |
| `alu_b` | `and` | D<8, mode=0x0, b11_8=0 | rD <- rX & Y (bit operations work mod 512) | exact |
| `alu_b` | `andn` | D<8, mode=0x0, b11_8=2 | rD <- ~rX & Y | exact |
| `alu_b` | `or` | D<8, mode=0x0, b11_8=4 | rD <- rX \| Y | exact |
| `alu_b` | `orn` | D<8, mode=0x0, b11_8=6 | rD <- ~rX \| Y | exact |
| `alu_b` | `xor` | D<8, mode=0x0, b11_8=8 | rD <- rX ^ Y | exact |
| `alu_b` | `xnor` | D<8, mode=0x0, b11_8=10 | rD <- ~(rX ^ Y) | exact |
| `alu_b` | `absadd` | D<8, b11_8∈{12,13} | rD <- \|rX\| + Y | exact |
| `alu_b` | `subabs` | D<8, b11_8∈{14,15} | rD <- Y - \|rX\| | exact |
| `alu_b` | `min` | D≥8, b11_8∈{0,1} | rD <- min(rX, Y) | exact |
| `alu_b` | `max` | D≥8, b11_8∈{4,5} | rD <- max(rX, Y) | exact |
| `alu_b` | `minn` | D≥8, b11_8∈{2,3} | rD <- min(-rX, Y) | measured |
| `alu_b` | `maxn` | D≥8, b11_8∈{6,7} | rD <- max(-rX, Y) | measured |
| `alu_b` | `absx` | D≥8, b11_8∈{8,9}, mode=0x0 | rD <- \|rX\| | exact |
| `alu_b` | `absy` | D≥8, b11_8∈{10,11}, mode=0x0 | rD <- \|Y\| | exact |
| `alu_b` | `cmp` | D≥8, b11_8=14, e_family=0 | flags of rD + Y, nothing written (partner of setflags) | measured |
| `alu_b` | `divseed` | D≥8, b11_8=14, e_family=2, Y<8 | r(d) <- rX sign(rY) 2^-e, r(d+1) <- R[m], rY <- m (\|rY\| = m 2^e, m in [0.5, 1)); X even (r0 r2 r4 r6) | exact |
| `alu_b` | `bic` | D≥8, b11_8=14, e_family=1 | rD <- +-rY | fitted |
| `alu_b` | `shl` | D≥8, b11_8=12, mode=0x1 | rD <- rX * 2^Y (Y is the 4-bit field, not a register) | measured |
| `alu_b` | `shr` | D≥8, b11_8=12, mode=0x2 | rD <- rX * 2^-Y | measured |
| `alu_b` | `shrn` | D≥8, b11_8=12, mode=0x3 | rD <- rX * 2^-(16+Y) | measured |
| `alu_b` | `frexp` | D≥8, b11_8=12, mode=0x0 | rD <- mantissa in [0.5,1), rY <- exponent | measured |
| `alu_b` | `ph_add` | D≥8, b11_8=14, Y<8, mode=0x0, e_family=3 | rD <- rD + rY | measured |
| `alu_b` | `ph_sub` | D≥8, b11_8=14, Y≥8, mode=0x0, e_family=3 | rD <- rD - rY | measured |
| `alu_b` | `ph_abs` | D≥8, b11_8=14, Y<8, mode=0x1, e_family=3 | rD <- \|rY\| | measured |
| `alu_b` | `ph_clamp` | D≥8, b11_8=14, Y≥8, mode=0x1, e_family=3 | rD <- clamp(rY, -1, 1) | measured |
| `alu_b` | `ph_wrap` | D≥8, b11_8=14, Y<8, mode=0x2, e_family=3 | rD <- wrap(rY) into [-1, 1) | measured |
| `alu_b` | `ph_tri` | D≥8, b11_8=14, Y≥8, mode=0x2, e_family=3 | rD <- 1 - 2\|wrap(-rY - 1.5)\| | measured |
| `alu_b` | `ph_mod2` | D≥8, b11_8=14, Y<8, mode=0x3, e_family=3 | rD <- (rY mod 2) / 2 | measured |
| `alu_b` | `ph_neg` | D≥8, b11_8=14, Y≥8, mode=0x3, e_family=3 | rD <- -rY | measured |
| `sfu` | `recipseed` | D≥8, b11_8=15, f_family=0, mode=0x0, Y<8 | rD <- sign*2^-e (0 when 2^-e >= 512), r(D+1) <- ~1/m (exactly the seed ROM value R[k]), rY <- m (\|rY\| = m 2^e) | measured |
| `sfu` | `expsplit` | D≥8, b11_8=15, f_family=0, mode=0x1, Y≥8 | rD <- e^floor(rY) (device ROM, n = -19..6; 0 below, saturated above), rY <- frac(rY) | measured |
| `sfu` | `sign` | D≥8, b11_8=15, f_family=0, mode=0x2, Y≥8 | rD <- +-1 | measured |
| `sfu` | `floor` | D≥8, b11_8=15, f_family=0, mode=0x3, Y≥8 | rD <- floor(rY) | measured |
| `sfu` | `ceil` | D≥8, b11_8=15, f_family=0, mode=0x3, Y<8 | rD <- ceil(rY) | measured |
| `sfu` | `sqrtsplit` | D≥8, b11_8=15, f_family=0, mode=0x1, Y<8 | rY <- m in [1,4), r(D+1) <- ~1/sqrt m, r(D+3) <- sign*2^-k (measured; emu/dspcore.h) | measured |
| `sfu` | `log2split` | D≥8, b11_8=15, f_family=0, mode=0x2, Y<8 | rD <- exponent, rY <- mantissa in [0.75,1.5) | measured |
| `sfu` | `atanhsplit` | D≥8, b11_8=15, f_family=0, mode=0x0, Y≥8 | rD <- (x-1)/2^k, rY <- (x+1)/2^k, rD+1 <- ~1/m | measured |
| `f_words` | `f4` | D≥8, b11_8=15, f_family=1, mode=0x0 | rD <- r(Y & 7) | measured |
| `f_words` | `f5` | D≥8, b11_8=15, f_family=1, mode=0x1 | f5 form (f5c = delay-ring position when Y = c) | measured |
| `f_words` | `f6` | D≥8, b11_8=15, f_family=1, mode=0x2 | rD <- (rD + h(Y-8)) mod 512 | measured |
| `f_words` | `f7` | D≥8, b11_8=15, f_family=1, mode=0x3 | section marker, no effect | measured |
| `mac` | `mac_p1` | op1=2, op2=0, coefreg=0 | rD <- rX*Y + W | exact |
| `mac` | `mac_p1b9` | op1=2, op2=0, coefreg=1 | rD <- rX*Y + W | inferred |
| `mac` | `mac_p1c0` | op1=2, op2=0, coefreg=0 | rD <- rX*Y + W | inferred |
| `mac` | `mac_p1c1` | op1=2, op2=1, coefreg=0 | rD <- rX*Y + W | inferred |
| `mac` | `mac_p1c2` | op1=2, op2=2, coefreg=0 | rD <- rX*Y + W | measured |
| `mac` | `mac_p1c3` | op1=2, op2=3, coefreg=0 | rD <- rX*Y + W | measured |
| `mac` | `mac_p1w2` | op1=3, op2=0, coefreg=0 | rD <- -rX*Y + W | exact |
| `mac` | `mac_p1w2b9` | op1=3, op2=0, coefreg=1 | rD <- -rX*Y + W | inferred |
| `mac` | `mac_p1w2c0` | op1=3, op2=0, coefreg=0 | rD <- -rX*Y + W | inferred |
| `mac` | `mac_p1w2c1` | op1=3, op2=1, coefreg=0 | rD <- -rX*Y + W | inferred |
| `mac` | `mac_p1w2c2` | op1=3, op2=2, coefreg=0 | rD <- -rX*Y + W | measured |
| `mac` | `mac_p1w2c3` | op1=3, op2=3, coefreg=0 | rD <- -rX*Y + W | measured |
| `mac` | `mac_p1w` | op1=4, op2=0, coefreg=1 | rD <- rX*Y + rC*Z | measured |
| `mac` | `mac_p1wc0` | op1=4, op2=0, coefreg=0 | rD <- rX*Y + rC*Z | measured |
| `mac` | `mac_p1wc1` | op1=4, op2=1, coefreg=0 | rD <- rX*Y + imm16*Z | inferred |
| `mac` | `mac_p1wc2` | op1=4, op2=2, coefreg=0 | rD <- rX*Y + imm24*Z | measured |
| `mac` | `mac_p1wc3` | op1=4, op2=3, coefreg=0 | rD <- rX*Y + imm32*Z | measured |
| `mac` | `mac_p1p2w` | op1=5, op2=0, coefreg=1 | rD <- -rX*Y + rC*Z | measured |
| `mac` | `mac_p1p2wc0` | op1=5, op2=0, coefreg=0 | rD <- -rX*Y + rC*Z | inferred |
| `mac` | `mac_p1p2wc1` | op1=5, op2=1, coefreg=0 | rD <- -rX*Y + imm16*Z | inferred |
| `mac` | `mac_p1p2wc2` | op1=5, op2=2, coefreg=0 | rD <- -rX*Y + imm24*Z | measured |
| `mac` | `mac_p1p2wc3` | op1=5, op2=3, coefreg=0 | rD <- -rX*Y + imm32*Z | measured |
| `mac` | `mac_p1p2` | op1=6, op2=0, coefreg=1 | rD <- rX + W + rC*Z | measured |
| `mac` | `mac_p1p2c0` | op1=6, op2=0, coefreg=0, modeY=0 | rD <- rX + W + Z (no products: op2 = 000, op1 = 6/7; spec 7) | measured |
| `mac` | `mac_p1p2c0y` | op1=6, op2=0, coefreg=0, modeY≥1 | rD <- rX*Y + W + c*Z (immediate Y with bit 9 clear: products kept; not measured) | inferred |
| `mac` | `mac_p1p2c1` | op1=6, op2=1, coefreg=0 | rD <- rX*Y + W + imm16*Z | inferred |
| `mac` | `mac_p1p2c2` | op1=6, op2=2, coefreg=0 | rD <- rX*Y + W + imm24*Z | measured |
| `mac` | `mac_p1p2c3` | op1=6, op2=3, coefreg=0 | rD <- rX*Y + W + imm32*Z | measured |
| `mac` | `mac_p1p2wp2` | op1=7, op2=0, coefreg=1 | rD <- -rX + W + rC*Z | measured |
| `mac` | `mac_p1p2wp2c0` | op1=7, op2=0, coefreg=0, modeY=0 | rD <- -rX + W + Z (no products: op2 = 000, op1 = 6/7; spec 7) | measured |
| `mac` | `mac_p1p2wp2c0y` | op1=7, op2=0, coefreg=0, modeY≥1 | rD <- -rX*Y + W + c*Z (immediate Y with bit 9 clear: products kept; not measured) | inferred |
| `mac` | `mac_p1p2wp2c1` | op1=7, op2=1, coefreg=0 | rD <- -rX*Y + W + imm16*Z | inferred |
| `mac` | `mac_p1p2wp2c2` | op1=7, op2=2, coefreg=0 | rD <- -rX*Y + W + imm24*Z | measured |
| `mac` | `mac_p1p2wp2c3` | op1=7, op2=3, coefreg=0 | rD <- -rX*Y + W + imm32*Z | measured |
| `mac` | `mac_p1c1r` | op1=2, op2=1, coefreg=1 | rD <- rX*Y + W | inferred |
| `mac` | `mac_p1c2r` | op1=2, op2=2, coefreg=1 | rD <- rX*Y + W | inferred |
| `mac` | `mac_p1c3r` | op1=2, op2=3, coefreg=1 | rD <- rX*Y + W | inferred |
| `mac` | `mac_p1w2c1r` | op1=3, op2=1, coefreg=1 | rD <- -rX*Y + W | inferred |
| `mac` | `mac_p1w2c2r` | op1=3, op2=2, coefreg=1 | rD <- -rX*Y + W | inferred |
| `mac` | `mac_p1w2c3r` | op1=3, op2=3, coefreg=1 | rD <- -rX*Y + W | inferred |
| `mac` | `mac_p1wc1r` | op1=4, op2=1, coefreg=1 | rD <- rX*Y + imm16*Z | inferred |
| `mac` | `mac_p1wc2r` | op1=4, op2=2, coefreg=1 | rD <- rX*Y + imm24*Z | inferred |
| `mac` | `mac_p1wc3r` | op1=4, op2=3, coefreg=1 | rD <- rX*Y + imm32*Z | inferred |
| `mac` | `mac_p1p2wc1r` | op1=5, op2=1, coefreg=1 | rD <- -rX*Y + imm16*Z | inferred |
| `mac` | `mac_p1p2wc2r` | op1=5, op2=2, coefreg=1 | rD <- -rX*Y + imm24*Z | inferred |
| `mac` | `mac_p1p2wc3r` | op1=5, op2=3, coefreg=1 | rD <- -rX*Y + imm32*Z | inferred |
| `mac` | `mac_p1p2c1r` | op1=6, op2=1, coefreg=1 | rD <- rX*Y + W + imm16*Z | inferred |
| `mac` | `mac_p1p2c2r` | op1=6, op2=2, coefreg=1 | rD <- rX*Y + W + imm24*Z | inferred |
| `mac` | `mac_p1p2c3r` | op1=6, op2=3, coefreg=1 | rD <- rX*Y + W + imm32*Z | inferred |
| `mac` | `mac_p1p2wp2c1r` | op1=7, op2=1, coefreg=1 | rD <- -rX*Y + W + imm16*Z | inferred |
| `mac` | `mac_p1p2wp2c2r` | op1=7, op2=2, coefreg=1 | rD <- -rX*Y + W + imm24*Z | inferred |
| `mac` | `mac_p1p2wp2c3r` | op1=7, op2=3, coefreg=1 | rD <- -rX*Y + W + imm32*Z | inferred |
| `transfer` | `ld` | mode=0xfe, R<8 | rR <- mem[ref] | exact |
| `transfer` | `st` | mode=0xfe, R≥8 | mem[ref] <- rR | exact |
| `transfer` | `ld` | mode∈{0xff,0x3e,0xe,0x10,0x12}, R<8 | same address as fe; the assembler always writes fe | measured |
| `transfer` | `st` | mode∈{0xff,0x3e,0xe,0x10,0x12}, R≥8 | same address as fe | measured |
| `transfer` | `ldidx` | mode=0x28, R<8 | rR <- bank0[floor(h6*16384) + Y] | exact |
| `transfer` | `stidx` | mode=0x28, R≥8 | bank0[floor(h6*16384) + Y] <- rR | exact |
| `transfer` | `ldb5` | mode=0xda, R<8 | rR <- bank5[floor(h6*16384) + Y] | exact |
| `transfer` | `ldb5` | mode=0xde, R≥8 | bank5[floor(h6*16384) + Y] <- rR | exact |
| `transfer` | `ldb5h7` | mode=0xdb, R<8 | rR <- bank5[floor(h7*16384) + Y] | exact |
| `transfer` | `ldb5h7` | mode=0xdf, R≥8 | bank5[floor(h7*16384) + Y] <- rR | exact |
| `transfer` | `ldb5d` | mode∈{0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7} | rR <- bank5[N*16 + Y] with N = mode & 7 | exact |
| `transfer` | `setflags` | mode=0x9b, R≥8 | flags of unit Y of this instruction -> condition group X-8 (X = 9..d -> groups 1..5) | measured |
| `transfer` | `tapreq` | mode=0xa9 | delay-tap request at address rR; results land in bank 2 at [h5..h5+2] | measured |
| `cond` | `cond` |  | if cond then unit skip_true does not execute, else unit skip_false does not | measured |
| `jmp` | `jmp` | tag=9 | jump by the offset stored in a slot word | measured |
| `hreg` | `hset` | T=0, tag=4 | h4 <- value (delay base for f6c) | measured |
| `hreg` | `hset` | T=1, tag=4 | h5 <- value (tap-result address) | measured |
| `hreg` | `hset` | T=2, tag=4 | h6 <- value (index of mode-28 transfers) | measured |
| `hreg` | `hset` | T=3, tag=4 | h7 <- value | measured |
| `empty` | `noop` |  | the one-word instruction 0001 (class 0x000) | exact |
| `raw` | `raw` |  | verbatim body words (an encoding without a known meaning) | raw |

130 variants in 11 form groups. Notes of the forms (`$doc` in isa.json):
* `alu_a` $doc_operands: X is printed only for the ops that use it as a source (add3..subxy); for the op-0 sub-codes it is the sub-code itself and the operand is Y, which may be an immediate.
* `alu_a` $doc_x: None
* `alu_a` $doc_arity: Operands are the form's list unless a variant gives `shows`; the op-0 sub-codes fix X in the encoding, so their source form is `D, Y` (see `shows` on those variants).
* `alu_b` $doc_x: The nibble at 11-8 is op*2 + X[2] (spec 5.1), so the X field only holds X[1:0]; decoders rebuild X from both.
* `alu_b` $doc_nibble_e: Nibble E is split by bits 7-6 (cmp 00, bic 01, phase shaping 11). Those bits are X[1:0] of the ALU layout, so those variants carry their own 'e_family' field: the overlap is stated, not inferred.
* `alu_b` $doc_d: This family encodes D = 8 + d (spec 6.3/6.4): the source names the register r(d) and the field is 8 + d.
* `sfu` $doc: D = 8+d, nibble F, bits 7-6 = 00. The function comes from the mode bits 5-4. Section 6.4 gives cf06 (Y bit 3 = 0) as the reciprocal seed and cf16/cf26/cf36 (Y bit 3 = 1) as the sqrt split / log2 split / ceiling, so the function is entirely those two bits; Y bit 3 is NOT an independent variant selector there. The atanh split is the reciprocal-seed form with a logarithm's operand.
* `sfu` $doc_x: Nibble F: bits 11-8 = 1111, bits 7-6 select the family (00 special functions, 01 f4..f7), bits 5-4 the function/sub-op. The spec does not name bits 8-6, so they are kept as free bits.
* `sfu` $doc_mode: These forms reuse bits 5-4 (the `mode` field of the ALU layout) as a function/sub-op selector, so they take no immediate: the selector sits in that field, not an operand mode.
* `sfu` $doc_selector: bits 5-4 (the `mode` field of the ALU layout) are a function/sub-op selector here, not an operand mode: neither form takes an immediate.
* `sfu` $doc_d: This family encodes D = 8 + d (spec 6.3/6.4): the source names the register r(d) and the field is 8 + d.
* `f_words` $doc_x: Nibble F: bits 11-8 = 1111, bits 7-6 select the family (00 special functions, 01 f4..f7), bits 5-4 the function/sub-op. The spec does not name bits 8-6, so they are kept as free bits.
* `f_words` $doc_mode: These forms reuse bits 5-4 (the `mode` field of the ALU layout) as a function/sub-op selector, so they take no immediate: the selector sits in that field, not an operand mode.
* `f_words` $doc_selector: bits 5-4 (the `mode` field of the ALU layout) are a function/sub-op selector here, not an operand mode: neither form takes an immediate.
* `f_words` $doc_d: This family encodes D = 8 + d (spec 6.3/6.4): the source names the register r(d) and the field is 8 + d.
* `mac` $doc: spec 7. The word is fully determined by (op1, op2): op1 bit 0 negates P1, bit 1 adds W, bit 2 adds P2; with op2 = 000 and op1 in 6/7 there are no products (rD <- +-rX + W + Z) and op1 in 4/5 keeps both; op2 = 001 multiplies in the W modes.
* `mac` $doc_c: C is the coefficient register, used only when the coefficient mode is 0 (spec 5.2). With an immediate coefficient its bits carry no register, so they are declared free rather than 0.
* `mac` $doc_confidence: op2[11:10] (coefficient mode) and coefreg[9] are independent in the *encoding*: the corpus shows every combination occurring. That does not make the *semantics* of each combination known. Each variant therefore carries its own status, and the evidence shows which kind of proof supports it: a formula measured on hardware vs. a pattern seen in code.
* `mac` $doc_free: Fields marked {"free": true} are bits the encoding leaves unused for that variant (C, Z, W when a term is absent). They are not required to be zero: no measurement says the hardware ignores them, so the decoder accepts any value and the encoder must preserve whatever it was given.
* `mac` $doc_coefreg: bit 9 (spec 5.2) is independent of the coefficient mode: the corpus has both states in every mode, so every combination is declared. Those with bit 9 set are inferred.
* `transfer` $doc_store: spec 5.3: load vs store is the R field alone (0-7 load, 8-15 store r(R-8)); the mode byte is fe in both cases.
* `transfer` $doc_shows: A plain access (`fe`) takes the register and a reference [addr]; the indexed / bank-5 / flag / tap forms take the register and a number for their Y field (spec 5.3, 9.0).
* `cond` $doc_neg: `neg` is not written: the source says `!cond3` and the assembler sets it. A trailing 1 or 0 is still accepted for a hand-written line that spells it out.
* `jmp` $doc: 009Y: the signed 16-bit value at slot Y>>1 is the offset in 16-bit words from the instruction end
* `jmp` $doc_slot: spec 5.4b: the offset is the signed 16-bit word at slot Y>>1 of the immediate stream / free slots. In a class with no R position (0x058 = C + I...) it is one of the trailing I words.
* `hreg` $doc: 40TY: T selects h4..h7, mode bits 5-4: 0 register, 1 imm16, 2 imm24, 3 imm32 (reads 0). Values are integers = value*16384
<!-- /isadoc:mnemonics -->
