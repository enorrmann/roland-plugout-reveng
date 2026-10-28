// Plaits VOSIMOscillator: two formants windowed by the carrier (plaits_common.h)
//   OSC1 COLOR = formant 1, OSC2 COLOR = formant 2 (1 .. 16 x the note), OSC1 MIX = carrier shape
#include "plaits_common.h"
#include "plaits/dsp/oscillator/vosim_oscillator.h"

S1_PARAM(formant1, S1_OSC1_COLOR, 1) { out[0] = powf(16.0f, knob / 255.0f); }
S1_PARAM(formant2, S1_OSC2_COLOR, 1) { out[0] = powf(16.0f, knob / 255.0f); }
S1_PARAM(shape, S1_OSC1_MIX, 1) { out[0] = knob / 255.0f; }

static plaits::VOSIMOscillator osc;

S1_INIT { osc.Init(); }

S1_DSP_A {
  float out, f = plaits_frequency();
  osc.Render(f, f * s1_param(formant1, 0), f * s1_param(formant2, 0), s1_param(shape, 0), &out, 1);
  return out;
}
