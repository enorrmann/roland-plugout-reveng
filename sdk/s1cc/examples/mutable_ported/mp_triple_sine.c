/* mp_triple_sine.c: Braids TRIPLE_SINE: three sines at the note and two intervals (Braids' chord table).
 * OSC1 COLOR = interval 2, OSC2 COLOR = interval 3, OSC1 MIX = level. DSP B: mp_voice.h. */
#include <math.h>
#include "mp_voice.h"

static const float kIntervals[] = { 0.0f, 0.03f, 3.0f, 4.0f, 5.0f, 7.0f, 9.0f, 12.0f, 15.0f, 16.0f, 19.0f, 24.0f };
S1_PARAM(r2, S1_OSC1_COLOR, 1) { out[0] = powf(2.0f, kIntervals[knob * 12 / 256] / 12.0f); }
S1_PARAM(r3, S1_OSC2_COLOR, 1) { out[0] = powf(2.0f, kIntervals[knob * 12 / 256] / 12.0f); }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.25f + 0.5f * knob / 255.0f) / 2.0f; }

static float p1, p2, p3;

S1_DSP_A {
  float dt = mp_dt();
  float y = mp_sin(mp_advance(&p1, dt));
  y += mp_sin(mp_advance(&p2, dt * s1_param(r2, 0)));
  y += mp_sin(mp_advance(&p3, dt * s1_param(r3, 0)));
  return y * s1_param(level, 0);
}
