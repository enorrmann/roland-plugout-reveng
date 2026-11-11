/* edm_blown.c: Braids BLOWN (digital_oscillator.cc RenderBlown: a reed exciting a bore) on SYSTEM-S1CC, 6 voices.
 * The slave makes the breath pressure (OSC1 MIX) with its noise (OSC1 COLOR), times the AMP envelope (mp_dsp_b.h MP_NONE: the blowing follows the ADSR); master B runs the reed and the bore in
 * the voice's own line of the delay memory (EDM_STR_BLOWN, microcode/edm_str.s master_blown, docs/vol6_reference_designs_applications/mutable_memory.md
 * §3.3.5) and the body filter (OSC2 COLOR, Braids' COLOR). */
#define MP_FILTER MP_NONE
#include "../mutable_ported/include/mp_dsp_a.h"
#include "../mutable_ported/include/mp_dsp_b.h"

S1_BUILD("NAME=MP-BLOWN EDM_LINES=1 EDM_STR_ONE=1 EDM_STR_BLOWN=1");
/* Braids blows at 0.8 with noise +-0.68 .. 0.29 of it: on the device a square with noise (2026-11-04: H3 -10 dB,
   H5 -16 dB, H7 -15 dB, noise -30 dB). Here the pressure is OSC1 MIX, 0.35 (just above the reed's threshold: round)
   .. 0.9 (Braids': square), and the noise OSC1 COLOR, 0 .. 0.25 of the pressure */
S1_PARAM(pressure, S1_OSC1_MIX, 1) { out[0] = 0.35f + 0.55f * knob / 255.0f; }
S1_PARAM(noise, S1_OSC1_COLOR, 1) { float k = knob / 255.0f; out[0] = 0.25f * k * k; }

static float w;

S1_DSP_A {
  float p = s1_param(pressure, 0);
  return p + mp_noise(&w) * s1_param(noise, 0) * p;
}
