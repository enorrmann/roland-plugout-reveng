"""The data-flow facts of docs/log/VOICE_ALLOCATION_LOG.md §6, checked on every Roland PLUG-OUT image in the repo (study only).

    python3 allplugouts.py            images: work/unpack/*/region1.bin and the JX-3P of engine/build/jx

For each image: the six main programs (descriptors with slot 0x0100, in image order = DSP0..5, re/alloc_scan.py) and
the slot programs (0x6000 / 0xB000). Per image it prints
  * pair window: loads / stores of bank 5 0x400-0x7FF per main (A4xx);
  * bank 1: the words that slot programs store, and how many of them the mains read as stored or at +1;
  * MXM: main copies `M 0nn -> Z 4nn` with the same nn, and with another nn (re/listing.forwards rule);
  * IOMD words written by each main.
Nothing is copied: the programs are disassembled in memory from the local images.
"""
import glob
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
import alloc_mxm  # noqa: E402
import alloc_scan  # noqa: E402
import disasm  # noqa: E402
import isa  # noqa: E402

XFER = re.compile(r'\b(ld|st) r(\d), \[([0-9a-f]{4})\]')
TABLE = None


def program(b, off, size):
    global TABLE
    TABLE = TABLE or isa.Table.load()
    w = alloc_mxm.words_of(b, off, size + 3)
    text, _ = disasm.program(w[:size // 2], TABLE, '%x' % off)
    ins = []
    for line in text.splitlines():
        if line.startswith((';', '.')) or not line.strip():
            continue
        ins.append([(op, int(r), int(a, 16)) for op, r, a in XFER.findall(line.split(';')[0])])
    return ins


def refs(ins, lo, hi):
    ld, st = set(), set()
    for xs in ins:
        for op, _, a in xs:
            if lo <= a < hi:
                (ld if op == 'ld' else st).add(a)
    return ld, st


def forwards(ins):
    out = []
    for k in range(1, len(ins)):
        prev = {r: a for op, r, a in ins[k - 1] if op == 'ld'}
        out += [(prev[r], a) for op, r, a in ins[k] if op == 'st' and r in prev]
    return out


def images():
    out = [(os.path.basename(os.path.dirname(p)), p) for p in sorted(glob.glob(os.path.join(ROOT, 'work', 'unpack', '*', 'region1.bin')))]
    jx = os.path.join(ROOT, 'examples', 'from_scratch', 'engine', 'build', 'jx', 'region1.bin')
    if os.path.exists(jx):
        out.insert(0, ('JX-3P', jx))
    return out


def study(name, path):
    b = open(path, 'rb').read()
    base = alloc_scan.best_base(b)
    desc = [d for d in alloc_scan.descriptors(b, base) if d[4]]
    mains = [program(b, off, n) for _, a, n, off, _ in desc if a == 0x100]
    slots = [program(b, off, n) for _, a, n, off, _ in desc if a in (0x6000, 0xB000)]
    print(f'== {name}: {len(mains)} mains, {len(slots)} slot programs (base {base:08x})')
    if len(mains) < 6:
        return
    slot_out = set()
    for s in slots:
        slot_out |= refs(s, 0x2000, 0x3000)[1]
    for d, m in enumerate(mains[:6]):
        pl, ps = refs(m, 0xA400, 0xA800)
        b1l, b1s = refs(m, 0x2000, 0x3000)
        fw = [(a, c) for a, c in forwards(m) if a >> 9 == 0x40 and c >> 9 == 0x42]
        same = sum(1 for a, c in fw if c - a == 0x400)
        io = refs(m, 0x6000, 0x7000)[1]
        f = lambda s: ' '.join('%04x' % x for x in sorted(s)) or '-'   # noqa: E731
        print(f'  main{d}: A4xx ld {f(pl)} | st {f(ps)}')
        if slot_out and d in (2, 3):
            print(f'         slot outputs read as stored {len(b1l & slot_out)}, at +1 {len(b1l & {a + 1 for a in slot_out})}'
                  f' (of {len(slot_out)}); mains store those +1 words: {len(b1s & {a + 1 for a in slot_out})}')
        if fw:
            print(f'         M->Z copies: {same} same word, {len(fw) - same} other')
        if io:
            print(f'         IOMD written: {f(io)}')


if __name__ == '__main__':
    for name, path in images():
        study(name, path)
