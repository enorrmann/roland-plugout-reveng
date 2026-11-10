# Volume I — Chapter 3: Inter-DSP Communication and Audio Path (SYSTEM-8 and SYSTEM-1)

> **Scope:** Part A (§1–§10) SYSTEM-8; Part B (§11–§19) SYSTEM-1. Common to both: the address notation and the
> microcode syntax (§1), and the ring semantics of banks 0 and 1 (§4.1, §4.2).

## Part A — SYSTEM-8

| | |
|---|---|
| Version | 2.0 (2026-10-22). Measurements behind each fact: [lab log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md) |
| Scope | How the six DSPs of a slave BMC, the ARM and the other BMCs exchange data, and which way the audio of a voice reaches the output. Which DSP does what with each voice: [Vol. III ch. 1](../vol3_voice_allocation_scheduling/system8_voice_allocation.md) |
| Device | SYSTEM-8, OS 1.x, PLUG-OUT in slot 3 (the tests). The SYSTEM-1: Part B (§11–§19) |
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

## 10. Evidence

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


---

## Part B — SYSTEM-1

| | |
|---|---|
| Version | 2.0 (2026-10-22); effects chain 2026-10-23. Measurements behind each fact: [S1 wave-bus lab log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s1_wave_bus_log.md) |
| Scope | How the four DSPs of the SYSTEM-1 (A and B of the master, A and B of the slave) and the ARM exchange data, and which way the audio of a PLUG-OUT reaches the output. Which DSP does what with each voice: [Vol. III ch. 2](../vol3_voice_allocation_scheduling/system1_voice_allocation.md) |
| Device | SYSTEM-1, OS 1.30, PLUG-OUT in slot 1. Same ISA as the SYSTEM-8, other topology |
| Base documents | [ch. 1](soc_overview.md) §6, [Vol. III ch. 3](../vol3_voice_allocation_scheduling/slot_program_model.md), [examples/system_1/README.md](../../examples/system_1/README.md) (PLUG-OUT interface) |
| Reference code | C-S1V4 (`examples/system_1/`, measured on the device); C-S1SY (`examples/system_1_synth/`, effects chain); C-S1JX (`examples/system_1_jx/`, not installed) |

Evidence: **[M]** measured on the device · **[L]** read in OS 1.30 or in Roland's SYSTEM-1 PLUG-OUTs (SH-2, SH-101,
PROMARS) · **[C]** our code · **[E]** emulator · **[I]** inferred · **[A]** open. Syntax and address notation: §1.

## 11. Differences with the SYSTEM-8 that change the design

| | SYSTEM-8 | SYSTEM-1 |
|---|---|---|
| BMC | master + 2 slaves | master + **1** slave |
| DSPs per BMC | 6 in a chain (MXM) | **2 (A and B)**, no MXM in any program read [L] |
| communication between the DSPs of one chip | MXM + pair mailbox `A400`–`A7FF` | **bank 5 shared** by A and B (`A000`–`A1FF`) [M] |
| between chips | IOMD `6000`–`6003` (slave 1 → slave 0) | IOMD: slave → master `6006`–`600D` and `6026`–`602D`; master → slave `600E`–`6021` [L], [M] in part |
| output | `6020`/`6021` of slave 0 | master: `A118`/`A119` → OS program on master A → `6000`/`6001` [M] |
| ARM | with FPU | **without FPU** ([ch. 1](soc_overview.md) §6.3) |

## 12. Topology

```
 SLAVE (voices)                                           MASTER
 ┌─────────────────────────────┐                          ┌──────────────────────────────────────────────┐
 │ DSP A ◄──shared bank 5──► DSP B │──IOMD 6006–600D──────►  │ DSP B @0x1000 (PLUG-OUT) ──A118/A119──► DSP A │──► 6000/6001 ──► output
 │   ▲                          │──IOMD 6026–602D──────►  │                     shared bank 5        @0x82 (OS) │
 │   └───────────IOMD 600E–6021 ◄──────────────────────────│ (per-voice modulation, stock synth)           │
 └─────────────────────────────┘                          └──────────────────────────────────────────────┘
   slave ARM: region 1 (engine)                             master ARM: OS + region 0 (model, assigner)
```

## 13. Memory model

