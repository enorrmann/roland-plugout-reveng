/* mp_triple_square.c: Braids TRIPLE_SQUARE: three band-limited squares at the note and two intervals (Braids'
 * chord table, as mp_triple_saw.c).
 * OSC1 COLOR = interval of square 2, OSC2 COLOR = interval of square 3, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

static const float kRatio[12] = { 1.000000f, 1.001734f, 1.189207f, 1.259921f, 1.334840f, 1.498307f, 1.681793f, 2.000000f, 2.378414f, 2.519842f, 2.996614f, 4.000000f };
static const float kRatioInv[12] = { 1.000000f, 0.998269f, 0.840896f, 0.793701f, 0.749154f, 0.667420f, 0.594604f, 0.500000f, 0.420448f, 0.396850f, 0.333710f, 0.250000f };

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.75f * knob / 255.0f) / 2.0f; }

static float ph1, ph2, ph3;

static inline float sq(float *ph, float dt, float rdt) {
  return mp_pulse_bl(mp_advance(ph, dt), 0.5f, dt, rdt);
}

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float y = sq(&ph1, dt, rdt);
  int i2 = (int)(s1_color(0) * 11.999f), i3 = (int)(s1_color(1) * 11.999f);
  y += sq(&ph2, dt * kRatio[i2], rdt * kRatioInv[i2]);
  y += sq(&ph3, dt * kRatio[i3], rdt * kRatioInv[i3]);
  return y * s1_param(level, 0);
}
