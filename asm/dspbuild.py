"""General build pipeline for our DSP microcode, SYSTEM-1 and SYSTEM-8 (docs/MICROCODE_OPTIMIZATION.md §6).

A generator only writes the plain program: one op or one transfer per line, load delays as "mov rN, rN" fillers,
';' comments. Everything else is here, the same for every plug-out:

    import dspbuild
    b = dspbuild.build(lines, dspbuild.S1)        # framing S1 (SYSTEM-1 section) or S8 (SYSTEM-8 slot / main)
    b.words, b.text, b.plain_words                # optimized words with the framing, its text, the plain words
    dspbuild.write('s1m0', out_dir, b)            # s1m0.s / s1m0.hex + check_optimized warnings

build() runs, in order (each can be turned off for one call, or for every build with DSP_OPT=0):
  1. phwrap: the manual 11-op phase wrap of our early programs (addxy t, x, #-1 / max t, t, r5 / (mul t, t, #256 /
     min t, t, r6) x4 / mul t, t, #2 / subxy x, t, x) becomes ph_wrap x, r3, x (Roland's form), when the temporary
     t is dead afterwards. It gives the same result except for a phase of exactly 1.0 (manual: 1.0, ph_wrap: -1.0).
  2. schedule: asm/dspsched.py (dependencies, packing, op fusion, reordering).
  3. assemble, with the framing words around the code.
  4. verify (DSP_VERIFY=0 to skip): the scheduled program against the plain one (after phwrap) in the emulator's C
     core, on 4 random states (registers, banks 0 / 1 / 5, bank 6, IOMD: values on the quant grid, as a program
     or the ARM writes them) for 6 samples each; every register, flag and memory word must be equal. A difference
     raises. Programs already verified are remembered by their words (asm/build/verified.txt).

    python3 asm/dspbuild.py FILE.s [--s1|--s8] [-o DIR]    a hand-written plain .s -> optimized .s / .hex
"""
import hashlib
import os
import random
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..'))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import asm  # noqa: E402
import dspsched  # noqa: E402

# framing: (words before the code, words after it)
S1 = ([0x0A02, 0xFF71], [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001])     # SYSTEM-1 section / main (make_s1.py)
S8 = ([0x0A02, 0xFF70], [0x0B02, 0x00F8, 0x0001])                     # SYSTEM-8 slot / main (make_v3.py)
VERIFIED = os.path.join(HERE, 'build', 'verified.txt')


def enabled(name, default=True):
    v = os.environ.get(name)
    return default if v is None else v == '1'


# ---------------------------------------------------------------- 1. phwrap
_WRAP = [r'addxy r(?P<t>\d), r(?P<x>\d), #-1(\.0)?(:32)?', r'max r(?P=t), r(?P=t), r5']
_WRAP += [r'mul r(?P=t), r(?P=t), #256(\.0)?(:32)?', r'min r(?P=t), r(?P=t), r6'] * 4
_WRAP += [r'mul r(?P=t), r(?P=t), #2(\.0)?(:32)?', r'subxy r(?P=x), r(?P=t), r(?P=x)']
_WRAP_RE = re.compile('^' + r'\n'.join(_WRAP) + '$')


def _code(lines):
    """indices of the lines that are code (not comments, not empty), with their text"""
    return [(i, x.split(';')[0].strip()) for i, x in enumerate(lines) if x.split(';')[0].strip()]


def _dead_after(code, k, reg):
    """is register `reg` written before it is read in code[k:] (or never read again)?"""
    for _, x in code[k:]:
        m = dspsched.XFER.match(x)
        if m:
            if m.group(1) == 'st' and int(m.group(2)) == reg:
                return False
            if m.group(1) == 'ld' and int(m.group(2)) == reg:
                return True
            continue
        regs = [int(r) for r in dspsched.REG.findall(x)]
        if not regs:
            continue
        reads = set(regs[1:]) | ({regs[0]} if x.split()[0] in dspsched.READS_DEST else set())
        if reg in reads or x.split()[0] not in dspsched.OPS:
            return False
        if regs[0] == reg:
            return True
    return True


def phwrap(lines):
    """lines with every dead-temporary manual wrap replaced by ph_wrap (see the module docstring)"""
    code = _code(lines)
    n = len(_WRAP)
    out_idx, k, drop, put = [], 0, set(), {}
    while k + n <= len(code):
        block = '\n'.join(x for _, x in code[k:k + n])
        m = _WRAP_RE.match(block)
        if m and _dead_after(code, k + n, int(m.group('t'))):
            x = m.group('x')
            put[code[k][0]] = f'ph_wrap r{x}, r3, r{x}'
            drop.update(i for i, _ in code[k + 1:k + n])
            k += n
            continue
        k += 1
    del out_idx
    return [put.get(i, x) for i, x in enumerate(lines) if i not in drop]


# ---------------------------------------------------------------- 3. assemble
_TABLE = []


def assemble(lines, framing=S1):
    if not _TABLE:
        _TABLE.append(asm.Assembler().table)
    r = asm.Assembler(_TABLE[0]).assemble('\n'.join(lines) + '\n')
    if not r.ok:
        raise ValueError(r.errors)
    return list(framing[0]) + r.words + list(framing[1]), r


