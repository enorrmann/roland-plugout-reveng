/* mp_toy.c: Braids TOY / the CRUSHER of a sampler: a saw through a sample-rate reducer (hold) and a bit reducer.
 * OSC1 COLOR = sample rate 96 kHz .. 1 kHz, OSC2 COLOR = levels 256 .. 4, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph, hp, held;

S1_DSP_A {
  float p = mp_advance(&ph, mp_dt());
  float saw = p + p - 1.0f;
  float h = hp + s1_exp2(-6.585f * s1_color(0));              /* hold phase step 96^-c */
  float nb = 8.0f - 6.0f * s1_color(1);                       /* bits: n = 2^nb levels per unit */
  if (h >= 1.0f) {                                          /* the hold clock ticks: take a new sample */
    h -= 1.0f;
    held = floorf(saw * s1_exp2(nb)) * s1_exp2(-nb);
  }
  hp = h;
  return held * s1_param(level, 0);
}
