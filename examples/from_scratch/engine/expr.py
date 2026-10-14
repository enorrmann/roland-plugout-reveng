"""Expression language of the engine's parameter model.

An output of the model is a DSP write whose value is an expression over the
current parameter values (and the slave index). expr.py evaluates them in
Python (float32, like the engine) and compiles them to the byte code that
engine.c interprets (see eval_expr() there).

    P(0x6EC) / 255          parameter value
    clamp(x, 0, 1), pick(i, [a, b, c]), x >= 3, exp2(x), knots(x, lo, step, [...])
"""
import struct

def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]

# op codes (keep in sync with engine.c)
OPS = ['const', 'param', 'slave', 'add', 'sub', 'mul', 'div', 'min', 'max', 'ge', 'lt',
       'sel', 'exp2', 'pick', 'knots', 'abs', 'floor', 'interp']
OP = {n: i for i, n in enumerate(OPS)}


def lift(x):
    return x if isinstance(x, E) else E('const', float(x))


class E:
    def __init__(self, op, *a):
        self.op, self.a = op, a

    def __add__(s, o): return E('add', s, lift(o))
    def __radd__(s, o): return E('add', lift(o), s)
    def __sub__(s, o): return E('sub', s, lift(o))
    def __rsub__(s, o): return E('sub', lift(o), s)
    def __mul__(s, o): return E('mul', s, lift(o))
    def __rmul__(s, o): return E('mul', lift(o), s)
    def __truediv__(s, o): return E('div', s, lift(o))
    def __rtruediv__(s, o): return E('div', lift(o), s)
    def __ge__(s, o): return E('ge', s, lift(o))
    def __lt__(s, o): return E('lt', s, lift(o))
    def __neg__(s): return E('sub', lift(0.0), s)

    def params(self):
        if self.op == 'param':
            return {self.a[0]}
        out = set()
        for x in self.a:
            if isinstance(x, E):
                out |= x.params()
            elif isinstance(x, (list, tuple)):
                for y in x:
                    if isinstance(y, E):
                        out |= y.params()
        return out

    def ev(self, vals, slave):
        op, a = self.op, self.a
        if op == 'const': return f32(a[0])
        if op == 'param': return f32(float(vals.get(a[0], 0)))
        if op == 'slave': return f32(float(slave))
        if op == 'pick':
            i = int(a[0].ev(vals, slave))
            t = a[1]
            return t[max(0, min(len(t) - 1, i))].ev(vals, slave)
        if op == 'interp':
            x = a[0].ev(vals, slave); xs, ys = a[1], a[2]
            if x <= xs[0]: return f32(ys[0])
            if x >= xs[-1]: return f32(ys[-1])
            k = 0
            while x >= xs[k + 1]: k += 1
            fr = f32(f32(x - xs[k]) / f32(xs[k + 1] - xs[k]))
            return f32(ys[k] + f32(f32(ys[k + 1] - ys[k]) * fr))
        if op == 'knots':
            x = a[0].ev(vals, slave); lo, step, t = a[1], a[2], a[3]
            p = (x - lo) / step
            if p <= 0: return f32(t[0])
            if p >= len(t) - 1: return f32(t[-1])
            k = int(p); fr = f32(p - k)
            return f32(t[k] + (t[k + 1] - t[k]) * fr)
        v = [x.ev(vals, slave) for x in a]
        if op == 'add': return f32(v[0] + v[1])
        if op == 'sub': return f32(v[0] - v[1])
        if op == 'mul': return f32(v[0] * v[1])
        if op == 'div': return f32(v[0] / v[1])
        if op == 'min': return min(v[0], v[1])
        if op == 'max': return max(v[0], v[1])
        if op == 'ge': return 1.0 if v[0] >= v[1] else 0.0
        if op == 'lt': return 1.0 if v[0] < v[1] else 0.0
        if op == 'sel': return v[1] if v[0] != 0 else v[2]
        if op == 'exp2': return f32(2.0 ** v[0])
        if op == 'abs': return abs(v[0])
        if op == 'floor': return float(int(v[0] // 1))
        raise ValueError(op)

    def code(self, consts):
        """Byte code (postfix): list of 16-bit words; constants go to consts."""
        op, a = self.op, self.a
        def c(x):
            x = f32(x)
            if x in consts:
                return consts.index(x)
            consts.append(x); return len(consts) - 1
        if op == 'const': return [OP['const'], c(a[0])]
        if op == 'param': return [OP['param'], a[0]]
        if op == 'slave': return [OP['slave']]
        if op == 'pick':
            out = a[0].code(consts)
            for x in a[1]:
                out += x.code(consts)
            return out + [OP['pick'], len(a[1])]
        if op == 'interp':
            tab = [f32(x) for x in list(a[1]) + list(a[2])]
            first = _find(consts, tab)
            if first < 0:
                first = len(consts)
                consts.extend(tab)
            return a[0].code(consts) + [OP['interp'], first, len(a[1])]
        if op == 'knots':
            first = len(consts)
            for y in a[3]:
                c(y)
            return a[0].code(consts) + [OP['knots'], c(a[1]), c(a[2]), first, len(a[3])]
        out = []
        for x in a:
            out += x.code(consts)
        return out + [OP[op]]


def _find(consts, tab):
    """index of tab as a run in consts (tables are shared between outputs), or -1"""
    n = len(tab)
    for i in range(len(consts) - n + 1):
        if consts[i] == tab[0] and consts[i:i + n] == tab:
            return i
    return -1


def P(i): return E('param', i)
SLAVE = E('slave')
def clamp(x, lo, hi): return E('min', E('max', lift(x), lift(lo)), lift(hi))
def emin(a, b): return E('min', lift(a), lift(b))
def emax(a, b): return E('max', lift(a), lift(b))
def sel(c, a, b): return E('sel', lift(c), lift(a), lift(b))
def exp2(x): return E('exp2', lift(x))
def eabs(x): return E('abs', lift(x))
def floor(x): return E('floor', lift(x))
def pick(i, table): return E('pick', lift(i), [lift(t) for t in table])
def knots(x, lo, step, table): return E('knots', lift(x), float(lo), float(step), [float(t) for t in table])
