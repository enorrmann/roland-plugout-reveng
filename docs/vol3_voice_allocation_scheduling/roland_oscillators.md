# Volume III — Chapter 7: Roland's Oscillators in the SYSTEM-8 and SYSTEM-1 PLUG-OUTs

> **Scope:** both (Roland's SYSTEM-8 and SYSTEM-1 PLUG-OUTs, each marked).

Study specification (read only, no bytes are ever copied: [Vol. VII ch. 1](../vol7_cleanroom_evidence_archive/cleanroom_boundary_contract.md)). How
Roland's oscillator programs compute the waveforms and which instructions they use so that they fit in the budget of
each sample. The rules that come out of here for our microcode are in
[Vol. III ch. 6](microcode_optimization.md).

State: 2026-10-23. Sources:
* SYSTEM-8: the local corpus `work/corpus/*.bin`, which is not in git;
* SYSTEM-1: `work/unpack/<NAME>_S1_Code1`, listed with `python3 system-1/s1_progs.py <dir> --listing`.

Everything was disassembled with `asm/disasm.py`. The statistics of §4 come from counting mnemonics in those listings.

Tags: **[code]** = read in the binaries; **[measured]** = tested on the device; **[inferred]**.

## 1. Common scheme (all models, both devices) [code]

All the oscillators follow the same steps, with the same constants on the SYSTEM-8 and on the SYSTEM-1:

| step | what it does | instructions |
|---|---|---|
| 1. pitch → increment | `x = p − 4.75` (p = pitch in octaves, 4.75 = A 440), `x·ln 2`, split into exponent and mantissa, polynomial of e^y | `addxy … #-4.75`, `mul … #0.69314718`, `expsplit`, `newton r2, #1.375001`, chain `mac_p1p2c3` / `mac_p1p2c2` (Horner with two coefficients per instruction) |
| 2. limit | increment × sampling constant, cap (JX-3P: 0.249 per sub-sample) | `mul … #0.0022916666`, `min … #0.248999998` |
| 3. 1 / increment | width of the transitions, in samples: `256·inc`, reciprocal seed + 3 Newton | `mul r6, r0, #256`, `recipseed r4, r6`, `newton r4, r6` ×3 |
| 4. phase | `phase += inc`, wrapped to [−1, 1) | `addxy rF, rI, rF` + `ph_wrap rF, r3, rF` |
| 5. shape | triangle of the phase, scaled by the reciprocal (slope = 1/inc), clipped to ±1: a ramp one sample wide at each discontinuity | `ph_tri r3, r3, -r3`, `mul r3, r3, #256`, `mul r3, r3, r4`, `ph_clamp r3, r3, -r3` |
| 6. smoothing | the ramp goes through sin(π/2·x), a polynomial of degree 7: the step is band-limited (BLEP-like) | `mul … #1.570796013`, `mac_p1` / `ymacd` with 2.742e−6, −1.984e−4, 8.333e−3, −1/6 |
| 7. mix | saw = phase − correction; square / pulse = `sign` of the shifted phase, with the same correction; the waves are added with the levels (bank 6 `C0xx`) | `ph_add`, `sign r1, -r3`, `mac_p1 r2, r5, r6, r2` |

Differences between devices:
* **SYSTEM-8, JX-3P and Juno:** steps 4–6 are repeated **4 times per sample** (sub-samples) and the 4 are decimated with
  a filter (`mac_p1p2c3r`, `mac_p1p2`).
* **SYSTEM-1, SH-2:** each voice computes **once per sample** the shapes that the panel mixes (saw, square / pulse,
  sub) and applies the smoothing to each discontinuity: 4–6 reciprocal + sine blocks per voice.

## 2. SYSTEM-8: the DCO of the JX-3P (`JX_6000_60a1b594`, 325 instructions) [code]

DCO slot (DCO1 / DCO2, programs of 325 and 357 instructions). Slot interface: [Vol. VI ch. 6](../vol6_reference_designs_applications/custom_osc.md) §2.

1. Input `[2201]` (pitch). Steps 1–3: about 20 instructions, once per sample.
2. Per sub-sample, about 26 instructions, ×4:
   * `mac_p1 r3, r3, #0.5, #0.5 | mov r7, r3 | st … | ld … | ld …`: half phase and five transfers in a single
     instruction;
   * `ph_tri`, then `mul r5, r3, #256 | ph_add r1, r3, r7` and `mul r5, r5, r4 | ph_wrap r1, r3, r1`: each instruction
     carries one A op and one B op;
   * `ph_clamp`, and the sine polynomial in 4 double instructions: `mac_p1 … | mul …`, `ymacd … | mul …`;
   * sync and crossing: `cmp`, `setflags`, `cond` and `sum8`, all in the same instruction as the arithmetic.
3. Outputs `[221f]` / `[221b]`.

46 % of the instructions carry two or three operations; only 23 of 325 are transfers only.

## 3. SYSTEM-1: the voice of the SH-2 (section `0x1900` of the slave, 353 instructions) [code]

A listing of one voice section. The constants are those of the JX-3P:

```text
ld r0, [0037] | ld r1, [0026]                         ; pitch of the voice, level
addxy r0, r0, #-4.75 | mov r4, r1
mul r1, r0, #0.69314718
expsplit r0, -r1
mac_p1 r3, r1, #2.56533e-05, #0.0002821868 | newton r2, #1.375001
mac_p1p2c3 r3, r3, r2, #0.0007054673, r1, #0.0063492062   ; ... 4 more: e^y by Horner, 2 terms / instruction
...
addxy r3, r4, r0 | st r4, [0106] | st r6, [0113] | ld r5, [0104]
ph_wrap r3, r3, r3 | st r1, [0110] | st r5, [0107]     ; wrapped phase: one op
mul r6, r0, #256 | ld r1, [0105]
recipseed r4, r6                                       ; 1 / (256 inc): seed + 3 Newton
newton r4, r6
newton r4, r6
newton r4, r6
mac_p1 r3, r1, #0.5, #0.5 | st r4, [0112]
mul r1, r1, #-1 | ph_tri r3, r3, -r3                  ; A op + B op in one instruction (class 0x00b0)
mul r3, r3, #256 | ld r4, [0112]
mul r3, r3, r4
mul r3, r3, #0.25
ph_clamp r3, r3, -r3                                   ; one-sample ramp at the discontinuity
mul r3, r3, #1.570796013
mul r4, r3, r3
mac_p1 r2, r4, #0, #2.742e-06 | mul r5, r4, r3         ; sin(π/2 x): MAC + mul in one instruction
mac_p1 r7, r4, #-0.0001984, #0.00833333 | mul r6, r4, r4
ymacd r2, r6, r7 | mul r7, r4, r5
```

(Text of the listing of `s1_progs.py`, without the `@` annotations of the immediates; it is not a `dsp` block because
it uses forms that our assembler does not write yet.)

The section does that block for each discontinuity of the mix: 12 `recipseed`, 37 `newton`, 14 `ph_tri` and 12
`ph_clamp`. 38 % of the instructions carry two or more operations.

## 4. All the oscillator programs [code]

`instr.` = instructions; `fused` = instructions with two or more operations (not counting `ld`/`st`/`cond`); `transf.
only` = instructions with only `ld`/`st`. Mnemonic columns: number of occurrences.

SYSTEM-8 (corpus):

| program | instr. | fused | transf. only | `ph_wrap` | `ph_tri` | `ph_clamp` | `recipseed` | `newton` | `mac_p1*` | `ymacd` |
|---|---|---|---|---|---|---|---|---|---|---|
| JX-3P DCO `JX_6000_60a1b594` | 325 | 150 | 23 | 24 | 8 | 8 | 2 | 8 | 42 | 8 |
| JX-3P DCO `JX_6000_60a1d5f4` | 357 | 116 | 29 | 16 | 8 | 8 | 10 | 32 | 20 | 0 |
| Juno-106 `J106_6000_60a1d790` | 523 | 310 | 23 | 8 | 8 | 24 | 2 | 8 | 90 | 40 |
| Juno-60 `J60_6000_60a1c780` | 510 | 250 | 30 | 8 | 24 | 24 | 10 | 32 | 88 | 24 |
| Jupiter-4 `JP4_6000_60a1c450` | 616 | 338 | 40 | 8 | 24 | 24 | 10 | 32 | 112 | 48 |
| SH-1000 [inferred: prefix `S100`] `S100_6000_60a16bc0` | 272 | 109 | 23 | 4 | 12 | 9 | 5 | 16 | 54 | 9 |
| SH-101 `SH_0100_60a12bc0` (in the main program, [Vol. VI ch. 6](../vol6_reference_designs_applications/custom_osc.md) §11) | 610 | 149 | 145 | 4 | 16 | 17 | 16 | 49 | 68 | 19 |

SYSTEM-1 (voice sections of the slave):

| PLUG-OUT / program | instr. | fused | `ph_wrap` | `ph_tri` | `ph_clamp` | `recipseed` | `newton` | `mac_p1*` | `ymacd` |
|---|---|---|---|---|---|---|---|---|---|
| SH-2 section `0x1900` | 353 | 135 | 4 | 14 | 12 | 12 | 37 | 59 | 13 |
| SH-2 section `0x3100` | 245 | 71 | 4 | 10 | 8 | 8 | 25 | 30 | 9 |
| SH-101 main `0x0100` | 575 | 137 | 4 | 16 | 17 | 16 | 49 | 68 | 18 |
| PROMARS section `0x1900` | 344 | 161 | 4 | 12 | 12 | 5 | 16 | 57 | 28 |

No Roland oscillator wraps the phase by hand: they all use `ph_wrap` (or `ph_mod2` / `ph_abs` for phases 0..1)
[code].

## 5. Instructions Roland uses to save cycles [code]

| technique | form | example |
|---|---|---|
| phase wrapped in one op | `ph_wrap rD, r3, rY` (X always r3) | `ph_wrap r3, r3, r3` |
| shapes from the phase | `ph_tri rD, r3, -rY` (triangle), `ph_clamp rD, r3, -rY` (±1), `ph_abs`, `ph_add`, `ph_mod2` (phase 0..1), `sign rD, -rY` | `mul r1, r1, #-1 \| ph_tri r3, r3, -r3` |
| A op + B op | class `0x00b0`: an op of unit A with one of unit B (`ph_*`, `min`/`max`, `sign`, `cmp`) | `mul r5, r5, r4 \| ph_wrap r1, r3, r1` |
| MAC + A op | `mac_p1 … \| mul …`, `ymacd … \| mul …` | sine polynomial |
| 2-term Horner | `mac_p1p2c3`, `mac_p1p2c2`: two coefficients per instruction | e^y in 5 instructions |
| up to 5 transfers per instruction | `ld`/`st` in the same instruction as the ops (limits: [examples/from_scratch/emu/chain.py](../../examples/from_scratch/emu/chain.py) `access_violations`) | `mac_p1 … \| mov r7, r3 \| st … \| ld … \| ld …` |
| selection without jumps | `cond c, …` / `setflags` / `cmp` in the instruction of the op | sync of the JX-3P |
| reciprocal | `recipseed` + 3 `newton`, without dividing | width of the ramps |

Cost: on the SYSTEM-8, an A/A/B instruction (class `0x160`) costs the same as a single-op one [measured,
[Vol. II ch. 4](../vol2_dsp_core_isa/instruction_set_reference.md) "time budget"]. On the SYSTEM-1, C-S1SY11 (576 fused instructions on DSP A, 649
unfused) works on the device [measured, 2026-10-23]: the fusion does not take it over budget.
