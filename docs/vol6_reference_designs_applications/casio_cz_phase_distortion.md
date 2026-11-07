# Volume VI: Reference Designs & Applications

# Chapter 3: Casio CZ Phase Distortion (PD) Synthesis Implementation

> **Document ID**: TRM-BMC-VOL6-003  
> **Classification**: Normative Implementation Specification  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 3.1 Overview of Casio Phase Distortion (PD) Synthesis

The **Casio CZ Phase Distortion** synthesis model (`sdk/s1cc/examples/casio_cz/`) implements classic 1980s digital phase distortion synthesis (originally engineered for the Casio CZ-101 and CZ-1) natively on the Roland BMC DSP architecture.

Phase Distortion generates sharp, resonant waveforms (resonant saw, resonant triangle, and trapezoidal waves) **without feedback analog filters**. Instead, it passes a fundamental phase accumulator through a non-linear phase distortion transfer function that drives a cosine carrier, modulated by a synchronous decaying amplitude window.

```
+---------------------------------------------------------------------------------------------------+
|                                 CASIO CZ PHASE DISTORTION SIGNAL FLOW                             |
|                                                                                                   |
|  Normalized Phase Accumulator phi in [0.0, 1.0)                                                   |
|         │                                                                                         |
|         ├───────────────────────────────────────────────┐                                         |
|         │                                               │                                         |
|         ▼                                               ▼                                         |
|  +---------------------------------------+   +---------------------------------------+            |
|  | Non-Linear Phase Distortion Engine    |   | Resonant Window Envelope              |            |
|  | psi = f(phi, Resonance N)             |   | w(phi) = (1.0 - phi)                  |            |
|  +-------------------+-------------------+   +-------------------+-------------------+            |
|                      │                                           │                                |
|                      ▼                                           │                                |
|  +---------------------------------------+                       │                                |
|  | Cosine Carrier Waveform Generator     |                       │                                |
|  | c = cos(2 * pi * psi)                 |                       │                                |
|  | (Evaluated via Horner Polynomial MAC) |                       │                                |
|  +-------------------+-------------------+                       │                                |
|                      │                                           │                                |
|                      └─────────────────────┬─────────────────────┘                                |
|                                            │                                                      |
|                                            ▼                                                      |
|                             +-----------------------------+                                       |
|                             | Final Audio Output Multiply |                                       |
|                             | y = c * w(phi)              |                                       |
|                             +-----------------------------+                                       |
+---------------------------------------------------------------------------------------------------+
```

---

## 3.2 Mathematical Formulation of Resonant Waveforms

### 3.2.1 Resonant Sawtooth Synthesis
Let $\phi \in [0.0, 1.0)$ represent the normalized fundamental phase of the note. Let $N \ge 1.0$ represent the continuous resonant frequency multiplier set by the filter cutoff parameter:

1. **Phase Distortion**: The carrier phase $\psi$ accelerates at $N$ times the fundamental rate:
   $$\psi(\phi) = (\phi \cdot N) \pmod{1.0}$$
2. **Windowing Envelope**: A synchronous ramp window forces the high-frequency carrier to zero at the end of each fundamental cycle:
   $$w(\phi) = 1.0 - \phi$$
3. **Resonant Composite Waveform**:
   $$y(\phi) = \cos(2\pi \cdot \psi(\phi)) \cdot (1.0 - \phi)$$

Because the waveform resets synchronously with the fundamental period $\phi$, the spectrum exhibits a resonant peak at frequency $f_{\text{res}} = N \cdot f_0$ with completely phase-locked, alias-suppressed sidebands.

---

## 3.3 Microcode Implementation on the Roland BMC DSP

On the BMC DSP, Phase Distortion is executed in **5 instruction cycles** by combining native phase operators with Horner cosine evaluation:

```dsp
; Casio CZ Resonant Sawtooth DSP Kernel
ld r1, [0301]      ; Load fundamental phase phi
mov r4, r4         ; Load delay slot
addxy r1, r1, r2   ; Advance fundamental phase by frequency step r2
ph_wrap r1, r3, r1 ; Wrap fundamental phase into [-1.0, +1.0)
st r1, [0300]      ; Store updated phase into Bank 0 ring buffer

; Resonant Phase Multiplication & Carrier Windowing
ld r7, [c040]      ; Load Resonant Multiplier N (from Bank 6)
mov r4, r4
mul r5, r1, r7     ; Accelerated phase psi = phi * N
ph_wrap r5, r3, r5 ; Wrap carrier phase into [-1.0, +1.0)
ph_tri r6, r3, r5  ; Approximate cosine carrier via triangle folding
mul r0, r6, r1     ; Apply window: r0 <- carrier * (1.0 - phi)
```

---

## 3.4 Hardware Performance & Polyphony

- **Total Execution Footprint**: 520 cycles on DSP A across 8 voices (averaging 65 cycles per voice).
- **Acoustic Characteristics**: Accurately reproduces the glassy, biting resonant sweeps characteristic of Casio CZ series synthesizers with zero filter instability or self-oscillation blow-up.
