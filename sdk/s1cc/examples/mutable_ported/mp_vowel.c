/* mp_vowel.c: Braids VOWEL / Plaits speech in their simplest form: a band-limited pulse (the glottis) through three
 * formant band passes in parallel (DSP B), the vowel morphing a -> e -> i -> o -> u on a knob (formant table after
 * Braids' vowel data, Peterson & Barney averages). The AMP envelope is the VCA.
 * OSC1 COLOR = vowel, OSC2 COLOR = formant shift (-1 .. +1 octave), OSC1 MIX = level, RESONANCE = formant width. */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_filter.h"

static const float kF[5][3] = {               /* Hz: a, e, i, o, u */
  { 730.0f, 1090.0f, 2440.0f }, { 530.0f, 1840.0f, 2480.0f }, { 270.0f, 2290.0f, 3010.0f },
  { 570.0f, 840.0f, 2410.0f }, { 300.0f, 870.0f, 2240.0f },
};
static const float kA[3] = { 1.0f, 0.5f, 0.25f };            /* formant levels */
static float formant(int knob, int k) {
  float x = knob / 255.0f * 4.0f;
  int i = (int)x;
  if (i > 3) i = 3;
  float t = x - i;
  return kF[i][k] + (kF[i + 1][k] - kF[i][k]) * t;
}
/* w of each formant for the double-sampled SVF (2 pi f / (2 fs)) */
S1_PARAM(vowel, S1_OSC1_COLOR, 3) { for (int k = 0; k < 3; k++) out[k] = 6.2831853f * formant(knob, k) / (2.0f * MP_FS); }
S1_PARAM(shift, S1_OSC2_COLOR, 1) { out[0] = powf(2.0f, (knob - 128) / 128.0f); }
S1_PARAM(width, S1_FILTER_RESONANCE, 1) { out[0] = 0.5f - 0.45f * knob / 255.0f; }        /* SVF damping */
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph;
static struct mp_svf f1, f2, f3;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float p = mp_advance(&ph, dt);
  return mp_pulse_bl(p, 0.8f, dt, rdt) * s1_param(level, 0);
}

S1_DSP_B {
  float sh = s1_param(shift, 0), q = s1_param(width, 0);
  float y = kA[0] * mp_svf_process(&f1, x, s1_param(vowel, 0) * sh, q).bp;
  y += kA[1] * mp_svf_process(&f2, x, s1_param(vowel, 1) * sh, q).bp;
  y += kA[2] * mp_svf_process(&f3, x, s1_param(vowel, 2) * sh, q).bp;
  return y * s1_env(0);
}
