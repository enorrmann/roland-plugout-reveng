# Volume III — Chapter 9: SYSTEM-1 Polyphony Studies — Six Voices, a Polyphonic SH-101, SYSTEM-8 Microcode on the SYSTEM-1

> **Scope:** SYSTEM-1 only. Static analysis of OS 1.30 ([Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md) §9)
> and of Roland's SYSTEM-1 PLUG-OUTs, plus emulator measurements; nothing of §1–§2 was patched or installed. More voices
> through an own PLUG-OUT work on the device: **C-S1V8 plays 8 voices** ([Vol. VI ch. 2](../vol6_reference_designs_applications/c_s1v8_system1_reference.md));
> the voice model is [ch. 2](system1_voice_allocation.md).

Tags: **[measured]** on the device · **[emulator]** measured in `emu/cchain.py` · **[code]** seen in the binary ·
**[estimated]** computed, not measured · **[inferred]** · **[open]**.

The DSP time budget used below is the one measured on the SYSTEM-1: slave DSP A **703** instructions per sample ran,
826 hung; slave DSP B 708 ran ([Vol. I ch. 1](../vol1_system_architecture/soc_overview.md) §6.2). The figures marked
"budget 925" were computed with the SYSTEM-8 value and are given with that label.

## 1. Six voices for the stock synth (patching the OS)

Goal (request of 2026-10-22): a patched SYSTEM-1 with 6 voices of polyphony instead of 4. This section gathers what
is in the 1.30 images and what is missing.

### 1.1 Summary

| Layer | What fixes 4 voices | Do 6 fit? |
|---|---|---|
| Master: assigner (`0x604A26B0`) | loops `cmp #4`, indices `& 3`, 4-entry tables | yes, an ARM patch (about 30 sites) [code] |
| Master → slave: parameter ids | `0x110+v`, `0x120+v`, `0x128+v` (group 2) | yes: the blocks have 16, 8 and 8 ids [code]; `0x114`–`0x117` and `0x124`–`0x127` reach the slave [measured, C-S1V8] |
| Slave: DSP voice programs | 4 voice sections per DSP | time: ≈ 740 instructions in the worst case [estimated], between the 703 that ran and the 826 that hung on slave DSP A: [open]; space and data: [open] |
| Slave: ARM engine | writes pitch, gate, etc. of 4 voices | [open] |
| Install | the OS does not go into slot 1 ([Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md) §9.2) | either flash the patched OS or port it to a PLUG-OUT (§1.5) |

### 1.2 Slave: voice programs and budget

`python3 system-1/voice_sets.py` lists the "sets" the slave loads. A set is a series of 12-byte entries
`{program address (bytes), length (bytes), pointer}` [code]:

| Address | Contents |
|---|---|
| `0x0000` | header (data `0000 0001`, then code) |
| `0x0100` | main part (empty in the voice sets) |
| `0x1900` | voice 0 |
| `0x3100` | voice 1 |
| `0x4900` | voice 2 |
| `0x6100` | voice 3 |
| `0x7FF2` | closing of 7 half-words |

The voice sections are `0x1800` bytes apart. The slave service `FC40` places each voice program by an index 0–7 →
(A/B, section address, section 2–5) (§2.2): two DSPs (A/B) with 4 voice sections each [code]. There are 50 sets: 2 of
start-up (empty voices of 4 instructions) and 48 of voice, in identical pairs (one per DSP), 24 variants (probably
waveform combinations). Each voice program has an `ef75`/`ef74` section. The four programs of a set differ only in
the data addresses of each voice [code]:

| Voice | Bank-1 words | Other fields |
|---|---|---|
| 0 | `2004`, `2006` | `003c`, `0019` |
| 1 | `200e`, `2010` | `003e`, `001a` |
| 2 | `2014`, `2016` | `005a`, `001c` |
| 3 | `201e`, `2020` | `005c`, `001d` |

**Cost** (1 instruction = 1 cycle):

| | instructions per sample |
|---|---|
| worst set, 4 voices | 494 (voices 123/121/123/121 + 6) |
| worst set, 6 voices (4 + 2 copies of the longest voice) | **≈ 740** [estimated] |
| lightest set, 4 → 6 voices | 302 → 454 |

The main programs of the slave's DSPs (241 and 237 instructions, [Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md)
§15.2) run in the same DSPs as the sections and add to their time; whether they iterate over 4 voices and have to be
extended is [open].

**Space** [open]: 6 sections with the present step (`0x100 + 6 × 0x1800 = 0x9100`) do not fit before `0x7FF2`. The
longest voice program takes `0x44A` bytes, so with a step of `0x1000` they would fit. How control passes from one
section to the next (padding between sections, absolute jumps) and whether the ARM driver imposes the step are not
known.

