"""Nonlinear exact fit of measured instructions (raw probes from dspclass.py --raw).

    python3 dspnlfit.py work/dspclass/bunit_raw.jsonl [--ops D=3,X=1,Y=2]

For every output that changed: the smallest sum of up to 3 terms c * atom (c from least squares) that
reproduces every probe exactly. Atoms: 1, r0..r7, and over the operands D, X, Y: |a|, min(a,b), max(a,b),
a*b, sign(a), step(a) (a >= 0), |a|*b.
"""
import argparse, itertools, json
import numpy as np

WRAP = False


def atoms(R, ops):
    A = {'1': np.ones(len(R))}
    for i in range(8):
        A[f'r{i}'] = R[:, i]
    names = list(ops)
    col = {n: R[:, ops[n]] for n in names}
    for a in names:
        A[f'|{a}|'] = np.abs(col[a])
        A[f'sign({a})'] = np.sign(col[a])
        A[f'step({a})'] = (col[a] >= 0).astype(float)
    for a, b in itertools.combinations(names, 2):
        A[f'min({a},{b})'] = np.minimum(col[a], col[b])
        A[f'max({a},{b})'] = np.maximum(col[a], col[b])
    for a, b in itertools.combinations_with_replacement(names, 2):
        A[f'{a}*{b}'] = col[a] * col[b]
    for a in names:
        for b in names:
            if a != b:
                A[f'|{a}|*{b}'] = np.abs(col[a]) * col[b]
                A[f'{a}*step({b})'] = col[a] * (col[b] >= 0)
                A[f'step({a}-{b})'] = (col[a] - col[b] >= 0).astype(float)
                A[f'{a}*step({a}-{b})'] = col[a] * (col[a] - col[b] >= 0)
    return A


def fit(A, y, maxterms=3):
    keys = list(A)
    M = np.stack([A[k] for k in keys], axis=1)
    tol = 1e-6 * max(1.0, float(np.max(np.abs(y))))
    for k in range(1, maxterms + 1):
        for idx in itertools.combinations(range(len(keys)), k):
            sub = M[:, idx]
            c, *_ = np.linalg.lstsq(sub, y, rcond=None)
            if np.max(np.abs(sub @ c - y)) < tol:
                return ' '.join(f'{v:+.6g}*{keys[i]}' for v, i in zip(c, idx) if abs(v) > 1e-12).replace('*1 ', ' ').rstrip('*1')
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('raw')
    ap.add_argument('--ops', default='D=3,X=1,Y=2')
    ap.add_argument('--wrap', action='store_true', help='read results > 256 as v - 512')
    a = ap.parse_args()
    global WRAP
    WRAP = a.wrap
    ops = {k: int(v[1:] if v.startswith('r') else v) for k, v in (t.split('=') for t in a.ops.split(','))}
    for line in open(a.raw):
        rec = json.loads(line)
        X, Y = np.array(rec['X']), np.array(rec['Y'])
        if WRAP:
            Y = np.where(Y > 256, Y - 512, Y)          # results >= 256 read as wrapped negatives (v - 512)
        R = X[:, :8]
        A = atoms(R, ops)
        res = []
        for j in range(8):
            if np.max(np.abs(Y[:, j] - X[:, j])) > 1e-9:
                bad = np.abs(Y[:, j] - X[:, j]) > 1e-9
                if bad.sum() <= 1:
                    continue                      # one bad read of an unchanged register
                f = fit(A, Y[:, j])
                if f is None:                     # try without one probe (a bad read)
                    for i in range(len(R)):
                        keep = [q for q in range(len(R)) if q != i]
                        f = fit({k: v[keep] for k, v in A.items()}, Y[keep, j])
                        if f:
                            f += ' ~'
                            break
                res.append(f'r{j} = {f or "? " + " ".join(f"{v:+.4g}" for v in Y[:, j])}')
        print(f'{" ".join(rec["ins"]):24s} ' + ('; '.join(res) or 'no change'), flush=True)


if __name__ == '__main__':
    main()
