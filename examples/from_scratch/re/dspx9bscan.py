"""Where does `X9bY` write? Master DSP3 writes a unique marker with `1803 0040 b9bY` (r3) every sample; the ARM scans
the MXM windows, IOMD, and DSP3's banks 0 (RAW), 1, 2 (IEEE) and 5 (RAW) for it.

    python3 dspx9bscan.py 1 2 3          # ports
"""
import os, struct, sys, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value   # noqa: E402
import dspmaster, dspasm as A          # noqa: E402
import dspbus                          # noqa: E402

EXTRA = [('DSP3 bank0 RAW', 0x41704000, 0x2000), ('DSP3 bank1 IEEE', 0x41710000, 0x2000),
         ('DSP3 bank2 IEEE', 0x41720000, 0x1800), ('DPC3 RAW', 0x41122000, 0x1000), ('DPC2 RAW', 0x4111E000, 0x1000)]


def main():
    ports = [int(x) for x in sys.argv[1:]] or [1, 2, 3]
    dspmaster.CONTEXT = 'dsp3'
    with dspmaster.Session(Peek(), size=64) as ses:
        for port in ports:
            m = 0.3125 + port / 1024 + 1 / 65536
            enc = {struct.unpack('<I', struct.pack('<f', m))[0]: 'IEEE', A.raw(m): 'RAW'}
            P = [[0x0A02, 0xFF70], A.a('move', 3, imm=m), [0x0001], [0x1803, 0x0040, 0xB9B0 | port],
                 [0x0001], [0x0A02, 0xFF70], [0x0B02, 0x00F8], [0x0001]]
            ses.load([w for i in P for w in i]); time.sleep(0.3)
            found = 0
            for name, base, size in dspbus.REGIONS + EXTRA:
                for a in range(base, base + size, 1024):
                    ws = ses.pk.read(a, min(256, (base + size - a) // 4))
                    for i, w in enumerate(ws):
                        if w in enc:
                            found += 1
                            print(f'port {port}: {name} {a + 4 * i:08x} ({enc[w]})', flush=True)
            print(f'port {port}: {found} hits', flush=True)


if __name__ == '__main__':
    main()
