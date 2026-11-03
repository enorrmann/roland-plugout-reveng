/* mp_struck_drum.c: Braids STRUCK_DRUM: a few partials at a drum membrane's ratios (Braids' kDrumPartials, one
 * octave above the note), each decaying from the note start, plus noise modes: low-passed noise ringing through
 * two of the partials (Braids multiplies them). The overall level is the AMP envelope.
 * OSC1 COLOR = decay (short .. long), OSC2 COLOR = harmonics -> noise, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

/* 2^(kDrumPartials / 1536) x 2 (the partials 0, 1041, 1747, 3072; the second unison one folded into the first) */
static const float kRatio[4] = { 2.0f, 3.198f, 4.398f, 8.0f };
static const float kAmp[4] = { 0.6f, 0.12f, 0.16f, 0.09f };
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph[4], amp[4], lp1, lp2, w, last_trig;

S1_DSP_A {
  float trig = s1_trig(), dt = mp_dt(), y = 0.0f;
  int strike = trig != last_trig;
  last_trig = trig;
  float c = s1_color(1);
  /* decay of the fundamental 60 ms .. 1.5 s; the high partials twice as fast */
  float rt = s1_exp2(-4.6f * s1_color(0)) * (1.0f / (0.06f * MP_FS));
  float pp[4];
  for (int i = 0; i < 4; i++) {
    float a = strike ? kAmp[i] : amp[i];
    amp[i] = a - a * ((1.0f + 0.5f * i) * rt);
    pp[i] = a * mp_sin(mp_advance(&ph[i], mp_step(dt, kRatio[i])));
    y += pp[i];
  }
  /* noise low-passed around the note (two one-poles), ringing through partials 1 and 3 */
  float f = fminf(0.5f, 16.0f * dt);
  lp1 += (mp_noise(&w) - lp1) * f;
  lp2 += (lp1 - lp2) * f;
  float h = fminf(c * 2.5f, 1.0f), n = fmaxf(c + c - 1.0f, 0.0f);
  float noisy = (pp[1] + pp[3]) * lp2 * 24.0f;
  y = pp[0] + (y - pp[0]) * h + noisy * n;
  return y * s1_param(level, 0);
}
