/* mp_vowel.c: Braids VOWEL / Plaits speech in their simplest form: a band-limited pulse (the glottis) through three
 * formant band passes in parallel (DSP B), the vowel morphing a -> e -> i -> o -> u on a knob (formant table after
 * Braids' vowel data, Peterson & Barney averages). The AMP envelope is the VCA.
 * OSC1 COLOR = vowel, OSC2 COLOR = formant shift (-1 .. +1 octave), both with their COLOR MOD source (s1_color, on
 * DSP A: the formants go to DSP B by s1_export), OSC1 MIX = level, RESONANCE = formant width. */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_filter.h"

static const float kF[5][3] = {               /* Hz: a, e, i, o, u */
  { 730.0f, 1090.0f, 2440.0f }, { 530.0f, 1840.0f, 2480.0f }, { 270.0f, 2290.0f, 3010.0f },
  { 570.0f, 840.0f, 2410.0f }, { 300.0f, 870.0f, 2240.0f },
};
static const float kA[3] = { 1.0f, 0.5f, 0.25f };            /* formant levels */
S1_PARAM(width, S1_FILTER_RESONANCE, 1) { out[0] = 0.5f - 0.45f * knob / 255.0f; }        /* SVF damping */
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph;
static struct mp_svf f1, f2, f3;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float p = mp_advance(&ph, dt);
  /* the vowel a -> e -> i -> o -> u along the color: each formant linear between the table's points (h_j: 0 .. 1 in
     the j-th quarter); x the shift 2^(2 c2 - 1) (-1 .. +1 octave); w = 2 pi f / (2 fs), to DSP B by s1_export */
  float v = 4.0f * s1_color(0), sh = s1_exp2(2.0f * s1_color(1) - 1.0f) * (6.2831853f / (2.0f * MP_FS));
  float h1 = fminf(fmaxf(v, 0.0f), 1.0f), h2 = fminf(fmaxf(v - 1.0f, 0.0f), 1.0f);
  float h3 = fminf(fmaxf(v - 2.0f, 0.0f), 1.0f), h4 = fminf(fmaxf(v - 3.0f, 0.0f), 1.0f);
  for (int k = 0; k < 3; k++)
    s1_export(k, (kF[0][k] + (kF[1][k] - kF[0][k]) * h1 + (kF[2][k] - kF[1][k]) * h2 + (kF[3][k] - kF[2][k]) * h3 +
                  (kF[4][k] - kF[3][k]) * h4) * sh);
  return mp_pulse_bl(p, 0.8f, dt, rdt) * s1_param(level, 0);
}

S1_DSP_B {
  float q = s1_param(width, 0);
  float y = kA[0] * mp_svf_process(&f1, x, s1_import(0), q).bp;
  y += kA[1] * mp_svf_process(&f2, x, s1_import(1), q).bp;
  y += kA[2] * mp_svf_process(&f3, x, s1_import(2), q).bp;
  return y * s1_env(0) * s1_lfo_amp();          /* AMP envelope x the host LFO (LFO TO AMP) */
}
