/* mp_square.c: band-limited pulse with pulse width (Braids SQUARE / Plaits VA, polyBLEP at both edges).
 * OSC1 COLOR = pulse width 50 .. 95 %, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt(), pw = 0.5f + 0.45f * s1_color(0);
  float p = mp_advance(&ph, dt);
  float sq = mp_pulse_bl(p, pw, dt, rdt);
  return sq * s1_param(level, 0);
}
