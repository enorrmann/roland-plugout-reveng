/* mp_sync.c: Braids SAW_SYNC: a saw hard-synced to the note (reset at every period of the master), its frequency a
 * ratio above it: the classic sync sweep. polyBLEP at the reset, which is the master's edge.
 * OSC1 COLOR = slave ratio 1 .. 8, OSC1 MIX = level. DSP B: mp_voice.h. */
#include <math.h>
#include "mp_voice.h"

S1_PARAM(ratio, S1_OSC1_COLOR, 1) { out[0] = powf(8.0f, knob / 255.0f); }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float p = mp_advance(&ph, dt);
  float r = s1_param(ratio, 0);
  float s = mp_frac(p * r);                                 /* the slave: frac(p r), reset with the master */
  /* the step at the master's reset: from the slave's last value (frac(r)) down to 0 */
  float last = mp_frac(r);
  /* the slave falls by 2 last at the master's edge: the master's band-limited saw minus its naive one, x last */
  float y = s + s - 1.0f + last * (mp_saw_bl(p, dt, rdt) - (p + p - 1.0f));
  return y * s1_param(level, 0);
}
