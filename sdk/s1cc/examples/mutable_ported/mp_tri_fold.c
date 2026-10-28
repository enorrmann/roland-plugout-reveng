/* mp_tri_fold.c: Braids TRIANGLE_FOLD: a triangle through a wavefolder (the DSP's own triangle op does the folding).
 * OSC1 COLOR = fold gain 1 .. 5, OSC1 MIX = level. DSP B: mp_voice.h. */
#include "mp_voice.h"

S1_PARAM(gain, S1_OSC1_COLOR, 1) { out[0] = 1.0f + 4.0f * knob / 255.0f; }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float p = mp_advance(&ph, mp_dt());
  return mp_fold(s1_tri(p + p) * s1_param(gain, 0)) * s1_param(level, 0);
}
