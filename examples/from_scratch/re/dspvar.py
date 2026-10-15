"""Which words of an instruction are operands (change between copies of the same code)?

    python3 dspvar.py work/corpus/*.bin

Copies of the same code: the two voice halves of a program (found by shifting
the instruction sequence against itself) and programs whose instruction head
sequences are identical (DCO1/DCO2, effect instances). For aligned instructions
with equal words in all but some positions, those positions are operands.
Prints, per instruction head, the body positions seen varying, with examples.
"""
import collections, glob, os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dspmc, dspdis


def heads(ins):
    return [i[0] for _, i in ins]


def self_shift(ins):
    h = heads(ins)
    n = len(h)
    best = (0, 0)
    for d in range(max(8, n // 4), n - 8):
        m = sum(1 for i in range(n - d) if h[i] == h[i + d])
        if m > best[0]:
            best = (m, d)
    return best


def aligned_pairs(progs):
    out = []
    for name, ins in progs.items():
        m, d = self_shift(ins)
        if m > 0.3 * len(ins):
            out += [(ins[i][1], ins[i + d][1]) for i in range(len(ins) - d)]
    names = sorted(progs)
    for i, a in enumerate(names):
        for b in names[i + 1:]:
            A, B = progs[a], progs[b]
            if len(A) == len(B) and heads(A) == heads(B) and A != B:
                out += [(x[1], y[1]) for x, y in zip(A, B)]
    return out


def main():
    progs = {os.path.basename(f): list(dspdis.instructions(dspmc.blob_words(f))) for f in sys.argv[1:]}
    var = collections.defaultdict(collections.Counter)
    seen = collections.Counter()
    for x, y in aligned_pairs(progs):
        if len(x) != len(y) or x[0] != y[0]:
            continue
        diff = [k for k in range(1, len(x)) if x[k] != y[k]]
        if 0 < len(diff) <= len(x) // 2 + 1:
            seen[x[0]] += 1
            for k in diff:
                var[x[0]][k] += 1
    for h, n in seen.most_common(400):
        print(f'{h:04x} x{n:4d} varying body positions: ' + ' '.join(f'{k}:{c}' for k, c in sorted(var[h].items())))


if __name__ == '__main__':
    main()
