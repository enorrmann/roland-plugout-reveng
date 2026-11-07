# Volume VII: Clean-Room Archive & Evidence Logs

# Chapter 1: Clean-Room Boundary Specification & Team Separation Contract

> **Document ID**: TRM-BMC-VOL7-001  
> **Classification**: Normative Governance & Procedural Standard  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 1.1 Purpose & Legal Precedent

This document establishes the formal, verifiable operational separation rules governing reverse engineering and clean-room implementation of the Roland BMC synthesis architecture. The methodology adheres strictly to established international clean-room engineering precedents (*Sega v. Accolade*, *Sony v. Connectix*, *NEC v. Intel*) to enable legal interoperability and independent software execution without copyright contamination.

```
+---------------------------------------------------------------------------------------------------+
|                                 CLEAN-ROOM SEPARATION ARCHITECTURE                                |
|                                                                                                   |
|  +-------------------------------------+                +--------------------------------------+  |
|  |     TEAM A: Reverse Engineering     |                |     TEAM B: Clean-Room Authors       |  |
|  |                                     |                |                                      |  |
|  | - Direct Access to Firmware Binaries|                | - ZERO Access to Disassembly/Binaries|  |
|  | - Ghidra / IDA Pro Static Analysis  |                | - Writes 100% Original Code          |  |
|  | - Hardware USB Bus Sniffing         |                | - Consumes ONLY Functional Specs     |  |
|  | - Extracts Architecture & Protocols |                | - Benchmarks Against Test Vectors    |  |
|  +------------------+------------------+                +-------------------+------------------+  |
|                     │                                                       ▲                     |
|                     │ Delivers Draft Specifications                         │ Permitted Delivery  |
|                     ▼                                                       │                     |
|  +--------------------------------------------------------------------------+------------------+  |
|  |                         INDEPENDENT CLEAN-ROOM REVIEWER FIREWALL                             |  |
|  |                                                                                             |  |
|  | - Filters out accidental implementation artifacts, decompiled C pseudocode, and symbol names |  |
|  | - Authorizes only pure functional requirements (REQ-ARCH, REQ-FUNC, REQ-DATA)               |  |
|  | - Maintains immutable audit logs of all information passing across the boundary             |  |
|  +---------------------------------------------------------------------------------------------+  |
+---------------------------------------------------------------------------------------------------+
```

---

## 1.2 Operational Roles & Isolation Boundaries

### 1.2.1 Team A (Analysis & Reverse Engineering)
- **Authorized Assets**: Inspects raw `SYSTEM8_UP.BIN`, `SYSTEM1_UPD.BIN`, plugin binaries (`JUNO-60.vst3`, `JX-3P.vst3`), Ghidra disassembly projects, and hardware USB telemetry.
- **Delivery Constraints**: Team A is legally prohibited from authoring production firmware or DSP microcode. Team A's sole permitted output is normative architectural documentation (`docs/vol1_*` through `docs/vol5_*`) and JSON verification test vectors.

### 1.2.2 The Independent Reviewer (Firewall Guardian)
- Serves as the unidirectional gateway between Team A and Team B.
- Inspects all documentation and specifications passing to Team B.
- **Mandatory Rejection**: Strips any accidental C pseudocode decompilation dumps, internal Roland function addresses, or disassembled microcode instructions.

### 1.2.3 Team B (Clean-Room Implementation & Synthesis)
- **Contamination Restrictions**: Team B personnel must NEVER inspect Roland binary executables, Ghidra disassembly projects, or Team A laboratory scratchpads.
- **Authorized Sources**: Works exclusively from approved specifications (`docs/vol1_*` through `docs/vol6_*`) and test vectors (`cleanroom/test-vectors/*.json`).
- **Verifiable Provenance**: All code authored by Team B must maintain an auditable decision log recording the functional requirement ID justifying each implementation decision.

---

## 1.3 Contaminated vs Authorized Materials Matrix

| Asset Description | Classification | Team A Access | Team B Access | Handling Rule |
|---|---|---|---|---|
| `plugins/**/*.vst3`, `*.dat`, `unins000.exe` | **Contaminated** | Read-Only | **PROHIBITED** | Third-party copyrighted binaries. Must never enter Team B workspaces. |
| `SYSTEM8_UP.BIN`, `SYSTEM1_UPD.BIN` | **Contaminated** | Read-Only | **PROHIBITED** | Original factory firmware. |
| Raw Ghidra Projects & IDA Databases | **Contaminated** | Full Access | **PROHIBITED** | Analysis intermediate scratchpads. |
| `docs/vol1_*` to `docs/vol5_*` Specifications | **Authorized** | Authoring | **Full Access** | Abstract functional specifications approved by Reviewer. |
| `cleanroom/test-vectors/*.json` | **Authorized** | Authoring | **Full Access** | Numeric hashes, header byte lengths, golden vectors. |
| Physical Synthesizer Hardware (S8 / S1) | **Authorized** | Full Access | **Full Access** | Black-box testing and protocol validation. |

---

## 1.4 Cryptographic Verification & Signature Auditing (`emu/sigcheck.py`)

Prior to releasing any clean-room binary or microcode package (e.g. `C-V4` or `C-S1V8`), the build must pass automated cryptographic signature scanning (`emu/sigcheck.py`):
1. **N-Gram Matching**: Scans target binaries against a multi-megabyte corpus of original Roland firmware binaries across all 16-bit, 32-bit, and 64-bit word windows.
2. **Binary Zero-Overlap Threshold**: Rejects any clean-room build containing contiguous sequences of instruction words matching proprietary Roland microcode.
