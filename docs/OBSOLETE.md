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

## Frozen work records of Vol. VII (`NOTES.md`, `PLAN.md`, `REVIEW.md`, `ASM_PENDIENTES.md`)

Dated records of the work, in the order it happened (`NOTES.md` and `ASM_PENDIENTES.md` in Spanish). Deleted on
2026-11-10; the last version is in the git history (commit `4632d9a`). No tool, test or chapter refers to them. Content checked against the chapters section by section on 2026-11-10; what was only in
these files was added to the places named below.

### `vol7_cleanroom_evidence_archive/NOTES.md`

| section of `NOTES.md` | content now in |
|---|---|
| "Región 0", "Región 0 en C", "Qué sigue dependiendo de Roland" | [examples/from_scratch/model/README.md](../examples/from_scratch/model/README.md) (SYNINFO, descriptor, parameter list, slot memory, assigner vtable, kernel table `0x01000400`); [Vol. VI ch. 1](vol6_reference_designs_applications/c_v4_system8_reference.md) §5 |
| "Región 1", "Engine del JX-3P emulado" | [Vol. IV ch. 3](vol4_firmware_container_protocol/os_service_table.md) §2 and §2.1 (service table, what the slave calls in the engine, descriptors); [Vol. III ch. 1](vol3_voice_allocation_scheduling/system8_voice_allocation.md) §4 (words per voice: pitch, gate, velocity) |
| "Microcódigo DSP" | [Vol. IV ch. 2](vol4_firmware_container_protocol/plugout_virtual_machine.md) §7 "DSP microcode in the engine"; [Vol. III ch. 3](vol3_voice_allocation_scheduling/slot_program_model.md) |
| "C-SAW", "Próximos pasos (2026-10-14)", "Modelo de parámetros propio" | [examples/from_scratch/engine/README.md](../examples/from_scratch/engine/README.md) (parameter model, part ids `0x2DB`–`0x2E0`, float parameters `0x74D` / `0x7E4`, six-word descriptors, tests) |
| "Motor DSP en el VST3", "LFO y envolventes según el render del VST", "Osciladores: VST vs microcódigo" | [examples/from_scratch/re/README.md](../examples/from_scratch/re/README.md) "What the model computes"; [Vol. III ch. 5](vol3_voice_allocation_scheduling/dsp_main_programs.md) (constants in MAIN0 / MAIN1); [Vol. III ch. 7](vol3_voice_allocation_scheduling/roland_oscillators.md) |
| "ISA del DSP: primeros hallazgos", "Formato de instrucción" (both), "ALU `0902 w`", "Unidades, plantillas y predicación", "Predicación por tiempo", "Contexto limpio", "SAW de Roland al 90 %", "Puertos X9bY", "Banderas `X9bY`", "Próximos pasos del ISA" | [Vol. II ch. 4](vol2_dsp_core_isa/instruction_set_reference.md) (the specification), [ch. 3](vol2_dsp_core_isa/instruction_word_encoding.md), [ch. 7](vol2_dsp_core_isa/isa_overview.md), [ch. 5](vol2_dsp_core_isa/sfu_and_accelerators.md), [ch. 8](vol2_dsp_core_isa/isa_discrepancies.md) |
| "Formato numérico del DSP" | [Vol. II ch. 2](vol2_dsp_core_isa/data_types_arithmetic.md); memory map of the DSP blocks: [Vol. I ch. 2](vol1_system_architecture/memory_map.md), [Vol. II ch. 6](vol2_dsp_core_isa/memory_banks_dma.md) |
| "Pruebas en el equipo sobre los DSP del maestro", "Peek exacto en los DSP del maestro", "Pokes de microcódigo en vivo por MIDI", "Envío rápido de tablas" | [Vol. I ch. 1](vol1_system_architecture/soc_overview.md) (what runs where, the lab tools); [Vol. VI ch. 6](vol6_reference_designs_applications/custom_osc.md) §7.1; [Vol. II ch. 11](vol2_dsp_core_isa/slave_dsp_readback.md); [Vol. IV ch. 3](vol4_firmware_container_protocol/os_service_table.md) §2.1 (parameter transport to the slave) |
| "ISA del DSP medido en el equipo por audio USB" | bench: [examples/from_scratch/engine/README.md](../examples/from_scratch/engine/README.md) "ISA test bench"; results: Vol. II ch. 4 |
| "SAW propio", "Plugin de laboratorio", "Wavetables desde WAV y presupuesto de tiempo", "Wavetable verdadera en el banco 5" | [Vol. VI ch. 6](vol6_reference_designs_applications/custom_osc.md); budget: [Vol. III ch. 4](vol3_voice_allocation_scheduling/execution_timing_budgets.md) |
| "Verificación de lo pendiente" (bank-5 and bank-0 indexed access), "Tomas de retardo `{8+d}f6c` / `Xa9Y` y bancos 2/4/5" | [Vol. II ch. 9](vol2_dsp_core_isa/operation_uses.md), [ch. 6](vol2_dsp_core_isa/memory_banks_dma.md), [ch. 1](vol2_dsp_core_isa/core_architecture.md); buses: [Vol. I ch. 3](vol1_system_architecture/bus_interconnect.md) Part A |
| "Muestras largas" | [Vol. II ch. 1](vol2_dsp_core_isa/core_architecture.md) (delay line, EDM port); [Vol. VI ch. 7](vol6_reference_designs_applications/mutable_memory.md); [Vol. I ch. 2](vol1_system_architecture/memory_map.md) |
| "Posibles próximos pasos: emulador" | [Vol. V ch. 5](vol5_toolchain_compiler_sdk/emulator.md) |
| "C-V4" | [Vol. VI ch. 1](vol6_reference_designs_applications/c_v4_system8_reference.md) |
| "Falta", "Próximos pasos", "Pendiente", device state at the end of a session | plans and states of their date; nothing to keep |

