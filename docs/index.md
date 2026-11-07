# Roland BMC System-on-Chip (SoC) Technical Reference Manual (TRM) Suite

> **Document Suite ID**: TRM-BMC-MASTER-001  
> **Target Silicon**: Roland BMC ASIC ("Escadron" Multi-Core Audio SoC)  
> **Target Instruments**: Roland SYSTEM-8, Roland SYSTEM-1 / SYSTEM-1m  
> **Publication Standard**: Texas Instruments TMS320C6000 / Analog Devices SHARC Processor Reference Standard  
> **Release Status**: Comprehensive Technical Reference Suite  

---

## Executive Overview

The **Roland BMC** (Custom Audio Application-Specific Integrated Circuit, internal project designation *Escadron*) is a proprietary heterogeneous multi-core System-on-Chip (SoC) designed by Roland Corporation to power the AIRA and Boutique lines of digital synthesizers and circuit-modeling instruments. 

This Technical Reference Manual (TRM) Suite provides the complete, authoritative, bit-level hardware reference, Instruction Set Architecture (ISA) manual, voice allocation specification, operating system jump table reference, and clean-room implementation guide for engineers authoring custom microcode, compilers, emulators, and native synthesis engines for the platform.

```
+---------------------------------------------------------------------------------------------------------+
|                                    ROLAND BMC SOC & TRM SUITE MAP                                       |
|                                                                                                         |
|   +------------------------------------+   +------------------------------------+                       |
|   | VOLUME I: SYSTEM ARCHITECTURE      |   | VOLUME II: DSP CORE & ISA REF      |                       |
|   | - BMC SoC & Multi-Chip Topologies  |   | - VLIW Pipeline & Register File    |                       |
|   | - Global Memory & Address Spaces   |   | - 32-bit Roland RAW Number Model   |                       |
|   | - IOMD Wave Bus & Interconnects    |   | - Complete 123-Opcode Reference    |                       |
|   | - Host Control Plane & μT-Kernel   |   | - SFU Non-Linear Accelerators      |                       |
|   +-----------------+------------------+   +-----------------+------------------+                       |
|                     │                                        │                                          |
|                     ├────────────────────────────────────────┴──────────────────┐                       |
|                     ▼                                                           ▼                       |
|   +------------------------------------+   +------------------------------------+                       |
|   | VOLUME III: VOICE ALLOCATION       |   | VOLUME IV: FIRMWARE & PROTOCOLS    |                       |
|   | - System-8 8-Voice Clustered Model |   | - Code*.Dat & LZSS Container Model |                       |
|   | - System-1 4/8-Voice Dual-DSP Core |   | - Two-Region Virtual Machine       |                       |
|   | - Dynamic Slot Microcode Chaining  |   | - Fixed Syscall Tables (0113FC00h) |                       |
|   | - 96 kHz Cycle Budgets & Timing    |   | - USB-MIDI SysEx Protocol Engine   |                       |
|   +-----------------+------------------+   +-----------------+------------------+                       |
|                     │                                                           │                       |
|                     └───────────────────────┬───────────────────────────────────┘                       |
|                                             ▼                                                           |
|   +------------------------------------+   +------------------------------------+                       |
|   | VOLUME V: TOOLCHAIN & SDKs         |   | VOLUME VI: REFERENCE DESIGNS       |                       |
|   | - s1cc LLVM 19 Compiler Internals  |   | - C-V4 8-Voice SYSTEM-8 Engine     |                       |
|   | - s1dsp.h C API & Intrinsics       |   | - C-S1V8 8-Voice SYSTEM-1 Synth    |                       |
|   | - mport Mutable Instruments SDK    |   | - Casio CZ Phase Distortion Engine |                       |
|   | - asm.py, disasm.py, dspsched.py   |   | - Mutable Braids / Plaits Ports    |                       |
|   +-----------------+------------------+   +-----------------+------------------+                       |
|                     │                                        │                                          |
|                     └───────────────────┬────────────────────┘                                          |
|                                         ▼                                                               |
|   +-----------------------------------------------------------------------------+                       |
|   | VOLUME VII: CLEAN-ROOM ARCHIVE & EVIDENCE LOGS                              |                       |
|   | - Team A / Team B Operational Separation & Firewall Boundary Contract       |                       |
|   | - Normative Functional, Data & Performance Requirements (REQ-ARCH/FUNC/DATA)|                       |
|   | - Deterministic Verification Plan & Golden Hash JSON Vectors                |                       |
|   | - Forensic Binary Disassembly Reports & Preserved Historical Lab Logs       |                       |
|   +-----------------------------------------------------------------------------+                       |
+---------------------------------------------------------------------------------------------------------+
```

