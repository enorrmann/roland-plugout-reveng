// Minimal stand-in for stmlib/utils/dsp.h: the fixed-point interpolation helpers the Braids oscillators use.
// Semantics as in Mutable Instruments' stmlib (table[i] + (table[i+1] - table[i]) * fraction, truncating shifts).
#ifndef MPORT_STMLIB_UTILS_DSP_H_
#define MPORT_STMLIB_UTILS_DSP_H_
#include "stmlib/stmlib.h"

namespace stmlib {

inline int16_t Interpolate824(const int16_t* table, uint32_t phase) {
  int32_t a = table[phase >> 24];
  int32_t b = table[(phase >> 24) + 1];
  return a + ((b - a) * static_cast<int32_t>((phase >> 8) & 0xffff) >> 16);
}

inline uint16_t Interpolate824(const uint16_t* table, uint32_t phase) {
  uint32_t a = table[phase >> 24];
  uint32_t b = table[(phase >> 24) + 1];
  return a + ((b - a) * static_cast<uint32_t>((phase >> 8) & 0xffff) >> 16);
}

inline int16_t Interpolate824(const uint8_t* table, uint32_t phase) {
  int32_t a = table[phase >> 24];
  int32_t b = table[(phase >> 24) + 1];
  return (a << 8) + ((b - a) * static_cast<int32_t>(phase & 0xffffff) >> 16) - 32768;
}

inline int16_t Interpolate1022(const int16_t* table, uint32_t phase) {
  int32_t a = table[phase >> 22];
  int32_t b = table[(phase >> 22) + 1];
  return a + ((b - a) * static_cast<int32_t>((phase >> 6) & 0xffff) >> 16);
}

inline int16_t Interpolate115(const int16_t* table, uint16_t phase) {
  int32_t a = table[phase >> 5];
  int32_t b = table[(phase >> 5) + 1];
  return a + ((b - a) * static_cast<int32_t>(phase & 0x1f) >> 5);
}

inline int16_t Interpolate88(const int16_t* table, uint16_t index) {
  int32_t a = table[index >> 8];
  int32_t b = table[(index >> 8) + 1];
  return a + ((b - a) * static_cast<int32_t>(index & 0xff) >> 8);
}

inline uint16_t Interpolate88(const uint16_t* table, uint16_t index) {
  int32_t a = table[index >> 8];
  int32_t b = table[(index >> 8) + 1];
  return a + ((b - a) * static_cast<int32_t>(index & 0xff) >> 8);
}

inline int16_t Crossfade(const int16_t* table_a, const int16_t* table_b, uint32_t phase, uint16_t balance) {
  int32_t a = Interpolate824(table_a, phase);
  int32_t b = Interpolate824(table_b, phase);
  return a + ((b - a) * static_cast<int32_t>(balance) >> 16);
}

inline int16_t Crossfade(const uint8_t* table_a, const uint8_t* table_b, uint32_t phase, uint16_t balance) {
  int32_t a = Interpolate824(table_a, phase);
  int32_t b = Interpolate824(table_b, phase);
  return a + ((b - a) * static_cast<int32_t>(balance) >> 16);
}

inline int16_t Mix(int16_t a, int16_t b, uint16_t balance) {
  return (a * (65535 - balance) + b * balance) >> 16;
}

inline uint16_t Mix(uint16_t a, uint16_t b, uint16_t balance) {
  return (a * (65535 - balance) + b * balance) >> 16;
}

}  // namespace stmlib

#endif
