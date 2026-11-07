# Volume II: Roland BMC DSP Core & ISA Reference

# Chapter 5: Special Function Units (SFU), Seed ROMs & Mathematical Accelerators

> **Document ID**: TRM-BMC-VOL2-005  
> **Classification**: Normative Hardware Reference  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 5.1 Hardware Special Function Units (SFU) Overview

The Roland BMC DSP core embeds dedicated hardware accelerators to compute nonlinear transcendental functions, reciprocal division, roots, and periodic audio waveforms without incurring multi-cycle pipeline stalls:

1. **Seed ROM Tables**: Internal hardware lookup tables for reciprocal ($1/x$) and reciprocal square root ($1/\sqrt{x}$) seed generation.
2. **Newton-Raphson / Goldschmidt Convergence Units**: High-throughput quadratic convergence execution stages.
3. **Phase Distortion Synthesis Accelerators**: Native zero-overhead modulo wrapping and waveshaping operators (`ph_wrap`, `ph_clamp`, `ph_tri`).
4. **Logarithmic & Exponential Splitting Engines**: `log2split` and `expsplit` engines supporting wide dynamic range pitch calculation.

```
+---------------------------------------------------------------------------------------------------+
|                                     BMC DSP SFU ACCELERATOR ARCHITECTURE                          |
|                                                                                                   |
|  [ Operands: rX, rY ]                                                                             |
|            │                                                                                      |
|            ├───────────────┬────────────────────────┬────────────────────────┬─────────────────┐  |
|            ▼               ▼                        ▼                        ▼                 ▼  |
|  +-------------------+ +----------------------+ +----------------------+ +------------------+ ...|
|  | Reciprocal Seed   | | Inv Square Root Seed | | Log2 / Exp Engines   | | Phase Distort    |     |
|  | 9-bit ROM (Table) | | Table Q (9-bit)      | | Exponent/Frac Split  | | Wrap, Clamp, Tri |     |
|  +---------+---------+ +----------+-----------+ +----------+-----------+ +--------+---------+     |
|            │                      │                        │                      │               |
|            ▼                      ▼                        ▼                      │               |
|  +-------------------+ +----------------------+            │                      │               |
|  | Newton-Raphson    | | Goldschmidt Iteration|            │                      │               |
|  | Quadratic Refine  | | Refinement           |            │                      │               |
|  +---------+---------+ +----------+-----------+            │                      │               |
|            │                      │                        │                      │               |
|            ▼                      ▼                        ▼                      ▼               |
|  [ Result Writeback: High-Precision 32-bit Roland RAW Value Latched to Target Registers ]         |
+---------------------------------------------------------------------------------------------------+
```

---

## 5.2 Reciprocal Division Engine (`RECIPSEED` + `NEWTON`) `[VERIFIED]`

Direct division on standard DSP architectures requires iterative software loops or heavy microcoded multi-cycle instructions. The Roland BMC DSP achieves full-precision floating-point reciprocal calculation ($1/y$) in **4 instruction cycles** using Goldschmidt / Newton-Raphson quadratic approximation:

### 5.2.1 Operational Mechanism

1. **Seed Extraction (`recipseed rD, rY`)**:
   The hardware decomposes input $r_Y$ into mantissa $m \in [0.5, 1.0)$ and exponent $e$. It accesses an internal 9-bit seed ROM to obtain initial estimate:
   $$x_0 \approx \frac{1}{m}$$
2. **Quadratic Convergence Refinement (`newton rD, rY`)**:
   The `newton` instruction executes parallel refinement across adjacent register pairs:
   $$r_{D+1} \leftarrow 2.0 - (r_{D+1} \cdot r_Y), \quad r_D \leftarrow r_D \cdot r_{D+1}$$
   Two iterations achieve a precision of $\epsilon < 10^{-8}$ across the entire normalized floating-point domain.

```dsp
; Full-Precision 32-bit Reciprocal Division (r0 <- 1.0 / r1)
recipseed r0, r1    ; Extract 9-bit reciprocal seed from hardware ROM
newton r0, r1       ; Refinement iteration 1: doubles valid significant bits
newton r0, r1       ; Refinement iteration 2: full 29-bit mantissa convergence
```

