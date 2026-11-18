# jx3p_fx — the JX-3P CHORUS as a master effect on the CRUSHER knob

**JX-3P-S1-FX** = the JX-3P port of `../jx3p_ported` (its 12 wave-pair voice kernels, unchanged) plus the chorus of
the S8 JX-3P PLUG-OUT (EFFECT TYPE 2 "JX-3P CHORUS", Roland's DSP4 slot program `fx_ch`, `jx3p_ported/README.md` §8)
running **once per sample on the voices' mix** on master DSP B, after the voices and before the OS delay / reverb.
The slaves are not touched: **8 voices**. The CRUSHER knob is the chorus level.

`../jx3p_ported` stays the reference without the effect (its `make_waves.sh` builds JX-3P-S1, CRUSHER off).

Tags: [code] read from Roland's program / engine; [measured] on the device or by capture; [sim] emulator against
the native C; [inferred] reasoning, not checked.

## 1. Build and install

```sh
sh sdk/s1cc/examples/jx3p_ported/tools/make_waves.sh     # once: the 12 pair kernels (sdk/s1cc/build/jx3p_wAB)
sh sdk/s1cc/examples/jx3p_fx/make_fx.sh                  # -> plugins/JX-3P-S1-FX (12 pairs)
ONE_PAIR=1 sh sdk/s1cc/examples/jx3p_fx/make_fx.sh JX-3P-S1-FXT   # tests: the SAW / SAW pair only (fast)
python3 tools/plugout_tool.py install plugins/JX-3P-S1-FX --slot 1 --force --no-patches -y
```

`make_fx.sh`: builds the effect kernel (`s1cc build jx3p_fx.c -o sdk/s1cc/build/jx3p_fx`), then
`examples/system_s1cc_boutique/build_multi.sh` with the same variables as `make_waves.sh` plus
`FX_KERNEL=sdk/s1cc/build/jx3p_fx` and `PANEL=jx3p_fx_waves.panel`. `ONE_PAIR=1` passes the kernel as
`KERNELS="0,0:sdk/s1cc/build/jx3p_w00"` (with `SECT_SWITCH=1` a single kernel must be in the pair form, or `multi.c`
does not compile: no `kmap2`).

| file | what |
|---|---|
| `jx3p_fx.c` | the effect kernel: `S1_MASTER` only (`S1_DSP_A` = the profile's test signal, a saw chord, never run) |
| `jx3p_fx_waves.panel` | `jx3p_ported/jx3p_waves.panel` with CRUSHER lit |
| `make_fx.sh` | the build (§1), the build parameters (§3) |
| `fx_sim.py` | the master program on the emulator against the native C (§6) |
| `fx_changes.patch`, `files/` | the saved copy of 2026-11-18 (§8) |

## 2. The knob and the mix

| | |
|---|---|
| knob | CRUSHER = patch word `0x18C` (`S1_CRUSHER`, measured with C-S1SY 2026-10-23), the word Roland's PLUG-OUT layout uses for EFFECT LEVEL: Roland's JX-3P presets bring their chorus level [code] |
| wet gain | `jxc_level[knob] x 1.1 x JXF_WET` (Roland's EFFECT LEVEL curve, `C1F2`, 0 .. 0.9; x 1.1 in the program) [code] |
| dry gain | `1 - (1 - JXF_DRY) knob / 255` (Roland: always 1, `C1F1 C1F3 = 1`) [code] |
| out L / R | `dry x + wet (chorus_L / R + hiss_L / R)`, x VOLUME -> the OS delay / reverb sends `2000` / `2002` |

With the defaults (all build parameters 1) this is Roland's mix: at full CRUSHER wet 0.99, dry 1, about 50 / 50 [code].

## 3. Build parameters

`make_fx.sh` takes them from the environment; any of them set, it writes `sdk/s1cc/build/jx3p_fx_src/jx3p_fx_cfg.c`
(their `#define JXF_*`, then `#include` of `jx3p_fx.c`, as `make_waves.sh` makes the pairs) and builds that. Unset = 1
= Roland's.

| variable | macro | effect | range |
|---|---|---|---|
| `FX_WET` | `JXF_WET` | x the wet gain (and the hiss's) | any |
| `FX_DRY` | `JXF_DRY` | the dry gain at full CRUSHER, linear from 1 at CRUSHER 0 | 0 .. 1 |
| `FX_DEPTH` | `JXF_DEPTH` | x the LFO's depth on the BBD clock (`C1F4`): 1 -> delay 1.80 .. 7.03 ms, 2 -> 1.80 .. 12.3 ms | 0 .. 4 (`_Static_assert`) |
| `FX_RATE` | `JXF_RATE` | x the LFO's rate (`C1F5`, 0.463 Hz) | 0 .. 16 (`_Static_assert`) |

```sh
FX_WET=1.5 FX_DEPTH=1.5 ONE_PAIR=1 sh sdk/s1cc/examples/jx3p_fx/make_fx.sh JX-3P-S1-FXT
```

## 4. The effect (`jx3p_fx.c`)

The chorus of `jx3p_ported/jx3p_chorus.h` (the native model from Roland's listing and engine words), written with
what runs on master B.

| stage | what | source |
|---|---|---|
| input | `m = x` (the mix, L = R on 6006); two biquad lowpasses `0.07921255 (1, 2, 1) / (1, -1.21644402, 0.533294)` (12.4 kHz, the BBD's anti-alias); highpass `0.999019 (x - x1) + 0.9980383 y1` (~30 Hz) | [code] instr. 26..34 |
| lines | `m` into delay-memory lines 0 (L) and 1 (R), one request each per sample | |
| LFO | `p = ph_wrap(p + C1F5 RATE + d)` in [-1, 1); `d = -2^-21 sign(p)`; `tri_L = 1 - |p|`, `tri_R = |p|` (`|p| = p sign(p)`): triangle 0.463 Hz, L / R antiphase | [code] instr. 35..43; the sign of `d` [inferred] |
| delay | `384 (1.30823231 C1F4 DEPTH tri + 0.449236)` samples (384 BBD clocks of `1 / x` samples); read by the request with its own fraction (`s1_line_tap(l, d, 2)`, the hardware's 1/64, as Roland's program reads `[4000]..[4005]`) | [code] instr. 46..74, 88..306; one interpolated line instead of the clocked BBD [inferred: its aliasing is not modelled] |
| output stage | two one-poles of `C1F7 = 0.23046267` per side | [inferred]: one instruction (288 / 301) not decoded |
| hiss | `s1_fx_noise()` (white, -1 .. 1) -> highpass `0.8582945 (u - u1) + 0.716599 y1` (~5.1 kHz) -> biquad `0.0489201 (1, 2, 1) / (1, -1.4190722, 0.614753)` (9.5 kHz) x `C1F6 (tri + 0.3)` per side, added to the wet | [code] instr. 42..46, 75..87; the filters' order and the noise's level [inferred] |
| hiss level | `C1F6 = jxc_tone[128]`: EFFECT TONE fixed at 128, the value Roland's JX-3P chorus presets use most (26 of the 105 EFFECT TYPE 2 patches of 1 / 2 Preset.bin); 10 (Roland's lowest) was not audible on the device | [code] + [measured] |
| not modelled | the fade-in after an effect-type change (instr. 10..17) | |

The hiss's level follows `tri + 0.3`: per side a smooth dip of about -8 dB at the bottom of the triangle, every
2.16 s, L and R 1.08 s apart [measured: capture `work/captures/jx3p_fx_cut_60s.wav`, no notes, CRUSHER up]. That is
Roland's program, not a fault.

Cost: 179 s1cc units; master B (`s1mm1`) **204 instructions** with Roland's values, 208 with build parameters
(481 ran on the device, 640 hung). Lines at `0x440000` / `0x444000` (`EDM_LINES`'s), 2 requests per sample (6 ran).

### 4.1 What master B allows, and how the kernel keeps to it

| need | how | why |
|---|---|---|
| phase wrap | `s1_wrap` -> `ph_wrap rX, r3, rX` | the form of Roland's chorus (instr. 38) and of the OS's own reverb on this master B (docs Vol. I ch. 3 §16.3). A wrap made with `q - (1 + sign(q - 1))` jumped (§7) |
| sign | `sign rX, rX` only | `sign rD, rX` gave 0 for negative inputs (MP-EDMWT9, `asm/master_b_forms.txt`) |
| floor of the delay | none: the request's own fraction | no floor form ran on master B |
| constants | master parameters `{K:value}` (s1cc), written by region 0 | non-round immediates run wrong on master B |
| profile | `S1_COMPILE("--no-range-fold")` | the profile's notes are shorter than an LFO period: it never sees a wrap nor a negative phase and would fold the LFO's signs to constants |

`ld2` / `st2` (two transfers, no op, in one instruction): the build warns that no program that ran has this shape.
MP-PLSTRC has the same shapes and plays, but whether that proves them is doubtful: **not promoted**.

## 5. What the effect needed outside this folder

All of it does nothing in a build without `FX_KERNEL` [code; the slave programs of a build with and without it differ
only by the noise store below: diff of the generated programs, 2026-11-18].

| where | what |
|---|---|
| s1cc `include/s1dsp.h`, `src/graph.cpp`, `lower.cpp`, `s1cc.h`, `runtime/s1rt.c` | `s1_out_r(y)`: the effect's right output (store `{OUTR}`; the return value is the left). `s1_fx_noise()`: input `{IN:noise}` (range -1 .. 1). `s1_line_tap(line, d, 2)`: the request's fraction `{LT:l:2}` (native: `floor(64 frac(d)) / 64`). Error E128 for both outside `S1_MASTER` |
| s1cc `src/kernel.cpp` | master B: `sign` and `ph_wrap` always in the same-register form (`mov rD, rX` first) |
| `include/s1panel.h` | `S1_CRUSHER 0x18C` |
| `examples/system_s1cc_boutique/make_s1cc.py` | option `FX_KERNEL=dir` (MFX): the effect's kernel read beside the voice's (`MK`, `MTAB`); the KM frame with one line voice (`STR_NV 1`, lines = the effect's); `master_fx` (the effect once on `6006`, then `master_fx_sum`); the request block `edm_km_block`; the effect's singles on master-B parameter ids that ran (`C01D`, `C01E`, `C038..C03D`, then `C020..C025`, `C030..C035`, `C040..C045`, `C048..C04D`, `C028..C02D`: the per-voice ids MP-PLSTRING wrote); `#define MASTER_FX 1` in `s1_programs.h`; `IN:noise` = `6008`; 6008 out of `OUT_ZERO` |
| `microcode/edm_str.s` | fragment `master_fx_sum`: left / right x VOLUME (`C01C`) -> `2000` / `2002`, the effect's return -> `A118` / `A119` |
| `microcode/host_b.s` | `head_b`: `st r2, [6008]` after the host noise (`MFX_NOISE`): slave B's noise source to the master (master B has no proven op for a noise of its own) |
| `build.sh`, `build_multi.sh` | region 0 with `-DEDM_LINES` when the build has `MASTER_FX` |
| `model.c` | `MASTER_FX` frame (the lines code counts `STR_NV`, the assigner keeps NV; no gate writes); `edm_call`: nothing written when its site and block are already this build's (§7); `edm_str` only at the first load after selection (`fx_loaded`), a reload only `str_bright`; `km_param` (`MASTER_FX`): the effect's knob rewrites **all** its parameters and constants (§7) |

## 6. Simulation

`fx_sim.py`: the built `s1mm1.s` on the C emulator with the bank-0 ring, the lines (request ages from bank 0 one
sample late, `x0`, `x1`, fraction 1/64) and the master parameters as region 0 writes them; input = the native
reference's test signal, noise = its sequence; output L / R against `ref.raw.m` / `ref.raw.r`.

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/jx3p_fx/jx3p_fx.c -o sdk/s1cc/build/jx3p_fx
E="KERNEL=sdk/s1cc/build/jx3p_w00 FX_KERNEL=sdk/s1cc/build/jx3p_fx VOICES=8 SECT_N=2 ARM_WORDS=1 VOICE_SECT_TEST=1"
env $E OUTDIR=DIR python3 examples/system_s1cc_boutique/make_s1cc.py
env $E python3 sdk/s1cc/examples/jx3p_fx/fx_sim.py DIR 20000 fx_level=255
```

| build | samples | L / R SNR |
|---|---|---|
| sign-made wrap, Roland's values | 20 000 | 69.4 / 67.4 dB |
| `ph_wrap`, Roland's values | 260 000 (2.7 s, through the wraps) | 67 dB falling slowly to ~35 dB: no step at the wraps; the LFO phase drifts from the C's (the accumulator's precision) |
| `ph_wrap`, WET 1.5, DEPTH 1.5 | 20 000 | 55.1 / 53.4 dB |

## 7. Log on the device (2026-11-18, frozen)

| install | result | cause / fix |
|---|---|---|
| first JX-3P-S1 with the effect | CRUSHER did nothing, a patch change hung | after reinstalling the same build it played (mild); the state of the first install [inferred] |
| same | the effect only sometimes, hangs at patch changes | the OS calls `load` at every patch change; `edm_call` rewrote the live call site in the OS effect (three words, not atomic for the DSP) every time [code]. Fix: nothing written when site and block are already there; `edm_str` once per selection |
| + EFFECT TONE 128 | the chorus and its hiss only after some notes | the parameters written at load were lost when the master program landed after it; only a note of voice 0 wrote them again [inferred from the symptom]. Fix: the effect's knob (sent again by the OS after load and at every patch change) rewrites all of them |
| 18:45 build | **chorus present right at selection** (user) | |
| user: "the envelopes changed" | not the effect: String 1 (1-1) has VCA MODE ENV-1 (the VCA follows the FILTER faders); A/B with JX-3P-S1-NOFX and HEAD (JX-3P-S1-W00) | 40 / 64 presets of 1 Preset.bin use ENV-1; for the AMP faders: 5-3, 7-6 (ENV-2) |
| JX-3P-S1-FX (12 pairs) | plays, no hang; a periodic cut | user screenshot `saltos.jpg`; capture: L / R envelope flips every 14.97 s = the LFO jumping half a cycle: `sign(q - 1)` gave 0 just above 1, `q` fell on 0 instead of -1 [measured + inferred]. A min-only wrap compiled to `shl` (never ran on master B): dropped. Fix: `ph_wrap` |
| JX-3P-S1-FXT (`ph_wrap`, SAW / SAW) | **OK** (user) | |
| JX-3P-S1-FXT, WET 1.5, DEPTH 1.5 | installed 2026-11-18, not heard yet | |
| JX-3P-S1-FX (12 pairs), WET 1.5, DEPTH 1.5 | installed 2026-11-18 (master B 208), not heard yet | |

## 8. Saved copy (2026-11-18, before the tree was reset to HEAD 7416c25)

| path | what |
|---|---|
| `fx_changes.patch` | the diff of the tracked files against HEAD 7416c25 at that time (includes `jx3p_ported` hunks that were NOT applied again: panel CRUSHER on, `make_waves.sh` FX default) |
| `files/<repo path>` | the same files whole as they were, plus the kernel and the simulator where they first were (`jx3p_ported/`, `edm/`) |

Since then (not in the copy): the `ph_wrap` wrap (`jx3p_fx.c`, s1cc `kernel.cpp` `wrap`), the build parameters, this
folder's `make_fx.sh` / panel / `fx_sim.py`.

## 9. Open

* A capture of the S8's own JX-3P CHORUS to compare with (level, hiss, the output stage).
* The output stage's undecoded instruction; the hiss's filter order and noise level.
* `ld2` / `st2` shapes on master B (§4.1).
* Docs outside: `jx3p_ported/README.md` §8.4 (where it runs: decided here), s1cc internals §3.5 (`s1_out_r`,
  `s1_fx_noise`, fraction tap), SYSTEM-S1CC-BOUTIQUE README (`FX_KERNEL`), `docs/TOOLS.md` (`fx_sim.py`).
