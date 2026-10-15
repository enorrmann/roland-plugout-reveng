# DSP instruction set (Roland BMC DSP) — reference

State 2026-10-15. Only what was **measured on the hardware** (master DSP4, live probes; see "Method"), each item
marked with how sure it is. The history, raw evidence and dead ends are in `NOTES.md` (sections from
"ISA del DSP medido en el equipo por audio USB" on). Hardware context: `../../ARCHITECTURE.md`.

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
  immediate 32 bits. Immediates are the top bits of a RAW word, taken **in order from a continuous bit stream** at
  the end of the instruction (24 bits = 1.5 words; past the end it reads the next instruction). *(certain)*
* Two A ops with an immediate each in one instruction made it illegal (nothing stored); a MAC may take several
  immediates. *(measured, few cases)*
* Register field bit 3 (Y of A/B, Z and W of MAC) = **negate** (Y = 8 + n → −rn). *(certain for A; seen in MAC)*

## 4. Unit A (16-bit word)

`D[15:12] op[11:9] X[8:6] modeY[5:4] Y[3:0]` — *(exact, fitted with error 0 over random values)*

| op | result in rD |
|---|---|
| 0 | X is a sub-code: 0, 3 = nothing; 1 = **Y** (move / load immediate); 2 = Newton step: rD = rD·rD+1, rD+1 = 2 − rD+1·rY, rY = rD+1·rY (reciprocal); 4, 5 = D + Y + hidden value; 6, 7 = D + Y |
| 1 | D + X + Y |
| 2 | X · Y |
| 3 | D + X · Y |
| 4 | Y + X · D |
| 5 | Y − X · D |
| 6 | X + Y |
| 7 | Y − X |

D ≥ 8: r(D−8) = X + Y + r[op] (the op field becomes the index of a third register). *(exact, single-op class)*

## 5. Unit B (same layout)

Measured with even B nibble, X = r1, Y = r2 *(exact, 7 probes)*: `3c42` |X| + Y; `3e42` Y − |X|; `b042`
**min(X, Y)**; `b442` **max(X, Y)**; `b842` |X|; `ba42` |Y|; `be42` Y. The other B ops return saturated-looking
bit patterns (`7ffxxxxx`): comparisons or integer conversions, **not decoded**.

## 6. MAC (two words)

`w1 = D[15:12] op1[11:9] X[8:6] modeY[5:4] Y[3:0]`, `w2 = Z[15:12] op2[11:9] C[8:6] modeW[5:4] W[3:0]`
(w1 before the transfers, w2 first in the slot array). P1 = Y·X, P2 = c·Z. op1: bit 0 = sign of P1, bit 1 = add
W, bit 2 = add P2; op1 0/1 = no write. op2 bits 11–10 = mode of c (00: c = rC, 10: imm24, 11: imm32); with op2 =
000 and op1 = 6/7 there are no products (W ± X + Z). *(exact on the corpus cases below; op2 corner cases partly)*
Examples: `0803 0c0a 9281` = r1 + r2·(r0 − r1) (lerp); `2b09 2c8f … 5983 … a2aaaa` = r3 + r7·r2 − r5/6;
`0807 3cca 1cb6 002e3bc7 01a01a01` = r3·r2 + 2/315 + (2/2835)·r1; `1107 7522 … 0025 800d00 022222` =
r4·(−1/5040) + 1/120.

## 7. Transfers and references

Transfer `X mm Y` at a T position: mode byte mm = `fe`: X 0–7 loads rX ← [slot Y/2], X 8–15 stores
[slot Y/2] ← r(X−8). Slots are counted from the first word after the transfers, MAC second words included.
*(certain)* Other mode bytes (`9b`, `a9`, `e0`, `e1`, `db`, …) do not change r0–r7: **not decoded** (likely
address registers / delay-line access). Reference words: `02nn`/`04nn`/`06nn` local data, `C0nn` parameter block,
`A0nn` bank 5, `22nn` mailbox between main and slot program (from the static analysis).

## 8. Condition words (C)

*(measured on master DSP3, a context no other task shares, and on DSP4 slot `0xEFDC`)*
* A C word predicates the A/B ops of its instruction. Low byte = which op and how (`x10`, `x11` → op 1,
  `x12` → op 2; `x00`, `x18`, `x30` → none). High byte = condition code: **bit 0 negates**, bits 4–1 select
  **one of 16 conditions**.
* The 16 conditions are **static context bits**, not result flags: no A or B op changed them, they do not change over
  time, and they differ per context: DSP3 main program `0111111011101110`, DSP4 slot `0xEFDC` `0111101111100111`
  (condition 0 first). Probably set per task by the ARM (the OS program swap takes a "task" argument, 2 or 3).
  Roland's code uses them to specialise shared code per slot / voice; **data-dependent choices use min / max / abs
  (unit B)**, not C.
* The earlier "varies over time" on DSP4 came from reads with a zeroed word and from other tasks; with a private
  context the result is stable.

## 9. Open

1. What sets the 16 context condition bits (ARM side); the second word of C instructions in the slot array.
2. Memory between executions: on master DSP3 (private, runs once per 1024-sample block, 46.9/s) memory persists
   and a counter accumulates; on DSP4 slot `0xEFDC` (shared task, ~4 000 runs/s) it did not. Load latency /
   load-after-store effects: **hypothesis**, not settled.
3. Unit B ops returning `7ffxxxxx`; odd B nibbles; hidden values of A op 0 sub 4/5; D ≥ 8 inside multi-op classes.
4. Transfer modes other than `fe`.

## Method

`re/dsppeek.py` (region 0 `-DPEEK`, MIDI CC1 + pitch bend in, SysEx out) writes test programs into master DSP4
slot `0xEFDC` (`re/dspmaster.py`; context `DSPCTX=dsp3` (default: master DSP3 main program, private) or `efdc`; original restored; `--restore` repairs a killed run) and reads bank 0 (RAW
view, majority of 7 reads). `re/dspprobe.py` (fixed registers), `re/dspfit.py` / `re/dspclass.py` /
`re/dspnlfit.py` (random registers, exact smallest-formula search), `re/dspslots.py` (template positions),
`re/dsptemplates.py` (templates). Only one tool may use the MIDI port at a time.
