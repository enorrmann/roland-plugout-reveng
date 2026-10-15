"""Measure corpus instructions on a master DSP and fit what each one computes (exact, dspfit search).

    python3 dspclass.py list                       # header classes by frequency, with sample instructions
    python3 dspclass.py run 0x0a8 [-k 4] [-n 6]    # the k most frequent instructions of class 0x0a8
    python3 dspclass.py run all -k 3               # every class, by frequency
    python3 dspclass.py ins "1503 2987 7505" ...   # given instructions

For each instruction: its memory references (one per transfer, dspdis.fields) are moved to a free area
of master DSP4 bank 0 ([0790+k]) and preloaded with distinct values; r0..r7 get random values; the
instruction runs; r0..r7 and the memory words are read back (dspmaster.Session, dsppeek transport).
Every output that changed is fitted over r0..r7 and m0..m(k-1) (up to 3 monomials of degree <= 2).
"""
import argparse, collections, itertools, json, os, random, sys
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dspmc, dspdis                    # noqa: E402
from dsppeek import Peek, raw_value     # noqa: E402
from dspmaster import Session, words_of  # noqa: E402
from dspprobe import hi16               # noqa: E402

CORPUS = os.path.join(HERE, '..', '..', '..', 'work', 'corpus')
REGOUT, MEM = 0x780, 0x790
RAW = None
PAD = int(os.environ.get('PAD', '0'))   # 0001 no-ops between the instruction and the stores
VALUES = [k / 8 for k in range(-7, 8) if k] + [1.5, -1.5]


def corpus():
    c = collections.Counter()
    for f in sorted(os.listdir(CORPUS)):
        w = dspmc.words(open(os.path.join(CORPUS, f), 'rb').read())
        for _, ins in dspdis.instructions(w):
            c[tuple(ins)] += 1
    return c


def remap(ins):
    """instruction with its transfer references moved to MEM + k; returns (words, k, moves)"""
    ops, moves, bridge, rest = dspdis.fields(list(ins))
    if not moves:
        return list(ins), 0, []
    body = list(ins)
    idx = [i for i, w in enumerate(body) if i > 0 and dspdis.is_move(w)]
    # the refs follow the last transfer after the bridge words, in transfer order
    start = idx[-1] + 1 + len(bridge)
    for k in range(len(moves)):
        body[start + k] = MEM + k
    return body, len(moves), moves


def program(ins, regs, mem):
    L = ['0a02 ff70']
    for k, v in enumerate(mem):                               # memory preload through r0
        L += [f'0903 0050 {hi16(v):04x}', f'0c03 8fe0 {MEM + k:04x}']
    L += [f'0903 {r:x}050 {hi16(v):04x}' for r, v in enumerate(regs)]
    L += [' '.join(f'{w:04x}' for w in ins)] + ['0001'] * PAD
    L += [f'0c03 {8 + r:x}fe0 {REGOUT + r:04x}' for r in range(8)]
    L += ['0a02 ff70', '0b02 00f8', '0001']
    return words_of('\n'.join(L))


def fit_robust(X, names, y, x0):
    """exact fit; else with one probe dropped (a bad read); an output that differs from its input in
    one probe only is a bad read of an unchanged word (None)"""
    y, x0 = np.array(y), np.array(x0)
    bad = np.abs(y - x0) > 1e-9
    if bad.sum() <= 1 and len(y) >= 5:
        return None
    f = fit(X, names, y)
    if not f.startswith('?'):
        return f
    for i in range(len(y)):
        keep = [j for j in range(len(y)) if j != i]
        g = fit(X[keep], names, y[keep])
        if not g.startswith('?'):
            return g + ' ~'
    return f


def fit(X, names, y, deg=2, maxterms=3):
    mons = [()]
    for d in range(1, deg + 1):
        mons += list(itertools.combinations_with_replacement(range(len(names)), d))
    A = np.array([[np.prod([row[i] for i in m]) if m else 1.0 for m in mons] for row in X])
    y = np.array(y)
    tol = 1e-4 * max(1.0, float(np.max(np.abs(y))))
    if np.max(np.abs(y)) < tol:
        return '0'
    for k in range(1, maxterms + 1):
        for idx in itertools.combinations(range(len(mons)), k):
            sub = A[:, idx]
            c, *_ = np.linalg.lstsq(sub, y, rcond=None)
            if float(np.max(np.abs(sub @ c - y))) < tol:
                return ' '.join(f'{v:+.6g}' + ('·' + '·'.join(names[i] for i in mons[j]) if mons[j] else '')
                                for v, j in zip(c, idx))
    return '? ' + ' '.join(f'{v:+.4g}' for v in y)


