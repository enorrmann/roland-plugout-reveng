/* mp_filter.h: filters of the mutable_ported kernels, for DSP B (mp_dsp_a.h for the rules). The cutoff arrives as w,
 * 2 pi fc / fs at 96 kHz; the coefficients are linear in w (no tan(), no division), which the high sample rate
 * allows, and the SVF runs twice per sample (each half with w/2): stable up to ~20 kHz at every resonance. */
#ifndef MP_FILTER_H_
#define MP_FILTER_H_
#include <math.h>
#include "s1dsp.h"

#ifndef MP_FS
#define MP_FS 96000.0f
#endif

/* soft clip, cubic, odd: x - x^3 / 3 on [-1, 1], 2/3 beyond (the shape of Plaits' overdrive at low drive) */
static inline float mp_soft(float x) {
  x = s1_clamp(x);
  return x - x * x * x * (1.0f / 3.0f);
}


/* state variable filter (Chamberlin, as Braids' svf.h, in float), double sampled. Stable while f + q < 2: q (the
   damping) 1.4 (no resonance, Butterworth-like) .. 0.02, f at most 0.55 (~17 kHz at 2 fs). Outputs low, band and
   high pass of the last half sample */
struct mp_svf { float lp, bp; };
struct mp_svf_out { float lp, bp, hp; };

static inline struct mp_svf_out mp_svf_process(struct mp_svf *s, float x, float w, float q) {
  float f = fminf(w, 0.55f);                  /* per half sample (w / 2 at 2 fs = w at fs) */
  struct mp_svf_out o;
  float lp = s->lp, bp = s->bp, hp;
  lp += f * bp;
  hp = x - lp - q * bp;
  bp += f * hp;
  lp += f * bp;
  hp = x - lp - q * bp;
  bp += f * hp;
  s->lp = lp;
  s->bp = bp;
  o.lp = lp;
  o.bp = bp;
  o.hp = hp;
  return o;
}

/* 4-pole ladder (the transistor ladder of Plaits' virtual analog VCF engine, in its simplest form): four one-poles
   y += a (x - y), a = w (stable up to a = 1), resonance k (0..4: self-oscillates near 4) fed back through a soft
   clip; the passband loss of the feedback is made up by (1 + k / 2) */
struct mp_ladder { float y1, y2, y3, y4; };

static inline float mp_ladder_process(struct mp_ladder *s, float x, float w, float k) {
  float a = s1_clamp(w);
  float in = mp_soft((x - k * s->y4) * 0.5f) * 2.0f;
  s->y1 += a * (in - s->y1);
  s->y2 += a * (s->y1 - s->y2);
  s->y3 += a * (s->y2 - s->y3);
  s->y4 += a * (s->y3 - s->y4);
  return s->y4 * (1.0f + 0.5f * k);
}

/* one pole low pass and its high pass complement */
struct mp_onepole { float y; };
static inline float mp_onepole_lp(struct mp_onepole *s, float x, float w) {
  s->y += s1_clamp(w) * (x - s->y);
  return s->y;
}

/* low pass gate (Plaits fx/low_pass_gate.h): one control g 0..1 opens the VCA and the low pass together, the
   vactrol's gesture; the cutoff follows g squared (fc from ~20 Hz to w_max) */
struct mp_lpg { struct mp_svf svf; };
static inline float mp_lpg_process(struct mp_lpg *s, float x, float g, float w_max, float q) {
  float w = w_max * g * g + 0.0013f;
  struct mp_svf_out o = mp_svf_process(&s->svf, x, w, q);
  return o.lp * g;
}

#endif
