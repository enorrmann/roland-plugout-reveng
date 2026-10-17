"""Unit-B `{8+d}f5Y` (bits 7-6 = 01, sub 1) returns a time-varying value (2026-10-17). Master DSP3: r1 = 0 (or
--init), the op, r1 -> bank 5 every sample; read it N times over a few seconds and fit value vs time.

    python3 dspf5.py [word ...]      # default 9f5c 9f5d 9f5e 9f5f 9f54 9f50
"""
import os, sys, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
from dspmaster import Session, words_of
import dspasm as A
BANK5 = {'dsp3': 0x41122000, 'efdc': 0x41126000}[os.environ.get('DSPCTX', 'dsp3')]


def main():
    ops = sys.argv[1:] or ['9f5c', '9f5d', '9f5e', '9f5f', '9f54', '9f50']
    with Session(Peek(), size=400 if os.environ.get('DSPCTX', 'dsp3') == 'dsp3' else 256) as ses:
        for op in ops:
            P = [[0x0A02, 0xFF70], A.a('move', 1, imm=0.0, bits=16), A.a('move', 4, imm=0.0, bits=16), [1],
                 words_of(f'0a02 {op}'), [1], A.st(1, 0xA240), [0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
            ses.load([w for i in P for w in i]); time.sleep(0.1)
            pts = []
            t0 = time.time()
            for _ in range(12):
                pts.append((time.time() - t0, raw_value(ses.pk.read(BANK5 + 4 * 0x240, 1)[0])))
                time.sleep(0.25)
            n = len(pts); mt = sum(t for t, _ in pts) / n; mv = sum(v for _, v in pts) / n
            slope = sum((t - mt) * (v - mv) for t, v in pts) / sum((t - mt) ** 2 for t, _ in pts)
            print(f'{op}: first {pts[0][1]:.4f} last {pts[-1][1]:.4f}  slope {slope:+.4f}/s'
                  f'  (x16384 = {slope * 16384:+.0f} samples/s)', flush=True)


if __name__ == '__main__':
    main()
