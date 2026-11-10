# Volume III — Chapter 4: Execution Time and Instruction Budgets

> The measured time budgets, each with its source. Certainty tags: [measured] / [M] on the device, [E] / [sim]
> emulator, [C] our code, [L] / [code] read in Roland's code, [est] estimate.

## 1. The model: one cycle per instruction, the whole program once per sample

* Each DSP runs all its programs (main + slots / sections) once per sample, at **96 kHz** [exact]
  ([Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §1; measured: ~96 000 executions/s on master DSP3, and an
  oscillator computed with fs = 96 000 is in tune within ±1.2 cents on the slaves,
  [Vol. II ch. 3](../vol2_dsp_core_isa/instruction_word_encoding.md) §1).
* **1 cycle per instruction, whatever its content**: 1 op, A/A/B, 1 or 2 MACs, up to 6 transfers, markers [measured]
  (ch. 1 §1; [Vol. II ch. 4](../vol2_dsp_core_isa/instruction_set_reference.md) §14: `re/dspbudget.py
  mac|mac2|xfer4|xfer6`).
* **Jumps**: a taken jump costs about one extra cycle; what a jump skips costs nothing (a jump over 1500 fillers kept
  the counter running; a 4-instruction loop body fits 180 iterations, not 185, with ~25 other instructions → ≈ 5
  cycles per iteration) [measured] (ch. 3 §5.4b).
* All the programs of a DSP add to the same budget; a SYSTEM-1 section gives space, not time
  ([ch. 3](slot_program_model.md) §1, §3.2).

## 2. Budgets per DSP

| device / DSP | ran | hung | source |
|---|---|---|---|
| SYSTEM-8, every DSP | ≈ 925–945 instructions per sample (master DSP3: the counter runs with 900 fillers + 22 and stops with 920–950) | above that | ch. 3 §1 [measured]; ch. 1 §1 |
| SYSTEM-8, slave DSP2/3 (main2/3 = 189 + both oscillator slots) | a DCO2 slot of 733 instructions (DCO1 slot 6) | 753 | ch. 3 §1 [measured] |
| SYSTEM-1, slave DSP A | **703** (MP-CSAW, 8 voices, 5854 bytes, 2026-10-28) | **826** (also with pure filler) | ch. 1 §1; [examples/system_1_synth/README.md](../../examples/system_1_synth/README.md) §4 [measured] |
| SYSTEM-1, slave DSP B | **708** (C-S1SYR, 2026-10-23) | upper limit not searched | examples/system_1_synth/README.md §4 [measured] |

What happens when going over: the end of the program is lost; on the slaves the device can stay unstable (wrong / no
sound, LFO-like garbage) until a **power cycle** (seen twice on the slaves) (ch. 3 §1; ch. 1 §1). Why it leaves the
slaves unstable is open (ch. 1 §11 item 7).

## 3. Space limits that act like budgets

| limit | value | source |
|---|---|---|
| SYSTEM-1 main program | **6 KiB** (`0x0100`–`0x18FF`): 6534 bytes / 702 instructions and 6740 / 578 hung; 5854 / 703 ran (2026-10-28). Whether it is the region size or the bytes executed per sample is open | [ch. 3](slot_program_model.md) §3.3 [measured] |
| size of an instruction | 1–31 words (2–62 bytes); average 6–8 bytes in Roland's programs, ≈ 8–10 in ours; 6 KiB ≈ 600–750 of our instructions | ch. 1 §1; [system-1/ARCHITECTURE.md](../../system-1/ARCHITECTURE.md) §1.0 |
| SYSTEM-8 program window | 64 KiB per DSP; main space `0x0100`–`0x5FFF` (DSP2–5) or `0x7FFF` (DSP0/1); each DCO slot `0x5000` | ch. 3 §2.1 [code] |

## 4. What the known programs cost

| program | instructions per sample | source |
|---|---|---|
| JX-3P on the SYSTEM-8, 4 voices per slave: DSP0 / DSP1 | 516 / 514 | [ch. 1](system8_voice_allocation.md) §3.3 [E] |
| JX-3P: DSP2 / DSP3 (MAIN2 + DCO1 SAW + DCO2 SAW) | 839 / 834 (≈ 90 % of the budget: hence two voices per oscillator DSP) | ch. 1 §3.3 [E] |
| JX-3P: DSP4 / DSP5 | 532 / 513 | ch. 1 §3.3 [E] |
| Roland's oscillators | ≈ 160 per voice (DCO1 SAW: 325 for two voices) | ch. 1 §8 [E] |
| our naive saw (§5.2 of ch. 1) | 33 per voice: time for ≈ 25 voices on an oscillator DSP | ch. 1 §8 [C] |
| C-S1V4 / C-S1V8, slave DSP A | 137 (4 saws) / 269 (8 saws) | [ch. 2](system1_voice_allocation.md) §5 [M] |
| SH-101 on the SYSTEM-1, slave DSP A main | 575 | ch. 2 §5 [L] |
| JX-3P microcode on the SYSTEM-1 (4 DSPs) | 4 complete voices: 3748 of ≈ 3660 (do not fit); 4 voices with one DCO: 3098 (fit) | ch. 2 §6 [E] |
| C-S1SY, by step (DSP A / B) | e.g. step 9: 693 / 652; step 10 (`ph_wrap`): 517 / 652; step 11 with `FUSE`: 576 / 603; step 12 (scheduler): 553 / 471 | [ch. 6](microcode_optimization.md) §4, §5 |
| MP-CSAW2, 8 voices (SYSTEM-S1CC) | A 664 (498 + section 166), B 701 (512 + section 189) | [ch. 8](s1_optimizations.md) §1 [sim] |
| MP-CSAW2-ARM (envelopes and LFO on the ARM) | B 375 | ch. 8 §3 |

## 5. How to stay inside the budget

* The mandatory rules for own microcode (phase ops, packing, fusion, no repeated loads, bit-exact validation) and the
  scheduler: [ch. 6](microcode_optimization.md).
* What Roland's oscillators do to fit: [ch. 7](roland_oscillators.md).
* SYSTEM-1 catalogue of measured savings (envelopes / LFO / pitch on the ARM, half rate with jumps, LUT costs):
  [ch. 8](s1_optimizations.md).
* Native ops that replace sequences (shifts, SFU, `add3`, two-product MAC, indexed loads):
  [Vol. II ch. 10](../vol2_dsp_core_isa/native_ops.md).
* The SYSTEM-S1CC build applies the SYSTEM-1 limits by itself (`make_s1cc.py`: `BUDGET_A` / `BUDGET_B` /
  `MAIN_BYTES`) and chooses how many voices fit ([system-1/ARCHITECTURE.md](../../system-1/ARCHITECTURE.md) §1.0).
