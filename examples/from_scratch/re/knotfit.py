"""Least-squares knots (piecewise linear at given x, optionally in log2) for a
measured curve: prints the expression for engine/model_spec.py and its error.

usage: python3 re/knotfit.py meas.json ID TARGET [log|lin]
"""
import json, sys
import numpy as np

XS = [1, 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32, 40, 48, 64, 80, 96, 128, 160, 192, 224, 255]

def fit(c, log=True, xs=XS):
    lo = xs[0]
    v = np.arange(lo, 256, dtype=float)
    y = np.array(c[lo:], float)
    g = np.log2(y) if log else y
    xs = np.array(xs, float)
    A = np.zeros((len(v), len(xs)))
    for r, x in enumerate(v):
        k = min(int(np.searchsorted(xs, x, 'right') - 1), len(xs) - 2)
        t = (x - xs[k]) / (xs[k + 1] - xs[k])
        A[r, k] += 1 - t; A[r, k + 1] += t
    k, *_ = np.linalg.lstsq(A, g, rcond=None)
    k = np.float32(k).astype(float)
    f = A @ k
    f = 2 ** f if log else f
    err = np.max(np.abs(f / y - 1)) if log else np.max(np.abs(f - y)) / np.max(np.abs(y))
    return list(k), float(err)

if __name__ == '__main__':
    M = json.load(open(sys.argv[1]))['0']
    i, t = sys.argv[2], sys.argv[3]
    log = (sys.argv[4] if len(sys.argv) > 4 else 'log') == 'log'
    c = M[i]['out'][t]
    k, e = fit(c, log)
    s = f'interp(x, [{", ".join(f"{x:.7g}" for x in k)}])'
    print(f'v0={c[0]!r} ' + (f'exp2({s})' if log else s) + f'   # max {"rel " if log else ""}err {e:.3%}')
