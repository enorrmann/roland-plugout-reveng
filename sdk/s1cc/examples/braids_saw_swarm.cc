// Braids SAW_SWARM (braids/digital_oscillator.cc, DigitalOscillator::RenderSawSwarm, (c) Emilie Gillet, MIT),
// as an s1cc voice program (docs/S1CC.md). The per-sample loop body is the original code (integers, Q15, uint32
// phases, Interpolate88 of ws_moderate_overdrive); what Render() computes once per block from the knobs is in the
// S1_PARAM functions, also the original code. Per voice: 7 saws + waveshaper on slave DSP A (s1_voice), the
// high-pass on slave DSP B (s1_voice_b).
#include <stdint.h>
#include "s1dsp.h"
#include "braids/resources.h"
#include "stmlib/utils/dsp.h"

using namespace braids;
using stmlib::Interpolate88;
using stmlib::Interpolate824;

// TIMBRE (OSC1 COLOR): Braids' detune -> ratio of each saw's increment to the centre's (k = -3..3)
S1_PARAM(ratio, 0x168, 7) {
  int32_t parameter = knob * 32767 / 255;
  int32_t detune = parameter + 1024;
  detune = (detune * detune) >> 9;
  for (int16_t i = 0; i < 7; ++i) {
    int32_t saw_detune = detune * (i - 3);
    out[i] = s1_exp2(saw_detune / 65536.0f / 128.0f / 12.0f);
  }
}

// COLOR (OSC2 COLOR): offset of the high-pass cutoff from the note, in Braids pitch units (1/128 semitone)
S1_PARAM(hp_offset, 0x16D, 1) {
  int32_t parameter = knob * 32767 / 255;
  out[0] = parameter < 10922 ? ((parameter - 10922) * 24) >> 5 : ((parameter - 10922) * 12) >> 5;
}

static uint32_t phase[7];
static int32_t lp, bp;

S1_VOICE {
  // increments: the centre saw's step (uint32 phase per sample) x the ratios of the TIMBRE table
  float centre = s1_increment() * 2147483648.0f;           // our step is for a phase -1..1: x 2^31 -> uint32
  int32_t sample = -28672;
  for (int i = 0; i < 7; ++i) {
    phase[i] += static_cast<uint32_t>(centre * s1_param(ratio, i));
    sample += phase[i] >> 19;
  }
  sample = Interpolate88(ws_moderate_overdrive, sample + 32768);
  s1_export(0, s1_increment());
  return sample / 32768.0f;
}

S1_VOICE_B {
  // the high-pass of RenderSawSwarm: f = lut_svf_cutoff at pitch + offset, damp = lut_svf_damp[0]
  int32_t sample = static_cast<int32_t>(x * 32768.0f);
  int32_t pitch = static_cast<int32_t>((s1_pitch() + 1.0f) * 1536.0f);   // our octaves -> Braids 1/128 semitone
  int32_t hp_cutoff = pitch + static_cast<int32_t>(s1_param(hp_offset, 0));
  if (hp_cutoff < 0) hp_cutoff = 0; else if (hp_cutoff > 32767) hp_cutoff = 32767;
  int32_t f = Interpolate824(lut_svf_cutoff, hp_cutoff << 17);
  int32_t damp = lut_svf_damp[0];
  int32_t notch, hp;
  notch = sample - (bp * damp >> 15);
  lp += f * bp >> 15;
  CLIP(lp)
  hp = notch - lp;
  bp += f * hp >> 15;
  int32_t result = hp;
  CLIP(result)
  return result / 32768.0f;
}