---

## 5.3 Square Root & Inverse Square Root Engine (`SQRTSPLIT`) `[VERIFIED]`

Computing $1/\sqrt{y}$ and $\sqrt{y}$ is accelerated via the hardware `sqrtsplit` instruction coupled with internal Table Q:

### 5.3.1 Decomposition Semantics
Given operand $r_Y$:
1. The hardware evaluates $m = |r_Y| \cdot 4^k \in [1.0, 4.0)$.
2. Mantissa $m$ is placed into $r_Y$.
3. An initial 9-bit estimate of $1/\sqrt{m}$ from Table Q is placed into $r_{D+1}$.
4. Exponent scaling factor $\pm 2^{-k}$ is written to $r_{D+3}$.
5. Standard polynomial or Newton refinement steps converge to exact square root representations without branch overhead.

---

## 5.4 Logarithmic & Exponential Accelerators (`LOG2SPLIT` & `EXPSPLIT`) `[VERIFIED]`

Synthesizer pitch calculation relies heavily on volt-per-octave and MIDI note-to-frequency conversions ($f = f_0 \cdot 2^{(\text{note}-69)/12}$):

### 5.4.1 `LOG2SPLIT` Operation
Decomposes input $x$ into:
$$x = m \cdot 2^e, \quad m \in [0.75, 1.5)$$
The fractional mantissa $m$ and integer exponent $e$ are output to target registers, allowing subsequent evaluation via low-order Horner polynomial approximation.

### 5.4.2 `EXPSPLIT` Operation
Computes base-2 and natural exponentials by partitioning the operand into integer floor $\lfloor x \rfloor$ and fractional remainder $\{x\}$:
- Computes $2^{\lfloor x \rfloor}$ via an internal hardwired seed ROM spanning $> \pm 8$ octaves.
- Returns fractional term $\{x\}$ for polynomial interpolation.

---

## 5.5 Phase Distortion & Waveform Synthesis Accelerators `[VERIFIED]`

Generating classic analog waveforms (Sawtooth, Triangle, Variable-Slope Pulse) at 96 kHz without aliasing requires continuous phase manipulation:

### 5.5.1 The Modulo Phase Wrapper (`PH_WRAP`)
On standard architectures, wrapping an accumulating phase variable $p \in \mathbb{R}$ into the fundamental cycle range $[-1.0, +1.0)$ requires branch instructions or a multi-instruction chain of floating-point comparisons. The BMC DSP core computes this in **a single cycle**:

```dsp
; Optimized 96 kHz Sawtooth Oscillator Core (2 Instructions Total)
addxy r1, r1, r2    ; Advance phase accumulator (r1 += r2 frequency delta)
ph_wrap r1, r3, r1  ; Wrap phase into [-1.0, +1.0) using canonical r3 register
```

### 5.5.2 Triangle Waveform Generator (`PH_TRI`)
Generates a symmetrical bipolar triangle wave from an active phase accumulator:

$$v_{\text{tri}} = 1.0 - 2.0 \cdot \left| \text{wrap}(r_Y) \right|$$

```dsp
; Phase to Bipolar Triangle Synthesis
ph_tri r0, r3, r1   ; r0 <- 1.0 - 2.0 * |wrap(r1)|
```

### 5.5.3 Phase Saturation Limiter (`PH_CLAMP`)
Clamps phase excursion to $[-1.0, +1.0]$, preventing buffer overruns in resonant phase-distortion tables:

```dsp
; Phase Clamp Operation
ph_clamp r0, r3, r1 ; r0 <- clamp(r1, -1.0, +1.0)
```

---

## 5.6 Hardware Frame Timer Register (`F5C`) `[MEASURED]`

The DSP core monitors global system time via the hardware counter instruction `f5c`:
- Reads an autonomous 32-bit hardware counter that decrements by $1/16384$ per 96 kHz sample frame modulo 512.
- Used by reverb delay networks and chorus modulation LFOs to determine absolute sample phase and hardware buffer alignment across asynchronous DSP cores.
