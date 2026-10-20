# DSP instruction set (Roland BMC DSP) — reference

State 2026-10-20 (corpus_all 99.5 % decoded; every decoded line re-assembles to its own words; open items in
`DSP_ISA_DISCREPANCIES.md`). Only what was **measured on the hardware** (master DSP4, live probes; see "Method"), each item
marked with how sure it is. The history, raw evidence and dead ends are in `NOTES.md` (sections from
"ISA del DSP medido en el equipo por audio USB" on). Hardware context: `ARCHITECTURE.md`.

**This overview is behind the spec: where they differ, `DSP_ISA_SPEC.md` wins.** **Technical specification for assembler / compiler writers: `DSP_ISA_SPEC.md`** (bit layouts, all 118
templates, immediate packing, execution semantics, encoding examples). This file is the short overview.
**What the decoded operations are good for (delays, indexed delay lines, hidden registers…): `DSP_USOS.md`.**

Tools: `re/dspisa.py` (decoder / disassembler / simulator), `re/dsp_templates.txt` + `re/dsp_units_ab.txt`
(measured templates), `re/dspmc.py` (word order), `re/dspnum.py` (number format).

## 1. Program and words

* A program is a list of 16-bit words. In memory each 32-bit LE word holds two of them, **upper half first**
  (execution order). Programs start `0a02 ff70` and end `0b02 00f8 0001`. *(certain)*
* An instruction = **header** + body. Header = `class << 5 | length` (length in 16-bit words incl. the header;
  `0001` = a one-word no-op). *(certain: all 84 corpus programs tile exactly)*
* Numbers ("RAW"): bit 31 sign (sign-magnitude), bits 30–29 exponent s, bits 28–0 mantissa M:
  value = ±M / 2^(32−4s). 1.0 = `30000000`, 0.5 = `28000000`, −1 = `b0000000`. Max ≈ 512. *(certain)*

## 2. Templates (the class)

The 11-bit class is a **template**: it fixes what each body position is. Words carry no type marks (a `0000` at a
transfer position *is* a transfer). The class is not a simple bit field; every class was measured
(`re/dspslots.py`). 118 classes, 99.9 % of the corpus. *(measured per class)*

Position letters (`re/dsp_templates.txt`, unit A/B split in `re/dsp_units_ab.txt`):

| letter | meaning |
|---|---|
| A | ALU op, unit A |
| B | ALU op, unit B (same word layout, other operations) |
| M / N | MAC first word / second word |
| C | condition (predication) word |
| T | transfer (load / store) |
| R | memory reference (address) |
| I | immediate bits |

General order: `[A/B/M/C words] [T words] [slots: N words, then R words] [I bits]`. Examples:

| class | template | example |
|---|---|---|
| 0x048 | A + imm | `0902 3c01`, `0903 3050 2800`, `0904 1430 2800 0000` |
| 0x050 | B | `0a02 b042` |
| 0x0A8 | A A | `1503 2987 7505` |
| 0x0B0 | A B | `1603 5544 9ee1` |
| 0x040 | M N + imm | `0803 0c0a 9281`, `0807 3cca 1cb6 <imm32> <imm32>` |
| 0x088 | M A N + imm | `1107 7522 6504 0025 800d 0002 2222` |
| 0x060 / 0x0F0 / 0x1F0 / 0x3C0 | 1 / 2 / 3 / 4 transfers + their references | `0c03 0fe0 2200`, `1e05 0fe0 1fe2 0400 0401` |
| 0x0A0 | M T N R | `1404 0c4a 6a90 8280` |
| 0x158 | M T T N R R + imm | `2b09 2c8f 6fe2 7fe4 5983 0407 0403 a2aa aa00` |
| 0x168 | A A C + imm | `2d06 1050 1052 0e12 3000 0000` |

## 3. Operands and immediates

