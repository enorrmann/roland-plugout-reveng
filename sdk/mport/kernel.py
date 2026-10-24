"""Per-sample kernels written with values instead of registers -> our DSP microcode (docs/MUTABLE_SDK.md §3).

A ported component describes its sample loop as operations on values; the kernel picks the registers, writes one
unit per line (the form every generator of this repo writes: make_synth.py, make_poly.py) and puts the load-delay
fillers where the plain program needs them. asm/dspsched.py then packs and fuses it as for any other program.

    k = Kernel('swarm v0', regs=(0, 1, 2, 3, 4, 7))     # r5 / r6 belong to the host program (C-S1SY: 0 and 1)
    p = k.state(0x0500)                                  # ring word: load 0x0501 (last sample), store 0x0500
    inc = k.mul(k.load(0xA030), 0.0091)
    q = k.wrap(k.add(p.value, inc))
    p.write(q)
    k.store(q, 0xA151)
    lines = k.lines()                                    # 'ld r0, [0501]', 'mov r5, r5', 'mul r1, r0, #0.0091', ...

Operations (x: a value; y: a value, -value (negated register) or a number = immediate):

    mul(x, y) = x y            add(x, y) = x + y           sub(x, y) = x - y
    mad(acc, x, y) = acc + x y        ymac(acc, x, y) = y + x acc (Horner step, op 'ymacd')
    min(x, y), max(x, y)       mov(y)                     wrap(x): into [-1, 1)
    tri(x) = 1 - 2 |wrap(-x - 1.5)|   clamp(x) to [-1, 1]  (the phase ops of unit B, Roland's form X = r3)

Semantics kept from the hardware (DSP_ISA.md §9): all units of an instruction read before they write (a result may
take the register of an operand that dies there); a loaded register is usable one instruction later (filler in the
plain program, removed by the scheduler). Only op kinds already run on the SYSTEM-1 are emitted (the OPS set of
dspsched; asm/proven_forms.txt for the fused pairs).
"""
import re


class Value:
    """an SSA value: defined by one line, read by later lines; reg is set by the allocator"""
    __slots__ = ('k', 'n', 'name', 'reg')

    def __init__(self, k, n, name=''):
        self.k, self.n, self.name, self.reg = k, n, name, None

    def __neg__(self):
        return Neg(self)

    def __repr__(self):
        return f'%{self.n}' + (f'({self.name})' if self.name else '')


class Neg:
    """-x as a Y operand (bit 3 of the register field)"""
    __slots__ = ('v',)

    def __init__(self, v):
        self.v = v


class Ring:
    """one word of bank-0 ring state: the value of the last sample is at addr + 1, this sample's goes to addr"""

    def __init__(self, k, addr, name):
        self.k, self.addr, self.name = k, addr, name
        self._value = None

    @property
    def value(self):
        if self._value is None:
            self._value = self.k.load(self.addr + 1, self.name)
        return self._value

    def write(self, v):
        self.k.store(v, self.addr)


class KernelError(Exception):
    pass


