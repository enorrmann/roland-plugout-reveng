"""The slave emulator on the C core (dspcore.c): Slave (six DSPs) and Bmc2 (two slaves), with the ARM-side calls.

    import cchain
    s = cchain.Slave(voices.programs('v4'), chain.Model())
    s.p_bank(0, 0x21, 0.5); s.step(); s.iomd.get(0x6020, 0.0)
    s.run(n, (0x6020,))          # n samples in C, returns {word: [values]} (no per-sample Python call)

The shared library is built with cc the first time (and whenever a C source is newer, dspref.py). The instruction
semantics are the C core's (the reference since 2026-10-29), decoded once by dspdec.h through dspcomp.py.

Slave(..., native=True, threads=True): programs translated to native code (dspjit.py, cached in build/jit/) and the
independent DSP groups of a sample on their own threads (for run() / step(n) with n >= 64). Both give the same bits
as the interpreter (native=False, threads=False).
"""
import ctypes as C
import os

import chain
import dspcomp
import dspjit
import dspref

HERE = os.path.dirname(os.path.abspath(__file__))
lib = dspref.lib                                   # the core library (dspref.py builds and loads it)


class Words:
    """dict-like view of a C array of words base .. base + n (IOMD, bank 6)"""

    def __init__(self, ptr, base, n, owner):
        self.a = C.cast(ptr, C.POINTER(C.c_double * n)).contents
        self.base, self.n, self._owner = base, n, owner

    def _i(self, addr):
        i = addr - self.base
        return i if 0 <= i < self.n else None

    def get(self, addr, default=0.0):
        i = self._i(addr)
        return default if i is None else self.a[i]

    def __getitem__(self, addr):
        i = self._i(addr)
        if i is None:
            raise KeyError(addr)
        return self.a[i]

    def __setitem__(self, addr, v):
        i = self._i(addr)
        if i is None:
            raise KeyError(f'{addr:#x} outside {self.base:#x}-{self.base + self.n - 1:#x}')
        self.a[i] = float(v)

    def update(self, d):
        for k, v in d.items():
            self[k] = v

    def items(self):
        return [(self.base + i, v) for i, v in enumerate(self.a) if v]


def _flag_bits(s):
    return sum(1 << i for i, ch in enumerate(s) if ch == '1')


def _flag_str(bits):
    return ''.join('1' if bits >> i & 1 else '0' for i in range(32))


class DspView:
    """registers, flags and memory of one DSP (read and write, for tests)"""

    def __init__(self, slave, n):
        self.s, self.n = slave, n
        self.bank5 = C.cast(lib().sl_bank5(slave.h, n), C.POINTER(C.c_double * 0x400)).contents
        self.params = Words(lib().sl_params_dsp(slave.h, n), 0xC000, 0x2000, slave)   # Model.own_params
        self.bank0 = C.cast(lib().sl_bank0(slave.h, n), C.POINTER(C.c_double * chain.RING)).contents
        self.bank1 = C.cast(lib().sl_bank1(slave.h, n), C.POINTER(C.c_double * chain.RING)).contents

    def state(self):
        """(regs, flags string, pending {group: '4 bits'}, hidden {4..7})"""
        r, f, pm, pv, h = (C.c_double * 8)(), C.c_uint32(), C.c_uint32(), C.c_uint32(), (C.c_double * 8)()
        lib().sl_get_state(self.s.h, self.n, r, C.byref(f), C.byref(pm), C.byref(pv), h)
        bits = _flag_str(pv.value)
        pending = {g: bits[4 * g:4 * g + 4] for g in range(8) if pm.value >> 4 * g & 0xF}
        return list(r), _flag_str(f.value), pending, {k: h[k] for k in range(4, 8)}

    def set_state(self, regs=None, flags=None, pending=None, hidden=None):
        r0, f0, p0, h0 = self.state()
        regs = r0 if regs is None else regs
        flags = f0 if flags is None else flags
        pending = p0 if pending is None else pending
        hidden = h0 if hidden is None else hidden
        pm = pv = 0
        for g, b4 in pending.items():
            pm |= 0xF << 4 * g
            pv |= _flag_bits(b4) << 4 * g
        h = (C.c_double * 8)(*[hidden.get(k, 0.0) for k in range(8)])
        lib().sl_set_state(self.s.h, self.n, (C.c_double * 8)(*regs), _flag_bits(flags), pm, pv, h)

    @property
    def regs(self):
        return self.state()[0]

    def _mxm(self, f):
        a, v = (C.c_int * 0x2000)(), (C.c_double * 0x2000)()
        n = f(self.s.h, self.n, a, v, 0x2000)
        return {a[k]: v[k] for k in range(n)}

    def mxm_out(self):
        """MXM writes of the running sample (empty between samples)"""
        return self._mxm(lib().sl_mxm_out)

    def mxm_in(self):
        """latched MXM inputs"""
        return self._mxm(lib().sl_mxm_in)


