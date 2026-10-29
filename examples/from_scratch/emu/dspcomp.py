"""Front end of the C core (dspcore.c): every instruction of a chain.Program becomes a list of units and conditions.

The decoding is the C decoder's (dspdec.h dsp_decode: template, slot area, immediate stream, units, jumps); this module
only packs the result into ctypes arrays (and dicts, for dspjit.py). What depends on run-time values (registers,
flags, hidden registers, memory) is left to the C core.
"""
import ctypes as C
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dspref  # noqa: E402

K_A, K_B, K_M, K_J, K_HID, K_X28, K_XB5, K_XFLAG, K_XMEM, K_XTAP = range(1, 11)


class Op(C.Structure):
    _fields_ = [('kind', C.c_int32), ('unit', C.c_int32), ('w', C.c_int32), ('w2', C.c_int32), ('a', C.c_int32),
                ('b', C.c_int32), ('c', C.c_int32), ('has', C.c_int32), ('imm', C.c_double * 3)]


class Cond(C.Structure):
    _fields_ = [('cc', C.c_int32), ('neg', C.c_int32), ('op_t', C.c_int32), ('op_f', C.c_int32)]


class Insn(C.Structure):
    _fields_ = [('op0', C.c_int32), ('nop', C.c_int32), ('cond0', C.c_int32), ('ncond', C.c_int32),
                ('err', C.c_int32)]


_OPS, _CONDS, _JOFF = (Op * 64)(), (Cond * 64)(), (C.c_int32 * 64)()


def decode(ins):
    """(ops, conds, jumps) of one instruction: ops = [dict] in body order, conds = [(cc, neg, op skipped if true,
    op skipped if false)], jumps = {op index: offset in words from the instruction end}. Raises ValueError for an
    instruction that cannot run."""
    no, nc = C.c_int(), C.c_int()
    e = dspref.lib().dsp_decode(dspref.words_arg(ins), len(ins), _OPS, 64, C.byref(no), _CONDS, 64, C.byref(nc),
                                _JOFF)
    if e:
        raise ValueError(f'undecodable instruction ({e}): {" ".join(f"{w:04x}" for w in ins)}')
    ops, jumps = [], {}
    for k in range(no.value):
        o = _OPS[k]
        ops.append(dict(kind=o.kind, unit=o.unit, w=o.w, w2=o.w2, a=o.a, b=o.b, c=o.c, has=o.has, imm=list(o.imm)))
        if o.kind == K_J:
            jumps[k] = _JOFF[k]
    conds = [(c.cc, c.neg, c.op_t, c.op_f) for c in _CONDS[:nc.value]]
    return ops, conds, jumps


def decode_program(prog):
    """[(ops, conds) or None] per instruction of a chain.Program; None = undecodable (fails only if it runs, as in
    chain.py). Jump units carry the target instruction index in 'a' (-1 = no instruction starts there). Cached on
    the Program (compile_program, dspjit.translate and dspjit.groups each ask for it)."""
    cached = getattr(prog, '_dspcomp_decoded', None)
    if cached is not None:
        return cached
    key = tuple(tuple(i) for i in prog.ins)               # the result depends on the words only
    out = _BY_WORDS.get(key)
    if out is None:
        out = _BY_WORDS[key] = _decode_program(prog)
    try:
        prog._dspcomp_decoded = out
    except AttributeError:
        pass
    return out


_BY_WORDS = {}                                            # decode_program results by program words (per process)


def _decode_program(prog):
    out = []
    for i, ins in enumerate(prog.ins):
        try:
            o, c, jumps = decode(list(ins))
        except ValueError:
            out.append(None)
            continue
        if len(o) > 64:
            raise ValueError('more than 64 units in one instruction')
        for k, n in jumps.items():
            o[k]['a'] = prog.pos.get(prog.start[i] + len(ins) + n, -1)
        out.append((o, c))
    return out


def compile_program(prog, decoded=None):
    """ctypes arrays (ninsn, insn, nops, ops, nconds, conds) of a chain.Program, for the interpreter"""
    insn, ops, conds = [], [], []
    for x in decoded or decode_program(prog):
        if x is None:
            insn.append((len(ops), 0, len(conds), 0, 1))
            continue
        o, c = x
        insn.append((len(ops), len(o), len(conds), len(c), 0))
        ops += o
        conds += c
    ia = (Insn * max(1, len(insn)))(*[Insn(*x) for x in insn])
    oa = (Op * max(1, len(ops)))()
    for k, o in enumerate(ops):
        oa[k] = Op(o['kind'], o['unit'], o['w'], o['w2'], o['a'], o['b'], o['c'], o['has'],
                   (C.c_double * 3)(*o['imm']))
    ca = (Cond * max(1, len(conds)))(*[Cond(*x) for x in conds])
    return len(insn), ia, len(ops), oa, len(conds), ca
