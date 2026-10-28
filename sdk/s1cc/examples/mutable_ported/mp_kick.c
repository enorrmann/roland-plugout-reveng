/* mp_kick.c: an analog bass drum (Braids KICK / Plaits analog bass drum, their core): a sine whose pitch falls from
 * a few times the note to the note after each note start, plus a click. The level decay is the AMP envelope
 * (DECAY / RELEASE short, SUSTAIN 0).
 * OSC1 COLOR = pitch sweep depth, OSC2 COLOR = sweep time, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph, env, last_trig;

S1_DSP_A {
  float trig = s1_trig();
  if (trig != last_trig) {                      /* a note start: the sweep restarts, the phase too (a clean attack) */
    env = 1.0f;
    ph = 0.0f;
  }
  last_trig = trig;
  float e = env;
  env = e - e * (s1_exp2(-4.39f * s1_color(1)) * (1.0f / (0.005f * MP_FS)));   /* T = 5 ms x 2^(4.39 c): 5 .. 105 ms */
  float p = mp_advance(&ph, mp_step(mp_dt(), 1.0f + (6.0f * s1_color(0)) * e * e));
  return mp_sin(p) * s1_param(level, 0);
}
