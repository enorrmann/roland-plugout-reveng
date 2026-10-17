"""Which ops of a real Roland instruction run, for chosen flag values (2026-10-17). Master DSP3: r0 = 0, each
`--set G=v` does cmp r0 + v into group G (1..5), two no-ops, then r1..r7 = distinct values, the instruction, and
r0..r7 are read back. Memory references are moved to scratch addresses 0x700.. (dspverify.layout).

    python3 dspcreal.py "b247 3044 3045 ee0f 1220 0e14 b9b3" --set 1=1 2=-1 [--set ...]   (several --set = cases)
"""
import argparse, os, sys
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value
from dspmaster import Session
import dspasm as A, dspverify

VALS = [0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875]


NREF = [0]


def program(ins, sets):
    P = [[0x0A02, 0xFF70]]
    for k in range(NREF[0]):                       # references preloaded with (k + 1) / 32
        P += [A.a('move', 7, imm=(k + 1) / 32, bits=16), A.st(7, 0x700 + k)]
    P += [A.a('move', 0, imm=0.0, bits=16)]
    for g, v in sets:
        P.append([0x0D8 << 5 | 4, 0x8E10, (8 + g) << 12 | 0x9B1, A.raw(v) >> 16 if v else 0])
    P += [[1], [1]] + [A.a('move', r, imm=VALS[r], bits=16) for r in range(1, 8)] + [[1], ins, [1], [1]]
    P += [A.st(r, 0x780 + r) for r in range(8)] + [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return [w for i in P for w in i]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ins')
    ap.add_argument('--set', nargs='+', action='append', default=[])
    a = ap.parse_args()
    ins = [int(x, 16) for x in a.ins.split()]
    _, refs = dspverify.layout(ins)
    for k, i in enumerate(refs):
        ins[i] = 0x700 + k
    NREF[0] = len(refs)
    with Session(Peek(), size=400) as ses:
        for case in a.set:
            sets = [(int(s.split('=')[0]), float(s.split('=')[1])) for s in case]
            ses.load(program(ins, sets))
            got = [raw_value(x) for x in ses.read(0x780, 8)]
            ch = ' '.join(f'r{r}={got[r]:g}' for r in range(8) if abs(got[r] - VALS[r]) > 1e-9)
            print(f'{" ".join(f"g{g}={v:+g}" for g, v in sets):24s} -> {ch or "no change"}', flush=True)


if __name__ == '__main__':
    main()
