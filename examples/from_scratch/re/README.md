> [!NOTE]
> **TRM Technical Reference Manual**: This implementation example is documented normatively in [Volume VI: Reference Designs & Applications](../../../docs/vol6_reference_designs_applications/c_v4_system8_reference.md).

# Ghidra on the JX-3P VST3 (static analysis only)

The VST3 has no DSP microcode; its voices run in `CJx3pSim`, a native float
model whose parameters are named (`examples/from_scratch/re/vst_sim.py params`; the model is described
under "What the model computes" below). Ghidra 11.3.2 (`~/ghidras`) decompiles it; project and output live
in `work/ghidra` (ignored by git: the output is Roland's code, keep it out of
the repository).

```bash
G=~/ghidras/ghidra_11.3.2_PUBLIC/support/analyzeHeadless
# once: import + auto analysis (~5 min), then decompile some functions
$G work/ghidra jx3p_vst -import plugins/JX-3P/JX-3P.vst3 \
   -scriptPath examples/from_scratch/re/ghidra -postScript Decompile.java work/ghidra/out 0x1803b2530
# later: reuse the analysed project
$G work/ghidra jx3p_vst -process JX-3P.vst3 -noanalysis -readOnly \
   -scriptPath examples/from_scratch/re/ghidra -postScript Decompile.java work/ghidra/out <addr> ...
# name the fields (P_<parameter>, st_<offset> = internal state) and helpers
python3 examples/from_scratch/re/ghidra/annotate.py plugins/JX-3P/JX-3P.vst3 \
   work/ghidra/out/0x1803b2530.c work/ghidra/out/voice0.c
```

| address (1.0.7) | |
|---|---|
| 0x1803b2530, 0x1803ba830, ... (8) | per-sample render of voice 0..7 (LFO, ENV1/2, OSC, LPF, AMP) |
| 0x1803aa580 | common part: HPF, effects, delay, reverb, master |
| 0x180391630 | initialisation: constants of the model (two sets, one per sample rate) |
| 0x180382bf0 | parameter registration (names) |
| 0x1803b23f0 `wrap1`, 0x1803b2480 `tri`, 0x1803ba7d0 `lfsr24`, 0x1803aa520 `shl`, 0x1807445f0 `expf` | helpers |

The decompiled render reads like a compiled block diagram: delay chains
(`st_x = st_y` = z^-1), compares and selects, 24-bit noise, `x * 2^n` shifts.
That is what one expects the microcode programs to compute too, which makes it
the reference for the DSP ISA work.

## What the model computes (JX-3P.vst3 1.0.7, static analysis, 2026-10-14)

`plugins/JX-3P/JX-3P.vst3` holds no fragment of the 17 microcode programs, in any byte order. It shares with region 1
the float tables of the engine (more than 100 KB from `0x60A258A0`, `.rdata` `0x9C8744` in the VST) and the control
classes `CDSPJx3p{Env,Lfo,Osc,Flt,Amp}Voice`, `VoiceCmn`, `VoiceMix`, `Mst`, `Noise`, `PatchLev`,
`Efx{Od,Ds,Fz,Cr,Ch,Ph,Cmn}`: the ARM code of the PLUG-OUT, whose methods are a table lookup and a parameter write.
In the VST the writes go to `CJx3pSim`. All nine VST3s are built this way (`CDspSim` and a float model per synth, no
microcode and no interpreter of the DSP ISA), so none of them is a bit-exact reference.

* **Parameters**: 1138 named parameters. Per voice 110 (index stride `0x6E`, field stride `0x3F80`): LFO (Rate,
  Delay, Sin / Sqr / S&H Sw, Trig…), ENV1 / ENV2 (Attack, Decay, Sustain, Release, "LFO trigger env sw"), OSC (Feet,
  Detune, Sync, Pulse Sw, Bend, LFO / Env mod, PWM), LPF (Cutoff, Resonance, Env / LFO / Key / Velocity mod), AMP;
  then the common ones: HPF, effects (OD / DS / FZ / Crusher / Phaser / Chorus / Flanger), delay, reverb, master.
  The constructor of `CDSPJx3pEnvVoice` (`0x180375FC0`, voice 0..7) fixes the indices: voice 0 = `0x1F` (LFO trigger
  env sw), `0x20`–`0x23` (ENV1 A/D/S/R), `0x24`, `0x25`–`0x28` (ENV2); in the ARM code the same methods write
  `P{u}:0x2F + stage (+0x17)` / `0x33 + stage`.
* **Render**: eight functions of about 33 KB, one per voice (about 5900 SSE instructions, each using the 110 fields
  of its voice); helper `0x1803B2270` = piecewise polynomial (a table of `0xD0` bytes per piece).
* **LFO** (per sample): rate 0..1 (or the external input / tempo) → `f = 0.46624·e^(5.28342·rate) − 0.41338`, phase
  increment `f / 65536` (at most 0.015534) × Rate Ratio × a sample-rate factor. Phase in [−1, 1) with wrap. Shapes:
  Sin = polynomial on |phase|, `−1.181x³ + 1.8136x² + 1.3674x − 1`; Sqr = sign of the phase; S&H = a sample of the
  24-bit LFSR noise at each phase wrap. Output = sum of each shape × its switch. Delay: a ramp 0..1 from the note
  that multiplies the output when LFO Delay is on. How LFO Trig / Reset Sw restart the phase is not verified.
* **ENV1 / ENV2** run at 1 kHz (a counter adds 1/48 or 1/88.2 per sample, by sample rate, and fires the stage when it
  wraps to −1). Gate = the voice's gate, or the LFO's square with "LFO trigger env sw"; a rising edge starts the
  attack. Attack is linear: `env += Attack` per tick up to 1.0. Decay is one pole towards `S = Sustain`:
  `env = S + (1 − Decay)(env − S)`. Release: `env = (1 − Release)·env`, floor 0. In decay and release a small drift
  `−1.5e−5·(0.5 − env)` is added while env < 0.5. Attack, Decay and Release are an increment / a coefficient per
  tick, not times.
* **Modulations**: pitch = Detune + Feet + bend × range + ENV1 × Env1 Mod Level (with inversion) × Osc Env Mod Sw +
  LFO × Lfo Mod Level × Osc LFO Mod Sw (+ mod wheel); PWM = the PWMod level; cutoff = LPF Cutoff through a cubic,
  clamped to 0..1, plus the ENV / LFO / Key / Velocity modulations.
* **Oscillators**: pitch − 4.75 → a table (double precision) → increment, limited to ±512; phase in [−1, 1) with
  wrap; **four sub-samples per sample** (positions 0.25 / 0.5 for the sync); smoothed transition, odd polynomial and
  a chain of delays (decimation FIR). The microcode computes the pitch exponential instead of reading a table:
  x = (pitch − 4.75)·ln 2, then e^(2y) with the series 2ⁿ/n! split into two Horner chains run in parallel (even
  terms 1, 2, 2/3, 4/45, 2/315, 4/14175; odd terms, halved, 1, 2/15, 4/315, 2/2835, 2.565e−5); π/2 appears four
  times in SAW / SQR, once per sub-sample. `vst_anchor.py init.c work/corpus/JX_*.bin` searches every constant of
  the VST's initialisation as a RAW immediate in the microcode.

The same constants in the main programs: `docs/vol3_voice_allocation_scheduling/dsp_main_programs.md`.
