"""Parameter model of the from-scratch engine: what the engine writes to the
slave DSPs for every parameter value. Our own description, written from
measurements of the JX-3P's behaviour (re/measure.py, deps.py, probe.py,
sweepfit.py, knotfit.py): formulas where one fits exactly, otherwise
least-squares knots (marked with their error). Nothing here is copied code;
the numbers are the measured response.

    out(source, targets, expr, when=None)

source  parameter id whose change triggers the write (vt+0x30), None = start-up only
targets 'P<dsp>:<id>[/r<ramp>f<flag>]' parameter block (+0xA0 on change, +0xA4 at
        start-up), 'B<dsp>:<id>' bank 5 (+0xB0); several separated by spaces
expr    expression over the current parameter values (expr.py); P(id), SLAVE
when    optional condition: the write happens only if it is not 0

Several sources may write the same DSP id; the last write wins, as on the
JX-3P. At start-up / full refresh the sources are evaluated in REFRESH_ORDER.
"""
from expr import (P, SLAVE, E, lift, clamp, emin, emax, sel, exp2, eabs, floor, pick, knots)

OUT = []


def out(src, targets, expr, when=None):
    OUT.append((src, targets.split(), lift(expr), None if when is None else lift(when)))


def eq(x, v):
    return E('ge', lift(x), lift(v - 0.5)) * E('lt', lift(x), lift(v + 0.5))


def interp(x, ys, xs=None):
    """piecewise linear through (xs[i], ys[i]), constant outside"""
    xs = XS if xs is None else xs
    return E('interp', lift(x), [float(a) for a in xs], [float(b) for b in ys])


def bip(x, k):
    """-k at 0, 0 at 128, +k at 255 (two slopes)"""
    d = lift(x) - 128
    return sel(lift(x) < 128, d * (k / 128), d * (k / 127))


def per_slave(a, b):
    return pick(SLAVE, [a, b])


def toward(p, q):
    """panel offset q (0..100 %) pulls the patch value p towards 255, in integers"""
    return floor(lift(p) + (255 - lift(p)) * emin(q, 100) / 100)


XS = [1, 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32, 40, 48, 64, 80, 96, 128, 160, 192, 224, 255]
XS_VEL = [1, 2, 3, 4, 6, 8, 12, 16, 20, 24, 28, 32, 40, 48, 56, 64, 72, 80, 88, 96, 100, 104, 112, 120, 127]
XS_MIX = [128, 136, 144, 152, 160, 176, 192, 208, 224, 240, 255]
XS_14 = [1, 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32, 40, 48, 64, 80, 96, 100, 128, 255]

