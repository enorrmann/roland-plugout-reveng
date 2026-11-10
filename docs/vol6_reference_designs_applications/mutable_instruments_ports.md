# Volume VI — Chapter 4: Mutable Instruments Ports

> **Scope:** SYSTEM-1.

> The full descriptions (Spanish) are [examples/mutable_system/README.md](../../examples/mutable_system/README.md)
> (MUTABLE-SYSTEM), [examples/mutable_system/CANDIDATES.md](../../examples/mutable_system/CANDIDATES.md) (what else can
> be ported) and [sdk/s1cc/examples/mutable_ported/README.md](../../sdk/s1cc/examples/mutable_ported/README.md) (the
> rewritten kernels). The shapes that need memory are [ch. 7](mutable_memory.md). Code of origin: Mutable Instruments
> (Emilie Gillet, MIT), `~/git/eurorack`.

## 1. Three ways the project ported Mutable code

| way | tool | where | state |
|---|---|---|---|
| hand port of one component, with a C++ reference and a bit-exact integer model | `sdk/mport` ([Vol. V ch. 3](../vol5_toolchain_compiler_sdk/mport_mutable_sdk.md)) | Braids SAW_SWARM in MUTABLE-SYSTEM | built and validated in the emulator; installed in slot 1 on 2026-10-24, not heard yet (README of `mutable_system`) |
| wrappers that compile the original C / C++ of Braids / Plaits | s1cc ([Vol. V ch. 1](../vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md)) | `sdk/s1cc/examples/mutable/` | exact with respect to the module but expensive: Braids CSAW 160 instructions per voice (4 voices) against 44 of `mp_csaw` (8 voices); `BRAIDS-CSAW` (wrapper, 4 voices) installed 2026-10-27 gave noise and clicks in low notes on the device, main suspect `recipseed` (mutable_ported README §7) |
| kernels **rewritten** in C in the form the DSP runs cheaply | s1cc | `sdk/s1cc/examples/mutable_ported` | 34 kernels as of 2026-10-28; with the full host (`MOD_DSP=auto`) 25 fit 8 voices, 7 fit 7, 2 fit 5; more added on 2026-11-03 (§3) |

## 2. MUTABLE-SYSTEM (`examples/mutable_system/`)

State 2026-10-24: built and validated in the emulator, **installed in slot 1** (it replaced Roland's SH-2), **not heard
yet** on the device [open]. It is the 8-voice synth C-S1SY13F ([examples/system_1_synth/README.md](../../examples/system_1_synth/README.md))
with the two basic oscillators per voice (DCO1 + DCO2) replaced by Braids' **SAW_SWARM** (7 detuned saws, a tanh
waveshaper and a high-pass). Everything after the oscillator is unchanged (per-voice LFO, VCA and filter ADSR, 4-pole
filter, the OS's delay and reverb). Since the same day the oscillator can also come from a voice in C / C++ compiled with
s1cc (`KERNEL=`, its §8).

| DSP | per voice | instructions |
|---|---|---|
| slave A `s1m0` | LFO → pitch + range → increment (`exp2`) → 7 saws → waveshaper → × level × tremolo → `A151+v`; increment → `A188+v` | **638** (C-S1SY13F: 585; maximum tested 682, 693 worked at step 9) |
| slave B `s1m1` | high-pass of SAW_SWARM on `A151+v` → VCA ADSR → filter ADSR → 4-pole filter | **564** (C-S1SY13F: 471; maximum tested 708) |
| master B `s1mm1` | volume → send / return of the OS's delay and reverb | 7 |

Validation [sim] (its §4): `--pack-check` and `--sim-check` bit-identical on 7 panels; the complete chain against
Braids' C++ within ±0.4 dB up to 4 kHz, −1.9 dB (4–12 kHz) and −10 to −15 dB (12–40 kHz) because of the 4-pole filter
with `GMAX` 0.7. With the s1cc kernel `braids_saw_swarm_s1.cc`: DSP A 626 and DSP B 620 instructions, the synth's output
within ±0.03 dB per band of the hand version ([Vol. V ch. 1](../vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md)
§12).

## 3. The rewritten kernels (`sdk/s1cc/examples/mutable_ported`)

Ports in C of the oscillators and filters of Braids, Plaits and stmlib, **rewritten** for the SYSTEM-1 slave DSPs as s1cc
compiles them (no divisions, `s1_dt` / `s1_rdt`, branch-free polyBLEP). Each `mp_*.c` is a complete kernel for
SYSTEM-S1CC: the oscillator on DSP A, the host's filter and envelopes on DSP B (`include/mp_dsp_b.h`: Chamberlin SVF with
double sampling, 4-pole ladder, one-pole, low-pass gate). Checked with `check_all.sh` (s1cc build + check + 8-voice
budget).

State (README header, 2026-10-28): verified against their native reference (`s1cc check`) and in the complete emulator
chain (`make_s1cc.py --sim`); **MP-CSAW and MP-CSAW2 (6 voices) tested on the device** (MP-CSAW2 with 7 voices and
`MOD_DSP=B`: installed, not confirmed). Its §8: "[measured 2026-10-28] `MP-CSAW` (6 voices, host LFO, COLOR MOD) works on
the device. The other kernels, untested." 16 kernels read OSC1 / OSC2 COLOR with their COLOR MOD (`s1_color`). §4.1
(2026-11-03) adds all the memory-less oscillators of Braids, Plaits, Peaks and Tides that were missing (32 more kernels; table with
instructions per voice, SNR and voices per kernel in the README).

Filters (its §5): the SVF low-pass of all the kernels (B 38–39 per voice), `mp_filter_ladder` (45), `mp_filter_bp`,
`mp_filter_hp` (39), `mp_lpg` (42), `mp_vowel` (3 parallel formants, 49 per voice; 8 voices: B 534).

What the work added to the compiler and the host (its §6): `s1_dt()` / `s1_rdt()` (−25 ops per voice in every band-limited
oscillator), float phases in [−1, 1) (one-op wrap), normalised `min` / `max` (SVF of DSP B 42 → 39 per voice), `s1_color`,
`s1_lfo_filter` / `s1_lfo_amp`, regression tests.

Other results: MP-CSAW2-8 with indexed COLOR MOD ran 8 voices on the SYSTEM-1 (DSP B 707 of 708,
2026-10-29, [Vol. II ch. 10](../vol2_dsp_core_isa/native_ops.md) §5); MP-CSAW2-ARM (envelopes and LFO on the ARM), 8
voices, DSP B 375 instructions, installed 2026-10-30 ([Vol. III ch. 8](../vol3_voice_allocation_scheduling/s1_optimizations.md) §3).

## 4. What else can be ported (`CANDIDATES.md`, 2026-10-24)

The inventory of `~/git/eurorack` (commit `f4cfa76`) with the budget per DSP, the measured costs of the basic pieces
(increment 12 units, saw 3–4, square / triangle 1–2, degree-9 odd polynomial 7, …), the memory and the operations that
were missing, and the proposed order: SAW_SWARM strike, SampleRateReducer on CRUSHER, FM / feedback FM, Plaits'
low-pass gate, phase distortion or Z oscillator, Warps modulator, a fixed-tap diffuser; everything with variable delay or
tables read by phase need the computed index ([ch. 7](mutable_memory.md)). Budget of MUTABLE-SYSTEM: slave DSP A 638 used, slave DSP B 564,
master DSP B 7 (limits: [Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §1).

Most of the oscillators that CANDIDATES.md and the MUTABLE-SYSTEM README list as candidates exist as `mp_*` kernels
(§3); the shapes that need memory are [ch. 7](mutable_memory.md).
