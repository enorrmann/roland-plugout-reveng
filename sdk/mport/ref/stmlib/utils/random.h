// Minimal stand-in for stmlib/utils/random.h: the linear congruential generator of stmlib (same constants), with a
// seed the reference renderer sets (sdk/mport/reference.py reproduces the strike phases from it).
#ifndef MPORT_STMLIB_UTILS_RANDOM_H_
#define MPORT_STMLIB_UTILS_RANDOM_H_
#include "stmlib/stmlib.h"

namespace stmlib {

class Random {
 public:
  static inline void Seed(uint32_t seed) { rng_state_ = seed; }
  static inline uint32_t state() { return rng_state_; }
  static inline uint32_t GetWord() {
    rng_state_ = rng_state_ * 1664525L + 1013904223L;
    return state();
  }
  static inline int16_t GetSample() { return static_cast<int16_t>(GetWord() >> 16); }
  static inline float GetFloat() { return static_cast<float>(GetWord()) / 4294967296.0f; }
 private:
  static uint32_t rng_state_;
};

}  // namespace stmlib

#endif
