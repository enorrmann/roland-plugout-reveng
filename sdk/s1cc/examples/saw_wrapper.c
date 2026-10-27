/* saw_wrapper.c: the whole of the wrapper between a plain C sawtooth (saw_core.c) and an s1cc voice
 * (docs/examples_s1cc_saw_README.md §3). It is the only file that has to include the SDK.
 *
 *   S1_VOICE      slave DSP A, every sample: trigger -> new note, one sample of the library, x gate
 *   S1_INIT       the wrapper's and the library's first values (the native reference starts from them too)
 *   S1_PARAM      OSC1 COLOR (S1_OSC1_COLOR) -> saw level
 *   S1_VOICE_B    not here: DSP B is left free for the filter / envelopes of the host program (C-S1SY)
 *
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/saw_wrapper.c sdk/s1cc/examples/saw_core.c -I sdk/s1cc/examples
 * Check:  sdk/s1cc/s1cc check sdk/s1cc/build/saw_wrapper --note 57
 * In the synth: KERNEL=<build dir> sh examples/mutable_system/build.sh
 *
 * s1cc inlines the library (both files become one program: linked.ll / voice.ll in the build directory), fits the
 * polyBLEP's two corners as functions of the phase (docs/S1CC.md §4.3, §6) and keeps the globals and the struct's
 * fields as the voice's state (one ring word each). The branch and the call in the library are not a problem; what a
 * branch hides from the profiling is the range of the values next to it, and that is what s1_range declares. */
#include <math.h>
#include "s1dsp.h"
#include "saw_core.h"

/* OSC1 COLOR: saw level, 0 .. 1: computed on the PC for knob = 0..255, written to the DSP as a parameter */
S1_PARAM(level, S1_OSC1_COLOR, 1) { out[0] = knob / 255.0f; }

static struct saw voice;                /* the library's state: phase, frequency, differencer */
static float last_trig;                 /* the trigger of the last sample: a change is a new note */
static float trig_now;                  /* this sample's trigger */

S1_INIT {
    last_trig = 0.0f;
    trig_now = 0.0f;
    saw_note_hz = 440.0f;
    voice.hz = 0.0f;
    voice.phase = 0.0f;
    voice.x1 = 0.0f;
}

S1_VOICE {
    trig_now = s1_trig();
    if (trig_now != last_trig) {        /* a new note: the library restarts, at the pitch of this sample */
        last_trig = trig_now;
        saw_note_hz = 440.0f * s1_exp2(s1_pitch() - 4.75f);
        saw_start(&voice);
    }
    return saw_tick(&voice, s1_param(level, 0)) * s1_gate();
}
