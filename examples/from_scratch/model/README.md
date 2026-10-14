# Region 0 (the model) in C

Our own code for the master BMC side of the PLUG-OUT. It contains nothing
from another PLUG-OUT and calls nothing but the kernel's parameter services
(`0x01000400`): no hooks, no stock code. With `engine/` (region 1) it makes
`plugins/C-SAW` entirely ours, except for the DSP microcode the engine copies
at build time.

```bash
examples/from_scratch/model/build.sh            # region 0 only (model/build/)
examples/from_scratch/engine/build.sh           # both regions -> plugins/C-SAW
python3 examples/from_scratch/model/test_emu.py 1000   # compare with the JX-3P
MODEL=jx examples/from_scratch/engine/build.sh  # old layout: stock region 0
```

| File | |
|---|---|
| `model.c` | SYNINFO, `init` / descriptor, create / destroy / select, the note assigner (17-entry vtable) |
| `params.py` | our parameter list (groups, parameters, value labels) and the slot memory (INIT patches, patch2 / sequencer defaults); writes `build/model_list.h`, `build/storage.bin` |
| `link.ld`, `storage.S` | image layout: SYNINFO at 0, slot memory at `0x8000`, code from `0x118000` |
| `test_emu.py` | differential test in unicorn: same calls into both region 0s, kernel traffic compared call by call |

## What region 0 has to provide

* **SYNINFO** at offset 0: `+0x24` `init()` and `+0x28` the parameter record
  list. The OS calls `init` many times (it must return the same descriptor),
  then `desc[1]()` (create) and writes the slot into `desc[0]`.
* **Descriptor**: `[1]` create, `[2]` destroy, `[3]`/`[4]` assigner of part
  A/B, `[5]` 0, `[6]` `select(part, mode)`.
* **Parameter list**: records `{i32 type, u32 size, …}` ending in `-1`: group
  (0), parameter (1, id / range / label list / kind / panel position / name),
  label list header (2) + labels (3), and types 4 and 7 (meaning unknown).
  The ids are the JX-3P ones because the engine uses them. Ours encodes
  byte for byte like the stock list, except one label (`JX-3P CH` → `CHORUS`).
* **Slot memory** at `base + 0x8000`: the OS keeps the slot's patches *inside
  region 0* (table of `base + 0x8000` per slot at `0x602B9194`, accessors
  `0x6000870E..0x600087A0`):

  | Offset | Size | |
  |---|---|---|
  | `0x008000` | 64 × `0x200` | patch: u32 per id, index = id − `0x6CD`, name = 16 chars in words 64–67 (first char in the high byte) |
  | `0x010000` | 64 × `0x898` | patch2 |
  | `0x032600` | 64 × `0x3710` | patch3 (sequencer: header + 64 steps of 54 words) |
  | `0x10EA00` / `0x10EC00` / `0x10F498` | one of each | edit buffers |

  This is why a stub region 0 cannot be small. We ship 64 `INIT PATCH`
  patches; the plugin's bank transfer (`plugout_tool.py install`, unless
  `--no-patches`) overwrites them.

## The assigner

Object of `0xA4` bytes (voice count at `+4`, split mode at `+0xA0`, as in the
stock object). Voice parameters go through `set(part, 2, id, value)`:

| id | value |
|---|---|
| `0x429 + v` | note |
| `0x43A + v` | velocity, 0 = gate off |
| `0x44B + v` | portamento legato flag |
| `0x45C` | voice playing the highest note |
| `0x465` | bend − `0x2000` (clamped) |
| `0x467` / `0x46A` | CC1 / CC11 |

Split modes (`select`): 0 whole (part 0, 8 voices), 1 / 2 lower / upper half
(parts 1 / 2, 4 voices; the upper half uses voices 4–7), 3 / 4 parts 1 / 2 with
8 voices. Key assign (patch `0x6FF`): POLY (same note again, else the oldest
free voice, else the oldest one released under hold, else steal), MONO (voice
0, last note wins, falls back to the lowest key still down), UNISON (all
voices). Hold (CC64), all notes / sound off (CC120, 123–127), reset
controllers (CC121); portamento (`0x6FD` time ≠ 0 with mode `0x6FE`) sets the
legato flags at the start of a phrase.

**Status (2026-10-14): emulation only.** 1000 random sequences of 120 events
(notes, hold, CCs, bend, key assign / portamento changes, split modes, voice
counts) give the same kernel calls as the JX-3P region 0; a deliberate bug is
caught. Not yet run on hardware. Differences from the stock model: key assign
value 3 (not reachable from the panel, range 0–2) plays as POLY, and the
CC-to-parameter mapping of the stock base class is not there (the JX-3P
overrides it with a no-op too).
