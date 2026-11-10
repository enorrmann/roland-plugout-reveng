# Volume II — Chapter 2: Data Types and Arithmetic

> Every statement has its authority in [ch. 1](core_architecture.md) §3, §5, [ch. 3](instruction_word_encoding.md) §2,
> [ch. 4](instruction_set_reference.md) §8, §10, or in the tools named in the text. Certainty tags as in ch. 1:
> **[exact]** verified bit by bit or in all cases, **[measured]**.

## 1. The RAW number format **[exact]**

The DSP has one data type: a 32-bit sign-magnitude number with a 2-bit exponent ("RAW"). Registers r0–r7 and every
data word the DSP loads or stores are RAW.

```
 31   30 29  28                                                       0
┌────┬──────┬──────────────────────────────────────────────────────────┐
│ S  │  s   │ M (29-bit unsigned magnitude)                            │
└────┴──────┴──────────────────────────────────────────────────────────┘
value = (−1)^S · M / 2^(32 − 4·s)
```

| s | range of \|v\| | resolution |
|---|---|---|
| 0 | < 1/8 | 2^−32 |
| 1 | < 2 | 2^−28 |
| 2 | < 32 | 2^−24 |
| 3 | < 512 | 2^−20 |

Source: ch. 1 §3; ch. 3 §2. Measured on 2026-10-14 with 74 floats written through the IEEE view of master DSP4's
bank 5 and read back through the SD31 and RAW views (`examples/from_scratch/re/dspnum.py`, module docstring).

## 2. Encoding constants

What Roland's constants and the hardware use: the **smallest s** with M = |v|·2^(32−4s) < 2^29, M rounded (ch. 3 §2).
`re/dspnum.py encode` implements it. Examples (from ch. 3 §2; re-computed with that rule on 2026-11-10):

| value | RAW |
|---|---|
| 1.0 | `30000000` |
| 0.5 | `28000000` |
| 0.25 | `24000000` |
| −1.0 | `b0000000` |
| 2.0 | `42000000` |
| 100 | `66400000` |
| 1/192 | `01555555` |
| π/2 | `3921fb54` |
| ln 2 | `2b17217f` |
| 0 | `00000000` |

Immediates in the instruction stream are the **top** 16, 24 or 32 bits of a RAW word, the rest zero: imm16 `2800` =
0.5, imm24 `2c4000` = 0.765625 (ch. 3 §4.4). Immediates of hidden-register loads (`40TY`) are integers = value·16384
instead (ch. 1 §5.6).

## 3. Results: exact, then truncated toward zero **[exact]**

* The ops compute the **exact** result with no intermediate rounding (`D + X·Y`; the MAC with two products plus W;
  the MAC with D ≥ 8 = result/2) and encode it **once**, **truncating toward zero** to the smallest exponent in which
  the mantissa fits in 29 bits. Verified bit by bit on the master with `re/dspexact.py`: 48/48 random cases each in
  add, subtract, product, multiply-accumulate, MAC and MAC/2 (ch. 1 §3; ch. 3 §2).
* **Overflow**: if |result| ≥ 512 the magnitude is taken **modulo 512** and the sign is kept: 300 + 300 → 88,
  −300·2 → −88, 511·2 → 510; 256 + 256 → 0 with flag V (ch. 1 §3; ch. 4 §8).
* There is no divide op: reciprocals come from the seed + Newton steps ([ch. 5](sfu_and_accelerators.md) §2;
  ch. 4 §6.2).

## 4. Other number domains inside the units

| unit / op | domain | wrap / rounding | source |
|---|---|---|---|
| unit B bit ops (`and`, `andn`, `or`, `orn`, `xor`, `xnor`; D < 8, nibbles 0–a) | two's-complement fixed point, 23 fraction bits, range 512 | result **mod 512**: a negative reads as 512 + x (words `7ffxxxxx`) | ch. 1 §3, §5.2 **[measured]** |
| shifts `shl` / `shr` / `shrn` (unit B, D ≥ 8, nibble c, modes 1–3) | two's complement with **32 fraction bits** | rounds toward −∞ at 2^−32, wraps into [−512, 512) (2.65·2⁸ = 679 → −345), then the RAW encoding truncates toward 0; a negative result on the 2^−32 grid is 1 LSB below `mul` | ch. 1 §5.2 **[exact, 2026-10-29]** |
| `divseed` (unit B, nibble e, bits 7–6 = 10) | numerator·2^−e in two's complement, 32 fraction bits | toward −∞ | C reference `emu/dspcore.h`, measured 2026-10-29 (`re/dspnative.py raw_e10_*`) |
| `log2split` with Y < 0 | Y read in two's complement | rY rounded toward −∞ at 2^−32, then RAW truncates | ch. 1 §5.3 **[exact, 2026-10-29]** |
| `f6Y` (`f6c` … `f6f`) | — | (rd + h) **mod 512** | ch. 1 §5.3 |
| hidden registers (`40TY` mode 0) | — | h ← rY **mod 512** | ch. 1 §5.6 |

## 5. Flags from results **[measured]**

`X9bY` copies the flags of unit Y's result into group X − 8 (X = 9…d → groups 1…5); per group c0 = ≠ 0, c1 = ≥ 0,
c2 = no overflow (|x| < 512), c3 = ≤ 0. For min/max, N is the sign of the result and Z means "operands equal". One
instruction of latency; the flags persist until rewritten. Group 0 reads `0111` and conditions 24–31 read `1111`
(constants). Source: ch. 1 §5.5; ch. 4 §8.

## 6. Order of reads and writes **[exact / measured]**

* All ops of an instruction read their operands before any of them writes; a store in the same instruction stores the
  **old** register value (ch. 1 §4.2; ch. 4 §10).
* A loaded register is visible one instruction later (load latency 1) (ch. 1 §7.3).
* An op and a load that write the same register in one instruction are **added** (ch. 1 §7.3; ch. 4 §9). Avoid.

## 7. How the ARM sees the numbers

* The ARM sees DSP data memory through three views: IEEE float, Q31 ("SD31", saturated to [−1, 1)) and RAW; the DSP
  itself only works in RAW. For bank 0 the views are at +0, +0x2000 and +0x4000 of `0x41400000 + n·0x100000`
  (ch. 1 §3, §8.5; ch. 3 §2; `re/dspnum.py`).
* SYSTEM-8: the OS services that write bank 5 (`+0xB0`) and bank 6 (`+0xA4`, `+0xA0`) take a C `float`; the OS
  converts it to RAW ([Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §4.5–§4.6).
* SYSTEM-1: the services `FD6C` and `FCEC` receive the IEEE-754 bit pattern of a `float` in an integer register
  (the SYSTEM-1 ARM has no FPU) ([system-1/WAVE_BUS.md](../../system-1/WAVE_BUS.md) §4.2–§4.3;
  [system-1/ARCHITECTURE.md](../../system-1/ARCHITECTURE.md) §1.1).

> Paths `re/…` and `emu/…` are relative to `examples/from_scratch/`.
