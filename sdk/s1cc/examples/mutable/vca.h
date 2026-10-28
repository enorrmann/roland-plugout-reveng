/* vca.h: DSP B of every examples/mutable kernel: the host's AMP envelope (SYSTEM-S1CC builds the ADSR on DSP B when
 * a kernel reads s1_env(0); docs/examples_system_s1cc_README.md §3.1). x = the oscillator, from DSP A. */
#ifndef MUTABLE_VCA_H_
#define MUTABLE_VCA_H_
#include "s1dsp.h"

S1_DSP_B { return x * s1_env(0); }

#endif
