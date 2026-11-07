# Volume I: System Architecture & Hardware

# Chapter 4: Host Control Plane, ARM Subsystems & μT-Kernel OS

> **Document ID**: TRM-BMC-VOL1-004  
> **Classification**: Normative Hardware Reference  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 4.1 Control Plane Processor Architecture

The control plane of the Roland BMC platform is driven by 32-bit ARM Cortex-M microcontrollers embedded directly on the ASIC die. The processor hierarchy operates in an Asymmetric Multi-Processing (AMP) configuration with distinct core counts and floating-point capabilities across platforms:

```
+--------------------------------------------------------------------------------------------------+
|                                    CONTROL PLANE CORE ARCHITECTURE                               |
|                                                                                                  |
|                   SYSTEM-8                                             SYSTEM-1                  |
|  ┌───────────────────────────────────────────────┐ ┌──────────────────────────────────────────┐  |
|  │ Master BMC                                    │ │ Master BMC                               │  |
|  │  - Core 0: ARM Cortex-M4 with VFPv4-SP        │ │  - Core 0: ARM Cortex-M3 (Soft-Float)    │  |
|  │    (μT-Kernel OS, GUI, MIDI, Plug-Out Region 0│ │    (μT-Kernel OS, GUI, Plug-Out Region 0)│  |
|  │  - Core 1: ARM Cortex-M4 with VFPv4-SP        │ │                                          │  |
|  │    (Master FX Driver, Reverb/Delay Scheduling)│ │                                          │  |
|  └───────────────────────┬───────────────────────┘ └────────────────────┬─────────────────────┘  |
|                          │ Internal USB / Mailbox                       │ IOMD / Mailbox         |
|  ┌───────────────────────┴───────────────────────┐ ┌────────────────────┴─────────────────────┐  |
|  │ Slave BMCs (0 and 1)                          │ │ Slave BMC                                │  |
|  │  - Single Core: ARM Cortex-M4 with VFPv4-SP   │ │  - Single Core: ARM Cortex-M3 (Soft-Float│  |
|  │    (Slave OS Runtime, Plug-Out Region 1 Engine│ │    (Slave OS Runtime, Plug-Out Region 1) │  |
|  └───────────────────────────────────────────────┘ └──────────────────────────────────────────┘  |
+--------------------------------------------------------------------------------------------------+
```

### 4.1.1 Floating-Point Unit (VFP) Analysis `[VERIFIED]`

Disassembly of firmware binaries demonstrates a fundamental architectural distinction between the two platforms:

- **SYSTEM-8 Firmware**: Employs ARMv7E-M instruction binaries compiled with hardware floating-point enabled (`-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16`). Hardware VFP instructions (`vldr`, `vstr`, `vadd.f32`, `vmul.f32`) appear extensively throughout the Master OS (`dumps/os_full.bin`: 3,054 VFP instructions, 10.6 per 10 KiB) and Plug-Out Region 1 engine drivers (JX-3P: 679 VFP instructions, 24.6 per 10 KiB; SH-2: 992 VFP instructions, 47.5 per 10 KiB).
- **SYSTEM-1 Firmware**: Compiled strictly for soft-float ABI (`-mcpu=cortex-m3 -mfloat-abi=soft`). Analysis of Master OS 1.30, Slave OS 1.30, and SYSTEM-1 Plug-Out binaries reveals 0 to 8 VFP instructions (pure random noise, < 0.4 per 10 KiB). Executing hardware VFP opcodes on SYSTEM-1 triggers an unhandled `UsageFault` (Undefined Instruction) exception.

---

## 4.2 Software Execution Model & Firmware Roles

| Physical Core | Software Module | Load Address | Execution Role | Communication Interfaces |
|---|---|---|---|---|
| **S8 Master Core 0** | `m0_core0` (μT-Kernel) | `0x60000000` | UI, Menus, MIDI Router (`0x6000345C`), USB Stack, Sequencer | USB Host, STM32 UART, QSPI |
| **S8 Master Core 0** | Plug-Out Region 0 | `0x61600000` + slot·`0x200000` | High-level synthesis model, parameter metadata (`SYNINFO`), note allocator | Syscall interface to Core 0 OS |
| **S8 Master Core 1** | `m1_tcm` / `m1_sdram` | `0x01100000` / `0x61000000` | Global FX management (`CDSPSystem8Rev`, `Dly`, `Mst`), DAC setup | Shared memory IPC with Core 0 |
| **S8 Slave Core** | `s_boot` / `s_data` | `0x01100000` / `0x60000000` | DSP lifecycle management, voice parameter push (`0x01103F2C`), syscall table (`0x0113FC00`) | Internal USB endpoint, DSP MMIO |
| **S8 Slave Core** | Plug-Out Region 1 | `0x60800000` + slot·`0x200000` | Low-level synthesis engine, voice DSP microcode loader, voice callbacks | Syscall table (`0x0113FC00`) |
| **S1 Master Core** | `"SYSTEM-1:Appli"` | `0x60000000` | UI, Patch DB, Panel Scanner, Voice Modulation DSP driver, Syscalls (`0x0100FC00`) | USB, Flash, IOMD |
| **S1 Master Core** | Plug-Out Region 0 | `0x60800000` | Model object, panel parameter capture (`vt+0x30`), patch data table | Syscall table (`0x0100FC00`) |
| **S1 Slave Core** | Slave OS | `0x60000000` | Slave runtime, Voice DSP loader, Syscalls (`0x0100FC00`) | IOMD Wave Bus, DSP MMIO |
| **S1 Slave Core** | Plug-Out Region 1 | `0x60200000` | Engine object (`vt+0x30`), parameter conversion to DSP RAW format | Syscall table (`0x0100FC00`) |
| **Panel Micro** | `subcpu` (STM32F103) | `0x08000000` | Potentiometer ADC sampling, rotary encoder tracking, keybed scanning | High-speed UART to Master Core 0 |

