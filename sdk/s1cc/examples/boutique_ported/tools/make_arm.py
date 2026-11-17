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
EXTEND = [('OSC FREQ MOD DST', 0x198, 103, 3, 2, 0, True),       # CC 102 never reaches the model (PARAMLOG 2026-11-12)
          ('FLT ENV MOD SRC', 0x1AD, 104, 2, 1, 1, True),    # default 1 = ENV1 (user 2026-11-12; the JP-08's is 0)
          ('AMP LEVEL', 0x1B5, 105, 0, 255, 255, True),
          ('BEND RANGE', 0x246, 106, 25, 24, 2, True)]     # semitones; by CC and the patch (user 2026-11-12)
# the other JP-08 parameters have a SYSTEM-1 control (tools/jp08_prm.py PANEL): LFO TO AMP = AMP LFO MOD, OSC2 RANGE +
# COARSE = OSC2 RANGE (semitones -36..+24), PORTAMENTO = PORTA SW / TIME, BEND RANGE, LPF TYPE = FLT LPF SLOPE,
# DELAY / DELAY TIME = DELAY LEVEL / TIME (the SYSTEM-1's effect, not the JP-08's); ASSIGN MODE stays here until its
# modes are read from the firmware
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
R = 44100.0 / 96000.0                  # the Boutique's rate over the SYSTEM-1's (tools/bq_arm.py R)
SIXTEENTH = STEP // 16                 # the LFO advances in sixteenths of a step


