"""make_arm.py: the ARM side of the voice's filter coefficient and VCA gain (SYSTEM-S1CC-BOUTIQUE ARM_CUSTOM,
examples/system_s1cc_boutique/README.md §11; README.md §11 here).

    python3 sdk/s1cc/examples/boutique_ported/tools/make_arm.py MODEL      (MODEL: jp08)

Reads bq_MODEL_tables.h (the knob tables of the kernel, already per SYSTEM-1 knob) and writes bq_MODEL_arm.h for
examples/system_s1cc_boutique/engine.c: Q24 tables (1 = 1 << 24; the SYSTEM-1's ARM has no FPU) and arm_custom(v),
which the 1 ms tick calls per voice after the host's envelopes and LFO:

    u = 10.625 CUTOFF + 0.1666 + min((1 - RESO) / 2, 0.075) + ENV MOD x env1^2 + KEY TRACK x (pitch - 4) + LFO octaves
    g = Roland's map of u (instructions 223..267 of the lifted voice: a piecewise curve above 11, e^(ln2 u - 3.2924) x
        0.0035997 (96 kHz), then sin / (sin + cos) of it, x 2): table over u in [U0, U1], linear interpolation
    G = Roland's VCA curve of x = env2^2 (instruction 321): max(0.444274 x^3 - 0.005 + 0.186188 x, 0)

and writes G into the AMP envelope's word, g into the FILTER envelope's word: the kernel reads them as s1_env(0),
s1_env(1). The arithmetic of g follows the lifted C (bq_MODEL_host2x_voice.h) operation by operation in double.

Also the model's own envelopes and LFO (ARM_CUSTOM_ENV, ARM_CUSTOM_LFO; README.md §11), from the exact tables
bq_MODEL_tables.h and Roland's per-sample arithmetic (bq_MODEL_voice.h instructions 14..49, 118..204), as closed forms
over one step of 1 ms (96 samples at 96 kHz):

    attack   x <- 1.05 - (1.05 - x) (1 - max(A, 7.927e-6))         until x >= 0.8925, then decay
    decay    x <- s + (x - s) (0.99 + 0.01 max(1 - D, 1.2e-4))       s = 0.87 S + 0.02 max(max(1 - D, 1.2e-4) + 0.01, 1) - 0.02
    release  x <- t + (x - t) (0.99 + 0.01 max(1 - R, 1.43e-4))      t = 0.02 max(max(1 - R, 1.43e-4) + 0.01, 1) - 0.000708
    LFO      phase in [-1, 1) += min(floor(RATE 65536) 2^-32, 0.0071359) per sample; sine = Roland's polynomial of
             |phase|, triangle, saw down, square (JP-08 waves 0..3; S&G / S&H stay the host's)
    delay    y <- 1 - (1 - y) (0.9 + 0.1 DELAY), from 0 at each note start; the LFO is x max(5 y - 4, 0)^4
"""
import math
import struct
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


def amp_of(x):
    """Roland's VCA curve (lifted instruction 667) of x = env2^2 x 0.99995 - (0.5 - 0.5 lfo) x AMP LFO MOD"""
    return max(0.444274 * x * x * x - 0.005 + 0.7 * (x * 0.265983), 0.0)


def note_dt(n):
    """f / 96000 of MIDI note n, as SYSTEM-S1CC's per-note word (make_s1cc.py note_dt; float32 pitch word)"""
    p = struct.unpack('<f', struct.pack('<f', (n - 12) / 12.0))[0]
    return 440.0 * 2.0 ** (p - 4.75) / 96000.0


