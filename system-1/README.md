# SYSTEM-1 Research Subsystem & Hardware Analysis

This directory contains research scripts, analysis tools, and historical records for the **Roland SYSTEM-1** hardware synthesizer (specifically the physical hardware host, distinct from SYSTEM-8 plug-out engines).

All normative hardware, memory, voice scheduling, and microcode specifications have been integrated into the **[Roland BMC Technical Reference Manual Suite](../docs/index.md)**.

---

## 1. Technical Reference Manual (TRM) Cross-Reference

| Subject / Domain | Normative TRM Chapter | Key Topics Covered |
|:---|:---|:---|
| **SoC Architecture & Clocks** | [Volume I: SoC Overview](../docs/vol1_system_architecture/soc_overview.md) | Dual-BMC topology (Master + 1 Slave), Cortex-M3 cores, 96 kHz frame timing |
| **DSP Data Banks & Memory Map** | [Volume I: Memory Map](../docs/vol1_system_architecture/memory_map.md) <br> [Volume II: Memory Banks & DMA](../docs/vol2_dsp_core_isa/memory_banks_dma.md) | Bank 0–7 sematics, delay buffer measurements, OS parameter regions |
| **Audio Routing & FX Chain** | [Volume I: Bus Interconnect](../docs/vol1_system_architecture/bus_interconnect.md) | Slave A $\to$ Slave B $\to$ IOMD `0x6006`–`0x600D` $\to$ Master B $\to$ `0xA118`/`0xA119` $\to$ OS `0x6000`/`0x6001` |
| **Voice Allocation (4, 6, 8 Voices)** | [Volume III: SYSTEM-1 Voice Allocation](../docs/vol3_voice_allocation_scheduling/system1_voice_allocation.md) | Host note dispatch (`0x110+v`, `0x120+v`), Bank 5 mailbox, voice stealing, 6-voice feasibility |
| **Timing & Cycle Budgets** | [Volume III: Execution Timing Budgets](../docs/vol3_voice_allocation_scheduling/execution_timing_budgets.md) | 703–826 cycle headroom per 96 kHz frame; JX-3P / SH-101 / PROMARS cycle budgets |
| **Firmware & OS Jump Table** | [Volume IV: Container Formats](../docs/vol4_firmware_container_protocol/container_formats.md) <br> [Volume IV: OS Service Table](../docs/vol4_firmware_container_protocol/os_service_table.md) | Unencrypted `SYSTEM1_UPD.BIN` 1.30, `Code1.Dat` 1.20, `FC30`/`FC44` loader syscalls |
| **Microcode & C Compiler** | [Volume V: s1cc Compiler Internals](../docs/vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md) | LLVM 19 backend targeting dual-DSP BMC cores |
| **Reference Engines** | [Volume VI: C-S1V8 Reference Design](../docs/vol6_reference_designs_applications/c_s1v8_system1_reference.md) | Verified 8-voice native plug-out engine on physical hardware |

---

## 2. Research & Extraction Scripts

All scripts are executed from the repository root directory. They require `firmware/sys1_sys_v130.zip` or plug-in assets in `plugins/SYSTEM-1/`:

| Script | Purpose & Functionality |
|:---|:---|
| `python3 system-1/extract_fw.py` | Decompresses updater images v1.30 and v1.20 into `work/sys1_v130/` and `work/sys1_v120/`. |
| `python3 system-1/voice_sets.py [IMAGE] [--entries]` | Catalogs slave DSP program sets and computes execution cycle costs under 4-voice and 6-voice configurations. |
| `python3 system-1/fnmatch.py A B 0xADDR:N …` | Matches functions between firmware revisions A and B, masking relative PC branch offsets. |
| `python3 system-1/dsp_fit.py [--voices 2] [...]` | Simulates DSP instruction cycles and inter-core transfers for JX-3P microcode on SYSTEM-1. Requires `examples/from_scratch/engine/build/programs.h`. |
| `python3 system-1/s1_progs.py [DIR…] [--listing]` | Disassembles and budgets plug-out DSP microcode (SH-101, SH-2, PROMARS), profiling IOMD link requirements. |
| `python3 system-1/vfp_count.py [IMAGE…]` | Scans ARM binaries for Vector Floating Point (VFP) opcodes (used in SYSTEM-8, absent in SYSTEM-1). |
| `python3 system-1/asgn_sites.py [IMAGE] [START END]` | Locates voice allocator branch target instructions containing hardcoded 4-voice limits in the master firmware. |

---

## 3. Reference Implementations

- **C-S1V4 / C-S1V8**: [examples/system_1/](../examples/system_1/README.md) — 4-voice and 8-voice native engines running on hardware.
- **C-S1RAMP**: [examples/system_1_ramp/](../examples/system_1_ramp/README.md) — Parameter slew rate and hardware ramp profiling.
- **C-S1SY**: [examples/system_1_synth/](../examples/system_1_synth/README.md) — Custom dual-DCO synthesizer engine with filter and envelope modulation.
- **C-S1JX**: [examples/system_1_jx/](../examples/system_1_jx/README.md) — JX-3P microcode relocated across 4 DSP slots on SYSTEM-1.
- **MUTABLE-SYSTEM**: [examples/mutable_system/](../examples/mutable_system/README.md) — Mutable Instruments Braids synthesis models ported via `s1cc`.
- **SYSTEM-S1CC**: [examples/system_s1cc/](../examples/system_s1cc/README.md) — Minimal runtime microkernel for pure `s1cc` DSP compilation.

---

## 4. Hardware Status & Measured Findings

- **Firmware Status**: Firmware v1.30 (`SYSTEM1_UPD.BIN`) runs on hardware unencrypted, eliminating the need for SysEx memory dumps.
- **Voice Polyphony**:
  - Stock engine is architected for 4 voices.
  - Custom engine **C-S1V8 verified at 8 voices on physical hardware**; host firmware passes voice indices 4–7 transparently across Bank 5 mailbox `0xA000`–`0xA1FF`.
  - Theoretical 6-voice stock modification feasible within ~740 cycles (out of 925 budget).
- **DSP Core Topology**: Each Roland BMC ASIC contains two DSP cores (DSP A at `FC30`, DSP B at `FC34`). Queues `FC38` and `FC3C` provide data streaming.