**Data** [open]: voices 4 and 5 need their own words in bank 1 and in the other fields without overwriting anything.
The present spacing (4, 0xA, 6, 0xA) is not regular: what occupies each gap has to be mapped.

### 1.3 Master → slave

The assigner sends the data of each voice with `set(obj, 2, id, value)` (`0x600094FC` → `0x60008BD4`), choosing the
voices with a one-byte bit mask (bits 0–3) [code]:

| Ids | Use (inferred from the value) | Block gap |
|---|---|---|
| `0x110 + v` | note / pitch | `0x110`–`0x11F` |
| `0x120 + v` | velocity / level; 0 when the sustain is released | `0x120`–`0x127` |
| `0x128 + v` | trigger (1/0) | `0x128`–`0x12F` |
| `0x130` | pitch bend | — |

Voices 4 and 5 would use `0x114/0x115`, `0x124/0x125` and `0x12C/0x12D`. **The slave receives `0x114`–`0x117` and
`0x124`–`0x127`** [measured with C-S1V8, 2026-10-22]; the trigger ids `0x12C+` were not tested.

### 1.4 Master: assigner

Voice assigner at `0x604A26B0`, code at `0x60009500`–`0x6000A400` (1.30). Initialisation at `0x600096E4`: `+0x08` =
note of each voice (4 entries), `+0x218` = another per-voice table (4 entries), `+0x18` = 128 entries per key. Modes
(`obj[+4]`, chosen by parameter `0x192` of group 3): `0x60009608`, `0x600096B8` and `0x600096B6`. Notes:
`0x60009732`, `0x6000994C`, `0x6000A094`, `0x60009D60` and `0x60009E8A` [code].

`python3 system-1/asgn_sites.py` lists 58 candidate instructions (`#4`, `#3`, `0x120`). Not all are about voices: the
`movs r0, #3` after `0x6000A2F8` are group 3 of `set()`. Each site has to be reviewed by hand.

The object takes at least `0x218 + 4×4` bytes. With 6 voices the tables grow; whether they fit (what follows
`0x604A26B0` + size) or the object has to be moved is [open].

### 1.5 How it would be installed

1. **Flash the patched OS** with the mechanism the plug-in uses (`70 20 00 00`, container `UPD_POUT_INFO`,
   [Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md) §9.2) or with Roland's update procedure. It is
   recovered by reinstalling `SYSTEM1_UPD.BIN` 1.30 with the official procedure. `tools/plugout_tool.py` does not send
   `70 20 …` containers.
2. **Port it to a slot-1 PLUG-OUT**: extract the model (master) and the engine (slave) of the stock synth from the OS
   and adapt them to the SYSTEM-1 PLUG-OUT interface (regions at `0x60800000`/`0x60200000`, `SYSTEM1_SYNINFO`, table
   `0x0100FC00`; [Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md) §9.3). A much larger port than a patch.

### 1.6 Open steps

1. Slave: find the ARM code that uses the sets (pointer tables at `0x6004A86C`…: 23 variants × 4 columns) and the
   one that writes the ids `0x110/0x120/0x128` to the DSPs.
2. Slave: map the per-voice data words (bank 1 and the other fields) to choose room for voices 4 and 5.
3. Slave: measure the cost of the main programs together with the sections (`voice_sets.py` only counts the sets).
4. Master: review the 58 sites and write the assigner patch.
5. Build the image and check it in the emulator (`examples/from_scratch/emu/`) before touching the device.

## 2. The SH-101 PLUG-OUT with 4 voices

Request of 2026-10-22: take the SH-101 PLUG-OUT (`plugins/SH-101/Script/Code1.Dat`, version 14.FA, for SYSTEM-1) and
make it play 4 notes of polyphony.

### 2.1 What the SH-101 is

* A monophonic synth. On the SYSTEM-1 panel the VCA mode selector stays at `MONO` (manual, table of controls)
  [code/manual]. `Script.xml` has no poly-mode parameters; only `CHORD MEMORY Note (voice 1…16)`.
* Region 1 (engine, slave, `0x60200000`): **two DSP programs**, both without voice sections [code]:

| Set (entries `{addr, len, ptr}`) | Program | Size | DSP |
|---|---|---|---|
| `0x60221FBC`: `0x0` header, **`0x100` main**, `0x7FF2` closing | `0x60220B44` | 2611 half-words, **575 instructions**, 1 voice section mark (`ef75`) | slave A (`FC30`) |
| `0x60224194`: the same | `0x60224130` | 41 half-words, 12 instructions | slave B (`FC34`) |

