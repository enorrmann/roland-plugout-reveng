/* mp_grainlet.c: Plaits GrainletOscillator (the grain engine's first voice): a formant sine restarted with the
 * note, under a carrier window whose shape bends from a short burst to a full bell. Plaits' window is a sine of a
 * piecewise-linear warped phase (Carrier(): three shapes); here the warp is one form for all three, two slopes and a
 * plateau, w = min(p, bp) s1 + max(p - bp, 0) s2 + off, limited to top, computed on the DSP from the color (the
 * breakpoints' two divisions are s1_recip, so the shape follows its COLOR MOD).
 * OSC1 COLOR = window shape, OSC2 COLOR = formant ratio 1 .. 16, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt();
  float p = mp_advance(&ph, dt);
  float shape = fminf(3.0f * s1_color(0), 2.999f);
  int seg0 = shape < 1.0f, seg2 = shape >= 2.0f;
  float t = 1.0f - (shape - floorf(shape));
  float t3 = t * t * t, u = 1.0f - t, u3 = u * u * u;
  /* seg 0: s1 = 1 + 15 t^3, bp = 1 / s1;  seg 1: bp = 0.001 + 0.499 t^3, s1 = 0.5 / bp, s2 = 0.5 / (1 - bp);
     seg 2: s1 = 0.5 + 14.5 u^3, bp = 0.5 / s1 (at most 1). One reciprocal of a, one of 1 - bp1 */
  float bp1 = 0.001f + 0.499f * t3;
  float a = seg0 ? 1.0f + 15.0f * t3 : seg2 ? 0.5f + 14.5f * u3 : bp1;
  float ra = s1_recip(a);
  float bp = seg0 ? ra : seg2 ? fminf(0.5f * ra, 1.0f) : bp1;
  float s1 = seg0 || seg2 ? a : 0.5f * ra;
  float s2 = seg0 || seg2 ? 0.0f : 0.5f * s1_recip(1.0f - bp1);
  float off = seg2 ? 0.25f : 0.75f, top = seg2 ? 0.75f : 1.75f;
  float w = fminf(fminf(p, bp) * s1 + fmaxf(p - bp, 0.0f) * s2 + off, top);
  float carrier = (mp_sin(mp_frac(w)) + 1.0f) * 0.5f;
  float formant = mp_sin(mp_frac(p * s1_exp2(4.0f * s1_color(1))));
  return (carrier * formant + carrier - 0.5f) * s1_param(level, 0);
}
