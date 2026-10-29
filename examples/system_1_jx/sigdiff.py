"""Instruction signatures of the C-S1JX programs that no Roland SYSTEM-1 program has (docs/examples_system_1_jx_README.md
section 12).

    python3 sigdiff.py              # the final programs of the plan (as the build makes them)
    python3 sigdiff.py --units      # per unit (MAIN0-5, DCO1@2/3), with handoff and split, before concatenation
    python3 sigdiff.py --dat plugins/C-S1JXS/Script/Code1.Dat     # the programs of an installed build
    python3 sigdiff.py --corpus     # only the reference sets: what the SYSTEM-1 programs use
    options: --suspects (only features no good device build has), --sig (also whole signatures)

Reference ("seen on the SYSTEM-1") = every DSP program in the program sets of the SYSTEM-1 OS 1.30
(work/sys1_v130/app_master.bin, app_slave.bin; system-1/extract_fw.py) and of the SYSTEM-1 PLUG-OUTs SH-101, SH-2,
PROMARS (plugins/<NAME>_S1_Code1/Script/Code1.Dat or work/unpack/<NAME>_S1_Code1). Those programs run on the device,
so a feature they use is known to be accepted by it. A feature of ours that none of them uses is a suspect, not a
proof: the device may accept it (the SYSTEM-8 Roland programs, a second reference, run it on the same DSP family).

Features of one instruction (each one checked on its own, then the whole signature):
  cl      class (header >> 5)
  cls     class and length
  op      each operation mnemonic (disassembler, asm/disasm.py); `raw` if the instruction is kept verbatim
  acc     per-instruction access counts per (bank, ld/st): `b0 st x2`
  mode    transfer mode byte other than fe (indexed, hidden registers, flags)
  jump    009Y units (jumps) and C units (conditions)
  pred    per C unit: its condition code (`cc N`) and the kind of unit it can skip (an op, or a transfer: `skips st
          b0`); 40TY hidden-register loads
  ntr     number of transfers
  sig     class + sorted ops + sorted accesses: the whole shape of the instruction
"""
import collections
import os
import re
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
for p in ('asm', 'examples/from_scratch/re', 'examples/from_scratch/emu', 'system-1'):
    sys.path.insert(0, os.path.join(ROOT, p))
sys.path.insert(0, HERE)
import dspdis  # noqa: E402
import dspref  # noqa: E402
import disasm  # noqa: E402
import isa  # noqa: E402
import s1_progs as S  # noqa: E402

S1_PLUGOUTS = ('SH-101', 'SH-2', 'PROMARS')
OS_IMAGES = ('app_master.bin', 'app_slave.bin')
KINDS = ('cl', 'cls', 'op', 'acc', 'mode', 'jump', 'pred', 'ntr', 'sig')
_TABLE = None


def table():
    global _TABLE
    if _TABLE is None:
        _TABLE = isa.Table.load()
    return _TABLE


# ---------------------------------------------------------------- features
def transfers(ins):
    """[(ld/st, bank or None, mode byte)] of one instruction (the bank rule of chain.access_violations)"""
    try:
        L = dspref.layout(list(ins))
    except ValueError:
        return None, None
    slot_pos = [i for i, t in enumerate(L.t) if t in 'NS']
    out = []
    for w, t in zip(L.body, L.t):
        if t != 'T':
            continue
        mode, op = (w >> 4) & 0xFF, 'st' if w >> 12 >= 8 else 'ld'
        if mode == 0xFE:
            s = (w & 15) >> 1
            bank = L.body[slot_pos[s]] >> 12 if s < len(slot_pos) else None
        elif mode == 0x28:
            bank = 0x0
        elif mode in (0xDA, 0xDE, 0xDB, 0xDF) or 0xD0 <= mode <= 0xD7:
            bank = 0xA
        else:
            bank = None
        out.append((op, bank, mode))
    return out, L


BANK = {0x0: 'b0', 0x2: 'b1', 0x6: 'iomd', 0x8: 'mxm', 0xA: 'b5', 0xC: 'b6', None: 'reg'}


