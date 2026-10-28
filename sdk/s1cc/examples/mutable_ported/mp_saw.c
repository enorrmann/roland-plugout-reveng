/* mp_saw.c: band-limited sawtooth (polyBLEP), the basic analog oscillator of Braids / Plaits, for SYSTEM-S1CC.
 * DSP A: the saw; DSP B: mp_dsp_b.h (SVF low pass + envelopes). OSC1 MIX = level. */
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float p = mp_advance(&ph, dt);
  float saw = mp_saw_bl(p, dt, rdt);
  return saw * s1_param(level, 0);
}
