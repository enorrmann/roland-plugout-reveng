// Minimal stand-in for Mutable Instruments' stmlib/stmlib.h (the eurorack checkout has the stmlib submodule
// empty). Only what the ported Braids/Plaits sources need to compile on the PC for reference renders
// (sdk/mport/reference.py); written for this repo, docs/MUTABLE_SDK.md §6.
#ifndef MPORT_STMLIB_H_
#define MPORT_STMLIB_H_
#include <stdint.h>
#include <stddef.h>

#ifndef NULL
#define NULL 0
#endif

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#define CLIP(x) if (x < -32767) x = -32767; if (x > 32767) x = 32767;
#define CONSTRAIN(var, min, max) \
  if (var < (min)) { var = (min); } else if (var > (max)) { var = (max); }

#define JOIN(lhs, rhs) JOIN_1(lhs, rhs)
#define JOIN_1(lhs, rhs) JOIN_2(lhs, rhs)
#define JOIN_2(lhs, rhs) lhs##rhs
#define STATIC_ASSERT(expression, message) \
  struct JOIN(__static_assertion_at_line_, __LINE__) { \
    int JOIN(STATIC_ASSERTION_FAILED_AT_LINE_, __LINE__)[(expression) ? 1 : -1]; }

#define IN_RAM
#define UNROLL2(x) x; x;
#define UNROLL4(x) x; x; x; x;

namespace stmlib {
typedef union { uint16_t value; uint8_t bytes[2]; } Word;
typedef union { uint32_t value; uint16_t words[2]; uint8_t bytes[4]; } LongWord;
}  // namespace stmlib

#endif
