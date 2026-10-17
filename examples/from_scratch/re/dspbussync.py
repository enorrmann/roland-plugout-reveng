"""Timing of the MXM bus between master DSP3 (main) and DSP4 (slot 0xF7DC), 2026-10-17. DSP3 writes its sample
counter to Z 85F0 at the start and to 85F1 at the end of its program (N no-ops apart); DSP4 reads O 83F0/83F1 at
the start and at the end of its program (N no-ops apart) and also its own copy of DSP3's counter is not available,
so differences between the four reads (in samples) tell whether the bus is live or latched per sample.

    python3 dspbussync.py [--nops 200]
"""
import argparse, os, sys, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A

NOP = [1]
INC = 2.0 ** -16
B3, B4 = 0x41122000, 0x41126000


def dsp3_prog(n):
    P = [[0x0A02, 0xFF70], A.ld(0, 0xA3F0), NOP, NOP, A.a('add', 0, 0, imm=INC), A.st(0, 0xA3F0), A.st(0, 0x85F0)]
    P += [NOP] * n + [A.st(0, 0x85F1), [0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return [w for i in P for w in i]


def dsp4_prog(n):
    P = [[0x0A02, 0xFF70], A.ld(1, 0x83F0), A.ld(2, 0x83F1)] + [NOP] * n + [A.ld(3, 0x83F0), A.ld(4, 0x83F1), NOP, NOP]
    P += [A.a('sub', 5, 1, 2), A.st(5, 0xA3C0),      # start: a - b   (85F0 written early, 85F1 late, by DSP3)
          A.a('sub', 5, 1, 3), A.st(5, 0xA3C1),      # a' - a          (does 83F0 change during DSP4's program?)
          A.a('sub', 5, 2, 4), A.st(5, 0xA3C2),      # b' - b
          A.a('sub', 5, 3, 4), A.st(5, 0xA3C3),      # end: a' - b'
          A.st(1, 0xA3C4), [0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return [w for i in P for w in i]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--nops', type=int, nargs='+', default=[10, 200])
    a = ap.parse_args()
    pk = Peek()
    for n in a.nops:
        dspmaster.CONTEXT = 'dsp3'
        with dspmaster.Session(pk, size=400) as s3:
            s3.load(dsp3_prog(n))
            dspmaster.CONTEXT = 'efdc'
            with dspmaster.Session(pk, 4, 0xF7DC, size=256) as s4:
                s4.load(dsp4_prog(n))
                time.sleep(0.4)
                seen = {}
                for _ in range(40):
                    d = tuple(round(raw_value(x) / INC, 2) for x in pk.read(B4 + 4 * 0x3C0, 4))
                    seen[d] = seen.get(d, 0) + 1
                print(f'nops {n}: (start a-b, a\'-a, b\'-b, end a\'-b\') in samples -> {seen}', flush=True)
        dspmaster.CONTEXT = 'dsp3'


if __name__ == '__main__':
    main()
