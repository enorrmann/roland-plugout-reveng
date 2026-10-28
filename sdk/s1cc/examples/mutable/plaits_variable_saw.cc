// Plaits VariableSawOscillator: saw with a movable notch / slope (plaits_common.h; Plaits' VA engine, OSC 2)
//   OSC1 COLOR = pulse width, OSC2 COLOR = waveshape
#include "plaits_common.h"
#include "plaits/dsp/oscillator/variable_saw_oscillator.h"

S1_PARAM(pw, S1_OSC1_COLOR, 1) { out[0] = 0.5f + 0.49f * (knob / 255.0f); }
S1_PARAM(waveshape, S1_OSC2_COLOR, 1) { out[0] = knob / 255.0f; }

static plaits::VariableSawOscillator osc;

S1_INIT { osc.Init(); }

S1_DSP_A {
  float out;
  osc.Render(plaits_frequency(), s1_param(pw, 0), s1_param(waveshape, 0), &out, 1);
  return out;
}
