# Volume II: Roland BMC DSP Core & ISA Reference

# Chapter 4: Complete Instruction Set Architecture (ISA) Reference Manual

> **Document ID**: TRM-BMC-VOL2-004  
> **Classification**: Normative Hardware Reference & Programmer's Manual  
> **Release Target**: Texas Instruments TMS320C6000 / ADI SHARC Reference Standard  

---

## 4.1 Master Opcode Matrix & Classification

The Roland BMC DSP execution pipeline decodes four primary functional unit categories:
1. **Unit A**: 3-Operand / 4-Operand Scalar Fixed/Floating Arithmetic & Newton Iteration.
2. **Unit B**: Special Function Unit (SFU) Non-Linear Functions, Phase Manipulators, Min/Max & Bitwise Logic.
3. **Dual MAC Unit**: Two-word High-Throughput Parallel Multiply-Accumulate Engines.
4. **Transfer Units (`T`)**: Memory Load, Memory Store, Indexed Addressing, and Status Flag Registers.

```
+--------------------------------------------------------------------------------------------------+
|                                  BMC DSP MASTER OPCODE MAP                                       |
+----------+---------------------------------------------------------------------------------------+
|  Unit A  | addxy, add3, sum8, subxy, mul, mov, absadd, subabs, addh4, addh5, newton              |
+----------+---------------------------------------------------------------------------------------+
|  Unit B  | min, max, minn, maxn, absx, absy, shl, shr, shrn, and, andn, or, orn, xor, xnor, cmp  |
+----------+---------------------------------------------------------------------------------------+
|  SFU     | recipseed, sqrtsplit, log2split, expsplit, atanhsplit, floor, ceil, sign, frexp       |
+----------+---------------------------------------------------------------------------------------+
|  Phase   | ph_wrap, ph_clamp, ph_tri, ph_abs, ph_add, ph_sub, ph_mod2, ph_neg                    |
+----------+---------------------------------------------------------------------------------------+
|  MAC     | mac_p1, mac_p1p2, mac_p1w, mac_p1p2w, mac_p1p2wp2, mac_p1wc3, mac_p1p2c3, ymacd       |
+----------+---------------------------------------------------------------------------------------+
| Transfer | ld, st, ldidx, stidx, ldb5, ldb5d, ldb5h7, tapreq, f5, f6, f7                         |
+----------+---------------------------------------------------------------------------------------+
| Control  | jmp, cond, setflags, hset, noop                                                       |
+----------+---------------------------------------------------------------------------------------+
```

---

## 4.2 Arithmetic Instructions (Unit A)

---

### `ADDXY` — Add Register to Register
**Functional Class**: Unit A Scalar ALU  
**Syntax**: `addxy rD, rX, rY`  
**Certainty Level**: `[VERIFIED]`  

#### Binary Word Layout
```
  15       12 11     9 8     6 5    4 3     0
┌────────────┬────────┬───────┬──────┬───────┐
│     D      │  110b  │   X   │ 00b  │   Y   │
└────────────┴────────┴───────┴──────┴───────┘
```

#### Execution Semantics (RTL)
```
temp = rX + rY
rD = truncate_to_raw(temp)
flags.Z = (temp == 0.0)
flags.N = (temp < 0.0)
flags.V = (|temp| >= 512.0)
```

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
addxy r0, r1, r2    ; r0 <- r1 + r2
```

---

### `SUBXY` — Subtract Register from Register
**Functional Class**: Unit A Scalar ALU  
**Syntax**: `subxy rD, rX, rY`  
**Certainty Level**: `[VERIFIED]`  

#### Binary Word Layout
```
  15       12 11     9 8     6 5    4 3     0
┌────────────┬────────┬───────┬──────┬───────┐
│     D      │  111b  │   X   │ 00b  │   Y   │
└────────────┴────────┴───────┴──────┴───────┘
```

#### Execution Semantics (RTL)
```
temp = rY - rX
rD = truncate_to_raw(temp)
flags.Z = (temp == 0.0)
flags.N = (temp < 0.0)
flags.V = (|temp| >= 512.0)
```

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
subxy r0, r1, r2    ; r0 <- r2 - r1
```

