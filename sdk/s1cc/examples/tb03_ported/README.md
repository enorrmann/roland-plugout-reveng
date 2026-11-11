# tb03_ported — the Roland TB-03 voice on SYSTEM-S1CC

State: 2026-11-11. The voice of the Roland TB-03 (Boutique, firmware `BQ2_UPD.BIN` 1.07) as an s1cc kernel for the
SYSTEM-1 (`examples/system_s1cc`, PLUG-OUT `TB-03-S1`, one voice). Nothing in it is written by ear: the DSP part is
Roland's voice program translated operation by operation (`dsplift.py`, [Vol. V ch. 8](../../../../docs/vol5_toolchain_compiler_sdk/dsplift.md)),
the ARM part is the values Roland's own ARM code writes, recorded in an emulator. There is no TB-03 in the project:
the reference everything is compared with is the lifted program itself.

| file | what it is |
|---|---|
| `tb03.c` | the kernel: panel mapping, the voice's inputs (pitch, gate, accent, slide), DSP A / DSP B |
| `tb03_voice.h` | the TB-03 voice program as C (generated, §2; do not edit) |
| `tb03_tables.h` | the ARM side as data: parameter init, knob tables (generated, §3; do not edit) |
| `tb03_import_ranges.txt` | the measured ranges of the values DSP A hands to DSP B (§2) |
| `tools/tb03_extract.py` | `BQ2_UPD.BIN` → decoded `BQ2:Appli` image and the DSP programs as hex (§1) |
| `tools/make_voice.sh` | the DSP program → `tb03_voice.h` (§2) |
| `tools/tb03_arm.py` | the ARM image → `tb03_tables.h` (§3) |

## 1. Source

`BQ2_UPD.BIN` = "Boutique2 System Program 107" (TB-03 1.07, 2021-01-21). Its module `BQ2:Appli` (0x40-byte header +
LZSS) is the ARM image (base 0x60000000); it carries the DSP programs as the SYSTEM-1 does, `{address, bytes,
pointer}` program sets, halfwords upper first ([Vol. V ch. 8 §5](../../../../docs/vol5_toolchain_compiler_sdk/dsplift.md)).

```sh
python3 sdk/s1cc/examples/tb03_ported/tools/tb03_extract.py BQ2_UPD.BIN        # -> work/tb03 (not in git)
```

| output | content |
|---|---|
| `work/tb03/BQ2_Appli.bin` | the decoded ARM image (SHA-256 checked) |
| `work/tb03/progs/v_0022.hex` | **the voice** (voice DSP set `0x6014e184`, program at byte `0x0022`, 751 instructions) |
| `work/tb03/progs/b_*.hex` | the second DSP (set `0x6015006c`): a list of calls, post-processing, effects, output |
| `work/tb03/progs/fx*_*.hex` | effect variants |

The voice program reads parameters `C000…C03E`, the per-note inputs `A003…A007` and `A104…A109` (from the second
DSP), and writes `A100…A103`; `A100` is the voice output.

## 2. The voice program (`tb03_voice.h`)

```sh
sh sdk/s1cc/examples/tb03_ported/tools/make_voice.sh [BQ2_UPD.BIN]
```

runs `dsplift.py --bq2 --kernel --split 415 --zero-cells 0bd,0bf --import-ranges tb03_import_ranges.txt`
([Vol. V ch. 8 §6](../../../../docs/vol5_toolchain_compiler_sdk/dsplift.md)):

| option | why |
|---|---|
| `--bq2` | the Boutique DSP: implicit flags, control words |
| `--kernel` | straight-line C for s1cc: registers as values, the bank-0 ring as delay chains `D_x_k`, conditions as 0 / 1 floats blended in (no compares) |
| `--split 415` | 751 instructions do not fit one slave DSP (703 / 708): instructions 0…414 become `tb03_voice_a` (DSP A), 415…750 `tb03_voice_b` (DSP B). 20 values cross with `s1_export` / `s1_import` (`TB03_VOICE_SLOTS`); 20 more are constants in the TB-03 setup and are written into B |
| `--zero-cells 0bd,0bf` | two bank-0 cells part A reads that only part B writes (a value from B back to A, which SYSTEM-S1CC has no path for); both are 0 with the TB-03's parameters |
| `--import-ranges` | the range of every handed value, measured on the lift (a held constant stays a constant, the rest give s1cc its range) |

The result is every operation of Roland's program in order, as C; `tb03.c` supplies its memory through the macros
`KP` (parameters, §3), `KIN` (inputs, §4), `KOUT` (only `A100` is kept), `KE` (bank 7: 0).

What the program is, read from the lift: pitch with glide (slide), two oscillators (saw, square: `C029` / `C02A`),
the filter with its envelope (ENV MOD, DECAY, accent sweep), the accent, the VCA envelope. A third oscillator, an FM
matrix and an LFO are in the program but off (their parameters are 0 after the init, no knob moves them); `tb03.c`
compiles the voice with fast floating point so clang removes `x * 0` and those parts cost nothing.

