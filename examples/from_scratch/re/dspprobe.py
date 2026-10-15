"""Exact instruction probes on a master DSP (dspmaster.py + dsppeek.py): registers before -> after.

    python3 dspprobe.py "0902 0c01" ["0803 0c01 0000" ...]     # one probe per argument (';' = more lines)

Each probe: r0..r7 = REGS (0903 d050 imm16), the instruction lines, then r0..r7 stored to bank 0
[STORE..STORE+7] with one 0c03 per register (7809 with different registers only kept the last one).
Prints the registers that changed, as floats (exact RAW words in --raw).
"""
import argparse, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value   # noqa: E402
import dspmaster                       # noqa: E402

STORE = 0x780                          # bank 0 words used for the results (free area of master DSP4)
REGS = [0.5, 0.25, 0.125, -0.375, 0.0625, -0.75, 0.875, -0.0625]   # r0..r7: the only registers
# transfers can store (X 8-15 in XfeY = store r0..r7), and 0903 with D >= 8 does not load r8..


def hi16(v):
    if v == 0:
        return 0
    s, v = (1, -v) if v < 0 else (0, v)
    for e in range(4):
        m = v * 2 ** (32 - 4 * e)
        if m < 2 ** 29:
            w = s << 31 | e << 29 | int(round(m))
            assert w & 0xFFFF == 0, v
            return w >> 16
    raise ValueError(v)


def program(lines, regs=REGS):
    L = ['0a02 ff70'] + [f'0903 {r:x}050 {hi16(v):04x}' for r, v in enumerate(regs)] + list(lines)
    L += [f'0c03 {8 + r:x}fe0 {STORE + r:04x}' for r in range(len(regs))] + ['0a02 ff70', '0b02 00f8', '0001']
    return dspmaster.words_of('\n'.join(L))


def probe(pk, lines, regs=REGS):
    a, b = dspmaster.run(pk, program(lines, regs), (STORE, len(regs)))
    if a != b:
        print('  (unstable between two reads)', a, b)
    return b


def fmt(regs, after, raw=False):
    out = []
    for r, (v0, w) in enumerate(zip(regs, after)):
        v = raw_value(w)
        if raw or abs(v - v0) > 1e-9:
            out.append(f'r{r}={v:+.6g}' + (f'[{w:08x}]' if raw else ''))
    return ' '.join(out) or 'no change'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('probes', nargs='+')
    ap.add_argument('--raw', action='store_true')
    ap.add_argument('--regs', help='comma-separated r0..r7 values')
    a = ap.parse_args()
    regs = [float(x) for x in a.regs.split(',')] if a.regs else REGS
    pk = Peek()
    for p in a.probes:
        print(f'{p:28s} {fmt(regs, probe(pk, p.split(";"), regs), a.raw)}')


if __name__ == '__main__':
    main()
