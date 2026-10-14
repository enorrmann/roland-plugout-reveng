# C-SAW: from-scratch engine (region 1) in C

Our own ARM code for the slave BMCs. Foreign data: only the DSP programs
(microcode), copied at build time from the user's JX-3P PLUG-OUT — the main
program of each of the six DSPs and the **SAW** program in both oscillator
slots. Nothing of Roland's is stored in the repository (`build/` is generated).

Region 0 (the model on the master: panel, note assigner) is still the stock
JX-3P one; replacing it with C is the next step.

**Status (2026-10-14, hardware): it plays.** Eight voices, correct pitch. It
starts with the filter closed and opens with the panel CUTOFF, which arrives as
an id outside the patch (part level, group 1) with a learned curve; patch
parameters are still ignored, so no other control responds. Earlier silent
builds and how they were fixed: see examples/from_scratch/NOTES.md (slot
programs need `+0x70`; ids outside the patch must use the device's values, not
the emulator's 0). `hybrid.sh` / `plugins/C-HYB` is the diagnostic that found it.

```bash
examples/from_scratch/engine/build.sh                  # -> plugins/C-SAW
python3 examples/from_scratch/engine/test_emu.py        # compare with the JX-3P in emu1
python3 tools/plugout_tool.py install plugins/C-SAW --slot 3
```

## What the engine does

| Slave call | Engine |
|---|---|
| SYNINFO `+0x24` init (called many times) | returns the same descriptor: `D[1]` create, `D[2]` destroy, `D[5]` main object |
| `vt[0]` | slave index (`+0xF0`): slave 0 = voices 0–3, slave 1 = 4–7 |
| `vt+0x08` | registers 45 slots / programs / data with OS `+0x28..+0x6C` (same order and contents as the JX-3P) |
| `vt+0x0C` (also) | starts the slot programs like the JX-3P does when the waveforms / effect type arrive: mute ramps, `+0xC0` (waits for the DSP frame sync), `+0x70(dsp, &prog, &data, ctx, ctx, slot, task, x0, x1)` for DCO1 (`0x6000`, task 2) and DCO2 (`0xB000`, task 3) on DSP2/3 and the effect on DSP4 (`0xB000`, task 3, `0x1F0`, `0x80`), unmute. `+0x70` clears the slot's task bit, loads, and enables it again (`s_boot` `0x01105F94`/`0x01105F5C`): registering the programs in `vt+8` alone does not start them — second silent test |
| `vt+0x0C`, `vt+0x24(1)` | sends the start-up state: 242 parameter block values (`+0xA4`), 173 bank-5 values (`+0xB0`), 4 × `+0xBC`; separate tables per slave |
| `vt+0x10` / `vt+0x18` | oscillator mute / unmute (`+0xA0` ramps) |
| `vt+0x30(id, 0, value)` | `0x429+v` note → pitch `B0(u, 0x21+0x12k, (note−12)/12)`; `0x43A+v` velocity → gate/velocity on DSP 4+u and the envelope gate `B0(u, 0x22+0x12k)`, "any voice on" flags on DSP0/1; `0x45C` → last-voice flags on DSP5 (u = lv>>1, k = lv&1) |
| on/off ids outside the patch (`0x4A1`, `0x4A2`, `0x4A3`, `0x73E`) | read with OS `+0x00 get(0, group, id)` whenever the state is sent, and applied when they arrive through `vt+0x30`: bank-5 outputs = value ≠ 0 (DSP0/1, 2–5). In the emulator the OS returns 0 for every id outside the patch, so a fixed table had them all off: the first hardware test (2026-10-14) was silent |
| everything else | no-op (panel parameters are ignored: fixed sound = JX-3P default patch with SAW) |

`gen_tables.py` gets all tables by running the JX-3P engine in emu1 with its
default patch. `test_emu.py` runs both engines in emu1 and compares every OS
call: registration, state and note on/off on all eight voices are identical.

Build details: `-mcpu=cortex-m4 -mfloat-abi=hard`, linked at slot 1
(`0x60A00000`); the image has no `.bss` (the slave does not clear the engine
window, its RAM test leaves patterns there). Relocations = words that move by
0x200000 between links at `0x60A00000` and `0x60C00000` (77). `+0xB0` gets a
pointer to the id; the slave reads it on entry (`0x0110DE7C`), so a stack
variable is fine.
