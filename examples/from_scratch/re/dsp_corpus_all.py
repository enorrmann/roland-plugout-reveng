"""Collect EVERY DSP program found in Roland images (2026-10-17): descriptor scan (dsp_corpus.scan) plus a
signature scan (a 4-byte-aligned `0a02 ff70` that is not an end marker, parsed instruction by instruction up to
the end marker `0a02 ff70 0b02 00f8 0001`). Unique programs (by word sequence) go to OUTDIR as
<source>_<offset>.txt (16-bit words in execution order); prints which ones are new w.r.t. work/corpus.

    python3 dsp_corpus_all.py OUTDIR name=image.bin ...
"""
import hashlib, os, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import dsp_corpus, dspmc

SIG = b'\x70\xff\x02\x0a'
END = b'\x70\xff\x02\x0a\xf8\x00\x02\x0b'


def by_signature(d):
    i = d.find(SIG)
    while i >= 0:
        if i % 2 == 0 and d[i:i + 8] != END:
            w, p = [], i
            while p + 4 <= len(d) and len(w) < 0x4000:
                lo, hi = struct.unpack_from('<2H', d, p)
                w += [hi, lo]
                p += 4
                if w[-4:] == [0x0A02, 0xFF70, 0x0B02, 0x00F8]:
                    nxt = struct.unpack_from('<H', d, p)[0] if p + 2 <= len(d) else 1
                    w.append(nxt)
                    break
            ok = w[-5:-1] == [0x0A02, 0xFF70, 0x0B02, 0x00F8]
            try:
                n = sum(1 for _ in dspmc_instructions(w))
            except ValueError:
                ok = False
            if ok:
                yield i, w
        i = d.find(SIG, i + 2)


def parses(d, off, n):
    """the bytes form a whole number of instructions (ALU/transfer words, no length 0)"""
    try:
        w = dspmc.words(d[off:off + n], d[off + n:off + n + 2] if n % 4 else None)
        return sum(len(i) for _, i in dspmc_instructions(w)) == len(w) and len(w) > 3
    except (ValueError, struct.error):
        return False


def dspmc_instructions(w):
    import dspdis
    return dspdis.instructions(w)


def key(w):
    return hashlib.md5(struct.pack('<%dH' % len(w), *w)).hexdigest()


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    known = set()
    cdir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'work', 'corpus')
    for f in os.listdir(cdir):
        if f.endswith('.bin'):
            known.add(key(dspmc.blob_words(os.path.join(cdir, f))))
    seen = {}
    for arg in sys.argv[2:]:
        name, path = arg.split('=', 1)
        d = open(path.split('@')[0], 'rb').read()
        progs = []
        if '@' in path:                       # name=image.bin@0xLOAD: any descriptor pointing into the image
            path, load = path.split('@')
            d = open(path, 'rb').read()
            base = int(load, 0)
            desc = [(a, n, src) for a, n, src in dsp_corpus.descriptors(d)
                    if base <= src < base + len(d) - 8 and n >= 8 and parses(d, src - base, n)]
        else:
            base, desc = dsp_corpus.scan(d)
        for a, n, src in desc:
            off = src - base
            progs.append((off, dspmc.words(d[off:off + n], d[off + n:off + n + 2] if n % 4 else None)))
        progs += list(by_signature(d))
        new = 0
        for off, w in progs:
            k = key(w)
            if k in seen:
                continue
            seen[k] = f'{name}_{off:07x}'
            if k not in known:
                new += 1
                open(os.path.join(out, f'{name}_{off:07x}.txt'), 'w').write(' '.join(f'{x:04x}' for x in w) + '\n')
        print(f'{name}: {len(progs)} programs found ({len(desc)} by descriptor), {new} new unique', flush=True)
    print(f'total unique {len(seen)}, already in corpus {len(set(seen) & known)} of {len(known)}')


if __name__ == '__main__':
    main()
