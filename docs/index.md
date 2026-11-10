# Documentation Index — Roland SYSTEM-8 / SYSTEM-1 PLUG-OUT Reverse Engineering

This folder holds the documentation of the repository, in seven volumes. The scope and status of the project are in
the root [`README.md`](../README.md); the usage of every tool is in [`TOOLS.md`](TOOLS.md); terms and abbreviations
are in the [glossary](GLOSSARY.md). Documents that are no longer maintained are listed in [OBSOLETE.md](OBSOLETE.md).

**Scope** of each chapter (column "for"): **S8** = SYSTEM-8, **S1** = SYSTEM-1, **both** = common to both devices or
with one part for each (the chapter says which part is which). The DSP instruction set is the same on both devices.

Each fact carries the status stated in its chapter; each chapter gives the legend of its evidence tags. The common
ones are:

| tag | meaning |
|---|---|
| **[M]** | measured on the device |
| **[L]** | read in Roland's microcode, OS or ARM code (static) |
| **[C]** | our code |
| **[E]** | emulator prediction (`examples/from_scratch/emu`) |
| **[I]** | inferred, no direct test |
| **[A]** | open |
| **[sim]**, **[est]**, **[code]** | measured in the emulator, estimated, read in the code |

`dsp` code blocks are checked with `python3 tools/doccheck.py docs/vol*/*.md` (they must assemble).

```
 Vol. I    System architecture        hardware, memory maps, inter-DSP paths, ARM side, MIDI routing, S1 ARM clock
 Vol. II   DSP core and ISA           hardware reference, arithmetic, encoding, instruction set, SFU, banks, native ops
 Vol. III  Voices and scheduling      S8 / S1 voice allocation, program memory, budgets, main programs, optimization
 Vol. IV   Firmware and protocol      containers, PLUG-OUT code and OS interface, service tables, USB install, firmware
 Vol. V    Toolchain and SDK          s1cc, s1dsp.h, sdk/mport, asm/ toolchain, emulator, audio capture
 Vol. VI   Reference designs          C-V4, C-S1V4/V8, CZ-SYSTEM, Mutable ports, wavetables, C-SAW, memory oscillators
 Vol. VII  Clean room and evidence    boundary, clean-room spec, test plan, generated reports, lab logs, notes
```

## 1. Volume I — System architecture

| Chapter | for | What it covers |
|---|---|---|
| [ch. 1 System architecture](vol1_system_architecture/soc_overview.md) | both | §1–§5 SYSTEM-8: master and slave BMCs, DSPs, panel MCU, which code runs where, flows, tools. §6 SYSTEM-1: hardware, capacity of the slave DSPs, no FPU, code per processor, flows, DSP windows, tools. |
| [ch. 2 Memory maps](vol1_system_architecture/memory_map.md) | both | ARM address spaces and DSP windows of both devices, each table marked with its chip. |
| [ch. 3 Inter-DSP communication and audio path](vol1_system_architecture/bus_interconnect.md) | both | Part A SYSTEM-8: memory model per bank, transport mechanisms (rings, MXM, pair mailbox, bank 5/6 from the ARM, IOMD), audio path of both slaves, execution order, a note traced end to end, design rules. Part B SYSTEM-1: shared bank 5, `FD6C`/`FCEC`, IOMD between chips, PLUG-OUT-mode output chain, header/closing rule, the panel effects (delay + reverb), design rules. |
| [ch. 4 The ARM side](vol1_system_architecture/host_control_plane.md) | both | Processors, programs, RTOS, FPU and panel CPU of both devices. |
| [ch. 5 MIDI and SysEx routing](vol1_system_architecture/midi_sysex_routing.md) | S8 | Routing between the host, the master and the slaves: where a message is received, filtered and echoed; tests and open items. |
| [ch. 6 Slave ARM clock](vol1_system_architecture/s1_arm_clock.md) | S1 | A 1 ms tick by hooking the OS cyclic handler client (`0x20002BFC`); writing DSP parameters from it; telemetry through audio. |

