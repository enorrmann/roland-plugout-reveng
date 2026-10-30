"""List scheduler for our DSP microcode (SYSTEM-1 and SYSTEM-8: same ISA). docs/MICROCODE_OPTIMIZATION.md §5.

    import dspsched
    packed = dspsched.schedule(lines)          # one-unit lines (with "mov rN, rN" fillers) -> instruction texts

The input is the plain program the generators write: one op or one transfer per line, a load delay written as a
"mov rN, rN" filler, comments with ';'. The output does the same thing in fewer instructions: independent ops share
an instruction ("op1 | op2 | op3"), loads and stores go into op instructions, the fillers are gone, and the units are
reordered as far as the dependencies allow (pack() of make_synth.py only merged a line into the instruction before
it, in program order).

Semantics the schedule keeps (the emulator's, measured on the hardware; DSP_HARDWARE.md, pack() comment):
  * every unit of an instruction reads before any unit writes;
  * a loaded register is usable from the next instruction on; a store reads the register as the instruction starts;
  * memory: a load after a store to the same address comes one instruction later; a store after a load may share
    the load's instruction; two stores to one address keep their order (one instruction apart).

Dependencies (i before j in program order -> minimum instruction distance):
  register read after write (op or load result) 1; write after read 0; write after write 1;
  same address (normalised: banks 0 / 1 / 5 repeat every 0x800, IOMD every 0x2000): store -> load 1, load -> store 0,
  store -> store 1. Anything the scheduler does not understand (an unknown op, a condition) is a barrier: everything
  before it comes first, it gets an instruction of its own, everything after it comes after.

Limits per instruction (the ones proven on the device with pack() / FUSE: C-S1SY8 .. C-S1SY11): at most 3 ops, all of
the one-destination kinds in OPS; at most 3 transfers, no address twice, bank-0 loads <= 2, bank-0 stores <= 2, any
other bank <= 1 load and <= 1 store; and the instruction must assemble (asm.Assembler).

Priority: the longest dependency path to the end of the program (critical path), then program order. Validation:
the scheduled program must give the same samples as the unscheduled one in the emulator (make_synth.py --pack-check,
test_sched.py).
"""
import heapq
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import asm  # noqa: E402

XFER = re.compile(r'^(ld|st) r(\d), \[([0-9a-f]{4})\]$')
IDX = re.compile(r'^ldidx r(\d), (\d+)$')            # bank 0 [h6 + Y] (X28Y): after a hset h6 with one instruction between
IDX5 = re.compile(r'^ldb5(?:h7)? r(\d), (\d+)$')    # bank 5 [h6 / h7 + Y] (XdaY / XdbY): the same, a fixed table
HSET = re.compile(r'^hset h(\d), r(\d)$')
FILLER = re.compile(r'^mov r(\d), r\1$')
REG = re.compile(r'\br(\d)\b')
OPS = {'mul', 'mad', 'addxy', 'subxy', 'add', 'min', 'max', 'mov', 'ph_wrap', 'ph_tri', 'ph_clamp', 'ymacd'}
READS_DEST = {'mad', 'ymacd', 'add'}


# MAC forms (measured exact 2026-10-28 / -29): register coefficient, and the immediate coefficient K (c1/c2/c3:
# 16/24/32 bits, read at byte C of the slot array) with and without W
MAC_FORMS = frozenset({'mac_p1w', 'mac_p1p2w', 'mac_p1p2', 'mac_p1p2wp2'} |
                      {f'{b}c{k}' for b in ('mac_p1w', 'mac_p1p2w', 'mac_p1p2', 'mac_p1p2wp2') for k in (1, 2, 3)})


