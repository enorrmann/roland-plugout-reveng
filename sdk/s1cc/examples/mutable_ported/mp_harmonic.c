/* mp_harmonic.c: additive oscillator (Plaits' HarmonicOscillator / additive engine): 8 harmonics by the Chebyshev
 * recursion s(k+1) = 2 cos s(k) - s(k-1), each faded out before it would alias.
 * OSC1 COLOR = brightness (harmonic k at b^(k-1)), OSC2 COLOR = even / odd balance, OSC1 MIX = level.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

#define NH 8
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 1.5f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt();
  float p = mp_advance(&ph, dt);
  float s1 = mp_sin(p), c2 = 2.0f * mp_cos(p);
  /* harmonic k has b^(k-1) / sum, b = 0.15 + 0.8 c (brightness); 1 / sum of the 8 fitted in c (degree 3, 0.3 %);
     even harmonics x 2 c2 */
  float c = s1_color(0), b = 0.15f + 0.8f * c, even = 2.0f * s1_color(1);
  float amp = 0.848173f + c * (-0.761414f + c * (-0.175419f + c * 0.236805f));
  float sk1 = 0.0f, sk = s1, y = 0.0f;
  for (int k = 1; k <= NH; k++) {
    /* harmonic k fades out between 0.4 and 0.48 of the sample rate */
    float g = s1_clamp((0.48f - k * dt) * 12.5f) * 0.5f + 0.5f;
    float a = amp * ((k & 1) ? 1.0f : even);
    amp *= b;
    y += a * g * sk;
    float sn = c2 * sk - sk1;
    sk1 = sk;
    sk = sn;
  }
  return y * s1_param(level, 0);
}
