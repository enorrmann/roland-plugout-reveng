// Plaits HarmonicOscillator: additive, 8 harmonics by Chebyshev recursion (plaits_common.h; Plaits' additive engine)
//   OSC1 COLOR = brightness (harmonic n at brightness^(n-1)), OSC2 COLOR = odd / even balance
#include "plaits_common.h"
#include "plaits/dsp/oscillator/harmonic_oscillator.h"

#define HARMONICS 8
/* the amplitudes, computed on the PC for every knob position: 8 table values per knob, no DSP work */
S1_PARAM(bright, S1_OSC1_COLOR, HARMONICS) {
  float b = 0.2f + 0.8f * knob / 255.0f, a = 1.0f, sum = 0.0f;
  for (int i = 0; i < HARMONICS; i++) { out[i] = a; sum += a; a *= b; }
  for (int i = 0; i < HARMONICS; i++) out[i] /= sum;
}
S1_PARAM(even, S1_OSC2_COLOR, 1) { out[0] = knob / 255.0f; }

static plaits::HarmonicOscillator<HARMONICS> osc;

S1_INIT { osc.Init(); }

S1_DSP_A {
  float amplitudes[HARMONICS], out = 0.0f;
  for (int i = 0; i < HARMONICS; i++)      /* even harmonics (i odd: harmonic i + 1) scaled by OSC2 COLOR */
    amplitudes[i] = s1_param(bright, i) * ((i & 1) ? s1_param(even, 0) * 2.0f : 1.0f);
  osc.Render<1>(plaits_frequency(), amplitudes, &out, 1);
  return out;
}
