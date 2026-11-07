# Volume II: Roland BMC DSP Core & ISA Reference

# Chapter 6: DSP Memory Banks, Circular Ring Buffers & DMA

> **Document ID**: TRM-BMC-VOL2-006  
> **Classification**: Normative Hardware Reference  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 6.1 DSP Memory Organization & Address Decoding

The Roland BMC DSP core accesses memory through 16-bit physical reference words partitioned into **eight logical banks** (Bank 0 through Bank 7). Memory addresses are denoted in hexadecimal with bank prefixes:

```
Reference: [ B n n n ]
             │ └─ Offset within bank (0x000 - 0x1FFF)
             └─── Bank Selector Bits 15:13 (000b to 111b)
```

```
+---------------------------------------------------------------------------------------------------+
|                                  BMC DSP INTERNAL MEMORY MAP                                      |
|                                                                                                   |
|  Bank 0 [0000h - 07FFh] : DT0 Rotating Ring Buffer (2048 Words, Modulo 0x800)                    |
|  Bank 1 [2000h - 27FFh] : DT1 Main Program <-> Slot Mailbox (2048 Words, Modulo 0x800)           |
|  Bank 2 [4000h - 4FFFh] : DT2 Windowed Delay Buffer DMA Aperture (External SDRAM Delay Lines)     |
|  Bank 3 [6000h - 60FFh] : IOMD Serial Wave Bus Registers (Inter-ASIC Audio Transport)            |
|  Bank 4 [8000h - 87FFh] : MXM Inter-DSP Crossbar (Zones M, O, Z, S; Paired DSP Cores)            |
|  Bank 5 [A000h - A7FFh] : DPC Dual-Port Static RAM (A000h-A3FFh Local, A400h-A7FFh Paired Mailbox|
|  Bank 6 [C000h - C1FFh] : Synthesizer Parameter Registers (ARM Hardware Ramp Controlled)         |
|  Bank 7 [E000h - E0FFh] : System Hardware Telemetry & Frame Counter Registers                     |
+---------------------------------------------------------------------------------------------------+
```

---

## 6.2 Bank 0 (DT0): Rotating Ring Buffer Mechanics `[VERIFIED]`

Bank 0 comprises 2,048 32-bit words (`0x800`). It functions as an autonomous, hardware-managed **circular rotating ring buffer**:

### 6.2.1 Ring Pointer Progression Semantics
At every 96 kHz audio sample tick ($T_s = 10.4166\ \mu\text{s}$), the internal ring base pointer $p$ increments by exactly one word modulo 2048:

$$p_{n+1} = (p_n + 1) \pmod{2048}$$

A microcode instruction referencing address $R \in [0, 2047]$ accesses physical memory location:

$$\text{Physical Address}(R) = (R + p_n) \pmod{2048}$$

### 6.2.2 State Retention Across Sample Frames
- **Intra-Sample Consistency**: Within a single 96 kHz frame execution, a value stored to $R$ is immediately readable from $R$.
- **Next-Sample Frame Readback**: In the subsequent frame $n+1$, the base pointer has shifted forward by 1 word. Therefore, data written to $R$ in sample $n$ **appears at address $R + 1$ in sample $n+1$**:
  $$\text{Data stored at } [R] \text{ during frame } n \equiv \text{Data read from } [R + 1] \text{ during frame } n+1$$
- **$k$-Sample Delay Lines**: Reading a signal delayed by $k$ audio samples is implemented by storing to $[R]$ and loading from $[R + k]$.

### 6.2.3 Canonical Phase Accumulator State Pattern `[VERIFIED]`

Synthesizer oscillators, LFOs, and filter states maintain continuity across audio frames by reading from $R + 1$ and storing back into $R$:

```dsp
; Preserving Oscillator Phase Across 96 kHz Audio Frames
ld r1, [0301]      ; Load phase state from previous sample (stored at 0300 in frame n-1)
mov r4, r4         ; 1-cycle load latency delay slot
addxy r1, r1, r2   ; Advance phase (r1 += r2 frequency delta)
st r1, [0300]      ; Store updated phase into 0300 (will appear at 0301 in frame n+1)
```

---

