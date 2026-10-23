"""Does a SYSTEM-8 PLUG-OUT's DSP microcode fit in the SYSTEM-1? Time per DSP, traffic between DSPs, best placement.

    python3 system-1/dsp_fit.py                       # JX-3P, 4 voices (one SYSTEM-8 slave), DCO SAW/SAW, no effect
    python3 system-1/dsp_fit.py --dco1 sqr --dco2 noise --fx ch
    python3 system-1/dsp_fit.py --dco2 none           # one DCO only (DCO2 slot empty)
    python3 system-1/dsp_fit.py --voices 2            # only the voice-0/1 chain (DSP0 -> DSP2 -> DSP4)
    python3 system-1/dsp_fit.py --budget 900 --reserve-ma 60 --samples 4000

Study only (docs/log/VOICE_ALLOCATION_LOG.md §7 scope): Roland's programs are read at run time from the local copy
`examples/from_scratch/engine/build/programs.h` (made by engine/build.sh from plugins/JX-3P), run in the C emulator
(`examples/from_scratch/emu/cchain.py`) and disassembled with `emu/listing.py`. Nothing is copied into the repo.
MAIN0/MAIN1 of programs.h carry the same-size envelope patches of engine/programs.py; the count is the same.

What it measures (system-1/DSP_FIT.md):
 1. per program: bytes, static instructions, executed instructions per sample (min / mean / max over the run, with
    random bank-5 inputs and parameters changed every 250 samples, so the jumps take both ways);
 2. links between the six SYSTEM-8 DSPs, in words per sample: MXM (Z 84nn -> M 80nn / O 82nn of DSP n+2, S 86nn
    back), the pair window A400-A7FF, and the audible IOMD output 6020/6021;
 3. placement on the SYSTEM-1 (slave A/B, master A/B): every assignment of the units (one main each; DCO and FX
    slots on the chip of their main, because they talk to it through its bank 1) that keeps each DSP within the
    budget and the chip-to-chip words within the IOMD bus (slave -> master 6006-600D = 8, master -> slave
    600E-6021 = 20). Prints the best one (least loaded DSP peak) or why nothing fits.
"""
import argparse
import itertools
import os
import random
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..'))
EMU = os.path.join(ROOT, 'examples', 'from_scratch', 'emu')
sys.path.insert(0, EMU)
import chain     # noqa: E402
import cchain    # noqa: E402
import listing   # noqa: E402

DCO1 = {'saw': 'DCO1_SAW', 'sqr': 'DCO1_SQR'}
DCO2 = {'saw': 'DCO2_SAW', 'sqr': 'DCO2_SQR', 'noise': 'DCO2_NOISE', 'none': None}
FX = {'od': 'FX_OD', 'ds': 'FX_DS', 'ch': 'FX_CH', 'fz': 'FX_FZ', 'cr': 'FX_CR', 'ph': 'FX_PH'}
S1_DSPS = ('slave A', 'slave B', 'master A', 'master B')
CHIP = (0, 0, 1, 1)                    # 0 = slave, 1 = master
UP, DOWN = 8, 20                       # IOMD words slave -> master (6006-600D), master -> slave (600E-6021)


def static_count(name):
    return len(listing.instructions(name))


def measure(progs, samples, seed):
    """{dsp: [program names]} -> {(dsp, k): (min, mean, max)} executed instructions per sample of program k.
    Program k alone is (prefix k) - (prefix k-1): the slots run after the main and read what it wrote."""
    out = {}
    for dsp, names in progs.items():
        prev = None
        for k in range(len(names)):
            per = run_dsp(progs, dsp, names[:k + 1], samples, seed)
            out[(dsp, k)] = per if prev is None else [a - b for a, b in zip(per, prev)]
            prev = per
    return {key: (min(v), sum(v) / len(v), max(v)) for key, v in out.items()}


