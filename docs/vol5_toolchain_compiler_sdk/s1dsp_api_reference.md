# Volume V: Toolchain, Compiler & SDK Specification

# Chapter 2: The `s1dsp.h` C/C++ API & Hardware Intrinsics Reference

> **Document ID**: TRM-BMC-VOL5-002  
> **Classification**: Normative Programmer's Reference  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 2.1 Overview of the `s1dsp.h` Architecture

The **`s1dsp.h`** header file (`sdk/s1cc/include/s1dsp.h`) provides the standardized C and C++ programming interface for developing audio synthesis microcode targeted at the Roland BMC DSP architecture.

A single C/C++ source file authored against `s1dsp.h` serves three unified engineering roles:
1. **Compilation to Microcode**: Translated by `s1cc` into native BMC VLIW machine code.
2. **Native PC Bit-Exact Verification**: Compiled natively on the development host with `sdk/s1cc/runtime/s1rt.c` to produce a golden mathematical reference.
3. **Automated Profiling & Range Analysis**: Instrumented during native compilation to extract variable dynamic ranges, verify absence of overflows, and compute polynomial approximations for lookup tables.

---

## 2.2 Execution Entry Points & Macros

```c
#include <s1dsp.h>

/* Primary Oscillator Entry Point: Executes on Slave DSP A every sample */
S1_DSP_A
{
    float phase_inc = s1_increment();
    static float phase;
    phase += phase_inc;
    phase = s1_wrap(phase);
    return phase;
}

/* Secondary Processing Entry Point: Executes on Slave DSP B every sample */
S1_DSP_B
{
    float audio_in = s1_import(0);
    float gain = s1_param(LEVEL, 0);
    return audio_in * gain;
}
```

### 2.2.1 Core Macros Definition

| Macro Name | Target Processor | Execution Frequency | Description |
|---|---|---|---|
| `S1_DSP_A` | Slave DSP A | 96 kHz per voice | Primary voice generator block. Implements oscillators and wavetables in place of the factory DCOs. |
| `S1_DSP_B` | Slave DSP B | 96 kHz per voice | Post-processing / filter block. Receives audio from DSP A, executes dynamic filters and waveshapers. |
| `S1_PARAM(name, id, count)` | Host PC (Build Time) | Run once at compile | Generates parameter lookup tables for continuous front-panel knobs ($0 \le \text{knob} \le 255$). |
| `S1_INIT` | Host PC / Simulation | Cold Boot | Initializes static filter and oscillator variables prior to audio processing. |

---

## 2.3 Comprehensive Hardware Intrinsics Catalog

### 2.3.1 Pitch, Frequency & Phase Intrinsics

```c
/* Returns active voice pitch in octaves (4.75 = A440; includes LFO and pitch bend) */
float s1_pitch(void);

/* Returns normalized phase increment delta: 2.0 * f / 96000.0 */
float s1_increment(void);

/* Returns normalized sample period step (f / 96000.0) and inverse step (96000.0 / f) */
float s1_dt(void);
float s1_rdt(void);
```

### 2.3.2 Voice Lifecycle & Trigger Intrinsics

```c
/* Returns current gate state: 1.0f (Key Pressed) or 0.0f (Key Released) */
float s1_gate(void);

/* Toggles between 0.0f and 1.0f on every new note-on event */
float s1_trig(void);
```

### 2.3.3 Phase Distortion & Waveform Synthesis Intrinsics

```c
/* Wraps accumulator x into normalized fundamental range [-1.0, +1.0) */
/* Lowers to native single-cycle opcode: ph_wrap */
float s1_wrap(float x);

/* Clamps excursion to [-1.0, +1.0]. Lowers to: ph_clamp */
float s1_clamp(float x);

/* Generates bipolar triangle wave: 1.0 - 2.0 * |wrap(x)|. Lowers to: ph_tri */
float s1_tri(float x);
```

### 2.3.4 Mathematical & Special Function Unit (SFU) Intrinsics

| Function Signature | Equivalent Hardware Opcode | Cycle Cost | Dynamic Precision |
|---|---|---|---|
| `float s1_recip(float x)` | `recipseed` + 2 `newton` | 3 Cycles | $\epsilon < 10^{-8}$ (Floating Reciprocal $1/x$) |
| `float s1_sqrt(float x)` | `sqrtsplit` + 2 `newton` | 11 Cycles | $\epsilon < 2 \times 10^{-7}$ (Square Root $\sqrt{x}$) |
| `float s1_rsqrt(float x)` | `sqrtsplit` + 2 `newton` | 13 Cycles | $\epsilon < 2 \times 10^{-7}$ (Inverse Root $1/\sqrt{x}$) |
| `float s1_log2(float x)` | `log2split` + Horner Deg 6 | 9 Cycles | $\epsilon < 5 \times 10^{-6}$ (Base-2 Logarithm) |
| `float s1_exp2(float x)` | `expsplit` + Horner Deg 4 | 8 Cycles | $\epsilon < 3 \times 10^{-5}$ ($2^x$ Power-of-Two) |
| `float s1_floor(float x)` | `floor` (SFU Unit B) | 1 Cycle | Exact integer truncation $\lfloor x \rfloor$ |
| `float s1_ceil(float x)` | `ceil` (SFU Unit B) | 1 Cycle | Exact integer ceiling $\lceil x \rceil$ |
| `float s1_sign(float x)` | `sign` (SFU Unit B) | 1 Cycle | Signum function: $-1.0, \; 0.0, \; +1.0$ |
| `float s1_abs(float x)` | `absx` (Unit B ALU) | 1 Cycle | Absolute magnitude $|x|$ |

### 2.3.5 Parallel Accumulation Intrinsics

```c
/* 3-Operand Register Sum: a + b + c. Lowers to native opcode: sum8 */
float s1_sum3(float a, float b, float c);

/* Dual Product Multiply-Accumulate: (a*b) + (c*d) + e. Lowers to: mac_p1p2 */
float s1_mac2(float a, float b, float c, float d, float e);
```

### 2.3.6 Inter-DSP Data Transport Intrinsics

```c
/* Exports value v from DSP A to DSP B across the shared Bank 5 mailbox */
void s1_export(int channel, float v);

/* Imports value sent by DSP A inside DSP B within the same audio frame */
float s1_import(int channel);
```

### 2.3.7 Wavetable & Memory Lookups

```c
/* Reads word i (0..126) from the active wavetable buffer in Bank 5 */
/* Lowers to single-cycle indexed transfer: hset + ldb5 */
float s1_ldb5(int word_index);

/* Retrieves pre-interpolated knob parameter from Bank 6 */
float s1_param(name, int index);
```
