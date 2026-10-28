// Plaits StringSynthOscillator: divide-down saws and squares of a string machine (plaits_common.h)
//   OSC1 COLOR = registration (Plaits' chord engine: registrations[] and ComputeRegistration, copied below because
//   they are private to chord_engine.cc; computed on the PC per knob position), OSC2 COLOR = gain
#include "plaits_common.h"
#include "plaits/dsp/oscillator/string_synth_oscillator.h"

static const float kRegistrations[8][6] = {     /* plaits/dsp/engine/chord_engine.cc */
  { 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },  // Square
  { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },  // Saw
  { 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f },  // Saw + saw
  { 0.33f, 0.0f, 0.33f, 0.0f, 0.33f, 0.0f },  // Full saw
  { 0.33f, 0.0f, 0.0f, 0.33f, 0.0f, 0.33f },  // Full saw + square hybrid
  { 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f },  // Saw + high square harmo
  { 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f },  // Square + high square harmo
  { 0.0f, 0.1f, 0.1f, 0.0f, 0.2f, 0.6f },  // Saw+square + high harmo
};
S1_PARAM(reg, S1_OSC1_COLOR, 7) {
  float r = knob / 255.0f * (8 - 1.001f);
  int i = (int)r;
  float t = r - i;
  for (int k = 0; k < 6; k++) out[k] = kRegistrations[i][k] + (kRegistrations[i + 1][k] - kRegistrations[i][k]) * t;
  out[6] = 0.0f;
}
S1_PARAM(gain, S1_OSC2_COLOR, 1) { out[0] = 0.2f + 0.8f * knob / 255.0f; }

static plaits::StringSynthOscillator osc;

S1_INIT { osc.Init(); }

S1_DSP_A {
  float registration[7], out = 0.0f;
  for (int k = 0; k < 7; k++)
    registration[k] = s1_param(reg, k);
  osc.Render(plaits_frequency(), registration, s1_param(gain, 0), &out, 1);
  return out;
}
