"""The DSP reference from Python: a thin ctypes binding of the C core (dspdec.h decoder, dspcore.h / dspcore.c
semantics). It has no semantics of its own; it replaces re/dspisa.py (archived 2026-10-29, archive/dspisa.py).

    import dspref
    m = dspref.Machine()                     # one DSP, flat memory (the tools' simulator)
    m.regs = [0.5, 0.25, 0, 0, 0, 0, 0, 0]
    m.run([0x0902, 0x2C01])                  # one instruction; m.jump = offset of a jump that ran, else None
    m.mem[0xA200]                            # memory (a dict-like view: get / items / update / clear)
    pid = m.add(program)                     # a program (list of instructions), decoded once
    m.run_program(pid)                       # one pass, jumps followed
    dspref.layout(ins).t                     # template letters, slots, units, refs (tools that parse code)
    dspref.quant(v), dspref.raw_value(w)

The shared library is built with cc the first time (and whenever a C source is newer); emu/cchain.py uses the same.
"""
import ctypes as C
import os
import shutil
import subprocess

HERE = os.path.dirname(os.path.abspath(__file__))
RE = os.path.join(HERE, '..', 're')                     # dsp_templates.txt, dsp_units_ab.txt
SRCS = [os.path.join(HERE, f) for f in ('dspcore.c', 'dspcore.h', 'dspdec.h', 'qmask.h')]
# compiler: DSP_CC=gcc|clang|<path> (default: clang if installed, else cc). -ffp-contract=off: an FMA would change the
# bits. (The old -fno-gcse -fno-peephole2 hid a NaN-sign difference, not a GCC bug: quant() makes every NaN
# canonical, EMULATOR.md §3.6.1.)
CC = os.environ.get('DSP_CC') or ('clang' if shutil.which('clang') else 'cc')
CFLAGS = ['-O2', '-ffp-contract=off', '-fno-math-errno', '-shared', '-fPIC', '-w']
LIB = os.path.join(HERE, 'build', f'dspcore-{os.path.basename(CC)}.so')   # one per compiler

# condition bits (condition 0 first) of a context, measured; conditions 16-31 as first read on the master
CONTEXT_BITS = {'dco': '0011011001100110', 'master_dsp3': '0111111011101110', 'master_dsp4_efdc': '0111101111100111'}
HIGH_BITS = '1110111011111111'
CONTEXT = 'dco'
# hidden registers h4..h7 at start (master DSP4): h4 = delay base, h5 = tap-result address, h6 / h7 = indexes
HIDDEN = {4: 288.0, 5: 0x105 / 16384, 6: 0.0, 7: 0.0}
FLAG_GROUP = {0x9: 1, 0xA: 2, 0xB: 3, 0xC: 4, 0xD: 5}
DMAX = 64
_lib = None


class DLayout(C.Structure):
    _fields_ = [('cls', C.c_int), ('n', C.c_int), ('t', C.c_char * (DMAX + 1)), ('body', C.c_uint16 * DMAX),
                ('slots', C.c_uint16 * DMAX), ('nslots', C.c_int), ('imm', C.c_uint16 * (DMAX + 64)),
                ('nimm', C.c_int), ('refs', C.c_uint64), ('nlist', C.c_uint16 * DMAX), ('nn', C.c_int),
                ('units', C.c_int * DMAX), ('nunits', C.c_int), ('mac_off', C.c_int)]


