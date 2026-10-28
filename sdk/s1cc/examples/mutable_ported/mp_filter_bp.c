/* mp_filter_bp.c: the SVF's band pass on a band-limited pulse (a nasal, vocal sweep).
 * DSP B: MP_SVF_BP. OSC1 COLOR = pulse width, OSC1 MIX = level.
 * The COLORs follow their COLOR MOD source (s1_color). */
#define MP_FILTER MP_SVF_BP
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt(), pw = 0.5f + 0.45f * s1_color(0);
  float p = mp_advance(&ph, dt);
  return mp_pulse_bl(p, pw, dt, rdt) * s1_param(level, 0);
}
