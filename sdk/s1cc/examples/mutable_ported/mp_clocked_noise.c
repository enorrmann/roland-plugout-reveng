/* mp_clocked_noise.c: Braids CLOCKED_NOISE: noise sampled and held at the note's rate (a pitched, digital noise).
 * OSC1 COLOR = clock ratio 1/4 .. 4 x the note, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph, w, held;

S1_DSP_A {
  float n = mp_noise(&w);
  float p = ph + mp_step(mp_dt(), s1_exp2(4.0f * s1_color(0) - 2.0f));
  if (p >= 1.0f) {
    p -= 1.0f;
    held = n;
  }
  ph = p;
  return held * s1_param(level, 0);
}
