/* mp_saw_swarm.c: Braids SAW_SWARM: seven saws spread around the note (naive saws: at 96 kHz and with seven of them
 * the aliasing is masked, as in Braids, which does not band-limit them either).
 * OSC1 COLOR = detune (Braids: detune grows with the square of the knob), OSC1 MIX = level. DSP B: mp_dsp_b.h. */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

/* the frequency ratio of saw i (i - 3 semitone fractions): Braids' detune = (TIMBRE + 1024)^2 >> 9, x (i - 3) */
S1_PARAM(ratio, S1_OSC1_COLOR, 7) {
  float t = knob / 255.0f, det = (t + 0.03f) * (t + 0.03f) * 1.2f;      /* semitones between neighbours */
  for (int i = 0; i < 7; i++) out[i] = powf(2.0f, (i - 3) * det / 12.0f);
}
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.25f + 0.5f * knob / 255.0f) / 3.0f; }

static float ph[7];

S1_DSP_A {
  float dt = mp_dt(), sum = 0.0f;
  for (int i = 0; i < 7; i++) {
    float p = mp_advance(&ph[i], dt * s1_param(ratio, i));
    sum += p;
  }
  return (sum + sum - 7.0f) * s1_param(level, 0);
}
