/* mp_lpg.c: Plaits' low pass gate (mp_filter.h): the AMP envelope opens a VCA and a low pass together, the Buchla
 * "bongo" gesture; on a wavefolded triangle (Buchla's own pairing). Set AMP ATTACK 0, DECAY short, SUSTAIN 0.
 * DSP B: MP_LPG (FILTER CUTOFF = the gate's top cutoff, RESONANCE). OSC1 COLOR = fold, OSC1 MIX = level.
 * The COLORs follow their COLOR MOD source (s1_color). */
#define MP_FILTER MP_LPG
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = 0.25f + 0.5f * knob / 255.0f; }

static float ph;

S1_DSP_A {
  float p = mp_advance(&ph, mp_dt());
  return mp_fold(s1_tri(p + p) * (1.0f + 3.0f * s1_color(0))) * s1_param(level, 0);
}
