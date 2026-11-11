/* dsplift_k.h: runtime of the C that `dsplift.py --kernel` writes (docs/vol5_toolchain_compiler_sdk/dsplift.md §6).
 *
 * The same unit semantics as dsplift_rt.h, in a form s1cc compiles (s1dsp.h intrinsics, no RAW rounding, no
 * memory): registers and flags are local arrays with constant indexes (clang turns them into values), the SFU
 * parts become intrinsics. The including file defines, before including the lifted C:
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

/* Python's x % m and the phase ops of unit B */
#define pymod(x, m) ((x) - (m) * s1_floor((x) / (m)))
#define pymin(x, y) fminf(x, y)
#define pymax(x, y) fmaxf(x, y)
#define wrap1(v) s1_wrap(v)
#define fabs(x) s1_abs(x)

/* unit results for the condition bits (dsplift.py --kernel kernel_insn): rvv = the value whose sign gives N, rzd =
   the value whose zero gives Z (min / max: Z = X == Y, so rzd = X - Y); no comparisons (s1_sign) */
#define RES(x) (rs = 1, rvv = (x), rzd = (x))
#define RESV(x) do { float _v = (x); rs = 1, rvv = _v, rzd = _v; } while (0)
#define RESZ(v, d) (rs = 1, rvv = (v), rzd = (d))
#define AWRAP(e) (e)
/* the A unit's overflow: |r| >= 512 wraps to copysign(|r| mod 512, r) (dspcore.h a_op); only where --overflow-wrap asks */
#define KOVERFLOW(e) ({ float _r = (e); _r - 512.0f * s1_sign(_r) * s1_floor(s1_abs(_r) * (1.0f / 512.0f)); })
#define KDONE(u, dest) do { rv[u] = rs ? rvv : n[dest]; rz[u] = rs ? rzz : n[dest] == 0; } while (0)
#define KFLAGS(g, dest) do { float _v = rs ? rvv : n[dest]; int _z = rs ? rzz : n[dest] == 0; um[g] = 1; \
        uv[4 * (g)] = !_z; uv[4 * (g) + 1] = !(_v < 0); uv[4 * (g) + 2] = 1; uv[4 * (g) + 3] = _v < 0 || _z; } while (0)
#define KSETFLAGS(g, u) do { um[g] = 1; uv[4 * (g)] = !rz[u]; uv[4 * (g) + 1] = !(rv[u] < 0); uv[4 * (g) + 2] = 1; \
        uv[4 * (g) + 3] = rv[u] < 0 || rz[u]; } while (0)

/* the SFU (dspcore.h sfu) with intrinsics: recipseed gives the exact reciprocal (the newton steps after it then
 * keep it: rD = 1/Y, r(D+1) = 1, rY = 1); expsplit gives e^floor(Y) and frac(Y) */
#define SFU(w) do { const int _d = ((w) >> 12) - 8, _m = ((w) >> 4) & 3, _y = (w) & 15; float _Y = r[_y & 7]; \
        if (_m == 0 && !(_y & 8)) { if (_Y != 0) { n[_d] = s1_recip(_Y); n[(_d + 1) & 7] = 1.0f; n[_y & 7] = 1.0f; } } \
        else if (_m == 1 && (_y & 8)) { float _f = s1_floor(_Y); n[_d] = s1_exp(_f); n[_y & 7] = _Y - _f; } \
        else if (_m == 1) { n[_d] = 0.0f; /* sqrtsplit: not supported */ } \
        else if (_m == 2) n[_d] = s1_sign(_Y); \
        else if (_y & 8) n[_d] = s1_floor(_Y); \
        else n[_d] = s1_ceil(_Y); } while (0)
#endif
