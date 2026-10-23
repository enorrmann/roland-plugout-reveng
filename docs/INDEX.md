# Documentation index

Every document of the repository lives in this folder. Names keep their original file name where it was unique;
per-folder READMEs are named `<folder>_README.md`. Links inside the documents point to the other files here.
Status of each item is in the document itself; the scope of the project is in the root [`README.md`](../README.md).

## Start here

| Document | What it covers |
|---|---|
| [ARCHITECTURE.md](ARCHITECTURE.md) | Hardware of the SYSTEM-8 (master and slave BMCs, DSPs, panel MCU) and which code runs where. Spanish. |
| [PROTOCOL.md](PROTOCOL.md) | The USB-MIDI install protocol of the PLUG-OUT plugins, with code addresses. Used by `tools/plugout_tool.py`. |
| [CUSTOM_PLUGOUTS.md](CUSTOM_PLUGOUTS.md) | Firmware container (`Code*.Dat`), slots, relocation, the OS interface, the parameter model, and the changes verified on hardware. |
| [PLUGOUT_VS_BASE.md](PLUGOUT_VS_BASE.md) | Whether PLUG-OUTs add processing or only configure the base system. |

## Base firmware (SYSTEM-8 updater and OS)

| Document | What it covers |
|---|---|
| [BASE_FIRMWARE.md](BASE_FIRMWARE.md) | `SYSTEM8_UP.BIN`: container, regions, the OS (`App1_Main`), the `tools/sys8fw.py` toolchain, and the RE results. |
| [VERSIONS.md](VERSIONS.md) | Updater versions 1.30, 1.31 and 1.32: what changed in each image. |
| [BASE_ENGINE.md](BASE_ENGINE.md) | What the decoded base engine (the region code of the stock plugouts) contains. |
| [base-engine_README.md](base-engine_README.md) | The `base-engine/` folder: decoded base-engine artefacts and how they were made. |
| [data-formats.md](data-formats.md) | Container, flash image and region formats (clean-room specification set, see below). |
| [report_index.md](report_index.md) | Index of the per-image reports made by `sys8fw report`. |
| [p1y01_m0c0a_up.md](p1y01_m0c0a_up.md) | Report on the OS image (`App1_Main`): not decoded in the report. |
| [p1y01_p0m.region0.md](p1y01_p0m.region0.md), [p1y01_p0s.region0.md](p1y01_p0s.region0.md), [p1y01_pl1m.region0.md](p1y01_pl1m.region0.md), [p1y01_pl1m.region1.md](p1y01_pl1m.region1.md), [p1y01_pl1s.region0.md](p1y01_pl1s.region0.md), [p1y01_pl1s.region1.md](p1y01_pl1s.region1.md), [p1y01_pl2m.region0.md](p1y01_pl2m.region0.md), [p1y01_pl2m.region1.md](p1y01_pl2m.region1.md) | Generated reports per region of each image in the updater (classes, presets, labels, virtual methods). |

## DSP (the chips that make the sound)

| Document | What it covers |
|---|---|
| [DSP_HARDWARE.md](DSP_HARDWARE.md) | Technical reference of the DSP chip: arithmetic, seed ROMs, timing. Spanish. |
| [DSP_ISA.md](DSP_ISA.md) | Instruction set of the DSP as measured on the device. |
| [DSP_ISA_SPEC.md](DSP_ISA_SPEC.md) | Specification for assembler and compiler writers (used by `asm/`). |
| [DSP_ISA_DISCREPANCIES.md](DSP_ISA_DISCREPANCIES.md) | Where the measured ISA, the assembler and the disassembler disagree, and what is still open. |
| [DSP_PROGRAM_MEMORY.md](DSP_PROGRAM_MEMORY.md) | Program memory of a DSP on SYSTEM-8 and SYSTEM-1: main program, slots / voice sections, sizes, who enables what, usage variants (main only, sections, overrunning a zone: open). Spanish. |
| [DSP_ARM_SIDE.md](DSP_ARM_SIDE.md) | DSP ports, banks and the ARM-side configuration (static analysis). |
| [SLAVE_DSP_READBACK.md](SLAVE_DSP_READBACK.md) | Reading the memory of the slave DSPs from the host: what is verified, the open blockers, and the plan. Spanish. |
| [MIDI_SYSEX.md](MIDI_SYSEX.md) | MIDI and SysEx routing between the host, the master and the slaves: where a message is received, filtered and echoed, with the tests done and what is still open. Spanish. |
| [DSP_MAIN_PROGRAMS.md](DSP_MAIN_PROGRAMS.md) | What the six main programs MAIN0–MAIN5 of the voice do, where each one runs, and what is still open. Spanish. |
| [DSP_USOS.md](DSP_USOS.md) | What the operations found in the microcode do. Spanish. |
| [REVIEW.md](REVIEW.md) | Review, fixes and tests of the assembler and the disassembler (`asm/`). |

