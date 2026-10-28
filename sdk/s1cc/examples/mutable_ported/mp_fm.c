/* mp_fm.c: two-operator FM (Braids FM / Plaits FM engine): a sine carrier phase-modulated by a sine at a ratio.
 * OSC1 COLOR = ratio (Plaits' quantised ratios), OSC2 COLOR = index 0 .. 2.5, OSC1 MIX = level. DSP B: mp_voice.h. */
#include "mp_voice.h"

static const float kRatios[] = { 0.5f, 1.0f, 1.41421f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f, 5.0f, 6.0f, 7.0f };
S1_PARAM(ratio, S1_OSC1_COLOR, 1) { out[0] = kRatios[knob * 12 / 256]; }
S1_PARAM(index, S1_OSC2_COLOR, 1) { float t = knob / 255.0f; out[0] = 2.5f * t * t; }
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float pc, pm;

S1_DSP_A {
  float dt = mp_dt();
  float c = mp_advance(&pc, dt);
  float m = mp_advance(&pm, mp_step(dt, s1_param(ratio, 0)));
  return mp_sin(c + s1_param(index, 0) * mp_sin(m)) * s1_param(level, 0);
}