# ---------------------------------------------------------------- 4. verify
def _emu():
    p = os.path.join(ROOT, 'examples', 'from_scratch', 'emu')
    if p not in sys.path:
        sys.path.insert(0, p)
    import chain
    import cchain
    import dspisa
    return chain, cchain, dspisa


def _key(a, b):
    return hashlib.sha1((' '.join('%04x' % w for w in a) + '|' + ' '.join('%04x' % w for w in b)).encode()).hexdigest()


def verify(plain_words, opt_words, states=4, samples=6, seed=1):
    """the two programs give the same registers, flags and memory from random on-grid states (raises if not)"""
    key = _key(plain_words, opt_words)
    done = set(open(VERIFIED).read().split()) if os.path.exists(VERIFIED) else set()
    if key in done:
        return True
    chain, cchain, dspisa = _emu()
    r = random.Random(seed)

    def val():
        return dspisa.quant(r.choice([r.uniform(-1, 1), r.uniform(-4, 4), r.uniform(-0.01, 0.01), 0.0, 1.0]))
    P = [chain.Program(plain_words, 'plain', strict=False), chain.Program(opt_words, 'opt', strict=False)]
    for st in range(states):
        init = {'regs': [val() for _ in range(8)], 'b0': [val() for _ in range(chain.RING)],
                'b1': [val() for _ in range(chain.RING)], 'b5': [val() for _ in range(0x400)],
                'p': [val() for _ in range(0x200)], 'io': [val() for _ in range(0x100)]}
        sl = []
        for p in P:
            s = cchain.Slave({0: (p, [])}, threads=False)
            d = s.d[0]
            d.set_state(regs=init['regs'])
            d.bank0[:] = init['b0']
            d.bank1[:] = init['b1']
            d.bank5[:] = init['b5']
            for i, v in enumerate(init['p']):
                s.params[0xC000 + i] = v
            for i, v in enumerate(init['io']):
                s.iomd[0x6000 + i] = v
            sl.append(s)
        for t in range(samples):
            snaps = []
            for s in sl:
                s.step()
                d = s.d[0]
                snaps.append((d.state(), list(d.bank0), list(d.bank1), list(d.bank5),
                              [s.iomd.get(0x6000 + i) for i in range(0x2000)]))
            if repr(snaps[0]) != repr(snaps[1]):
                raise AssertionError(f'dspbuild.verify: the optimized program differs from the plain one '
                                     f'(state {st}, sample {t})')
    os.makedirs(os.path.dirname(VERIFIED), exist_ok=True)
    with open(VERIFIED, 'a') as f:
        f.write(key + '\n')
    return True


# ---------------------------------------------------------------- build / write
class Built:
    def __init__(self, words, text, plain_words, plain_lines, report):
        self.words, self.text, self.plain_words, self.plain_lines, self.report = \
            words, text, plain_words, plain_lines, report

    def count(self):
        return len(self.text)


def build(lines, framing=S1, phwrap_=None, schedule=None, verify_=None):
    """plain lines -> Built (see the module docstring); None = the DSP_OPT / DSP_VERIFY default"""
    opt = enabled('DSP_OPT')
    lines = [x for x in lines]
    plain = phwrap(lines) if (opt if phwrap_ is None else phwrap_) else lines
    sched = opt if schedule is None else schedule
    text = dspsched.schedule(plain) if sched else [x for x in plain if x.split(';')[0].strip()]
    words, rep = assemble(text, framing)
    plain_words, _ = assemble(plain, framing)
    if sched and (enabled('DSP_VERIFY') if verify_ is None else verify_):
        verify(plain_words, words)
    return Built(words, text, plain_words, plain, rep)


def write(name, out_dir, b, comment='', check=True):
    """<name>.s (the optimized text) and <name>.hex (words with the framing); check_optimized warnings"""
    s_path = os.path.join(out_dir, name + '.s')
    open(s_path, 'w').write((f'; {comment}\n' if comment else '') + '\n'.join(b.text) + '\n')
    open(os.path.join(out_dir, name + '.hex'), 'w').write(
        f'# {name}{", " + comment if comment else ""}\n' + '\n'.join('%04x' % x for x in b.words) + '\n')
    if check:
        sys.path.insert(0, os.path.join(ROOT, 'tools'))
        import check_optimized
        n, w = check_optimized.check(s_path)
        for kind, at in w.items():
            if at:
                print(f'check_optimized: warning: {name}.s: {len(at)} x {kind} ({check_optimized.HINT[kind]})')


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('-')]
    if not args:
        print(__doc__)
        return 2
    framing = S8 if '--s8' in sys.argv else S1
    out = sys.argv[sys.argv.index('-o') + 1] if '-o' in sys.argv else None
    for path in args:
        lines = open(path).read().splitlines()
        b = build(lines, framing)
        name = os.path.splitext(os.path.basename(path))[0] + '_opt'
        write(name, out or os.path.dirname(os.path.abspath(path)), b, f'optimized from {os.path.basename(path)}')
        print(f'{path}: {dspsched.count(lines)} lines -> {b.count()} instructions, {len(b.words)} words')
    return 0


if __name__ == '__main__':
    sys.exit(main())