| bank | scope | between samples | ARM access | evidence |
|---|---|---|---|---|
| 0 (`0nnn`) | per DSP or per chip [A]; own designs use different ranges in A and B | ring (write R, read R+1) | — | ring [M] with C-S1V4 |
| 1 (`2nnn`) | shared by A and B of the slave [I] (the stock main A writes `2004`…, B `2000`…, without overlapping); the banks 1 of the master's A and B are distinct [I] (§16) | ring | — | [L], [I] |
| 3 (`6nnn`) IOMD | per chip; link words between chips | persists | — | [M] in part (§14.4) |
| 5 (`A000`–`A1FF`) | one per chip; `A1xx` shared by A and B; **`A0xx` written by the ARM (`FD6C` sel 0) is seen by DSP A but not by DSP B** | persists | `FD6C(sel, obj, bits)` (§14.2) | [M] (`A150`, `A151+v`… from A to B; B reading `A038+v` from the ARM: silent, C-S1SY step 3) |
| 6 (`Cnnn`) | per DSP | persists | `FCEC(sel, id, bits)` immediate, `FCE4` with ramp | [M] (`C01C` of master B; `C040`–`C04B` of slave B and `C044`–`C055` of slave A, C-S1SY) |

Bank-5 words reserved by the system [L]: `A004`, `A005`, `A01D` (frame counter of the OS header and of the closings,
§15.3); `A006`–`A009` (the stock slave ARM); `A118`–`A11D` on the master (output, §15.1). Which ranges the OS and the
official PLUG-OUTs use in each bank and DSP: [Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §7.

## 14. Mechanisms

### 14.1 Shared bank 5: from one DSP to the other of the same chip [M]

What DSP A writes in bank 5 is read by DSP B, and the other way round. It replaces the SYSTEM-8's MXM and pair
mailbox. The latency (0 or 1 sample, depending on the order of A and B in the sample) is not measured [A]; no tested
design depends on it.

Example, C-S1V4: DSP A sums the voices in `A150`; DSP B (`examples/system_1/s1m1.s`) takes it to the master:

```dsp
; s1m1 (slave B, main @0x0100): A150 x C000 -> 6006 / 6007; the other outputs to 0
ld r7, [c000]      ; output level of DSP B (set by the OS)
ld r0, [a150]      ; mix written by DSP A
mov r4, r4
mul r1, r0, r7
st r1, [6006]      ; to the master, left
st r1, [6007]      ; to the master, right
mov r0, #0.0
st r0, [6008]
st r0, [6009]
st r0, [600a]
st r0, [600b]
st r0, [600c]
st r0, [600d]
```

### 14.2 Bank 5 from the ARM: `FD6C` [M], [L]

The same code on the master and on the slave (OS 1.30: master `0x010094B6`, slave `0x01009F4E`) [L]:

```
FD6C(sel, obj, bits):   idx = obj[2]
    sel 0: *(0x40101000 + 4·idx)         = bits     -> word A000 + idx
    sel 1: *(0x40101400 + 4·idx)         = bits     -> word A100 + idx
    other sel: does nothing
```

`bits` is the IEEE-754 pattern of a `float` in an integer register (no FPU). Use in C-S1V4:

```c
/* examples/system_1/engine.c */
typedef void (*bank_fn)(int sel, const uint32_t *obj, uint32_t value);
static void b5(int idx, uint32_t bits)                        /* word A000 + idx */
{
    uint32_t obj[3] = { 0, 0, (uint32_t)idx };
    ((bank_fn)SVC(0x16C))(0, obj, bits);
}
```

With the two `sel` values the ARM reaches `A000`–`A1FF`: that is the bank 5 a PLUG-OUT can use.

### 14.3 Bank 6 from the ARM: `FCEC` [M]

`FCEC(sel, id, bits)`: parameter `C0id` of DSP A (`sel 0`) or B (`sel 1`) of the calling chip, immediate value.
Measured examples: VOLUME → `C01C` of master B; on the slave, the ADSR and the filter of C-S1SY → `C040`–`C04B` of
DSP B and the ranges/waves → `C044`–`C055` of DSP A. It is Roland's path for the controls (the SH-2 reads its
controls at `C0xx`) and the only reliable one to the slave's DSP B: `A0xx` written with `FD6C sel 0` does not reach it
(§13).

