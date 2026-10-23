"""Native translation of DSP programs for the C core (docs/EMULATOR.md §3.6 point 2), and the thread groups.

Every program becomes one C function: the instruction words are constants, jumps are gotos, the conditions are
tests of a local flags word, and the units call the inline functions of dspcore.h that the interpreter uses (with
constant words gcc folds them to the few operations each unit really does). The translation adds nothing to the
semantics; it only drops work whose result is known:

* quant(): dspisa quantises all eight registers after every A/B/M unit. quant is idempotent, so a register needs it
  only if it may hold an unquantised value: one written by a unit (superset by unit type), or loaded since the last
  unconditional unit, or anything at a jump target / program start;
* the result and flags of a unit are kept only when an X9bY of the same instruction reads them.

The functions of one program set go into one shared library, built by cc and cached by the hash of its source in
emu/build/jit/. test_core.py compares the translation with the interpreter and with chain.py.

groups(): DSPs that share an IOMD word (one writes a word another reads or writes, static addresses: a transfer of
mode fe; the indexed modes only reach banks 0 and 5) or bank 1 (Model.shared1) must run on one thread, in model
order; the other DSPs of a sample are independent (MXM and the pair window are latched).
"""
import ctypes as C
import hashlib
import os
import shutil
import subprocess

import dspcomp
from dspcomp import K_A, K_B, K_M, K_J, K_HID, K_X28, K_XB5, K_XFLAG, K_XMEM

HERE = os.path.dirname(os.path.abspath(__file__))
JIT_DIR = os.path.join(HERE, 'build', 'jit')
# compiler: DSP_CC=gcc|clang|<path> (default: clang if installed, else cc). -ffp-contract=off: an FMA would change the
# bits. (The old -fno-gcse -fno-peephole2 hid a NaN-sign difference, not a GCC bug: quant() now makes every NaN
# canonical, EMULATOR.md §3.6.1.)
CC = os.environ.get('DSP_CC') or ('clang' if shutil.which('clang') else 'cc')
CFLAGS = ['-O2', '-ffp-contract=off', '-fno-math-errno', '-shared', '-fPIC', '-w']
LDFLAGS = []
ALL = frozenset(range(8))
_loaded = {}


def lit(v):
    """exact C literal of a double"""
    return v.hex() if v == v and abs(v) != float('inf') else ('NAN' if v != v else ('-' if v < 0 else '') + 'INFINITY')


def writes(o):
    """registers a unit may write, from its word (the branches of a_op / b_op / sfu / mac in dspcore.h)"""
    w, d = o['w'], o['w'] >> 12
    op, x, mode, y = (w >> 9) & 7, (w >> 6) & 7, (w >> 4) & 3, w & 15
    if o['kind'] == K_A:
        if d >= 8:
            return {d - 8}
        if op == 0:
            return {d, min(d + 1, 7), y & 7} if x == 2 else ({d} if x in (1, 4, 5, 6, 7) else set())
        return {d}
    if o['kind'] == K_B:
        b = (w >> 8) & 15
        if d >= 8 and b == 0xF:
            b76, s = (w >> 6) & 3, d - 8
            if b76 == 1:
                return {s}
            if b76:
                return set()
            if mode == 0:
                return {s, y & 7, (s + 1) & 7} if y & 8 else {s, (s + 1) & 7}
            if mode == 1 and not y & 8:
                return {y & 7, (s + 1) & 7, 7}
            if mode in (1, 2) and not (mode == 2 and y & 8):
                return {s, y & 7}
            return {s}
        if d >= 8 and b == 0xE:
            return {d & 7} if (w >> 6) & 3 in (1, 3) else set()
        if d >> 3 == 1 and b & 0xE == 0xC and not mode:
            return {d & 7, y & 7}
        return {d & 7}
    if o['kind'] == K_M:
        return {d & 7} if (w >> 9) & 7 >= 2 and (o['w2'] >> 9) & 7 != 3 else set()
    if o['kind'] in (K_X28, K_XB5, K_XMEM) and o['a'] < 8:
        return {o['a']}
    return set()


