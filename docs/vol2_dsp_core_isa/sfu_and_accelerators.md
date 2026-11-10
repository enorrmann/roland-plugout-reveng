# Volume II — Chapter 5: Special-Function Unit, Seed ROMs and Phase Operations

> **Scope:** both (same ISA).

> The non-linear operations in one place; sources: [ch. 1](core_architecture.md) §5.1–§5.3,
> [ch. 4](instruction_set_reference.md) §6.2–§6.4 and [ch. 9](operation_uses.md). The ROM contents were checked
> against the C reference `examples/from_scratch/emu/dspcore.h` (`recip_seed`, `RSQRT_Q`, `EXP_ROM`).

## 1. Where the functions live

| function | unit and encoding | mnemonic (`asm/isa.json`) | source |
|---|---|---|---|
| Newton step | unit A, op 0, sub-code X = 2 | `newton` | ch. 1 §5.1 |
| reciprocal seed | unit B, D ≥ 8, nibble f, bits 7–6 = 00, mode 00, Y bit 3 = 0 | `recipseed` | ch. 1 §5.3 |
| atanh split | same, mode 00, Y bit 3 = 1 | `atanhsplit` | ch. 1 §5.3 |
| root split | mode 01, Y bit 3 = 0 | `sqrtsplit` | ch. 1 §5.3 |
| exponential split | mode 01, Y bit 3 = 1 | `expsplit` | ch. 1 §5.3 |
| log2 split | mode 10, Y bit 3 = 0 | `log2split` | ch. 1 §5.3 |
| sign | mode 10, Y bit 3 = 1 | `sign` | ch. 1 §5.3 |
| ceiling | mode 11, Y bit 3 = 0 | `ceil` | ch. 1 §5.3 |
| floor | mode 11, Y bit 3 = 1 | `floor` | ch. 1 §5.3 |
| reciprocal seed with a numerator | unit B, D ≥ 8, nibble e, bits 7–6 = 10 | `divseed` | `emu/dspcore.h` (measured 2026-10-29) |
| frexp | unit B, D ≥ 8, nibble c, mode 0 | `frexp` | ch. 1 §5.2 |
| shifts X·2^Y, X·2^−Y, X·2^−(16+Y) | unit B, D ≥ 8, nibble c, modes 1/2/3 | `shl` / `shr` / `shrn` | ch. 1 §5.2 |
| phase shaping | unit B, D ≥ 8, nibble e, bits 7–6 = 11 | `ph_add`, `ph_sub`, `ph_abs`, `ph_clamp`, `ph_wrap`, `ph_tri`, `ph_mod2`, `ph_neg` | ch. 1 §5.2 |
| register move, delay position, add hidden, markers | unit B, D ≥ 8, nibble f, bits 7–6 = 01 | `f4`, `f5`, `f6`, `f7` | ch. 1 §5.3 |

All of them cost one cycle like any instruction (ch. 1 §1). r(d) = r(D − 8).

## 2. Reciprocal and division **[exact, 2026-10-28]**

* `recipseed`: |Y| = m·2^e, m ∈ [0.5, 1): r(d) ← sign·2^−e (**0 if 2^−e ≥ 512**), r(d+1) ← R[k], **rY ← m** (the input
  is overwritten).
* `newton` (A op 0 sub 2): rD ← D·r(D+1); r(D+1) ← 2 − r(D+1)·rY; rY ← r(D+1)·rY, all with the old values; results
  ready in the next instruction.
* `recipseed` + three `newton` steps on r(d), r(d+1), rY: r(d) = 1/Y with an error of 10⁻⁸ (Goldschmidt). Roland's SAW
  oscillator uses `0902 4086` three times after the seed (ch. 4 §6.2).
* The division must not read Y after `recipseed`, which overwrites it ([ch. 10](native_ops.md) §5).

**ROM R** (64 entries, `re/dspseed.py`, 64/64): k = the 6 mantissa bits after the leading 1 (steps of 1/128 in
[0.5, 1)); `R[k] = round(4096 / (0.5 + (2k + 1)/256)) / 4096` (ch. 1 §5.3; `recip_seed` in `emu/dspcore.h`).

## 3. Square root **[exact, 2026-10-29]**

`sqrtsplit`: |Y| = m·4^−k, m ∈ [1, 4): rY ← m, r(d+1) ← Q[j]/512, **r(d+3)** ← sign·2^−k. With rY = r(d+1) or r(d+3)
the two writes mix bits (do not use). √|x| = r(d+3)·m·r(d+1), with r(d+1) refined by Newton (ch. 1 §5.3; ch. 9 §6).

**ROM Q** (48 entries, 9 bits): j = ⌊(m − 1)·16⌋; `Q[j] = round(512 / √(1 + (j + 0.5)/16))`:

```text
504 490 476 464 452 442 432 422 414 406 398 391 384 377 371 365 359 354 349 344 339 334 330 326
322 318 314 311 307 304 300 297 294 291 288 285 283 280 277 275 272 270 268 266 263 261 259 257
```

Identical to `RSQRT_Q` in `emu/dspcore.h`. **Difference between the sources:** ch. 4 §6.4 (state 2026-10-17) still
gives an 8-bit table Q/256 and the third output as r7; the 2026-10-29 measurement (384 raw cases + 3072 in
`re/dspnative.py`) replaced both. `asm/isa.json` still describes the third output as r7.

