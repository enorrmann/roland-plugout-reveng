// Braids SAW_SWARM through a thin wrapper (docs/S1CC.md §3.2): the original class, unchanged, rendering one sample
// per DSP sample. No algorithm code is copied: s1cc compiles braids/digital_oscillator.cc and finds what runs.
//   sdk/s1cc/s1cc build sdk/s1cc/examples/braids_wrapper_saw_swarm.cc ~/git/eurorack/braids/digital_oscillator.cc \
//       ~/git/eurorack/braids/resources.cc -I ~/git/eurorack
// sdk/s1cc/s1cc build sdk/s1cc/examples/braids_wrapper_saw_swarm.cc ~/git/eurorack/braids/digital_oscillator.cc ~/git/eurorack/braids/resources.cc -I ~/git/eurorack
#include <stdint.h>
#include "s1dsp.h"
#define private public                 // RenderSawSwarm is private in Braids: the wrapper calls it directly
#include "braids/digital_oscillator.h"
#undef private

// the knobs as Braids sees them: parameter 0..32767 (TIMBRE = OSC1 COLOR, COLOR = OSC2 COLOR)
S1_PARAM(timbre, S1_OSC1_COLOR, 1) { out[0] = knob * 32767 / 255; }
S1_PARAM(color, S1_OSC2_COLOR, 1) { out[0] = knob * 32767 / 255; }

static braids::DigitalOscillator osc;

S1_INIT {
  osc.Init();
  osc.set_shape(braids::OSC_SHAPE_SAW_SWARM);
  //osc.strike_ = false;                 // no random start phases (the DSP has no stmlib RNG): phases from 0
}

S1_DSP_A {
  static const uint8_t sync = 0;
  int16_t out;
  osc.pitch_ = static_cast<int16_t>(s1_pitch() * 1536.0f + 1536.0f);     // our octaves -> MIDI note << 7
  osc.set_parameters(static_cast<int16_t>(s1_param(timbre, 0)), static_cast<int16_t>(s1_param(color, 0)));
  osc.RenderSawSwarm(&sync, &out, 1);
  return out / 32768.0f;
}
