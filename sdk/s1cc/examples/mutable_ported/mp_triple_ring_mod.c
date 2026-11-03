/* mp_triple_ring_mod.c: Braids TRIPLE_RING_MOD: three sines (the note and two ratios) multiplied together, then a
 * moderate overdrive (Braids' ws_moderate_overdrive, here a cubic soft clip).
 * OSC1 COLOR = pitch of sine 2 (-1 .. +2 octaves), OSC2 COLOR = pitch of sine 3, OSC1 MIX = level.
 * DSP B: mp_dsp_b.h. The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph1, ph2, ph3;

S1_DSP_A {
  float dt = mp_dt();
  float a = mp_sin(mp_advance(&ph1, dt));
  float b = mp_sin(mp_advance(&ph2, mp_step(dt, s1_exp2(3.0f * s1_color(0) - 1.0f))));
  float c = mp_sin(mp_advance(&ph3, mp_step(dt, s1_exp2(3.0f * s1_color(1) - 1.0f))));
  float x = 1.4f * a * b * c;
  x = fminf(fmaxf(x, -1.0f), 1.0f);
  return x * (1.5f - 0.5f * x * x) * s1_param(level, 0);
}
