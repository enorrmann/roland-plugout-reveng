# Volume VI — Chapter 2: C-S1V4 / C-S1V8 — the Own PLUG-OUT C-V4 Ported to the SYSTEM-1

> The full description (Spanish: use, SYSTEM-1 PLUG-OUT interface, audio chain, test order, risks, parameter capture
> PARAMLOG, eight voices) is [examples/system_1/README.md](../../examples/system_1/README.md); every fact below comes
> from it unless another source is named; section numbers in parentheses are those of the README.

## 1. What it is and its state

* **C-S1V4** is the SYSTEM-8 C-V4 ([ch. 1](c_v4_system8_reference.md)) rewritten for the SYSTEM-1 PLUG-OUT interface:
  four polyphonic saw voices with a gate; all the ARM code and all the microcode are ours (no Roland bytes; Roland's
  programs were only read to study the interface).
* State 2026-10-22: **C-S1V4 works on the device** [measured, by ear]: single notes, four-note chords and the panel's
  VOLUME. C-S1TONE checked the loading and the chain slave → master → output first; VOLUME did not act until the
  parameter capture (§6) showed that it is id `0x100`. Tuning not measured with an instrument.
* **C-S1V8 (8 voices) also works** [measured, by ear, 2026-10-22]: eight held notes all sound, none is cut. The stock
  SYSTEM-1 has 4 voices; the OS forwards the ids of voices 4–7 to the slave unchanged (§7).

## 2. Build and install (§1)

```sh
sh examples/system_1/build.sh                          # -> plugins/C-S1V4/Script/Code1.Dat (and Code.Dat)
TONE=1 NAME=C-S1TONE sh examples/system_1/build.sh     # start-up test: voice 0 always sounds (A)
VOICES=8 NAME=C-S1V8 sh examples/system_1/build.sh     # 8 voices of polyphony (measured: works)
python3 examples/system_1/make_s1.py --sim             # simulates the three programs
python3 tools/plugout_tool.py install plugins/C-S1V4 --slot 1 --force --no-patches -y
```

The SYSTEM-1 has a single slot: installing C-S1V4 **replaces the SH-2**, which is recovered with
`python3 tools/plugout_tool.py install plugins/SH-2 --slot 1 --force -y`.

## 3. Structure (§2.1–§2.3)

| region | load address | runs on | C-S1V4 |
|---|---|---|---|
| 0 | `0x60800000` | master | `model.c`: SYNINFO, 4-voice assigner, model object that loads the master's DSP B, 64 empty patches at `+0x1000` |
| 1 | `0x60200000` | slave | `engine.c`: SYNINFO, engine that loads the slave's DSPs A and B and writes pitch and gate |

A single slot, no relocations. The SYSTEM-1 BMCs have no FPU: built for Cortex-M3 with `-mfloat-abi=soft`, the float
values are bit patterns precomputed by `make_s1.py` (§2.1).

Audio chain in PLUG-OUT mode (§2.2):

```
slave DSP A   s1m0 @0x0100 : 4 saws x gate -> sum x 0.25 -> A150   (bank 5 shared by A and B)
slave DSP B   s1m1 @0x0100 : A150 x C000 -> IOMD 6006 (L), 6007 (R); 6008..600D = 0
master DSP B  s1mm1 @0x1000: IOMD 6006 / 6007 -> A118 / A119
master DSP A  (the OS's program at 0x82): A118 / A119 -> DC filter -> x C004 -> 6000 / 6001 (output)
```

If the model loads nothing on master B, the PLUG-OUT is silent: that is why `model.c` loads its own DSP-B program at
`0x1000`, as the SH-2 does (§2.2). The verified microcode of `s1m1` and `s1mm1` is reproduced in
[system-1/WAVE_BUS.md](../../system-1/WAVE_BUS.md) §4.1 and §5.1; the first voice of `s1m0` in
[Vol. III ch. 2](../vol3_voice_allocation_scheduling/system1_voice_allocation.md) §5.1.

