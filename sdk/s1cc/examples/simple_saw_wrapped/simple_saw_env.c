/* simple_saw_env.c: simple_saw_wrapped.c with its own amplitude envelope, for SYSTEM-S1CC
 * (examples/system_s1cc: the PLUG-OUT that is only the kernel, no host VCA; docs/examples_system_s1cc_README.md).
 *
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_env.c --check   (saw.c by itself)
 * Synth:  KERNEL=sdk/s1cc/build/simple_saw_env sh examples/system_s1cc/build.sh
 *
 * When each part runs (docs/S1CC.md §3.1): S1_PARAM on the PC (256 knob positions; row 128 until the control moves);
 * S1_INIT on the PC only; S1_DSP_A on the slave's DSP A, every sample, every voice. */
#include <math.h>
#include "s1dsp.h"
#include "saw.h"

/* OSC1 COLOR -> level, OSC2 COLOR -> detune of the second saw (0 .. +50 cents), as simple_saw_wrapped.c */
S1_PARAM(level, S1_OSC1_COLOR, 1) { out[0] = knob / 255.0f; }
S1_PARAM(detune, S1_OSC2_COLOR, 1) { out[0] = s1_exp2(knob / 255.0f * 0.5f / 12.0f); }

/* AMP ATTACK / RELEASE -> coefficient of a one-pole envelope: time 1 ms .. 2 s (exponential), k = 1 - e^(-1 / t fs) */
static float env_k(int knob) { return 1.0f - expf(-1.0f / (0.001f * powf(2000.0f, knob / 255.0f) * 96000.0f)); }
S1_PARAM(attack, S1_AMP_ATTACK, 1) { out[0] = env_k(knob); }
S1_PARAM(release, S1_AMP_RELEASE, 1) { out[0] = env_k(knob); }

static struct saw osc1, osc2;
static float env;                       /* 0 .. 1, starts at 0 */

S1_INIT {
    saw_init(&osc1);
    saw_init(&osc2);
    env = 0.0f;
}

S1_DSP_A {
    float hz = 440.0f * s1_exp2(s1_pitch() - 4.75f);
    saw_set_frequency(&osc1, hz, 96000.0f);
    saw_set_frequency(&osc2, hz * s1_param(detune, 0), 96000.0f);
    float gate = s1_gate();
    float k = gate > env ? s1_param(attack, 0) : s1_param(release, 0);   /* towards 1 while held, 0 after */
    env += (gate - env) * k;
    float mix = 0.5f * (saw_process(&osc1) + saw_process(&osc2));
    return mix * s1_param(level, 0) * env;
}
