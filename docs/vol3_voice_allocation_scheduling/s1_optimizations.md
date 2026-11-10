# Volume III — Chapter 8: SYSTEM-1 Optimizations (SYSTEM-S1CC): Catalogue

> **Scope:** SYSTEM-1.

What can be got out of the SYSTEM-1 slave DSPs and how much, measured with the emulator on real builds. Reference:
MP-CSAW2 (`sdk/s1cc/build/mp_csaw2`, compiled with `--regs-a 0,1,2,3 --regs-b 0,1,2,3`), `CMOD_IDX=1 MOD_DSP=B`, 8
voices. Measured limits: DSP A 703 and DSP B 708 instructions per sample. State as of 2026-10-29. What is measured on
the device is in [Vol. II ch. 10](../vol2_dsp_core_isa/native_ops.md) and in the README of each example.

Legend: **[M]** measured on the device · **[sim]** measured in the emulator · **[est]** estimated · **[code]** read in
the OS.

## 1. Starting point [sim]

| DSP | instructions (8 voices) | who uses them |
|---|---|---|
| A | 664 (498 + section 166) | two CSAW per voice (kernel, 81 per voice), modulated pitch (`exp2`), mixer |
| B | 701 (512 + section 189) | per-voice LFO, 2 ADSR per voice, pitch envelope, COLOR MOD, kernel B (37 per voice) |

## 2. Candidates, from the largest to the smallest saving

| # | what | saving | state | what is missing |
|---|---|---|---|---|
| 1 | ADSR envelopes on the ARM, with chained ramps (§3) | **B −213** | [sim] the saving; **[M] the mechanism** (2026-10-30) | write `ARM_ENV`; test 16 parameters at once on the slave |
| 2 | per-voice LFO on the ARM (§3) | **B −119** | [sim] | same; per-voice shape and phase on the ARM |
| 3 | modulated pitch (LFO and pitch envelope → `dt`) on the ARM, with a per-voice ramp (§4) | **A −115** | [sim] (`LFO_PITCH=0`) | 8 parameters of DSP A with a ramp instead of `A040+v` |
| 4 | envelopes at half rate with a jump: half the voices in even samples, the other half in odd ones (§5) | B ≈ −105 | [est] | first jump on the S1 (on the S8 it is measured) |
| 5 | global LFO (no reset per note): voice 0 computes it and the others copy its value (fade, → amp and → filter stay per voice) | B −83 | [sim] | panel option; loses KEY TRIGGER |
| 6 | new op groups (`TRY_FORMS`, [examples/system_s1cc/README.md](../../examples/system_s1cc/README.md) §4.1) | 0–20 per DSP | [M] per group | continue the bisection |

**1 + 2 + 3 together**: DSP B from 701 to 369 (−47 %), DSP A from 664 to 549. That room goes to the kernel: two longer
B kernels, or per-voice filters.

## 3. Envelopes and LFO on the ARM: chained ramps [M]

* **Hardware ramp** [M]
  * `FCE4(sel, id, target, y, z)`: linear, one step every 1/3 ms, from 4 to 97 ms from 0 to 1 according to `y`.
  * `+8` of the block is the current value and `+C` the increment per step.
  * Each `y` is a fixed time (4.00 ms with `y = 0`) for any distance [M, 2026-10-30]: each segment lasts exactly what
    is chosen.
  * Detail in [examples/system_1_ramp/README.md](../../examples/system_1_ramp/README.md).
* **Without a tick** [M, 2026-10-30]
  * `FE3C(sel, id, fn)` registers an own function in the end-of-ramp table.
  * The DSP's interrupt calls `fn(id)` at every end of a ramp with `z = 1` and does not release the entry.
  * `fn` launches the next segment.
  * Measured on the master: 6225 calls in a row, one every 4.00 ms ± 0.02, without hanging.
* **Done: `ARM_ENV` / `ARM_LFO`** in SYSTEM-S1CC ([examples/system_s1cc/README.md](../../examples/system_s1cc/README.md)
  §3.7). MP-CSAW2-ARM: 8 voices, DSP B 375 instructions. Installed on 2026-10-30.
