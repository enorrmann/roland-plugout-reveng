# Obsolete Documents

The files below are **obsolete**: their content is in the chapters named in the right-hand column, which are the only
maintained source. The files are kept unchanged in place as raw data (measurements, listings, dates); nothing in
them is updated, and where they disagree with the chapters, the chapters are right. Do not cite them from new text.

## SYSTEM-1 documents (Spanish), integrated into the volumes

| obsolete file | content now in |
|---|---|
| `system-1/ARCHITECTURE.md` | [Vol. I ch. 1](vol1_system_architecture/soc_overview.md) §6 (§1 → §6.1, §1.0 → §6.2, §1.1 → §6.3, §2 → §6.4, §3 → §6.5, §4 → §6.6, §5 → §6.7) |
| `system-1/WAVE_BUS.md` | [Vol. I ch. 3](vol1_system_architecture/bus_interconnect.md) Part B (§1 → §11, §2 → §12, §3 → §13, §4.n → §14.n, §5.n → §15.n, §6 → §17, §7 → §18, §8 → §19, §9 → evidence at the end of §19) |
| `system-1/FX_CHAIN.md` | [Vol. I ch. 3](vol1_system_architecture/bus_interconnect.md) §16 (§1 → §16.1, §2 → §16.2, §3 → §16.3, §4 and §5b → §16.4, §5 → §16.5, §6 → §16.7, §7 → §16.6) |
| `system-1/DSP_BANKS.md` | [Vol. II ch. 6](vol2_dsp_core_isa/memory_banks_dma.md) §7 (§n → §7.n) |
| `system-1/FIRMWARE.md` | [Vol. IV ch. 6](vol4_firmware_container_protocol/base_firmware.md) §9 (§n → §9.n) |
| `system-1/VOICE_ALLOCATION.md` | [Vol. III ch. 2](vol3_voice_allocation_scheduling/system1_voice_allocation.md) (same section numbers) |
| `system-1/VOCES_6.md` | [Vol. III ch. 9](vol3_voice_allocation_scheduling/system1_polyphony_studies.md) §1 (§n → §1.n) |
| `system-1/SH101_POLY.md` | [Vol. III ch. 9](vol3_voice_allocation_scheduling/system1_polyphony_studies.md) §2 (§n → §2.n) |
| `system-1/DSP_FIT.md` | [Vol. III ch. 9](vol3_voice_allocation_scheduling/system1_polyphony_studies.md) §3 (§n → §3.n) |

`system-1/README.md` and the scripts of `system-1/` (`*.py`) stay in use.

Values that the chapters state differently from these files (the chapters hold the final value):
* SYSTEM-1 slave DSP A budget: 703 instructions per sample run, 826 hang (the files say "between 682 and 826").
* The slave has two DSPs, A (`FC30`) and B (`FC34`); `FC38`/`FC3C` are data queues (`SH101_POLY.md` §2 reads them as
  queues of DSP2/DSP3, and its 4-voice plan uses DSP2–DSP5, which do not exist).
* The panel effects sound when the PLUG-OUT's master-B program does the send and the return (`FX_CHAIN.md` was
  written before the result of step 13).
* USB audio capture exists on the SYSTEM-1 (`tools/s1_capture.py`); `ARCHITECTURE.md` §5 says it does not.
* Slave DSP A's bank 0 wraps at `0x0400`; `DSP_BANKS.md` §1 gives `0000`–`07FF` without the wrap.

## Frozen lab logs (version 1 of the wave-bus and voice-allocation documents)

| obsolete file | content now in |
|---|---|
| `vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md` | [Vol. I ch. 3](vol1_system_architecture/bus_interconnect.md) Part A |
| `vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md` | [Vol. III ch. 1](vol3_voice_allocation_scheduling/system8_voice_allocation.md) |
| `vol7_cleanroom_evidence_archive/historical_lab_logs/s1_wave_bus_log.md` | [Vol. I ch. 3](vol1_system_architecture/bus_interconnect.md) Part B |
| `vol7_cleanroom_evidence_archive/historical_lab_logs/s1_voice_allocation_log.md` | [Vol. III ch. 2](vol3_voice_allocation_scheduling/system1_voice_allocation.md) |

The chapters cite a log section ("log §n") as the measurement behind a fact; the log is the raw record of that
measurement, not a description of the current state.