## 2. Volume II — DSP core and ISA

| Chapter | for | What it covers |
|---|---|---|
| [ch. 1 DSP core hardware reference](vol2_dsp_core_isa/core_architecture.md) | both | Arithmetic, units, seed ROMs, banks, access rules, timing; measured on the SYSTEM-8, with the SYSTEM-1 values where they differ. |
| [ch. 2 Data types and arithmetic](vol2_dsp_core_isa/data_types_arithmetic.md) | both | The RAW number format and the bit-exact arithmetic. |
| [ch. 3 Instruction encoding](vol2_dsp_core_isa/instruction_word_encoding.md) | both | ISA specification §1–§5: word format, classes, templates, slot array, immediates, word formats. |
| [ch. 4 Instruction set reference](vol2_dsp_core_isa/instruction_set_reference.md) | both | ISA specification §6–§15: ALU / MAC / SFU semantics, predication, transfers and memory, execution semantics, examples, template table, open items, mnemonic index of `asm/isa.json`. |
| [ch. 5 SFU, seed ROMs and phase operations](vol2_dsp_core_isa/sfu_and_accelerators.md) | both | Special-function operations and their tables. |
| [ch. 6 Memory banks, ports and ARM-side configuration](vol2_dsp_core_isa/memory_banks_dma.md) | both | §1–§6 SYSTEM-8: ARM-side static analysis, corpus statistics. §7 SYSTEM-1: banks per DSP, ranges used by the OS and Roland's PLUG-OUTs, rules, delay-line measurement. |
| [ch. 7 ISA overview (measured)](vol2_dsp_core_isa/isa_overview.md) | both | The instruction set as measured on the device, in summary form. |
| [ch. 8 ISA discrepancies](vol2_dsp_core_isa/isa_discrepancies.md) | both | Where the measured ISA, the assembler and the disassembler disagree, with the probe that would close each. |
| [ch. 9 What the decoded operations are good for](vol2_dsp_core_isa/operation_uses.md) | both | Uses of each operation (delay line, indexed transfers, hidden registers, buses, flags). |
| [ch. 10 Native ops](vol2_dsp_core_isa/native_ops.md) | both | Status per op (SYSTEM-8, emulator, s1cc, host, SYSTEM-1) and the plan to use them all. |
| [ch. 11 Reading the memory of the slave DSPs](vol2_dsp_core_isa/slave_dsp_readback.md) | S8 | What is verified, the open blockers, and the plan. |

## 3. Volume III — Voice allocation and scheduling

