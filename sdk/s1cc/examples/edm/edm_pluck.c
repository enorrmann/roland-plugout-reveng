/* edm_pluck.c: the excitation of the first oscillator in the big delay memory (docs/vol6_reference_designs_applications/mutable_memory.md §3.3): a
 * short burst of noise at every note start, nothing else. Built with EDM_KS=1 (examples/system_s1cc/build.sh), the
 * master's region 0 turns the OS's delay (master B, program 0x0, a 6 s line in the delay memory) into a
 * Karplus-Strong string: delay time = the note's period (bank 5 A001 / A002), feedback C004 near 1; the burst
 * goes through the send 2000 / 2002 and rings in the line at the note's pitch. One string (the OS's line):
 * the last note played sets the pitch.
 * OSC1 COLOR = burst length (1 .. 16 ms), OSC1 MIX = level. No DSP B: the burst goes to the mix as it is. */
#include "../mutable_ported/include/mp_dsp_a.h"

S1_BUILD("NAME=MP-EDMKS EDM_KS=1");
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
