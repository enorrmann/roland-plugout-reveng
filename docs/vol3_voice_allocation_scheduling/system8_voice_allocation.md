# Volume III: Voice Engine & Dynamic Scheduling

# Chapter 1: Roland SYSTEM-8 Polyphonic Voice Architecture & Dispatch

> **Document ID**: TRM-BMC-VOL3-001  
> **Classification**: Normative Architectural Specification  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 1.1 Architectural Overview: The 8-Voice Clustered Model

The Roland SYSTEM-8 delivers **8 discrete polyphonic synthesizer voices** in native Plug-Out operations (JX-3P, JUNO-106, JUPITER-8, SH-2, and custom engines such as C-V4). 

Polyphony is distributed across an Asymmetric Multi-Processing (AMP) cluster of two physical Slave BMC ASICs:
- **Slave BMC 0**: Hosts **Voices 0 through 3** (Local Voices $lv \in [0, 3]$).
- **Slave BMC 1**: Hosts **Voices 4 through 7** (Local Voices $lv \in [0, 3]$, offset by $4 \times \text{Slave ID}$).

```
+---------------------------------------------------------------------------------------------------+
|                                  SYSTEM-8 VOICE ALLOCATION ARCHITECTURE                           |
|                                                                                                   |
|  [ MASTER BMC: ARM Core 0 ]                                                                       |
|   MIDI / Keybed Influx  ===>  Region 0 Note Allocator (model.c)                                   |
|                                - Evaluates Polyphonic Voice Stealing Rules                        |
|                                - Issues Parameter Updates: Note (0x429+v), Velocity (0x43A+v)     |
|                                                 │                                                 |
|                      Internal USB Broadcast Bus │ (Both Slaves receive all 8 voice IDs)           |
|                                                 ▼                                                 |
|                    ┌────────────────────────────┴───────────────────────────┐                     |
|                    ▼                                                        ▼                     |
|  [ SLAVE BMC 0: Voices 0 - 3 ]                           [ SLAVE BMC 1: Voices 4 - 7 ]            |
|   ARM Core (s_data / engine.c)                            ARM Core (s_data / engine.c)            |
|   - Local Voice filter: lv = id - 4*0                     - Local Voice filter: lv = id - 4*1     |
|   - Active condition: 0 <= lv < 4                         - Active condition: 0 <= lv < 4         |
|   - Writes Bank 5 & Bank 6 via syscalls                   - Writes Bank 5 & Bank 6 via syscalls   |
|   ┌──────────────────────────────────────────────┐        ┌──────────────────────────────────────┐|
|   │ DSP0/1 : Modulation, Pitch, LFO, Envelopes   │        │ DSP0/1 : Modulation, Pitch, LFO, Env │|
|   │ DSP2/3 : Dual Oscillators (DCO1/DCO2 Slots)  │        │ DSP2/3 : Dual Oscillators (DCO1/DCO2)│|
|   │ DSP4/5 : 4-Pole VCF, VCA, Output Summing     │        │ DSP4/5 : 4-Pole VCF, VCA, Output Sum │|
|   └──────────────────────┬───────────────────────┘        └──────────────────┬───────────────────┘|
|                          │                                                   │                    |
|                          │  Summed Voices 0..3                               │ Summed Voices 4..7 |
|                          ▼                                                   ▼                    |
|                [ IOMD Mix: 6020h/6021h ] <====== Wave Bus Link ====== [ IOMD 6000h..6003h ]       |
+---------------------------------------------------------------------------------------------------+
```

---

## 1.2 Note Dispatch Protocol & Parameter IDs

The Master ARM operating system broadcasts voice control parameters to both Slaves using **Parameter Group 2** message frames:

| Parameter ID | Parameter Name | Value Range | Function & Protocol Behavior |
|---|---|---|---|
| `0x429 + v` | Voice $v$ MIDI Pitch | $0 \le \text{note} \le 127$ | Transmits raw semitone index. Converted to volt/octave ratio by Engine. |
| `0x43A + v` | Voice $v$ Velocity / Gate | $0 \le \text{vel} \le 127$ | Key-on velocity ($1 \le \text{vel} \le 127$). **Value 0 indicates Key-Release (Gate Off)**. |
| `0x44B + v` | Portamento Legato Strobe | $0 \text{ or } 1$ | Portamento glide continuation flag. |
| `0x45C` | Highest Voice Track | $0 \le v \le 7$ | Telemetry flag identifying the active voice with highest pitch. |

### 1.2.1 Slave Local Voice Filtering `[VERIFIED]`

Because both Slave ASICs monitor the shared internal USB parameter bus, each Slave engine driver must filter incoming IDs based on its physical hardware strap index (`e->first = 4 * slave_id`):

```c
/* Region 1 Engine Parameter Handler (vt+0x30) */
void engine_param(engine_t *e, int id, float value)
{
    /* 1. Process Voice Note-On / Pitch Updates */
    if (id >= 0x429 && id < (0x429 + 8)) {
        int lv = id - 0x429 - e->first;
        if (lv >= 0 && lv < 4) {
            engine_set_pitch(e, lv, value);
        }
        return;
    }

    /* 2. Process Voice Gate / Velocity Updates */
    if (id >= 0x43A && id < (0x43A + 8)) {
        int lv = id - 0x43A - e->first;
        if (lv >= 0 && lv < 4) {
            engine_set_gate(e, lv, value);
        } else {
            /* Voice belongs to the other slave: update active stealing state */
            engine_track_peer_flags(e);
        }
        return;
    }
}
```

> [!CAUTION]
> If a custom Slave engine driver fails to mask parameter IDs against `e->first`, both Slaves will execute identical note events simultaneously. Because Slave 1 audio sums into Slave 0, voices will play back with phase-canceling double-trigger artifacts.

---

## 1.3 Intrachip Voice Partitioning (Roland Family A Pipeline)

On each Slave BMC ASIC, the 4 local voices ($lv \in [0, 3]$) are processed across two parallel three-stage DSP processing chains:
- **Chain A (Local Voices 0 and 1)**: `DSP0` (Envelopes) $\to$ `DSP2` (Oscillators) $\to$ `DSP4` (Filters/VCA/Mix).
- **Chain B (Local Voices 2 and 3)**: `DSP1` (Envelopes) $\to$ `DSP3` (Oscillators) $\to$ `DSP5` (Filters/VCA/Output).

Let $u = lv \gg 1$ denote the chain selector ($u = 0 \implies \text{Chain A}$, $u = 1 \implies \text{Chain B}$), and let $k = lv \ \& \ 1$ denote the sub-voice index ($k = 0 \implies \text{Voice A}$, $k = 1 \implies \text{Voice B}$).

### 1.3.1 DSP Hardware Role Assignment Table `[VERIFIED]`

| DSP Core | Stage | Local Voices | Execution Function | Main Microcode Program | Dynamic Slot Programs |
|---|---|---|---|---|---|
| **`DSP0`** | Stage 0 | Voices 0, 1 | Modulators, LFO, Pitch calculation, Filter ENV1, Amp ENV2 | `MAIN0` (5,066 bytes) | Slot `0x8000` (Aux Modulation) |
| **`DSP1`** | Stage 0 | Voices 2, 3 | Modulators, LFO, Pitch calculation, Filter ENV1, Amp ENV2 | `MAIN1` (5,018 bytes) | Slot `0x8000` (Aux Modulation) |
| **`DSP2`** | Stage 1 | Voices 0, 1 | Dual Oscillators: Mailbox marshaling & oscillator mixing | `MAIN2` (2,068 bytes) | DCO1 (`0x6000`), DCO2 (`0xB000`) |
| **`DSP3`** | Stage 1 | Voices 2, 3 | Dual Oscillators: Mailbox marshaling & oscillator mixing | `MAIN3` (2,018 bytes) | DCO1 (`0x6000`), DCO2 (`0xB000`) |
| **`DSP4`** | Stage 2 | Voices 0, 1 | 4-Pole VCF, VCA, Slave 1 Wave Bus summing, Master Mix Out | `MAIN4` (5,174 bytes) | Plug-Out FX (`0xB000`, Overdrive/Chorus) |
| **`DSP5`** | Stage 2 | Voices 2, 3 | 4-Pole VCF, VCA, Dispatches audio to DSP4 via Mailbox | `MAIN5` (5,016 bytes) | Slot `0x6000` (Auxiliary Processor) |

