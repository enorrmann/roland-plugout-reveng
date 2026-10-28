/* mp_dsp_a.h: DSP A building blocks of the mutable_ported kernels (oscillators, noise, folding) (docs/examples_mutable_ported_README.md), written for the
 * SYSTEM-1 slave DSPs as s1cc compiles them:
 *   - no division in the voice: what depends on a knob goes to an S1_PARAM table (runs on the PC), what depends on
 *     the note comes from the host's per-note words (s1_dt, s1_rdt); recipseed, the hardware reciprocal, is not
 *     proven on the SYSTEM-1 (W910);
 *   - phases are floats in [0, 1): `p += dt; if (p >= 1) p -= 1;` is recognised and exact (ph_wrap);
 *   - every `if` costs both sides (the DSP has no branches): branch-free forms where they are cheaper;
 *   - one sample per call, at 96 kHz.
 * Algorithms after Mutable Instruments' Braids / Plaits / stmlib (Emilie Gillet, MIT), rewritten for this DSP. */
#ifndef MP_DSP_A_H_
#define MP_DSP_A_H_
#include <math.h>
#include "s1dsp.h"

#ifndef MP_FS
#define MP_FS 96000.0f
#endif

/* the voice's phase step in cycles per sample (phase 0..1) and its inverse: per-note tables the host writes with the
   pitch (s1_dt / s1_rdt: two loads; computing them would take two exp2 and the division recipseed) */
static inline float mp_dt(void) { return s1_dt(); }
static inline float mp_rdt(void) { return s1_rdt(); }

/* frac(x) for x of either sign: one wrap (the DSP's ph_wrap works on [-1, 1)) */
static inline float mp_frac(float x) { return 0.5f * s1_wrap(x + x - 1.0f) + 0.5f; }

/* a phase step dt x ratio, at most 1/2 a cycle per sample: the wrap below takes one cycle off, and a ratio x a high
   note can pass 1 (the phase would then leave [0, 1) and grow); above Nyquist there is nothing to hear anyway */
static inline float mp_step(float dt, float ratio) { return fminf(dt * ratio, 0.5f); }

/* advance a phase in [0, 1) (dt < 1) */
static inline float mp_advance(float *p, float dt) {
  float x = *p + dt;
  if (x >= 1.0f) x -= 1.0f;
  *p = x;
  return x;
}

/* band-limited sawtooth 2 frac(t) - 1, its edge at t = 0 (polyBLEP, stmlib/dsp/polyblep.h, two-sample form). With w
   the phase taken to -1/2 .. 1/2 by one wrap and u = w / dt limited to [-1, 1] (one sample each side of the edge),
   naive saw + stmlib's residual is exactly 2w - u (2 - |u|): no sign to compute (a sign function blurs at the edge
   and gave one-sample errors where a period lands on it, 2026-10-28), about 8 ops. Everything with a jump is built
   from it: a pulse is the difference of two saws, a sync reset a saw scaled by the jump. dt < 1/2, rdt = 1/dt */
static inline float mp_saw_bl(float t, float dt, float rdt) {
  float w = 0.5f * s1_wrap(t + t);
  float tc = fmaxf(fminf(w, dt), -dt);
  float u = s1_range(tc * rdt, -1.0f, 1.0f);
  return w + w - u * (2.0f - fabsf(u));
}

/* band-limited pulse: +1 while frac(t) < pw, -1 after, edges at 0 and pw (two saws) */
static inline float mp_pulse_bl(float t, float pw, float dt, float rdt) {
  return mp_saw_bl(t - pw, dt, rdt) - mp_saw_bl(t, dt, rdt) + pw + pw - 1.0f;
}

/* sine of a phase in [0, 1): the DSP's triangle (one op) through an odd polynomial, |error| < 2e-4 */
static inline float mp_sin(float p) {
  float t = s1_tri(p + p);                  /* +-1 triangle in phase with sin(2 pi p) */
  float t2 = t * t;
  return t * (1.5704f + t2 * (-0.6427f + t2 * 0.0727f));
}
static inline float mp_cos(float p) {
  float t = s1_tri(p + p + 0.5f);
  float t2 = t * t;
  return t * (1.5704f + t2 * (-0.6427f + t2 * 0.0727f));
}

/* wavefolder: the DSP triangle of the scaled signal (Braids' fold tables are this shape, without their smoothing) */
static inline float mp_fold(float x) { return s1_tri(x * 0.5f - 0.5f); }

/* a free-running noise source: a Weyl sequence through a squaring hash (no integer ops; white enough for audio).
   state *w in [0, 1) */
static inline float mp_noise(float *w) {
  float x = mp_advance(w, 0.6180339887f);
  float h = s1_wrap(x * x * 4093.0f);       /* fract-like scramble, -1..1 */
  return h;
}

#endif
