# Volume III: Voice Engine & Dynamic Scheduling

# Chapter 4: 96 kHz Execution Timing, Cycle Budgets & Optimization

> **Document ID**: TRM-BMC-VOL3-004  
> **Classification**: Normative Hardware Reference & Benchmarking Guide  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 4.1 The 96.000 kHz Frame Execution Boundary

The Roland BMC DSP architecture enforces hard-real-time audio execution governed strictly by the audio sample clock:

$$F_s = 96.000\text{ kHz} \implies T_s = \frac{1}{F_s} = 10.4166\ \mu\text{s}$$

Within this brief $10.4166\ \mu\text{s}$ time window, every active DSP core must complete its entire instruction stream—encompassing the Main Program, all active dynamic slots, parameter loads, and output streaming to the Wave Bus.

```
+---------------------------------------------------------------------------------------------------+
|                                96 kHz SAMPLE FRAME TIME BUDGET ENVELOPE                           |
|                                                                                                   |
|  |<----------------------------------- 10.4166 microseconds ------------------------------------>|
|  |                                                                                                |
|  +-------------------------------------------------------+----------------------------------------+
|  |                  ACTIVE EXECUTION PERIOD              |            HARDWARE IDLE / HEADROOM    |
|  |  Main Program + Dynamic Slots (Oscillators/Filters)   |  Pipeline waits for next 96 kHz pulse  |
|  +-------------------------------------------------------+----------------------------------------+
|  ▲                                                       ▲                                        ▲
|  Frame Sync Strobe                                       Microcode Finish                         Next Strobe
|  (PC restarts at 0x0100)                                 (Return delimiter / trailer)             (Overrun if late)
+---------------------------------------------------------------------------------------------------+
```

---

## 4.2 Platform Timing Budgets & Overrun Limits

Because each instruction word consumes exactly **1 instruction cycle**, the execution budget is measured directly as the maximum allowable instruction count per sample frame:

| Metric / Dimension | Roland SYSTEM-8 | Roland SYSTEM-1 | Evidence Level |
|---|---|---|---|
| **Audio Sample Rate ($F_s$)** | 96.000 kHz | 96.000 kHz | `[MEASURED]` |
| **Sample Frame Period ($T_s$)** | $10.4166\ \mu\text{s}$ | $10.4166\ \mu\text{s}$ | `[MEASURED]` |
| **Nominal DSP Core Frequency** | $\approx 90.6\text{ MHz}$ | $\approx 68.0\text{ MHz}$ | `[INFERRED]` |
| **Measured Safe Cycle Budget** | **925 – 945 Instructions** | **703 – 708 Instructions** | `[MEASURED]` |
| **Confirmed Overrun Stall Limit** | $\ge 950$ Instructions | $\ge 826$ Instructions | `[MEASURED]` |
| **Main Program Byte Boundary** | 64 KiB Space | **6,144 Bytes (6 KiB)** | `[MEASURED]` |
| **Main Instruction Capacity** | $\approx 2,500$ Instructions | $\approx 600 - 720$ Instructions | `[MEASURED]` |

### 4.2.1 Measured Operating Envelope on SYSTEM-8 `[MEASURED]`
- Swept systematically using calibrated filler loops (`re/dspbudget.py`).
- At **900 filler instructions + 22 operational instructions (922 cycles)**, the DSP core runs with 100% audio fidelity and zero jitter.
- Beyond **945 cycles**, the pipeline overruns the hardware frame sync strobe.
- **Overrun Consequences**: The hardware drops sample frames, resulting in harsh periodic clicking, loss of voice synchronization, or an unrecoverable DSP watchdog crash that leaves the instrument unresponsive until a full power cycle.

### 4.2.2 Measured Operating Envelope on SYSTEM-1 `[MEASURED]`
- On Slave DSP A, a benchmark kernel with **703 instructions executed cleanly** without dropped samples.
- A test kernel expanded to **826 instructions triggered an immediate system freeze**.
- On Slave DSP B, programs up to **708 instructions** ran with full stability.
- **Physical Size Trap**: On SYSTEM-1, exceeding **6,144 bytes (`0x18FF`)** in the Main Program halts the DSP core immediately, even if total instruction count is well within the 703-cycle limit (e.g. 6,534 bytes with 702 instructions failed, whereas 5,854 bytes with 703 instructions succeeded).

---

## 4.3 Deterministic Instruction Cost Model `[VERIFIED]`

The BMC DSP hardware implements a completely flat cycle execution profile:

$$\text{Total Execution Time} = \sum_{i=1}^{N_{\text{instructions}}} \text{Cycles}(i)$$

