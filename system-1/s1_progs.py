"""DSP programs of a SYSTEM-1 PLUG-OUT: where each one goes, its cost, and what it exchanges (study only).

    python3 system-1/s1_progs.py                      # SH-101, SH-2, PROMARS (work/unpack/<NAME>_S1_Code1)
    python3 system-1/s1_progs.py work/unpack/SH-2_S1_Code1 --listing

For each program of region 0 (master) and region 1 (slave), found as {address, bytes, pointer} entries of the
program sets: the address (0x0100 main, 0x1000 master B, 0x1900/0x3100 voice sections), static instructions,
instructions executed per sample in the C emulator (random bank-5 / parameter / IOMD inputs; equal to static when
the program has no jumps), the IOMD words it loads and stores (600E-6021 master -> slave, 6006-600D and
6026-602D slave -> master) and the bank-5 words it stores. Used by system-1/DSP_FIT.md §4 to compare how far each
mono synth could be made polyphonic. Needs the unpacked Code1.Dat:
`python3 tools/plugout_build.py unpack plugins/<NAME>/Script/Code1.Dat work/unpack/<NAME>_S1_Code1`.
"""
import os
import random
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..'))
for p in ('asm', 'examples/from_scratch/emu', 'examples/from_scratch/re'):
    sys.path.insert(0, os.path.join(ROOT, p))
import chain     # noqa: E402
import cchain    # noqa: E402
import disasm    # noqa: E402
import isa       # noqa: E402

REGIONS = (('region0', 0x60800000, 'master'), ('region1', 0x60200000, 'slave'))
SLOTS = (0x0, 0x82, 0x100, 0x1000, 0x1900, 0x3100, 0x4900, 0x6100, 0x5FF2, 0x6DF2, 0x6FF2, 0x7FF2)
CODE = (0x100, 0x1000, 0x1900, 0x3100, 0x4900, 0x6100)
XFER = re.compile(r'\b(ld|st) r\d, \[([0-9a-f]{4})\]')
DEFAULT = ['work/unpack/SH-101_S1_Code1', 'work/unpack/SH-2_S1_Code1', 'work/unpack/PROMARS_S1_Code1']


def sets(img, base):
    """program sets: runs of consecutive {address, bytes, pointer} entries -> [[(address, bytes, pointer)]]"""
    out, cur, last = [], [], None
    for o in range(0, len(img) - 12, 4):
        a, n, p = struct.unpack_from('<3I', img, o)
        if a in SLOTS and base <= p < base + len(img) and 0 < n < 0x8000 and n % 2 == 0:
            if cur and o - last != 12:
                out.append(cur)
                cur = []
            cur.append((a, n, p))
            last = o
    return out + [cur] if cur else out


def words(img, base, p, n):
    """halfwords in execution order, leading 0000/0001 data halfwords skipped"""
    w = []
    for q in range(p - base, p - base + (n + 3) // 4 * 4, 4):
        lo, hi = struct.unpack_from('<2H', img, q)
        w += [hi, lo]
    w = w[:n // 2]
    i = 0
    while i < len(w) and w[i] in (0, 1):
        i += 1
    return w[i:]


def executed(w, samples=2000, seed=1):
    s = cchain.Slave({0: (chain.Program(w, 'p'), [])}, native=False, threads=False)
    r = random.Random(seed)
    per = []
    for t in range(samples):
        if t % 200 == 0:
            for i in range(0x200):
                s.p_bank(0, i, r.choice([0.0, 1.0, r.uniform(-1, 1)]))
            for i in range(0x100):
                s.params[0xC000 + i] = r.choice([0.0, 1.0, r.uniform(0, 1)])
            for i in range(0x40):
                s.iomd[0x6000 + i] = r.uniform(-1, 1)
        a = s.ninstr_dsp(0)
        s.step()
        per.append(s.ninstr_dsp(0) - a)
    per = per[samples // 10:]
    return min(per), max(per)


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    table = isa.Table.load()
    f = lambda s: ' '.join('%x' % x for x in sorted(s)) or '-'
    for d in args or DEFAULT:
        d = d if os.path.isabs(d) else os.path.join(ROOT, d)
        print(f'== {os.path.basename(d)}')
        for reg, base, chip in REGIONS:
            img = open(os.path.join(d, reg + '.bin'), 'rb').read()
            for k, st in enumerate(sets(img, base)):
                print(f'  {chip} set {k}: ' + '  '.join('%#x' % a for a, _, _ in st))
                for a, n, p in st:
                    if a not in CODE:
                        continue
                    w = words(img, base, p, n)
                    text, _ = disasm.program(w, table, '%x' % a)
                    nst = sum(1 for line in text.splitlines()
                              if line.strip() and not line.startswith((';', '.')))
                    mn, mx = executed(w)
                    io = {'ld': set(), 'st': set()}
                    b5 = set()
                    for op, ad in XFER.findall(text):
                        v = int(ad, 16)
                        if 0x6000 <= v < 0x6100:
                            io[op].add(v)
                        if 0xA000 <= v < 0xA800 and op == 'st':
                            b5.add(v)
                    print(f'    {a:#06x} {n:5d} B  static {nst:4d}  executed {mn}-{mx}')
                    print(f'           IOMD ld {f(io["ld"])}')
                    print(f'           IOMD st {f(io["st"])}')
                    print(f'           bank 5 st {f(b5)}')
                    if '--listing' in sys.argv:
                        print(text)


if __name__ == '__main__':
    main()