## Own PLUG-OUT (from scratch)

| Document | What it covers |
|---|---|
| [EMULATOR.md](EMULATOR.md) | Emulador de un PLUG-OUT de Roland completo: qué existe (núcleo del DSP en C, código nativo e hilos: un slave del JX-3P en tiempo real), qué falta (servicios del OS, banco 6, fidelidad del ISA, línea de retardo, maestro), orden de trabajo y estado para retomar (§5). Spanish. |
| [FROM_SCRATCH.md](FROM_SCRATCH.md) | Plug-out propio desde cero (C-V4, ocho voces medidas): arquitectura, flujo de la voz, microcódigo (`.s`/`.hex`, por qué Python genera ensamblador), motor ARM, región 0, build, instalación, emulador, mapa de ficheros y pendientes. Sustituye a `MINI_PLUGOUT.md`. Spanish. |
| [VOICE_ALLOCATION.md](VOICE_ALLOCATION.md) | **Technical specification** (v2.0, 2026-10-22): voice ids and the path of a note, per-DSP split of Roland family A, bank-5 words the ARM writes per voice, DCO slot interface with an oscillator example, family B, C-V4 as reference implementation, voice limit. Code examples checked with `tools/doccheck.py`. Lab notebook (v1, frozen): [log/VOICE_ALLOCATION_LOG.md](log/VOICE_ALLOCATION_LOG.md). Spanish. |
| [NOTES.md](NOTES.md) | Working notes of the from-scratch PLUG-OUT: interfaces found, what runs on the device, next steps. Spanish, unfinished. |
| [CUSTOM_OSC.md](CUSTOM_OSC.md) | Own DSP microcode oscillator (C-SAW): description and the audio checks. |
| [SCRIPTS.md](SCRIPTS.md) | What each script in `examples/from_scratch/re/` does and what it needs. |
| [examples_from_scratch_engine_README.md](examples_from_scratch_engine_README.md) | The C engine of the from-scratch PLUG-OUT (region 1) and its build. |
| [examples_from_scratch_model_README.md](examples_from_scratch_model_README.md) | The own region 0 (parameter model) in C. |
| [examples_from_scratch_re_ghidra_README.md](examples_from_scratch_re_ghidra_README.md) | Static analysis of the JX-3P VST3 with Ghidra. |

## Examples (one folder per experiment)

| Document | Experiment |
|---|---|
| [examples_sys8_lab_README.md](examples_sys8_lab_README.md) | S8LAB: the SYSTEM-8 image in PLG1 used as a lab, with its tests. |
| [PLAN.md](PLAN.md) | Plan and results of the S8LAB lab (`examples/sys8_lab/`). |
| [examples_wt_live_README.md](examples_wt_live_README.md) | C-WTLIVE: live wavetable PLUG-OUT (slot 3). |
| [examples_wt_morph_README.md](examples_wt_morph_README.md) | C-MORPH: one wavetable position that morphs with an LFO. |
| [OPTIONS.md](OPTIONS.md) | The options considered for the wavetable space of the morph example. |
| [examples_wt_pwm_README.md](examples_wt_pwm_README.md) | C-PWM: wavetable PLUG-OUT with a WAV inside. |
| [examples_sd_txt_README.md](examples_sd_txt_README.md) | SD-TXT: read a file from the SD card in a region 0 init hook. |

The other example folders have no README: `examples/cave_tone`, `examples/mem_name`, `examples/mem_names`,
`examples/mem_scan`, `examples/midi_send`, `examples/preset_fill`. Each one is a hook or capture experiment; the
root README lists them with their status.

## SYSTEM-1 (the device)

Kept together in the `system-1/` folder (documents and scripts), at the user's request (2026-10-22).

