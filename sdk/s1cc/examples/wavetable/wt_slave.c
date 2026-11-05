/* wt_slave.c: the kernel of the slave wavetable tests (docs/MUTABLE_MEMORY.md §4.1): DSP A gives nothing of its own
 * (SYSTEM-S1CC's SL_WT host code writes every voice's wavetable output over it, microcode/sl_wt.s); DSP B is the
 * shared filter + AMP envelope of mp_dsp_b.h (FILTER CUTOFF / RESONANCE / ENV, AMP A D S R).
 * SL_WT=b5 (the frame in bank 5, WORKS with 8 voices 2026-11-05); SL_WT=b0 NAME=WT-SLAVE0 (a bank-0 loop, not run with
 * the rules of docs/MUTABLE_MEMORY.md §4.2).
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/wavetable/wt_slave.c
 *         KERNEL=sdk/s1cc/build/wt_slave sh examples/system_s1cc/build.sh */
#include "../mutable_ported/include/mp_dsp_a.h"
#include "../mutable_ported/include/mp_dsp_b.h"

S1_DSP_A {
  return 0.0f * s1_gate();
}

S1_BUILD("NAME=WT-SLAVE5 SL_WT=b5 MIX=0");
S1_COMPILE("--regs 0,1,2,3 --no-fused");
