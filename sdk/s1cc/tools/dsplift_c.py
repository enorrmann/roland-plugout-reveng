"""dsplift_c.py: the C that `dsplift.py --kernel` writes (docs/vol5_toolchain_compiler_sdk/dsplift.md §6).

The program is run symbolically, instruction by instruction, with the semantics of dsplift.py's units (all units read
the registers of the instruction start, loads land at its end, condition bits arrive one instruction later, a unit a
condition may skip is blended by a 0 / 1 weight). What it computes becomes an expression graph:

  - exact folding only: constants (float32 arithmetic), x * 1, x + 0, x * 0 (the DSP has no inf / NaN), x + (-y) ->
    x - y, 1 - (1 - b) -> b for 0 / 1 values, a blend whose weight is a constant; common subexpressions are shared;
  - what reaches no output is dropped, across samples too (a delay chain only lives while something live reads it);
  - a value is written as a named local when it is used more than once, stored in a bank-0 cell, named in the names
    file, or too long to inline; the rest is inlined;
  - idioms are macros of dsplift_k.h that expand to the same expression: SEL (a condition's blend), LERP (a + t (b - a)),
    K_NZ / K_GE0 / K_LE0 (the condition bits), K_RECIP (recipseed and its newton steps).

The C keeps the evaluation tree of every operation (no reassociation), so it computes what the per-instruction form
computed; the names file (--names) and the constants (--consts) only change names and fold known values.
"""
import re
import struct

import numpy as np


def f32(x):
    return float(np.float32(x))


def flit(v):
    """the shortest C float literal of the float32 value v"""
    if v == 0:
        return '-0.0f' if struct.pack('>f', v)[0] & 0x80 else '0.0f'
    s = str(np.float32(v))
    if s in ('inf', '-inf', 'nan'):
        raise SystemExit('dsplift_c: literal %r' % v)
    if '.' not in s and 'e' not in s:
        s += '.0'
    return s + 'f'


# ---------------------------------------------------------------- expression graph
class Node:
    __slots__ = ('id', 'op', 'val', 'args', 'k', 'bool', 'hints', 'uses', 'mat', 'name', 'txt', 'part', 'src')

    def __init__(self, nid, op, val, args, k, is_bool):
        self.id, self.op, self.val, self.args, self.k, self.bool = nid, op, val, args, k, is_bool
        self.hints, self.uses, self.mat, self.name, self.txt, self.part, self.src = [], 0, False, None, None, None, None


PREC_ATOM, PREC_NEG, PREC_MUL, PREC_ADD = 100, 90, 80, 70
FOLD = {
    's1_sign': lambda x: float(np.sign(np.float32(x))),
    's1_abs': lambda x: abs(x),
    'fminf': lambda x, y: min(x, y),
    'fmaxf': lambda x, y: max(x, y),
    's1_floor': lambda x: float(np.floor(np.float32(x))),
    's1_ceil': lambda x: float(np.ceil(np.float32(x))),
    'K_NZ': lambda z: 1.0 if z != 0 else 0.0,
    'K_Z': lambda z: 0.0 if z != 0 else 1.0,
    'K_GE0': lambda v: 0.0 if v < 0 else 1.0,
    'K_LT0': lambda v: 1.0 if v < 0 else 0.0,
    'K_LE0': lambda v: 1.0 if v <= 0 else 0.0,
    'K_GT0': lambda v: 0.0 if v <= 0 else 1.0,
    'K_LE0VZ': lambda v, z: 1.0 if v < 0 or z == 0 else 0.0,
    'K_GT0VZ': lambda v, z: 0.0 if v < 0 or z == 0 else 1.0,
    'K_CLAMP': lambda x: max(-1.0, min(1.0, x)),
    'pymod': lambda x, m: float(np.float32(x) - np.float32(m) * np.floor(np.float32(x) / np.float32(m))),
}
BOOL_CALLS = {'K_NZ', 'K_Z', 'K_GE0', 'K_LT0', 'K_LE0', 'K_GT0', 'K_LE0VZ', 'K_GT0VZ'}
# 1 - b of a condition bit: its complement (a macro that expands to 1 - b), and back (exact for 0 / 1)
COMPL = {'K_NZ': 'K_Z', 'K_Z': 'K_NZ', 'K_GE0': 'K_LT0', 'K_LT0': 'K_GE0', 'K_LE0': 'K_GT0', 'K_GT0': 'K_LE0',
         'K_LE0VZ': 'K_GT0VZ', 'K_GT0VZ': 'K_LE0VZ'}

# idioms of Roland's programs, as the lift computes them: (macro, where its argument is in the tree, the template);
# a node is the macro when the template built on that argument is the node itself (dsplift_k.h has the same text)
IDIOMS = [
    ('K_SIN', lambda n: n.args[0].args[1] if n.op == '-' and n.args[0].op == '+' else None,
     'x2 = X * X; x3 = x2 * X; '
     'out = x2 * x3 * (-0.00019836426f * x2 + 0.008333325f + x2 * x2 * 2.7418137e-06f) + X + -0.16666603f * x3'),
    ('K_EXP', lambda n: n.args[0].args[0].args[0] if n.op == '*' and n.args[0].op == 'call' and
     n.args[0].val == 's1_exp' and n.args[0].args[0].op == 'call' and n.args[0].args[0].val == 's1_floor' else None,
     'f = s1_floor(X); r = X - f; q = 0.25f * r * r; '
     'a = q * (q * (2.565328e-05f * r + 0.00028218678f) + 0.006349206f + 0.0007054673f * r); '
     'b = q * (q * (a + 0.08888889f + 0.012698413f * r) + 0.6666667f + 0.13333333f * r); '
     'out = s1_exp(f) * (q * (b + 2.0f + 0.6666667f * r) + 1.0f + r)'),
    ('K_TRI', lambda n: (n.args[1].args[1].args[0].args[0].args[0].args[0]
                         if n.op == '-' and n.args[1].op == '*' and n.args[1].args[1].op == 'call'
                         and n.args[1].args[1].val == 's1_abs' and n.args[1].args[1].args[0].op == 'call'
                         and n.args[1].args[1].args[0].args[0].op == '-'
                         and n.args[1].args[1].args[0].args[0].args[0].op == 'neg' else None),
     'out = 1.0f - 2.0f * s1_abs(s1_wrap(-X - 1.5f))'),
]


