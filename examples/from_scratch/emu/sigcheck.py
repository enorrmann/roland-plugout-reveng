"""Signature check: no Roland microcode in a build or in our programs (docs/log/VOICE_ALLOCATION_LOG.md §7).

    python3 sigcheck.py FILE...      FILE = a built region (.bin) or a program (.hex / .txt, 16-bit words)

The Roland programs are the ones engine/programs.py copies at build time (MICROCODE: MAIN0-5, DCO, FX), read from
engine/build/programs.h (local, not in git). A .bin is searched for every 16-byte window (4 aligned 32-bit words, as
stored in the image) of every Roland program; a .hex for every run of 8 consecutive 16-bit words. Windows that also
occur in our own programs (generic markers such as `0a02 ff70`) are ignored. Exit status 1 if anything matches.
"""
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
FS = os.path.abspath(os.path.join(HERE, '..'))
PROGRAMS_H = os.path.join(FS, 'engine', 'build', 'programs.h')
sys.path.insert(0, os.path.join(FS, 'engine'))
import programs  # noqa: E402  (only for the names in MICROCODE)

W32, W16 = 4, 8


def roland_programs(path=PROGRAMS_H):
    """{NAME: [uint32]} of the Roland programs present in programs.h"""
    src = open(path).read()
    out = {}
    for name in programs.MICROCODE:
        m = re.search(r'mc_%s\[\d+\][^{]*\{(.*?)\};' % name.upper(), src, re.S)
        if m:
            out[name.upper()] = [int(t, 16) for t in re.findall(r'0x([0-9a-f]+)', m.group(1))]
    return out


def halves(words32):
    """32-bit image words -> 16-bit words in execution order (upper half first)"""
    out = []
    for w in words32:
        out += [w >> 16, w & 0xFFFF]
    return out


def own_word_lists():
    """{path: 16-bit words} of our programs: v3, v4 mains, our saw and null, the empty slot program of engine.c"""
    lists = {'engine.c mc_empty': halves([0x0a02ff70, 0x0a02ff70, 0x0b0200f8, 0x00010000])}
    paths = [os.path.join(FS, d, f) for d in ('v3', 'v4') for f in sorted(os.listdir(os.path.join(FS, d)))
             if f.endswith('.hex')] + [os.path.join(FS, 'gate', 'dco2_null.hex')]
    for p in paths:
        lists[os.path.realpath(p)] = read_hex(p)
    return lists


def read_hex(path):
    return [int(t, 16) for line in open(path) for t in line.split('#')[0].split()]


def windows(seq, n):
    return {tuple(seq[i:i + n]) for i in range(len(seq) - n + 1)}


def check_bin(blob, rol, own16):
    own32 = set()
    for o in own16.values():
        o = o + [0] * (len(o) & 1)
        w32 = [o[i] << 16 | o[i + 1] for i in range(0, len(o), 2)]
        own32 |= windows(w32, W32)
    hits = []
    for name, w in rol.items():
        for i in range(len(w) - W32 + 1):
            win = tuple(w[i:i + W32])
            if win in own32:
                continue
            if blob.find(struct.pack('<4I', *win)) >= 0:
                hits.append((name, i))
    return hits


def check_words(path, words, rol, own16):
    """the generic windows are those of our other programs (not of the file under test)"""
    ownw = set()
    for k, o in own16.items():
        if k != os.path.realpath(path):
            ownw |= windows(o, W16)
    mine = windows(words, W16)
    hits = []
    for name, w in rol.items():
        common = (windows(halves(w), W16) & mine) - ownw
        if common:
            hits.append((name, len(common)))
    return hits


def main(paths):
    if not os.path.exists(PROGRAMS_H):
        raise SystemExit(f'{PROGRAMS_H} missing: run engine/build.sh once (it copies the Roland programs locally)')
    rol = roland_programs()
    own16 = own_word_lists()
    bad = 0
    for p in paths:
        if p.endswith('.bin'):
            hits = check_bin(open(p, 'rb').read(), rol, own16)
        else:
            hits = check_words(p, read_hex(p), rol, own16)
        names = sorted({h[0] for h in hits})
        print(f'{p}: {"CLEAN" if not hits else "ROLAND BYTES: " + ", ".join(names)} '
              f'({len(rol)} Roland programs checked)')
        bad |= bool(hits)
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
