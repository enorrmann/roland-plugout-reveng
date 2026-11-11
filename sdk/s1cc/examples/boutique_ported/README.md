# boutique_ported — the Roland Boutique JP-08 / JU-06 / JX-03 voice on SYSTEM-S1CC

State: 2026-11-11. The voice of the first-generation Roland Boutiques as an s1cc kernel for the SYSTEM-1
(`examples/system_s1cc`). One firmware (`BQ_UPD.BIN` 1.21) holds the three models; this example builds one model
per PLUG-OUT (`BQ_MODEL`; JP-08 done: `JP-08-S1`). Nothing is written by ear: the DSP part is Roland's microcode
translated operation by operation (`dsplift.py`, [Vol. V ch. 8](../../../../docs/vol5_toolchain_compiler_sdk/dsplift.md)),
the ARM part is what Roland's ARM code writes, recorded by running it in an emulator.

| file | what it is |
|---|---|
| `bq.c` | the kernel: per-voice inputs (pitch, gate, PWM source), DSP A / DSP B, build options |
| `bq_jp08_voice.h` | the JP-08 voice program as C (generated, §2; do not edit) |
| `bq_jp08_tables.h` | the JP-08 ARM side as data: parameter words, one table per panel control (generated, §3) |
| `bq_names.txt` | names and the constant inputs of the lift (§2; hand-written) |
| `bq_import_ranges.txt` | the ranges of the values DSP A hands to DSP B (§2) |
| `tools/bq_extract.py` | `BQ_UPD.BIN` → ARM image and DSP programs (§1) |
| `tools/bq_fw.py` | runs pieces of the ARM image in unicorn (§3) |
| `tools/bq_arm.py` | the ARM image → `bq_MODEL_tables.h` and the model's program patches (§3) |
| `tools/make_voice.py` | the DSP programs → `bq_MODEL_voice.h` (§2) |

## 1. Source

`BQ_UPD.BIN` = "Boutique System Program 1.21" (`boutique_sys_121`, 2019-09-12); its modules are named `PX-11:...`.
`PX-11:Appli` (0x40-byte header + LZSS, `tools/lzss.py`) is the ARM image, base `0x60000000`, SHA-256
`282b69e4…a202`.

```sh
python3 sdk/s1cc/examples/boutique_ported/tools/bq_extract.py BQ_UPD.BIN          # -> work/bq1 (not in git)
```

**One firmware, three models** [code]. The synth object (global `0x601c32c0`) keeps the model in byte `+0x400`:
0 = JP-08, 1 = JU-06, 2 = JX-03 (read from `[0x602eebc8 + 0x50]`; the USB name code `0x6001195c` writes "JP" / "JU"
/ "JX" by it).

**Two DSP chips, two voices each** [code]. Program memory per chip, the programs each sample runs (byte addresses):

| program | chip 0 | chip 1 | what it does |
|---|---|---|---|
| pre | `0x1000` | `0x0400` | LFO (rate, delay, 5 shapes), noise; shared by the chip's voices |
| voice | `0x1900` (A), `0x2c00` (B) | `0x1200`, `0x2500` | glide, two ADSRs, modulation, mixer, HPF, 4-pole filter, VCA |
| OSC1 / OSC2 slots | `0x3f00` / `0x4f00` (A), `0x5700` / `0x6700` (B) | `0x3800` / `0x4800`, `0x5000` / `0x6000` | one of 9 oscillator programs |
| post | `0x1400` | — | the four voices to stereo, level, soft clip |
| effects | — | `0x0800` | chorus / delay |

The 9 oscillator programs (variant table `0x60166e30`, per slot; chosen by the ARM at `0x6002a5d2`, `0x600292d6` from
the tables at `0x6006ad00` (OSC1) / `0x6006ad48` (OSC2), `[model × 6 + WAVE]`):

| variant | instructions | JP-08 OSC1 / OSC2 positions | JX-03 | JU-06 |
|---|---|---|---|---|
| 0 | 131 | 0 / 0, 3 | 0 | |
| 1 | 123 | 1 | 1 | |
| 2 | 134 | 2 (saw) / 1, 4 | | |
| 3 | 152 | 3, 4 (pulse, square) / 2, 5 | | |
| 4 | 27 | 5 (noise) | | |
| 5 | 264 (+ empty OSC2) | | | all (the DCO: saw, pulse, sub, noise) |
| 6 | 132 | | 2, 3 (saw) | |
| 7 | 152 | | 4, 5 | |
| 8 | 33 | | noise | |

