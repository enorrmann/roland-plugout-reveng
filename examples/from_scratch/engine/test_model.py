"""Check the parameter model (model_spec.py) against the JX-3P engine in emu1.

1. changes: for every source id and random contexts, the DSP writes of one
   parameter change (vt+0x30)
2. refresh: the final DSP state after start-up for random complete states

usage: python3 test_model.py [n]      (reference only; needs build/proj from build.sh)
"""
import math, os, random, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import model_spec as M
import ref, run_jx

RANGES = {0x6cd: 6, 0x6d4: 6, 0x6d6: 6, 0x6d7: 6, 0x6d9: 6, 0x6db: 6, 0x6dc: 6, 0x6df: 3, 0x6d3: 2,
          0x736: 3, 0x6ff: 3, 0x6fe: 2, 0x701: 2, 0x702: 2, 0x748: 6, 0x700: 25, 0x738: 3,
          0x22c: 3, 0x519: 3, 0x544: 3, 0x56e: 3, 0x73e: 2, 0x4a1: 2, 0x4a2: 2, 0x4a3: 2}
SOURCES = sorted({s for s, *_ in M.OUT if s is not None})


def rand_state(rng):
    return {i: rng.randrange(RANGES.get(i, 256)) for i in M.REFRESH_ORDER}


def parse(t):
    kind = t[0]
    dsp, rest = t[1:].split(':')
    return (kind, int(dsp), int(rest.split('/')[0], 16))


def model_writes(vals, slave, sources):
    w = {}
    for src in sources:
        for s, targets, e, when in M.OUT:
            if s != src:
                continue
            if when is not None and when.ev(vals, slave) == 0:
                continue
            v = e.ev(vals, slave)
            for t in targets:
                w[parse(t)] = v
    return w


def model_state(vals, slave):
    return model_writes(vals, slave, [None] + M.REFRESH_ORDER)


def close(a, b):
    if a is None or b is None:
        return a is b
    return abs(a - b) <= 2e-6 + 0.03 * abs(b)     # knotted curves: a few % at worst


def err(a, b):
    return abs(a - b) / max(abs(b), 1e-3)


def compare(m, j):
    bad = {}
    for k in set(m) | set(j):
        a, b = m.get(k), j.get(k)
        if not close(a, b):
            bad[k] = (a, b)
    return bad


def fmt(k):
    return f'{k[0]}{k[1]}:{k[2]:#x}'


def main():
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 20
    rng = random.Random(1)
    fails = {}
    worst = {}
    quirks = 0
    for slave in (0, 1):
        e = ref.boot({}, slave)
        for it in range(n):
            vals = rand_state(rng)
            for i, v in vals.items():
                run_jx.setp(e, i, v)
            e.call(e.u32(e.vt + 0x24), e.o, 1)      # JX-3P caches some flags: make them current
            for x in SOURCES:
                v = rng.randrange(RANGES.get(x, 256))
                e.log.clear()
                run_jx.setp(e, x, v)
                vals[x] = v
                j = ref.last_writes(e, e.log)
                e.call(e.u32(e.vt + 0x24), e.o, 1)
                m = model_writes(vals, slave, [x])
                bad = compare(m, j)
                quirk = set()
                if bad:
                    fresh = ref.state(vals, slave)      # the JX-3P caches some part flags
                    for k in list(bad):
                        if k in m and close(m[k], fresh.get(k)):
                            quirks += 1
                            quirk.add(k)
                            del bad[k]
                for k in (set(m) & set(j)) - quirk:
                    if j[k] is not None and m[k] is not None:
                        worst[(x, k)] = max(worst.get((x, k), 0), err(m[k], j[k]))
                for k, ab in bad.items():
                    fails.setdefault((x, k), (slave, v, ab))
    print(f'changes: {len(SOURCES)} sources x {n} contexts x 2 slaves, {len(fails)} (source, target) differ'
          f' ({quirks} writes differ only by the JX-3P\'s cached part flags; the model follows its fresh state)')
    for (x, k), (slave, v, ab) in sorted(fails.items())[:40]:
        print(f'  {x:#x} -> {fmt(k)}: slave {slave} value {v}: model {ab[0]} jx {ab[1]}')
    big = sorted(((e, x, k) for (x, k), e in worst.items() if e > 0.005), reverse=True)[:12]
    print('largest relative errors (model approximations):',
          ', '.join(f'{x:#x}->{fmt(k)} {e:.1%}' for e, x, k in big))

    rf = {}
    for slave in (0, 1):
        for it in range(max(2, n // 4)):
            vals = rand_state(rng)
            j = ref.state(vals, slave)
            m = model_state(vals, slave)
            for k, ab in compare(m, j).items():
                rf.setdefault(k, (slave, ab))
    print(f'refresh: {len(rf)} DSP ids differ')
    for k, (slave, ab) in sorted(rf.items())[:40]:
        print(f'  {fmt(k)}: slave {slave}: model {ab[0]} jx {ab[1]}')
    return 1 if fails or rf else 0


if __name__ == '__main__':
    sys.exit(main())
