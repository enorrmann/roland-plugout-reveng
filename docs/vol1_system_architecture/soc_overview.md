# Volume I: System Architecture & Hardware

# Chapter 1: Roland BMC SoC Architecture & Multi-Chip Topologies

> **Document ID**: TRM-BMC-VOL1-001  
> **Classification**: Normative Hardware Reference  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 1.1 Executive Overview

The **Roland BMC** (Custom Audio Application-Specific Integrated Circuit, internal project designation *Escadron*) is a proprietary heterogeneous multi-core System-on-Chip (SoC) designed by Roland Corporation to power the AIRA and Boutique line of digital synthesizer and modeling instruments (principally the **Roland SYSTEM-8** and **Roland SYSTEM-1**). 

The SoC integrates high-performance 32-bit control-plane processor cores (ARM Cortex-M architecture), proprietary high-throughput Very Long Instruction Word (VLIW) audio Digital Signal Processor (DSP) cores operating synchronously at a fixed 96 kHz frame boundary, flexible direct memory access (DMA) controllers, external memory interfaces (SDR SDRAM and QSPI Flash), high-speed serial audio links (Wave Bus / IOMD), and USB host/device transceivers.

```
+-----------------------------------------------------------------------------------------+
|                                    ROLAND BMC SoC                                       |
|                                                                                         |
|  +---------------------------+  +--------------------------+  +----------------------+  |
|  |     Control Subsystem     |  |      DSP Subsystem       |  |    Memory Systems    |  |
|  |                           |  |                          |  |                      |  |
|  | - ARM Cortex-M Core 0     |  | - 2 to 6 VLIW Audio DSPs |  | - External SDRAM Ctl |  |
|  |   (SysTick, MPU, DWT)     |  |   (96 kHz frame loop)    |  |   (0x60000000)       |  |
|  | - ARM Cortex-M Core 1     |  | - Special Function Units |  | - Internal SRAM/TCM  |  |
|  |   (Dual-core in S8 Master)|  |   (SFU: Recip, Sqrt, Exp)|  | - Dedicated DT0..DT7 |  |
|  | - μT-Kernel Real-Time OS  |  | - Dual Accumulators / MAC|  |   DSP Memory Banks   |  |
|  +---------------------------+  +--------------------------+  +----------------------+  |
|               |                              |                           |              |
|  +-----------------------------------------------------------------------------------+  |
|  |                        Interconnect & Peripheral Fabric                           |  |
|  |                                                                                   |  |
|  | - Wave Bus (IOMD High-Speed Serial Audio Matrix: 6000h - 60FFh)                    |  |
|  | - Inter-DSP Microcode Crossbar (MXM Bus: Zones M, O, Z, S)                         |  |
|  | - USB 2.0 High-Speed Host (BMCBulk Inter-ASIC Bus) / Device (Host DAW/SysEx)     |  |
|  | - High-Speed UARTs (STM32 Panel Controller Interconnect, 31.25k MIDI)              |  |
|  | - Delay Buffer DMA Engine (DT2 Circular Buffer Audio Storage)                      |  |
|  +-----------------------------------------------------------------------------------+  |
+-----------------------------------------------------------------------------------------+
```

---

## 1.2 System-Level Multi-BMC Topologies

To scale polyphony and processing complexity across instrument tiers, Roland deploys the BMC SoC in asymmetric multi-processing (AMP) clustered topologies. Identical physical silicon is utilized across master and slave nodes, differentiated at boot time through hardware strap pins, firmware images, and peripheral wiring.

### 1.2.1 Roland SYSTEM-8 Clustered Topology (Triple-BMC) `[VERIFIED]`

The SYSTEM-8 employs a **Triple-BMC** distributed architecture yielding up to **8 polyphonic voices** for complex multi-oscillator Plug-Out models alongside dedicated master processing, global delay/reverb effects, and panel scanning.

```
                              Host PC (DAW / Web Installer)
                                            │
                                  USB Audio / MIDI (96 kHz)
                                            │
                     ┌──────────────────────┴──────────────────────┐
                     │ BMC #0: Master Controller                   │
                     │  - ARM Core 0: μT-Kernel OS, UI, Sequencer  │
                     │  - ARM Core 1: Master FX & Audio Routing    │
                     │  - DSP0..DSP5 + DSPM: Reverb, Chorus, MFX   │
                     │  - SDRAM: 32 MB (0x60000000)                │
                     └──────────────┬──────────────────────────────┘
                                    │ Internal USB Host ("BMCBulk")
                                    │ Parameter Updates & Note Dispatch
                     ┌──────────────┴──────────────┐
                     │                             │
      ┌──────────────┴──────────────┐┌─────────────┴──────────────┐
      │ BMC #1: Slave 0             ││ BMC #2: Slave 1            │
      │  - Voices 0 to 3 Engine     ││  - Voices 4 to 7 Engine    │
      │  - ARM Core: s_boot/s_data  ││  - ARM Core: s_boot/s_data │
      │  - DSP0..DSP5: Voice Micro  ││  - DSP0..DSP5: Voice Micro │
      │  - SDRAM: 16 MB (0x60000000)││  - SDRAM: 16 MB (0x60000000)│
      └──────────────┬──────────────┘└─────────────┬──────────────┘
                     │                             │
                     │  IOMD Serial Wave Bus       │ IOMD Slave 1 Out
                     │  Audio Stream (6020h/6021h) │ (6000h..6003h)
                     ▼                             │
            Summed Output Bus ◄────────────────────┘
                     │
                     ▼
           Master DAC (WM2418) / USB Stream
```

