/* mp_triple_saw.c: Braids TRIPLE_SAW: three band-limited saws at the note and two intervals (Braids' chord table:
 * unison, fifths, octaves, minor / major chords as the knob turns).
 * OSC1 COLOR = interval of saw 2, OSC2 COLOR = interval of saw 3, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

static const float kIntervals[] = { 0.0f, 0.03f, 3.0f, 4.0f, 5.0f, 7.0f, 9.0f, 12.0f, 15.0f, 16.0f, 19.0f, 24.0f };
/* 2^(interval / 12) and its inverse (for 1/dt of that saw: rdt / k, no division on the DSP), chosen by the color as
   Braids quantises the knob */
static const float kRatio[12] = { 1.000000f, 1.001734f, 1.189207f, 1.259921f, 1.334840f, 1.498307f, 1.681793f, 2.000000f, 2.378414f, 2.519842f, 2.996614f, 4.000000f };
static const float kRatioInv[12] = { 1.000000f, 0.998269f, 0.840896f, 0.793701f, 0.749154f, 0.667420f, 0.594604f, 0.500000f, 0.420448f, 0.396850f, 0.333710f, 0.250000f };

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.75f * knob / 255.0f) / 2.0f; }

static float ph1, ph2, ph3;

static inline float saw(float *ph, float dt, float rdt) {
  float p = mp_advance(ph, dt);
  return mp_saw_bl(p, dt, rdt);
}

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float y = saw(&ph1, dt, rdt);
  int i2 = (int)(s1_color(0) * 11.999f), i3 = (int)(s1_color(1) * 11.999f);
  y += saw(&ph2, dt * kRatio[i2], rdt * kRatioInv[i2]);
  y += saw(&ph3, dt * kRatio[i3], rdt * kRatioInv[i3]);
  return y * s1_param(level, 0);
}
