"""Bus references of Roland's JX-3P programs, read from their listings (study only; nothing is copied or executed).

The programs come from engine/build/programs.h (local, copied from the user's PLUG-OUT by engine/programs.py; MAIN0
and MAIN1 carry the envelope patches of programs.py, which do not touch any bus reference) and are disassembled with
asm/disasm.py, as dsp/dump_main.py does.

    python3 listing.py MAIN4                  loads and stores of banks 1, 3, 4, 5 per instruction
    python3 listing.py --summary              the cross-program facts used by docs/log/VOICE_ALLOCATION_LOG.md §6
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
PROGRAMS_H = os.path.join(ROOT, 'examples', 'from_scratch', 'engine', 'build', 'programs.h')
S_BOOT = os.path.join(ROOT, 'work', 'os', 's_boot', 's_boot.bin')
sys.path.insert(0, os.path.join(ROOT, 'asm'))

XFER = re.compile(r'\b(ld|st) r(\d), \[([0-9a-f]{4})\]')


def available():
    return os.path.exists(PROGRAMS_H)


def words(name):
    src = open(PROGRAMS_H).read()
    m = re.search(r'mc_%s\[(\d+)\].*?=\s*\{(.*?)\};' % name, src, re.S)
    w = []
    for t in re.findall(r'0x([0-9a-f]+)', m.group(2)):
        v = int(t, 16)
        w += [v >> 16, v & 0xFFFF]
    n = int(re.search(r'MC_%s_BYTES (0x[0-9a-f]+)' % name, src).group(1), 16) // 2
    return w[:n]


def instructions(name):
    """[(line, [(op, reg, addr)])] in execution order"""
    import disasm
    import isa
    text, _ = disasm.program(words(name), isa.Table.load(), name)
    out = []
    for line in text.splitlines():
        if line.startswith(';') or line.startswith('.') or not line.strip():
            continue
        code = line.split(';')[0]
        out.append((line, [(op, int(r), int(a, 16)) for op, r, a in XFER.findall(code)]))
    return out


def refs(name, lo, hi):
    """(loads, stores): sets of addresses in [lo, hi)"""
    ld, st = set(), set()
    for _, xs in instructions(name):
        for op, _, a in xs:
            if lo <= a < hi:
                (ld if op == 'ld' else st).add(a)
    return ld, st


def forwards(name):
    """pairs (source, destination): a store of rX whose value is the load of rX in the previous instruction
    (the core's model: a store sees a load of the instruction before; the pattern `ld r0,[a]` / `st r0,[b] | ld r0,[c]`
    is how every Roland main copies its inputs)"""
    ins = instructions(name)
    out = []
    for k in range(1, len(ins)):
        prev = {r: a for op, r, a in ins[k - 1][1] if op == 'ld'}
        for op, r, a in ins[k][1]:
            if op == 'st' and r in prev:
                out.append((prev[r], a))
    return out


def dt1_table():
    """bank 1 (DT1) base of DSP0..5 in the slave OS bank table (s_boot 0x0111C580), None if s_boot is not here"""
    if not os.path.exists(S_BOOT):
        return None
    import struct
    d = open(S_BOOT, 'rb').read()
    return [struct.unpack_from('<I', d, 0x0111C580 - 0x01100000 + 4 * n)[0] for n in range(6)]


def summary():
    dco_out = set()
    for p in ('DCO1_SAW', 'DCO1_SQR', 'DCO2_SAW', 'DCO2_SQR', 'DCO2_NOISE'):
        dco_out |= refs(p, 0x2200, 0x2300)[1]
    ld2, st2 = refs('MAIN2', 0x2200, 0x2300)
    print('DCO slot outputs (bank 1):', ' '.join('%04x' % a for a in sorted(dco_out)))
    print('MAIN2 reads them +1      :', ' '.join('%04x' % a for a in sorted(ld2 & {a + 1 for a in dco_out})))
    print('MAIN2 reads them as is   :', ' '.join('%04x' % a for a in sorted(ld2 & dco_out)) or '-')
    print('MAIN2 Z <- M forwards    :', ' '.join('%04x->%04x' % p for p in forwards('MAIN2') if p[1] >> 8 == 0x84))
    for m in ('MAIN0', 'MAIN1', 'MAIN4', 'MAIN5'):
        ld, st = refs(m, 0xA400, 0xA800)
        print(f'{m} pair window: loads', ' '.join('%04x' % a for a in sorted(ld)), ' stores',
              ' '.join('%04x' % a for a in sorted(st)))
    print('MAIN4 IOMD inputs -> bank 1:', ' '.join('%04x->%04x' % p for p in forwards('MAIN4') if p[0] >> 8 == 0x60))
    print('MAIN4 IOMD outputs:', ' '.join('%04x' % a for a in sorted(refs('MAIN4', 0x6000, 0x6100)[1])))
    t = dt1_table()
    print('DT1 bases DSP0..5 (s_boot 0x0111C580):', ' '.join('%08x' % x for x in t) if t else 'no s_boot.bin')


if __name__ == '__main__':
    if not available():
        raise SystemExit(f'{PROGRAMS_H} missing: run engine/build.sh once')
    if sys.argv[1:] == ['--summary']:
        summary()
    else:
        for line, xs in instructions(sys.argv[1]):
            if any(a >> 13 in (1, 3, 4, 5) for _, _, a in xs):
                print(line.split(';')[0].rstrip())
