/* edm_sawcomb.c: Braids SAW_COMB (macro_oscillator.cc RenderSawComb + digital_oscillator.cc RenderComb) on
 * SYSTEM-S1CC: the band-limited saw with the SVF low pass and the AMP envelope on the slave (mp_saw.c +
 * mp_dsp_b.h), then a comb per voice in its own line of the delay memory on master B (EDM_STR_COMB,
 * docs/MUTABLE_MEMORY.md §3.3.5): delay = the period of note + OSC1 COLOR (+-64 semitones, Braids' TIMBRE),
 * feedback = OSC2 COLOR (bipolar, Braids' COLOR), output saw + comb. 6 voices. OSC1 MIX = level. */
#include "../mutable_ported/include/mp_dsp_a.h"
#include "../mutable_ported/include/mp_dsp_b.h"

S1_BUILD("NAME=MP-SAWCOMB EDM_STR=1 EDM_STR_ONE=1 EDM_STR_COMB=1");
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.5f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float p = mp_advance(&ph, dt);
  return mp_saw_bl(p, dt, rdt) * s1_param(level, 0);
}
