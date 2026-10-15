"""Fit the formula of an instruction from exact probes on a master DSP (dspprobe.py).

    python3 dspfit.py "0902 3{op}41" --op 0-7 -n 6        # {op} -> B nibble = op << 1 (X = r1 here)
    python3 dspfit.py "0803 3{op}41 2{op2}4" ...          # {op2}: op2 << 1 in bits 11-9 of w2

Operands: D = old r3 (destination r3), X = r1, Y/Z = r2, W = r4 (the template must use these).
For every op value, N probes with random r0..r7 from +-k/8, then least squares on monomials of
D, X, Z, W up to degree 3; prints the formula (coefficients rounded to 1/64, residual) and the
other registers the instruction changed.
"""
import argparse, itertools, os, random, sys
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value   # noqa: E402
import dspprobe                        # noqa: E402

VARS = {'D': 3, 'X': 1, 'Z': 2, 'W': 4}
ALLVARS = {'r0': 0, 'r1': 1, 'r2': 2, 'r3': 3, 'r4': 4, 'r5': 5, 'r6': 6, 'r7': 7}
VALUES = [k / 8 for k in range(-7, 8) if k] + [1.5, -1.5]


def monomials(deg=3):
    names = list(VARS)
    out = [()]
    for d in range(1, deg + 1):
        out += list(itertools.combinations_with_replacement(names, d))
    return out


def fit(rows, results, deg=3, maxterms=3):
    """smallest set of monomials (up to maxterms) that reproduces every result exactly"""
    mons = monomials(deg)
    A = np.array([[np.prod([r[VARS[v]] for v in m]) if m else 1.0 for m in mons] for r in rows])
    y = np.array(results)
    tol = 1e-4 * max(1.0, float(np.max(np.abs(y))))
    for k in range(0, maxterms + 1):
        for idx in itertools.combinations(range(len(mons)), k):
            if k == 0:
                if np.max(np.abs(y)) < tol:
                    return '0', 0.0
                continue
            sub = A[:, idx]
            coef, *_ = np.linalg.lstsq(sub, y, rcond=None)
            err = float(np.max(np.abs(sub @ coef - y)))
            if err < tol:
                terms = [f'{c:+.6g}' + ('·' + '·'.join(mons[i]) if mons[i] else '') for c, i in zip(coef, idx)]
                return ' '.join(terms), err
    return 'no formula with <= %d terms' % maxterms, float('nan')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('template')
    ap.add_argument('--op', default='0-7', help='values for {op} (B nibble = op << 1)')
    ap.add_argument('--op2', default='0', help='values for {op2} (w2 nibble = op2 << 1)')
    ap.add_argument('-n', type=int, default=7)
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--deg', type=int, default=3)
    ap.add_argument('--terms', type=int, default=3)
    ap.add_argument('--all', action='store_true', help='fit over r0..r7 (degree 2 is enough)')
    a = ap.parse_args()
    if a.all:
        VARS.clear(); VARS.update(ALLVARS)
    rng = lambda s: range(int(s.split('-')[0]), int(s.split('-')[-1]) + 1)
    random.seed(a.seed)
    sets = [[random.choice(VALUES) for _ in range(8)] for _ in range(a.n)]
    pk = Peek()
    for op in rng(a.op):
        for op2 in rng(a.op2):
            line = a.template.replace('{op}', f'{op << 1:x}').replace('{op2}', f'{op2 << 1:x}')
            rows, res, side = [], [], set()
            for regs in sets:
                after = [raw_value(w) for w in dspprobe.probe(pk, line.split(';'), regs)]
                rows.append(regs)
                res.append(after[3])
                side |= {f'r{i}' for i in range(8) if i != 3 and abs(after[i] - regs[i]) > 1e-9}
            f, err = fit(rows, res, a.deg, a.terms)
            if 'no formula' in f:
                f += '  results ' + ' '.join(f'{v:+.6g}' for v in res)
            print(f'{line:24s} r3 = {f:40s} (max err {err:.2g})' + (f'  also changes {sorted(side)}' if side else ''),
                  flush=True)


if __name__ == '__main__':
    main()