def _native(mn, r):
    """the native ops (DSP_HARDWARE.md §5, measured exact on the SYSTEM-8 master with re/dspnative.py 2026-10-28):
    r = the operand registers in text order -> (registers written, registers read), or None for an unknown op.
    Several write more than their D: recipseed / atanhsplit r(D), r(D+1) and rY; newton r(D), r(D+1), rY"""
    d = r[0]
    if mn in ('shl', 'shr', 'shrn'):                     # shl rD, rX, rK: K is the 4-bit shift, not a register
        return {d}, {r[1]}
    if mn in ('floor', 'ceil', 'sign'):
        return {d}, {r[1]}
    if mn in ('expsplit', 'log2split'):                  # rD and rY
        return {d, r[1]}, {r[1]}
    if mn in ('recipseed', 'atanhsplit'):                # rD, r(D+1), rY
        return {d, (d + 1) & 7, r[1]}, {r[1]}
    if mn == 'frexp':                                    # frexp rD, rX, rY: rD <- m, rY <- e
        return {d, r[2]}, {r[1]}
    if mn == 'newton':                                   # rD <- rD r(D+1); r(D+1) <- 2 - r(D+1) rY; rY <- r(D+1) rY
        return {d, (d + 1) & 7, r[1]}, {d, (d + 1) & 7, r[1]}
    if mn == 'add3':                                     # rD <- rD + rX + Y
        return {d}, set(r)
    if mn == 'sum8':                                     # rD <- rX + rY + rR
        return {d}, set(r[1:])
    if mn == 'mac_p1':                                   # rD <- rX rY + rW (one product)
        return {d}, set(r[1:])
    if mn in MAC_FORMS:                                  # rD <- (+-) rX rY + c rZ (+ rW), c a register or a #K
        return {d}, set(r[1:])
    if mn == 'sqrtsplit':                                # rY <- m, r(D+1) <- seed, r(D+3) <- sign 2^-k (2026-10-29)
        return {(d + 1) & 7, (d + 3) & 7, r[1]}, {r[1]}
    return None


NATIVE = {'shl', 'shr', 'shrn', 'floor', 'ceil', 'sign', 'expsplit', 'log2split', 'recipseed', 'atanhsplit', 'frexp',
          'newton', 'add3', 'sum8', 'sqrtsplit', 'mac_p1'} | MAC_FORMS
MAX_OPS, MAX_XFER = 3, 3
# op families (DSP_HARDWARE.md §5.2-5.4), for the patterns of asm/bad_forms.txt (`family:minmax + family:minmax`)
B_FAMILY = {**{m: 'macp2' for m in MAC_FORMS},          # the MAC ops with a P2 term
            **{m: 'minmax' for m in ('min', 'max', 'minn', 'maxn', 'absx', 'absy')},
            **{m: 'e' for m in ('cmp', 'bic', 'divseed', 'ph_add', 'ph_sub', 'ph_abs', 'ph_clamp', 'ph_wrap', 'ph_tri',
                                'ph_mod2', 'ph_neg')},
            **{m: 'shift' for m in ('shl', 'shr', 'shrn', 'frexp')},
            **{m: 'sfu' for m in ('floor', 'ceil', 'sign', 'recipseed', 'expsplit', 'log2split', 'sqrtsplit',
                                  'atanhsplit')},
            **{m: 'abs' for m in ('absadd', 'subabs')}}

_TABLE = []
_ASSEMBLES = {}


def assembles(text):
    if text not in _ASSEMBLES:
        if not _TABLE:
            _TABLE.append(asm.Assembler().table)
        _ASSEMBLES[text] = asm.Assembler(_TABLE[0]).assemble(text + '\n').ok
    return _ASSEMBLES[text]


def mem_key(addr):
    """the memory word an address reaches (dspcore.h rd / wr: banks 0, 1, 5 take 11 bits, IOMD 13)"""
    bank, off = addr >> 13, addr & 0x1FFF
    return bank, off & 0x7FF if bank in (0, 1, 5) else off


