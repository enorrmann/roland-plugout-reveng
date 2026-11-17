#!/usr/bin/env python3
"""jupiter8_bank.py: patches of Roland's JUPITER-8 PLUG-OUT bank (KoaBankFile, plugins/JUPITER-8/Patch/*.bin) ->
JP-08 patch values -> a bank of our JP-08 PLUG-OUT (the same path as jp08_prm.py; README.md §13.8).

    python3 sdk/s1cc/examples/boutique_ported/tools/jupiter8_bank.py BANK.bin[:RANGES] --bank OUT.bin [--prm DIR] [--show]

A TRANSLATION, not a faithful copy: the JUPITER-8 PLUG-OUT is another engine (the SYSTEM-8's), with its own scales
and curves; the JP-08 is a model of the same instrument, so the controls correspond, but the same value need not
sound the same. What is read from the plugin (plugins/JUPITER-8/Script/Script.xml string / number tables and
ScriptEdit.xml offsets), and how it maps (JP-08 values: README.md §13.3):

    LFO WAVE        SIN, TRI, SAW, SQR, RANDOM 1, RANDOM 2           -> LFO WAVE 0..5 as is
    VCO LFO MOD     offset -128 (bipolar)                            -> OSC LFO MOD 2 max(v - 128, 0)
    VCO ENV MOD     offset -128                                      -> OSC ENV MOD 2 |v - 128|
    VCO1 / VCO2 MOD (the VCO-1 / VCO-2 switch, both on = centre)     -> OSC FREQ MOD DST 2 / 0 / 1 (none: no LFO / ENV
                                                                        pitch modulation)
    VCO1 WAVE       SAW, PWM, TRI, SIN, SQR, NOISE                   -> OSC1 WAVE 2, 3, 1, 0, 4, 5
    VCO2 WAVE       the same table                                   -> OSC2 WAVE saw 1, pulse 2, sine 0 (TRI -> sine,
                                                                        SQR -> pulse, NOISE -> saw: OSC2 has no others);
                                                                        LOW FREQ on: + 3 (the LOW positions)
    VCO1 / VCO2 RANGE  64', 32', 16', 8', 4', 2'                     -> OSC1 RANGE as is; OSC2 RANGE in semitones
    VCO2 SUB RANGE  offset -36: semitones                            -> with VCO2 RANGE: (range - 3) 12 + sub - 36
    VCO PWM SOURCE  MAN, LFO, ENV-1 (+), ENV-1 (-), ENV-2 (+), ENV-2 (-) -> PWM SOURCE 1, 2, 0, 0, 0, 0 (the JP-08 has
                                                                        ENV1 only)
    MIXER VCO1 / VCO2                                                -> MIX BALANCE 255 m2 / (m1 + m2)
    VCF ENV MOD     offset -128                                      -> FLT ENV MOD 2 |v - 128|; below 128: ENV1
                                                                        POLARITY 1 (it also inverts pitch, as the JP-08);
                                                                        VCF ENV MOD 0: the sign of VCO ENV MOD
    VCF ENV MOD SOURCE  0 / 1                                        -> FLT ENV MOD SRC 1 (ENV1) / 0 (ENV2) [inferred
                                                                        from the presets: 0 where ENV1 is the filter's]
    VCF LFO MOD     offset -128                                      -> FLT LFO MOD 2 |v - 128|
    VCF KEY FOLLOW  128 = 0 (bipolar)                                -> FLT KEY FOLLOW 2 max(v - 128, 0)
    VCF SLOPE       -24 (0), -18 (1), -12 (2..5)                     -> FLT LPF SLOPE 0, 0, 1
    VCA LFO MOD     offset -128                                      -> AMP LFO MOD 0..3 by |v - 128|: 0, <= 10, <= 25,
                                                                        more
    ENV1 / ENV2 KEY FOLLOW                                           -> ENV2 KEY FOLLOW bits 0 / 1
    ASSIGN MODE     POLY-1 0, MONO 1, UNI 2, POLY-2 3 (ScriptEdit buttons) -> 0, 2 (SOLO), 3, 1
    BEND RANGE      offset +1                                        -> BEND RANGE v + 1 (<= 24)
    PORTAMENTO (0 = off)                                             -> PORTA SW = v > 0, PORTA TIME v
    the rest (cutoff, resonance, HPF, PWM DEPTH, CROSS MOD, FINE TUNE, ADSRs, LFO RATE / DELAY, SYNC)  -> as is
    not taken: VCA LEVEL / TONE, effects, LFO KEY TRIG, legato-only portamento, bend / mod sensitivities, arpeggio
"""
import os
import sys
import xml.etree.ElementTree as ET

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..', '..', '..'))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import jp08_prm as J  # noqa: E402
import patches as PT  # noqa: E402

