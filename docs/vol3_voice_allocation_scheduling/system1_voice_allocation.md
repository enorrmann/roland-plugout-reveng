# Volume III — Chapter 2: SYSTEM-1 Voice Allocation to the DSPs

> **Scope:** SYSTEM-1. The SYSTEM-8 counterpart is [ch. 1](system8_voice_allocation.md); the studies of more voices
> (patching the OS, polyphonic SH-101, SYSTEM-8 microcode) are [ch. 9](system1_polyphony_studies.md).

| | |
|---|---|
| Version | 2.0 (2026-10-22). Measurements behind each fact: [lab log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s1_voice_allocation_log.md) |
| Scope | How a note reaches a voice, which DSP computes each voice in the stock synth, in Roland's PLUG-OUTs and in ours, the program memory (main and sections) and the number of voices. Transport between DSPs: [Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) |
| Device | SYSTEM-1, OS 1.30. SYSTEM-8: [ch. 1](system8_voice_allocation.md) |
| Reference code | C-S1V4 / C-S1V8 (`examples/system_1/`, measured); C-S1JX (`examples/system_1_jx/`, not installed) |

Evidence: **[M]** measured · **[L]** read in the OS or in Roland's PLUG-OUTs · **[C]** our code · **[E]** emulator ·
**[I]** inferred · **[A]** open. `dsp` blocks: they assemble (`python3 tools/doccheck.py
docs/vol3_voice_allocation_scheduling/system1_voice_allocation.md`).

---

## 1. Summary

| | value | evidence |
|---|---|---|
| voices of the stock synth | 4 | [L] |
| voices of a PLUG-OUT | those its assigner distributes: **8 measured** (C-S1V8) | [M] |
| voice ids (group 2) | `0x110 + v` note, `0x120 + v` velocity (0 = release), `0x128 + v` trigger | [L], [M] |
| who receives the voices | the slave: engine of region 1, `vt+0x30 param(id, 0, value)` for each changed id | [M] |
| voice DSPs | slave A and B (2) | [L] |
| per-voice modulation (stock synth) | on the **master** (20 words over IOMD to the slave) | [L] |

---

## 2. From the key to the voice

### 2.1 Path

```
key / MIDI ─► master's OS ─► assigner (SYNINFO+0x2C of region 0; vtable of 9:
                                   reset, changed, note_off, note_on, cc, part 0x81, bend, -, -)
                 └─ services FC04 set(group, id, v), FC08 flush(group), FC0C touch, FC00 get
OS ─► slave: dispatcher 0x6004CE64 → engine vt+0x30 (id, 0, value) for each changed id of groups 0–3
OS ─► master: model (SYNINFO+0x30) vt+0x30 (id, flag, value) for the parameters (panel, VOLUME 0x100)
```

### 2.2 Voice ids (group 2) [L], [M]

| id | content | block | evidence |
|---|---|---|---|
| `0x110 + v` | note | 16 ids (`0x110`–`0x11F`) | [M] v = 0–7 |
| `0x120 + v` | velocity; 0 = release | 8 ids (`0x120`–`0x127`) | [M] v = 0–7 |
| `0x128 + v` | trigger (1/0) | 8 ids | [L] |
| `0x130` | bend | — | [L] |
| `0x131` | modulation | — | [L] |
| `0x134` | expression | — | [L] |

### 2.3 Assigner and engine of an own PLUG-OUT

The assigner of C-S1V4/C-S1V8 is the poly rule of [ch. 1](system8_voice_allocation.md) §2.3 with `NV` voices. The
slave's engine receives the ids and writes bank 5:

```c
/* examples/system_1/engine.c */
static void e_param(engine_t *e, int id, int flag, int value)    /* vt+0x30 */
{
    (void)flag;
    if (id >= 0x110 && id < 0x110 + NV)
        voice_pitch(e, id - 0x110, value);       /* FD6C(0, {0, 0, 0x30 + v}, pitch_bits[note]) */
    else if (id >= 0x120 && id < 0x120 + NV)
        voice_gate(e, id - 0x120, value);        /* FD6C(0, {0, 0, 0x38 + v}, 1.0 / 0.0) */
}
```

`pitch_bits[n]` = IEEE bits of `(n − 12)/12` (no FPU, table computed on the PC by `make_s1.py`).

---

## 3. Stock synth [L]

### 3.1 Split

| DSP | main | voice sections | what |
|---|---|---|---|
| master | modulation (`0x60097A7C`, 696 instr.) and mixer (`0x60095B0C`) | — | envelopes/LFO per voice → 20 words `600E`–`6021` to the slave |
| slave A | `0x600185B0` (241 instr.) | 4, one per voice | prepares the inputs of the sections (bank 1), collects their outputs, passes data and audio to B (`A100`–`A10D`) |
| slave B | `0x60018F44` (237 instr.) | 4, one per voice | feeds and collects its sections; outputs `6006`–`600D` × `C000` |

