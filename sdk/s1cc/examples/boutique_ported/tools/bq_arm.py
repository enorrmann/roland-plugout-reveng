"""bq_arm.py: the ARM side of one Boutique model as data, from the firmware run in an emulator (bq_fw.py).

    python3 sdk/s1cc/examples/boutique_ported/tools/bq_arm.py MODEL      (MODEL: jp08, ju06, jx03; README.md §3)

Writes
    work/bq1/MODEL_patches.json        the immediate patches the ARM makes in DSP chip 0's programs for this model
                                       [(type, byte address, value)]: make_voice.sh applies them before the lift
    sdk/.../bq_MODEL_tables.h          MODEL_init[0x70]: every parameter word C000..C06F of chip 0 (shared C000..C037,
                                       voice A C038..C06F) after the model's set-up and the default panel below;
                                       one table per mapped panel control: MODEL_<name>[256][n] = the words that
                                       control writes for knob value 0..255 (the patch value the firmware gets)

The Boutique (first generation) runs at 44.1 kHz, the SYSTEM-1 at 96 kHz: the words that are per-sample rates are
converted here (RATE_WORDS, r = 44100 / 96000): one-pole coefficients c -> 1 - (1 - c)^r, the envelope decay /
release words (factor 1 - 0.01 c per sample) -> (1 - (1 - 0.01 c)^r) / 0.01, the LFO phase step x r, the LFO delay
factor (0.9 + 0.1 c) -> its r-th power. The program's own rate constants are converted by make_voice.sh.
"""
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import bq_fw as fx  # noqa: E402

ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..', '..', '..'))
R = 44100.0 / 96000.0
MODELS = {'jp08': 0, 'ju06': 1, 'jx03': 2}


def one_pole(c):
    return 1.0 - (1.0 - min(max(c, 0.0), 1.0)) ** R


def env_rate(c):
    f = min(max(1.0 - 0.01 * c, 0.0), 1.0)
    return (1.0 - f ** R) / 0.01


def lfo_delay(c):
    f = 0.9 + 0.1 * c
    return (min(max(f, 0.0), 1.0) ** R - 0.9) / 0.1


RATE_WORDS = {0x0A: lambda c: c * R, 0x0B: lfo_delay, 0x3C: one_pole, 0x40: one_pole, 0x5E: one_pole,
              0x3D: env_rate, 0x3F: env_rate, 0x41: env_rate, 0x43: env_rate}

# SYSTEM-1 control -> (table name, the model's patch parameter id); README.md §4
PANEL = {   # (table, patch parameter id, SYSTEM-1 control, how the knob reads the table, the words used or None)
    #   uni: row = knob; bi: 128 = 0, row = 2 |knob - 128| (255 at the ends), the words negated below 128;
    #   rev: row = 255 - knob (the balance's OSC1 side); sq: row = knob^2 / 255 (finer at the low end)
    'jp08': [('lfo_rate', 0x18E, 'S1_LFO_RATE', 'uni', None), ('lfo_delay', 0x18F, 'S1_LFO_FADE', 'uni', None),
             ('lfo_pitch', 0x196, 'S1_LFO_TO_PITCH', 'bi', None),
             ('lfo_filter', 0x1AE, 'S1_LFO_TO_FILTER', 'bi', None),
             ('env_pitch', 0x197, 'S1_PITCH_ENV_AMOUNT', 'bi', None),
             ('xmod', 0x19B, 'S1_OSC1_COLOR', 'sq', None), ('pwm', 0x199, 'S1_OSC2_COLOR', 'uni', None),
             ('tune2', 0x1A0, 'S1_OSC2_FINE', 'uni', None),
             ('mix1', 0x1A2, 'S1_OSC1_MIX', 'rev', [0x5A]), ('mix2', 0x1A2, 'S1_OSC2_MIX', 'uni', [0x5B]),
             ('cutoff', 0x1A9, 'S1_FILTER_CUTOFF', 'uni', None),
             ('reso', 0x1AA, 'S1_FILTER_RESONANCE', 'uni', None),
             ('envmod', 0x1AC, 'S1_FILTER_ENV_AMOUNT', 'bi', None),
             ('keyfollow', 0x1AF, 'S1_FILTER_KEY_TRACK', 'uni', None),
             ('env1_a', 0x1BC, 'S1_FILTER_ATTACK', 'uni', None), ('env1_d', 0x1BD, 'S1_FILTER_DECAY', 'uni', None),
             ('env1_s', 0x1BE, 'S1_FILTER_SUSTAIN', 'uni', None), ('env1_r', 0x1BF, 'S1_FILTER_RELEASE', 'uni', None),
             ('env2_a', 0x1C6, 'S1_AMP_ATTACK', 'uni', None), ('env2_d', 0x1C7, 'S1_AMP_DECAY', 'uni', None),
             ('env2_s', 0x1C8, 'S1_AMP_SUSTAIN', 'uni', None), ('env2_r', 0x1C9, 'S1_AMP_RELEASE', 'uni', None)],
}


def row_of(kind, knob):
    """(table row, sign) a SYSTEM-1 knob value reads"""
    if kind == 'bi':
        return min(255, 2 * abs(knob - 128)), (-1.0 if knob < 128 else 1.0)
    if kind == 'rev':
        return 255 - knob, 1.0
    if kind == 'sq':
        return knob * knob // 255, 1.0
    return knob, 1.0