# the JP-08 parameters without a SYSTEM-1 control: SYSTEM-1 patch words PLUGOUT_EXTEND01.. (ids 0x1B8..), set by CC for
# the tests (examples/system_s1cc_boutique model.c, ARM_CUSTOM_CC_TABLE) and by tools/jp08_prm.py from a JP-08 file.
# Stored value = (JP-08 value - JP-08 default) & 0xFF, so an empty word (0) is the JP-08's default (README.md §13.5).
# (name, patch parameter id, CC, positions (0 = 0..255 from CC x 2), JP-08 max, default, applied here)
EXTEND = [('OSC FREQ MOD DST', 0x198, 102, 3, 2, 0, True),
          ('FLT LPF SLOPE', 0x1AB, 103, 2, 1, 1, False),
          ('FLT ENV MOD SRC', 0x1AD, 104, 2, 1, 0, False),
          ('AMP LEVEL', 0x1B5, 105, 0, 255, 255, True),
          ('ENV1 POLARITY', 0x1C0, 106, 2, 1, 0, False),
          ('ENV2 KEY FOLLOW', 0x1CA, 107, 4, 3, 0, False),
          ('ASSIGN MODE', 0x245, 108, 5, 4, 0, False),
          ('PORTA SW', 0x242, 109, 2, 1, 0, False),
          ('PORTA TIME', 0x243, 85, 0, 255, 100, False),
          ('BEND RANGE', 0x246, 86, 25, 24, 2, False),
          ('DELAY LEVEL', 0x239, 87, 16, 15, 0, False),
          ('DELAY TIME', 0x23A, 88, 16, 15, 0, False),
          ('DELAY FEEDBACK', 0x23B, 89, 16, 15, 0, False),
          ('OSC2 RANGE', 0x19F, 90, 0, 255, 0, False)]
EXT0 = 0x1B8                           # PLUGOUT_EXTEND01 (SYSTEM-1 OS 1.30 parameter table, README.md §13.5)


def q(x):
    return int(round(x * Q))


def rows(src, model, name):
    """an exact table MODEL_name[256][n] of bq_MODEL_tables.h, as rows of floats"""
    i = src.index('static const float %s_%s[256]' % (model, name))
    body = src[src.index('{', i) + 1:src.index('};', i)]
    r = [[float(x.rstrip('f')) for x in row.split(',') if x.strip()] for row in re.findall(r'\{([^{}]*)\}', body)]
    assert len(r) == 256, (name, len(r))
    return r


STEP = 96                              # samples per step (ARM_SEG_MS = 1 at 96 kHz)
SIXTEENTH = STEP // 16                 # the LFO advances in sixteenths of a step


def env_tables(ex, model, n):
    """per knob: attack / decay / release factors over a step, sustain parts, release target (Q24)"""
    a, d, sus, r = (rows(ex, model, 'env%d_%s' % (n, x)) for x in 'adsr')
    att = [q((1.0 - max(a[k][0], 7.927418e-06)) ** STEP) for k in range(256)]
    t133 = [max(1.0 - d[k][0], 0.00011998415) for k in range(256)]
    dec = [q((0.99 + 0.01 * t) ** STEP) for t in t133]
    sus_d = [q(0.02 * max(t + 0.01, 1.0) - 0.02) for t in t133]
    sus_s = [q(0.87 * sus[k][0]) for k in range(256)]
    t143 = [max(1.0 - r[k][0], 0.00014305115) for k in range(256)]
    rel = [q((0.99 + 0.01 * t) ** STEP) for t in t143]
    rtg = [q(0.02 * max(t + 0.01, 1.0) - 0.0007080078) for t in t143]
    att1 = [q(1.0 - max(a[k][0], 7.927418e-06)) for k in range(256)]     # per sample: the step the attack ends in
    dec1 = [q(0.99 + 0.01 * t) for t in t133]
    return att, dec, sus_s, sus_d, rel, rtg, att1, dec1


def arr(name, vals, per=8, ty='int32_t'):
    out = ['static const %s %s[%d] = {' % (ty, name, len(vals))]
    for i in range(0, len(vals), per):
        out.append('    ' + ', '.join('%d' % v for v in vals[i:i + per]) + ',')
    out.append('};')
    return out


