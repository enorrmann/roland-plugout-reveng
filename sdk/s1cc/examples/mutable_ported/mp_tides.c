/* mp_tides.c: Tides 2's ramp generator at audio rate: a slope (rise / fall time ratio) bent by a shape (from
 * exponential through linear to logarithmic, Tides' ramp_shaper as a power curve) and its smoothness (folded above
 * the middle, smoothed below: here the fold side only). The slope's divisions come from the knob table (on the PC).
 * OSC1 COLOR = shape, OSC2 COLOR = slope (table, no COLOR MOD), OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * OSC1 COLOR follows its COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }
S1_PARAM(slope, S1_OSC2_COLOR, 2) {
  float s = 0.02f + 0.96f * knob / 255.0f;
  out[0] = 1.0f / s; out[1] = 1.0f / (1.0f - s);
}

static float ph;

S1_DSP_A {
  float p = mp_advance(&ph, mp_dt());
  float u = fminf(p * s1_param(slope, 0), (1.0f - p) * s1_param(slope, 1));   /* 0 -> 1 -> 0 */
  u = fminf(fmaxf(u, 0.0f), 1.0f);
  /* shape: u^k, k = 2^(3 (0.5 - c)): 2.8 (exponential) .. 1 .. 0.35 (logarithmic) */
  float k = s1_exp2(3.0f - 6.0f * s1_color(0));
  float v = s1_exp2(fmaxf(k * s1_log2(fmaxf(u, 1e-5f)), -18.0f));
  return (v + v - 1.0f) * s1_param(level, 0);
}
