"""Descriptor table of a Roland region 1 image: every DSP program descriptor (slot address, size, size, program
pointer) with its slot. Used for docs/VOICE_ALLOCATION.md.

    python3 alloc_scan.py region1.bin 60A00000     # BASE = the slot run address of the region 1 image (slot 1)
"""
import struct, sys
SLOTS = {0x100: 'MAIN', 0x6000: 'DCO1', 0xB000: 'DCO2', 0x8000: 'X'}
SIG = bytes([0x70, 0xff, 0x02, 0x0a])        # first bytes of a DSP program (uint32 0x0a02ff70, little endian)


def descriptors(b, base):
    n = len(b) // 4
    W = struct.unpack('<%dI' % n, b[:n * 4])
    out = []
    for i in range(n - 3):
        a, n1, n2, p = W[i:i + 4]
        if a in SLOTS and n1 == n2 and 0x20 < n1 < 0x4000 and base <= p < base + len(b) - 4:
            out.append((i * 4, a, n1, p - base, b[p - base:p - base + 4] == SIG))
    return out


def best_base(b):
    """the run address whose descriptors point at program starts (signature) most often"""
    cands = [0x60200000, 0x60800000, 0x60A00000, 0x60C00000, 0x61000000, 0x60000000, 0x60400000, 0x60600000]
    return max(cands, key=lambda c: sum(1 for d in descriptors(b, c) if d[4]))


if __name__ == '__main__':
    b = open(sys.argv[1], 'rb').read()
    base = int(sys.argv[2], 16) if len(sys.argv) > 2 else best_base(b)
    rows = descriptors(b, base)
    print('base %08x' % base)
    for pos, a, n, off, ok in rows:
        print('desc %06x  %-5s size %5d  prog %06x%s' % (pos, SLOTS[a], n, off, '' if ok else '  (no program signature)'))
    print(len(rows), 'descriptors')
