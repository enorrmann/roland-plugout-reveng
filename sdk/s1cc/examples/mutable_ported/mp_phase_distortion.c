/* mp_phase_distortion.c: Casio CZ phase distortion (Plaits' phase distortion engine, its basic shape): the phase
 * runs fast up to a knee, slow after it, and drives a cosine: from sine to saw-like.
 * OSC1 COLOR = knee (distortion), OSC1 MIX = level. DSP B: mp_voice.h. */
#include "mp_voice.h"

/* knee k and the two slopes 0.5 / k, 0.5 / (1 - k), on the PC (no division on the DSP) */
S1_PARAM(pd, S1_OSC1_COLOR, 3) {
  float k = 0.5f - 0.47f * knob / 255.0f;
  out[0] = k;
  out[1] = 0.5f / k;
  out[2] = 0.5f / (1.0f - k);
}
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float p = mp_advance(&ph, mp_dt());
  float k = s1_param(pd, 0);
  float w = p < k ? p * s1_param(pd, 1) : 0.5f + (p - k) * s1_param(pd, 2);
  return -mp_cos(w) * s1_param(level, 0);
}
