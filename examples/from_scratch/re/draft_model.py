"""Draft of the engine's parameter model from the measured responses
(re/measure.py): one expression per (source id, curve), targets grouped.
Prints Python for engine/model_spec.py, to be curated by hand.

usage: python3 re/draft_model.py meas.json > draft.py
"""
import json, math, os, sys
import numpy as np

TOL_A, TOL_R = 2e-6, 2e-5


def ok(f, y):
    return bool(np.all(np.abs(f - y) <= TOL_A + TOL_R * np.abs(y)))


def num(x):
    for d in (255, 254, 128, 127, 100, 64, 32, 12, 24, 1):
        if abs(x * d - round(x * d)) < 1e-6 * max(1, abs(x * d)):
            n = round(x * d)
            return f'{n}' if d == 1 else f'{n} / {d}'
    return repr(float(np.float32(x)))


def fit(c, pid):
    pts = [(v, y) for v, y in enumerate(c) if y is not None]
    v = np.array([p[0] for p in pts], float)
    y = np.array([p[1] for p in pts], float)
    p = f'P({pid})'
    if np.allclose(y, y[0], atol=TOL_A):
        return 'const', num(y[0])
    if len(pts) <= 8 and v[0] == 0 and list(v) == list(range(len(v))):
        return 'pick', f'pick({p}, [{", ".join(num(t) for t in y)}])'
    a, b = np.polyfit(v, y, 1)
    if ok(a * v + b, y):
        return 'lin', f'{p} * ({num(a)})' + (f' + {num(b)}' if abs(b) > 1e-9 else '')
    # threshold
    lo, hi = y[0], y[-1]
    if set(np.round(y, 7)) <= {round(lo, 7), round(hi, 7)}:
        t = int(v[np.argmax(np.round(y, 7) == round(hi, 7))])
        return 'thr', f'sel({p} >= {t}, {num(hi)}, {num(lo)})'
    for k in (y[-1], -y[0]):
        bip = np.where(v < 128, (v - 128) / 128 * k, (v - 128) / 127 * k)
        if ok(bip, y):
            return 'bip', f'bip({p}, {num(k)})'
        if ok(np.abs(bip), y):
            return 'absbip', f'eabs(bip({p}, {num(k)}))'
    mid = (y > y.min() + 1e-9) & (y < y.max() - 1e-9)
    if mid.sum() >= 3:
        a, b = np.polyfit(v[mid], y[mid], 1)
        if ok(np.clip(a * v + b, y.min(), y.max()), y):
            return 'clin', f'clamp({p} * ({num(a)}) + {num(b)}, {num(y.min())}, {num(y.max())})'
    pos = y > 0
    if pos.sum() > 10:
        s1, s0 = np.polyfit(v[pos], np.log2(y[pos]), 1)
        f = np.clip(2 ** (s0 + s1 * v), y.min(), y.max())
        if ok(f, y):
            return 'exp2', (f'clamp({num(2 ** s0)} * exp2({p} / {num(1 / s1)}), '
                            f'{num(y.min())}, {num(y.max())})')
    # (v + 1) / 129 to a small power, scaled
    for n in (2, 3, 4):
        g = ((v + 1) / 129) ** n
        k = float(np.dot(g, y) / np.dot(g, g))
        if ok(k * g, y):
            return 'pow', f'{num(k)} * ' + ' * '.join([f'(({p} + 1) / 129)'] * n)
    # fallback: least-squares knots (log2 domain when positive), v = 0 apart
    if len(pts) == 256:
        sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
        import knotfit
        log = bool(np.all(y[1:] > 0))
        k, err = knotfit.fit(c, log)
        body = f'interp({p}, [{", ".join(f"{x:.7g}" for x in k)}])'
        body = f'exp2({body})' if log else body
        return 'knots', f'sel({p} >= 1, {body}, {num(y[0])})   # max {"rel " if log else ""}err {err:.2%}'
    return 'table', None


def main():
    M = json.load(open(sys.argv[1]))
    src = {}
    for i, x in M['0'].items():
        for k in x['out']:
            src.setdefault(k.split('/')[0], set()).add(i)
    for i, x in M['0'].items():
        if not x['out']:
            continue
        uniq = {}
        for k, c in x['out'].items():
            uniq.setdefault(tuple(None if y is None else round(y, 7) for y in c), []).append(k)
        same = M['1'][i]['out'] == x['out']
        print(f'    # {i}' + ('' if same else '  (slave 1 differs)') + (f'  other OS calls {x["other"]}' if x['other'] else ''))
        for c, ks in uniq.items():
            kind, e = fit(c, i)
            combo = sorted(set().union(*(src[k.split('/')[0]] for k in ks)) - {i})
            tg = ' '.join(ks)
            note = f'  # also written by {", ".join(combo)}' if combo else ''
            print(f'    out({tg!r}, {e}),{note}')


if __name__ == '__main__':
    main()