class Unit:
    __slots__ = ('i', 'text', 'kind', 'reg', 'addr', 'dest', 'dests', 'reads', 'barrier', 'idx', 'hset', 'b5', 'h')

    def __init__(self, i, text):
        self.i, self.text = i, text
        self.barrier = False
        self.addr = None
        self.idx = self.hset = self.b5 = False
        self.h = None                          # the hidden register an indexed load reads / a hset writes
        m = IDX.match(text) or IDX5.match(text)
        if m:                                  # an indexed load (bank 0, or bank 5: b5): no static address
            self.kind, self.reg, self.idx, self.b5 = 'ld', int(m.group(1)), True, text.startswith('ldb5')
            self.h = 7 if text.startswith('ldb5h7') else 6
            self.dest, self.dests, self.reads = self.reg, {self.reg}, set()
            return
        m = HSET.match(text)
        if m:                                  # a hidden-register load: alone in its instruction, ordered only with
            self.hset, self.h = True, int(m.group(1))   # its register and the indexed loads, not a barrier
            self.kind, self.reg, self.dest, self.dests, self.reads = 'op', None, None, set(), {int(m.group(2))}
            return
        m = XFER.match(text)
        if m:
            self.kind, self.reg, self.addr = m.group(1), int(m.group(2)), int(m.group(3), 16)
            self.dest = self.reg if self.kind == 'ld' else None
            self.dests = {self.dest} if self.kind == 'ld' else set()
            self.reads = set() if self.kind == 'ld' else {self.reg}
            return
        self.kind = 'op'
        mn = text.split()[0]
        regs = [int(r) for r in REG.findall(text)]
        if mn in NATIVE and regs and '|' not in text:
            self.dests, self.reads = _native(mn, regs)
            self.dest = regs[0]
            return
        if mn not in OPS or not regs or '|' in text:
            self.barrier = True
            self.dest, self.dests, self.reads = None, set(), set(range(8))
            return
        self.dest = regs[0]
        self.dests = {self.dest}
        if mn in ('ph_wrap', 'ph_tri', 'ph_clamp'):      # `ph_wrap rD, r3, rY`: that r3 is part of the opcode
            regs = regs[:1] + regs[2:]
        self.reads = set(regs[1:]) | ({regs[0]} if mn in READS_DEST else set())


def units_of(lines):
    out = []
    for x in lines:
        x = x.split(';')[0].strip()
        if not x or FILLER.match(x):
            continue
        out.append(Unit(len(out), x))
    return out


def dependencies(units):
    """preds[j] = [(i, minimum distance)]"""
    n = len(units)
    preds = [[] for _ in range(n)]
    last_write = {}            # reg -> unit
    reads_since = {}           # reg -> [units] reading it since its last write
    last_store, loads_since = {}, {}
    barrier = None
    last_hset, any_store, idx_since = {}, None, {}      # per hidden register h6 / h7
    for j, u in enumerate(units):
        dep = {}

        def need(i, d):
            if i is not None and dep.get(i, -1) < d:
                dep[i] = d
        if u.barrier:
            for i in range(barrier + 1 if barrier is not None else 0, j):
                need(i, 1)
            if barrier is not None:
                need(barrier, 1)
        elif barrier is not None:
            need(barrier, 1)
        for r in u.reads:
            need(last_write.get(r), 1)
        for w in u.dests:
            need(last_write.get(w), 1)
            for i in reads_since.get(w, ()):
                need(i, 0)
        if u.addr is not None:
            k = mem_key(u.addr)
            if u.kind == 'ld':
                need(last_store.get(k), 1)
            else:
                need(last_store.get(k), 1)
                for i in loads_since.get(k, ()):
                    need(i, 0)
        if u.idx:                              # any bank-0 word: after every bank-0 store, two after the hset
            if not u.b5:                       # (bank 5: a table nobody stores into)
                need(any_store, 1)
            need(last_hset.get(u.h), 2)
        if u.kind == 'st' and u.addr >> 13 == 0:   # a bank-0 store after an indexed load of bank 0
            for i in idx_since.get(6, []):
                if not units[i].b5:
                    need(i, 0)
        if u.hset:                             # a new h: after the indexed loads that used the old one
            for i in idx_since.get(u.h, []):
                need(i, 1)
        dep.pop(j, None)
        preds[j] = list(dep.items())
        for r in u.reads:
            reads_since.setdefault(r, []).append(j)
        for w in u.dests:
            last_write[w] = j
            reads_since[w] = []
        if u.idx:
            idx_since.setdefault(u.h, []).append(j)
        if u.kind == 'st' and u.addr >> 13 == 0:   # the indexed loads read bank 0 only
            any_store = j
        if u.hset:
            last_hset[u.h], idx_since[u.h] = j, []
        if u.barrier:
            barrier = j
            for r in range(8):
                last_write[r] = j
                reads_since[r] = []
        if u.addr is not None:
            k = mem_key(u.addr)
            if u.kind == 'ld':
                loads_since.setdefault(k, []).append(j)
            else:
                last_store[k] = j
                loads_since[k] = []
    return preds