def main():
    model = sys.argv[1]
    src = open(os.path.join(EX, 'bq_%s_tables.h' % model)).read()
    cut, reso, envmod, kf = (table(src, model, n) for n in ('cutoff', 'reso', 'envmod', 'keyfollow'))
    base = [q(10.625 * c + 0.16659999) for c in cut]
    rterm = [q(min((1.0 - r) * 0.5, 0.075)) for r in reso]
    gt = [q(g_of(U0 + (U1 - U0) * i / NU)) for i in range(NU + 1)]
    at = [q(amp_of(i / NA)) for i in range(NA + 1)]       # over x in [0, 1] (x <= 0: 0)
    pitch = [q((n - 12) / 12.0 - 4.0) for n in range(128)]
    ex = open(os.path.join(EX, 'bq_%s_tables.h' % model)).read()
    lfop = [q(x[0]) for x in rows(ex, model, 'lfo_pitch')]      # OSC LFO MOD (C04A): octaves at LFO = 1
    envp = [q(x[0]) for x in rows(ex, model, 'env_pitch')]      # OSC ENV MOD (C04B): octaves at ENV1 = 1
    alev = [q(x[0]) for x in rows(ex, model, 'amp_level')]      # AMP LEVEL (C06F)
    alfo = [q(x[0]) for x in rows(ex, model, 'amp_lfo')][:4]     # AMP LFO MOD (C06B), 0..3
    notelog = [q(math.log2(note_dt(n))) for n in range(128)]
    mant = [int(round((2.0 ** (i / 1024.0) - 1.0) * (1 << 23))) for i in range(1025)]
    lfod = [q(x[0]) for x in rows(ex, model, 'lfo_filter')]     # FLT LFO MOD (C064): octaves at LFO = 1
    rate = rows(ex, model, 'lfo_rate')
    inc16 = [int(round(min(math.floor(rate[k][0] * 65536.0) * 2.0 ** -32, 0.0071359314) * 2.0 ** 31 * SIXTEENTH))
             for k in range(256)]                               # phase span 2 = 2^32
    fade16 = [q((0.1 * x[0] + 0.9) ** SIXTEENTH) for x in rows(ex, model, 'lfo_delay')]
    envt = {0: env_tables(ex, model, 2), 1: env_tables(ex, model, 1)}   # engine i 0 = AMP = ENV2, 1 = FILTER = ENV1
    o = ['/* bq_%s_arm.h: the %s voice\'s filter coefficient and VCA gain on the ARM (SYSTEM-S1CC-BOUTIQUE ARM_CUSTOM).'
         % (model, model.upper()),
         '   Written by sdk/s1cc/examples/boutique_ported/tools/make_arm.py from bq_%s_tables.h; do not edit. */' % model,
         '',
         '#ifdef ARM_CUSTOM_CC_TABLE',
         '/* model.c (region 0): MIDI CC -> PLUGOUT_EXTEND word (README.md §13.5): CC 0..127 -> the JP-08 value (n positions:',
         '   value x n >> 7; 0: x 2, 127 -> 255), stored as (value - default) & 0xFF; steps 0 means 0..255 */',
         'static const struct { uint8_t cc; uint16_t id; uint8_t steps, max, def; } arm_custom_cc[] = {']
    for i, (nm, pid, cc, st, mx, df, ap) in enumerate(EXTEND):
        o.append('    { %d, 0x%03X, %d, %d, %d },   /* EXTEND%02d: %s (JP-08 0x%03X)%s */'
                 % (cc, EXT0 + i, st, mx, df, i + 1, nm, pid, '' if ap else ' stored only, not applied yet'))
    o += ['};',
          '#else',
          '#define ARM_CUSTOM_NK 11',
          '/* the knobs it reads: FILTER CUTOFF, RESONANCE, ENV AMOUNT, KEY TRACK; LFO TO PITCH = OSC LFO MOD, PITCH ENV',
          '   AMOUNT = OSC ENV MOD (JP-08: ENV1 -> pitch), OSC1 / OSC2 RANGE; EXTEND01 OSC FREQ MOD DST, EXTEND04 AMP LEVEL;',
          '   LFO TO AMP = AMP LFO MOD (4 positions: knob >> 6) (README.md §13.5) */',
          'static const uint16_t arm_custom_id[ARM_CUSTOM_NK] = { 0x17D, 0x17F, 0x181, 0x186, 0x163, 0x17A, 0x16A, 0x16F,',
          '                                                        0x%03X, 0x%03X, 0x165 };' % (EXT0, EXT0 + 3),
          '/* their values before the first knob / patch arrives (the JP-08 defaults; EXTEND 0 = the JP-08 default) */',
          '#define ARM_CUSTOM_INIT { 128, 128, 128, 128, 0, 0, 2, 2, 0, 0, 0 }',
          '#define ARM_EXT(k, def) ((uint8_t)((k) + (def)))      /* EXTEND word -> the JP-08 value */',
         '#define ARM_U0 (%d)            /* the g table: u from U0, step (U1 - U0) / NU, Q24 */' % q(U0),
         '#define ARM_USTEP_SH %d' % (24 - int(math.log2(NU / (U1 - U0)))),
         '#define ARM_NU %d' % NU]
    assert (U1 - U0) / NU == 2.0 ** -int(math.log2(NU / (U1 - U0)))
    o += arr('arm_base', base) + arr('arm_rterm', rterm) + arr('arm_envmod', [q(x) for x in envmod])
    o += arr('arm_kf', [q(x) for x in kf]) + arr('arm_pitch', pitch) + arr('arm_lfod', lfod)
    o += arr('arm_g', gt) + arr('arm_amp', at)
    o += ['/* pitch (README.md §13): OSC LFO MOD / OSC ENV MOD in octaves, AMP LEVEL, AMP LFO MOD; log2 of each note\'s',
          '   f / 96000; the mantissa bits of 2^(i / 1024) */']
    o += arr('arm_lfop', lfop) + arr('arm_envp', envp) + arr('arm_alev', alev) + arr('arm_alfo', alfo)
    o += arr('arm_notelog', notelog) + arr('arm_mant', mant)
    o += ['', '/* ARM_CUSTOM_ENV: the model\'s envelopes, [i][knob], i 0 = AMP (ENV2), 1 = FILTER (ENV1); per step of 1 ms */',
          '#define ARM_CUSTOM_ENV 1']
    for j, nm in enumerate(('att', 'dec', 'sus_s', 'sus_d', 'rel', 'rtg', 'att1', 'dec1')):
        o.append('static const int32_t arm_env_%s[2][256] = {' % nm)
        for i in (0, 1):
            vals = envt[i][j]
            o.append('    {' + ', '.join('%d' % x for x in vals) + '},')
        o.append('};')
    o += ['', '/* ARM_CUSTOM_LFO: the model\'s LFO rate (phase per sixteenth of a step), delay factor per sixteenth */',
          '#define ARM_CUSTOM_LFO 1']
    o += arr('arm_custom_lfo_inc16', inc16, ty='uint32_t')
    o += arr('arm_custom_lfo_fade16', fade16)
    o += ['#define ARM_ATT_TOP %d                /* 1.05 */' % q(1.05),
          '#define ARM_ATT_END %d                /* 0.8925: the end of the attack */' % q(0.8925),
          '#define ARM_STEP %d                      /* samples per step */' % STEP,
          '#define ARM_SIN_A %d  /* Roland\'s sine of t = |phase|: (A t^2 - B t - C t^3 - D) x 2 */' % q(3.125),
          '#define ARM_SIN_B %d' % q(0.04159999), '#define ARM_SIN_C %d' % q(2.0799866), '#define ARM_SIN_D %d' % q(0.5039997)]
    o += ['''
/* one envelope over n steps of 1 ms: st = ST_ATT / ST_DEC / ST_REL, *l the level (Q24); k = S, A, D, R knobs */
static int arm_custom_env(int i, int st, int32_t *l, const uint8_t *k, int n)
{
    int32_t x = *l;
    for (int j = 0; j < n; j++) {
        if (st == ST_ATT) {
            int32_t y = ARM_ATT_TOP - qmul(ARM_ATT_TOP - x, arm_env_att[i][k[1]]);
            if (y < ARM_ATT_END) {
                x = y;
                continue;
            }
            /* the attack ends in this step: sample by sample (as Roland), the rest of the step in decay */
            int32_t s = arm_env_sus_s[i][k[0]] + arm_env_sus_d[i][k[2]];
            for (int m = 0; m < ARM_STEP; m++) {
                if (st == ST_ATT) {
                    x = ARM_ATT_TOP - qmul(ARM_ATT_TOP - x, arm_env_att1[i][k[1]]);
                    if (x >= ARM_ATT_END)
                        st = ST_DEC;
                } else {
                    x = s + qmul(x - s, arm_env_dec1[i][k[2]]);
                }
            }
        } else if (st == ST_DEC) {
            int32_t s = arm_env_sus_s[i][k[0]] + arm_env_sus_d[i][k[2]];
            x = s + qmul(x - s, arm_env_dec[i][k[2]]);
        } else if (st == ST_REL) {
            int32_t t = arm_env_rtg[i][k[3]];
            x = t + qmul(x - t, arm_env_rel[i][k[3]]);
            if (x - t < 64 && t - x < 64) {           /* within 4e-6 of the target: done */
                x = t;
                st = ST_IDLE;
            }
        }
        if (x < 0)
            x = 0;
    }
    *l = x;
    return st;
}

/* Roland's LFO shapes (pre program, instructions 26..45) of the phase ph (2^32 = span 2): 1 = done, 0 = the host's */
static int arm_custom_lfo_shape(int sh, uint32_t ph, int32_t *w)
{
    int32_t p = (int32_t)ph >> 7;                      /* -1..1 */
    switch (sh) {
    case 0: {                                          /* sine: (3.125 t^2 - 0.0416 t - 2.08 t^3 - 0.504) x 2 */
        int32_t t = p < 0 ? -p : p, t2 = qmul(t, t), t3 = qmul(t2, t);
        *w = 2 * (qmul(ARM_SIN_A, t2) - qmul(ARM_SIN_B, t) - qmul(ARM_SIN_C, t3) - ARM_SIN_D);
        return 1;
    }
    case 1: {                                          /* triangle: s1_tri(wrap(p - 0.5)) = 1 - 2 |wrap(-q - 1.5)| */
        int32_t a = (int32_t)(-(ph - 0x40000000u) - 0xC0000000u) >> 7;
        *w = Q1 - 2 * (a < 0 ? -a : a);
        return 1;
    }
    case 2:                                            /* saw down: -wrap(p - 1) */
        *w = -((int32_t)(ph - 0x80000000u) >> 7);
        return 1;
    case 3: {                                          /* square: sign(wrap(p - 1)) */
        int32_t a = (int32_t)(ph - 0x80000000u);
        *w = a > 0 ? Q1 : a < 0 ? -Q1 : 0;
        return 1;
    }
    default:
        return 0;
    }
}

/* the delay's gain of y: max(5 y - 4, 0)^4 */
static int32_t arm_custom_lfo_gain(int32_t y)
{
    int32_t t = 5 * y - 4 * Q1;
    if (t <= 0)
        return 0;
    t = qmul(t, t);
    return qmul(t, t);
}''']
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

