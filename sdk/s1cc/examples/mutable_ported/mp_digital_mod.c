/* mp_digital_mod.c: Braids DIGITAL_MODULATION: a carrier at the note, QPSK-modulated (I / Q of one of four
 * symbols) by a symbol clock below it. Braids sends a preamble and then a byte stream; here every symbol is drawn
 * at random (the sound is the same: phase jumps of a quarter turn at the symbol rate).
 * OSC1 COLOR = symbol rate (1/32 .. 1/2 of the note), OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph, ps, w, si = 0.7f, sq = 0.7f;

S1_DSP_A {
  float dt = mp_dt();
  float p = mp_advance(&ph, dt);
  float s = ps + dt * s1_exp2(4.0f * s1_color(0) - 5.0f);
  float n = mp_noise(&w);
  if (s >= 1.0f) {                                      /* a new symbol: I and Q signs at random */
    s -= 1.0f;
    si = n > 0.0f ? 0.7f : -0.7f;
    sq = mp_frac(n * 8.0f) > 0.5f ? 0.7f : -0.7f;
  }
  ps = s;
  return (si * mp_sin(p) + sq * mp_cos(p)) * s1_param(level, 0);
}
