/* plaits_common.h: what every plaits_*.cc wrapper shares. Plaits (plaits/dsp/oscillator/*.h, (c) Emilie Gillet,
 * MIT) is compiled from the original headers with the real stmlib (sdk/s1cc/s1cc deps); each wrapper is one
 * oscillator rendering one sample per DSP sample, for SYSTEM-S1CC:
 *   S1_DSP_A  the oscillator
 *   S1_DSP_B  the host's AMP envelope as the VCA (vca.h)
 * Plaits counts frequencies in cycles per sample at 48 kHz; the DSP runs at 96 kHz: f = s1_increment() / 2 (our
 * increment is for a phase -1..1). Knob ranges follow the Plaits engine that uses the oscillator.
 * Build: sdk/s1cc/s1cc build sdk/s1cc/examples/mutable/plaits_z.cc -I ~/git/eurorack --regs 0,1,2,3,4,5,6,7 --check */
#ifndef MUTABLE_PLAITS_COMMON_H_
#define MUTABLE_PLAITS_COMMON_H_
#include <math.h>
#include "s1dsp.h"
#include "vca.h"
#include "plaits/dsp/oscillator/oscillator.h"   /* kMaxFrequency: the engines include it before the others */

static inline float plaits_frequency(void) { return s1_increment() * 0.5f; }   /* cycles per sample */

#endif
