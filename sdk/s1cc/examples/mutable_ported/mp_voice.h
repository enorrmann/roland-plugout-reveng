/* mp_voice.h: the DSP B every mutable_ported oscillator kernel shares, and its panel. An oscillator kernel defines
 * S1_DSP_A (and its own S1_PARAM knobs on OSC1 / OSC2 COLOR, ...) and includes this file:
 *   x (from DSP A) -> filter (MP_FILTER, default the SVF low pass) -> x the AMP envelope (s1_env(0)) -> out
 * The cutoff moves with the FILTER envelope (s1_env(1)) times FILTER ENV AMOUNT and with the key (KEY TRACK).
 * Both envelopes are SYSTEM-S1CC's ADSRs in assembly on DSP B (docs/examples_system_s1cc_README.md §3.1).
 *   FILTER CUTOFF 20 Hz .. 17 kHz, RESONANCE, ENV AMOUNT -8 .. +8 octaves (128 = 0), KEY TRACK 0 .. 1 (pitch).
 * MP_FILTER: MP_SVF_LP (default), MP_SVF_BP, MP_SVF_HP, MP_LADDER, MP_LPG (the AMP envelope drives the gate),
 * MP_NONE (VCA only). */
#ifndef MP_VOICE_H_
#define MP_VOICE_H_
#include <math.h>
#include "mp_dsp.h"
#include "mp_filter.h"

#define MP_SVF_LP 0
#define MP_SVF_BP 1
#define MP_SVF_HP 2
#define MP_LADDER 3
#define MP_LPG 4
#define MP_NONE 5
#ifndef MP_FILTER
#define MP_FILTER MP_SVF_LP
#endif

/* PC tables: w0 = 2 pi fc / (2 fs) for the double-sampled SVF (and the same w for the one-poles of the ladder) */
#if MP_FILTER != MP_NONE
S1_PARAM(cutoff, S1_FILTER_CUTOFF, 1) {
  float fc = 20.0f * powf(1000.0f, knob / 255.0f);
  out[0] = 6.2831853f * fc / (2.0f * MP_FS) * (MP_FILTER == MP_LADDER ? 2.0f : 1.0f);
}
S1_PARAM(reso, S1_FILTER_RESONANCE, 1) {
  float r = knob / 255.0f;
  out[0] = MP_FILTER == MP_LADDER ? 3.9f * r : 1.4f - 1.38f * r;   /* ladder: feedback k; SVF: damping q */
}
S1_PARAM(envamt, S1_FILTER_ENV_AMOUNT, 1) { out[0] = (knob - 128) / 128.0f * 8.0f; }
S1_PARAM(keytrack, S1_FILTER_KEY_TRACK, 1) { out[0] = knob / 255.0f; }

static struct mp_svf mp_b_svf;
static struct mp_ladder mp_b_ladder;
#endif

S1_DSP_B {
#if MP_FILTER == MP_NONE
  return x * s1_env(0);
#else
  /* cutoff: the knob, + envelope x amount octaves, + key track x (pitch - A4) octaves, at most w = 1 (~15 kHz) */
  float oct = s1_env(1) * s1_param(envamt, 0) + s1_param(keytrack, 0) * (s1_pitch() - 4.75f);
  float w = fminf(s1_param(cutoff, 0) * s1_exp2(s1_range(oct, -10.0f, 10.0f)), 1.0f);
#if MP_FILTER == MP_LADDER
  return mp_ladder_process(&mp_b_ladder, x, w, s1_param(reso, 0)) * s1_env(0);
#elif MP_FILTER == MP_LPG
  return mp_lpg_process((struct mp_lpg *)&mp_b_svf, x, s1_env(0), w, s1_param(reso, 0));
#else
  struct mp_svf_out o = mp_svf_process(&mp_b_svf, x, w, s1_param(reso, 0));
  float y = MP_FILTER == MP_SVF_BP ? o.bp : MP_FILTER == MP_SVF_HP ? o.hp : o.lp;
  return y * s1_env(0);
#endif
#endif
}

#endif
