/* edm_bowed.c: Braids BOWED (digital_oscillator.cc RenderBowed: a bow on a string, two waveguides) on SYSTEM-S1CC,
 * 3 voices (two delay-memory lines each: 6 tapreq, the measured limit). The slave makes the bow velocity, Braids'
 * lut_bowing_envelope settles at 0.16: here 0.16 x OSC1 MIX x the AMP envelope (mp_dsp_b.h MP_NONE: the stroke
 * follows the ADSR); master B runs the string, the friction and the body (EDM_STR_BOWED, microcode/edm_str.s
 * master_bowed, docs/vol6_reference_designs_applications/mutable_memory.md §3.3.5). OSC1 COLOR = bow pressure (Braids' TIMBRE), OSC2 COLOR = bow
 * position (Braids' COLOR). */
#define MP_FILTER MP_NONE
#include "../mutable_ported/include/mp_dsp_a.h"
#include "../mutable_ported/include/mp_dsp_b.h"

S1_BUILD("NAME=MP-BOWED EDM_LINES=1 EDM_STR_ONE=1 EDM_STR_BOWED=1");
/* bow velocity 0.04 .. 0.16 (above it the string goes to its second mode at most pressures: bowed_sim.py) */
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.04f + 0.12f * knob / 255.0f; }

/* the hair's noise on the bow velocity, +-8 % through a one-pole (~2 kHz): without it the string was perfectly
   periodic (harmonics 90 .. 100 dB over the floor, 2026-11-04) and did not sound like a string */
static float w, nz;

S1_DSP_A {
  float n = nz + 0.12f * (mp_noise(&w) - nz);
  nz = n;
  return s1_param(level, 0) * (1.0f + 0.08f * n);
}