class Graph:
    def __init__(self):
        self.nodes, self.memo, self.k, self.part = [], {}, 0, None
        self.inexact = 0                       # blends with weight 1 folded although the sides are not 0 / 1

    def _new(self, op, val, args, is_bool=False):
        key = (self.part, op, val, tuple(a.id for a in args))
        n = self.memo.get(key)
        if n is None:
            n = Node(len(self.nodes), op, val, tuple(args), self.k, is_bool)
            n.part = self.part
            self.nodes.append(n)
            self.memo[key] = n
        return n

    def num(self, v):
        v = f32(v)
        return self._new('num', struct.pack('>f', v), (), v in (0.0, 1.0))

    def var(self, name, is_bool=False):
        """a C name read as it is: a static, an argument, an input"""
        return self._new('var', name, (), is_bool)

    def ld(self, macro, a):
        return self._new('ld', (macro, a), ())

    @staticmethod
    def value(n):
        return struct.unpack('>f', n.val)[0]

    def isnum(self, n, v=None):
        return n.op == 'num' and (v is None or self.value(n) == v)

    def neg(self, x):
        if x.op == 'num':
            return self.num(-self.value(x))
        if x.op == 'neg':
            return x.args[0]
        return self._new('neg', None, (x,))

    def bin(self, op, x, y):
        if x.op == 'num' and y.op == 'num':
            a, b = np.float32(self.value(x)), np.float32(self.value(y))
            with np.errstate(all='ignore'):
                r = {'+': a + b, '-': a - b, '*': a * b, '/': a / b}[op]
            if np.isfinite(r):
                return self.num(float(r))
        if op == '+':
            if self.isnum(x, 0):
                return y
            if self.isnum(y, 0):
                return x
            if y.op == 'neg':
                return self.bin('-', x, y.args[0])
            if y.op == 'num' and self.value(y) < 0:
                return self.bin('-', x, self.num(-self.value(y)))
            if x.op == 'neg':
                return self.bin('-', y, x.args[0])
            if y.op == '*' and y.args[0].op == 'num' and self.value(y.args[0]) < 0:
                return self.bin('-', x, self.bin('*', self.num(-self.value(y.args[0])), y.args[1]))
            if x.op == '*' and x.args[0].op == 'num' and self.value(x.args[0]) < 0:
                return self.bin('-', y, self.bin('*', self.num(-self.value(x.args[0])), x.args[1]))
            for a, m in ((x, y), (y, x)):                 # a + t * (b - a) -> LERP(a, b, t)
                if m.op == '*':
                    for t, d in (m.args, m.args[::-1]):
                        if d.op == '-' and d.args[1] is a:
                            return self._new('lerp', None, (a, d.args[0], t))
            return self._new('+', None, (x, y))
        if op == '-':
            if self.isnum(y, 0):
                return x
            if self.isnum(x, 0):
                return self.neg(y)
            if y.op == 'neg':
                return self.bin('+', x, y.args[0])
            if y.op == 'num' and self.value(y) < 0:
                return self.bin('+', x, self.num(-self.value(y)))
            if self.isnum(x, 1) and y.op == '-' and self.isnum(y.args[0], 1) and y.args[1].bool:
                return y.args[1]                          # 1 - (1 - b) = b, exact for b = 0 / 1
            if self.isnum(x, 1) and y.op == 'call' and y.val in COMPL:
                return self._new('call', COMPL[y.val], y.args, True)
            if x is y:
                return self.num(0.0)
            return self._new('-', None, (x, y), self.isnum(x, 1) and y.bool)
        if op == '*':
            if self.isnum(x, 0) or self.isnum(y, 0):
                return self.num(0.0)                      # the DSP has no inf / NaN
            if self.isnum(x, 1):
                return y
            if self.isnum(y, 1):
                return x
            if self.isnum(x, -1):
                return self.neg(y)
            if self.isnum(y, -1):
                return self.neg(x)
            if x.op == 'neg':
                return self.neg(self.bin('*', x.args[0], y))
            if y.op == 'neg':
                return self.neg(self.bin('*', x, y.args[0]))
            # recipseed: s1_sign(Y) * s1_recip(fmaxf(s1_abs(Y), 2^-9)) -> K_RECIP(Y)
            if x.op == 'call' and x.val == 's1_sign' and y.op == 'call' and y.val == 's1_recip':
                m = y.args[0]
                if (m.op == 'call' and m.val == 'fmaxf' and m.args[0].op == 'call' and m.args[0].val == 's1_abs'
                        and m.args[0].args[0] is x.args[0] and self.isnum(m.args[1], 2.0 ** -9)):
                    return self._new('call', 'K_RECIP', (x.args[0],))
            return self._new('*', None, (x, y), x.bool and y.bool)
        if op == '/':
            if self.isnum(y, 1):
                return x
            return self._new('/', None, (x, y))
        raise ValueError(op)

    def call(self, f, *args):
        f = {'fabs': 's1_abs', 'pymin': 'fminf', 'pymax': 'fmaxf', 'wrap1': 's1_wrap'}.get(f, f)
        if f == 'AWRAP':
            return args[0]
        if f in FOLD and all(a.op == 'num' for a in args):
            return self.num(FOLD[f](*[self.value(a) for a in args]))
        if f in ('fminf', 'fmaxf') and args[0] is args[1]:
            return args[0]
        if f == 'K_LE0VZ' and args[0] is args[1]:
            f, args = 'K_LE0', args[:1]
        if (f == 'fmaxf' and self.isnum(args[0], -1) and args[1].op == 'call' and args[1].val == 'fminf'
                and self.isnum(args[1].args[0], 1)):
            f, args = 'K_CLAMP', args[1].args[1:]
        return self._new('call', f, args, f in BOOL_CALLS or (f in ('fminf', 'fmaxf') and all(a.bool for a in args)))

    @staticmethod
    def xfade(n):
        """w * b + a - w * a (a crossfade as a MAC computes it) -> (a, b, w); products in either order (exact)"""
        if n.op != '-' or n.args[0].op != '+' or n.args[1].op != '*' or n.args[0].args[0].op != '*':
            return None
        (m1, a), m2 = n.args[0].args, n.args[1]
        if a is m2.args[0]:
            w = m2.args[1]
        elif a is m2.args[1]:
            w = m2.args[0]
        else:
            return None
        if m1.args[0] is w:
            return a, m1.args[1], w
        if m1.args[1] is w:
            return a, m1.args[0], w
        return None

    def idioms(self):
        """the nodes that are an idiom of IDIOMS become that macro on its argument; XFADE"""
        found = {}
        for n in list(self.nodes):
            xf = self.xfade(n)
            if xf:
                n.op, n.val, n.args = 'call', 'XFADE', xf
                found['XFADE'] = found.get('XFADE', 0) + 1
                continue
            for mac, where, tpl in IDIOMS:
                try:
                    x = where(n)
                except (IndexError, AttributeError):
                    x = None
                if x is None:
                    continue
                self.part, self.k = n.part, n.k
                loc = {'X': x}
                for st in tpl.split(';'):
                    nm, e = st.split('=', 1)
                    loc[nm.strip()] = self.eval(Parser(e).expr(), loc)
                if loc['out'] is n:
                    n.op, n.val, n.args = 'call', mac, (x,)
                    found[mac] = found.get(mac, 0) + 1
                    break
        self.part = None
        return found

    def eval(self, e, loc):
        t = e[0]
        if t == 'num':
            return self.num(e[1])
        if t == 'id':
            return loc[e[1]]
        if t == 'neg':
            return self.neg(self.eval(e[1], loc))
        if t == 'bin':
            return self.bin(e[1], self.eval(e[2], loc), self.eval(e[3], loc))
        if t == 'call':
            return self.call(e[1], *[self.eval(a, loc) for a in e[2]])
        raise ValueError(e)

    def sel(self, w, a, b):
        """w ? a : b for w = 0 / 1, as b + w (a - b) (the blend of a unit a condition may skip)"""
        if a is b or self.isnum(w, 0):
            return b
        if self.isnum(w, 1):
            if not (a.bool and b.bool):
                self.inexact += 1
            return a
        return self._new('sel', None, (w, a, b), a.bool and b.bool)


