"""External delay memory (EDM, 0x200000 words per BMC) through the ARM indirect port (2026-10-17, master):
address -> sys+0x40, 0xA0000000 -> sys+0x44 (read command; bit 31 clears when done), data at 0x41158000 (IEEE),
+4, +8 (RAW). IDM: sys+0x50/+0x54, port +0x10. Writes with the driver's sequence (address, then data to the
port; s_boot 0x0111210a) did NOT land yet. The f7dc delay line lives at EDM 0x110000-0x1FFFFF (983040 words).

    python3 dspedm.py read ADDR [N]          # hex address, N words
    python3 dspedm.py scan [STEP]            # nonzero words over the whole EDM (STEP words apart, default 0x1000)
"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value   # noqa: E402

SYS, PORT = 0x41100000, 0x41158000


def read(pk, a, idm=False):
    pk.write(SYS + (0x50 if idm else 0x40), [a])
    pk.write(SYS + (0x54 if idm else 0x44), [0xA0000000])
    return pk.read(PORT + (0x18 if idm else 8), 1)[0]


def main():
    pk = Peek()
    if sys.argv[1] == 'read':
        a = int(sys.argv[2], 16)
        for i in range(int(sys.argv[3]) if len(sys.argv) > 3 else 1):
            w = read(pk, a + i)
            print(f'{a + i:#08x}: {w:08x} {raw_value(w):+.6g}')
    else:
        step = int(sys.argv[2], 0) if len(sys.argv) > 2 else 0x1000
        nz = [(a, read(pk, a)) for a in range(0, 0x200000, step)]
        nz = [(a, w) for a, w in nz if w]
        print(f'{len(nz)} nonzero of {0x200000 // step}')
        for a, w in nz:
            print(f'  {a:#08x}: {w:08x} {raw_value(w):+.6g}')


if __name__ == '__main__':
    main()
