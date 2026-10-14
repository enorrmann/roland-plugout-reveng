"""Collect DSP microcode blobs from SYSTEM-8 images and print word statistics.

usage: python3 dsp_corpus.py outdir name=image.bin [name=image.bin ...]

A program is found through its descriptor {u32 dsp_addr, u32 bytes, u32 bytes,
u32 src} (dsp_addr < 0x10000 and even) whose src points at a blob starting with
70 ff 02 0a. The load address of each image is guessed from those descriptors.
Unique blobs are written to outdir as <name>_<dsp_addr>_<src>.bin.
"""
import sys, os, struct, hashlib, math, collections
sys.path.insert(0, os.path.dirname(__file__))
from dspmc import words as dsp_words

SIG = b'\x70\xff\x02\x0a'

def descriptors(d):
    for o in range(0, len(d) - 16, 4):
        a, n, n2, src = struct.unpack_from('<4I', d, o)
        if n == n2 and 0 < n < 0x10000 and a < 0x10000 and a % 2 == 0:
            yield a, n, src

def scan(d):
    sig, i = set(), d.find(SIG)
    while i >= 0:
        sig.add(i); i = d.find(SIG, i + 2)
    votes = collections.Counter(src - s for _, _, src in descriptors(d) for s in sig
                                if src > s and (src - s) % 0x100000 == 0)
    if not votes:
        return None, []
    base = votes.most_common(1)[0][0]
    return base, [(a, n, src) for a, n, src in descriptors(d) if src - base in sig]

def entropy(c):
    t = sum(c.values())
    return -sum(v / t * math.log2(v / t) for v in c.values())

def main():
    out = sys.argv[1]; os.makedirs(out, exist_ok=True)
    blobs = {}
    for arg in sys.argv[2:]:
        name, path = arg.split('=', 1)
        d = open(path, 'rb').read()
        base, progs = scan(d)
        if base is None:
            print(f'{name}: no descriptors'); continue
        for a, n, src in progs:
            blob = d[src - base: src - base + n]
            h = hashlib.md5(blob).hexdigest()
            if h not in blobs:
                blobs[h] = blob
                open(os.path.join(out, f'{name}_{a:04x}_{src:08x}.bin'), 'wb').write(blob)
        print(f'{name}: base {base:#x}, {len(progs)} descriptors')
    words, even, odd = collections.Counter(), collections.Counter(), collections.Counter()
    nxt = collections.defaultdict(collections.Counter)
    for b in blobs.values():
        w = dsp_words(b)
        for i, x in enumerate(w):
            words[x] += 1; (even if i % 2 == 0 else odd)[x] += 1
            if i + 1 < len(w): nxt[x][w[i + 1]] += 1
    print(f'{len(blobs)} unique programs, {sum(map(len, blobs.values()))} bytes, '
          f'length % 4: {collections.Counter(len(b) % 4 for b in blobs.values())}')
    print(f'word entropy even {entropy(even):.2f} / odd {entropy(odd):.2f} (execution order)')
    for k, v in words.most_common(40):
        print(f'{k:04x} n={v:5d} H(next)={entropy(nxt[k]):5.2f}  next:',
              ' '.join(f'{a:04x}:{b}' for a, b in nxt[k].most_common(4)))

if __name__ == '__main__':
    main()
