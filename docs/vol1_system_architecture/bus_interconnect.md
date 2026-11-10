# Volume I — Chapter 3: Inter-DSP Communication and Audio Path (SYSTEM-8)

| | |
|---|---|
| Version | 2.0 (2026-10-22). Measurements behind each fact: [lab log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md) |
| Scope | How the six DSPs of a slave BMC, the ARM and the other BMCs exchange data, and which way the audio of a voice reaches the output. Which DSP does what with each voice: [Vol. III ch. 1](../vol3_voice_allocation_scheduling/system8_voice_allocation.md) |
| Device | SYSTEM-8, OS 1.x, PLUG-OUT in slot 3 (the tests). The SYSTEM-1 has its own document: [system-1/WAVE_BUS.md](../../system-1/WAVE_BUS.md) (summary in §11 below) |
| Base documents | ISA and timing: [Vol. II](../vol2_dsp_core_isa/core_architecture.md); ARM windows: [ch. 2](memory_map.md); program memory: [Vol. III ch. 3](../vol3_voice_allocation_scheduling/slot_program_model.md) |
| Reference code | C-V4 (`examples/from_scratch/v4/`, own microcode, 8 voices measured on the device); engine `examples/from_scratch/v3/engine_v3.c` |

Evidence level of each fact: **[M]** measured on the device · **[L]** read in Roland's
microcode or ARM code (static) · **[C]** our code · **[E]** emulator prediction (`examples/from_scratch/emu`) ·
**[I]** inferred, no direct test · **[A]** open. In parentheses, the lab-log section that supports it
(e.g. *(log §4b)*; "VA log" = the voice-allocation log
[s8_voice_allocation_log.md](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md)).

The `dsp` blocks of this document are microcode in the syntax of `asm/`; all of them assemble
(`python3 tools/doccheck.py docs/vol1_system_architecture/bus_interconnect.md`). The `c` blocks are excerpts of code
in the repository.

---

## 1. Conventions

### 1.1 DSP memory addresses

A memory reference is a 16-bit word `Bnnn`. Bits 15–13 select the bank; the rest, the word inside the bank. In the
text they are written in hexadecimal without prefix and, in program tables, in brackets (`[2207]` = "word `2207`").

| prefix | bank | name | example |
|---|---|---|---|
| `0nnn` | 0 | DT0, data RAM (ring) | `[0300]` |
| `2nnn` | 1 | DT1, main ↔ slot mailbox (ring) | `[2207]` |
| `4nnn` | 2 | DT2, window of the delay line | `[4300]` |
| `6nnn` | 3 | IOMD, inputs and outputs of the BMC | `[6020]` |
| `8nnn` | 4 | MXM, bus between DSPs of the same BMC | `[8404]` |
| `Annn` | 5 | DPC, persistent memory; `A400`–`A7FF` = pair mailbox | `[A021]` |
| `Cnnn` | 6 | parameters written by the ARM (read-only for the DSP) | `[C000]` |
| `Ennn` | 7 | unknown (`E000` = frame counter) | `[E000]` |

### 1.2 Time units and numbering

* **Sample**: 1/96 000 s. Each DSP runs all its programs once per sample (§6). "Sample n+1" is the next run.
* **Slave**: each SYSTEM-8 has one master BMC and two slaves (slave 0: voices 0–3, slave 1: voices 4–7). Each slave
  has six DSPs, `DSP0`–`DSP5`, grouped in **pairs** (0,1), (2,3), (4,5).
* **Local voice** `lv` = global voice − 4 · slave (0–3). In Roland's family A, `u = lv >> 1` selects the chain
  (0: voices 0/1, 1: voices 2/3) and `k = lv & 1` the voice inside the chain (A or B).

### 1.3 Syntax of the microcode examples

```dsp
ld r1, [a021]      ; load: r1 <- word A021 (bank 5)
mov r4, r4         ; filler instruction: the load is seen in the next instruction (latency 1)
st r1, [8404]      ; store r1 to word 8404 (MXM, zone Z)
```

