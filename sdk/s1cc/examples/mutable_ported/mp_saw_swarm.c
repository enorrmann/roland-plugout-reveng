/* mp_saw_swarm.c: Braids SAW_SWARM: seven saws spread around the note (naive saws: at 96 kHz and with seven of them
 * the aliasing is masked, as in Braids, which does not band-limit them either).
 * OSC1 COLOR = detune (Braids: detune grows with the square of the knob), OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

/* the frequency ratio of saw i (i - 3 semitone fractions): Braids' detune = (TIMBRE + 1024)^2 >> 9, x (i - 3) */
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.75f * knob / 255.0f) / 3.0f; }

static float ph[7];

S1_DSP_A {
  /* det = 1.2 (c + 0.03)^2 semitones between neighbours; saw i at 2^((i - 3) det / 12): r^1..3 and their inverses */
  float t = s1_color(0) + 0.03f, x = t * t * (1.2f / 12.0f);
  float r1 = s1_exp2(x), q1 = s1_exp2(-x), r2 = r1 * r1, q2 = q1 * q1;
  const float ratio[7] = { q2 * q1, q2, q1, 1.0f, r1, r2, r2 * r1 };
  float dt = mp_dt(), sum = 0.0f;
  for (int i = 0; i < 7; i++) {
    float p = mp_advance(&ph[i], dt * ratio[i]);
    sum += p;
  }
  return (sum + sum - 7.0f) * s1_param(level, 0);
}
