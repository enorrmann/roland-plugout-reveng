// Plaits SineOscillator: table-interpolated sine (plaits_common.h)
#include "plaits_common.h"
#include "plaits/dsp/oscillator/sine_oscillator.h"

static plaits::SineOscillator osc;

S1_INIT { osc.Init(); }

S1_DSP_A {
  float out;
  osc.Render(plaits_frequency(), &out, 1);
  return out;
}
