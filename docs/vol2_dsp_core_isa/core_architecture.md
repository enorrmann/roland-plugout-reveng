# Volume II: Roland BMC DSP Core & ISA Reference

# Chapter 1: DSP Core Pipeline, Register File & Processor State

> **Document ID**: TRM-BMC-VOL2-001  
> **Classification**: Normative Hardware Reference  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 1.1 Core Architecture Overview

The **Roland BMC DSP** is a custom 32-bit high-throughput Very Long Instruction Word (VLIW) digital signal processor core engineered specifically for zero-latency, hard-real-time audio synthesis algorithms running at a fixed frame rate of $F_s = 96.000\text{ kHz}$.

Unlike general-purpose embedded DSPs (such as TI C6000 or ADI SHARC), the Roland BMC DSP core operates under a deterministic, statically scheduled **run-to-completion** microcode execution paradigm:

- **Zero Pipeline Interrupts**: The processor pipeline contains no hardware interrupts, preemptive multitasking, or software trap exceptions.
- **Deterministic Single-Cycle Instruction Cost**: Every instruction word executes in exactly **1 instruction cycle**, regardless of whether it triggers a single scalar arithmetic operation, a dual-accumulator multiply-accumulate (MAC), Special Function Unit (SFU) reciprocal approximations, or up to 6 parallel memory transfers.
- **Synchronous Sample Loop**: At every 96 kHz audio sample pulse ($T_s = 10.4166\ \mu\text{s}$), the program counter (PC) restarts execution at address `0x0100` (Main Program) and processes through active slot/section microcode.
- **Shared Processor State Across Program Slots**: Multiple sub-programs (Main Program, DCO1 Slot, DCO2 Slot, FX Slot) run sequentially within a single audio sample frame. All active slots share the same physical register file (`r0`–`r7`), condition flags, and hidden registers without automatic context saving or clearing.

```
+---------------------------------------------------------------------------------------------------+
|                                 ROLAND BMC DSP CORE ARCHITECTURE                                  |
|                                                                                                   |
|  +---------------------------------------------------------------------------------------------+  |
|  |                             Instruction Pipeline (1 Cycle / Inst)                           |  |
|  |                                                                                             |  |
|  |     [ FETCH ]  ========>  [ DECODE / SLOT ]  ========>  [ PARALLEL EXECUTE & WRITEBACK ]    |  |
|  |   Program Counter        VLIW Class Template           ALU Unit A (Scalar Arith / Newton)   |  |
|  |   (64 KiB / 32 KiB)      1 to 31 Words Length          ALU Unit B (SFU / Logic / Phase)     |  |
|  |                          Immediate Slot Unpack         Dual MAC Unit (Parallel Products)    |  |
|  |                                                        Transfer Units T0..T5 (Loads/Stores) |  |
|  +---------------------------------------------------------------------------------------------+  |
|                 │                                                            │                    |
|                 ▼                                                            ▼                    |
|  +------------------------------+                           +----------------------------------+  |
|  |    Visible Register File     |                           |    Condition Code & Status       |  |
|  |                              |                           |                                  |  |
|  |  r0: 32-bit Roland RAW Acc   |                           |  32 Condition Bits               |  |
|  |  r1: 32-bit Roland RAW Acc   |                           |  (8 Groups x 4 Flags)            |  |
|  |  r2: 32-bit Roland RAW Acc   |                           |  Group 0: Fixed 0111b            |  |
|  |  r3: 32-bit Roland RAW Acc   |                           |  Group 1-5: Settable by X9bY     |  |
|  |  r4: 32-bit Roland RAW Acc   |                           |  Group 6-7: Fixed 1111b          |  |
|  |  r5: 32-bit Roland RAW Acc   |                           +----------------------------------+  |
|  |  r6: 32-bit Roland RAW Acc   |                                            │                    |
|  |  r7: 32-bit Roland RAW Acc   |                           +----------------------------------+  |
|  +------------------------------+                           |    Hidden Architecture Regs      |  |
|                 │                                           |                                  |  |
|                 ▼                                           |  h4: Delay Base Address / Sum 4  |  |
|  +-------------------------------------------------------+  |  h5: Delay Tap Address / Sum 5   |  |
|  |           Memory Interface (Banks 0 - 7)              |  |  h6: Indexed Base / Ring Offset  |  |
|  |   Bank 0 (DT0 Ring), Bank 1 (DT1), Bank 2 (DT2 DMA),  |  |  h7: Auxiliary Pointer Reg       |  |
|  |   Bank 3 (IOMD), Bank 4 (MXM), Bank 5 (DPC), Bank 6   |  +----------------------------------+  |
|  +-------------------------------------------------------+                                        |
+---------------------------------------------------------------------------------------------------+
```

