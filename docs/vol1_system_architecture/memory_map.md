# Volume I: System Architecture & Hardware

# Chapter 2: Global Memory Map & Address Spaces

> **Document ID**: TRM-BMC-VOL1-002  
> **Classification**: Normative Hardware Reference  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 2.1 Global Address Space Partitioning

The Roland BMC SoC implements a unified 32-bit physical address space for each processor node. Because the platform operates under an Asymmetric Multi-Processing (AMP) cluster architecture, **physical addresses are node-local**: identical numerical memory addresses denote physically distinct memory arrays depending on whether they are accessed from the Master BMC, Slave BMC 0, or Slave BMC 1.

```
+------------------+ 0xFFFFFFFF
|                  |
| Reserved / Periph|
|                  |
+------------------+ 0x62000000
|  External SDRAM  | (32 MB Space: OS Kernel, Plug-Out Regions, Heap)
+------------------+ 0x60000000
|                  |
| Memory-Mapped IO | (DSP Banks, MMIO Peripherals, Wave Bus Control)
|                  |
+------------------+ 0x40000000
| System SRAM / TCM| (Fast Execution, RTOS Stacks, Bootloader)
+------------------+ 0x01000000
| Boot ROM / Vector|
+------------------+ 0x00000000
```

---

## 2.2 Global Physical Address Map (Host ARM Control Plane)

The table below defines the normative physical memory map for both Master and Slave BMC ASICs:

