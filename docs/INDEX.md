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
| [DSP_ARM_SIDE.md](DSP_ARM_SIDE.md) | DSP ports, banks and the ARM-side configuration (static analysis). |
| [SLAVE_DSP_READBACK.md](SLAVE_DSP_READBACK.md) | Reading the memory of the slave DSPs from the host: what is verified, the open blockers, and the plan. Spanish. |
| [MIDI_SYSEX.md](MIDI_SYSEX.md) | MIDI and SysEx routing between the host, the master and the slaves: where a message is received, filtered and echoed, with the tests done and what is still open. Spanish. |
| [DSP_MAIN_PROGRAMS.md](DSP_MAIN_PROGRAMS.md) | What the six main programs MAIN0–MAIN5 of the voice do, where each one runs, and what is still open. Spanish. |
| [DSP_USOS.md](DSP_USOS.md) | What the operations found in the microcode do. Spanish. |
| [REVIEW.md](REVIEW.md) | Review, fixes and tests of the assembler and the disassembler (`asm/`). |

## Own PLUG-OUT (from scratch)

| Document | What it covers |
|---|---|
| [FROM_SCRATCH.md](FROM_SCRATCH.md) | Plug-out propio desde cero (C-V4, ocho voces medidas): arquitectura, flujo de la voz, microcódigo (`.s`/`.hex`, por qué Python genera ensamblador), motor ARM, región 0, build, instalación, emulador, mapa de ficheros y pendientes. Sustituye a `MINI_PLUGOUT.md`. Spanish. |
| [VOICE_ALLOCATION.md](VOICE_ALLOCATION.md) | Asignación de voces de Roland por DSP y flujo de datos del slave (familia A), por qué C-V3 no suena (preguntas A–H con pruebas en el emulador `examples/from_scratch/emu/`), candidato v4, plan de medida y comprobación de firmas. Spanish. |
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
| [WAVE_BUS.md](WAVE_BUS.md) | Ruta de señal de los plug-outs: flujo por DSP, MXM, banco 5, qué carga Roland al arrancar, mediciones de audio y diagnóstico del mudo (2026-10-21). Único documento de este tema. |