/* 2^x (x Q24 octaves) as IEEE float bits: the exponent from floor(x), the mantissa from arm_mant (linear) */
static uint32_t arm_exp2_bits(int32_t x)
{
    int32_t e = x >> 24;
    uint32_t f = (uint32_t)x & 0xFFFFFFu, i = f >> 14, r = f & 0x3FFFu;
    uint32_t m = (uint32_t)arm_mant[i] + (uint32_t)(((uint64_t)(uint32_t)(arm_mant[i + 1] - arm_mant[i]) * r) >> 14);
    if (e < -126)
        return 0;
    return ((uint32_t)(e + 127) << 23) | (m & 0x7FFFFFu);
}

/* the JP-08's pitch modulation (lifted instructions 211..231): LFO x OSC LFO MOD + ENV1 out x OSC ENV MOD, octaves, to
   OSC1 and / or OSC2 by OSC FREQ MOD DST (C04F..C052: 0 = VCO-2 (0, 1, 0, 1), 1 = both, 2 = VCO-1 (1, 0, 1, 0));
   no SYSTEM-1 LFO -> PITCH or pitch envelope (the build has LFO_PITCH=0). Writes the voice's note words: s1_dt /
   s1_rdt = OSC1 (note, OSC1 RANGE, its modulation), s1_pitch = 4.75 + o2 - o1 (octaves; the kernel takes 2^(p - 4.75)) */
