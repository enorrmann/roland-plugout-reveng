/* mp_toy.c: Braids TOY / the CRUSHER of a sampler: a saw through a sample-rate reducer (hold) and a bit reducer.
 * OSC1 COLOR = sample rate 96 kHz .. 1 kHz, OSC2 COLOR = levels 256 .. 4, OSC1 MIX = level. DSP B: mp_dsp_b.h. */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(rate, S1_OSC1_COLOR, 1) { out[0] = powf(96.0f, -knob / 255.0f); }          /* hold phase step */
S1_PARAM(bits, S1_OSC2_COLOR, 2) { float n = powf(2.0f, 8.0f - 6.0f * knob / 255.0f); out[0] = n; out[1] = 1.0f / n; }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph, hp, held;

S1_DSP_A {
  float p = mp_advance(&ph, mp_dt());
  float saw = p + p - 1.0f;
  float h = hp + s1_param(rate, 0);
  if (h >= 1.0f) {                                          /* the hold clock ticks: take a new sample */
    h -= 1.0f;
    held = floorf(saw * s1_param(bits, 0)) * s1_param(bits, 1);
  }
  hp = h;
  return held * s1_param(level, 0);
}
