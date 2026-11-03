/* mp_wave_terrain.c: Plaits wave terrain engine: a circle of radius r around (offset, 0) traced at the note, the
 * output the height of a terrain z(x, y) along it. Terrain 0 of Plaits (no division):
 * z = (squash(sin(8 x)) - sin(4 y (x + 1.571))) 0.57, squash(u) = 2u / (1 + |2u|) (here the cubic soft clip).
 * OSC1 COLOR = radius, OSC2 COLOR = offset, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_filter.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt();
  float p = mp_advance(&ph, dt);
  float tame = fminf(fmaxf(1.4f - 20.0f * dt, 0.1f), 1.0f);
  float r = (0.1f + 0.9f * s1_color(0)) * tame, o = 1.9f * s1_color(1) - 0.95f;
  float x = r * mp_cos(p) * (1.0f - fabsf(o)) + o, y = r * mp_sin(p);
  /* sin(2 pi u) of u = 1.273 x and u = 0.637 y (x + 1.571), both made periodic by a wrap */
  float a = mp_soft(mp_sin(mp_frac(1.273f * x)) * 2.0f);
  float b = mp_sin(mp_frac(0.637f * y * (x + 1.571f)));
  return (a - b) * 0.5f * s1_param(level, 0);
}
