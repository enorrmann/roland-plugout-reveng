/* mp_sine_triangle.c: Braids SINE_TRIANGLE: a folded sine and a folded triangle, crossfaded; the fold gain falls
 * with the pitch (Braids' attenuation above notes 92 / 80) so high notes stay clean.
 * OSC1 COLOR = fold, OSC2 COLOR = sine / triangle balance, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt();
  float p = mp_advance(&ph, dt);
  /* attenuation ~1 below 1.5 kHz, ~0 at 6 kHz: 1 - 16 (dt - 0.016) clamped */
  float att = fminf(fmaxf(1.25f - 16.0f * dt, 0.0f), 1.0f);
  float g = 1.0f + 4.0f * s1_color(0) * att;
  float tri = s1_tri(p + p - 0.5f);
  float a = mp_fold(mp_sin(p) * g), b = mp_fold(tri * g);
  return (a + (b - a) * s1_color(1)) * s1_param(level, 0);
}
