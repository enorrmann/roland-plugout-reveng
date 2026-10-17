"""IOMD (bank 3) between master DSPs (2026-10-17): DSP3 stores salted markers to the words that slave programs use
as outputs (20-27, 60-67, B0-B7); DSP4 (slot 0xF7DC) loads the words they use as inputs (00-07, 10-17, E0-E3) and
the output words; the ARM reads the IOMD RAW block. Shows whether IOMD links DSPs or only goes out.
"""
import os, random, sys, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A

NOP = [1]
OUTS = list(range(0x20, 0x28)) + list(range(0x60, 0x68)) + list(range(0xB0, 0xB8))
INS = list(range(0x00, 0x08)) + list(range(0x10, 0x18)) + list(range(0xE0, 0xE4))
SALT = random.randint(1, 255) / 2 ** 14


def mk(w):
    return SALT + w / 512


def p3():
    P = [[0x0A02, 0xFF70]]
    for w in OUTS:
        P += [A.a('move', 1, imm=mk(w)), A.st(1, 0x6000 + w)]
    return [x for i in P + [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]] for x in i]


def p4():
    P = [[0x0A02, 0xFF70]]
    for k, w in enumerate(INS + OUTS):
        P += [A.ld(1, 0x6000 + w), NOP, NOP, A.st(1, 0xA380 + k)]
    return [x for i in P + [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]] for x in i]


def who(v):
    for w in OUTS:
        if abs(v - mk(w)) < 1e-7:
            return f'DSP3 out {w:02x}'
    return f'{v:+.5f}'


pk = Peek()
dspmaster.CONTEXT = 'dsp3'
with dspmaster.Session(pk, size=400) as s3:
    s3.load(p3())
    dspmaster.CONTEXT = 'efdc'
    with dspmaster.Session(pk, 4, 0xF7DC, size=256) as s4:
        s4.load(p4())
        time.sleep(0.4)
        got = [raw_value(x) for x in pk.read(0x41126000 + 4 * 0x380, len(INS + OUTS))]
        print('DSP4 reads:', ' '.join(f'{w:02x}={who(v)}' for w, v in zip(INS + OUTS, got)))
        arm = [raw_value(x) for x in pk.read(0x4113E000, 0x100)]
        print('ARM IOMD RAW words holding DSP3 markers:', [f'{i:02x}<-{who(v)}' for i, v in enumerate(arm) if 'DSP3' in who(v)])
dspmaster.CONTEXT = 'dsp3'