- **BMC #0 (Master)**: Hosts the primary human-machine interface (HMI), MIDI routing engine, USB device interface, parameter management framework (`SYNINFO`), and global effects chain (DSP4 slot `0xEFDC`, `0xF7DC`).
- **BMC #1 (Slave 0)**: Executes polyphonic synthesis microcode for Voices 0 through 3. DSP4 sums local voice audio with digital audio received from Slave 1 via IOMD registers `0x6000`–`0x6003`.
- **BMC #2 (Slave 1)**: Executes polyphonic synthesis microcode for Voices 4 through 7. Its post-VCA mix output is piped across the Wave Bus to Slave 0.

### 1.2.2 Roland SYSTEM-1 Clustered Topology (Dual-BMC) `[VERIFIED]`

The SYSTEM-1 employs an optimized **Dual-BMC** architecture delivering **4 polyphonic voices** in standard factory patches (extendable up to 6 or 8 voices under custom microcode scheduling) with single-slot Plug-Out execution.

```
                              Host PC (DAW / Web Installer)
                                            │
                                  USB Audio / MIDI (96 kHz)
                                            │
                     ┌──────────────────────┴──────────────────────┐
                     │ BMC #0: Master Controller                   │
                     │  - Single ARM Cortex-M (No FPU)             │
                     │  - μT-Kernel OS ("SYSTEM-1:Appli")          │
                     │  - DSP A, DSP B: Modulators, Master Mix     │
                     └──────────────┬──────────────────────────────┘
                                    │
                                    │ Control Mailbox & IOMD Wave Bus
                                    │ (IOMD 6006h-600Dh, 6026h-602Dh)
                                    │
                     ┌──────────────┴──────────────────────────────┐
                     │ BMC #1: Slave Engine                        │
                     │  - Single ARM Cortex-M (No FPU)             │
                     │  - Slave OS & Plug-Out Engine (Region 1)    │
                     │  - DSP A, DSP B: 4 Voice Polyphonic Engine  │
                     └─────────────────────────────────────────────┘
```

- **BMC #0 (Master)**: Manages UI controls, potentiometers/sliders, MIDI communication, patch memory, and runs real-time voice modulation algorithms (e.g., LFO, pitch envelope generation) transferred to the slave via IOMD registers `0x600E`–`0x6021`.
- **BMC #1 (Slave)**: Executes polyphonic oscillators, filters, and envelope VCAs across dual DSP cores (DSP A and DSP B). Audio mix is returned to the Master via IOMD registers `0x6006`–`0x600D`.

---

## 1.3 Architectural Comparison Matrix: SYSTEM-8 vs SYSTEM-1

The table below delineates the normative architectural differences between the two hardware platforms:

| Architectural Dimension | Roland SYSTEM-8 | Roland SYSTEM-1 | Evidence Level |
|---|---|---|---|
| **ASIC Configuration** | 3 × BMC (1 Master, 2 Slaves) | 2 × BMC (1 Master, 1 Slave) | `[VERIFIED]` |
| **DSP Cores per BMC** | 6 Audio DSPs (`DSP0`..`DSP5`) + `DSPM` | 2 Audio DSPs (`DSP A`, `DSP B`) | `[VERIFIED]` |
| **Total DSPs in System** | 21 Cores (7 per BMC × 3) | 4 Cores (2 per BMC × 2) | `[VERIFIED]` |
| **Control CPU Architecture** | ARM Cortex-M with VFPv4-SP (FPU) | ARM Cortex-M without FPU (Soft-FP) | `[VERIFIED]` |
| **CPU Core Topology** | Master: Dual-Core; Slaves: Single-Core | Master: Single-Core; Slave: Single-Core | `[MEASURED]` |
| **Audio Frame Rate ($F_s$)**| 96.000 kHz | 96.000 kHz | `[MEASURED]` |
| **DSP Instruction Cycle Budget** | **925 – 945 cycles** per sample | **703 – 708 cycles** per sample | `[MEASURED]` |
| **DSP Program Space (Main)** | Up to 64 KiB per DSP | **6 KiB** (6,144 bytes, `0x0100`–`0x18FF`) | `[MEASURED]` |
| **Total DSP Program Space** | 64 KiB (Dynamic slots in upper bank) | 32 KiB (Sectioned: 4 × 6 KiB + Term) | `[VERIFIED]` |
| **Inter-DSP Core Interconnect**| Dedicated MXM bus + Bank 5 mailbox | Shared Bank 5 (`0xA000`–`0xA1FF`) | `[VERIFIED]` |
| **Inter-ASIC Audio Streaming** | Wave Bus IOMD (`0x6000`–`0x6021`) | Wave Bus IOMD (`0x6006`–`0x602D`) | `[VERIFIED]` |
| **Plug-Out Slots** | 3 Concurrent Slots (`PLG1`, `PLG2`, `PLG3`)| 1 Single Slot (`PLUG-OUT`) | `[VERIFIED]` |
| **Base Factory Polyphony** | 8 Voices (4 per slave BMC) | 4 Voices (Single slave BMC) | `[VERIFIED]` |
| **Panel Hardware Interface** | STM32F103 MCU via high-speed UART | Integrated panel scanning logic | `[STATIC_ANALYSIS]` |

