/* wt_slave1k.c: the slave wavetable with one frame in a bank-0 loop of DSP A (docs/vol6_reference_designs_applications/mutable_memory.md §4.1,
 * SL_WT=b0 SL_MORPH=0): OSC1 COLOR steps through the frames (region 1 writes the nearest frame of the whole table into
 * the loop; no mix of frames). Default WT-SLAVE768 (768 points, WORKS 2026-11-05). 1024 points do not fit: DSP A's
 * bank 0 has 1024 words, wrapping at 0x0400 (WT-ALIAS); WT-SLAVE1K (loop to 0x0450) failed. Variants by environment:
 * NAME=WT-SLAVE512 SL_L=512 A_BLOCK0=0x0300 SL_KOUT=0x0330 SL_P=0x0340 SL_C=0x0350 (works).
 * DSP B: filter + AMP envelope of mp_dsp_b.h.
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/wavetable/wt_slave1k.c
 *         KERNEL=sdk/s1cc/build/wt_slave1k sh examples/system_s1cc/build.sh */
#include "../mutable_ported/include/mp_dsp_a.h"
#include "../mutable_ported/include/mp_dsp_b.h"

S1_DSP_A {
  return 0.0f * s1_gate();
}

S1_BUILD("NAME=WT-SLAVE768 SL_WT=b0 SL_MORPH=0 SL_L=768 SL_S=0x0040 A_BLOCK0=0x0360 SL_KOUT=0x0390 SL_P=0x03A0 "
         "SL_C=0x03B0 MIX=0");
S1_COMPILE("--regs 0,1,2,3 --no-fused");
