/* mp_nes_triangle.c: Plaits' NES triangle (chiptune engine): a triangle quantised to 16 steps, the 4-bit DAC sound.
 * OSC1 COLOR = steps 4 .. 32, OSC1 MIX = level. DSP B: mp_dsp_b.h. */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(steps, S1_OSC1_COLOR, 2) { float n = 4.0f + 28.0f * knob / 255.0f; out[0] = n * 0.5f; out[1] = 2.0f / n; }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float p = mp_advance(&ph, mp_dt());
  float t = s1_tri(p + p);                                  /* -1 .. 1 */
  float q = floorf(t * s1_param(steps, 0)) * s1_param(steps, 1);
  return q * s1_param(level, 0);
}