---

### `MUL` — Scalar Multiplication
**Functional Class**: Unit A Scalar ALU  
**Syntax**: `mul rD, rX, rY`  
**Certainty Level**: `[VERIFIED]`  

#### Binary Word Layout
```
  15       12 11     9 8     6 5    4 3     0
┌────────────┬────────┬───────┬──────┬───────┐
│     D      │  010b  │   X   │ mY   │   Y   │
└────────────┴────────┴───────┴──────┴───────┘
```

#### Execution Semantics (RTL)
```
temp = rX * Y
rD = truncate_to_raw(temp)
flags.Z = (temp == 0.0)
flags.N = (temp < 0.0)
flags.V = (|temp| >= 512.0)
```

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
mul r0, r1, r2      ; r0 <- r1 * r2
```

---

### `ADD3` — 3-Operand Register Accumulation
**Functional Class**: Unit A Scalar ALU  
**Syntax**: `add3 rD, rX, rY`  
**Certainty Level**: `[VERIFIED]`  

#### Binary Word Layout
```
  15       12 11     9 8     6 5    4 3     0
┌────────────┬────────┬───────┬──────┬───────┐
│     D      │  001b  │   X   │ 00b  │   Y   │
└────────────┴────────┴───────┴──────┴───────┘
```

#### Execution Semantics (RTL)
```
temp = rD + rX + rY
rD = truncate_to_raw(temp)
```

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
add3 r0, r1, r2     ; r0 <- r0 + r1 + r2
```

---

### `SUM8` — 4-Operand Vector Sum
**Functional Class**: Unit A Scalar ALU (Extended Variant $D \ge 8$)  
**Syntax**: `sum8 rD, rX, rY, rR`  
**Certainty Level**: `[VERIFIED]`  

#### Binary Word Layout
```
  15       12 11     9 8     6 5    4 3     0
┌────────────┬────────┬───────┬──────┬───────┐
│   8 + D    │   R    │   X   │ 00b  │   Y   │
└────────────┴────────┴───────┴──────┴───────┘
```

#### Execution Semantics (RTL)
```
temp = rX + rY + rR
rD = truncate_to_raw(temp)
```

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
sum8 r0, r1, r2, r3 ; r0 <- r1 + r2 + r3
```

---

### `MOV` — Register / Immediate Move
**Functional Class**: Unit A Transfer / ALU  
**Syntax**: `mov rD, rY` or `mov rD, #imm`  
**Certainty Level**: `[VERIFIED]`  

#### Binary Word Layout
```
  15       12 11     9 8     6 5    4 3     0
┌────────────┬────────┬───────┬──────┬───────┐
│     D      │  000b  │ 001b  │ mY   │   Y   │
└────────────┴────────┴───────┴──────┴───────┘
```

#### Execution Semantics (RTL)
```
rD = Y
```

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
mov r0, r1          ; r0 <- r1
mov r2, #0.5        ; r2 <- 0.5 (RAW 0x28000000)
```

---

## 4.3 Phase Operations & Synthesis Accelerators (Unit B)

---

### `PH_WRAP` — Modulo [-1.0, +1.0) Phase Wrapping
**Functional Class**: Unit B Phase Distortion Accelerator  
**Syntax**: `ph_wrap rD, rX, rY`  (Roland canonical form uses $rX = r_3$)  
**Certainty Level**: `[VERIFIED]`  

#### Execution Semantics (RTL)
```
val = rY
while (val >= 1.0)  val -= 2.0
while (val < -1.0)  val += 2.0
rD = val
```

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
ph_wrap r1, r3, r1  ; Wrap oscillator phase in r1 to [-1.0, +1.0)
```

---

### `PH_CLAMP` — Phase Saturation Limiter
**Functional Class**: Unit B Phase Distortion Accelerator  
**Syntax**: `ph_clamp rD, rX, rY`  
**Certainty Level**: `[VERIFIED]`  

