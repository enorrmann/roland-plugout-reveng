"""Jumps (2026-10-17): A op 0, X = 2 with an imm16 (`0090 nnnn`) = jump forward nnnn words from the end of the
instruction (hypothesis from the corpus: every offset lands on an instruction boundary). Master DSP3 probes:
registers r0..r7 = 0.5 .. (dspprobe), then the test lines, then r0..r7 stored.

    python3 dspjump.py
"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek
import dspprobe

SET = {r: f'0903 {r:x}050 3000' for r in range(8)}          # r = 1.0 (3 words)
CASES = {
    'jump 3 over r0=1':            ['0b03 0090 0003', SET[0], SET[1]],
    'jump 6 over r0=1, r1=1':      ['0b03 0090 0006', SET[0], SET[1], SET[2]],
    'jump 0':                      ['0b03 0090 0000', SET[0], SET[1]],
    'cond true (c1) skips jump':   ['1c04 0090 0210 0003', SET[0], SET[1]],
    'cond false (c0) keeps jump':  ['1c04 0090 0010 0003', SET[0], SET[1]],
    '0092 (Y=2) jump 3':           ['0b03 0092 0003', SET[0], SET[1]],
    '00c1 alone':                  ['0902 00c1', SET[0], SET[1]],
    '00c2 alone':                  ['0902 00c2', SET[0], SET[1], SET[2], SET[3]],
}


def main():
    pk = Peek()
    for name, lines in CASES.items():
        after = dspprobe.probe(pk, lines)
        print(f'{name:30s} {dspprobe.fmt(dspprobe.REGS, after)}', flush=True)


if __name__ == '__main__':
    main()