# knots (log2 values unless noted), fitted with re/knotfit.py
K = {
    'lfo_delay': [-11.73404, -11.77133, -11.80862, -11.8459, -11.88318, -11.92045, -11.99499, -12.0695, -12.144, -12.29293, -12.44174, -12.59048, -12.88758, -13.18402, -13.47993, -14.06903, -14.65183, -15.23048, -16.34654, -17.369, -18.22665, -18.84893, -19.22166],  # max rel err 1.37 %
    'hpf': [-11.69671, -10.86148, -10.33514, -9.949736, -9.645406, -9.389578, -8.98604, -8.675128, -8.411359, -7.998594, -7.681858, -7.411923, -6.990698, -6.667224, -6.390799, -5.958543, -5.625255, -5.339217, -4.890309, -4.539519, -4.245729, -3.993801, -3.778918],  # 0.62 %
    'env_attack': [-3.673766, -3.709678, -3.745586, -3.78149, -3.81739, -3.853286, -3.925065, -3.996824, -4.068568, -4.211992, -4.355319, -4.498574, -4.784732, -5.070312, -5.355405, -5.923194, -6.485492, -7.044174, -8.126466, -9.130153, -9.994698, -10.65208, -11.07006],  # 1.29 %
    'env_decay': [-3.321928, -3.643856, -4.058894, -4.321928, -4.580627, -4.609363, -4.666827, -4.724281, -4.781727, -4.896585, -5.011391, -5.12616, -5.355523, -5.584604, -5.813463, -6.270086, -6.724466, -7.177346, -8.070492, -8.937655, -9.759978, -10.50834, -11.12759],  # 0.68 %
    'level': [-7.906653, -6.900125, -6.308624, -5.887039, -5.558553, -5.284352, -4.854008, -4.523013, -4.241832, -3.799302, -3.456042, -3.160774, -2.690968, -2.319849, -1.995894, -1.467608, -1.035993, -0.6498936, 0.00771432, 0.5780033, 1.101445, 1.59296, 2.049011],  # 0.64 %
    'fx_level': [0.05735281, 0.1080076, 0.1533664, 0.1944327, 0.2319491, 0.2668099, 0.3288087, 0.3825886, 0.4313577, 0.5142574, 0.5829428, 0.6438113, 0.7435617, 0.8231193, 0.8933867, 1.001892, 0.9995074, 1.00009, 0.999976, 1.000006, 0.9999983, 1, 0.9999998],  # linear, 0.26 % of range
    'fx_amount': [-9.954506, -8.947977, -8.356477, -7.934891, -7.606405, -7.332204, -6.901861, -6.570866, -6.289685, -5.847155, -5.503895, -5.208626, -4.738821, -4.367701, -4.043746, -3.51546, -3.083846, -2.697746, -2.040138, -1.469849, -0.9464073, -0.4548924, 0.001159127],  # 0.64 %
    'porta_a': [-6.416884, -7.1307, -7.613044, -7.979336, -8.275672, -8.529066, -8.937621, -9.260386, -9.538776, -9.986045, -10.34065, -10.64967, -11.15125, -11.55631, -11.91474, -12.51232, -13.01387, -13.47033, -14.27759, -15.01647, -15.736, -16.45934, -17.18159],  # 0.59 %
    'porta_b': [-6.455024, -7.343334, -7.925991, -8.367067, -8.725623, -9.034268, -9.538231, -9.942967, -10.296, -10.87169, -11.33385, -11.73858, -12.39492, -12.91863, -13.37566, -14.11025, -14.69086, -15.1953, -15.99852, -16.63343, -17.16803, -17.62826, -18.02182],  # 1.02 %
    'mix_hi': [0.4981297, 0.5380427, 0.6035624, 0.6921486, 0.7962176, 1.056097, 1.360799, 1.683553, 2.001894, 2.292055, 2.516252],  # linear, XS_MIX, 0.17 %
    'vel': [-8.973446, -8.013495, -7.467719, -7.08144, -6.568623, -6.217786, -5.750829, -5.429054, -5.154726, -4.896619, -4.634842, -4.365518, -3.792991, -3.203048, -2.628546, -2.08601, -1.581834, -1.116261, -0.6865319, -0.2903114, -0.1046925, 0.07746616, 0.4201576, 0.7389011, 1.000871],  # XS_VEL, 1.3 %
}


def curve(name, x, v0):
    """knotted log curve with its own value at 0"""
    return sel(lift(x) >= 1, exp2(interp(x, K[name])), v0)


def lfo_rate(x):
    """25 steps, three per octave (x1, x4/3, x3/2)"""
    i = floor((lift(x) * 4 + 4) / 41)
    o = floor(i / 3)
    return (32 / 1125) * exp2(o) * pick(i - o * 3, [1, 4 / 3, 1.5])


def ramp(t, r=3, f=0):
    return ' '.join(f'{x}/r{r}f{f}' for x in t.split())

# ---- LFO -------------------------------------------------------------------
LFO_RATE = 'B0:0x28 B0:0x3a B1:0x28 B1:0x3a'
out(0x6cd, ramp('P0:0x2c P0:0x43 P1:0x2c P1:0x43'), pick(P(0x6cd), [1, 0, 0, 0, 0, 0]))
out(0x6cd, ramp('P0:0x2d P0:0x44 P1:0x2d P1:0x44'), pick(P(0x6cd), [0, 1, 0, 0, 0, 0]))
out(0x6cd, ramp('P0:0x2e P0:0x45 P1:0x2e P1:0x45'), pick(P(0x6cd), [0, 0, 1, 1, 1, 1]))
out(0x6cd, 'B0:0x2c B0:0x3e B1:0x2c B1:0x3e', pick(P(0x6cd), [1, 1, 2, 2, 2, 2]))
out(0x6ce, ramp('P0:0x27 P0:0x3e P1:0x27 P1:0x3e'), sel(P(0x6ce) >= 1, 1, 0))
out(0x6ce, 'B0:0x2d B0:0x3f B1:0x2d B1:0x3f', curve('lfo_delay', P(0x6ce), 2))
out(0x6cf, ramp('P0:0x26 P0:0x3d P1:0x26 P1:0x3d', 0), P(0x6cf) / 255)
out(0x6cf, LFO_RATE, lfo_rate(P(0x6cf)) * emax(1, P(0x3ef) / 100))
out(0x3ef, LFO_RATE, lfo_rate(P(0x6cf)) * emax(1, P(0x3ef) / 100))
out(0x74d, ramp('P0:0x26 P0:0x3d P1:0x26 P1:0x3d', 0), 0)
out(0x6d3, 'B0:0x2a B0:0x3c B1:0x2a B1:0x3c', sel(P(0x6d3) >= 1, 0, 1))
out(0x73e, 'B0:0x30 B0:0x31 B0:0x42 B0:0x43 B1:0x30 B1:0x31 B1:0x42 B1:0x43', sel(P(0x73e) >= 1, 1, 0))
out(0x14, ramp('P0:0x20 P0:0x37 P1:0x20 P1:0x37'),
    sel(P(0x14) >= 1, interp(P(0x14), [0.0003279336, 0.0006556209, 0.0009832337, 0.001310944, 0.00163858,
                                       0.001966016, 0.002620712, 0.003275144, 0.003929308, 0.005236812,
                                       0.006542781, 0.007848534, 0.01045364, 0.01306076, 0.01563744,
                                       0.02088427, 0.02581603, 0.03138, 0.03242145, 0.03242145, 0.03242145],
                        XS_14), 0))

