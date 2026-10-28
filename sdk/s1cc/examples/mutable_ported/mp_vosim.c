/* mp_vosim.c: Braids VOSIM: two formants (squared sines at their own rates, restarted every period of the note) whose
 * sum is windowed by the period: a vowel-like, buzzy pulse train.
 * OSC1 COLOR = formant 1 (1 .. 8 x the note), OSC2 COLOR = formant 2 (2 .. 16 x), OSC1 MIX = level. DSP B: mp_voice.h. */
#include <math.h>
#include "mp_voice.h"

S1_PARAM(f1, S1_OSC1_COLOR, 1) { out[0] = powf(8.0f, knob / 255.0f); }
S1_PARAM(f2, S1_OSC2_COLOR, 1) { out[0] = 2.0f * powf(8.0f, knob / 255.0f); }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float p = mp_advance(&ph, mp_dt());
  float a = mp_sin(mp_frac(p * s1_param(f1, 0)));
  float b = mp_sin(mp_frac(p * s1_param(f2, 0)));
  float w = 1.0f - p;
  return (a * a + b * b - 1.0f) * w * s1_param(level, 0);
}
