# Volume VI — Chapter 1: Own PLUG-OUT from Scratch (C-V4, SYSTEM-8): How It Is Made and How to Rebuild It

> Paths are relative to `examples/from_scratch/` unless stated otherwise.

State: 2026-10-22. **C-V4 works on the device**: eight voices (four per slave) with their note, single and in chords,
in tune within ±3 cents ([S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6.7). All the code and all the DSP microcode are ours (the rule
of not using Roland's microcode applies to our plug-outs; the emulator and the study may run it from the local copy,
[S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §7). The signature check (`emu/sigcheck.py`) finds no Roland bytes in the build.

This document replaces `examples/from_scratch/MINI_PLUGOUT.md` (diary of 2026-10-21; what is still valid from it is in
§11). Tags: **[measured]** = on the device; **[code]** = it is in our code; **[listing]** = read from Roland's
disassembled microcode (study only); **[inferred]** = unmeasured deduction.

---

## 1. What has to be built

A SYSTEM-8 PLUG-OUT is a file `Code8_<slot>.Dat` (one per slot, 1–3) with **two regions of ARM code**:

| region | runs on | what it does | our code |
|---|---|---|---|
| 0 ("model") | master BMC | SYNINFO, list of panel parameters, patch memory, **voice assigner** (which note goes to which voice) | `model/` |
| 1 ("engine") | the two slave BMCs (slave 0: global voices 0–3, slave 1: global voices 4–7; §1.1) | loads the programs of the slave's six DSPs and translates notes and parameters into writes to the DSP | `v3/engine_v3.c` |

And, inside region 1, **the DSP microcode**: one main program per DSP (six) and the slot programs (oscillators
DCO1/DCO2 in DSP2 and DSP3; effect in DSP4). It is what generates the sound.

```
 PC ──USB-MIDI──► master (region 0: assigner) ──parameters 0x429+v (note), 0x43A+v (velocity)──►
      slave 0 / slave 1 (region 1: engine_v3.c) ──bank 5 of each DSP──► DSP0..DSP5 ──► audio
```

C-V4 uses the own region 0 of `model/` (that of C-SAW), the engine `v3/engine_v3.c` unchanged and the six main programs
of `v4/`. The slot programs are those of v3: our saw in DCO2 and our null program in DCO1.

### 1.1 Global voices and local voices

There are not two kinds of voice: it is **the same voice counted two ways**, depending on who looks at it.

* **Global voice (0–7)**: the number the assigner of region 0 uses, on the master, for the whole instrument. It is the
  one that goes in the parameter id: note `0x429 + v`, velocity `0x43A + v` [code, `model/model.c`]. It is also the
  "voice" that `voicelog.py` shows.
* **Local voice (0–3)**: the same voice seen from the slave that plays it. Each slave has four: slave 0 plays the global
  ones 0–3 and slave 1 the 4–7. The engine computes it as `lv = v − 4·slave` (`e->first = 4 * e->slave` in
  `engine_v3.c`) [code]. Both slaves run the same code and the same DSP programs, so inside a slave only the local voices
  0–3 exist.

Inside the slave, the local voice decides the DSPs and the addresses: `u = lv >> 1` is the **pair** (0: voices 0 and 1;
1: voices 2 and 3) and `k = lv & 1` is **voice A or B** inside each DSP (each oscillator DSP carries two voices).

| global voice | slave | local voice `lv` | pair `u` | A/B `k` | pitch at | oscillator in | gate at |
|---|---|---|---|---|---|---|---|
| 0 | 0 | 0 | 0 | A | DSP0 `A021` | DSP2 voice A | DSP4 `A02A` |
| 1 | 0 | 1 | 0 | B | DSP0 `A033` | DSP2 voice B | DSP4 `A039` |
| 2 | 0 | 2 | 1 | A | DSP1 `A021` | DSP3 voice A | DSP5 `A029` |
| 3 | 0 | 3 | 1 | B | DSP1 `A033` | DSP3 voice B | DSP5 `A038` |
| 4 | 1 | 0 | 0 | A | DSP0 `A021` of slave 1 | DSP2 voice A | DSP4 `A02A` |
| 5 | 1 | 1 | 0 | B | DSP0 `A033` | DSP2 voice B | DSP4 `A039` |
| 6 | 1 | 2 | 1 | A | DSP1 `A021` | DSP3 voice A | DSP5 `A029` |
| 7 | 1 | 3 | 1 | B | DSP1 `A033` | DSP3 voice B | DSP5 `A038` |

Example: global voice 6 is local voice 2 of slave 1. Its pitch is written by slave 1's engine into the `A021` of its
DSP1, the saw comes out of voice A of its DSP3 and the gate is at the `A029` of its DSP5. The assigner decides which
global voice plays each note; in split mode, the upper half uses voices 4–7 [code, `model.c` `vid`].

In the rest of this document and in [Vol. III ch. 1](../vol3_voice_allocation_scheduling/system8_voice_allocation.md), "voice 0…3" alone means local voice, except when talking
about `voicelog` or slave 1, where "global voice" is said or the numbers 4–7 are given.

## 2. The flow of a voice in C-V4

Each slave has six DSPs. Local voices 0 and 1 live in DSP0 → DSP2 → DSP4; 2 and 3 in DSP1 → DSP3 → DSP5 → DSP4. It is
the same layout that Roland uses in JX-3P, JUNO-60, JUNO-106, JUPITER-4 and JUPITER-8 ([Vol. III ch. 1](../vol3_voice_allocation_scheduling/system8_voice_allocation.md) §3; the reference "[Vol. III ch. 1](../vol3_voice_allocation_scheduling/system8_voice_allocation.md) §6.2" of the original points to the version-1 text, now the [S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6.2).

```
ARM: pitch → bank 5 of DSP0 (voice 0: A021, voice 1: A033)   and of DSP1 (voice 2: A021, voice 3: A033)
ARM: gate  → bank 5 of DSP4 (voice 0: A02A, voice 1: A039)   and of DSP5 (voice 2: A029, voice 3: A038)

DSP0 v4m0: A021 → Z 404, A033 → Z 405 ─MXM─► DSP2 M 004 / 005
DSP1 v4m1: A021 → Z 400, A033 → Z 401 ─MXM─► DSP3 O 200 / 201
DSP2 v4m2: M 004 → [2207], M 005 → [2214]  ─► DCO2 slot (saw): [2207] → [2221], [2214] → [2229]
           [2222] → Z 404, [222A] → Z 405 (the outputs, one sample later) ─MXM─► DSP4 M 004 / 005
DSP3 v4m3: the same with O 200/201 → Z 400 / 401 ─MXM─► DSP5 O 200 / 201
DSP5 v4m5: O 200 × gate(A029) → A400,  O 201 × gate(A038) → A401  (mailbox of the pair DSP4/DSP5)
DSP4 v4m4: M 004 × gate(A02A) + M 005 × gate(A039) + A400 + A401 + IOMD 6000..6003 → 6020, 6021 (audio)
           and the four single voices → 6060..6063 (the ones the other slave adds through 6000..6003)
```

The hardware rules that make this work, all checked on the device with C-V4 or before:

| rule | consequence in the code | evidence |
|---|---|---|
| **MXM** (bank 4, `8nnn`): what a DSP stores in Z `4nn` is read by the DSP of the next pair at M `nn` (if the writer is even) or O `nn` (if it is odd), one sample later; if nobody writes, 0 is read | write **every sample**; read with the **same** word number | [measured] master; C-V4 |
| **Bank 1** (`2nnn`) and bank 0 are **rings**: what is stored at R in sample n is read at R+1 in n+1 | the main program writes the oscillator's input (`[2207]`) and the slot reads it in the same sample; the slot writes the output (`[2221]`) and the main program reads it **at `[2222]`** in the next sample | [measured] with C-V4 |
| The **main program runs before** the slots of its DSP | that is why the input is read at the same address | [inferred] from the ring + C-SAW in tune |
| **Bank 5 `A400`–`A7FF`** is a mailbox shared by the pair (0,1), (2,3), (4,5) | DSP5 leaves its voices in `A400`/`A401`, DSP4 adds them | [measured] with C-V4 |
| Only **`6020`/`6021`** (IOMD of DSP4) reach the audio; `60B0`–`60B7` do not | the final mix goes to `6020` and `6021` | [measured] [S8 wave-bus log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md) §4b |
| **Slave 1** does not reach the audio through its own `6020`; it reaches slave 0 through IOMD `6000`–`6003` | DSP4 adds `6000`–`6003` and writes `6060`–`6063` | [measured] with C-V4 (which exact word: open) |

The mistakes of v3 were exactly three of these rules: it read M 005/006 in DSP4 (invented offset), sent voices 2 and 3
to `60B0` and did not add the other slave ([S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6.3).

## 3. The microcode: `.s` and `.hex` files and why there is an assembler inside the `.py`

### 3.1 What each format is

| extension | what it contains | who generates it | who uses it |
|---|---|---|---|
| `.s` | source code in the assembly language of `asm/` (`ld r1, [8004]`, `mul r1, r1, r4`, `st r1, [6020]`). Readable, for review | `make_v3.py` / `make_v4.py` | people; `asm/asm.py` can assemble it again |
| `.hex` | the assembled program: **one 16-bit word per line**, in execution order, with a `#` comment line | `make_v3.py` / `make_v4.py` (they call `asm/asm.py`) | `engine/programs.py` (puts it into `programs.h`), the emulator, `sigcheck.py` |
| `.txt` (in `re/dspexp/own/`) | the same as `.hex`, several words per line | `re/dspasm.py` (the saw) | `programs.py`, `make_v3.saw_dco2_words()` |

Each program starts with `0a02 ff70` (start mark; it does nothing [measured], Roland puts it the same way) and ends with
`0b02 00f8 0001` (the end sequence that all Roland programs use). `make_v3.words()` adds them.

Complete example, `v4/v4m0.s` (DSP0):

```
; v4m0: voices 0 and 1 pitch -> MXM Z 404, 405 (DSP2 M 004, 005)
ld r1, [a021]
mov r4, r4
st r1, [8404]
ld r1, [a033]
mov r4, r4
st r1, [8405]
```

and its `v4/v4m0.hex` (here on one line; in the file, one word per line), with each instruction annotated:

```
0a02 ff70                start
0c03 1fe0 a021           ld r1, [a021]     (header 0c03 = class and length, 1fe0 = load to r1 from the reference, a021)
0902 4044                mov r4, r4
0c03 9fe0 8404           st r1, [8404]     (9fe0 = store of r1)
0c03 1fe0 a033           ld r1, [a033]
0902 4044                mov r4, r4
0c03 9fe0 8405           st r1, [8405]
0b02 00f8 0001           end
```

The format of the instructions is in [Vol. II ch. 4](../vol2_dsp_core_isa/instruction_set_reference.md).

The `mov r4, r4` between the load and its use is a margin instruction: a load takes one instruction to be available to
the operations [measured, [Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §7.3] (see the note in that section: later measurements show the next
instruction already sees the loaded value; the filler is the safe convention). Other rules the programs respect: at most
one load and one store to bank 5 per instruction; up to 6 transfers per instruction.

In the build, `engine/programs.py` packs each `.hex` into a C array `mc_<NAME>[]` of 32-bit words (upper half first, as
in Roland's image) plus `MC_<NAME>_BYTES`, inside `engine/build/programs.h`.

### 3.2 The names `v4m0` … `v4m5`: what they mean and where each runs

The name has three parts: **`v4`** = version of the plug-out (v3, v4…), **`m`** = main program ("main", the one that runs
in slot `0x0100` of a DSP every sample), **the digit** = **the DSP where it runs**, 0 to 5. So `v4m2` is the main
program of version 4 that runs on DSP2. It is the same numbering as Roland's main programs (`MAIN0` … `MAIN5`, DSP0 …
DSP5).

| program | DSP | local voices (§1.1) | what it does |
|---|---|---|---|
| `v4m0` | DSP0 | 0, 1 | takes from bank 5 the pitch of voices 0 and 1 (`A021`, `A033`) and sends it to DSP2 by MXM (Z `404`/`405`) |
| `v4m1` | DSP1 | 2, 3 | the same for voices 2 and 3, towards DSP3 (Z `400`/`401`) |
| `v4m2` | DSP2 | 0, 1 | passes the two pitches to the inputs of the DCO2 slot (`[2207]`, `[2214]`), reads the two saws (`[2222]`, `[222A]`) and sends them to DSP4 |
| `v4m3` | DSP3 | 2, 3 | the same for voices 2 and 3, towards DSP5 |
| `v4m4` | DSP4 | 0, 1 (+ 2, 3 and the other slave) | applies the gate to voices 0 and 1, adds voices 2 and 3 arriving from DSP5 through `A400`/`A401` and the other slave through IOMD `6000`–`6003`, and writes the output `6020`/`6021` and the single voices `6060`–`6063` |
| `v4m5` | DSP5 | 2, 3 | applies the gate to voices 2 and 3 and leaves them in the mailbox `A400`/`A401` for DSP4 |

Things to keep in mind:

* **The digit does decide where it runs**, but not because of the file name: it decides the **array index** in the
  engine. `e_load` registers `main_prog[d]` on DSP `d` (OS service `+0x40 + 4·d`, slot `0x0100`) [code, `engine_v3.c`].
  That index `d` is DSP `d` is measured with C-SAWPOLY and with C-V4 (with another order it would not sound).
* **Both slaves run the same six programs.** `v4m0` runs on DSP0 of slave 0 (global voices 0 and 1) and on DSP0 of slave
  1 (voices 4 and 5). What changes between slaves is which notes the engine sends them (`lv = voice − 4·slave`).
* **In the build they are loaded with the names of v3.** `engine_v3.c` asks for `V3M0` … `V3M5` (arrays `mc_V3M0` …).
  `build_v4.sh` makes those names contain the v4 ones (`OWN_PROGS=v3m0=v4/v4m0.hex,…`), so `v4m2.hex` ends up in
  `mc_V3M2`, which is `main_prog[2]`, which runs on DSP2.
* **Slot programs**, which do not follow this naming rule: `v3saw` (saw, DCO2 slot `0xB000` of DSP2 and DSP3) and
  `dco2_null` (null program, DCO1 slot `0x6000` of DSP2 and DSP3; the name comes from it having been written for DCO2).
  In DSP0/DSP1 (slot `0x8000`) and DSP4/DSP5 (slots `0x6000` and effect `0xB000`) go the engine's own empty programs
  (`mc_empty`).
* `v4bm0` … `v4bm5` (only with `make_v4.py --no-ring`): the variant C-V4B. Same rule: the digit is the DSP.

### 3.3 Why the assembler is inside the `.py`

`make_v3.py` and `make_v4.py` do not contain fixed programs: they are **generators**. Each main program is written as a
Python function that receives addresses and returns assembly lines. For example `prog_pitch(name, desc, out0, out1)`
produces the program of DSP0 **and** that of DSP1 changing only the destination Z word; `prog_dco_pair` does DSP2 and
DSP3. Then `words()` calls `asm.Assembler().assemble(text)` (the assembler of `asm/`, in the same process) and writes
the `.s` and `.hex`.

The reasons:

1. **A single source for the addresses.** The pitch and gate ids (`PITCH_A`, `GATE`), the oscillator's inputs and
   outputs (`DCO_IN`, `DCO_READ`), the mailbox (`PAIR`) and the IOMD words are Python constants. The programs, the
   emulator (`emu/`) and the tests (`emu/test_chain.py`) use them at once. If an address changes, it changes in all of
   them.
2. **Almost equal programs without copy and paste.** The pairs DSP0/DSP1, DSP2/DSP3 and DSP4/DSP5 differ in two or three
   addresses. `make_v4.py` only redefines the functions that change with respect to v3 and reuses the rest
   (`v3.prog_pitch`, `v3.words`).
3. **Variants by option.** `make_v4.py --no-ring` generates v4b (it reads `[2221]`/`[2229]` instead of
   `[2222]`/`[222A]`) with the same function and another parameter.
4. **The emulator assembles in memory.** `emu/voices.py` calls `make_v4.sources()` and assembles without going through
   files, so a test can modify a program (e.g. `roland_layout_dsp2()` in `test_chain.py`).

The `.s` are written to read and review them. **They are not edited by hand**: they are regenerated with
`python3 make_v4.py`, and the build does it by itself. If some day a program grows and stops being parametric, it can
live as a `.s` and be assembled with `asm/asm.py` directly; the resulting `.hex` enters the build the same way.

The saw (`re/dspexp/own/saw_dco2.txt` = `v3/v3saw.hex`) is another generator: `re/dspasm.py saw dco2` computes its
constants (phase increment for the sampling frequency of 96 kHz) and writes the words.

## 4. The ARM engine (region 1): `v3/engine_v3.c`

It compiles for Cortex-M4 (`arm-none-eabi-gcc`), without a standard library. It talks to the slave's OS only through its
service table at `0x0113FC00` [code]:

| service | use |
|---|---|
| `+0x00` `get(part, grp, id)` | read a parameter (e.g. the note `0x429+v`) |
| `+0x28/+0x40/+0x58 + 4·dsp` | register slots, programs and data blocks of each DSP (in `e_load`) |
| `+0x70` `swap` | change the program of a slot and activate its task (DCO1 = task 2, DCO2 = 3) |
| `+0xA0` `p_ramp` | write a parameter with a ramp (bank 6) |
| `+0xB0` `p_bank(dsp, &id, v)` | write the word `A000 + id` of a DSP's bank 5: **this is how pitch and gate arrive** |
| `+0xC0` `sync` | wait for the DSP's frame boundary before a change |
| `+0xF0` | index of the slave (0 or 1) |

Pieces of the file:

* **Descriptors** `dspdesc_t {addr, n1, n2, src, os[2]}`: address of the slot (`0x0100` main, `0x6000` DCO1, `0xB000`
  DCO2/effect, `0x8000` in DSP0/1), size twice and pointer to the array of `programs.h`. The last two fields are used by
  the OS; a descriptor cannot be shared between DSPs.
* `main_prog[6]` = `V3M0`…`V3M5` (in C-V4 those names contain the v4 ones: `build_v4.sh` maps them), `dco1_prog` =
  `DCO2NULL`, `dco2_prog` = `V3SAW`, `fx_prog` = empty ones.
* **`e_load`** (vtable `+0x08`): registers on each DSP its slots, programs and data.
* **`e_start`** (`+0x0C`): sends the state of the parameters, activates the slots with `swap` and sets the gates to 0.
* **`e_param`** (`+0x30`): `0x429 + voice` = note → `voice_pitch` (bank 5 of DSP `lv>>1`, id `0x21 + 0x12·(lv&1)`, value
  `(note − 12)/12`, so 4.75 = A 440); `0x43A + voice` = velocity → `voice_gate` (bank 5 of DSP `4 + (lv>>1)`, id
  `0x2A + 0xF·(lv&1) − (lv>>1)`, 1.0 or 0.0). `lv` = voice minus `4·slave`.
* The other parameters go through the table `model.h`, which `engine/gen_model.py` generates from `engine/model_spec.py`
  (the reproduced parameter model of the JX-3P). The main programs of v4 do not read those parameters; it stays for when
  there are filter, envelope and LFO.
* **Exports**: `plugout_init` (returns the creation descriptor) and the structure `syninfo` in the section `.syninfo`,
  which the OS looks for when loading the region.

## 5. Region 0: `model/`

Documented in [examples/from_scratch/model/README.md](../../examples/from_scratch/model/README.md). The essentials:

* `model.c`: SYNINFO, descriptor, create/destroy/select and the **assigner** (17 vtable entries). It sends the slaves
  the note (`0x429 + voice`) and the velocity (`0x43A + voice`, 0 = release).
* `params.py`: the list of panel parameters (ids of the JX-3P, because the engine uses them) and the memory of 64 INIT
  patches inside the region itself (that is why region 0 cannot be small).
* `VOICELOG=1` compiles a diagnostic log: each assigned note and velocity goes out by SysEx
  `F0 7D 05 <type> <voice> <value> <note> F7` on the device's MIDI 1. `voicelog/voicelog.py` crosses it with the audio.
  The note is only sent if it changes (`retune`), so a voice that repeats a note sends no note message.

## 6. Build step by step

```sh
cd examples/from_scratch
VOICELOG=1 NAME=C-V4 sh v4/build_v4.sh        # -> plugins/C-V4/Script/Code8_{1,2,3}.Dat
python3 emu/sigcheck.py engine/build/engine_0x60A00000.bin engine/build/engine_0x60C00000.bin
```

Requirements: `arm-none-eabi-gcc/ld/objcopy`, Python 3 and Roland's PLUG-OUT `plugins/JX-3P` on disk (see §10, a
dependency that remains).

What each script does, in order:

1. **`v4/build_v4.sh`**: `python3 make_v4.py` (writes `v4m0..5.s/.hex`); sets `NAME` (C-V4, or C-V4B with `V4B=1`) and
   `OWN_PROGS=v3m0=v4/v4m0.hex,…,v3saw=v3/v3saw.hex,dco2null=gate/dco2_null.hex`; calls `v3/build_v3.sh`.
2. **`v3/build_v3.sh`**: regenerates the v3 ones (not used in C-V4) and creates `v3/build_engine.sh`, a copy of
   `engine/build.sh` that compiles `v3/engine_v3.c` instead of `engine/engine.c` (rule: `engine/` is not modified).
3. **`v3/build_engine.sh`** (copy of `engine/build.sh`):
   1. `model/build.sh` compiles region 0 (with `-DVOICELOG` if `VOICELOG=1`) at two addresses (slot 1 and 2) and
      deduces the relocations by comparing them.
   2. `engine/programs.py plugins/JX-3P engine/build` writes `programs.h` with the programs of `OWN_PROGS`.
   3. `engine/gen_model.py` writes `model.h`.
   4. `arm-none-eabi-gcc` compiles the engine; `ld` links it at `0x60A00000` and `0x60C00000`; the words that differ are
      the relocations.
   5. It assembles the project (`engine/build/projC/`: `plugout.json`, `region0.bin`, `region1.bin`, `relocs.json`) and
      `tools/plugout_build.py set-id` / `plugin` turns it into `plugins/<NAME>/Script/Code8_{1,2,3}.Dat`.
4. **`emu/sigcheck.py`**: confirms that the build contains no Roland bytes (exits with 1 if it finds any).

## 7. Install and measure

```sh
python3 tools/plugout_tool.py install plugins/C-V4 --slot 3 --force --no-patches -y
# on the panel: select PLG3 without playing notes
cd examples/from_scratch/voicelog
python3 voicelog.py                                   # 8 single notes
python3 voicelog.py --groups 48,52,55,60 64,67,72,76  # two chords
```

Rules: one installation at a time and, after each one, select PLG3 again; never two tools that talk to the device
(MIDI/ALSA) at once. `voicelog.py` records the audio with `arecord -D hw:SYSTEM8,0` at 96 kHz.

## 8. Verifying without the device: the emulator

`emu/` (detail in [S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6):

```sh
cd examples/from_scratch/emu
python3 voices.py v4            # each voice alone: level and frequency at 6020
python3 test_chain.py           # 20 tests (A–H and regression), about 20 minutes
python3 test_chain.py A D       # only some
```

`cchain.py` (C core; before, `chain.py` on `re/dspisa.py`, archived on 2026-10-29) runs our programs and models the
banks, MXM, the pair mailbox, IOMD and the link between slaves. Each hardware rule is a field of `Model`, so a test can
compare the measured model with the alternative. **Before installing a microcode change, it has to go through the
emulator**: a program the emulator gives silent will almost certainly be silent on the device.

> Note: the "about 20 minutes" of `test_chain.py` is the time before the C core; on the C core it runs 20/20 in ≈ 5 s
> ([Vol. V ch. 5](../vol5_toolchain_compiler_sdk/emulator.md) §4, §5.1).

## 9. Map of files

**What makes up C-V4**

| file | what it is |
|---|---|
| `v4/make_v4.py` | generator of the six main programs of C-V4 (and of v4b with `--no-ring`) |
| `v4/v4m0.s` … `v4m5.s` | generated assembly source of each main program (DSP0 … DSP5) |
| `v4/v4m0.hex` … `v4m5.hex` | the same programs assembled (they enter the build) |
| `v4/build_v4.sh` | build of C-V4 / C-V4B |
| `v3/engine_v3.c` | the ARM engine (region 1) |
| `v3/v3saw.hex` | the saw of the DCO2 slot (copy of `re/dspexp/own/saw_dco2.txt`, generated by `re/dspasm.py`) |
| `gate/dco2_null.hex` | own null program for the DCO1 slot (outputs nothing; the slot cannot stay empty) |
| `model/` | region 0 (`model.c`, `params.py`, `storage.S`, `link.ld`, `build.sh`) |
| `engine/programs.py`, `engine/gen_model.py`, `engine/model_spec.py`, `engine/expr.py`, `engine/link.ld`, `engine/build.sh` | shared build pieces (not modified) |

**v3 (previous version, measured: only voice 0 sounded)**

| file | what it is |
|---|---|
| `v3/make_v3.py` | generator of v3; also the functions and constants that v4 reuses |
| `v3/v3m0..5.s/.hex` | the main programs of v3 (kept: the regression checks that they do not change) |
| `v3/build_v3.sh` | build of C-V3 / base of the build of v4 |
| `v3/build_engine.sh` | **generated** by `build_v3.sh` (copy of `engine/build.sh` pointing to `engine_v3.c`) |

**Tools**

| file | what it is |
|---|---|
| `emu/chain.py`, `emu/voices.py`, `emu/test_chain.py` | emulator of the slaves' DSPs, programs and engine, tests |
| `emu/listing.py`, `emu/allplugouts.py` | reading of the bus references of Roland's programs (study, nothing is copied) |
| `emu/sigcheck.py` | signature check: no Roland bytes |
| `voicelog/voicelog.py` | measurement on the device: voice assigned to each note and whether it is heard, with its frequency |
| `emu/dspref.py` (C: `emu/dspdec.h`, `emu/dspcore.h`), `re/dspdis.py`, `asm/asm.py`, `asm/disasm.py` | simulator (the reference), slicer, assembler and disassembler of the DSP |

**Generated by the build (not edited, not in the repository)**: `engine/build/` (`programs.h`, `model.h`, ELF/BIN,
`projC/`), `model/build/`, `plugins/C-V4/`.

## 10. What is missing and known debts

* **Sound**: today each voice is a saw with a gate (no envelope, filter or LFO). The next natural step is an envelope in
  DSP4/DSP5 (the gate is already there) and then filter and LFO. The panel's parameters are still those of the JX-3P and
  almost none does anything.
* **Build dependency**: `programs.py` still copies the 17 Roland programs to `programs.h` (it checks their SHA-256 in
  `plugins/JX-3P`), although `engine_v3.c` does not use them and the compiler discards them (`sigcheck.py` confirms it).
  For a fully independent build, `programs.py` should be able to work only with `OWN_PROGS`. It has not been changed
  because `engine/` is not touched without permission.
* **Link between slaves**: C-V4 adds `6000`–`6003` and writes `6060`–`6063`; which of the two the hardware uses, and
  which word goes to which input, is not known.
* **Mono**: the tuning in mono mode has not been measured with C-V4 (with C-V3L it gave ±40 cents).
* **Level**: the mix adds up to eight saws of amplitude 1 without scaling; with big chords it can saturate.
* **More than 8 voices**: there is no measured hardware limit; it depends on whether the OS delivers more voice ids.
  Analysis and proposed test in [S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §8.

* **Port to the SYSTEM-1**: C-S1V4 (`examples/system_1/`, [examples/system_1/README.md](../../examples/system_1/README.md)) works on the device (four
  voices, volume). It reuses the saw and the assigner of C-V4.
* **Mapping parameters**: on the SYSTEM-1, which id each control sends was captured by forwarding to the PC each
  parameter that region 0 receives ([examples/system_1/README.md](../../examples/system_1/README.md) §6). On the SYSTEM-8 region 0 does not receive those
  calls; the proposed path is to compare values in the assigner's `changed(group)` and send the differences with the
  `VOICELOG` sender (§6.3 of that document). Not done.

## 11. History (from `MINI_PLUGOUT.md`, 2026-10-21)

1. **Own saw in `asm/`** (`dsp/saw_dco2.s`): bit-identical to that of `re/dspasm.py` in simulation (`dsp/check_saw.py`;
   a constant 1/24 in 16 bits gave 1.45·10⁻⁶ of error, with `#…:32` 0).
2. **Output test** (`dsp/make_iomd_test.py`, `engine.c -DIOMDTEST`, plugin C-IOMD, without notes): three fixed saws from
   the DCO1 slot of DSP5 [measured]:

   | word | result |
   |---|---|
   | `6020` | comes out: 880 Hz, harmonics 1/n, right channel of the capture |
   | `60B0` | does not come out (440 Hz absent in both channels) |
   | `60B1` | does not come out (660 Hz absent) |

   The complete sweep of IOMD words is in [S8 wave-bus log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_wave_bus_log.md) §4b.
3. C-MINI / C-MINI2 / C-SUB3: monophonic voice with Roland's main programs; only slave 0 reached the audio.
4. C-SAWPOLY (2026-10-22): Roland's chain with our saws: eight voices, slave 1 does arrive [measured]. It contained
   Roland microcode, only for measuring.
5. C-V3 / C-V3L: own main programs; only voice 0 sounded. The emulator explained why ([S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6.3).
6. **C-V4**: the three fixes; eight voices [measured].

Decisions of then that still stand: gate and pitch through bank 5 with the ids of the JX-3P (the assigner of region 0
uses them); removing panel options = removing `P(...)` records from `model/params.py` (not tested on the device).