# ---------------------------------------------------------------- the unit text of dsplift.py, parsed
TOK = re.compile(r'\s*(?:(0x[0-9a-fA-F]+(?:\.[0-9a-fA-F]*)?p[+-]?\d+f?)|(\d+\.\d*(?:e[+-]?\d+)?f?|\d+e[+-]?\d+f?)|(\d+)'
                 r'|([A-Za-z_]\w*)|(.))')


def tokens(s):
    out, pos = [], 0
    s = s.strip()
    while pos < len(s):
        m = TOK.match(s, pos)
        if not m:
            raise SystemExit('dsplift_c: cannot read %r' % s[pos:])
        pos = m.end()
        if m.group(1):
            out.append(('num', float.fromhex(m.group(1).rstrip('f'))))
        elif m.group(2):
            out.append(('num', float(m.group(2).rstrip('f'))))
        elif m.group(3):
            out.append(('num', float(m.group(3))))
        elif m.group(4):
            out.append(('id', m.group(4)))
        elif m.group(5).strip():
            out.append(('op', m.group(5)))
    return out


class Parser:
    def __init__(self, s):
        self.t, self.i = tokens(s), 0

    def peek(self):
        return self.t[self.i] if self.i < len(self.t) else ('end', None)

    def take(self, v=None):
        tk = self.peek()
        if v is not None and tk[1] != v:
            raise SystemExit('dsplift_c: expected %r, got %r' % (v, tk))
        self.i += 1
        return tk

    def stmts(self):
        out = []
        while self.peek()[0] != 'end':
            tk = self.peek()
            if tk == ('op', '{') or tk == ('op', '}') or tk == ('op', ';'):
                self.take()
                continue
            if tk == ('id', 'float'):
                self.take()
                name = self.take()[1]
                self.take('=')
                out.append(('set', name, self.expr()))
                continue
            if tk[0] == 'id' and tk[1] in ('RES', 'RESV', 'RESZ'):
                self.take()
                self.take('(')
                args = [self.expr()]
                while self.peek() == ('op', ','):
                    self.take()
                    args.append(self.expr())
                self.take(')')
                out.append((tk[1].lower(), args))
                continue
            name = self.take()[1]
            self.take('=')
            out.append(('set', name, self.expr()))
        return out

    def expr(self, prec=0):
        left = self.unary()
        while True:
            tk = self.peek()
            p = {'+': 1, '-': 1, '*': 2, '/': 2}.get(tk[1]) if tk[0] == 'op' else None
            if p is None or p <= prec:
                return left
            self.take()
            left = ('bin', tk[1], left, self.expr(p))

    def unary(self):
        tk = self.peek()
        if tk == ('op', '-'):
            self.take()
            return ('neg', self.unary())
        if tk == ('op', '('):
            self.take()
            e = self.expr()
            self.take(')')
            return e
        self.take()
        if tk[0] == 'num':
            return ('num', tk[1])
        if self.peek() == ('op', '('):
            self.take()
            args = []
            if self.peek() != ('op', ')'):
                args.append(self.expr())
                while self.peek() == ('op', ','):
                    self.take()
                    args.append(self.expr())
            self.take(')')
            return ('call', tk[1], args)
        return ('id', tk[1])


PARSED = {}


def parse_unit(txt):
    if txt not in PARSED:
        PARSED[txt] = Parser(txt).stmts()
    return PARSED[txt]


