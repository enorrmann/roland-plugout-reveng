/* written by sdk/s1cc/tools/dsplift.py --kernel: jx3p_dco2_noise=/home/emilio/git/roland-plugout-reveng/work/jx3p/jx3p_dco2_noise.txt */
#include "dsplift_k.h"

/* jx3p_dco2_noise: written by sdk/s1cc/tools/dsplift.py --kernel. Every operation of the program, in order, as
 * expressions (dsplift.md §6): a name per value that is used more than once, stored in bank 0 or too long;
 * the number after each line = the instruction that makes it. SEL(w, a, b) = w ? a : b (w = 0 / 1), LERP(a, b, t) = a + t (b - a),
 * K_NZ / K_GE0 / K_LE0 = the condition bits, K_RECIP = the reciprocal (dsplift_k.h). Delay chains: X_zK =
 * what cell X held K samples ago. */
static float jx3p_dco2_noise_f0;
static inline __attribute__((always_inline)) void jx3p_dco2_noise(void)
{
    KOUT(0x2221, 0.0f);
    KOUT(0x221d, 0.0f);

    jx3p_dco2_noise_f0 = jx3p_dco2_noise_f0;
}