| program | where | what it does (§2.3) |
|---|---|---|
| `s1m0` | slave A `0x0100` | 4 voices: pitch `A030+v` (octaves, `(note−12)/12`), the saw of C-V4 with the phase in the bank-0 ring (writes `P`, reads `P+1`; `P = 0x300 + 0x10·v`), × gate `A038+v`, sum × 0.25 → `A150`. 137 instructions |
| `s1m1` | slave B `0x0100` | `A150 × C000` → `6006`, `6007`; `6008`–`600D` = 0 |
| `s1mm1` | master B `0x1000` | `6006`/`6007` → `A118`/`A119`; `A11A`–`A11D` = 0 |
| `s1c0`, `s1c1` | slave A/B `0x7FF2` | closing of program memory: frame counter `E000` → `A004`/`A005`, as in all SYSTEM-1 PLUG-OUTs |

SYSTEM-1 programs start with `0a02 ff71` and end with `0a02 ff70 0b02 00f8 0001`. `make_s1.py --sim` (no hardware):
A 440 → 440 crossings per second, a 4-note chord reaches 0.78 peak, no notes gives an exact 0 (§2.3).

**Voices in the main program, not in sections** (§2.4): C-S1V4 puts the four voices in the main program of slave DSP A
and switches off the OS's eight voice sections (`FC44(i, 0)`). The comparison main-vs-sections is in §2.4 of the
README and in [Vol. III ch. 3](../vol3_voice_allocation_scheduling/slot_program_model.md) §3.

> Note: §2.4 of the README gives "about 925 instructions per sample and DSP" for the SYSTEM-1 (the SYSTEM-8 figure).
> The later measurement on the SYSTEM-1 slave is 703 ran / 826 hung for DSP A and 708 ran for DSP B
> ([Vol. III ch. 4](../vol3_voice_allocation_scheduling/execution_timing_budgets.md) §2).

## 4. Eight voices: C-S1V8 (§7)

| | C-S1V4 | C-S1V8 |
|---|---|---|
| assigner (region 0) | poly of 4 | poly of 8 (same rule: same note > oldest free > released with hold > oldest) |
| ids | `0x110`–`0x113`, `0x120`–`0x123` | `0x110`–`0x117`, `0x120`–`0x127` |
| slave bank 5 | pitch `A030`–`A033`, gate `A038`–`A03B` | pitch `A030`–`A037`, gate `A038`–`A03F` |
| phase (bank 0) | `0300`–`0331` | `0300`–`0371` |
| `s1m0` (slave DSP A) | 137 instructions | **269** |
| level per voice | 1/4 | 1/8 (a chord of 8 does not saturate: 0.57 peak in the simulation) |

Result [measured, by ear]: the eight voices sound at once and no note cuts another. So the master's OS accepts 8 voices
from a PLUG-OUT assigner and sends the slave the ids `0x114`–`0x117` and `0x124`–`0x127` like those of voices 0–3.
Beyond 8 [inferred]: the velocity block has 8 ids (`0x120`–`0x127`; `0x128` starts the trigger block), so with these
ids the maximum is 8; notes (`0x110`–`0x11F`) would have room for 16.

## 5. Where it led

C-S1V4 / C-S1V8 are the base of the later SYSTEM-1 PLUG-OUTs: C-S1SY (the synth built step by step,
[examples/system_1_synth/README.md](../../examples/system_1_synth/README.md)), MUTABLE-SYSTEM
([examples/mutable_system/README.md](../../examples/mutable_system/README.md)) and SYSTEM-S1CC
([examples/system_s1cc/README.md](../../examples/system_s1cc/README.md)). The SYSTEM-1 interface (SYNINFO, vtables, DSP
program queues), the open risks and the PARAMLOG parameter capture are §3–§6 of the README.
