# Volume III — Chapter 5: The Main Programs of the DSPs (MAIN0–MAIN5): What They Are and Where They Run

> **Scope:** SYSTEM-8 (Roland's JX-3P microcode, family A).

> References to "line N" of the wave-bus document point to the
> [S8 wave-bus lab log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md).

State: 2026-10-22 (complete flow in the [S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6.2). Description
of the six main programs of the voice microcode of the JX-3P (Roland PLUG-OUT), with which part of the voice each one
does and on which DSP it runs. Tags: **[measured]** = observed on the device or by audio; **[inferred]** = deduced from
Roland's static listing, without a test on the device; **[open]** = unresolved.

## 1. What each program is

A "main program" is the code that runs every sample (96 kHz) on a DSP and that calls the effect or oscillator slots.
The slot programs (DCO1, DCO2, effects) are loaded separately with `+0x70` and are called by the main program through
the `22nn` mailbox (see [Vol. VI ch. 6](../vol6_reference_designs_applications/custom_osc.md) §2).

| program | bytes | words (execution order) | function [inferred unless stated] |
|---|---|---|---|
| MAIN0 | 0x13CA | 2533 | envelopes ENV1 and ENV2, LFO (constants of the VST's render in [NOTES.md](../vol7_cleanroom_evidence_archive/NOTES.md) "ISA del DSP"), pitch of the voice; its outputs go via MXM (`8405`–`8416`) to DSP2 |
| MAIN1 | 0x139A | 2509 | the same as MAIN0 for the second voice of the pair (A/B with other addresses) |
| MAIN2 | 0x0814 | — | DSP2 (oscillators): forwards what it reads from MXM (`8005`…) to its Z `8405`…; writes the sync and metal of DCO2 (`[2209]`, `[220a]`, `[220b]`) [measured by audio] |
| MAIN3 | 0x07E2 | — | DSP3: like MAIN2, for the other pair of voices of the slave [inferred] |
| MAIN4 | 0x1436 | — | DSP4 (filter and VCA): copies `8005`–`8016` to `[220E]`–`[2219]`; the amplitude envelope `[029D]`; **writes the output to IOMD** `6020`–`6024` and `6060`–`6066` (unconditionally, at the end) [measured, [S8 wave-bus log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md) §3] |
| MAIN5 | 0x1398 | — | DSP5: filter/VCA of voices 2 and 3; hands them to DSP4 through the pair mailbox `A400`–`A403`, which MAIN4 adds to the output [listing, [S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6]; it also copies to IOMD `60B0`–`60B7` (silent [measured]) |

The sizes and the words of MAIN0/MAIN1 were checked with the SHA-256 of `engine/programs.py` (they match the JX-3P
PLUG-OUT 1.01, `plugins/JX-3P/Script/Code8_1.Dat`).

## 2. Where each one runs

A slave BMC has six DSPs (DSP0..DSP5) that share IOMD and MXM with their neighbours. The split of the voice is:

| DSP | main program | oscillator slot | effect slot | role |
|---|---|---|---|---|
| DSP0 | MAIN0 | — | — | envelopes, LFO, pitch (in the listing) |
| DSP1 | MAIN1 | — | — | envelopes, LFO (second pair of voices) |
| DSP2 | MAIN2 | DCO1 `0x6000`, DCO2 `0xB000` | — | oscillators, voices A and B |
| DSP3 | MAIN3 | DCO1 `0x6000`, DCO2 `0xB000` | — | oscillators, voices A and B |
| DSP4 | MAIN4 | effect at `0xB000` (PLUG-OUT) | — | filter and VCA, output to IOMD |
| DSP5 | MAIN5 | — | — | voices 2 and 3 → DSP4 through `A400`–`A403`; copy to IOMD `60B0`–`60B7` |

Source: [Vol. I ch. 1](../vol1_system_architecture/soc_overview.md) (table of slaves) and [S8 wave-bus log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md) §2–§3 (listings of MAIN4/MAIN5).

Both slaves run the six main programs: MAIN0 on DSP0 and MAIN1 on DSP1 of **each** slave (local voices 0–1 and 2–3)
[code `e_load`; measured with C-SAWPOLY and C-V4, [S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6].

**What is not resolved:**
- **[open]** The complete path of the amplitude envelope: ENV2 of MAIN0 → DSP2 → DSP4 (`[0261]` → MXM `840A` →
  `[029D]`), with a contradiction pointed out in the [S8 wave-bus log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md) §4k-ter.
- **[open]** Why only some output words reach the audio ([S8 wave-bus log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md) §5).

## 3. Envelopes and LFO inside MAIN0/MAIN1

Location in the listing of MAIN0 (`dsp/dump_main.py MAIN0`, instruction numbering of that listing):
- ENV1: attack `C02F`, decay/sustain `C030`/`C031`, release `C032`, state `[024E]`, lines 233–249.
- ENV2: block at lines 289–305; attack `C033`, output in `r0` stored at `[025B]` and `[0261]` (lines 46–48); `[0261]`
  goes out through MXM `840A` (line 618).
- LFO: rate `A028` ← `0x3EF`/`0x6CF`, shape `A02C` ← `0x6CD`, delay `A02D` ← `0x6CE` ([NOTES.md](../vol7_cleanroom_evidence_archive/NOTES.md) "ISA del DSP").
- Outputs of envelopes and LFO to MXM `8405`–`8416` (lines 613–636).

Experiments ([S8 wave-bus log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md) §4k–§4p): with env2 = 0 the slow attack disappears even with env1 = 255, and
the other way round it comes back; the slow attack is given by ENV2 [measured, both directions].

## 4. How to reproduce the listing

```
cd examples/from_scratch/engine && python3 -c "..."   # extracts main0/main1 from work/jup8/region1.bin or from the
                                                       # unpacked region1.bin of plugins/JX-3P
cd examples/from_scratch/re && python3 dspdis.py main0.bin > main0.dis
```

`engine/programs.py` checks the SHA-256 of each program before using it, so the listing comes from the same binary that
is installed. `re/dspmc.py` `words()` gives the words in execution order (upper half first).

## 5. What each thing touches when changing a program

- `MC_TEST=main0=file` (and `main1=`, …) replaces the program in `engine/programs.py`; the registered size follows the
  file. A program longer than the registered one does not run whole (`engine/build.sh`).
- The user's current rule (2026-10-21): each component keeps the DSP and the slot that Roland assigned to it
  (oscillator in DSP2/3, LFO and envelope in DSP0/1, filter and VCA in DSP4/5); the program is replaced in its same DSP
  and slot, and it is not moved or merged with another function ([S8 wave-bus log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md) §4g). An attempt to disable
  ENV/LFO without touching MAIN0/MAIN1 (leaving the code, not using it) comes first; replacing MAIN0/MAIN1 is the next
  step, not the first.

> Paths `engine/…`, `re/…`, `dsp/…` are relative to `examples/from_scratch/`.