def lib():
    global _lib
    if _lib is None:
        if not os.path.exists(LIB) or os.path.getmtime(LIB) < max(map(os.path.getmtime, SRCS)):
            os.makedirs(os.path.dirname(LIB), exist_ok=True)
            tmp = LIB + f'.{os.getpid()}.tmp'           # several processes may build it at once
            subprocess.check_call([CC, *CFLAGS, '-pthread', '-o', tmp, SRCS[0], '-lm'])
            os.replace(tmp, LIB)
        L = C.CDLL(LIB)
        P, I, D, U = C.c_void_p, C.c_int, C.POINTER(C.c_double), C.c_uint32
        W = C.POINTER(C.c_uint16)
        L.sl_new.restype = P
        L.sl_new.argtypes = [I, I, I, I, I, C.POINTER(I), I, C.POINTER(I), I, C.POINTER(I), I, C.POINTER(I), I]
        L.sl_free.argtypes = [P]
        L.sl_add_prog.argtypes = [P, I, C.c_void_p, I, C.c_void_p, I, C.c_void_p]
        L.sl_set_progs.argtypes = [P, I, C.POINTER(I), I]
        L.sl_get_state.argtypes = [P, I, D, C.POINTER(U), C.POINTER(U), C.POINTER(U), D]
        L.sl_set_state.argtypes = [P, I, D, U, U, U, D]
        L.sl_set_native.argtypes = [P, I, C.c_void_p]
        L.sl_set_trace.argtypes = [P, C.c_void_p]
        L.sl_set_groups.argtypes = [P, C.POINTER(I), C.POINTER(I), I]
        L.sl_ngroups.argtypes = [P]
        L.sl_ninstr_dsp.restype = C.c_int64
        L.sl_ninstr_dsp.argtypes = [P, I]
        L.sl_quant.restype = C.c_double
        L.sl_quant.argtypes = [C.c_double]
        L.sl_quant_check.restype = C.c_int64
        L.sl_quant_check.argtypes = [C.c_int64, C.c_uint64, D]
        L.sl_pymod_check.restype = C.c_int64
        L.sl_pymod_check.argtypes = [C.c_int64, C.c_uint64, D]
        for f in ('sl_iomd', 'sl_params'):
            getattr(L, f).restype = D
            getattr(L, f).argtypes = [P]
        L.sl_params_dsp.restype = D
        L.sl_params_dsp.argtypes = [P, I]
        for f in ('sl_bank5', 'sl_bank0', 'sl_bank1'):
            getattr(L, f).restype = D
            getattr(L, f).argtypes = [P, I]
        L.sl_t.restype = L.sl_ninstr.restype = C.c_int64
        L.sl_t.argtypes = L.sl_ninstr.argtypes = [P]
        L.sl_error.restype = C.c_char_p
        L.sl_error.argtypes = [P]
        L.sl_mxm_out.argtypes = L.sl_mxm_in.argtypes = [P, I, C.POINTER(I), D, I]
        L.sl_step.argtypes = [P, I, C.POINTER(I), I, D]
        L.sl_set_pipeline.argtypes = [P, I, I, C.POINTER(I), I, C.POINTER(I), I]
        # decoder and machine
        L.dsp_load_templates.argtypes = [C.c_char_p]
        L.dsp_raw_value.restype = C.c_double
        L.dsp_raw_value.argtypes = [U]
        L.dsp_decode.argtypes = [W, I, C.c_void_p, I, C.POINTER(I), C.c_void_p, I, C.POINTER(I),
                                 C.POINTER(C.c_int32)]
        L.dsp_layout.argtypes = [W, I, C.POINTER(DLayout)]
        L.dsp_machine_new.restype = P
        L.dsp_machine_mem.restype = D
        L.dsp_machine_mem.argtypes = [P]
        L.dsp_machine_set_access.argtypes = [P, C.c_void_p]
        L.dsp_machine_add.argtypes = [P, W, I, C.POINTER(I), I]
        L.dsp_machine_run.argtypes = [P, I]
        L.dsp_exec1.argtypes = [P, W, I, C.POINTER(I), C.POINTER(I)]
        L.sl_set_edm.argtypes = [P, I, C.c_double, I, C.c_int64]
        L.dsp_machine_end_sample.argtypes = [P]
        L.dsp_check.argtypes = [W, I, I, C.c_char_p, I]
        L.dsp_hidden_use.argtypes = [W, I, C.POINTER(I), C.POINTER(I)]
        if L.dsp_load_templates(RE.encode()) < 0:
            raise FileNotFoundError(os.path.join(RE, 'dsp_templates.txt'))
        _lib = L
    return _lib


def words_arg(ins):
    return (C.c_uint16 * max(1, len(ins)))(*ins)


def quant(v):
    """RAW encoding of an op result (DSP_HARDWARE.md §3)"""
    return lib().sl_quant(float(v))


def raw_value(w):
    """value of a RAW word"""
    return lib().dsp_raw_value(w & 0xFFFFFFFF)