**The model's immediates** [code, emulated]. At set-up the ARM rewrites immediates inside the DSP programs through an
indirect port: `0x40100900` = `0x40000000 | type << 28 | (0x50000 chip 0, 0x70000 chip 1) + byte address`,
`0x40100904` = the value; type 0 / 1 / 2 = a 16 / 24 / 32-bit immediate at that byte of the immediate stream. JP-08:
110 such writes on chip 0. The voice program is therefore not the same in the three models.

**Sample rate** [code]. The first-generation Boutique runs at **44.1 kHz** (the oscillator's phase step is
`440 / 88200` per sub-sample, four sub-samples per sample); the SYSTEM-1 at 96 kHz. The TB-03 (second generation)
runs at 96 kHz. Every per-sample constant is converted (§2, §3), r = 44100 / 96000.

## 2. The voice program (`bq_MODEL_voice.h`)

```sh
python3 sdk/s1cc/examples/boutique_ported/tools/make_voice.py jp08 2 3       # MODEL OSC1-variant OSC2-variant
```

1. The programs that make voice A of chip 0, each with the model's immediate patches (`work/bq1/MODEL_patches.json`).
2. The program's own per-sample constants converted to 96 kHz (`RATE_IMM`): gate counter step, filter frequency
   scale (`0.007836163 × r`), LFO step limit, shortest attack. The VCA level smoothing (`0.0075531`, a MAC
   coefficient) is not converted: it only smooths a constant.
3. Concatenated in this order and cut in two (`--split`): **DSP A** = pre, voice instructions 0…184 (LFO, glide,
   envelopes, modulation, the oscillators' pitch), OSC2; **DSP B** = OSC1, voice 185… (mixer, HPF, filter, VCA).
   OSC2 is on A because its output modulates OSC1's pitch (CROSS MOD). Every value flows A → B (6 values,
   `bq_import_ranges.txt`). `--zero-cells 41b`: OSC1's sync output, read on A, used only with OSC2 SYNC (off).
4. `dsplift.py --bq2 --kernel --bank1-ring --kout 203a`: the programs of one chip hand values over in bank 1, which
   is a ring as bank 0 (store at X, read X + 1 next sample; `--bank1-ring`, added for this port); `203a` is the
   voice's output to the post program. `--consts`: the parameter words no panel control moves.

Inputs (`bq_names.txt`): `A080` pitch (octaves, 4.75 = A 440: `s1_pitch()`), `A081` gate; `A090…A093` the PWM
source (`bq.c`); the rest constants:

| input | value | why |
|---|---|---|
| `A084`, `A086` | 5.5 | the envelopes are used as env² × −0.09999484 × (1 − 2a): 5.5 = +0.99995 (with 1 they were ×0.1: no audible envelope, measured on the lift 2026-11-11) |
| `A088`, `A089` | 0.2296875 | the oscillators' frequency factor (the ARM writes 2^range per note): 8′ × 44100 / 96000 |
| `A082`, `A083` | 0 | portamento off |
| others | 0 / 1 | sync, key follow of the envelopes, the chip-1 links: off |

New decoder templates (`examples/from_scratch/re/dsp_templates.txt`, 2026-11-11, from neighbouring decoded
instructions, not measured): `0x0288 MBBTNIRIII` (pre programs; same shape as the verified `0x0510`),
`0x04a8 MABBTNIRII`, `0x05ec ABBCTTTIRRR` (the saw oscillators, variants 2 and 6).

## 3. The ARM side (`bq_MODEL_tables.h`)

```sh
pip install unicorn
python3 sdk/s1cc/examples/boutique_ported/tools/bq_arm.py jp08
```

`tools/bq_fw.py` maps the image, copies what the start code copies (ITCM `0x6016725c` → `0x01000300`, `0xe97c`
bytes; `0x60175bd8` → `0x01010000`; SRAM data `0x60175c40` → `0x20000000`), replaces `operator new` (`0x6005660c`)
by a bump allocator, and records the writes to the parameter windows (`0x4014c000` chip 0, `0x4016c000` chip 1, 16
bytes per word `C000 + i`, value at +4) and to the program port. The full boot does not run (an RTOS that waits for
interrupts); instead:

| step | function |
|---|---|
| constructor of the synth object | `0x60054714` (creates the parameter sub-objects) |
| init (model byte from the global) | vtable 0 `0x6002472a` |
| model set-up (propagates the model) | vtable 3 `0x60024e1e` |
| first update (57 words, the patches) | vtable 4 `0x60024c1c` |
| a whole patch | vtable 10 `0x60024ff0` |
| one parameter | vtable 13 `0x6002b3e2` (`obj, id + 0x141, 0, value`) |

