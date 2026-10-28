/* mp_vosim.c: Braids VOSIM: two formants (squared sines at their own rates, restarted every period of the note) whose
 * sum is windowed by the period: a vowel-like, buzzy pulse train.
 * OSC1 COLOR = formant 1 (1 .. 8 x the note), OSC2 COLOR = formant 2 (2 .. 16 x), OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float p = mp_advance(&ph, mp_dt());
  float a = mp_sin(mp_frac(p * s1_exp2(3.0f * s1_color(0))));
  float b = mp_sin(mp_frac(p * s1_exp2(3.0f * s1_color(1) + 1.0f)));
  float w = 1.0f - p;
  return (a * a + b * b - 1.0f) * w * s1_param(level, 0);
}
