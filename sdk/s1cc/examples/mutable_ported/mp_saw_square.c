/* mp_saw_square.c: Braids SAW_SQUARE: a variable saw (a saw whose ramp is cut by a notch of width w, Braids
 * VARIABLE_SAW) crossfaded with a square of the same width.
 * OSC1 COLOR = width, OSC2 COLOR = saw / square balance, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float p = mp_advance(&ph, dt);
  float pw = fminf(fmaxf(0.5f + 0.45f * s1_color(0), 2.0f * dt), 1.0f - 2.0f * dt);
  float s0 = mp_saw_bl(p, dt, rdt), s1 = mp_saw_bl(p - pw, dt, rdt);
  float vsaw = 0.5f * (s0 + s1);                         /* two saws a width apart: Braids' variable saw */
  float sq = s1 - s0 + pw + pw - 1.0f;
  return (vsaw + (0.58f * sq - vsaw) * s1_color(1)) * s1_param(level, 0);
}
