/* mp_variable_shape.c: Plaits' VariableShapeOscillator, the VA oscillator that morphs saw -> square -> triangle:
 * band-limited saw and pulse (polyBLEP) and the triangle (alias-poor as it is), crossfaded.
 * OSC1 COLOR = shape (saw .. square .. triangle), OSC2 COLOR = pulse width, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt(), pw = 0.5f + 0.45f * s1_color(1);
  float p = mp_advance(&ph, dt);
  float saw = mp_saw_bl(p, dt, rdt);
  float sq = mp_saw_bl(p - pw, dt, rdt) - saw + pw + pw - 1.0f;
  float tri = s1_tri(p + p);
  float t = 2.0f * s1_color(0) - 1.0f;   /* crossfade saw (-1) -> square (0) -> triangle (1), two at a time */
  float y = sq + fmaxf(-t, 0.0f) * (saw - sq) + fmaxf(t, 0.0f) * (tri - sq);
  return y * s1_param(level, 0);
}