def measure(ses, ins, n=6, seed=1):
    if ins and isinstance(ins[0], tuple):          # a sequence: only the last instruction's references are moved
        pre = [w for part in ins[:-1] for w in part]
        body, k, moves = remap(ins[-1])
        body = pre + body
        ins = tuple(pre) + tuple(ins[-1])
    else:
        body, k, moves = remap(ins)
    rnd = random.Random(seed)
    names = [f'r{i}' for i in range(8)] + [f'm{i}' for i in range(k)]
    X, Y = [], []
    for _ in range(n):
        regs = [rnd.choice(VALUES) for _ in range(8)]
        mem = [rnd.choice(VALUES) for _ in range(k)]
        ses.load(program(body, regs, mem))
        out = ses.read(REGOUT, 0x10 + k)
        after = [raw_value(w) for w in out[:8]] + [raw_value(w) for w in out[0x10:0x10 + k]]
        X.append(regs + mem)
        Y.append(after)
    X, Y = np.array(X), np.array(Y)
    res = []
    for j, nm in enumerate(names):
        if np.max(np.abs(Y[:, j] - X[:, j])) > 1e-9:
            f = fit_robust(X, names, Y[:, j], X[:, j])
            if f is not None:
                res.append(f'{nm} = {f}')
    mv = ' '.join(f'[m{i}]<-r{reg - 8}' if wr else f'r{reg}<-[m{i}]' for i, (reg, wr, _) in enumerate(moves))
    if RAW is not None:
        RAW.write(json.dumps({'ins': [f'{w:04x}' for w in ins], 'k': k, 'X': X.tolist(), 'Y': Y.tolist()}) + '\n')
        RAW.flush()
    return body, mv, res or ['no change']


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['list', 'run', 'ins'])
    ap.add_argument('args', nargs='*')
    ap.add_argument('-k', type=int, default=4)
    ap.add_argument('-n', type=int, default=7)
    ap.add_argument('--raw', help='append raw probes (jsonl) here')
    a = ap.parse_args()
    c = corpus()
    global RAW
    RAW = open(a.raw, 'a') if a.raw else None
    if a.cmd == 'list':
        by = collections.defaultdict(collections.Counter)
        for ins, f in c.items():
            by[ins[0] >> 5][ins] += f
        for cls, ctr in sorted(by.items(), key=lambda x: -sum(x[1].values())):
            ex = ' | '.join(' '.join(f'{w:04x}' for w in i) for i, _ in ctr.most_common(2))
            print(f'{cls:#06x} {sum(ctr.values()):5d} {len(ctr):4d}  {ex}')
        return
    if a.cmd == 'run':
        by = collections.defaultdict(collections.Counter)
        for ins, f in c.items():
            by[ins[0] >> 5][ins] += f
        classes = sorted(by, key=lambda x: -sum(by[x].values())) if a.args[0] == 'all' else [int(x, 0) for x in a.args]
        todo = [i for cls in classes for i, _ in by[cls].most_common(a.k)]
    else:
        todo = [tuple(tuple(int(x, 16) for x in part.split()) for part in s.split(';')) if ';' in s else tuple(int(x, 16) for x in s.split()) for s in a.args]
    excl = {0x0b02}
    with Session(Peek(), size=256) as ses:
        for ins in todo:
            if not isinstance(ins[0], tuple) and ins[0] in excl:
                continue
            body, mv, res = measure(ses, ins, a.n)
            flat = [w for part in ins for w in part] if isinstance(ins[0], tuple) else ins
            print(f'{" ".join(f"{w:04x}" for w in flat):48s} {mv:24s} ' + '; '.join(res), flush=True)


if __name__ == '__main__':
    main()
