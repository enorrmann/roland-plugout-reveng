/* mp_triple_sine.c: Braids TRIPLE_SINE: three sines at the note and two intervals (Braids' chord table).
 * OSC1 COLOR = interval 2, OSC2 COLOR = interval 3, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

static const float kIntervals[] = { 0.0f, 0.03f, 3.0f, 4.0f, 5.0f, 7.0f, 9.0f, 12.0f, 15.0f, 16.0f, 19.0f, 24.0f };
/* 2^(interval / 12), chosen by the color as Braids quantises the knob */
static const float kRatio[12] = { 1.000000f, 1.001734f, 1.189207f, 1.259921f, 1.334840f, 1.498307f, 1.681793f, 2.000000f, 2.378414f, 2.519842f, 2.996614f, 4.000000f };
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.75f * knob / 255.0f) / 2.0f; }

static float p1, p2, p3;

S1_DSP_A {
  float dt = mp_dt();
  float y = mp_sin(mp_advance(&p1, dt));
  y += mp_sin(mp_advance(&p2, dt * kRatio[(int)(s1_color(0) * 11.999f)]));
  y += mp_sin(mp_advance(&p3, dt * kRatio[(int)(s1_color(1) * 11.999f)]));
  return y * s1_param(level, 0);
}
