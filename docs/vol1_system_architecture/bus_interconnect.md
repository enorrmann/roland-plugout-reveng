# Volume I: System Architecture & Hardware

# Chapter 3: Bus Interconnects, Wave Bus & Inter-ASIC Matrix

> **Document ID**: TRM-BMC-VOL1-003  
> **Classification**: Normative Hardware Reference  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 3.1 Overview of Inter-DSP and Inter-ASIC Interconnects

The Roland BMC platform incorporates a hierarchical communication matrix to stream audio samples and dispatch control parameters across DSP cores and multi-ASIC clusters at a fixed 96 kHz frame boundary:

1. **MXM (Microcode Crossbar Matrix, Bank 4)**: A synchronous single-cycle latch crossbar operating between the 6 DSP cores on a single SYSTEM-8 BMC ASIC.
2. **IOMD Serial Wave Bus (Bank 3)**: A dedicated inter-chip real-time digital audio transport linking physical BMC ASICs across board traces.
3. **Paired Core Mailbox (Bank 5, `0xA400`–`0xA7FF`)**: A shared dual-port memory segment coupling adjacent even/odd DSP cores (0/1, 2/3, 4/5).
4. **Delay Buffer DMA Engine (Bank 2, DT2)**: Hardware circular buffer DMA channels providing windowed memory access to SDRAM delay lines.

```
+---------------------------------------------------------------------------------------------------+
|                                 SYSTEM-8 INTER-ASIC AUDIO TOPOLOGY                                |
|                                                                                                   |
|  [SLAVE 1 BMC] (Voices 4-7)                                                                       |
|   DSP0/1 (Envelopes) --MXM--> DSP2/3 (Oscillators) --MXM--> DSP4/5 (VCA/Filter)                  |
|                                                                    |                              |
|                                                          IOMD Mix Out (6020h/6021h)               |
|                                                                    |                              |
|                                                       Wave Bus Link Cable                         |
|                                                                    |                              |
|  [SLAVE 0 BMC] (Voices 0-3)                                        v                              |
|   DSP0/1 (Envelopes) --MXM--> DSP2/3 (Oscillators) --MXM--> DSP4/5 (VCA/Filter)                  |
|                                                              ^     |                              |
|                                          IOMD Input (6000h..03h)   | (Summed Voices 0..7)         |
|                                                                    v                              |
|                                                          IOMD Mix Out (6020h/6021h)               |
|                                                                    |                              |
|  [MASTER BMC]                                                      v                              |
|   Global Effects DSP4 (Chorus/Reverb) <------------------ IOMD Input Stream                       |
|   Master Output DSPM (Limiter/Mastering)                                                          |
|   DAC WM2418 / USB Audio 96 kHz Output                                                            |
+---------------------------------------------------------------------------------------------------+
```

---

## 3.2 MXM Crossbar Architecture (SYSTEM-8 Bank 4) `[VERIFIED]`

On the SYSTEM-8 BMC ASIC, the 6 voice DSP cores are structured in three paired stages: Stage 0 (DSP0, DSP1: Modulation & Envelopes), Stage 1 (DSP2, DSP3: Digital Oscillators & Dynamic Slots), and Stage 2 (DSP4, DSP5: Filters & Output Mixers). 

Inter-core data transmission is orchestrated through **Bank 4 (MXM)**, which provides 4 memory zones of 512 words (`0x200`) each:

```
Reference: [ 8 z n n ]
             │ │ └─ Word offset within zone (0x000 - 0x1FF)
             │ └─── Zone Selector:
             │        0: Zone M (Even predecessor input: 0x8000 - 0x81FF)
             │        2: Zone O (Odd predecessor input:  0x8200 - 0x83FF)
             │        4: Zone Z (Forward output:         0x8400 - 0x85FF)
             │        6: Zone S (Feedback return input:  0x8600 - 0x87FF)
             └───── Bank 4 Selector (Bits 15:13 = 100b)
```

### 3.2.1 MXM Routing Topology & Physical Wiring