| Address Range | Size | Access | Description / Master Context | Description / Slave Context | Evidence |
|---|---|---|---|---|---|
| `0x0000_0000` – `0x0000_7FFF` | 32 KiB | R/W | Internal Boot ROM / Fast SRAM | Internal Boot ROM / Fast SRAM | `[STATIC_ANALYSIS]` |
| `0x0100_0000` – `0x0100_03FF` | 1 KiB | R/W | Vector Table & Fast ISR Vectors | Vector Table & Fast ISR Vectors | `[STATIC_ANALYSIS]` |
| `0x0100_0400` – `0x010F_FFFF` | ~1 MiB | R/W | Master Parameter Engine Kernel | Slave Kernel Runtime | `[VERIFIED]` |
| `0x0110_0000` – `0x0111_FFFF` | 128 KiB | R/W | `m1_tcm` (Master Core 1 TCM) | `s_boot` (Slave Boot & Voice Control) | `[VERIFIED]` |
| `0x0113_FC00` – `0x0113_FFFF` | 1 KiB | R | Master OS Fixed Jump Table | **Slave Fixed Syscall Jump Table** | `[VERIFIED]` |
| `0x4000_0000` – `0x400F_FFFF` | 1 MiB | R/W | System Control, Clocks, Timers, UART | System Control, Clocks, Timers, UART | `[STATIC_ANALYSIS]` |
| `0x4010_0000` – `0x4010_FFFF` | 64 KiB | R/W | *(S1)* DSP Subsystem Common Control | *(S1)* Shared Bank 5 (`FD6C` Access) | `[MEASURED]` |
| `0x4014_C000` – `0x4014_FFFF` | 16 KiB | R/W | *(S1)* DSP A Parameter Block | *(S1)* DSP A Parameters (`C000`–`C0FF`) | `[MEASURED]` |
| `0x4016_C000` – `0x4016_FFFF` | 16 KiB | R/W | *(S1)* DSP B Parameter Block | *(S1)* DSP B Parameters (`C000`–`C0FF`) | `[MEASURED]` |
| `0x4110_4318` + $n\cdot$`0x1000` | 4 KiB | R/W | *(S8)* DSP $n$ Slot Task Control | *(S8)* DSP $n$ Slot Task Control | `[VERIFIED]` |
| `0x4111_4000` + $n\cdot$`0x4000` | 16 KiB | R/W | *(S8)* DSP $n$ Bank 5 (DPC floats) | *(S8)* DSP $n$ Bank 5 (DPC floats) | `[VERIFIED]` |
| `0x4113_E000` – `0x4113_E3FF` | 1 KiB | R/W | *(S8)* IOMD Buffer Registers (RAW) | *(S8)* IOMD Buffer Registers (RAW) | `[MEASURED]` |
| `0x4140_0000` + $n\cdot$`0x100000` | 64 KiB | R | *(S8)* DSP $n$ DT0 (IEEE Float View) | *(S8)* DSP $n$ DT0 (IEEE Float View) | `[VERIFIED]` |
| `+0x0000_2000` | 64 KiB | R | *(S8)* DSP $n$ DT0 (SD31 Q31 View) | *(S8)* DSP $n$ DT0 (SD31 Q31 View) | `[VERIFIED]` |
| `+0x0000_4000` | 64 KiB | R | *(S8)* DSP $n$ DT0 (RAW 32-bit View) | *(S8)* DSP $n$ DT0 (RAW 32-bit View) | `[VERIFIED]` |
| `0x4148_0000` + $n\cdot$`0x100000` | 64 KiB | R/W | *(S8)* DSP $n$ Program Memory Space | *(S8)* DSP $n$ Program Memory Space | `[VERIFIED]` |
| `0x6000_0000` – `0x607F_FFFF` | 8 MiB | R/W | Master μT-Kernel OS Executable | Slave `s_data` Image / Voice Engine | `[VERIFIED]` |
| `0x6080_0000` – `0x609F_FFFF` | 2 MiB | R/W | *(S1)* Plug-Out Region 0 (Model) | *(S8)* Plug-Out Slot 0 Engine (Reg 1) | `[VERIFIED]` |
| `0x6020_0000` – `0x603F_FFFF` | 2 MiB | R/W | *(S8)* System-8 Engine Driver Image | *(S1)* Plug-Out Region 1 (Engine) | `[VERIFIED]` |
| `0x60A0_0000` – `0x60BF_FFFF` | 2 MiB | R/W | *(S8)* Plug-Out Slot 1 Engine (Master) | *(S8)* Plug-Out Slot 1 Engine (Slave) | `[VERIFIED]` |
| `0x60C0_0000` – `0x60DF_FFFF` | 2 MiB | R/W | *(S8)* Plug-Out Slot 2 Engine (Master) | *(S8)* Plug-Out Slot 2 Engine (Slave) | `[VERIFIED]` |
| `0x60E0_0000` – `0x60FF_FFFF` | 2 MiB | R/W | *(S8)* Plug-Out Slot 3 Engine (Master) | *(S8)* Plug-Out Slot 3 Engine (Slave) | `[VERIFIED]` |
| `0x6100_0000` – `0x615F_FFFF` | 6 MiB | R/W | Master Core 1 SDRAM Engine | Unallocated Slave SDRAM | `[VERIFIED]` |
| `0x6160_0000` – `0x617F_FFFF` | 2 MiB | R/W | *(S8)* Plug-Out Slot 0 Model (Reg 0) | Unallocated Slave SDRAM | `[VERIFIED]` |
| `0x6180_0000` – `0x619F_FFFF` | 2 MiB | R/W | *(S8)* Plug-Out Slot 1 Model (Reg 0) | Unallocated Slave SDRAM | `[VERIFIED]` |
| `0x61A0_0000` – `0x61BF_FFFF` | 2 MiB | R/W | *(S8)* Plug-Out Slot 2 Model (Reg 0) | Unallocated Slave SDRAM | `[VERIFIED]` |
| `0x61C0_0000` – `0x61DF_FFFF` | 2 MiB | R/W | *(S8)* Plug-Out Slot 3 Model (Reg 0) | Unallocated Slave SDRAM | `[VERIFIED]` |

> [!WARNING]
> **Context-Dependent Address Hazard**: Physical address `0x60000000` maps to the **Master μT-Kernel OS image** when referenced on the Master BMC, but references the **Slave `s_data` voice engine** on a Slave BMC. Calling Master OS entry points from Slave Region 1 code causes catastrophic invalid branch faults and hardware lockups.

---

## 2.3 DSP Core Internal Memory Spaces

Each proprietary VLIW DSP core addresses its local internal memory through a 16-bit reference space partitioned into 8 logical memory banks (Banks 0 through 7). 