* Both sets are referenced from the descriptors `0x6021F7EC` (`{…, 3, set, …}`) and `0x60220960` [code].

### 2.2 How the slave OS loads programs (1.30)

Slave service table at `0x0100FC00` (copied from `0x6005A6B0`, 0x288 bytes) [code]:

| Service | Function | What it does |
|---|---|---|
| `0x0100FC30` | `0x60003ECF` | queues a program set for **DSP A** (queue `0x600603E0`) |
| `0x0100FC34` | `0x60003ED9` | the same for **DSP B** (queue `0x600603F0`) |
| `0x0100FC38` / `FC3C` | `0x60003EE3` / `0x60003EED` | data queues (`0x60060400` / `0x60060410`; [Vol. IV ch. 3](../vol4_firmware_container_protocol/os_service_table.md)) |
| `0x0100FC28` / `FC2C` | `0x60003EBD` / `0x60003EC5` | further queues (`0x60060420` / `0x60060430`); not used by any PLUG-OUT read [open] |
| `0x0100FC40` | `0x60003F22` | places a voice program: `FC40(index 0–7, program set, 3 data sets)` → (A/B, section address, section) through its own queue (`0x600602d8`) [code]; an own program placed this way runs [measured 2026-10-28] |
| `0x0100FC44` | `0x60003F96` | switches one of those 8 sections on/off: the bit `1 << section` of `[0x60018520 + A/B·4 + 0x68]` means **off**; `FC44(i, 1)` clears it (on), `FC44(i, 0)` sets it [code] |

Index of `0x0100FC40` → (A/B, section address, section):

| index | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|---|---|---|---|---|---|---|---|---|
| A/B | 0 | 0 | 1 | 1 | 0 | 0 | 1 | 1 |
| address | `0x1900` | `0x4900` | `0x1900` | `0x4900` | `0x3100` | `0x6100` | `0x3100` | `0x6100` |
| section | 2 | 4 | 2 | 4 | 3 | 5 | 3 | 5 |

The SH-101 engine (`0x602016F8`) [code]:

1. calls `FC44(i, 0)` for i = 0…7 (switches off the 8 voice sections of the stock synth);
2. queues its large set on DSP A (`FC30`) and the small one on DSP B (`FC34`).

So **the whole SH-101 (oscillator, filter, envelopes…) is one program of 575 instructions on slave DSP A**; slave
DSP B only runs the 12-instruction program that passes the output on [code].

### 2.3 What 4 voices would need

One voice of the SH-101 costs 575 instructions; the slave DSP A budget is 703, so **two voices do not fit in one DSP**
and the slave has only two DSPs (A and B). With the SH-101's own microcode the slave can hold at most **2 voices**
(one per DSP), and then the bus to the master is the limit (§3.4). 4 voices would need a lighter voice program.

What a 2-voice version needs [inferred]:
1. **Microcode**: a copy of the 575-instruction program on slave DSP B, with its output and state addresses moved so
   the two copies do not overwrite each other; the 12-instruction program that forwards the output, extended to two
   inputs.
2. **Engine (region 1)**: each patch parameter written today to DSP A also written to DSP B; pitch and gate of each
   voice only to its DSP. The parameter-writing wrapper (services `0x0100FCxx` with a DSP unit) has to be found and
   duplicated or redirected.