## 6.3 Bank 1 (DT1): Main-to-Slot Communication Mailbox `[VERIFIED]`

Bank 1 (`0x2000`–`0x27FF`) is a 2,048-word rotating ring buffer dedicated to passing data between the **Main Program** and **Dynamic Program Slots** (e.g. DCO1, DCO2, Custom FX) within the same DSP core.

### 6.3.1 Execution Timing & Phase Alignment
The Main Program executes before active slots within each audio frame. Consequently, communications between Main and Slots exhibit a deterministic 1-sample pipeline phase:

| Step | Executing Entity | Memory Reference | Execution Boundary | Functional Role |
|---|---|---|---|---|
| 1 | Main Program | Store `[2207]` | Sample $n$ | Main writes pitch/frequency parameter to Slot input mailbox. |
| 2 | Oscillator Slot | Load `[2207]` | Sample $n$ | Slot reads pitch input synchronously in current sample. |
| 3 | Oscillator Slot | Store `[2221]` | Sample $n$ | Slot outputs generated audio waveform to mailbox word `2221`. |
| 4 | Main Program | Load `[2222]` | Sample $n+1$ | Main reads slot audio output shifted by 1 word due to ring rotation. |

```dsp
; Main Program Routing Audio Output from Dynamic Slot 2 to Inter-DSP Bus
ld r1, [2222]      ; Load Slot 2 audio (written by slot into 2221 in previous sample)
mov r4, r4         ; Load delay slot
st r1, [8404]      ; Transmit audio across MXM bus to next DSP stage
```

---

## 6.4 Bank 2 (DT2): Delay DMA Window & SDRAM Buffers `[MEASURED]`

Bank 2 (`0x4000`–`0x4FFF`) provides a windowed direct memory access (DMA) aperture into large external SDRAM delay buffers:
- **Ping-Pong Buffer Rotation**: An autonomous hardware DMA engine streams circular delay lines between external SDRAM and internal Bank 2 cache lines.
- **Tap Request Instruction (`TAPREQ`)**: Microcode initiates delay read taps using hidden registers `h4` (delay base) and `h5` (tap destination).
- Used extensively in the SYSTEM-8 Master BMC DSP4 global reverb networks (`0xEFDC`, `0xF7DC`).

---

## 6.5 Bank 5 (DPC): Dual-Port Persistent Memory `[VERIFIED]`

Unlike Banks 0 and 1, **Bank 5 does NOT rotate**:
- Address references are static across consecutive sample frames.
- **Partitioning**:
  - `0xA000` – `0xA3FF` (Lower 1,024 words): Core-local persistent RAM. On Slave cores, `0xA000`–`0xA0FF` is updated by the ARM control plane with voice parameters (Pitch, Gate, Velocity).
  - `0xA400` – `0xA7FF` (Upper 1,024 words): Paired Core Mailbox. Shared symmetrically between adjacent cores (`DSP0`/`DSP1`, `DSP2`/`DSP3`, `DSP4`/`DSP5`).

---

## 6.6 Bank 6: Parameter RAM & Hardware Ramps `[VERIFIED]`

Bank 6 (`0xC000`–`0xC1FF`) contains synthesizer parameters updated dynamically by the ARM control plane:
- **Read-Only to DSP Microcode**: The DSP can only load from Bank 6 (`ld rX, [C0id]`). Stores are prohibited by hardware decoding logic.
- **Hardware Linear Ramp Generation**: When the ARM initiates a parameter update via the `FCE4` syscall, hardware interpolation logic ramps the value across 4 ms to 97 ms in discrete steps of **1/3 ms**. The DSP samples smooth intermediate values without software interpolation overhead.

---

## 6.7 Hardware Access Boundaries & Scheduling Rules

1. **Bank 0 Constraint**: Maximum of 2 loads and 2 stores per instruction word.
2. **Bank 5 Constraint**: Maximum of 1 load and 1 store per instruction word (Strictly 1 total access per instruction on SYSTEM-1).
3. **Slave Memory Access Wall**: Slave DSP cores must never address Bank 5 at or above `0xA200` (`[MEASURED]`). Memory accesses above `0xA1FF` cause corrupted latch values and audio anomalies.
