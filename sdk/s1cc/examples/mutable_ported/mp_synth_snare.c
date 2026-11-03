/* mp_synth_snare.c: Plaits synthetic snare drum: two sines (the shell's two modes, 1 : 1.47) with a quick pitch drop
 * at the note start, plus noise; DSP B high-passes. The decay is the AMP envelope; the drop and the noise mix the
 * kernel's.
 * OSC1 COLOR = tone / noise, OSC2 COLOR = mode balance, OSC1 MIX = level. DSP B: mp_dsp_b.h (high-pass).
 * The COLORs follow their COLOR MOD source (s1_color). */
#define MP_FILTER MP_SVF_HP
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.75f * knob / 255.0f) * 0.6f; }

static float p1, p2, env, w, last_trig;

S1_DSP_A {
  float trig = s1_trig(), dt = mp_dt();
  if (trig != last_trig) {
    env = 1.0f;
    p1 = 0.0f;
    p2 = 0.0f;
  }
  last_trig = trig;
  float e = env;
  env = e * (1.0f - 1.0f / (0.01f * MP_FS));
  float k = 1.0f + 0.6f * e;
  float a = mp_sin(mp_advance(&p1, mp_step(dt, k)));
  float b = mp_sin(mp_advance(&p2, mp_step(dt, 1.47f * k)));
  float tone = a + (b - a) * s1_color(1) * 0.5f;
  return (tone + (mp_noise(&w) - tone) * s1_color(0)) * s1_param(level, 0);
}
