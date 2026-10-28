/* mp_kick.c: an analog bass drum (Braids KICK / Plaits analog bass drum, their core): a sine whose pitch falls from
 * a few times the note to the note after each note start, plus a click. The level decay is the AMP envelope
 * (DECAY / RELEASE short, SUSTAIN 0).
 * OSC1 COLOR = pitch sweep depth, OSC2 COLOR = sweep time, OSC1 MIX = level. DSP B: mp_voice.h. */
#include <math.h>
#include "mp_voice.h"

S1_PARAM(depth, S1_OSC1_COLOR, 1) { out[0] = 6.0f * knob / 255.0f; }                       /* extra ratio at the start */
S1_PARAM(decay, S1_OSC2_COLOR, 1) { out[0] = expf(-1.0f / (MP_FS * (0.005f + 0.1f * knob / 255.0f))); }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph, env, last_trig;

S1_DSP_A {
  float trig = s1_trig();
  if (trig != last_trig) {                      /* a note start: the sweep restarts, the phase too (a clean attack) */
    env = 1.0f;
    ph = 0.0f;
  }
  last_trig = trig;
  float e = env;
  env = e * s1_param(decay, 0);
  float p = mp_advance(&ph, mp_step(mp_dt(), 1.0f + s1_param(depth, 0) * e * e));
  return mp_sin(p) * s1_param(level, 0);
}
