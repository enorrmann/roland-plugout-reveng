/* dsplift_k.h: runtime of the C that `dsplift.py --kernel` writes (docs/vol5_toolchain_compiler_sdk/dsplift.md §6).
 *
 * The same unit semantics as dsplift_rt.h, in a form s1cc compiles (s1dsp.h intrinsics, no RAW rounding, no
 * memory): the lifted C is expressions on named values (dsplift_c.py), the SFU parts and the condition bits are the
 * macros below. The including file defines, before including the lifted C:
 *   KP(a)      a parameter word (C000..): a constant or s1_param(...)
 *   KIN(a)     a bank-5 word the program reads and does not write (an input)
 *   KOUT(a, v) a bank-5 word the program writes (an output; may do nothing)
 *   KE(a)      a bank-7 word (E000..)
 * Natively (no s1dsp.h) the intrinsics fall back to libm (S1_NATIVE_FALLBACK below). */
#ifndef DSPLIFT_K_H
#define DSPLIFT_K_H
#include <math.h>
#ifdef DSPLIFT_K_NATIVE
static inline float s1_recip(float x) { return 1.0f / x; }
static inline float s1_floor(float x) { return floorf(x); }
static inline float s1_ceil(float x) { return ceilf(x); }
static inline float s1_exp(float x) { return expf(x); }
static inline float s1_sign(float x) { return x > 0 ? 1.0f : x < 0 ? -1.0f : 0.0f; }
static inline float s1_wrap(float x) { return x - 2.0f * floorf((x + 1.0f) * 0.5f); }
static inline float s1_clamp(float x) { return fminf(fmaxf(x, -1.0f), 1.0f); }
static inline float s1_abs(float x) { return fabsf(x); }
static inline float s1_range(float x, float lo, float hi) { (void)lo, (void)hi; return x; }
#else
#include "s1dsp.h"
#endif

/* Python's x % m (unit B's phase ops) */
#define pymod(x, m) ((x) - (m) * s1_floor((x) / (m)))

/* the idioms of the lifted C (dsplift_c.py): each expands to the expression the operation computes */
/* SEL(w, a, b): w ? a : b for w = 0 / 1, without a compare (a unit a condition may skip: old + w (new - old)) */
#define SEL(w, a, b) ((b) + (w) * ((a) - (b)))
/* LERP(a, b, t) = a + t (b - a): a one-pole step, a crossfade */
#define LERP(a, b, t) ((a) + (t) * ((b) - (a)))
/* XFADE(a, b, w) = (1 - w) a + w b, as a MAC computes it: w b + a - w a */
#define XFADE(a, b, w) ((w) * (b) + (a) - (w) * (a))
/* the condition bits, 0 / 1, from the exact SFU sign (docs §6.2), and their complements (1 - bit) */
#define K_NZ(z) s1_abs(s1_sign(z))                                            /* z != 0 */
#define K_Z(z) (1.0f - K_NZ(z))                                               /* z == 0 */
#define K_GE0(v) fminf(1.0f, 1.0f + s1_sign(v))                               /* v >= 0 */
#define K_LT0(v) (1.0f - K_GE0(v))                                            /* v < 0 */
#define K_LE0VZ(v, z) (1.0f - fmaxf(s1_sign(v), 0.0f) * s1_abs(s1_sign(z)))   /* v < 0 or z == 0 */
#define K_GT0VZ(v, z) (1.0f - K_LE0VZ(v, z))
#define K_LE0(v) K_LE0VZ(v, v)                                                /* v <= 0 */
#define K_GT0(v) (1.0f - K_LE0(v))                                            /* v > 0 */
/* max(-1, min(1, x)) */
#define K_CLAMP(x) fmaxf(-1.0f, fminf(1.0f, x))
/* Roland's polynomials (the TB-03 voice): sin(x) for |x| <= pi/2, e^x (expsplit: e^floor(x) times the series of the
   fraction), and the triangle of unit B's phase op (x in 0..1 -> -1..1..-1) */
#define K_SIN(x) ({ float _x = (x), _x2 = _x * _x, _x3 = _x2 * _x; \
        _x2 * _x3 * (-0.00019836426f * _x2 + 0.008333325f + _x2 * _x2 * 2.7418137e-06f) + _x - 0.16666603f * _x3; })
#define K_EXP(x) ({ float _x = (x), _f = s1_floor(_x), _r = _x - _f, _q = 0.25f * _r * _r; \
        float _a = _q * (_q * (2.565328e-05f * _r + 0.00028218678f) + 0.006349206f + 0.0007054673f * _r); \
        float _b = _q * (_q * (_a + 0.08888889f + 0.012698413f * _r) + 0.6666667f + 0.13333333f * _r); \
        s1_exp(_f) * (_q * (_b + 2.0f + 0.6666667f * _r) + 1.0f + _r); })
#define K_TRI(x) (1.0f - 2.0f * s1_abs(s1_wrap(-(x) - 1.5f)))
/* recipseed with its newton steps: the exact reciprocal; |y| below 2^-9 is taken as 2^-9, y = 0 gives 0 */
#define K_RECIP(y) (s1_sign(y) * s1_recip(fmaxf(s1_abs(y), 0x1p-9f)))
/* the A unit's overflow: |r| >= 512 wraps to copysign(|r| mod 512, r) (dspcore.h a_op); only where --overflow-wrap asks */
#define KOVERFLOW(e) ({ float _r = (e); _r - 512.0f * s1_sign(_r) * s1_floor(s1_abs(_r) * (1.0f / 512.0f)); })
#endif
