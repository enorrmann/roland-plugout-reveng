/* simple_saw_wrapped.c: the wrapper between the plain C sawtooth (saw.c) and an s1cc voice
 * (docs/examples_simple_saw_wrapped_README.md). The only file that includes the SDK. Two instances of the library
 * per voice: two saws, the second detuned by OSC2 COLOR.
 *
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_wrapped.c --check
 *         (saw.c comes in by itself: it is the source next to saw.h, which this file includes)
 * In the synth: KERNEL=sdk/s1cc/build/simple_saw_wrapped sh examples/mutable_system/build.sh
 *
 * When each part runs (docs/S1CC.md §3.1):
 *   S1_PARAM   on the PC, at build time: once per knob position 0..255 -> a table in the plug-out. On the synth the
 *              ARM writes the row of the knob's position into a DSP parameter: 128 at load, then whenever the
 *              control moves. s1_param() in the voice reads that parameter.
 *   S1_INIT    on the PC only, before the first sample of the native reference / profiling. It never runs on the
 *              DSP: s1cc reads the state it leaves and makes the DSP start from the same values.
 *   S1_DSP_A   on the synth, slave DSP A, every sample (96 kHz), for each of the 8 voices (one copy of the code and
 *              of the state per voice), whether a note is playing or not. On the PC, in the reference and the
 *              profiling. */
#include "s1dsp.h"
#include "saw.h"

/* OSC1 COLOR -> level of the two saws, 0 .. 1 */
S1_PARAM(level, S1_OSC1_COLOR, 1) { out[0] = knob / 255.0f; }

/* OSC2 COLOR -> frequency ratio of the second saw: 0 .. +50 cents (s1_exp2 runs on the PC here, like all the table) */
S1_PARAM(detune, S1_OSC2_COLOR, 1) { out[0] = s1_exp2(knob / 255.0f * 0.5f / 12.0f); }

/* the two instances of the library: their fields (phase, inc) are the voice's state, 4 ring words, 8 copies */
static struct saw osc1, osc2;

S1_INIT {
    saw_init(&osc1);
    saw_init(&osc2);
}

S1_DSP_A {
    float hz = 440.0f * s1_exp2(s1_pitch() - 4.75f);   /* s1_pitch: octaves, 4.75 = A 440; LFO and RANGE included */
    saw_set_frequency(&osc1, hz, 96000.0f);            /* the DSP runs at 96 kHz */
    saw_set_frequency(&osc2, hz * s1_param(detune, 0), 96000.0f);
    float mix = 0.5f * (saw_process(&osc1) + saw_process(&osc2));
    return mix * s1_param(level, 0) * s1_gate();
}
