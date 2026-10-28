/* mp_variable_shape.c: Plaits' VariableShapeOscillator, the VA oscillator that morphs saw -> square -> triangle:
 * band-limited saw and pulse (polyBLEP) and the triangle (alias-poor as it is), crossfaded.
 * OSC1 COLOR = shape (saw .. square .. triangle), OSC2 COLOR = pulse width, OSC1 MIX = level. DSP B: mp_voice.h. */
#include "mp_voice.h"

/* crossfade weights for saw, square, triangle at the knob position (two at a time) */
S1_PARAM(shape, S1_OSC1_COLOR, 3) {
  float s = knob / 255.0f * 2.0f;
  out[0] = s < 1.0f ? 1.0f - s : 0.0f;
  out[1] = s < 1.0f ? s : 2.0f - s;
  out[2] = s < 1.0f ? 0.0f : s - 1.0f;
}
S1_PARAM(pw, S1_OSC2_COLOR, 1) { out[0] = 0.5f + 0.45f * knob / 255.0f; }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt(), pw = s1_param(pw, 0);
  float p = mp_advance(&ph, dt);
  float saw = mp_saw_bl(p, dt, rdt);
  float sq = mp_saw_bl(p - pw, dt, rdt) - saw + pw + pw - 1.0f;
  float tri = s1_tri(p + p);
  float y = saw * s1_param(shape, 0) + sq * s1_param(shape, 1) + tri * s1_param(shape, 2);
  return y * s1_param(level, 0);
}