A 16-bit DSP memory reference is structured as follows:

```
  15    13 12                                                   0
┌─────────┬───────────────────────────────────────────────────────┐
| Bank ID | Offset within Bank                                    |
| (3-bit) | 13-bit word address (0x0000 - 0x1FFF)                 |
└─────────┴───────────────────────────────────────────────────────┘
```

### 2.3.1 DSP Internal Memory Bank Architecture

| Bank ID | Hex Prefix | Bank Name | Word Capacity | Addressing Behavior | Inter-DSP Visibility | Access Rules | Evidence |
|---|---|---|---|---|---|---|---|
| **Bank 0** | `0nnn` | **DT0** Data RAM | 2,048 (0x800) | **Rotating Ring Buffer** (wraps modulo 0x800) | Core Local Only | Max 2 loads per instruction word | `[VERIFIED]` |
| **Bank 1** | `2nnn` | **DT1** Slot Mailbox | 2,048 (0x800) | Rotating Ring Buffer | Main $\leftrightarrow$ Active Slot | 1 load / 1 store per instruction | `[VERIFIED]` |
| **Bank 2** | `4nnn` | **DT2** Delay DMA | Windowed | Ping-Pong DMA Buffer Pages | Core Local Only | Dedicated tap requests (`tapreq`) | `[MEASURED]` |
| **Bank 3** | `6nnn` | **IOMD** Serial Bus | 256 (0x100) | Static Persistent Register Array | Inter-ASIC Wave Bus | Hardware owner DSP slot only | `[VERIFIED]` |
| **Bank 4** | `8nnn` | **MXM** Crossbar | 4 × 512 (0x200)| Latched Single-Sample Pipeline | Paired DSP Cores | Zones M, O, Z, S | `[VERIFIED]` |
| **Bank 5** | `Annn` | **DPC** Shared Mail | 1,024 (0x400) | Static Persistent RAM | `A000`..`A3FF`: Local; `A400`..`A7FF`: Paired | Max 1 load & 1 store per inst. | `[VERIFIED]` |
| **Bank 6** | `Cnnn` | **PARAM** Registers | 512 (0x200) | Hardware Linear Ramp Generators | Core Local Only | Read-Only for DSP, Written by ARM | `[VERIFIED]` |
| **Bank 7** | `Ennn` | System Telemetry | 256 (0x100) | Hardware Counter Registers | Core Local Only | `E000` = Global 96 kHz Frame Ctr | `[VERIFIED]` |

---

## 2.4 Control Register Bitfields

### 2.4.1 Slot Execution Task Control Register (`0x41104318` + $n\cdot$`0x1000`) `[VERIFIED]`

Controls runtime dispatch and execution gating for DSP slot microcode segments:

```
  31                                           4   3   2   1   0
┌───────────────────────────────────────────────┬───┬───┬───┬───┐
│                   Reserved                    │S3 │S2 │S1 │EN │
└───────────────────────────────────────────────┴───┴───┴───┴───┘
```

| Bit Field | Name | Access | Reset | Description |
|---|---|---|---|---|
| 31:4 | `RESERVED` | RO | 0 | Reserved for future expansion; reads as zero. |
| 3 | `SLOT3_EN` | R/W | 0 | Slot 3 microcode execution enable (FX / Upper Oscillator). |
| 2 | `SLOT2_EN` | R/W | 0 | Slot 2 microcode execution enable (DCO2 / Dynamic Slot). |
| 1 | `SLOT1_EN` | R/W | 0 | Slot 1 microcode execution enable (DCO1 / Primary Osc). |
| 0 | `DSP_CORE_EN` | R/W | 0 | Master DSP core execution strobe enable. When cleared, DSP idles. |

### 2.4.2 Hardware Parameter Ramp Register (`0x4016C000` + $16\cdot$`id`) `[MEASURED]`

Represents the memory-mapped configuration structure manipulated by the ARM control plane to trigger automatic linear parameter interpolation within Bank 6 (`C0id`):

