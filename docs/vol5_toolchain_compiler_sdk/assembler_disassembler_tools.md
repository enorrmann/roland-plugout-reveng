# Volume V: Toolchain, Compiler & SDK Specification

# Chapter 4: Developer Toolchain, Assembler, Disassembler & Utilities

> **Document ID**: TRM-BMC-VOL5-004  
> **Classification**: Normative Toolchain Manual  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 4.1 Master Toolchain Architecture

The reverse-engineering and clean-room development suite includes standalone command-line tools to assemble microcode, inspect binary firmware images, schedule VLIW instructions, and communicate with physical Roland hardware:

```
+---------------------------------------------------------------------------------------------------+
|                                 BMC DSP REVERSE-ENGINEERING TOOLCHAIN                             |
|                                                                                                   |
|  [ Assembly & Disassembly Layer (asm/) ]                                                          |
|  - asm.py        : Roland BMC DSP Macro Assembler (Translates dspasm to 16-bit words)             |
|  - disasm.py     : High-Fidelity Microcode Disassembler (Bit-exact round-trip verification)       |
|  - dspsched.py   : Optimizing VLIW Instruction Scheduler & Slot Packing Pipeline                  |
|  - s1rules.py    : Hardware Rule Linter & Access Barrier Verifier (R2 through R10)                |
|                                                                                                   |
|  [ Firmware & Packaging Layer (tools/) ]                                                          |
|  - plugout_tool.py : Plug-Out Installer, Container Packer, and SysEx USB Communications           |
|  - sys8fw.py       : Firmware Update Archive Inspector and LZSS Decompressor                      |
|  - doccheck.py     : TRM Technical Documentation Assembly Syntax Verifier                         |
|                                                                                                   |
|  [ Simulation & Emulation Layer (emu/) ]                                                          |
|  - dspref.py / emu/dspdec.h : Golden C Simulation Model of the Roland BMC DSP Pipeline            |
+---------------------------------------------------------------------------------------------------+
```

---

## 4.2 `asm.py` — Roland BMC DSP Macro Assembler `[VERIFIED]`

The assembler translates human-readable DSP assembly listings into binary word sequences matching the exact execution format of the BMC DSP:

```bash
python3 asm/asm.py input.s -o output.bin
```

### 4.2.1 Assembler Directives Catalog

| Directive | Syntax | Functional Description |
|---|---|---|
| `.class` | `.class 0x0040` | Explicitly forces the class template for the subsequent instruction (Used when multiple templates fit the unit signature). |
| `.slot` | `.slot @k 0xNNNN` | Injects raw 16-bit word `0xNNNN` into slot index $k$ (For words unreferenced by active operands). |
| `.free` | `.free @n 0xMASK` | Restores unused operand bits cleared by standard variant encodings. |
| `.words` | `.words 0xNNNN ...`| Injects literal raw 16-bit words into the stream (Used for unknown trailing blocks). |

### 4.2.2 Immediate Number Syntax
- Standard Floating-Point Immediate: `#0.5`, `#-1.0`, `#3.14159`.
- Explicit Width Specifier: `#0.5:16` (16-bit mode), `#0.5:24` (24-bit mode), `#0.5:32` (32-bit mode).
- Byte Offset Alignment: `#0.5@4:16` (Forces 16-bit immediate placement at byte offset 4 of the slot array).

```dsp
; Example Valid Assembler Listing
.class 0x0048
mov r0, #0.5:16
addxy r1, r1, r2 | st r1, [0300]
```

---

## 4.3 `disasm.py` — Microcode Disassembler `[VERIFIED]`

Disassembles raw DSP microcode binaries into source text that re-assembles to the identical binary word sequence (100% round-trip verified on all 128 firmware programs):

```bash
python3 asm/disasm.py program.bin --class-table asm/isa.json
```

- Automatically tiles variable-length VLIW instructions by parsing the 11-bit class and 5-bit length from every header word.
- Maps memory references into human-readable bank notation (`[0400]`, `[A021]`, `[6020]`).

---

## 4.4 `dspsched.py` — VLIW Packing & Scheduling Engine `[VERIFIED]`