# ---------------------------------------------------------------- names and constants
class Names:
    """--names FILE: lines
         param C019 cutoff [knob]   a parameter word (KP), its name; knob: set by the panel (not a --consts value)
         in A004 pitch              a bank-5 input (KIN)
         out A100 voice             a bank-5 output (KOUT)
         unused A101 A102           outputs the including file ignores (KOUT does nothing): not computed
         cell 041 flt_env           the value stored in bank-0 cell 041
         value t137 decay_eff       rename a value the C calls t137 (its default name: the instruction that makes it)
         section 99 text...         a heading above the values from instruction 99 on
         const A003 1               a memory word that is this constant in the caller's setup (as --consts)
       --consts FILE: lines `ADDR VALUE` or `ADDR-ADDR VALUE` (hex, C000.. / A000.. / E000..): memory words that are
       constants in the including file's setup (KP / KIN / KE give that value)"""

    def __init__(self, path=None, consts=None):
        self.param, self.inp, self.out, self.unused, self.cell, self.value, self.section = {}, {}, {}, set(), {}, {}, []
        self.consts, self.knob = [], set()
        if path:
            for ln in open(path):
                ln = ln.split('#', 1)[0].strip()
                if not ln:
                    continue
                w = ln.split()
                kw = w[0]
                if kw == 'param':
                    self.param[int(w[1], 16)] = w[2]
                    if w[3:] == ['knob']:
                        self.knob.add(int(w[1], 16))
                elif kw == 'in':
                    self.inp[int(w[1], 16)] = w[2]
                elif kw == 'out':
                    self.out[int(w[1], 16)] = w[2]
                elif kw == 'unused':
                    self.unused |= {int(x, 16) for x in w[1:]}
                elif kw == 'cell':
                    self.cell[int(w[1], 16)] = w[2]
                elif kw == 'value':
                    self.value[w[1]] = w[2]
                elif kw == 'const':
                    self.add_const(w[1], w[2])
                elif kw == 'section':
                    self.section.append((int(w[1]), ln.split(None, 2)[2]))
                else:
                    raise SystemExit('%s: unknown line %r' % (path, ln))
        self.section.sort()
        if consts:
            for ln in open(consts):
                ln = ln.split('#', 1)[0].strip()
                if ln:
                    self.add_const(*ln.split())

    def add_const(self, a, v):
        lo, hi = (a.split('-') + [a])[:2]
        lo, hi = int(lo, 16), int(hi, 16)
        if any(lo <= k <= hi for k in self.knob):
            raise SystemExit('constant %s: covers a knob parameter' % a)
        self.consts.append((lo, hi, float(v)))

    def const(self, a):
        for lo, hi, v in self.consts:
            if lo <= a <= hi:
                return v
        return None

    def macro(self, kind, a):
        nm = {'KP': self.param, 'KIN': self.inp, 'KOUT': self.out}.get(kind, {}).get(a)
        if nm is None:
            return None
        return {'KP': 'P_', 'KIN': 'IN_', 'KOUT': 'OUT_'}[kind] + nm.upper()

    def static(self, x, k):
        """the delay-chain static: what cell x held k samples ago"""
        return ('%s_z%d' % (self.cell[x], k)) if x in self.cell else 'D_%03x_%d' % (x, k)


# ---------------------------------------------------------------- symbolic run of the instructions
class State(dict):
    """the machine state of a part: r0..r7, f<c>, pm<g>, pv<c>, c_<x>, K5_<a>; a missing entry is created on its first
    read by `init` (part B: the values part A hands over, or the part's own delay chains)"""

    def __init__(self, init):
        super().__init__()
        self.init, self.order = init, []

    def __missing__(self, key):
        v = self.init(key)
        self[key] = v
        self.order.append(key)
        return v