def op_signature(op_texts):
    """the form of a group of ops in one instruction: sorted (mnemonic, operand kind) with kind reg / imm / i32"""
    out = []
    for t in op_texts:
        mn = t.split()[0]
        out.append(f'{mn}:{"i32" if ":32" in t else "imm" if "#" in t else "reg"}')
    return tuple(sorted(out))


def signatures(scheduled):
    """Counter of the op groups (op_signature) that share an instruction in scheduled lines (texts joined by ' | ')"""
    import collections
    out = collections.Counter()
    for line in scheduled:
        if not line.strip() or line.startswith(';'):
            continue
        ops = [u.strip() for u in line.split('|') if u.strip().split()[0] not in ('ld', 'st', 'ldidx', 'ldb5', 'ldb5h7')]
        if len(ops) > 1:
            out[op_signature(ops)] += 1
    return out


def form_text(sig):
    """an op signature as asm/proven_forms.txt writes it: 'mad:reg + mul:imm'"""
    return ' + '.join(sig)


def parse_forms(text):
    """'mad:reg + mul:imm; add:imm + mov:i32' -> set of signatures"""
    return {tuple(sorted(x.strip() for x in f.split('+'))) for f in text.split(';') if f.strip()}


def candidate_forms(lines, proven):
    """op groups the device has not run that would shorten this program: [(signature, instructions saved alone,
    times used when every encodable group is allowed)], the proven-only length and the every-group length"""
    base = schedule(lines, proven)
    free = schedule(lines, None)
    used = {s: c for s, c in signatures(free).items() if s not in proven}
    out = []
    for sig, c in used.items():
        out.append((sig, len(base) - len(schedule(lines, proven | {sig})), c))
    Ins.allowed = proven
    return sorted(out, key=lambda x: (-x[1], -x[2])), len(base), len(free)


def load_bad(path=None):
    """op groups that hung the device (asm/bad_forms.txt): never fused, not even when every group is allowed"""
    return load_proven(path or os.path.join(HERE, 'bad_forms.txt'))


def load_bad_patterns(path=None):
    """asm/bad_forms.txt: one group per line, terms joined by ' + '; a term is `mnemonic:kind` (fnmatch wildcards:
    `mac_*:*`) or `family:NAME` (B_FAMILY). A group of ops is bad when some of its ops match every term"""
    path = path or os.path.join(HERE, 'bad_forms.txt')
    out = []
    if os.path.exists(path):
        for line in open(path):
            line = line.split('#')[0].strip()
            if line:
                out.append(tuple(t.strip() for t in line.split(' + ')))
    return out


def _term_matches(term, op):
    import fnmatch
    mn = op.split(':')[0]
    if term.startswith('family:'):
        return B_FAMILY.get(mn) == term[7:]
    return fnmatch.fnmatchcase(op, term)


def _pattern_matches(pattern, sig):
    """some distinct ops of sig match the pattern's terms, one each"""
    import itertools
    if len(pattern) > len(sig):
        return False
    return any(all(_term_matches(t, op) for t, op in zip(pattern, ops)) for ops in itertools.permutations(sig, len(pattern)))


_BADP = []


def is_bad(sig):
    """a group (op_signature) that asm/bad_forms.txt forbids"""
    if not _BADP:
        _BADP.append(load_bad_patterns())
    return any(_pattern_matches(p, sig) for p in _BADP[0])


