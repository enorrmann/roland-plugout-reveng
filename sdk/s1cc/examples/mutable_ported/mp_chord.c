/* mp_chord.c: Plaits' chord engine in one voice: four band-limited saws on a chord (Plaits' chord table: octave,
 * fifth, sus4, minor, minor 7, minor 9, major, major 7, ...), a paraphonic pad from one key.
 * OSC1 COLOR = chord, OSC1 MIX = level. DSP B: mp_dsp_b.h.
 * The COLORs follow their COLOR MOD source (s1_color). */
#include <math.h>
#include "include/mp_dsp_a.h"
#include "include/mp_dsp_b.h"

static const float kChords[11][3] = {            /* semitones above the root (plaits/dsp/chords/chord_bank.cc) */
  { 12.0f, 19.0f, 24.0f }, { 7.0f, 12.0f, 19.0f }, { 5.0f, 7.0f, 12.0f }, { 3.0f, 7.0f, 12.0f },
  { 3.0f, 7.0f, 10.0f }, { 3.0f, 10.0f, 14.0f }, { 4.0f, 7.0f, 12.0f }, { 4.0f, 7.0f, 11.0f },
  { 4.0f, 7.0f, 10.0f }, { 4.0f, 11.0f, 14.0f }, { 5.0f, 7.0f, 10.0f },
};
/* ratio and 1/ratio of the three upper notes per chord (1/ratio: 1/dt of that saw without a division), chosen by
   the color: one const array per note (small arrays: the DSP picks the word by comparisons) */
static const float kR0[11] = { 2.000000f, 1.498307f, 1.334840f, 1.189207f, 1.189207f, 1.189207f, 1.259921f, 1.259921f, 1.259921f, 1.259921f, 1.334840f };
static const float kR1[11] = { 2.996614f, 2.000000f, 1.498307f, 1.498307f, 1.498307f, 1.781797f, 1.498307f, 1.498307f, 1.498307f, 1.887749f, 1.498307f };
static const float kR2[11] = { 4.000000f, 2.996614f, 2.000000f, 2.000000f, 1.781797f, 2.244924f, 2.000000f, 1.887749f, 1.781797f, 2.244924f, 1.781797f };
static const float kI0[11] = { 0.500000f, 0.667420f, 0.749154f, 0.840896f, 0.840896f, 0.840896f, 0.793701f, 0.793701f, 0.793701f, 0.793701f, 0.749154f };
static const float kI1[11] = { 0.333710f, 0.500000f, 0.667420f, 0.667420f, 0.667420f, 0.561231f, 0.667420f, 0.667420f, 0.667420f, 0.529732f, 0.667420f };
static const float kI2[11] = { 0.250000f, 0.333710f, 0.500000f, 0.500000f, 0.561231f, 0.445449f, 0.500000f, 0.529732f, 0.561231f, 0.445449f, 0.561231f };

S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.75f * knob / 255.0f) / 2.5f; }

static float ph[4];

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float y = mp_saw_bl(mp_advance(&ph[0], dt), dt, rdt);
  int c = (int)(s1_color(0) * 10.999f);
  const float r[3] = { kR0[c], kR1[c], kR2[c] }, ri[3] = { kI0[c], kI1[c], kI2[c] };
  for (int i = 0; i < 3; i++) {
    float d = mp_step(dt, r[i]);
    y += mp_saw_bl(mp_advance(&ph[i + 1], d), d, rdt * ri[i]);
  }
  return y * s1_param(level, 0);
}
