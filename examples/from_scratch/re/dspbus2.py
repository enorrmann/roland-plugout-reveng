"""Inter-DSP bus test: master DSP3 (main program) stores markers (dspbus.marker) to one MXM zone, master DSP4
(slot 0xF7DC, replacing the global delay for a moment) loads candidate references of all four zones and stores
them to bank 5 [A3C0..]; which loads return DSP3's markers shows the routing as seen by a DSP.

    python3 dspbus2.py 85F0             # DSP3 writes 85F0..85F7
"""
import os, sys, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value   # noqa: E402
import dspmaster, dspasm as A          # noqa: E402
import dspbus                          # noqa: E402

NOP = [0x0001]
CAND = [z + 0xF0 + k for z in (0x8100, 0x8300, 0x8500, 0x8700) for k in (0, 1)] + \
       [z + 0xF0 + k for z in (0x8000, 0x8200, 0x8400, 0x8600) for k in (0, 1)]


def reader():
    P = [[0x0A02, 0xFF70]]
    for i, ref in enumerate(CAND):
        P += [A.ld(1, ref), NOP, NOP, A.st(1, 0xA3C0 + i)]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [0x0001]]
    return [w for i in P for w in i]


def main():
    ref = int(sys.argv[1], 16)
    inv = {round(dspbus.marker(r), 9): r for r in range(0x8000, 0x8800)}
    pk = Peek()
    dspmaster.CONTEXT = 'dsp3'
    with dspmaster.Session(pk, size=128) as w:
        w.load(dspbus.prog([ref]))
        dspmaster.CONTEXT = 'efdc'
        with dspmaster.Session(pk, 4, 0xF7DC, size=200) as r:
            r.load(reader()); time.sleep(0.3)
            vals = pk.read(0x41126000 + 4 * 0x3C0, len(CAND))
    for c, v in zip(CAND, vals):
        x = round(raw_value(v), 9)
        print(f'DSP4 load [{c:04x}] = {x:+.6f}' + (f'  <- DSP3 store [{inv[x]:04x}]' if x in inv else ''))


if __name__ == '__main__':
    main()
