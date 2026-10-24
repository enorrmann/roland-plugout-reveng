"""A component alone on one emulated DSP (docs/MUTABLE_SDK.md §6): the port measured without a host program.

    import bench
    out, info = bench.run(SawSwarm(), note=69, knobs={'ratio': 128, 'hp': 0}, n=9600)

Layout of the bench program: pitch word at bank 5 A030 (written as the ARM writes it, float32), the component's
parameters from C040 up (in the order of Component.params), its state words from bank-0 0x0100 up (2 words each:
the ring pair S / S+1), its outputs stored to bank 5 A1F0 up. inc = the host's increment of the pitch
(Kernel.increment). The program is scheduled with asm/dspsched.py (sched=False: plain, one unit per instruction) and
runs on the C core of the emulator (examples/from_scratch/emu/cchain.py).
"""
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
for p in (os.path.join(ROOT, 'asm'), os.path.join(ROOT, 'examples', 'from_scratch', 're'),
          os.path.join(ROOT, 'examples', 'from_scratch', 'emu')):
    if p not in sys.path:
        sys.path.insert(0, p)
import asm  # noqa: E402
import dspsched  # noqa: E402

import mi  # noqa: E402
from kernel import Kernel  # noqa: E402

PITCH = 0xA030
OUT = 0xA1F0
PARAM0 = 0x40
STATE0 = 0x0100


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def layout(comp):
    """{param name: first C0xx index}, {state name: ring address}"""
    prm, i = {}, PARAM0
    for p in comp.params:
        prm[p.name] = i
        i += p.count
    st = {s: STATE0 + 2 * j for j, s in enumerate(comp.state)}
    return prm, st


def lines(comp, regs=(0, 1, 2, 3, 4, 5, 6, 7)):
    prm_at, st_at = layout(comp)
    k = Kernel(f'bench: {comp.name}', regs=regs)
    ins = {}
    if 'inc' in comp.inputs:
        ins['inc'] = k.increment(k.load(PITCH, 'pitch'))
    prm = {name: (lambda base: (lambda i: k.param(base + i)))(base) for name, base in prm_at.items()}
    st = {name: k.state(a, name) for name, a in st_at.items()}
    outs = comp.kernel(k, ins, prm, st)
    for j, name in enumerate(comp.outputs):
        k.store(outs[name], OUT + j)
    return k.lines()


def program(src, sched=True):
    code = dspsched.schedule(src) if sched else [x for x in src if x.strip() and not x.startswith(';')]
    r = asm.Assembler().assemble('\n'.join(code) + '\n')
    if not r.ok:
        raise SystemExit(r.errors)
    return [0x0A02, 0xFF71] + r.words + [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001], code


def run(comp, note, knobs, n, sched=True):
    """-> (list of n output samples of the first output, info dict)"""
    import chain
    import cchain
    src = lines(comp)
    words, code = program(src, sched)
    sl = cchain.Slave({0: (chain.Program(words, comp.name), [])}, chain.Model(order=(0,), own_params=(0,)))
    prm_at, _ = layout(comp)
    for p in comp.params:
        for i, x in enumerate(p.values(knobs.get(p.name, 128))):
            sl.d[0].params[0xC000 + prm_at[p.name] + i] = f32(x)
    sl.d[0].bank5[PITCH & 0x3FF] = f32(mi.pitch_word(note))
    out = sl.run(n, ((0, OUT),))[(0, OUT)]
    plain = sum(1 for x in src if x.strip() and not x.startswith(';') and not dspsched.FILLER.match(x))
    return out, {'instructions': len(code), 'units': plain, 'program': code}
