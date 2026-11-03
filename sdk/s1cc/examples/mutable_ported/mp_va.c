/* mp_va.c: Plaits virtual analog engine, its core: two oscillators, each a band-limited saw -> pulse morph, the
 * second detuned by an interval (Plaits' detune / chord table, here a continuous 0 .. +19 semitones with the unison
 * and fifth / octave spots of the knob as Plaits quantises them: a table on the PC).
 * OSC1 COLOR = shape (saw -> square -> narrow pulse), OSC2 COLOR = detune of the second oscillator (table),
 * OSC1 MIX = level. DSP B: mp_dsp_b.h. OSC1 COLOR follows its COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.75f * knob / 255.0f) * 0.5f; }
/* ratio of the second oscillator and its inverse: the knob in 0 .. 19 semitones with plateaus at 0, 7, 12, 19 */
S1_PARAM(det, S1_OSC2_COLOR, 2) {
  static const float kSpots[5] = { 0.0f, 0.07f, 7.0f, 12.0f, 19.0f };
  float x = knob / 255.0f * 3.999f;
  int i = (int)x;
  float fr = x - i;
  fr = fr < 0.25f ? 0.0f : fr > 0.75f ? 1.0f : (fr - 0.25f) * 2.0f;   /* plateaus at each spot */
  float semis = kSpots[i] + (kSpots[i + 1] - kSpots[i]) * fr;
  out[0] = powf(2.0f, semis / 12.0f);
  out[1] = 1.0f / out[0];
}

static float pa, pb;

static inline float osc(float *ph, float dt, float rdt, float shape) {
  float p = mp_advance(ph, dt);
  float pw = fmaxf(0.5f - 0.45f * fmaxf(shape + shape - 1.0f, 0.0f), 2.0f * dt);
  float s0 = mp_saw_bl(p, dt, rdt), s1 = mp_saw_bl(p - pw, dt, rdt);
  float sq = s1 - s0 + pw + pw - 1.0f;
  return s0 + (sq - s0) * fminf(shape + shape, 1.0f);
}

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt(), sh = s1_color(0);
  float db = mp_step(dt, s1_param(det, 0));
  float y = osc(&pa, dt, rdt, sh) + osc(&pb, db, rdt * s1_param(det, 1), sh);
  return y * s1_param(level, 0);
}
