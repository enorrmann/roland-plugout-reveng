/* mp_twin_peaks.c: Braids TWIN_PEAKS_NOISE: noise through two resonant band-passes, one at the note and one at an
 * interval from it, then an overdrive. Braids uses two-pole resonators (y = s x + c y1 - r^2 y2); written that way
 * the DSP's fixed-point values put too much rounding noise into the recursion at r ~ 0.999 (+17..+41 dB in the high
 * bands), so here they are the double-sampled Chamberlin SVF of mp_filter.h (band-pass output, same peaks).
 * OSC1 COLOR = resonance, OSC2 COLOR = second peak (-2 .. +2 octaves), OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_filter.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float w;
static struct mp_svf fa, fb;

S1_DSP_A {
  float dt = mp_dt();
  float q = 0.12f - 0.1f * s1_color(0);                 /* damping: Braids' q 65240 .. 65495 */
  float x = mp_noise(&w) * q * 16.0f;                    /* band-pass peak gain ~ 1 / q; narrow: x 8 more */
  float ya = mp_svf_process(&fa, x, 6.2832f * dt, q).bp;
  float yb = mp_svf_process(&fb, x, 6.2832f * mp_step(dt, s1_exp2(4.0f * s1_color(1) - 2.0f)), q).bp;
  return mp_soft(ya + yb) * 1.5f * s1_param(level, 0);
}
