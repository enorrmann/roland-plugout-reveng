"""Delay-tap experiment (NOTES.md "Tomas de retardo"): own delay line through the delay-memory DMA.

Every sample: counter c += INC (ring state, wraps in [-1, 1)), store c to the write port [W], optionally set the
f6c base (`1d06 4022 7fe0 c1fc <imm32>`), r3 = D, `bf6c` (r3 += base), the result destination `0b03 4050 <W+1>`,
the request `3a90`, then load [W+1], [W+2], [W+3] (sample, older sample, fraction) and store them, c and c - tap
to bank 5 [A3C0..] (read by the ARM, coherent). Works only where the ARM connected bank 2 to the delay memory:
master DSP4 slot 0xF7DC (W = 4300, write position 324; `--base 324` gives tap = x(n - 2 - 16384 D)).
Measured 2026-10-16 (TAPINC=-20, --base 324): D = 1 -> 16386 samples, D = 0.5 -> 8194.

    TAPINC=-20 python3 dsptap.py --f7dc --W 0x4300 --base 324 --D 1 0.5 --hold 1 --reads 2
    env: TAPREQ=0c02|1b04 (request form), TAPDEST=0 (no destination word: requests ignored), TAPTWO=1
"""
import argparse, os, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from dsppeek import Peek, raw_value   # noqa: E402
import dspmaster                       # noqa: E402
import dspasm as A                     # noqa: E402

S = 0x7C0           # counter state (ring)
OUT = 0x780
OUT5 = 0xA3C0                      # bank 5 (DPC, plain memory): coherent ARM reads
DPC_RAW = {3: 0x41122000, 4: 0x41126000}
INC = 2 ** float(os.environ.get('TAPINC', -12))   # TAPINC=-20: wraps after 2^21 samples (21.8 s)
NOP = [0x0001]
REQ = {'0c02': lambda d: [0x060 << 5 | 2, d << 12 | 0xA90],                    # transfer only (as J60 fcf0)
       '1b04': lambda d: [0x1B04, 0xC510, d << 12 | 0xA90, 0x0008]}           # Roland f7dc: r4 = max(r4, imm) + request
REQFORM = os.environ.get('TAPREQ', '0c02')
TWO = os.environ.get('TAPTWO') == '1'
DEST = os.environ.get('TAPDEST', '1') == '1'


def program(D, mode, W=0x4000, pads=1, dreg=3, base=None):
    P = [A.ld(1, S + 1), NOP, A.a('add', 1, 1, imm=INC)] + A.wrap(1, 2) + [A.st(1, S)]
    P += [A.st(1, W), NOP]
    if base is not None:     # set the f6c base (Roland: `1d06 4022 7fe0 c1fc <imm32>`, base = imm >> 22)
        P += [[0x1D06, 0x4022, 0x7FE0, 0xC1FC, (base << 6) & 0xFFFF, 0]]
    P += [A.a('move', dreg, imm=D)]
    if mode in ('f6c', 'both'):
        P += [[0x050 << 5 | 2, (8 + dreg) << 12 | 0xF6C]] + [NOP] * pads
    if DEST:         # destination of the tap results in bank 2 (Roland: `0b03 4050 0301` before the request)
        P += [[0x0B03, 0x4050, (W + 1) & 0x1FFF]]
    if mode in ('a90', 'both'):
        P += [REQ[REQFORM](dreg)] + [NOP] * pads
    if TWO:          # a second tap request (Roland's slot makes two per sample): r7 = D, f6c, request
        P += [A.a('move', 7, imm=D), [0x050 << 5 | 2, 0xFF6C], NOP, [0x060 << 5 | 2, 0x7A90], NOP]
    P += [A.ld(4, W + 1), A.ld(5, W + 2), A.ld(6, W + 3), NOP, NOP]
    for o in (OUT, OUT5):
        P += [A.st(4, o), A.st(5, o + 1), A.st(6, o + 2), A.st(1, o + 3)]
    P += [A.a('sub', 4, 4, 1), A.a('sub', 5, 5, 1)]                          # c - tap  (sub: Y - X)
    P += [A.st(4, OUT + 4), A.st(5, OUT + 5), A.st(4, OUT5 + 4), A.st(5, OUT5 + 5)]
    P = [[0x0A02, 0xFF70], A.a('move', 5, imm=0.0, bits=16), A.a('move', 6, imm=1.0, bits=16)] + P
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [0x0001]]
    return [w for ins in P for w in ins]


def d(x):
    """counter difference -> samples, modulo the 8192-sample wrap"""
    n = 2 / INC
    return ((x / INC) + n / 2) % n - n / 2


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--D', type=float, nargs='+', default=[1.0])
    ap.add_argument('--mode', default='both', choices=['none', 'f6c', 'a90', 'both'])
    ap.add_argument('--W', type=lambda s: int(s, 0), default=0x4000)
    ap.add_argument('--pads', type=int, default=1)
    ap.add_argument('--base', type=int, default=None, help='set the f6c base first (Roland: 324)')
    ap.add_argument('--hold', type=float, default=0.2, help='seconds before reading')
    ap.add_argument('--gap', type=float, default=0.0, help='seconds between reads')
    ap.add_argument('--reads', type=int, default=1, help='single (non-majority) reads per case')
    ap.add_argument('--f7dc', action='store_true', help='run in master DSP4 slot 0xF7DC (Roland delay, base 4300)')
    a = ap.parse_args()
    dsp = slot = None
    if a.f7dc:      # the slot whose bank-2 window the ARM connected to the delay memory
        dspmaster.CONTEXT, dsp, slot = 'efdc', 4, 0xF7DC
    with dspmaster.Session(Peek(), dsp, slot, size=160) as ses:
        for D in a.D:
            ses.load(program(D, a.mode, a.W, a.pads, base=a.base))
            time.sleep(a.hold)
            for _ in range(a.reads):
                time.sleep(a.gap)
                r = ses.pk.read(DPC_RAW[ses.dsp] + 4 * (OUT5 & 0x3FF), 6)
                v = [raw_value(x) for x in r]
                print(f'D={D:<10g} {a.mode:4s} tap1 {r[0]:08x} tap2 {r[1]:08x} tap3 {r[2]:08x} ({v[2]:+.6g}) '
                      f'c {v[3]:+.5f}  c-tap1 {d(v[4]):+.3f}  c-tap2 {d(v[5]):+.3f} samples', flush=True)


if __name__ == '__main__':
    main()
