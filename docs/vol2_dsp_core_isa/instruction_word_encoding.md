# Volume II: Roland BMC DSP Core & ISA Reference

# Chapter 3: Instruction Word Formats, Class Templates & Access Rules

> **Document ID**: TRM-BMC-VOL2-003  
> **Classification**: Normative Hardware Reference  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 3.1 VLIW Instruction Stream & Word Alignment

The Roland BMC DSP core executes a variable-length Very Long Instruction Word (VLIW) instruction stream composed of 16-bit halfword packets:

```
Stream: [ Header ] [ Unit Op 1 ] [ Unit Op 2 ] [ Transfers T0..Tn ] [ Slot Array / Immediates ]
```

### 3.1.1 Memory Packing & Execution Ordering `[VERIFIED]`

In physical program memory, instructions are packed into 32-bit little-endian storage words. The core fetches 32-bit doublewords and processes the **upper halfword (bits 31:16) first**, followed by the **lower halfword (bits 15:0)**:

$$\text{Execution Order: } \text{Halfword } 0 = \text{mem32}[i][31:16], \quad \text{Halfword } 1 = \text{mem32}[i][15:0]$$

---

## 3.2 Instruction Header Format

Every instruction commences with a single 16-bit **Header Word** that dictates the structural template and total word length of the instruction packet:

```
  15                                            5 4                   0
┌────────────────────────────────────────────────┬─────────────────────┐
│                 Class Template                 │     Word Length     │
│                    (11 bits)                   │      (5 bits)       │
└────────────────────────────────────────────────┴─────────────────────┘
```

| Field Name | Bit Range | Width | Binary Encoding | Functional Description |
|---|---|---|---|---|
| **Class** | Bits 15:5 | 11 bits | `class[10:0]` | **Template Identifier**: Defines the syntactic unit assignment for every subsequent word in the instruction body. (139 known templates, 118 verified). |
| **Length** | Bits 4:0 | 5 bits | `length[4:0]` | **Instruction Word Count**: Total number of 16-bit words comprising this instruction packet, **including the header word itself** (Valid range: $1 \le \text{Length} \le 31$). |

- **One-Word NOP (`0x0001`)**: Header with $\text{Class} = 0$, $\text{Length} = 1$. Consumes 1 execution cycle and performs no state change.
- **Header Boundary Rule**: The next instruction header begins immediately at halfword offset $\text{PC} + \text{Length}$.

---

## 3.3 Class Templates & Position Letters

The BMC DSP does NOT utilize self-describing opcode tags on individual words. Instead, **the 11-bit Class acts as a structural blueprint**: the physical position of a 16-bit halfword within the instruction packet entirely determines its operational role.

### 3.3.1 Canonical Position Letters

| Code | Syntactic Role | Unit Type & Description | Section Reference |
|---|---|---|---|
| `A` | ALU Unit A | Scalar Arithmetic / Multiply / Newton Reciprocal | Chapter 4 (§4.2) |
| `B` | ALU Unit B | SFU Operations / Logic / Shift / Phase Distort | Chapter 4 (§4.3) |
| `M` | MAC First Word | Primary Multiply-Accumulate Operator ($w_1$) | Chapter 4 (§4.4) |
| `N` | MAC Second Word | Secondary Accumulator Operator ($w_2$, in Slot Array) | Chapter 4 (§4.4) |
| `C` | Control Word | Predication / Branch Condition / Hidden Reg Load | Chapter 1 (§1.5) |
| `T` | Transfer Word | Memory Load / Store / Flag Latch Direction | Chapter 4 (§4.5) |
| `R` | Reference Word | 16-bit Physical Memory Address within Bank | Chapter 6 (§6.1) |
| `I` | Immediate Word | 16-bit MSB Slice of Immediate Stream | Chapter 2 (§2.6) |

### 3.3.2 Structural Layout Progression

Words inside an instruction packet follow a deterministic structural progression:

```
[ A / B / M / C Operation Units ]  ==>  [ T Transfer Units ]  ==>  [ Slot Array: N words, R words ]  ==>  [ I Immediate Words ]
```

### 3.3.3 Representative Class Template Catalog `[VERIFIED]`