J8 = os.path.join(ROOT, 'plugins', 'JUPITER-8')


def j8_values(path, script):
    """[(name, {JUPITER-8 value name: value})] of a JUPITER-8 bank (struct "patch": 64 int2x4, name, 48 int8x4)"""
    lay = PT.ScriptLayout(script)
    root = ET.parse(script).getroot()
    types = {e.findtext('type'): e for e in root.findall('structType')}
    names = []

    def walk(t):
        e = types[t]
        subs = e.findall('struct')
        if not subs:
            names.extend(v.findtext('name') for v in e.findall('value') if v.findtext('name') != 'name')
            return
        for c in subs:
            walk(c.findtext('type'))
    walk('patch')
    out = []
    for p in PT.load_bank(path, lay):
        m = p.data['patch']
        v = [m[2 * k] * 16 + m[2 * k + 1] for k in range(64)]
        ex = [int(''.join('%x' % m[256 + 8 * k + j] for j in range(8)), 16) for k in range(48)]
        ex = [x - (1 << 32) if x >= 1 << 31 else x for x in ex]
        out.append((p.name, dict(zip(names, v + ex))))
    return out


def semi_value(st):
    """the JP-08's OSC2 RANGE value of st semitones (the middle of its span)"""
    st = max(-36, min(24, st))
    return J.OSC2_SEMI.index(st) + J.OSC2_SEMI.count(st) // 2