* **SYSTEM-8**: its ARM has an FPU. When porting it, `float` instead of Q24. The ramp of the S8 is `+0xA0`, with another
  input format ([Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §4.6); the equivalent of `FE3C` would have to be found in its OS.
* **Design (`ARM_ENV`)**
  * Per voice and envelope, one parameter of DSP B (`C0id`, 16 ids) and a state on the ARM: stage, level and target.
  * On note on, `FCE4` towards the first point of the attack.
  * At each end of a ramp, the next point of the stage: segments of ≤ 97 ms, and the exponential curve approximated by
    lines. The error of a chord of length h on an exponential of time constant τ is ≈ (h/τ)²/8. With 4 ms segments it
    gives < 1 % for times of 0.1 s or more (τ ≥ 15 ms), and 95 % for 1 ms.
  * **Limit: a ramp lasts at least 4 ms**: the duration depends only on `y`, not on the distance [M]. Attacks and
    decays of less than ~10 ms cannot be done with ramps: an immediate jump (`FCEC`) clicks. For that there are two
    options: leave in the DSP a pole that follows the ARM's value (2–3 ops per envelope), or accept 4 ms as the
    minimum.
  * On note off, release from the current value (`+8`).
  * In DSP B, the ADSR (43 lines) becomes `ld` + `st` in the COLOR MOD table.
* **LFO**: the same, with one parameter per voice. The shape comes from a table on the ARM (any shape, no cost in the
  DSP) and the phase is per voice.
* **Risks**
  * The function runs in the interrupt.
  * There is a single `id` register per DSP: it is not known what happens if two ramps end together.
  * The 3 kHz steps in the amplitude: a pole in the DSP smooths them with 2 ops per voice.

> Later state (2026-11-03, [Vol. I ch. 6](../vol1_system_architecture/s1_arm_clock.md)): the ARM envelopes and LFO of
> SYSTEM-S1CC were also made with a 1 ms tick hooked into the OS cyclic handler (`FCEC` to `C080`–`C09F` every 1 ms,
> or `FCE4` every 4 ms only for ids < `0x80`).

## 4. Modulated pitch on the ARM [sim]

Today DSP A computes 2^y (Taylor + three squarings, ~14 instructions per voice with 8 voices), with y = LFO + PITCH ENV +
RANGE. The ARM can send `dt` (and `rdt`) already modulated with a ramp. The modulation is slow (< 40 Hz), so 1/3 ms
steps with linear interpolation in `dt` are more than enough. The chain of §3 has to work, because the LFO and the pitch
envelope advance by themselves. `LFO_PITCH=0` gives the saving: DSP A 664 → 549.

## 5. Half rate with jumps [est]

A per-sample counter and a conditional jump that skips the envelopes of voices 0–3 in even samples and of 4–7 in odd
ones. The rates double, and the worst case per sample halves. What is skipped costs nothing and the taken jump costs 1
cycle ([Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §6). Jumps and conditions are measured on the S8; on the S1 none has been
tested and Roland does not use them there. First step: a PLUG-OUT with a jump that skips half of the voices
([Vol. II ch. 10](../vol2_dsp_core_isa/native_ops.md) §3, point 3). If §3 works, this stops being needed for the envelopes, but it serves for
any control-rate code.

## 6. Tables (LUT) in the DSP [sim]

`LFO_LUT`, `ENV_CURVE` ([examples/system_s1cc/README.md](../../examples/system_s1cc/README.md) §3.6).

* **They do not save.** `LFO_LUT` +23, `ENV_CURVE` +109 (DSP B, 8 voices).
* **They give shapes.** A true sine and envelope curves of choice, with all the envelopes independent over the same
  table.
* **The size is not the problem.** The 128 words the ARM writes are enough; what costs is the ~7 instructions of each
  lookup.
* **Interpolating costs 4 of those 7.** Without interpolating N ≥ 256 would be needed so as not to hear steps, and there
  is no room.
* **On the ARM side** (§3), a table costs nothing in the DSP.

## 7. What was learnt from the scheduler (`asm/dspsched.py`, 2026-10-29)

* `ldb5` / `ldb5h7` (bank 5 at h6 / h7 + Y) are scheduled as bank-5 loads: two instructions after their `hset`, no
  other bank-5 transfer in the instruction. `asm/s1rules.py` counts them in R2 / R5.
* `hset` goes **alone** in its instruction and is not a barrier: it is only ordered with the register it
  reads and with the indexed loads of its h. h6 and h7 are tracked separately, so two lookups can be in flight at once.
* If only waits are left (the distance 2 after an `hset`), the scheduler puts a filler (`mov r0, r0`).
* When interleaving two voices (`PAIRS`), an `hset` with its loads goes whole (`make_s1cc.py _chunks`): the h is a
  single one per DSP.
* A lookup per voice inside the voice's code is a serial chain of ~10 instructions. Together in a global block,
  alternating h6 / h7, they cost ~7 per lookup. COLOR MOD (`colors_idx_all`) was already done that way.

## 8. Trap: compiling the kernels with 4 registers

The kernels of SYSTEM-S1CC use r0–r3 (the host interleaves two voices: r0–r3 and r4–r7). A kernel compiled without
`--regs-a 0,1,2,3 --regs-b 0,1,2,3` (with 8 registers, like the one some s1cc tests leave) gives other code and more
instructions. On 2026-10-29, `sdk/s1cc/build/mp_csaw2` had been left compiled that way and MP-CSAW2 stopped fitting in
8 voices (A 844). Recompiling it as `sdk/s1cc/examples/mutable_ported/check_all.sh` does solved it.

## 9. Pending tests on the device, in order

1. The end-of-ramp chain (measured with C-S1RAMP, 2026-10-30) with 16 parameters at once on the slave: seen directly
   with `ARM_ENV`.
2. **MP-CSAW2-LUT** (`plugins/`): LFO shapes and the `sq` curve from bank-5 tables (indexed bank-5 loads run on the S1:
   WT-SLAVE5, [Vol. VI ch. 7](../vol6_reference_designs_applications/mutable_memory.md) §4.1).
3. **PLUG-OUT with a jump** (§5).

The ramp duration depends only on `y` (C-S1RAMP, 2026-10-30).
