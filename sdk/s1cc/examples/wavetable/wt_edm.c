/* wt_edm.c: WT-EDM, the whole wavetable in the delay memory of master B, one voice per slave voice
 * (docs/vol6_reference_designs_applications/mutable_memory.md §4.4): master B reads each voice's sample (one tapreq per voice, OSC1 COLOR = the nearest
 * frame) and sends it to the slave by 600E + v (SL_WT=m); the slave's DSP B filters and amplifies it (mp_dsp_b.h:
 * FILTER CUTOFF / RESONANCE / ENV, AMP A D S R). 6 voices (the measured limit of 6 tapreq per sample). The first notes
 * fill the table (Sys8.wav, 1.96 s; each note blocks the OS <= 500 ms until it is full).
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/wavetable/wt_edm.c
 *         KERNEL=sdk/s1cc/build/wt_edm sh examples/system_s1cc/build.sh */
#include "../mutable_ported/include/mp_dsp_a.h"
#include "../mutable_ported/include/mp_dsp_b.h"

S1_DSP_A {
  return 0.0f * s1_gate();
}

S1_BUILD("NAME=WT-EDM EDM_WT=1 WT_SLAVE=1 SL_WT=m MIX=0");
S1_COMPILE("--regs 0,1,2,3 --no-fused");
