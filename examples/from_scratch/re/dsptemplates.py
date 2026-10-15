"""Infer the template of each instruction class from the position map (dspslots.py output) and the corpus.

    python3 dsptemplates.py work/dspclass/slots_top60.txt [more ...] > work/dspclass/templates.txt

Position labels from the map (probes 3c01 / 5c01): A (r3/r5 = .75), M (1.5 = MAC first word), T (loads a
memory value into r3/r5), '?' (no effect of its own). The '?' positions are resolved with the class's most
frequent corpus instruction: after the transfers come the MAC second words (N, one per M), then the
references (R, one per transfer), then immediates (I: as many words as the operand modes of the A/M/N words
need); '?' before the transfers (or left over) are C (another unit, not decoded yet).
"""
import collections, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dspclass   # noqa: E402

IMMBITS = {0: 0, 1: 16, 2: 24, 3: 32}


def labels(entry):
    out = []
    for pos in entry.split(' | '):
        _, res = pos.split(':', 1)
        a, b = (res.split(' / ') + [''])[:2]
        if a == 'X' or b == 'X':
            out.append('X')
        elif 'r3=0.75' in a and 'r5=0.75' in b:
            out.append('A')
        elif 'r3=1.5' in a and 'r5=1.5' in b:
            out.append('M')
        elif re.search(r'r3=', a) and re.search(r'r5=', b):
            out.append('T')
        else:
            out.append('?')
    return out


def imm_words(ins, lab):
    bits = 0
    body = ins[1:]
    for w, k in zip(body, lab):
        if k == 'A':
            bits += IMMBITS[(w >> 4) & 3]
        elif k == 'M':
            bits += IMMBITS[(w >> 4) & 3]
        elif k == 'N':
            bits += IMMBITS[(w >> 4) & 3] + IMMBITS[(w >> 10) & 3]
    return (bits + 15) // 16


def resolve(lab, ins):
    lab = list(lab)
    nM, nT = lab.count('M'), lab.count('T')
    last_t = max([i for i, k in enumerate(lab) if k == 'T'], default=-1)
    after = [i for i, k in enumerate(lab) if k == '?' and i > last_t]
    before = [i for i, k in enumerate(lab) if k == '?' and i < last_t]
    for i in before:
        lab[i] = 'C'
    q = list(after)
    for _ in range(nM):
        if q:
            lab[q.pop(0)] = 'N'
    for _ in range(nT):
        if q:
            lab[q.pop(0)] = 'R'
    ni = imm_words(ins, lab) if ins else 0
    rest = q
    # immediates are at the end; whatever stays between is C
    for i in rest[len(rest) - ni:] if ni else []:
        lab[i] = 'I'
    for i in rest[:len(rest) - ni] if ni else rest:
        lab[i] = 'C'
    return ''.join(lab)


def main():
    c = dspclass.corpus()
    best = {}
    for ins, f in c.items():
        k = (ins[0] >> 5, len(ins))
        if k not in best or f > c[best[k]]:
            best[k] = ins
    for path in sys.argv[1:]:
        for line in open(path):
            m = re.match(r'(0x[0-9a-f]+) ([0-9a-f]{4}) len (\d+): (.*)', line.strip())
            if not m:
                continue
            cls, length, entry = int(m.group(1), 16), int(m.group(3)), m.group(4)
            lab = labels(entry)
            ins = best.get((cls, length))
            t = resolve(lab, ins)
            ex = ' '.join(f'{w:04x}' for w in ins) if ins else ''
            print(f'{cls:#06x} {length:2d} {"".join(lab):14s} {t:14s} {ex}')


if __name__ == '__main__':
    main()
