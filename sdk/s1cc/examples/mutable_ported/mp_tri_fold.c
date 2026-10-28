/* mp_tri_fold.c: Braids TRIANGLE_FOLD: a triangle through a wavefolder (the DSP's own triangle op does the folding).
 * OSC1 COLOR = fold gain 1 .. 5, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float p = mp_advance(&ph, mp_dt());
  return mp_fold(s1_tri(p + p) * (1.0f + 4.0f * s1_color(0))) * s1_param(level, 0);
}
