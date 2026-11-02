/* mp_saw.c: band-limited sawtooth (polyBLEP), the basic analog oscillator of Braids / Plaits, for SYSTEM-S1CC.
 * DSP A: the saw; DSP B: mp_dsp_b.h (SVF low pass + envelopes). OSC1 MIX = level. */

//#define MP_FILTER MP_NONE 
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"


S1_PARAM(level1, S1_OSC1_MIX, 1) { out[0] = 0.5f * knob / 255.0f; }    /* two oscillators: half the level of one */
S1_PARAM(level2, S1_OSC2_MIX, 1) { out[0] = 0.5f * knob / 255.0f; }

/* RANGE selectors 0..5 = 64' .. 2' = -3 .. +2 octaves (anything else, as the row 128 before the panel arrives: 8') */
static float range_oct(int knob) { return knob <= 5 ? knob - 3.0f : 0.0f; }
/* DCO2's ratio to the host's dt and its inverse (for 1/dt, no division on the DSP), one table per control */
S1_PARAM(range1, S1_OSC1_RANGE, 2) { out[0] = powf(2.0f, -range_oct(knob)); out[1] = 1.0f / out[0]; }
S1_PARAM(range2, S1_OSC2_RANGE, 2) { out[0] = powf(2.0f, range_oct(knob)); out[1] = 1.0f / out[0]; }
S1_PARAM(fine2, S1_OSC2_FINE, 2) { out[0] = powf(2.0f, (knob - 128) / 128.0f / 12.0f); out[1] = 1.0f / out[0]; }
// ============================================================
// OSCILLATOR PHASE
// ============================================================

// Independent phase accumulators for both oscillators.
static float ph1, ph2;

// ============================================================
// DSP AUDIO PROCESSING
// ============================================================

S1_DSP_A {
  // solo suena lo del b que incluye noise y un sub
}