/* mp_feedback_fm.c: Braids FEEDBACK_FM: a sine modulated by its own previous output (from sine to saw-like).
 * OSC1 COLOR = feedback 0 .. 0.4 cycles, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph, fb1, fb2;

S1_DSP_A {
  float p = mp_advance(&ph, mp_dt());
  float y = mp_sin(p + (0.4f * s1_color(0)) * (fb1 + fb2));    /* two-sample average: tames the feedback's chatter */
  fb2 = fb1;
  fb1 = y;
  return y * s1_param(level, 0);
}
