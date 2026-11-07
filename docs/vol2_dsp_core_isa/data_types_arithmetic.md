# Volume II: Roland BMC DSP Core & ISA Reference

# Chapter 2: Data Types, Roland RAW Arithmetic & Number Models

> **Document ID**: TRM-BMC-VOL2-002  
> **Classification**: Normative Hardware Reference  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 2.1 The Roland 32-bit Custom Floating-Point Format ("RAW")

The Roland BMC DSP arithmetic logic units (ALU Unit A, ALU Unit B, and MAC units) operate natively and exclusively on a proprietary 32-bit sign-magnitude floating-point numerical representation designated internally as the **Roland RAW Format**.

### 2.1.1 Bitfield Specification

A 32-bit RAW word is partitioned into three discrete bit fields:

```
  31  30 29  28                                                    0
┌────┬──────┬───────────────────────────────────────────────────────┐
│ S  │  s   │ Mantissa M (29-bit Unsigned Integer)                  │
└────┴──────┴───────────────────────────────────────────────────────┘
```

| Field Name | Bit Range | Width | Type | Functional Definition |
|---|---|---|---|---|
| **Sign ($S$)** | Bit 31 | 1 bit | Binary | Arithmetic Sign bit: $S = 0 \implies \text{Positive}$, $S = 1 \implies \text{Negative}$. |
| **Exponent ($s$)** | Bits 30:29 | 2 bits | Unsigned | Scaling Range Selector: $s \in \{0, 1, 2, 3\}$. |
| **Mantissa ($M$)** | Bits 28:0 | 29 bits | Unsigned | 29-bit Unsigned Magnitude Fraction: $M \in [0, 2^{29}-1]$. |

### 2.1.2 Mathematical Formulation `[VERIFIED]`

The real numerical value $v \in \mathbb{R}$ represented by a Roland RAW word $(S, s, M)$ is governed by the exact equation:

$$v = (-1)^S \cdot \frac{M}{2^{32 - 4s}}$$

Equivalently, the quantization step size $\Delta_s$ and maximum dynamic range for each exponent tier $s$ are formulated as:

$$\Delta_s = 2^{4s - 32}, \quad |v|_{\max} = \frac{2^{29}-1}{2^{32 - 4s}} = 2^{4s - 3} - 2^{4s - 32}$$

---

## 2.2 Exponent Ranges, Resolution & Dynamic Envelope

The 2-bit exponent $s$ establishes four non-overlapping scaling tiers optimized for audio processing dynamics:

| Exponent $s$ | Exponent Bits `[30:29]` | Scaling Factor $2^{-(32-4s)}$ | Absolute Range $|v|$ | Quantization Step ($\Delta_s$) | Primary Application |
|---|---|---|---|---|---|
| **$s = 0$** | `00b` | $2^{-32}$ | $[0.0, \; 0.125)$ | $2^{-32} \approx 2.3283 \times 10^{-10}$ | Ultra-low amplitude envelopes, filter damping, reverb tail decay |
| **$s = 1$** | `01b` | $2^{-28}$ | $[0.0, \; 2.0)$ | $2^{-28} \approx 3.7253 \times 10^{-9}$ | **Normalized Audio Domain**: Oscillators, Filters, Unit Gain Signals |
| **$s = 2$** | `10b` | $2^{-24}$ | $[0.0, \; 32.0)$ | $2^{-24} \approx 5.9605 \times 10^{-8}$ | Filter Q, Resonance coefficients, Pitch multipliers |
| **$s = 3$** | `11b` | $2^{-20}$ | $[0.0, \; 512.0)$ | $2^{-20} \approx 9.5367 \times 10^{-7}$ | Frequency Phase Accumulators, Delay Pointers, Large Gain Stages |

---

## 2.3 Canonical Encodings & Reference Bit Constants

To minimize quantization noise, constants must be encoded using the **minimal valid exponent $s$** such that the required mantissa satisfies $M < 2^{29}$:

