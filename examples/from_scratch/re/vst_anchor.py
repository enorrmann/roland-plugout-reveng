"""Anchor VST state constants in the DSP microcode.

    python3 vst_anchor.py work/ghidra/out/init.c work/corpus/JX_*.bin

init.c = Ghidra output of CJx3pSim's initialisation (0x180391630) passed
through ghidra/annotate.py. Every `st_<off> = 0x<float bits>;` (both sample
rate sets) is encoded in the DSP's RAW format (dspnum.py) and searched as a
32-bit immediate (two consecutive words, execution order) in the programs;
prints offset, value, and where it occurs. Trivial values (0, +-1, 0.5, ...)
are skipped: they are everywhere.
"""
import collections, os, re, struct, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dspmc, dspnum, dspdis

TRIVIAL = {0.0, 1.0, -1.0, 0.5, -0.5, 0.25, -0.25, 2.0, 4.0, 0.75}


def constants(path):
    out = collections.defaultdict(set)
    for m in re.finditer(r'\b(st_[0-9a-f]+|P_\w+) = (0x[0-9a-f]+);', open(path).read()):
        v = struct.unpack('<f', struct.pack('<I', int(m.group(2), 16)))[0]
        if v == v and v not in TRIVIAL and abs(v) < 500 and abs(v) > 1e-9:
            out[m.group(1)].add(v)
    return out


def imm_index(progs):
    idx = collections.defaultdict(list)
    for name, w in progs.items():
        for a, ins in dspdis.instructions(w):
            for k in range(1, len(ins) - 1):
                idx[ins[k] << 16 | ins[k + 1]].append((name, a))
    return idx


def main():
    consts = constants(sys.argv[1])
    progs = {os.path.basename(p)[:-4]: dspmc.blob_words(p) for p in sys.argv[2:]}
    idx = imm_index(progs)
    keys = sorted(idx)
    import bisect
    for var in sorted(consts, key=lambda s: int(s[3:], 16) if s.startswith('st_') else 0):
        for v in sorted(consts[var]):
            r = dspnum.encode(v)
            hits = []
            i = bisect.bisect_left(keys, r - 4)
            while i < len(keys) and keys[i] <= r + 4:
                hits += idx[keys[i]]; i += 1
            if hits:
                where = collections.Counter(n for n, a in hits)
                print(f'{var:10s} {v: .9g}  {r:08x}  ' + ', '.join(f'{n}:{",".join(str(a) for nn, a in hits if nn == n)}' for n in where))


if __name__ == '__main__':
    main()
