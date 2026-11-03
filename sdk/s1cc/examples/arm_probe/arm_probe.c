/* arm_probe.c: the host's envelopes and LFO made audible, to measure them by USB capture
 * (docs/examples_system_s1cc_README.md §3.7). Three fixed tones on DSP B, each with the amplitude of one host signal:
 *   1000 Hz  x s1_env(0)                    AMP envelope
 *   2200 Hz  x s1_env(1)                    FILTER envelope
 *   4100 Hz  x (0.5 + s1_lfo_filter() / 8)  the LFO (LFO TO FILTER, +-4 octaves -> 0..1)
 * The note's pitch is not used. Capture: arecord -D hw:SYSTEM1,0 -f S32_LE -r 96000 -c 2; demodulate each tone
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/arm_probe/arm_probe.c --regs 0,1,2,3,4,5,6,7
 *         ARM_ENV=1 ARM_LFO=1 NAME=ARM-PROBE KERNEL=sdk/s1cc/build/arm_probe sh examples/system_s1cc/build.sh */
#include "s1dsp.h"

static float p1, p2, p3;                 /* DSP B: the three phases, -1..1 */

S1_DSP_A {
    return s1_gate() * 0.0f;             /* nothing: the probe is on DSP B */
}

S1_DSP_B {
    p1 = s1_wrap(p1 + 2.0f * 1000.0f / 96000.0f);
    p2 = s1_wrap(p2 + 2.0f * 2200.0f / 96000.0f);
    p3 = s1_wrap(p3 + 2.0f * 4100.0f / 96000.0f);
    float lfo = 0.5f + s1_range(s1_lfo_filter(), -4.0f, 4.0f) * 0.125f;
    return 0.25f * (s1_tri(p1) * s1_env(0) + s1_tri(p2) * s1_env(1) + s1_tri(p3) * lfo) + 0.0f * x;
}