---

## 1.2 Execution Pipeline & Timing Characteristics

The pipeline executes instructions sequentially with strict single-cycle latency and non-interlocked operand forwarding:

### 1.2.1 Read-Before-Write Execution Semantics `[VERIFIED]`

Within any single instruction word containing multiple parallel operations (e.g. Unit A ALU, Unit B ALU, and parallel transfers):
- **All units read source registers before any unit writes back its destination register**.
- If an instruction specifies:
  $$\text{Unit A: } r_4 \leftarrow r_3 + r_1 \quad \parallel \quad \text{Unit B: } r_3 \leftarrow r_0 \cdot 2.0$$
  Unit A reads the *previous* contents of $r_3$ from the preceding instruction.
- **Store Operand Latch**: If an instruction issues a store operation $\text{st } r_X, [\text{address}]$ simultaneously with an arithmetic writeback to $r_X$, the memory store captures the *old* value of $r_X$.

### 1.2.2 Load-to-Use Latency (1-Cycle Delay Slot) `[VERIFIED]`

Memory load instructions (`ld rX, [addr]`) exhibit an architectural **load-to-use latency of 1 cycle**:
- A register targeted by a load transfer is **not updated until the end of the subsequent instruction cycle**.
- Attempting to consume $r_X$ in the instruction immediately following its load yields either stale register data or pipeline bubbles.
- Compilers and microcode authors must schedule independent arithmetic or insert a single-cycle filler (`mov r4, r4` or `nop`) between a load and its first dependent operation:

```dsp
; Correct Load-to-Use Scheduling Pattern
ld r1, [a021]      ; Load initiated from Bank 5 (r1 will be ready at cycle N+2)
mov r4, r4         ; 1-cycle latency padding slot (or independent operation)
st r1, [8404]      ; r1 is now fully valid and latched into MXM output
```

### 1.2.3 Branch & Jump Execution Latency `[MEASURED]`

The BMC DSP core supports PC-relative branch operations (`jmp` / `0x009Y`):
- **Taken Branch**: Consumes **2 execution cycles** (1 instruction cycle + 1 pipeline refill cycle).
- **Fall-Through / Untaken Conditional Branch**: Consumes **1 execution cycle**.
- **Skipped Instructions**: Instructions bypassed via branch jumps or condition (`cond`) predication masks consume **0 cycles** from the execution time budget.

---

## 1.3 Visible Register File (`r0` through `r7`) `[VERIFIED]`

The visible register file consists of **eight 32-bit universal accumulators / registers**, designated `r0` through `r7`:

```
  31  30 29  28                                                    0
┌────┬──────┬───────────────────────────────────────────────────────┐
│ S  │  s   │ Mantissa M (29-bit Unsigned Fraction)                 │
└────┴──────┴───────────────────────────────────────────────────────┘
```

- Every register stores numbers in the Roland 32-bit Custom Floating-Point ("RAW") format ($S = \text{Sign}$, $s = \text{Exponent}$, $M = \text{Mantissa}$).
- **Transfer Gating**: Only registers `r0`–`r7` can be directly loaded from or stored to memory banks by Transfer (`T`) units.
- **Even/Odd Register Pairing**: High-throughput polyphonic kernels (such as `s1cc` and `base-engine`) partition the register file into two symmetrical processing banks:
  - Bank Lower: `r0`, `r1`, `r2`, `r3` (Allocated to Voice Pair A).
  - Bank Upper: `r4`, `r5`, `r6`, `r7` (Allocated to Voice Pair B).
- **Operand Inversion (Bit 3 Negation)**: Arithmetic operand fields specifying register $Y$ implement automatic algebraic negation when bit 3 is asserted ($Y = 8 + n \implies -r_n$).

---

## 1.4 Hidden Architectural Registers (`h4` through `h7`) `[MEASURED]`

In addition to `r0`–`r7`, the DSP core maintains four internal 32-bit architectural registers designated `h4`, `h5`, `h6`, and `h7`:

