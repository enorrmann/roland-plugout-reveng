"""Is MXM a mixing bus? (2026-10-17) DSP3 writes a to S 87F0 (word 0x1F0 of MXM5) and DSP4 writes b to O 83F0 (the
same word); both read the word back (DSP3 as S, DSP4 as O) and the ARM reads MXM5 word 0x1F0. Also DSP3 writes the
same S word twice in one sample (a, then a2).
"""
import os, sys, time, struct
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A

NOP = [1]


def p3(a, a2=None):
    P = [[0x0A02, 0xFF70], A.ld(2, 0x87F0), NOP, NOP, A.st(2, 0xA3E0), A.a('move', 1, imm=a, bits=16), A.st(1, 0x87F0)]
    if a2 is not None:
        P += [A.a('move', 1, imm=a2, bits=16), A.st(1, 0x87F0)]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return [w for i in P for w in i]


def p4(b):
    P = [[0x0A02, 0xFF70], A.ld(2, 0x83F0), NOP, NOP, A.st(2, 0xA3E0)]
    if b is not None:
        P += [A.a('move', 1, imm=b, bits=16), A.st(1, 0x83F0)]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return [w for i in P for w in i]


pk = Peek()
dspmaster.CONTEXT = 'dsp3'
with dspmaster.Session(pk, size=128) as s3:
    dspmaster.CONTEXT = 'efdc'
    with dspmaster.Session(pk, 4, 0xF7DC, size=256) as s4:
        for a, a2, b in ((0.25, None, 0.125), (0.25, None, None), (0.25, 0.5, None), (0.375, None, 0.0625)):
            dspmaster.CONTEXT = 'dsp3'; s3.load(p3(a, a2))
            dspmaster.CONTEXT = 'efdc'; s4.load(p4(b))
            time.sleep(0.4)
            r3 = raw_value(pk.read(0x41122000 + 4 * 0x3E0, 1)[0])
            r4 = raw_value(pk.read(0x41126000 + 4 * 0x3E0, 1)[0])
            arm = struct.unpack('<f', struct.pack('<I', pk.read(0x41730000 + 4 * 0x1F0, 1)[0]))[0]
            print(f'DSP3 writes S={a}{"," + str(a2) if a2 is not None else ""}, DSP4 writes O={b}: DSP3 reads S {r3:g}, '
                  f'DSP4 reads O {r4:g}, ARM MXM5[0x1f0] {arm:g}', flush=True)
dspmaster.CONTEXT = 'dsp3'
