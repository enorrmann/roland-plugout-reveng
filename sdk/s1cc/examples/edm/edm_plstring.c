/* edm_plstring.c: Plaits' string (plaits/dsp/physical_modelling/string.cc) on SYSTEM-S1CC, 6 voices: the excitation
 * on the slave, the string in its own line of the delay memory on master B (EDM_STR_PL, docs/MUTABLE_MEMORY.md
 * §3.3.5): damping = Plaits' Q 0.5 low pass (two one-poles) at 24 .. 84 semitones above the note (FILTER CUTOFF),
 * dispersion = three first-order allpasses (OSC2 COLOR), decay held / released AMP DECAY / AMP RELEASE; the output
 * is the string alone. Excitation: a 5 ms burst of noise per note through a one-pole whose cutoff is OSC1 COLOR (the
 * excitation brightness, Plaits' TIMBRE in its string engine); OSC1 MIX = level. */
#include "../mutable_ported/include/mp_dsp_a.h"

S1_BUILD("NAME=MP-PLSTRING EDM_STR=1 EDM_STR_ONE=1 EDM_STR_PL=1");
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }
S1_PARAM(bright, S1_OSC1_COLOR, 1) {                /* the burst's one-pole: 0.03 (dark) .. 1 (white noise) */
  float k = knob / 255.0f;
  out[0] = 0.03f + 0.97f * k * k;
}

static float env, w, lp, last_trig;

S1_DSP_A {
  float trig = s1_trig();
  if (trig != last_trig)
    env = 1.0f;
  last_trig = trig;
  float e = env;
  env = e * 0.9979f;                                /* 5 ms (1 / (5 ms x 96 kHz)) */
  float x = mp_noise(&w) * e;
  float y = lp + s1_param(bright, 0) * (x - lp);
  lp = y;
  return y * s1_param(level, 0);
}

// todo mover la linea de delay a C tambien