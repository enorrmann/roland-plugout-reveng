"""Delay-tap measurement inside Roland's own delay program (master DSP4 slot 0xF7DC, m1_tcm_f7dc_01134238), so the
bank-2 transfers happen at the instruction positions the delay hardware expects.

Changes to Roland's program: the delay input r0 (stored to [4300] at halfword 58) is replaced by a slow counter
c (ring state [07C0], c += 2^-20 per sample, also stored to [A3C3]); after the tap-1 interpolation (halfword 164)
r0 (lerp), r1 ([4302]) and r2 ([4303]) go to [A3C0..A3C2]; r3/r4 (tap delays after f6c) to [A3C4]/[A3C5], r3/r7 before halfword 141 to [A3C6]/[A3C7]; the
audio output (halfword 289) is zeroed. Bank 5 (DPC4, plain memory) is read by the ARM at 0x41126000 (RAW).

    python3 dsprdly.py --reads 10 --gap 1 [--d3 12.5]   # --d3: force the tap-1 delay r3 (before f6c)
"""
import argparse, os, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value   # noqa: E402
import dspmaster, dspmc                # noqa: E402
import dspasm as A                     # noqa: E402

CORPUS = os.path.join(HERE, '..', '..', '..', 'work', 'corpus', 'm1_tcm_m1_tcm_f7dc_01134238.bin')
INC = 2 ** -20
DPC4 = 0x41126000
OUT = 0xA3C0


def program(d3=None, d4=None, drop=(), remove=False):
    w = dspmc.blob_words(CORPUS)
    starts, i = [], 0
    while i < len(w):
        starts.append(i)
        i += w[i] & 0x1F
    ins = {s: w[s:s + (w[s] & 0x1F)] for s in starts}
    NOP = [0x0001]
    out = []
    for s in starts:
        if s == 58:      # delay input := counter
            out += [A.ld(0, 0x7C1), NOP, A.a('add', 0, 0, imm=INC), A.st(0, 0x7C0), A.st(0, OUT + 3)]
        if s == 141:
            out += [A.st(3, OUT + 6), A.st(7, OUT + 7)]
        if s == 141 and d3 is not None:
            out += [A.a('move', 3, imm=d3)]
        if s == 151 and d4 is not None:
            out += [A.a('move', 4, imm=d4)]
        if s == 151:
            out += [A.st(3, OUT + 4), A.st(4, OUT + 5)]
        if s == 289:     # silence the effect output
            out += [NOP, A.a('move', 0, imm=0.0, bits=16), A.a('move', 1, imm=0.0, bits=16)]   # after the load latency
        if s in drop:
            if not remove:
                out.append([0x0001])                          # dropped: a no-op, same cycle count
        else:
            out.append(ins[s])
        if s == 164:
            out += [A.st(0, OUT), A.st(1, OUT + 1), A.st(2, OUT + 2)]
    return [x for i in out for x in i]


def parse_drop(text):
    out = set()
    for part in filter(None, text.split(',')):
        lo, _, hi = part.partition('-')
        out |= set(range(int(lo), int(hi or lo) + 1))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--reads', type=int, default=10)
    ap.add_argument('--gap', type=float, default=1.0)
    ap.add_argument('--hold', type=float, default=0.5)
    ap.add_argument('--d3', type=float, nargs='*', default=[None])
    ap.add_argument('--drop', default='', help='halfword starts to replace by 0001, e.g. "68,87,107" or "2-53"')
    ap.add_argument('--remove', action='store_true', help='delete the dropped instructions (changes timing)')
    ap.add_argument('--d4', type=float, default=None)
    a = ap.parse_args()
    dspmaster.CONTEXT = 'efdc'
    with dspmaster.Session(Peek(), 4, 0xF7DC, size=200) as ses:
        for d3 in a.d3:
            ses.load(program(d3, a.d4, parse_drop(a.drop), a.remove))
            time.sleep(a.hold)
            for _ in range(a.reads):
                r = ses.pk.read(DPC4 + 4 * (OUT & 0x3FF), 8)
                v = [raw_value(x) for x in r]
                print(f'd3={d3} lerp {r[0]:08x} [4302] {r[1]:08x} [4303] {r[2]:08x} ({v[2]:+.6g}) c {v[3]:+.6f} '
                      f'r3 {v[6]:+.6g} -> {v[4]:+.6g} r7 {v[7]:+.6g} r4 {v[5]:+.6g}  (c-lerp)/inc {(v[3] - v[0]) / INC:+.2f}  '
                      f'(c-[4302])/inc {(v[3] - v[1]) / INC:+.2f}', flush=True)
                time.sleep(a.gap)


if __name__ == '__main__':
    main()
