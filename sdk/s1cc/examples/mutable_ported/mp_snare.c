/* mp_snare.c: a snare (Braids SNARE / Plaits snare drum, their core): a short falling sine (the shell) and noise (the
 * wires), the balance on a knob; the level decay is the AMP envelope. DSP B: the SVF high pass takes off the boom
 * (MP_SVF_HP: set FILTER CUTOFF low).
 * OSC1 COLOR = noise / tone balance, OSC1 MIX = level. */
#define MP_FILTER MP_SVF_HP
#include <math.h>
#include "mp_voice.h"

S1_PARAM(snappy, S1_OSC1_COLOR, 2) { float t = knob / 255.0f; out[0] = 1.0f - t; out[1] = t; }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.25f + 0.5f * knob / 255.0f) * 0.6f; }   /* tone + noise peak ~1.2 */

static float ph, env, w, last_trig;

S1_DSP_A {
  float trig = s1_trig();
  if (trig != last_trig)
    env = 1.0f;
  last_trig = trig;
  float e = env;
  env = e * 0.9995f;                            /* the shell's pitch falls over ~20 ms */
  float p = mp_advance(&ph, mp_step(mp_dt(), 1.0f + 1.5f * e));
  float tone = mp_sin(p) * e;
  return (tone * s1_param(snappy, 0) + mp_noise(&w) * s1_param(snappy, 1)) * s1_param(level, 0);
}