def features(ins):
    """{kind: set of feature strings} of one instruction"""
    f = {k: set() for k in KINDS}
    cls = ins[0] >> 5
    f['cl'].add(f'{cls:#05x}')
    f['cls'].add(f'{cls:#05x}/{len(ins)}')
    dec = disasm.disassemble_insn(table(), list(ins)) if len(ins) > 1 else None
    ops = []
    if dec is None:
        ops = ['nop1']
    elif dec.status == 'raw':
        ops = ['raw']
    else:
        for part in dec.text.split(' | '):
            m = part.split()[0] if part.split() else ''
            if m not in ('ld', 'st'):
                ops.append(m)
    f['op'] |= set(ops)
    tr, L = transfers(ins)
    acc = collections.Counter()
    if tr is None:
        f['op'].add('nolayout')
        tr = []
    for op, bank, mode in tr:
        if mode == 0x9B:
            f['mode'].add('9b flags')
            continue
        if mode != 0xFE:
            f['mode'].add(f'{mode:02x}')
        acc[(BANK.get(bank, f'bank{bank:x}' if bank is not None else 'bank?'), op)] += 1
    for (b, op), n in acc.items():
        f['acc'].add(f'{b} {op} x{n}')
    if L is not None:
        if 'J' in L.t:
            f['jump'].add('jump')
        tpos = [i for i, t in enumerate(L.t) if t == 'T']
        for w, t in zip(L.body, L.t):
            if t != 'C':
                continue
            if w >> 8 == 0x40:
                f['pred'].add('40TY hidden')
                continue
            f['jump'].add('cond')
            f['pred'].add(f'cc {(w >> 9) & 31}')
            for c in (0, 1):
                u = L.skipped_unit(w, c)
                if not u:
                    continue
                pos = L.units[u - 1]
                if L.t[pos] == 'T':
                    op, bank, mode = tr[tpos.index(pos)]
                    f['pred'].add(f'skips {op} {BANK.get(bank, "?")}' + ('' if mode == 0xFE else f' mode {mode:02x}'))
                else:
                    f['pred'].add('skips op')
    f['ntr'].add(str(sum(acc.values())))
    f['sig'].add(f'{cls:#05x} ' + ' '.join(sorted(ops)) + ' ; ' +
                 ' '.join(f'{b} {op} x{n}' for (b, op), n in sorted(acc.items())))
    return f


def program_features(words):
    """[(instruction index, words, features)]"""
    return [(k, ins, features(ins)) for k, (_, ins) in enumerate(dspdis.instructions(list(words)))]


# ---------------------------------------------------------------- corpora
def unpack(dat):
    d = tempfile.mkdtemp()
    subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'plugout_build.py'), 'unpack', dat, d], check=True,
                   stdout=subprocess.DEVNULL)
    return d


def plugout_programs(d):
    """[(name, words)] of every program in the sets of an unpacked PLUG-OUT (region 0 and 1)"""
    out = []
    for reg, base, chip in S.REGIONS:
        path = os.path.join(d, reg + '.bin')
        if not os.path.exists(path):
            continue
        img = open(path, 'rb').read()
        for k, st in enumerate(S.sets(img, base)):
            for a, n, p in st:
                if a in S.CODE:
                    out.append((f'{chip} set {k} @{a:#06x}', S.words(img, base, p, n)))
    return out


def os_programs(path):
    """the program sets of a SYSTEM-1 OS image (base 0x60000000; voice_sets.py's entries, every code slot)"""
    img = open(path, 'rb').read()
    out = []
    for k, st in enumerate(S.sets(img, 0x60000000)):
        for a, n, p in st:
            if a in S.CODE:
                out.append((f'set {k} @{a:#06x}', S.words(img, 0x60000000, p, n)))
    return out


def s1_corpus():
    """{source: [(name, words)]}; distinct programs only"""
    src = {}
    for f in OS_IMAGES:
        path = os.path.join(ROOT, 'work', 'sys1_v130', f)
        if os.path.exists(path):
            src['OS 1.30 ' + f] = os_programs(path)
    for name in S1_PLUGOUTS:
        d = os.path.join(ROOT, 'work', 'unpack', name + '_S1_Code1')
        if not os.path.exists(os.path.join(d, 'region1.bin')):
            dat = os.path.join(ROOT, 'plugins', name + '_S1_Code1', 'Script', 'Code1.Dat')
            if not os.path.exists(dat):
                continue
            d = unpack(dat)
        src[name] = plugout_programs(d)
    seen = set()
    for k in src:
        uniq = []
        for n, w in src[k]:
            if tuple(w) not in seen and len(w) > 2:
                seen.add(tuple(w))
                uniq.append((n, w))
        src[k] = uniq
    return src


def s8_corpus():
    """the JX-3P's own SYSTEM-8 programs (run on the S8): a second reference"""
    import s1jx as J
    return {'JX-3P (SYSTEM-8)': sorted(J.roland_programs().items())}