```c
/* examples/system_1/model.c */
typedef void (*env_fn)(int sel, int id, uint32_t value);
static void set_volume(int v)
{
    volume = v < 0 ? 0 : v > 255 ? 255 : v;
    ((env_fn)SVC(0xEC))(1, M_VOLUME, volume_bits[volume]);     /* M_VOLUME = 0x1C */
}
```

`FCE4(sel, id, target, y, z)` (with ramp): linear hardware ramp, one step every 1/3 ms, from 4 to 97 ms from 0 to 1
according to `y`; `+8` of the block = current value, `+C` = increment [M]
([examples/system_1_ramp/README.md](../../examples/system_1_ramp/README.md)). Parameter block layout written by the
OS's `set_param` (`0x01008D26`: `sel` 0 → `0x4014C000`, `sel` 1 → `0x4016C000`, `+ 16·id`): `+0` mode
(`0xE0000000` immediate, `0xC0000000` ramp), `+4` value, `+8` current value or target, `+0xC` ramp step [M].

### 14.4 IOMD between chips

| words | direction | who (Roland) | evidence |
|---|---|---|---|
| `6006`–`600D` | slave → master | slave A or B write; master A or B read (SH-101: slave A `600B`–`600D`, slave B `6006`–`600A`) | [L]; `6006`/`6007` [M] with C-S1V4 |
| `6026`–`602D` | slave → master | slave B writes (SH-2: `6026`–`602D`; PROMARS: `6026`, `6027`); master A reads | [L] (`system-1/s1_progs.py`) |
| `600E`–`6021` | master → slave (20 words) | master A (modulation) and B (PROMARS: `6010`) write; slave A and B read | [L]; `600E`–`6015` master B → slave A [M] (WT-M2S, only with an own main program on master A: §15.1) |
| `6000`/`6001` | master A → output | the OS's output program | [M] |

Latency between chips: ≥ 1 sample [I]. In the stock synth the master's mixer sums `6006`–`600D` in L/R pairs with
gains `C005`–`C00E` [L] (§15.2); in PLUG-OUT mode the allocation is free.

### 14.5 Rings (banks 0 and 1)

The same semantics as on the SYSTEM-8 (§4.1, §4.2). Measured example, phase of a saw of C-S1V4 (`s1m0.s`, voice 0):
`ld r1, [0301]` … `st r1, [0300]`.

## 15. Audio path

### 15.1 PLUG-OUT mode [M], [L]

```
slave A/B ──IOMD 6006…──► master B (PLUG-OUT program @0x1000) ──A118/A119──► master A @0x82 (OS):
    A118/A119 → DC filter (C002, C003) → × C004 → 6000 / 6001 → output
```

* When switching to PLUG-OUT mode the OS queues on master A the output program at `0x82` and on master B, at
  `0x1000`, a program that sets `A118`–`A11D` to 0, together with the effect program at `0x0` (§16). Then it calls
  `load` of the region-0 model [L].
* The output program at `0x82` is 126 B long and ends at `0x100`: master A goes on with its main program `0x0100`,
  which the OS does **not** replace. If the PLUG-OUT does not load its own, what the previous mode left keeps running,
  and it writes the master → slave words `600E`–`6021` after our master B: the slave reads 0 (or a signal) instead of
  ours [M] ([Vol. VI ch. 7](../vol6_reference_designs_applications/mutable_memory.md) §4.3). Roland's PLUG-OUTs load
  their `0x0100` (`FC30 {0x100, 0x6DF2, 0x7FF2}`); SYSTEM-S1CC loads an empty one (`MA_MAIN`, 2026-11-05).
* **A PLUG-OUT that does not load its master-B program is silent** [M] (C-S1TONE). The panel VOLUME does not go
  through the OS program either: the PLUG-OUT applies it (`C01C`) [M].

Minimal master-B program (C-S1V4, `examples/system_1/s1mm1.s`), measured; it skips the panel effects (for the
version that passes through them see §16.4):

```dsp
; s1mm1 (master B @0x1000): 6006 / 6007 of the slave x VOLUME (C01C) -> A118 / A119
ld r0, [6006]
ld r1, [6007]
ld r7, [c01c]      ; VOLUME, written by the master ARM with FCEC(1, 0x1C, ...)
mov r4, r4
mul r0, r0, r7
mul r1, r1, r7
st r0, [a118]      ; read by the OS's output program on master A
st r1, [a119]
mov r2, #0.0
st r2, [a11a]
st r2, [a11b]
st r2, [a11c]
st r2, [a11d]
```