#### Execution Semantics (RTL)
```
val = rY
if (val > 1.0)  val = 1.0
if (val < -1.0) val = -1.0
rD = val
```

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
ph_clamp r1, r3, r1 ; Clamp phase excursion to [-1.0, +1.0]
```

---

### `PH_TRI` — Triangle Wave Fold Generator
**Functional Class**: Unit B Waveform Synthesis Accelerator  
**Syntax**: `ph_tri rD, rX, rY`  
**Certainty Level**: `[VERIFIED]`  

#### Execution Semantics (RTL)
Transforms normalized linear phase into a bipolar triangle waveform:
$$r_D = 1.0 - 2.0 \cdot \left| \text{wrap}(r_Y) \right|$$

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
ph_tri r1, r3, r1   ; Generate bipolar triangle output from phase r1
```

---

## 4.4 Relational, Logic & Shift Operations (Unit B)

---

### `MIN` / `MAX` — Extremum Selection
**Functional Class**: Unit B Relational ALU  
**Syntax**: `min rD, rX, rY` / `max rD, rX, rY`  
**Certainty Level**: `[VERIFIED]`  

#### Execution Semantics (RTL)
```
min: rD = (rX < rY) ? rX : rY
max: rD = (rX > rY) ? rX : rY
```

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
min r0, r1, r2      ; r0 <- min(r1, r2)
max r3, r4, r5      ; r3 <- max(r4, r5)
```

---

### `SHL` — Power-of-Two Bitwise Shift
**Functional Class**: Unit B Shift ALU  
**Syntax**: `shl rD, rX, shift_imm`  
**Certainty Level**: `[VERIFIED]`  

#### Execution Semantics (RTL)
Computes $r_X \cdot 2^k$ under 23-bit fractional two's-complement arithmetic with modulo 512 dynamic wrapping:
$$r_D = \text{RAW\_encode}\left( (r_X \cdot 2^k) \pmod{512.0} \right)$$

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
shl r0, r1, 2       ; r0 <- r1 * 4.0
```

---

### `AND` / `OR` / `XOR` — Two's-Complement Bitwise Logic
**Functional Class**: Unit B Boolean ALU  
**Syntax**: `and rD, rX, rY` / `or rD, rX, rY` / `xor rD, rX, rY`  
**Certainty Level**: `[MEASURED]`  

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
and r0, r1, r2      ; r0 <- r1 bitwise AND r2
or  r3, r4, r5      ; r3 <- r4 bitwise OR r5
xor r6, r7, r0      ; r6 <- r7 bitwise XOR r0
```

---

## 4.5 Multiply-Accumulate Operations (Dual MAC Unit)

---

### `MAC_P1` — Single Product Multiply-Accumulate
**Functional Class**: MAC Unit Single Product  
**Syntax**: `mac_p1 rD, rX, rY, rW`  
**Certainty Level**: `[VERIFIED]`  

#### Execution Semantics (RTL)
$$r_D = r_W + (r_X \cdot r_Y)$$

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
mac_p1 r0, r1, r2, r0   ; r0 <- r0 + (r1 * r2)
```

---

### `MAC_P1P2` — Dual Parallel Product Accumulation
**Functional Class**: MAC Unit Dual Extended Product  
**Syntax**: `mac_p1p2 rD, rX, rY, rZ, rC, rW`  
**Certainty Level**: `[VERIFIED]`  

