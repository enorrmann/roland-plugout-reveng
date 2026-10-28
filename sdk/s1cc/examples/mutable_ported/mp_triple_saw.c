/* mp_triple_saw.c: Braids TRIPLE_SAW: three band-limited saws at the note and two intervals (Braids' chord table:
 * unison, fifths, octaves, minor / major chords as the knob turns).
 * OSC1 COLOR = interval of saw 2, OSC2 COLOR = interval of saw 3, OSC1 MIX = level. DSP B: mp_dsp_b.h. */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

static const float kIntervals[] = { 0.0f, 0.03f, 3.0f, 4.0f, 5.0f, 7.0f, 9.0f, 12.0f, 15.0f, 16.0f, 19.0f, 24.0f };
static float interval(int knob) {                      /* semitones, quantised to the table as Braids does */
  int i = knob * 12 / 256;
  return kIntervals[i];
}
/* ratio k and 1/k (for 1/dt of that saw: rdt / k, no division on the DSP) */
S1_PARAM(r2, S1_OSC1_COLOR, 2) { out[0] = powf(2.0f, interval(knob) / 12.0f); out[1] = 1.0f / out[0]; }
S1_PARAM(r3, S1_OSC2_COLOR, 2) { out[0] = powf(2.0f, interval(knob) / 12.0f); out[1] = 1.0f / out[0]; }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.25f + 0.5f * knob / 255.0f) / 2.0f; }

static float ph1, ph2, ph3;

static inline float saw(float *ph, float dt, float rdt) {
  float p = mp_advance(ph, dt);
  return mp_saw_bl(p, dt, rdt);
}

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float y = saw(&ph1, dt, rdt);
  y += saw(&ph2, dt * s1_param(r2, 0), rdt * s1_param(r2, 1));
  y += saw(&ph3, dt * s1_param(r3, 0), rdt * s1_param(r3, 1));
  return y * s1_param(level, 0);
}