### 15.2 Stock synth [L]

| DSP | program | what it does |
|---|---|---|
| master | `0x60097A7C` (696 instr.) | per-voice modulation: reads `A038`–`A048` from the ARM, writes 20 words `600E`–`6021` to the slave and 20 more through bank 5 (`A100`–`A113`) |
| master | `0x60095B0C` (58 instr.; variants `0x600959DC`, `0x60095A24`) | mixer: `6006`–`600D` × `C005`–`C00E`, DC filter, × `C00F` × `C010` → `6000`/`6001` |
| slave A, main | `0x600185B0` (241 instr.) | reads `600E`–`6021` and `A006`–`A009`; feeds the voice sections (bank 1); passes data and audio to B through `A100`–`A10D` |
| slave B, main | `0x60018F44` (237 instr.) | reads `A100`–`A10D`; collects its sections; writes `6006`–`600D` × `C000` |

On which master DSP each of the two master programs runs: [A].

### 15.3 Header `0x0000` and closing `0x7FF2`: what is loaded in `load` [M], [L]

Each DSP starts its sample in a **header** at `0x0000` and ends it with a **closing** at `0x7FF2`; both keep a frame
counter in bank 5 (`A01D` master A, `A004` master B and slave A, `A005` slave B). The header is set by the OS.

**Rule: a PLUG-OUT never queues the entry `0x0000` in `load`.** Doing it hangs the device when the PLUG-OUT is
selected (fatal OS error, all panel lights on, only switching off gets out) [M]: the header is code the DSP is
running. Roland's PLUG-OUTs have the entry `0x0000` in their program tables (that is why it shows up when reading the
images), but their engines pass to `FC30`/`FC34` a pointer to the **next entry** [L]. Real `load` sequence, observed
by running Roland's code in Unicorn (`examples/system_1_jx/test_arm.py`, `Region` harness with on-demand mapping):

| PLUG-OUT | region | calls in `load` (addresses of each queued set) |
|---|---|---|
| SH-101 | 1 (slave) | `FC44(0…7, 0)`; `FC30 {0x100, 0x7FF2}`; `FC34 {0x100, 0x7FF2}` |
| SH-2 | 1 | `FC44(0, 1)`, `FC44(4, 1)`, the rest 0; `FC30 {0x100, 0x7FF2, 0x3100}`, `FC30 {0x1900, 0x100, 0x7FF2, 0x3100}`, `FC30 {0x3100}`; `FC34 {0x100, 0x7FF2}` |
| PROMARS | 1 | `FC44(0, 1)`, `FC44(2, 1)`, the rest 0; `FC30 {0x100, 0x7FF2}`, `FC30 {0x1900, 0x100, 0x7FF2}`; `FC34 {0x100, 0x1900, 0x7FF2}`, `FC34 {0x1900, 0x7FF2}` |
| SH-101, SH-2, PROMARS | 0 (master) | `FC30 {0x100, 0x6DF2, 0x7FF2}`, `FC30 {0x6DF2, 0x7FF2}`; `FC34 {0x1000, 0x7FF2}`, `FC34 {0x5FF2, 0x6FF2, 0x1000, 0x7FF2}`, `FC34 {0x6FF2, 0x1000, 0x7FF2}`; two `FC38` and three `FC3C` (data queues: pointer to region 0, `0x110001D` / `0x110001F`, a third value) |

None queues `0x0000`. The several `FC30`/`FC34` of one `load` are overlapping sets (suffixes of one table); what the
data queues `FC38`/`FC3C` do is not studied [A] (our PLUG-OUTs that do not use them work).

For reference, Roland's header (the same in all three; our assembled version, `s1jx.frame_header`, is byte-identical).
**It must not be loaded**:

```dsp
; header @0x0000 of the DSP whose counter is A004 (set by the OS; do NOT queue it)
ld r0, [a004]
cmp r0, r0, #0
ld r0, [e021]
cond !12, 1, 0 | st r0, [a004] | st r0, [a004]
```

The closing is queued (C-S1V4, Roland):

```dsp
; closing @0x7FF2 (preceded by a half-word 0000): frame counter E000 -> A004
ld r0, [e000]
st r0, [a004]
```

