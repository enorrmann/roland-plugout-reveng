"""Hardware probes for the open items of DSP_ISA_DISCREPANCIES.md (2026-10-20), on master DSP3 via dspprobe.

    python3 review_probes.py            # run all, print expected / measured, write work/review_probes.json

Each probe is a list of instructions: ('asm', text) is assembled with asm/ (allow_inferred), ('hex', words)
is taken as it is. The registers start at dspprobe.REGS; the probe stores r0..r7 and prints what changed.
Needs the lab plugin installed in slot 3 and PLG3 selected (see DSP_ISA_DISCREPANCIES.md).
"""
import json, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(ROOT, 'asm'))
import dspprobe                       # noqa: E402
from dsppeek import Peek, raw_value   # noqa: E402
import asm, isa                       # noqa: E402

TABLE = isa.Table.load()
ASM = asm.Assembler(TABLE, allow_inferred=True)
REGS = dspprobe.REGS                  # r0..r7 = .5 .25 .125 -.375 .0625 -.75 .875 -.0625


def words(items):
    out = []
    for kind, v in items:
        if kind == 'asm':
            # a MAC has two classes in the table (0x040 and 0x098): pin the one the spec 5.2 example uses
            r = ASM.assemble('.class 0x0040\n' + v if v.startswith('mac') else v)
            assert not r.errors, (v, r.errors)
            out += r.words
        else:
            out += [int(t, 16) for t in v.split()]
    return ' '.join('%04x' % w for w in out)


def run(pk, name, items, expect, note=''):
    lines = [words(items)]
    after = dspprobe.probe(pk, lines, REGS)
    got = [raw_value(w) for w in after]
    reg = 1 if name.startswith('D8') else 0      # the 540c probes store the result in r1
    return {'probe': name, 'note': note, 'expect': expect, 'measured': got[reg],
            'reg': reg, 'match': abs(got[reg] - expect) < 1e-3, 'all_regs': got}


def main():
    pk = Peek()
    r1, r2, r3, r4, r5 = [REGS[i] for i in (1, 2, 3, 4, 5)]
    P = []
    # D1: MAC, Y a register, immediate coefficient read at byte C (spec 4.4). Three byte positions.
    for b in (2, 4, 6):
        P.append(('D1 coef at byte %d' % b, [('asm', 'mac_p1wc2 r0, r1, r2, #0.5@%d, r3' % b)],
                  r1 * r2 + 0.5 * r3, 'coefficient 0.5 stored at byte %d, C = %d' % (b, b)))
    # D3: bit 3 of a MAC Y (negation or not?)
    P.append(('D3 Y = r2', [('asm', 'mac_p1 r0, r1, r2, r3')], r1 * r2 + r3, ''))
    P.append(('D3 Y = -r2', [('asm', 'mac_p1 r0, r1, -r2, r3')], r1 * r2 + r3,
              'expect equal to the r2 case if bit 3 is not a negation (spec 5.2)'))
    # D5: unit A with D >= 8 in a multi-op class (class 0x0a8 = AA): r0 <- X + Y + r[op]
    for op in range(8):
        w = 0x8043 | op << 9
        P.append(('D5 AA op %d' % op, [('hex', '1503 %04x 4c42' % w)], r1 + r3 + REGS[op],
                  'r0 = r1 + r3 + r%d; second op addxy r4, r1, r2 = 4c42' % op))
    # D7: hidden-register Y is a byte (1) or a word (1): different words are read
    P.append(('D7 hset Y=1', [('hex', '0b04 4051 1234 5678'), ('hex', '0902 0140')],
              REGS[0] + REGS[0] + 0x3456 / 16384, 'byte reading: h5 from 3456; word reading would give 5678'))
    P.append(('D7 hset Y=0', [('hex', '0b04 4050 1234 5678'), ('hex', '0902 0140')],
              REGS[0] + REGS[0] + 0x1234 / 16384, 'control'))
    # D9: op1 6 with op2 = 000: no products (spec 7): r0 = X + W + Z, whatever the bit 9 says
    P.append(('D9 op1=6 bit9 set', [('asm', 'mac_p1p2 r0, r1, r2, r3, r4, r5')],
              r1 * r2 + r5 + r4 * r3, 'register coefficient product (bit 9 = 1)'))
    P.append(('D9 op1=6 bit9 clear', [('asm', 'mac_p1p2c0 r0, r1, r3, r5')],
              r1 + r5 + r3, 'no products: X + W + Z (spec 7); Y and C are not operands'))
    # D9: op1 4 with op2 = 000: both products kept (spec 7)
    P.append(('D9 op1=4 bit9 clear', [('hex', '0803 0842 3100')],
              r1 * r2 + r4 * r3, 'X*Y + C*Z with the table word of mac_p1wc0'))
    # D3 (W): bit 3 of a register W is a negation (spec 5.2): -r3 gives X*Y - r3
    P.append(('D3 W = r3', [('asm', 'mac_p1 r0, r1, r2, r3')], r1 * r2 + r3, 'W not negated'))
    P.append(('D3 W = -r3', [('asm', 'mac_p1 r0, r1, r2, -r3')], r1 * r2 - r3, 'W negated (bit 3 set)'))
    # D8 / D4: the spec's own 540c instruction (spec 4.4), coefficient word swept.
    # r1 = r0*Y + c*r7 + W with Y = 1.0 (byte 2), W = -4.7501221 (byte 11)
    W = -4.7501221
    for coef, c in (('2333 33c4', 0.2), ('2800 33c4', 0.5), ('2400 33c4', 0.25), ('2c00 33c4', 0.75)):
        P.append(('D8 540c coef %s' % coef, [('hex', '540c 1c12 af1b afe4 cfe6 7a2b 3000 0638 063d %s c008' % coef)],
                  REGS[0] + c * REGS[7] + W, 'r1 (not r0): coefficient read after Y at slot 4'))
    out = []
    for name, items, expect, note in P:
        r = run(pk, name, items, expect, note)
        r['note'] = note
        out.append(r)
        print('%-22s expect %+.6f  measured %+.6f  %s' % (name, expect, r['measured'],
                                                    'MATCH' if r['match'] else 'differs'))
    os.makedirs(os.path.join(ROOT, 'work'), exist_ok=True)
    json.dump(out, open(os.path.join(ROOT, 'work', 'review_probes.json'), 'w'), indent=1)


if __name__ == '__main__':
    main()