def device_good():
    """the programs that RAN on the SYSTEM-1 without hanging: the last installed version of every diagnostic build
    that test_s1jx.DEVICE marks as not hung (plugins/<NAME>, local), only the locations it ran (mains, master B, the
    slave sections it switched on)"""
    import test_s1jx as T
    out = {}
    for name, (hung, on) in T.DEVICE.items():
        dat = os.path.join(ROOT, 'plugins', name, 'Script', 'Code1.Dat')
        if hung is not False or not os.path.exists(dat):
            continue
        d = unpack(dat)
        progs = []
        for reg, base, chip in S.REGIONS:
            img = open(os.path.join(d, reg + '.bin'), 'rb').read()
            for k, st in enumerate(S.sets(img, base)):
                for a, n, p in st:
                    runs = a in (0x100, 0x1000) or (reg == 'region1' and a == 0x1900 and k in on)
                    if a in S.CODE and runs:
                        progs.append((f'{chip} set {k} @{a:#06x}', S.words(img, base, p, n)))
        out[name] = progs
    return out


def reference(corpus):
    """{kind: {feature: count}} over all instructions of the corpus"""
    ref = {k: collections.Counter() for k in KINDS}
    for progs in corpus.values():
        for _, w in progs:
            for _, _, f in program_features(w):
                for k in KINDS:
                    ref[k].update(f[k])
    return ref


# ---------------------------------------------------------------- report
def diff(name, words, ref1, ref8, refd, out):
    """the features of `words` that ref1 (SYSTEM-1) never has; returns {kind: {feature: [instruction indexes]}}.
    Columns: S8 = in the JX-3P's SYSTEM-8 programs; dev = in a program that ran on the SYSTEM-1 without hanging
    (device_good). A feature with dev `no` is a suspect"""
    miss = {k: collections.defaultdict(list) for k in KINDS}
    for k, ins, f in program_features(words):
        for kind in KINDS:
            for x in f[kind]:
                if x not in ref1[kind]:
                    miss[kind][x].append(k)
    n = sum(len(v) for kk, v in miss.items() if kk != 'sig')
    sus = sum(1 for kk, v in miss.items() if kk != 'sig' for x in v if x not in refd[kk])
    sig_sus = sum(1 for x in miss['sig'] if x not in refd['sig'])
    out.append(f'== {name}: {len(list(dspdis.instructions(list(words))))} instructions; not on the SYSTEM-1: '
               f'{n} features ({sus} not in a good device build either), {len(miss["sig"])} signatures ({sig_sus})')
    for kind in KINDS:
        if kind == 'sig' and '--sig' not in sys.argv:
            continue
        for x, ks in sorted(miss[kind].items(), key=lambda t: (x_in(t[0], refd[kind]), -len(t[1]), t[0])):
            s8 = 'S8 yes' if x in ref8[kind] else 'S8 NO '
            dev = 'dev ok' if x in refd[kind] else 'dev NO'
            if '--suspects' in sys.argv and x in refd[kind]:
                continue
            out.append(f'   {kind:4} {x:26} {s8} {dev} {len(ks):4} instr: {fmt(ks)}')
    return miss


def x_in(x, ref):
    return x in ref


def fmt(ks, n=12):
    return ' '.join(map(str, ks[:n])) + (' …' if len(ks) > n else '')


def final_programs():
    import s1jx as J
    pl = J.Plan(J.roland_programs())
    return [(f'{J.S1_NAMES[s1]} @{a:#06x} ({"+".join(u)})', w) for s1, a, u, w in pl.programs()]


def unit_programs():
    import s1jx as J
    pl = J.Plan(J.roland_programs())
    return list(pl.words.items())


def main(argv):
    corpus = s1_corpus()
    if not corpus:
        raise SystemExit('no SYSTEM-1 reference: run system-1/extract_fw.py and/or unpack the S1 PLUG-OUTs')
    ref1 = reference(corpus)
    ref8 = reference(s8_corpus())
    good = device_good()
    refd = reference(good)
    out = ['reference: ' + ', '.join(f'{k} {len(v)} programs' for k, v in corpus.items()),
           'ran on the device without hanging: ' + ', '.join(f'{k} ({len(v)})' for k, v in good.items())]
    if '--corpus' in argv:
        for kind in KINDS:
            if kind != 'sig':
                out.append(f'{kind}: ' + ', '.join(f'{x} ({n})' for x, n in sorted(ref1[kind].items())))
        print('\n'.join(out))
        return 0
    if '--dat' in argv:
        progs = plugout_programs(unpack(argv[argv.index('--dat') + 1]))
    elif '--units' in argv:
        progs = unit_programs()
    else:
        progs = final_programs()
    for name, w in progs:
        diff(name, w, ref1, ref8, refd, out)
    print('\n'.join(out))
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