def f32bits(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


def env_raw(ex, model, n):
    """the envelope's words at 44.1 kHz (ENV KEY FOLLOW scales them before the 96 kHz conversion), as float bits:
    attack A (inverse of bq_arm.py one_pole), decay / release 0.01 D, 0.01 R (inverse of env_rate)"""
    a, d, r = (rows(ex, model, 'env%d_%s' % (n, x)) for x in 'adr')
    att = [f32bits(1.0 - (1.0 - min(max(x[0], 0.0), 1.0)) ** (1.0 / R)) for x in a]
    dec = [f32bits(1.0 - (1.0 - min(max(0.01 * x[0], 0.0), 1.0)) ** (1.0 / R)) for x in d]
    rel = [f32bits(1.0 - (1.0 - min(max(0.01 * x[0], 0.0), 1.0)) ** (1.0 / R)) for x in r]
    return att, dec, rel


def env_tables(ex, model, n):
    """per knob: attack / decay / release factors over a step, sustain parts, release target (Q24). Roland's per-sample
    formulas on the 44.1 kHz words (their floors 7.927e-6 / 1.2e-4 / 1.43e-4 and the sustain / release target terms
    there, as at the JP-08's own rate), then x44^R per 96 kHz sample, x44^(96 R) per step: the same arithmetic as
    ENV KEY FOLLOW's (arm_env_coef) at f = 1, so that key follow off and on agree (until 2026-11-12 the floors were
    taken on the 96 kHz words: up to 4e-4 apart at the slowest settings)"""
    a, d, sus, r = (rows(ex, model, 'env%d_%s' % (n, x)) for x in 'adsr')
    A = [1.0 - (1.0 - min(max(x[0], 0.0), 1.0)) ** (1.0 / R) for x in a]
    dd = [1.0 - (1.0 - min(max(0.01 * x[0], 0.0), 1.0)) ** (1.0 / R) for x in d]
    rr = [1.0 - (1.0 - min(max(0.01 * x[0], 0.0), 1.0)) ** (1.0 / R) for x in r]
    ya = [max(x, 7.927418e-06) for x in A]
    yd = [min(x, 0.01 * (1 - 0.00011998415)) for x in dd]
    yr = [min(x, 0.01 * (1 - 0.00014305115)) for x in rr]
    att = [q((1.0 - y) ** (STEP * R)) for y in ya]
    dec = [q((1.0 - y) ** (STEP * R)) for y in yd]
    sus_d = [q(0.0002 - 2.0 * x if x < 1e-4 else 0.0) for x in dd]
    sus_s = [q(0.87 * sus[k][0]) for k in range(256)]
    rel = [q((1.0 - y) ** (STEP * R)) for y in yr]
    rtg = [q(0.02 - 0.0007080078 + (0.0002 - 2.0 * x if x < 1e-4 else 0.0)) for x in rr]
    att1 = [q((1.0 - y) ** R) for y in ya]       # per sample: the step the attack ends in
    dec1 = [q((1.0 - y) ** R) for y in yd]
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
    pwmw = [q(x[1] / 1.2) for x in rows(ex, model, 'pwm')]      # PWM (C054) / 1.2: the voice multiplies by 1.2
    porta = [q((1.0 - min(max(x[0], 0.0), 1.0)) ** (STEP * R)) for x in rows(ex, model, 'porta')]   # glide per step
    notelog = [q(math.log2(note_dt(n))) for n in range(128)]
    mant = [int(round((2.0 ** (i / 1024.0) - 1.0) * (1 << 23))) for i in range(1025)]
    lfod = [q(x[0]) for x in rows(ex, model, 'lfo_filter')]     # FLT LFO MOD (C064): octaves at LFO = 1
    rate = rows(ex, model, 'lfo_rate')
    inc16 = [int(round(min(math.floor(rate[k][0] * 65536.0) * 2.0 ** -32, 0.0071359314) * 2.0 ** 31 * SIXTEENTH))
             for k in range(256)]                               # phase span 2 = 2^32
    fade16 = [q((0.1 * x[0] + 0.9) ** SIXTEENTH) for x in rows(ex, model, 'lfo_delay')]
    envt = {0: env_tables(ex, model, 2), 1: env_tables(ex, model, 1)}   # engine i 0 = AMP = ENV2, 1 = FILTER = ENV1
    envr = {0: env_raw(ex, model, 2), 1: env_raw(ex, model, 1)}
    kff = [int(round((1.0 + 0.1 * (min(max((n - 12) / 12.0, 2.0), 7.0) - 7.0)) * 65536)) for n in range(128)]
    l2t = [q(math.log2(1.0 + i / 1024.0)) for i in range(1025)]
    o = ['/* bq_%s_arm.h: the %s voice\'s filter coefficient and VCA gain on the ARM (SYSTEM-S1CC-BOUTIQUE ARM_CUSTOM).'
         % (model, model.upper()),
         '   Written by sdk/s1cc/examples/boutique_ported/tools/make_arm.py from bq_%s_tables.h; do not edit. */' % model,
         '',
         '#ifdef ARM_CUSTOM_CC_TABLE',
         '#define ARM_CUSTOM_LPF_TYPE 1   /* model.c: the LPF TYPE button toggles 0x17E (FLT LPF SLOPE, README.md §13.7) */',
         '#define ARM_CUSTOM_LEGATO 1     /* model.c: the LEGATO button toggles 0x191 (PORTA SW, README.md §13.7) */',
         '#define ARM_CUSTOM_ASSIGN 1     /* model.c: the MONO button steps 0x192 = ASSIGN MODE 0..3 (README.md §13.7) */',
         '/* model.c (region 0): MIDI CC -> PLUGOUT_EXTEND word (README.md §13.5): CC 0..127 -> the JP-08 value (n positions:',
         '   value x n >> 7; 0: x 2, 127 -> 255), stored as (value - default) & 0xFF; steps 0 means 0..255 */',
         'static const struct { uint8_t cc; uint16_t id; uint8_t steps, max, def; } arm_custom_cc[] = {']
    for i, (nm, pid, cc, st, mx, df, ap) in enumerate(EXTEND):
        o.append('    { %d, 0x%03X, %d, %d, %d },   /* EXTEND%02d: %s (JP-08 0x%03X)%s */'
                 % (cc, EXT0 + i, st, mx, df, i + 1, nm, pid, '' if ap else ' stored only, not applied yet'))
    o += ['};',
          '#else',
          '#define ARM_CUSTOM_NK 19',
          '/* the knobs it reads: FILTER CUTOFF, RESONANCE, ENV AMOUNT, KEY TRACK; LFO TO PITCH = OSC LFO MOD, PITCH ENV',
          '   AMOUNT = OSC ENV MOD (JP-08: ENV1 -> pitch), OSC1 / OSC2 RANGE; EXTEND01 OSC FREQ MOD DST, EXTEND03 AMP LEVEL;',
          '   LFO TO AMP = AMP LFO MOD (4 positions: knob >> 6); OSC2 COARSE (signed semitones) (README.md §13.5) */',
          'static const uint16_t arm_custom_id[ARM_CUSTOM_NK] = { 0x17D, 0x17F, 0x181, 0x186, 0x163, 0x17A, 0x16A, 0x16F,',
          '                                                        0x%03X, 0x%03X, 0x165, 0x173, 0x%03X, 0x16D, 0x16E, 0x18B,'
          % (EXT0, EXT0 + 2, EXT0 + 1),
          '                                                        0x190, 0x191, 0x%03X };' % (EXT0 + 3),
          '/* 13 OSC2 COLOR = PWM (C054), 14 OSC2 COLOR MOD = PWM SOURCE: 0 MANUAL, 1 LFO, 2..4 ENV1, 5 MANUAL;',
          '   15 AMP TONE = ENV2 KEY FOLLOW (knob >> 6: 0 off, 1 ENV1, 2 ENV2, 3 both); 16 PORTAMENTO = PORTA TIME,',
          '   17 LEGATO (button word, model.c) = PORTA SW; 18 EXTEND04 BEND RANGE */',
          '/* their values before the first knob / patch arrives (the JP-08 defaults; EXTEND 0 = the JP-08 default) */',
          '#define ARM_CUSTOM_INIT { 128, 128, 128, 128, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }',
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
    o += arr('arm_pwm', pwmw) + arr('arm_porta', porta)
    o += arr('arm_notelog', notelog) + arr('arm_mant', mant)
    o += ['', '/* ARM_CUSTOM_ENV: the model\'s envelopes, [i][knob], i 0 = AMP (ENV2), 1 = FILTER (ENV1); per step of 1 ms */',
          '#define ARM_CUSTOM_ENV 1']
    for j, nm in enumerate(('att', 'dec', 'sus_s', 'sus_d', 'rel', 'rtg', 'att1', 'dec1')):
        o.append('static const int32_t arm_env_%s[2][256] = {' % nm)
        for i in (0, 1):
            vals = envt[i][j]
            o.append('    {' + ', '.join('%d' % x for x in vals) + '},')
        o.append('};')
    o += ['/* ENV KEY FOLLOW: the 44.1 kHz words [i][A, 0.01 D, 0.01 R][knob] (float bits), f per note (Q16), log2(1 + i / 1024) */',
          'static const uint32_t arm_kf_raw[2][3][256] = {']
    for i in (0, 1):
        o.append('    {' + ', '.join('{' + ', '.join('0x%08x' % x for x in envr[i][j]) + '}' for j in range(3)) + '},')
    o.append('};')
    o += arr('arm_kf_f', kff) + arr('arm_l2t', l2t)
    Q48 = float(1 << 48)
    o += ['#define ARM_KF_AMIN ((int64_t)%d)   /* 7.927418e-6, Q48 */' % round(7.927418e-06 * Q48),
          '#define ARM_KF_DMAX ((int64_t)%d)   /* 0.01 (1 - 1.2e-4), Q48 */' % round(0.01 * (1 - 0.00011998415) * Q48),
          '#define ARM_KF_RMAX ((int64_t)%d)   /* 0.01 (1 - 1.43e-4), Q48 */' % round(0.01 * (1 - 0.00014305115) * Q48),
          '#define ARM_KF_RTG0 %d   /* 0.02 - 0.0007080078, Q24 */' % q(0.02 - 0.0007080078),
          '#define ARM_KF_1E4 ((int64_t)%d)   /* 1e-4, Q48 */' % round(1e-4 * Q48),
          '#define ARM_KF_STEP ((int64_t)%d)   /* 96 R = 44.1, Q20 */' % round(STEP * R * (1 << 20)),
          '#define ARM_KF_R ((int64_t)%d)   /* R = 0.459375, Q20 */' % round(R * (1 << 20))]
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
/* ENV KEY FOLLOW (the JP-08's ENV2 KEY FOLLOW, here the AMP TONE knob >> 6: bit 0 ENV1, bit 1 ENV2; lifted
   instructions 116..172: A, D, R words x f, f = 1 + 0.1 (clamp(pitch, 2, 7) - 7), before Roland's per-sample
   formulas; the JP-08's ARM writes A085 / A087 = the two bits, tools/bq_fw.py): the coefficients of an envelope for its
   note, from the 44.1 kHz words (arm_kf_raw, float bits) x f, then as 96 kHz: x44^(R) per sample, x44^(96 R) per step,
   with ln / exp in 64-bit fixed point. Computed again only when the note or a knob changes */
static uint8_t arm_kf_k;                               /* AMP TONE >> 6 (arm_custom_voice) */
static int64_t arm_q48(uint32_t b)                     /* float bits (0 <= x <= 1) -> Q48 */
{
    int e = (int)((b >> 23) & 0xFF) - 127 - 23 + 48;
    int64_t m = (int64_t)((b & 0x7FFFFFu) | 0x800000u);
    if ((b & 0x7FFFFFFFu) == 0)
        return 0;
    return e >= 0 ? m << e : (e > -40 ? m >> -e : 0);
}
static int64_t arm_ln1m(int64_t y)                     /* ln(1 - y), y Q48 in [0, 1] -> Q48 (<= 0) */
{
    if (y < ((int64_t)1 << 42)) {                      /* y < 1/64: -(y + y^2/2 + y^3/3 + y^4/4) */
        int64_t y2 = ((y >> 16) * (y >> 16)) >> 16, y3 = ((y2 >> 16) * (y >> 16)) >> 16;
        int64_t y4 = ((y2 >> 16) * (y2 >> 16)) >> 16;
        return -(y + (y2 >> 1) + ((y3 * 21845) >> 16) + (y4 >> 2));   /* no 64-bit division: no libgcc */
    }
    uint32_t v = (uint32_t)((((int64_t)1 << 48) - y) >> 24);   /* 1 - y, Q24 */
    if (v == 0)
        return -((int64_t)40 << 48);
    int n = __builtin_clz(v) - 7;                      /* v << n in [2^24, 2^25) */
    uint32_t m = (v << n) - (1u << 24), i = m >> 14, r = m & 0x3FFFu;
    int32_t l2 = arm_l2t[i] + (int32_t)(((int64_t)(arm_l2t[i + 1] - arm_l2t[i]) * r) >> 14) - (n << 24);   /* log2, Q24 */
    return ((int64_t)l2 * 11629080) ;                  /* x ln 2 (Q24) -> Q48 */
}
static int32_t arm_expq(int64_t e)                     /* e^e, e Q48 <= 0 -> Q24 */
{
    if (e > -((int64_t)1 << 42)) {                     /* |e| < 1/64: 1 + e + e^2/2 + e^3/6 */
        int64_t a = -e, a2 = ((a >> 16) * (a >> 16)) >> 16, a3 = ((a2 >> 16) * (a >> 16)) >> 16;
        return (int32_t)((((int64_t)1 << 48) - a + (a2 >> 1) - ((a3 * 10923) >> 16)) >> 24);
    }
    int64_t x = ((e >> 24) * 24204406) >> 24;           /* x 1 / ln 2 (Q24): log2, Q24 */
    if (x < -((int64_t)30 << 24))
        return 0;
    int32_t xi = (int32_t)x, n = xi >> 24;             /* floor */
    uint32_t f = (uint32_t)xi & 0xFFFFFFu, i = f >> 14, r = f & 0x3FFFu;
    uint32_t mm = (uint32_t)arm_mant[i] + (uint32_t)(((uint64_t)(uint32_t)(arm_mant[i + 1] - arm_mant[i]) * r) >> 14);
    uint32_t q = (1u << 24) + (mm << 1);               /* 2^frac, Q24 */
    return (int32_t)(q >> -n);
}
static int32_t arm_kfc[2][NV][8];                      /* att, dec, sus_s, sus_d, rel, rtg, att1, dec1 (as the tables) */
static uint32_t arm_kfkey[2][NV];
static const int32_t *arm_env_coef(int i, int v, const uint8_t *k, int32_t *c)
{
    if (!((arm_kf_k >> (i == 1 ? 0 : 1)) & 1)) {      /* no key follow: the tables */
        c[0] = arm_env_att[i][k[1]]; c[1] = arm_env_dec[i][k[2]]; c[2] = arm_env_sus_s[i][k[0]];
        c[3] = arm_env_sus_d[i][k[2]]; c[4] = arm_env_rel[i][k[3]]; c[5] = arm_env_rtg[i][k[3]];
        c[6] = arm_env_att1[i][k[1]]; c[7] = arm_env_dec1[i][k[2]];
        return c;
    }
    int note = cust_note[v] & 127;
    uint32_t key = 0x80000000u | (uint32_t)k[1] | (uint32_t)k[2] << 8 | (uint32_t)k[3] << 16 | (uint32_t)note << 24;
    int32_t *kc = arm_kfc[i][v];
    if (arm_kfkey[i][v] != key) {
        arm_kfkey[i][v] = key;
        int64_t f = arm_kf_f[note];                    /* Q16 */
        int64_t ya = (arm_q48(arm_kf_raw[i][0][k[1]]) * f) >> 16, yd = (arm_q48(arm_kf_raw[i][1][k[2]]) * f) >> 16;
        int64_t yr = (arm_q48(arm_kf_raw[i][2][k[3]]) * f) >> 16;
        int64_t yam = ya < ARM_KF_AMIN ? ARM_KF_AMIN : ya;          /* max(A f, 7.927e-6) */
        int64_t ydm = yd > ARM_KF_DMAX ? ARM_KF_DMAX : yd;          /* 0.01 min(f D, 1 - 1.2e-4) */
        int64_t yrm = yr > ARM_KF_RMAX ? ARM_KF_RMAX : yr;          /* 0.01 min(f R, 1 - 1.43e-4) */
        int64_t la = arm_ln1m(yam), ld = arm_ln1m(ydm), lr = arm_ln1m(yrm);
        kc[0] = arm_expq((la >> 20) * ARM_KF_STEP);    /* x 96 R = 44.1 (Q20): one step */
        kc[6] = arm_expq((la >> 20) * ARM_KF_R);       /* x R (Q20): one sample */
        kc[1] = arm_expq((ld >> 20) * ARM_KF_STEP);
        kc[7] = arm_expq((ld >> 20) * ARM_KF_R);
        kc[4] = arm_expq((lr >> 20) * ARM_KF_STEP);
        /* sus_d = 0.02 max(t133 + 0.01, 1) - 0.02, t133 = max(1 - f D, 1.2e-4): 0.0002 - 2 f (0.01 D) below 1e-4 */
        kc[3] = yd < ARM_KF_1E4 ? (int32_t)((2 * ARM_KF_1E4 - 2 * yd) >> 24) : 0;
        kc[5] = ARM_KF_RTG0 + (yr < ARM_KF_1E4 ? (int32_t)((2 * ARM_KF_1E4 - 2 * yr) >> 24) : 0);
    }
    kc[2] = arm_env_sus_s[i][k[0]];
    return kc;
}

/* one envelope of voice v over n steps of 1 ms: st = ST_ATT / ST_DEC / ST_REL, *l the level (Q24); k = S, A, D, R knobs */
static int arm_custom_env(int i, int v, int st, int32_t *l, const uint8_t *k, int n)
{
    int32_t cb[8];
    const int32_t *c = arm_env_coef(i, v, k, cb);
    int32_t x = *l;
    for (int j = 0; j < n; j++) {
        if (st == ST_ATT) {
            int32_t y = ARM_ATT_TOP - qmul(ARM_ATT_TOP - x, c[0]);
            if (y < ARM_ATT_END) {
                x = y;
                continue;
            }
            /* the attack ends in this step: sample by sample (as Roland), the rest of the step in decay */
            int32_t s = c[2] + c[3];
            for (int m = 0; m < ARM_STEP; m++) {
                if (st == ST_ATT) {
                    x = ARM_ATT_TOP - qmul(ARM_ATT_TOP - x, c[6]);
                    if (x >= ARM_ATT_END)
                        st = ST_DEC;
                } else {
                    x = s + qmul(x - s, c[7]);
                }
            }
        } else if (st == ST_DEC) {
            int32_t s = c[2] + c[3];
            x = s + qmul(x - s, c[1]);
        } else if (st == ST_REL) {
            int32_t t = c[5];
            x = t + qmul(x - t, c[4]);
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
   no SYSTEM-1 LFO -> PITCH or pitch envelope (engine.c writes their words 0). Writes the voice's note words: s1_dt /
   s1_rdt = OSC2's step (note, OSC2 RANGE + COARSE, its modulation), s1_pitch = 4.75 + o1 - o2 (octaves): OSC1 adds
   it to its CROSS MOD exponent, so the DSP needs no exp of its own for it (4.75: s1cc's profile gives the note's
   pitch there, A 440 = 4.75 = the same step) */
/* PORTA (lifted instructions 81..98: per voice d += C03A (pitch - d) per 44.1 kHz sample with PORTA SW (A082) on, every
   note, not only legato; KEY TRACK and the envelopes' key follow keep the raw note): the glided pitch, as log2 of the
   step (arm_notelog's unit), moved toward the note every 1 ms by (1 - C03A)^(96 R) (arm_porta) */
static int32_t arm_gl[NV];
static uint8_t arm_gl_on[NV];
static void arm_custom_glide(int v, int note, const uint8_t *k)
{
    int32_t t = arm_notelog[note & 127];
    if (!arm_gl_on[v] || !(k[17] & 1))
        arm_gl[v] = t;
    else
        arm_gl[v] = t + qmul(arm_gl[v] - t, arm_porta[k[16]]);
    arm_gl_on[v] = 1;
}

static void arm_custom_pitch(int v, int32_t e1, int32_t lf, int note, const uint8_t *k)
{
    if (!arm_gl_on[v] || !(k[17] & 1))                /* no glide: the note at once (also at a note on) */
        arm_gl[v] = arm_notelog[note & 127];
    arm_gl_on[v] = 1;
    /* PITCH BEND (engine.c cust_bend, -8192..8191) x BEND RANGE semitones (EXTEND04), on both oscillators */
    int32_t gl = arm_gl[v] + (int32_t)(((int64_t)cust_bend * ARM_EXT(k[18], 2) * 1398101) >> 13);
    int32_t e1o = qmul(qmul(e1, e1), 16776407);          /* ENV1 out = env1^2 x 0.99995 */
    /* ENV1 POLARITY = the sign of FILTER ENV AMOUNT (knob < 128: inverted; README.md §13.7): Roland inverts ENV1 out
       for every destination (lifted instruction 157: x (1 - 2 A084), A084 = 1 / 0 for polarity 0 / 1) */
    if (k[2] < 128)
        e1o = -e1o;
    int32_t pm = qmul(lf, arm_lfop[k[4]]) + qmul(e1o, arm_envp[k[5]]);
    int dst = ARM_EXT(k[8], 0);
    /* OSC RANGE 64' .. 2' (0..5, the JP-08's OSC1 RANGE positions: factor 2^(k - 3), PX-11 0x60054dfc table 0x48) at
       SYSTEM-S1CC's octaves k - 1; OSC2 RANGE + COARSE: the JP-08's OSC2 RANGE in semitones (table 0x4a: -36..+24) */
    int r1 = k[6] > 5 ? 5 : k[6], r2 = k[7] > 5 ? 5 : k[7], c2 = (int8_t)k[11];
    int32_t o1 = ((int32_t)(r1 - 1) << 24) + (dst == 0 ? 0 : pm);
    int32_t o2 = gl + ((int32_t)(r2 - 1) << 24) + c2 * 1398101 + (dst == 2 ? 0 : pm);   /* 2^24 / 12 */
    b5(B5_DT + v, arm_exp2_bits(o2));
    b5(B5_RDT + v, arm_exp2_bits(-o2 - (14 << 24)));      /* 1 / dt / 16384 */
    b5(B5_PITCH + v, q24_bits(gl + o1 - o2 + 79691776));   /* 4.75 + o1 - o2 */
}
#define ARM_CUSTOM_PITCH 1

/* per voice, every step (engine.c tick, after the envelopes and the LFO): e0 / e1 the AMP / FILTER envelope, lf the
   LFO x fade (Q24), note the voice's note, k[] the knobs of arm_custom_id, lfo_k LFO TO FILTER */
static void arm_custom_voice(int v, int32_t e0, int32_t e1, int32_t lf, int note, const uint8_t *k, int lfo_k)
{
    arm_kf_k = k[15] >> 6;                             /* ENV KEY FOLLOW (AMP TONE): the envelopes read it */
    arm_custom_glide(v, note, k);
    arm_custom_pitch(v, e1, lf, note, k);
    /* FLT ENV MOD SRC (EXTEND02, default 1): the cutoff's envelope (lifted instruction 552: (A095 x ENV2 out + A094 x
       ENV1 out) x FLT ENV MOD; the JP-08's ARM writes A094 / A095 = 1 / 0 for value 1, 0 / 1 for 0: tools/bq_fw.py) */
    int32_t e1s = ARM_EXT(k[12], 1) ? qmul(e1, e1) : qmul(e0, e0);
    /* arm_envmod is negative below 128 (FILTER ENV AMOUNT's sign = ENV1 POLARITY): ENV2 has no polarity, so with ENV2
       as the source the sign goes back */
    if (!ARM_EXT(k[12], 1) && k[2] < 128)
        e1s = -e1s;
    int32_t u = arm_base[k[0]] + arm_rterm[k[1]] + qmul(arm_envmod[k[2]], qmul(e1s, 16776407))   /* x 0.99995 */
              + qmul(arm_kf[k[3]], arm_pitch[note & 127]) + qmul(lf, arm_lfod[lfo_k]);
    int32_t g = arm_lerp(arm_g, ARM_NU, u - ARM_U0, ARM_USTEP_SH);
    /* the VCA (lifted instructions 657..667): x = env2^2 x 0.99995 - (0.5 - 0.5 lfo) x AMP LFO MOD, Roland's curve, x AMP
       LEVEL */
    int32_t x = qmul(qmul(e0, e0), 16776407) - qmul(Q1 / 2 - lf / 2, arm_alfo[k[10] >> 6]);
    /* PWM (lifted instruction 218: (A090 + A091 (0.5 + 0.5 lfo) + A092 ENV1 out) x C054; the JP-08's ARM writes A092 /
       A090 / A091 = 1 for PWM SOURCE 0 ENV1 / 1 MANUAL / 2 LFO, tools/bq_fw.py) -> s1_arm(0), over 1.2 (the voice's
       x 1.2). ENV1 out with its polarity (FILTER ENV AMOUNT's sign) */
    int32_t src;
    if (k[14] == 1)
        src = Q1 / 2 + lf / 2;                         /* LFO */
    else if (k[14] >= 2 && k[14] <= 4) {
        src = qmul(qmul(e1, e1), 16776407);            /* ENV1 */
        if (k[2] < 128)
            src = -src;
    } else
        src = Q1;                                      /* MANUAL */
    ramp(P_ARM0 + v, qmul(src, arm_pwm[k[13]]));
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
