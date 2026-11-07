# Volume III: Voice Engine & Dynamic Scheduling

# Chapter 3: Slot Program Model, Main Microcode & Dynamic Dispatch

> **Document ID**: TRM-BMC-VOL3-003  
> **Classification**: Normative Architectural Specification  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 3.1 Main Program vs Dynamic Slot Execution Model

Microcode execution within the Roland BMC DSP core is organized hierarchically into two tiers:
1. **The Main Program (`MAIN`)**: A permanent, statically linked execution thread located at base offset `0x0100`. It initializes every 96 kHz audio sample frame, prepares parameter mailboxes, routes inter-DSP bus signals, and executes core filtering and mixing algorithms.
2. **Dynamic Program Slots (`SLOTS`)**: Swappable modular microcode kernels located at higher address boundaries (e.g. `0x6000`, `0x8000`, `0xB000`). Slots implement selectable synthesis components (Analog Circuit Behavior [ACB] oscillator shapes, digital cross-modulation, chorus, flanger, overdrive).

```
+---------------------------------------------------------------------------------------------------+
|                           SAMPLE FRAME EXECUTION SEQUENCE (96.000 kHz)                            |
|                                                                                                   |
|  Frame Sync Strobe (Every 10.4166 microseconds)                                                   |
|       │                                                                                           |
|       ▼                                                                                           |
|  +---------------------------------------------------------------------------------------------+  |
|  |  Stage 1: Main Program (PC = 0x0100)                                                        |  |
|  |  - Marshals incoming Bank 5 / Bank 6 parameters from ARM                                    |  |
|  |  - Injects modulation parameters into Bank 1 slot input mailbox                             |  |
|  |  - Reads MXM inter-core bus inputs from prior DSP stage                                     |  |
|  +---------------------------------------------------------------------------------------------+  |
|       │                                                                                           |
|       ▼                                                                                           |
|  +---------------------------------------------------------------------------------------------+  |
|  |  Stage 2: Dynamic Program Slots (Sequential Dispatch)                                       |  |
|  |  - Slot 1 (DCO1, PC = 0x6000): Evaluates Primary Waveform                                  |  |
|  |  - Slot 2 (Sync, PC = 0x8000): Evaluates Phase Synchronization                              |  |
|  |  - Slot 3 (DCO2, PC = 0xB000): Evaluates Secondary Waveform / Cross Modulation              |  |
|  +---------------------------------------------------------------------------------------------+  |
|       │                                                                                           |
|       ▼                                                                                           |
|  +---------------------------------------------------------------------------------------------+  |
|  |  Stage 3: Main Program Re-entry / Output Consolidation                                      |  |
|  |  - Reads slot audio outputs from Bank 1 (with 1-sample ring offset)                         |  |
|  |  - Applies 4-pole resonant VCF and amplitude VCA                                            |  |
|  |  - Dispatches audio stream to MXM / Wave Bus IOMD registers                                 |  |
|  +---------------------------------------------------------------------------------------------+  |
|       │                                                                                           |
|       ▼                                                                                           |
|  Idle / Pipeline Stall wait state until next 96 kHz hardware sync pulse                          |
+---------------------------------------------------------------------------------------------------+
```

---

## 3.2 Program Memory Map & Slot Base Addresses

### 3.2.1 Roland SYSTEM-8 Program Space Partitioning (64 KiB Space) `[VERIFIED]`

On each DSP of the SYSTEM-8, program space spans 64 KiB (`0x41480000 + n * 0x100000` from the ARM perspective):

| Address Offset | Size | Program Type | Target DSP Core | Hardware Role / Function |
|---|---|---|---|---|
| `0x0000` – `0x00FF` | 256 B | Hardwired Frame Stub | All DSPs | Hardware frame entry vector. Must never be modified. |
| `0x0100` – `0x5FFF` | ~24 KiB | **Main Program (`MAIN0`–`MAIN5`)** | `DSP0`–`DSP5` | Primary signal routing, envelopes, mixing, and filters. |
| `0x6000` – `0x7FFF` | 8 KiB | **Slot 1 (DCO1)** | `DSP2`, `DSP3` | Primary Oscillator Slot (Task Bit 1). |
| `0x8000` – `0x9FFF` | 8 KiB | **Slot 2 (Aux/Sync)** | `DSP0`, `DSP1`, `DSP2`, `DSP3` | Auxiliary Modulators or Sync Logic (Task Bit 2). |
| `0xB000` – `0xDFFF` | 12 KiB | **Slot 3 (DCO2 / FX)** | `DSP2`, `DSP3`, `DSP4` | Secondary Oscillator (DSP2/3) or Plug-Out FX (DSP4). |
| `0xEFDC` / `0xF7DC` | 8 KiB | **Global Effects Slots** | Master `DSP4` | System-8 Global Reverb and Delay Networks. |

### 3.2.2 Roland SYSTEM-1 Program Space Partitioning (32 KiB Space) `[VERIFIED]`

On the SYSTEM-1, program memory is strictly capped at 32 KiB and partitioned into fixed 6 KiB modular segments:

| Address Range | Size | Allocation Name | Hardware Role | Control API |
|---|---|---|---|---|
| `0x0000` – `0x00FF` | 256 B | OS Header Stub | Hardware initialization words (`0000 0001...`). | Hardwired |
| `0x0100` – `0x18FF` | **6,144 B (6 KiB)** | **Main Program** | Core audio execution loop. Exceeding 6 KiB halts ASIC. | Loaded via `FC30`/`FC34` |
| `0x1900` – `0x30FF` | 6,144 B (6 KiB) | **Section 2 (Slot 0)** | Voice Slot 0 (DSP A) / Filter Slot 0 (DSP B). | Enabled via `FC44(0/2, 1)` |
| `0x3100` – `0x48FF` | 6,144 B (6 KiB) | **Section 3 (Slot 1)** | Voice Slot 1 (DSP A) / Filter Slot 1 (DSP B). | Enabled via `FC44(4/6, 1)` |
| `0x4900` – `0x60FF` | 6,144 B (6 KiB) | **Section 4 (Slot 2)** | Voice Slot 2 (DSP A) / Filter Slot 2 (DSP B). | Enabled via `FC44(1/3, 1)` |
| `0x6100` – `0x78FF` | 6,144 B (6 KiB) | **Section 5 (Slot 3)** | Voice Slot 3 (DSP A) / Filter Slot 3 (DSP B). | Enabled via `FC44(5/7, 1)` |
| `0x7FF2` – `0x7FFF` | 14 B | **Termination Trailer** | 7 halfword hardware closing stub. | Mandatory at `0x7FF2` |

---

## 3.3 Slot Framing: Headers, Return Stubs & Trailers

To allow the hardware sequencer to chain execution between the Main Program and active dynamic slots, every slot microcode binary must incorporate normative framing halfwords:

### 3.3.1 Slot Header Framing `[VERIFIED]`
Every dynamic slot microcode program must start with standard preamble words:
- **SYSTEM-8 Preamble**:
  ```
  Halfwords: 0x0A02  0xFF70
  ```
- **SYSTEM-1 Preamble**:
  ```
  Halfwords: 0x0A02  0xFF71
  ```

### 3.3.2 Slot Return Sequence (Subroutine Return) `[VERIFIED]`
At the conclusion of slot execution, the microcode returns control back to the Main Program sequencer via a dedicated return delimiter:
```
Halfwords: 0x0A02  0xFF70  0x0B02  0x00F8  0x0001
```
- The halfword `0x00F8` in position $B$ signals an internal pipeline return strobe, restoring the program counter to the Main Program continuation vector.

### 3.3.3 SYSTEM-1 Hardware Termination Trailer (`0x7FF2`) `[VERIFIED]`
On SYSTEM-1 DSP cores, program memory must conclude at address `0x7FF2` with a 7-halfword hardware termination trailer that latches the system frame counter:
```
0x7FF2:  0x0000             ; Alignment padding
0x7FF4:  ld r0, [e000]      ; Sample frame counter register
0x7FF8:  st r0, [a004]      ; Latch counter into Bank 5 word A004
0x7FFC:  0x0001             ; Final one-word NOP
```

---

## 3.4 Dynamic Slot Switching & Execution Gating

### 3.4.1 SYSTEM-8 Runtime Task Control Register `[VERIFIED]`
The Slave ARM dynamically gates slot execution by writing to the DSP task enable register at MMIO address `0x41104318 + n * 0x1000`:
- **Bit 1**: Enables Slot 1 (`0x6000`, DCO1).
- **Bit 2**: Enables Slot 2 (`0x8000`, Sync).
- **Bit 3**: Enables Slot 3 (`0xB000`, DCO2 / FX).
- Microcode hot-swapping is performed by halting the slot bit, streaming new microcode words to `0x41480000 + slot_offset`, and re-asserting the task enable bit.

### 3.4.2 SYSTEM-1 Section Control API (`FC44`) `[VERIFIED]`
On SYSTEM-1, dynamic sections are toggled on the fly by invoking the `0x0100FC44` syscall from the ARM control plane:

```c
/* Toggling Dynamic Voice Sections on SYSTEM-1 */
void set_section_state(int section_index, int enable)
{
    /* section_index: 0..7, enable: 1 (active) or 0 (bypass) */
    typedef void (*slot_gate_fn)(int idx, int state);
    ((slot_gate_fn)(*(void **)(0x0100FC44u)))(section_index, enable);
}
```

---

## 3.5 Inter-Slot State Isolation & Register Inheritance `[MEASURED]`

Because the hardware does not perform context saving when transitioning between Main and Slot microcode:
- **Register Persistence**: A slot program inherits whatever values were left in `r0`–`r7` by the preceding program.
- **Accidental Coupling Hazard**: If Slot 1 leaves an unmanaged value in `r4`, and Slot 2 assumes `r4` contains zero or a normalized constant, severe audio artifacts or DC offset will occur.
- **Normative Best Practice**: Every slot program must initialize its working registers explicitly or pass data cleanly across Bank 1 (`DT1`) mailbox offsets.