static void arm_custom_pitch(int v, int32_t e1, int32_t lf, int note, const uint8_t *k)
{
    int32_t e1o = qmul(qmul(e1, e1), 16776407);          /* ENV1 out = env1^2 x 0.99995 */
    int32_t pm = qmul(lf, arm_lfop[k[4]]) + qmul(e1o, arm_envp[k[5]]);
    int dst = ARM_EXT(k[8], 0);
    int r1 = k[6] > 5 ? 5 : k[6], r2 = k[7] > 5 ? 5 : k[7];   /* OSC RANGE 64' .. 2' = -1 .. +4 octaves (SYSTEM-S1CC) */
    int32_t o1 = arm_notelog[note & 127] + ((int32_t)(r1 - 1) << 24) + (dst == 0 ? 0 : pm);
    int32_t d = ((int32_t)(r2 - r1) << 24) + (dst == 0 ? pm : dst == 2 ? -pm : 0);
    b5(B5_DT + v, arm_exp2_bits(o1));
    b5(B5_RDT + v, arm_exp2_bits(-o1 - (14 << 24)));      /* 1 / dt / 16384 */
    b5(B5_PITCH + v, q24_bits(d + 79691776));            /* + 4.75 */
}
#define ARM_CUSTOM_PITCH 1

/* per voice, every step (engine.c tick, after the envelopes and the LFO): e0 / e1 the AMP / FILTER envelope, lf the
   LFO x fade (Q24), note the voice's note, k[] the knobs of arm_custom_id, lfo_k LFO TO FILTER */
