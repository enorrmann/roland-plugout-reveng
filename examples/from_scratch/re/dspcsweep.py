"""Sweep the target byte of a C (condition) word: which ops of the instruction run, for a true and a false context
condition (master DSP3, DSPCTX=dsp3: condition 1 true, condition 0 false).

    python3 dspcsweep.py 0x168 [--from 0 --to 255]  > work/dspclass/csweep_168.txt

Each op of the test instruction copies a distinct register (r1 <- r5, r2 <- r6, r3 <- r7, ...), all destinations start
at 0, so the registers that changed tell which ops ran. Output per target byte: enabled ops for condition true / false.
"""
import argparse, os, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value       # noqa: E402
from dspmaster import Session, words_of   # noqa: E402
import dspprobe                           # noqa: E402

# class -> (instruction template with {C}, number of ops); ops copy r5/r6/r7 (and r4 for a 4th) into r1/r2/r3/(r0)
TESTS = {
    0x0B8: ('1703 1045 {C}', 1),                                 # A C
    0x168: ('2d06 1045 2046 {C} 0000 0000', 2),                  # A A C I I
    0x570: ('ae08 1045 2046 3047 {C} 0fe0 07f0 0000', 3),        # A A A C T R I
}
REGS = [0, 0, 0, 0, 0.0625, 0.5, 0.25, 0.125]


def program(line):
    L = ['0a02 ff70'] + [f'0903 {r:x}050 {dspprobe.hi16(v):04x}' for r, v in enumerate(REGS)] + [line]
    L += [f'0c03 {8 + r:x}fe0 {0x780 + r:04x}' for r in range(8)] + ['0a02 ff70', '0b02 00f8', '0001']
    return words_of('\n'.join(L))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cls')
    ap.add_argument('--from', dest='lo', type=int, default=0)
    ap.add_argument('--to', dest='hi', type=int, default=255)
    a = ap.parse_args()
    tmpl, nops = TESTS[int(a.cls, 0)]
    dest = [1, 2, 3, 0][:nops]
    with Session(Peek()) as ses:
        for t in range(a.lo, a.hi + 1):
            res = []
            for cc in (0x02, 0x00):                  # condition 1 (true), condition 0 (false) on master DSP3
                ses.load(program(tmpl.format(C=f'{cc:02x}{t:02x}')))
                v = [raw_value(w) for w in ses.read(0x780, 8)]
                if all(x == 0 for x in v[:4]) and all(x == 0 for x in v[4:]):
                    res.append('X')
                    continue
                res.append(''.join(str(i + 1) if abs(v[d]) > 1e-9 else '-' for i, d in enumerate(dest)))
            print(f'{t:02x} T:{res[0]} F:{res[1]}', flush=True)


if __name__ == '__main__':
    main()
