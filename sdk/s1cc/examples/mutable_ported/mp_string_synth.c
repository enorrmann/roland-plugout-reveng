/* mp_string_synth.c: Plaits StringSynthOscillator (string machine / organ, without the engine's chorus, which needs
 * a delay line): divide-down saws at 16', 8', 4' and 2' (f/2, f, 2f, 4f); squares come out of saw differences (a
 * square of one octave = saw of that octave x 2 - saw of the octave below), as Plaits writes its registrations.
 * OSC1 COLOR = registration (Plaits' 11 string-machine registrations, interpolated: a knob table), OSC1 MIX =
 * level. DSP B: mp_dsp_b.h. */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.75f * knob / 255.0f) * 0.5f; }
/* registration (saw / square of three octaves) -> gains of the four saws (Plaits' gain formulas) */
S1_PARAM(reg, S1_OSC1_COLOR, 4) {
  static const float kReg[11][6] = {
    { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f },
    { 0.4f, 0.0f, 0.2f, 0.0f, 0.4f, 0.0f }, { 0.3f, 0.0f, 0.0f, 0.3f, 0.0f, 0.4f },
    { 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f }, { 0.2f, 0.0f, 0.0f, 0.2f, 0.0f, 0.6f },
    { 0.0f, 0.2f, 0.1f, 0.0f, 0.2f, 0.5f }, { 0.0f, 0.3f, 0.0f, 0.3f, 0.0f, 0.4f },
    { 0.0f, 0.4f, 0.0f, 0.3f, 0.0f, 0.3f }, { 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f } };
  float x = knob / 255.0f * 9.999f, r[7];
  int i = (int)x;
  float fr = x - i;
  for (int k = 0; k < 6; k++)
    r[k] = kReg[i][k] + (kReg[i + 1][k] - kReg[i][k]) * fr;
  r[6] = 0.0f;
  out[0] = r[0] + 2.0f * r[1];
  out[1] = r[2] - r[1] + 2.0f * r[3];
  out[2] = r[4] - r[3] + 2.0f * r[5];
  out[3] = r[6] - r[5];
}

static float p8, p4, p2, p1;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float y = s1_param(reg, 0) * mp_saw_bl(mp_advance(&p8, dt * 0.5f), dt * 0.5f, rdt * 2.0f);
  y += s1_param(reg, 1) * mp_saw_bl(mp_advance(&p4, dt), dt, rdt);
  float d2 = fminf(dt * 2.0f, 0.5f), d1 = fminf(dt * 4.0f, 0.5f);
  y += s1_param(reg, 2) * mp_saw_bl(mp_advance(&p2, d2), d2, rdt * 0.5f);
  y += s1_param(reg, 3) * mp_saw_bl(mp_advance(&p1, d1), d1, rdt * 0.25f);
  return y * s1_param(level, 0);
}