#### Execution Semantics (RTL)
$$r_D = (r_X \cdot r_Y) + (r_C \cdot r_Z) + r_W$$

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
mac_p1p2 r0, r1, r2, r3, r4, r5 ; r0 <- (r1*r2) + (r4*r3) + r5
```

---

## 4.6 Special Function Units (SFU Approximators)

---

### `RECIPSEED` + `NEWTON` — High-Speed Reciprocal Division
**Functional Class**: SFU Mathematical Accelerator  
**Syntax**: `recipseed rD, rY` followed by `newton rD, rY`  
**Certainty Level**: `[VERIFIED]`  

#### Execution Semantics (RTL)
1. `recipseed` extracts the 9-bit seed approximation from internal ROM: $r_D \approx 1.0 / r_Y$.
2. `newton` executes Goldschmidt / Newton-Raphson quadratic convergence refinement:
   $$r_{D+1} \leftarrow 2.0 - (r_{D+1} \cdot r_Y), \quad r_D \leftarrow r_D \cdot r_{D+1}$$

#### Execution Cycles: 2 Cycles (1 cycle per instruction)  
#### Assembly Example
```dsp
recipseed r0, r1    ; Seed approximation r0 <- 1/r1
newton r0, r1       ; Quadratic refinement step
```

---

### `FLOOR` / `CEIL` / `SIGN` — Integer Truncation & Signum
**Functional Class**: SFU Mathematical Accelerator  
**Syntax**: `floor rD, rY` / `ceil rD, rY` / `sign rD, rY`  
**Certainty Level**: `[VERIFIED]`  

#### Execution Semantics (RTL)
```
floor: rD = floor(rY)
ceil:  rD = ceil(rY)
sign:  rD = (rY > 0) ? 1.0 : (rY < 0) ? -1.0 : 0.0
```

#### Execution Cycles: 1 Cycle  
#### Assembly Example
```dsp
floor r0, r1        ; r0 <- floor(r1)
ceil  r2, r3        ; r2 <- ceil(r3)
sign  r4, r5        ; r4 <- sign(r5)
```

---

## 4.7 Memory Transfer & Addressing Instructions (`T` Units)

---

### `LD` / `ST` — Direct Memory Load & Store
**Functional Class**: Transfer Unit (`T`)  
**Syntax**: `ld rD, [address]` / `st rX, [address]`  
**Certainty Level**: `[VERIFIED]`  

#### Addressing Mode
Direct 16-bit address within Bank 0 through Bank 7.

#### Load-to-Use Latency: 1 Cycle Delay Slot  
#### Assembly Example
```dsp
ld r0, [0400]       ; Load r0 from Bank 0 address 0x0400 (latched at N+2)
mov r4, r4          ; Delay slot padding
st r0, [0401]       ; Store r0 into Bank 0 address 0x0401
```

---

### `LDIDX` — Indexed Bank 0 Ring Read
**Functional Class**: Transfer Unit (`T 0x28`)  
**Syntax**: `ldidx rD, byte_offset`  
**Certainty Level**: `[VERIFIED]`  

#### Execution Semantics (RTL)
Addresses Bank 0 relative to hidden base index register `h6`:
$$\text{Physical Address} = (h_6 + \text{offset}) \pmod{2048}$$

#### Assembly Example
```dsp
hset h6, r1         ; Set base index in h6 from r1
mov r4, r4          ; Setup latency slot
ldidx r0, 0         ; Load r0 from Bank 0[h6 + 0]
```

---

### `LDB5` — Bank 5 Parameter Mailbox Read
**Functional Class**: Transfer Unit (`T 0xDA` / `0xDE`)  
**Syntax**: `ldb5 rD, byte_offset`  
**Certainty Level**: `[VERIFIED]`  

#### Assembly Example
```dsp
ldb5 r0, 0          ; Load r0 from Bank 5 mailbox offset
```

---

## 4.8 Control & Predication Directives

---

### `JMP` — Relative Branch
**Functional Class**: Control Flow  
**Syntax**: `jmp .+offset`  
**Certainty Level**: `[VERIFIED]`  

#### Execution Latency: 2 Cycles when Taken, 1 Cycle when Untaken  
#### Assembly Example
```dsp
jmp .+2             ; Branch forward by 2 instructions
```

---

### `COND` — Predicated Execution Gate
**Functional Class**: Execution Predication Gate  
**Syntax**: `cond [!]cc, skip_true, skip_false`  
**Certainty Level**: `[VERIFIED]`  

#### Assembly Example
```dsp
cond !12, 1, 0      ; If condition flag 12 is false, skip unit 1
```

---

### `HSET` — Hidden Architecture Register Configuration
**Functional Class**: Architectural Control Word (`40TY`)  
**Syntax**: `hset hReg, rY`  
**Certainty Level**: `[MEASURED]`  

#### Assembly Example
```dsp
hset h6, r0         ; Configure hidden register h6 from r0
```
