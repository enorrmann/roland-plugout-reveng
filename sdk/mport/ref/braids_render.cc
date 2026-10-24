// Reference renderer: Mutable Instruments' original Braids DigitalOscillator, compiled on the PC
// (sdk/mport/reference.py builds it against the eurorack checkout; docs/MUTABLE_SDK.md §6).
//   braids_render SHAPE PITCH P1 P2 N [SEED] [BLOCK] [STRIKE]  -> N int16 samples (little endian) on stdout
// SHAPE = DigitalOscillatorShape index (OSC_SHAPE_SAW_SWARM = 1), PITCH = Braids pitch (MIDI note << 7),
// P1 / P2 = parameters 0..32767 (TIMBRE / COLOR), SEED = stmlib Random state before the first render (default 1),
// BLOCK = samples per Render call (default 24, the size Braids uses), STRIKE = 0: the oscillator state starts
// cleared, with the saw phases at 2^31 (= our phase 0: the port's bank-0 state starts at 0) instead of the random
// values of the first Render (sample-by-sample comparisons).
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#define private public                      // the reference only: reach the state for STRIKE = 0
#include "braids/digital_oscillator.h"
#undef private
#include "stmlib/utils/random.h"

uint32_t stmlib::Random::rng_state_ = 0x21;

int main(int argc, char** argv) {
  if (argc < 6) {
    std::fprintf(stderr, "usage: %s SHAPE PITCH P1 P2 N [SEED] [BLOCK]\n", argv[0]);
    return 2;
  }
  int shape = std::atoi(argv[1]), pitch = std::atoi(argv[2]);
  int p1 = std::atoi(argv[3]), p2 = std::atoi(argv[4]);
  long n = std::atol(argv[5]);
  uint32_t seed = argc > 6 ? std::strtoul(argv[6], 0, 0) : 1;
  int block = argc > 7 ? std::atoi(argv[7]) : 24;
  int strike = argc > 8 ? std::atoi(argv[8]) : 1;
  stmlib::Random::Seed(seed);
  static braids::DigitalOscillator osc;
  osc.Init();
  osc.set_shape(static_cast<braids::DigitalOscillatorShape>(shape));
  std::vector<int16_t> buf(block);
  std::vector<uint8_t> sync(block, 0);
  if (!strike) {                            // a render of 0 samples does the Init and the strike, then clear
    osc.set_pitch(pitch);
    osc.set_parameters(p1, p2);
    osc.Render(sync.data(), buf.data(), 0);
    std::memset(&osc.state_, 0, sizeof(osc.state_));
    osc.phase_ = 0x80000000u;
    for (int i = 0; i < 6; ++i) osc.state_.saw.phase[i] = 0x80000000u;
  }
  for (long done = 0; done < n; done += block) {
    osc.set_pitch(pitch);
    osc.set_parameters(p1, p2);
    osc.Render(sync.data(), buf.data(), block);
    long k = n - done < block ? n - done : block;
    std::fwrite(buf.data(), sizeof(int16_t), k, stdout);
  }
  return 0;
}
