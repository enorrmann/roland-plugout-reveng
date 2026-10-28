/* mp_phase_distortion.c: Casio CZ phase distortion (Plaits' phase distortion engine, its basic shape): the phase
 * runs fast up to a knee, slow after it, and drives a cosine: from sine to saw-like.
 * OSC1 COLOR = knee (distortion), with its COLOR MOD source (s1_color); OSC1 MIX = level. DSP B: mp_dsp_b.h. */
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float p = mp_advance(&ph, mp_dt());
  /* the knee k = 0.5 2^-(g c) goes from 0.5 (sine) to 0.03 (saw-like) with the color c. The warped phase is the lower
     of the two lines through (0, 0) and (1, 1), slopes a = 0.5 / k = 2^(g c) and b = 0.5 / (1 - k): branch-free and
     continuous for any a and b, so b is a fit in c (degree 4, 1.4 %: the knee lands at w = 0.49..0.56 instead of 0.5)
     and the DSP divides nothing */
  float c = s1_color(0);
  float a = s1_exp2(4.0589f * c);                                   /* g = log2(0.5 / 0.03) */
  float b = 0.97436f + c * (-2.03726f + c * (4.05872f + c * (-3.9256f + c * 1.45078f)));
  float w = fminf(p * a, 1.0f - b * (1.0f - p));
  return -mp_cos(w) * s1_param(level, 0);
}