class Layout:
    """what the words alone determine (dspdec.h DLayout): t (letters, after the slot / jump rewriting), body,
    slots, units (body positions), used (slot numbers that are references), imm_words, n_list, cls"""

    def __init__(self, ins):
        L = DLayout()
        e = lib().dsp_layout(words_arg(ins), len(ins), C.byref(L))
        if e == -1:
            raise ValueError(f'unknown template for class {ins[0] >> 5:#x} len {len(ins)}')
        self.cls, self.t = L.cls, L.t.decode()
        self.body = list(L.body[:L.n])
        self.slots = list(L.slots[:L.nslots])
        self.imm_words = list(L.imm[:L.nimm])
        self.n_list = list(L.nlist[:L.nn])
        self.units = list(L.units[:L.nunits])
        # the reference slots of transfers in mode fe
        self.used = {(w & 15) >> 1 for w, k in zip(self.body, self.t) if k == 'T' and (w >> 4) & 0xFF == 0xFE}
        self.mac_off = bool(L.mac_off)


_LAYOUTS = {}


def layout(ins):
    """Layout(ins), cached by the words"""
    key = tuple(ins)
    L = _LAYOUTS.get(key)
    if L is None:
        L = _LAYOUTS[key] = Layout(list(ins))
    return L


def template(ins):
    """the template letters of an instruction, or None (undecodable)"""
    try:
        return layout(ins).t
    except ValueError:
        return None


def check(ins, noop_word=0xFF70):
    """None when every unit of the instruction has a measured effect, else the reason (dspdec.h dsp_check)"""
    why = C.create_string_buffer(200)
    return why.value.decode() if lib().dsp_check(words_arg(ins), len(ins), noop_word, why, 200) else None


def hidden_use(ins):
    """(hidden registers read, hidden registers loaded) by an instruction, as sets of 4..7"""
    r, ld = C.c_int(), C.c_int()
    lib().dsp_hidden_use(words_arg(ins), len(ins), C.byref(r), C.byref(ld))
    return {k for k in range(8) if r.value >> k & 1}, {k for k in range(8) if ld.value >> k & 1}


def _bits(s):
    return sum(1 << i for i, c in enumerate(s) if c == '1')


def _str(b):
    return ''.join('1' if b >> i & 1 else '0' for i in range(32))


class MemView:
    """dict-like view of the machine's flat memory (0..0xFFFF; bank 5 A000-BFFF folds to its 1024 words)"""

    def __init__(self, ptr, owner):
        self.a = C.cast(ptr, C.POINTER(C.c_double * 0x10000)).contents
        self._owner = owner

    @staticmethod
    def _k(a):
        return 0xA000 | (a & 0x3FF) if 0xA000 <= a < 0xC000 else a

    def get(self, a, default=0.0):
        a = self._k(a)
        return self.a[a] if 0 <= a < 0x10000 else default

    def __getitem__(self, a):
        return self.get(a)

    def __setitem__(self, a, v):
        a = self._k(a)
        if not 0 <= a < 0x10000:
            raise KeyError(f'{a:#x} outside the flat memory')
        self.a[a] = float(v)

    def __contains__(self, a):
        """every word of the flat memory exists (a word never written reads 0)"""
        a = self._k(a)
        return 0 <= a < 0x10000

    def update(self, d):
        for k, v in (d.items() if hasattr(d, 'items') else d):
            self[k] = v

    def items(self):
        """the nonzero words (a word never written reads 0, as one written with 0)"""
        return [(i, v) for i, v in enumerate(self.a) if v]

    def keys(self):
        return [i for i, _ in self.items()]

    def bank5(self):
        """bank 5, all 1024 words: {0xA000 + i: value}"""
        return {0xA000 + i: self.a[0xA000 + i] for i in range(0x400)}

    def clear(self):
        C.memset(C.addressof(self.a), 0, C.sizeof(self.a))

    def copy(self):
        return dict(self.items())


