"""Validate the C reference (emu/dspref.py, dspdec.h) against the archived Python model (archive/dspisa.py and its
decoder front end archive/dspcomp_py.py) on the local corpus: layout, decoded units and one-instruction execution
from random states. Run once at the port (2026-10-29): 9791 distinct instructions identical, 50 differ only in the
MAC coefficient (the rule measured that day, docs/DSP_ISA_DISCREPANCIES.md D8).

    python3 archive/validate_port.py
"""
import os, sys, glob, random, collections
ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, ROOT + '/archive'); import dspisa as OLD; sys.modules['dspisa'] = OLD
import dspcomp_py as OC
sys.path.insert(0, ROOT + '/examples/from_scratch/emu'); sys.path.insert(0, ROOT + '/examples/from_scratch/re')
import dspref, dspcomp, dspdis
files = sorted(glob.glob(ROOT + '/work/corpus_all/*.txt'))
seen = set(); cnt = collections.Counter(); ex = {}
rnd = random.Random(7)
M = dspref.Machine()
def note(k, s):
    cnt[k] += 1; ex.setdefault(k, s)
for f in files:
    w = [int(x, 16) for x in open(f).read().split()]
    for _, ins in dspdis.instructions(w):
        key = tuple(ins)
        if key in seen: continue
        seen.add(key)
        cnt['insns'] += 1
        s = ' '.join('%04x' % x for x in ins)
        try: Lo = OLD.layout(list(ins))
        except Exception as e: Lo = None
        try: Ln = dspref.layout(list(ins))
        except ValueError: Ln = None
        if (Lo is None) != (Ln is None): note('layout existence', s); continue
        if Lo is None: continue
        for a in ('t', 'slots', 'units', 'used', 'n_list', 'imm_words', 'mac_off'):
            if getattr(Lo, a) != getattr(Ln, a): note('layout ' + a, s)
        try: do = OC.decode(list(ins))
        except Exception: do = None
        try: dn = dspcomp.decode(list(ins))
        except ValueError: dn = None
        if (do is None) != (dn is None): note('decode existence', s); continue
        if do is None: continue
        macdiff = False
        if len(do[0]) != len(dn[0]): note('decode nops', s); continue
        for a, b in zip(do[0], dn[0]):
            if a['kind'] == 3 and a['imm'][1] != b['imm'][1]:
                macdiff = True; b = dict(b, imm=[b['imm'][0], a['imm'][1], b['imm'][2]])
            if a != b: note('decode op', s + ' %r %r' % (a, b))
        if do[1] != dn[1]: note('decode conds', s)
        if do[2] != dn[2]: note('decode jumps', s)
        if macdiff: cnt['mac coefficient differs (new rule)'] += 1; continue
        # execution
        for trial in range(3):
            regs = [rnd.choice([rnd.uniform(-2, 2), rnd.uniform(-300, 300), 0.0, 1.0]) for _ in range(8)]
            regs = [OLD.quant(x) for x in regs]
            ctx = rnd.choice(list(OLD.CONTEXT_BITS)); OLD.CONTEXT = ctx; OLD.FLAGS = None; OLD.PENDING = {}
            OLD.HIDDEN.clear(); OLD.HIDDEN.update({4: 288.0, 5: 0x105 / 16384, 6: 0.0, 7: 0.0})
            OLD.JUMP = None
            M.mem.clear(); M.reset(context=ctx, regs=regs)
            try:
                ro, mo, _ = OLD.run(list(ins), list(regs), {})
                OLD.run([1], ro)                                   # flag latency
                fo = OLD.FLAGS; jo = OLD.JUMP
            except Exception as e:
                ro = ('exc', type(e).__name__)
            try:
                M.run(list(ins)); jn = M.jump; rn = M.regs; M.run([1]); fn = M.flags
            except Exception as e:
                rn = ('exc', type(e).__name__)
            if isinstance(ro, tuple) or isinstance(rn, tuple):
                if ro != rn: note('exec exception', s + f' {ro} {rn}')
                break
            if [repr(x) for x in ro] != [repr(x) for x in rn]: note('exec regs', s + f' old {ro} new {rn} in {regs}'); break
            if fo != fn: note('exec flags', s + f' {fo} {fn}'); break
            mn = {k: v for k, v in M.mem.items()}
            mo2 = {(0xA000 | (k & 0x3FF) if 0xA000 <= k < 0xC000 else k): v for k, v in mo.items() if v}
            if mo2 != mn: note('exec mem', s + f' {mo2} {mn}'); break
            if jo != jn: note('exec jump', s); break
for k, v in sorted(cnt.items()): print(f'{v:7d}  {k}')
for k, s in ex.items(): print(k, '::', s[:300])
