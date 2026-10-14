"""Measure the JX-3P engine's DSP writes for every parameter value (reference
data for our own parameter model; nothing of it is built into the PLUG-OUT).

usage: python3 re/measure.py engine/build/proj/region1.bin out.json
"""
import json, os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE); sys.path.insert(0, os.path.join(HERE, '..', 'engine'))
import ref, run_jx
run_jx.IMG = sys.argv[1]
import gen_tables as G
from jxpatch import patch_params

def sweep(slave):
    e = G.start(slave)
    ids = [i for i, n, d in patch_params()] + [i for g, i in G.external_gets(slave)
                                               if i not in G.NOTE_IDS]
    defaults = {i: d for i, n, d in patch_params()}
    out = {}
    for i in ids:
        res, other = {}, set()
        for v in range(256):
            e.log.clear()
            run_jx.setp(e, i, v)
            for off, r0, r1, r2, r3, s0, st in e.log:
                if off in (0xA0, 0xA4):
                    k = f'P{r0}:{r1:#x}' + (f'/r{r2}f{r3}' if off == 0xA0 else '')
                elif off == 0xB0:
                    k = f'B{r0}:{e.u32(r1):#x}'
                elif off in (0x00, 0xF0):
                    continue
                else:
                    other.add(hex(off)); continue
                res.setdefault(k, [None] * 256)[v] = s0
        run_jx.setp(e, i, defaults.get(i, 0))
        out[hex(i)] = {'out': res, 'other': sorted(other)}
    return out

json.dump({s: sweep(s) for s in (0, 1)}, open(sys.argv[2], 'w'))