def run_dsp(progs, dsp, names, samples, seed):
    cache = {}

    def prog(n):
        if n not in cache:
            cache[n] = chain.Program(listing.words(n), n)
        return cache[n]
    p = {d: (prog(ns[0]), [prog(x) for x in ns[1:]]) for d, ns in progs.items()}
    p[dsp] = (prog(names[0]), [prog(x) for x in names[1:]])
    s = cchain.Slave(p, native=False, threads=False)
    r = random.Random(seed)
    per = []
    for t in range(samples):
        if t % 250 == 0:
            for d in range(6):
                for i in range(0x80):
                    s.p_bank(d, i, r.choice([0.0, 1.0, r.uniform(-1, 1)]))
            for i in range(0x40):
                s.params[0xC000 + i] = r.choice([0.0, 1.0, r.uniform(0, 1)])
        a = s.ninstr_dsp(dsp)
        s.step()
        per.append(s.ninstr_dsp(dsp) - a)
    return per[samples // 10:]          # skip the first samples (empty rings)


def links(mains):
    """[(src dsp, dst dsp, kind, words)] between the SYSTEM-8 DSPs present in `mains` ({dsp: main name})"""
    ref = {d: listing.refs(n, 0x6000, 0x8800) + listing.refs(n, 0xA400, 0xA800) for d, n in mains.items()}
    out = []
    for d, (io_ld, io_st, pw_ld, pw_st) in ref.items():
        z = {a & 0x1FF for a in io_st if 0x8400 <= a < 0x8600}
        for e in (d + 2,) if d + 2 in ref else ():                     # forward MXM: Z of d -> M/O of d+2
            zone = 0x8000 if d % 2 == 0 else 0x8200
            got = {a & 0x1FF for a in ref[e][0] if zone <= a < zone + 0x200}
            if z & got:
                out.append((d, e, 'MXM', len(z & got)))
        if d - 2 in ref:                                                # back MXM: M/O of d -> S of d-2
            zone = 0x8000 if d % 2 == 0 else 0x8200
            st = {a & 0x1FF for a in io_st if zone <= a < zone + 0x200}
            got = {a & 0x1FF for a in ref[d - 2][0] if 0x8600 <= a < 0x8800}
            if st & got:
                out.append((d, d - 2, 'MXM S', len(st & got)))
        e = d ^ 1                                                       # pair window A400-A7FF
        if e in ref and pw_st & ref[e][2]:
            out.append((d, e, 'A4xx', len(pw_st & ref[e][2])))
        audio = io_st & set(chain.AUDIBLE)
        if audio:
            out.append((d, 'out', 'IOMD', len(audio)))
    return out


def place(units, lk, budget, reserve):
    """units: [(name, dsp8, cost, is_slot)]; returns (feasible, best) with best = (peak, loads, assignment, up, down)"""
    mains = [u for u in units if not u[3]]
    slots = [u for u in units if u[3]]
    best = None
    feas = False
    for a in itertools.product(range(4), repeat=len(mains)):
        where = {u[1]: s1 for u, s1 in zip(mains, a)}
        for b in itertools.product((0, 1), repeat=len(slots)):
            asg = {u[0]: where[u[1]] for u in mains}
            load = [0.0, 0.0, float(reserve), 0.0]
            for u, s1 in zip(mains, a):
                load[s1] += u[2]
            for u, k in zip(slots, b):
                s1 = 2 * CHIP[where[u[1]]] + k
                asg[u[0]] = s1
                load[s1] += u[2]
            up = down = 0
            for src, dst, kind, n in lk:
                cs = CHIP[where[src]]
                cd = 1 if dst == 'out' else CHIP[where[dst]]           # the audio goes to master B
                if cs == 0 and cd == 1:
                    up += n
                elif cs == 1 and cd == 0:
                    down += n
            ok = max(load) <= budget and up <= UP and down <= DOWN
            key = (not ok, max(0, up - UP) + max(0, down - DOWN), max(load))
            if best is None or key < best[0]:
                best = (key, load, asg, up, down)
            feas |= ok
    return feas, best


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--dco1', choices=DCO1, default='saw')
    ap.add_argument('--dco2', choices=DCO2, default='saw')
    ap.add_argument('--fx', choices=FX, default=None, help='effect slot of DSP4 (default: none)')
    ap.add_argument('--voices', type=int, choices=(2, 4), default=4)
    ap.add_argument('--budget', type=float, default=925, help='instructions per sample and DSP (SYSTEM-8: 925-945)')
    ap.add_argument('--reserve-ma', type=float, default=40,
                    help="master A instructions taken by the OS output program at 0x82 [abierto]")
    ap.add_argument('--samples', type=int, default=2000)
    ap.add_argument('--seed', type=int, default=1)
    a = ap.parse_args()
    if not listing.available():
        raise SystemExit(f'{listing.PROGRAMS_H} missing: run examples/from_scratch/engine/build.sh once')

    dsps = range(6) if a.voices == 4 else (0, 2, 4)
    progs = {d: [f'MAIN{d}'] for d in range(6)}
    for d in (2, 3):
        progs[d] += [DCO1[a.dco1]] + ([DCO2[a.dco2]] if DCO2[a.dco2] else [])
    if a.fx:
        progs[4].append(FX[a.fx])
    print(f'JX-3P, {a.voices} voices, DCO1 {a.dco1}, DCO2 {a.dco2}, FX {a.fx or "-"}; {a.samples} samples, '
          f'budget {a.budget:.0f}, master A reserve {a.reserve_ma:.0f}\n')

    cost = measure(progs, a.samples, a.seed)
    print('SYSTEM-8 DSP  program      bytes  static  executed/sample min  mean   max')
    units, total = [], 0.0
    for d in range(6):
        for k, n in enumerate(progs[d]):
            nb = len(listing.words(n)) * 2
            mn, me, mx = cost[(d, k)]
            used = d in dsps
            print(f'  DSP{d}{"" if used else "*"}      {n:<11} {nb:6d} {static_count(n):7d}'
                  f'           {mn:6.0f} {me:6.1f} {mx:5.0f}')
            if used:
                units.append((n if k == 0 else f'{n}@{d}', d, mx, k > 0))
                total += mx
    if a.voices == 2:
        print('  (* not used for 2 voices)')
    print(f'  total of the used programs (max): {total:.0f} instructions per sample')
    s1_total = 4 * a.budget - a.reserve_ma
    print(f'  SYSTEM-1 has 4 DSPs: {s1_total:.0f} ({total / s1_total:.0%} of it)\n')

    lk = links({d: f'MAIN{d}' for d in dsps})
    print('links between SYSTEM-8 DSPs (words per sample):')
    for src, dst, kind, n in lk:
        print(f'  DSP{src} -> {"output" if dst == "out" else "DSP%d" % dst:<7} {kind:<6} {n:3d}')
    print()

    feas, (key, load, asg, up, down) = place(units, lk, a.budget, a.reserve_ma)
    print('best SYSTEM-1 placement' + ('' if feas else ' (DOES NOT FIT)') + ':')
    for s1 in range(4):
        names = [n for n, w in asg.items() if w == s1]
        extra = f' + OS {a.reserve_ma:.0f}' if s1 == 2 else ''
        flag = '  OVER' if load[s1] > a.budget else ''
        print(f'  {S1_DSPS[s1]:<9} {load[s1]:6.0f} / {a.budget:.0f}{flag}  {" ".join(names)}{extra}')
    print(f'  slave -> master {up} / {UP} IOMD words{"  OVER" if up > UP else ""}, '
          f'master -> slave {down} / {DOWN}{"  OVER" if down > DOWN else ""}')
    same = [(s, d) for s, d, k, _ in lk if d != 'out' and k.startswith('MXM')]
    print(f'  MXM links to rewire (no MXM seen in SYSTEM-1 programs): {len(same)}')
    print('\nverdict:', 'fits (time and bus), still unmeasured on the SYSTEM-1' if feas else
          'does not fit on the SYSTEM-1 with these programs')
    return 0 if feas else 1


if __name__ == '__main__':
    sys.exit(main())
