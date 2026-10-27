/* simple_saw_wrapped.c: the wrapper between the plain C sawtooth (saw.c) and an s1cc voice
 * (docs/examples_simple_saw_wrapped_README.md). The only file that includes the SDK.
 *
 *   S1_PARAM   OSC1 COLOR (S1_OSC1_COLOR, s1panel.h) -> saw level, a table computed on the PC for knob 0..255
 *   S1_INIT    the library's init: runs on the PC; its values are the voice's start on the DSP too
 *   S1_VOICE   slave DSP A, every sample: pitch -> frequency, one sample of the library, x level x gate
 *
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_wrapped.c \
 *             sdk/s1cc/examples/simple_saw_wrapped/saw.c -I sdk/s1cc/examples/simple_saw_wrapped --check
 * In the synth: KERNEL=sdk/s1cc/build/simple_saw_wrapped sh examples/mutable_system/build.sh */
#include "s1dsp.h"
#include "saw.h"

S1_PARAM(level, S1_OSC1_COLOR, 1) { out[0] = knob / 255.0f; }

static struct saw osc;                  /* the library's state: becomes the voice's state */

S1_INIT {
    saw_init(&osc);
}

S1_VOICE {
    float hz = 440.0f * s1_exp2(s1_pitch() - 4.75f);   /* s1_pitch: octaves, 4.75 = A 440 */
    saw_set_frequency(&osc, hz, 96000.0f);             /* the DSP runs at 96 kHz */
    return saw_process(&osc) * s1_param(level, 0) * s1_gate();
}