# ---- oscillators ----------------------------------------------------------------
OSC = 'P2:{a} P2:{b} P3:{a} P3:{b}'
def osc(a):
    return ramp(OSC.format(a=f'{a:#x}', b=f'{a + 0x16:#x}'))
out(0x6d0, osc(0x26), bip(P(0x6d0), 0.175))
out(0x6d4, 'B2:0x2a B2:0x38 B3:0x2a B3:0x38', sel(P(0x6d4) >= 2, 1, 0))     # PULSE flag (SAW program)
out(0x6d9, 'B2:0x2b B2:0x39 B3:0x2b B3:0x39', sel(P(0x6d9) >= 2, 1, 0))
out(0x6d4, osc(0x32), 1)                                                     # levels after the swap
out(0x6d9, osc(0x33), 1)
out(0x6d6, osc(0x28), pick(P(0x6d6), [0, 1, 0, 1, 1, 1]))
out(0x6d6, osc(0x2a), pick(P(0x6d6), [0, 0, 1, 1, 1, 1]))
out(0x6db, osc(0x29), pick(P(0x6db), [0, 1, 0, 1, 1, 1]))
out(0x6db, osc(0x2b), pick(P(0x6db), [0, 0, 1, 1, 1, 1]))
out(0x6d7, 'B2:0x26 B2:0x34 B3:0x26 B3:0x34', pick(P(0x6d7), [-3, -2, -1, 0, 1, 2]))
out(0x6dc, 'B2:0x27 B2:0x35 B3:0x27 B3:0x35', pick(P(0x6dc), [-3, -2, -1, 0, 1, 2]))
out(0x6dd, osc(0x21), bip(P(0x6dd), 1 / 24))
out(0x6df, 'B2:0x22 B2:0x28 B2:0x30 B2:0x36 B3:0x22 B3:0x28 B3:0x30 B3:0x36', pick(P(0x6df), [0, 1, 1]))
out(0x6df, 'B2:0x29 B2:0x37 B3:0x29 B3:0x37', pick(P(0x6df), [0, 0, 1]))
out(0x6e0, osc(0x20), clamp(P(0x6e0) / 127, 0, 1))
for i, a in ((0x6e1, 0x34), (0x6e2, 0x35)):
    out(i, osc(a), sel(P(i) < 128, P(i) / 256, interp(P(i), K['mix_hi'], XS_MIX)))
out(0x6e7, osc(0x27), eabs(bip(P(0x6e7), 1.967)))
out(0x6e7, 'B2:0x2d B2:0x3b B3:0x2d B3:0x3b', sel(P(0x6e7) >= 128, 0, 1))

# ---- filter / amplifier (DSP4 voices 0/1, DSP5 voices 2/3 at +0x80) -------------------
def vcf(a, r=3):
    return ramp(f'P4:{a:#x} P4:{a + 0x12:#x} P5:{a + 0x80:#x} P5:{a + 0x92:#x}', r)
