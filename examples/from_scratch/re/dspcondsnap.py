"""Read the 16 condition bits on master DSP3 after a prefix instruction (to find what writes them).

    python3 dspcondsnap.py "1803 0040 b9b2" --r3 -0.5
"""
import argparse, os, sys, time
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value
from dspmaster import Session, words_of
import dspprobe


def program(prefix, regs, group):
    L = [f'0903 {r:x}050 {dspprobe.hi16(v):04x}' for r, v in enumerate(regs)] + prefix.split(';')
    # condition k (group*8 + j) -> r(j) = 1 if true: op 1 r(j) = 0 always, op 2 r(j) = 1 skipped when false (b field 2)
    for j in range(8):
        k = group * 8 + j
        L.append(f'2d06 {j:x}050 {j:x}052 {2 * k:02x}02 0000 3000')
    L += [f'0c03 {8 + r:x}fe0 {0x780 + r:04x}' for r in range(8)]
    return words_of('\n'.join(L))


def snap(ses, prefix, regs):
    out = ''
    for g in (0, 1):
        ses.load(program(prefix, regs, g))
        v = [raw_value(w) for w in ses.read(0x780, 8)]
        out += ''.join('1' if abs(x - 1) < 1e-6 else '0' if x == 0 else '?' for x in v)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('prefix', nargs='+')
    ap.add_argument('--r3', type=float, default=0.5)
    a = ap.parse_args()
    with Session(Peek()) as ses:
        for p in a.prefix:
            for r3 in (a.r3, -a.r3, 0.0):
                regs = [0.25, 0.5, -0.25, r3, 0.125, -0.125, 0.75, -0.75]
                print(f'{p:28s} r3={r3:+.3f}: {snap(ses, p, regs)}', flush=True)


if __name__ == '__main__':
    main()
