/* mp_nes_triangle.c: Plaits' NES triangle (chiptune engine): a triangle quantised to 16 steps, the 4-bit DAC sound.
 * OSC1 COLOR = steps 4 .. 32, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float p = mp_advance(&ph, mp_dt());
  float t = s1_tri(p + p);                                  /* -1 .. 1 */
  float e = 1.0f + 3.0f * s1_color(0);                      /* steps n = 2^(e + 1): 4 .. 32 */
  float q = floorf(t * s1_exp2(e)) * s1_exp2(-e);
  return q * s1_param(level, 0);
}