out(0x6ea, vcf(0x20, 12), P(0x6ea) / 255)
out(0x2db, vcf(0x20, 0), P(0x2db) / 100)
out(0x7e4, vcf(0x20, 0), 0)
out(0x6ec, vcf(0x21), P(0x6ec) / 255)
out(0x2dd, vcf(0x21), toward(P(0x6ec), P(0x2dd)) / 255)
out(0x6ed, ramp('P4:0x48'), curve('hpf', P(0x6ed), 6.54e-05))
out(0x2dc, ramp('P4:0x48'), curve('hpf', toward(P(0x6ed), P(0x2dc)), 6.54e-05))
out(0x6ee, vcf(0x27), eabs(bip(P(0x6ee), 13.5)))
out(0x6ee, 'B4:0x26 B4:0x35 B5:0x25 B5:0x34', sel(P(0x6ee) >= 128, 0, 1))
out(0x6d1, vcf(0x26), bip(P(0x6d1), 2.2))
out(0x6f3, vcf(0x28), P(0x6f3) / 100 - 1.28)
out(0x7e3, vcf(0x29), P(0x7e3) / 255)
out(0x801, vcf(0x2f), P(0x801) / 255)
out(0x6f8, vcf(0x2e), 0)
out(0x6f8, vcf(0x2d), bip(P(0x6f8), 0.5))
out(0x709, ramp('P4:0x4b'), curve('level', P(0x709), 0))
for m, t in enumerate(('B4:0x2b B4:0x3a B5:0x2a B5:0x39', 'B4:0x2c B4:0x3b B5:0x2b B5:0x3a',
                       'B4:0x2d B4:0x3c B5:0x2c B5:0x3b')):
    out(0x736, t, sel(eq(P(0x736), m) + eq(P(0x736), 3) * (m == 0), 1, 0))     # 3 behaves as 0

# ---- envelopes: ENV-1 ids 0x2f..0x32, ENV-2 0x33..0x36, voice k + 0x17 -----------------
# circuit mod (0x744) scales the times of each voice by max(lo, 1 - a * cm), a per voice
ENV_A = {0: [(0.003015075, 0.25), (0.001974981, 0), (1.168625e-05, 0), (0.002454118, 0)],
         1: [(0.00169451, 0), (0.002828078, 0), (0.002267137, 0), (0.0005609412, 0)]}
GATE = [1, 0.7, 1, 0.7]                       # ENV-2 when VCA Mode = GATE


def cm_scale(v):
    a0, lo0 = ENV_A[0][v]
    a1, lo1 = ENV_A[1][v]
    return emax(per_slave(lo0, lo1), 1 - per_slave(a0, a1) * P(0x744))


def env_value(env, stage, v):
    pid = (0x6ef if env == 1 else 0x6f4) + stage
    x = P(pid)
    if stage == 2:
        val = x / 255
    elif stage == 0:
        val = curve('env_attack', x, 1)
    else:
        val = curve('env_decay', x, 0.7)
    if env == 2:
        val = sel(eq(P(0x736), 2), GATE[stage], val)
    return val if stage == 2 else val * cm_scale(v)


for env in (1, 2):
    for stage in range(4):
        pid = (0x6ef if env == 1 else 0x6f4) + stage
        for v in range(4):
            u, k = v >> 1, v & 1
            t = f'P{u}:{(0x2f if env == 1 else 0x33) + stage + 0x17 * k:#x}'
            e = env_value(env, stage, v)
            out(pid, ramp(t), e)
            if stage != 2:
                out(0x744, ramp(t), e)
            if env == 2:
                out(0x736, ramp(t), e)

# ---- portamento ------------------------------------------------------------------------
PORTA_ON = 'B0:0x23 B0:0x35 B1:0x23 B1:0x35'
PORTA_LEG = 'B0:0x25 B0:0x37 B1:0x25 B1:0x37'
porta_on = sel(P(0x6fd) >= 1, 1, 0)
porta_leg = eq(P(0x6fe), 1) * eq(P(0x6ff), 1)
for i in (0x6fd, 0x2df):
    t = P(0x6fd) if i == 0x6fd else toward(P(0x6fd), P(0x2df))
    out(i, ramp('P0:0x22 P0:0x39 P1:0x22 P1:0x39'), curve('porta_a', t, 0.0315643))
    out(i, ramp('P0:0x23 P0:0x3a P1:0x23 P1:0x3a'), curve('porta_b', t, 0.0520833))
    out(i, PORTA_ON, sel(t >= 1, 1, 0), when=1 - eq(P(0x6fe), 1) * eq(P(0x6ff), 0))
for i in (0x6fe, 0x6ff):
    out(i, PORTA_ON, porta_on)
    out(i, PORTA_LEG, porta_leg)

