// Plaits FastSineOscillator: a sine by a rotating complex number (plaits_common.h; Plaits' modal / additive voices)
#include "plaits_common.h"
#include "plaits/dsp/oscillator/sine_oscillator.h"

static plaits::FastSineOscillator osc;

S1_INIT { osc.Init(); }

S1_DSP_A {
  float out;
  osc.Render(plaits_frequency(), &out, 1);
  return out;
}
