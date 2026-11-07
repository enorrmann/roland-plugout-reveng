# Volume III: Voice Engine & Dynamic Scheduling

# Chapter 2: Roland SYSTEM-1 Voice Architecture & Dynamic Allocation

> **Document ID**: TRM-BMC-VOL3-002  
> **Classification**: Normative Architectural Specification  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 2.1 System-1 Voice Model: Dual-DSP Architecture

The Roland SYSTEM-1 operates on a streamlined Dual-BMC topology comprising a single Master BMC and a single Slave BMC. Unlike the 6-DSP chains of the SYSTEM-8, each SYSTEM-1 BMC hosts **two audio DSP cores**, designated **DSP A** and **DSP B**.

- **Factory Synthesizer Model**: Allocates **4 polyphonic voices** across DSP A and DSP B.
- **Extended Clean-Room Synthesizers (C-S1V8)**: Allocates up to **8 full polyphonic voices** on the single Slave BMC using optimized single-cycle microcode (`examples/system_1/`, `[MEASURED]`).

```
+---------------------------------------------------------------------------------------------------+
|                                  SYSTEM-1 VOICE DISPATCH TOPOLOGY                                 |
|                                                                                                   |
|  [ MASTER BMC ]                                                                                   |
|   Master ARM Core (μT-Kernel)                                                                     |
|    - Evaluates Keyboard & MIDI Input                                                              |
|    - Region 0 Allocator (model.c) updates Voice Parameter IDs:                                    |
|      0x110+v (Pitch), 0x120+v (Velocity/Gate), 0x128+v (Trigger)                                  |
|    - Master DSP A & B: Real-time modulation generators (LFO/Envelopes)                            |
|      Piped to Slave via IOMD Wave Bus 600Eh..6021h                                                |
|                                        │                                                          |
|                                        ▼                                                          |
|  [ SLAVE BMC: 4 to 8 Voices ]                                                                     |
|   Slave ARM Core (engine.c)                                                                       |
|    - Receives Parameter Updates (vt+0x30 handler)                                                 |
|    - Injects Pitch/Gate into Bank 5 via FD6C Syscall:                                             |
|      FD6C(sel=0, obj={0,0,0x30+v}, pitch_bits)  ==> Bank 5 Word A030h + v                         |
|      FD6C(sel=0, obj={0,0,0x38+v}, gate_bits)   ==> Bank 5 Word A038h + v                         |
|    - Dispatches Microcode to DSP A & DSP B via FC30h / FC34h                                      |
|                                                                                                   |
|    ┌───────────────────────────────────┐        ┌────────────────────────────────────┐            |
|    │           Slave DSP A             │        │            Slave DSP B             │            |
|    │  - Polyphonic Voice Oscillators   │        │  - Polyphonic VCF Filters & VCAs   │            |
|    │  - Evaluates Voices 0 to 7        ├───────>│  - Gated Amplitude Envelopes       │            |
|    │  - Sums Voices into Bank 5 A150h  │ Bank 5 │  - Scales by Master Level C000h    │            |
|    └───────────────────────────────────┘        └─────────────────┬──────────────────┘            |
|                                                                   │                               |
|                                                                   │ IOMD Stream (6006h - 600Dh)   |
|                                                                   ▼                               |
|  [ MASTER BMC: Audio Influx & DAC Output ]                                                        |
|   Master DSP B receives Slave stream ==> Master DSP A sums to 6000h/6001h ==> WM2418 DAC Output   |
+---------------------------------------------------------------------------------------------------+
```

---

## 2.2 Voice Control Parameter Protocol (Parameter Group 2) `[VERIFIED]`

The Master ARM broadcasts note allocations to the Slave BMC engine driver through **Parameter Group 2** message records:

