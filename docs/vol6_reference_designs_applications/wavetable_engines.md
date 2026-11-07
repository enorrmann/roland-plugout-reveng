# Volume VI: Reference Designs & Applications

# Chapter 5: Wavetable Synthesis Engines & Dynamic Bank 5 Injection

> **Document ID**: TRM-BMC-VOL6-005  
> **Classification**: Normative Implementation Specification  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 5.1 Dynamic Wavetable Architecture Overview

The Roland BMC DSP architecture supports dynamic wavetable synthesis through real-time streaming into **Bank 5 (`DPC`)** coupled with indexed memory load instructions (`ldidx` / `ldb5`):

```
+---------------------------------------------------------------------------------------------------+
|                                  WAVETABLE INJECTION ARCHITECTURE                                 |
|                                                                                                   |
|  [ Host PC / USB Midi Stream ]                                                                    |
|   Uploads 128-point single-cycle PCM WAV sample frames over USB                                   |
|                                  │                                                                |
|                                  ▼                                                                |
|  [ Slave ARM Core (engine.c) ]                                                                    |
|   Streams waveframe directly into Bank 5 (A080h - A0FFh) via Syscall FD6C / +0xB0                 |
|                                  │                                                                |
|                                  ▼                                                                |
|  [ Slave DSP Core: Wavetable Oscillator Kernel ]                                                  |
|   - Phase Accumulator advances at pitch frequency                                                 |
|   - Hidden Register h6 configured with base index: hset h6, base                                  |
|   - Single-Cycle Indexed Load reads sample: ldidx r0, offset                                      |
|   - Linear interpolation between sample index i and i+1                                           |
|   - Continuous morphing across waveframes via Bank 6 parameter C0id                               |
+---------------------------------------------------------------------------------------------------+
```

---

## 5.2 Real-Time Indexed Reading & Interpolation

A wavetable frame comprises 128 discrete 32-bit floating-point samples stored in Bank 5. Within the 96 kHz audio sample frame, the DSP core splits the phase accumulator into integer index $i \in [0, 127]$ and fractional remainder fraction $f \in [0.0, 1.0)$:

1. **Integer Base Configuration (`HSET`)**: Loads index $i$ into hidden base register `h6`.
2. **Adjacent Sample Load (`LDIDX`)**: Loads sample $S_i$ and sample $S_{i+1}$.
3. **Linear Interpolation (`MAC`)**: Computes $y = S_i + f \cdot (S_{i+1} - S_i)$.

```dsp
; Wavetable Sample Fetch & Linear Interpolation
hset h6, r1         ; Set base index in h6 from integer phase r1
mov r4, r4          ; Index setup latency delay slot
ldidx r0, 0         ; Load sample S[i] from Bank 0/5
ldidx r2, 1         ; Load adjacent sample S[i+1]
mov r4, r4          ; Load delay slot
subxy r3, r0, r2    ; r3 <- S[i+1] - S[i]
mac_p1 r0, r3, r7, r0 ; r0 <- S[i] + fraction * (S[i+1] - S[i])
```

---

## 5.3 Wavetable Engines Catalog

| Reference Design | Application Directory | Wavetable Capacity | Morphing Method | Measured Hardware Status |
|---|---|---|---|---|
| **`C-WTLIVE`** | `examples/wt_live/` | 1 Active Streamed Frame | Live USB Sample Injection | `[VERIFIED]` Production Ready |
| **`C-MORPH`** | `examples/wt_morph/` | 4 Embedded Waveframes | Dual-Frame Crossfade ($M \in [0, 1]$) | `[VERIFIED]` Production Ready |
| **`C-PWM`** | `examples/wt_pwm/` | Continuous Variable Pulse | Dynamic Slicing of Triangle Phase | `[VERIFIED]` Production Ready |
