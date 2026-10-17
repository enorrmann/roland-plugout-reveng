"""MXM latency with a common clock (2026-10-17): DSP3 sends its delay-ring write position (`f5c`, -1/16384 per
sample) on MXM Z 85F0; DSP4 (slot 0xF7DC) reads O 83F0 and its own `f5c`, stores the difference in samples.
If the position is a shared clock, the difference is the bus latency.
"""
import os, sys, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A

NOP = [1]


def prog3():
    P = [[0x0A02, 0xFF70], A.a('move', 1, imm=0.0, bits=16), NOP, [0x0A02, 0x9F5C], NOP, A.st(1, 0x85F0),
         A.st(1, 0xA3F8), [0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return [w for i in P for w in i]


def prog4():
    P = [[0x0A02, 0xFF70], A.a('move', 2, imm=0.0, bits=16), A.ld(1, 0x83F0), [0x0A02, 0xAF5C], NOP, NOP,
         A.a('sub', 3, 1, 2), A.st(3, 0xA3C8), A.st(2, 0xA3C9), A.st(1, 0xA3CA),   # own - received
         [0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return [w for i in P for w in i]


pk = Peek()
dspmaster.CONTEXT = 'dsp3'
with dspmaster.Session(pk, size=128) as s3:
    s3.load(prog3())
    dspmaster.CONTEXT = 'efdc'
    with dspmaster.Session(pk, 4, 0xF7DC, size=256) as s4:
        s4.load(prog4())
        time.sleep(0.4)
        for _ in range(6):
            d, own, rec = [raw_value(x) for x in pk.read(0x41126000 + 4 * 0x3C8, 3)]
            p3 = raw_value(pk.read(0x41122000 + 4 * 0x3F8, 1)[0])
            print(f'DSP4 own pos {own:.6f}, received DSP3 pos {rec:.6f}, own - received = {d:.8f} = {d * 16384:.3f} samples'
                  f'   (DSP3 pos read by ARM {p3:.6f})', flush=True)
            time.sleep(0.25)
dspmaster.CONTEXT = 'dsp3'