* Registers r0–r7 are the ones transfers can load/store. *(certain)*
* Operand **mode** (2 bits, see each format): 00 register, 01 immediate 16 bits, 10 immediate 24 bits, 11
  immediate 32 bits. Immediates are the top bits of a RAW word. They are read from the **slot array**: the words
  after the unit words, references included. An A/B op reads its immediate at **byte Y** of that array (Y is its
  4-bit field). A MAC reads Y at byte Y, then its coefficient right after Y's immediate (or at byte C when Y is a
  register), then W at byte W. A hidden-register load reads at byte Y. A read past the last word of the instruction
  continues into the next instruction. *(measured; MAC byte C measured 2026-10-20 at three byte positions, DISCREPANCIES D1)*
* Several A/B ops of one instruction **share** the same array: each reads it at its own byte, with its own width.
  Two reads of the same bits must agree. *(measured 2026-10-16; the byte rule 2026-10-17; this explains the old "two A ops
  with an immediate made it illegal")*
* Register field bit 3 (Y of A/B, Z and W of MAC) = **negate** (Y = 8 + n → −rn). *(certain for A; seen in MAC)*

## 4. Unit A (16-bit word)

`D[15:12] op[11:9] X[8:6] modeY[5:4] Y[3:0]` — *(exact, fitted with error 0 over random values)*

| op | result in rD |
|---|---|
| 0 | X is a sub-code: 0, 3 = nothing; 1 = **Y** (move / load immediate); 2 = Newton step: rD = rD·rD+1, rD+1 = 2 − rD+1·rY, rY = rD+1·rY (reciprocal); 4, 5 = D + Y + h4 / h5 (hidden per-context values: DSP4 `0xEFDC` 0.0313 / 320, DSP3 163 / 1.80469; not changed by A, B or MAC writes with D = 12/13 nor by the program-start B ops — source unknown); 6, 7 = D + Y |
| 1 | D + X + Y |
| 2 | X · Y |
| 3 | D + X · Y |
| 4 | Y + X · D |
| 5 | Y − X · D |
| 6 | X + Y |
| 7 | Y − X |

D ≥ 8: r(D−8) = X + Y + r[op] (the op field becomes the index of a third register). *(exact in class 0x048; measured 2026-10-20 in class 0x0a8 (AA)
with all eight values of op; the listing prints it as `sum8`, DISCREPANCIES D5)*

## 5. Unit B (same layout)

Measured with even B nibble, X = r1, Y = r2 *(exact, 7 probes)*: `3c42` |X| + Y; `3e42` Y − |X|; `b042`
**min(X, Y)**; `b442` **max(X, Y)**; `b842` |X|; `ba42` |Y|; `be42` Y. The other B ops return saturated-looking
bit patterns (`7ffxxxxx`): **(2026-10-17) bit operations** (AND/ANDN/OR/ORN/XOR/XNOR in two's complement,
negative results shown modulo 512, see §9). Raw grids on master DSP3 (X = r1,
Y = r2, D = r3 = −0.375; result word):

| op | (.25,.5) | (.5,.25) | (−.25,.5) | (.25,−.5) | (1.5,.125) | (0,.5) | (.5,0) | (−.25,−.5) | (.25,.75) | (−.25,.75) |
|---|---|---|---|---|---|---|---|---|---|---|
| `3042` | 0 | 0 | .5 | 0 | 0 | 0 | 0 | `7ff80000` | .25 | .75 |
| `3242` | .5 | .25 | 0 | `7ff80000` | .125 | .5 | 0 | 0 | .5 | 0 |
| `3442` | .75 | .75 | `7ffc0000` | `7ffc0000` | 1.625 | .5 | .5 | | | |
| `3842` | .75 | .75 | `7ff40000` | `7ffc0000` | 1.625 | .5 | .5 | | | |
| `3642` | `7ffbffff` | `7ff7ffff` | `2bffffff` | `7ffbffff` | `7fe7ffff` | `7fffffff` | `7ff7ffff` | | | |
| `3a42` | `7ff3ffff` | `7ff3ffff` | `2bffffff` | `23ffffff` | `7fe5ffff` | `7ff7ffff` | `7ff7ffff` | | | |

Unit B with D ≥ 8 has side effects on other registers (`0a02 cc42` → r4 = .5 and r2 = −1; `0a02 cf06` → r4 = 1,
r5 = 1.1377; `0a02 8f19` → r0 = 1; DSP3, r0..r7 = .5 .25 .125 0 .0625 −.75 .875 −.0625). The weird ops above
look like sign-dependent selects / sums with a second number format for negative results; usable only once
decoded. With random inputs (n = 12, `work/dspclass/bweird_raw.jsonl`, results > 256 read as v − 512) no formula of up
to 3 terms fits; `3a42` = −`3842` and `3b42` = −`3942` exactly (sign pairs like A op 4/5).

## 6. MAC (two words)

`w1 = D[15:12] op1[11:9] X[8:6] modeY[5:4] Y[3:0]`, `w2 = Z[15:12] op2[11:9] C[8:6] modeW[5:4] W[3:0]`
(w1 before the transfers, w2 first in the slot array). P1 = Y·X, P2 = c·Z. op1: bit 0 = sign of P1, bit 1 = add
W, bit 2 = add P2; op1 0/1 = no write. op2 bits 11–10 = mode of c: 00 register (bit 9 = 1 with a register
coefficient, spec 5.2), 01 imm16 (inferred), 10 imm24, 11 imm32. With op2 = 000 and op1 = 6/7 there are no products
(W ± X + Z). A coefficient immediate is read after Y's immediate, or at byte C when Y is a register (§3).
*(exact on the measured cases; op2 corner cases partly; bit 3 of Y is not a negation, measured 2026-10-20)*
Examples: `0803 0c0a 9281` = r1 + r2·(r0 − r1) (lerp); `2b09 2c8f … 5983 … a2aaaa` = r2·r7 + r3 + (−1/6)·r5
(`#-0.16666603@6`, Y not negated, measured 2026-10-20);
`0807 3cca 1cb6 002e3bc7 01a01a01` = r3·r2 + 2/315 + (2/2835)·r1; `1107 7522 … 0025 800d00 022222` =
r4·(−1/5040) + 1/120.

## 7. Transfers and references

Transfer `X mm Y` at a T position: mode byte mm = `fe`: X 0–7 loads rX ← [slot Y/2], X 8–15 stores
[slot Y/2] ← r(X−8). Slots are counted from the first word after the transfers, MAC second words included.
*(certain)* Other mode bytes, tested as loads of r3 from a word preloaded through `fe` (master DSP3):

| group | mode bytes | result |
|---|---|---|
| same memory as `fe` | `fe`, `ff`, `3e`, `0e`, `10`, `12` | loads the word |
| other memory | `e0`, `e1`, `db`, `ed`, `be`, `8e`, `ee`, `de`, `ce`, `7e`, `fc`, `fd`, `f0` | loads 0 (not the word) |
| indexed | `28` | rX ← bank0[h6 + Y] (ring-relative; see §9) |
| delay-tap request | `a9` | results to bank 2 [h5..] (see §9) |
| set flags | `9b` | `X9bY`: condition group X−8 ← flags of unit Y (2026-10-17, §8) |
| no load into rX | `9e` | r3 unchanged |

Seen as two nibbles (a, b) with b = `e`: a ∈ {f, 3, 0} same address, a ∈ {b, 8, e, d, c, 7} elsewhere, a = 9 no load.
Hypothesis: a selects an address (offset) register — delay-line addressing — and `9b`/`a9` load an address register
instead of a data register. Stores with the "other memory" modes did **not** show up in banks 0, 1, 2, 4, bank 5
or data RAM `+0x40000…` of DSP3 (one hit in bank 1 was not reproducible): probably external delay memory. **Not
decoded.** Reference words: `02nn`/`04nn`/`06nn` local data, `C0nn` parameter block,
`A0nn` bank 5, `22nn` mailbox between main and slot program (from the static analysis).

## 8. Condition words (C)

*(measured on master DSP3, a context no other task shares, and on DSP4 slot `0xEFDC`)*
* A C word predicates the A/B ops of its instruction. Low byte = which op and how (`x10`, `x11` → op 1,
  `x12` → op 2; `x00`, `x18`, `x30` → none). High byte = condition code: **bit 0 negates**, bits 4–1 select
  **one of 16 conditions**.
* **(Corrected 2026-10-17)** The 16 conditions are **flags**, 4 groups of 4, written by `X9bY`: group X−8 (X = 9/a/b
  → conds 4–7/8–11/12–15) ← flags of unit Y of the same instruction; per group c0 = ≠0, c1 = ≥0, c2 = no overflow
  (|x| < 512), c3 = ≤0. One instruction of latency; they persist until rewritten (that is why they looked static).
  Partner op: unit B nibble e, D ≥ 8, bits 7–6 = 00 = **compare** (flags of r(d) + Y, nothing written). Group 0
  stays `0111` on the master. Details `DSP_ISA_SPEC.md` §8.
* The earlier "varies over time" on DSP4 came from reads with a zeroed word and from other tasks; with a private
  context the result is stable.

## 9. Later findings (summary; details in `DSP_ISA_SPEC.md`)

* The DSP runs at **96 kHz**; bank 0 is a **ring** (a value stored at S is read at S + 1 in the next sample:
  state = load S + 1, store S); a load needs one instruction before an ALU op can use it; ops of one instruction
  read before they write.
* Registers are shared between the programs of one DSP within a sample.
* C word: high byte = cc << 1 | neg; low byte = unit skipped if true (bits 7–4) / if false (bits 3–0), units
  numbered from 1 (ops, then transfers). Context bits also measured on the slave DCO2 slot: `0011011001100110`.
* Unit B: odd nibble = X from r4–r7 (clamps `d150`/`d550`/`e190`/`e590`); SFU ops (`cf06` reciprocal seed, `8f19`
  exp split, `8f29` sign, `8f39` negative mask, `cf36` ceil).
* DCO2 slot inputs `2209`–`220c` (sync, METAL, sync pulse train), written by the main program.
* ~~`X9bY` does not set condition bits~~ — wrong: it does (2026-10-17, see §8); the old test used an op whose
  result was always 0. Bank numbering of reference prefixes and the inter-DSP routing (static analysis): `DSP_ARM_SIDE.md`.
* First own microcode (SAW, FOLD) plays on the hardware: `CUSTOM_OSC.md`.
* Multi-op classes 0x0B0 (A, B) and 0x160 (A, A, B): all ops read before writing; an A/A/B costs **one cycle**,
  like a single op. Per-sample budget ≈ 925–945 instructions on every DSP (main2/3 of the slaves = 189).
  Wavetables with one A/A/B instruction per point: `CUSTOM_OSC.md`. *(2026-10-16)*

* **Delay taps (2026-10-16, master DSP4 slot 0xF7DC)**: `[4300] ← x` writes the delay ring; `1d06 4022 … <imm>`
  sets a base (imm >> 22); `{8+d}f6c` = rd += base; `0b03 4050 0301` sets where the results go; `da90` requests
  the read; `[4301..4303]` = two adjacent samples + fraction; delay = 16384·(rd − 324) + 2–3 samples. Bank 2 = two
  pages alternating per sample, bank 5 plain memory. Details: `DSP_ISA_SPEC.md` §9.1.

* **Hidden registers / indexed loads (2026-10-16)**: a C-position word `40TY` loads h4..h7 (T = bits 7–6) from rY or
  an immediate (integer·16384); h4 = delay base, h5 = tap-result address (these were the "hidden values" of A op 0
  sub 4/5), h6 = index: `X28Y` loads/stores bank0[h6 + Y] (tables in memory). `DSP_ISA_SPEC.md` §9.0.

* **More 2026-10-16**: indexed transfers are ring-relative → `[S + k]` = variable delay < 2048 samples on any DSP;
  buses: `[60nn]` = IOMD word nn, DSP3 Z/S stores (`85nn`/`87nn`) → DSP4 O loads (`83nn`); MAC D ≥ 8 = result/2,
  op2 010 = imm16, an A op inside a MAC class reads N; SFU `cf16` sqrt split, `cf26` log2 split; unit B nibble 2/6 =
  min/max(−X, Y), c = frexp. Coverage tool: `python3 re/dspisa.py cov work/corpus` (93.6 %).

* **Jumps and loops (2026-10-17)**: a word `009Y` in a C position is a jump by the signed 16-bit word number
  Y >> 1 of the immediate/slot stream, in words from the end of the instruction; it is a unit for the C words
  (conditional jumps). Backward jumps make loops (measured on the master; a 4-iteration loop and forward jumps
  verified by audio on the slaves). Load programs with an empty program first: a torn program with a backward
  jump hangs the DSP. Unit-B nibble f bits 7–6 = 01: `f4Y` move, `f5c` delay write position, `f6Y` + h(Y−8),
  `f7Y` markers. Coverage 98.6 %.
* **All Roland programs decoded (2026-10-17)**: 212 SYSTEM-8 programs (base engine, OS, 9 PLUG-OUTs) + 4 SYSTEM-1;
  unit B D < 8 nibbles 0–a = bit ops (AND, ANDN, OR, ORN, XOR, XNOR; two's complement, mod 512); `9f0d` = atanh
  split for logs; only the section markers `ff70`/`ff71`/`ef75`/`ef74` remain without a visible effect.
* **Consolidated hardware reference: `DSP_HARDWARE.md`** (2026-10-17): bit-exact arithmetic (exact result,
  truncated toward zero), seed ROM tables, 1 cycle per instruction, MXM latched per sample with 1-sample latency.
* **Measured on the hardware (2026-10-17, `re/dspverify.py`)**: all inferred templates verified; immediates
  start **Y bytes** into the slot array (MAC: Y and W by field, C after Y skipping references); read ports: bank 0
  = 2 loads per instruction, bank 5 = 1; C field acts only if bit 3 of the other field is 0; unit B nibble e
  bits 7–6 = 11 = phase shaping (±, abs, clamp, wrap, triangle, phase 0–1, −Y), nibble c modes 1–3 = shifts;
  `ef75`/`ef74` = no effect, 1 cycle each (optional).
* **C condition field = 5 bits** (cond 0–31): `c9bY` → group 4 (cond 16–19), `d9bY` → group 5 (cond 20–23),
  verified on the master and by audio on the slaves. Group 0 = constant `0111`, cond 24–31 = always true.

## 9. Open (full list: `DSP_ISA_SPEC.md` §14)

0. (Solved 2026-10-17) `X9bY` = set flags. Left: group 0 / `c9bY`, units 3–4, flags of MAC/SFU results.
1. The second word of C instructions in the slot array.
2. (solved) The DSP runs at 96 kHz; data memory is a ring (state: load S + 1, store S); a load has one instruction
   of latency. See `DSP_ISA_SPEC.md` §9–§10.
3. (Solved 2026-10-17: unit B ops returning `7ffxxxxx` = bit ops; odd nibbles = X in r4–r7.) (hidden values solved: h4/h5).
   Resolved 2026-10-20 on the device: the MAC coefficient at byte C (D1), bit 3 of a MAC Y is not a negation
   (D3), the hidden-register Y is a byte offset (D7), D ≥ 8 in a multi-op class (D5), bit 9 of a MAC word
   (D9), the W operand's negation (measured), and the no-product MAC forms. Open: unit B words in classes 0x258 and
   0x140 (D6); where the coefficient is read after an immediate Y in plain layouts (D8); the letters of classes 0x240,
   0x572, 0x592 (D4). Full list and probes: `DSP_ISA_DISCREPANCIES.md`.
4. Transfer modes other than `fe`, `28`, `a9`.

## Method

`re/dsppeek.py` (region 0 `-DPEEK`, MIDI CC1 + pitch bend in, SysEx out) writes test programs into master DSP4
slot `0xEFDC` (`re/dspmaster.py`; context `DSPCTX=dsp3` (default: master DSP3 main program, private) or `efdc`; original restored; `--restore` repairs a killed run) and reads bank 0 (RAW
view, majority of 7 reads). `re/dspprobe.py` (fixed registers), `re/dspfit.py` / `re/dspclass.py` /
`re/dspnlfit.py` (random registers, exact smallest-formula search), `re/dspslots.py` (template positions),
`re/dsptemplates.py` (templates). Only one tool may use the MIDI port at a time.
