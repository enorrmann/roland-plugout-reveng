/* mp_triple_triangle.c: Braids TRIPLE_TRIANGLE: three triangles at the note and two intervals (Braids' chord
 * table, as mp_triple_saw.c). A triangle has no jump: the DSP's triangle op, no polyBLEP.
 * OSC1 COLOR = interval of triangle 2, OSC2 COLOR = interval of triangle 3, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

static const float kRatio[12] = { 1.000000f, 1.001734f, 1.189207f, 1.259921f, 1.334840f, 1.498307f, 1.681793f, 2.000000f, 2.378414f, 2.519842f, 2.996614f, 4.000000f };

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.75f * knob / 255.0f) / 2.0f; }

static float ph1, ph2, ph3;

static inline float tri(float *ph, float dt) {
  float p = mp_advance(ph, dt);
  return s1_tri(p + p - 0.5f);
}

S1_DSP_A {
  float dt = mp_dt();
  float y = tri(&ph1, dt);
  y += tri(&ph2, dt * kRatio[(int)(s1_color(0) * 11.999f)]);
  y += tri(&ph3, dt * kRatio[(int)(s1_color(1) * 11.999f)]);
  return y * s1_param(level, 0);
}