def exact_dest(o, dirty):
    """the destination register of a unit-B op whose result is one of its inputs, its negation, its magnitude or a
    clamp to +-1 (dspisa.b_op: min / max (+ -X forms), |X|, |Y|, Y, f4Y copy, ph_abs / ph_clamp / ph_neg, r = Y),
    when no register it reads may be off the quant grid (not in `dirty`) and its immediate is on the grid: then the
    result is on the grid and its quant() is a no-op. None otherwise."""
    if o['kind'] != K_B:
        return None
    w = o['w']
    d, b, mode, y = w >> 12, (w >> 8) & 15, (w >> 4) & 3, w & 15
    if d < 8:
        return None
    # no negating form (-X, -rY, ph_neg): a canonical NaN negated is not canonical (quant() in dspcore.h)
    neg_y = not mode and y & 8
    if b == 0xF:
        exact = (w >> 6) & 3 == 1 and mode == 0           # f4Y: rd = r(Y & 7), sign bit ignored
        mode = 0
    elif b == 0xE:
        sub = (w >> 6) & 3
        exact = (sub == 1 and not neg_y) or (sub == 3 and (mode, y & 8) in ((1, 0), (1, 8)))
        if sub == 3:
            mode = 0                                      # the phase forms read r(Y & 7), no immediate
    else:
        exact = b & 0xE in (0x0, 0x4, 0x8, 0xA, 0xE) and not (neg_y and b & 0xE != 0xA)  # min / max / |X| / |Y| / Y
    if not exact:
        return None
    reads = {(w >> 6) & 7, ((w >> 6) & 7) | 4, d & 7} | ({y & 7} if not mode else set())
    if reads & dirty:
        return None
    if mode and o['has'] and dspcomp.dspisa.quant(o['imm'][0]) != o['imm'][0]:
        return None
    return d & 7