| Class (Hex) | Template Signature | Typical Length | Common Functional Use Case |
|---|---|---|---|
| `0x048` | `A` + Immediate | 2 to 4 words | Scalar load immediate (`mov r0, #imm`), scalar multiply |
| `0x050` | `B` | 2 words | Scalar phase operations (`ph_wrap`, `ph_clamp`, `ph_tri`) |
| `0x0A8` | `A` `A` | 3 words | Parallel dual scalar arithmetic (e.g. `addxy` + `mul`) |
| `0x0B0` | `A` `B` | 3 words | Parallel oscillator phase advance + phase wrapping |
| `0x040` | `M` `N` + Immediate | 3 to 7 words | Dual product multiply-accumulate with polynomial coefficients |
| `0x060` | `T` `R` | 3 words | Single memory load or store (`ld r0, [0400]`) |
| `0x0F0` | `T` `T` `R` `R` | 5 words | Dual parallel loads (`ld r0, [0400] \| ld r1, [0401]`) |
| `0x1F0` | `T` `T` `T` `R` `R` `R` | 7 words | Triple parallel memory transfers |
| `0x3C0` | `T` `T` `T` `T` `R` `R` `R` `R` | 9 words | Quad parallel memory transfers |
| `0x0A0` | `M` `T` `N` `R` | 4 words | Parallel MAC calculation and simultaneous memory load |
| `0x158` | `M` `T` `T` `N` `R` `R` + Imm | 9 words | Biquad filter core: Parallel MAC + dual delay line transfers |
| `0x168` | `A` `A` `C` + Immediate | 6 words | Dual arithmetic with conditional predication skip |

---

## 3.4 The Slot Array & Immediate Packing Logic

The **Slot Array** encompasses all words positioned after the final Transfer (`T`) unit:

1. **MAC Secondary Words (`N`) First**: If the instruction contains $k$ MAC units, the first $k$ words of the slot array are dedicated to the $N$ halfwords ($w_2$).
2. **Transfer Reference Addresses (`R`)**: Memory transfers with mode byte `0xFE` consume memory reference words within the slot array. A transfer at position $T_m$ points to slot index $Y \gg 1$.
3. **Continuous Immediate Bit Stream (`I`)**: Immediate numerical values are packed MSB-first across unreferenced slot positions:
   - An ALU operation with immediate mode reads its operand starting **$Y$ bytes** into the slot array ($Y = \text{field } Y$).
   - A MAC operation reads operand $Y$ at byte offset $Y$, its polynomial coefficient $C$ immediately following $Y$, and operand $W$ at byte offset $W$.

---

## 3.5 Hardware Access Rules & Constraints (R2 through R10)

Exhaustive on-device testing and bisection (`asm/s1rules.py`, `docs/MICROCODE_OPTIMIZATION.md`) establish rigid hardware access boundaries. Violating any of these rules causes immediate DSP execution stalls, phase loss, or hardware freeze:

### Rule R2: Bank 5 Access Barrier `[MEASURED]`
Within any single instruction word, a DSP core may issue **at most one Bank 5 load and one Bank 5 store**:
$$\text{Count}(\text{ld}[\text{Bank 5}]) \le 1 \quad \text{and} \quad \text{Count}(\text{st}[\text{Bank 5}]) \le 1$$

### Rule R3: Bank 0 Access Limit `[MEASURED]`
Within any single instruction word, a DSP core may issue **at most two loads and two stores to Bank 0 (DT0)**:
$$\text{Count}(\text{ld}[\text{Bank 0}]) \le 2 \quad \text{and} \quad \text{Count}(\text{st}[\text{Bank 0}]) \le 2$$

### Rule R4: Bank 6 Parameter Read Limit `[MEASURED]`
Within any single instruction word, a DSP core may issue **at most two loads from Bank 6 (Parameters)**:
$$\text{Count}(\text{ld}[\text{Bank 6}]) \le 2$$

### Rule R5: Conservative Single Bank-5 Access on System-1 `[INFERRED]`
On the SYSTEM-1 platform, custom microcode compilers must restrict Bank 5 operations to **at most one access (load OR store) per instruction**:
$$\text{Count}(\text{ld}[\text{Bank 5}]) + \text{Count}(\text{st}[\text{Bank 5}]) \le 1$$

### Rule R6: Aggregate Interconnect Bandwidth Limit `[INFERRED]`
Within any single instruction word, the aggregate number of loads from Bank 5, Bank 6, and IOMD cannot exceed 2:
$$\text{Count}(\text{ld}[\text{Bank 5}]) + \text{Count}(\text{ld}[\text{Bank 6}]) + \text{Count}(\text{ld}[\text{IOMD}]) \le 2$$