# the patch the tables start from (and the init values of the controls the panel does not move)
DEFAULT = {
    'jp08': {0x1A2: 64, 0x1A9: 170, 0x1AA: 40, 0x1AC: 60, 0x1B5: 255, 0x1BC: 5, 0x1BD: 120, 0x1BE: 128,
             0x1BF: 60, 0x1C6: 2, 0x1C7: 100, 0x1C8: 200, 0x1C9: 80, 0x1A0: 128, 0x18E: 100,
             0x198: 1},     # OSC FREQ MOD DST = VCO1 + VCO2 (C04F..C052 = 1): LFO / ENV -> PITCH on both oscillators
}


def conv(a, v):
    f = RATE_WORDS.get(a)
    return f(v) if f else v


def chip0(p):
    return {a: v for (c, a), v in p.items() if c == 0 and a < 0x70}


def main():
    model = sys.argv[1]
    mode = MODELS[model]
    first = fx.setup(mode)
    patches = [(t, a, v) for (c, t, a), v in sorted(fx.portw().items()) if c == 0]
    json.dump(patches, open(os.path.join(ROOT, 'work', 'bq1', '%s_patches.json' % model), 'w'))
    words = chip0(first)
    fx.call(0x60024ff0, fx.O, 60, 100, 0, count=5000000)        # vtable 10: the whole patch (r1 != 0: send it)
    words.update(chip0(fx.params()))
    for pid, v in sorted(DEFAULT[model].items()):
        words.update(chip0(fx.setp(pid, v)))
    words.update(chip0(fx.setp(0x108 - 0x141, 127)))           # internal id 0x108: velocity -> C069 = (v / 127)^2
    tables = []
    cache = {}
    for name, pid, s1id, kind, only in PANEL[model]:
        if pid not in cache:
            cache[pid] = [chip0(fx.setp(pid, k)) for k in range(256)]
            fx.setp(pid, DEFAULT[model].get(pid, 0))
        rows = cache[pid]
        addrs = sorted({a for r in rows for a in r if len({round(rr.get(a, 0.0), 9) for rr in rows}) > 1})
        if only:
            addrs = [a for a in addrs if a in only]
        out_rows = []
        for k in range(256):
            row, sign = row_of(kind, k)
            out_rows.append([sign * conv(a, rows[row].get(a, words.get(a, 0.0))) for a in addrs])
        tables.append((name, pid, s1id, addrs, out_rows))
    init = [conv(a, words.get(a, 0.0)) for a in range(0x70)]
    out = ['/* bq_%s_tables.h: the %s ARM side as data (Boutique 1.21, PX-11:Appli), written by tools/bq_arm.py from'
           ' the firmware run in an emulator; per-sample rates converted 44.1 -> 96 kHz. Do not edit: README.md §3. */'
           % (model, model.upper()), '#ifndef BQ_%s_TABLES_H' % model.upper(), '#define BQ_%s_TABLES_H' % model.upper(), '']
    out.append('/* chip 0 parameter words C000..C06F after the set-up and the default patch (tools/bq_arm.py DEFAULT) */')
    out.append('static const float %s_init[0x70] = {' % model)
    for i in range(0, 0x70, 8):
        out.append('    ' + ', '.join('%.9ef' % x for x in init[i:i + 8]) + ',')
    out.append('};')
    for name, pid, s1id, addrs, rows in tables:
        out.append('')
        out.append('/* patch parameter %#x -> %s */' % (pid, ', '.join('C%03X' % a for a in addrs)))
        out.append('#define %s_%s_N %d' % (model.upper(), name.upper(), len(addrs)))
        out.append('static const unsigned short %s_%s_addr[%d] = {%s};' % (model, name, max(1, len(addrs)),
                                                                           ', '.join('0x%x' % a for a in addrs) or '0'))
        out.append('static const float %s_%s[256][%d] = {' % (model, name, max(1, len(addrs))))
        for r in rows:
            out.append('    {%s},' % (', '.join('%.9ef' % x for x in r) or '0.0f'))
        out.append('};')
    out.append('')
    out.append('/* the panel (README.md §4): one S1_PARAM per table, and the parameter words they give the voice */')
    out.append('#ifdef BQ_PANEL')
    for name, pid, s1id, addrs, rows in tables:
        out.append('S1_PARAM(%s, %s, %d) { for (int i = 0; i < %d; i++) out[i] = %s_%s[knob][i]; }'
                   % (name, s1id, max(1, len(addrs)), max(1, len(addrs)), model, name))
    out.append('static inline float %s_kp(int a)' % model)
    out.append('{')
    out.append('    switch (a) {')
    for name, pid, s1id, addrs, rows in tables:
        for i, a in enumerate(addrs):
            out.append('    case 0x%02x: return s1_param(%s, %d);' % (a, name, i))
    out.append('    default: return %s_init[a & 0x7f];' % model)
    out.append('    }')
    out.append('}')
    out.append('#endif')
    out.append('')
    out.append('#endif')
    dst = os.path.join(HERE, '..', 'bq_%s_tables.h' % model)
    open(dst, 'w').write('\n'.join(out) + '\n')
    print('%s: %d patches, %d tables (%s)' % (dst, len(patches), len(tables),
                                            ' '.join('%s:%d' % (t[0], len(t[3])) for t in tables)))


if __name__ == '__main__':
    main()
