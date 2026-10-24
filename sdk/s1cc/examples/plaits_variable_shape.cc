// Plaits' VariableShapeOscillator (plaits/dsp/oscillator/variable_shape_oscillator.h, (c) Emilie Gillet, MIT),
// compiled as it is with the real stmlib (sdk/s1cc/s1cc deps). A hand wrapper (docs/S1CC.md §3): one Render() of
// one sample per DSP sample. Plaits runs at 48 kHz with frequencies in cycles per sample: here f / 96000.
#include "s1dsp.h"
#include "plaits/dsp/oscillator/variable_shape_oscillator.h"

S1_PARAM(pw, 0x168, 1) { out[0] = 0.5f + 0.49f * (knob / 255.0f); }        // OSC1 COLOR: pulse width
S1_PARAM(shape, 0x16D, 1) { out[0] = knob / 255.0f; }                       // OSC2 COLOR: saw -> pulse

static plaits::VariableShapeOscillator osc;

S1_INIT { osc.Init(); }

S1_VOICE {
  float out;
  float f = s1_increment() * 0.5f;                       // our step is for a phase -1..1
  osc.Render(f, s1_param(pw, 0), s1_param(shape, 0), &out, 1);
  return out;
}
