/* mp_waveshaping.c: Plaits waveshaping engine: a slope (triangle with a variable rise) through a waveshaper and a
 * wavefolder. Plaits' shaper / folder tables (lookup_table_i16_table, lut_fold) are an odd curve and a fold: here
 * the cubic soft clip of mp_filter.h and the DSP's triangle fold (mp_fold). The fold gain falls with the pitch
 * (Plaits' Tame()) so high notes do not alias. The slope's two divisions come from a knob table (on the PC).
 * OSC1 COLOR = fold gain, OSC2 COLOR = slope (rise 5 % .. 95 %: table, no COLOR MOD), OSC1 MIX = level.
 * DSP B: mp_dsp_b.h. OSC1 COLOR follows its COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_filter.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }
S1_PARAM(slope, S1_OSC2_COLOR, 3) {
  float pw = 0.05f + 0.9f * knob / 255.0f;
  out[0] = pw; out[1] = 2.0f / pw; out[2] = 2.0f / (1.0f - pw);
}

static float ph;

S1_DSP_A {
  float dt = mp_dt();
  float p = mp_advance(&ph, dt);
  float pw = s1_param(slope, 0);
  /* -1 at 0, +1 at pw, -1 at 1 */
  float x = fminf(p * s1_param(slope, 1), (1.0f - p) * s1_param(slope, 2)) - 1.0f;
  float tame = fminf(fmaxf(1.5f - 24.0f * dt, 0.0f), 1.0f);
  float g = 1.0f + 6.0f * s1_color(0) * tame;
  (void)pw;
  return mp_fold(mp_soft(x * 1.2f) * g) * s1_param(level, 0);
}
