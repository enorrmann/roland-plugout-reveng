/* mp_square.c: band-limited pulse with pulse width (Braids SQUARE / Plaits VA, polyBLEP at both edges).
 * OSC1 COLOR = pulse width 50 .. 95 %, OSC1 MIX = level. DSP B: mp_voice.h. */
#include "mp_voice.h"

S1_PARAM(pw, S1_OSC1_COLOR, 1) { out[0] = 0.5f + 0.45f * knob / 255.0f; }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt(), pw = s1_param(pw, 0);
  float p = mp_advance(&ph, dt);
  float sq = mp_pulse_bl(p, pw, dt, rdt);
  return sq * s1_param(level, 0);
}