Takes linear, unbundled DSP microcode instructions (e.g. emitted by compiler lowering passes) and bundles them into dense, multi-operation VLIW instructions:

```bash
python3 asm/dspsched.py unoptimized.s -o scheduled.s
```

- **Pass 1: Dependency Analysis**: Computes Directed Acyclic Graphs of register read/write dependencies and load latencies.
- **Pass 2: Greedy Slot Packing**: Bundles compatible ALU, SFU, and Transfer units into known hardware classes (`re/dsp_templates.txt`).
- **Pass 3: Access Rule Linting**: Enforces rules R2 through R10 via `asm/s1rules.py`. Rejects illegal combinations with actionable error diagnostics.

---

## 4.5 `plugout_tool.py` — Plug-Out Management Utility `[VERIFIED]`

A command-line tool to query, inspect, unpack, repack, and install Plug-Out containers directly onto physical Roland instruments over USB-MIDI:

### 4.5.1 Command Reference

```bash
# Query attached hardware, firmware version, and occupied Plug-Out slots
python3 tools/plugout_tool.py info

# Install a Plug-Out package into physical Slot 1 (Auto-confirms, bypasses patch overwrite)
python3 tools/plugout_tool.py install plugins/PROMARS --slot 1 --force --no-patches -y

# Unpack a Code*.Dat container into raw decrypted ELF binaries and microcode blobs
python3 tools/plugout_tool.py unpack Script/Code1.Dat -d output_dir/

# Repack modified binaries into an encrypted Code*.Dat distribution package
python3 tools/plugout_tool.py pack input_dir/ -o Script/Code1.Dat --model-id 0x0001
```

---

## 4.6 `doccheck.py` — Technical Documentation Syntax Validator `[VERIFIED]`

Automated quality-assurance utility designed to maintain absolute mathematical and syntactic verifiability across the repository's markdown manuals:

```bash
python3 tools/doccheck.py docs/vol*/*.md
```

- Extracts every fenced code block tagged with ` ```dsp `.
- Assembles code blocks one by one using the internal `asm.Assembler()`.
- Emits non-zero exit codes if any technical document contains invalid, pseudo-, or uncompilable assembly listings.

---

## 4.7 Golden C Simulation & DSP Emulation Core `[VERIFIED]`

The repository provides a bit-exact, single-cycle software emulation core in C (`examples/from_scratch/emu/`):

- **Core Components**:
  - `dspdec.h`: Decoding engine parsing class templates (`dsp_templates.txt`, `dsp_units_ab.txt`), operand references, immediate extraction, and execution condition fields into `Op` / `Cond` structs.
  - `dspcore.h` / `dspcore.c`: High-performance execution core implementing bit-exact arithmetic semantics for every Roland DSP opcode, flags evaluation, and memory bank models.
  - `dspref.py`: Python ctypes binding exposing `dspref.Machine` for test harnesses, verifying against golden datasets (`work/golden_insn.txt`, 15,756 instructions).
  - `cchain.Slave` / `Bmc2`: Emulates clustered multi-DSP slave topologies matching measured physical audio output bit-for-bit.
  - `dspjit.py`: Native JIT translation engine compiling DSP programs directly into host x86-64 machine code for real-time 96 kHz polyphonic simulation.

---

## 4.8 `check_optimized.py` — Microcode Optimization Validator `[VERIFIED]`

A static analysis tool that verifies whether DSP assembly programs adhere to hardware performance rules and optimal fused encodings:

```bash
python3 tools/check_optimized.py examples/system_1_synth/s1m0.s
python3 tools/check_optimized.py --strict FILE...
```

| Warning Diagnostic | Condition Detected | Required Remediation |
|---|---|---|
| `manual-wrap` | Repeated pairs of `mul rX, rX, #256` + `min rX, rX, ...` | Replace with hardware `ph_wrap` / `ph_tri` / `ph_clamp` SFU instructions. |
| `filler` | Isolated `mov rN, rN` padding instructions | Remove padding; schedule operations into parallel slots. |
| `transfer-only` | Instruction word executing only `ld` / `st` transfers | Bundle memory transfers with adjacent arithmetic operations. |
| `fusable` | Sequential independent operations that fit into a single VLIW word | Merge into a single multi-unit instruction line. |