## 4. Exponential **[exact, 2026-10-28]**

`expsplit`: r(d) ← e^⌊Y⌋ from a ROM of its own, rY ← Y − ⌊Y⌋. Roland then evaluates e^frac by Horner (ch. 4 §6.4).
The ROM covers n = −19 … 6; below, 0; above, the largest RAW (511.999998). It is neither truncated nor rounded e^n:
up to 6·10⁻³ relative error at n = −18, 10⁻⁸ near 0 (ch. 1 §5.3). The words, from `EXP_ROM` in `emu/dspcore.h`:

| n | RAW | n | RAW | n | RAW |
|---|---|---|---|---|---|
| −19 | `00000018` | −10 | `0002f9af` | −1 | `25e2d58d` |
| −18 | `00000041` | −9 | `00081679` | 0 | `30000000` |
| −17 | `000000b2` | −8 | `0015fc21` | 1 | `42b7e151` |
| −16 | `000001e3` | −7 | `003bc2d7` | 2 | `4763992e` |
| −15 | `00000522` | −6 | `00a27290` | 3 | `5415e5bf` |
| −14 | `00000df3` | −5 | `01b993fe` | 4 | `63699205` |
| −13 | `000025ec` | −4 | `04b0556e` | 5 | `69469c4c` |
| −12 | `00006715` | −3 | `0cbed866` | 6 | `7936dc56` |
| −11 | `00011835` | −2 | `222a5554` | | |

## 5. Logarithms

* `log2split` **[exact, 2026-10-29]**: Y = m·2^e, m ∈ [0.75, 1.5): rY ← m, r(d) ← e. Y < 0 is read in two's
  complement: r(d) ← ⌊log2((Y + 512)/0.75)⌋, rY ← Y/2^e rounded toward −∞ at 2^−32 (then RAW truncates); e = 9 for
  |Y| < 115 (ch. 1 §5.3).
* `atanhsplit`: x = rY, (x+1)/(2·2^k) ∈ [0.5, 1): r(d) ← (x−1)/(2·2^k), rY ← (x+1)/(2·2^k), r(d+1) ← R[k] of rY; after
  Newton, r(d)·r(d+1) = (x−1)/(x+1) and ln x = 2·atanh of it. Roland (base engine, JUPITER-8): `9f0d` after an `8f25`
  log2 split, then `0902 1085` Newton (ch. 1 §5.3; ch. 4 §6.4).

## 6. Rounding to integers, sign, frexp, shifts

* `floor` ⌊Y⌋ and `ceil` ⌈Y⌉ **[exact, 2026-10-28]**; `floor` was read before as −(Y < 0), which coincides for |Y| < 1
  (ch. 1 §5.3).
* `sign`: ±1, 0 (ch. 1 §5.3).
* `frexp`: X = m·2^e, m ∈ [0.5, 1): r(d) ← m, rY ← e; X = 0 → 0, −31 (ch. 1 §5.2; ch. 4 §6.3).
* `shl` / `shr` / `shrn`: X·2^Y / X·2^−Y / X·2^−(16+Y), Y = the 4-bit field; arithmetic domain in
  [ch. 2](data_types_arithmetic.md) §4 (ch. 1 §5.2).

## 7. Phase shaping (unit B, D ≥ 8, nibble e, bits 7–6 = 11) **[measured]**

Bits 5–4 select the function, Y bit 3 the variant; no immediate (ch. 1 §5.2; ch. 4 §6.3):

| bits 5–4 | Y bit 3 = 0 | Y bit 3 = 1 |
|---|---|---|
| 00 | `ph_add`: r(d) + rY | `ph_sub`: r(d) − rY |
| 01 | `ph_abs`: \|rY\| | `ph_clamp`: clamp(rY, −1, 1) |
| 10 | `ph_wrap`: wrap(rY) into [−1, 1) | `ph_tri`: 1 − 2\|wrap(−rY − 1.5)\| |
| 11 | `ph_mod2`: (rY mod 2)/2 | `ph_neg`: −rY |

Roland's oscillators use `ph_wrap` / `ph_tri` / `ph_clamp` to save cycles ([Vol. III ch. 7](../vol3_voice_allocation_scheduling/roland_oscillators.md)).
On the SYSTEM-1 `ph_wrap`, `ph_clamp`, `ph_tri`, `min`, `max` are proven in the host of SYSTEM-S1CC
([ch. 10](native_ops.md) §2).

## 8. Nibble f, bits 7–6 = 01 **[measured]**

| form | function |
|---|---|
| `{8+d}f4Y` | r(d) ← r(Y & 7) |
| `{8+d}f5c` | r(d) ← delay-ring write position (counts down 1/16384 per sample, mod 512; own value per DSP); a free sample clock with an 87 s period (ch. 9 §5c) |
| `{8+d}f5Y`, Y ≠ c | reads 0 in our contexts (ch. 1 §5.3; ch. 4 §6.4 has the details) |
| `{8+d}f6Y`, Y = c–f | r(d) ← (r(d) + h(Y − 8)) mod 512 |
| `{8+d}f7Y` | section markers, no effect, 1 cycle (ch. 1 §9) |

> Paths `re/…` and `emu/…` are relative to `examples/from_scratch/`.