Found by bisection on the device with diagnostic builds of C-S1JX
([examples/system_1_jx/README.md](../../examples/system_1_jx/README.md) §10).

## 16. Panel effects (delay + reverb) in PLUG-OUT mode

Which OS DSP programs make the panel's delay and reverb, where they run, which memory words they share with a
PLUG-OUT and which parameters govern them. Sources: OS 1.30 (`work/sys1_v130/app_master.bin`, base `0x60000000`),
listed with `system-1/s1_progs.py` (`sets`, `words`) and `asm/disasm.py`; the SH-2, SH-101 and PROMARS PLUG-OUTs
(read only); PARAMLOG captures of 2026-10-23 with C-S1SY12L and C-S1SYFX. State: 2026-10-23. **The effects sound
when the PLUG-OUT's master-B main program does the send and the return (§16.4)** [M, C-S1SY13].

### 16.1 Panel controls [M]

PARAMLOG with C-S1SY12L (`examples/system_1/paramlog.py`), in the order they were moved:

| control | id | CC (channel 5) | values |
|---|---|---|---|
| REVERB amount | `0x18D` | 91 (`B4 5B`) | 0–255 |
| DELAY amount | `0x18E` | 94 (`B4 5E`) | 0–255 |
| DELAY time | `0x18F` | 13 (`B4 0D`) | 0–255 |

All three reach the region-0 model (master) through `vt+0x30(id, flag, value)`, like the other patch parameters.
`0x18B` (values 103–128, CC 69) appears interleaved again; it is AMP TONE (§16.6).

The OS treats them as generic group-3 parameters [L]: `0x60007302`/`0x6000730c`/`0x6000731a` load the id and jump to
`0x6000673c`; `0x6000733a`… jump to `0x60007fee`, with `r0 = 3`. It is the same pattern as for `0x18A`–`0x190`, with
no special case for the effects. The table at `0x60013d6e` (CC → id) holds `0x18D`–`0x18F` in place. The SH-2 has the
same CC → id table in its region 0 (`0x608094f2`: `0x18D`, `0x608094fe`: `0x18F`).

### 16.2 Master DSP programs in PLUG-OUT mode [L]

`s1_progs.sets()` on the OS finds 16 sets on the master. The PLUG-OUT-mode ones (queued by `0x6001ECA2`):

| set | DSP | address | size | what it is |
|---|---|---|---|---|
| 11 | master A | `0x82` | 126 B, 12 instructions | **output**: `A118`/`A119` → DC filter → × `C004` → `6000`/`6001` |
| 11 | master A | `0x6DF2`, `0x7FF2` | 14 B each | closings |
| 12 | master B | **`0x0`** | 1362 B, **107 instructions** | **delay + reverb** (§16.3): the only program that uses the delay memory (40 accesses to bank 2, `tapreq`, `h4`/`h5`) |
| 12 | master B | `0x1000` | 56 B, 11 instructions | `A118`–`A11D` = 0. The PLUG-OUT's `load` replaces it (C-S1V4: `s1mm1`) |
| 12 | master B | `0x5FF2`, `0x6FF2`, `0x7FF2` | 14 B | closings |

Sets 14 and 15 are the stock synth's (master A `0x100`, 696 modulation instructions; master B `0x1000`, 606 of mix).
Neither uses the delay memory.

**No Roland PLUG-OUT carries its own effect** [L]. Neither the SH-2 nor the SH-101 nor the PROMARS has accesses to
bank 2 (`4xxx`), `tapreq` or hidden registers in its programs, on the master or on the slave. The SH-2's master B
(`0x1000`, 406 instructions) is mix, filter and saturation with `C01C`–`C024`. The panel effects are always made by
the OS program at `0x0` of master B. A PLUG-OUT's `load` only replaces `0x1000`, so that program keeps running [I:
set 12 is loaded and nothing removes it].

### 16.3 The effect program (master B `0x0`) [L]

Order of execution per sample; instructions numbered as in the listing (`disasm.program`, without the `f7` header):

