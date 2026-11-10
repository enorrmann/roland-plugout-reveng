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

## 12. Template table (all classes of the corpus)

Measured with `re/dspslots.py` (+ `ab` pass for unit A vs B) and resolved with `re/dsptemplates.py` from each
class's most frequent length. Other lengths of a class differ in the number of trailing `I` words (verified for
the measured examples; [inferred] in general). Templates with a `C` inside the slot area, or several `C`, came
from the resolver's fallback and should be re-checked before an assembler emits them.

**2026-10-20.** Where a row here differs from `asm/isa.json`, the table keeps the layout that reassembles more
corpus instances (`asm/tests/class_layouts.py`). Class `0x0e0` is `CC`: its `C` words are a jump and a condition,
and the jump's offset word is in the slot array (§5.4b). Rows marked † are contradicted by the corpus (their spec
layout reassembles 0 instances, the table's reassembles 7–16): DISCREPANCIES D4. Classes `0x45a`, `0x4aa` and `0x60a`
were missing and are added.

| class | header (len n) | measured template (len) | fixed part | lengths in corpus | uses | most frequent instruction |
|---|---|---|---|---|---|---|
| `0x0f0` | `1e05` | `TTRR` (5) | `TTRR` + I… | 5 | 2611 | `1e05 0fe0 1fe2 2200 2200` |
| `0x060` | `0c03` | `TR` (3) | `TR` + I… | 2, 3 | 2217 | `0c03 2fe0 020c` |
| `0x048` | `0902` | `A` (2) | `A` + I… | 2, 3, 4 | 1700 | `0902 4086` |
| `0x040` | `0803` | `MN` (3) | `MN` + I… | 3, 4, 5, 6, 7, 8 | 1537 | `0803 0809 5280` |
| `0x050` | `0a02` | `B` (2) | `B` + I… | 2, 3, 4 | 1066 | `0a02 ff70` |
| `0x0a0` | `1405` | `MTNR` (5) | `MTNR` + I… | 4, 5, 6, 7, 8, 9, 10 | 725 | `1404 0c4a 6a90 8280` |
| `0x0a8` | `1503` | `AA` (3) | `AA` + I… | 3, 4, 5, 6, 7 | 673 | `1503 2987 7505` |
| `0x088` | `1107` | `MANIII` (7) | `MAN` + I… | 4, 5, 6, 7, 8, 9 | 630 | `1107 7522 6504 0025 800d 0002 2222` |
| `0x0c0` | `1804` | `ATR` (4) | `ATR` + I… | 3, 4, 5, 6 | 617 | `1804 7dd0 99b1 0000` |
| `0x158` | `2b07` | `MTTNRR` (7) | `MTTNRR` + I… | 7, 8, 9, 10, 11 | 544 | `2b09 2c8f 6fe2 7fe4 5983 0407 0403 a2aa aa00` |
| `0x170` | `2e05` | `AATR` (5) | `AATR` + I… | 4, 5, 6, 7, 8, 9 | 398 | `2e04 1442 3d87 a9b2` |
| `0x1f0` | `3e07` | `TTTRRR` (7) | `TTTRRR` + I… | 7 | 388 | `3e07 7fe0 2fe2 3fe4 a025 c022 c024` |
| `0x270` | `4e0a` | `MATTNRRII` (10) | `MATTNRR` + I… | 10, 11, 12 | 341 | `4e0a 254f 34e6 afe2 6fe4 0002 040c 0439 3921 fb00` |
| `0x0d8` | `1b04` | `BTR` (4) | `BTR` + I… | 3, 4, 5, 6 | 315 | `1b04 ae10 b9b1 0000` |
| `0x1a0` | `3406` | `ATTRR` (6) | `ATTRR` + I… | 5, 6, 7, 8 | 312 | `3406 2c86 afe0 3fe2 0216 0218` |
| `0x0b8` | `1703` | `AC` (3) | `AC` + I… | 3, 4, 5 | 293 | `1704 1050 1810 3000` |
| `0x348` | `6908` | `ATTTRRR` (8) | `ATTTRRR` + I… | 7, 8, 9, 10 | 290 | `6908 3cd0 9fe2 bfe4 99b1 0000 0265 0267` |
| `0x2c0` | `5809` | `MTTTNRRR` (9) | `MTTTNRRR` + I… | 9, 10, 11, 12, 13 | 250 | `5809 048d 2fe2 1fe4 5fe6 0000 0207 0209 a025` |
| `0x188` | `3105` | `ABTR` (5) | `ABTR` + I… | 4, 5, 6, 7 | 235 | `3105 34c4 ee10 b9b2 c400` |
| `0x0b0` | `1603` | `AB` (3) | `AB` + I… | 3, 4, 5, 6, 7 | 229 | `1604 54d0 9ec7 7000` |
| `0x128` | `2508` | `MATNRII` (8) | `MATNR` + I… | 6, 7, 8, 9, 10, 11 | 215 | `2507 1c4f 2483 7fe4 f012 3000 a023` |
| `0x328` | `6507` | `ABTTRR` (7) | `ABTTRR` + I… | 5, 6, 7, 8, 9 | 215 | `6505 7c0f be0c 99b1 b9b2` |
| `0x2f8` | `5f07` | `AATTRR` (7) | `AATTRR` + I… | 5, 6, 7, 8, 9, 10, 11 | 214 | `5f05 7dc9 6c09 99b1 a9b2` |
| `0x168` | `2d06` | `AACII` (6) | `AAC` + I… | 4, 5, 6, 7, 8 | 172 | `2d06 1050 1052 0e12 3000 0000` |
| `0x2a0` | `5408` | `MBTTNRR` (8) | `MBTTNRR` + I… | 8, 9, 11 | 160 | `5408 6d89 8ec5 8fe2 5fe4 0280 022f 0230` |
| `0x3c0` | `7809` | `TTTTRRRR` (9) | `TTTTRRRR` + I… | 9 | 157 | `7809 afe0 bfe2 0fe4 1fe6 0210 0211 020e 020f` |
| `0x090` | `1206` | `MBNCI` (6) | `MBNC` + I… | 4, 5, 6, 7, 8, 9 | 143 | `1204 570d fde8 0005` |
| `0x1d0` | `3a06` | `BTTRR` (6) | `BTTRR` + I… | 5, 6, 7, 8 | 139 | `3a06 8e10 b9b1 0fe2 0000 0210` |
| `0x058` | `0b02` | `C` (2) | `C` + I… | 2, 3, 4 | 138 | `0b02 00f8` |
| `0x5c0` | `b809` | `AATTTRRR` (9) | `AATTTRRR` + I… | 9, 10, 11, 12, 13 | 122 | `b80a 2487 3ee4 afe0 bfe2 a9b2 0215 021c 0ccc cc00` |
| `0x238` | `470c` | `MMTTNNRRIII` (12) | `MMTTNNRR` + I… | 9, 11, 12, 14 | 116 | `4709 4794 57d6 99b1 a9b2 0000 0000 2800 2800` |
| `0x080` | `1005` | `MMNN` (5) | `MMNN` + I… | 5, 7, 8, 9, 11 | 95 | `1005 048e 14ce 0000 0001` |
| `0x160` | `2c07` | `AABIII` (7) | `AAB` + I… | 4, 5, 6, 7 | 80 | `2c07 1447 3490 ce72 7000 0000 0200` |
| `0x5c2` | `b84b` | `AATTTTRRRR` (11) | `AATTTTRRRR` + I… | 11, 12, 14 | 75 | `b84b 1447 2507 afe0 bfe2 6fe4 7fe6 0206 0207 c025 c022` |
| `0x5b8` | `b70a` | `AACTTRRII` (10) | `AACTTRR` + I… | 8, 9, 10, 11 | 71 | `b708 6dd0 6dd2 1312 99b1 99b2 3000 b000` |
| `0x140` | `2808` | `MBTNRCI` (8) | `MBTNRC` + I… | 6, 8, 9, 10, 11, 12 | 70 | `2808 25b4 bedb 7fe2 0002 0438 2d21 8015` |
| `0x4e0` | `9c0c` | `MATTTNRRRII` (12) | `MATTTNRRR` + I… | 10, 12, 13, 14 | 69 | `9c0e 38d2 7cbc 9fe6 4fe8 5fea 4500 5000 5000 021a 0216 0217 0000 0002` |
| `0x118` | `2307` | `MABNII` (7) | `MABN` + I… | 6, 7, 8 | 64 | `2307 4e8e 3e80 ee62 0382 3000 0000` |
| `0x258` | `4b0b` | `MABTNRIIII` (11) | `MABTNR` + I… | 8, 9, 10, 11, 12 | 56 | `4b0a 65f5 75c5 8e22 b9b3 0006 ca00 00d3 d254 1f00` |
| `0x2f0` | `5e08` | `AACTRII` (8) | `AACTR` + I… | 6, 7, 8 | 45 | `5e08 7dd0 34d2 1a10 4fe4 4200 7000 0402` |
| `0x198` | `3305` | `ACTR` (5) | `ACTR` + I… | 5, 6 | 44 | `3305 3040 1b10 7fe0 a024` |
| `0x2e0` | `5c08` | `AABTRCI` (8) | `AABTRC` + I… | 5, 6, 7, 8, 9, 10 | 44 | `5c06 6d40 3cc0 ae10 b9b3 a400` |
| `0x4c0` | `9809` | `MABTTNRR` (9) | `MABTTNRR` + I… | 9, 10, 11, 12, 15 | 44 | `980f 2976 7c3e beeb 9fe2 6fe4 1e80 0429 042b 0000 0000 049b a5e3 0000 0000` |
| `0x558` | `ab0b` | `MTTTTNRRRR` (11) | `MTTTTNRRRR` + I… | 11, 13, 14 | 42 | `ab0b 2c0b bfe2 3fe4 7fe6 4fe8 a2c2 0217 021b c023 c025` |
| `0x340` | `6807` | `ACTTRR` (7) | `ACTTRR` + I… | 7, 9 | 37 | `6809 5064 1f13 dfe0 5fe2 0406 0410 3b33 3300` |
| `0x638` | `c70c` | `ATTTTRRRRII` (12) | `ATTTTRRRR` + I… | 9, 10, 11, 12 | 36 | `c709 3585 dfe0 99b1 0fe2 1fe4 0256 0254 c035` |
| `0x5a0` | `b40b` | `AABTTRRIII` (11) | `AABTTRR` + I… | 8, 9, 11 | 36 | `b40b 4424 5427 fe41 8fe0 2fe2 0250 0251 43a3 2930 6396` |
| `0x398` | `7307` | `BTTTRR` (7) | `BTTTRR` + I… | 7, 8, 10 | 35 | `7307 8e09 8fe0 b9b1 4fe2 0415 0411` |
| `0x180` | `3006` | `ABCII` (6) | `ABC` + I… | 5, 6 | 32 | `3006 0050 8012 1812 b000 0000` |
| `0x598` | `b307` | `AABCTR` (7) | `AABCTR` + I… | 7, 9 | 32 | `b307 4d08 3ed0 8e41 0930 99b1 3000` |
| `0x610` | `c209` | `ABTTTRRR` (9) | `ABTTTRRR` + I… | 7, 9, 11 | 30 | `c20b 1c47 8426 7fe0 afe2 2fe4 021c 022c a02b c400 0000` |
| `0x1e8` | `3d07` | `CTTRRC` (7) | `CTTRRC` + I… | 5, 7 | 30 | `3d07 4050 0fe2 7fe4 0105 4101 0422` |
| `0x530` | `a60d` | `MBTTTNRRRCII` (13) | `MBTTTNRRRC` + I… | 12, 13 | 29 | `a60c 2538 eee6 1fe2 4fe4 dfe6 0002 0415 0416 0410 0000 0000` |
| `0x55a` | `ab4d` | `MTTTTTNRRRRR` (13) | `MTTTTTNRRRRR` + I… | 13, 15 | 29 | `ab4d 14cd bfe2 2fe4 3fe6 6fe8 7fea 0001 020d 0206 0207 c027 c028` |
| `0x5b0` | `b609` | `AACCTRII` (9) | `AACCTR` + I… | 7, 9 | 28 | `b607 3044 3045 1220 0e13 5fe0 040f` |
| `0x190` | `3204` | `ACC` (4) | `ACC` + I… | 4, 5, 6 | 26 | `3204 3044 1b10 1220` |
| `0x100` | `200a` | `MMBNNIIII` (10) | `MMBNN` + I… | 10 | 25 | `200a 44a6 54e9 e5d4 0004 0005 0000 2c4f 762f bd2f` |
| `0x478` | `8f0b` | `MMTTTNNRRR` (11) | `MMTTTNNRRR` + I… | 9, 11, 14 | 25 | `8f0e 04aa 14ed 2fe4 3fe6 7fe8 0000 0001 0608 0619 c1fc 2fe2 9d2f e29d` |
| `0x5f0` | `be08` | `ABBTTRR` (8) | `ABBTTRR` + I… | 7, 8, 10 | 24 | `be07 3503 de0e 9f2b dfe0 b9b2 0417` |
| `0x5a4` | `b48a` | `AABTTTTRR` (10) | `AABTTTTRR` + I… | 10, 11, 16 | 22 | `b48a 6d49 3cc0 ce0b 9fe0 c9b1 b9b3 5fe2 043b 0433` |
| `0x0e8` | `1d04` | `CTR` (4) | `CTR` + I… | 3, 4, 5, 6 | 21 | `1d04 0910 0fe0 0248` |
| `0x110` | `2207` | `MMTNNR` (7) | `MMTNNR` + I… | 7, 8, 10, 11 | 21 | `2207 154e 350f 7fe4 0001 0003 c036` |
| `0x568` | `ad08` | `AAACCII` (8) | `AAACC` + I… | 8, 9 | 20 | `ad08 7040 7050 3052 0e12 1f30 0000 0000` |
| `0x2c8` | `5909` | `AAACIIII` (9) | `AAAC` + I… | 8, 9 | 20 | `5909 1c43 1c70 65b4 0b12 0000 0000 3000 0000` |
| `0x220` | `440b` | `MMBTNNRIII` (11) | `MMBTNNR` + I… | 10, 11, 12 | 20 | `440b 6566 054f fe69 dfe4 0003 0000 0232 0141 b230 0000` |
| `0x6c8` | `d908` | `BBTTTRR` (8) | `BBTTTRR` + I… | 8 | 20 | `d908 ae10 8ee0 ffe2 b9b1 c9b2 0000 0252` |
| `0x098` | `1307` | `MNIIII` (7) | `MN` + I… | 4, 7 | 19 | `1307 2c22 0b10 9941 022a 1c02 2a1c` |
| `0x0e0` | `1c04` | `CC` (2) | `CC` + I… | 3, 4 | 12 | `1c04 0090 0810 0009` |
| `0x2e8` | `5d05` | `AACC` (5) | `AACC` + I… | 5, 7 | 16 | `5d05 3cc4 45c7 0f10 1e30` |
| `0x720` | `e40b` | `TTTTTRRRRR` (11) | `TTTTTRRRRR` + I… | 11 | 15 | `e40b 8fe0 9fe2 1fe4 2fe6 7fe8 0600 0601 0603 0604 c1f7` |
| `0x4b8` | `970b` | `MABCTNRIII` (11) | `MABCTNR` + I… | 11, 12 | 14 | `970b 2476 75c6 be52 1830 4fe4 0002 b000 0413 0000 0000` |
| `0x268` | `4d09` | `MACTNRII` (9) | `MACTNR` + I… | 7, 9 | 12 | `4d09 54cd 1c64 1a20 2fe2 0004 0407 4200 0000` |
| `0x584` | `b08c` | `AABBCTTRRCI` (12) | `AABBCTTRRC` + I… | 12 | 12 | `b08c 24b4 3cc0 be0d ee50 1864 b9b3 6fe2 3000 0404 3000 0000` |
| `0x59c` | `b38e` | `AABCTTTRRRCCI` (14) | `AABCTTTRRRCC` + I… | 14 | 12 | `b38e 4537 3064 ee0f 1820 dfe0 efe2 b9b3 0402 040b b000 002f ae14 7a00` |
| `0x4e2` | `9c4c` | `MATTTTNRRRR` (12) | `MATTTTNRRRR` + I… | 12, 14 | 10 | `9c4c 648f 1440 dfe2 0fe4 7fe6 2fe8 0006 0226 c028 a032 021a` |
| `0x208` | `410b` | `MMANNCCCII` (11) | `MMANNCCC` + I… | 11 | 10 | `410b 4566 4569 1454 1a12 0006 0006 3000 0009 6e00 2654` |
| `0x458` | `8b0a` | `MMBTTNNRR` (10) | `MMBTTNNRR` + I… | 10 | 9 | `8b0a 4654 5696 8f2b 99b1 a9b2 0003 0003 2800 2800` |
| `0x378` | `6f06` | `BBTTR` (6) | `BBTTR` + I… | 6, 7 | 9 | `6f06 ae10 8ee0 b9b1 c9b2 0000` |
| `0x570` | `ae09` | `AAACTRII` (9) | `AAACTR` + I… | 8, 9, 11 | 9 | `ae0b 4072 4043 24b6 1812 ffe0 0412 0000 0001 a2b8 51eb` |
| `0x2d8` | `5b08` | `AABIIII` (8) | `AAB` + I… | 5, 8 | 9 | `5b08 6050 4d32 bedb 1b10 0000 0978 d4fd` |
| `0x5a2` | `b44a` | `AABTTTRRR` (10) | `AABTTTRRR` + I… | 10, 11 | 9 | `b44a 4d10 75c7 ee12 9fe4 b9b3 99b1 a800 a800 0459` |
| `0x592` | `b247` | `AATCCT` (7) † | `AATCCT` + I… | 7 | 8 | `b247 3044 3045 ee0f 1220 0e14 b9b3` |
| `0x490` | `920c` | `MAACTNRIIII` (12) | `MAACTNR` + I… | 10, 12 | 6 | `920c 75e6 5043 5052 1a23 ffe4 0029 3000 0231 4500 00c4 0000` |
| `0x5c4` | `b88d` | `AATTTTTRRRRR` (13) | `AATTTTTRRRRR` + I… | 13 | 6 | `b88d 0487 14c7 8fe0 9fe2 2fe4 3fe6 7fe8 060f 0620 060e 061f c1fd` |
| `0x722` | `e44d` | `TTTTTTRRRRRR` (13) | `TTTTTTRRRRRR` + I… | 13 | 6 | `e44d 8fe0 8fe2 afe4 9fe6 2fe8 0fea 8400 a400 0204 0200 8614 60e1` |
| `0x4c2` | `984c` | `MABTTTNRRRI` (12) | `MABTTTNRRR` + I… | 12, 14 | 6 | `984c 3412 6d94 fe16 9fe8 a9b2 b9b3 0003 2400 0000 0000 0269` |
| `0x5f2` | `be49` | `ABBTTTRR` (9) | `ABBTTTRR` + I… | 9 | 5 | `be49 0c07 de0a eede b9b2 1fe0 8fe2 0238 0243` |
| `0x590` | `b208` | `AABCIII` (8) | `AABC` + I… | 8 | 5 | `b208 3c20 5044 9ee1 2620 1f40 2800 0000` |
| `0x574` | `ae8e` | `AAACTTTRRRIII` (14) | `AAACTTTRRR` + I… | 12, 14 | 5 | `ae8e 7de8 5043 5050 1a23 ffe2 0fe4 2fe6 3000 0254 025a 0257 aa1d 2a00` |
| `0x130` | `260b` | `MBBNCCCCCI` (11) | `MBBNCCCCC` + I… | 11 | 4 | `260b 1472 aeda 803a 0036 1999 9999 2e66 6666 03fa 0945` |
| `0x512` | `a24d` | `MBBTTTNRRRII` (13) | `MBBTTTNRRR` + I… | 13 | 4 | `a24d 79a6 ae12 8ee0 b9b2 c9b3 dfe4 7a40 0000 022e 3000 0000 0000` |
| `0x572` | `ae4c` | `ATACTTRRRCI` (12) † | `ATACTTRRRC` + I… | 12 | 4 | `ae4c 7de6 5043 5050 1a23 ffe2 1fe4 3000 0251 0256 aa1d 2a00` |
| `0x600` | `c007` | `ABCCTR` (7) | `ABCCTR` + I… | 7 | 4 | `c007 6044 9ee1 2310 1a30 dfe0 024b` |
| `0x63a` | `c74c` | `ATTTTTRRRRR` (12) | `ATTTTTRRRRR` + I… | 12 | 4 | `c74c 0406 8fe0 4fe2 5fe4 6fe6 7fe8 0264 024d 024e c039 c03a` |
| `0x318` | `6305` | `ABII` (5) | `AB` + I… | 5 | 4 | `6305 6044 9ee1 2310 1a30` |
| `0x2d0` | `5a09` | `AABBIIII` (9) | `AABB` + I… | 9 | 4 | `5a09 0422 7890 b525 ce45 3000 2800 0029 9999` |
| `0x178` | `2f08` | `ABBCCCI` (8) | `ABBCCC` + I… | 8 | 4 | `2f08 1470 aeda 8034 3000 0000 03fa 0945` |
| `0x1b8` | `3705` | `BBTR` (5) | `BBTR` + I… | 5 | 4 | `3705 ce10 aed1 b9b1 a400` |
| `0x612` | `c24c` | `ABTTTTRRRRI` (12) | `ABTTTTRRRR` + I… | 12 | 4 | `c24c 24b6 be0d b9b2 afe0 1fe2 4fe4 0411 0412 0413 3000 0000` |
| `0x4b4` | `9692` | `MABCCTTNRRIIIIIII` (18) | `MABCCTTNRR` + I… | 18 | 4 | `9692 2c7b 5052 ee68 1f25 1f36 5fe4 6fe6 4fc2 3000 0406 0410 3b33 3300 0000 0000 0000 0000` |
| `0x108` | `2109` | `MMNNIIII` (9) | `MMNN` + I… | 9 | 4 | `2109 140b 140b 0b12 0024 0027 2780 00a7 8000` |
| `0x240` | `4809` | `MAANCCCI` (9) † | `MAANCCC` + I… | 8, 9 | 4 | `4809 1862 3040 3041 0b23 2940 0000 0000 0000` |
| `0x6e8` | `dd07` | `BCTTTR` (7) | `BCTTTR` + I… | 7 | 4 | `dd07 de0e 1830 b9b1 99bc 1fe0 062c` |
| `0x120` | `2405` | `MANI` (5) | `MAN` + I… | 5, 9 | 3 | `2405 0cc0 0cca 1a12 a007` |
| `0x59a` | `b34c` | `AABCTTRRCCI` (12) | `AABCTTRRCC` + I… | 12 | 2 | `b34c 4076 4050 ae12 1812 b9b3 7fe4 0000 d000 c007 0800 0000` |
| `0x60c` | `c18e` | `ABCTTTTRRRRCI` (14) | `ABCTTTTRRRRC` + I… | 14 | 2 | `c18e 4078 ae10 1810 efe2 b9b2 5fe4 6fe6 c400 042a 0406 c005 2200 0000` |
| `0x4ba` | `974e` | `MABCTTNRRCCCI` (14) | `MABCTTNRRCCC` + I… | 14 | 2 | `974e 5566 4079 ae12 1820 b9b3 6fe4 0006 c200 c006 8273 8124 0000 0000` |
| `0x310` | `6205` | `ABBT` (5) | `ABBT` + I… | 5 | 2 | `6205 3503 de0e 9f2b b9b2` |
| `0x388` | `7106` | `BCCTR` (6) | `BCCTR` + I… | 6 | 2 | `7106 fe08 0090 1820 b9b1 0080` |
| `0x1d8` | `3b06` | `CCCCC` (6) | `CCCCC` + I… | 6 | 2 | `3b06 0090 0092 1812 0001 00cd` |
| `0x482` | `904a` | `MAABCTNRI` (10) | `MAABCTNR` + I… | 10 | 2 | `904a 0cc0 0cca 6062 be0f 1a12 b9b4 a005 0008 0000` |
| `0x3b8` | `770a` | `CTTTRRRCC` (10) | `CTTTRRRCC` + I… | 10 | 2 | `770a 40a6 1fe0 3fe2 4fe4 0622 4001 4002 0000 0000` |
| `0x2b8` | `5709` | `MCTTNRRC` (9) | `MCTTNRRC` + I… | 9 | 2 | `5709 788d 4012 0fe4 2fe6 3300 0000 063f 0640` |
| `0x588` | `b10b` | `AABBTRCCII` (11) | `AABBTRCC` + I… | 11 | 1 | `b10b 2c03 4062 1075 eef6 3fe0 0625 0600 005f ffff ff00` |
| `0x566` | `acce` | `AAABCTTTRRRCI` (14) | `AAABCTTTRRRC` + I… | 14 | 1 | `acce 75e6 0042 0c8c be10 1a23 b9b4 efe2 3fe4 0000 0603 0605 0c86 0700` |
| `0x5a6` | `b4cc` | `AABTTTTTRRR` (12) | `AABTTTTTRRR` + I… | 12 | 1 | `b4cc 6d83 2d4a 8ec1 ffe0 99b1 a9b2 3fe2 1fe4 0223 0228 0224` |
| `0x320` | `6407` | `ABCTRI` (7) | `ABCTR` + I… | 7 | 1 | `6407 5050 ded9 0a12 3fe2 0000 a021` |
| `0x0c8` | `1904` | `BBI` (4) | `BB` + I… | 4 | 1 | `1904 ef75 8410 0000` |
| `0x45a` | `8b4f` | `MMBTTTNNIIIRRR` (14) | `MMBTTTNN` + I… | 12, 14, 15 | 72 | `8b4f 3454 2518 eee6 dfea 1fec 5fee 0016 0002 800d 0222 3000 040d 0442 043a` |
| `0x4aa` | `954c` | `MABBTTNIRII` (12) | `MABBTTN` + I… | 12 | 12 | `954c 2476 3cc0 ce12 eee6 b9b3 4fe4 0002 a800 040f 0000 0000` |
| `0x60a` | `c14d` | `ABCTTTIRRRII` (13) | `ABCTTT` + I… | 13 | 1 | `c14d 4078 be10 1810 afe2 5fe4 6fe6 c200 00f2 00ec c05b 2400 0000` |


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