| Source Core | Writing Address | Destination Core | Reading Address | Channel Payload | Evidence |
|---|---|---|---|---|---|
| `DSP0` | Zone Z `[84nn]` | `DSP2` | Zone M `[80nn]` | Voice Chain A Modulation & Tone | `[VERIFIED]` |
| `DSP1` | Zone Z `[84nn]` | `DSP3` | Zone O `[82nn]` | Voice Chain B Modulation & Tone | `[VERIFIED]` |
| `DSP2` | Zone Z `[84nn]` | `DSP4` | Zone M `[80nn]` | Voice Chain A Audio (Osc Mix) | `[VERIFIED]` |
| `DSP3` | Zone Z `[84nn]` | `DSP5` | Zone O `[82nn]` | Voice Chain B Audio (Osc Mix) | `[VERIFIED]` |
| `DSP2` | Zone M `[80nn]` (Store) | `DSP0` | Zone S `[86nn]` | Feedback Link Stage 1 $\to$ 0 | `[STATIC_ANALYSIS]` |
| `DSP4` | Zone M `[80nn]` (Store) | `DSP2` | Zone S `[86nn]` | Feedback Link Stage 2 $\to$ 1 | `[STATIC_ANALYSIS]` |

### 3.2.2 MXM Hardware Latency & Latch Rules `[MEASURED]`

1. **Exact 1-Sample Pipeline Latch**: MXM transfers implement a synchronous hardware register barrier. A value written to Zone Z (`[84nn]`) during sample frame $n$ becomes visible in the destination core's Zone M/O (`[80nn]`/`[82nn]`) at the start of sample frame $n+1$, completely invariant to execution order within the sample.
2. **Zero Auto-Accumulation**: MXM registers do not sum concurrent writes. If multiple instructions target the same offset, the final write overwrites prior contents.
3. **Zero Sample Retention**: MXM channels lack static storage retention. If an offset is not explicitly refreshed in frame $n$, the receiving core reads `0.0` (digital silence) during frame $n+1$.
4. **Preservation of Offset Index**: The word index $nn$ is strictly 1:1 preserved across the link (`Z 8404` maps identically to `M 8004`).

```dsp
; Master Stage 0 (DSP0) transmitting Voice A & B Pitch to Stage 1 (DSP2)
ld r1, [a021]      ; Load Voice A Pitch from Bank 5 mailbox (ARM updated)
mov r4, r4         ; Pipeline load latency filler slot
st r1, [8404]      ; Transmit to DSP2 Zone M word 0x004 for frame n+1
ld r1, [a033]      ; Load Voice B Pitch from Bank 5 mailbox
mov r4, r4
st r1, [8405]      ; Transmit to DSP2 Zone M word 0x005 for frame n+1
```

---

## 3.3 Paired Core Mailbox (Bank 5, `0xA400`–`0xA7FF`) `[VERIFIED]`

Within each physical DSP pair (`DSP0`/`DSP1`, `DSP2`/`DSP3`, `DSP4`/`DSP5`), the upper 1,024 words (`0x400`) of Bank 5 are cross-mapped between the two cores:

- Words written by the **Even DSP Core** into `[A400]`–`[A7FF]` are directly addressable by the **Odd DSP Core**.
- Words written by the **Odd DSP Core** into `[A400]`–`[A7FF]` are directly addressable by the **Even DSP Core**.
- Neither core can read back its own writes within this shared window.

### 3.3.1 Output Summing in Stage 2 (`DSP5` $\to$ `DSP4`) `[VERIFIED]`

Because only `DSP4` possesses hardware ownership of the primary IOMD output streaming registers (`0x6020`, `0x6021`), `DSP5` dispatches its processed polyphonic voices (Voices 2 and 3) across the paired mailbox into `DSP4` for final summation:

