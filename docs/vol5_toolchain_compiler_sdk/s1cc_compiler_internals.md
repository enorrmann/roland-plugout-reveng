# Volume V: Toolchain, Compiler & SDK Specification

# Chapter 1: The `s1cc` Compiler Architecture & Graph Lowering

> **Document ID**: TRM-BMC-VOL5-001  
> **Classification**: Normative Toolchain Specification  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 1.1 Toolchain Purpose & Pipeline Overview

The **`s1cc`** compiler is an optimizing high-level compiler pipeline developed to translate modern, standard C and C++ audio synthesis algorithms into bit-exact, hazard-free VLIW microcode for the proprietary Roland BMC DSP architecture.

The compilation pipeline operates as a multi-stage transformation:

```
  +-----------------------------------------------------------------------------------------------+
  |                                   s1cc COMPILER COMPILATION PIPELINE                          |
  |                                                                                               |
  |   C / C++ DSP Source (e.g. Plaits / Braids / Custom Synth Kernel)                             |
  |   Includes: <s1dsp.h>, S1_DSP_A, S1_DSP_B, s1_pitch(), s1_wrap()                             |
  |                                  │                                                            |
  |                                  ▼                                                            |
  |   [ STAGE 1: Clang / LLVM 19 Frontend ]                                                       |
  |   - Flags: -O3 -fno-slp-vectorize -target arm-none-eabi                                       |
  |   - Output: Standard LLVM Intermediate Representation (LLVM IR .ll)                           |
  |                                  │                                                            |
  |                                  ▼                                                            |
  |   [ STAGE 2: s1cc Graph Lowering & Optimization (lower.cpp / kernel.cpp) ]                    |
  |   - Constant Expression Folding & Table Identification                                        |
  |   - Automated Function Fitting (Horner Polynomial Conversion)                                 |
  |   - Native SFU Pattern Matching (recipseed, log2split, expsplit, ph_wrap)                     |
  |   - Directed Acyclic Graph (DAG) Expression Lowering                                          |
  |                                  │                                                            |
  |                                  ▼                                                            |
  |   [ STAGE 3: Register Allocation (RegAlloc) ]                                                 |
  |   - Graph Coloring / Live-Range Analysis across physical registers r0 - r7                    |
  |   - Partitioned Symmetrical Polyphonic Banks: r0-r3 (Voice A) & r4-r7 (Voice B)               |
  |                                  │                                                            |
  |                                  ▼                                                            |
  |   [ STAGE 4: VLIW Slot Scheduling & Hazard Verification (dspsched.py) ]                       |
  |   - Instruction Fusion (ALU A + ALU B + MAC Bundling)                                         |
  |   - Enforcement of Roland Hardware Access Rules R2 through R10                                |
  |   - Bank 0 Ring Pointer Tracking & Latency Slot Insertion                                     |
  |                                  │                                                            |
  |                                  ▼                                                            |
  |   Final Output: 16-bit VLIW Assembly / Raw Binary Microcode (.hex / .bin)                     |
  +-----------------------------------------------------------------------------------------------+
```

---

## 1.2 LLVM IR Lowering & Pattern Matching (`lower.cpp`)

The lowering stage inspects the LLVM IR Direct Acyclic Graph (DAG) to map high-level floating-point arithmetic onto native BMC hardware units:

### 1.2.1 Automated Native Opcode Selection
- **Reciprocal & Division ($x / y$)**: Recognized as $x \cdot (1/y)$. Lowers automatically to `recipseed` followed by `newton` quadratic convergence steps rather than software emulation loops.
- **Transcendental Functions (`exp2f`, `expf`, `log2f`)**: Lowers directly to `expsplit` and `log2split` SFU operations combined with 4th-to-6th degree Chebyshev polynomial evaluators.
- **Modulo Phase Wrapping**: Floating-point phase accumulation followed by conditional boundaries is folded into single-cycle `ph_wrap` instructions.
- **Power-of-Two Scalings**: Multiplications by large powers of two ($x \cdot 2^k$) are lowered to hardware `shl` / `shr` instructions.

### 1.2.2 Polynomial Fitting for Run-Time Lookup Tables
Array lookups inside inner audio loops ($y = \text{LUT}[i]$) that use computed fractional indices pose severe hazards on DSP architectures without multi-channel cache. `s1cc` detects `const` lookup tables and fits them at compile time into high-order polynomial expansions (Horner scheme), eliminating memory loads in favor of pipelined `mac_p1p2` chains.

---

## 1.3 Register Allocation & Physical Constraints

The BMC DSP architecture provides **eight 32-bit universal accumulators (`r0` through `r7`)**.

- **Polyphonic Partitioning**: To compute multiple polyphonic voices within the single-cycle execution budget, `s1cc` partitions the register file into two symmetrical register pairs:
  - **Register Bank 0**: `r0`, `r1`, `r2`, `r3` (Allocated to Voice Chain 0).
  - **Register Bank 1**: `r4`, `r5`, `r6`, `r7` (Allocated to Voice Chain 1).
- **Spill Management**: If the live-range analyzer detects register pressure exceeding 4 registers per voice, variables are temporarily spooled to scratchpad locations within the **Bank 0 Ring Buffer (`DT0`)**. Because Bank 0 shifts by 1 word every sample, spooled variables are dynamically referenced with frame offsets.

---

## 1.4 VLIW Slot Scheduling & Hazard Checking (`dspsched.py`)

The scheduling pass enforces strict compliance with the Roland hardware access constraints:

1. **Instruction Fusion**: Combines independent arithmetic operations and memory transfers into single multi-unit VLIW instructions:
   ```dsp
   ; Pipelined VLIW Instruction Scheduled by dspsched.py
   addxy r1, r1, r2 | ph_wrap r3, r3, r3 | st r1, [0300]
   ```
2. **Access Barrier Enforcement**:
   - Strictly enforces Rule R2 (At most 1 load and 1 store to Bank 5 per instruction).
   - Strictly enforces Rule R3 (At most 2 loads from Bank 0 per instruction).
   - Verifies against `asm/bad_forms.txt` to prevent illegal parallel unit pairings.
3. **Load-to-Use Latency Padding**: Analyzes dependent register chains. If an instruction attempts to consume register $r_X$ immediately following a `ld rX, [addr]` transfer, the scheduler moves an independent operation into the delay slot or inserts a benign single-cycle padding operation (`mov r4, r4`).