| instr. | what it does | parameters / memory |
|---|---|---|
| 1–10 | state (`A000`, conditions 25–27 of `E021`) | `A000`–`A003`, `E013`, `E020`, `E021` |
| 11–16 | **input**: reads `2000` / `2002` (the send the master-B main program leaves in that sample, §16.4), one-pole filter | state `0002`–`0006` |
| 17–45 | **delay**: read of the delay memory with `tapreq` (base `h4` = 256, result in `h5`), interpolation between two samples (`4005`/`4006`, `4008`/`4009`), feedback with a one-pole filter (0.994…, 0.996…), write `4001` | `C000`, `C001`, `C004`, `C005`, `C006`, `C007` (level, feedback, time [I: by their use]) |
| 46–53 | **delay mix** with the input (`ymacd`, `C001`, `C002`, `C003`) | → `0000` / `0001` |
| 54–61 | **reverb input**: (L + R)/32, × `C009`, two one-pole filters (0.989…, 0.9946…) | `C008`, `C009` |
| 62–76 | **reverb lines** in bank 2 (`400B`–`4015`): four cascaded `mac_p1w` stages, modulation with `ph_wrap` / `ph_abs` of a slow phase (`0x22`) | `C00C` |
| 77–95 | cascaded **allpass** (coefficients −0.579 / 0.486 / 0.877, −0.510 / 0.336) on `4017`–`402D`, L/R outputs separately | — |
| 96–104 | **output**: × `C00A`, × `C008`/`C00B`, mix with the input (`ymacd`), → `2000` / `2002` | `C008`, `C00A`, `C00B` |

Summary: **input → delay (`C000`–`C007`) → reverb (`C008`–`C00C`) → `2000`/`2002`**. The exact meaning of each
`C00x` (level, feedback, time) is inferred from where it is used [I].

### 16.4 Send and return: made by the master-B main program [L, E, M]

The effect takes its input from, and returns its result to, **bank 1 of master B itself**, through the **main
program at `0x1000`**; in PLUG-OUT mode that main program is the PLUG-OUT's.

* The stock mixer (master B `0x1000`, set 15) ends with `st r0, [2000] | st r1, [2002] | ld r0, [2001] | ld r1, [2003]` [L]:
  1. it writes the L/R mix to `2000`/`2002` (send);
  2. the effect program, on the same DSP, reads it in that sample and leaves the result (dry + delay + reverb, §16.3)
     in `2000`/`2002`;
  3. the main program reads that result in the next sample at `2001`/`2003` (bank-1 ring: what is written at S is
     read at S+1) and passes it to the output.
* The output program of master A (`0x82`) uses `2000`/`2001` and `2002`/`2003` of **its own** bank 1 only as the DC
  filter's state. Measured in the emulator running it alone with the parameters of §16.5 [E]: `6000 = A118 − [2001]`,
  `[2006] = A118 − [2001]`, `[2000]` = filter state (leak 0.99993), and `[2004]`/`[2005]` enter the output with
  `C002` = 0.0469. The banks 1 of the master's DSPs A and B are distinct [I, from the above].

Complete chain in PLUG-OUT mode:

```
slave A/B (ours)               -> 6006/6007
master B 0x1000 (PLUG-OUT):     6006/6007 x volume -> 2000/2002 (send)
master B 0x0    (OS, effect):   2000/2002 -> delay (C000-C007) -> reverb (C008-C00C) -> dry + wet -> 2000/2002
master B 0x1000 (PLUG-OUT):     2001/2003 (result of the previous sample) -> A118/A119
master A 0x82   (OS, output):   A118/A119 -> DC filter -> x C004 -> 6000/6001
```

Master-B program that passes through the effects (C-S1SY from step 13, `make_synth.py STEP=13`), followed by the
writes of 0 to `A11A`–`A11D` (the scheduler packs it into 7 instructions). **Measured: reverb, delay and delay time
sound; with the effects at 0 it sounds dry** [M, C-S1SY13, 2026-10-23]. The `s1mm1` of §15.1 (C-S1V4) writes
`A118`/`A119` directly and has no panel effects. The SH-2 does not use `2000`–`2003` in its master programs either
[L]; whether its effect sounds on the device is [A].

```dsp
; s1mm1, C-S1SY step 13 (master B @0x1000): send to the effect and take its return
ld r0, [6006]
ld r1, [6007]
ld r7, [c01c]
mov r4, r4
mul r0, r0, r7
mul r1, r1, r7
st r0, [2000]
st r1, [2002]
ld r2, [2001]
ld r3, [2003]
mov r4, r4
st r2, [a118]
st r3, [a119]
```

### 16.5 The parameters are written by the OS [M, C-S1SYFX, 2026-10-23]