static void arm_custom_voice(int v, int32_t e0, int32_t e1, int32_t lf, int note, const uint8_t *k, int lfo_k)
{
    arm_custom_pitch(v, e1, lf, note, k);
    int32_t e1s = qmul(e1, e1);
    int32_t u = arm_base[k[0]] + arm_rterm[k[1]] + qmul(arm_envmod[k[2]], qmul(e1s, 16776407))   /* x 0.99995 */
              + qmul(arm_kf[k[3]], arm_pitch[note & 127]) + qmul(lf, arm_lfod[lfo_k]);
    int32_t g = arm_lerp(arm_g, ARM_NU, u - ARM_U0, ARM_USTEP_SH);
    /* the VCA (lifted instructions 657..667): x = env2^2 x 0.99995 - (0.5 - 0.5 lfo) x AMP LFO MOD, Roland's curve, x AMP
       LEVEL */
    int32_t x = qmul(qmul(e0, e0), 16776407) - qmul(Q1 / 2 - lf / 2, arm_alfo[k[10] >> 6]);
    int32_t amp = qmul(arm_lerp(arm_amp, %d, x < 0 ? 0 : x, 24 - %d), arm_alev[ARM_EXT(k[9], 255)]);
    ramp(P_ENV0 + v, amp);
    ramp(P_ENV0 + 8 + v, g);
}''' % (NA, int(math.log2(NA)))]
    o.append('#endif /* ARM_CUSTOM_CC_TABLE */')
    dst = os.path.join(EX, 'bq_%s_arm.h' % model)
    open(dst, 'w').write('\n'.join(o) + '\n')
    print('%s: g over u %g..%g (%d points): g(-3) %.5f g(5) %.5f g(11) %.5f; amp(1) %.4f' % (
        dst, U0, U1, NU, g_of(-3), g_of(5), g_of(11), amp_of(1)))


if __name__ == '__main__':
    main()