The voice programs come in sets (48 voice programs, in identical pairs, 24 variants; `system-1/voice_sets.py`); the
four of a set only change in the data addresses of each voice.

### 3.2 Bank-1 words per voice (section ↔ main)

| voice | written by the main program of A | written by the main program of B | read back |
|---|---|---|---|
| 0 | `2004`–`2007` | `2000`–`2003` | `2005`, `2007`, `2009` |
| 1 | `200E`–`2011` | `200A`–`200D` | `200F`, `2013` |
| 2 | `2014`–`2017` | `2010`–`2013` | `2015`, `2017`, `2019` |
| 3 | `201E`–`2021` | `201A`–`201D` | `201F`, `2023` |

That A and B write into the same bank 1 without overwriting each other indicates that they share it [I]. What each
word is (pitch, level, shape): [A].

---

## 4. Program memory: main and sections

Each DSP of the SYSTEM-1 has 32 KiB of program ([ch. 3](slot_program_model.md) §3):

```
0x0000  OS header (do not load)   0x1900  section 2      0x4900  section 4
0x0100  MAIN (up to 0x18FF, 6 KiB) 0x3100  section 3      0x6100  section 5
                                                          0x7FF2  closing
```

* The main program always runs; the sections, only if their bit is set. The main program runs before the sections
  [I] (Roland's programs assume it).
* `FC44(index, on)` switches a section on or off; `FC40` places a voice program in a section; the OS switches them all
  off when reloading [L]. Indices:

| index | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|---|---|---|---|---|---|---|---|---|
| DSP | A | A | B | B | A | A | B | B |
| section | `0x1900` | `0x4900` | `0x1900` | `0x4900` | `0x3100` | `0x6100` | `0x3100` | `0x6100` |

* The programs are loaded in **sets** `{address, bytes, pointer}` terminated by `{0, 0, 0}`, queued with `FC30` (DSP A)
  or `FC34` (DSP B) from `load` (master: vt[2] of the model; slave: vt+0x08 of the engine) [L], [M]. **Never with the
  entry `0x0000`** (the OS header): queuing it hangs the device [M] ([Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md)
  §15.3, with the `load` sequence of Roland's PLUG-OUTs). An own voice **section** is placed with `FC40`; queued with
  `FC30` it is copied but does not run [M] ([ch. 3](slot_program_model.md) §3.2). Example, slave of C-S1JX (main + a
  section switched on; this build queues the section with `FC30`):

```c
/* examples/system_1_jx/engine.c */
static struct s1_prog set_a[] = {
    { 0x1900, MC_SA_SEC_BYTES, mc_SA_SEC },         /* section 2 */
    { 0x0100, MC_SA_MAIN_BYTES, mc_SA_MAIN },       /* main */
    { 0x7FF2, MC_CLOSE_A004_BYTES, mc_CLOSE_A004 }, /* closing */
    { 0, 0, 0 },
};

static void e_load(engine_t *e)                                           /* vt+0x08 */
{
    for (int i = 0; i < 8; i++)
        ((slot_fn)SVC(0x44))(i, 0);                 /* all sections off */
    ((s1_queue_fn)SVC(0x30))(set_a);                /* DSP A */
    ((s1_queue_fn)SVC(0x34))(set_b);                /* DSP B */
    ((slot_fn)SVC(0x44))(SEC_A_1900, 1);            /* index 0: A, 0x1900 */
    ((slot_fn)SVC(0x44))(SEC_B_1900, 1);            /* index 2: B, 0x1900 */
    jx_send_state();
}
```

---

## 5. PLUG-OUTs: who does what

| | voices | slave A | slave B | master A | master B | evidence |
|---|---|---|---|---|---|---|
| SH-101 | 1 | main 575 instr. (the whole synth); sections off | 12 instr.: `A10A`–`A10C` × `C000` → `6006`–`600A` | 410 instr.: modulation → `600E`–`6021` | 436 instr.: mix | [L] |
| SH-2 | 1 | main 108 + sections `0x1900` (353) and `0x3100` (245) | 62 instr.: → `6006`–`600B`, `6026`–`602D` | 367 instr. | 406 instr.: mix, volume (`C01C`…), saturation → `A118`/`A119` | [L] |
| PROMARS | 1 | main 373 + section `0x1900` (344) | main 4 + section `0x1900` (69) | 426 instr. | 313 instr. | [L] |
| **C-S1V4** | 4 | 4 saws in the main (137 instr.) → `A150` | `A150` × `C000` → `6006`/`6007` | (OS) | `6006`/`6007` × `C01C` → `A118`/`A119` | [M] |
| **C-S1V8** | 8 | 8 saws (269 instr.) | same | (OS) | same | [M] |
| **C-S1JX** | 4 | MAIN5 + section MAIN3 (voices 2, 3) | MAIN1 + section DCO1 (voices 2, 3) | MAIN2 + MAIN4 (voices 0, 1) | MAIN0 + DCO1 + tail (voices 0, 1) | [C], [E] |

Instructions of Roland's PLUG-OUTs: executed per sample in the emulator (they have no jumps: static = executed),
`system-1/s1_progs.py` ([Vol. III ch. 9](system1_polyphony_studies.md) §3.4).

### 5.1 C-S1V4: words per voice [M]

| voice | pitch (ARM → bank 5 of the slave) | gate | phase (bank 0, ring) | output |
|---|---|---|---|---|
| v (0–3; 0–7 in C-S1V8) | `A030 + v`, `(note − 12)/12` | `A038 + v` (1.0 / 0.0) | writes `0300 + 0x10·v`, reads `+1` | sum × 1/NV → `A150` |

Voice 0 of the main program of slave A (`examples/system_1/s1m0.s`; the formula of the saw is explained in
[ch. 1](system8_voice_allocation.md) §5.2):

```dsp
mov r5, #0.0
mov r6, #1.0
mov r7, #0.0             ; accumulator of the mix
ld r1, [0301]            ; phase of voice 0
ld r0, [a030]            ; pitch of voice 0 (ARM)
mov r4, r4
add r0, #-4.75
mul r2, r0, #0.04332169878499658
mov r3, #0.041666666666666664:32
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, r3
mul r3, r3, #0.009166666666666667
addxy r1, r3, r1
addxy r2, r1, #-1.0
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
st r1, [0300]
ld r4, [a038]            ; gate of voice 0 (ARM)
mov r4, r4
mul r1, r1, r4
addxy r7, r7, r1         ; to the mix; after the 4 voices: r7 x 0.25 -> st [a150]
```

### 5.2 C-S1JX: the JX-3P split per chip [C], [E]

Each chip runs one two-voice chain of the JX-3P (local voices 0/1 of the SYSTEM-8 on the master, 2/3 on the slave);
the split, the memory remapping and the tests are in
[examples/system_1_jx/README.md](../../examples/system_1_jx/README.md) §3–§6. Since the master also runs voices,
**both regions write bank 5**: each one evaluates the complete model and writes only the words of its chip:

```c
/* examples/system_1_jx/jx.h: S8 DSP d even = master (JX_CHIP 0), odd = slave (JX_CHIP 1) */
static void dsp_bank(int dsp, int id, float v)
{
    if ((dsp & 1) != JX_CHIP || id < 0x20 || id >= 0x50)
        return;
    uint16_t w = b5_index[dsp][id - 0x20];          /* table generated by s1jx.py */
    if (w != 0xFFFF)
        b5_write(w, fbits(v));                      /* FD6C(w >> 8, {0, 0, w & 0xFF}, bits) */
}
```

---

## 6. Number of voices

| | SYSTEM-1 |
|---|---|
| the OS delivers more than 4 voices to a PLUG-OUT assigner | **yes, 8**: C-S1V8 sounds with 8 held notes and the slave receives `0x114`–`0x117` / `0x124`–`0x127` [M] |
| more than 8 | the block of velocities ends at `0x127` (`0x128` starts the trigger block); the notes have room for 16. Another way of sending the gate would be needed [A] |
| DSP time | slave A: 703 instructions per sample ran, 826 hung; slave B: 708 ran [M] ([Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §1; C-S1SY, [examples/system_1_synth/README.md](../../examples/system_1_synth/README.md) §4); C-S1V8 uses 269 on slave A |
| voices of the JX-3P with its microcode | 4 complete ones do not fit (3748 instructions); 4 with one DCO (3098) do not fit the measured budget (4 × 703 − 40 = 2772) [E] ([Vol. III ch. 9](system1_polyphony_studies.md) §3) |
| voices of SH-101 / SH-2 / PROMARS with their microcode | 2 at most [E] ([Vol. III ch. 9](system1_polyphony_studies.md) §3.4) |

---

## 7. Open

| # | what |
|---|---|
| 1 | what each bank-1 word per voice is and each of the 20 modulation words `600E`–`6021` |
| 2 | on which DSP of the master the stock modulation and mixer run |
| 3 | more than 8 voices (another gate) |
| 4 | per-voice measurement (like `voicelog` of the SYSTEM-8): the parameter capture (`examples/system_1/paramlog.py`) is the basis |

## 8. Evidence

| datum | source |
|---|---|
| voice ids, stock assigner (`0x604A26B0`), "4 voices" sites | [Vol. III ch. 9](system1_polyphony_studies.md) §1.3–§1.4; [s1_voice_allocation_log.md](../vol7_cleanroom_evidence_archive/historical_lab_logs/s1_voice_allocation_log.md) §2 |
| C-S1V4 and C-S1V8 measured | [examples/system_1/README.md](../../examples/system_1/README.md) §4, §7 |
| sections, indices of `FC44` | [Vol. III ch. 9](system1_polyphony_studies.md) §2.2; [ch. 3](slot_program_model.md) §3 |
| costs of Roland's PLUG-OUTs | [Vol. III ch. 9](system1_polyphony_studies.md) §3.4 |