| Hidden Register | Hardware Role | Programmatic Read Access | Programmatic Write Access | Evidence |
|---|---|---|---|---|
| **`h4`** | Delay Line Base Address / Accumulator Base | Additive operand in ALU `addh4` (`A op 0 sub 4`) | Loaded via `40TY` command word (`hset h4`) | `[MEASURED]` |
| **`h5`** | Delay Tap Destination Address / Aux Base | Additive operand in ALU `addh5` (`A op 0 sub 5`) | Loaded via `40TY` command word (`hset h5`) | `[MEASURED]` |
| **`h6`** | Indexed Memory Offset Base Register | Address offset generator for `ldidx` (`T 0x28`) | Loaded via `40TY` command word (`hset h6`) | `[MEASURED]` |
| **`h7`** | Auxiliary Index Base Pointer | Address generator for secondary Bank 5 indexing | Loaded via `40TY` command word (`hset h7`) | `[MEASURED]` |

### 1.4.1 Configuration via `40TY` Directives `[MEASURED]`

Hidden registers are not targeted by regular `st` transfers. They are configured via dedicated 16-bit command halfwords placed in the Condition/Control (`C`) slot of an instruction word:

```
  15       12 11   8 7     4 3     0
┌────────────┬──────┬───────┬───────┐
│    0x4     │  0   │   T   │   Y   │
└────────────┴──────┴───────┴───────┘
```

- When $T = 0$: Loads register `h[4 + (Y & 3)]` from scalar register $r_Y$ or from an immediate slot.
- **Pipeline Setup Latency**: When loading an index register (`hset h6`), at least **one instruction cycle must elapse** before executing an indexed transfer (`ldidx`) that references the new base pointer.

---

## 1.5 Condition Codes & Status Flag Architecture `[MEASURED]`

The DSP core maintains a 32-bit status register partitioned into **8 Condition Groups** containing 4 hardware flags each:

```
  31         28 27         24 23         20 19         16 15         12 11          8 7           4 3           0
┌──────────────┬──────────────┬──────────────┬──────────────┬──────────────┬──────────────┬──────────────┬──────────────┐
│   Group 7    │   Group 6    │   Group 5    │   Group 4    │   Group 3    │   Group 2    │   Group 1    │   Group 0    │
│ (Fixed 1111) │ (Fixed 1111) │ (User / Dyn) │ (User / Dyn) │ (User / Dyn) │ (User / Dyn) │ (User / Dyn) │ (Fixed 0111) │
└──────────────┴──────────────┴──────────────┴──────────────┴──────────────┴──────────────┴──────────────┴──────────────┘
```

### 1.5.1 Flag Bit Semantics

Within each 4-bit group $G_k$, bits represent standardized arithmetic comparison and overflow states:

| Flag Bit Offset | Flag Name | Mnemonic | Hardware Condition |
|---|---|---|---|
| Bit 0 | Zero Flag | `Z` | Asserted if arithmetic result magnitude $|v| = 0.0$. |
| Bit 1 | Negative Flag | `N` | Asserted if sign bit $S = 1$ ($v < 0.0$). |
| Bit 2 | Overflow Flag | `V` | Asserted if arithmetic result exceeded dynamic range $|v| \ge 512.0$. |
| Bit 3 | Carry / Comparison | `C` | Asserted on unsigned wrap or comparison predicate match. |

### 1.5.2 Group Behavior & Immutability

- **Group 0 (Bits 3:0)**: Read-Only hardwired constant state `0111b` (Carry = 0, Overflow = 1, Negative = 1, Zero = 1). Used for unconditional truth evaluations.
- **Groups 1 through 5 (Bits 23:4)**: Dynamically updated via the `setflags` / `X9bY` transfer operation. A calculation unit (ALU Unit A, Unit B, or MAC) writes its comparison flags into group $X - 8$.
- **Groups 6 and 7 (Bits 31:24)**: Read-Only hardwired constant state `1111b`.

### 1.5.3 Predicated Execution via `cond` Word `[VERIFIED]`

An instruction containing a Condition Word (`C`) evaluates condition bit $cc \in [0, 31]$. If the evaluated bit matches the programmed polarity, selected execution units are conditionally suppressed without altering instruction cycle timing:

```
C Word: [ 00 | cc[4:0] | neg[0] | skip_true[3:0] | skip_false[3:0] ]
```

- `skip_true`: Identifies which processing unit (ALU 1, ALU 2, or Transfer) is suppressed when condition evaluates True.
- `skip_false`: Identifies which processing unit is suppressed when condition evaluates False.
