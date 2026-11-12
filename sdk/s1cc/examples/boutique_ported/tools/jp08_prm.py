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
64 v + 32); the others into PLUGOUT_EXTEND words as (value - JP-08 default) & 0xFF (tools/make_arm.py EXTEND, the same
table the PLUG-OUT's CCs use). SYSTEM-1 values the JP-08 has not: the SYSTEM-1's defaults (its Script.xml), with the
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

# the SYSTEM-1's patch values in file order (OS 1.30 table 0x5b15c, ids 0x160..0x19F, 0x1B0..0x1DF) and defaults
# (SYSTEM-1 Script.xml struct "patch"; OSC2_COARSE as the .PRM writes it: bank byte - 11, README.md §13.5)
S1 = ('LFO_WAVE 0 LFO_FADE 0 LFO_RATE 0 LFO_PITCH 128 LFO_FLT 128 LFO_AMP 128 LFO_TRIG 0 OSC1_WAVE 0 OSC1_COLOR 0 '
      'OSC1_MOD 0 OSC1_RANGE 0 OSC1_CRS_MOD 0 OSC2_WAVE 0 OSC2_COLOR 0 OSC2_MOD 0 OSC2_RANGE 0 OSC2_TUNE 128 '
      'OSC2_RING 0 OSC2_SYNC 0 OSC2_COARSE 0 MIX_OSC1 128 MIX_OSC2 0 MIX_SUB_OSC 0 MIX_NOISE 0 MIX_OSC_TYPE 1 '
      'MIX_NOISE_TYPE 1 PIT_ENV 128 PIT_ATK 0 PIT_DCY 0 FLT_LPF 255 FLT_TYPE 1 FLT_RESO 0 FLT_HPF 0 FLT_ENV 128 '
      'FLT_ATK 0 FLT_DCY 0 FLT_SUS 0 FLT_REL 0 FLT_KEY 128 AMP_ATK 0 AMP_DCY 0 AMP_SUS 255 AMP_REL 0 AMP_TONE 128 '
      'CRUSHER 0 REVERB 0 DELAY 0 DELAY_TM 0 PORTAMENTO 0 LEGATO 0 MONO 0 BEND_RANGE 1 MOD_ASGN 0 TEMPO_SYNC 0 '
      'MOD_ID1 0 MOD_ID2 0 MOD_ID3 0 MOD_PRM1 0 MOD_PRM2 0 MOD_PRM3 0 PAT_LEV 128 PAT_LEV_SW 1 OSC1_EX 0 OSC2_EX 0 '
      'INPUT_JACK_SW 0 ARP_SW 0 ARP_TYPE 0 ARP_STEP 0 SCAT_TYPE 0 SCAT_DEP 0 CTRL_OCT 0 CTRL_HOLD 0').split()
S1_DEF = {S1[i]: int(S1[i + 1]) for i in range(0, len(S1), 2)}
for _i in range(40):
    S1_DEF['PLUGOUT_EXTEND%02d' % (_i + 1)] = 0
S1_NAMES = list(S1_DEF)                                  # 112, in file order
# JP-08 value -> SYSTEM-1 panel value (README.md §13.5); the EXTEND ones come from make_arm.EXTEND
PANEL = {'LFO RATE': ['LFO_RATE'], 'LFO DELAY TIME': ['LFO_FADE'], 'LFO WAVE': ['LFO_WAVE'],
         'OSC LFO MOD': ['LFO_PITCH'], 'OSC ENV MOD': ['PIT_ENV'], 'PWM': ['OSC2_COLOR', 'OSC1_COLOR'],
         'OSC1 CROSS MOD': ['OSC1_CRS_MOD'], 'OSC1 RANGE': ['OSC1_RANGE'], 'OSC1 WAVE': ['OSC1_WAVE'],
         'OSC2 SYNC': ['OSC2_SYNC'], 'OSC2 TUNE': ['OSC2_TUNE'], 'OSC2 WAVE': ['OSC2_WAVE'], 'HPF': ['FLT_HPF'],
         'CUTOFF': ['FLT_LPF'], 'RESONANCE': ['FLT_RESO'], 'FLT ENV MOD': ['FLT_ENV'], 'FLT LFO MOD': ['LFO_FLT'],
         'FLT KEY FOLLOW': ['FLT_KEY'], 'ENV1 ATTACK': ['FLT_ATK'], 'ENV1 DECAY': ['FLT_DCY'],
         'ENV1 SUSTAIN': ['FLT_SUS'], 'ENV1 RELEASE': ['FLT_REL'], 'ENV2 ATTACK': ['AMP_ATK'],
         'ENV2 DECAY': ['AMP_DCY'], 'ENV2 SUSTAIN': ['AMP_SUS'], 'ENV2 RELEASE': ['AMP_REL']}
# PWM SOURCE MANUAL / LFO / ENV -> OSC2 COLOR MOD manual (0) / LFO (1) / FILTER ENV (3: the JP-08's ENV is ENV1)
PWM_SOURCE = [0, 1, 3]


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
    s1.update({'LFO_TRIG': 0, 'CRUSHER': 0, 'REVERB': 0, 'DELAY': 0, 'DELAY_TM': 0, 'PORTAMENTO': 0})
    for n, names in PANEL.items():
        for s in names:
            s1[s] = vals[n]
    s1['MIX_OSC1'] = 255 - vals['MIX BALANCE']
    s1['MIX_OSC2'] = vals['MIX BALANCE']
    s1['OSC2_MOD'] = PWM_SOURCE[vals['PWM SOURCE']]
    s1['LFO_AMP'] = 64 * vals['AMP LFO MOD'] + 32
    jp = {n: pid for n, pid, _, _ in JP08}
    for i, (nm, pid, cc, st, mx, df, ap) in enumerate(EXTEND):
        assert jp[nm] == pid, nm
        s1['PLUGOUT_EXTEND%02d' % (i + 1)] = (vals[nm] - df) & 0xFF
    return s1


def write_prm(path, s1, name):
    out = ''.join('%s(%d);\r\n' % (n, s1[n]) for n in S1_NAMES) + 'PATCH_NAME(%s);' % name
    open(path, 'wb').write(out.encode('latin-1'))


# KoaBankFile00002 (README.md §13.5, docs Vol. IV ch. 4 "Bank files"): name 16 + header 27 + struct "patch" 640:
# 64 int2x4 (a byte as two nibbles), the name as 16 x 2 nibbles in 128 bytes (NAME0), 48 int8x4 (32 bits, 8 nibbles)
HEADER = bytes.fromhex('010a00020100070604060406040604060406040604080000030200')   # a SH-2 bank record's (unread)
TAG = b'PG-JP08S1'


def bank_record(s1, name):
    m = bytearray()
    for n in S1_NAMES[:64]:
        v = s1[n] + (11 if n == 'OSC2_COARSE' else 0)    # the bank stores the unsigned value (measured, §13.5)
        m += bytes((v >> 4 & 15, v & 15))
    nm = bytearray()
    for ch in name.encode('latin-1')[:16].ljust(16):
        nm += bytes((ch >> 4, ch & 15))
    m += nm.ljust(128, b'\0')
    for n in S1_NAMES[64:]:
        v = s1[n] & 0xFFFFFFFF
        m += bytes((v >> (28 - 4 * j)) & 15 for j in range(8))
    assert len(m) == 640
    return name.encode('latin-1')[:16].ljust(16) + HEADER + bytes(m)


def inputs(args):
    out = []
    for a in args:
        if os.path.isdir(a):
            fs = [f for f in os.listdir(a) if re.fullmatch(r'JP08_PATCH(\d+)\.PRM', f, re.I)]
            out += [os.path.join(a, f) for f in sorted(fs, key=lambda f: int(re.search(r'(\d+)', f).group(1)))]
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