class Run:
    def __init__(self, G, names, used_cc, used_g, unit_code, bq2, bq2_flagless, overflow_wrap, own5, K_A, K_B, K_M,
                 K_XMEM, K_XFLAG, name):
        self.G, self.names, self.used_cc, self.used_g = G, names, used_cc, used_g
        self.unit_code, self.bq2, self.flagless, self.overflow_wrap, self.own5 = unit_code, bq2, bq2_flagless, \
            overflow_wrap, own5
        self.KA, self.KB, self.KM, self.KX, self.KF, self.name = K_A, K_B, K_M, K_XMEM, K_XFLAG, name
        self.kouts = []                     # (node id order key, address, value node)
        self.cellhint = {}                  # node id -> cell names stored

    def load(self, a, S):
        G, nm = self.G, self.names
        c = self.cell_of(a)
        if c is not None:
            return S['c_%03x' % c]
        if 0xA000 <= a < 0xC000:
            if a in self.own5:
                return S['K5_%04x' % a]
            kind = 'KIN'
        elif 0xC000 <= a < 0xE000:
            kind = 'KP'
        elif a >= 0xE000:
            kind = 'KE'
        else:
            return None
        c = nm.const(a)
        return G.num(c) if c is not None else G.ld(kind, a)

    def insn(self, k, units, normal, S):
        G = self.G
        G.k = k
        r = [S['r%d' % i] for i in range(8)]
        n = list(r)
        skips = []
        for cc, neg, T, F in normal:
            c = S['f%d' % cc]
            if neg:
                c = G.bin('-', G.num(1.0), c)
            if 1 <= T <= len(units) or 1 <= F <= len(units):
                skips.append((c, T, F))

        def weight(i):
            w = None
            for c, T, F in skips:
                for x in ([G.bin('-', G.num(1.0), c)] if T == i + 1 else []) + ([c] if F == i + 1 else []):
                    w = x if w is None else G.bin('*', w, x)
            return w
        newp = {}
        nam = nb = 0
        for i, (uk, o) in enumerate(units):
            if uk == 'ctl':
                continue
            kind = o['kind']
            w = weight(i)
            if kind in (self.KA, self.KB, self.KM):
                txt = self.unit_code(o)
                if k in self.overflow_wrap:
                    txt = txt.replace('AWRAP(', 'KOVERFLOW(')
                txt = re.sub(r'RESZ\((n\d), \((-?)(r\d)\) == ([^)]+)\);', r'RESZ(\1, (\2\3) - (\4));', txt)
                old = list(n)
                loc, res, wrote = {}, None, set()

                def ev(e):
                    t = e[0]
                    if t == 'num':
                        return G.num(e[1])
                    if t == 'id':
                        v = e[1]
                        if re.fullmatch(r'r[0-7]', v):
                            return r[int(v[1])]
                        if re.fullmatch(r'n[0-7]', v):
                            return n[int(v[1])]
                        if v in loc:
                            return loc[v]
                        raise SystemExit('%s %d: unknown name %s in %r' % (self.name, k, v, txt))
                    if t == 'neg':
                        return G.neg(ev(e[1]))
                    if t == 'bin':
                        return G.bin(e[1], ev(e[2]), ev(e[3]))
                    if t == 'call':
                        return G.call(e[1], *[ev(x) for x in e[2]])
                    raise ValueError(e)
                for st in parse_unit(txt) if txt else []:
                    if st[0] == 'set':
                        v = ev(st[2])
                        if re.fullmatch(r'n[0-7]', st[1]):
                            n[int(st[1][1])] = v
                            wrote.add(int(st[1][1]))
                        else:
                            loc[st[1]] = v
                    elif st[0] == 'res':
                        x = ev(st[1][0])
                        res = (x, x)
                    elif st[0] == 'resv':
                        x = ev(st[1][0])
                        res = (x, x)
                    elif st[0] == 'resz':
                        res = (ev(st[1][0]), ev(st[1][1]))
                dest = (o['w'] >> 12) & 7
                grp = None
                if self.bq2 is not None and not self.flagless(o):
                    if kind == self.KB:
                        nb += 1
                        grp = 2 + nb if nb <= 2 else None
                    else:
                        nam += 1
                        grp = nam if nam <= 2 else None
                if grp in self.used_g:
                    v, z = res if res else (n[dest], n[dest])
                    newp[grp] = (v, z, w)
                if w is not None:
                    for rr in wrote:
                        n[rr] = G.sel(w, n[rr], old[rr])
            elif kind == self.KF:
                raise SystemExit('%s %d: X9bY not in --kernel' % (self.name, k))
            elif kind == self.KX:
                a = o['b']
                if o['a'] < 8:
                    src = self.load(a, S)
                    if src is None:
                        continue
                    n[o['a']] = src if w is None else G.sel(w, src, n[o['a']])
                else:
                    v = r[o['a'] - 8]
                    if self.cell_of(a) is not None:
                        key = 'c_%03x' % self.cell_of(a)
                    elif 0xA000 <= a < 0xC000:
                        key = 'K5_%04x' % a
                    else:
                        continue
                    nv = v if w is None else G.sel(w, v, S[key])
                    S[key] = nv
                    if key.startswith('c_'):
                        self.cellhint.setdefault(nv.id, []).append(self.cell_of(a))
                        if a in self.kout_extra:
                            self.kouts.append((len(G.nodes), a, nv, G.part))
                    else:
                        self.kouts.append((len(G.nodes), a, nv, G.part))
        one = G.num(1.0)
        for g in self.used_g:                          # the pending bits of the instruction before -> condition bits
            pm = S['pm%d' % g]
            if G.isnum(pm, 0):
                continue
            for c in self.used_cc:
                if c >> 2 == g:
                    S['f%d' % c] = G.sel(pm, S['pv%d' % c], S['f%d' % c])
        for g in self.used_g:                          # this instruction's -> pending
            if g in newp:
                v, z, w = newp[g]
                bits = {0: lambda: G.call('K_NZ', z), 1: lambda: G.call('K_GE0', v), 2: lambda: one,
                        3: lambda: G.call('K_LE0VZ', v, z)}
                S['pm%d' % g] = w if w is not None else one
                for c in self.used_cc:
                    if c >> 2 == g:
                        S['pv%d' % c] = bits[c & 3]()
            else:
                S['pm%d' % g] = G.num(0.0)
        for i in range(8):
            S['r%d' % i] = n[i]


