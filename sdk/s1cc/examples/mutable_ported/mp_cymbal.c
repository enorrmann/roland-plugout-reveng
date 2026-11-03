/* mp_cymbal.c: Braids CYMBAL: six square waves at inharmonic ratios (the 808 hat's metal, two octaves lower than
 * a hat and at half the key tracking) through a band-pass, crossfaded with clocked noise; DSP B high-passes it.
 * Decay = AMP envelope (DECAY / RELEASE, SUSTAIN 0).
 * OSC1 COLOR = band-pass frequency, OSC2 COLOR = metal -> noise, OSC1 MIX = level. DSP B: mp_dsp_b.h (high-pass).
 * The COLORs follow their COLOR MOD source (s1_color). */
#define MP_FILTER MP_SVF_HP
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

/* Braids: root = increment of note 40 + pitch / 2; ratios 24273 / 12561 / 18417 / 22452 / 31858 x 2^-14 x 2^10 */
static const float kRatio[6] = { 1.0f, 1.4815f, 0.7667f, 1.1241f, 1.3704f, 1.9445f };
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph[6], pn, held, w, lo, bp;

S1_DSP_A {
  /* half the key tracking: the note's step to the power 1/2, around 1 kHz */
  float dt = 0.02f * s1_sqrt(mp_dt() * 50.0f);
  float metal = -3.0f;
  for (int i = 0; i < 6; i++)
    metal += mp_advance(&ph[i], mp_step(dt, kRatio[i])) >= 0.5f ? 1.0f : 0.0f;
  float n = mp_noise(&w);
  float q = pn + mp_step(dt, 24.0f);                   /* noise clock 24 x the root */
  if (q >= 1.0f) { q -= 1.0f; held = n; }
  pn = q;
  /* Chamberlin band-pass on the metal, f from COLOR (1 .. 12 kHz), q ~ 0.4 */
  float f = 0.065f * s1_exp2(3.5f * s1_color(0));
  lo += f * bp;
  float hi = metal * 0.33f - lo - 0.4f * bp;
  bp += f * hi;
  float m = bp * 0.8f;
  return (m + (held - m) * s1_color(1)) * s1_param(level, 0);
}
