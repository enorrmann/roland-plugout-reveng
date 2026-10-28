/* mp_harmonic.c: additive oscillator (Plaits' HarmonicOscillator / additive engine): 8 harmonics by the Chebyshev
 * recursion s(k+1) = 2 cos s(k) - s(k-1), each faded out before it would alias.
 * OSC1 COLOR = brightness (harmonic k at b^(k-1)), OSC2 COLOR = even / odd balance, OSC1 MIX = level. */
#include <math.h>
#include "mp_voice.h"

#define NH 8
S1_PARAM(amp, S1_OSC1_COLOR, NH) {
  float b = 0.15f + 0.8f * knob / 255.0f, a = 1.0f, sum = 0.0f;
  for (int i = 0; i < NH; i++) { out[i] = a; sum += a; a *= b; }
  for (int i = 0; i < NH; i++) out[i] /= sum;
}
S1_PARAM(even, S1_OSC2_COLOR, 1) { out[0] = 2.0f * knob / 255.0f; }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.5f + 1.0f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt();
  float p = mp_advance(&ph, dt);
  float s1 = mp_sin(p), c2 = 2.0f * mp_cos(p);
  float sk1 = 0.0f, sk = s1, y = 0.0f;
  for (int k = 1; k <= NH; k++) {
    /* harmonic k fades out between 0.4 and 0.48 of the sample rate */
    float g = s1_clamp((0.48f - k * dt) * 12.5f) * 0.5f + 0.5f;
    float a = s1_param(amp, k - 1) * ((k & 1) ? 1.0f : s1_param(even, 0));
    y += a * g * sk;
    float sn = c2 * sk - sk1;
    sk1 = sk;
    sk = sn;
  }
  return y * s1_param(level, 0);
}
