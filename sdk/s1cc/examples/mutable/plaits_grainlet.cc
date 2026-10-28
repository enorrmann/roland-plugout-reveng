// Plaits GrainletOscillator: formant grains windowed by the carrier (plaits_common.h; Plaits' grain engine)
//   OSC1 COLOR = formant (1 .. 16 x the note), OSC2 COLOR = carrier shape, OSC1 MIX = carrier bleed
#include "plaits_common.h"
#include "plaits/dsp/oscillator/grainlet_oscillator.h"

S1_PARAM(formant, S1_OSC1_COLOR, 1) { out[0] = powf(16.0f, knob / 255.0f); }
S1_PARAM(shape, S1_OSC2_COLOR, 1) { out[0] = knob / 255.0f; }
S1_PARAM(bleed, S1_OSC1_MIX, 1) { out[0] = knob / 255.0f; }

static plaits::GrainletOscillator osc;

S1_INIT { osc.Init(); }

S1_DSP_A {
  float out, f = plaits_frequency();
  osc.Render(f, f * s1_param(formant, 0), s1_param(shape, 0), s1_param(bleed, 0), &out, 1);
  return out;
}
