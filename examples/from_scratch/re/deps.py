"""Which parameters change what the JX-3P engine writes when parameter X
changes (reference measurement for the model's combined outputs).

usage: python3 re/deps.py out.json [slave]
"""
import json, os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'engine'))
import ref, run_jx
from jxpatch import patch_params

def writes(e, i, v):
    e.log.clear(); run_jx.setp(e, i, v)
    return ref.last_writes(e, e.log)

def main():
    slave = int(sys.argv[2]) if len(sys.argv) > 2 else 0
    D = {i: d for i, n, d in patch_params()}
    ids = [i for i in ref.refresh_ids(slave) if i not in ref.VOICE_IDS]
    e = ref.boot(D, slave)
    deps = {}
    for x in ids:
        base = {v: writes(e, x, v) for v in (1, 100, 200)}
        if not any(base.values()):
            continue
        found = {}
        for y in ids:
            if y == x:
                continue
            y0 = e.vals.get(y, 0)
            for vy in (0, 1, 3, 255):
                if vy == y0:
                    continue
                run_jx.setp(e, y, vy)
                for v in (1, 100, 200):
                    w = writes(e, x, v)
                    diff = [str(k) for k in set(w) | set(base[v])
                            if abs((w.get(k) or 0) - (base[v].get(k) or 0)) > 1e-6 or (k in w) != (k in base[v])]
                    if diff:
                        found.setdefault(hex(y), set()).update(diff)
                run_jx.setp(e, y, y0)
            writes(e, x, D.get(x, 0))
        if found:
            deps[hex(x)] = {y: sorted(t) for y, t in found.items()}
            print(hex(x), {y: len(t) for y, t in found.items()}, flush=True)
        run_jx.setp(e, x, D.get(x, 0))
    json.dump(deps, open(sys.argv[1], 'w'), indent=1)

main()