class Kernel:
    def __init__(self, name='', regs=(0, 1, 2, 3, 4, 5, 6, 7)):
        self.name = name
        self.regs = tuple(regs)
        self.ops = []                          # (kind, dest Value or None, text template, operands)
        self._n = 0

    # ---- values ---------------------------------------------------------------------------------------------
    def _new(self, name=''):
        self._n += 1
        return Value(self, self._n, name)

    def _op(self, kind, fmt, operands, name='', acc=None):
        d = self._new(name)
        self.ops.append(('op', d, fmt, operands, acc))
        return d

    def comment(self, text):
        self.ops.append(('comment', None, '; ' + text, (), None))

    def load(self, addr, name=''):
        d = self._new(name or f'[{addr:04x}]')
        self.ops.append(('ld', d, f'ld {{d}}, [{addr:04x}]', (), None))
        return d

    def param(self, idx, name=''):
        """parameter C0idx of this DSP (FCEC(sel, idx, bits) from the ARM)"""
        return self.load(0xC000 + idx, name or f'c0{idx:02x}')

    def store(self, v, addr):
        self._check(v)
        self.ops.append(('st', None, f'st {{0}}, [{addr:04x}]', (v,), None))

    def state(self, addr, name=''):
        return Ring(self, addr, name or f'ring {addr:04x}')

    # ---- operations -----------------------------------------------------------------------------------------
    def mov(self, y, name=''):
        return self._op('mov', 'mov {d}, {0}', (y,), name)

    def mul(self, x, y, name=''):
        return self._op('mul', 'mul {d}, {0}, {1}', (self._x(x), y), name)

    def add(self, x, y, name=''):
        return self._op('addxy', 'addxy {d}, {0}, {1}', (self._x(x), y), name)

    def sub(self, x, y, name=''):
        """x - y: 'subxy d, X, Y' computes Y - X"""
        if isinstance(y, (int, float)):
            return self.add(x, -float(y), name)
        if isinstance(y, Neg):
            return self.add(x, y.v, name)
        return self._op('subxy', 'subxy {d}, {0}, {1}', (y, x), name)

    def mad(self, acc, x, y, name=''):
        """acc + x y (the result takes acc's register)"""
        return self._op('mad', 'mad {d}, {0}, {1}', (self._x(x), y), name, acc=self._x(acc))

    def ymac(self, acc, x, y, name=''):
        """y + x acc: one Horner step (c + x t) with t = acc"""
        return self._op('ymacd', 'ymacd {d}, {0}, {1}', (self._x(x), y), name, acc=self._x(acc))

    def min(self, x, y, name=''):
        return self._op('min', 'min {d}, {0}, {1}', (self._x(x), y), name)

    def max(self, x, y, name=''):
        return self._op('max', 'max {d}, {0}, {1}', (self._x(x), y), name)

    def wrap(self, x, name=''):
        return self._op('ph_wrap', 'ph_wrap {d}, r3, {0}', (self._x(x),), name)

    def tri(self, x, name=''):
        return self._op('ph_tri', 'ph_tri {d}, r3, -{0}', (self._x(x),), name)

    def clamp(self, x, name=''):
        return self._op('ph_clamp', 'ph_clamp {d}, r3, -{0}', (self._x(x),), name)

    # ---- composite helpers ----------------------------------------------------------------------------------
    def horner(self, x, coeffs):
        """c0 + x (c1 + x (c2 + ...)), coeffs from the constant term up (at least 2)"""
        c = [float(v) for v in coeffs]
        t = self.add(self.mul(x, c[-1]), c[-2])             # c[n-1] + x c[n]
        for ci in reversed(c[:-2]):
            t = self.ymac(t, x, ci)                         # ci + x t
        return t

    def poly_odd(self, x, coeffs, name=''):
        """x (a1 + a3 x^2 + a5 x^4 + ...): coeffs = (a1, a3, a5, ...)"""
        x2 = self.mul(x, x, 'x2')
        t = self.horner(x2, coeffs) if len(coeffs) > 1 else None
        return self.mul(t, x, name) if t is not None else self.mul(x, float(coeffs[0]), name)

    def exp2(self, x, name=''):
        """2^x for |x| <= 8: (e^(x ln2 / 16))^16, e^y by 4 Taylor terms (make_synth.py exp2_to, the form proven on
        the SYSTEM-1; 1.2 cents at |x| = 8)"""
        y = self.mul(x, 0.6931471805599453 / 16, 'y')
        t = self.mov(1 / 24, 't')
        for c in (1 / 6, 0.5, 1.0, 1.0):
            t = self.ymac(t, y, c)
        for _ in range(4):
            t = self.mul(t, t)
        return t

    def increment(self, pitch, name='inc'):
        """our phase step of a pitch word (octaves, 4.75 = A 440): 2^(pitch - 4.75) x 2 x 440 / 96000, as C-S1SY's
        saw (make_synth.py v8_saw)"""
        return self.mul(self.exp2(self.add(pitch, -4.75)), 2 * 440.0 / 96000, name)

    # ---- allocation and output ------------------------------------------------------------------------------
    @staticmethod
    def _x(v):
        if not isinstance(v, Value):
            raise KernelError(f'operand {v!r} must be a value (only Y may be an immediate or a negated value)')
        return v

    def _check(self, v):
        self._x(v)

    def _reads(self, op):
        _, _, _, operands, acc = op
        vals = [o.v if isinstance(o, Neg) else o for o in operands if isinstance(o, (Value, Neg))]
        return vals + ([acc] if acc is not None else [])

    def allocate(self):
        """linear scan in program order: a value lives from its line to its last read; free registers are taken
        least recently freed first (fewer false dependencies for the scheduler); an accumulator that is read again
        later is copied first (mov)"""
        ops = self._expand_acc()
        last = {}
        for i, op in enumerate(ops):
            for v in self._reads(op):
                last[v.n] = i
        free, live = list(self.regs), {}
        for i, op in enumerate(ops):
            reads = self._reads(op)
            for v in reads:
                if v.reg is None:
                    raise KernelError(f'{self.name}: {v} read before it is defined')
            for v in reads:                                 # operands that die here: free before the result is
                if last[v.n] == i and live.pop(v.n, None) is not None:   # placed (units read before they write)
                    free.append(v.reg)
            d, acc = op[1], op[4]
            if d is None:
                continue
            if acc is not None:
                free.remove(acc.reg)                        # the accumulator's register (it died here)
                d.reg = acc.reg
            elif not free:
                raise KernelError(f'{self.name}: out of registers at {self._text(op)!r}; live: '
                                  + ', '.join(map(repr, live.values())))
            else:
                d.reg = free.pop(0)
            if d.n in last:
                live[d.n] = d
            else:
                free.append(d.reg)                          # a result nobody reads
        return ops

    def _expand_acc(self):
        """mad / ymacd write their accumulator's register: when the accumulator is still read afterwards, work on
        a copy (mov) so the old value survives"""
        later = {}
        for i, op in enumerate(self.ops):
            for v in self._reads(op):
                later.setdefault(v.n, []).append(i)
        out = []
        for i, op in enumerate(self.ops):
            acc = op[4]
            if acc is not None and any(j > i for j in later.get(acc.n, [])):
                c = self._new(f'copy {acc}')
                out.append(('op', c, 'mov {d}, {0}', (acc,), None))
                op = op[:4] + (c,)
            out.append(op)
        return out

    @staticmethod
    def _operand(o):
        if isinstance(o, Neg):
            return f'-r{o.v.reg}'
        if isinstance(o, Value):
            return f'r{o.reg}'
        x = float(o)
        return f'#{x!r}'

    def _text(self, op):
        kind, d, fmt, operands, _ = op
        if kind == 'comment':
            return fmt
        args = [self._operand(o) for o in operands]
        return fmt.format(*args, d=f'r{d.reg}' if d is not None else '')

    def lines(self, fillers=True):
        """the plain program: one unit per line; fillers=True puts 'mov rN, rN' after a load whose register the next
        line uses (load latency 1), as the hand-written generators do"""
        ops = self.allocate()
        out = [self._text(op) for op in ops]
        if self.name:
            out.insert(0, f'; {self.name}')
        return add_fillers(out, self.regs) if fillers else out

    def count(self):
        return sum(1 for op in self.ops if op[0] != 'comment')


_REG = re.compile(r'\br(\d)\b')


def regs_of(line):
    return {int(t) for t in _REG.findall(line.split(';')[0])}


def add_fillers(lines, regs=range(8)):
    """after 'ld rN, [...]': a harmless 'mov rS, rS' when the next unit reads or writes rN"""
    out = []
    code = [i for i, x in enumerate(lines) if x.strip() and not x.startswith(';')]
    nxt = {a: b for a, b in zip(code, code[1:])}
    for i, x in enumerate(lines):
        out.append(x)
        if not x.startswith('ld ') or i not in nxt:
            continue
        dst = int(x.split()[1][1])
        if dst in regs_of(lines[nxt[i]]):
            spare = next(r for r in list(regs) + list(range(8)) if r != dst)
            out.append(f'mov r{spare}, r{spare}')
    return out
