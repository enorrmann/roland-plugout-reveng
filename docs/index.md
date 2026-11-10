# Documentation Index — Roland SYSTEM-8 / SYSTEM-1 PLUG-OUT Reverse Engineering

This folder holds the documentation of the repository, ordered in seven volumes. Every chapter keeps the content of
the original document it comes from (its first lines say which one: "Formerly `docs/X.md`"); the table in
[§9](#9-old-document-names--new-location) maps each old name to its new place, so references to old names in code
comments, logs and commit messages can be resolved. The scope and status of the project are in the root
[`README.md`](../README.md); the usage of every tool is in [`TOOLS.md`](TOOLS.md).

Each fact carries the status stated in its chapter. Where a chapter uses evidence tags, the legend is given at its top;
the tags used in the originals are:

| tag | meaning |
|---|---|
| **[M]** | measured on the device |
| **[L]** | read in Roland's microcode, OS or ARM code (static) |
| **[C]** | our code |
| **[E]** | emulator prediction (`examples/from_scratch/emu`) |
| **[I]** | inferred, no direct test |
| **[A]** | open |
| **[sim]**, **[est]**, **[code]** | measured in the emulator, estimated, read in the code (SYSTEM-1 optimization catalogue) |

`dsp` code blocks are checked with `python3 tools/doccheck.py docs/vol*/*.md system-1/*.md` (they must assemble).

```
 Vol. I    System architecture        hardware, memory maps, inter-DSP paths, ARM side, MIDI routing, S1 ARM clock
 Vol. II   DSP core and ISA           hardware reference, arithmetic, encoding, instruction set, SFU, banks, native ops
 Vol. III  Voices and scheduling      S8 / S1 voice allocation, program memory, budgets, main programs, optimization
 Vol. IV   Firmware and protocol      containers, PLUG-OUT code and OS interface, service tables, USB install, updater
 Vol. V    Toolchain and SDK          s1cc, s1dsp.h, sdk/mport, asm/ toolchain, emulator, audio capture
 Vol. VI   Reference designs          C-V4, C-S1V4/V8, CZ-SYSTEM, Mutable ports, wavetables, C-SAW, memory oscillators
 Vol. VII  Clean room and evidence    boundary, clean-room spec, test plan, generated reports, frozen lab logs, notes
```

## 1. Volume I — System architecture

| Chapter | What it covers |
|---|---|
| [ch. 1 System architecture](vol1_system_architecture/soc_overview.md) | Hardware of the SYSTEM-8 (master and slave BMCs, DSPs, panel MCU) and which code runs where. |
| [ch. 2 Memory maps](vol1_system_architecture/memory_map.md) | ARM address spaces and the DSP windows, collected from the source documents. |
| [ch. 3 Inter-DSP communication and audio path](vol1_system_architecture/bus_interconnect.md) | Technical specification (v2.0, 2026-10-22): memory model per bank (scope, ring, latency, ARM access), transport mechanisms (bank-0/1 rings, MXM, pair mailbox, bank 5/6 from the ARM, IOMD ownership and audible words) with semantics, rules and microcode examples, audio path of both slaves, execution order, an end-to-end note trace, design rules. Frozen lab log: [s8_wave_bus_log.md](vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md). |
| [ch. 4 The ARM side](vol1_system_architecture/host_control_plane.md) | Processors, programs, RTOS, FPU and panel CPU, as found in the OS image. |
| [ch. 5 MIDI and SysEx routing](vol1_system_architecture/midi_sysex_routing.md) | Routing between the host, the master and the slaves: where a message is received, filtered and echoed, with the tests done and what is still open. |
| [ch. 6 SYSTEM-1 slave ARM clock](vol1_system_architecture/s1_arm_clock.md) | A 1 ms tick by hooking the OS cyclic handler client (`0x20002BFC`), measured; writing DSP parameters from it (`FCEC` to `C080`–`C09F` exact; ramps and `C100`… do not reach DSP B); telemetry through audio. |

## 2. Volume II — DSP core and ISA

| Chapter | What it covers |
|---|---|
| [ch. 1 DSP core hardware reference](vol2_dsp_core_isa/core_architecture.md) | Technical reference of the DSP chip: arithmetic, seed ROMs, timing. |
| [ch. 2 Data types and arithmetic](vol2_dsp_core_isa/data_types_arithmetic.md) | The RAW number format and the bit-exact arithmetic, collected from the hardware reference and the ISA specification. |
| [ch. 3 Instruction encoding](vol2_dsp_core_isa/instruction_word_encoding.md) | ISA specification part 1 (§1–§5): word format, classes, templates, access rules. |
| [ch. 4 Instruction set reference](vol2_dsp_core_isa/instruction_set_reference.md) | ISA specification part 2 (§6–§14), for assembler and compiler writers (used by `asm/`). |
| [ch. 5 SFU, seed ROMs and phase operations](vol2_dsp_core_isa/sfu_and_accelerators.md) | Special-function operations and their tables. |
| [ch. 6 Memory banks, ports and ARM-side configuration](vol2_dsp_core_isa/memory_banks_dma.md) | DSP ports, banks and the ARM-side configuration (static analysis). |
| [ch. 7 ISA overview (measured)](vol2_dsp_core_isa/isa_overview.md) | Instruction set of the DSP as measured on the device. |
| [ch. 8 ISA discrepancies](vol2_dsp_core_isa/isa_discrepancies.md) | Where the measured ISA, the assembler and the disassembler disagree, and what is still open. |
| [ch. 9 What the decoded operations are good for](vol2_dsp_core_isa/operation_uses.md) | What the operations found in the microcode do. |
| [ch. 10 Native ops](vol2_dsp_core_isa/native_ops.md) | Shifts, SFU, Newton, 3-operand add, two-product MAC, conditions, jumps, indexed loads, delay taps: status per op (measured on SYSTEM-8, emulator, s1cc, host, tested on SYSTEM-1), the plan to use them all, how to resume. |
| [ch. 11 Reading the memory of the slave DSPs](vol2_dsp_core_isa/slave_dsp_readback.md) | Reading slave DSP memory from the host: what is verified, the open blockers, and the plan (investigation, 2026-10-21). |

## 3. Volume III — Voice allocation and scheduling

| Chapter | What it covers |
|---|---|
| [ch. 1 SYSTEM-8 voice allocation](vol3_voice_allocation_scheduling/system8_voice_allocation.md) | Technical specification (v2.0, 2026-10-22): voice ids and the path of a note, per-DSP split of Roland family A, bank-5 words the ARM writes per voice, DCO slot interface with an oscillator example, family B, C-V4 as reference implementation, voice limit. Frozen lab log: [s8_voice_allocation_log.md](vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md). |
| [ch. 2 SYSTEM-1 voice allocation](vol3_voice_allocation_scheduling/system1_voice_allocation.md) | The SYSTEM-1 counterpart (from `system-1/VOICE_ALLOCATION.md`, which stays in `system-1/`). |
| [ch. 3 DSP program memory](vol3_voice_allocation_scheduling/slot_program_model.md) | Program memory of a DSP on SYSTEM-8 and SYSTEM-1: main program, slots / voice sections, sizes, who enables what, usage variants (main only, sections, overrunning a zone: open). |
| [ch. 4 Execution time and instruction budgets](vol3_voice_allocation_scheduling/execution_timing_budgets.md) | Budgets per DSP and device, collected from the source documents with their measurements. |
| [ch. 5 Main programs MAIN0–MAIN5](vol3_voice_allocation_scheduling/dsp_main_programs.md) | What the six main programs of the voice do, where each one runs, and what is still open. |
| [ch. 6 Optimized microcode — mandatory rules](vol3_voice_allocation_scheduling/microcode_optimization.md) | Rules for own microcode on both devices (phase ops, packing, fusion, no repeated loads, bit-exact validation) and `tools/check_optimized.py`, which runs after every build. |
| [ch. 7 Roland's oscillators](vol3_voice_allocation_scheduling/roland_oscillators.md) | How Roland's oscillators compute the waveforms (SYSTEM-8: JX-3P, Juno-106/60, Jupiter-4, SH-101, SH-1000; SYSTEM-1: SH-2, SH-101, PROMARS) and which instructions save cycles (`ph_wrap`/`ph_tri`/`ph_clamp`, fused ops, 2-term Horner, Newton reciprocal). |
| [ch. 8 SYSTEM-1 optimizations](vol3_voice_allocation_scheduling/s1_optimizations.md) | SYSTEM-1 / SYSTEM-S1CC catalogue: measured savings (ARM envelopes −213, ARM LFO −119 on DSP B; ARM pitch −115 on DSP A), end-of-ramp chain without a tick, LUT costs, scheduler lessons, pending device tests. |

## 4. Volume IV — Firmware, containers and protocol

| Chapter | What it covers |
|---|---|
| [ch. 1 Binary container formats](vol4_firmware_container_protocol/container_formats.md) | `Code*.Dat`, flash image, regions, LZSS, `SYSTEM8_UP.BIN`. |
| [ch. 2 PLUG-OUT code and the OS interface](vol4_firmware_container_protocol/plugout_virtual_machine.md) | Firmware container, slots, relocation, the OS interface, the parameter model, and the changes verified on hardware. |
| [ch. 3 OS service tables](vol4_firmware_container_protocol/os_service_table.md) | The OS services PLUG-OUT code calls, with their sources. |
| [ch. 4 USB install protocol](vol4_firmware_container_protocol/usb_sysex_protocol_spec.md) | The USB-MIDI install protocol of the PLUG-OUT plugins, with code addresses. Used by `tools/plugout_tool.py`. |
| [ch. 5 Web installer](vol4_firmware_container_protocol/web_installer.md) | The browser installer in `web/` (Web MIDI + SysEx): use, checks that block an install, port handling, tests (Node + headless Chrome). |
| [ch. 6 SYSTEM-8 base firmware](vol4_firmware_container_protocol/base_firmware.md) | `SYSTEM8_UP.BIN`: container, regions, the OS (`App1_Main`), the `tools/sys8fw.py` toolchain, and the RE results. |
| [ch. 7 Updater versions](vol4_firmware_container_protocol/firmware_versions.md) | Updater versions 1.30, 1.31 and 1.32: what changed in each image. |
| [ch. 8 Decoded base engine](vol4_firmware_container_protocol/base_engine.md) | What the decoded base engine (the region code of the stock plugouts) contains. The `base-engine/` folder is described in [base-engine/README.md](../base-engine/README.md). |
| [ch. 9 PLUG-OUT vs base system](vol4_firmware_container_protocol/plugout_vs_base.md) | Whether PLUG-OUTs add processing or only configure the base system. |

## 5. Volume V — Toolchain and SDK

| Chapter | What it covers |
|---|---|
| [ch. 1 s1cc](vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md) | The C / C++ → DSP microcode compiler (LLVM 19, C++): IR → graph (if-conversion, state, signedness, islands, speculated values), native profiling, function fitting, (register, scale, offset) lowering, spilling, kernel format, `s1cc build` / `check`, examples, integration in MUTABLE-SYSTEM (`KERNEL=`), tests. |
| [ch. 2 s1dsp.h / s1panel.h API](vol5_toolchain_compiler_sdk/s1dsp_api_reference.md) | The SDK header functions, as declared in `sdk/s1cc/include/`. |
| [ch. 3 sdk/mport](vol5_toolchain_compiler_sdk/mport_mutable_sdk.md) | Porting Mutable Instruments DSP code: number model, `Kernel` (values → registers → microcode), LUT → polynomial, components, C++ reference + bit-exact integer model + emulator bench, SAW_SWARM port and its measured differences, CLI, tests, porting recipe. |
| [ch. 4 Microcode toolchain](vol5_toolchain_compiler_sdk/assembler_disassembler_tools.md) | `asm/`: assembler, disassembler, scheduler, SYSTEM-1 rules and checks. Review and open items: [REVIEW.md](vol7_cleanroom_evidence_archive/REVIEW.md), [ASM_PENDIENTES.md](vol7_cleanroom_evidence_archive/ASM_PENDIENTES.md). |
| [ch. 5 Emulator](vol5_toolchain_compiler_sdk/emulator.md) | Emulator of a complete Roland PLUG-OUT: what exists (DSP core in C, native code and threads), what is missing (OS services, bank 6, ISA fidelity, delay line, master), work order and status. |
| [ch. 6 Audio capture (SYSTEM-1)](vol5_toolchain_compiler_sdk/audio_capture.md) | Devices (MIDI `hw:3,0,0`, audio `hw:3,0` S32_LE 96k, MIDI channel 5), `amidi` + `arecord`, decoding the raw file, classifying the wave by harmonics; `tools/s1_capture.py`. |
| [TOOLS.md](TOOLS.md) | Usage and parameters of every tool: the hand-written guide [TOOLS_GUIDE.md](TOOLS_GUIDE.md) (build, `--sim`/`--sim-check`, `check_optimized`, install, emulator) + the reference generated by `tools/tools_usage.py`. |

## 6. Volume VI — Reference designs

| Chapter | What it covers |
|---|---|
| [ch. 1 C-V4 (SYSTEM-8)](vol6_reference_designs_applications/c_v4_system8_reference.md) | Own PLUG-OUT from scratch (C-V4, eight voices measured): architecture, voice flow, microcode (`.s`/`.hex`, why Python generates assembly), ARM engine, region 0, build, install, emulator, file map and open items. |
| [ch. 2 C-S1V4 / C-S1V8 (SYSTEM-1)](vol6_reference_designs_applications/c_s1v8_system1_reference.md) | C-V4 ported to the SYSTEM-1; full example document: [examples/system_1/README.md](../examples/system_1/README.md). |
| [ch. 3 CZ-SYSTEM](vol6_reference_designs_applications/casio_cz_phase_distortion.md) | Casio CZ phase distortion as an s1cc kernel for SYSTEM-S1CC; emulator-verified, not yet on hw. |
| [ch. 4 Mutable Instruments ports](vol6_reference_designs_applications/mutable_instruments_ports.md) | MUTABLE-SYSTEM and `sdk/s1cc/examples/mutable_ported` (34 kernels; with the full host 25 fit 8 voices, 7 fit 7, 2 fit 5; MP-CSAW / MP-CSAW2 tested on hw). |
| [ch. 5 Wavetable engines](vol6_reference_designs_applications/wavetable_engines.md) | C-WTLIVE, C-MORPH, C-PWM on the SYSTEM-8; the SYSTEM-1 wavetable kernels. |
| [ch. 6 Custom oscillator C-SAW](vol6_reference_designs_applications/custom_osc.md) | Own DSP microcode oscillator: description and the audio checks. |
| [ch. 7 Mutable oscillators that need memory](vol6_reference_designs_applications/mutable_memory.md) | Strings, combs, wavetables, chorus, granular, speech: which SYSTEM-1 memory serves (bank 0 with `ldidx`, bank 5, EDM of master B, ARM tick), how much fits, what s1cc lacks, proposed order. |

## 7. Volume VII — Clean room, reports and logs

| Chapter / file | What it covers |
|---|---|
| [ch. 1 Clean-room boundary](vol7_cleanroom_evidence_archive/cleanroom_boundary_contract.md) | Team separation and the authorised material. |
| [ch. 2 Clean-room specification: overview, architecture, functional requirements, data formats](vol7_cleanroom_evidence_archive/functional_and_data_requirements.md) | Former `overview.md`, `cleanroom_architecture.md`, `functional-spec.md`, `data-formats.md`, one part each. |
| [ch. 3 Test plan and evidence register](vol7_cleanroom_evidence_archive/test_vectors_verification_plan.md) | Former `test-plan.md` and `evidence.md`. Vectors and checker: [cleanroom/test-vectors/README.md](../cleanroom/test-vectors/README.md). |
| [ch. 4 Generated reports of the firmware images](vol7_cleanroom_evidence_archive/firmware_reverse_engineering_reports.md) | Index of the per-image reports made by `sys8fw report` (former `report_index.md`); the reports themselves are in [raw_ghidra_reports/](vol7_cleanroom_evidence_archive/raw_ghidra_reports/) (`p1y01_m0c0a_up.md`: OS image, not decoded in the report; `p1y01_*.region*.md`: classes, presets, labels, virtual methods per region). |
| [ch. 5 Transfer protocol and state machines](vol7_cleanroom_evidence_archive/cleanroom_protocol_spec.md) | Former `protocol-spec.md` and `state-machines.md`. |
| [historical_lab_logs/](vol7_cleanroom_evidence_archive/historical_lab_logs/) | Frozen lab notebooks (v1 until 2026-10-22): every test, measurement and correction with its section number; old "§n" references of WAVE_BUS / VOICE_ALLOCATION point here. `s8_*` = SYSTEM-8 (former `docs/log/`), `s1_*` = SYSTEM-1 (former `system-1/log/`). |
| [NOTES.md](vol7_cleanroom_evidence_archive/NOTES.md) | Working notes of the from-scratch PLUG-OUT: interfaces found, what runs on the device, next steps. Spanish, unfinished. |
| [PLAN.md](vol7_cleanroom_evidence_archive/PLAN.md) | Plan and results of the S8LAB lab (`examples/sys8_lab/`). |
| [REVIEW.md](vol7_cleanroom_evidence_archive/REVIEW.md), [ASM_PENDIENTES.md](vol7_cleanroom_evidence_archive/ASM_PENDIENTES.md) | Review, fixes and tests of `asm/`; its open items (Spanish). |
| [PROMPT_ANALISIS_DSP.md](vol7_cleanroom_evidence_archive/PROMPT_ANALISIS_DSP.md), [PROMPT_TRM_OVERHAUL.md](vol7_cleanroom_evidence_archive/PROMPT_TRM_OVERHAUL.md) | Work prompts kept as they were; not a description of the hardware. |

## 8. Documents outside `docs/`

### 8.1 Examples (one folder per experiment)

| Document | Experiment |
|---|---|
| [examples/from_scratch/engine/README.md](../examples/from_scratch/engine/README.md) | The C engine of the from-scratch PLUG-OUT (region 1) and its build. |
| [examples/from_scratch/model/README.md](../examples/from_scratch/model/README.md) | The own region 0 (parameter model) in C. |
| [examples/from_scratch/re/README.md](../examples/from_scratch/re/README.md) | Static analysis of the JX-3P VST3 with Ghidra. |
| [examples/from_scratch/re/SCRIPTS.md](../examples/from_scratch/re/SCRIPTS.md) | What each script in `examples/from_scratch/re/` does and what it needs. |
| [examples/sys8_lab/README.md](../examples/sys8_lab/README.md) | S8LAB: the SYSTEM-8 image in PLG1 used as a lab, with its tests. |
| [examples/wt_live/README.md](../examples/wt_live/README.md) | C-WTLIVE: live wavetable PLUG-OUT (slot 3). |
| [examples/wt_morph/README.md](../examples/wt_morph/README.md) | C-MORPH: one wavetable position that morphs with an LFO; options considered: [OPTIONS.md](../examples/wt_morph/OPTIONS.md). |
| [examples/wt_pwm/README.md](../examples/wt_pwm/README.md) | C-PWM: wavetable PLUG-OUT with a WAV inside. |
| [examples/sd_txt/README.md](../examples/sd_txt/README.md) | SD-TXT: read a file from the SD card in a region 0 init hook. |
| [examples/system_1/README.md](../examples/system_1/README.md) | C-S1V4: C-V4 ported to the SYSTEM-1: PLUG-OUT interface (SYNINFO, vtables, DSP program queues, audio path slave → master), build, test order, open risks, parameter capture (PARAMLOG). Works on the device (notes, chords, volume); 8-voice C-S1V8 works too. Spanish. |
| [examples/system_1_jx/README.md](../examples/system_1_jx/README.md) | C-S1JX: the JX-3P's own DSP microcode (one DCO, 4 voices) relocated onto the SYSTEM-1's four DSPs; built and tested without hardware, not installed (paused). Spanish. |
| [examples/system_1_ramp/README.md](../examples/system_1_ramp/README.md) | C-S1RAMP: the SYSTEM-1 parameter ramps (`FCE4`): linear, one step per 1/3 ms, 4–97 ms full scale by `y`; `+8` = live value. Spanish. |
| [examples/system_1_synth/README.md](../examples/system_1_synth/README.md) | C-S1SY: C-S1V8 extended step by step on the device (DCO2, levels, ranges, fine tune, waves, amp ADSR, 4-pole low-pass, filter ADSR); how panel values reach each DSP; DSP A budget between 682 and 826; transfer packing. Spanish. |
| [examples/system_s1cc/README.md](../examples/system_s1cc/README.md) | SYSTEM-S1CC: a SYSTEM-1 PLUG-OUT that is only an s1cc kernel; voices, budget guard, host ADSRs (s1_env), OSC COLOR with COLOR MOD. Spanish. |
| [examples/mutable_system/README.md](../examples/mutable_system/README.md), [CANDIDATES.md](../examples/mutable_system/CANDIDATES.md) | MUTABLE-SYSTEM: C-S1SY13F with Braids' SAW_SWARM in place of DCO1 + DCO2 (installed in slot 1, not heard yet); what else can be ported. Spanish. |
| [sdk/s1cc/examples/mutable_ported/README.md](../sdk/s1cc/examples/mutable_ported/README.md) | Braids / Plaits oscillators, filters and drums rewritten in C for the SYSTEM-1 DSP. Spanish. |
| [sdk/s1cc/examples/casio_cz/README.md](../sdk/s1cc/examples/casio_cz/README.md) | CZ-SYSTEM kernel. Spanish. |
| [sdk/s1cc/examples/simple_saw_wrapped/README.md](../sdk/s1cc/examples/simple_saw_wrapped/README.md) | The minimal example of a plain C library (a naive saw) and its wrapper for an S1 voice. Spanish. |

The other example folders have no README: `examples/cave_tone`, `examples/mem_name`, `examples/mem_names`,
`examples/mem_scan`, `examples/midi_send`, `examples/preset_fill`. Each one is a hook or capture experiment; the
root README lists them with their status.

### 8.2 SYSTEM-1 (the device)

Kept together in the `system-1/` folder (documents and scripts), at the user's request (2026-10-22). Spanish.

| Document | What it covers |
|---|---|
| [system-1/README.md](../system-1/README.md) | Index of the folder, scripts and status. |
| [system-1/FIRMWARE.md](../system-1/FIRMWARE.md) | SYSTEM-1 OS images (updater 1.30 unencrypted, plug-in `Code1.Dat` 1.20), extraction, known addresses. |
| [system-1/VOCES_6.md](../system-1/VOCES_6.md) | 6-voice analysis: slave DSP voice programs and budget, master→slave ids, allocator sites, plan. |
| [system-1/SH101_POLY.md](../system-1/SH101_POLY.md) | SH-101 PLUG-OUT on SYSTEM-1 to 4 voices: its DSP programs, the slave's program-loading services, plan. |
| [system-1/ARCHITECTURE.md](../system-1/ARCHITECTURE.md) | SYSTEM-1 architecture: chips, what code runs where, flows, DSP windows, tools. |
| [system-1/VOICE_ALLOCATION.md](../system-1/VOICE_ALLOCATION.md) | SYSTEM-1 voice allocation, technical specification (v2.0). Lab log: [s1_voice_allocation_log.md](vol7_cleanroom_evidence_archive/historical_lab_logs/s1_voice_allocation_log.md). |
| [system-1/WAVE_BUS.md](../system-1/WAVE_BUS.md) | SYSTEM-1 inter-DSP communication and audio path, technical specification (v2.0). Lab log: [s1_wave_bus_log.md](vol7_cleanroom_evidence_archive/historical_lab_logs/s1_wave_bus_log.md). |
| [system-1/DSP_BANKS.md](../system-1/DSP_BANKS.md) | What is in each DSP data bank (ring 0, mailbox 1, delay memory 2, IOMD, bank 5, parameters 6, counters 7), which ranges the OS and the official PLUG-OUTs use, rules for ours, the delay-line / `ldidx` measurement. |
| [system-1/FX_CHAIN.md](../system-1/FX_CHAIN.md) | Panel delay and reverb in PLUG-OUT mode (OS program on master B, bank-1 link, parameters, ids `0x18D`–`0x18F`, what is missing for them to sound with C-S1SY). |
| [system-1/DSP_FIT.md](../system-1/DSP_FIT.md) | Does a SYSTEM-8 PLUG-OUT's microcode (JX-3P) fit the SYSTEM-1's 4 DSPs? `dsp_fit.py`; 4 voices: no; 2 voices: on paper. |

## 9. Old document names → new location

| old `docs/` name | new location |
|---|---|
| `ARCHITECTURE.md` | [Vol. I ch. 1](vol1_system_architecture/soc_overview.md) |
| `WAVE_BUS.md` | [Vol. I ch. 3](vol1_system_architecture/bus_interconnect.md) |
| `MIDI_SYSEX.md` | [Vol. I ch. 5](vol1_system_architecture/midi_sysex_routing.md) |
| `S1_ARM_CLOCK.md` | [Vol. I ch. 6](vol1_system_architecture/s1_arm_clock.md) |
| `DSP_HARDWARE.md` | [Vol. II ch. 1](vol2_dsp_core_isa/core_architecture.md) (arithmetic also in [ch. 2](vol2_dsp_core_isa/data_types_arithmetic.md), seed ROMs in [ch. 5](vol2_dsp_core_isa/sfu_and_accelerators.md)) |
| `DSP_ISA_SPEC.md` | §1–§5: [Vol. II ch. 3](vol2_dsp_core_isa/instruction_word_encoding.md); §6–§14: [Vol. II ch. 4](vol2_dsp_core_isa/instruction_set_reference.md) |
| `DSP_ARM_SIDE.md` | [Vol. II ch. 6](vol2_dsp_core_isa/memory_banks_dma.md) |
| `DSP_ISA.md` | [Vol. II ch. 7](vol2_dsp_core_isa/isa_overview.md) |
| `DSP_ISA_DISCREPANCIES.md` | [Vol. II ch. 8](vol2_dsp_core_isa/isa_discrepancies.md) |
| `DSP_USOS.md` | [Vol. II ch. 9](vol2_dsp_core_isa/operation_uses.md) |
| `NATIVE_OPS.md` | [Vol. II ch. 10](vol2_dsp_core_isa/native_ops.md) |
| `SLAVE_DSP_READBACK.md` | [Vol. II ch. 11](vol2_dsp_core_isa/slave_dsp_readback.md) |
| `VOICE_ALLOCATION.md` | [Vol. III ch. 1](vol3_voice_allocation_scheduling/system8_voice_allocation.md) |
| `DSP_PROGRAM_MEMORY.md` | [Vol. III ch. 3](vol3_voice_allocation_scheduling/slot_program_model.md) |
| `DSP_MAIN_PROGRAMS.md` | [Vol. III ch. 5](vol3_voice_allocation_scheduling/dsp_main_programs.md) |
| `MICROCODE_OPTIMIZATION.md` | [Vol. III ch. 6](vol3_voice_allocation_scheduling/microcode_optimization.md) |
| `ROLAND_OSCILLATORS.md` | [Vol. III ch. 7](vol3_voice_allocation_scheduling/roland_oscillators.md) |
| `S1_OPTIMIZATIONS.md` | [Vol. III ch. 8](vol3_voice_allocation_scheduling/s1_optimizations.md) |
| `CUSTOM_PLUGOUTS.md` | [Vol. IV ch. 2](vol4_firmware_container_protocol/plugout_virtual_machine.md) |
| `PROTOCOL.md` | [Vol. IV ch. 4](vol4_firmware_container_protocol/usb_sysex_protocol_spec.md) |
| `WEB_INSTALLER.md` | [Vol. IV ch. 5](vol4_firmware_container_protocol/web_installer.md) |
| `BASE_FIRMWARE.md` | [Vol. IV ch. 6](vol4_firmware_container_protocol/base_firmware.md) |
| `VERSIONS.md` | [Vol. IV ch. 7](vol4_firmware_container_protocol/firmware_versions.md) |
| `BASE_ENGINE.md` | [Vol. IV ch. 8](vol4_firmware_container_protocol/base_engine.md) |
| `PLUGOUT_VS_BASE.md` | [Vol. IV ch. 9](vol4_firmware_container_protocol/plugout_vs_base.md) |
| `S1CC.md` | [Vol. V ch. 1](vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md) |
| `MUTABLE_SDK.md` | [Vol. V ch. 3](vol5_toolchain_compiler_sdk/mport_mutable_sdk.md) |
| `EMULATOR.md` | [Vol. V ch. 5](vol5_toolchain_compiler_sdk/emulator.md) |
| `S1_AUDIO_CAPTURE.md` | [Vol. V ch. 6](vol5_toolchain_compiler_sdk/audio_capture.md) |
| `TOOLS.md`, `TOOLS_GUIDE.md` | [TOOLS.md](TOOLS.md), [TOOLS_GUIDE.md](TOOLS_GUIDE.md) (unchanged names) |
| `FROM_SCRATCH.md` | [Vol. VI ch. 1](vol6_reference_designs_applications/c_v4_system8_reference.md) |
| `CUSTOM_OSC.md` | [Vol. VI ch. 6](vol6_reference_designs_applications/custom_osc.md) |
| `MUTABLE_MEMORY.md` | [Vol. VI ch. 7](vol6_reference_designs_applications/mutable_memory.md) |
| `clean-room-boundary.md` | [Vol. VII ch. 1](vol7_cleanroom_evidence_archive/cleanroom_boundary_contract.md) |
| `overview.md`, `cleanroom_architecture.md`, `functional-spec.md`, `data-formats.md` | [Vol. VII ch. 2](vol7_cleanroom_evidence_archive/functional_and_data_requirements.md) |
| `test-plan.md`, `evidence.md` | [Vol. VII ch. 3](vol7_cleanroom_evidence_archive/test_vectors_verification_plan.md) |
| `report_index.md`, `p1y01_*.md` | [Vol. VII ch. 4](vol7_cleanroom_evidence_archive/firmware_reverse_engineering_reports.md), [raw_ghidra_reports/](vol7_cleanroom_evidence_archive/raw_ghidra_reports/) |
| `protocol-spec.md`, `state-machines.md` | [Vol. VII ch. 5](vol7_cleanroom_evidence_archive/cleanroom_protocol_spec.md) |
| `log/WAVE_BUS_LOG.md`, `log/VOICE_ALLOCATION_LOG.md` | [s8_wave_bus_log.md](vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md), [s8_voice_allocation_log.md](vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) |
| `system-1/log/WAVE_BUS_LOG.md`, `system-1/log/VOICE_ALLOCATION_LOG.md` | [s1_wave_bus_log.md](vol7_cleanroom_evidence_archive/historical_lab_logs/s1_wave_bus_log.md), [s1_voice_allocation_log.md](vol7_cleanroom_evidence_archive/historical_lab_logs/s1_voice_allocation_log.md) |
| `NOTES.md`, `PLAN.md`, `REVIEW.md`, `ASM_PENDIENTES.md`, `PROMPT_*.md` | [vol7_cleanroom_evidence_archive/](vol7_cleanroom_evidence_archive/) (same names) |
| `SCRIPTS.md` | [examples/from_scratch/re/SCRIPTS.md](../examples/from_scratch/re/SCRIPTS.md) |
| `OPTIONS.md` | [examples/wt_morph/OPTIONS.md](../examples/wt_morph/OPTIONS.md) |
| `examples_<folder>_README.md`, `examples_mutable_system_CANDIDATES.md` | the README (or CANDIDATES.md) inside the example folder, §8.1 |
| `examples_mutable_ported_README.md`, `examples_casio_cz_README.md`, `examples_simple_saw_wrapped_README.md` | `sdk/s1cc/examples/<name>/README.md`, §8.1 |
| `examples_from_scratch_re_ghidra_README.md` | [examples/from_scratch/re/README.md](../examples/from_scratch/re/README.md) |
| `base-engine_README.md` | [base-engine/README.md](../base-engine/README.md) |
| `cleanroom_test-vectors_README.md` | [cleanroom/test-vectors/README.md](../cleanroom/test-vectors/README.md) |
| `INDEX.md` | this file |