class Machine:
    """one DSP with flat memory: the simulator of the tools (make_*.py, dspasm.py, dspnative.py, sim_main.py).
    Starts with the condition bits of `context` and the hidden registers HIDDEN (both overridable)."""

    def __init__(self, context=None, hidden=None):
        L = lib()
        self.h = L.dsp_machine_new()
        self.mem = MemView(L.dsp_machine_mem(self.h), self)
        self._progs = {}
        self.jump = None
        self._access = None
        self.reset(context, hidden)

    def __del__(self):
        if getattr(self, 'h', None) and _lib is not None:
            _lib.sl_free(self.h)
            self.h = None

    def reset(self, context=None, hidden=None, regs=None):
        flags = CONTEXT_BITS[context or CONTEXT] + HIGH_BITS
        h = dict(HIDDEN)
        h.update(hidden or {})
        self.set_state(regs=regs or [0.0] * 8, flags=flags, pending={}, hidden=h)

    def state(self):
        r, h = (C.c_double * 8)(), (C.c_double * 8)()
        f, pm, pv = C.c_uint32(), C.c_uint32(), C.c_uint32()
        lib().sl_get_state(self.h, 0, r, C.byref(f), C.byref(pm), C.byref(pv), h)
        bits = _str(pv.value)
        pending = {g: bits[4 * g:4 * g + 4] for g in range(8) if pm.value >> 4 * g & 0xF}
        return list(r), _str(f.value), pending, {k: h[k] for k in range(4, 8)}

    def set_state(self, regs=None, flags=None, pending=None, hidden=None):
        r0, f0, p0, h0 = self.state() if hasattr(self, '_init') else ([0.0] * 8, '0' * 32, {}, dict(HIDDEN))
        self._init = True
        regs = r0 if regs is None else regs
        flags = f0 if flags is None else flags
        pending = p0 if pending is None else pending
        hidden = h0 if hidden is None else hidden
        pm = pv = 0
        for g, b4 in pending.items():
            pm |= 0xF << 4 * g
            pv |= _bits(b4) << 4 * g
        h = (C.c_double * 8)(*[float(hidden.get(k) or 0.0) for k in range(8)])
        lib().sl_set_state(self.h, 0, (C.c_double * 8)(*map(float, regs)), _bits(flags), pm, pv, h)

    regs = property(lambda self: self.state()[0], lambda self, r: self.set_state(regs=r))
    flags = property(lambda self: self.state()[1], lambda self, f: self.set_state(flags=f))
    hidden = property(lambda self: self.state()[3], lambda self, h: self.set_state(hidden=h))

    def set_edm(self, base=324.0, lat=2, length=983040):
        """give the machine a delay line (DSP_HARDWARE.md §8.4): [4300] goes to the EDM at end_sample; taps read it"""
        lib().sl_set_edm(self.h, 0, base, lat, length)

    def end_sample(self):
        """the end of a sample: the delay line's DMA, then the sample counter (f5c, tap positions) + 1"""
        lib().dsp_machine_end_sample(self.h)

    def set_access(self, fn):
        """fn(op 'r' / 'w', address, value) -> value read: takes every memory access (None: the flat memory)"""
        if fn is None:
            self._access = None
            lib().dsp_machine_set_access(self.h, None)
            return
        cb = C.CFUNCTYPE(C.c_double, C.c_int, C.c_int64, C.c_double)(
            lambda op, a, v: float(fn(chr(op), a, v) or 0.0))
        self._access = cb
        lib().dsp_machine_set_access(self.h, cb)

    def run(self, ins):
        """one instruction; sets self.jump (words from the instruction end, or None). Raises on undecodable
        instructions (ValueError) and newton with r7 (IndexError)."""
        jumped, off = C.c_int(), C.c_int()
        e = lib().dsp_exec1(self.h, words_arg(ins), len(ins), C.byref(jumped), C.byref(off))
        if e == -2:
            raise ValueError(f'undecodable instruction {" ".join(f"{w:04x}" for w in ins)}')
        if e == -1:
            raise IndexError('newton with r7')
        self.jump = off.value if jumped.value else None
        return self.jump

    def add(self, program):
        """decode a program (list of instructions) once; returns its id (cached by the words)"""
        key = tuple(tuple(i) for i in program)
        pid = self._progs.get(key)
        if pid is None:
            words, starts = [], []
            for ins in program:
                starts.append(len(words))
                words += list(ins)
            pid = lib().dsp_machine_add(self.h, words_arg(words), len(words), (C.c_int * max(1, len(starts)))(*starts),
                                        len(starts))
            self._progs[key] = pid
        return pid

    def run_program(self, program):
        """one pass of a program (an id from add, or a list of instructions), jumps followed"""
        pid = program if isinstance(program, int) else self.add(program)
        if lib().dsp_machine_run(self.h, pid):
            raise RuntimeError(lib().sl_error(self.h).decode())