3. **Model (region 0)**: the SH-101 assigner is mono; it has to become polyphonic and send note/gate per voice (the
   stock per-voice ids are `0x110+v`, `0x120+v`, `0x128+v`; the SH-101's are to be seen).
4. **Check**: the DSP emulator (`examples/from_scratch/emu/`) with the SH-101 program, then install in slot 1
   (`plugout_tool.py install … --slot 1`).

[open]: whether the data memory of each DSP is its own (bank 0 per DSP or per chip, [Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md)
§19), whether the program uses single-owner resources (delay memory, owned IOMD) and the format of the engine's
parameter writes. Next step: list the references of the two SH-101 programs (575 and 12 instructions) to the banks
1–5 with `asm/disasm.py` (inputs, outputs, parameters).

## 3. Does the microcode of a SYSTEM-8 PLUG-OUT fit the SYSTEM-1? (`dsp_fit.py`)

State: 2026-10-22. A study tool, without hardware: it measures the DSP time and the traffic between DSPs of Roland's
JX-3P programs and searches a placement on the four DSPs of the SYSTEM-1. Question: can the JX-3P be installed with
its microcode on the SYSTEM-1, with 4 voices?

### 3.1 Use

```sh
python3 system-1/dsp_fit.py                          # JX-3P, 4 voices (one SYSTEM-8 slave), DCO SAW/SAW, no effect
python3 system-1/dsp_fit.py --dco1 sqr --dco2 noise --fx ch
python3 system-1/dsp_fit.py --voices 2               # only the chain of voices 0/1 (DSP0 -> DSP2 -> DSP4)
python3 system-1/dsp_fit.py --budget 703 --reserve-ma 40 --samples 4000
```

It needs `examples/from_scratch/engine/build/programs.h` (made by `examples/from_scratch/engine/build.sh` from
`plugins/JX-3P`; the result depends on which DCO programs that build put in it). The microcode is read from that local
copy at run time; nothing is copied into the repository. About 10 s with 1500 samples. Exit code 0 if it fits, 1 if
not.

| option | default | what it is |
|---|---|---|
| `--dco1` | `saw` | program of the DCO1 slot (`saw`, `sqr`) in DSP2 and DSP3 |
| `--dco2` | `saw` | DCO2 slot (`saw`, `sqr`, `noise`, `none` = no DCO2) |
| `--fx` | none | effect slot of DSP4 (`od`, `ds`, `ch`, `fz`, `cr`, `ph`) |
| `--voices` | 4 | 4 = the six DSPs of one slave; 2 = DSP0, DSP2, DSP4 |
| `--budget` | 925 | instructions per sample and DSP. 925–945 is the SYSTEM-8 value [measured]; the SYSTEM-1 value is 703 (slave A) [measured]: use `--budget 703` |
| `--reserve-ma` | 40 | instructions of master DSP A taken by the OS output program at `0x82` [estimated] |
| `--samples`, `--seed` | 2000, 1 | emulated samples and seed of the random inputs |

### 3.2 What it measures

1. **Time per program** [emulator]: each program runs in the C emulator with random inputs in bank 5 and in the
   parameters (`C000`–`C03F`), which change every 250 samples so the jumps go both ways. It gives the executed
   instructions per sample (minimum, mean, maximum). The cost of a slot is the difference between the DSP with and
   without it (the slots run after the main program and read what it writes). It also gives bytes and static
   instructions (`emu/listing.py`).
2. **Links between the SYSTEM-8 DSPs** [code, from the listings]: words per sample of
   * forward MXM: `Z 84nn` of DSP n read as `M 80nn` (n even) or `O 82nn` (n odd) by DSP n+2;
   * backward MXM: `M`/`O` written by DSP n and read as `S 86nn` by DSP n−2;
   * the pair window `A400`–`A7FF` (DSP0↔1, 2↔3, 4↔5);
   * the audible output `6020`/`6021`.
3. **Placement on the SYSTEM-1**: it tries every assignment of the main programs to the four DSPs (slave A/B, master
   A/B); each slot (DCO, effect) goes to one of the two DSPs of the **same chip** as its main program, because they
   talk through bank 1 (shared by A and B [inferred], [ch. 2](system1_voice_allocation.md) §3.2). Conditions:
   * each DSP ≤ `--budget` (master A with `--reserve-ma` already taken);
   * slave → master words ≤ 8 (IOMD `6006`–`600D`; the audio output counts here if it leaves the slave);
   * master → slave words ≤ 20 (`600E`–`6021`).

   It shows the best placement (first those that meet the bus, then the lowest peak) and how many MXM links would
   have to be rewritten: bank 4 does not appear in the SYSTEM-1 programs read [code], so each MXM link would have to
   move to another mechanism (`A1xx` mailbox inside the chip, IOMD between chips) by patching addresses in Roland's
   microcode.

It does **not** measure the latency of `A1xx` and of the IOMD on the SYSTEM-1, nor the ARM work (the JX-3P engine is
Thumb with VFP and SYSTEM-8 services; the SYSTEM-1 has no FPU and another interface,
[examples/system_1/README.md](../../examples/system_1/README.md) §3).

### 3.3 Results (2026-10-22, budget 925) [emulator]

JX-3P 1.01, DCO SAW/SAW (Roland's DCO programs), no effect, 1500 samples:

| SYSTEM-8 DSP | program | bytes | static | executed (max.) |
|---|---|---|---|---|
| DSP0 | MAIN0 (LFO/envelopes voices 0,1) | 5066 | 528 | 516 (mean 453) |
| DSP1 | MAIN1 (voices 2,3) | 5018 | 526 | 514 (mean 450) |
| DSP2 | MAIN2 + DCO1_SAW + DCO2_SAW | 2068 + 2×4104 | 189 + 2×325 | 839 |
| DSP3 | MAIN3 + DCO1_SAW + DCO2_SAW | 2018 + 2×4104 | 184 + 2×325 | 834 |
| DSP4 | MAIN4 (filter/VCA voices 0,1, sum, output) | 5174 | 532 | 532 |
| DSP5 | MAIN5 (voices 2,3) | 5016 | 513 | 513 |
| | **total, 4 voices** | | | **3748** |

Links: DSP0→2 MXM 19, DSP1→3 MXM 17, DSP2→4 MXM 22, DSP3→5 MXM 20, DSP2→0 and DSP4→2 MXM S 3 each, DSP0→1 `A4xx` 7,
DSP4↔5 `A4xx` 4 + 4, output 2.

* **4 voices: does not fit.** 3748 instructions per sample against 3660 of the four DSPs at budget 925
  (4 × 925 − 40); with the measured SYSTEM-1 budget (4 × 703 − 40 = 2772) even less.
* **2 voices** (`--voices 2`, 1887 instructions): at budget 925 the best placement was slave A 721 (MAIN2, MAIN4),
  slave B 650 (DCO1_SAW, DCO2_SAW), master A 516 + OS (MAIN0), master B 0; bus slave → master 5 of 8, master → slave
  19 of 20. Slave A's 721 is above the measured 703 [inferred: arithmetic]. It would need 4 MXM links rewritten in
  Roland's microcode and the JX-3P ARM engine ported (or an own one that gives the microcode the same bank-5 inputs).
* **One DCO, 4 voices** (`--dco2 none`): 3098 instructions; at budget 925 it fitted (master: MAIN2 + MAIN4 / MAIN0 +
  DCO1; slave: MAIN3 + MAIN5 / MAIN1 + DCO1), above the 2772 of the measured budget [inferred: arithmetic]. Built as
  C-S1JX ([examples/system_1_jx/README.md](../../examples/system_1_jx/README.md)): paused, it still hangs
  the device (at least one cause left in MAIN1, slave B, §10.7 of that README).

### 3.4 Comparison: is it easier to give polyphony to the SH-101, SH-2 or PROMARS? (`s1_progs.py`)

```sh
python3 system-1/s1_progs.py                         # all three (work/unpack/<NAME>_S1_Code1)
python3 system-1/s1_progs.py work/unpack/SH-2_S1_Code1 --listing
```

For each program of the sets of region 0 (master) and region 1 (slave): address, static instructions and executed
instructions per sample in the emulator, IOMD words read and written, bank-5 words written. The three PLUG-OUTs are
native SYSTEM-1 ones (their ARM code already runs there, without FPU), but **monophonic**.

Measured 2026-10-22 [emulator; no program has jumps, so static = executed]:

| | slave A | slave B | master A (modulation) | master B (mix) | IOMD master → slave |
|---|---|---|---|---|---|
| SH-101 | 575 | 12 | 410 | 436 | 20 (`600E`–`6021`, all) |
| SH-2 | 108 + 353 (`0x1900`) + 245 (`0x3100`) = 706 | 62 | 367 | 406 | 10 |
| PROMARS | 373 + 344 (`0x1900`) = 717 | 4 + 69 (`0x1900`) | 426 | 313 | 6 + 1 (`6010` from master B) |

The slave also writes `6026`–`602D` (SH-2, PROMARS) and master A reads them, so the slave → master bus has at least
16 words (`6006`–`600D` and `6026`–`602D`) [code].

**Voices** [inferred], assuming the worst case: the whole slave-A program and the master-A modulation are per voice
(duplicated), slave B and master B are global. With the measured budget (slave A 703, slave B 708; master A not
measured, 925 used, 40 for the OS):

| | 2 voices | limit |
|---|---|---|
| SH-101 | slave A 575, slave B 12 + 575 = 587; master A 2 × 410 + 40 = 860; **IOMD 40 of 20** | the bus: its modulation already fills the 20 words to the slave |
| SH-2 | slave A 706 (over 703), slave B 62 + 706 = 768 (over 708) | slave time |
| PROMARS | slave A 717 (over 703), slave B 73 + 717 = 790 (over 708); master A 892 | slave time |

None reaches 4 voices. If part of the slave program were global (for example the PROMARS main program, 373), the
picture changes [open: which words are per voice is not known].

What 2 voices need in the three: a polyphonic assigner (an own one, changing the pointer `SYNINFO+0x2C` of region 0),
a second copy of the voice programs with their state relocated (banks 0/1 and per-voice words of bank 5) and hooks in
Roland's ARM code (master and slave) so that it writes note, gate and modulation of the second voice to the copy's
addresses. None of this is done.
