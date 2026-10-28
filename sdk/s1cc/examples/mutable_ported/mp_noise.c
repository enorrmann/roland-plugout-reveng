/* mp_noise.c: Braids FILTERED_NOISE: white noise into DSP B's filter, here the resonant band pass (MP_SVF_BP), its
 * centre following the key (set KEY TRACK up), as Braids' noise tuned by the note.
 * OSC1 MIX = level. */
#define MP_FILTER MP_SVF_BP
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float w;

S1_DSP_A { return mp_noise(&w) * s1_param(level, 0); }
