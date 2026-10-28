/* mp_filter_ladder.c: the 4-pole ladder (mp_filter.h, after Plaits' virtual analog VCF) on a band-limited saw.
 * DSP B: MP_LADDER (FILTER CUTOFF, RESONANCE self-oscillates near the top, ENV AMOUNT, KEY TRACK). OSC1 MIX = level. */
#define MP_FILTER MP_LADDER
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float p = mp_advance(&ph, dt);
  return mp_saw_bl(p, dt, rdt) * s1_param(level, 0);
}
