"""ARCHIVED 2026-10-29 (the Python front end, before dspdec.h): front end of the C core (dspcore.c): every instruction of a chain.Program becomes a list of units and conditions.

The decoding is re/dspisa.py's own (dspisa.Layout: template, slot area, immediate stream, units, jumps); this module
only replays the immediate reads of the A / B / M units in run()'s order and packs the result into ctypes arrays.
What depends on run-time values (registers, flags, hidden registers, memory) is left to the C core.
"""
import ctypes as C
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
import dspisa  # noqa: E402  (archive/dspisa.py)

K_A, K_B, K_M, K_J, K_HID, K_X28, K_XB5, K_XFLAG, K_XMEM = range(1, 10)


class Op(C.Structure):
    _fields_ = [('kind', C.c_int32), ('unit', C.c_int32), ('w', C.c_int32), ('w2', C.c_int32), ('a', C.c_int32),
                ('b', C.c_int32), ('c', C.c_int32), ('has', C.c_int32), ('imm', C.c_double * 3)]


class Cond(C.Structure):
    _fields_ = [('cc', C.c_int32), ('neg', C.c_int32), ('op_t', C.c_int32), ('op_f', C.c_int32)]


class Insn(C.Structure):
    _fields_ = [('op0', C.c_int32), ('nop', C.c_int32), ('cond0', C.c_int32), ('ncond', C.c_int32),
                ('err', C.c_int32)]


def decode(ins):
    """(ops, conds, jumps) of one instruction: ops = [dict] in body order, conds = [(cc, neg, op skipped if true,
    op skipped if false)], jumps = {op index: offset in words from the instruction end}. Raises where dspisa.run
    would."""
    L = dspisa.Layout(ins)
    imm = L.imm()
    n_words = iter(L.n_list)
    ops, pos_op, jumps = [], {}, {}

    def add(pos, **kw):
        pos_op[pos] = len(ops)
        o = dict(kind=0, unit=0, w=0, w2=0, a=0, b=0, c=0, has=0, imm=[0.0, 0.0, 0.0])
        o.update(kw)
        ops.append(o)
        return o

    def take(mode, field=None):
        return dspisa.raw_value(imm.take(mode, field))

    # the immediate reads below are those of a_op / b_op / mac: A and B start from byte Y; a MAC reads Y at byte Y,
    # C right after it (or at byte C when Y is a register) and W at byte W. They do not depend on run-time values.
    for pos, (w, k) in enumerate(zip(L.body, L.t)):
        unit = L.units.index(pos) + 1 if pos in L.units else 0
        if k == 'A':
            o = add(pos, kind=K_A, unit=unit, w=w)
            if (w >> 4) & 3:
                o['imm'][0], o['has'] = take((w >> 4) & 3, w & 15), 1
        elif k == 'B':
            o = add(pos, kind=K_B, unit=unit, w=w)
            if (w >> 4) & 3 and not (w >> 12 >= 8 and (w >> 8) & 15 == 0xF):
                o['imm'][0], o['has'] = take((w >> 4) & 3, w & 15), 1
        elif k == 'M':
            w2 = next(n_words)
            o = add(pos, kind=K_M, unit=unit, w=w, w2=w2)
            ym, cm, wm = (w >> 4) & 3, (w2 >> 10) & 3, (w2 >> 4) & 3
            if ym:
                o['imm'][0] = take(ym, w & 15)
            if cm:
                o['imm'][1] = take(cm) if ym else take(cm, (w2 >> 6) & 7)
            if wm:
                o['imm'][2] = take(wm, w2 & 15)
        elif k == 'J':
            jumps[len(ops)] = L.jump(w)
            add(pos, kind=K_J)
        elif k == 'C' and w >> 8 == 0x40:
            tg, m = 4 + ((w >> 6) & 3), (w >> 4) & 3
            if m == 0:
                add(pos, kind=K_HID, a=tg, b=0, c=w & 15)
            elif m != 3:
                o = add(pos, kind=K_HID, a=tg, b=1)
                o['imm'][0] = L.hidden_static(pos, w)
        elif k == 'T':
            reg, slot, mode = w >> 12, (w & 15) >> 1, (w >> 4) & 0xFF
            if mode == 0x28:
                add(pos, kind=K_X28, a=reg, b=w & 15)
            elif mode in (0xDA, 0xDE, 0xDB, 0xDF):
                add(pos, kind=K_XB5, a=reg, b=w & 15, c=6 if mode in (0xDA, 0xDE) else 7)
            elif 0xD0 <= mode <= 0xD7:
                add(pos, kind=K_XB5, a=reg, b=(mode & 7) << 4 | (w & 15), c=0)
            elif mode == 0x9B:
                add(pos, kind=K_XFLAG, a=dspisa.FLAG_GROUP.get(reg, 0), b=w & 15)
            elif mode == 0xFE and slot < len(L.slots):
                add(pos, kind=K_XMEM, a=reg, b=L.slots[slot])
    conds = []
    for w, k in zip(L.body, L.t):
        if k == 'C':
            sk = [L.skipped_unit(w, c) for c in (True, False)]
            sk = [pos_op.get(L.units[u - 1], -1) if u else -1 for u in sk]
            conds.append(((w >> 9) & 31, (w >> 8) & 1, sk[0], sk[1]))
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
        except (ValueError, StopIteration, IndexError):
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
