// Plaits ZOscillator: sync-swept formant with a carrier window (CZ-like; plaits_common.h)
//   OSC1 COLOR = formant (1 .. 16 x the note), OSC2 COLOR = carrier shape, OSC1 WAVE = mode 0..5 -> 0..1
#include "plaits_common.h"
#include "plaits/dsp/oscillator/z_oscillator.h"

S1_PARAM(formant, S1_OSC1_COLOR, 1) { out[0] = powf(16.0f, knob / 255.0f); }
S1_PARAM(shape, S1_OSC2_COLOR, 1) { out[0] = knob / 255.0f; }
S1_PARAM(mode, S1_OSC1_WAVE, 1) { out[0] = knob / 255.0f; }

static plaits::ZOscillator osc;

S1_INIT { osc.Init(); }

S1_DSP_A {
  float out, f = plaits_frequency();
  osc.Render(f, f * s1_param(formant, 0), s1_param(shape, 0), s1_param(mode, 0), &out, 1);
  return out;
}