def translate(name, prog, decoded, trace=False):
    """C source of one program: int name(Slave *s, Dsp *d); trace = call s->trace after every instruction"""
    targets = {o['a'] for x in decoded if x for o in x[0] if o['kind'] == K_J and o['a'] >= 0}
    L = [f'int {name}(Slave *s, Dsp *d)', '{',
         '    double r[8], o[8];', '    memcpy(r, d->regs, sizeof r);',
         '    uint32_t fl = d->flags, pm = d->pmask, pv = d->pval;', '    int64_t n = 0;', '    int ret = 0;',
         '    (void)s;']
    dirty = set(ALL)
    for i, x in enumerate(decoded):
        if i in targets:
            dirty = set(ALL)
        L.append(f'I{i}: {{')
        if x is None:
            L += ['    ret = -2; goto END;', '}']
            continue
        ops, conds = x
        units = {o['unit']: k for k, o in enumerate(ops) if o['kind'] in (K_A, K_B, K_M)}
        flagged = {o['b'] for o in ops if o['kind'] == K_XFLAG and o['a'] and o['b'] in units}
        has_flag = any(o['kind'] == K_XFLAG and o['a'] and o['b'] in units for o in ops)
        jumps = [o['a'] for o in ops if o['kind'] == K_J]
        L.append('    memcpy(o, r, sizeof o);')
        dirty_in = set(dirty)          # every unit reads r = the registers at the instruction start (exact_dest)
        if has_flag:
            L.append('    uint32_t um = 0, uv = 0;')
        if jumps:
            L.append('    int jt = -2;')
        for u in sorted(flagged):
            L.append(f'    int ex{u} = 0; double rv{u} = 0; int rz{u} = 0;')
        for j, (cc, neg, _, _) in enumerate(conds):
            L.append(f'    const int c{j} = (int)((fl >> {cc}) & 1) != {neg};')
        for k, o in enumerate(ops):
            sk = [f'c{j}' for j, c in enumerate(conds) if c[2] == k] + \
                 [f'!c{j}' for j, c in enumerate(conds) if c[3] == k]
            guard = f'if (!({" || ".join(sk)})) ' if sk else ''
            body = []
            kind, w = o['kind'], o['w']
            if kind in (K_A, K_B, K_M):
                body.append('Res res = {0, 0, 0};')
                if kind == K_A:
                    body.append(f'if (a_op({w:#x}, {lit(o["imm"][0])}, d, r, o, &res)) {{ ret = -1; goto END; }}')
                elif kind == K_B:
                    body.append(f'b_op({w:#x}, {lit(o["imm"][0])}, d, r, o, &res);')
                else:
                    im = o['imm']
                    body.append(f'mac({w:#x}, {o["w2"]:#x}, {lit(im[0])}, {lit(im[1])}, {lit(im[2])}, r, o);')
                q = dirty | writes(o)
                ex = exact_dest(o, dirty | dirty_in)
                if ex is not None:
                    q = q - {ex}
                body += [f'o[{r}] = quant(o[{r}]);' for r in sorted(q)]
                u = o['unit']
                if u in flagged:
                    dest = (w >> 12) & 7
                    body.append(f'ex{u} = 1; rv{u} = res.set ? res.v : o[{dest}]; '
                                f'rz{u} = res.set ? res.z : o[{dest}] == 0;')
                body.append('(void)res;')
                dirty = dirty if sk else set()
            elif kind == K_J:
                body.append(f'jt = {o["a"]};')
            elif kind == K_HID:
                body.append(f'd->hidden[{o["a"]}] = ' + (f'pymod(r[{o["c"] & 7}], 512);' if o['b'] == 0
                                                         else f'{lit(o["imm"][0])};'))
            elif kind in (K_X28, K_XB5, K_XMEM):
                if kind == K_X28:
                    addr = f'(((int64_t)(d->hidden[6] * 16384) + {o["b"]}) & 0x7FF)'
                elif kind == K_XB5 and o['c']:
                    addr = f'(0xA000 + (int64_t)(d->hidden[{o["c"]}] * 16384) + {o["b"]})'
                elif kind == K_XB5:
                    addr = f'{0xA000 + o["b"]:#x}'
                else:
                    addr = f'{o["b"]:#x}'
                if o['a'] < 8:
                    body.append(f'o[{o["a"]}] = rd(s, d, {addr});')
                    dirty |= {o['a']}
                else:
                    body.append(f'wr(s, d, {addr}, r[{o["a"] - 8}]);')
            elif kind == K_XFLAG:
                if o['a'] and o['b'] in units:
                    sh = 4 * o['a']
                    m = (0xF << sh) & 0xFFFFFFFF
                    body.append(f'if (ex{o["b"]}) {{ um |= {m:#x}u; '
                                f'uv = (uv & ~{m:#x}u) | flag_bits(rv{o["b"]}, rz{o["b"]}) << {sh}; }}')
            if body:
                L.append(f'    {guard}{{ ' + ' '.join(body) + ' }')
        L.append('    fl = (fl & ~pm) | pv;')
        L.append('    pm = um; pv = uv;' if has_flag else '    pm = 0; pv = 0;')
        L += ['    memcpy(r, o, sizeof r);', '    n++;']
        if trace:
            L.append(f'    if (s->trace) s->trace(d->n, {i}, r, fl, pm, pv);')
        if jumps:
            cases = ' '.join(f'case {t}: goto I{t};' for t in sorted(set(jumps)) if t >= 0)
            L.append(f'    if (jt != -2) {{ if (n > MAXSTEPS) {{ ret = -4; goto END; }} '
                     f'switch (jt) {{ {cases} default: ret = -3; goto END; }} }}')
        L.append('}')
    L += ['END:', '    memcpy(d->regs, r, sizeof r);', '    d->flags = fl; d->pmask = pm; d->pval = pv;',
          '    d->ninstr += n;', '    return ret;', '}', '']
    return '\n'.join(L)