The patch parameter ids and names are tables in the image (26 bytes per entry from `0x60066b1c`: JP-08 `0x18e…`,
JU-06 `0x1d0…`, JX-03 `0x200…`). Internal id `0x108` = velocity → `C069 = (v / 127)²` (127: the SYSTEM-1 keyboard
has no velocity).

Each table row is the SYSTEM-1 knob value already mapped (`PANEL`, kinds `uni`, `bi`, `rev`, `sq`), the per-sample
words converted to 96 kHz (`RATE_WORDS`): attacks `1 − (1 − c)^r`, decay / release `(1 − (1 − 0.01 c)^r) / 0.01`, LFO
step `× r`, LFO delay factor `(0.9 + 0.1 c)^r`, HPF `1 − (1 − c)^r`.

## 4. Panel (JP-08)

| SYSTEM-1 | JP-08 | words |
|---|---|---|
| LFO RATE / FADE | LFO RATE / DELAY TIME | `C00A`, `C013` / `C00B` |
| LFO → PITCH (bipolar, 128 = 0) | OSC LFO MOD | `C04A` |
| LFO → FILTER (bipolar) | FLT LFO MOD | `C064` |
| LFO → AMP | — (0 in this build, §7) | `C06B` |
| PITCH ENV AMOUNT (bipolar) | OSC ENV MOD (ENV1 → pitch) | `C04B` |
| OSC1 COLOR | OSC1 CROSS MOD (knob² / 255) | `C04D` |
| OSC2 COLOR | PWM | `C053`, `C054` |
| OSC2 COLOR MOD | PWM SOURCE: MANUAL, LFO, ENV (pitch / filter / amp env all = ENV1) | inputs `A090…A092` |
| OSC2 FINE | OSC2 TUNE | `C046` |
| OSC1 MIX / OSC2 MIX | MIX BALANCE's two sides | `C05A` / `C05B` |
| FILTER CUTOFF / RESONANCE | CUTOFF / RESONANCE | `C05F` / `C060` |
| FILTER ENV AMOUNT (bipolar) | FLT ENV MOD | `C065` |
| FILTER KEY TRACK | FLT KEY FOLLOW | `C066` |
| FILTER A D S R | ENV1 | `C03C…C03F` |
| AMP A D S R | ENV2 (VCA) | `C040…C043` |

Fixed: OSC FREQ MOD DST = VCO1 + VCO2 (`C04F…C052` = 1), AMP LEVEL 255, OSC1 saw (variant 2), OSC2 pulse / square
(variant 3), range 8′.

## 5. Build, check, install

Validate natively first (seconds), then build (minutes: the profile is most of it, §6):

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/boutique_ported/bq.c -o sdk/s1cc/build/bq_jp08
sdk/s1cc/s1cc check sdk/s1cc/build/bq_jp08 --note 69 env2_a=0 env1_a=0 env2_s=255 cutoff=200 mix1=255 mix2=0
KERNEL=sdk/s1cc/build/bq_jp08 sh examples/system_s1cc/build.sh
python3 tools/plugout_tool.py install plugins/JP-08-S1 --slot 1 --force --no-patches -y
```

`s1cc check` with every knob at 128 gives silence (ENV2 ATTACK 128 is slow): give it the knobs above.

## 6. Cost and voices

| build | DSP A | DSP B | voices |
|---|---|---|---|
| JP-08, saw + pulse, 2026-11-11 | 472 | 413 | 1 (split A / B) |

Where the time of one voice goes (Roland instructions): pre 59, voice 339, each oscillator 123–152 (four
sub-samples per sample and a 28-tap decimation FIR). Ways to more voices, by gain:
1. the oscillators at two sub-samples (96 kHz × 2 = 192 kHz, more than Roland's 176.4 kHz): ~130 per voice;
2. envelopes and filter coefficients on the ARM (1 ms ramps): ~130 per voice, not sample exact;
3. pre (LFO, noise) once for all voices, as Roland: ~50 per voice.
Estimate: 4–5 voices with all three.

## 7. Open

- LFO → AMP: `C06B` fixed at 0; with the knob low its values span 1e-25 … 1e-8 and s1cc scales them wrongly.
- OSC1 / OSC2 WAVE and RANGE (a wave is a different program; range is the per-note factor `A088` / `A089`), OSC2
  SYNC, portamento, ENV1 POLARITY, the post program and the effects.
- JU-06, JX-03 (`BQ_MODEL`).
- Readable names for the lift (`bq_names.txt` `cell` / `section` lines, as `tb03_names.txt`).
- On the device: JP-08-S1 installed 2026-11-11 (first build: envelopes ×0.1, LFO / PWM inputs wrong); the build with
  the fixes of §2 / §4 not heard yet.
