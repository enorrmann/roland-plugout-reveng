/* mp_fm4.c: Plaits six-op FM engine, a subset: four sine operators in a stack (op4 -> op3 -> op2 -> op1, DX7
 * algorithm 1's right half) with feedback on op4, the operator ratios from a knob table of DX-style ratio sets.
 * Six operators with their envelopes do not fit a voice here.
 * OSC1 COLOR = modulation index (all modulators), OSC2 COLOR = ratio set (table, no COLOR MOD), OSC1 MIX = level.
 * DSP B: mp_dsp_b.h. OSC1 COLOR follows its COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }
S1_PARAM(ratio, S1_OSC2_COLOR, 3) {
  static const float kSets[8][3] = {
    { 1.0f, 1.0f, 1.0f }, { 2.0f, 1.0f, 1.0f }, { 1.0f, 2.0f, 3.0f }, { 3.0f, 1.0f, 7.0f },
    { 1.0f, 3.5f, 1.0f }, { 2.0f, 5.0f, 9.0f }, { 1.41f, 2.0f, 7.0f }, { 0.5f, 3.0f, 11.0f } };
  int i = knob * 8 / 256;
  out[0] = kSets[i][0]; out[1] = kSets[i][1]; out[2] = kSets[i][2];
}

static float p1, p2, p3, p4, fb;

S1_DSP_A {
  float dt = mp_dt(), idx = 0.3f * s1_color(0);
  float o4 = mp_sin(mp_frac(mp_advance(&p4, mp_step(dt, s1_param(ratio, 2))) + 0.1f * fb));
  fb = o4;
  float o3 = mp_sin(mp_frac(mp_advance(&p3, mp_step(dt, s1_param(ratio, 1))) + idx * o4));
  float o2 = mp_sin(mp_frac(mp_advance(&p2, mp_step(dt, s1_param(ratio, 0))) + idx * o3));
  float o1 = mp_sin(mp_frac(mp_advance(&p1, dt) + idx * o2));
  return o1 * s1_param(level, 0);
}
