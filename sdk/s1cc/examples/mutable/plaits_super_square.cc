// Plaits SuperSquareOscillator: two hard-synced / detuned squares (plaits_common.h; Plaits' VA engine)
//   OSC1 COLOR = shape (below 0.5: detune; above: sync ratio)
#include "plaits_common.h"
#include "plaits/dsp/oscillator/super_square_oscillator.h"

S1_PARAM(shape, S1_OSC1_COLOR, 1) { out[0] = knob / 255.0f; }

static plaits::SuperSquareOscillator osc;

S1_INIT { osc.Init(); }

S1_DSP_A {
  float out;
  osc.Render(plaits_frequency(), s1_param(shape, 0), &out, 1);
  return out;
}