```
Offset 0x00: [ Mode / Target Status ]  (Bit 31:28 = Mode Control, Bit 27:0 = Target Fraction)
Offset 0x04: [ Start Value RAW ]      (32-bit Roland RAW encoded floating point)
Offset 0x08: [ Current Value RAW ]    (32-bit Roland RAW actively updated every 1/3 ms)
Offset 0x0C: [ Delta Step RAW ]       (32-bit RAW linear step increment per tick)
```

- When triggered via the ARM service `FCE4` (`SVC 0xE4`), the hardware updates the `Current Value` field every **1/3 ms** across a programmed ramp duration ranging from **4 ms to 97 ms**.
- The DSP core samples this register synchronously via references to `[C0id]` without stalling the execution pipeline.

---

## 2.5 Hardware Diagnostic & Telemetry Map (`CEZUtilDSPDump` Table) `[STATIC_ANALYSIS]`

The Master OS (`m1_sdram` table at `0x61014BB0`) and Slave debug subsystem (`s_data`) maintain a 129-entry hardware descriptor table (`struct { uint32_t flags; const char *name; uint32_t address; uint32_t bytes; }`) used by the internal firmware console for memory inspection and DSP diagnostics:

| Entry | Symbol Name | Target MMIO Base | Byte Size | Word Count | Functional Role |
|---|---|---|---|---|---|
| 0 | `Sys Control` | `0x41100000` | `0x080` (128 B) | `0x20` (32) | Core system clock, reset, and DMA arbitration |
| 1 | `MXM0M` | `0x411D0000` | `0x800` (2 KiB) | `0x200` (512) | Mixer Crossbar Matrix 0 (Master input M) |
| 2 | `MXM0O` | `0x411D0800` | `0x800` (2 KiB) | `0x200` (512) | Mixer Crossbar Matrix 0 (Offset output O) |
| 3 | `MXZ0` | `0x411D1000` | `0x800` (2 KiB) | `0x200` (512) | Mixer Crossbar Matrix 0 (Zero output Z) |
| 4 | `MXS0` | `0x411D1800` | `0x800` (2 KiB) | `0x200` (512) | Mixer Crossbar Matrix 0 (Sum output S) |
| 5 | `MXE0` | `0x411D2000` | `0x040` (64 B) | `0x010` (16) | Mixer Crossbar Matrix 0 (Enable registers E) |
| 6 | `MXM1M` | `0x411D8000` | `0x800` (2 KiB) | `0x200` (512) | Mixer Crossbar Matrix 1 (Master input M) |
| 7 | `MXM1O` | `0x411D8800` | `0x800` (2 KiB) | `0x200` (512) | Mixer Crossbar Matrix 1 (Offset output O) |
| 8 | `MXZ1` | `0x411D9000` | `0x800` (2 KiB) | `0x200` (512) | Mixer Crossbar Matrix 1 (Zero output Z) |
| 9 | `MXS1` | `0x411D9800` | `0x800` (2 KiB) | `0x200` (512) | Mixer Crossbar Matrix 1 (Sum output S) |
| 10 | `MXE1` | `0x411DA000` | `0x040` (64 B) | `0x010` (16) | Mixer Crossbar Matrix 1 (Enable registers E) |

### 2.5.1 Per-DSP Core MMIO Block Layout (`0x41100000`)
- **Control Blocks**: `0x41102000` (DSP0), `0x41104000` (DSP1), `0x41105000` (DSP2), `0x41106000` (DSP3), `0x41107000` (DSP4), `0x41108000` (DSP5), `0x41109000` (DSPM).
  - Offset `+0x310`: Run/Mode register (bit 0 = core enable, bit 1 = single-step).
  - Offset `+0x318`: Task execution register (gating slots 1, 2, 3).
  - Offsets `+0x400`, `+0x700`: Indirect data register ports.
- **FIR Coefficient Tables**: `0x41152000` and `0x41153000`.
- **Slave Generic Bank Access Dispatcher**: Located in `s_boot` at entry point `0x0111C2E0`.
- **Inter-ASIC USB Bulk Link**: Master host sends parameter frames to slaves via `0x010125C1` $\to$ `0x0100D548` (`BMCBulk%d Attached`).

