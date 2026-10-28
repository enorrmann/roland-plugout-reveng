/* mp_fm.c: two-operator FM (Braids FM / Plaits FM engine): a sine carrier phase-modulated by a sine at a ratio.
 * OSC1 COLOR = ratio (0.5 .. 7.5 in steps of 0.5), OSC2 COLOR = index 0 .. 2.5, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

/* ratio 0.5 .. 7.5 in steps of 0.5 along the color: a function, not Plaits' table (0.5, 1, 1.41, 1.5, 2, 2.5, 3, 3.5, 4,
   5, 6, 7), so it costs ~4 ops on the DSP with COLOR MOD; only the inharmonic 1.41 is gone */
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float pc, pm;

S1_DSP_A {
  float dt = mp_dt();
  float c = mp_advance(&pc, dt);
  float m = mp_advance(&pm, mp_step(dt, (0.5f + 0.5f * floorf(s1_color(0) * 14.999f))));
  return mp_sin(c + (2.5f * s1_color(1) * s1_color(1)) * mp_sin(m)) * s1_param(level, 0);
}
