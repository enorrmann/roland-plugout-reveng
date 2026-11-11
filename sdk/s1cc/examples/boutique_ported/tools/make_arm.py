"""make_arm.py: the ARM side of the voice's filter coefficient and VCA gain (SYSTEM-S1CC-BOUTIQUE ARM_CUSTOM,
examples/system_s1cc_boutique/README.md §11; README.md §11 here).

    python3 sdk/s1cc/examples/boutique_ported/tools/make_arm.py MODEL      (MODEL: jp08)

Reads bq_MODEL_host_tables.h (the knob tables of the kernel, already per SYSTEM-1 knob) and writes bq_MODEL_arm.h for
examples/system_s1cc_boutique/engine.c: Q24 tables (1 = 1 << 24; the SYSTEM-1's ARM has no FPU) and arm_custom(v),
which the 1 ms tick calls per voice after the host's envelopes and LFO:

    u = 10.625 CUTOFF + 0.1666 + min((1 - RESO) / 2, 0.075) + ENV MOD x env1^2 + KEY TRACK x (pitch - 4) + LFO octaves
    g = Roland's map of u (instructions 223..267 of the lifted voice: a piecewise curve above 11, e^(ln2 u - 3.2924) x
        0.0035997 (96 kHz), then sin / (sin + cos) of it, x 2): table over u in [U0, U1], linear interpolation
    G = Roland's VCA curve of x = env2^2 (instruction 321): max(0.444274 x^3 - 0.005 + 0.186188 x, 0)

and writes G into the AMP envelope's word, g into the FILTER envelope's word: the kernel reads them as s1_env(0),
s1_env(1). The arithmetic of g follows the lifted C (bq_MODEL_host2x_voice.h) operation by operation in double.
"""
import math
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EX = os.path.abspath(os.path.join(HERE, '..'))
Q = 1 << 24
U0, U1, NU = -6.0, 26.0, 1024          # the table of g: u in [U0, U1], NU steps
NA = 256                               # the table of G: env in [0, 1], NA steps


def table(src, model, name):
    """the knob table MODEL_name[256][1] of the kernel's header, as floats"""
    i = src.index('static const float %s_%s[256]' % (model, name))
    body = src[src.index('{', i) + 1:src.index('};', i)]
    v = [float(x.rstrip('f')) for x in re.findall(r'-?[0-9.]+e[+-]\d+f', body)]
    assert len(v) == 256, (name, len(v))
    return v


def sin_r(x):                          # Roland's polynomial (K_SIN)
    x2 = x * x
    x3 = x2 * x
    return x2 * x3 * (-0.00019836426 * x2 + 0.008333325 + x2 * x2 * 2.7418137e-06) + x - 0.16666603 * x3


def g_of(u):
    """the filter coefficient for the cutoff units u (lifted instructions 224..267)"""
    t224 = u * 0.25
    t225 = t224 * t224
    t226 = t225 * t224
    t227 = u - 11.0
    t228 = 2.3801785 * (t226 * t224) + (54.536343 * t225 - 48.64617 * t224 - 19.821596 * t226)
    t229 = t228 * 4.0 if t227 >= 0 else u
    c_0b5 = max(11.0 + 0.0009999871 * (u - 11.0) if t227 > 0 else t229, -3.0)
    t241 = math.exp(0.6931472 * c_0b5 - 3.292449) * 0.0035997033
    t242 = t241 * t241
    t244 = t242 * t242
    s = sin_r(t241)
    t249 = t244 * (0.041666627 - 0.0013888478 * t242 + t244 * (2.4795532e-05 - 2.3841858e-07 * t242))
    c = 1.0 - 0.5 * t242 + t249
    return 2.0 * s / (s + c)


def amp_of(e):
    x = e * e * 0.99995 # c_08e = env^2 x -0.09999484 x -10
    return max(0.444274 * x * x * x - 0.005 + 0.7 * (x * 0.265983), 0.0)


def q(x):
    return int(round(x * Q))


def arr(name, vals, per=8):
    out = ['static const int32_t %s[%d] = {' % (name, len(vals))]
    for i in range(0, len(vals), per):
        out.append('    ' + ', '.join('%d' % v for v in vals[i:i + per]) + ',')
    out.append('};')
    return out