Diagnostic build `PARAMLOG=1` with `fx_dump()` in `model.c`: it reads the master's parameter blocks (layout in §14.3)
and sends them by SysEx while REVERB / DELAY / DELAY time go through 0, 64, 128, 192 and 255. **The OS itself writes
the effect parameters when the knobs move**; the model does nothing:

| parameter | changed by | measured values (word `+4`, float) |
|---|---|---|
| master B `C009` (send to the reverb) | REVERB `0x18D` | 0 → 0.0382 (64) → 0.1625 (128) → 0.464 (192) → 1.0 (255), ≈ (v/255)^2.5 |
| master B `C002` (delay level) | DELAY `0x18E` | linear: 0.25 (64), 0.75 (192), 1.0 (255) |
| master B `C001` | DELAY `0x18E` | 1.0 with the delay > 0, else 0 |
| master B `C004` | DELAY `0x18E` | 0.6 with the delay > 0, else 0 |
| DELAY time `0x18F` | — | none of `C000`–`C010`; with the delay at 0 nothing changes. The effect reads `A000`–`A003` of its bank 5, probably the read positions [A] |

Fixed in PLUG-OUT mode:
* master B: `C000` = 1, `C003` = 1, `C007` = 1, `C008` = 1, `C00A` = 0.498, `C00B` = 1, `C00C` = 0.5; `C005`, `C006`,
  `C00D`–`C010` = 0;
* master A: `C000` = 0, `C001` = 1, `C002` = 0.0469, `C003` = 1, `C004` = 1, `C005`–`C007` = 0.

Services that the three Roland PLUG-OUTs use and C-S1SY does not: `FC30` (queue of master DSP A), `FCE4` (parameter
with ramp), `FD6C`, `FE14` and `FE28`. What they do in the OS:
* `FE14(x)` → `0x01009EEC`: sets or clears a bit of a chip register (mask `0x10` if x ≠ 0, `0x08` otherwise);
* `FE28(v)` → `0x01009F3E`: writes with a ramp the parameter `sel` 0, id 0 from v (float, × 1.0, to fixed point with
  shift −24).

The SH-2 calls them from the life-cycle methods of its model (`0x60809DE8`…`0x60809E84`); `FE14` only with x = 0 in
what was seen. What exactly they do: [A].

### 16.6 The CRUSHER knob and AMP TONE: an effect of the PLUG-OUT [M, L]

* PARAMLOG with C-S1SYFX (extended dump of the master: B `C000`–`C03F`, A `C000`–`C00F`): CRUSHER `0x18C` (CC 12) and
  AMP TONE `0x18B` (CC 69, bipolar, 128 = centre) **change no master parameter**. The OS does not implement them in
  PLUG-OUT mode.
