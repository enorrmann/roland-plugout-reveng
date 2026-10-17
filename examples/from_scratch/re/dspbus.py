"""Where do bank-3 (IOMD, `6nnn`) and bank-4 (MXM, `8nnn`) stores go? Master DSP3 stores markers every sample to
the given references; the ARM scans the MXM windows of all six DSPs and the IOMD block for them (IEEE or RAW
encodings). Marker of reference R: 0.125 + R/65536 (unique, exact in both formats).

    python3 dspbus.py 85F0 60D8          # references to mark (8 consecutive words each)
"""
import os, struct, sys, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value   # noqa: E402
import dspmaster, dspasm as A          # noqa: E402

REGIONS = [('MXM0', 0x411D0000, 0x2040), ('MXM1', 0x411D8000, 0x2040)] + \
          [(f'MXM{n}', 0x41430000 + (n - 2) * 0x100000, 0x2040) for n in range(2, 6)] + \
          [('IOMD', 0x4113C000, 0x400), ('IOMD SD31', 0x4113D000, 0x400), ('IOMD RAW', 0x4113E000, 0x400)]


def marker(ref):
    return 0.125 + ref / 65536          # unique per reference, exact in RAW and IEEE


def prog(refs):
    P = [[0x0A02, 0xFF70]]
    for g, ref in enumerate(refs):
        for k in range(8):
            P += [A.a('move', 1, imm=marker(ref + k)), A.st(1, ref + k)]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [0x0001]]
    return [w for i in P for w in i]


def main():
    refs = [int(x, 16) for x in sys.argv[1:]] or [0x85F0, 0x60D8]
    enc = {}
    for g in range(len(refs)):
        for k in range(8):
            m = marker(refs[g] + k)
            enc[struct.unpack('<I', struct.pack('<f', m))[0]] = (refs[g] + k, 'IEEE')
            enc[A.raw(m)] = (refs[g] + k, 'RAW')
    if os.environ.get('BUSDSP') == '4':          # 2026-10-17: writer = master DSP4 slot 0xF7DC
        dspmaster.CONTEXT = 'efdc'
        sess = dspmaster.Session(Peek(), 4, 0xF7DC, size=256)
    else:
        dspmaster.CONTEXT = 'dsp3'
        sess = dspmaster.Session(Peek(), size=400)
    with sess as ses:
        ses.load(prog(refs)); time.sleep(0.3)
        for name, base, size in REGIONS:
            words = []
            for a in range(base, base + size, 1024):
                words += ses.pk.read(a, min(256, (base + size - a) // 4))
            hits = [(i, w) for i, w in enumerate(words) if w in enc]
            for i, w in hits:
                print(f'{name:9s} {base + 4 * i:08x} (+{4 * i:#06x}, word {i:#05x}): ref {enc[w][0]:04x} ({enc[w][1]})')
            if not hits:
                print(f'{name:9s} no markers', flush=True)


if __name__ == '__main__':
    main()