```dsp
; DSP5 (Stage 2, Odd Core): Gate Voices 2 & 3 and transmit to DSP4 via Mailbox
ld r4, [a029]      ; Load Voice 2 Gate envelope coefficient
ld r1, [8200]      ; Load Voice 2 Audio from DSP3 (via MXM Zone O)
mov r4, r4
mul r1, r1, r4     ; Apply Voice 2 Gate
st r1, [a400]      ; Dispatch Voice 2 to Paired Mailbox word A400 (read by DSP4)
ld r4, [a038]      ; Load Voice 3 Gate envelope coefficient
ld r2, [8201]      ; Load Voice 3 Audio from DSP3
mov r4, r4
mul r2, r2, r4     ; Apply Voice 3 Gate
st r2, [a401]      ; Dispatch Voice 3 to Paired Mailbox word A401 (read by DSP4)
```

In `DSP4`, the main program reads `[A400]` and `[A401]`, adds local Voices 0 and 1, scales the mix by the master volume parameter `[C000]`, and outputs the stereo stream to IOMD:

```dsp
; DSP4: Sum local voices with DSP5 voices and output to IOMD
ld r0, [0300]      ; Voice 0 Audio
ld r1, [0301]      ; Voice 1 Audio
mov r4, r4
addxy r0, r0, r1   ; Sum Voices 0 + 1
ld r2, [a400]      ; Voice 2 Audio from DSP5
ld r3, [a401]      ; Voice 3 Audio from DSP5
mov r4, r4
addxy r2, r2, r3   ; Sum Voices 2 + 3
addxy r0, r0, r2   ; Total Slave Voice Sum (Voices 0..3)
ld r7, [c000]      ; Slave Output Master Level
mov r4, r4
mul r0, r0, r7     ; Scale by Master Level
st r0, [6020]      ; Wave Bus Left Channel Output
st r0, [6021]      ; Wave Bus Right Channel Output
```

---

## 3.4 IOMD Serial Wave Bus Architecture (Bank 3) `[VERIFIED]`

The **IOMD Serial Wave Bus** is an inter-ASIC audio streaming matrix comprising 256 logical register words (`0x6000`–`0x60FF`) per BMC ASIC. IOMD channels are hardware-latched, serialized across external board connections, and demultiplexed at the destination ASIC synchronously at 96.000 kHz.

### 3.4.1 Register Ownership Boundaries

To prevent bus contention, register ranges within Bank 3 are assigned to dedicated hardware core owners. Writes executed by non-owning DSP cores are discarded by hardware logic:

| Register Range | Size | Core Owner (SYSTEM-8) | Core Owner (SYSTEM-1) | Primary Function |
|---|---|---|---|---|
| `0x6000` – `0x60BF` | 192 Words | `DSP4` / `DSP5` | `DSP A` | External Audio Streams & Primary Mix |
| `0x60C0` – `0x60DF` | 32 Words | `DSP2` / `DSP3` | `DSP B` | Auxiliary Inter-ASIC Control Channels |
| `0x60E0` – `0x60FF` | 32 Words | `DSP0` / `DSP1` | Common Logic | Telemetry & Synchronization Channels |

### 3.4.2 SYSTEM-8 Wave Bus Register Channel Allocation `[MEASURED]`

| Register Address | Direction | Associated Core | Physical Mapping / Function | Measured Behavior |
|---|---|---|---|---|
| `0x6000` – `0x6003` | Input | `DSP4` | Slave 1 Digital Audio Influx | Slave 0 sums these inputs into local mix (`6020`/`6021`). |
| `0x6004` – `0x6005` | Input | `DSP4` | Auxiliary Inter-ASIC Influx | Monitored by Roland firmware; reads digital silence. |
| `0x6020` | Output | `DSP4` | Master Output Left Channel | Verified: Directly captured via USB audio channel 1. |
| `0x6021` | Output | `DSP4` | Master Output Right Channel | Verified: Directly captured via USB audio channel 0. |
| `0x6063` | Output | `DSP4` | Auxiliary Mono Feed | Verified: Audible on both USB channels at -23.9 dB attenuation. |
| `0x60B0` – `0x60B7` | Output | `DSP5` | Mirror Registers | Writes persist locally; does NOT reach physical audio output. |
| `0x60E0` – `0x60E2` | Input | `DSP0` | Inter-ASIC Clock / Sync | Hardware frame alignment synchronization words. |

