# Volume VI: Reference Designs & Applications

# Chapter 2: The C-S1V8 8-Voice SYSTEM-1 Synthesizer Reference Design

> **Document ID**: TRM-BMC-VOL6-002  
> **Classification**: Normative Reference Implementation Guide  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 2.1 Overview: 8-Voice Polyphony on SYSTEM-1 Hardware

While the factory Roland firmware restricts the SYSTEM-1 to 4 polyphonic voices, the **`C-S1V8`** reference engine (`examples/system_1/`) demonstrates that the single Slave BMC hardware has sufficient computational bandwidth to execute **8 discrete polyphonic voices** simultaneously at 96 kHz.

- **Dual-DSP Partitioning**: Slave DSP A computes 8 oscillator phases in parallel; Slave DSP B executes dynamic 4-pole low-pass filtering and envelope VCAs.
- **Cycle Headroom**: DSP A executes in **517 cycles** (well within the 703-cycle limit); DSP B executes in **652 cycles** (within the 708-cycle limit).
- **RAMP Parameter Engine**: Leverages the hardware linear ramp interpolator (`FCE4`) and the end-of-ramp interrupt hook (`FE3C`) to execute smooth envelope transitions without consuming CPU timer ticks.

```
+---------------------------------------------------------------------------------------------------+
|                                  C-S1V8 8-VOICE PROCESSING PIPELINE                               |
|                                                                                                   |
|  [ MASTER BMC ]                                                                                   |
|   Region 0 Allocator (model.c)                                                                    |
|    - 8-Voice Polyphonic State Machine                                                             |
|    - Broadcasts Note IDs 0x110..0x117 & Velocity IDs 0x120..0x127                                 |
|                                       │                                                           |
|                                       ▼                                                           |
|  [ SLAVE BMC: ARM Core (engine.c) ]                                                               |
|   - Injects Pitch Patterns to Bank 5 (A030h..A037h) via FD6C                                      |
|   - Triggers Hardware Ramps for Envelopes via FCE4 / FE3C                                         |
|                                       │                                                           |
|                                       ▼                                                           |
|  +---------------------------------------------------------------------------------------------+  |
|  |  [ SLAVE DSP A: 8-Voice Polyphonic Oscillator Kernel ]                                       |  |
|  |  - Computes 8 Phase Accumulators in Parallel using Native ph_wrap                            |  |
|  |  - Lower Voices (0-3) in r0-r3; Upper Voices (4-7) in r4-r7                                  |  |
|  |  - Sums all 8 voice waveforms into Bank 5 word [A150h]                                      |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                       │                                                           |
|                                       │ Bank 5 Shared Memory [A150h]                              |
|                                       ▼                                                           |
|  +---------------------------------------------------------------------------------------------+  |
|  |  [ SLAVE DSP B: 8-Voice Filter & VCA Engine ]                                                |  |
|  |  - Reads Voice Sum from [A150h]                                                             |  |
|  |  - Applies Resonant 4-Pole Low-Pass Filter                                                   |  |
|  |  - Modulates by Hardware Master Volume [C000h]                                              |  |
|  |  - Dispatches Stereo Audio Stream to Wave Bus IOMD [6006h] & [6007h]                        |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                       │                                                           |
|                                       ▼                                                           |
|  [ MASTER BMC: Audio Routing & DAC Output (WM2418) ]                                              |
+---------------------------------------------------------------------------------------------------+
```

---

## 2.2 Parallel 8-Voice Microcode Kernel (DSP A)

To fit 8 voices into 517 cycles, DSP A leverages the single-cycle `ph_wrap` instruction and groups voices into symmetrical register pairs:

```dsp
; Voice 0 Phase Accumulator & Modulo Wrapping
ld r1, [0301]      ; Load previous phase (Voice 0)
mov r4, r4         ; Latency padding
addxy r1, r1, r2   ; Advance phase by frequency increment r2
ph_wrap r1, r3, r1 ; Native single-cycle wrap to [-1.0, +1.0)
st r1, [0300]      ; Store updated phase into Bank 0 ring buffer

; Voice 1 Phase Accumulator
ld r5, [0311]      ; Load previous phase (Voice 1)
mov r4, r4
addxy r5, r5, r6   ; Advance phase by frequency increment r6
ph_wrap r5, r3, r5 ; Native single-cycle wrap
st r5, [0310]      ; Store updated phase

; Accumulate Voice Audio into Bank 5 Inter-DSP Mailbox
addxy r1, r1, r5   ; Sum Voice 0 + 1
st r1, [a150]      ; Dispatch partial sum to Bank 5 word A150
```

---

## 2.3 Hardware Linear Parameter Ramps (`FCE4` & `FE3C`) `[MEASURED]`

Instead of software envelope calculation, C-S1V8 offloads envelope segments directly to the BMC ASIC parameter hardware:

1. **Hardware Ramp Trigger (`FCE4`)**:
   `FCE4(sel, id, target, y, z)` drives parameter `C0id` toward `target` along an exact linear ramp spanning **4 ms to 97 ms** (in discrete hardware steps of **1/3 ms**).
2. **Interrupt Chaining (`FE3C`)**:
   `FE3C(sel, id, callback)` registers an interrupt handler invoked by hardware when the linear ramp finishes. The handler immediately triggers the subsequent envelope phase (Attack $\to$ Decay $\to$ Sustain):

```c
/* Chaining Envelopes via Hardware Interrupts without Timer Ticks */
static void on_attack_complete(void)
{
    /* Transition to Decay: Ramp to Sustain Level over 45 ms */
    ((ramp_fn)SVC(0xE4))(1, PARAM_VCA, sustain_level_bits, 12, 0);
    /* Re-arm interrupt for decay completion */
    ((hook_fn)SVC(0x23C))(1, PARAM_VCA, on_decay_complete);
}
```

---

## 6.2.4 Hardware Verification & Audio Loopback Test Rig `[MEASURED]`

Physical verification of running SYSTEM-1 Plug-Out engines is performed via synchronous USB-MIDI and USB Audio capture using `tools/s1_capture.py`:

| Parameter | Configuration | Verification Method |
|---|---|---|
| **USB MIDI Interface** | `hw:3,0,0` (`SYSTEM-1 MIDI 1`) | Serial ALSA raw MIDI client |
| **USB Audio Interface** | `hw:3,0` (`SYSTEM1` Audio Card) | Linux ALSA PCM capture device |
| **Audio Stream Format** | `S32_LE`, 96,000 Hz, 2 Channels (Stereo) | Signed 32-bit Little-Endian PCM |
| **MIDI Dispatch Channel** | Channel 5 (`0x94` Note-On / `0x84` Note-Off) | Hardware-configured communication channel |

### Automated Automated Verification Command
```bash
# Triggers Note-On 60 (Middle C), records 2.0 seconds of 96 kHz PCM, and asserts spectral stability
python3 tools/s1_capture.py --note 60 --duration 2.0 --output out.raw
```