---

## Master Table of Contents

### [Volume I: System Architecture & Hardware](vol1_system_architecture/soc_overview.md)
*Covers SoC architecture, multi-ASIC topologies, clock generation, memory partitions, and high-speed inter-chip buses.*
- [Chapter 1: Roland BMC SoC Architecture & Multi-Chip Topologies](vol1_system_architecture/soc_overview.md)
- [Chapter 2: Global Memory Map & Address Spaces](vol1_system_architecture/memory_map.md)
- [Chapter 3: Bus Interconnects, Wave Bus & Inter-ASIC Matrix](vol1_system_architecture/bus_interconnect.md)
- [Chapter 4: Host Control Plane, ARM Subsystems & μT-Kernel OS](vol1_system_architecture/host_control_plane.md)

### [Volume II: Roland BMC DSP Core & ISA Reference](vol2_dsp_core_isa/core_architecture.md)
*The definitive instruction set manual and programmer's hardware specification for the proprietary 96 kHz VLIW audio DSP.*
- [Chapter 1: DSP Core Pipeline, Register File & Processor State](vol2_dsp_core_isa/core_architecture.md)
- [Chapter 2: Data Types, Roland RAW Arithmetic & Number Models](vol2_dsp_core_isa/data_types_arithmetic.md)
- [Chapter 3: Instruction Word Formats, Class Templates & Access Rules](vol2_dsp_core_isa/instruction_word_encoding.md)
- [Chapter 4: Complete Instruction Set Architecture (ISA) Reference Manual](vol2_dsp_core_isa/instruction_set_reference.md)
- [Chapter 5: Special Function Units (SFU), Seed ROMs & Mathematical Accelerators](vol2_dsp_core_isa/sfu_and_accelerators.md)
- [Chapter 6: DSP Memory Banks, Circular Ring Buffers & DMA](vol2_dsp_core_isa/memory_banks_dma.md)

### [Volume III: Voice Engine & Dynamic Scheduling](vol3_voice_allocation_scheduling/system8_voice_allocation.md)
*Polyphonic voice allocation algorithms, note dispatch protocols, dynamic microcode slot execution, and 96 kHz timing budgets.*
- [Chapter 1: Roland SYSTEM-8 Polyphonic Voice Architecture & Dispatch](vol3_voice_allocation_scheduling/system8_voice_allocation.md)
- [Chapter 2: Roland SYSTEM-1 Voice Architecture & Dynamic Allocation](vol3_voice_allocation_scheduling/system1_voice_allocation.md)
- [Chapter 3: Slot Program Model, Main Microcode & Dynamic Dispatch](vol3_voice_allocation_scheduling/slot_program_model.md)
- [Chapter 4: 96 kHz Execution Timing, Cycle Budgets & Optimization](vol3_voice_allocation_scheduling/execution_timing_budgets.md)

### [Volume IV: Firmware Containers, OS & SysEx Protocol](vol4_firmware_container_protocol/container_formats.md)
*Binary package structures, XOR decryption, 7-in-8 MIDI serialization, fixed syscall jump tables, and USB installation state machines.*
- [Chapter 1: Binary Container Formats, Firmware Update Images & Compression](vol4_firmware_container_protocol/container_formats.md)
- [Chapter 2: Plug-Out Architecture, Region 0/1 Model & Lifecycle](vol4_firmware_container_protocol/plugout_virtual_machine.md)
- [Chapter 3: Fixed Vector OS Service Tables (Syscalls & APIs)](vol4_firmware_container_protocol/os_service_table.md)
- [Chapter 4: USB-MIDI SysEx Installation Protocol & Web Installer](vol4_firmware_container_protocol/usb_sysex_protocol_spec.md)