| Document | What it covers |
|---|---|
| [../system-1/README.md](../system-1/README.md) | Index of the folder, scripts and status. Spanish. |
| [../system-1/FIRMWARE.md](../system-1/FIRMWARE.md) | SYSTEM-1 OS images (updater 1.30 unencrypted, plug-in `Code1.Dat` 1.20), extraction, known addresses. Spanish. |
| [../system-1/VOCES_6.md](../system-1/VOCES_6.md) | 6-voice analysis: slave DSP voice programs and budget, master→slave ids, allocator sites, plan. Spanish. |
| [../system-1/SH101_POLY.md](../system-1/SH101_POLY.md) | SH-101 PLUG-OUT on SYSTEM-1 to 4 voices: its DSP programs, the slave's program-loading services, plan. Spanish. |
| [../system-1/ARCHITECTURE.md](../system-1/ARCHITECTURE.md) | SYSTEM-1 architecture (counterpart of ARCHITECTURE.md): chips, what code runs where, flows, DSP windows, tools. Spanish. |
| [../system-1/VOICE_ALLOCATION.md](../system-1/VOICE_ALLOCATION.md) | SYSTEM-1 voice allocation, technical specification (v2.0): voice ids, assigner/engine code, stock split, program memory (main, sections, `FC44`, program sets), Roland and own PLUG-OUTs (SH-101, SH-2, PROMARS, C-S1V4/V8, C-S1JX), voice count. Notebook: `system-1/log/`. Spanish. |
| [../system-1/WAVE_BUS.md](../system-1/WAVE_BUS.md) | SYSTEM-1 inter-DSP communication and audio path, technical specification (v2.0): shared bank 5 per chip, `FD6C`/`FCEC` semantics, IOMD links (incl. `6026`–`602D`), PLUG-OUT-mode output chain with the master-B program, frame headers/closes, end-to-end note. Notebook: `system-1/log/`. Spanish. |
| [../system-1/DSP_FIT.md](../system-1/DSP_FIT.md) | Does a SYSTEM-8 PLUG-OUT's microcode (JX-3P) fit the SYSTEM-1's 4 DSPs? `dsp_fit.py`: emulated time per program, inter-DSP links, placement search. 4 voices: no; 2 voices: on paper. Spanish. |
| [examples_system_1_jx_README.md](examples_system_1_jx_README.md) | C-S1JX (`examples/system_1_jx/`): the JX-3P's own DSP microcode (one DCO, 4 voices) relocated onto the SYSTEM-1's four DSPs (`s1jx.py`, `dspreloc.py`), ARM side with the full parameter model (`model_full.py`, the pre-trim `model_spec.py`), start-up checklist, independent tests (`test_s1jx.py`, `test_arm.py`), risks; what was removed from `engine/model_spec.py`. Built and tested without hardware, not installed. Spanish. |
| [examples_system_1_README.md](examples_system_1_README.md) | C-S1V4: the from-scratch C-V4 ported to the SYSTEM-1 (`examples/system_1/`): the SYSTEM-1 PLUG-OUT interface (SYNINFO, vtables, DSP program queues, audio path slave → master), build, test order, open risks, parameter capture (PARAMLOG). Works on the device (notes, chords, volume); 8-voice variant C-S1V8 works too. Spanish. |

## Clean-room specification

A specification of the PLUG-OUT firmware and protocol, written from the evidence without the tools' code (see
`clean-room-boundary.md`). It is a separate track from the tools.

| Document | What it covers |
|---|---|
| [overview.md](overview.md) | Overview of the firmware and the scope of the specification. |
| [cleanroom_architecture.md](cleanroom_architecture.md) | Logical architecture. |
| [functional-spec.md](functional-spec.md) | Functional requirements and acceptance criteria. |
| [protocol-spec.md](protocol-spec.md) | PLUG-OUT firmware transfer over USB-MIDI. |
| [state-machines.md](state-machines.md) | States and transitions of the transfer. |
| [test-plan.md](test-plan.md) | Validation and conformance matrix. |
| [evidence.md](evidence.md) | Register of the evidence behind each statement. |
| [clean-room-boundary.md](clean-room-boundary.md) | Team separation and the authorised material. |
| [cleanroom_test-vectors_README.md](cleanroom_test-vectors_README.md) | Test vectors and their checker (`cleanroom/test-vectors/`). |
| [WAVE_BUS.md](WAVE_BUS.md) | **Technical specification** (v2.0, 2026-10-22): memory model per bank (scope, ring, latency, ARM access), transport mechanisms (bank-0/1 rings, MXM, pair mailbox, bank 5/6 from the ARM, IOMD ownership and audible words) each with semantics, rules and microcode examples, audio path of both slaves, execution order, an end-to-end note trace, design rules. Lab notebook (v1, frozen, all measurements): [log/WAVE_BUS_LOG.md](log/WAVE_BUS_LOG.md). Spanish. |
| [log/WAVE_BUS_LOG.md](log/WAVE_BUS_LOG.md), [log/VOICE_ALLOCATION_LOG.md](log/VOICE_ALLOCATION_LOG.md) | Frozen lab notebooks (v1 of WAVE_BUS / VOICE_ALLOCATION, until 2026-10-22): every test, measurement and correction with its section number; old "§n" references point here. Spanish. |