class Slave:
    """one BMC: six DSPs. programs = {dsp: (main chain.Program, [slot chain.Programs])}"""

    TRACE = C.CFUNCTYPE(None, C.c_int, C.c_int, C.POINTER(C.c_double), C.c_uint32, C.c_uint32, C.c_uint32)

    def __init__(self, programs, model=None, index=0, native=True, threads=True, trace=None):
        """trace(dsp, instruction, regs, flags, pending mask, pending values) after every instruction (debugging;
        native code is then built with the calls)"""
        self.m = m = model or chain.Model()
        self.index = index
        L = lib()
        order = (C.c_int * 6)(*m.order)
        sh = [x for p in m.shared1 for x in p]
        s5 = [x for p in getattr(m, 'shared5', ()) for x in p]
        op = list(getattr(m, 'own_params', ()))
        self.h = L.sl_new(int(m.ring1), m.mxm_shift, m.mxm_delay, int(m.dp_pairs), m.dp_delay, order,
                          len(m.order), (C.c_int * max(1, len(sh)))(*sh), len(m.shared1),
                          (C.c_int * max(1, len(s5)))(*s5), len(s5) // 2, (C.c_int * max(1, len(op)))(*op), len(op))
        if m.edm:                                  # the delay line (DSP_HARDWARE.md §8.4)
            L.sl_set_edm(self.h, *m.edm)
        self._ids = {}
        per_dsp = {}
        for n, (main, slots) in programs.items():
            per_dsp[n] = [main] + list(slots) if m.main_first else list(slots) + [main]
            ids = [self._prog_id(p) for p in per_dsp[n]]
            L.sl_set_progs(self.h, n, (C.c_int * len(ids))(*ids), len(ids))
        self.native_lib = None
        if native and self._ids:
            fns, self.native_lib = dspjit.build([p for _, p in self._ids.values()], trace=trace is not None)
            for (pid, _), f in zip(self._ids.values(), fns):
                L.sl_set_native(self.h, pid, f)
        if trace is not None:
            self._trace = Slave.TRACE(lambda dsp, i, r, f, pm, pv: trace(dsp, i, r[:8], f, pm, pv))
            L.sl_set_trace(self.h, C.cast(self._trace, C.c_void_p))
        self.groups = dspjit.groups(per_dsp, m) if threads else None
        self.pipeline = pipeline_plan(per_dsp, m) if threads else None
        if self.pipeline:
            a, b, wa, wb = self.pipeline
            L.sl_set_pipeline(self.h, a, b, (C.c_int * max(1, len(wa)))(*wa), len(wa),
                              (C.c_int * max(1, len(wb)))(*wb), len(wb))
        if self.groups and len(self.groups) > 1:
            flat = [n for g in self.groups for n in g]
            L.sl_set_groups(self.h, (C.c_int * len(flat))(*flat),
                            (C.c_int * len(self.groups))(*[len(g) for g in self.groups]), len(self.groups))
        self.iomd = Words(L.sl_iomd(self.h), 0x6000, 0x2000, self)
        self.params = Words(L.sl_params(self.h), 0xC000, 0x2000, self)
        self.d = [DspView(self, n) for n in range(6)]
        flags = dspref.CONTEXT_BITS[dspref.CONTEXT] + dspref.HIGH_BITS
        for d in self.d:                       # hidden = dspref.HIDDEN at construction, default flags
            d.set_state(regs=[0.0] * 8, flags=flags, pending={}, hidden=dict(dspref.HIDDEN))

    def _prog_id(self, p):
        if id(p) not in self._ids:
            self._ids[id(p)] = (lib().sl_add_prog(self.h, *dspcomp.compile_program(p)), p)
        return self._ids[id(p)][0]

    def __del__(self):
        if getattr(self, 'h', None) and dspref._lib is not None:
            dspref._lib.sl_free(self.h)
            self.h = None

    @property
    def t(self):
        return lib().sl_t(self.h)

    @property
    def ninstr(self):
        return lib().sl_ninstr(self.h)

    def ninstr_dsp(self, n):
        return lib().sl_ninstr_dsp(self.h, n)

    # ---------------- ARM side ----------------
    def p_bank(self, dsp, ident, value):
        """OS +0xB0: bank 5 word `ident` of DSP `dsp` (engine.c p_bank)"""
        self.d[dsp].bank5[ident & 0x3FF] = float(value)

    # ---------------- samples ----------------
    def step(self, n=1):
        if lib().sl_step(self.h, n, None, 0, None):
            raise RuntimeError(lib().sl_error(self.h).decode())

    def run(self, n, words=chain.AUDIBLE):
        """n samples; {word: [value after each sample]}: word = an IOMD address, or (dsp, address) = that word as the
        DSP reads it (its bank 5, ...)"""
        keys = list(words)
        w = [(k[0] + 1) << 16 | k[1] if isinstance(k, tuple) else k for k in keys]
        rec = (C.c_double * (n * len(w)))()
        if lib().sl_step(self.h, n, (C.c_int * max(1, len(w)))(*w), len(w), rec):
            raise RuntimeError(lib().sl_error(self.h).decode())
        return {a: list(rec[k::len(w)]) for k, a in enumerate(keys)}


def access_sets(progs):
    """static memory accesses of a DSP's programs: {'r': set, 'w': set} of addresses (mode fe transfers), and
    'indexed' = True if an indexed bank-5 transfer (XdaY / Xd0Y ...) appears (its address is not static)"""
    out = {'r': set(), 'w': set(), 'indexed': False}
    for p in progs:
        for x in dspcomp.decode_program(p):
            for o in (x[0] if x else []):
                if o['kind'] == dspcomp.K_XMEM:
                    out['r' if o['a'] < 8 else 'w'].add(o['b'])
                elif o['kind'] == dspcomp.K_XB5:
                    out['indexed'] = True
    return out


def pipeline_plan(per_dsp, m):
    """(a, b, bank-5 offsets a writes, offsets the others write) when the shared-bank-5 pair (a, b) of the model can
    run as a two-stage pipeline (a one sample ahead on its own thread) with the same bits; else None. Conditions:
    one Model.shared5 pair, a before b in the order, MXM latched; a touches no IOMD, MXM, pair window (A400-A7FF)
    or shared bank 1; no indexed bank-5 transfer in a or b; a reads and writes no bank-5 word the others write."""
    pairs = list(getattr(m, 'shared5', ()))
    if len(pairs) != 1 or not m.mxm_delay:
        return None
    a, b = pairs[0]
    order = list(m.order)
    if a not in order or b not in order or order.index(a) > order.index(b):
        return None
    if any(a in p for p in m.shared1) or len([n for n in order if per_dsp.get(n)]) < 2:
        return None
    acc = {n: access_sets(per_dsp.get(n, [])) for n in order}
    A = acc[a]
    if A['indexed'] or acc[b]['indexed']:
        return None
    for addr in A['r'] | A['w']:
        bank = addr >> 13
        if bank in (3, 4) or (bank == 5 and addr & 0x7FF >= 0x400):
            return None
    b5 = lambda s: {x & 0x3FF for x in s if x >> 13 == 5 and x & 0x7FF < 0x400}  # noqa: E731
    wa = b5(A['w'])
    wb = set().union(*[b5(acc[n]['w']) for n in order if n != a])
    if (b5(A['r']) | wa) & wb:
        return None
    return a, b, sorted(wa), sorted(wb)


class Bmc2(chain.Bmc2):
    """two C slaves; Model.link as in chain.Bmc2"""

    def __init__(self, programs, model=None, **kw):
        self.m = model or chain.Model()
        self.s = [Slave(programs, self.m, 0, **kw), Slave(programs, self.m, 1, **kw)]
