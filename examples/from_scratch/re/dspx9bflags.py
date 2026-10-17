"""X9bY as "set condition group (X & 7) from the result of unit Y of the same instruction" (hypothesis from the
corpus: every X9bY is followed by C words on group X & 7, and its instruction has an op in unit Y). Master DSP3
main (DSPCTX=dsp3): for each value v, r1 = v, then the test instruction (default Roland's `1b03 8e09 b9b1`,
r0 = -r1), pads, then the 16 condition bits through C selects -> bank 5 (A200 + 16k + cc, DPC3 RAW view).

    python3 dspx9bflags.py [--ins "1b03 8e09 b9b1"] [--pads 1] [--values 0.5 -0.5 0 ...]
"""
import argparse, os, sys, time
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value
from dspmaster import Session, words_of
import dspasm as A

BANK5 = {'dsp3': 0x41122000, 'efdc': 0x41126000}[os.environ.get('DSPCTX', 'dsp3')]   # bank 5, RAW view
OUT = 0x200


REG = 1


def block(k, v, ins, pads, pre):
    P = [A.a('move', 0, imm=0.0, bits=16), A.a('move', REG, imm=v)] + [words_of(p) for p in pre] + [[1]]
    P += [words_of(ins)] + [[1]] * pads
    for half in (0, 1):
        for r in range(8):
            P.append(words_of(f'2d06 {r:x}050 {r:x}052 {2 * (8 * half + r):02x}12 0000 3000'))
        P += [A.st(r, 0xA000 + OUT + 16 * k + 8 * half + r) for r in range(8)]
    return P


def program(values, ins, pads, pre):
    P = [[0x0A02, 0xFF70]]
    for k, v in enumerate(values):
        P += block(k, v, ins, pads, pre)
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return [w for i in P for w in i]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--ins', default='1b03 8e09 b9b1')
    ap.add_argument('--pre', nargs='*', default=[], help='extra instructions after setting r0 = 0, r1 = v')
    ap.add_argument('--pads', type=int, default=1)
    ap.add_argument('--values', type=float, nargs='+', default=[0.5, -0.5, 0.0, 2.0, -2.0, 0.0625, -0.0625, 1e-6])
    ap.add_argument('--reg', type=int, default=1, help='register that gets v')
    a = ap.parse_args()
    global REG
    REG = a.reg
    with Session(Peek(), size=400 if os.environ.get('DSPCTX', 'dsp3') == 'dsp3' else 256) as ses:
        for b in range(0, len(a.values), 3):
            vals = a.values[b:b + 3]
            ses.load(program(vals, a.ins, a.pads, a.pre))
            time.sleep(0.2)
            reads = [ses.pk.read(BANK5 + 4 * OUT, 16 * len(vals)) for _ in range(3)]
            for k, v in enumerate(vals):
                rows = {''.join('1' if raw_value(r[16 * k + c]) == 1.0 else '0' if r[16 * k + c] == 0 else '?'
                                for c in range(16)) for r in reads}
                print(f'r{REG} = {v:+g}: bits 0..15 = {" / ".join(" ".join(p[i:i + 4] for i in range(0, 16, 4)) for p in sorted(rows))}',
                      flush=True)


if __name__ == '__main__':
    main()