* SH-2, SH-101 and PROMARS implement them in their master-B main program (`0x1000`): they read `C01C`–`C037`, written
  by their model, with the crusher mask `bic r4, r1, -r5`. Their manuals call them EFFECT DEPTH ("modifies the tonal
  character by distorting the waveform") and AMP TONE (brightness). In the VST, `EFFECT TYPE` (0–5) chooses the effect;
  the names do not appear in `Script.xml` / `ScriptEdit.xml`, which only carry `EFX CRUSHER`.
* The LPF type button only sends CC 115, with no parameter for the model.
* On the SYSTEM-8 each PLUG-OUT carries its effects in the effect slot of DSP4 (JX-3P / Juno: OVERDRIVE, DISTORTION,
  CHORUS, FUZZ, CRUSHER, PHASER). The chorus/ensemble is global on the master (`m1_tcm` `efdc`/`f7dc`) and the
  Juno-60's is in `J60_b000_60a1f8e8`: triangular LFO, two interpolated taps in the delay memory, BBD filters. The
  attempts of C-S1SY: [examples/system_1_synth/README.md](../../examples/system_1_synth/README.md) §7.4.

### 16.7 Open

1. The order main program / effect inside the sample on master B (the step-13 design works either way: if the main
   program ran first, it would read its own send of the previous sample).
2. `FE14` / `FE28` and bank 5 `A000`–`A003` of the effect (DELAY time).

## 17. Complete example: one note in C-S1V4 (voice 0)

| step | where | what | mechanism |
|---|---|---|---|
| 1 | master, region 0 | the assigner picks voice v and does `set(2, 0x110 + v, note)`, `set(2, 0x120 + v, vel)` | [Vol. III ch. 2](../vol3_voice_allocation_scheduling/system1_voice_allocation.md) §2 |
| 2 | slave, region 1 | `param(0x110 + v)` → `FD6C(0, {0, 0, 0x30 + v}, (note − 12)/12)`; `param(0x120 + v)` → `0x38 + v` = 1.0 | §14.2 |
| 3 | slave A, `s1m0` | saw of voice v with pitch `A030+v`, × gate `A038+v`, sum × 0.25 → `A150` | §14.1, §14.5 |
| 4 | slave B, `s1m1` | `A150` × `C000` → `6006`/`6007` | §14.1, §14.4 |
| 5 | master B, `s1mm1` | × `C01C` → `A118`/`A119` | §15.1 |
| 6 | master A, OS `@0x82` | DC filter, × `C004` → `6000`/`6001` | §15.1 |

## 18. Design rules (SYSTEM-1)

1. Always load an own program on master B (`0x1000`) that writes `A118`/`A119`; otherwise silence. To keep the panel
   effects, send to `2000`/`2002` and read `2001`/`2003` (§16.4).
2. Apply VOLUME (`0x100`) in the PLUG-OUT (e.g. `C01C` of master B).
3. Usable bank 5: `A020`–`A1FF` (except `A118`–`A11F` on the master); two programs of the same chip must not share
   words by accident (it is a common bank).
4. Write bank 5 from the ARM only with `sel` 0/1 (`A000`–`A1FF`).
5. Slave → master: `6006`–`600D` (8 words) and `6026`–`602D` (8 more); master → slave: `600E`–`6021` (20).
6. **Never queue `0x0000`** (it hangs the device, §15.3). Queue programs, empty section ends and the closing `0x7FF2`,
   as Roland's PLUG-OUTs do in their `load`.
7. Budget **measured on slave A: 703 instructions per sample run, 826 hang the device** when the PLUG-OUT is selected
   (also with pure `mov r4, r4` filler); slave B: 708 run [M] — less than the ≈ 925 of the SYSTEM-8. Packing transfers
   with the operations, as Roland does, makes programs fit: a load is used in the next instruction [M]
   ([examples/system_1_synth/README.md](../../examples/system_1_synth/README.md) §4–5).
8. **Access limits per instruction** (breaking them hangs the device [M]): bank 5 ≤ 1 load and ≤ 1 store (better: a
   single access), bank 0 ≤ 2 loads and ≤ 2 stores, bank 6 ≤ 2 loads. The slave DSPs store only in `A100`–`A1FF` [I].
   The emulator rejects programs that break them (`emu/chain.py access_violations`); `asm/s1rules.py` checks the
   rules at build time. Detail: [examples/system_1_jx/README.md](../../examples/system_1_jx/README.md) §5.

## 19. Open (SYSTEM-1)

| # | what |
|---|---|
| 1 | whether bank 0 is per DSP or per chip; whether bank 1 is really shared between A and B of the slave |
| 2 | latency of bank 5 between A and B, and of IOMD between chips |
| 3 | what `6026`–`602D` are in the stock synth and whether they reach the audio in PLUG-OUT mode |
| 4 | `FCE4`: measured (linear, 3 kHz); missing the dependence on the distance and the modes `z` ≥ 2 |
| 5 | a sweep of the IOMD words like the SYSTEM-8's (§4.7) |

Evidence: audio chain of C-S1TONE / C-S1V4, VOLUME: [S1 wave-bus lab log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s1_wave_bus_log.md)
§4 and [examples/system_1/README.md](../../examples/system_1/README.md) §2.2, §6; `FD6C` identical on master and slave,
`6026`–`602D`: `system-1/s1_progs.py` and [Vol. III ch. 9](../vol3_voice_allocation_scheduling/system1_polyphony_studies.md) §3.4;
Roland's `load` (does not queue `0x0000`) and the hang when queuing it: §15.3 and
[examples/system_1_jx/README.md](../../examples/system_1_jx/README.md) §10; stock programs of master and slave:
[S1 wave-bus lab log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s1_wave_bus_log.md) §2, §3 and
[Vol. III ch. 9](../vol3_voice_allocation_scheduling/system1_polyphony_studies.md) §1.