Hardware rules that affect all examples [M] ([Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §7.3): a
load is visible one instruction later; at most 2 loads from bank 0, 1 load and 1 store of bank 5 per instruction; up
to 6 transfers per instruction.

---

## 2. Topology

```
                    ┌──────────── BMC slave 0 (voices 0–3) ────────────┐
 slave ARM ───────► │ bank 5 / bank 6 of each DSP                       │
 (region 1)         │                                                   │
                    │  DSP0 ──MXM Z→M──► DSP2 ──MXM Z→M──► DSP4 ──► IOMD 6020/6021 ──► output
                    │   │ A404… (pair)    │ bank 1              ▲ A400…       ▲
                    │   ▼                 ▼ slots DCO1/DCO2      │ (pair)      │ IOMD 6000–6003
                    │  DSP1 ──MXM Z→O──► DSP3 ──MXM Z→O──► DSP5 ┘             │
                    └──────────────────────────────────────────────────────── │ ─┘
                    ┌──────────── BMC slave 1 (voices 4–7) ──────────┐         │
                    │  the same chain; its output enters slave 0 ────┼─────────┘  [M] exists, words [A]
                    └────────────────────────────────────────────────┘
                    Master BMC: OS, panel, MIDI; DSP4 effects (slots 0xEFDC, 0xF7DC); DSP3 idle
```

The "output" arrow is not characterised at pin level: it is known that `6020`/`6021` of slave 0 are heard and that
nothing else of the slave is heard without going through those two words (§5) [M].

---

## 3. Memory model per bank

| bank | words | scope | between samples | seen by other DSPs | ARM access | evidence |
|---|---|---|---|---|---|---|
| 0 (`0nnn`) | 0x800 | private to the DSP | **ring**: what is stored at R is read at R+1 in the next sample | no | read (IEEE/SD31/RAW views, `0x41400000 + n·0x100000`), relative to the ring | [M] (log §4i; [Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §7.2) |
| 1 (`2nnn`) | 0x800 | private to the DSP; DSP0 and DSP1 have the same base in the ARM table | **ring**, like bank 0 | only DSP0↔DSP1 [I] | not used | ring [M] with C-V4; bases [L] `s_boot 0x0111C580` (VA log §6.3 D/E) |
| 2 (`4nnn`) | window | delay line (DMA) | two alternating pages | no | configuration | [M] on the master; out of scope here ([Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §8.4) |
| 3 (`6nnn`) | 0x100 | **one block per BMC**; each word has an owner DSP | persists | yes (§4.7) | RAW read `0x4113E000` | [M] (log §4b, §4i) |
| 4 (`8nnn`) | 4 × 0x200 (M, O, Z, S) | point-to-point channels between DSPs | **1-sample latch**, no retention | yes, only the wired pair (§4.3) | read | [M] on the master, [M] on the slaves with C-V4 |
| 5 (`A000`–`A3FF`) | 0x400 | private to the DSP | persists | no | **write** with service `+0xB0` (§4.5); RAW read `0x41114000 + n·0x4000 + 0x2000` | [M] |
| 5 (`A400`–`A7FF`) | 0x400 | **shared by the pair** | one DSP writes, the other reads; latch [I] | yes, the pair (§4.4) | — | [L]; DSP5→DSP4 [M] with C-V4 |
| 6 (`Cnnn`) | per DSP | parameters, read-only for the DSP | persists; hardware ramps | no | write `+0xA4` immediate / `+0xA0` with ramp (§4.6) | [M] |

Design consequence: **there is no memory common to the six DSPs**. What one DSP computes reaches another only by MXM
(fixed chain), by the pair mailbox or by IOMD. Own bank 5 and bank 6 are the ARM's path into each DSP.

---

## 4. Transport mechanisms

### 4.1 Bank 0 ring: state of a program between samples [M]

**Semantics.** Reference R addresses physical word `(R + p) mod 0x800`, and `p` advances by 1 each sample. Inside a
sample, what is stored at R is read back at R; in the next sample it appears at R+1; after k samples, at R+k.

**Pattern.** To keep a value from one sample to the next: load from `S+1`, store to `S`. Example: the phase of the
C-S1V4 saw (voice 0, `examples/system_1/s1m0.s`; same ISA):

```dsp
ld r1, [0301]      ; phase of the previous sample (stored at 0300)
mov r4, r4
addxy r1, r1, r3   ; advance the phase (r3 = increment)
st r1, [0300]      ; store: in the next sample it will be at 0301
```

**Rules.** Two programs of the same DSP share the ring: their ranges must not overlap, counting that a datum moves
one word per sample (if a program reads `R+k`, reserve k words of margin). A delay of k samples is `st [R]` +
`ld [R+k]`. The ARM cannot leave a fixed table in bank 0 (it moves too).

### 4.2 Bank 1: main ↔ slot mailbox (ring) [M]

The slots (DCO programs at `0x6000`/`0xB000` of DSP2/3, effects at `0xB000` of DSP4) talk to the main program of the
same DSP through bank 1. Bank 1 is also a ring, and **the main program runs before the slots** inside the sample (VA
log §6.3 D, F). Hence the timing:

| step | who | word | when |
|---|---|---|---|
| 1 | main writes the slot input | `[2207]` | sample n |
| 2 | the slot reads the input | `[2207]` (same word) | sample n |
| 3 | the slot writes its output | `[2221]` | sample n |
| 4 | main reads the output | **`[2222]`** (= `2221` + 1) | sample n+1 |

Example: main program of DSP2 of C-V4 (`examples/from_scratch/v4/v4m2.s`), measured on the device:

```dsp
; v4m2 (DSP2, main): DCO2 pitch from MXM; saw outputs -> MXM
ld r1, [8004]      ; pitch of voice A, from DSP0 (MXM, §4.3)
mov r4, r4
st r1, [2207]      ; pitch input of the DCO2 slot, voice A (step 1)
ld r1, [8005]      ; pitch of voice B
mov r4, r4
st r1, [2214]      ; pitch input of DCO2, voice B
ld r1, [2222]      ; output of voice A that the slot wrote to 2221 in the previous sample (step 4)
mov r4, r4
st r1, [8404]      ; to DSP4 (MXM)
ld r1, [222a]      ; output of voice B (written at 2229)
mov r4, r4
st r1, [8405]
```

Full interface of Roland's DCO slots: [Vol. III ch. 1](../vol3_voice_allocation_scheduling/system8_voice_allocation.md) §5.

Typical mistake (made in C-V3, VA log §6.3 A/D): reading the output at `[2221]`. With the ring it returns another
word from several samples ago; in C-V3 it coincided with another saw and hid the bug.

### 4.3 MXM (bank 4): channels between DSPs of the same BMC [M]

**Zones** (local addresses of each DSP; `8405` in DSP0 and `8405` in DSP2 are different words):

| zone | addresses | use |
|---|---|---|
| M | `8000`–`81FF` | input from the **even** DSP of the previous pair |
| O | `8200`–`83FF` | input from the **odd** DSP of the previous pair |
| Z | `8400`–`85FF` | output to the next pair |
| S | `8600`–`87FF` | return input: what the DSP two positions ahead writes to its M/O |

**Wiring of the slaves** (the word number `nn` is preserved):

| writes | reads | evidence |
|---|---|---|
| DSP0 `Z 84nn` | DSP2 `M 80nn` | [M] C-V4 |
| DSP1 `Z 84nn` | DSP3 `O 82nn` | [M] C-V4 |
| DSP2 `Z 84nn` | DSP4 `M 80nn` | [M] C-V4 |
| DSP3 `Z 84nn` | DSP5 `O 82nn` | [M] C-V4 |
| DSP2 `M 80nn` (store) | DSP0 `S 86nn` | [L] |
| DSP4 `M 80nn` (store) | DSP2 `S 86nn` | [L] |

**Timing and rules** [M on the master, DSP3↔DSP4]:
* **1-sample latch**: during its whole program of sample n, the reader sees what was written in sample n−1, whatever
  the order of the DSPs.
* **No summing**: two writes in the same sample → the last one stays.
* **No retention**: if nobody writes the word in a sample, the reader gets 0. It must be written every sample.

Example: pitch of voices 0/1 from DSP0 to DSP2 (C-V4, `v4m0.s`; the `v4m2.s` of §4.2 receives it with `ld [8004]`):

```dsp
; v4m0 (DSP0, main): pitch of voices 0 and 1 -> DSP2
ld r1, [a021]      ; pitch of voice A, written by the ARM (§4.5)
mov r4, r4
st r1, [8404]      ; Z 404 -> DSP2 M 004, one sample later
ld r1, [a033]      ; pitch of voice B
mov r4, r4
st r1, [8405]      ; Z 405 -> DSP2 M 005
```

Typical mistake (C-V3, VA log §6.3 A/G): assuming an offset (`Z 404` → `M 005`). DSP4 read `M 006`, which nobody
wrote (0), and voice 1 was silent.

### 4.4 Pair mailbox (bank 5, `A400`–`A7FF`) [L, M]

**Semantics.** Words `A400`–`A7FF` are shared by the two DSPs of a pair: what one writes the other reads. In no
Roland program does a DSP read what it writes itself in that zone [L]. The emulator models a 1-sample latch, like
MXM [I]; the real latency is not measured [A].

**Use in Roland's family A** [L]:

| writes | words | reads | what |
|---|---|---|---|
| DSP0 (MAIN0) | `A404`–`A406`, `A409`–`A40E` | DSP1 (MAIN1) reads `A404`, `A409`–`A40E` | modulation data common to the two chains |
| DSP4 (MAIN4) | `A404`–`A407` | DSP5 (MAIN5) | data from DSP4 to DSP5 |
| DSP5 (MAIN5) | `A400`–`A403` | DSP4 (MAIN4) | **voices 2 and 3 processed** → output mix |

Example: DSP5 hands its two voices to DSP4 (C-V4, `v4m5.s`), [M] with C-V4 (voices 2, 3, 6 and 7 audible):

```dsp
; v4m5 (DSP5, main): voices 2 and 3 with gate -> pair mailbox
ld r4, [a029]      ; gate of voice 2 (own bank 5 of DSP5, written by the ARM)
ld r1, [8200]      ; saw of voice 2 (from DSP3 via MXM, zone O)
mov r4, r4
mul r1, r1, r4
st r1, [a400]      ; to DSP4
ld r4, [a038]      ; gate of voice 3
ld r2, [8201]
mov r4, r4
mul r2, r2, r4
st r2, [a401]      ; to DSP4
```

Typical mistake (C-V3, VA log §6.3 B): sending the voices of DSP5 to IOMD `60B0`, which does not reach the output
(§5).

### 4.5 Own bank 5: per-voice parameters from the ARM (`+0xB0`) [M]

The slave ARM writes a bank-5 word of a DSP with the OS service at `0x0113FC00 + 0xB0`:

```c
/* examples/from_scratch/v3/engine_v3.c */
#define OS(off) (*(void *volatile *)(0x0113FC00u + (off)))
typedef void (*bank_fn)(int dsp, const uint32_t *id, float v);          /* +0xB0 */
static void p_bank(int dsp, uint32_t id, float v) { ((bank_fn)OS(0xB0))(dsp, &id, v); }

/* local voice lv (0..3): bank 5 of DSP lv>>1, id 0x21 + 0x12*(lv&1) */
static void voice_pitch(int lv, int note)
{
    p_bank(lv >> 1, 0x21 + 0x12 * (lv & 1), (float)(note - 12) / 12.0f);
}
```

* Arguments: `dsp` 0–5 of the slave that runs the code; `id` by pointer (word index, `A000 + id`); `float` value
  (the OS converts it to the DSP's RAW format).
* The DSP reads the word with `ld rX, [A0id]`; it persists until the next write. There is no sample latch: the DSP
  sees the new value in the sample in which the ARM wrote it or in the next one [I].
* Full list of per-voice ids: [Vol. III ch. 1](../vol3_voice_allocation_scheduling/system8_voice_allocation.md) §4.

### 4.6 Bank 6: DSP parameters (`+0xA4`, `+0xA0`) [M]

```c
/* examples/from_scratch/engine/engine.c */
typedef void (*set_fn)(int dsp, int id, float v);                                /* +0xA4: immediate */
typedef void (*ramp_fn)(int dsp, int id, int ramp, int flag, int sp, float v);   /* +0xA0: with ramp */
static void p_set(int dsp, int id, float v) { ((set_fn)OS(0xA4))(dsp, id, v); }
static void p_ramp(int dsp, int id, int ramp, int flag, int sp, float v)
{
    ((ramp_fn)OS(0xA0))(dsp, id, ramp, flag, sp, v);
}
```

The DSP reads `C0id` (read-only). With `+0xA0` the hardware takes the value to the target in a ramp (`ramp` = encoded
duration; `flag` = wait for the end); it is used for panel levels and times. Example of use: output level of MAIN4,
`C000`, which multiplies the mix before `6020` [L].

### 4.7 IOMD (bank 3): inputs and outputs of the BMC [M]

**Semantics.** 256 words `6000`–`60FF` per BMC. It is not shared memory: each word has **one owner DSP** (configured
by the ARM); writes by another DSP do not appear; what the owner writes persists and the ARM sees it [M] (log §4i).
Ownership blocks: `00`–`BF` DSP4/5, `C0`–`DF` DSP2/3, `E0`–`FF` DSP0/1 [M] (addresses).

**Words used by Roland's voice programs** and their measured effect (sweep of log §4b: a SAW of a different
frequency in each word, USB capture):

| words | written / read (Roland) | reaches the output | evidence |
|---|---|---|---|
| `6020` | written by DSP4 | **yes**, channel 1 of the capture | [M] |
| `6021` | written by DSP4 | **yes**, channel 0 | [M] |
| `6063` | written by DSP4 | **yes**, both channels (−23.9 dB); why, [A] | [M] |
| `6024` | written by DSP4 | doubtful: +1.4 dB over the expected harmonic | [M], [A] |
| `6022`, `6023`, `6060`–`6062`, `6064`–`6066` | written by DSP4 | no | [M] |
| `60B0`–`60B7` | written by DSP5 (copies of `[2204]`–`[221D]`) | no | [M] |
| `6000`–`6006` | read by DSP4 | — (inputs: slave 1 arrives here, §5.2) | [M] |
| `6010`–`6017` | read by DSP5 | — (inputs; origin [A]) | [M] |
| `60E0`–`60E2` | read by DSP0 | — (inputs; origin [A]) | [M] |

In the USB capture (`arecord -D hw:SYSTEM8,0 -f FLOAT_LE -r 96000 -c 2`), channel 1 is what is heard on the left
output in these tests [M] (confirm in each session).

### 4.8 Latency summary

| leg | latency | evidence |
|---|---|---|
| ARM → bank 5 / bank 6 → DSP | 0–1 samples (depending on when the ARM writes) | [I] |
| main → slot (bank 1, same word) | 0 | [M] (C-V4, C-SAW) |
| slot → main (bank 1, word + 1) | 1 | [M] |
| DSP → DSP (MXM) | exactly 1 | [M] |
| DSP → pair DSP (`A4xx`) | 1 in the emulator | [I] |
| DSP → IOMD → DSP of the same BMC | 0 or 1 depending on the order of the DSPs | [I] |
| slave 1 → slave 0 (IOMD) | ≥ 1 | [I] |

---

## 5. Audio path

### 5.1 Slave 0

The only thing heard from a slave is what **DSP4 of slave 0** writes to `6020`/`6021` [M]. Roland's DSP4 mixes
eight sources there [L] (VA log §6.2):

| source | reaches DSP4 through | stored at | read in the mix |
|---|---|---|---|
| voices 0 and 1 (its own filter/VCA) | — | `[221E]`, `[2229]` | `[2228]` … (ring) |
| voices 2 and 3 (from DSP5) | mailbox `A400`, `A401` | `[2234]`, `[223D]` | |
| slave 1 | IOMD `6000`–`6003` | `[2246]`, `[2249]`, `[224C]`, `[224D]` | |

The eight loads are summed with coefficients `C044`–`C048` and the mix is multiplied by `C04B` (level) and `C000`
before `6020`/`6021`. The minimal version of this mix is the one of C-V4 (`v4m4.s`), measured with the eight voices:

```dsp
; v4m4 (DSP4, main): voices 0, 1 with gate + voices 2, 3 (A400/A401) + slave 1 (6000-6003) -> 6020/6021
ld r4, [a02a]      ; gate of voice 0
ld r1, [8004]      ; saw of voice 0 (DSP2 Z 404)
mov r4, r4
mul r1, r1, r4
st r1, [6060]      ; single voice (Roland also writes them to 6060-6063)
ld r4, [a039]      ; gate of voice 1
ld r2, [8005]
mov r4, r4
mul r2, r2, r4
st r2, [6061]
addxy r1, r1, r2
ld r2, [a400]      ; voice 2, already with its gate (DSP5, §4.4)
mov r4, r4
st r2, [6062]
addxy r1, r1, r2
ld r2, [a401]      ; voice 3
mov r4, r4
st r2, [6063]
addxy r1, r1, r2
ld r2, [6000]      ; slave 1
mov r4, r4
addxy r1, r1, r2
ld r2, [6001]
mov r4, r4
addxy r1, r1, r2
ld r2, [6002]
mov r4, r4
addxy r1, r1, r2
ld r2, [6003]
mov r4, r4
addxy r1, r1, r2
st r1, [6020]      ; output (channel 1 of the capture)
st r1, [6021]      ; output (channel 0)
```

### 5.2 Slave 1

Slave 1 has **no** audible output of its own: no word `6000`–`60FF` that it writes is heard on its own [M] (log §4e,
§4q: a drone on all the words of slave 1 was not heard). It is heard when DSP4 of slave 0 sums its inputs
`6000`–`6003` [M] (C-V4: voices 4–7 audible). **Which word of slave 1 reaches each input is not known** [A]; C-V4
writes its voices to `6060`–`6063` and its mix to `6020`/`6021`, and that works. Fixing it needs one build that only
writes `6060`–`6063` and another that only writes `6020`/`6021`.

### 5.3 Master

The master receives the mix of slave 0 and applies its effects (DSP4, slots `0xEFDC`, `0xF7DC`) [L]; its DSP3 is
idle [M]. How the word reaches the master (wave bus, pins) is not characterised [A] (log §1,
[Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §11).

---

## 6. Execution order inside the sample

| relation | order | evidence |
|---|---|---|
| main and slots of one DSP | main first | [M] (C-SAW in tune, C-V4; VA log §6.3 F) |
| DCO1 and DCO2 of one DSP | DCO1 before DCO2 | [I] (VA log §6.3 F) |
| DSP5 slot and DSP4 main | the DSP5 slot writes later | [M] by survival at `6021`/`6063` (log §4c) |
| DSP5 main and its slot | unknown | [A] |
| between DSPs in general | set by the BMC scheduler; MXM and the mailbox do not depend on it (latch) | [I] |

All programs of a DSP share `r0`–`r7`, the flags and the hidden registers: what one leaves the next one sees, also
across samples [M] ([Vol. III ch. 3](../vol3_voice_allocation_scheduling/slot_program_model.md) §1).

---

## 7. Complete example: one note of voice 0 in C-V4

Path of a note from the ARM of slave 0 to `6020`, with the sample in which each step happens (t = sample in which the
ARM writes the pitch):

| t | where | what | mechanism |
|---|---|---|---|
| t | ARM, `voice_pitch(0, note)` | `p_bank(0, 0x21, (note−12)/12)` | §4.5 |
| t | DSP0, `v4m0` | `ld [a021]` → `st [8404]` | §4.3 |
| t+1 | DSP2, `v4m2` | `ld [8004]` → `st [2207]` | MXM latch |
| t+1 | DSP2, DCO2 slot (saw) | reads `[2207]`, writes `[2221]` | §4.2 |
| t+2 | DSP2, `v4m2` | `ld [2222]` → `st [8404]` | bank-1 ring |
| t+3 | DSP4, `v4m4` | `ld [8004]` × gate `[a02a]` → `st [6020]` | MXM latch |

Total delay: 3 samples (31 µs). The gate (`p_bank(4, 0x2a, 1.0)`) reaches DSP4 directly: pitch and gate do not
travel together, so a pitch written after the gate is heard with up to 3 samples of the previous note.
`engine_v3.c` rewrites the pitch when it opens the gate for that reason.

---

## 8. Design rules

1. Respect the fixed chain: DSP0/1 → DSP2/3 → DSP4/5 → `6020`/`6021` of slave 0. MXM only goes forward (Z) and two
   positions back (S).
2. Write each MXM word in **every** sample (no retention) and read the same word number that was written.
3. Read the outputs of a slot at word **+1**, one sample later (bank-1 ring).
4. Pass the voices of DSP5 to DSP4 through the mailbox `A400`–`A403`; not through IOMD `60B0`.
5. DSP4 of slave 0 must sum `6000`–`6003` if slave 1 has to be heard.
6. **The DSP4 main program cannot be empty**: with it empty the whole device is silent [M] (log §4l). The other five
   main programs can be empty without breaking anything [M].
7. Reserve margins in the rings if two programs share a DSP (§4.1).
8. Check every change in the emulator (`examples/from_scratch/emu/test_chain.py`), with tests that can fail.

---

## 9. Open

| # | what | how to decide it |
|---|---|---|
| 1 | which words of slave 1 enter through `6000`–`6003` of slave 0 | two builds: one only with `6060`–`6063`, another only with `6020`/`6021` |
| 2 | why `6063` is heard on both channels and `6024` is doubtful | batch with `6024` at another frequency; map with notes |
| 3 | real latency of the pair mailbox | dsppeek on the master with the two DSPs of a pair |
| 4 | origin of the inputs `6010`–`6017` (DSP5) and `60E0`–`60E2` (DSP0) | read IOMD with the ARM during a note |
| 5 | whether DSP0 and DSP1 really share bank 1 | different values at `[2210]` of each, read by the ARM |
| 6 | order of MAIN5 and its slot | data path from an input of MAIN5 to an audible word |

---

## 10. Evidence and old references

| fact | lab-log section |
|---|---|
| sweep of the 38 IOMD words | [s8_wave_bus_log.md](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md) §4b |
| order DSP5 slot / MAIN4 | log §4c |
| slave 1 without its own output; test C-S1W | log §4e, §4q |
| mechanisms | log §4i |
| empty DSP4 main = silent; variants | log §4l |
| amplitude envelope of the JX-3P (ENV2, `[029D]`, VCA factors) | log §4j–§4p |
| tuning (fine tune of OSC2) | log §4h |
| bank-1 ring, mailbox, MXM without offset, slave 1 through `6000`–`6003` | [s8_voice_allocation_log.md](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6.3, §6.7 |

References "[Vol. I ch. 3](bus_interconnect.md) §n" written before 2026-10-22 point to those sections of the lab log.

---

## 11. SYSTEM-1 counterpart (summary)

The SYSTEM-1 has the same ISA but another topology; its full specification is
[system-1/WAVE_BUS.md](../../system-1/WAVE_BUS.md) (Spanish, v2.0). The differences that change the design, from
its §1:

| | SYSTEM-8 | SYSTEM-1 |
|---|---|---|
| BMC | master + 2 slaves | master + **1** slave |
| DSPs per BMC | 6 in a chain (MXM) | **2 (A and B)**, no MXM in any program read [L] |
| communication between the DSPs of one chip | MXM + pair mailbox `A400`–`A7FF` | **shared bank 5** of A and B (`A000`–`A1FF`) [M] |
| between chips | IOMD `6000`–`6003` (slave 1 → slave 0) | IOMD: slave → master `6006`–`600D` and `6026`–`602D`; master → slave `600E`–`6021` [L], [M] in part |
| output | `6020`/`6021` of slave 0 | master: `A118`/`A119` → OS program on master A → `6000`/`6001` [M] |
| ARM | with FPU | **without FPU** ([system-1/ARCHITECTURE.md](../../system-1/ARCHITECTURE.md) §1.1) |

PLUG-OUT-mode audio path [M]: slave A → B (bank 5) → IOMD `6006`–`600D` → master B (PLUG-OUT program at `0x1000`)
→ `A118`/`A119` → master A (OS program at `0x82`) → `6000`/`6001` → output. A PLUG-OUT that does not load its
master-B program is silent [M] (C-S1TONE).