def evidence_text(sig):
    """Roland's use of a group (asm/roland_forms.py), or '' without the local corpora"""
    try:
        import roland_forms
        return roland_forms.text(sig)
    except Exception:                                                   # noqa: BLE001
        return ''


def load_proven(path=None):
    """op groups proven on the device (asm/proven_forms.txt: one signature per line, ops joined by ' + ')"""
    path = path or os.path.join(HERE, 'proven_forms.txt')
    out = set()
    for line in open(path):
        line = line.split('#')[0].strip()
        if line:
            out.add(tuple(sorted(line.split(' + '))))
    return out


def _bank_name(addr):
    """as asm/s1rules.py bank()"""
    return {0x0: '0', 0x1: '1', 0x3: 'io', 0x5: '5', 0x6: '6', 0x7: 'e'}.get(addr >> 13, f'{addr >> 13:x}')


_BAD = []


def _bad():
    if not _BAD:
        _BAD.append(load_bad() if os.path.exists(os.path.join(HERE, 'bad_forms.txt')) else set())
    return _BAD[0]


class Ins:
    allowed = None                     # set of op signatures that may share an instruction (None: any)
    one_bank5 = False                  # True: one bank-5 access per instruction, load or store (the SYSTEM-1 rule 5,
                                       # docs/examples_system_1_jx_README.md §5; a load with a store of bank 5 was in
                                       # the half of C-S1JX's MAIN3 that hung the device)
    shapes = None                      # [(Counter of 'ld5' / 'st0' ..., number of ops)]: an instruction may only be
                                       # part of one of these (asm/s1rules.py proven_shapes); None: any

    def __init__(self):
        self.ops, self.xf = [], []

    def text(self):
        return ' | '.join([u.text for u in self.ops] + [u.text for u in self.xf])

    def fits(self, u):
        if not self._fits(u):
            return False
        if self.shapes is None or u.barrier or u.hset:
            return True
        import collections
        acc = collections.Counter(f'{x.kind}{("5" if x.b5 else "0") if x.idx else _bank_name(x.addr)}'
                                  for x in self.xf + ([u] if u.kind != 'op' else []))
        ops = len(self.ops) + (u.kind == 'op')
        return any(ops <= p_ops and not (acc - p_acc) for p_acc, p_ops in self.shapes)

    def _fits(self, u):
        if u.barrier or u.hset:
            return not self.ops and not self.xf
        if any(x.hset for x in self.ops):
            return False
        if any(x.barrier for x in self.ops):
            return False
        if u.kind == 'op':
            if len(self.ops) >= MAX_OPS:
                return False
            # asm/bad_forms.txt (signatures and patterns: families, wildcards): never, whatever is allowed
            if self.ops and is_bad(op_signature([x.text for x in self.ops] + [u.text])):
                return False
            if self.ops and self.allowed is not None:
                return op_signature([x.text for x in self.ops] + [u.text]) in self.allowed
            return True
        if len(self.xf) >= MAX_XFER:
            return False
        if u.b5:                               # an indexed bank-5 load: no other bank-5 access in the instruction
            return not any(x.b5 or (not x.idx and x.addr >> 13 == 5) for x in self.xf)
        if u.idx:                              # a bank-0 load: at most 2 per instruction with the literal ones
            return sum(1 for x in self.xf if x.kind == 'ld' and ((x.idx and not x.b5) or (not x.idx and x.addr >> 13 == 0))) < 2
        if any(not x.idx and mem_key(x.addr) == mem_key(u.addr) for x in self.xf):
            return False
        bank = u.addr >> 13
        if bank == 5 and any(x.b5 for x in self.xf):
            return False
        if self.one_bank5 and bank == 5 and any(not x.idx and x.addr >> 13 == 5 for x in self.xf):
            return False
        same = sum(1 for x in self.xf if x.kind == u.kind and ((5 if x.b5 else 0) if x.idx else x.addr >> 13) == bank)
        return same < (2 if bank == 0 else 1)

    def add(self, u):
        (self.ops if u.kind == 'op' else self.xf).append(u)

    def remove(self, u):
        (self.ops if u.kind == 'op' else self.xf).remove(u)


