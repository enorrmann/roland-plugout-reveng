/* simple_saw_hostenv.c: two saws, a low-pass and the host's two envelopes, for SYSTEM-S1CC
 * (docs/examples_simple_saw_wrapped_README.md §7, docs/examples_system_s1cc_README.md §3.1). The kernel has no envelope
 * of its own: SYSTEM-S1CC builds the ADSRs in assembly on DSP B (C-S1SY's, proven on the device) as soon as a kernel
 * calls s1_env(), and the kernel only reads them, as a synth uses its two envelopes:
 *   s1_env(0)  AMP ATTACK / DECAY / SUSTAIN / RELEASE      -> the level of the voice (the VCA)
 *   s1_env(1)  FILTER ATTACK / DECAY / SUSTAIN / RELEASE   -> the cutoff of the low-pass (FILTER ENV AMOUNT octaves)
 *
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_hostenv.c --regs 0,1,2,3,4,5,6,7 --check
 *         (saw.c and filter.c by themselves: the sources next to saw.h and filter.h)
 * Synth:  KERNEL=sdk/s1cc/build/simple_saw_hostenv NAME=S1CC-HOSTENV sh examples/system_s1cc/build.sh
 *         (ENV_TMIN / ENV_TMAX: the envelope times at fader 0 / 255, default 0.001 / 10 s)
 *
 * One voice, one sample, on the slave chip (the code below is copied once per voice, 8 voices):
 *   DSP A   S1_DSP_A: the two saws, mixed -> bank 5 A151+v = x of DSP B.
 *           Host (make_s1cc.py, because the kernel uses s1_env): gate A038+v -> A160+v, trigger A060+v -> A168+v,
 *           the copies DSP B reads (in C-S1SY, DSP B reading what the ARM wrote gave silence).
 *   DSP B   Host first: one ADSR in assembly per envelope the kernel reads (here both), reading those copies; each
 *           leaves its level 0..1 in a bank-0 word of the voice's block.
 *           Then S1_DSP_B: x, s1_env(0) and s1_env(1) (those words) -> low-pass -> x env 0 -> the voice's output word;
 *           the mix adds the 8 voices -> master -> VOLUME, delay, reverb.
 * No s1_export / s1_import: the envelopes and the filter act on the whole voice, which x brings from A. A value that
 * B needs from A besides the signal: simple_saw_export.c.
 *
 * s1_env(i): only in S1_DSP_B (E117), i = 0 or 1 (E118). The faders go to DSP B parameters C040.. / C044.. (engine.c);
 * defaults until a fader moves: AMP S 255 / A 0 / D 128 / R 60, FILTER S 0 / A 0 / D 128 / R 60. In the native
 * reference and in `s1cc check` s1_env() is the gate (s1rt.c): the kernel is verified with square envelopes, the real
 * ones on the whole chain (make_s1cc.py --sim, by shape). The ADSR follows the gate itself (attack = gate - latch): a
 * trigger and a gate that reach the DSP samples apart lose nothing (the trap env.c fell into, README §6). */
#include <math.h>
#include "s1dsp.h"
#include "saw.h"
#include "filter.h"

/* the kernel's own panel: S1_PARAM tables, run on the PC, rows written by the ARM (DSP A C070.., DSP B C060..) */
S1_PARAM(mix1, S1_OSC1_MIX, 1) { out[0] = knob / 255.0f; }
S1_PARAM(mix2, S1_OSC2_MIX, 1) { out[0] = knob / 255.0f; }
S1_PARAM(fine2, S1_OSC2_FINE, 1) { out[0] = s1_exp2((knob - 128) / 128.0f / 12.0f); }
/* FILTER CUTOFF -> coefficient of the low-pass, 20 Hz .. 20 kHz (exponential); lowpass_coefficient runs on the PC */
S1_PARAM(cutoff, S1_FILTER_CUTOFF, 1) { out[0] = lowpass_coefficient(20.0f * powf(1000.0f, knob / 255.0f), 96000.0f); }
/* FILTER ENV AMOUNT -> octaves the filter envelope moves the cutoff: -8 .. +8, 128 = none (bipolar, as C-S1SY) */
S1_PARAM(env_amount, S1_FILTER_ENV_AMOUNT, 1) { out[0] = (knob - 128) / 128.0f * 8.0f; }

static struct saw osc1, osc2;            /* DSP A's state */
static struct lowpass lp;                /* DSP B's state; the envelopes' state is the host's */

S1_INIT {
    saw_init(&osc1);
    saw_init(&osc2);
    lowpass_init(&lp);
}

S1_DSP_A {
    float hz = 440.0f * s1_exp2(s1_pitch() - 4.75f);
    saw_set_frequency(&osc1, hz, 96000.0f);
    saw_set_frequency(&osc2, hz * s1_param(fine2, 0), 96000.0f);
    return 0.5f * (saw_process(&osc1) * s1_param(mix1, 0) + saw_process(&osc2) * s1_param(mix2, 0));
}

S1_DSP_B {                               /* x = the two saws, from DSP A */
    /* filter envelope: the cutoff moves env_amount octaves at the top of the envelope (g <= 0.9: stable) */
    float g = fminf(s1_param(cutoff, 0) * s1_exp2(s1_env(1) * s1_param(env_amount, 0)), 0.9f);
    return lowpass_process(&lp, x, g) * s1_env(0);       /* amp envelope: the VCA */
}