def to_jp08(j):
    c = lambda x: max(0, min(255, int(x)))   # noqa: E731
    v = {n: df for n, _, _, df in J.JP08}
    v['LFO WAVE'] = min(j['LFO WAVE'], 5)
    v['LFO RATE'] = c(j['LFO RATE'])
    v['LFO DELAY TIME'] = c(j['LFO DELAY TIME'])
    m1, m2 = j['VCO1 MOD'], j['VCO2 MOD']
    v['OSC FREQ MOD DST'] = 1 if (m1 and m2) else 2 if m1 else 0
    v['OSC LFO MOD'] = c(2 * max(j['VCO LFO MOD'] - 128, 0)) if (m1 or m2) else 0
    v['OSC ENV MOD'] = c(2 * abs(j['VCO ENV MOD'] - 128)) if (m1 or m2) else 0
    v['PWM'] = c(j['VCO PWM DEPTH'])
    v['PWM SOURCE'] = {0: 1, 1: 2}.get(j['VCO PWM SOURCE'], 0)
    v['OSC1 CROSS MOD'] = c(j['VCO1 CROSS MOD'])
    v['OSC1 RANGE'] = min(max(j['VCO1 RANGE'], 0), 5)
    v['OSC1 WAVE'] = [2, 3, 1, 0, 4, 5][min(j['VCO1 WAVE'], 5)]
    v['OSC2 SYNC'] = 1 if j['VCO2 SYNC'] else 0
    v['OSC2 RANGE'] = semi_value((min(max(j['VCO2 RANGE'], 0), 5) - 3) * 12 + j['VCO2 SUB RANGE'] - 36)
    v['OSC2 TUNE'] = c(j['VCO2 FINE TUNE'])
    v['OSC2 WAVE'] = [1, 2, 0, 0, 2, 1][min(j['VCO2 WAVE'], 5)] + (3 if j['VCO2 LOW FREQ'] else 0)
    a, b = j['MIXER VCO1'], j['MIXER VCO2']
    v['MIX BALANCE'] = c(round(255 * b / (a + b))) if a + b else 128
    v['HPF'] = c(j['HPF CUTOFF'])
    v['CUTOFF'] = c(j['VCF CUTOFF'])
    v['RESONANCE'] = c(j['VCF RESONANCE'])
    v['FLT LPF SLOPE'] = 0 if j['VCF SLOPE'] <= 1 else 1
    e = j['VCF ENV MOD'] - 128
    v['FLT ENV MOD'] = c(2 * abs(e))
    # ENV1 POLARITY: the filter's sign; with no filter ENV MOD, the pitch's (one polarity for both on the JP-08)
    v['ENV1 POLARITY'] = 1 if (e < 0 if e != 0 else j['VCO ENV MOD'] < 128) else 0
    v['FLT ENV MOD SRC'] = 0 if j['VCF ENV MOD SOURCE'] else 1
    v['FLT LFO MOD'] = c(2 * abs(j['VCF LFO MOD'] - 128))
    v['FLT KEY FOLLOW'] = c(2 * max(j['VCF KEY FOLLOW'] - 128, 0))
    d = abs(j['VCA LFO MOD'] - 128)
    v['AMP LFO MOD'] = 0 if d == 0 else 1 if d <= 10 else 2 if d <= 25 else 3
    for i, s in ((1, 'ENV1'), (2, 'ENV2')):
        for x in ('ATTACK', 'DECAY', 'SUSTAIN', 'RELEASE'):
            v['%s %s' % (s, x)] = c(j['%s %s' % (s, x)])
    v['ENV2 KEY FOLLOW'] = (1 if j['ENV1 KEY FOLLOW'] else 0) | (2 if j['ENV2 KEY FOLLOW'] else 0)
    v['ASSIGN MODE'] = [0, 2, 3, 1][min(j['ASSIGN MODE'], 3)]
    v['BEND RANGE'] = min(j['BEND RANGE'] + 1, 24)
    v['PORTA SW'] = 1 if j['PORTAMENTO'] > 0 else 0
    v['PORTA TIME'] = c(j['PORTAMENTO'])
    return v


def main():
    argv = sys.argv[1:]

    def take(flag):
        if flag in argv:
            i = argv.index(flag)
            x = argv[i + 1]
            del argv[i:i + 2]
            return x
        return None
    bank, prm = take('--bank'), take('--prm')
    show = '--show' in argv
    specs = [a for a in argv if a != '--show']
    if not specs or not (bank or prm):
        raise SystemExit(__doc__)
    pats = []
    for spec in specs:
        path, ranges = PT.resolve_bank(spec, J8)
        allp = j8_values(path, os.path.join(J8, 'Script', 'Script.xml'))
        idx = PT.parse_ranges(ranges, len(allp)) if ranges else range(len(allp))
        for i in idx:
            name, j = allp[i]
            vals = to_jp08(j)
            pats.append((J.to_s1(vals), (name + ' ' * 16)[:16]))
            print('%s %d: "%s"' % (os.path.basename(path), i + 1, name))
            if show:
                print('  JP-08: ' + ', '.join('%s %d' % (n, vals[n]) for n, _, _, _ in J.JP08))
    if prm:
        os.makedirs(prm, exist_ok=True)
        for i, (s1, name) in enumerate(pats):
            J.write_prm(os.path.join(prm, 'PLUGOUT_PATCH%d.PRM' % (i + 1)), s1, name)
    if bank:
        recs = [J.bank_record(s1, name) for s1, name in pats[:64]]
        empty = J.to_s1({n: df for n, _, _, df in J.JP08})
        recs += [J.bank_record(empty, '-' * 16)] * (64 - len(recs))
        open(bank, 'wb').write(b'KoaBankFile00002' + J.TAG + b''.join(recs))
        print('%s: %d patches' % (bank, len(pats)))


if __name__ == '__main__':
    main()