## 15. Appendix: assembler mnemonic index, generated from `asm/isa.json`

Every variant the assembler (`asm/asm.py`) and the disassembler (`asm/disasm.py`) know, in the order of
`asm/isa.json` (form group → mnemonic → field values that select it → result → status recorded there). The
semantics columns are copied from the file; the section column is the `$section` of this specification it cites.
The status is the one written in `isa.json`; where the measured semantics is more precise than the file, the note
below the table gives it. Generated on 2026-11-10 from the `isa.json` of that date; if the file changes, this table is stale.

| group (§) | mnemonic | selected by | result | status |
|---|---|---|---|---|
| `alu_a` (6.1) | `add3` | op=1, D<8 | rD <- rD + rX + Y | exact |
| `alu_a` (6.1) | `mul` | op=2, D<8 | rD <- rX * Y | exact |
| `alu_a` (6.1) | `mad` | op=3, D<8 | rD <- rD + rX * Y (unit A op 3; see mac_* for the MAC unit) | exact |
| `alu_a` (6.1) | `ymacd` | op=4, D<8 | rD <- Y + rX * rD | exact |
| `alu_a` (6.1) | `ymsubd` | op=5, D<8 | rD <- Y - rX * rD | exact |
| `alu_a` (6.1) | `addxy` | op=6, D<8 | rD <- rX + Y | exact |
| `alu_a` (6.1) | `subxy` | op=7, D<8 | rD <- Y - rX | exact |
| `alu_a` (6.1) | `nop` | op=0, X=0, mode=0x0, Y=0, D<8 | no effect (the one-word 0001 is class 0x000 instead) | exact |
| `alu_a` (6.1) | `mov` | op=0, X=1, D<8 | rD <- Y (also loads an immediate) | exact |
| `alu_a` (6.1) | `newton` | op=0, X=2, D<8 | rD <- rD*r(D+1); r(D+1) <- 2-r(D+1)*rY; rY <- r(D+1)*rY | exact |
| `alu_a` (6.1) | `addh4` | op=0, X=4, D<8 | rD <- rD + Y + h4 | exact |
| `alu_a` (6.1) | `addh5` | op=0, X=5, D<8 | rD <- rD + Y + h5 | exact |
| `alu_a` (6.1) | `add` | op=0, X=6, D<8 | rD <- rD + Y | exact |
| `alu_a` (6.1) | `sub` | op=0, X=7, D<8 | rD <- rD + Y (identical to sub-code 6; the spec keeps them apart) | exact |
| `alu_a` (6.1) | `add3r` | op=0, D<8 | r(D-8) <- rX + Y + r[op]; only measured in a single-op class (spec 6.1) | exact |
| `alu_a` (6.1) | `sum8` | D≥8 | r(D-8) <- X + Y + r[op] | measured |
| `alu_b` (6.3) | `and` | D<8, mode=0x0, b11_8=0 | rD <- rX & Y (bit operations work mod 512) | exact |
| `alu_b` (6.3) | `andn` | D<8, mode=0x0, b11_8=2 | rD <- ~rX & Y | exact |
| `alu_b` (6.3) | `or` | D<8, mode=0x0, b11_8=4 | rD <- rX \| Y | exact |
| `alu_b` (6.3) | `orn` | D<8, mode=0x0, b11_8=6 | rD <- ~rX \| Y | exact |
| `alu_b` (6.3) | `xor` | D<8, mode=0x0, b11_8=8 | rD <- rX ^ Y | exact |
| `alu_b` (6.3) | `xnor` | D<8, mode=0x0, b11_8=10 | rD <- ~(rX ^ Y) | exact |
| `alu_b` (6.3) | `absadd` | D<8, b11_8∈{12,13} | rD <- \|rX\| + Y | exact |
| `alu_b` (6.3) | `subabs` | D<8, b11_8∈{14,15} | rD <- Y - \|rX\| | exact |
| `alu_b` (6.3) | `min` | D≥8, b11_8∈{0,1} | rD <- min(rX, Y) | exact |
| `alu_b` (6.3) | `max` | D≥8, b11_8∈{4,5} | rD <- max(rX, Y) | exact |
| `alu_b` (6.3) | `minn` | D≥8, b11_8∈{2,3} | rD <- min(-rX, Y) | measured |
| `alu_b` (6.3) | `maxn` | D≥8, b11_8∈{6,7} | rD <- max(-rX, Y) | measured |
| `alu_b` (6.3) | `absx` | D≥8, b11_8∈{8,9}, mode=0x0 | rD <- \|rX\| | exact |
| `alu_b` (6.3) | `absy` | D≥8, b11_8∈{10,11}, mode=0x0 | rD <- \|Y\| | exact |
| `alu_b` (6.3) | `cmp` | D≥8, b11_8=14, e_family=0 | flags of rD + Y, nothing written (partner of setflags) | measured |
| `alu_b` (6.3) | `divseed` | D≥8, b11_8=14, e_family=2, Y<8 | r(d) <- rX sign(rY) 2^-e, r(d+1) <- R[m], rY <- m (\|rY\| = m 2^e, m in [0.5, 1)); X even (r0 r2 r4 r6) | exact |
| `alu_b` (6.3) | `bic` | D≥8, b11_8=14, e_family=1 | rD <- +-rY | fitted |
| `alu_b` (6.3) | `shl` | D≥8, b11_8=12, mode=0x1 | rD <- rX * 2^Y (Y is the 4-bit field, not a register) | measured |
| `alu_b` (6.3) | `shr` | D≥8, b11_8=12, mode=0x2 | rD <- rX * 2^-Y | measured |
| `alu_b` (6.3) | `shrn` | D≥8, b11_8=12, mode=0x3 | rD <- rX * 2^-(16+Y) | measured |
| `alu_b` (6.3) | `frexp` | D≥8, b11_8=12, mode=0x0 | rD <- mantissa in [0.5,1), rY <- exponent | measured |
| `alu_b` (6.3) | `ph_add` | D≥8, b11_8=14, Y<8, mode=0x0, e_family=3 | rD <- rD + rY | measured |
| `alu_b` (6.3) | `ph_sub` | D≥8, b11_8=14, Y≥8, mode=0x0, e_family=3 | rD <- rD - rY | measured |
| `alu_b` (6.3) | `ph_abs` | D≥8, b11_8=14, Y<8, mode=0x1, e_family=3 | rD <- \|rY\| | measured |
| `alu_b` (6.3) | `ph_clamp` | D≥8, b11_8=14, Y≥8, mode=0x1, e_family=3 | rD <- clamp(rY, -1, 1) | measured |
| `alu_b` (6.3) | `ph_wrap` | D≥8, b11_8=14, Y<8, mode=0x2, e_family=3 | rD <- wrap(rY) into [-1, 1) | measured |
| `alu_b` (6.3) | `ph_tri` | D≥8, b11_8=14, Y≥8, mode=0x2, e_family=3 | rD <- 1 - 2\|wrap(-rY - 1.5)\| | measured |
| `alu_b` (6.3) | `ph_mod2` | D≥8, b11_8=14, Y<8, mode=0x3, e_family=3 | rD <- (rY mod 2) / 2 | measured |
| `alu_b` (6.3) | `ph_neg` | D≥8, b11_8=14, Y≥8, mode=0x3, e_family=3 | rD <- -rY | measured |
| `sfu` (6.4) | `recipseed` | D≥8, b11_8=15, f_family=0, mode=0x0, Y<8 | rD <- sign*2^-e (0 when 2^-e >= 512), r(D+1) <- ~1/m, rY <- m (\|rY\| = m 2^e) | measured |
| `sfu` (6.4) | `expsplit` | D≥8, b11_8=15, f_family=0, mode=0x1, Y≥8 | rD <- e^floor(rY) (device ROM, n = -19..6; 0 below, saturated above), rY <- frac(rY) | measured |
| `sfu` (6.4) | `sign` | D≥8, b11_8=15, f_family=0, mode=0x2, Y≥8 | rD <- +-1 | measured |
| `sfu` (6.4) | `floor` | D≥8, b11_8=15, f_family=0, mode=0x3, Y≥8 | rD <- floor(rY) | measured |
| `sfu` (6.4) | `ceil` | D≥8, b11_8=15, f_family=0, mode=0x3, Y<8 | rD <- ceil(rY) | measured |
| `sfu` (6.4) | `sqrtsplit` | D≥8, b11_8=15, f_family=0, mode=0x1, Y<8 | rY <- m in [1,4), r(D+1) <- ~1/sqrt m, r7 <- sign*2^-k | measured |
| `sfu` (6.4) | `log2split` | D≥8, b11_8=15, f_family=0, mode=0x2, Y<8 | rD <- exponent, rY <- mantissa in [0.75,1.5) | measured |
| `sfu` (6.4) | `atanhsplit` | D≥8, b11_8=15, f_family=0, mode=0x0, Y≥8 | rD <- (x-1)/2^k, rY <- (x+1)/2^k, rD+1 <- ~1/m | measured |
| `f_words` (6.3/6.4) | `f4` | D≥8, b11_8=15, f_family=1, mode=0x0 | rD <- r(Y & 7) | measured |
| `f_words` (6.3/6.4) | `f5` | D≥8, b11_8=15, f_family=1, mode=0x1 | f5 form (f5c = delay-ring position when Y = c) | measured |
| `f_words` (6.3/6.4) | `f6` | D≥8, b11_8=15, f_family=1, mode=0x2 | rD <- (rD + h(Y-8)) mod 512 | measured |
| `f_words` (6.3/6.4) | `f7` | D≥8, b11_8=15, f_family=1, mode=0x3 | section marker, no effect | measured |
| `mac` (7) | `mac_p1` | op1=2, op2=0, coefreg=0 | rD <- rX*Y + W | exact |
| `mac` (7) | `mac_p1b9` | op1=2, op2=0, coefreg=1 | rD <- rX*Y + W | inferred |
| `mac` (7) | `mac_p1c0` | op1=2, op2=0, coefreg=0 | rD <- rX*Y + W | inferred |
| `mac` (7) | `mac_p1c1` | op1=2, op2=1, coefreg=0 | rD <- rX*Y + W | inferred |
| `mac` (7) | `mac_p1c2` | op1=2, op2=2, coefreg=0 | rD <- rX*Y + W | measured |
| `mac` (7) | `mac_p1c3` | op1=2, op2=3, coefreg=0 | rD <- rX*Y + W | measured |
| `mac` (7) | `mac_p1w2` | op1=3, op2=0, coefreg=0 | rD <- -rX*Y + W | exact |
| `mac` (7) | `mac_p1w2b9` | op1=3, op2=0, coefreg=1 | rD <- -rX*Y + W | inferred |
| `mac` (7) | `mac_p1w2c0` | op1=3, op2=0, coefreg=0 | rD <- -rX*Y + W | inferred |
| `mac` (7) | `mac_p1w2c1` | op1=3, op2=1, coefreg=0 | rD <- -rX*Y + W | inferred |
| `mac` (7) | `mac_p1w2c2` | op1=3, op2=2, coefreg=0 | rD <- -rX*Y + W | measured |
| `mac` (7) | `mac_p1w2c3` | op1=3, op2=3, coefreg=0 | rD <- -rX*Y + W | measured |
| `mac` (7) | `mac_p1w` | op1=4, op2=0, coefreg=1 | rD <- rX*Y + rC*Z | measured |
| `mac` (7) | `mac_p1wc0` | op1=4, op2=0, coefreg=0 | rD <- rX*Y + rC*Z | measured |
| `mac` (7) | `mac_p1wc1` | op1=4, op2=1, coefreg=0 | rD <- rX*Y + imm16*Z | inferred |
| `mac` (7) | `mac_p1wc2` | op1=4, op2=2, coefreg=0 | rD <- rX*Y + imm24*Z | measured |
| `mac` (7) | `mac_p1wc3` | op1=4, op2=3, coefreg=0 | rD <- rX*Y + imm32*Z | measured |
| `mac` (7) | `mac_p1p2w` | op1=5, op2=0, coefreg=1 | rD <- -rX*Y + rC*Z | measured |
| `mac` (7) | `mac_p1p2wc0` | op1=5, op2=0, coefreg=0 | rD <- -rX*Y + rC*Z | inferred |
| `mac` (7) | `mac_p1p2wc1` | op1=5, op2=1, coefreg=0 | rD <- -rX*Y + imm16*Z | inferred |
| `mac` (7) | `mac_p1p2wc2` | op1=5, op2=2, coefreg=0 | rD <- -rX*Y + imm24*Z | measured |
| `mac` (7) | `mac_p1p2wc3` | op1=5, op2=3, coefreg=0 | rD <- -rX*Y + imm32*Z | measured |
| `mac` (7) | `mac_p1p2` | op1=6, op2=0, coefreg=1 | rD <- rX + W + rC*Z | measured |
| `mac` (7) | `mac_p1p2c0` | op1=6, op2=0, coefreg=0, modeY=0 | rD <- rX + W + Z (no products: op2 = 000, op1 = 6/7; spec 7) | measured |
| `mac` (7) | `mac_p1p2c0y` | op1=6, op2=0, coefreg=0, modeY≥1 | rD <- rX*Y + W + c*Z (immediate Y with bit 9 clear: products kept; not measured) | inferred |
| `mac` (7) | `mac_p1p2c1` | op1=6, op2=1, coefreg=0 | rD <- rX*Y + W + imm16*Z | inferred |
| `mac` (7) | `mac_p1p2c2` | op1=6, op2=2, coefreg=0 | rD <- rX*Y + W + imm24*Z | measured |
| `mac` (7) | `mac_p1p2c3` | op1=6, op2=3, coefreg=0 | rD <- rX*Y + W + imm32*Z | measured |
| `mac` (7) | `mac_p1p2wp2` | op1=7, op2=0, coefreg=1 | rD <- -rX + W + rC*Z | measured |
| `mac` (7) | `mac_p1p2wp2c0` | op1=7, op2=0, coefreg=0, modeY=0 | rD <- -rX + W + Z (no products: op2 = 000, op1 = 6/7; spec 7) | measured |
| `mac` (7) | `mac_p1p2wp2c0y` | op1=7, op2=0, coefreg=0, modeY≥1 | rD <- -rX*Y + W + c*Z (immediate Y with bit 9 clear: products kept; not measured) | inferred |
| `mac` (7) | `mac_p1p2wp2c1` | op1=7, op2=1, coefreg=0 | rD <- -rX*Y + W + imm16*Z | inferred |
| `mac` (7) | `mac_p1p2wp2c2` | op1=7, op2=2, coefreg=0 | rD <- -rX*Y + W + imm24*Z | measured |
| `mac` (7) | `mac_p1p2wp2c3` | op1=7, op2=3, coefreg=0 | rD <- -rX*Y + W + imm32*Z | measured |
| `mac` (7) | `mac_p1c1r` | op1=2, op2=1, coefreg=1 | rD <- rX*Y + W | inferred |
| `mac` (7) | `mac_p1c2r` | op1=2, op2=2, coefreg=1 | rD <- rX*Y + W | inferred |
| `mac` (7) | `mac_p1c3r` | op1=2, op2=3, coefreg=1 | rD <- rX*Y + W | inferred |
| `mac` (7) | `mac_p1w2c1r` | op1=3, op2=1, coefreg=1 | rD <- -rX*Y + W | inferred |
| `mac` (7) | `mac_p1w2c2r` | op1=3, op2=2, coefreg=1 | rD <- -rX*Y + W | inferred |
| `mac` (7) | `mac_p1w2c3r` | op1=3, op2=3, coefreg=1 | rD <- -rX*Y + W | inferred |
| `mac` (7) | `mac_p1wc1r` | op1=4, op2=1, coefreg=1 | rD <- rX*Y + imm16*Z | inferred |
| `mac` (7) | `mac_p1wc2r` | op1=4, op2=2, coefreg=1 | rD <- rX*Y + imm24*Z | inferred |
| `mac` (7) | `mac_p1wc3r` | op1=4, op2=3, coefreg=1 | rD <- rX*Y + imm32*Z | inferred |
| `mac` (7) | `mac_p1p2wc1r` | op1=5, op2=1, coefreg=1 | rD <- -rX*Y + imm16*Z | inferred |
| `mac` (7) | `mac_p1p2wc2r` | op1=5, op2=2, coefreg=1 | rD <- -rX*Y + imm24*Z | inferred |
| `mac` (7) | `mac_p1p2wc3r` | op1=5, op2=3, coefreg=1 | rD <- -rX*Y + imm32*Z | inferred |
| `mac` (7) | `mac_p1p2c1r` | op1=6, op2=1, coefreg=1 | rD <- rX*Y + W + imm16*Z | inferred |
| `mac` (7) | `mac_p1p2c2r` | op1=6, op2=2, coefreg=1 | rD <- rX*Y + W + imm24*Z | inferred |
| `mac` (7) | `mac_p1p2c3r` | op1=6, op2=3, coefreg=1 | rD <- rX*Y + W + imm32*Z | inferred |
| `mac` (7) | `mac_p1p2wp2c1r` | op1=7, op2=1, coefreg=1 | rD <- -rX*Y + W + imm16*Z | inferred |
| `mac` (7) | `mac_p1p2wp2c2r` | op1=7, op2=2, coefreg=1 | rD <- -rX*Y + W + imm24*Z | inferred |
| `mac` (7) | `mac_p1p2wp2c3r` | op1=7, op2=3, coefreg=1 | rD <- -rX*Y + W + imm32*Z | inferred |
| `transfer` (5.3) | `ld` | mode=0xfe, R<8 | rR <- mem[ref] | exact |
| `transfer` (5.3) | `st` | mode=0xfe, R≥8 | mem[ref] <- rR | exact |
| `transfer` (5.3) | `ld` | mode∈{0xff,0x3e,0xe,0x10,0x12}, R<8 | same address as fe; the assembler always writes fe | measured |
| `transfer` (5.3) | `st` | mode∈{0xff,0x3e,0xe,0x10,0x12}, R≥8 | same address as fe | measured |
| `transfer` (5.3) | `ldidx` | mode=0x28, R<8 | rR <- bank0[floor(h6*16384) + Y] | exact |
| `transfer` (5.3) | `stidx` | mode=0x28, R≥8 | bank0[floor(h6*16384) + Y] <- rR | exact |
| `transfer` (5.3) | `ldb5` | mode=0xda, R<8 | rR <- bank5[floor(h6*16384) + Y] | exact |
| `transfer` (5.3) | `ldb5` | mode=0xde, R≥8 | bank5[floor(h6*16384) + Y] <- rR | exact |
| `transfer` (5.3) | `ldb5h7` | mode=0xdb, R<8 | rR <- bank5[floor(h7*16384) + Y] | exact |
| `transfer` (5.3) | `ldb5h7` | mode=0xdf, R≥8 | bank5[floor(h7*16384) + Y] <- rR | exact |
| `transfer` (5.3) | `ldb5d` | mode∈{0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7} | rR <- bank5[N*16 + Y] with N = mode & 7 | exact |
| `transfer` (5.3) | `setflags` | mode=0x9b, R≥8 | flags of unit Y of this instruction -> condition group X-8 (X = 9..d -> groups 1..5) | measured |
| `transfer` (5.3) | `tapreq` | mode=0xa9 | delay-tap request at address rR; results land in bank 2 at [h5..h5+2] | measured |
| `cond` (5.4) | `cond` |  | if cond then unit skip_true does not execute, else unit skip_false does not | measured |
| `jmp` (5.4b) | `jmp` | tag=9 | jump by the offset stored in a slot word | measured |
| `hreg` (9.0) | `hset` | T=0, tag=4 | h4 <- value (delay base for f6c) | measured |
| `hreg` (9.0) | `hset` | T=1, tag=4 | h5 <- value (tap-result address) | measured |
| `hreg` (9.0) | `hset` | T=2, tag=4 | h6 <- value (index of mode-28 transfers) | measured |
| `hreg` (9.0) | `hset` | T=3, tag=4 | h7 <- value | measured |
| `empty` (3) | `noop` |  | the one-word instruction 0001 (class 0x000) | exact |
| `raw` (3) | `raw` |  | verbatim body words (an encoding without a known meaning) | raw |

130 variants in 11 form groups. Notes:
* `sqrtsplit`: `isa.json` states the third output as r7; the measured output is **r(d+3)** (C reference `examples/from_scratch/emu/dspcore.h`; [ch. 1](core_architecture.md) §5.3).
  With d = 4 both coincide.
* `recipseed`: "~1/m" is exactly the ROM value R[k] ([ch. 1](core_architecture.md) §5.3).
* The `mac_*` mnemonics are only names; what each computes is its result column and §7 (op2 = 000 with
  op1 = 6/7 has no products; with op1 = 4/5 both products are kept).
