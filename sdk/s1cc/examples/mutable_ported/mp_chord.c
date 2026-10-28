/* mp_chord.c: Plaits' chord engine in one voice: four band-limited saws on a chord (Plaits' chord table: octave,
 * fifth, sus4, minor, minor 7, minor 9, major, major 7, ...), a paraphonic pad from one key.
 * OSC1 COLOR = chord, OSC1 MIX = level. DSP B: mp_voice.h. */
#include <math.h>
#include "mp_voice.h"

static const float kChords[11][3] = {            /* semitones above the root (plaits/dsp/chords/chord_bank.cc) */
  { 12.0f, 19.0f, 24.0f }, { 7.0f, 12.0f, 19.0f }, { 5.0f, 7.0f, 12.0f }, { 3.0f, 7.0f, 12.0f },
  { 3.0f, 7.0f, 10.0f }, { 3.0f, 10.0f, 14.0f }, { 4.0f, 7.0f, 12.0f }, { 4.0f, 7.0f, 11.0f },
  { 4.0f, 7.0f, 10.0f }, { 4.0f, 11.0f, 14.0f }, { 5.0f, 7.0f, 10.0f },
};
/* ratio and 1/ratio of the three upper notes (1/ratio: 1/dt of that saw without a division) */
S1_PARAM(chord, S1_OSC1_COLOR, 6) {
  int c = knob * 11 / 256;
  for (int i = 0; i < 3; i++) { out[2 * i] = powf(2.0f, kChords[c][i] / 12.0f); out[2 * i + 1] = 1.0f / out[2 * i]; }
}
S1_PARAM(level, S1_OSC1_MIX, 1) { out[0] = (0.25f + 0.5f * knob / 255.0f) / 2.5f; }

static float ph[4];

S1_DSP_A {
  float dt = mp_dt(), rdt = mp_rdt();
  float y = mp_saw_bl(mp_advance(&ph[0], dt), dt, rdt);
  for (int i = 0; i < 3; i++) {
    float d = mp_step(dt, s1_param(chord, 2 * i));
    y += mp_saw_bl(mp_advance(&ph[i + 1], d), d, rdt * s1_param(chord, 2 * i + 1));
  }
  return y * s1_param(level, 0);
}
