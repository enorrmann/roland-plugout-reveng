/* mp_fm_drum.c: Peaks FM drum: a sine carrier phase-modulated by a sine modulator at a ratio, both with a falling
 * pitch envelope and the modulation index decaying from the note start; noise mixed in.
 * OSC1 COLOR = modulation index, OSC2 COLOR = modulator ratio 0.5 .. 4, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float pc, pm, env, last_trig;

S1_DSP_A {
  float trig = s1_trig(), dt = mp_dt();
  if (trig != last_trig) {
    env = 1.0f;
    pc = 0.0f;
    pm = 0.0f;
  }
  last_trig = trig;
  float e = env;
  env = e * (1.0f - 1.0f / (0.04f * MP_FS));             /* 40 ms */
  float d = mp_step(dt, 1.0f + 2.0f * e * e);
  float m = mp_sin(mp_advance(&pm, mp_step(d, s1_exp2(3.0f * s1_color(1) - 1.0f))));
  float p = mp_advance(&pc, d);
  return mp_sin(mp_frac(p + m * e * s1_color(0))) * s1_param(level, 0);
}
