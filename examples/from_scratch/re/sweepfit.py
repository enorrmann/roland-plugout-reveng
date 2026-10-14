"""Sweep one parameter 0..255 in a context and print fitted model lines.

usage: python3 re/sweepfit.py ID [ctx_id=value ...] [slave=N]
"""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from probe import probe
from draft_model import fit

def sweep(x, ctx, slave=0, rng=range(256)):
    curves = {}
    for v in rng:
        for k, y in probe(x, v, ctx, slave).items():
            curves.setdefault(k, [None] * 256)[v] = y
    return curves

def lines(x, ctx, slave=0):
    curves = sweep(x, ctx, slave)
    uniq = {}
    for k, c in curves.items():
        uniq.setdefault(tuple(None if y is None else round(y, 7) for y in c), []).append(f'{k[0]}{k[1]}:{k[2]:#x}')
    out = []
    for c, ks in uniq.items():
        kind, e = fit(list(c), x)
        out.append(f'    out({" ".join(ks)!r}, {e}),')
    return out

if __name__ == '__main__':
    x = int(sys.argv[1], 16); ctx = {}; slave = 0
    for a in sys.argv[2:]:
        k, v = a.split('=')
        if k == 'slave': slave = int(v)
        else: ctx[int(k, 16)] = int(v)
    print('\n'.join(lines(x, ctx, slave)))
