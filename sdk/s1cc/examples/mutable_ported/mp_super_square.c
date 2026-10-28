/* mp_super_square.c: Plaits' SuperSquareOscillator: two band-limited squares, detuned below the knob's middle,
 * the second hard-synced and swept above it (a "supersaw" for squares).
 * OSC1 COLOR = shape, OSC1 MIX = level. DSP B: mp_dsp_b.h. */
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

/* Plaits: ratio = shape < 0.5 ? 0.51 + 0.98 shape : 1 + 16 (shape - 0.5)^2; above 0.5 the second square is synced */
S1_PARAM(shape, S1_OSC1_COLOR, 3) {
  float s = knob / 255.0f;
  float r = s < 0.5f ? 1.0f + 0.04f * s : 1.0f + 16.0f * (s - 0.5f) * (s - 0.5f);
  out[0] = r;
  out[1] = 1.0f / r;
  out[2] = s < 0.5f ? 0.0f : 1.0f;                 /* sync on */
}
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.25f + 0.5f * knob / 255.0f) / 2.0f; }

static float p1, p2;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float a = mp_advance(&p1, dt);
  float d2 = mp_step(dt, s1_param(shape, 0));
  float b = mp_advance(&p2, d2);
  float sync = s1_param(shape, 2);
  /* synced: the second square's phase is the master's times the ratio (reset with it); free: its own phase */
  float bs = mp_frac(a * s1_param(shape, 0));
  float pb = b + (bs - b) * sync;
  float y = mp_pulse_bl(a, 0.5f, dt, rdt) + mp_pulse_bl(pb, 0.5f, d2, rdt * s1_param(shape, 1));
  return y * s1_param(level, 0);
}