# ---------------------------------------------------------------- C
class Emitter:
    LIMIT = 100                 # characters of an inlined expression before its parts get names

    def __init__(self, G, names, cellhint):
        self.G, self.names, self.cellhint = G, names, cellhint
        self.taken, self.statics, self.used_values = {}, set(), set()

    def prec(self, n):
        if n.mat or n.op in ('var', 'imp', 'ld', 'call', 'sel', 'lerp'):
            return PREC_ATOM
        if n.op == 'num':
            return PREC_NEG if self.G.value(n) < 0 or flit(self.G.value(n)).startswith('-') else PREC_ATOM
        return {'neg': PREC_NEG, '*': PREC_MUL, '/': PREC_MUL, '+': PREC_ADD, '-': PREC_ADD}[n.op]

    def ref(self, n):
        return (n.name or 'v%d' % n.id) if n.mat else n.txt

    def text(self, n):
        G, op = self.G, n.op
        if op == 'num':
            return flit(G.value(n))
        if op == 'var':
            return n.val
        if op == 'imp':
            return self.imptext(n)
        if op == 'ld':
            kind, a = n.val
            return '%s(%s)' % (kind, self.names.macro(kind, a) or '0x%04x' % a)
        if op == 'call':
            return '%s(%s)' % (n.val, ', '.join(self.ref(a) for a in n.args))
        if op == 'sel':
            return 'SEL(%s)' % ', '.join(self.ref(a) for a in n.args)
        if op == 'lerp':
            return 'LERP(%s)' % ', '.join(self.ref(a) for a in n.args)
        if op == 'neg':
            a = n.args[0]
            s = self.ref(a)
            return '-' + ('(%s)' % s if self.prec(a) < PREC_ATOM else s)
        x, y = n.args
        p = self.prec(n)
        sx, sy = self.ref(x), self.ref(y)
        if self.prec(x) < p:
            sx = '(%s)' % sx
        if self.prec(y) <= p and not (y.op == 'num' and self.prec(y) == PREC_ATOM):
            sy = '(%s)' % sy
        return '%s %s %s' % (sx, op, sy)

    def ident(self, base, part):
        """a C name not yet used in this part (the parts are separate functions)"""
        base = re.sub(r'\W', '_', base)
        taken = self.taken.setdefault(part, set(self.statics))
        nm, i = base, 2
        while nm in taken:
            nm, i = '%s_%d' % (base, i), i + 1
        taken.add(nm)
        return nm

    def plan(self, roots, forced=()):
        """use counts, which nodes get a name, their text; roots = nodes the C needs (outputs, exports, statics)"""
        G = self.G
        live = set()
        stack = list(roots)
        while stack:
            n = stack.pop()
            if n.id in live:
                continue
            live.add(n.id)
            stack.extend(n.args)
        for n in G.nodes:
            n.uses, n.mat, n.name, n.txt = 0, False, None, None
        for n in G.nodes:
            if n.id in live:
                for a in n.args:
                    a.uses += 1
        for n in roots:
            n.uses += 1
        self.live = live
        for n in G.nodes:
            if n.id not in live:
                continue
            trivial = n.op in ('num', 'var')
            named = bool(n.hints) or n.id in self.cellhint
            n.mat = n.id in forced or (not trivial and (n.uses > 1 or (named and n.op != 'ld')))
            n.txt = self.text(n)
            while len(n.txt) > self.LIMIT:
                big = [a for a in n.args if not a.mat and a.op not in ('num', 'var', 'ld')]
                if not big:
                    break
                max(big, key=lambda a: len(a.txt)).mat = True
                n.txt = self.text(n)
        for n in G.nodes:                                  # names, in program order
            if n.id in live and n.mat:
                n.name = self.ident(self.basename(n), n.part)
        for n in G.nodes:                                  # the names file's renames of default names
            if n.id in live and n.mat and n.name in self.names.value:
                self.used_values.add(n.name)
                n.name = self.ident(self.names.value[n.name], n.part)
        for n in G.nodes:                                  # the texts again, with the names
            if n.id in live:
                n.txt = self.text(n)

    def basename(self, n):
        if n.hints:
            return n.hints[0]
        if n.op == 'var':
            return n.val + '_in'
        if n.op == 'imp':
            x = n.val
            if n.src.name:
                return n.src.name                 # the name the value has in part A
            if x.startswith('c_'):
                return self.names.cell.get(int(x[2:], 16), x)
            return 'a_' + x
        cells = self.cellhint.get(n.id, [])
        for x in cells:
            if x in self.names.cell:
                return self.names.cell[x]
        if n.op == 'ld':
            kind, a = n.val
            nm = {'KP': self.names.param, 'KIN': self.names.inp}.get(kind, {}).get(a)
            if nm:
                return nm
        if cells:
            return 'c_%03x' % cells[0]
        if n.op == 'ld':
            return '%s_%04x' % ({'KP': 'p', 'KIN': 'in', 'KE': 'e'}[kind], a)
        if n.op == 'call' and n.val in BOOL_CALLS:
            a = n.args[0]
            sfx = n.val[2:].lower().replace('vz', '')
            return '%s_%s' % (a.name, sfx) if a.name else 'b%d_%s' % (n.k, sfx)
        return 't%d' % n.k

    def body(self, ids, kouts=(), lo=0, hi=1 << 30):
        """the named nodes among ids (program order) as `float name = ...;` lines, the outputs (KOUT) where the program
        writes them, the section headings of the instructions lo..hi-1"""
        out, sec = [], [s for s in self.names.section if lo <= s[0] < hi]
        items = [(nid, 0, self.G.nodes[nid]) for nid in ids if self.G.nodes[nid].mat]
        items += [(pos - 0.5, 1, (a, v)) for pos, a, v in kouts]
        width = 96
        for key, kind, x in sorted(items, key=lambda t: t[0]):
            k = x.k if kind == 0 else x[1].k
            while sec and sec[0][0] <= k:
                if out:
                    out.append('')
                out.append('    /* ---- %s ---- */' % sec.pop(0)[1])
            if kind == 0:
                ln = '    float %s = %s;' % (x.name, self.text_of_def(x))
                out.append(ln.ljust(width) + ' /* %d */' % x.k)
            else:
                a, v = x
                out.append('    KOUT(%s, %s);' % (self.names.macro('KOUT', a) or '0x%04x' % a, self.ref(v)))
        return out

    def text_of_def(self, n):
        mat, n.mat = n.mat, False
        t = self.text(n)
        n.mat = mat
        return t


