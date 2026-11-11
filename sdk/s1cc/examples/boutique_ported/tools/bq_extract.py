"""bq_extract.py: the Boutique JP-08 / JU-06 / JX-03 DSP programs and ARM image out of Roland's updater.

    python3 sdk/s1cc/examples/boutique_ported/tools/bq_extract.py BQ_UPD.BIN [OUTDIR]     (OUTDIR: work/bq1)

BQ_UPD.BIN = "Boutique System Program 1.21" (boutique_sys_121, 2019-09-12; module names "PX-11:..."): one firmware for
the three first-generation Boutiques, the model is chosen at run time (README.md §1). The module "PX-11:Appli"
(0x40-byte header + LZSS, tools/lzss.py) is the ARM image (base 0x60000000); it is decoded and checked (SHA-256),
then the voice DSP programs are written as hex word files, halfwords in execution order (upper half of each 32-bit
little-endian word first):
    OUTDIR/PX11_Appli.bin               the decoded ARM image
    OUTDIR/progs/g0_pre_1000.hex        DSP chip 0: LFO, noise (shared by its two voices)
    OUTDIR/progs/g0_voiceA_1900.hex     DSP chip 0, voice A: glide, two ADSRs, modulation, mixer, filter, VCA
    OUTDIR/progs/g0_voiceB_2c00.hex     DSP chip 0, voice B (the same program on other memory)
    OUTDIR/progs/g0_post_1400.hex       DSP chip 0: the four voices to stereo, saturation
    OUTDIR/progs/g1_*.hex, fx_0800.hex  DSP chip 1 (voices C, D) and the effects
    OUTDIR/progs/oscV_SLOT.hex          the 9 oscillator programs V = 0..8 for the oscillator slots of voice A
                                        (3f00 = OSC1, 4f00 = OSC2); which one runs is model x waveform (README.md §1)
"""
import hashlib
import os
import struct
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import lzss  # noqa: E402

APPLI_SHA256 = '282b69e4952a445e95099d46597531efdcbd8dd4f9aa6416a657e5b06c56a202'
B = 0x60000000
# program set entries {address, bytes, pointer} (12 bytes each) in PX-11:Appli 1.21
NAMED = {'g0_pre': 0x6014acf0, 'g0_voiceA': 0x6014acfc, 'g0_voiceB': 0x6014acc0, 'g0_post': 0x6014accc,
         'g1_pre': 0x6014dd2c, 'g1_voiceA': 0x6014dd44, 'g1_voiceB': 0x6014dd50, 'fx': 0x6014f848}
OSC_ROWS = {0x3f00: 0x60166e30, 0x4f00: 0x60166e54}    # 9 entry pointers (variants 0..8) per oscillator slot


def appli(path):
    data = open(path, 'rb').read()
    i = data.find(b'PX-11:Appli     ')
    if i < 0:
        raise SystemExit('%s: no PX-11:Appli module' % path)
    stored, _cks, _run, size = struct.unpack_from('<4I', data, i + 0x30)
    img = lzss.decode(data[i + 0x40:i + 0x40 + stored], size)
    h = hashlib.sha256(img).hexdigest()
    if APPLI_SHA256 and h != APPLI_SHA256:
        raise SystemExit('%s: PX-11:Appli is not version 1.21 (SHA-256 %s)' % (path, h))
    return img


def program(img, entry):
    """(address, halfwords) of the program set entry at `entry`"""
    a, n, p = struct.unpack_from('<3I', img, entry - B)
    w = []
    for q in range(p - B, p - B + (n + 3) // 4 * 4, 4):
        lo, hi = struct.unpack_from('<2H', img, q)
        w += [hi, lo]
    return a, w[:n // 2]


def main():
    src = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(ROOT, 'work', 'bq1')
    img = appli(src)
    os.makedirs(os.path.join(out, 'progs'), exist_ok=True)
    open(os.path.join(out, 'PX11_Appli.bin'), 'wb').write(img)

    def write(name, a, w):
        open(os.path.join(out, 'progs', '%s_%04x.hex' % (name, a)), 'w').write(' '.join('%04x' % x for x in w) + '\n')
    for name, e in NAMED.items():
        write(name, *program(img, e))
    for slot, row in OSC_ROWS.items():
        for v in range(9):
            a, w = program(img, struct.unpack_from('<I', img, row + 4 * v - B)[0])
            assert a == slot
            write('osc%d' % v, a, w)
    print('%s: PX-11:Appli %d bytes (SHA-256 %s), %d programs' % (out, len(img), hashlib.sha256(img).hexdigest(),
                                                                  len(NAMED) + 18))


if __name__ == '__main__':
    main()
