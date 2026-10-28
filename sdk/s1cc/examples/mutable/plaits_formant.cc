// Plaits FormantOscillator: a carrier whose phase resets a formant sine (plaits_common.h)
//   OSC1 COLOR = formant (1 .. 16 x the note), OSC2 COLOR = phase shift
#include "plaits_common.h"
#include "plaits/dsp/oscillator/formant_oscillator.h"

S1_PARAM(formant, S1_OSC1_COLOR, 1) { out[0] = powf(16.0f, knob / 255.0f); }
S1_PARAM(shift, S1_OSC2_COLOR, 1) { out[0] = knob / 255.0f; }

static plaits::FormantOscillator osc;

S1_INIT { osc.Init(); }

S1_DSP_A {
  float out, f = plaits_frequency();
  osc.Render(f, f * s1_param(formant, 0), s1_param(shift, 0), &out, 1);
  return out;
}
