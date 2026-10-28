/* mp_formant.c: Plaits' FormantOscillator: a sine at a formant frequency, reset at every period of the note
 * (hard sync) and faded by the period (a falling window), so its spectrum is a formant peak over the note's harmonics.
 * OSC1 COLOR = formant (1 .. 16 x the note), OSC2 COLOR = phase shift, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * COLOR MOD on OSC2 COLOR (s1_color); OSC1 COLOR is a table. */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(ratio, S1_OSC1_COLOR, 1) { out[0] = powf(16.0f, knob / 255.0f); }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float p = mp_advance(&ph, mp_dt());
  float f = mp_frac(p * s1_param(ratio, 0) + s1_color(1));
  return mp_sin(f) * (1.0f - p) * s1_param(level, 0);
}
