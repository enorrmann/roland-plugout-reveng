"""Snapshot the 16 condition bits of the C words on a DSP context (DSPCTX, default efdc here).

    DSPCTX=efdc python3 dspcond.py [-n 200]

Program: for each condition index k (two programs: 0-7, 8-15), r(k mod 8) = cond_k ? 1 : 0 through
`2d06 r=0 r=1 <2k>12 0000 3000` (op 2 runs iff the condition holds), then r0..r7 -> bank 0. Many single reads
(no majority) give snapshots of the condition bits at different times; prints how often each pattern appears.
"""
import argparse, collections, os, sys, time
os.environ.setdefault('DSPCTX', 'efdc')
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value       # noqa: E402
from dspmaster import Session, words_of, bank0   # noqa: E402


def program(group):
    L = ['0a02 ff70']
    for r in range(8):
        k = group * 8 + r
        L.append(f'2d06 {r:x}050 {r:x}052 {2 * k:02x}12 0000 3000')
    L += [f'0c03 {8 + r:x}fe0 {0x780 + r:04x}' for r in range(8)]
    L += ['0a02 ff70', '0b02 00f8', '0001']
    return words_of('\n'.join(L))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('-n', type=int, default=200)
    a = ap.parse_args()
    with Session(Peek()) as ses:
        for g in (0, 1):
            ses.load(program(g))
            time.sleep(0.1)
            pats = collections.Counter()
            bad = 0
            for _ in range(a.n):
                w = ses.pk.read(bank0(ses.dsp, 0x780), 8)
                v = [raw_value(x) for x in w]
                if any(x not in (0.0, 1.0) for x in v):
                    bad += 1
                    continue
                pats[''.join(str(int(x)) for x in v)] += 1
            print(f'conditions {8 * g}-{8 * g + 7} (bit k = condition {8 * g}+k), {a.n - bad} good reads:')
            for p, c in pats.most_common():
                print(f'   {p}  {c}')


if __name__ == '__main__':
    main()