### [Volume V: Toolchain, Compiler & SDK Specification](vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md)
*Compiler internals for LLVM 19 lowering to BMC microcode, C/C++ API documentation, Mutable Instruments porting SDK, and developer utilities.*
- [Chapter 1: The `s1cc` Compiler Architecture & Graph Lowering](vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md)
- [Chapter 2: The `s1dsp.h` C/C++ API & Hardware Intrinsics Reference](vol5_toolchain_compiler_sdk/s1dsp_api_reference.md)
- [Chapter 3: Mutable Instruments Porting SDK (`mport`) & Fixed-Point Transformations](vol5_toolchain_compiler_sdk/mport_mutable_sdk.md)
- [Chapter 4: Developer Toolchain, Assembler, Disassembler & Utilities](vol5_toolchain_compiler_sdk/assembler_disassembler_tools.md)

### [Volume VI: Reference Designs & Applications](vol6_reference_designs_applications/c_v4_system8_reference.md)
*Production reference engines, clean-room synthesizer models, physical modeling ports, and dynamic wavetable implementations.*
- [Chapter 1: The C-V4 8-Voice From-Scratch SYSTEM-8 Reference Design](vol6_reference_designs_applications/c_v4_system8_reference.md)
- [Chapter 2: The C-S1V8 8-Voice SYSTEM-1 Synthesizer Reference Design](vol6_reference_designs_applications/c_s1v8_system1_reference.md)
- [Chapter 3: Casio CZ Phase Distortion (PD) Synthesis Implementation](vol6_reference_designs_applications/casio_cz_phase_distortion.md)
- [Chapter 4: Mutable Instruments Eurorack Ports & Physical Modeling](vol6_reference_designs_applications/mutable_instruments_ports.md)
- [Chapter 5: Wavetable Synthesis Engines & Dynamic Bank 5 Injection](vol6_reference_designs_applications/wavetable_engines.md)

### [Volume VII: Clean-Room Archive & Evidence Logs](vol7_cleanroom_evidence_archive/cleanroom_boundary_contract.md)
*Legal clean-room boundary contracts, formal functional specifications, deterministic verification vectors, forensic binary reports, and historical lab notes.*
- [Chapter 1: Clean-Room Boundary Specification & Team Separation Contract](vol7_cleanroom_evidence_archive/cleanroom_boundary_contract.md)
- [Chapter 2: Functional, Data & Performance Requirements Specification](vol7_cleanroom_evidence_archive/functional_and_data_requirements.md)
- [Chapter 3: Clean-Room Test Vectors & System Verification Plan](vol7_cleanroom_evidence_archive/test_vectors_verification_plan.md)
- [Chapter 4: Firmware Reverse Engineering Synthesis & Binary Analysis](vol7_cleanroom_evidence_archive/firmware_reverse_engineering_reports.md)
- [Preserved Historical Laboratory Logs (Non-Normative)](vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md)

---

## Evidence & Certainty Taxonomy Guide

Every technical assertion, register mapping, memory address, and execution cycle metric within this manual suite carries an explicit normative certainty classification:

- **`[VERIFIED]`**: Bit-exact reproducibility confirmed across exhaustive automated sweeps on physical hardware (e.g. Roland SYSTEM-8 or SYSTEM-1) or mathematical equivalence established.
- **`[MEASURED]`**: Directly observed and benchmarked on physical hardware under specific test vectors (e.g. cycle headroom, USB audio captures, scope timing).
- **`[STATIC_ANALYSIS]`**: Derived from rigorous Ghidra / IDA Pro disassembly and reverse engineering of production firmware binaries (`SYSTEM8_UP.BIN`, `SYSTEM1_UPD.BIN`, VST3 images).
- **`[INFERRED]`**: Logically deduced from observed architectural constraints and structural boundaries; awaiting dedicated hardware loopback verification.
- **`[UNRESOLVED]`**: Open architectural discrepancy or uncharacterized MMIO register currently under active investigation.