# ---- detune: circuit mod spread and UNISON offsets per voice ------------------------------
# value = base + d * cm + (Key Assign = UNISON) * u
DETUNE = {  # target: (base, d slave 0, u slave 0, d slave 1, u slave 1)
    'B2:0x24': (0, 0, 0, 7.006536e-05, 0.006666667),
    'B2:0x25': (8.333333e-05, 1.751634e-06, 0, 7.181699e-05, 0.006666667),
    'B2:0x32': (0, 3.608366e-05, -0.001733333, -2.627451e-05, -0.005),
    'B2:0x33': (8.333333e-05, 3.783529e-05, -0.001733333, -2.452288e-05, -0.005),
    'B3:0x24': (0, -1.821699e-05, 0.003433333, -7.821046e-05, 0.003333333),
    'B3:0x25': (8.333333e-05, -1.646536e-05, 0.003433333, -7.645883e-05, 0.003333333),
    'B3:0x32': (0, 3.503268e-05, -0.007441667, -5.254902e-05, -0.0025),
    'B3:0x33': (8.333333e-05, 3.678431e-05, -0.007441667, -5.079739e-05, -0.0025),
}
for t, (b, d0, u0, d1, u1) in DETUNE.items():
    e = b + per_slave(d0, d1) * P(0x744) + eq(P(0x6ff), 2) * per_slave(u0, u1)
    for i in (0x744, 0x6ff, 0x737):
        out(i, t, e)

# ---- 0x737 (patch, not on the panel) with circuit mod: filter spread, levels, tuning drift ------
def p3(x, n):
    y = (lift(x) + 1) / 129
    r = y
    for _ in range(n - 1):
        r = r * y
    return r
T737 = {  # target: (slave 0, slave 1); ('pow', n, k) = k * ((v + 1) / 129)^n, ('drift', a) = 1 + a * (v + 1)
    'B4:0x2f B4:0x3e B5:0x2e B5:0x3d': (('pow', 4, 0.25), ('pow', 4, 0.25)),
    'B4:0x28 B4:0x37 B5:0x27 B5:0x36': (('pow', 3, 0), ('pow', 3, 0)),
    'P0:0x24 P0:0x25': (('drift', 0), ('drift', 0.0006201267)),
    'P0:0x3b P0:0x3c': (('drift', 0.0001550913), ('drift', 0.0002325773)),
    'P1:0x24 P1:0x25': (('drift', 0.0004651546), ('drift', 0.0006201267)),
    'P1:0x3b P1:0x3c': (('drift', 0.0003100634), ('drift', 0.0002480745)),
    'P4:0x2b': (('pow', 3, 0), ('pow', 3, 0.01666667)),
    'P4:0x31': (('pow', 3, 0), ('pow', 3, 0.02)),
    'P4:0x3d': (('pow', 3, 0.04166667), ('pow', 3, 0.025)),
    'P4:0x43': (('pow', 3, 0.02), ('pow', 3, 0)),
    'P5:0xab': (('pow', 3, 0.0227), ('pow', 3, 0.004166667)),
    'P5:0xb1': (('pow', 3, 0.02), ('pow', 3, 0.02)),
    'P5:0xbd': (('pow', 3, 0.006508333), ('pow', 3, 0.0125)),
    'P5:0xc3': (('pow', 3, 0.01), ('pow', 3, 0.01)),
    'P4:0x2c P4:0x3e P5:0xac P5:0xbe': (('pow', 3, 0), ('pow', 3, 0)),
}
SPREAD = {  # circuit mod adds c * cm to these (slave 0, slave 1)
    'P4:0x2b': (0, 0.0003503268), 'P4:0x3d': (0.000875817, 0.0005254902),
    'P5:0xab': (0.0004771451, 8.758171e-05), 'P5:0xbd': (0.0001368026, 0.0002627451),
    'P4:0x2c P4:0x3e P5:0xac P5:0xbe': (0, 0),
}
def t737(spec):
    if spec[0] == 'drift':
        return 1 + spec[1] * (P(0x737) + 1)
    return spec[2] * p3(P(0x737), spec[1])
for t, (s0, s1) in T737.items():
    e = per_slave(t737(s0), t737(s1))
    if t in SPREAD:
        e = e + per_slave(*SPREAD[t]) * P(0x744)
        out(0x744, t if t.startswith('B') else ramp(t), e)
    out(0x737, t if t.startswith('B') else ramp(t), e)

