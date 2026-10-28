// Plaits VariableShapeOscillator: saw -> pulse -> triangle with pulse width (plaits_common.h)
//   OSC1 COLOR = pulse width, OSC2 COLOR = shape (saw .. square .. triangle)
#include "plaits_common.h"
#include "plaits/dsp/oscillator/variable_shape_oscillator.h"

S1_PARAM(pw, S1_OSC1_COLOR, 1) { out[0] = 0.5f + 0.49f * (knob / 255.0f); }
S1_PARAM(shape, S1_OSC2_COLOR, 1) { out[0] = knob / 255.0f; }

static plaits::VariableShapeOscillator osc;

S1_INIT { osc.Init(); }

S1_DSP_A {
  float out;
  osc.Render(plaits_frequency(), s1_param(pw, 0), s1_param(shape, 0), &out, 1);
  return out;
}
