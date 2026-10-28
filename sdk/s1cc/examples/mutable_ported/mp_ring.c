/* mp_ring.c: ring modulation of two sines (Braids TRIPLE_RING_MOD with two, Warps' analog ring mod in its pure form).
 * OSC1 COLOR = ratio of the second sine 0.5 .. 8, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float p1, p2;

S1_DSP_A {
  float dt = mp_dt();
  float a = mp_sin(mp_advance(&p1, dt));
  float b = mp_sin(mp_advance(&p2, mp_step(dt, s1_exp2(4.0f * s1_color(0) - 1.0f))));
  return a * b * s1_param(level, 0);
}
