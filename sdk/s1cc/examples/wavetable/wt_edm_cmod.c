/* wt_edm_cmod.c: WT-EDM-CMOD, WT-EDM (wt_edm.c, docs/MUTABLE_MEMORY.md §4.4) with COLOR MOD per voice: the host makes
 * each voice's OSC1 COLOR with its COLOR MOD source (s1_color(0): knob + LFO / pitch env / filter env / amp env / sub);
 * slave A turns it into the voice's frame and sends it to master B (WT_CMOD, 6008 + v), which reads that voice's
 * frame of the whole table. The kernel's own output is not used (SL_WT=m replaces it); reading s1_color(0) is what
 * makes the host compute the color.
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/wavetable/wt_edm_cmod.c
 *         KERNEL=sdk/s1cc/build/wt_edm_cmod sh examples/system_s1cc/build.sh */
#include "../mutable_ported/include/mp_dsp_a.h"
#include "../mutable_ported/include/mp_dsp_b.h"

S1_DSP_A {
  return 0.001f * s1_color(0) * s1_gate();
}

S1_BUILD("NAME=WT-EDM-CMOD EDM_WT=1 WT_SLAVE=1 WT_CMOD=1 SL_WT=m MIX=0");
S1_COMPILE("--regs 0,1,2,3 --no-fused");
