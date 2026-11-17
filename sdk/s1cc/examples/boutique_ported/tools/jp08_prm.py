#!/usr/bin/env python3
"""jp08_prm.py: JP-08 patch files -> SYSTEM-1 patches of our JP-08 PLUG-OUT (README.md §13).

    python3 sdk/s1cc/examples/boutique_ported/tools/jp08_prm.py IN... [--prm DIR] [--bank FILE] [--first N] [--show]

IN: JP-08 patch files (JP08_PATCH<n>.PRM, as the JP-08 writes them into BACKUP/) or folders holding them (taken in
patch-number order). Writes
    --prm DIR    PLUGOUT_PATCH<n>.PRM, n = --first (default 1) on: the SYSTEM-1's own patch files (its RESTORE)
    --bank FILE  a KoaBankFile00002 bank (64 records, the rest "----------------" with the JP-08 defaults) for
                 tools/plugout_tool.py install --bank FILE (the plugin folder needs a SYSTEM-1 Script.xml: any SYSTEM-1
                 PLUG-OUT's, they share the layout, README.md §13.5)
    --show       each patch: the JP-08 values and the SYSTEM-1 values they became

Reading (README.md §13.2, the JP-08's own reader 0x600110d4 / 0x60011664): records end at ';'; a byte outside
0x20..0x7E throws the partial record away; a record is parameter k when its 16-character padded name occurs in it with
the first '(' right after it; the value is '-'? digits up to ')'; parameters start at the JP-08 default and a value
out of range becomes the default; PATCH_NAME(...) gives up to 16 characters.

Mapping (README.md §13.5): the JP-08 values with a SYSTEM-1 control go into its panel value as the PLUG-OUT's knob
reads it (all unipolar, row = value: MIX BALANCE v -> OSC1 MIX 255 - v, OSC2 MIX v; AMP LFO MOD 0..3 -> LFO TO AMP
64 v + 32); OSC2 RANGE (semitones) -> OSC2 RANGE octave + OSC2 COARSE, PORTA SW / TIME -> PORTAMENTO, BEND RANGE, FLT LPF SLOPE ->
LPF TYPE, DELAY LEVEL / TIME -> DELAY / DELAY TIME (x 17); only the ones without a SYSTEM-1 control go into
PLUGOUT_EXTEND words, as (value - JP-08 default) & 0xFF (tools/make_arm.py EXTEND, the same table the PLUG-OUT's CCs
use). SYSTEM-1 values the JP-08 has not: the SYSTEM-1's defaults (its Script.xml), with the
SYSTEM-1's own effects (CRUSHER, REVERB, DELAY), LFO KEY TRIG and PORTAMENTO at 0.
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from make_arm import EXTEND, EXT0  # noqa: E402

# the JP-08's patch file table (PX-11:Appli 1.21: names 0x60066b1c, min / max / default 0x6005a794; README.md §13.3)
JP08 = [('LFO RATE', 0x18E, 255, 0), ('LFO DELAY TIME', 0x18F, 255, 0), ('LFO WAVE', 0x190, 5, 0),
        ('OSC LFO MOD', 0x196, 255, 0), ('OSC ENV MOD', 0x197, 255, 0), ('OSC FREQ MOD DST', 0x198, 2, 0),
        ('PWM', 0x199, 255, 0), ('PWM SOURCE', 0x19A, 2, 0), ('OSC1 CROSS MOD', 0x19B, 255, 0),
        ('OSC1 RANGE', 0x19C, 5, 0), ('OSC1 WAVE', 0x19D, 5, 0), ('OSC2 SYNC', 0x19E, 1, 0),
        ('OSC2 RANGE', 0x19F, 255, 0), ('OSC2 TUNE', 0x1A0, 255, 128), ('OSC2 WAVE', 0x1A1, 5, 0),
        ('MIX BALANCE', 0x1A2, 255, 0), ('HPF', 0x1A8, 255, 0), ('CUTOFF', 0x1A9, 255, 255),
        ('RESONANCE', 0x1AA, 255, 0), ('FLT LPF SLOPE', 0x1AB, 1, 1), ('FLT ENV MOD', 0x1AC, 255, 0),
        ('FLT ENV MOD SRC', 0x1AD, 1, 0), ('FLT LFO MOD', 0x1AE, 255, 0), ('FLT KEY FOLLOW', 0x1AF, 255, 0),
        ('AMP LEVEL', 0x1B5, 255, 255), ('AMP LFO MOD', 0x1B6, 3, 0), ('ENV1 ATTACK', 0x1BC, 255, 0),
        ('ENV1 DECAY', 0x1BD, 255, 0), ('ENV1 SUSTAIN', 0x1BE, 255, 255), ('ENV1 RELEASE', 0x1BF, 255, 0),
        ('ENV1 POLARITY', 0x1C0, 1, 0), ('ENV2 ATTACK', 0x1C6, 255, 0), ('ENV2 DECAY', 0x1C7, 255, 0),
        ('ENV2 SUSTAIN', 0x1C8, 255, 255), ('ENV2 RELEASE', 0x1C9, 255, 0), ('ENV2 KEY FOLLOW', 0x1CA, 3, 0),
        ('DELAY LEVEL', 0x239, 15, 0), ('DELAY TIME', 0x23A, 15, 0), ('DELAY FEEDBACK', 0x23B, 15, 0),
        ('PORTA SW', 0x242, 1, 0), ('PORTA TIME', 0x243, 255, 100), ('ASSIGN MODE', 0x245, 4, 0),
        ('BEND RANGE', 0x246, 24, 2)]

# the SYSTEM-1's patch values and files: tools/s1_bank.py (moved from here 2026-11-17)
sys.path.insert(0, os.path.join(HERE, '..', '..', '..', '..', '..', 'tools'))
from s1_bank import S1_DEF, S1_NAMES, write_prm, bank_record, HEADER  # noqa: E402,F401
# JP-08 value -> SYSTEM-1 panel value (README.md §13.5); the EXTEND ones come from make_arm.EXTEND
PANEL = {'LFO RATE': ['LFO_RATE'], 'LFO DELAY TIME': ['LFO_FADE'], 'LFO WAVE': ['LFO_WAVE'],
         'OSC LFO MOD': ['LFO_PITCH'], 'OSC ENV MOD': ['PIT_ENV'], 'PWM': ['OSC2_COLOR', 'OSC1_COLOR'],
         'OSC1 CROSS MOD': ['OSC1_CRS_MOD'], 'OSC1 RANGE': ['OSC1_RANGE'], 'OSC1 WAVE': ['OSC1_WAVE'],
         'OSC2 SYNC': ['OSC2_SYNC'], 'OSC2 TUNE': ['OSC2_TUNE'], 'OSC2 WAVE': ['OSC2_WAVE'], 'HPF': ['FLT_HPF'],
         'CUTOFF': ['FLT_LPF'], 'RESONANCE': ['FLT_RESO'], 'FLT LFO MOD': ['LFO_FLT'],
         'FLT KEY FOLLOW': ['FLT_KEY'], 'ENV1 ATTACK': ['FLT_ATK'], 'ENV1 DECAY': ['FLT_DCY'],
         'ENV1 SUSTAIN': ['FLT_SUS'], 'ENV1 RELEASE': ['FLT_REL'], 'ENV2 ATTACK': ['AMP_ATK'],
         'ENV2 DECAY': ['AMP_DCY'], 'ENV2 SUSTAIN': ['AMP_SUS'], 'ENV2 RELEASE': ['AMP_REL']}
# OSC2 RANGE -> semitones (the JP-08's table 0x4a over 0..255, measured in tools/bq_fw.py: round(12 log2 factor))
OSC2_SEMI = ([-36] * 3 + [-35] * 2 + [-34] * 3 + [-33] * 2 + [-32] * 3 + [-31] * 2 + [-30] * 2 + [-29] * 3 + [-28] * 2
             + [-27] * 3 + [-26] * 2 + [-25] * 2 + [-24] * 6 + [-23] * 5 + [-22] * 5 + [-21] * 5 + [-20] * 5 + [-19] * 5
             + [-18] * 5 + [-17] * 5 + [-16] * 5 + [-15] * 5 + [-14] * 5 + [-13] * 4 + [-12] * 8 + [-11] * 6
             + [-10] * 6 + [-9] * 6 + [-8] * 6 + [-7] * 6 + [-6] * 7 + [-5] * 6 + [-4] * 6 + [-3] * 6 + [-2] * 6
             + [-1] * 6 + [0] * 6 + [1] * 5 + [2] * 5 + [3] * 5 + [4] * 5 + [5] * 5 + [6] * 5 + [7] * 5 + [8] * 5
             + [9] * 5 + [10] * 5 + [11] * 4 + [12] * 4 + [13] * 2 + [14] * 3 + [15] * 2 + [16] * 3 + [17] * 2
             + [18] * 2 + [19] * 3 + [20] * 2 + [21] * 3 + [22] * 2 + [23] * 2 + [24] * 2)
assert len(OSC2_SEMI) == 256, len(OSC2_SEMI)
# PWM SOURCE 0 ENV1 / 1 MANUAL / 2 LFO [code: the JP-08's ARM writes A092 / A090 / A091 = 1, tools/bq_fw.py setp
# 0x19A] -> OSC2 COLOR MOD 3 (FILTER ENV) / 0 (MANUAL) / 1 (LFO): the PLUG-OUT's ARM reads 0 MANUAL, 1 LFO, 2..4 ENV1
PWM_SOURCE = [3, 0, 1]


def read_jp08(path):
    """the JP-08's reader: {name: value} for the 43 parameters, and the patch name"""
    d = open(path, 'rb').read()
    vals = {n: df for n, _, _, df in JP08}
    name = ' ' * 16
    line = bytearray()
    for b in d:
        if b == 0x3B:                                    # ';' ends the record
            rec = line.decode('latin-1')
            line = bytearray()
            name = parse_name(rec, name)
            parse_record(rec, vals)
            continue
        if not 0x20 <= b <= 0x7E or len(line) >= 127:    # CR / LF (or a long line): the partial record goes
            line = bytearray()
            continue
        line.append(b)
    for n, _, mx, df in JP08:                           # out of range: the default (as the JP-08)
        if not 0 <= vals[n] <= mx:
            vals[n] = df
    return vals, name


