# C-SAW: from-scratch engine (region 1) in C

Our own ARM code for the slave BMCs, with our own parameter model. The only
foreign data are the DSP programs (microcode): `programs.py` cuts them out of
the user's JX-3P PLUG-OUT at build time (static copy at fixed offsets, SHA-256
checked, nothing executed): the main program of each of the six DSPs, the
oscillator programs (SAW/PULSE, SQR, NOISE) and the six effect programs.
Nothing of Roland's is stored in the repository (`build/` is generated).

Region 0 (parameter list, slot memory, note assigner) is ours too:
`examples/from_scratch/model/`, built by `build.sh`. The container metadata
is written by `build.sh`; the plugin folder holds only `Script/Code8_<n>.Dat`
(`plugout_tool.py install` sends only the firmware; the slot starts with the
64 INIT patches of region 0). `MODEL=jx` builds with the stock JX-3P region 0,
Script.xml and preset banks instead.

```bash
examples/from_scratch/engine/build.sh                  # -> plugins/C-SAW
python3 examples/from_scratch/engine/test_model.py      # model vs JX-3P (reference, emu1)
python3 examples/from_scratch/engine/test_emu.py        # built engine vs JX-3P (emu1)
python3 tools/plugout_tool.py install plugins/C-SAW --slot 3
```

**Status (2026-10-14):** the parameter model is complete in emulation (all
panel / patch parameters, waveform and effect program swaps); not yet on
hardware. Before it, the hardware build played with fixed tables (eight voices,
only the panel CUTOFF responded).

**DSP descriptors (2026-10-14):** program and data descriptors are six words
`{addr, bytes, bytes, src, 0, 0}` (the OS uses the last two) and each one is
used by one DSP only, with its own data descriptor, as in the JX-3P image.
The engine had four-word descriptors shared by DSP2 and DSP3; fixed, not yet
re-tested on hardware with waveform / effect changes.

**ISA test bench:** `DSPTEST=1 MC_TEST=... build.sh` (driven by
`re/dspaudio.py`) runs test microcode in the DCO2 slots of DSP2/DSP3; see
NOTES.md "ISA del DSP medido en el equipo por audio USB".

## Files

| File | |
|---|---|
| `engine.c` | the engine: model interpreter, parameter state, program registration and swaps, notes |
| `model_spec.py` | **the parameter model**: for each parameter, the DSP writes as expressions over the current values |
| `expr.py` | the expression language (Python evaluator + byte code compiler) |
| `gen_model.py` | `model_spec.py` -> `build/model.h` (tables, byte code, velocity curve) |
| `programs.py` | microcode copy -> `build/programs.h` |
| `ref.py`, `test_model.py`, `test_emu.py` | reference (JX-3P in emu1, tests only) and tests |
| `gen_tables.py` | old table generator (ran the JX-3P engine); no longer part of the build, kept for `re/` tools and `hybrid.sh` |

## The parameter model

`out(source, targets, expr, when=None)`: when `source` changes (vt+0x30), the
engine evaluates `expr` (and `when`) with the current parameter values and
writes the result to every target: `P<dsp>:<id>/r<ramp>f<flag>` = parameter
block (`+0xA0` with ramp; `+0xA4` at start-up), `B<dsp>:<id>` = bank 5
(`+0xB0`). Several sources may write the same DSP id; the last write wins (as
on the JX-3P). On start-up / full refresh (vt+0x0C, vt+0x24(1)) the engine
reads all 72 parameters with OS `+0x00 get(0, group, id)` and evaluates every
source in `REFRESH_ORDER`, then the four voices.

Written from measurements of the JX-3P's behaviour (`re/measure.py`,
`deps.py`, `probe.py`, `sweepfit.py`, `knotfit.py`, `spec_blocks.py`):

* exact formulas where one fits: linear, two-slope bipolar (`bip`), lists
  (`pick`), thresholds, LFO rate (`floor((4v+4)/41)` steps of x1, x4/3, x3/2
  per octave), bend depth `(sens/255)/12 * (min(range, 23) + 1) * {1,2,3}`,
  `((v+1)/129)^n` for 0x737, panel offsets `floor(p + (255-p) * min(q,100)/100)`
  (resonance, HPF, portamento, effect amount), LFO rate x `max(1, q/100)`
* least-squares knots (piecewise linear, log2 domain for times) for curves
  without a closed form: envelope times, LFO delay, HPF, level, mix, portamento,
  effect curves, velocity; max error 0.3–2 % (listed in `model_spec.py`)
* combined outputs: envelopes x circuit mod (per-voice factor
  `max(lo, 1 - a*cm)`), VCA Mode GATE, per-voice detune (circuit mod spread,
  UNISON offsets), portamento flags, part routing states (0x22C/0x519/0x544/
  0x56E), effect parameters per effect type
* per-slave constants where the two slaves differ (voice spread, routing)
* float parameters: the OS returns float32 bits for 0x74D and 0x7E4 (`F(id)`);
  0x7E4 is the cutoff the JX-3P writes last on a refresh (an integer 0 here
  closed the filter at start-up on hardware), the panel cutoff 0x2DB pulls it
  towards 1 (`F + (1 - F) * q / 100`)

The JX-3P caches some part flags in its 0x22C handler, so its writes depend on
the order in which those values arrive; the model follows the current values
(what the JX-3P gives after a full refresh). The tests count those cases
separately.

## Removed from the model (2026-10-21) and where the full one is

Three experiments trimmed `model_spec.py` in place: `b50213f` removed ENV-2 (C-NOENV: the VCA takes the note gate,
see `programs.py` PATCH_SITES), `0396d20` removed ENV-1, `b440296` ("mini") removed the VCF outputs (cutoff 0x6EA /
0x2DB / 0x7E4, resonance 0x6EC / 0x2DD, LFO 0x6D1, key follow 0x6F3, 0x7E3, 0x801, brilliance 0x6F8, bend /
aftertouch to the filter 0x73A / 0x73C, and the VCF part of 0x465 / 0x467). The full model is the one of `013d3fe`;
checked again on 2026-10-22 with this `test_model.py` (unchanged, importing it as `model_spec`): 0 differences in
changes (72 sources x 4 contexts x 2 slaves) and at refresh, largest approximation error 2 %. With the current
trimmed file `test_model.py 4` reports 92 differing (source, target) pairs and 84 ids at refresh, so the "0
differences" above no longer holds for it. The full model lives on, unchanged, in
`examples/system_1_jx/model_full.py` (C-S1JX, `docs/examples_system_1_jx_README.md` section 8).

## Program registration and swaps

`vt+0x08` registers slots, programs and data per DSP (OS `+0x28/+0x40/+0x58 +
4*dsp`) with the programs of the current waveforms and effect type. A change of
DCO1 / DCO2 waveform (0x6D4 / 0x6D9) or effect type (0x748) mutes the level
ids, waits for the DSP frame (`+0xC0`), replaces the slot program on DSP2/3 or
DSP4 (`+0x70(dsp, &prog, &data, ctx, ctx, slot, task, x0, x1)`) and ramps the
levels back; `vt+0x0C` does the same once at start-up (the slot programs only
run once `+0x70` has enabled their task bit). The empty slot program and the
data blocks (zeros) are ours.

Build details: `-mcpu=cortex-m4 -mfloat-abi=hard`, linked at slot 1
(`0x60A00000`) and slot 2 to find the relocations; the image has no `.bss` (the
slave does not clear the engine window). `+0xB0` gets a pointer to the id; the
slave reads it on entry, so a stack variable is fine.