### Rule R7: Slave Bank 5 Write Window `[VERIFIED]`
On Slave BMC cores, DSP microcode may store to Bank 5 **only within the window `0xA100`–`0xA1FF`**. The lower window (`0xA000`–`0xA0FF`) is reserved for ARM parameter injection via service `FD6C`.

### Rule R8: Proven Operation Combination Groups `[MEASURED]`
Parallel operations (ALU + ALU, ALU + SFU, MAC + ALU) executing within the same instruction word must belong to verified safe combination sets (`asm/proven_forms.txt`). 

### Rule R9: Prohibited Conflict Pairs `[VERIFIED]`
Microcode must NEVER bundle conflicting operation families into the same instruction:
- Two Unit B operations of the same mathematical family (e.g. `min` + `max`) in parallel.
- A MAC operation with an immediate coefficient alongside another parallel arithmetic unit.

### Rule R10: Bank 5 Hardware Mirroring Boundary `[MEASURED]`
Slave DSP cores must never access Bank 5 addresses at or above `0xA200`. While the simulator models Bank 5 as 1,024 words, physical hardware accesses to `0xA200`–`0xA3FF` corrupt internal latch states.

---

## 3.6 Parallel Microcode Scheduling Example

The following verified assembly block demonstrates legal parallel unit packing respecting rules R2–R8:

```dsp
; Legal Parallel Instruction: Phase advance + Wave wrapping + Dual ring store
addxy r1, r1, r3 | ph_wrap r2, r3, r2 | st r1, [0300]
```

---

## 3.7 Instruction Word Encoding Discrepancies & Anomaly Catalog

Hardware probe verification sessions on the Roland SYSTEM-8 master DSP3 (`re/review_probes.py`) and static corpus analysis have characterized the following closed and open architectural discrepancies:

### 3.7.1 Characterized & Verified Discrepancies `[VERIFIED]`

| ID | Subject | Architectural Rule & Measured Behavior | Status |
|---|---|---|---|
| **D1** | MAC Coefficient Position | Immediate coefficient is read at byte $C$ ($C \in \{2, 4, 6\}$) of the slot array. | `[VERIFIED]` |
| **D3** | Register Operand Negation | Bit 3 of register $Y$ is **not** a negation flag (free bit restored by `.free`); bit 3 of register $W$ **is** an algebraic negation. | `[VERIFIED]` |
| **D5** | Destination $D \ge 8$ (`sum8`) | Unit A with destination $D \ge 8$ forms `sum8 r(D-8), rX, rY, rR`, computing $r(D-8) \leftarrow X + Y + r[\text{op}]$ (measured across classes `0x048` and `0x0A8`). | `[VERIFIED]` |
| **D7** | Hidden Register `hset` | `hset h5` with $Y \in \{0, 1\}$ loads its value from byte 1 (`0x3456`), not word 1 (`0x5678`). | `[VERIFIED]` |
| **D8** | Slot Array Coefficient Offset | The immediate MAC coefficient is located at byte $C$ of the slot array, plus 8 bytes when bit 9 of the second instruction word is asserted. Evaluated across 20 forms (`mac_p1wk*`, `mac_n1wk*`, `mac_p1p2k*`) with 1,536 test cases each. | `[VERIFIED]` |
| **D9** | MAC Opcode Bit 9 | In opcode 1 = 6 with op2 = 000 (`mac_p1p2c0`), bit 9 cleared converts the execution to the no-product additive form $X + W + Z$. | `[VERIFIED]` |

### 3.7.2 Open Architectural Anomalies `[UNRESOLVED]`

| ID | Class / Mnemonic | Anomaly Description | Impact & Workaround |
|---|---|---|---|
| **D4** | Classes `0x240`, `0x572`, `0x592` | Reassembly conflicts between formal layout specs and empirical corpus tokens (`MAACN` vs `MAANCCC`, `AAACTTRRRC` vs `ATACTTRRRC`, `AABCCT` vs `AATCCT`). | `[UNRESOLVED]` Disassembler prioritizes maximum-entropy corpus match. |
| **D6** | Classes `0x258`, `0x140` | 11 instances in `0x258` and 9 in `0x140` contain transfer words (`cd0f`, `cd0e`: Bank 5 store, mode `d0`) at positions designated for ALU operations. | `[UNRESOLVED]` Kept verbatim in disassembly listings. |
| **D10** | Over-the-End Operand Fetch | 20 corpus instructions fetch immediate bytes that extend beyond the physical boundary of the instruction word array, reading into the succeeding instruction in program memory. | `[UNRESOLVED]` Assembler emits verbatim; custom code must avoid straddling word boundaries. |

