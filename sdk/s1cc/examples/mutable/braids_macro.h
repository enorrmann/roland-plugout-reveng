/* braids_macro.h: Braids' MacroOscillator (braids/macro_oscillator.cc, (c) Emilie Gillet, MIT) as a SYSTEM-S1CC kernel,
 * compiled from the original sources. Each braids_*.cc defines BRAIDS_SHAPE and includes this file:
 *   S1_DSP_A  one MacroOscillator::Render() of one sample per DSP sample, shape fixed at build time
 *   S1_DSP_B  the host's AMP envelope as the VCA (vca.h)
 * Panel: OSC1 COLOR = TIMBRE, OSC2 COLOR = COLOR (Braids parameters 0..32767), AMP A / D / S / R = the envelope.
 * Build: sdk/s1cc/s1cc build sdk/s1cc/examples/mutable/braids_csaw.cc -I ~/git/eurorack --regs 0,1,2,3,4,5,6,7 --check
 * (macro_oscillator.cc, analog_oscillator.cc, digital_oscillator.cc, resources.cc come by themselves) */
#include <stdint.h>
#include "s1dsp.h"
#include "braids/macro_oscillator.h"
#include "vca.h"

/* the notes it is played at: C0..C8 (16 Hz .. 4.2 kHz). What Braids only does above (both edges of a pulse in one
   sample, a pitch clamp) costs nothing then */
S1_NOTE_RANGE(12, 108);

S1_PARAM(timbre, S1_OSC1_COLOR, 1) { out[0] = knob * 32767 / 255; }
S1_PARAM(color, S1_OSC2_COLOR, 1) { out[0] = knob * 32767 / 255; }

static braids::MacroOscillator osc;

#ifndef BRAIDS_BLOCK
#define BRAIDS_BLOCK 1                    /* 2: the shape renders sample pairs (`size -= 2` in Braids) */
#endif

static int16_t block[BRAIDS_BLOCK];       /* the last block; DSP state */
#if BRAIDS_BLOCK > 1
static uint8_t pos;                       /* sample of the block this DSP sample outputs */
#endif

static void render(int16_t pitch, int16_t timbre, int16_t color) {
  static const uint8_t sync[BRAIDS_BLOCK] = {0};
  osc.set_shape(BRAIDS_SHAPE);            /* a store the optimizer sees: Render's fn_table_[shape_] folds */
  osc.set_pitch(pitch);
  osc.set_parameters(timbre, color);
  osc.Render(sync, block, BRAIDS_BLOCK);
}

S1_INIT {
  osc.Init();
  render(60 << 7, 16384, 16384);          /* one block: the shape's first-render Init() is done, never on the DSP */
}

S1_DSP_A {
  int16_t pitch = static_cast<int16_t>(s1_pitch() * 1536.0f + 1536.0f);     /* our octaves -> MIDI note << 7 */
  int16_t timbre = static_cast<int16_t>(s1_param(timbre, 0)), color = static_cast<int16_t>(s1_param(color, 0));
#if BRAIDS_BLOCK > 1
  /* one block every BRAIDS_BLOCK samples, as Braids: the DSP computes the render every sample (no branches) and
     keeps it only when pos wraps, so this costs one render, not BRAIDS_BLOCK */
  if (pos == 0)
    render(pitch, timbre, color);
  int16_t out = block[0];
  for (int i = 1; i < BRAIDS_BLOCK; i++)
    if (pos == i)
      out = block[i];
  pos = pos + 1 == BRAIDS_BLOCK ? 0 : pos + 1;
  return out / 32768.0f;
#else
  render(pitch, timbre, color);
  return block[0] / 32768.0f;
#endif
}
