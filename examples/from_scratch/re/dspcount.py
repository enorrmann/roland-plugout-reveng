"""How often is a C (condition) word true? Count on the master DSP (dspmaster.Session).

    python3 dspcount.py 0e12 1312 [--pre "0a02 b042"] [--secs 1]

A load in the instruction right after a store returns the stored word (pipeline), hence the 0001 no-ops.
Program (runs on every execution of the slot): [07a1] += 2^-16 (all executions) and
[07a0] += 2^-16 when the condition word selects the first of two loads
(2d06 1050 1052 <C> 0001 0000: r1 = 2^-16 or 0). Two reads <secs> apart give the executions per second and the
fraction of executions where the condition held.
"""
import argparse, os, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value       # noqa: E402
from dspmaster import Session, words_of   # noqa: E402

INC = 2 ** -16


def program(cw, pre):
    L = ['0a02 ff70',
         '0903 3050 0001',                         # r3 = 2^-16
         '0c03 2fe0 07a1', '0001', '0001', '0902 2c83', '0c03 afe0 07a1',   # [07a1] += r3 (load -> ALU: 2 instructions)
         '0001',                                  # a load right after a store returns the stored word
         '0c03 0fe0 07a0', '0001', '0001']        # r0 = [07a0]
    L += [pre] if pre else []
    L += [f'2d06 1050 1052 {cw} 0001 0000',        # r1 = cond ? 2^-16 : 0
          '0902 0c01', '0c03 8fe0 07a0',           # [07a0] = r0 + r1
          '0a02 ff70', '0b02 00f8', '0001']
    return words_of('\n'.join(L))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('words', nargs='+')
    ap.add_argument('--pre', default='')
    ap.add_argument('--secs', type=float, default=1.0)
    a = ap.parse_args()
    with Session(Peek()) as ses:
        for cw in a.words:
            ses.load(program(cw, a.pre))
            time.sleep(0.05)
            m0 = [raw_value(w) for w in ses.read(0x7a0, 2)]
            t0 = time.time()
            time.sleep(a.secs)
            m1 = [raw_value(w) for w in ses.read(0x7a0, 2)]
            dt = time.time() - t0
            hits, total = (m1[0] - m0[0]) / INC, (m1[1] - m0[1]) / INC
            print(f'{cw}: {total / dt:9.0f} executions/s, condition true {hits / total if total else float("nan"):.4f}'
                  f'  ({hits:.0f}/{total:.0f})', flush=True)


if __name__ == '__main__':
    main()
