/* mp_ring.c: ring modulation of two sines (Braids TRIPLE_RING_MOD with two, Warps' analog ring mod in its pure form).
 * OSC1 COLOR = ratio of the second sine 0.5 .. 8, OSC1 MIX = level. DSP B: mp_voice.h. */
#include <math.h>
#include "mp_voice.h"

S1_PARAM(ratio, S1_OSC1_COLOR, 1) { out[0] = 0.5f * powf(16.0f, knob / 255.0f); }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float p1, p2;

S1_DSP_A {
  float dt = mp_dt();
  float a = mp_sin(mp_advance(&p1, dt));
  float b = mp_sin(mp_advance(&p2, mp_step(dt, s1_param(ratio, 0))));
  return a * b * s1_param(level, 0);
}
