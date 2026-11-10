# Volume III — Chapter 1: SYSTEM-8 Voice Allocation to the DSPs

| | |
|---|---|
| Version | 2.0 (2026-10-22). Measurements behind each fact: [lab log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) |
| Scope | How a note from the keyboard reaches a voice, which DSP computes each part of each voice, which words the ARM writes for each voice, the interface of the oscillator slots, the two families of Roland PLUG-OUTs and the voice limit. The transport mechanisms (MXM, rings, mailbox, IOMD) are in [Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) |
| Device | SYSTEM-8 (master + 2 slaves, 6 DSPs per slave). SYSTEM-1: [system-1/VOICE_ALLOCATION.md](../../system-1/VOICE_ALLOCATION.md) (summary in [ch. 2](system1_voice_allocation.md)) |
| Reference code | assigner `examples/from_scratch/model/model.c`; engines `examples/from_scratch/engine/engine.c` (JX-3P with Roland's microcode) and `v3/engine_v3.c` (C-V4, own microcode) |

Evidence: **[M]** measured on the device · **[L]** read in Roland's microcode or ARM code · **[C]** our code · **[E]**
emulator · **[I]** inferred · **[A]** open. *(log §n)* = section of the lab log that supports it. `dsp` blocks: they
assemble (`python3 tools/doccheck.py docs/vol3_voice_allocation_scheduling/system8_voice_allocation.md`). Notation of
addresses and times: [Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §1.

---

## 1. Summary

| | value | evidence |
|---|---|---|
| voices | 8: 4 per slave (slave 0: 0–3, slave 1: 4–7) | [M] |
| who assigns | region 0 of the PLUG-OUT (on the master), at the OS's request | [C], [L] |
| how a note reaches a slave | voice parameters of group 2: `0x429 + v` note, `0x43A + v` velocity; **both slaves receive all eight** | [M] |
| split inside a slave (family A) | two chains of three DSPs: local voices 0/1 in DSP0 → DSP2 → DSP4, voices 2/3 in DSP1 → DSP3 → DSP5 | [L], [M] with C-V4 |
| voices per oscillator DSP | 2 (voice A and voice B), in the slots DCO1 `0x6000` and DCO2 `0xB000` | [L] |
| real limit | DSP time (≈ 925 instructions per sample and DSP), not a number of voices | [M] (§8) |

---

## 2. From the key to the voice

### 2.1 Path

```
key / MIDI ─► master's OS ─► region 0: assigner (vt[3] note_on, vt[2] note_off, …)
                                    └─ set(part, 2, 0x429+v, note); set(part, 2, 0x43A+v, velocity) ─► OS
OS ─► both slaves: engine of region 1, vt+0x30 param(id, value) for each changed voice id
      slave s: local voice lv = v − 4·s; if 0 ≤ lv < 4, it writes pitch and gate into its DSPs (§4)
```

### 2.2 Voice ids (group 2)

| id | content | sent by | evidence |
|---|---|---|---|
| `0x429 + v` (v = 0–7) | MIDI note of voice v | assigner | [L], [M] |
| `0x43A + v` | velocity 1–127; **0 = release** | assigner | [L], [M] |
| `0x44B + v` | portamento legato flag | our assigner | [C] |
| `0x45C` | voice with the highest note | assigner | [L], [C] |
| `0x42D + v`, `0x43E + v` | sent by Roland's assigner; meaning unknown | Roland | [L], [A] |

The ids are in blocks of 17 (`0x429`, `0x43A`, `0x44B`, `0x45C`): there is room for more than 8 voices (§8) [I].

### 2.3 Assigner (region 0)

The assigner is an object of the PLUG-OUT; the OS calls it with each key. Excerpt of our poly rule ("same note > oldest
free voice > released voice with hold > oldest voice"), common to C-V4 and to the SYSTEM-1 ports:

```c
/* examples/system_1/model.c (the same rule as examples/from_scratch/model/model.c) */
static void key(struct assign *a, int note, int vel)
{
    ...
    int same = -1, idle = -1, held = -1, v;
    for (int k = NV - 1; k >= 0; k--) {          /* order[]: the most recently used first */
        int c = a->order[k];
        if (a->v[c].note == note) same = c;
        if (idle < 0 && !a->v[c].vel) idle = c;
        if (held < 0 && a->v[c].held == 1) held = c;
    }
    if (same >= 0) v = same;
    else if (idle >= 0) v = idle;
    else if (held >= 0) v = held;
    else v = a->order[NV - 1];
    gate_off(a, v);                     /* a new key always starts with the gate closed */
    play(a, v, note, vel);              /* set(2, ID_NOTE + v, note); set(2, ID_VEL + v, vel) */
    use(a, v);
}
```

Practical consequence: with all voices free, the first note goes to the **oldest voice in the order**, not to voice 0.
To know which voice played which note in a measurement, `examples/from_scratch/voicelog/voicelog.py` is used (log of
the voice ids).

### 2.4 Engine (region 1): voice filter per slave

```c
/* examples/from_scratch/engine/engine.c, e_param (vt+0x30) */
if (id >= 0x429 && id < 0x429 + 8) {
    v = id - 0x429 - e->first;                   /* first = 4 * slave */
    if (v >= 0 && v < 4) note_pitch(e, v, value);
    return;
}
if (id >= 0x43A && id < 0x43A + 8) {
    v = id - 0x43A - e->first;
    if (v >= 0 && v < 4) note_gate(e, v, value);
    else active_flags(e);                         /* a voice of the other slave */
    return;
}
```

If an engine accepts the eight voices on both slaves, each note sounds twice (slave 1 reaches the output through slave
0, [Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §5.2) [M] *(log WB §4e)*.

---

## 3. Split per DSP (family A: JX-3P, JUNO-60, JUNO-106, JUPITER-4, JUPITER-8)

### 3.1 Functions

| DSP | local voices | function | main program (JX-3P) | slots |
|---|---|---|---|---|
| DSP0 | 0, 1 | LFO, envelopes ENV1 (filter) and ENV2 (amplitude), portamento, pitch | MAIN0, 5066 B | `0x8000`: empty in our reproduction (`engine.c`); in Roland [A] |
| DSP1 | 2, 3 | same | MAIN1, 5018 B | `0x8000`: same |
| DSP2 | 0, 1 | oscillators: prepares the inputs of the slots, mixes their outputs | MAIN2, 2068 B | DCO1 `0x6000` (task 2), DCO2 `0xB000` (task 3) |
| DSP3 | 2, 3 | same | MAIN3, 2018 B | DCO1, DCO2 |
| DSP4 | 0, 1 + mix | filter (4-pole VCF), VCA, **output mix** of the slave | MAIN4, 5174 B | effect `0xB000` (OD, DS, CH, FZ, CR, PH) |
| DSP5 | 2, 3 | filter and VCA; hands the voices to DSP4 | MAIN5, 5016 B | `0x6000`: empty in our reproduction; in Roland [A] |

[L] from sizes and pointers of the descriptor table (`re/alloc_scan.py`, *log §1, §2*); [M] with C-SAWPOLY (Roland's
chain, audible voices) and with C-V4 (own microcode with the same layout, the 8 voices).

### 3.2 Data flow of one chain (local voices 0/1)

| hop | words | what travels | evidence |
|---|---|---|---|
| ARM → DSP0 | bank 5 `A021` (pitch A), `A033` (pitch B), `A022`/`A034` (gate) | note and gate | [C], [L] |
| DSP0 → DSP2 | `Z 8404`–`8416` → `M 8004`–`8016` (19 words) | pitches with modulation, envelopes, LFO | [L] |
| DSP0 → DSP1 | mailbox `A404`, `A409`–`A40E` | common data (LFO) for the other chain | [L] |
| DSP2 → slots | bank 1 `[2201]`…`[2219]` | inputs of DCO1/DCO2 (§5) | [L] |
| slots → DSP2 | `[221B]`…`[2229]`, read at +1 | audio of the oscillators | [L], [M] |
| DSP2 → DSP4 | `Z 8404`–`841A` → `M 8004`–`801A` | audio of the two voices (`8416`–`8419`), pitches and envelopes forwarded | [L] |
| DSP4 → DSP2 | `M 801B`–`801D` → `S 861B`–`861D` | return | [L] |
| DSP2 → DSP0 | `M 8017`–`8019` → `S 8617`–`8619` | return | [L] |
| DSP4 ↔ DSP5 | mailbox `A404`–`A407` (→ DSP5), `A400`–`A403` (→ DSP4) | voices 2/3 already filtered towards the mix | [L], [M] |
| DSP4 → output | IOMD `6020`/`6021` | mix of the slave | [M] |

The chain of voices 2/3 is the same with DSP1 → DSP3 (`Z 8400`–`8410` → `O 8200`–`8210`) → DSP5 (`Z 8400`–`8414` →
`O 8200`–`8214`), without S returns [L].

### 3.3 Time cost (JX-3P, 4 voices per slave) [E]

| DSP | programs | instructions executed per sample (max.) |
|---|---|---|
| DSP0 / DSP1 | MAIN0 / MAIN1 | 516 / 514 |
| DSP2 / DSP3 | MAIN2 + DCO1 SAW + DCO2 SAW | 839 / 834 |
| DSP4 / DSP5 | MAIN4 / MAIN5 | 532 / 513 |

Measured with `system-1/dsp_fit.py` (emulator; see [system-1/DSP_FIT.md](../../system-1/DSP_FIT.md)). DSP2/3 are at
90 % of the budget: that is why Roland puts **two** voices per oscillator DSP.

---

## 4. Words the ARM writes per voice

Formulas of `engine.c` (`note_pitch`, `note_gate`, `active_flags`), verified against the JX-3P engine
(`engine/test_emu.py`, section on notes; result in [NOTES.md](../vol7_cleanroom_evidence_archive/NOTES.md)) [C], [L]. `u = lv >> 1`, `k = lv & 1`.

| local voice | pitch (octaves, `(note − 12)/12`) | envelope gate | VCA: gate / velocity curve / velocity |
|---|---|---|---|
| 0 | DSP0 `A021` | DSP0 `A022` | DSP4 `A02A` / `A029` / `A022` |
| 1 | DSP0 `A033` | DSP0 `A034` | DSP4 `A039` / `A038` / `A031` |
| 2 | DSP1 `A021` | DSP1 `A022` | DSP5 `A029` / `A028` / `A021` |
| 3 | DSP1 `A033` | DSP1 `A034` | DSP5 `A038` / `A037` / `A030` |
| all | — | — | "some voice sounds": DSP0 and DSP1 `A029`, `A03B` |

```c
/* examples/from_scratch/engine/engine.c */
static void note_pitch(engine_t *e, int lv, int note)
{
    int u = lv >> 1, k = lv & 1;
    e->note[lv] = note;
    p_bank(u, 0x21 + 0x12 * k, (float)(note - 12) / 12.0f);
}

static void note_gate(engine_t *e, int lv, int vel)
{
    int u = lv >> 1, k = lv & 1, d = 4 + u, s = 0xf * k - u;   /* the ids of DSP5 go one lower */
    vel = clampi(vel, 0, 127);
    if (vel > 0) {
        p_bank(d, 0x2a + s, 1.0f);
        p_bank(d, 0x29 + s, vel_curve[vel]);
        p_bank(d, 0x22 + s, (float)vel / 127.0f);
        p_bank(u, 0x22 + 0x12 * k, 1.0f);
        e->on[lv] = 1;
    } else {
        p_bank(u, 0x22 + 0x12 * k, 0.0f);
        e->on[lv] = 0;
    }
    active_flags(e);
}
```

A pitch of `4.75` octaves is A 440 Hz (note 69). The patch parameters (filter, envelopes, LFO, mix…) are not per voice:
they are written by the parameter model (`engine/model_spec.py`; the complete version, with filter and envelopes, is
`examples/system_1_jx/model_full.py`, see [examples/system_1_jx/README.md](../../examples/system_1_jx/README.md) §8).

---

## 5. Interface of the oscillator slots (DSP2/DSP3)

### 5.1 Words

Bank 1, private to each DSP, a ring ([Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §4.2). The main program writes the inputs before the
slots run; it reads each output one sample later at word +1.

| | DCO1 voice A | DCO1 voice B | DCO2 voice A | DCO2 voice B |
|---|---|---|---|---|
| pitch (octaves) | `[2201]` | `[220E]` | `[2207]` | `[2214]` |
| other inputs | `[2203]`–`[2206]` | `[2210]`–`[2213]` | `[2209]`–`[220C]` | `[2216]`–`[2219]` |
| outputs (written) | `[221F]`, `[221B]` | `[2227]`, `[2223]` | `[2221]`, `[221D]` | `[2229]`, `[2225]` |
| outputs (read by the main program) | `[2220]`, `[221C]` | `[2228]`, `[2224]` | `[2222]`, `[221E]` | `[222A]`, `[2226]` |

DCO2 inputs for the panel's **Cross Mod** [M] ([Vol. VI ch. 6](../vol6_reference_designs_applications/custom_osc.md) §2): `[2209]`/`[2216]` = sync enabled,
`[220A]`/`[2217]` = METAL, `[220B]`/`[2218]` = one impulse per cycle of DCO1 (sync pulses).

Rules:
* The two outputs of each voice carry different values in Roland's code (not decoded); writing the same signal in both
  works [M].
* Registers: the slots see what the previous program of the same DSP left. Roland's DCO1 **reads `r2`/`r3` before
  writing them** (in the emulator it depends on what MAIN2 leaves) [E]: do not reorder or separate MAIN2 and DCO1
  without keeping those registers ([examples/system_1_jx/README.md](../../examples/system_1_jx/README.md) §4.1).
* To a registered slot the engine always assigns a program (an own null one, `gate/dco2_null.hex`, if there is no
  oscillator) instead of leaving it without a program [C].

### 5.2 Example: saw oscillator in the DCO2 slot (C-V4, voice A)

Our program `examples/from_scratch/re/dspexp/own/saw_dco2.txt` (voice A; voice B is the same with `[2214]`, phase
`0680` and outputs `[2229]`/`[2225]`). Pitch `p` in octaves → phase increment `2^(p − 4.75) · 880/96000` (A 440 in a
phase from −1 to 1) with a series of `e^x` raised to the 16th; phase in the bank-0 ring.

```dsp
mov r5, #0.0
mov r6, #1.0
ld r1, [0671]              ; phase of the previous sample (ring, WAVE_BUS §4.1)
ld r0, [2207]              ; pitch written by the main program in this sample
add r0, #-4.75             ; octaves relative to A 440
mul r2, r0, #0.04332169878499658     ; x = (p - 4.75) · ln 2 / 16
mov r3, #0.041666666666666664:32     ; e^x by Horner: 1/24, 1/6, 1/2, 1, 1
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3             ; (e^x)^16 = 2^(p - 4.75)
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667    ; · 880 / 96000
addxy r1, r3, r1           ; phase += increment
addxy r2, r1, #-1.0        ; if phase >= 1: subtract 2 (saturation in four steps)
max r2, r2, r5
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #256.0
min r2, r2, r6
mul r2, r2, #2.0
subxy r1, r2, r1
st r1, [0670]              ; phase for the next sample
st r1, [2221]              ; output of DCO2, voice A (the main program reads it at 2222)
st r1, [221d]
```

In tune to ±1.2 cents from note 36 to 96 [M] ([Vol. VI ch. 6](../vol6_reference_designs_applications/custom_osc.md)).

---

## 6. Family B (SH-101, SH-2, SYSTEM-100, PROMARS, base of the SYSTEM-8)

| PLUG-OUT | main1 | main5 | what it does |
|---|---|---|---|
| SH-101, SH-2, SYSTEM-100 | 200 B | 84 B | main0 reads `8613`–`8617`, `8627`–`862B`; main2 reads `8000`–`8026`; main4 writes IOMD `6020`… |
| PROMARS | 200 B | 84 B | the same (main4 5640 B) |
| SYSTEM-8 base (`Code8_0`) | 5566 B | 4900 B | main0/main1 read `8613`/`860F` and write `8400`…; main5 writes `60B0`… |

[L] *(log §2.2, §6.2)*. main1 and main5 are templates: another organisation of voices (the first three are
monophonic). They do not use the pair mailbox for the voice. **They are no reference for four voices per slave.**

Sizes of the main programs of family A (bytes, main0–main5): JX-3P 5066/5018/2068/2018/5174/5016; JUNO-60
4610/4564/1548/1512/6122/7406; JUNO-106 4982/4934/3000/2950/5344/5236; JUPITER-4 4374/4328/1482/1446/6474/7464;
JUPITER-8 5838/5790/3040/2990/5038/4912 [L].

---

## 7. Reference implementation: C-V4 (own microcode)

C-V4 reproduces the layout of family A with own minimal programs (`examples/from_scratch/v4/make_v4.py`,
`build_v4.sh`), and sounds with the 8 voices on both slaves, each with its note (−3.2 to +2.6 cents) [M] *(log §6.7)*.

| DSP | program | does |
|---|---|---|
| DSP0 | `v4m0` | `A021`/`A033` → `Z 8404`/`8405` ([Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §4.3) |
| DSP1 | `v4m1` | `A021`/`A033` → `Z 8400`/`8401` |
| DSP2 | `v4m2` + saw in DCO2 | `M 8004`/`8005` → `[2207]`/`[2214]`; `[2222]`/`[222A]` → `Z 8404`/`8405` (WAVE_BUS §4.2) |
| DSP3 | `v4m3` + saw in DCO2 | `O 8200`/`8201` → slot; outputs → `Z 8400`/`8401` |
| DSP4 | `v4m4` | voices 0/1 × gate `A02A`/`A039`, + `A400`/`A401` + IOMD `6000`–`6003` → `6020`/`6021` (WAVE_BUS §5.1) |
| DSP5 | `v4m5` | voices 2/3 × gate `A029`/`A038` → mailbox `A400`/`A401` (WAVE_BUS §4.4) |

Example of the main program of DSP1 (the chain of voices 2/3 uses zone Z towards the odd DSP):

```dsp
; v4m1 (DSP1): pitch of voices 2 and 3 -> DSP3 (zone O)
ld r1, [a021]
mov r4, r4
st r1, [8400]      ; -> DSP3 O 200
ld r1, [a033]
mov r4, r4
st r1, [8401]      ; -> DSP3 O 201
```

Mistakes C-V4 corrected with respect to C-V3 *(log §5, §6.3)*: an assumed MXM offset (voice 1 silent); outputs of the
slot read without the +1 of the ring; voices of DSP5 sent to `60B0` (inaudible); DSP4 of slave 0 not summing
`6000`–`6003` (slave 1 silent).

---

## 8. Voice limit

There is no measurement of a limit of 8 in the hardware [M]. Every layer that fixes "8" is software:

| layer | what it fixes today | changeable in a PLUG-OUT? | evidence |
|---|---|---|---|
| region 0 (`model/model.c`) | `NV_MAX 8`, ids `0x429 + v`, `0x43A + v` with v = 0–7 | yes | [C] |
| master's OS | calls the assigner with a number of voices and forwards the voice ids | **unknown** whether it forwards ids above `0x430` | [A] |
| engines | `lv = v − 4 · slave` | yes | [C] |
| microcode | 2 voices per oscillator DSP | yes | [L], [C] |
| link between slaves | 4 IOMD words (`6000`–`6003`), each can carry a mix | does not limit | [L], [M] |
| **DSP time** | ≈ 925–945 instructions per sample and DSP; going over hangs the slave until it is switched off | **physical limit** | [M] (DSP_HARDWARE §1) |

A saw like the one of §5.2 costs 33 instructions per voice [C]: an oscillator DSP would have time for ≈ 25. Roland's
oscillators cost ≈ 160 per voice (DCO1 SAW: 325 for two voices) [E].

Test that would decide the open point (not done): region 0 with `NV_MAX 12` and an engine that plays a fixed tone when
it receives `0x431` (voice 8). If it sounds, the OS forwards ids above voice 7. On the SYSTEM-1 the equivalent was
measured (8 voices with ids `0x114`–`0x117`, [system-1/VOICE_ALLOCATION.md](../../system-1/VOICE_ALLOCATION.md) §6).

---

## 9. Open

| # | what | state |
|---|---|---|
| 1 | meaning of `0x42D + v` and `0x43E + v` (Roland's assigner) | [A] |
| 2 | Roland's `e_load` (how it indexes its program table `0x60A2581C`) | [A]; our order `main_prog[d]` → DSP d is [M] |
| 3 | complete semantics of MAIN0–MAIN5 (beyond their bus references and the amplitude envelope) | [A]; part in [Vol. III ch. 5](dsp_main_programs.md) |
| 4 | whether the OS delivers more than 8 voices on the SYSTEM-8 | [A] (§8) |
| 5 | why voices 3 and 4 have no note message in the C-V4 log | [C]: our assigner does not forward an equal note; no effect |

---

## 10. Evidence and old references

| datum | section of the lab log ([S8 voice-allocation lab log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md)) |
|---|---|
| method (descriptors, sizes, listings) | §0, §1 |
| families A and B in all the images | §2.1, §2.2 |
| pointer table of the JX-3P | §2.4 |
| DCO outputs and words `2220`–`222A` | §2.5, §6.3 D |
| C-SAWPOLY (Roland's chain on the device) | §4 |
| mistakes made | §5 |
| emulator model, questions A–H, C-V4 measured | §6.1–§6.7 |
| signature check (`sigcheck.py`) | §7 |
| voice limit | §8 |

References "[Vol. III ch. 1](system8_voice_allocation.md) §n" written before 2026-10-22 point to those sections of the lab log.

> Paths `re/…`, `engine/…`, `model/…`, `v3/…` are relative to `examples/from_scratch/`.
