/* mp_modal.c: Plaits modal engine, its resonator: a bank of band-pass modes (the double-sampled SVF of mp_filter.h)
 * at the partials of a stiff bar / plate (ratio_n = n sqrt(1 + B n^2), B the stiffness), struck at every note
 * start by a short burst of noise (gain set for a peak near 0.5 at every damping). Plaits has up to 24 modes; here 5. The modes' decay is their Q.
 * OSC1 COLOR = stiffness (harmonic .. bell), OSC2 COLOR = damping, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * OSC2 COLOR follows its COLOR MOD source (s1_color); the stiffness is a knob table (the square roots, on the PC). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_filter.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }
S1_PARAM(ratio, S1_OSC1_COLOR, 5) {
  float b = 0.0002f + 0.08f * (knob / 255.0f) * (knob / 255.0f);
  for (int n = 1; n <= 5; n++)
    out[n - 1] = n * sqrtf(1.0f + b * n * n);
}

static const float kAmp[5] = { 1.0f, 0.7f, 0.5f, 0.35f, 0.25f };
static float w, burst, last_trig;
static struct mp_svf m[5];

S1_DSP_A {
  float trig = s1_trig(), dt = mp_dt();
  if (trig != last_trig)
    burst = 1.0f;
  last_trig = trig;
  float x = mp_noise(&w) * burst;
  burst = burst * 0.998f;                               /* ~5 ms burst */
  float q = 0.002f + 0.05f * s1_color(1);
  float y = 0.0f;
  for (int i = 0; i < 5; i++)
    y += kAmp[i] * mp_svf_process(&m[i], x * q * 30.0f, 6.2832f * mp_step(dt, s1_param(ratio, i)), q).bp;
  return mp_soft(y) * s1_param(level, 0);

}
