/* mp_digital_filter.c: Braids DIGITAL_FILTER_LP / PK / BP / HP: not a filter but its sound, synthesized: a sine at
 * the "resonance" frequency, restarted at every period of the note (at the phase of the filter type), under a
 * window that falls over the period (saw) or rises and falls (triangle). LP / PK: window x (1 + sine) - 1, BP / HP:
 * window x sine. Braids' second component (a square carrier through an integrator) is left out.
 * MP_DF_TYPE 0 LP (default), 1 PK, 2 BP, 3 HP.
 * OSC1 COLOR = resonance frequency (1 .. 16 x the note), OSC2 COLOR = window saw -> triangle, OSC1 MIX = level.
 * DSP B: mp_dsp_b.h. The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

#ifndef MP_DF_TYPE
#define MP_DF_TYPE 0
#endif
static const float kReset[4] = { 0.0f, 0.5f, 0.25f, 0.75f };     /* kPhaseReset / 2^32 */

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.75f * knob / 255.0f; }

static float ph, pm;

S1_DSP_A {
  float dt = mp_dt();
  float p = mp_advance(&ph, dt);
  float r = s1_exp2(4.0f * s1_color(0));
  /* the modulator restarts with the note's period: its phase is frac(p r) + reset, no state of its own */
  float m = mp_frac(p * r + kReset[MP_DF_TYPE]);
  float carrier = mp_sin(m);
  float saw = 1.0f - p, tri = 1.0f - fabsf(p + p - 1.0f);
  float w = saw + (tri - saw) * s1_color(1);
#if MP_DF_TYPE < 2
  float y = w * (carrier + 1.0f) - 1.0f;
#else
  float y = w * carrier;
#endif
  (void)pm;
  return y * s1_param(level, 0);
}
