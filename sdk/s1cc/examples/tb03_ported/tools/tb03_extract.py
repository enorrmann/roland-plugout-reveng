"""tb03_extract.py: the TB-03's DSP programs and ARM image out of Roland's updater (sdk/s1cc/examples/tb03_ported).

    python3 sdk/s1cc/examples/tb03_ported/tools/tb03_extract.py BQ2_UPD.BIN [OUTDIR]     (OUTDIR: work/tb03)

BQ2_UPD.BIN = "Boutique2 System Program 107" (boutique2_sys_107.zip, TB-03 1.07, 2021-01-21). The module "BQ2:Appli"
(0x40-byte header + LZSS, the codec of tools/lzss.py) is decoded and checked (SHA-256), then the TB-03's DSP program
sets are written as hex word files, halfwords in execution order (each 32-bit little-endian word upper half first),
the leading 0000 / 0001 data words skipped:
    OUTDIR/BQ2_Appli.bin             the decoded ARM image (base 0x60000000)
    OUTDIR/progs/v_ADDR.hex          voice DSP (set 0x6014e184): 0x0000, 0x0022 (the voice), 0x1c6c
    OUTDIR/progs/b_ADDR.hex          second DSP (set 0x6015006c): call list, post-processing, effects, output
    OUTDIR/progs/fxN_ADDR.hex        effect variants (sets 0x60150828 ... 0x60151e6c)
README.md §1.
"""
import hashlib
import os
import struct
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import lzss  # noqa: E402

APPLI_SHA256 = '17251065193165d79cb2bf4764aee717dcb5622ccb718bcc5d84d84525aeedba'
BASE = 0x60000000
SETS = {'v': (0x6014e184, 3), 'b': (0x6015006c, 11), 'fx1': (0x60150828, 3), 'fx2': (0x60150864, 4),
        'fx3': (0x60150b28, 3), 'fx4': (0x60150b64, 4), 'fx5': (0x6015143c, 4), 'fx6': (0x60151e6c, 4)}


def appli(path):
    data = open(path, 'rb').read()
    i = data.find(b'BQ2:Appli       ')
    if i < 0:
        raise SystemExit('%s: no BQ2:Appli module' % path)
    stored, _cks, _run, size = struct.unpack_from('<4I', data, i + 0x30)
    img = lzss.decode(data[i + 0x40:i + 0x40 + stored], size)
    if hashlib.sha256(img).hexdigest() != APPLI_SHA256:
        raise SystemExit('%s: BQ2:Appli is not version 1.07 (SHA-256 differs)' % path)
    return img


def main():
    src = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(ROOT, 'work', 'tb03')
    img = appli(src)
    os.makedirs(os.path.join(out, 'progs'), exist_ok=True)
    open(os.path.join(out, 'BQ2_Appli.bin'), 'wb').write(img)
    for name, (addr, count) in SETS.items():
        for k in range(count):
            a, n, p = struct.unpack_from('<3I', img, addr - BASE + 12 * k)
            w = []
            for q in range(p - BASE, p - BASE + (n + 3) // 4 * 4, 4):
                lo, hi = struct.unpack_from('<2H', img, q)
                w += [hi, lo]
            w = w[:n // 2]
            i = 0
            while i < len(w) and w[i] in (0, 1):
                i += 1
            open(os.path.join(out, 'progs', '%s_%04x.hex' % (name, a)), 'w').write(' '.join('%04x' % x for x in w[i:]) + '\n')
    print('%s: BQ2:Appli %d bytes, %d programs' % (out, len(img), sum(c for _, c in SETS.values())))


if __name__ == '__main__':
    main()
