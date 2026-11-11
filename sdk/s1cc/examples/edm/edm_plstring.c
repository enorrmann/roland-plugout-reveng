/* edm_plstring.c: Plaits' string (plaits/dsp/physical_modelling/string.cc) on SYSTEM-S1CC, 6 voices, all in C: the
 * excitation on the slave (S1_DSP_A), the string on master B (S1_MASTER) in its own line of the delay memory
 * (s1_line_tap / s1_line_write, docs/vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md §3.5). The same string as
 * edm_plstring_asm.c (MP-PLSTRING, ran on the device 2026-11-04 with the loop in microcode/edm_str.s master_pstr and
 * the tuning in make_s1cc.py pl_loop); here the loop and its tuning are this file's:
 *   damping    two equal one-poles b at 24 .. 84 semitones above the note (FILTER CUTOFF): Plaits' SVF at Q 0.5
 *   dispersion three first-order allpasses a (OSC2 COLOR): the 8th partial 300 amt^3 cents sharp on every note
 *   tuning     d + 1 + the phase delay of the interpolation g, the one-poles and the allpasses at the note = the
 *              period (S1_VOICE_PARAM: bisection on the PC, per note and 8 steps of each knob)
 *   decay      the feedback per pass: AMP DECAY while the key is held, AMP RELEASE after it
 *   output     the string alone (not the burst), as Plaits
 * Excitation: a 5 ms burst of noise per note through a one-pole whose cutoff is OSC1 COLOR (Plaits' TIMBRE in its
 * string engine); OSC1 MIX = level. */
#include <complex.h>
#include <math.h>

#include "../mutable_ported/include/mp_dsp_a.h"

S1_BUILD("NAME=MP-PLSTRC");
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }
S1_PARAM(bright, S1_OSC1_COLOR, 1)
{ /* the burst's one-pole: 0.03 (dark) .. 1 (white noise) */
  float k = knob / 255.0f;
  out[0] = 0.03f + 0.97f * k * k;
}

static float env, w, lp, last_trig;

S1_DSP_A
{
  float trig = s1_trig();
  if (trig != last_trig)
    env = 1.0f;
  last_trig = trig;
  float e = env;
  env = e * 0.9979f; /* 5 ms (1 / (5 ms x 96 kHz)) */
  float x = mp_noise(&w) * e;
  float y = lp + s1_param(bright, 0) * (x - lp);
  lp = y;
  return y * s1_param(level, 0);
}

/* ---- the string (master B) ---- */

/* the feedback per pass of the loop: AMP DECAY with the key held (0.9 .. 0.9999), AMP RELEASE after it */
S1_PARAM(hold, S1_AMP_DECAY, 1) { out[0] = 1.0f - powf(10.0f, -(1.0f + 3.0f * knob / 255.0f)); }
S1_PARAM(rel, S1_AMP_RELEASE, 1) { out[0] = 1.0f - powf(10.0f, -(0.3f + 3.0f * knob / 255.0f)); }

#define SR 96000.0
#define STRETCH 300.0 /* cents of the 8th partial at full OSC2 COLOR (x amt^3) */

static double ph(double complex h) { return -carg(h); } /* a stage's phase lag, 0 .. pi (no wrap) */
static double ap3_delay(double a, double f)
{ /* the three allpasses' delay at f, in samples */
  double ww = 2 * M_PI * f / SR;
  double complex z = cexp(-I * ww);
  return 3 * ph((a + z) / (1 + a * z)) / ww;
}

/* per note, FILTER CUTOFF (ka) and OSC2 COLOR (kb): the request's delay d (whole samples), the interpolation g, the
 * one-poles' b and the allpasses' a, the whole loop tuned to the period by bisection (make_s1cc.py pl_loop, which
 * MP-PLSTRING ran with); a smaller dispersion when the allpasses alone are longer than the period allows */
S1_VOICE_PARAM(tune, 4, S1_FILTER_CUTOFF, S1_OSC2_COLOR)
{
  double f0 = 440.0 * pow(2.0, (note - 69) / 12.0), p = SR / f0, w0 = 2 * M_PI / p;
  double complex z = cexp(-I * w0);
  double fc = f0 * pow(2.0, (24.0 + 60.0 * ka / 255.0) / 12.0);
  double b = fmin(1.0, 1.0 - exp(-2 * M_PI * fc / SR));
  double amt = kb / 255.0, want = STRETCH * amt * amt * amt;
  for (int tries = 0; tries < 20; tries++)
  {
    double a = 0.0, lo_a = -0.995, hi_a = 0.0;
    if (want > 0 && 8 * f0 < SR / 2)
      for (int i = 0; i < 50; i++)
      {
        a = (lo_a + hi_a) / 2;
        if (1200 * log2(1 + (ap3_delay(a, f0) - ap3_delay(a, 8 * f0)) / p) > want)
          lo_a = a;
        else
          hi_a = a;
      }
    double fil = 2 * ph(b / (1 - (1 - b) * z)) + 3 * ph((a + z) / (1 + a * z));
#define TOTAL(t) (floor(t) + 1 + (ph((1 - ((t) - floor(t))) + ((t) - floor(t)) * z) + fil) / w0)
    if (TOTAL(0.0) > p)
    { /* the filters alone are longer than the period */
      want *= 0.5;
      continue;
    }
    double lo = 0.0, hi = p;
    for (int i = 0; i < 60; i++)
    {
      double t = (lo + hi) / 2;
      if (TOTAL(t) > p)
        hi = t;
      else
        lo = t;
    }
#undef TOTAL
    double t = (lo + hi) / 2;
    out[0] = (float)floor(t);
    out[1] = (float)(t - floor(t));
    out[2] = (float)b;
    out[3] = (float)a;
    return;
  }
  out[0] = (float)fmax(floor(p - 1), 0.0);
  out[1] = 0.0f;
  out[2] = (float)b;
  out[3] = 0.0f;
}

static float lp1, lp2, ax1, ay1, ax2, ay2, ax3, ay3;

/* x: the voice's excitation (what S1_DSP_A returned); returns the string */
S1_MASTER
{
  float d = s1_vparam(tune, 0), g = s1_vparam(tune, 1), b = s1_vparam(tune, 2), a = s1_vparam(tune, 3);
  float x0 = s1_line_tap(0, d, 0), x1 = s1_line_tap(0, d, 1); /* w[n - d - 1], w[n - d - 2] */
  float z = x0 + g * (x1 - x0);
  lp1 += b * (z - lp1); /* damping: two one-poles */
  lp2 += b * (lp1 - lp2);
  float y1 = a * (lp2 - ay1) + ax1; /* dispersion: y = a (u - y[n-1]) + u[n-1], three times */
  ax1 = lp2;
  ay1 = y1;
  float y2 = a * (y1 - ay2) + ax2;
  ax2 = y1;
  ay2 = y2;
  float y3 = a * (y2 - ay3) + ax3;
  ax3 = y2;
  ay3 = y3;
  /* the feedback by the gate as a sum (no compare on master B: only the op forms of asm/master_b_forms.txt ran) */
  float fb = s1_param(rel, 0) + s1_gate() * (s1_param(hold, 0) - s1_param(rel, 0));
  s1_line_write(0, x + fb * y3);
  return y3;
}