# ---------------------------------------------------------------- the kernel
def emit_kernel(ctx):
    """ctx: name, insns [(units, normal)], unit_code, touched, own5, used_cc, used_g, live_r, live_f, bq2,
    bq2_flagless, overflow_wrap, split, zero_cells, export_base, ranges, names, K (the dspcomp kinds), warn.
    Returns the C lines."""
    name, insns, names, K = ctx['name'], ctx['insns'], ctx['names'], ctx['K']
    touched, split, N = ctx['touched'], ctx['split'], len(insns)
    if split is not None and (ctx['live_r'] or ctx['live_f']):
        raise SystemExit('%s: registers / condition bits live between samples: no --split' % name)
    # the cells whose delay chain only ever holds 0: assumed for all at first (None), then the greatest fixed point
    # (a chain starts at 0; if every sample writes 0 when the assumed chains hold 0, they hold 0 forever)
    known, cand = ctx.get('known'), set()
    G = Graph()
    run = Run(G, names, ctx['used_cc'], ctx['used_g'], ctx['unit_code'], ctx['bq2'], ctx['bq2_flagless'],
              ctx['overflow_wrap'], ctx['own5'], K['A'], K['B'], K['M'], K['XMEM'], K['XFLAG'], name)

    # bank 0: a cell's value at the start of a sample is what the nearest touched address below held one or more
    # samples ago (the ring moves by one word per sample); each delay chain belongs to the part that writes it last
    def src(a):
        for d in range(1, 64):
            if a - d in touched:
                return a - d, d
        return None
    parts = [('a', 0, split), ('b', split, N)] if split is not None else [('', 0, N)]
    cell_of = ctx.get('cell_of') or (lambda a: a & 0x7FF if a < 0x2000 else None)
    run.cell_of = cell_of
    run.kout_extra = set(ctx.get('kout_extra') or ())
    written = [{cell_of(o['b']) for uk, o in units if uk == 'op' and o['kind'] == K['XMEM'] and o['a'] >= 8
                and cell_of(o['b']) is not None} for units, _ in insns]
    cells = {p: set().union(*written[lo:hi]) for p, lo, hi in parts}
    owner = {}
    for a in sorted(touched):
        owner[a] = next((p for p, _, _ in reversed(parts) if a in cells[p]), None)
        if owner[a] is None:
            s = src(a)
            owner[a] = owner[s[0]] if s else parts[0][0]
    statics = {}                                    # static name -> (cell, k)

    anyw = set().union(*cells.values())

    def livein(a, part):
        s = src(a)
        if s is None:
            return G.num(0.0)
        x, d = s
        while x not in anyw:                        # x only delays its own source: the source, further back
            s = src(x)
            if s is None:
                return G.num(0.0)
            x, d = s[0], d + s[1]
        if owner[x] != part:
            if a in ctx['zero_cells']:
                return G.num(0.0)
            raise SystemExit('%s: --split %d: cell %03x of part %s comes from %03x of the other part '
                             '(a value back from B to A): not possible' % (name, split, a, part.upper(), x))
        nm = names.static(x, d)
        cand.add(x)
        if known is None or x in known:
            return G.num(0.0)
        statics[nm] = (x, d)
        return G.var(nm)
    ranges = ctx['ranges'] or {}
    states = {}

    def init_first(key):
        if key.startswith('r'):
            i = int(key[1:])
            return G.var('%s_r%d' % (name, i)) if i in ctx['live_r'] else G.num(0.0)
        if key.startswith('f'):
            c = int(key[1:])
            return G.var('%s_f%d' % (name, c), True) if c in ctx['live_f'] else G.num(0.0)
        if key.startswith('pm') or key.startswith('pv'):
            return G.num(0.0)
        if key.startswith('c_'):
            return livein(int(key[2:], 16), parts[0][0])
        return G.var(key)                           # K5_xxxx: the program's own bank-5 word

    def init_b(key):
        SA = states['a']
        if key.startswith('K5_'):
            return G.var(key)
        if key.startswith('c_'):
            a = int(key[2:], 16)
            s = src(a)
            if not (a in cells['a'] or (s and owner[s[0]] == 'a')):
                return livein(a, 'b')
        v = SA[key]                                 # what part A has at its end
        if v.op == 'num':
            return G.num(G.value(v))                # a constant: no hand-over
        if key in ranges and ranges[key][0] == ranges[key][1]:
            return G.num(ranges[key][0])            # measured constant (--import-ranges)
        if v.id not in impof:                       # one slot per value of part A
            impof[v.id] = G._new('imp', key, (), key[0] in 'fp')
            impof[v.id].src = v
        return impof[v.id]
    impof = {}
    for p, lo, hi in parts:
        G.part = p
        S = State(init_first if p in ('', 'a') else init_b)
        states[p] = S
        for k in range(lo, hi):
            run.insn(k, insns[k][0], insns[k][1], S)
    G.part = None
    idioms = G.idioms()

    # what is needed: the outputs, then (to a fixed point) the statics and hand-overs the needed values read
    kouts = [(pos, a, v, p) for pos, a, v, p in run.kouts if a not in names.unused]
    roots = [v for _, _, v, _ in kouts]
    for i in sorted(ctx['live_r']):
        roots.append(states[''][('r%d' % i)])
    for c in sorted(ctx['live_f']):
        roots.append(states['']['f%d' % c])
    chains, imps = {}, []

    def reach(rs):
        seen, stack = set(), list(rs)
        while stack:
            n = stack.pop()
            if n.id not in seen:
                seen.add(n.id)
                stack.extend(n.args)
        return seen
    while True:
        live = reach(roots)
        new = []
        for nid in sorted(live):
            n = G.nodes[nid]
            if n.op == 'var' and n.val in statics:
                x, d = statics[n.val]
                if chains.get(x, 0) < d:
                    for kk in range(chains.get(x, 0) + 1, d + 1):
                        statics.setdefault(names.static(x, kk), (x, kk))
                    if x not in chains:
                        new.append(states[owner[x]]['c_%03x' % x])
                    chains[x] = d
            if n.op == 'imp' and n not in imps:
                imps.append(n)
                new.append(states['a'][n.val])
            if n.op == 'var' and n.val.startswith('K5_') and n.val not in chains:
                chains[n.val] = 0
                new.append(states[parts[-1][0]][n.val])
        if not new:
            break
        roots += new
    zero = {x for x in cand if (known is None or x in known) and G.isnum(states[owner[x]]['c_%03x' % x], 0)}
    if zero != known:
        ctx['known'] = zero
        return emit_kernel(ctx)
    imps.sort(key=lambda n: n.id)
    slot = {n.val: ctx['export_base'] + i for i, n in enumerate(imps)}

    em = Emitter(G, names, run.cellhint)

    def imptext(n):
        e = 's1_import(%d)' % slot[n.val]
        if n.val in ranges:
            lo, hi = ranges[n.val]
            m = 0.25 * (hi - lo) + 1e-3
            e = 's1_range(%s, %s, %s)' % (e, flit(f32(lo - m)), flit(f32(hi + m)))
        return e
    em.imptext = imptext

    # the end of each part: hand-overs, statics; ordered so that no static is overwritten before it is read
    def ends(p):
        S, out = states[p], []
        if p == 'a':
            for n in imps:
                out.append(('s1_export(%d, %%s);' % slot[n.val], None, S[n.val]))
        if p == '':
            for i in sorted(ctx['live_r']):
                out.append(('%s_r%d = %%s;' % (name, i), '%s_r%d' % (name, i), S['r%d' % i]))
            for c in sorted(ctx['live_f']):
                out.append(('%s_f%d = %%s;' % (name, c), '%s_f%d' % (name, c), S['f%d' % c]))
        for x in sorted(k for k in chains if not isinstance(k, str)):
            if owner[x] != p:
                continue
            for kk in range(chains[x], 1, -1):
                t = names.static(x, kk)
                out.append(('%s = %%s;' % t, t, G.var(names.static(x, kk - 1))))
            t = names.static(x, 1)
            out.append(('%s = %%s;' % t, t, S['c_%03x' % x]))
        for x in sorted(k for k in chains if isinstance(k, str)):
            if p == parts[-1][0]:
                out.append(('%s = %%s;' % x, x, S[x]))
        return out
    forced = set(n.id for n in imps)
    allroots = list(roots) + [v for p, _, _ in parts for _, _, v in ends(p)]
    em.statics = set(statics) | {x for x in chains if isinstance(x, str)}
    for _ in range(64):
        em.taken = {}
        em.plan(allroots, forced)
        order, bad = {}, None
        for p, _, _ in parts:
            ws = ends(p)

            def reads(n, acc):
                if n.mat:
                    return acc
                if n.op == 'var':
                    acc.add(n.val)
                for a in n.args:
                    reads(a, acc)
                return acc
            rd = [reads(v, set()) for _, _, v in ws]
            todo, seq = list(range(len(ws))), []
            while todo:
                for i in todo:
                    t = ws[i][1]
                    if t is None or not any(t in rd[j] for j in todo if j != i):
                        break
                else:
                    t = ws[todo[0]][1]
                    bad = next(G.nodes[nid] for nid in sorted(em.live) if G.nodes[nid].op == 'var'
                               and G.nodes[nid].val == t)
                    break
                todo.remove(i)
                seq.append(i)
            if bad:
                break
            order[p] = [ws[i] for i in seq]
        if bad is None:
            break
        forced.add(bad.id)
    else:
        raise SystemExit('%s: the statics cannot be ordered' % name)

    # ---- the C
    out = ['/* %s: written by sdk/s1cc/tools/dsplift.py --kernel%s. Every operation of the program, in order, as'
           % (name, (' --split %d' % split) if split is not None else ''),
           ' * expressions (dsplift.md §6): a name per value that is used more than once, stored in bank 0 or too long;',
           ' * the number after each line = the instruction that makes it. SEL(w, a, b) = w ? a : b (w = 0 / 1), LERP(a, b, t) = a + t (b - a),',
           ' * K_NZ / K_GE0 / K_LE0 = the condition bits, K_RECIP = the reciprocal (dsplift_k.h). Delay chains: X_zK =',
           ' * what cell X held K samples ago. */']
    usedmac = set()
    for nid in em.live:
        n = G.nodes[nid]
        if n.op == 'ld' and names.macro(*n.val):
            usedmac.add((names.macro(*n.val), n.val[1]))
    usedmac |= {(names.macro('KOUT', a), a) for _, a, _, _ in kouts if names.macro('KOUT', a)}
    for m, a in sorted(usedmac, key=lambda x: x[1]):
        out.append('#define %s 0x%04x' % (m, a))
    for x in sorted(k for k in chains if not isinstance(k, str)):
        out.append('static float %s;%s' % (', '.join(names.static(x, kk) for kk in range(1, chains[x] + 1)),
                                           ('   /* part %s */' % owner[x].upper()) if split is not None else ''))
    for x in sorted(k for k in chains if isinstance(k, str)):
        out.append('static float %s;' % x)
    if ctx['live_r']:
        out.append('static float %s;            /* registers read before written: from the sample before */'
                   % ', '.join('%s_r%d' % (name, i) for i in sorted(ctx['live_r'])))
    if ctx['live_f']:
        out.append('static float %s;' % ', '.join('%s_f%d' % (name, c) for c in sorted(ctx['live_f'])))
    if split is not None:
        out.append('/* the parts\' arguments: what the including file\'s KIN / KP / KOUT use (default: none) */')
        out.append('#ifndef KARGS_A\n#define KARGS_A void\n#endif\n#ifndef KARGS_B\n#define KARGS_B void\n#endif')
        out.append('/* handed from part A to part B: s1_export / s1_import slots %d.. (%s) */'
                   % (ctx['export_base'], ' '.join(n.val for n in imps)))
        out.append('#define %s_SLOTS %d' % (name.upper(), len(imps)))
    for p, lo, hi in parts:
        fn = '%s_%s(KARGS_%s)' % (name, p, p.upper()) if p else '%s(void)' % name
        out.append('static inline __attribute__((always_inline)) void %s' % fn)
        out.append('{')
        ids = [nid for nid in em.live if G.nodes[nid].part == p]
        lines = em.body(ids, kouts=[(pos, a, v) for pos, a, v, q in kouts if q == p], lo=lo, hi=hi)
        out += lines
        if order[p]:
            out.append('')
        for fmt, _, v in order[p]:
            out.append('    ' + fmt % em.ref(v))
        out.append('}')
    for v in sorted(set(names.value) - em.used_values):
        ctx['warn'].append('%s: names file: no value %s' % (name, v))
    if G.inexact:
        ctx['warn'].append('%s: %d blends with weight 1 folded to their new value (not 0 / 1 values)' % (name, G.inexact))
    print('%s: %d values named, %d delay cells, %d values A -> B%s; idioms %s' % (
        name, sum(1 for nid in em.live if G.nodes[nid].mat), sum(chains[x] for x in chains if not isinstance(x, str)),
        len(imps), (' (%s)' % ' '.join(n.val for n in imps)) if imps else '',
        ' '.join('%s %d' % kv for kv in sorted(idioms.items())) or 'none'), file=__import__('sys').stderr)
    return out
