/* wt_ctrl.c: control for the slave wavetable tests (docs/vol6_reference_designs_applications/mutable_memory.md §4.2): wt_slave.c's build (DSP B =
 * mp_dsp_b.h filter + AMP envelope, not fused, MIX=0) with a plain saw on DSP A instead of SL_WT: do all 8 voices
 * sound with this chain?
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/wavetable/wt_ctrl.c
 *         KERNEL=sdk/s1cc/build/wt_ctrl sh examples/system_s1cc/build.sh */
#include "../mutable_ported/include/mp_dsp_a.h"
#include "../mutable_ported/include/mp_dsp_b.h"

static float ph;

S1_DSP_A {
  ph = s1_wrap(ph + s1_increment());
  return 0.5f * ph;
}

S1_BUILD("NAME=WT-CTRL MIX=0");
S1_COMPILE("--regs 0,1,2,3 --no-fused");