def _object(prog, hdr, trace=False):
    """compile one program to build/jit/p_<hash>.o (cached); returns (function name, path, Popen or None)"""
    src = translate('PROG', prog, dspcomp.decode_program(prog), trace)
    name = 'p_' + hashlib.sha1((hdr + src + ' '.join([CC] + CFLAGS)).encode()).hexdigest()[:16]
    obj = os.path.join(JIT_DIR, name + '.o')
    if os.path.exists(obj):
        return name, obj, None
    c = os.path.join(JIT_DIR, name + '.c')
    tmp = f'.{os.getpid()}.tmp'                      # several processes may build the same object at once
    with open(c + tmp, 'w') as f:
        f.write('#include "dspcore.h"\n\n' + src.replace('int PROG(', f'int {name}(', 1))
    os.replace(c + tmp, c)
    flags = [f for f in CFLAGS if f != '-shared']
    return name, obj, subprocess.Popen([CC, *flags, '-c', '-I', HERE, '-o', obj + tmp, c])


def build(progs, trace=False):
    """native functions for a list of chain.Programs: [ctypes function address]. Each program is one object file,
    compiled in parallel and cached by the hash of its source; the set is linked into one library (cached too)."""
    hdr = open(os.path.join(HERE, 'dspcore.h')).read() + open(os.path.join(HERE, 'qmask.h')).read()
    os.makedirs(JIT_DIR, exist_ok=True)
    built, running = [], []

    def reap(k):
        while len(running) > k:
            obj, pr = running.pop(0)
            if pr.wait():
                raise RuntimeError(f'cc failed for {obj}')
            os.replace(obj + f'.{os.getpid()}.tmp', obj)

    for p in progs:                                      # at most one cc per core
        reap(os.cpu_count() or 4)
        name, obj, pr = _object(p, hdr, trace)
        built.append((name, obj))
        if pr is not None:
            running.append((obj, pr))
    reap(0)
    objs = sorted({obj for _, obj in built})
    key = hashlib.sha1(' '.join(objs).encode()).hexdigest()[:16]
    if key not in _loaded:
        so = os.path.join(JIT_DIR, f'set_{key}.so')
        if not os.path.exists(so):
            tmp = so + f'.{os.getpid()}.tmp'
            subprocess.check_call([CC, '-shared', *LDFLAGS, '-o', tmp, *objs, '-lm'])
            os.replace(tmp, so)
        _loaded[key] = C.CDLL(so)
    lib = _loaded[key]
    return [C.cast(getattr(lib, name), C.c_void_p).value for name, _ in built], lib


def iomd_refs(progs):
    """(reads, writes): IOMD words of a DSP's programs (static transfer addresses)"""
    rd, wr = set(), set()
    for p in progs:
        for x in dspcomp.decode_program(p):
            for o in (x[0] if x else []):
                if o['kind'] == K_XMEM and o['b'] >> 13 == 3:
                    (rd if o['a'] < 8 else wr).add(o['b'])
    return rd, wr


def groups(dsp_progs, model):
    """thread groups for a slave: {dsp: [Program]} -> [[dsp, ...]] in model order, or None if the model needs one
    thread (same-sample MXM, immediate pair window)"""
    if not model.mxm_delay or (model.dp_pairs and not model.dp_delay):
        return None
    order = [n for n in model.order]
    refs = {n: iomd_refs(dsp_progs.get(n, [])) for n in order}
    parent = {n: n for n in order}

    def find(a):
        while parent[a] != a:
            a = parent[a]
        return a

    for a in order:
        for b in order:
            if a != b and refs[a][1] & (refs[b][0] | refs[b][1]):
                parent[find(a)] = find(b)
    for a, b in tuple(model.shared1) + tuple(getattr(model, 'shared5', ())):
        if a in parent and b in parent:
            parent[find(a)] = find(b)
    out = {}
    for n in order:
        out.setdefault(find(n), []).append(n)
    return list(out.values())
