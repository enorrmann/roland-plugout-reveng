// Braids SAW_SWARM for the SYSTEM-1 budget (docs/S1CC.md §11): braids_saw_swarm.cc with the high-pass coefficient
// computed the cheap way. Braids reads f = 2 sin(pi min(fc / fs, 1/8)) from lut_svf_cutoff at pitch + offset; here
// fc / fs = increment x 2^(offset / 12) / 2, from the increment DSP A exports and a knob table, and the sine of a
// small angle is a short fitted polynomial. The per-sample loop is the original's.
#include <math.h>
#include <stdint.h>
#include "s1dsp.h"
#include "braids/resources.h"
#include "stmlib/utils/dsp.h"

using namespace braids;
using stmlib::Interpolate88;

S1_PARAM(ratio, S1_OSC1_COLOR, 7) {                      // TIMBRE: Braids' detune -> increment ratios of the 7 saws
  int32_t parameter = knob * 32767 / 255;
  int32_t detune = parameter + 1024;
  detune = (detune * detune) >> 9;
  for (int16_t i = 0; i < 7; ++i)
    out[i] = s1_exp2(detune * (i - 3) / 65536.0f / 128.0f / 12.0f);
}

S1_PARAM(hp_h, S1_OSC2_COLOR, 1) {                       // COLOR: pi 2^(offset / 12) / 2, offset in semitones as Braids
  int32_t parameter = knob * 32767 / 255;
  int32_t off = parameter < 10922 ? ((parameter - 10922) * 24) >> 5 : ((parameter - 10922) * 12) >> 5;
  out[0] = 3.14159265f * s1_exp2(off / 128.0f / 12.0f) / 2.0f;
}

static uint32_t phase[7];
static int32_t lp, bp;

S1_VOICE {
  float centre = s1_increment() * 2147483648.0f;
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
  int32_t sample = static_cast<int32_t>(x * 32768.0f);
  float u = fminf(fmaxf(s1_import(0) * s1_param(hp_h, 0), 0.000267f), 0.3927f);   // pi fc / fs, Braids' limits
  int32_t f = static_cast<int32_t>(2.0f * sinf(u) * 32767.0f);
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
