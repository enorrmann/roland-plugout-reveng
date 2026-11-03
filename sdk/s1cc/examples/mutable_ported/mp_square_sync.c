/* mp_square_sync.c: Braids SQUARE_SYNC: a square hard-synced to the note, its frequency up to 8 times it, mixed
 * with the master square. The slave's edges inside the period are naive (above the master, frac(p r) jumps at
 * every slave cycle; polyBLEP only at the master's reset), as Braids' sync table makes them.
 * OSC1 COLOR = slave ratio 1 .. 8, OSC2 COLOR = master / slave balance, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f * 0.75f; }

static float ph;

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float p = mp_advance(&ph, dt);
  float r = s1_exp2(3.0f * s1_color(0));
  float rdts = rdt * s1_exp2(-3.0f * s1_color(0));
  float s = mp_frac(p * r);
  float slave = mp_pulse_bl(s, 0.5f, fminf(dt * r, 0.5f), rdts);
  float master = mp_pulse_bl(p, 0.5f, dt, rdt);
  return (master + (slave - master) * s1_color(1)) * s1_param(level, 0);
}
