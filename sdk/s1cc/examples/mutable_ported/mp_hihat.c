/* mp_hihat.c: a metallic hi-hat (Braids CYMBAL / Plaits hi-hat, the 808 recipe): six squares at inharmonic ratios
 * of the note, summed and high passed in DSP B (MP_SVF_HP, FILTER CUTOFF high); the level decay is the AMP envelope.
 * OSC1 COLOR = noise mixed in, OSC1 MIX = level. */
#define MP_FILTER MP_SVF_HP
#include "mp_voice.h"

static const float kRatios[6] = { 1.0f, 1.304f, 1.466f, 1.787f, 1.932f, 2.536f };   /* the 808's cymbal ratios */
S1_PARAM(noise, S1_OSC1_COLOR, 1) { out[0] = knob / 255.0f; }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.25f + 0.5f * knob / 255.0f) / 6.0f; }   /* six squares + noise */

static float ph[6], w;

S1_DSP_A {
  float dt = mp_dt(), y = 0.0f;
  for (int i = 0; i < 6; i++) {
    float p = mp_advance(&ph[i], mp_step(dt, kRatios[i]));
    y += p < 0.5f ? 1.0f : -1.0f;
  }
  return (y + mp_noise(&w) * s1_param(noise, 0) * 3.0f) * s1_param(level, 0);
}