---

## 1.4 Clock Distribution & Timing Domains

The BMC architecture is governed by two synchronous master clock branches: the **Control Plane Domain** and the **Audio/DSP Synchronous Domain**.

```
                        +----------------------+
                        |  Master Crystal Osc  |
                        |      24.576 MHz      |
                        +----------+-----------+
                                   |
                   +---------------+---------------+
                   |                               |
                   ▼                               ▼
       +-----------------------+       +-----------------------+
       |   Control Plane PLL   |       |   Audio Synthesizer   |
       |  (ARM Cortex-M Core)  |       |     PLL Multiplier    |
       +-----------+-----------+       +-----------+-----------+
                   |                               |
                   | (S8: ~150 MHz)                | (49.152 MHz Master Audio)
                   | (S1: ~100-120 MHz)            |
                   ▼                               ▼
       +-----------------------+       +-----------------------+
       |  ARM System Clock     |       |   DSP Execution PLL   |
       |  - AHB/APB Bus Clocks |       |      (~90.6 MHz)      |
       |  - SysTick Timer      |       +-----------+-----------+
       |  - DWT Cycle Counter  |                   |
       +-----------------------+                   ▼
                                       +-----------------------+
                                       | Fixed 96.000 kHz Fs   |
                                       | Frame Strobe Generator|
                                       +-----------+-----------+
                                                   |
                                                   ▼
                                       +-----------------------+
                                       | Exactly 1 Audio Frame |
                                       | = 10.4166 microseconds|
                                       | = 944 Cycles (S8)     |
                                       | = 708 Cycles (S1)     |
                                       +-----------------------+
```

### 1.4.1 Control Plane Clock Domain `[MEASURED]`
- **System-8 ARM Cores**: Clocked at approximately 150 MHz. ARM cores feature full ARMv7E-M instruction support with hardware VFPv4 single-precision floating point (`-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16`).
- **System-1 ARM Cores**: Clocked at approximately 100–120 MHz. The core does not implement hardware floating-point instructions; soft-float runtime ABI is mandatory (`-mcpu=cortex-m3 -mfloat-abi=soft`).
- **Real-Time Cyclic Timer**: Both systems run a μT-Kernel cyclic handler at an exact measured period of **1.00 ms** (verified via DWT `CYCCNT` telemetry with autocorrelation > 0.99).

### 1.4.2 Audio & DSP Execution Domain `[MEASURED]`
- Audio sampling frequency is locked at $F_s = 96\text{ kHz}$ ($\text{period } T_s = 10.4166\ \mu\text{s}$).
- The DSP execution pipeline operates on a non-preemptive run-to-completion model. At every 96 kHz frame pulse, the DSP program counter restarts at `0x0100` (Main Program) and processes through active slot/section microcode.
- **Instruction Execution Rate**: Exactly **1 cycle per instruction word**, regardless of whether the instruction executes 1 ALU operation, an SFU accelerator, a parallel multiply-accumulate (MAC), or up to 6 parallel data transfers.
- **Stall & Overrun Behavior**: If total DSP instruction cycle count exceeds the available frame time (945 cycles on S8, 708 cycles on S1), execution overruns the hardware frame sync boundary, resulting in audio dropouts, loss of phase synchronization, or unrecoverable DSP slave watchdog timeout stalls.

---

## 1.5 Subsystem Interconnects & High-Speed Buses

The BMC architecture implements three distinct tiers of data interconnect:

1. **MXM (Microcode Crossbar Matrix)**: An ultra-low-latency inter-DSP register crossbar local to each BMC ASIC (present on SYSTEM-8). It links adjacent DSP pairs (0/1 $\to$ 2/3 $\to$ 4/5) with single-sample latch guarantees.
2. **IOMD Serial Wave Bus**: A multi-channel real-time serialized audio interconnect operating at 96 kHz. It streams 32-bit digital audio words between physical BMC ASICs with zero CPU overhead.
3. **Internal USB 2.0 Host ("BMCBulk")**: A proprietary internal bulk-transfer protocol connecting the Master ARM USB host to Slave ARM USB endpoints. Used during boot to stream Slave OS binaries (`s_boot.bin`, `s_data.bin`), upload synthesis engines (Plug-Out Region 1), and transmit runtime MIDI note/pitch-bend dispatch events.
4. **STM32 Panel Controller UART**: A dedicated high-speed bidirectional asynchronous link operating between the Master ARM core and the auxiliary STM32F103 microcontroller governing panel potentiometers, rotary encoders, faders, and tactile switch matrixes.
