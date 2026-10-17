"""MXM routing on the master, both directions at once (2026-10-17). DSP3 (main) and DSP4 (slot 0xF7DC) each store
unique salted markers to their four MXM zones (M 81F0, O 83F0, Z 85F0, S 87F0, words F0..F1) and load the same
eight words, storing what they read to bank 5; the ARM also scans the MXM windows of DSP2..5. Prints, for every
read and every window word, which writer/zone the value came from.
"""
import os, random, sys, struct, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsppeek import Peek, raw_value
import dspmaster, dspasm as A

NOP = [1]
ZONES = {'M': 0x81F0, 'O': 0x83F0, 'Z': 0x85F0, 'S': 0x87F0}
SALT = random.randint(1, 255) / 2 ** 12


def marker(dsp, zone, k):
    return SALT + dsp * 0.25 + 'MOZS'.index(zone) / 32 + k / 256


def prog(dsp):
    P = [[0x0A02, 0xFF70]]
    for z, base in ZONES.items():          # read first (what arrives), then write
        for k in range(2):
            P += [A.ld(1, base + k), NOP, NOP, A.st(1, 0xA3D0 + 2 * 'MOZS'.index(z) + k)]
    for z, base in ZONES.items():
        for k in range(2):
            P += [A.a('move', 1, imm=marker(dsp, z, k)), A.st(1, base + k)]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [1]]
    return [w for i in P for w in i]


def who(v):
    for d in (3, 4):
        for z in 'MOZS':
            for k in range(2):
                if abs(v - marker(d, z, k)) < 1e-7:
                    return f'DSP{d} {z}[{k}]'
    return None


def main():
    pk = Peek()
    dspmaster.CONTEXT = 'dsp3'
    with dspmaster.Session(pk, size=400) as s3:
        s3.load(prog(3))
        dspmaster.CONTEXT = 'efdc'
        with dspmaster.Session(pk, 4, 0xF7DC, size=256) as s4:
            s4.load(prog(4))
            time.sleep(0.5)
            for dsp, base in ((3, 0x41122000), (4, 0x41126000)):
                vals = [raw_value(x) for x in pk.read(base + 4 * 0x3D0, 8)]
                for i, v in enumerate(vals):
                    z, k = 'MOZS'[i // 2], i % 2
                    print(f'DSP{dsp} reads {z} {ZONES[z] + k:04x}: {who(v) or f"{v:.6f} (not a marker)"}')
            for n in range(2, 6):
                b = 0x41430000 + (n - 2) * 0x100000
                for zi, zn in enumerate('MOZS'):
                    words = pk.read(b + zi * 0x800, 0x200)
                    for i, w in enumerate(words):
                        v = struct.unpack('<f', struct.pack('<I', w))[0]
                        src = who(round(v, 9)) if w else None
                        if src:
                            print(f'ARM window MXM{n} zone {zn} word {i:#05x}: {src}')
    dspmaster.CONTEXT = 'dsp3'


if __name__ == '__main__':
    main()
