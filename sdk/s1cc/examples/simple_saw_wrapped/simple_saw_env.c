/* simple_saw_env.c: two saws and an ADSR, for SYSTEM-S1CC (examples/system_s1cc: the PLUG-OUT that is only the
 * kernel, no host VCA; docs/examples_system_s1cc_README.md). Two plain C libraries, saw.c and env.c; this file only
 * adapts them to the voice, maps the panel as C-S1SY does, and splits the work over the slave's two DSPs:
 *   S1_DSP_A  the saws: OSC1 MIX -> level of saw 1, OSC2 MIX -> level of saw 2, OSC2 FINE -> saw 2 +-1 semitone
 *   S1_DSP_B  the ADSR on x = the saws (AMP ATTACK / DECAY / SUSTAIN / RELEASE)
 *
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_env.c --regs 0,1,2,3,4,5,6,7 --check
 *         (saw.c, env.c by themselves; all 8 registers: SYSTEM-S1CC has no host program that keeps r5 / r6)
 * Synth:  KERNEL=sdk/s1cc/build/simple_saw_env sh examples/system_s1cc/build.sh
 *
 * When each part runs (docs/S1CC.md §3.1): S1_PARAM on the PC (256 knob positions; row 128 until the control moves);
 * S1_INIT on the PC only; S1_DSP_A on the slave's DSP A and S1_DSP_B on its DSP B, every sample, every voice. */
#include <math.h>
#include "s1dsp.h"
#include "saw.h"
#include "env.h"

/* mixer: level of each saw, 0 .. 1 */
S1_PARAM(mix1, S1_OSC1_MIX, 1) { out[0] = knob / 255.0f; }
S1_PARAM(mix2, S1_OSC2_MIX, 1) { out[0] = knob / 255.0f; }

/* OSC2 FINE -> frequency ratio of saw 2: +-1 semitone, 128 = in tune (C-S1SY's FINE_SPAN) */
S1_PARAM(fine2, S1_OSC2_FINE, 1) { out[0] = s1_exp2((knob - 128) / 128.0f / 12.0f); }

/* AMP ATTACK / DECAY / RELEASE: time 1 ms .. 2 s (exponential) -> coefficient; SUSTAIN: level 0 .. 1.
   env_coefficient runs here, on the PC */
static float env_time(int knob) { return 0.001f * powf(2000.0f, knob / 255.0f); }
S1_PARAM(attack, S1_AMP_ATTACK, 1) { out[0] = env_coefficient(env_time(knob), 96000.0f); }
S1_PARAM(decay, S1_AMP_DECAY, 1) { out[0] = env_coefficient(env_time(knob), 96000.0f); }
S1_PARAM(sustain, S1_AMP_SUSTAIN, 1) { out[0] = knob / 255.0f; }
S1_PARAM(release, S1_AMP_RELEASE, 1) { out[0] = env_coefficient(env_time(knob), 96000.0f); }


static struct saw osc1, osc2;           /* DSP A's state */
static struct env amp;                  /* DSP B's state (a state belongs to one of the two) */
static float last_trig;                 /* s1_trig() toggles at every note start (engine.c) */

S1_INIT {
    saw_init(&osc1);
    saw_init(&osc2);
    env_init(&amp);
    last_trig = 0.0f;
}

S1_DSP_A {
    float hz = 440.0f * s1_exp2(s1_pitch() - 4.75f);
    saw_set_frequency(&osc1, hz, 96000.0f);
    saw_set_frequency(&osc2, hz * s1_param(fine2, 0), 96000.0f);
    return 0.5f * (saw_process(&osc1) * s1_param(mix1, 0) + saw_process(&osc2) * s1_param(mix2, 0));
}

S1_DSP_B {                              /* x = the saws of DSP A */
    float trig = s1_trig();
    float start = trig != last_trig ? 1.0f : 0.0f;          /* a note starts */
    last_trig = trig;
    struct env_params p = { s1_param(attack, 0), s1_param(decay, 0), s1_param(release, 0), s1_param(sustain, 0) };
    return x * env_process(&amp, s1_gate(), start, &p);
}
