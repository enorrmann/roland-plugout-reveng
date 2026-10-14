"""Measure the per-slave / per-state blocks of the model and print them as
literals for engine/model_spec.py (PART table, 0x737 table, effect curves)."""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from probe import probe
import knotfit
import numpy as np

def tname(k):
    return f'{k[0]}{k[1]}:{k[2]:#x}'

def part_table():
    ctx = {0x739: 100, 0x73a: 50, 0x73b: 200, 0x73c: 30}
    states = [{0x22c: 0, 0x519: 0}, {0x22c: 1, 0x519: 0}, {0x22c: 1, 0x519: 1}, {0x22c: 1, 0x519: 2}]
    rows = {}
    for s in (0, 1):
        for n, stt in enumerate(states):
            w = probe(0x22c, stt[0x22c], {**ctx, 0x519: stt[0x519]}, slave=s)
            for k, v in w.items():
                rows.setdefault(k, [[None] * 4, [None] * 4])[s][n] = round(v, 7)
    print('PART_TABLE = {  # target: ([state 0..3 slave 0], [.. slave 1]); None = not written')
    for k in sorted(rows):
        print(f'    {tname(k)!r}: {rows[k]},')
    print('}')

def t737():
    print('T737 = {  # target: (kind, slave 0, slave 1): pow3/pow4 -> k * ((v+1)/129)^n, drift -> 1 + a*(v+1)')
    for s_tab in [0]:
        pass
    res = {}
    for s in (0, 1):
        w0 = probe(0x737, 0, {0x744: 0}, slave=s); w128 = probe(0x737, 128, {0x744: 0}, slave=s)
        for k in w128:
            res.setdefault(k, [None, None])[s] = (w0[k], w128[k])
    for k in sorted(res):
        a = res[k]
        def cls(p):
            if p is None: return None
            y0, y128 = p
            if abs(y128 - 1) < 0.2 and y0 >= 1:
                return ('drift', float(f'{(y0 - 1):.7g}'))
            if y128 == 0: return ('zero', 0)
            r = y0 / y128
            n = 3 if abs(r - (1 / 129) ** 3) < abs(r - (1 / 129) ** 4) else 4
            return (f'pow{n}', float(f'{y128:.7g}'))
        print(f'    {tname(k)!r}: ({cls(a[0])}, {cls(a[1])}),')
    print('}')

def fx():
    for t in (2, 4, 5):
        for x in (0x6f9, 0x749):
            curves = {}
            for v in range(256):
                for k, y in probe(x, v, {0x748: t}).items():
                    curves.setdefault(k, [None] * 256)[v] = y
            for k, c in curves.items():
                y = np.array(c, float)
                print(f'# type {t} {x:#x} {tname(k)}: ' + ' '.join(f'{v}:{c[v]:.6g}' for v in (0, 1, 2, 64, 128, 192, 255)))
                if len(set(np.round(y, 6))) > 3:
                    log = bool(np.all(y[1:] > 0))
                    kk, e = knotfit.fit(c, log)
                    print(f"    ({t}, {x:#x}, {tname(k)!r}, {log}, [{', '.join(f'{z:.7g}' for z in kk)}]),  # err {e:.2%}")

if __name__ == '__main__':
    {'part': part_table, '737': t737, 'fx': fx}[sys.argv[1]]()
