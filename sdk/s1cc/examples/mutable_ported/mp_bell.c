/* mp_bell.c: Braids STRUCK_BELL, a few partials: four sines at a bell's inharmonic ratios, each with its own decay
 * from the note start (the high ones die first). The AMP envelope is the overall level (long DECAY / RELEASE).
 * OSC1 COLOR = decay time, OSC2 COLOR = inharmonicity, OSC1 MIX = level. DSP B: mp_voice.h. */
#include <math.h>
#include "mp_voice.h"

static const float kRatio[4] = { 1.0f, 2.756f, 5.404f, 8.933f };     /* a bell's (Rayleigh) partials */
static const float kAmp[4] = { 1.0f, 0.6f, 0.4f, 0.25f };
S1_PARAM(decay, S1_OSC1_COLOR, 4) {
  float t = 0.2f + 3.0f * knob / 255.0f;                            /* seconds of the fundamental */
  for (int i = 0; i < 4; i++) out[i] = expf(-1.0f / (MP_FS * t / (1.0f + i)));
}
S1_PARAM(inharm, S1_OSC2_COLOR, 4) {
  float s = 0.5f + knob / 255.0f;                                   /* 0.5 .. 1.5 x the stretch */
  for (int i = 0; i < 4; i++) out[i] = 1.0f + (kRatio[i] - 1.0f) * s;
}
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.25f + 0.5f * knob / 255.0f) / 2.0f; }

static float ph[4], amp[4], last_trig;

S1_DSP_A {
  float trig = s1_trig(), dt = mp_dt(), y = 0.0f;
  int strike = trig != last_trig;
  last_trig = trig;
  for (int i = 0; i < 4; i++) {
    float a = strike ? kAmp[i] : amp[i];
    amp[i] = a * s1_param(decay, i);
    y += a * mp_sin(mp_advance(&ph[i], mp_step(dt, s1_param(inharm, i))));
  }
  return y * s1_param(level, 0);
}
