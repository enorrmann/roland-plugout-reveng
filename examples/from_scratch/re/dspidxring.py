"""Is the indexed transfer (mode 28, h6) ring-relative like normal bank-0 references? On master DSP3: a counter
kept with indexed transfers, c = [h6 + a] + INC -> [h6], for a = 0 and a = 1; ring-relative addressing counts only
with a = 1 (96000/s), absolute addressing only with a = 0. Output to bank 5 (DPC3 RAW).

    python3 dspidxring.py
"""
import os, sys, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value   # noqa: E402
import dspmaster, dspasm as A          # noqa: E402

NOP = [0x0001]
INC = 2 ** -16
O, RD = 0xA3C0, 0x41122000


def prog(a, idx=0x3E0):
    P = [[0x0A02, 0xFF70], A.a('move', 0, imm=idx / 16384), NOP, [0x0B02, 0x4080],
         [0x0C02, 0x0280 | a], NOP, NOP, A.a('add', 0, 0, imm=INC), [0x0C02, 0x8280], A.st(0, O),
         [0x0A02, 0xFF70], [0x0B02, 0x00F8], [0x0001]]
    return [w for i in P for w in i]


def main():
    dspmaster.CONTEXT = 'dsp3'
    with dspmaster.Session(Peek(), size=64) as ses:
        for a in (0, 1):
            ses.load(prog(a)); time.sleep(0.3)
            c1 = raw_value(ses.pk.read(RD + 4 * (O & 0x3FF), 1)[0]); t = time.time(); time.sleep(0.5)
            c2 = raw_value(ses.pk.read(RD + 4 * (O & 0x3FF), 1)[0])
            print(f'load [h6+{a}] store [h6]: {(c2 - c1) / INC / (time.time() - t):9.0f} increments/s', flush=True)


if __name__ == '__main__':
    main()
