/* edm_string.c: the excitation of a Karplus-Strong string per voice (docs/MUTABLE_MEMORY.md §3.3.5): at every note
 * a short burst of noise, nothing else. Built with EDM_STR=1 (examples/system_s1cc/build.sh), the slave sends voice
 * v's burst to master B on IOMD 6006 + v and master B rings it in voice v's own line in the delay memory at the
 * note's period (microcode/edm_str.s). OSC1 COLOR = burst length (1 .. 16 ms), OSC1 MIX = level; on the master:
 * AMP DECAY / AMP RELEASE = how long the string rings while held / after, FILTER CUTOFF = brightness. */
#include "../mutable_ported/include/mp_dsp_a.h"

S1_BUILD("NAME=MP-EDMSTR EDM_STR=1");
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }
S1_PARAM(decay, S1_OSC1_COLOR, 1) {                 /* per-sample factor of a 1 .. 16 ms burst */
  float ms = 1.0f + 15.0f * knob / 255.0f;
  out[0] = 1.0f - 1.0f / (ms * 96.0f);
}

static float env, w, last_trig;

S1_DSP_A {
  float trig = s1_trig();
  if (trig != last_trig)
    env = 1.0f;
  last_trig = trig;
  float e = env;
  env = e * s1_param(decay, 0);
  return mp_noise(&w) * e * s1_param(level, 0);
}