---

## 1.4 Bank 5 Hardware Memory Map for Voice Parameters

The Slave ARM writes note pitch and envelope trigger values directly into Bank 5 of the corresponding DSP core using the OS service in the fixed vector jump table (`0x0113FC00 + 0xB0`):

```c
/* Fixed Vector Jump Table Service +0xB0: Write Float to Bank 5 */
typedef void (*bank5_write_fn)(int dsp, const uint32_t *p_offset, float value);
#define OS_WRITE_BANK5 ((bank5_write_fn)(*(void **)(0x0113FC00u + 0xB0u)))
```

### 1.4.1 Voice-to-Register Address Mapping Matrix `[VERIFIED]`

| Local Voice ($lv$) | Target DSP Core | Pitch Register (`A000 + id`) | Gate Register (`A000 + id`) | VCA Core | VCA Gate Register | VCA Level Register |
|---|---|---|---|---|---|---|
| **Local Voice 0** | `DSP0` | `[A021]` | `[A022]` | `DSP4` | `[A029]` | `[A02A]` |
| **Local Voice 1** | `DSP0` | `[A033]` | `[A034]` | `DSP4` | `[A038]` | `[A039]` |
| **Local Voice 2** | `DSP1` | `[A021]` | `[A022]` | `DSP5` | `[A029]` | `[A02A]` |
| **Local Voice 3** | `DSP1` | `[A033]` | `[A034]` | `DSP5` | `[A038]` | `[A039]` |

### 1.4.2 Mathematical Pitch Formulation
Pitch values are converted by the ARM from standard MIDI semitone numbers into **fractional octave ratios** relative to MIDI Note 12 (C0 = 0.0):

$$v_{\text{pitch}} = \frac{\text{MIDI\_Note} - 12}{12.0} \in [0.0, \; 9.5833]$$

The receiving DSP microcode loads `v_pitch` from Bank 5 (`ld r1, [A021]`) and passes it to the `expsplit` or Horner pitch polynomial kernel.

---

## 1.5 Inter-Stage MXM Audio Transport & Voice Chaining

Data flows sequentially between stages across the hardware **MXM Bus (Bank 4)** with strict 1-sample pipeline latch timing:

```dsp
; DSP0 (Stage 0): Dispatch Pitch to DSP2 via MXM Zone Z
ld r1, [a021]      ; Load Voice 0 Pitch (updated by ARM)
mov r4, r4         ; Load delay slot
st r1, [8404]      ; Transmit to DSP2 Zone M word 0x004
ld r2, [a033]      ; Load Voice 1 Pitch
mov r4, r4
st r2, [8405]      ; Transmit to DSP2 Zone M word 0x005
```

```dsp
; DSP2 (Stage 1): Consume Pitch from MXM Zone M and feed Oscillator Slots
ld r1, [8004]      ; Receive Voice 0 Pitch from DSP0
mov r4, r4
st r1, [2207]      ; Write to DCO1 Slot input mailbox (Bank 1)
ld r2, [8005]      ; Receive Voice 1 Pitch from DSP0
mov r4, r4
st r2, [2214]      ; Write to DCO2 Slot input mailbox (Bank 1)
```

At Stage 2, `DSP5` routes its filtered Voices 2 and 3 into the paired core mailbox (`A400`/`A401`). `DSP4` reads `[A400]` and `[A401]`, adds local Voices 0 and 1, and streams the summed 4-voice audio mix out to `[6020]` (Left) and `[6021]` (Right) on the Wave Bus.
