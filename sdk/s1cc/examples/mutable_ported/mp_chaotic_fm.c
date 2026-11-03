/* mp_chaotic_fm.c: Braids CHAOTIC_FEEDBACK_FM: two-operator phase modulation where the modulator's frequency
 * follows the carrier's last output sample (the feedback goes into the frequency, not the phase): chaotic at high
 * index.
 * OSC1 COLOR = modulation index, OSC2 COLOR = modulator pitch (-2 .. +3 octaves around one octave up),
 * OSC1 MIX = level. DSP B: mp_dsp_b.h. The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph, pmod, prev;

S1_DSP_A {
  float dt = mp_dt();
  float p = mp_advance(&ph, dt);
  float pm = 0.5f * s1_color(0) * mp_sin(pmod);
  float y = mp_sin(mp_frac(p + pm));
  /* Braids: increment of (note + 12 + COLOR) / 2 x (129 + prev / 512) / 256: 0.25 .. 0.75 x, around 0.5 */
  float dm = mp_step(dt, s1_exp2(5.0f * s1_color(1) - 2.0f) * (0.5f + 0.25f * prev));
  mp_advance(&pmod, dm);
  prev = y;
  return y * s1_param(level, 0);
}
