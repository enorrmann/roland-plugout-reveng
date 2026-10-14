"""Reference only (tests): the JX-3P engine in emu1 as a black box.

state(vals, slave) -> {('P'|'B', dsp, id): value}: last value written to each
DSP parameter after start-up (vt+8, vt+0xC, vt+0x24(1)) with the OS returning
`vals` for every get. ('P' = parameter block +0xA0/+0xA4, 'B' = bank 5 +0xB0.)
"""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 're'))
import run_jx
from emu1 import Emu1
Emu1.PREC = None                       # full float32 values
run_jx.IMG = os.path.join(HERE, 'build', 'jx', 'region1.bin')   # from programs.py

VOICE_IDS = set(range(0x429, 0x453)) | {0x45C}


def last_writes(e, log, out=None):
    out = {} if out is None else out
    for off, r0, r1, r2, r3, s0, st in log:
        if off in (0xA0, 0xA4):
            out[('P', r0, r1)] = s0
        elif off == 0xB0:
            out[('B', r0, e.u32(r1))] = s0
    return out


def boot(vals, slave):
    e = run_jx.boot(slave)
    e.vals.update(vals)
    e.log.clear()
    e.call(e.u32(e.vt + 8), e.o)
    e.call(e.u32(e.vt + 0xC), e.o)
    e.call(e.u32(e.vt + 0x24), e.o, 1)
    return e


def state(vals, slave):
    e = boot(vals, slave)
    return last_writes(e, e.log)


def refresh_ids(slave=0):
    """ids the engine reads (OS +0x00) during a full refresh"""
    e = boot({}, slave)
    e.log.clear()
    e.call(e.u32(e.vt + 0x24), e.o, 1)
    seen = []
    for off, r0, r1, r2, r3, s0, st in e.log:
        if off == 0 and r2 not in seen:
            seen.append(r2)
    return seen