$$s = \min \left\{ k \in \{0, 1, 2, 3\} \;\middle|\; \lfloor |v| \cdot 2^{32 - 4k} \rfloor < 2^{29} \right\}$$

$$M = \min \left( 2^{29}-1, \; \lfloor |v| \cdot 2^{32 - 4s} + 0.5 \rfloor \right)$$

### 2.3.1 Standard Mathematical Constant Table `[VERIFIED]`

| Nominal Value | Exponent $s$ | Mantissa $M$ (Hex) | Exact RAW Hex Word | Exact Evaluated Value | Absolute Error |
|---|---|---|---|---|---|
| **$0.0$** | 0 | `0x00000000` | `0x00000000` | $0.0$ | $0.0$ |
| **$+0.25$** | 1 | `0x04000000` | `0x24000000` | $+0.25$ | $0.0$ |
| **$+0.5$** | 1 | `0x08000000` | `0x28000000` | $+0.5$ | $0.0$ |
| **$+1.0$** | 1 | `0x10000000` | `0x30000000` | $+1.0$ | $0.0$ |
| **$+2.0$** | 2 | `0x02000000` | `0x42000000` | $+2.0$ | $0.0$ |
| **$-1.0$** | 1 | `0x10000000` | `0xB0000000` | $-1.0$ | $0.0$ |
| **$+100.0$** | 3 | `0x06400000` | `0x66400000` | $+100.0$ | $0.0$ |
| **$\pi / 2$** ($\approx 1.5707963$) | 1 | `0x1921FB54` | `0x3921FB54` | $+1.570796326$ | $< 10^{-9}$ |
| **$\ln 2$** ($\approx 0.6931472$) | 1 | `0x0B17217F` | `0x2B17217F` | $+0.693147180$ | $< 10^{-9}$ |
| **$1 / 192$** (96 kHz step) | 0 | `0x01555555` | `0x01555555` | $+0.005208333$ | $< 10^{-10}$ |

---

## 2.4 Arithmetic Behavior, Rounding & Dynamic Range

Verification across exhaustive randomized hardware test vectors (`re/dspexact.py`, 48/48 random full-precision vectors per operator family) confirms the exact arithmetic execution semantics:

### 2.4.1 Infinite-Precision Accumulation & Truncation Toward Zero `[VERIFIED]`

- Internal arithmetic operations ($D + X \cdot Y$, parallel MAC products $P_1 + P_2 + W$, and scaled MAC/2) compute intermediate products and sums with **exact extended internal precision** (no intermediate truncation).
- Writeback to destination registers is performed in a single final quantization step:
  1. The exact intermediate magnitude $|v_{\text{intermediate}}|$ is evaluated against exponent bounds $s \in \{0, 1, 2, 3\}$.
  2. The smallest exponent $s$ accommodating the value is selected.
  3. The final 29-bit mantissa is formed by **strict truncation toward zero**:
     $$M = \left\lfloor |v_{\text{intermediate}}| \cdot 2^{32 - 4s} \right\rfloor$$
  4. The sign bit $S$ is asserted if $v_{\text{intermediate}} < 0$.

### 2.4.2 Dynamic Overflow Wrapping ($|v| \ge 512.0$) `[VERIFIED]`

The absolute dynamic limit of the RAW representation is $512.0$ ($s = 3, M = 2^{29}$). If an arithmetic operation produces a magnitude exceeding this boundary:
- **Modulo 512 Wrapping**: The hardware does NOT saturate to full-scale. Instead, the magnitude wraps **modulo 512.0**:
  $$|v_{\text{stored}}| = |v_{\text{exact}}| \pmod{512.0}$$
- **Sign Preservation**: The original algebraic sign $S$ is retained across the overflow event.
- **Hardware Flag**: The Overflow flag (`V`) is asserted in the active condition group.

*Example Measured Cases*:
- $300.0 + 300.0 = 600.0 \implies 600.0 \pmod{512.0} = 88.0$ (Stored as RAW `+88.0`, Flag `V` = 1).
- $256.0 + 256.0 = 512.0 \implies 512.0 \pmod{512.0} = 0.0$ (Stored as RAW `0.0`, Flag `V` = 1).

