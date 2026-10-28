/* mp_super_square.c: Plaits' SuperSquareOscillator: two band-limited squares, detuned below the knob's middle,
 * the second hard-synced and swept above it (a "supersaw" for squares).
 * OSC1 COLOR = shape, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.75f * knob / 255.0f) / 2.0f; }

static float p1, p2;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float a = mp_advance(&p1, dt);
  /* shape s: below 0.5 a slight detune r = 1 + 0.04 s, above it the second square synced at r = 1 + 16 (s - 0.5)^2;
     1 / r for its polyBLEP: below 0.5 1 - 0.04 s, above a seed in x = s - 0.5 and one Newton step (1 %) */
  float s = s1_color(0), x = fmaxf(s - 0.5f, 0.0f);
  float sync = s < 0.5f ? 0.0f : 1.0f;
  float r = s < 0.5f ? 1.0f + 0.04f * s : 1.0f + 16.0f * x * x;
  float ri = 1.087708f + x * (-2.781294f + x * 1.934073f);
  ri = s < 0.5f ? 1.0f - 0.04f * s : ri * (2.0f - r * ri);
  float d2 = mp_step(dt, r);
  float b = mp_advance(&p2, d2);
  /* synced: the second square's phase is the master's times the ratio (reset with it); free: its own phase */
  float bs = mp_frac(a * r);
  float pb = b + (bs - b) * sync;
  float y = mp_pulse_bl(a, 0.5f, dt, rdt) + mp_pulse_bl(pb, 0.5f, d2, rdt * ri);
  return y * s1_param(level, 0);
}
