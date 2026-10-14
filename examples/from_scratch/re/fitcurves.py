"""Fit the measured responses (re/measure.py) with simple formula families."""
import json, math, sys
import numpy as np

def fits(c):
    pts = [(v, y) for v, y in enumerate(c) if y is not None]
    v = np.array([p[0] for p in pts], float); y = np.array([p[1] for p in pts], float)
    tol = lambda f: np.all(np.abs(f - y) <= 2e-6 + 2e-5 * np.abs(y))
    if len(pts) <= 8:
        return ('steps', [round(float(t), 7) for t in y])
    if tol(np.full_like(y, y[0])):
        return ('const', float(y[0]))
    a, b = np.polyfit(v, y, 1)
    if tol(a * v + b):
        return ('lin', float(a), float(b))
    # bipolar around 128: (v-128)/128*k below, (v-128)/127*k above
    k = y[-1] if v[-1] == 255 else None
    if k is not None:
        f = np.where(v < 128, (v - 128) / 128 * k, (v - 128) / 127 * k)
        if tol(f):
            return ('bip', float(k))
    # clamped linear
    for lo in sorted(set(y)):
        pass
    # exp2 with clamps: y = a * 2^(v/s)
    pos = y > 0
    if pos.sum() > 10:
        lv = np.log2(y[pos]); s1, s0 = np.polyfit(v[pos], lv, 1)
        f = np.clip(2 ** (s0 + s1 * v), y.min(), y.max())
        if tol(f):
            return ('exp2', float(2 ** s0), float(1 / s1), float(y.min()), float(y.max()))
    # power of v/255
    if pos.sum() > 10 and y[0] == 0:
        lp = np.polyfit(np.log(v[pos] / 255), np.log(y[pos]), 1)
        f = np.exp(lp[1]) * (v / 255) ** lp[0]
        if tol(f):
            return ('pow', float(np.exp(lp[1])), float(lp[0]))
    # clamped linear: lin on the non-saturated part
    for lo_i in (0,):
        ym, yM = y.min(), y.max()
        mid = (y > ym + 1e-9) & (y < yM - 1e-9)
        if mid.sum() >= 3:
            a, b = np.polyfit(v[mid], y[mid], 1)
            f = np.clip(a * v + b, ym, yM)
            if tol(f):
                return ('clin', float(a), float(b), float(ym), float(yM))
    return ('table', None)

if __name__ == '__main__':
    M = json.load(open(sys.argv[1]))
    kinds = {}
    for i, x in M['0'].items():
        uniq = {}
        for k, c in x['out'].items():
            uniq.setdefault(tuple(None if y is None else round(y, 6) for y in c), []).append(k)
        for c, ks in uniq.items():
            f = fits(c)
            kinds.setdefault(f[0], []).append((i, ks[0]))
            if f[0] == 'table':
                pts = [(v, y) for v, y in enumerate(c) if y is not None]
                print('TABLE', i, ks, ' '.join(f'{v}:{y:.5g}' for v, y in pts[::16]))
    print({k: len(v) for k, v in kinds.items()})