# ---- bend / modulation ----------------------------------------------------------------------
bend_mul = (emin(P(0x700), 23) + 1) * pick(P(0x738), [1, 2, 3, 1, 1, 1])
BEND_OSC = osc(0x23)
BEND_VCF = vcf(0x23)
# part switches 0x22C / 0x519 / 0x544 / 0x56E (all four rewrite the same ids): routing state
# 0 normal (0x22C != 1); with 0x22C = 1: 1 = 0x519 is 0, 2 / 3 = 0x519 is 1 and this slave's
# flag (0x544 slave 0, 0x56E slave 1) is / is not 1, 4 = 0x519 >= 2. Values per state for
# slave 0 / slave 1. The sens ids get their sens value or 0 (sens_on, below).
own = per_slave(eq(P(0x544), 1), eq(P(0x56e), 1))
state = sel(eq(P(0x22c), 1), pick(P(0x519), [1, 2 + (1 - own), 4]), 0)
sens_on = 1 - eq(P(0x22c), 1) * (eq(P(0x519), 0) + eq(P(0x519), 1) * emax(eq(P(0x544), 1), eq(P(0x56e), 1)))
SENS = {'P2:0x23 P2:0x39 P3:0x23 P3:0x39': P(0x739) / 255 / 12 * bend_mul,
        'P4:0x23 P4:0x35 P5:0xa3 P5:0xb5': P(0x73a) / 255 / 12 * bend_mul,
        'P2:0x25 P2:0x3b P3:0x25 P3:0x3b': P(0x73b) / 255,
        'P4:0x25 P4:0x37 P5:0xa5 P5:0xb7': P(0x73c) * (10 / 255)}
ROUTING = {
    'P0:0x28 P0:0x3f P1:0x28 P1:0x3f P5:0xc4': ([1, 0, 1, 1, 0], [1, 0, 1, 1, 0]),
    'P0:0x29 P0:0x41 P1:0x2a P1:0x41': ([1, 1, 1, 0, 1], [0, 1, 1, 0, 1]),
    'P0:0x2b P0:0x42 P1:0x2b P1:0x42 B4:0x20': ([0, 0, 0, 1, 0], [1, 0, 0, 1, 0]),
    'P4:0x44 P4:0x45': ([1, 1, 1, 0, 0], [0, 1, 1, 0, 0]),
    'P4:0x46 P4:0x47': ([1, 0, 1, 0, 0], [0, 0, 1, 0, 0]),
    'P0:0x21 P0:0x38 P1:0x21 P1:0x38 P0:0x2a P0:0x40 P1:0x29 P1:0x40': ([0] * 5, [0] * 5),
}
for i in (0x22c, 0x519, 0x544, 0x56e):
    for t, S in SENS.items():
        out(i, t, sens_on * S)
    for t, (v0, v1) in ROUTING.items():
        out(i, t, per_slave(pick(state, v0), pick(state, v1)))

# bend / modulation sensitivities: off when 0x22C = 1 and 0x519 = 0, or 0x519 = 1 with a part flag
# set (the JX-3P caches this in its 0x22C handler; here it follows the current values)
out(0x739, BEND_OSC, sens_on * P(0x739) / 255 / 12 * bend_mul)
out(0x73a, BEND_VCF, sens_on * P(0x73a) / 255 / 12 * bend_mul)
for i in (0x700, 0x738):
    out(i, BEND_OSC, sens_on * P(0x739) / 255 / 12 * bend_mul)
    out(i, BEND_VCF, sens_on * P(0x73a) / 255 / 12 * bend_mul)
out(0x73b, osc(0x25), sens_on * P(0x73b) / 255)
out(0x73c, vcf(0x25), sens_on * P(0x73c) * (10 / 255))
out(0x465, osc(0x22) + ' ' + vcf(0x22), P(0x465) * (0.03113 / 255))
out(0x467, osc(0x24) + ' ' + vcf(0x24), P(0x467) / 255)


# ---- on/off switches outside the patch -------------------------------------------------------
out(0x4a1, 'B0:0x26 B0:0x38 B1:0x26 B1:0x38', sel(P(0x4a1) >= 1, 1, 0))
out(0x4a2, 'B4:0x21 B4:0x30 B5:0x20 B5:0x2f', sel(P(0x4a2) >= 1, 1, 0))
out(0x4a3, 'B2:0x2c B4:0x24 B2:0x3a B4:0x33 B3:0x2c B5:0x23 B3:0x3a B5:0x32', sel(P(0x4a3) >= 1, 1, 0))
out(0x701, 'B0:0x24 B0:0x36 B1:0x24 B1:0x36', sel(P(0x701) >= 1, 1, 0))
out(0x702, 'B0:0x27 B0:0x39 B1:0x27 B1:0x39', sel(P(0x702) >= 1, 1, 0))

