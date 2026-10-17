"""Time budget of a master DSP (default: idle master DSP3) and the cost of multi-op instructions, without audio.

    python3 dspbudget.py single 100 400 800 ...     # K single-op fillers (A: r4 = r5) before a counter
    python3 dspbudget.py aab 100 300 ...            # K A/A/B fillers (r4 = r5 | r3 = r5 | r2 = max(r1, r5))
    python3 dspbudget.py skip 500 1500 ...          # a jump over K single-op fillers (do skipped words cost?)
    python3 dspbudget.py loop 100 200 ...           # a loop of K iterations (4 instructions each) before the counter

The program is K fillers followed by a counter kept in the ring memory (load [S + 1], add 2^-16, wrap to [-1, 1), store [S]):
while the whole program fits in one sample period the counter grows by 96000 * 2^-16 = 1.46 per second; when the
fillers push the counter past the end of the budget it stops (or slows). Comparing the K where that happens for
single-op and A/A/B fillers gives the cost of an A/A/B instruction. The context's original program is saved and
restored (dspmaster.Session, area enlarged to the longest program).
"""
import argparse, os, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value   # noqa: E402
import dspmaster                       # noqa: E402
import dspasm as A                     # noqa: E402

S = 0x780
INC = 2 ** -16          # 1.46 per second at 96 kHz; the counter wraps in [-1, 1) (period 1.37 s)


def program(kind, k):
    if kind in ('skip', 'loop'):
        P = [[0x0A02, 0xFF70], A.a('move', 5, imm=0.0, bits=16), A.a('move', 6, imm=1.0, bits=16)]
        if kind == 'skip':
            P += [[0x0B03, 0x0090, 2 * k]] + [A.a('move', 4, y=5)] * k          # jump over k 2-word fillers
        else:
            lim = A.raw(k / 256)
            body = [A.a('add', 4, 4, imm=1 / 256, bits=16),                   # r4 += 1/256
                    [0x0D8 << 5 | 5, 0xCE30, 0xB9B1, lim >> 16 ^ 0x8000, lim & 0xFFFF],   # cmp r4 - k/256
                    [0x0001]]
            n = sum(len(i) for i in body) + 4
            P += [A.a('move', 4, imm=0.0, bits=16)] + body + [[0x0E0 << 5 | 4, 0x0090, 0x1A10, -n & 0xFFFF]]
        P += [A.ld(1, S + 1), [0x0001], A.a('add', 1, 1, imm=INC)] + A.wrap(1, 2) + [A.st(1, S)]
        P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [0x0001]]
        return [w for ins in P for w in ins]
    fill = {'single': A.a('move', 4, y=5),
            'aab': A.aab(A.a('move', 4, y=5), A.a('move', 3, y=5), A.b('max', 2, 1, 5))}[kind]
    P = [[0x0A02, 0xFF70], A.a('move', 5, imm=0.0, bits=16), A.a('move', 6, imm=1.0, bits=16)] + [fill] * k
    P += [A.ld(1, S + 1), [0x0001], A.a('add', 1, 1, imm=INC)] + A.wrap(1, 2) + [A.st(1, S)]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [0x0001]]
    return [w for ins in P for w in ins]


def rate(ses, gap=0.6):
    c1 = raw_value(ses.read(S, 1, k=1)[0])
    t1 = time.time()
    time.sleep(gap)
    c2 = raw_value(ses.read(S, 1, k=1)[0])
    return ((c2 - c1) % 2) / (time.time() - t1), c1, c2


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('kind', choices=['single', 'aab', 'skip', 'loop'])
    ap.add_argument('k', type=int, nargs='+')
    a = ap.parse_args()
    longest = max(len(dspmaster.adapt(program(a.kind, k))) for k in a.k)
    size = (longest + 1) // 2 + 8
    pk = Peek()
    with dspmaster.Session(pk, size=size) as ses:
        print(f'context {dspmaster.CONTEXT}, area {size} words; expected rate {96000 * INC:.1f}/s')
        for k in a.k:
            ses.load(program(a.kind, k))
            time.sleep(0.3)
            r, c1, c2 = rate(ses)
            print(f'{a.kind} K={k:5d} ({len(program(a.kind, k))} halfwords): counter {c1:9.4f} -> {c2:9.4f}, '
                  f'rate {r:7.2f}/s', flush=True)


if __name__ == '__main__':
    main()
