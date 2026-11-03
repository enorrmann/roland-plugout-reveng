/* mp_square_sub.c: Braids SQUARE_SUB: a pulse with width plus a square one or two octaves down (Braids: COLOR
 * below half = two octaves, above = one, the sub level growing from the middle).
 * OSC1 COLOR = width, OSC2 COLOR = sub (left: -2 oct, right: -1 oct), OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.75f * knob / 255.0f) * 0.67f; }

static float ph, ps;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float p = mp_advance(&ph, dt);
  float c = s1_color(1);
  float two = c < 0.5f ? 1.0f : 0.0f;                   /* two octaves down on the left half */
  float k = 0.5f - 0.25f * two;                          /* sub step: 1/2 or 1/4 of the note */
  float q = mp_advance(&ps, dt * k);
  float pw = fminf(fmaxf(0.5f - 0.45f * s1_color(0), 2.0f * dt), 0.5f);
  float sq = mp_pulse_bl(p, pw, dt, rdt);
  float sub = mp_pulse_bl(q, 0.5f, dt * k, rdt * (2.0f + 2.0f * two));
  return (sq + sub * fabsf(c + c - 1.0f)) * s1_param(level, 0);
}