A readable hand-written version of the voice (verified against `tb03_voice.h`) is not written yet.

## 3. The ARM side (`tb03_tables.h`)

```sh
pip install unicorn
python3 sdk/s1cc/examples/tb03_ported/tools/tb03_arm.py work/tb03 -o sdk/s1cc/examples/tb03_ported/tb03_tables.h
```

The TB-03's ARM code is C++ with soft float; instead of reading it, its functions are run in an ARM emulator
(image at 0x60000000, ITCM copy `0x60163b4c` → `0x01000300` length `0xd710`, SRAM data `0x601712a8` → `0x20000000`
length `0x560`, as its start code does) and every write to the voice DSP's parameter window (`0x4016c000`, 16 bytes
per parameter, the value at +4) is recorded:

| table | from |
|---|---|
| `tb03_init[0x40]` | the voice object's constructor `0x6001da18` and init `0x6001e138`: every parameter's start value |
| `tb03_tune`, `tb03_cutoff`, `tb03_resonance`, `tb03_envmod`, `tb03_accent` (256), `tb03_decay` (128) | the knob handlers (dispatcher `0x6001d520`) called with every knob value: TUNE `0x6001e108` → `C010`, CUTOFF `0x6001defe` → `C019`, RESONANCE `0x6001ded0` → `C01A`, ENV MOD `0x6001de20` → `C01B`, DECAY `0x6001de86` → `C00D`, ACCENT `0x6001ddf2` → `C01C` |

## 4. Panel and inputs (`tb03.c`)

| TB-03 | SYSTEM-1 | parameter |
|---|---|---|
| CUTOFF | FILTER CUTOFF | `C019` = `tb03_cutoff[knob]` |
| RESONANCE | FILTER RESONANCE | `C01A` |
| ENV MOD | FILTER ENV AMOUNT (0…255, not bipolar) | `C01B` |
| DECAY | FILTER DECAY | `C00D` = `tb03_decay[knob >> 1]` |
| ACCENT | OSC1 COLOR | `C01C` |
| TUNE | OSC2 FINE (128 = in tune, ±1 octave) | `C010` |
| WAVEFORM | OSC1 WAVE (position 0 saw, 1…5 square) | `C029` / `C02A` = 1 / 0 or 0 / 1 |

| input | value |
|---|---|
| `A003` | 1 (the TB-03 init) |
| `A004` | pitch in octaves, `s1_pitch()` ((note − 12) / 12) |
| `A005` gate, `A006` accent | the gate word with `GATE_ACCENT=1`: 0 off, 1 on, 2 on with accent (velocity ≥ 100, as the TB-03's ARM `0x6001d490`) |
| `A007` slide | a new note while the gate stayed on (legato, `MONO_LEGATO=1`: the host keeps the gate on and changes the pitch), until the gate closes (`0x6001e376`) |
| `A104…A109` | 0: with the TB-03's parameters they do not change the output (measured on the lift) |

Gate, accent and slide are made from 0 / 1 arithmetic (`s1_sign`, `fminf`, `fmaxf`), not compares: a compare the
profiling never sees flip would be folded by s1cc.

## 5. Not ported

The second DSP (overdrive, delay, the output stage) and the sequencer. DSP B returns `A100 × TB03_GAIN` (12).

## 6. Build, check, install

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/tb03_ported/tb03.c -o sdk/s1cc/build/tb03 --check
KERNEL=sdk/s1cc/build/tb03 VOICES=1 sh examples/system_s1cc/build.sh
python3 tools/plugout_tool.py install plugins/TB-03-S1 --slot 1 --force --no-patches -y
```

`S1_BUILD("NAME=TB-03-S1 MONO_LEGATO=1 GATE_ACCENT=1 MIX=0")`, `S1_COMPILE("--no-fused --regs 0,1,2,3,4,5,6,7")`.
The s1cc driver finds `dsplift_k.h` (`sdk/s1cc/tools`) by itself.

| check [sim, 2026-11-11] | result |
|---|---|
| `s1cc check` (note 57, knobs 128) | DSP A 479 instructions, DSP B 438 (budgets 703 / 708); B against the native C: 68.4 dB (first 10 ms), 48.7 dB (all), bands ±0.00 dB |
| port against the lifted program (`dsplift.py --bq2`, doubles), sustained notes | 76–80 dB |
| port against the lifted program, 16-step sequences with accent and slide | 34–48 dB, bands within ±0.1 dB; the rest is the oscillator edges (one sample earlier or later under float rounding) |
| exports | the values handed A → B arrive one sample late (the slave ring): a pure one-sample delay of part B |

On the device: earlier builds installed and played (slot 1); the build with the envelope fix (s1cc `mulLV`,
2026-11-11: DECAY and ENV MOD had no effect before) is not heard on the device yet.
