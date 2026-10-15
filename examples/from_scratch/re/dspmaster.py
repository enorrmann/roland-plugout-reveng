"""Run a test program on a DSP of the MASTER and read its data bank, live (dsppeek.py transport).

    python3 dspmaster.py prog.txt [--read 0x400 8] [--dsp 4] [--slot 0xEFDC] [--wait 0.05]

The program (16-bit words in execution order, '#' comments, as in re/dspexp/audio) replaces the
global-effect program at <slot> of master DSP <dsp> for a moment: the original words are read,
the test program is written (an empty program first, then the body, then the head, so the DSP
never runs a half-written program), we wait, read bank 0 (RAW view) twice, and put the original
program back (tail first, head last). Values are printed as RAW words and decoded floats.
While it runs, that global effect (reverb/delay of the SYSTEM-8) is replaced.
"""
import argparse, os, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value   # noqa: E402

EMPTY = [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001, 0x0000]


def words_of(text):
    return [int(t, 16) for line in text.splitlines() for t in line.split('#')[0].split()]


def pack(hw):
    """halfwords in execution order -> 32-bit memory words (upper half first)"""
    hw = list(hw) + [0] * (len(hw) % 2)
    return [hw[i] << 16 | hw[i + 1] for i in range(0, len(hw), 2)]


def majority(reads):
    out = []
    for col in zip(*reads):
        best = max(set(col), key=col.count)
        if col.count(best) * 2 <= len(col):
            print(f'  no majority: {[f"{x:08x}" for x in col]}', file=sys.stderr)
        out.append(best)
    return out


def prog_addr(dsp, slot):
    return 0x41480000 + dsp * 0x100000 + slot


def bank0(dsp, idx):
    return 0x41404000 + dsp * 0x100000 + 4 * idx       # RAW view


def run(pk, prog, read=(0x400, 8), dsp=4, slot=0xEFDC, wait=0.05):
    pa = prog_addr(dsp, slot)
    new, empty = pack(prog), pack(EMPTY)
    new += [0] * max(0, len(empty) - len(new))
    orig = pk.read(pa, len(new))
    try:
        pk.write(pa, empty)                                    # empty task first
        if len(new) > len(empty):
            pk.write(pa + 4 * len(empty), new[len(empty):])    # body
        for i in reversed(range(len(empty))):                  # head last
            pk.write(pa + 4 * i, [new[i]])
        time.sleep(wait)
        first, n = read
        # single words read while the DSP is running sometimes come back wrong (0 or 4ea5f810,
        # one word per read of 8): read 5 times and take the per-word majority
        reads = []
        for _ in range(5):
            reads.append(pk.read(bank0(dsp, first), n))
            time.sleep(wait / 5)
        a = majority(reads)
        b = majority(reads[::-1])
    finally:
        pk.write(pa, empty)                                    # restore: empty head, tail, head
        if len(orig) > len(empty):
            pk.write(pa + 4 * len(empty), orig[len(empty):])
        for i in reversed(range(len(empty))):
            pk.write(pa + 4 * i, [orig[i]])
    if pk.read(pa, len(orig)) != orig:
        print('WARNING: original program not restored exactly', file=sys.stderr)
    return a, b


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('prog')
    ap.add_argument('--read', nargs=2, default=['0x400', '8'])
    ap.add_argument('--dsp', type=int, default=4)
    ap.add_argument('--slot', default='0xEFDC')
    ap.add_argument('--wait', type=float, default=0.05)
    a = ap.parse_args()
    prog = words_of(open(a.prog).read())
    first, n = int(a.read[0], 0), int(a.read[1], 0)
    x, y = run(Peek(), prog, (first, n), a.dsp, int(a.slot, 0), a.wait)
    for i, (u, v) in enumerate(zip(x, y)):
        print(f'[{first + i:04x}] {u:08x} {raw_value(u):+.9g}' + ('' if u == v else f'   then {v:08x} {raw_value(v):+.9g}'))


if __name__ == '__main__':
    main()