def parse_record(rec, vals):
    o, c = rec.find('('), rec.find(')')
    for n, _, _, _ in JP08:
        key = n.ljust(16)
        i = rec.find(key)
        if i < 0 or o < 0 or c < 0 or o - i != 16 or o + 1 >= c:
            continue
        m = re.fullmatch(r'-?\d+', rec[o + 1:c])
        if m:
            vals[n] = int(rec[o + 1:c])
        return


def parse_name(rec, name):
    i = rec.find('PATCH_NAME')
    o, c = rec.find('('), rec.find(')')
    if i < 0 or o - i != 10 or c < 0:
        return name
    return (rec[o + 1:c] + ' ' * 16)[:16]


def to_s1(vals):
    """the SYSTEM-1 patch values (S1_NAMES order) of a JP-08 patch"""
    s1 = dict(S1_DEF)
    s1.update({'LFO_TRIG': 0, 'CRUSHER': 0, 'REVERB': 0, 'DELAY': 0, 'DELAY_TM': 0})
    for n, names in PANEL.items():
        for s in names:
            s1[s] = vals[n]
    s1['MIX_OSC1'] = 255 - vals['MIX BALANCE']
    s1['MIX_OSC2'] = vals['MIX BALANCE']
    s1['OSC2_MOD'] = PWM_SOURCE[vals['PWM SOURCE']]
    s1['LFO_AMP'] = 64 * vals['AMP LFO MOD'] + 32
    s1['AMP_TONE'] = 64 * vals['ENV2 KEY FOLLOW'] + 32  # the PLUG-OUT reads knob >> 6: 0 off, 1 ENV1, 2 ENV2, 3 both
    # OSC2 RANGE 0..255 = semitones -36..+24 from 8' (PX-11 0x60054dfc table 0x4a, README.md §13.3): the octave on
    # OSC2 RANGE (64' .. 2' = 0..5, 8' = 3), the rest on OSC2 COARSE (-6..+6)
    st = OSC2_SEMI[vals['OSC2 RANGE']]
    k2 = max(0, min(5, 3 + (st + 6) // 12 if st >= 0 else 3 - ((-st + 5) // 12)))
    s1['OSC2_RANGE'], s1['OSC2_COARSE'] = k2, st - 12 * (k2 - 3)
    s1['PORTAMENTO'] = vals['PORTA TIME']                 # the PORTAMENTO knob = PORTA TIME
    s1['LEGATO'] = vals['PORTA SW']                       # the LEGATO button = PORTA SW (README.md §13.7)
    # ASSIGN MODE 0 POLY1 / 1 POLY2 / 2 SOLO / 3 UNISON (strings at 0x600c2ca8) -> MONO (0x192, the OS's range 0..3;
    # the PLUG-OUT's MONO button steps it); 4 has no name in the firmware: POLY1. BEND RANGE: EXTEND04 (below).
    # DELAY FEEDBACK: not taken (user 2026-11-12)
    s1['MONO'] = vals['ASSIGN MODE'] if vals['ASSIGN MODE'] <= 3 else 0
    s1['FLT_TYPE'] = vals['FLT LPF SLOPE']
    # FLT ENV MOD + ENV1 POLARITY -> FILTER ENV AMOUNT: the knob reads row 2 |k - 128| (one bit less), below 128 =
    # polarity 1 (inverted)
    v = vals['FLT ENV MOD']
    s1['FLT_ENV'] = min(255, 128 + (v + 1) // 2) if vals['ENV1 POLARITY'] == 0 else 128 - (v + 1) // 2
    s1['DELAY'] = 17 * vals['DELAY LEVEL']                # 0..15 -> 0..255 (the SYSTEM-1's own delay)
    s1['DELAY_TM'] = 17 * vals['DELAY TIME']
    jp = {n: pid for n, pid, _, _ in JP08}
    for i, (nm, pid, cc, st, mx, df, ap) in enumerate(EXTEND):
        assert jp[nm] == pid, nm
        s1['PLUGOUT_EXTEND%02d' % (i + 1)] = (vals[nm] - df) & 0xFF
    return s1


TAG = b'PG-JP08S1'


def inputs(args):
    out = []
    for a in args:
        if os.path.isdir(a):
            fs = [f for f in os.listdir(a) if re.fullmatch(r'JP08_PATCH(\d+)\.PRM', f, re.I)]
            out += [os.path.join(a, f) for f in sorted(fs, key=lambda f: int(re.search(r'PATCH(\d+)', f, re.I).group(1)))]
        else:
            out.append(a)
    return out


def main():
    argv = sys.argv[1:]

    def take(flag):
        if flag in argv:
            i = argv.index(flag)
            v = argv[i + 1]
            del argv[i:i + 2]
            return v
        return None
    prm, bank, first = take('--prm'), take('--bank'), int(take('--first') or 1)
    show = '--show' in argv
    files = inputs([a for a in argv if a != '--show'])
    if not files:
        raise SystemExit(__doc__)
    pats = []
    for f in files:
        vals, name = read_jp08(f)
        s1 = to_s1(vals)
        pats.append((s1, name))
        print('%s: "%s"' % (f, name))
        if show:
            print('  JP-08: ' + ', '.join('%s %d' % (n, vals[n]) for n, _, _, _ in JP08))
            print('  SYSTEM-1: ' + ', '.join('%s %d' % (n, s1[n]) for n in S1_NAMES if s1[n] != S1_DEF[n]))
    if prm:
        os.makedirs(prm, exist_ok=True)
        for i, (s1, name) in enumerate(pats):
            write_prm(os.path.join(prm, 'PLUGOUT_PATCH%d.PRM' % (first + i)), s1, name)
        print('%s: PLUGOUT_PATCH%d..%d.PRM' % (prm, first, first + len(pats) - 1))
    if bank:
        recs = [bank_record(s1, name) for s1, name in pats[:64]]
        empty = to_s1({n: df for n, _, _, df in JP08})
        recs += [bank_record(empty, '-' * 16)] * (64 - len(recs))
        open(bank, 'wb').write(b'KoaBankFile00002' + TAG + b''.join(recs))
        print('%s: %d patches (+ %d empty)' % (bank, min(len(pats), 64), 64 - min(len(pats), 64)))


if __name__ == '__main__':
    main()
