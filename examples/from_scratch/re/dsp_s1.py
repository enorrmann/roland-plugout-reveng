"""DSP programs of SYSTEM-1 images (2026-10-17): no descriptor table like the SYSTEM-8; programs start with
`0a02 ff71` (or follow an end marker) and end with `0a02 ff70 0b02 00f8 0001`. Segments the image between end
markers and keeps the segments that parse as whole instructions.

    python3 dsp_s1.py OUTDIR name=image.bin ...
"""
import os, struct, sys, hashlib
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import dspdis

END = [0x0A02, 0xFF70, 0x0B02, 0x00F8]


def halfwords(d, off):
    w = []
    for p in range(off, len(d) - 3, 4):
        lo, hi = struct.unpack_from('<2H', d, p)
        w += [hi, lo]
    return w


def segments(d):
    for align in (0, 2):
        w = halfwords(d, align)
        ends = [i for i in range(0, len(w) - 4) if w[i:i + 4] == END]
        prev = None
        for e in ends:
            if prev is not None:
                s = prev
                while s < e and w[s] in (0x0000, 0x0001):
                    s += 1
                seg = w[s:e + 5]
                if 6 < len(seg) < 0x2000:
                    try:
                        if sum(len(i) for _, i in dspdis.instructions(seg)) == len(seg):
                            yield align + 2 * s, seg
                    except ValueError:
                        pass
            prev = e + 5


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    seen = set()
    for arg in sys.argv[2:]:
        name, path = arg.split('=', 1)
        d = open(path, 'rb').read()
        n = 0
        for off, w in segments(d):
            k = hashlib.md5(bytes(str(w), 'ascii')).hexdigest()
            if k in seen:
                continue
            seen.add(k)
            n += 1
            open(os.path.join(out, f'{name}_{off:07x}.txt'), 'w').write(' '.join(f'{x:04x}' for x in w) + '\n')
        print(f'{name}: {n} new programs', flush=True)


if __name__ == '__main__':
    main()
