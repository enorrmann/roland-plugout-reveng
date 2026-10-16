"""Fit the formula of one instruction over r0..r7 (monomials up to degree 2) from probes with random registers
on master DSP3 (dspprobe). Prints the changed registers' formulas.

    python3 dspmacfit.py "0803 8c0a 9281" -n 14
"""
import argparse, itertools, os, random, sys
import numpy as np
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value   # noqa: E402
import dspprobe, dspmaster             # noqa: E402

VALUES = [k / 8 for k in range(-7, 8) if k]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ins')
    ap.add_argument('-n', type=int, default=14)
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--dump', action='store_true', help='print every probe (inputs -> changed registers)')
    ap.add_argument('--values', help='comma-separated pool of register values')
    a = ap.parse_args()
    rnd = random.Random(a.seed)
    global VALUES
    if a.values:
        VALUES = [float(v) for v in a.values.split(',')]
    mons = [()] + [(i,) for i in range(8)] + list(itertools.combinations_with_replacement(range(8), 2))
    rows, outs = [], []
    pk = Peek()
    with dspmaster.Session(pk, size=64) as ses:
        for _ in range(a.n):
            regs = [rnd.choice(VALUES) for _ in range(8)]
            ses.load(dspprobe.program([a.ins], regs))
            r = [raw_value(w) for w in ses.read(dspprobe.STORE, 8)]
            rows.append(regs); outs.append(r)
    if a.dump:
        for regs, r in zip(rows, outs):
            ch = ' '.join(f'r{d}={r[d]:+.8g}' for d in range(8) if abs(r[d] - regs[d]) > 1e-12)
            print('in ' + ' '.join(f'{v:+.4g}' for v in regs) + '  ->  ' + ch)
    X = np.array([[np.prod([reg[i] for i in m]) for m in mons] for reg in rows])
    for d in range(8):
        y = np.array([o[d] for o in outs])
        if np.allclose(y, [reg[d] for reg in rows]):
            continue
        best = None
        for k in (1, 2, 3):                     # sparse: smallest exact subset of monomials
            for sub in itertools.combinations(range(len(mons)), k):
                c, res, *_ = np.linalg.lstsq(X[:, sub], y, rcond=None)
                if np.abs(X[:, sub] @ c - y).max() < 1e-6:
                    best = (sub, c); break
            if best: break
        if best:
            terms = ' + '.join(f'{c:+.6g}·' + ('·'.join(f'r{i}' for i in mons[s]) or '1') for s, c in zip(*best))
            print(f'r{d} = {terms}')
        else:
            print(f'r{d}: no formula with <= 3 terms (values {y[:4]})')


if __name__ == '__main__':
    main()
