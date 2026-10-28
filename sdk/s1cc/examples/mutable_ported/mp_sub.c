/* mp_sub.c: Braids SAW_SUB / SQUARE_SUB: a band-limited saw with a square one octave below.
 * OSC1 COLOR = sub level, OSC1 MIX = level. DSP B: mp_voice.h. */
#include "mp_voice.h"

S1_PARAM(sub, S1_OSC1_COLOR, 1) { out[0] = knob / 255.0f; }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.25f + 0.5f * knob / 255.0f) * 0.67f; }

static float ph, ps;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float p = mp_advance(&ph, dt);
  float q = mp_advance(&ps, dt * 0.5f);
  float saw = mp_saw_bl(p, dt, rdt);
  float sq = mp_pulse_bl(q, 0.5f, dt * 0.5f, rdt + rdt);
  return (saw + sq * s1_param(sub, 0)) * s1_param(level, 0);
}