MAC_ALT = True                                         # mad -> mac_p1 when that lets it share an instruction


def mac_alt(u):
    """`mad rD, rX, Y` as the MAC's `mac_p1 rD, rX, Y, rD` (rD <- rX Y + rD) and `mul rD, rX, Y` as
    `mac_p1 rD, rX, Y, #0` (the same values; DSP_HARDWARE.md §5.4): the MAC unit beside A / B ops, as Roland's
    SYSTEM-1 programs do (class MA, a one-product MAC). None for any other unit"""
    if not MAC_ALT or u.kind != 'op':
        return None
    m = re.match(r'^mad r(\d), r(\d), (r\d|#\S+)$', u.text)     # Y a register or an immediate (Roland's S1:
    if m:                                                         # `mac_p1:imm + mul:reg` x80, asm/roland_forms.py)
        d, x, y = m.groups()
        return Unit(u.i, f'mac_p1 r{d}, r{x}, {y}, r{d}')
    m = re.match(r'^mul r(\d), r(\d), (r\d|#\S+)$', u.text)     # a product: rX Y + #0 (Roland's class MA:
    if m:                                                         # Y and W immediates, x67)
        d, x, y = m.groups()
        return Unit(u.i, f'mac_p1 r{d}, r{x}, {y}, #0.0')
    return None


def schedule(lines, allowed=None):
    """one-unit lines -> scheduled instruction texts (see the module docstring). allowed: the op groups that may
    share an instruction (load_proven(): the ones proven on the device); None = every encodable group"""
    Ins.allowed = allowed
    units = units_of(lines)
    n = len(units)
    if not n:
        return []
    preds = dependencies(units)
    succs = [[] for _ in range(n)]
    for j, ps in enumerate(preds):
        for i, d in ps:
            succs[i].append((j, d))
    height = [0] * n                                   # critical path: longest distance to the end
    for i in range(n - 1, -1, -1):
        height[i] = max((d + height[j] for j, d in succs[i]), default=0) + 1
    npred = [len(p) for p in preds]
    earliest = [0] * n
    at = [None] * n
    ready = [j for j in range(n) if not npred[j]]      # all predecessors placed
    out, done, c = [], 0, 0
    while done < n:
        ins = Ins()
        heap = [(-height[j], j) for j in ready if earliest[j] <= c]
        rest = [j for j in ready if earliest[j] > c]
        heapq.heapify(heap)
        later = []
        while heap:
            _, j = heapq.heappop(heap)
            u = units[j]
            # a predecessor in this same instruction is only allowed at distance 0
            if any(d >= 1 and at[i] == c for i, d in preds[j]):
                later.append(j)
                continue
            placed = False
            for cand in (u, mac_alt(u)):              # a mad that does not fit as an A op: on the MAC (mac_p1)
                if cand is None or not ins.fits(cand):
                    continue
                ins.add(cand)
                if assembles(ins.text()):
                    units[j] = cand
                    placed = True
                    break
                ins.remove(cand)
            if not placed:
                later.append(j)
                continue
            at[j] = c
            done += 1
            for s_, d in succs[j]:
                npred[s_] -= 1
                earliest[s_] = max(earliest[s_], c + d)
                if not npred[s_]:
                    if earliest[s_] <= c:
                        heapq.heappush(heap, (-height[s_], s_))   # may still join this instruction
                    else:
                        rest.append(s_)
        if not ins.ops and not ins.xf and not later and rest:   # only waits (a hset's distance 2): one filler
            out.append('mov r0, r0')
            ready = rest
            c += 1
            continue
        if not ins.ops and not ins.xf:
            raise RuntimeError(f'scheduler stuck at instruction {c}: {[units[j].text for j in later[:4]]}')
        out.append(ins.text())
        ready = later + rest
        c += 1
    return out


def count(lines):
    return sum(1 for x in lines if x.split(';')[0].strip())
