/* mp_clocked_noise.c: Braids CLOCKED_NOISE: noise sampled and held at the note's rate (a pitched, digital noise).
 * OSC1 COLOR = clock ratio 1/4 .. 4 x the note, OSC1 MIX = level. DSP B: mp_voice.h. */
#include <math.h>
#include "mp_voice.h"

S1_PARAM(ratio, S1_OSC1_COLOR, 1) { out[0] = 0.25f * powf(16.0f, knob / 255.0f); }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph, w, held;

S1_DSP_A {
  float n = mp_noise(&w);
  float p = ph + mp_step(mp_dt(), s1_param(ratio, 0));
  if (p >= 1.0f) {
    p -= 1.0f;
    held = n;
  }
  ph = p;
  return held * s1_param(level, 0);
}