def main():
    model = sys.argv[1]
    src = open(os.path.join(EX, 'bq_%s_host_tables.h' % model)).read()
    cut, reso, envmod, kf = (table(src, model, n) for n in ('cutoff', 'reso', 'envmod', 'keyfollow'))
    base = [q(10.625 * c + 0.16659999) for c in cut]
    rterm = [q(min((1.0 - r) * 0.5, 0.075)) for r in reso]
    gt = [q(g_of(U0 + (U1 - U0) * i / NU)) for i in range(NU + 1)]
    at = [q(amp_of(i / NA)) for i in range(NA + 1)]
    pitch = [q((n - 12) / 12.0 - 4.0) for n in range(128)]
    lfod = [q(4.0 * (k - 128) / 127.0) for k in range(256)]    # the host's LFO -> FILTER: 4 (k - 128) / 127 octaves
    o = ['/* bq_%s_arm.h: the %s voice\'s filter coefficient and VCA gain on the ARM (SYSTEM-S1CC-BOUTIQUE ARM_CUSTOM).'
         % (model, model.upper()),
         '   Written by sdk/s1cc/examples/boutique_ported/tools/make_arm.py from bq_%s_host_tables.h; do not edit. */' % model,
         '#define ARM_CUSTOM_NK 4',
         '/* the knobs it reads: FILTER CUTOFF, RESONANCE, ENV AMOUNT, KEY TRACK */',
         'static const uint16_t arm_custom_id[ARM_CUSTOM_NK] = { 0x17D, 0x17F, 0x181, 0x186 };',
         '#define ARM_U0 (%d)            /* the g table: u from U0, step (U1 - U0) / NU, Q24 */' % q(U0),
         '#define ARM_USTEP_SH %d' % (24 - int(math.log2(NU / (U1 - U0)))),
         '#define ARM_NU %d' % NU]
    assert (U1 - U0) / NU == 2.0 ** -int(math.log2(NU / (U1 - U0)))
    o += arr('arm_base', base) + arr('arm_rterm', rterm) + arr('arm_envmod', [q(x) for x in envmod])
    o += arr('arm_kf', [q(x) for x in kf]) + arr('arm_pitch', pitch) + arr('arm_lfod', lfod)
    o += arr('arm_g', gt) + arr('arm_amp', at)
    o += ['''
static int32_t arm_lerp(const int32_t *t, int n, int32_t x, int sh)    /* x: index in Q(sh), clamped to the table */
{
    if (x <= 0)
        return t[0];
    int i = x >> sh;
    if (i >= n)
        return t[n];
    int32_t f = x & ((1 << sh) - 1);
    return t[i] + (int32_t)(((int64_t)(t[i + 1] - t[i]) * f) >> sh);
}

/* per voice, every step (engine.c tick, after the envelopes and the LFO): e0 / e1 the AMP / FILTER envelope, lf the
   LFO x fade (Q24), note the voice's note, k[] the knobs of arm_custom_id, lfo_k LFO TO FILTER */
static void arm_custom_voice(int v, int32_t e0, int32_t e1, int32_t lf, int note, const uint8_t *k, int lfo_k)
{
    int32_t e1s = qmul(e1, e1);
    int32_t u = arm_base[k[0]] + arm_rterm[k[1]] + qmul(arm_envmod[k[2]], qmul(e1s, 16776407))   /* x 0.99995 */
              + qmul(arm_kf[k[3]], arm_pitch[note & 127]) + qmul(lf, arm_lfod[lfo_k]);
    int32_t g = arm_lerp(arm_g, ARM_NU, u - ARM_U0, ARM_USTEP_SH);
    int32_t amp = arm_lerp(arm_amp, %d, e0 < 0 ? 0 : e0, 24 - %d);
    ramp(P_ENV0 + v, amp);
    ramp(P_ENV0 + 8 + v, g);
}''' % (NA, int(math.log2(NA)))]
    dst = os.path.join(EX, 'bq_%s_arm.h' % model)
    open(dst, 'w').write('\n'.join(o) + '\n')
    print('%s: g over u %g..%g (%d points): g(-3) %.5f g(5) %.5f g(11) %.5f; amp(1) %.4f' % (
        dst, U0, U1, NU, g_of(-3), g_of(5), g_of(11), amp_of(1)))


if __name__ == '__main__':
    main()
