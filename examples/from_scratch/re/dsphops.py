"""Unit-B nibble-f forms that use the hidden registers (2026-10-17): load h4..h7 from registers with `40TY`
(h4 <- r0 = 0.5, h5 <- r1 = 0.25, h6 <- r2 = 0.125, h7 <- r6 = 0.875), then the op, on master DSP3 (dspprobe).

    python3 dsphops.py [word ...]      # default: the forms seen in JX_b000
"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek
import dspprobe

H = ['0b02 4000', '0b02 4041', '0b02 4082', '0b02 40c6', '0001']
OPS = ['0a02 af6f', '0a02 9f6e', '0a02 9f6d', '0a02 cf6c', '0a02 9f6f', '0a02 bf4b', '0a02 9f49', '0a02 df4d',
       '0a02 ff4f', '0a02 bf48', '0a03 cf54 3000', '0a02 9f4c', '0a02 9f5c', '0a02 9f7c']


def main():
    pk = Peek()
    for op in (sys.argv[1:] or OPS):
        if ' ' not in op:
            op = '0a02 ' + op
        for pre in (H, ['0001']):
            after = dspprobe.probe(pk, pre + [op, '0001'])
            print(f'{op:16s} {"with h" if pre is H else "no h  "}  {dspprobe.fmt(dspprobe.REGS, after)}', flush=True)


if __name__ == '__main__':
    main()