Statements of `NOTES.md` that the chapters state differently (the chapters hold the final value; most are corrected
further down in the file itself):
* "El engine NO carga programa DSP": it does; the sound is DSP microcode carried by the engine.
* Assigner vtable entries 14 / 15 as `setVoices` / `setMode`: they are `init(voices)` / `set_voices(voices)`
  (`model.c`, which runs on the device).
* "El `get` del maestro no es una tabla en `0x01000400`": it is the kernel's table (dump `kernel_01000000.bin`).
* The DSP "runs once per block of 1024 samples (46.9/s)": every program runs at 96 kHz; the data memory is a ring.
* Memory "shared / not reproducible" between runs, and the trace that filled the bank: the ring, and the load latency
  of one instruction.
* The 16 conditions "are not flags" but fixed context bits; `X9bY` "not decoded" / "no se confirma": `X9bY` sets the
  flags of group X − 8 from the result of unit Y, with a latency of one instruction, and they persist.
* The condition field of a C word as 4 bits: it has 5 (conditions 0–31).
* "Two A ops with an immediate make the instruction illegal": the ops of an instruction share the immediate, read
  at byte Y of the slot area.
* r2 and r3 "are 1.0 at the start of the slot": the registers are shared between the slots of a DSP.
* Hidden values h4 / h5 of op 0: the delay registers (base of `f6c`, destination of the tap results).
* "MAC op2 = 011" (0.0898): in class 0x098 that word is a C word.
* "No indexed reads" (wavetable only as immediates): indexed loads exist (`X28Y` bank 0, `Xda`/`Xdb` bank 5).
* Table "Confirmado" of the audio bench (`04c1` copy, `0081` multiply, `0541`): read with uninitialised registers;
  the exact table of unit A is the one of Vol. II ch. 4.
* Corpus coverage figures (90.4 %, 93.6 %, 98.4 %, 98.6 %): of their date; current figure in `asm/tests/baseline.json`.

### `vol7_cleanroom_evidence_archive/PLAN.md`

| part of `PLAN.md` | content now in |
|---|---|
| plan, assessment, status of S8LAB | [examples/sys8_lab/README.md](../examples/sys8_lab/README.md) |
| steps 1–2 (OS strings, name pointer array, COLOR id `0x6D5`) | [Vol. IV ch. 6](vol4_firmware_container_protocol/base_firmware.md) §8.4 (parameter-name pointer array); `examples/sys8_lab/README.md` |
| steps 3–4 (patch cords, EXTEND matrix, host LFO on COLOR) | `examples/sys8_lab/README.md` (tests and layout facts); [Vol. IV ch. 9](vol4_firmware_container_protocol/plugout_vs_base.md) §8 (patch matrix) |
| steps 5–8 (DSP program descriptors of the SYSTEM-8 engine, main programs, JUPITER-8 relation, candidate SAW) | [Vol. IV ch. 8](vol4_firmware_container_protocol/base_engine.md) §5.4 |

Statements of `PLAN.md` that the chapters state differently:
* Descriptor sizes "in words", "83 distinct programs", and the family sizes derived from them: the size field is in
  bytes and the 85 oscillator records point at 79 distinct programs (corrected in step 7 of the file; table in
  Vol. IV ch. 8 §5.4).
* "COLOR selects a different program" (step 6) is a reading of the table, not a measurement; which record serves
  which (WAVE, COLOR) is not established.
* OS string addresses of step 1 with nine hex digits (`0x6012EEC14`, `0x6012EEEBC`) are typing errors of the record.

### `vol7_cleanroom_evidence_archive/REVIEW.md` and `ASM_PENDIENTES.md`

| file | content now in |
|---|---|
| `REVIEW.md` (review of `asm/` of 2026-10-20: defects fixed, tests, open items, device follow-up) | rules: [Vol. V ch. 4](vol5_toolchain_compiler_sdk/assembler_disassembler_tools.md) §2–§3; open items: [Vol. II ch. 8](vol2_dsp_core_isa/isa_discrepancies.md) (D4, D6, D8, D10); one regression test per defect: `asm/tests/test_regressions.py`; current figures: `asm/tests/baseline.json` |
| `ASM_PENDIENTES.md` (13 shortcomings of `asm/asm.py`, 2026-10-21) | [Vol. V ch. 4](vol5_toolchain_compiler_sdk/assembler_disassembler_tools.md) §2.1: items 1–8 re-run on 2026-11-10 with the result and the form to write; items 9–13 (load latency, ring, budget, simulator, equivalence test) are the scheduler, the build pipeline and the emulator (§4, §5, ch. 5) |

The figures of `REVIEW.md` ("Result", "Tests") are those of 2026-10-20.