---

## 2.5 Unit B Two's-Complement Bitwise Number Model `[MEASURED]`

While Unit A and MAC units operate strictly under the sign-magnitude RAW model, **Unit B Bitwise Logic Instructions** (`and`, `andn`, `or`, `orn`, `xor`, `xnor`) interpret 32-bit registers as **Two's-Complement Fixed-Point Words with 23 Fractional Bits**:

```
  31                                           23 22                                           0
┌────────────────────────────────────────────────┬──────────────────────────────────────────────┐
| Integer Component (Signed 9-bit, mod 512)     | Fractional Bits (23-bit Fixed Point)         |
└────────────────────────────────────────────────┴──────────────────────────────────────────────┘
```

- Bitwise boolean masks operate bit-for-bit across this two's-complement alignment.
- When a bitwise operation yields a negative numerical result in two's-complement, the hardware converts the result back into the RAW view modulo 512.0. Consequently, negative bitwise values appear in the upper range just below 512.0 as characteristic hex words of the form `0x7FFxxxxx`.

---

## 2.6 Immediate Value Encoding Formats (16-bit, 24-bit, 32-bit) `[VERIFIED]`

Instructions embedding constant numerical operands encode immediate values as truncated upper bit slices of the target RAW 32-bit word:

```
  31                               16 15                8 7                  0
┌────────────────────────────────────┬───────────────────┬───────────────────┐
│          Immediate 16-bit          │ Immediate 24-bit  │ Immediate 32-bit  │
│          (modeY = 01b)             │ (modeY = 10b)     │ (modeY = 11b)     │
└────────────────────────────────────┴───────────────────┴───────────────────┘
```

- **Mode 1 (`mode = 01b`, 16 bits)**: Transmits RAW bits `[31:16]`. Lower 16 bits are zero-padded by hardware on load.
- **Mode 2 (`mode = 10b`, 24 bits)**: Transmits RAW bits `[31:8]`. Lower 8 bits are zero-padded by hardware on load.
- **Mode 3 (`mode = 11b`, 32 bits)**: Transmits full 32-bit RAW word across two successive instruction slot words.

---

## 2.7 Cross-Domain Format Conversion Algorithms

### 2.7.1 IEEE-754 Single-Precision Float $\to$ Roland RAW Conversion (Host ARM C Reference)

```c
uint32_t float_to_raw(float val)
{
    if (val == 0.0f)
        return 0u;

    uint32_t sign = 0;
    if (val < 0.0f) {
        sign = 0x80000000u;
        val = -val;
    }

    /* Modulo 512 wrapping for out-of-range magnitudes */
    while (val >= 512.0f)
        val -= 512.0f;

    /* Select minimal exponent tier s */
    uint32_t s;
    if (val < 0.125f)
        s = 0;
    else if (val < 2.0f)
        s = 1;
    else if (val < 32.0f)
        s = 2;
    else
        s = 3;

    /* Scale: 2^(32 - 4*s) */
    double scale = (double)(1ULL << (32 - 4 * s));
    uint32_t mantissa = (uint32_t)(val * scale + 0.5);

    /* Clamp mantissa to 29 bits */
    if (mantissa > 0x1FFFFFFFu)
        mantissa = 0x1FFFFFFFu;

    return sign | (s << 29) | mantissa;
}
```

### 2.7.2 Roland RAW $\to$ IEEE-754 Single-Precision Float Conversion

```c
float raw_to_float(uint32_t raw)
{
    if (raw == 0u)
        return 0.0f;

    uint32_t sign     = raw >> 31;
    uint32_t s        = (raw >> 29) & 0x3u;
    uint32_t mantissa = raw & 0x1FFFFFFFu;

    double scale = (double)(1ULL << (32 - 4 * s));
    double val = (double)mantissa / scale;

    return sign ? (float)(-val) : (float)(val);
}
```
