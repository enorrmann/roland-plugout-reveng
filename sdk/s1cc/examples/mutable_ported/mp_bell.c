/* mp_bell.c: Braids STRUCK_BELL, a few partials: four sines at a bell's inharmonic ratios, each with its own decay
 * from the note start (the high ones die first). The AMP envelope is the overall level (long DECAY / RELEASE).
 * OSC1 COLOR = decay time, OSC2 COLOR = inharmonicity, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

static const float kRatio[4] = { 1.0f, 2.756f, 5.404f, 8.933f };     /* a bell's (Rayleigh) partials */
static const float kAmp[4] = { 1.0f, 0.6f, 0.4f, 0.25f };
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.75f * knob / 255.0f) / 2.0f; }

static float ph[4], amp[4], last_trig;

S1_DSP_A {
  float trig = s1_trig(), dt = mp_dt(), y = 0.0f;
  int strike = trig != last_trig;
  last_trig = trig;
  /* decay of the fundamental t = 0.2 s x 2^(4 c) (0.2 .. 3.2 s), partial i (1 + i) times faster: the per-sample
     factor 1 - (1 + i) / (fs t) (= e^-x to first order); inharmonicity 0.5 .. 1.5 x the stretch of the partials */
  float rt = s1_exp2(-4.0f * s1_color(0)) * (1.0f / (0.2f * MP_FS)), s = 0.5f + s1_color(1);
  for (int i = 0; i < 4; i++) {
    float a = strike ? kAmp[i] : amp[i];
    amp[i] = a - a * ((1.0f + i) * rt);
    y += a * mp_sin(mp_advance(&ph[i], mp_step(dt, 1.0f + (kRatio[i] - 1.0f) * s)));
  }
  return y * s1_param(level, 0);
}
