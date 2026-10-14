"""probe(x, vx, others) -> what the JX-3P engine writes for x = vx with the
other parameters set first (reference measurements)."""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'engine'))
import ref, run_jx
from jxpatch import patch_params
D = {i: d for i, n, d in patch_params()}
_e = {}
def eng(slave=0):
    if slave not in _e:
        _e[slave] = ref.boot(D, slave)
    return _e[slave]
def probe(x, vx, others=None, slave=0):
    e = eng(slave)
    old = {}
    for y, vy in (others or {}).items():
        old[y] = e.vals.get(y, D.get(y, 0)); run_jx.setp(e, y, vy)
    e.log.clear(); run_jx.setp(e, x, vx)
    w = ref.last_writes(e, e.log)
    run_jx.setp(e, x, D.get(x, 0))
    for y, v in old.items():
        run_jx.setp(e, y, v)
    return w