| Parameter ID | Parameter Function | Allocation Window | Protocol Encoding & Values | Evidence |
|---|---|---|---|---|
| `0x110 + v` | Voice $v$ Note Pitch | `0x110` – `0x11F` (16 Voice slots) | Raw MIDI semitone index. Normalized to octaves relative to C0 ($n=12$). | `[VERIFIED]` |
| `0x120 + v` | Voice $v$ Velocity / Gate | `0x120` – `0x127` (8 Voice slots) | Key velocity ($1 \le \text{vel} \le 127$). **Value 0 indicates Key Release**. | `[VERIFIED]` |
| `0x128 + v` | Voice $v$ Envelope Retrigger | `0x128` – `0x12F` (8 Voice slots) | Binary trigger pulse ($1 = \text{Trigger Pulse}$, $0 = \text{Idle}$). | `[STATIC_ANALYSIS]` |
| `0x130` | Global Pitch Bend | 1 ID | 14-bit pitch wheel value centered at `0x2000`. | `[VERIFIED]` |
| `0x131` | Global Modulation Wheel | 1 ID | Continuous controller value ($0 \le v \le 127$). | `[VERIFIED]` |
| `0x134` | Expression Controller | 1 ID | Continuous expression value ($0 \le v \le 127$). | `[STATIC_ANALYSIS]` |

---

## 2.3 Slave Bank 5 Memory Injection (`FD6C` Service) `[VERIFIED]`

Because the SYSTEM-1 ARM Cortex-M3 lacks a hardware floating-point unit (VFP), parameter conversions from semitone integers to 32-bit floating-point bit patterns are computed via lookup tables or host precomputations. 

The Slave engine writes values into DSP Bank 5 by executing the fixed OS jump service **`0x0100FC00 + 0x16C` (`FD6C` / `SVC 0x16C`)**:

```c
/* Disassembly of FD6C Service in Slave OS 1.30 (Address 0x01009F4E) */
void sys1_write_bank5(int sel, const uint32_t *obj, uint32_t ieee_bits)
{
    uint32_t idx = obj[2];
    if (sel == 0) {
        /* Writes to Bank 5 offset A000 + idx (Visible to DSP A) */
        *(volatile uint32_t *)(0x40101000u + (idx << 2)) = ieee_bits;
    } else if (sel == 1) {
        /* Writes to Bank 5 offset A100 + idx (Visible to DSP A and DSP B) */
        *(volatile uint32_t *)(0x40101400u + (idx << 2)) = ieee_bits;
    }
}
```

### 2.3.1 Engine Callback Implementation (`vt+0x30`)

```c
/* Region 1 Engine Parameter Callback (examples/system_1/engine.c) */
static void engine_param_system1(engine_t *e, int id, int flag, int value)
{
    (void)flag;
    /* Process Pitch Updates for Voices 0 to 7 */
    if (id >= 0x110 && id < (0x110 + NV)) {
        int v = id - 0x110;
        uint32_t obj[3] = { 0, 0, (uint32_t)(0x30 + v) };
        /* Write octave ratio IEEE pattern to Bank 5 word A030 + v */
        ((bank_fn)SVC(0x16C))(0, obj, pitch_lut_bits[value]);
    }
    /* Process Gate Updates for Voices 0 to 7 */
    else if (id >= 0x120 && id < (0x120 + NV)) {
        int v = id - 0x120;
        uint32_t obj[3] = { 0, 0, (uint32_t)(0x38 + v) };
        /* Write Gate state (1.0f or 0.0f IEEE pattern) to Bank 5 word A038 + v */
        uint32_t gate_bits = (value > 0) ? 0x3F800000u : 0x00000000u;
        ((bank_fn)SVC(0x16C))(0, obj, gate_bits);
    }
}
```

---

## 2.4 Program Memory Architecture & Section Scheduling

Each DSP core on the SYSTEM-1 possesses **32 KiB of physical program memory** partitioned into six structural execution blocks:

```
Address Window          Size        Block Type             Execution Behavior
0x0000 - 0x00FF         256 B       OS Header              Hardwired hardware frame stub (DO NOT OVERWRITE)
0x0100 - 0x18FF         6 KiB       Main Program           Runs unconditionally on every sample frame
0x1900 - 0x30FF         6 KiB       Section 2 (Slot 0)     Runs conditionally if enabled by FC44h
0x3100 - 0x48FF         6 KiB       Section 3 (Slot 1)     Runs conditionally if enabled by FC44h
0x4900 - 0x60FF         6 KiB       Section 4 (Slot 2)     Runs conditionally if enabled by FC44h
0x6100 - 0x78FF         6 KiB       Section 5 (Slot 3)     Runs conditionally if enabled by FC44h
0x7FF2 - 0x7FFF         14 B        Termination Trailer    7 halfwords: ld r0,[e000], st r0,[a0xx], no-op
```

### 2.4.1 Section Allocation Jump Table (`FC44` / `FC40`) `[VERIFIED]`

Dynamic voice sections are placed using syscall `0x0100FC00 + 0x40` (`FC40`) and enabled/disabled via `0x0100FC00 + 0x44` (`FC44`):

| Section Index | Target DSP Core | Base Physical Address | Hardware Section Role |
|---|---|---|---|
| **0** | DSP A | `0x1900` | Section 2: Voice Slot 0 |
| **1** | DSP A | `0x4900` | Section 4: Voice Slot 2 |
| **2** | DSP B | `0x1900` | Section 2: Filter Slot 0 |
| **3** | DSP B | `0x4900` | Section 4: Filter Slot 2 |
| **4** | DSP A | `0x3100` | Section 3: Voice Slot 1 |
| **5** | DSP A | `0x6100` | Section 5: Voice Slot 3 |
| **6** | DSP B | `0x3100` | Section 3: Filter Slot 1 |
| **7** | DSP B | `0x6100` | Section 5: Filter Slot 3 |

### 2.4.2 Program Load Queue Queuing (`FC30` & `FC34`) `[VERIFIED]`

Microcode programs are dispatched to the DSP cores in descriptors structured as `{address, size_bytes, pointer_to_words}` terminated by `{0, 0, 0}`:

```c
static struct s1_prog dsp_a_loadset[] = {
    { 0x1900, SEC_BYTES,  mc_sec_code },   /* Load Section 2 Microcode */
    { 0x0100, MAIN_BYTES, mc_main_code },  /* Load Main Program Microcode */
    { 0x7FF2, TERM_BYTES, mc_term_code },  /* Load Hardware Termination Stub */
    { 0, 0, 0 }
};

void load_system1_engine(void)
{
    /* 1. Disable all active dynamic sections */
    for (int i = 0; i < 8; i++)
        ((slot_fn)SVC(0x44))(i, 0);

    /* 2. Dispatch microcode image tables to DSP A and DSP B */
    ((s1_queue_fn)SVC(0x30))(dsp_a_loadset);
    ((s1_queue_fn)SVC(0x34))(dsp_b_loadset);

    /* 3. Strobe active execution sections */
    ((slot_fn)SVC(0x44))(0, 1);  /* Enable DSP A Section 2 */
    ((slot_fn)SVC(0x44))(2, 1);  /* Enable DSP B Section 2 */
}
```

---

## 2.5 Multi-Voice Microcode Kernel (8-Voice C-S1V8) `[MEASURED]`

In custom 8-voice synthesis kernels, DSP A computes 8 oscillator phases in parallel, packing voices into upper and lower register banks (`r0`–`r3` and `r4`–`r7`), and accumulating the mix into Bank 5 word `[A150]`. DSP B loads `[A150]`, applies global envelope and volume scaling, and outputs the result to Wave Bus registers `[6006]` and `[6007]`:

```dsp
; DSP B Output Stage: Load Voice Sum from Bank 5 and stream to Wave Bus
ld r7, [c000]      ; Master Volume Level
ld r0, [a150]      ; Voice mix accumulated by DSP A
mov r4, r4         ; Delay slot padding
mul r1, r0, r7     ; Scale mix by output master volume
st r1, [6006]      ; Transmit to Master BMC Left Channel
st r1, [6007]      ; Transmit to Master BMC Right Channel
```
