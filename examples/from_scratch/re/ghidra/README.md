# Ghidra on the JX-3P VST3 (static analysis only)

The VST3 has no DSP microcode; its voices run in `CJx3pSim`, a native float
model whose parameters are named (see `../vst_sim.py`, NOTES.md "Motor DSP en
el VST3"). Ghidra 11.3.2 (`~/ghidras`) decompiles it; project and output live
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
