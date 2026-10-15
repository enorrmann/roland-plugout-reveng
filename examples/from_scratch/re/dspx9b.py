"""Test whether X9bY writes the condition bits (agent hypothesis): on master DSP3, r3 = v, `1803 0040 b9bP`
(port P), `pad` no-ops, then a C select on condition cc: r1 = 0 if the condition holds, 1 if not.

    python3 dspx9b.py --port 1 --conds 4 5 6 7 --pads 0 1 2
"""
import argparse, os, sys, time
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value
from dspmaster import Session, words_of
import dspprobe

PREFIX = '1803 0040 b9b{port:x}'
VALUES = [0.5, -0.5, 0.0, 2.0, -2.0, 0.0625, -0.0625]


def program(v, port, pad, cc):
    regs = [0, 0, v, v, 0, 0, 0, 0]          # r2 = r3 = v
    L = [f'0903 {r:x}050 {dspprobe.hi16(x):04x}' for r, x in enumerate(regs)]
    L += [PREFIX.format(port=port)] + ['0001'] * pad
    L += [f'2d06 1050 1052 {cc << 1:02x}12 3000 0000']       # true: skip op1 -> r1 = 0 ; false: skip op2 -> r1 = 1
    L += [f'0c03 {8 + r:x}fe0 {0x780 + r:04x}' for r in range(8)]
    return words_of('\n'.join(L))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=1)
    ap.add_argument('--prefix', default=None, help='instruction with {port}, e.g. "1b03 be42 b9b{port:x}"')
    ap.add_argument('--conds', type=int, nargs='+', default=[4, 5, 6, 7])
    ap.add_argument('--pads', type=int, nargs='+', default=[0, 1, 2])
    a = ap.parse_args()
    global PREFIX
    if a.prefix: PREFIX = a.prefix
    with Session(Peek()) as ses:
        for pad in a.pads:
            for cc in a.conds:
                row = []
                for v in VALUES:
                    ses.load(program(v, a.port, pad, cc))
                    r1 = raw_value(ses.read(0x780, 2)[1])
                    row.append('T' if r1 == 0 else 'F' if abs(r1 - 1) < 1e-9 else '?')
                print(f'port {a.port} distance {pad + 1} cond {cc:2d}: ' +
                      ' '.join(f'{v:+g}:{c}' for v, c in zip(VALUES, row)), flush=True)


if __name__ == '__main__':
    main()