# ---- effect (DSP4 slot program; 0x748 type: 0 OD, 1 DS, 2 CHORUS, 3 FUZZ, 4 CRUSHER, 5 PHASER)
FX = P(0x748)
is_drive = pick(FX, [1, 1, 0, 1, 0, 0, 0])
is_type = lambda n: eq(FX, n)
amount = {0x6f9: P(0x6f9), 0x2e0: toward(P(0x6f9), P(0x2e0))}
FXK = {
    'ch_depth': [-10.10651, -9.09998, -8.50848, -8.086894, -7.758409, -7.484207, -7.053864, -6.722869, -6.441688, -5.999158, -5.655898, -5.36063, -4.890824, -4.519704, -4.19575, -3.667463, -3.235849, -2.849749, -2.192142, -1.621852, -1.09841, -0.6068955, -0.150844],  # 0.64 %
    'cr_tone': [-0.7192436, -0.7363934, -0.7592161, -0.7824058, -0.7980752, -0.8139263, -0.8459038, -0.8871384, -0.9096636, -0.9673521, -1.030936, -1.091075, -1.159913, -1.425283, -1.38424, -1.555536, -1.753679, -1.948672, -2.341498, -2.730232, -3.122204, -3.551004, -4.099436],  # 4.6 %
    'cr_rate': [-5.498785, -5.470436, -5.442071, -5.413689, -5.385291, -5.356879, -5.300004, -5.243059, -5.186061, -5.071857, -4.957367, -4.842677, -4.612468, -4.38116, -4.149189, -3.682405, -3.214314, -2.745021, -1.829917, -1.010826, -0.4095974, -0.1070239, -0.04038658],  # 1.8 %
    'ph_res': [-6.994353, -5.994353, -5.409391, -4.994353, -4.672425, -4.40479, -3.98761, -3.66982, -3.401874, -2.985943, -2.669442, -2.401098, -1.985449, -1.669721, -1.399561, -0.9891892, -0.9366723, -0.8781075, -0.7713718, -0.6718921, -0.5788352, -0.4914799, -0.4114701],  # 0.68 %
}
def lk(name, a, v0):
    return sel(lift(a) >= 1, exp2(interp(a, FXK[name])), v0)
def fx_amount(i, a):
    on = sel(a >= 1, 1, 0)
    out(i, ramp('P4:0x49'), sel(is_drive, sel(a >= 1, interp(a, K['fx_level']), 0), on), when=1 - is_type(6))
    out(i, ramp('P4:0x1f0'), sel(a >= 1, exp2(interp(a, K['fx_amount'])), 0), when=is_drive)
    out(i, ramp('P4:0x1f1'), 2048 / 255, when=is_drive)
    out(i, ramp('P4:0x1f2'), lk('ch_depth', a, 0), when=is_type(2))
    out(i, ramp('P4:0x1f3'), 1, when=is_type(2))
    out(i, ramp('P4:0x1f5'), on, when=is_type(4))
    out(i, ramp('P4:0x1f4'), clamp(a / 64, 0, 1), when=is_type(4))
    out(i, ramp('P4:0x1f2'), lk('cr_tone', a, 0.607416), when=is_type(4))
    out(i, ramp('P4:0x1f6'), clamp(a * (64 / 255), 0, 1), when=is_type(5))
    out(i, ramp('P4:0x1f0'), a * (11 / 255) - 8, when=is_type(5))
    out(i, ramp('P4:0x1f2'), floor(32 + a * 1.25) / 255, when=is_type(5))
    out(i, ramp('P4:0x1f3'), lk('ph_res', a, 0), when=is_type(5))
for i, a in amount.items():
    fx_amount(i, a)
def fx_tone(i):
    out(i, ramp('P4:0x1f4'), sel(P(0x749) < 128, (P(0x749) - 127) / 127, (P(0x749) - 128) / 127), when=is_drive)
    out(i, ramp('P4:0x1f6'), P(0x749) * (0.01 / 255), when=is_type(2))
    out(i, ramp('P4:0x1f1'), lk('cr_rate', P(0x749), 0.0216856), when=is_type(4))
    out(i, ramp('P4:0x1f1'), P(0x749) * (1 / 765) - 1 / 6, when=is_type(5))