| Instruction Structure | Content | Measured Execution Cost |
|---|---|---|
| **Simple Scalar Op** | 1 Unit A ALU (`addxy r0, r1, r2`) | **1 Cycle** |
| **SFU Function** | 1 Unit B Non-Linear Op (`recipseed r0, r1`) | **1 Cycle** |
| **Phase Modulator** | 1 Unit B Phase Op (`ph_wrap r1, r3, r1`) | **1 Cycle** |
| **Dual MAC Instruction** | Parallel Products + Accumulator (`mac_p1p2`) | **1 Cycle** |
| **Complex VLIW Bundle** | ALU A + ALU B + MAC + up to 6 Parallel Transfers | **1 Cycle** |
| **Taken Branch (`JMP`)** | Branch executed to non-sequential target | **2 Cycles** (1 op + 1 pipeline refill) |
| **Untaken Branch** | Fall-through condition | **1 Cycle** |
| **Predicated Suppression** | Instruction suppressed via `cond` skip mask | **0 Cycles** (Bypassed) |

---

## 4.4 High-Impact Microcode Optimization Techniques

To maximize polyphony within the strict 703-cycle (SYSTEM-1) or 945-cycle (SYSTEM-8) budgets, microcode compilers and engineers must leverage native architectural features:

### 4.4.1 Phase Wrapping: `PH_WRAP` vs Manual Mathematical Wrapping `[MEASURED]`
- **Naive Fixed/Floating Wrap**: Involves a multi-instruction chain of comparisons, multipliers, subtractions, and minimum limits:
  ```dsp
  ; Manual Phase Wrap Chain (Consumes 11 to 13 Instructions per Oscillator)
  mul r3, r3, #256.0
  addxy r1, r3, r1
  min r1, r1, r6
  ; ... repeated 4 times with scaling ...
  subxy r1, r5, r1
  ```
- **Native Accelerator (`ph_wrap`)**: Executes in **1 instruction cycle**:
  ```dsp
  ; Native Hardware Phase Wrap (Consumes 1 Cycle)
  ph_wrap r1, r3, r1
  ```
- **Net Optimization**: Saves **10 cycles per oscillator**. Across an 8-voice dual-oscillator synth (16 oscillators), this recovers **160 instruction cycles per frame**—freeing up over 22% of the total SYSTEM-1 budget.

### 4.4.2 VLIW Parallel Instruction Bundling (Instruction Fusion) `[VERIFIED]`
Because a VLIW instruction bundling an ALU operation, an SFU operation, and parallel memory transfers costs the same single cycle as an isolated transfer, the scheduling pass (`asm/dspsched.py`) fuses independent operations:

```dsp
; Unoptimized Serial Microcode (Consumes 3 Cycles)
addxy r1, r1, r2
ld r3, [0400]
st r1, [0300]

; Optimized Fused Parallel VLIW Microcode (Consumes 1 Cycle)
addxy r1, r1, r2 | ld r3, [0400] | st r1, [0300]
```

### 4.4.3 Indexed Memory Lookups (`LDIDX`) vs Polynomial Expansions
Wavetable oscillators and complex non-linear transfer curves can be implemented via indexed loads (`ldidx rD, offset`) through hidden register `h6`, reading Bank 0 in a single cycle rather than evaluating multi-term Chebyshev polynomial expansions.

---

## 4.5 Audio Telemetry & Overrun Verification Methodology

Because Slave BMC cores lack direct JTAG or UART telemetry to the host PC, verification of timing budgets is conducted via **USB Audio Telemetry Demodulation**:

```
+---------------------------------------------------------------------------------------------------+
|                                 USB AUDIO TELEMETRY TEST HARNESS                                  |
|                                                                                                   |
|  [ Slave DSP Core: Diagnostic Firmware ]                                                          |
|   - Slot executes target microcode under test.                                                    |
|   - Measures frame cycle headroom via hardware timer f5c.                                         |
|   - Modulates telemetry state onto fixed high-frequency sinusoids:                                |
|     * Carrier 1 (1000 Hz): Cycle Count Headroom (scaled 0.0 to 1.0)                               |
|     * Carrier 2 (2200 Hz): Frame Overrun Strobe (Pulse on timing deadline violation)              |
|                                        │                                                          |
|                                        ▼                                                          |
|  [ USB Audio Interface: 96 kHz Streaming ]                                                        |
|   Captures raw 32-bit floating-point audio:                                                       |
|   arecord -D hw:SYSTEM8,0 -f FLOAT_LE -r 96000 -c 2 capture.wav                                  |
|                                        │                                                          |
|                                        ▼                                                          |
|  [ Host Analysis Script: demodulate_telemetry.py ]                                                |
|   Heterodyne mixing + 250 Hz Low-Pass Filter ==> Exact Cycle Count & Zero-Overrun Verification     |
+---------------------------------------------------------------------------------------------------+
```