---

## 4.3 μT-Kernel Operating System & Real-Time Scheduling

The master control plane runs **μT-Kernel**, a standardized Japanese industrial hard-real-time operating system optimized for resource-constrained embedded microcontrollers:

- **Preemptive Priority Scheduling**: Tasks are scheduled according to fixed priorities (1 to 255). High-priority tasks govern MIDI input packet reception and cyclic voice parameter dispatches; lower-priority tasks handle OLED graphics rendering, flash filesystem transactions, and menu navigation.
- **Synchronization Primitives**: Core tasks communicate via kernel event flags (`tk_set_flg`), counting semaphores (`tk_sig_sem`), and fixed-size memory pools (`tk_get_mpf`).

### 4.3.1 The 1.00 ms Real-Time Cyclic Handler `[MEASURED]`

In addition to standard preemptive tasks, the Slave RTOS executes an unswappable cyclic interrupt handler (`driver\cyclic_handle.c`, handler `0x01008983`) configured for a deterministic period of **1.00 ms**:

```c
/* Standard Hook Pattern for 1.00 ms Cyclic ARM Processing */
typedef void (*cyc_fn)(int i);
#define CYC_CLIENT (*(volatile cyc_fn *)0x20002BFCu)

static cyc_fn cyc_os;

static void custom_tick(int i)
{
    /* 1. Execute original OS cyclic operations first */
    if (cyc_os)
        cyc_os(i);

    /* 2. Self-healing hook maintenance */
    if (CYC_CLIENT != custom_tick) {
        cyc_os = CYC_CLIENT;
        CYC_CLIENT = custom_tick;
    }

    /* 3. Execute low-jitter parameter update logic (e.g. ADSR, LFO) */
    advance_arm_modulators();
}

/* Lifecycle management in Region 1 Engine Driver */
void engine_load(void)   { cyc_os = CYC_CLIENT; CYC_CLIENT = custom_tick; }
void engine_deinit(void) { if (CYC_CLIENT == custom_tick) CYC_CLIENT = cyc_os; }
```

- **Wall Clock Time Measurement**: Real-time measurements must use the ARM Data Watchpoint and Trace (DWT) cycle counter (`DWT_CYCCNT`, enabled via `DEMCR` bit 24 and `DWT_CTRL` bit 0), benchmarked against `SysTick->LOAD + 1`.
- **Latency & Execution Budget**: The cyclic handler runs in interrupt context. Operations must complete in < 50 $\mu\text{s}$ to avoid degrading the operating system's internal task deadlines. Up to 33 hardware register writes (`FCEC`) can be executed per millisecond without causing audio stutter or RTOS instability.

---

## 4.4 Panel Subsystem Interface (STM32F103 UART Bridge) `[STATIC_ANALYSIS]`

The front panel controls (knobs, sliders, LED buttons, pitch bend lever) are decoupled from the main BMC SoC and handled by a dedicated **STMicroelectronics STM32F103** ("subcpu") microcontroller operating at 72 MHz:

```
  ┌────────────────────────────────────────────────────────┐
  │                 Front Panel Hardware                   │
  │  - Continuous Potentiometers (12-bit ADC Scanning)     │
  │  - Rotary Encoders (Hardware Quadrature Timers)        │
  │  - Keybed Contact Matrix (High-Speed Scanning)         │
  │  - Multi-Color LED Matrix (High-Frequency PWM)         │
  └───────────────────────────┬────────────────────────────┘
                              │
                              ▼
  ┌────────────────────────────────────────────────────────┐
  │         Panel Controller Micro (STM32F103)             │
  │  - Firmware Base: 0x08000000                           │
  │  - Scan Loop: 1 ms Continuous Debounce & Hysteresis    │
  └───────────────────────────┬────────────────────────────┘
                              │
                              │ Bidirectional High-Speed UART
                              │ Packet Framing: [STX, ID, LEN, DATA..., CRC]
                              ▼
  ┌────────────────────────────────────────────────────────┐
  │           Roland BMC Master ARM (Core 0)               │
  │  - UART DMA Receive Buffer                             │
  │  - Parameter Injection Router (0x01000400)             │
  └────────────────────────────────────────────────────────┘
```

1. **Continuous Scanning**: The STM32 continuously samples all analog potentiometers via multi-channel DMA. Hysteresis algorithms filter analog noise before transmitting delta update packets.
2. **In-System Firmware Updates**: The Master μT-Kernel OS embeds the production STM32 binary payload within its flash image (`subcpu` section). During cold boot, the Master ARM verifies the STM32 firmware revision via UART query. If mismatched, the Master forces the STM32 into internal system bootloader mode and flashes the updated binary across the UART bridge.