fx_tone(0x749)
# type change: initial values of the new effect program (after the +0x70 swap)
N = None
FX_INIT = {  # target: values for types 0..5 (None = not written)
    'P4:0x49': [0, 0, 0, 0, 0, 0], 'P4:0x4a': [1, 1, 1, 1, 1, 1],
    'P4:0x1f0': [0, 0, 1, 0, 1, -8], 'P4:0x1f1': [2048 / 255, 2048 / 255, 1, 2048 / 255, 0.0216856, -1 / 6],
    'P4:0x1f2': [1, 1, 0, 1, 0.607416, 0.12549], 'P4:0x1f3': [1, N, 1, N, 0.999878, 0],
    'P4:0x1f4': [-1, -1, 1, -1, 0, 1], 'P4:0x1f5': [N, N, 9.64953e-06, N, 0, 0],
    'P4:0x1f6': [N, N, 0, N, 3, 0], 'P4:0x1f7': [N, N, 0.230463, N, 0, 1],
    'P4:0x1f8': [N, N, N, N, 0, 0.5], 'P4:0x1f9 P4:0x1fa P4:0x1fc P4:0x1fe': [N, N, N, N, N, 0],
    'P4:0x1fb': [N, N, N, N, N, 0.5], 'P4:0x1fd P4:0x1ff': [N, N, N, N, N, 1],
    'B4:0x300 B4:0x301': [N, N, 0, N, N, N], 'B4:0x302': [N, N, 1, N, N, N],
}
for t, vals in FX_INIT.items():
    out(0x748, t, pick(FX, [0 if v is None else v for v in vals] + [0]),
        when=pick(FX, [0 if v is None else 1 for v in vals] + [0]))

fx_amount(0x748, amount[0x6f9])          # the new program gets the current amount and tone
fx_tone(0x748)

# ---- voice with the highest note (0x45C): one flag per voice on DSP5, this slave's first
for i in range(8):
    out(0x45c, f'B5:{0x3e + i:#x}', per_slave(eq(P(0x45c), i), eq(P(0x45c), (i + 4) & 7)))

# ---- start-up constants (no parameter) ----------------------------------------------------------
CONST = [('B0:0x21 B0:0x33 B1:0x21 B1:0x33', -1), ('B0:0x2b B0:0x3d B1:0x2b B1:0x3d', 1),
         ('B4:0x2a B4:0x39 B5:0x29 B5:0x38', 1), ('P4:0x2a P4:0x3c P5:0xaa P5:0xbc', -0.503937),
         ('P4:0x30 P4:0x42 P5:0xb0 P5:0xc2', 0.93),
         ('B0:0x20 B0:0x22 B0:0x29 B0:0x32 B0:0x34 B0:0x3b B1:0x20 B1:0x22 B1:0x29 B1:0x32 B1:0x34 '
          'B1:0x3b B2:0x20 B2:0x21 B2:0x2e B2:0x2f B3:0x20 B3:0x21 B3:0x2e B3:0x2f B4:0x23 B4:0x25 '
          'B4:0x2e B4:0x32 B4:0x34 B4:0x3d B5:0x22 B5:0x24 B5:0x2d B5:0x31 B5:0x33 B5:0x3c', 0)]
for t, v in CONST:
    out(None, t, v)

# order in which the JX-3P processes the ids on a full refresh (last write wins)
REFRESH_ORDER = [0x14, 0x22c, 0x2db, 0x2dc, 0x2dd, 0x2df, 0x2e0, 0x3ef, 0x465, 0x467, 0x4a1, 0x4a2,
                 0x4a3, 0x519, 0x544, 0x56e, 0x6cd, 0x6ce, 0x6cf, 0x6d0, 0x6d1, 0x6d3, 0x6d4, 0x6d6,
                 0x6d7, 0x6d9, 0x6db, 0x6dc, 0x6dd, 0x6df, 0x6e0, 0x6e1, 0x6e2, 0x6e7, 0x6ea, 0x6ec,
                 0x6ed, 0x6ee, 0x6ef, 0x6f0, 0x6f1, 0x6f2, 0x6f3, 0x6f4, 0x6f5, 0x6f6, 0x6f7, 0x6f8,
                 0x6f9, 0x6fd, 0x6fe, 0x6ff, 0x700, 0x701, 0x702, 0x709, 0x736, 0x737, 0x738, 0x739,
                 0x73a, 0x73b, 0x73c, 0x73e, 0x744, 0x748, 0x749, 0x74d, 0x7e3, 0x7e4, 0x801, 0x45c]
