/* mp_morph.c: Braids MORPH: triangle -> saw -> square -> pulse as the knob turns (two shapes crossfaded, the last
 * third a square whose width narrows, mixed with a sine), then a one-pole low-pass and an overdrive ("fuzz").
 * OSC1 COLOR = shape, OSC2 COLOR = low-pass + fuzz, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph, lp;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float p = mp_advance(&ph, dt);
  float c = s1_color(0) * 3.0f;                         /* 0..1 tri->saw, 1..2 saw->square, 2..3 square pw + sine */
  float saw = mp_saw_bl(p, dt, rdt);
  float tri = s1_tri(p + p - 0.5f);
  float pw = fmaxf(0.5f - 0.48f * fmaxf(c - 2.0f, 0.0f), 2.0f * dt);
  float sq = mp_pulse_bl(p, pw, dt, rdt);
  float a = fminf(c, 1.0f), b = fminf(fmaxf(c - 1.0f, 0.0f), 1.0f), d = fmaxf(c - 2.0f, 0.0f);
  float x = tri + (saw - tri) * a;
  x = x + (sq - x) * b;
  x = x + (0.5f * (sq + mp_sin(p)) - x) * d;
  /* Braids: cutoff falls and fuzz grows with COLOR (the fuzz fades out above note 80) */
  float g = s1_color(1);
  float f = fminf(1.0f, 0.02f + 32.0f * dt * s1_exp2(4.0f - 6.0f * g));
  lp += (x - lp) * f;
  float fz = lp * (1.0f + 3.0f * g);
  fz = fz * (1.5f - 0.5f * fz * fz);                    /* cubic overdrive, valid for |fz| <= 1 */
  fz = s1_range(fminf(fmaxf(fz, -1.0f), 1.0f), -1.0f, 1.0f);
  return (x + (fz - x) * g) * s1_param(level, 0);
}
