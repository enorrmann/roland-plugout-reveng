// Plaits NESTriangleOscillator: the 4-bit stepped triangle of the NES (plaits_common.h; Plaits' chiptune engine)
#include "plaits_common.h"
#include "plaits/dsp/oscillator/nes_triangle_oscillator.h"

static plaits::NESTriangleOscillator<> osc;

S1_INIT { osc.Init(); }

S1_DSP_A {
  float out;
  osc.Render(plaits_frequency(), &out, 1);
  return out;
}
