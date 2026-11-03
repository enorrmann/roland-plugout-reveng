/* mp_buzz.c: Braids BUZZ: a band-limited comb (pulse train) and a second one detuned by COLOR. Braids reads
 * tables of band-limited combs per pitch zone; here the comb is a raised cosine to a power, c^k with
 * c = (1 + cos 2 pi p) / 2, whose spectrum is Gaussian with ~ sqrt(k) harmonics: k follows TIMBRE and falls with
 * the pitch so the harmonics stay below ~20 kHz. Written as e^(k (c - 1)) (the same pulse for large k, Bessel
 * spectrum), one exp2 with its argument clamped; its mean (~ 1 / sqrt(1 + pi k)) is taken off.
 * OSC1 COLOR = brightness, OSC2 COLOR = detune of the second comb, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph1, ph2;

static inline float comb(float p, float k, float m) {
  float c = 0.5f + 0.5f * mp_cos(p);                  /* 0..1, 1 at the pulse */
  return s1_exp2(fmaxf(k * (c - 1.0f), -16.0f)) - m;  /* k already x log2(e) */
}

S1_DSP_A {
  float dt = mp_dt();
  /* harmonics n ~ 0.21 / dt at most (20 kHz), n^2 ~ k: lk = log2 k = 2 log2 n, from 1 to that */
  float lmax = -2.0f * s1_log2(dt) - 4.5f;
  float lk = fmaxf(0.5f, fminf(1.0f + 10.0f * s1_color(0), lmax));
  float k = s1_exp2(lk);
  float m = s1_rsqrt(1.0f + 3.14159f * k);           /* the mean, ~ 1 / sqrt(1 + pi k) */
  k = k * 1.442695f;
  float a = comb(mp_advance(&ph1, dt), k, m);
  float b = comb(mp_advance(&ph2, mp_step(dt, 1.0f + 0.06f * s1_color(1))), k, m);
  return (a + b) * s1_param(level, 0);
}
