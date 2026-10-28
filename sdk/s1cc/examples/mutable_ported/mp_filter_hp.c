/* mp_filter_hp.c: the SVF's high pass on a band-limited saw (thin, the Braids SAW_SWARM high pass as a filter).
 * DSP B: MP_SVF_HP. OSC1 MIX = level. */
#define MP_FILTER MP_SVF_HP
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float p = mp_advance(&ph, dt);
  return mp_saw_bl(p, dt, rdt) * s1_param(level, 0);
}
