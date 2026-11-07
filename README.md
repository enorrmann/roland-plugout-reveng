# Roland Custom Plug-Out Ecosystem & BMC DSP SDK

[![TRM Suite](https://img.shields.io/badge/Docs-TRM%20Suite%20(Vols%20I--VII)-blue.svg)](docs/index.md)
[![Verification](https://img.shields.io/badge/Cleanroom-12%2F12%20Vectors%20OK-brightgreen.svg)](cleanroom/test-vectors/)
[![DSP Assembly](https://img.shields.io/badge/Doccheck-47%2F47%20DSP%20Blocks%20Valid-success.svg)](tools/doccheck.py)
[![Python](https://img.shields.io/badge/Python-3.8%2B-informational.svg)](tools/)

**Build and execute your own custom synthesizers directly on Roland hardware.**

The primary objective of this project is to provide a complete, open-source development ecosystem—featuring a native **C/C++ compiler (`s1cc`)**, a **cycle-accurate software emulator (`emu/`)**, developer **SDKs (`sdk/`)**, and **USB deployment tools (`tools/`)**—enabling creators to author, simulate, and flash **custom PLUG-OUT engines** onto the **Roland SYSTEM-8** and **Roland SYSTEM-1 / SYSTEM-1m** without any proprietary Roland software or VST plugins.

The exhaustive reverse engineering of the Roland BMC ASIC ("Escadron" SoC) and the 7-Volume Technical Reference Manual serve as the foundational engineering basis that makes custom Plug-Out synthesis possible.

---

## 1. The Core Objective: Custom Plug-Out Synthesis

This project turns the Roland SYSTEM-8 and SYSTEM-1 into open digital synthesis platforms. You can write your own oscillators, filters, envelope generators, or entire synthesis models, test them on your PC, and install them into hardware slots over standard USB.

```
   ┌─────────────────────────────────────────────────────────────────────────┐
   │                       CUSTOM PLUG-OUT WORKFLOW                          │
   │                                                                         │
   │   1. DEVELOP                  2. SIMULATE                 3. DEPLOY     │
   │  ┌───────────────┐           ┌───────────────┐          ┌────────────┐  │
   │  │ C/C++ Code    │  s1cc     │ Bit-Exact C   │          │ Physical   │  │
   │  │ sdk/s1cc/     │──────────►│ Emulator      │─────────►│ Synthesizer│  │
   │  │ & s1dsp.h API │  Compiler │ emu/ & JIT    │ Verified │ SYSTEM-8/1 │  │
   │  └───────────────┘           └───────────────┘          └────────────┘  │
   │         ▲                            ▲                        ▲         │
   │         │                            │                        │         │
   │  [Developer SDK]            [Cycle Accuracy]          [plugout_tool]    │
   └─────────────────────────────────────────────────────────────────────────┘
```

### Proven Custom Engines Running on Physical Hardware
- **C-V4 (SYSTEM-8)**: An 8-voice polyphonic synthesizer running on real hardware across both Slave BMCs with **100% custom microcode** (zero Roland microcode).
- **C-S1V8 (SYSTEM-1)**: An expanded 8-voice native engine running on hardware, proving that the hardware can exceed its stock 4-voice limit.
- **Casio CZ Phase Distortion**: Complete CZ-style synthesis ported to BMC DSP microcode.
- **Mutable Instruments Ports (`sdk/mport/`)**: Eurorack models (Braids `SAW_SWARM`, Plaits engines) ported to the BMC DSP architecture.
- **C-WTLIVE**: Real-time wavetable synthesis with dynamic table streaming into DSP memory over USB.

---

## 2. The Developer Toolkit

### A. The C Compiler & SDK (`sdk/s1cc/`, `sdk/mport/`)
Instead of hand-writing raw assembly, you can write DSP synthesis code in modern C:
- **`s1cc` (LLVM 19 Backend)**: An optimizing compiler targeting the custom 96 kHz VLIW architecture of the Roland BMC DSP. It handles instruction selection, DAG lowering, register allocation (`r0`–`r7`, `h4`–`h7`), and parallel slot scheduling.
- **`s1dsp.h` Hardware API**: High-level C intrinsics and inlines mapping directly to single-cycle hardware operations:
  ```c
  #include <s1dsp.h>

  void render_voice(float dt) {
      static float phase = 0.0f;
      phase = dsp_ph_wrap(phase + dt);  // SFU hardware phase accumulator
      float sample = dsp_sin(phase);    // Polynomial Horner accelerator
      dsp_store_ring(sample);           // Circular Bank 0 ring buffer
  }
  ```
- **Mutable Instruments SDK (`sdk/mport/`)**: Automatic fixed-point transformations, polynomial approximations of lookup tables, and audio block wrappers for Eurorack DSP modules.

### B. The Cycle-Accurate Simulator (`examples/from_scratch/emu/`)
Develop and test audio without needing the physical synth attached:
- **Bit-Exact C Core (`dspcore.c`, `dspdec.h`)**: Faithfully models the BMC DSP pipeline, single-cycle instruction execution, RAW floating-point saturation ($[-512.0, +512.0)$), load latencies, and all 8 memory banks.
- **Real-Time JIT Simulation (`dspjit.py`)**: Native x86-64 code generation for testing complex polyphonic patches at full 96 kHz speed in software.
- **Automated Verification Harness (`test_core.py`)**: Regression-tested against a golden dataset of 15,756 hardware-verified instruction patterns.

### C. The Deployment Toolchain (`tools/plugout_tool.py`, `asm/`)
Flash your creations directly to hardware:
- **`tools/plugout_tool.py`**: Standalone command-line utility to query synth status, unpack containers, pack custom code, and flash firmware + patches over USB-MIDI.
- **`asm/asm.py` & `asm/disasm.py`**: Macro assembler and disassembler with round-trip verification across all commercial firmware programs.
- **`asm/dspsched.py`**: VLIW list scheduler packing independent operations into 1-to-31 word execution bundles.
- **`tools/check_optimized.py`**: Microcode linter that catches unoptimized instructions, illegal memory bank combinations, and pipeline hazards.

---

## 3. The Engineering Foundation (The Means to the Goal)

To make custom Plug-Out creation possible without proprietary tools, the entire platform was reverse-engineered from bare metal and specified to semiconductor industry standards.

### The 7-Volume Technical Reference Manual (TRM) Suite
The exhaustive engineering findings are documented under **[`docs/index.md`](docs/index.md)**:

| Volume | Title & Core Content | Key Information for Developers |
|:---|:---|:---|
| **[Volume I](docs/vol1_system_architecture/soc_overview.md)** | **System Architecture & Hardware** | Multi-BMC topology (Master + Slaves), global memory map, inter-chip IOMD audio routing (`0x6006`–`0x6021`), and Cortex-M control plane. |
| **[Volume II](docs/vol2_dsp_core_isa/core_architecture.md)** | **Roland BMC DSP Core & Full ISA** | Complete instruction reference (`mac`, `mul`, `add`, `ph_wrap`, `recip`, `sqrt`, `ldidx`), Roland 32-bit RAW number format, SFU seed ROMs, and Banks 0–7. |
| **[Volume III](docs/vol3_voice_allocation_scheduling/system8_voice_allocation.md)** | **Voice Allocation & Scheduling** | Polyphonic note dispatch (`0x429+v` on S8, `0x110+v` on S1), Bank 5 parameter mailboxes (`0xA000`), voice stealing, and 96 kHz cycle budgets (925 cyc S8 / 703 cyc S1). |
| **[Volume IV](docs/vol4_firmware_container_protocol/container_formats.md)** | **Firmware Containers, OS & Protocol** | `Code*.Dat` XOR encryption (`0xBACADABE`), LZSS compression, 7-in-8 USB SysEx packet framing, Region 0 (SYNINFO) / Region 1 (Engine Driver) VM, and OS syscall table (`0x0113FC00`). |
| **[Volume V](docs/vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md)** | **Compiler & Toolchain Specification** | LLVM 19 `s1cc` internals, instruction lowering, register spilling, `s1dsp.h` API, Mutable SDK, and CLI reference. |
| **[Volume VI](docs/vol6_reference_designs_applications/c_v4_system8_reference.md)** | **Reference Designs & Sound Labs** | Complete architectural walkthroughs of C-V4 (8-voice S8), C-S1V8 (8-voice S1), Casio CZ, and Mutable Ports. |
| **[Volume VII](docs/vol7_cleanroom_evidence_archive/cleanroom_boundary_contract.md)** | **Clean-Room Archive & Evidence** | Team A / Team B legal separation rules, formal functional specifications, and golden test vector suites. |

---

## 4. Hardware Verification & Capabilities

Every key feature of our toolchain and reference engines has been verified on real hardware:

| Platform | Feature | Status | Hardware Evidence |
|:---|:---|:---|:---|
| **SYSTEM-8** | USB Plug-Out Installation | **Verified** | Custom containers flash cleanly to Slots 1, 2, and 3 via `plugout_tool.py`. |
| **SYSTEM-8** | 8-Voice Native Synthesis (C-V4) | **Verified** | Real-time 8-voice polyphony across both Slave BMCs with custom microcode. |
| **SYSTEM-8** | 96 kHz Oscillator Tracking | **Verified** | Custom DCO tracks pitch accurately within ±1.2 cents across 5 octaves. |
| **SYSTEM-8** | Live Wavetable Audio Stream | **Verified** | Audio frames stream into Bank 5 memory over USB without re-flashing. |
| **SYSTEM-1** | 8-Voice Polyphony Expansion | **Verified** | C-S1V8 engine plays 8 voices on physical hardware (exceeding stock 4-voice limit). |
| **SYSTEM-1** | Low-Jitter 1.00 ms ARM Tick | **Verified** | Hardware cyclic interrupt (`0x01008983`) drives low-jitter parameter modulations. |
| **SYSTEM-1** | Audio Loopback Verification | **Verified** | Direct 96 kHz S32_LE capture and FFT analysis via `tools/s1_capture.py`. |

---

## 5. Quickstart: Building & Flashing a Custom Plug-Out

### 1. Requirements
- Python 3.8+
- Linux (zero setup, uses native `libasound`) or macOS/Windows (`pip install python-rtmidi`)
- Clang/LLVM 19 (for compiling C code via `s1cc`)

### 2. Connect and Inspect Your Synth
Connect the synthesizer via USB. The tooling automatically detects the control port:
```bash
# Verify USB connection
python3 tools/plugout_tool.py ports

# Query device identity, firmware, and installed slots
python3 tools/plugout_tool.py info
```

### 3. Simulate Your Synth Locally
Run synthesis tests on your PC using the cycle-accurate emulator:
```bash
# Run the DSP core verification suite against 15,756 golden test vectors
python3 examples/from_scratch/emu/test_core.py

# Run the C-V4 polyphonic voice allocation model
python3 examples/from_scratch/emu/test_chain.py
```

### 4. Flash a Custom Plug-Out to Hardware
Deploy an engine into a selected hardware slot with a single command:
```bash
# Flash the C-V4 8-voice custom engine into Slot 1
python3 tools/plugout_tool.py install examples/from_scratch/v4 --slot 1 -y

# Flash a SYSTEM-1 custom 8-voice engine
python3 tools/plugout_tool.py install examples/system_1 --slot 1 --no-patches -y
```

### 5. Quality Assurance & Regression Tests
Verify that all code and documentation remain mathematically sound and compilable:
```bash
# Validate all DSP microcode listings in the TRM suite
python3 tools/doccheck.py docs/index.md docs/vol*/*.md

# Validate clean-room golden test vectors
python3 cleanroom/test-vectors/check_vectors.py .
```

---

## 6. Repository Layout

```text
├── sdk/                                   # Software Development Kits for Custom Engines
│   ├── s1cc/                              # LLVM 19 C/C++ compiler targeting Roland BMC DSP
│   └── mport/                             # Mutable Instruments Eurorack porting toolkit
├── examples/                              # Working Custom Plug-Out Reference Engines
│   ├── from_scratch/                      # C-V4: 8-voice native SYSTEM-8 engine & C emulator (emu/)
│   ├── system_1/                          # C-S1V4 / C-S1V8: 8-voice native SYSTEM-1 engine
│   ├── system_1_synth/                    # C-S1SY: Custom dual-DCO synth engine
│   └── wt_live/                           # C-WTLIVE: Live USB wavetable player
├── tools/                                 # Host CLI utilities (plugout_tool.py, sys8fw.py, doccheck.py)
├── asm/                                   # DSP macro assembler, disassembler, scheduler (asm.py, dspsched.py)
├── docs/                                  # 7-Volume Technical Reference Manual Suite
│   ├── index.md                           # Master TRM Index and Engineering Roadmap
│   ├── vol1_system_architecture/          # Volume I: SoC Architecture & Memory Map
│   ├── vol2_dsp_core_isa/                 # Volume II: Roland BMC DSP Core Architecture & Full ISA
│   ├── vol3_voice_allocation_scheduling/  # Volume III: Voice Allocation & 96 kHz Cycle Budgets
│   ├── vol4_firmware_container_protocol/  # Volume IV: Binary Containers, SysEx, OS Syscalls
│   ├── vol5_toolchain_compiler_sdk/       # Volume V: Compiler Internals & SDK Reference
│   ├── vol6_reference_designs_applications/ # Volume VI: Production Reference Engines
│   └── vol7_cleanroom_evidence_archive/   # Volume VII: Clean-Room Specifications & Evidence
├── cleanroom/                             # Clean-room test vectors (container-vectors.json, check_vectors.py)
├── system-1/                              # SYSTEM-1 hardware analysis and extraction tools
├── firmware/                              # Official updater archives (v1.30–v1.32)
└── base-engine/                           # Decoded stock engine analysis and exported symbols
```

---

## 7. Clean-Room Boundary & Legal Architecture

This project is conducted under strict **clean-room reverse-engineering standards** (*Sega v. Accolade*, *Sony v. Connectix*) solely to enable interoperability and custom software creation:
- **Zero Redistribution of Proprietary Binaries**: Roland plugin binaries, firmware ROMs, and factory sound banks are excluded from version control.
- **Specification-Driven Implementations**: Clean-room developers operate exclusively from functional requirements ([Volume VII](docs/vol7_cleanroom_evidence_archive/cleanroom_boundary_contract.md)) and golden JSON test vectors.
- **100% Original Code**: All custom synthesis engines (`C-V4`, `C-S1V8`, `C-S1SY`) and SDK runtimes are built from original C and DSP microcode with zero Roland code.