| Chapter | for | What it covers |
|---|---|---|
| [ch. 1 SYSTEM-8 voice allocation](vol3_voice_allocation_scheduling/system8_voice_allocation.md) | S8 | Voice ids and the path of a note, split per DSP of Roland's family A, bank-5 words per voice, DCO slot interface, family B, C-V4 as reference, voice limit. |
| [ch. 2 SYSTEM-1 voice allocation](vol3_voice_allocation_scheduling/system1_voice_allocation.md) | S1 | Voice ids, assigner and engine code, stock split, program memory, Roland's and our PLUG-OUTs, voice count. |
| [ch. 3 DSP program memory](vol3_voice_allocation_scheduling/slot_program_model.md) | both | Main program, slots (S8) / sections (S1), sizes, who enables what, usage variants. |
| [ch. 4 Execution time and instruction budgets](vol3_voice_allocation_scheduling/execution_timing_budgets.md) | both | Measured budgets per DSP and device. |
| [ch. 5 Main programs MAIN0–MAIN5](vol3_voice_allocation_scheduling/dsp_main_programs.md) | S8 | What the six main programs of the JX-3P voice do and where each runs. |
| [ch. 6 Optimized microcode — mandatory rules](vol3_voice_allocation_scheduling/microcode_optimization.md) | both | Phase ops, packing, fusion, no repeated loads, bit-exact validation; `tools/check_optimized.py`. |
| [ch. 7 Roland's oscillators](vol3_voice_allocation_scheduling/roland_oscillators.md) | both | How Roland's oscillators of both devices compute the waveforms and which instructions save cycles. |
| [ch. 8 SYSTEM-1 optimizations](vol3_voice_allocation_scheduling/s1_optimizations.md) | S1 | Catalogue of measured savings of SYSTEM-S1CC. |
| [ch. 9 SYSTEM-1 polyphony studies](vol3_voice_allocation_scheduling/system1_polyphony_studies.md) | S1 | Six voices for the stock synth (OS patch), a polyphonic SH-101, the SYSTEM-8 JX-3P microcode on the SYSTEM-1 (`dsp_fit.py`, `s1_progs.py`). |

## 4. Volume IV — Firmware, containers and protocol

| Chapter | for | What it covers |
|---|---|---|
| [ch. 1 Binary container formats](vol4_firmware_container_protocol/container_formats.md) | both | `Code*.Dat`, flash image, regions, LZSS, `SYSTEM8_UP.BIN`, the `App1_Main` cipher. |
| [ch. 2 PLUG-OUT code and the OS interface](vol4_firmware_container_protocol/plugout_virtual_machine.md) | S8 | The JX-3P engine, the DSP microcode in the engine, writing new code, reading OS memory at run time, risks. |
| [ch. 3 OS service tables](vol4_firmware_container_protocol/os_service_table.md) | both | The OS services PLUG-OUT code calls (S8 `0x0113FC00`, S1 `0x0100FC00`), with their sources. |
| [ch. 4 USB install protocol](vol4_firmware_container_protocol/usb_sysex_protocol_spec.md) | both | The USB-MIDI install protocol, with code addresses; used by `tools/plugout_tool.py`. |
| [ch. 5 Web installer](vol4_firmware_container_protocol/web_installer.md) | both | The browser installer in `web/`: use, blocking checks, port handling, tests. |
| [ch. 6 Base firmware](vol4_firmware_container_protocol/base_firmware.md) | both | §1–§8 SYSTEM-8 (`SYSTEM8_UP.BIN`, the OS, `sys8fw.py`, OS dumps, sending MIDI from a PLUG-OUT, OS anatomy). §9 SYSTEM-1 (`SYSTEM1_UPD.BIN`, images, addresses, MIDI send). |
| [ch. 7 Updater versions](vol4_firmware_container_protocol/firmware_versions.md) | S8 | Updater versions 1.30, 1.31 and 1.32. |
| [ch. 8 Decoded base engine](vol4_firmware_container_protocol/base_engine.md) | S8 | The region code of the stock plugouts; artefacts in [base-engine/README.md](../base-engine/README.md). |
| [ch. 9 PLUG-OUT vs base system](vol4_firmware_container_protocol/plugout_vs_base.md) | S8 | Whether PLUG-OUTs add processing or only configure the base system. |

## 5. Volume V — Toolchain and SDK

| Chapter | for | What it covers |
|---|---|---|
| [ch. 1 s1cc](vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md) | S1 | The C / C++ → DSP microcode compiler (LLVM 19): IR → graph, lowering, kernel format, `s1cc build` / `check`, tests. |
| [ch. 2 s1dsp.h / s1panel.h API](vol5_toolchain_compiler_sdk/s1dsp_api_reference.md) | S1 | The SDK header functions. |
| [ch. 3 sdk/mport](vol5_toolchain_compiler_sdk/mport_mutable_sdk.md) | S1 | Porting Mutable Instruments DSP code: number model, kernels, references, SAW_SWARM. |
| [ch. 4 Microcode toolchain](vol5_toolchain_compiler_sdk/assembler_disassembler_tools.md) | both | `asm/`: assembler, disassembler, scheduler, SYSTEM-1 rules, checks. |
| [ch. 5 Emulator](vol5_toolchain_compiler_sdk/emulator.md) | both | C reference of the DSP, native code and threads, what is missing, work order. |
| [ch. 6 Audio capture](vol5_toolchain_compiler_sdk/audio_capture.md) | S1 | MIDI and audio devices, `amidi` + `arecord`, decoding, classifying the wave; `tools/s1_capture.py`. |
| [TOOLS.md](TOOLS.md) | both | Usage and parameters of every tool: the hand-written guide [TOOLS_GUIDE.md](TOOLS_GUIDE.md) + the reference generated by `tools/tools_usage.py`. |

## 6. Volume VI — Reference designs

| Chapter | for | What it covers |
|---|---|---|
| [ch. 1 C-V4](vol6_reference_designs_applications/c_v4_system8_reference.md) | S8 | Own PLUG-OUT from scratch (eight voices measured): architecture, microcode, engine, region 0, build, install, emulator. |
| [ch. 2 C-S1V4 / C-S1V8](vol6_reference_designs_applications/c_s1v8_system1_reference.md) | S1 | C-V4 ported to the SYSTEM-1; 8 voices. |
| [ch. 3 CZ-SYSTEM](vol6_reference_designs_applications/casio_cz_phase_distortion.md) | S1 | Casio CZ phase distortion as an s1cc kernel; emulator-verified, not yet on hw. |
| [ch. 4 Mutable Instruments ports](vol6_reference_designs_applications/mutable_instruments_ports.md) | S1 | MUTABLE-SYSTEM and the rewritten kernels. |
| [ch. 5 Wavetable engines](vol6_reference_designs_applications/wavetable_engines.md) | both | C-WTLIVE, C-MORPH, C-PWM on the SYSTEM-8; the SYSTEM-1 wavetable kernels. |
| [ch. 6 Custom oscillator C-SAW](vol6_reference_designs_applications/custom_osc.md) | S8 | Own DSP oscillator, wavetables from a WAV and in bank 5, audio checks. |
| [ch. 7 Oscillators that need memory](vol6_reference_designs_applications/mutable_memory.md) | S1 | Which memory serves (bank 0 with `ldidx`, bank 5, EDM of master B, ARM tick), how much fits, measurements. |

## 7. Volume VII — Clean room, reports and logs

| Chapter / file | for | What it covers |
|---|---|---|
| [ch. 1 Clean-room boundary](vol7_cleanroom_evidence_archive/cleanroom_boundary_contract.md) | both | Team separation and the authorised material. |
| [ch. 2 Clean-room specification](vol7_cleanroom_evidence_archive/functional_and_data_requirements.md) | both | Overview, logical architecture, functional requirements, data formats. |
| [ch. 3 Test plan and evidence register](vol7_cleanroom_evidence_archive/test_vectors_verification_plan.md) | both | Validation matrix and evidence; vectors in [cleanroom/test-vectors/README.md](../cleanroom/test-vectors/README.md). |
| [ch. 4 Generated reports of the firmware images](vol7_cleanroom_evidence_archive/firmware_reverse_engineering_reports.md) | S8 | Index of the per-image reports of `sys8fw report`; reports in [raw_ghidra_reports/](vol7_cleanroom_evidence_archive/raw_ghidra_reports/). |
| [ch. 5 Transfer protocol and state machines](vol7_cleanroom_evidence_archive/cleanroom_protocol_spec.md) | both | The clean-room description of the transfer. |
| [historical_lab_logs/](vol7_cleanroom_evidence_archive/historical_lab_logs/) | S8, S1 | Obsolete raw lab records cited as "log §n" ([OBSOLETE.md](OBSOLETE.md)). |
| [NOTES.md](vol7_cleanroom_evidence_archive/NOTES.md) | S8 | Working notes of the from-scratch PLUG-OUT (Spanish). |
| [PLAN.md](vol7_cleanroom_evidence_archive/PLAN.md) | S8 | Plan and results of the S8LAB lab (`examples/sys8_lab/`). |
| [REVIEW.md](vol7_cleanroom_evidence_archive/REVIEW.md), [ASM_PENDIENTES.md](vol7_cleanroom_evidence_archive/ASM_PENDIENTES.md) | both | Review, fixes and tests of `asm/`; its open items (Spanish). |
| [PROMPT_ANALISIS_DSP.md](vol7_cleanroom_evidence_archive/PROMPT_ANALISIS_DSP.md), [PROMPT_TRM_OVERHAUL.md](vol7_cleanroom_evidence_archive/PROMPT_TRM_OVERHAUL.md) | — | Work prompts kept as they were; not a description of the hardware. |

## 8. Documents outside `docs/`

### 8.1 Examples (one folder per experiment)

| Document | for | Experiment |
|---|---|---|
| [examples/from_scratch/engine/README.md](../examples/from_scratch/engine/README.md) | S8 | The C engine of the from-scratch PLUG-OUT (region 1) and its build. |
| [examples/from_scratch/model/README.md](../examples/from_scratch/model/README.md) | S8 | The own region 0 (parameter model) in C. |
| [examples/from_scratch/re/README.md](../examples/from_scratch/re/README.md) | S8 | Static analysis of the JX-3P VST3 with Ghidra. |
| [examples/from_scratch/re/SCRIPTS.md](../examples/from_scratch/re/SCRIPTS.md) | S8 | What each script in `examples/from_scratch/re/` does and what it needs. |
| [examples/sys8_lab/README.md](../examples/sys8_lab/README.md) | S8 | S8LAB: the SYSTEM-8 image in PLG1 used as a lab. |
| [examples/wt_live/README.md](../examples/wt_live/README.md) | S8 | C-WTLIVE: live wavetable PLUG-OUT (slot 3). |
| [examples/wt_morph/README.md](../examples/wt_morph/README.md) | S8 | C-MORPH; options considered: [OPTIONS.md](../examples/wt_morph/OPTIONS.md). |
| [examples/wt_pwm/README.md](../examples/wt_pwm/README.md) | S8 | C-PWM: wavetable PLUG-OUT with a WAV inside. |
| [examples/sd_txt/README.md](../examples/sd_txt/README.md) | S8 | SD-TXT: read a file from the SD card in a region 0 init hook. |
| [examples/system_1/README.md](../examples/system_1/README.md) | S1 | C-S1V4 / C-S1V8 (Spanish). |
| [examples/system_1_jx/README.md](../examples/system_1_jx/README.md) | S1 | C-S1JX: the JX-3P microcode on the four DSPs of the SYSTEM-1; paused (Spanish). |
| [examples/system_1_ramp/README.md](../examples/system_1_ramp/README.md) | S1 | C-S1RAMP: the parameter ramps (`FCE4`) (Spanish). |
| [examples/system_1_synth/README.md](../examples/system_1_synth/README.md) | S1 | C-S1SY, built step by step on the device (Spanish). |
| [examples/system_s1cc/README.md](../examples/system_s1cc/README.md) | S1 | SYSTEM-S1CC: a PLUG-OUT that is only an s1cc kernel (Spanish). |
| [examples/mutable_system/README.md](../examples/mutable_system/README.md), [CANDIDATES.md](../examples/mutable_system/CANDIDATES.md) | S1 | MUTABLE-SYSTEM and what else can be ported (Spanish). |
| [sdk/s1cc/examples/mutable_ported/README.md](../sdk/s1cc/examples/mutable_ported/README.md) | S1 | Braids / Plaits pieces rewritten in C (Spanish). |
| [sdk/s1cc/examples/casio_cz/README.md](../sdk/s1cc/examples/casio_cz/README.md) | S1 | CZ-SYSTEM kernel (Spanish). |
| [sdk/s1cc/examples/simple_saw_wrapped/README.md](../sdk/s1cc/examples/simple_saw_wrapped/README.md) | S1 | Minimal plain-C library and its S1 wrapper (Spanish). |

The other example folders have no README: `examples/cave_tone`, `examples/mem_name`, `examples/mem_names`,
`examples/mem_scan`, `examples/midi_send`, `examples/preset_fill` (S8 hook and capture experiments; the root README
lists them with their status).

### 8.2 SYSTEM-1 scripts

[system-1/README.md](../system-1/README.md): the study scripts of the SYSTEM-1 and the chapters that document it.