### 3.4.3 SYSTEM-1 Wave Bus Register Channel Allocation `[MEASURED]`

In the SYSTEM-1 architecture, IOMD interconnects the single Slave BMC directly with the Master BMC:

| Register Address | Direction | Flow Path | Function / Payload |
|---|---|---|---|
| `0x6006` – `0x600D` | Slave $\to$ Master | Slave `DSP B` $\to$ Master `DSP B` | Polyphonic Audio Stream (Left/Right + Aux). |
| `0x6026` – `0x602D` | Slave $\to$ Master | Slave `DSP B` $\to$ Master `DSP A` | Secondary Voice Audio Channels (Used in SH-2/PROMARS). |
| `0x600E` – `0x6021` | Master $\to$ Slave | Master `DSP A` $\to$ Slave `DSP A` | 20 Real-Time Modulation Streams (LFO, Pitch Env, Color Mod). |
| `0x6000` / `0x6001` | Master $\to$ DAC | Master `DSP A` $\to$ DAC | Primary Analog DAC Left/Right Audio Feed. |

```dsp
; SYSTEM-1 Slave B Output Stream Routing (C-S1V4 Model)
ld r7, [c000]      ; Master Volume Level (assigned by OS)
ld r0, [a150]      ; Polyphonic Voice Sum (accumulated by Slave DSP A)
mov r4, r4
mul r1, r0, r7     ; Scale mix by output master volume
st r1, [6006]      ; Output to Master BMC Left Audio Channel
st r1, [6007]      ; Output to Master BMC Right Audio Channel
mov r0, #0.0       ; Clear auxiliary streaming channels
st r0, [6008]
st r0, [6009]
st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d]
```

---

## 3.5 Delay Buffer DMA Controller (Bank 2, DT2) `[MEASURED]`

Bank 2 (`0x4000`–`0x4FFF`) does not represent a static internal SRAM array. Instead, it provides a hardware **windowed memory aperture** backed by an autonomous multi-channel DMA controller streaming to external SDRAM:

- **Circular Ring Buffering**: Configured by the ARM control plane to manage multi-second delay lines and reverb tanks in external SDRAM.
- **Ping-Pong Buffer Paging**: The DMA controller rotates double-buffered memory pages across consecutive 96 kHz audio frames.
- **DSP Tap Instructions (`tapreq`)**: The DSP initiates delay line read/write pointers by issuing specialized delay tap requests (`tapreq`) using hidden registers `h4` and `h5`, followed by standard indexed load instructions.

---

## 3.6 Diagnostic Readback & Slave Telemetry Barriers `[STATIC_ANALYSIS]`

A critical hardware boundary within the multi-BMC architecture is the **asymmetry of diagnostic readback**:

1. **Master DSP Direct Readback**: The Master ARM core can directly inspect its local DSP memory spaces (DT0, Bank 5, Bank 6, Program SRAM) via the physical memory window `0x41400000 + n * 0x100000`. This enables non-intrusive live peeking from host software (`dsppeek.py`).
2. **Slave DSP Read Isolation**: The Master BMC has no physical direct-memory bus bridging into the Slave BMC memory fabric. 
3. **USB-MIDI Slave Diagnostic Loopback**: Disassembly of `s_boot.bin` (`0x01102BD8`, `0x01107D76`) demonstrates that the Slave OS maintains an internal USB-MIDI SysEx packet reassembly queue (`TXMIDI`, `RXMIDI`, `TXDT`, `RXDT`). However, the Master μT-Kernel OS filters incoming generic SysEx messages and does not bridge raw diagnostic packets to the host PC.
4. **Audio Telemetry Demodulation**: To extract telemetry and verify internal registers from the Slave DSPs without hardware logic analyzers, custom firmware engines encode telemetry words into high-frequency audio carriers (e.g., 1000 Hz, 2200 Hz, 4100 Hz fixed tones), which are captured over USB audio at 96 kHz (`arecord -D hw:SYSTEM8,0 -f FLOAT_LE`) and demodulated on the host.
