"""dsplift.py: DSP microcode -> plain C, one function per program, with the emulator's semantics (a study tool).

    python3 sdk/s1cc/tools/dsplift.py OUT.c NAME=PROGRAM.hex [NAME=PROGRAM.hex ...]
    python3 sdk/s1cc/tools/dsplift.py OUT.c NAME=PROGRAM.hex --check [--samples 64] [--seed 1]

PROGRAM.hex: the program's 16-bit words in execution order, hexadecimal, separated by blanks (the form of the
corpus files and of `dis_all`-style extractions; a leading run of 0000 / 0001 data words is skipped with --skip-lead).

Every instruction becomes a C block that does what the emulator's C core (examples/from_scratch/emu/dspcore.h) does:
all units read the registers of the instruction start; conditions skip units; X9bY flags arrive one instruction
later; loads land in the register at the end of the instruction; jumps (009Y) are gotos. The disassembly of each
instruction is a comment above it. The runtime (registers, flags, hidden registers, memory) is
sdk/s1cc/tools/dsplift_rt.h; the C is meant to be read and rewritten by hand into a kernel (for example
sdk/s1cc/examples/tb03_ported), and to serve as the native oracle of that rewrite.

--check builds the lifted C with -DDSPLIFT_FLAT -DDSPLIFT_QUANT (the emulator's machine mode: flat memory, RAW
quantisation) and runs it against the emulator (dspref.Machine) for --samples samples, each with random values in
every word the program reads (0, 1, or uniform -1..1, as system-1/s1_progs.py) and the same start state. It
reports the largest difference of the registers and of the memory per program. The MAC of the lift is the double
path (the emulator takes an exact 128-bit path first), so differences of a few RAW LSB are expected; anything
larger is a lifter bug.

What is not modelled (reported as a comment in the C and on stderr): delay-tap requests (Xa9Y), the bank-4 MXM,
and condition words whose skipped unit does not exist in the instruction (the BQ2 control words 00aY / 00cY / 00fY
that the emulator decodes as conditions; docs/vol5_toolchain_compiler_sdk/dsplift.md §5).

Documentation: docs/vol5_toolchain_compiler_sdk/dsplift.md.
"""
import argparse
import ctypes as C
import os
import random
import re
import subprocess
import sys
import tempfile

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 'emu'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
import chain     # noqa: E402
import disasm    # noqa: E402
import dspcomp   # noqa: E402
import dspref    # noqa: E402
import isa       # noqa: E402
from dspcomp import K_A, K_B, K_M, K_J, K_HID, K_X28, K_XB5, K_XFLAG, K_XMEM, K_XTAP  # noqa: E402


BQ2 = False                                     # set by main() with --bq2


def lit(v):
    """exact C literal of a double"""
    return v.hex() if v == v and abs(v) != float('inf') else 'NAN'


def yop(mode, y, imm):
    if mode:
        return lit(imm)
    return ('-' if y & 8 else '') + 'r[%d]' % (y & 7)


def unit_a(o):
    """unit A (dspcore.h a_op)"""
    w = o['w']
    d, op, x, mode, y = w >> 12, (w >> 9) & 7, (w >> 6) & 7, (w >> 4) & 3, w & 15
    Y, D, X = yop(mode, y, o['imm'][0]), 'r[%d]' % (d & 7), 'r[%d]' % x
    if d >= 8:                                                    # sum8 / add3r
        return ['n[%d] = %s + %s + r[%d];' % (d - 8, X, Y, op), 'RES(n[%d]);' % (d - 8)]
    if op == 0:
        if x == 1:
            return ['n[%d] = %s;' % (d, Y)]
        if x == 2 and mode and BQ2:                               # BQ2: rD = (rY / 2)^2 (§5.4)
            return ['n[%d] = 0.25 * r[%d] * r[%d];' % (d, y & 7, y & 7)]
        if x == 2:                                                # newton
            if d == 7:
                return ['ABORT("newton with r7");']
            return ['n[%d] = %s * r[%d];' % (d, D, d + 1), 'n[%d] = 2 - r[%d] * r[%d];' % (d + 1, d + 1, y & 7),
                    'n[%d] = r[%d] * r[%d];' % (y & 7, d + 1, y & 7)]
        if x in (6, 7):
            return ['n[%d] = %s + %s;' % (d, D, Y)]
        if x in (4, 5):
            return ['n[%d] = %s + %s + M->H[%d];' % (d, D, Y, x)]
        return []
    e = {1: '%s + %s + %s' % (D, X, Y), 2: '%s * %s' % (X, Y), 3: '%s + %s * %s' % (D, X, Y),
         4: '%s + %s * %s' % (Y, X, D), 5: '%s - %s * %s' % (Y, X, D), 6: '%s + %s' % (X, Y),
         7: '%s - %s' % (Y, X)}[op]
    return ['n[%d] = AWRAP(%s);' % (d, e)]


def bq2_flagless(o):
    """BQ2 implicit flags: a mov (unit A, op 0, x 1: rD = Y) sets no condition group and takes no group number;
    the next arithmetic unit does (TB-03 voice, the piecewise cutoff map: docs/vol5_toolchain_compiler_sdk/dsplift.md §5.1)"""
    w = o['w']
    return o['kind'] == K_A and (w >> 12) < 8 and (w >> 9) & 7 == 0 and (w >> 6) & 7 == 1


def unit_b(o):
    """unit B (dspcore.h b_op, sfu)"""
    w = o['w']
    d, b, x, mode, y = w >> 12, (w >> 8) & 15, (w >> 6) & 7, (w >> 4) & 3, w & 15
    if d >= 8 and b == 0xF:
        b76 = (w >> 6) & 3
        if b76 == 0:
            return ['SFU(0x%04x);' % w]
        if b76 == 1:
            sub, yy = (w >> 4) & 3, w & 15
            if sub == 0:
                return ['n[%d] = r[%d];' % (d & 7, yy & 7)]                           # f4
            if sub == 2 and yy >= 12:
                return ['n[%d] = pymod(r[%d] + M->H[%d], 512);' % (d & 7, d & 7, yy - 8)]   # f6
            if sub == 1 and yy != 12:
                return ['n[%d] = 0.0;' % (d & 7)]
            if sub == 1:
                return ['n[%d] = F5C(M);' % (d & 7)]
        return []                                                  # f7: section marker
    Y, X = yop(mode, y, o['imm'][0]), 'r[%d]' % x
    if d >= 8 and b == 0xE:
        D, sub = 'r[%d]' % (d & 7), (w >> 6) & 3
        if sub == 0:
            return ['RESV(%s + %s);' % (D, Y)]                     # cmp
        if sub == 1:
            return ['n[%d] = %s;' % (d & 7, Y)]                    # bic
        if sub == 2:
            return ['DIVSEED(0x%04x);' % w]
        R, neg = 'r[%d]' % (y & 7), y & 8
        v = {0: '%s %s %s' % (D, '-' if neg else '+', R),
             1: ('pymax(-1.0, pymin(1.0, %s))' % R) if neg else 'fabs(%s)' % R,
             2: ('1 - 2 * fabs(wrap1(-%s - 1.5))' % R) if neg else 'wrap1(%s)' % R,
             3: ('-%s' % R) if neg else 'pymod(%s, 2) / 2' % R}[mode]
        return ['n[%d] = %s;' % (d & 7, v)]
    k0, k1 = d >> 3, b & 0xE
    if k0 == 0 and k1 <= 0xA:
        return ['n[%d] = BITOP(%d, %s, %s);' % (d & 7, k1, X, Y)]
    if k0 == 1 and k1 == 0xC:
        if mode:
            sh = y if mode == 1 else (-y if mode == 2 else -16 - y)
            return ['n[%d] = SHIFT(%s, %d);' % (d & 7, X, sh)]
        return ['FREXP(%d, %d, %s);' % (d & 7, y & 7, X)]
    e = {(0, 0xC): 'fabs(%s) + %s' % (X, Y), (0, 0xE): '%s - fabs(%s)' % (Y, X), (1, 0): 'pymin(%s, %s)' % (X, Y),
         (1, 4): 'pymax(%s, %s)' % (X, Y), (1, 2): 'pymin(-%s, %s)' % (X, Y), (1, 6): 'pymax(-%s, %s)' % (X, Y),
         (1, 8): 'fabs(%s)' % X, (1, 0xA): 'fabs(%s)' % Y, (1, 0xE): Y}.get((k0, k1))
    if e is None:
        return []
    out = ['n[%d] = %s;' % (d & 7, e)]
    if k0 == 1 and k1 <= 6:                                       # min / max: flags of the comparison
        out.append('RESZ(n[%d], (%s%s) == %s);' % (d & 7, '-' if k1 & 2 else '', X, Y))
    return out


def unit_m(o):
    """MAC (dspcore.h mac, double path)"""
    w1, w2 = o['w'], o['w2']
    d, op1, x, ym, y = w1 >> 12, (w1 >> 9) & 7, (w1 >> 6) & 7, (w1 >> 4) & 3, w1 & 15
    z, op2, c, wm, wf = w2 >> 12, (w2 >> 9) & 7, (w2 >> 6) & 7, (w2 >> 4) & 3, w2 & 15
    if op1 < 2 or (d >= 8 and op1 < 4):
        return []
    Y = lit(o['imm'][0]) if ym else 'r[%d]' % (y & 7)
    Cc = lit(o['imm'][1]) if (op2 >> 1) else 'r[%d]' % c
    W = lit(o['imm'][2]) if wm else ('-' if wf & 8 else '') + 'r[%d]' % (wf & 7)
    Z = ('-' if z & 8 else '') + 'r[%d]' % (z & 7)
    f1, f2 = Y, Cc
    if op2 == 0 and op1 >= 6:                                     # sum mode
        if not ym:
            f1 = None
        f2 = None
    X = 'r[%d]' % x
    p1 = X if f1 is None else '%s * %s' % (f1, X)
    terms = [('-' if op1 & 1 else '') + '(' + p1 + ')']
    if op1 & 2:
        terms.append('(' + W + ')')
    if op1 & 4:
        terms.append('(' + (Z if f2 is None else '%s * %s' % (f2, Z)) + ')')
    e = ' + '.join(terms)
    if d >= 8:
        e = '(%s) / 2' % e
    return ['n[%d] = %s;' % (d & 7, e)]




def bq2_units(ins, ops, conds_dec, warn, where):
    """BQ2 (Boutique) control words of one instruction: the units in numbering order (ops, control words,
    transfers), the ordinary conditions as (cc, neg, field T, field F), and the slot words. The condition words are
    found in the body from the decoder's conditions (same order; the op words taken out first). Control word = a C
    word whose field T is >= 8 (8 jump, 10 call, 12 / 13 / 14 unknown, 15 return); docs/vol5_toolchain_compiler_sdk/dsplift.md §5"""
    body = list(ins[1:])
    free = list(range(len(body)))
    for o in ops:                                  # the op words are not condition words
        for w in ([o['w']] + ([o['w2']] if o['kind'] == K_M else [])) if o['kind'] in (K_A, K_B, K_M) else []:
            for i in free:
                if body[i] == w & 0xFFFF:
                    free.remove(i)
                    break
    conds, pos = [], 0
    for cc, neg, _, _ in conds_dec:
        for i in free:
            w = body[i]
            if i >= pos and not w >> 14 and (w >> 9) & 0x1F == cc and (w >> 8) & 1 == neg:
                conds.append((cc, bool(neg), (w >> 4) & 15, w & 15, i))
                pos = i + 1
                break
        else:
            if any(body[i] >> 14 == 1 for i in free):      # an hset word (40TY): the decoder lists it here too
                continue
            warn.append('%s: condition word %d/%d not found in %s' % (where, cc, neg, ' '.join('%04x' % w for w in body)))
    ctl = [c for c in conds if c[2] >= 8]
    nslot = max([(c[3] >> 1) + 1 for c in ctl if c[2] in (8, 10)] or [0])
    slots = body[len(body) - nslot:] if nslot else []
    first_slot = len(body) - nslot
    conds = [c for c in conds if c[4] < first_slot]     # slot words read as conditions by the decoder
    ctl = [c[:4] for c in conds if c[2] >= 8]
    normal = [c[:4] for c in conds if c[2] < 8]
    nhead = 0
    while nhead < len(ops) and ops[nhead]['kind'] in (K_A, K_B, K_M, K_J):
        nhead += 1
    units = [('op', o) for o in ops[:nhead]] + [('ctl', c) for c in ctl] + [('op', o) for o in ops[nhead:]]
    return units, normal, slots


def raw_value(w):
    """a RAW word as a number (dspdec.h dsp_raw_value)"""
    sc = (w >> 29) & 3
    v = (w & 0x1FFFFFFF) / float(1 << (32 - 4 * sc))
    return -v if w >> 31 else v


def take_value(stream, mode, byte):
    """the immediate of mode 1/2/3 (16/24/32 bits) `byte` bytes into a word stream (dspdec.h dsp_take, no refs)"""
    bits = (0, 16, 24, 32)[mode]
    data = b''.join(w.to_bytes(2, 'big') for w in stream) + bytes(8)
    v = int.from_bytes(data[byte:byte + 4], 'big') >> (32 - bits)
    return raw_value((v << (32 - bits)) & 0xFFFFFFFF)


def bq2_mmcnn(ins):
    """BQ2 class 0x108: M1 M2 C N1 N2 I... (two MACs, the C word picks one; the immediate stream is N1 N2 I...,
    as the SYSTEM-1 corpus form `2109 140b 140b 0b12 0024 0027 ...`). Returns (ops, (cc, neg, T, F))."""
    body = list(ins[1:])
    stream = body[3:]
    ops = []
    for k, (w1, w2) in enumerate(((body[0], body[3]), (body[1], body[4]))):
        ym, cm, wm = (w1 >> 4) & 3, (w2 >> 10) & 3, (w2 >> 4) & 3
        imm = [take_value(stream, ym, w1 & 15) if ym else 0.0,
               take_value(stream, cm, ((w2 >> 6) & 7) + (8 if (w2 >> 9) & 1 else 0)) if cm else 0.0,
               take_value(stream, wm, w2 & 15) if wm else 0.0]
        ops.append(dict(kind=K_M, unit=k + 1, w=w1, w2=w2, a=0, b=0, c=0, has=1, imm=imm))
    c = body[2]
    return ops, ((c >> 9) & 0x1F, (c >> 8) & 1, (c >> 4) & 15, c & 15)


def lift(name, words, out, warn, bq2=None):
    """append the C function `static void NAME(LiftM *M)` of one program to out (a list of lines). bq2 = None, or
    the program's byte address in the BQ2 DSP's program memory (control words, implicit flags: §5)"""
    prog = chain.Program(words, name, strict=False)
    dec = dspcomp.decode_program(prog)
    table = isa.Table.load()
    dis = []
    for ins in prog.ins:                          # one disassembly line per instruction
        text, _ = disasm.program(list(ins), table, name)
        lines = [ln.split(';')[0].strip() for ln in text.splitlines()
                 if ln.strip() and not ln.startswith((';', '.'))]
        dis.append(' | '.join(lines) if lines else '?')
    targets = {o['a'] for x in dec if x for o in x[0] if o['kind'] == K_J}
    pos_of = {}                                   # BQ2: word address -> instruction index
    if bq2 is not None:
        for k, p in prog.pos.items():
            pos_of[bq2 // 2 + k] = p
        for k, x in enumerate(dec):
            if x is None:
                continue
            units, _, slots = bq2_units(prog.ins[k], x[0], x[1], [], k)
            for kind, u in units:
                if kind == 'ctl' and u[2] == 8 and (u[3] >> 1) < len(slots) and slots[u[3] >> 1] in pos_of:
                    targets.add(pos_of[slots[u[3] >> 1]])
    out.append('static void %s(LiftM *M)' % name)
    out.append('{')
    out.append('    double *r = M->r, n[8], rv[64];')
    out.append('    int rz[64], jto = -1;')
    out.append('    uint32_t um, uv;')
    out.append('    uint64_t sk, dn;')
    out.append('    (void)rv, (void)rz, (void)jto, (void)sk, (void)dn, (void)um, (void)uv;')
    for k, x in enumerate(dec):
        if k in targets:
            out.append('L%d:;' % k)
        words_k = ' '.join('%04x' % w for w in prog.ins[k])
        out.append('    /* %d: %s   [%s] */' % (k, dis[k], words_k))
        if x is None:
            out.append('    ABORT("%s: undecodable instruction %d");' % (name, k))
            warn.append('%s: instruction %d undecodable (%s)' % (name, k, words_k))
            continue
        ops, conds = x
        out.append('    {')
        out.append('        memcpy(n, r, sizeof n);')
        out.append('        um = uv = 0, sk = dn = 0, jto = -1;')
        if bq2 is None:
            units = [('op', o) for o in ops]
            for cc, neg, ot, of in conds:
                t = '1ull << %d' % ot if ot >= 0 else '0'
                f = '1ull << %d' % of if of >= 0 else '0'
                out.append('        sk |= ((M->F >> %d) & 1) != %d ? %s : %s;' % (cc, neg, t, f))
            slots = []
        elif prog.ins[k][0] >> 5 == 0x108:
            ops, cw = bq2_mmcnn(prog.ins[k])
            units, normal, slots = [('op', o) for o in ops], [cw], []
            out.append('        /* BQ2 class 0x108 read as M M C N N (§5.4) */')
        else:
            units, normal, slots = bq2_units(prog.ins[k], ops, conds, warn, '%s %d' % (name, k))
        if bq2 is not None:
            for cc, neg, T, F in normal:              # fields are 1-based unit numbers; 0 / out of range: none
                t = '1ull << %d' % (T - 1) if 1 <= T <= len(units) else '0'
                f = '1ull << %d' % (F - 1) if 1 <= F <= len(units) else '0'
                out.append('        sk |= ((M->F >> %d) & 1) != %d ? %s : %s;' % (cc, int(neg), t, f))
        jump = None
        nam = nb = 0                                 # BQ2 implicit flags: A/M units -> groups 1, 2; B -> 3, 4
        post = []
        for i, (ukind, o) in enumerate(units):
            body = []
            if ukind == 'ctl':
                cc, neg, T, Y = o
                own = '(((M->F >> %d) & 1) == %d)' % (cc, int(neg))      # its own condition: runs when false
                if T == 8:
                    tgt = slots[Y >> 1] if (Y >> 1) < len(slots) else None
                    if tgt is not None and tgt in pos_of:
                        body = ['if %s jto = %d;' % (own, pos_of[tgt])]
                        jump = True
                    else:
                        body = ['if %s jto = -2;' % own]
                        post.append('if (jto == -2) return;')
                        out.append('        /* jump to byte address 0x%04x (outside this program): ends it */'
                                   % (2 * tgt if tgt is not None else 0))
                elif T == 10:
                    tgt = slots[Y >> 1] if (Y >> 1) < len(slots) else 0
                    body = ['if %s CALL(M, 0x%04x);' % (own, 2 * tgt)]
                elif T == 15:
                    body = ['if %s jto = -2;' % own]
                    post.append('if (jto == -2) return;')
                else:
                    body = ['/* control word %x%x (cc %d): no known effect */' % (T, Y, cc)]
                    warn.append('%s: instruction %d: control word type %x (no known effect)' % (name, k, T))
            else:
                kind = o['kind']
                if kind in (K_A, K_B, K_M):
                    st = {K_A: unit_a, K_B: unit_b, K_M: unit_m}[kind](o)
                    body = ['RESET();'] + st + ['QUANT(n);', 'DONE(%d, %d);' % (o['unit'], (o['w'] >> 12) & 7)]
                    if bq2 is not None and not bq2_flagless(o):
                        if kind == K_B:
                            nb += 1
                            g = 2 + nb
                        else:
                            nam += 1
                            g = nam
                        if g <= 4 and (kind != K_B or nb <= 2) and (kind == K_B or nam <= 2):
                            body.append('SETFLAGS(%d, %d);' % (g, o['unit']))
                elif kind == K_J:
                    jump = True
                    body = ['jto = %d;' % o['a']]
                elif kind == K_HID:
                    body = (['M->H[%d] = pymod(r[%d], 512);' % (o['a'], o['c'] & 7)] if o['b'] == 0
                            else ['M->H[%d] = %s;' % (o['a'], lit(o['imm'][0]))])
                elif kind == K_X28:
                    ia = '(((int64_t)(M->H[6] * 16384) + %d) & 0x7FF)' % o['b']
                    body = (['n[%d] = RD(M, %s);' % (o['a'], ia)] if o['a'] < 8
                            else ['WR(M, %s, r[%d]);' % (ia, o['a'] - 8)])
                elif kind == K_XB5:
                    ia = ('(int64_t)(M->H[%d] * 16384) + %d' % (o['c'], o['b'])) if o['c'] else str(o['b'])
                    ia = '0xA000 + %s' % ia
                    body = (['n[%d] = RD(M, %s);' % (o['a'], ia)] if o['a'] < 8
                            else ['WR(M, %s, r[%d]);' % (ia, o['a'] - 8)])
                elif kind == K_XFLAG:
                    body = ['SETFLAGS(%d, %d);' % (o['a'], o['b'])] if o['a'] and o['b'] < 64 else []
                elif kind == K_XMEM:
                    body = (['n[%d] = RD(M, 0x%04x);' % (o['a'], o['b'])] if o['a'] < 8
                            else ['WR(M, 0x%04x, r[%d]);' % (o['b'], o['a'] - 8)])
                elif kind == K_XTAP:
                    body = ['TAP(M, r[%d]); /* no delay line: reads 0 */' % (o['a'] & 7)]
                    warn.append('%s: instruction %d: tap request (no delay line: reads 0)' % (name, k))
            if not body:
                continue
            guard = 'if (!(sk >> %d & 1)) ' % i if (conds if bq2 is None else normal) else ''
            out.append('        %s{ %s }' % (guard, ' '.join(body)))
        out.append('        M->F = (M->F & ~M->PM) | M->PV;')
        out.append('        M->PM = um, M->PV = uv;')
        out.append('        memcpy(r, n, sizeof n);')
        for ln in post:
            out.append('        ' + ln)
        if jump:
            out.append('        if (jto >= 0) switch (jto) { %s default: ABORT("jump"); }'
                       % ' '.join('case %d: goto L%d;' % (t, t) for t in sorted(targets)))
        out.append('    }')
    out.append('}')
    out.append('')
    return prog


def read_words(path, skip_lead):
    w = [int(x, 16) for x in open(path).read().split()]
    if skip_lead:
        i = 0
        while i < len(w) and w[i] in (0, 1):
            i += 1
        w = w[i:]
    i = 0                                     # stop at a header of length 0 (data after the code: 0000 padding)
    while i < len(w) and w[i] & 0x1F:
        i += w[i] & 0x1F
    if i < len(w):
        print('note: %s: words from %d on are not instructions (%04x), left out' % (path, i, w[i]), file=sys.stderr)
    return w[:i]


API = r'''
/* --check API (ctypes) */
LiftM *dsplift_new(void) { return calloc(1, sizeof(LiftM)); }
double *dsplift_mem(LiftM *M) { return M->flat; }
void dsplift_set(LiftM *M, const double *r, uint32_t F, const double *h) { memcpy(M->r, r, sizeof M->r); M->F = F; M->PM = M->PV = 0; memcpy(M->H, h, sizeof M->H); }
void dsplift_get(LiftM *M, double *r) { memcpy(r, M->r, sizeof M->r); }
void dsplift_end_sample(LiftM *M) { M->t++; }
'''


def check(c_path, progs, samples, seed):
    """the lift against the emulator (machine mode), random inputs; returns the number of programs that differ"""
    tmp = tempfile.mkdtemp(prefix='dsplift_')
    src = os.path.join(tmp, 'lift.c')
    body = open(c_path).read() + API
    body += 'typedef void (*pf)(LiftM *);\npf dsplift_progs[] = {%s};\n' % ', '.join(n for n, _ in progs)
    open(src, 'w').write(body)
    so = os.path.join(tmp, 'lift.so')
    subprocess.check_call([dspref.CC, '-O1', '-ffp-contract=off', '-fno-math-errno', '-shared', '-fPIC', '-w',
                           '-DDSPLIFT_FLAT', '-DDSPLIFT_QUANT', '-I', HERE, '-o', so, src, '-lm'])
    L = C.CDLL(so)
    L.dsplift_new.restype = C.c_void_p
    L.dsplift_mem.restype = C.POINTER(C.c_double)
    L.dsplift_mem.argtypes = [C.c_void_p]
    L.dsplift_set.argtypes = [C.c_void_p, C.POINTER(C.c_double), C.c_uint32, C.POINTER(C.c_double)]
    L.dsplift_get.argtypes = [C.c_void_p, C.POINTER(C.c_double)]
    L.dsplift_end_sample.argtypes = [C.c_void_p]
    pfs = (C.CFUNCTYPE(None, C.c_void_p) * len(progs)).in_dll(L, 'dsplift_progs')
    bad = 0
    for k, (name, prog) in enumerate(progs):
        reads = set()
        for x in dspcomp.decode_program(prog):
            for o in (x[0] if x else []):
                if o['kind'] == K_XMEM and o['a'] < 8:
                    reads.add(o['b'])
                if o['kind'] == K_XB5 and o['a'] < 8 and not o['c']:
                    reads.add(0xA000 + o['b'])
        rnd = random.Random(seed)
        m = dspref.Machine()
        regs, flags, _, hid = m.state()
        F = int(flags[::-1], 2)
        h = (C.c_double * 8)(*[hid.get(i, 0.0) for i in range(8)])
        M = L.dsplift_new()
        L.dsplift_set(M, (C.c_double * 8)(*regs), F, h)
        mem = L.dsplift_mem(M)
        pid = m.add(prog.ins)
        worst_r = worst_m = 0.0
        where, stopped = None, None
        for t in range(samples):
            for a in reads:
                if 0x0000 <= a < 0x2000 and t:
                    continue                        # bank 0 is the program's own state after the first sample
                v = rnd.choice([0.0, 1.0, rnd.uniform(-1, 1)])
                m.mem[a] = v
                mem[MemKey(a)] = v
            try:
                m.run_program(pid)
            except RuntimeError as e:
                stopped = 'the emulator stops at sample %d: %s' % (t, e)
                break
            pfs[k](M)
            r2 = (C.c_double * 8)()
            L.dsplift_get(M, r2)
            for i, (a, b) in enumerate(zip(m.state()[0], r2)):
                if abs(a - b) > worst_r:
                    worst_r, where = abs(a - b), 'r%d at sample %d' % (i, t)
            diff = np.abs(np.ctypeslib.as_array(m.mem.a) - np.ctypeslib.as_array(mem, (0x10000,)))
            a = int(np.argmax(diff))
            if diff[a] > worst_m:
                worst_m, where = float(diff[a]), 'word %04x at sample %d' % (a, t)
            m.end_sample()
            L.dsplift_end_sample(M)
        if stopped:
            print('  %-24s ----  not compared: %s' % (name, stopped))
            continue
        ok = max(worst_r, worst_m) < 1e-6
        bad += not ok
        print('  %-24s %s  max |diff| registers %.3g, memory %.3g%s' % (
            name, 'OK  ' if ok else 'DIFF', worst_r, worst_m, '' if ok else '  (%s)' % where))
    return bad


def MemKey(a):
    return 0xA000 | (a & 0x3FF) if 0xA000 <= a < 0xC000 else a


# ---------------------------------------------------------------- --kernel: s1cc-ready C (docs §6)
def sfu_kernel(w):
    """the SFU word w as straight-line C on named registers (dsplift_k.h: the intrinsics); recipseed gives the exact
    reciprocal and leaves r(D+1) = rY = 1, so the newton steps after it keep it"""
    d, m, y = (w >> 12) - 8, (w >> 4) & 3, w & 15
    Y = 'r%d' % (y & 7)
    if m == 0 and not y & 8:
        # dspcore.h sfu: rD = sign * 2^-e (0 when 2^-e >= 512, |Y| < 2^-9), r(D+1) = the seed, rY = the mantissa; the
        # newton steps after it make 1/Y. Here the exact reciprocal at once and r(D+1) = rY = 1, so the newton steps
        # keep it; no branch (s1cc compiles a select as arithmetic): |Y| below 2^-9 is taken as 2^-9 (the DSP gives 0
        # there) and Y = 0 gives 0 (the DSP leaves rD); s1cc divides by one sign only, hence sign(Y) / |Y|
        return ['n%d = s1_sign(%s) * s1_recip(fmaxf(s1_abs(%s), 0x1p-9f)); n%d = 1.0f; n%d = 1.0f;'
                % (d, Y, Y, (d + 1) & 7, y & 7)]
    if m == 1 and y & 8:
        return ['{ float _f = s1_floor(%s); n%d = s1_exp(_f); n%d = %s - _f; }' % (Y, d, y & 7, Y)]
    if m == 2 and not y & 8:
        raise SystemExit('log2split: not in --kernel')
    if m == 2:
        return ['n%d = s1_sign(%s);' % (d, Y)]
    if m == 3:
        return ['n%d = %s(%s);' % (d, 's1_floor' if y & 8 else 's1_ceil', Y)]
    raise SystemExit('SFU %04x: not in --kernel' % w)


def kernel_insn(k, prog, units, normal, unit_code, used_cc, used_g, own5, overflow_wrap, bq2, warn, name):
    """the C lines of instruction k (--kernel): registers r0..r7 / n0..n7, condition bits f, pending pm / pv.

    No comparison of floats: s1cc compiles a compare into clamped arithmetic, and near a tie (an envelope at its
    turning point) its select gave a value that was neither side (TB-03, 2026-11-11). The condition bits are floats
    0 / 1 made from the SFU sign of the result (exact): !Z = |sign(z)|, !N = min(1, 1 + sign(v)), N|Z =
    1 - max(sign(v), 0) |sign(z)|; a unit a condition may skip is blended, n = old + w (new - old), with w = 0 / 1"""
    out = ['    /* %d: %s */' % (k, ' '.join('%04x' % w for w in prog.ins[k])), '    {']
    written = set()
    for uk, o in units:
        if uk == 'op' and o['kind'] in (K_A, K_B, K_M):
            written |= {int(x) for x in re.findall(r'\bn(\d)\b', unit_code(o))}
        if uk == 'op' and o['kind'] == K_XMEM and o['a'] < 8:
            written.add(o['a'])
    if written:
        out.append('        float %s;' % ', '.join('n%d = r%d' % (i, i) for i in sorted(written)))
    skips = []
    for cc, neg, T, F in normal:
        c = ('(1.0f - f%d)' % cc) if neg else 'f%d' % cc        # the condition, 0 / 1
        if 1 <= T <= len(units) or 1 <= F <= len(units):
            skips.append((c, T, F))

    def weight(i):
        """the unit runs with weight w (0 / 1): skipped if the condition holds and it is T, or fails and it is F"""
        w = []
        for c, T, F in skips:
            if T == i + 1:
                w.append('(1.0f - %s)' % c)
            if F == i + 1:
                w.append(c)
        return ' * '.join(w) if w else None

    def blend(i, lines_fn, regs):
        """unit i's lines, blended by its weight when a condition can skip it"""
        w = weight(i)
        if w is None:
            return lines_fn()
        keep = ' '.join('float o%d = n%d;' % (r, r) for r in sorted(regs))
        mix = ' '.join('n%d = o%d + _w * (n%d - o%d);' % (r, r, r, r) for r in sorted(regs))
        return ['        { float _w = %s; %s' % (w, keep)] + lines_fn() + ['          %s }' % mix]
    newp = {}
    nam = nb = 0
    for i, (uk, o) in enumerate(units):
        if uk == 'ctl':
            out.append('        /* control word %x%x: no effect */' % (o[2], o[3]))
            continue
        kind = o['kind']
        if kind in (K_A, K_B, K_M):
            txt = unit_code(o)
            if k in overflow_wrap:
                txt = txt.replace('AWRAP(', 'KOVERFLOW(')
            txt = re.sub(r'RESZ\((n\d), \((-?)(r\d)\) == ([^)]+)\);', r'RESZ(\1, (\2\3) - (\4));', txt)
            dest = (o['w'] >> 12) & 7
            grp = None
            if bq2 is not None and not bq2_flagless(o):
                if kind == K_B:
                    nb += 1
                    grp = 2 + nb if nb <= 2 else None
                else:
                    nam += 1
                    grp = nam if nam <= 2 else None
            regs = {int(x) for x in re.findall(r'\bn(\d)\b', txt)}
            fl = ''
            if grp in used_g:
                vv = 'u%d_%d' % (k, i)
                out.append('        float %sv = 0.0f, %sz = 0.0f;' % (vv, vv))
                fl = ' %sv = rs ? rvv : n%d; %sz = rs ? rzd : n%d;' % (vv, dest, vv, dest)
                newp[grp] = (vv, weight(i))
            out += blend(i, lambda: ['        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; %s%s }'
                                      % (txt, fl)], regs)
        elif kind == K_XFLAG:
            raise SystemExit('%s %d: X9bY not in --kernel' % (name, k))
        elif kind == K_XMEM:
            a = o['b']
            w = weight(i)
            if o['a'] < 8:                                         # a load: blended like a unit
                if a < 0x2000:
                    src = 'c_%03x' % (a & 0x7FF)
                elif 0xA000 <= a < 0xC000:
                    src = ('K5_%04x' % a) if a in own5 else 'KIN(0x%04x)' % a
                elif 0xC000 <= a < 0xE000:
                    src = 'KP(0x%04x)' % a
                elif a >= 0xE000:
                    src = 'KE(0x%04x)' % a
                else:
                    warn.append('%s %d: access %04x not modelled' % (name, k, a))
                    continue
                out += blend(i, lambda: ['          n%d = %s;' % (o['a'], src)], {o['a']})
            else:
                v = 'r%d' % (o['a'] - 8)
                if a < 0x2000:
                    dst = 'c_%03x' % (a & 0x7FF)
                    out.append('        %s = %s;' % (dst, v) if w is None else
                               '        %s = %s + (%s) * (%s - %s);' % (dst, dst, w, v, dst))
                elif 0xA000 <= a < 0xC000:
                    val = v if w is None else 'K5_%04x + (%s) * (%s - K5_%04x)' % (a, w, v, a)
                    out.append('        K5_%04x = %s; KOUT(0x%04x, K5_%04x);' % (a, val, a, a))
                else:
                    warn.append('%s %d: access %04x not modelled' % (name, k, a))
    for g in used_g:                               # pending of the instruction before -> condition bits
        for c in used_cc:
            if c >> 2 == g:
                out.append('        f%d = f%d + pm%d * (pv%d - f%d);' % (c, c, g, c, c))
    for g in used_g:                               # this instruction's -> pending (a skipped unit sets none)
        if g in newp:
            vv, w = newp[g]
            sv, sz = 's1_sign(%sv)' % vv, 's1_abs(s1_sign(%sz))' % vv
            bits = {0: sz, 1: 'fminf(1.0f, 1.0f + %s)' % sv, 2: '1.0f', 3: '(1.0f - fmaxf(%s, 0.0f) * %s)' % (sv, sz)}
            out.append('        pm%d = %s; %s' % (g, w or '1.0f', ' '.join('pv%d = %s;' % (c, bits[c & 3])
                                                                        for c in used_cc if c >> 2 == g)))
        else:
            out.append('        pm%d = 0.0f;' % g)
    for i in sorted(written):
        out.append('        r%d = n%d;' % (i, i))
    out.append('    }')
    return out


STATE_RE = re.compile(r'\b(r[0-7]|f\d+|pm\d+|pv\d+|c_[0-9a-f]{3})\b')


def kernel_split(name, prog, insns, unit_code, touched, own5, used_cc, used_g, live_r, live_f, warn, bq2,
                 overflow_wrap, K, zero_cells, export_base, ranges=None):
    """--split K: instructions 0..K-1 as NAME_a (S1_DSP_A), K.. as NAME_b (S1_DSP_B). What B reads before writing it
    (registers, condition bits, bank-0 cells) A hands over with s1_export / s1_import (slots export_base..); each
    delay chain D_x belongs to the part that writes x last. A cell A reads from a chain B owns would be a value going
    back from B to A, which SYSTEM-S1CC has no path for: only the zero_cells (known 0 in the caller's setup) may."""
    if live_r or live_f:
        raise SystemExit('%s: registers / condition bits live between samples: no --split' % name)
    blocks = [kernel_insn(k, prog, u, n, unit_code, used_cc, used_g, own5, overflow_wrap, bq2, warn, name)
              for k, (u, n) in enumerate(insns)]
    N = len(blocks)
    writes = [set() for _ in range(N)]
    for k, b in enumerate(blocks):
        for ln in b:
            for m in re.finditer(r'\b(r[0-7]|f\d+|pm\d+|pv\d+|c_[0-9a-f]{3}) = ', ln):
                writes[k].add(m.group(1))
    # what B reads before it writes it (a read and a write in one instruction: the read first)
    imp, defined = [], set()
    for k in range(K, N):
        for ln in blocks[k]:
            for m in STATE_RE.finditer(ln):
                v = m.group(1)
                if v not in defined and v not in imp and not re.match(r'\s*' + v + r' = ', ln[m.start():]):
                    if not (ln[m.start():].startswith(v + ' = ') and v not in ln[m.end():]):
                        imp.append(v)
        defined |= writes[k]
    cellsA = {a for k in range(K) for a in [int(v[2:], 16) for v in writes[k] if v.startswith('c_')]}
    readA = set()
    for k in range(K):
        readA |= {int(v, 16) for ln in blocks[k] for v in re.findall(r'\bc_([0-9a-f]{3})\b', ln)}
    cellsB = {a for k in range(K, N) for a in [int(v[2:], 16) for v in writes[k] if v.startswith('c_')]}
    owner = {}                                     # the part that writes the cell last; a cell nobody writes only
    for a in sorted(touched):                      # carries its delay source along: that source's part
        if a in cellsB:
            owner[a] = 'B'
        elif a in cellsA:
            owner[a] = 'A'
        else:
            owner[a] = next((owner[a - d] for d in range(1, 64) if a - d in touched), 'A')
    need = {}
    for a in touched:
        for d in range(1, 64):
            if a - d in touched:
                need[a - d] = max(need.get(a - d, 0), d)
                break

    def livein(a, part):
        for d in range(1, 64):
            if a - d in touched:
                if owner[a - d] != part:
                    if a in zero_cells:
                        return '0.0f /* zero cell */'
                    raise SystemExit('%s: --split %d: cell %03x of part %s comes from %03x of the other part '
                                     '(a value back from B to A): not possible' % (name, K, a, part, a - d))
                return 'D_%03x_%d' % (a - d, d)
        return '0.0f'
    def src_owner(a):
        for d in range(1, 64):
            if a - d in touched:
                return owner[a - d]
        return None
    # a cell B reads before writing: from A when A wrote it this sample or its delay source is A's; otherwise B
    # takes it from its own delay chain
    imp = [v for v in imp if not v.startswith('c_') or int(v[2:], 16) in cellsA or src_owner(int(v[2:], 16)) == 'A']
    impc = [int(v[2:], 16) for v in imp if v.startswith('c_')]
    # --import-ranges: a value that kept one value in every measured scenario is that constant in B (not handed
    # over); the others get their measured range, widened by a quarter (s1cc scales a value by its range)
    ranges = ranges or {}
    const = {v: ranges[v][0] for v in imp if v in ranges and ranges[v][0] == ranges[v][1]}
    hand = [v for v in imp if v not in const]
    out = ['/* %s: written by sdk/s1cc/tools/dsplift.py --kernel --split %d (exact semantics without the RAW rounding) */'
           % (name, K)]
    for a in sorted(need):
        out.append('static float %s;   /* part %s */' % (', '.join('D_%03x_%d' % (a, kk) for kk in range(1, need[a] + 1)),
                                                     owner[a]))
    for a in sorted(own5):
        out.append('static float K5_%04x;' % a)
    out.append('/* the parts\' arguments: what the including file\'s KIN / KP / KOUT use (default: none) */')
    out.append('#ifndef KARGS_A\n#define KARGS_A void\n#endif\n#ifndef KARGS_B\n#define KARGS_B void\n#endif')
    out.append('/* handed from part A to part B: s1_export / s1_import slots %d.. */' % export_base)
    out.append('#define %s_SLOTS %d' % (name.upper(), len(hand)))
    for part, lo, hi in (('a', 0, K), ('b', K, N)):
        P = part.upper()
        out.append('static inline __attribute__((always_inline)) void %s_%s(KARGS_%s)' % (name, part, P))
        out.append('{')
        if part == 'a':
            out.append('    float %s;' % ', '.join('r%d = 0.0f' % i for i in range(8)))
            for c in used_cc:
                out.append('    float f%d = 0.0f;' % c)
            for g in used_g:
                out.append('    float pm%d = 0.0f, %s;' % (g, ', '.join('pv%d = 0.0f' % c for c in used_cc if c >> 2 == g)))
            for a in sorted((readA | cellsA | set(impc) | {x for x in need if owner[x] == 'A'}) & touched):
                out.append('    float c_%03x = %s;' % (a, livein(a, 'A')))
        else:
            slot = {v: export_base + i for i, v in enumerate(hand)}

            def imp_expr(v, cast=''):
                if v in const:
                    return '%s%r' % (cast, const[v]) + ('f' if not cast else '')
                e = 's1_import(%d)' % slot[v]
                if v in ranges:
                    lo, hi = ranges[v]
                    m = 0.25 * (hi - lo) + 1e-3
                    e = 's1_range(%s, %rf, %rf)' % (e, lo - m, hi + m)
                return cast + e
            slot.update({v: None for v in const})
            for i in range(8):
                v = 'r%d' % i
                out.append('    float %s = %s;' % (v, imp_expr(v) if v in slot else '0.0f'))
            for c in used_cc:
                v = 'f%d' % c
                out.append('    float %s = %s;' % (v, imp_expr(v) if v in slot else '0.0f'))
            for g in used_g:
                parts = ['pm%d = %s' % (g, imp_expr('pm%d' % g) if 'pm%d' % g in slot else '0.0f')]
                parts += ['pv%d = %s' % (c, imp_expr('pv%d' % c) if 'pv%d' % c in slot else '0.0f')
                          for c in used_cc if c >> 2 == g]
                out.append('    float %s;' % ', '.join(parts))
            readB = set()
            for k in range(K, N):
                readB |= {int(v, 16) for ln in blocks[k] for v in re.findall(r'\bc_([0-9a-f]{3})\b', ln)}
            for a in sorted(readB | cellsB):
                v = 'c_%03x' % a
                if v in slot:
                    init = imp_expr(v)
                elif src_owner(a) == 'B' or src_owner(a) is None:
                    init = livein(a, 'B')
                else:
                    init = '0.0f /* written before read */'
                out.append('    float %s = %s;' % (v, init))
        for k in range(lo, hi):
            out += blocks[k]
        if part == 'a':
            for i, v in enumerate(hand):
                out.append('    s1_export(%d, %s);' % (export_base + i, v))
        for a in sorted(need):
            if owner[a] == P:
                for kk in range(need[a], 1, -1):
                    out.append('    D_%03x_%d = D_%03x_%d;' % (a, kk, a, kk - 1))
                out.append('    D_%03x_1 = c_%03x;' % (a, a))
        out.append('}')
    print('%s: --split %d: %d values A -> B (%s), %d constants' % (name, K, len(hand), ' '.join(hand), len(const)),
          file=sys.stderr)
    return out


def kernel(name, words, warn, bq2=None, base=0, overflow_wrap=(), split=None, zero_cells=(), export_base=0,
           ranges=None):
    """one straight-line program (no jumps) as s1cc-ready C: `static inline void NAME(void)`, no loops and no arrays.
    Registers are named values; condition bits only for the groups a condition reads; bank-0 words are the ring
    resolved into delays (a word stored at X is read at X + 1 in the next sample): c_x = the cell this sample,
    D_x_k = what address x held k samples ago. Registers and condition bits read before they are written are kept
    from the sample before (static), as on the DSP. Parameters, bank-5 inputs / outputs and bank 7 go through the
    macros KP(a), KIN(a), KOUT(a, v), KE(a) of the including file; bank-5 words the program writes are its own state."""
    prog = chain.Program(words, name, strict=False)
    dec = dspcomp.decode_program(prog)
    kfun = {K_A: unit_a, K_B: unit_b, K_M: unit_m}
    touched, own5 = set(), set()
    insns = []                                        # per instruction: (units, normal conditions)
    for k, x in enumerate(dec):
        if x is None:
            raise SystemExit('%s: instruction %d undecodable: no --kernel' % (name, k))
        ops, conds = x
        ins = prog.ins[k]
        for o in ops:
            if o['kind'] == K_J:
                raise SystemExit('%s: jumps: no --kernel' % name)
            if o['kind'] in (K_X28, K_XB5, K_XTAP, K_HID):
                raise SystemExit('%s: indexed transfers / taps / hidden registers: no --kernel' % name)
        if bq2 is not None and ins[0] >> 5 == 0x108:
            ops, cw = bq2_mmcnn(ins)
            units, normal = [('op', o) for o in ops], [cw]
        elif bq2 is not None:
            units, normal, _ = bq2_units(ins, ops, conds, warn, '%s %d' % (name, k))
        else:
            units = [('op', o) for o in ops]
            normal = [(cc, neg, ot + 1 if ot >= 0 else 0, of + 1 if of >= 0 else 0) for cc, neg, ot, of in conds]
        for uk, o in units:
            if uk == 'op' and o['kind'] == K_XMEM:
                if o['b'] < 0x2000:
                    touched.add(o['b'] & 0x7FF)
                if 0xA000 <= o['b'] < 0xC000 and o['a'] >= 8:
                    own5.add(o['b'])
        insns.append((units, normal))
    used_cc = sorted({c[0] for _, normal in insns for c in normal})
    used_g = sorted({c >> 2 for c in used_cc})

    # unit code on named registers
    def unit_code(o):
        st = []
        for ln in kfun[o['kind']](o):
            mm = re.match(r'SFU\((0x[0-9a-f]+)\);', ln)
            st += sfu_kernel(int(mm.group(1), 16)) if mm else [ln]
        txt = ' '.join(st)
        txt = re.sub(r'(?<![\w.])(-?0x[0-9a-f]\.[0-9a-f]+p[+-]\d+)', r'\1f', txt)   # float constants (s1cc: float)
        txt = re.sub(r'(?<![\w.])([0-9]+\.[0-9]+)(?![0-9fpe])', r'\1f', txt)
        txt = re.sub(r'\bn\[(\d)\]', r'n\1', txt)
        txt = re.sub(r'\br\[(\d)\]', r'r\1', txt)
        if 'M->' in txt or 'DIVSEED' in txt or 'FREXP' in txt or 'BITOP' in txt or 'SHIFT' in txt:
            raise SystemExit('%s: unit %04x not in --kernel' % (name, o['w']))
        return txt

    # registers and condition bits read before written: static (they come from the sample before)
    seen_r, live_r = set(), set()
    seen_f, live_f = set(), set()
    for units, normal in insns:
        for c in normal:
            if c[0] not in seen_f:
                live_f.add(c[0])
        for uk, o in units:
            if uk != 'op':
                continue
            if o['kind'] in (K_A, K_B, K_M):
                txt = unit_code(o)
                for rr in re.findall(r'\br(\d)\b', txt):
                    if int(rr) not in seen_r:
                        live_r.add(int(rr))
            elif o['kind'] == K_XMEM and o['a'] >= 8 and o['a'] - 8 not in seen_r:
                live_r.add(o['a'] - 8)
        for uk, o in units:
            if uk == 'op' and o['kind'] in (K_A, K_B, K_M):
                seen_r |= {int(x) for x in re.findall(r'\bn(\d)\b', unit_code(o))}
            if uk == 'op' and o['kind'] == K_XMEM and o['a'] < 8:
                seen_r.add(o['a'])
        nam = nb = 0
        for uk, o in units:
            if uk == 'op' and o['kind'] in (K_A, K_M):
                nam += 1
                if nam <= 2:
                    seen_f |= {4 * nam + i for i in range(4)}
            if uk == 'op' and o['kind'] == K_B:
                nb += 1
                if nb <= 2:
                    seen_f |= {4 * (2 + nb) + i for i in range(4)}
    live_f = {c for c in live_f if c in used_cc}

    out = ['/* %s: written by sdk/s1cc/tools/dsplift.py --kernel (exact semantics without the RAW rounding) */' % name]
    if split is not None:
        return kernel_split(name, prog, insns, unit_code, touched, own5, used_cc, used_g, live_r, live_f, warn, bq2,
                            overflow_wrap, split, set(zero_cells), export_base, ranges)
    need = {}
    for a in touched:
        for d in range(1, 64):
            if a - d in touched:
                need[a - d] = max(need.get(a - d, 0), d)
                break
    for a in sorted(need):
        out.append('static float %s;' % ', '.join('D_%03x_%d' % (a, k) for k in range(1, need[a] + 1)))
    for a in sorted(own5):
        out.append('static float K5_%04x;' % a)
    if live_r:
        out.append('static float %s;            /* registers read before written: from the sample before */'
                   % ', '.join('%s_r%d' % (name, i) for i in sorted(live_r)))
    if live_f:
        out.append('static float %s;' % ', '.join('%s_f%d' % (name, c) for c in sorted(live_f)))
    out.append('static inline __attribute__((always_inline)) void %s(void)' % name)
    out.append('{')
    out.append('    float %s;' % ', '.join(('r%d = %s_r%d' % (i, name, i)) if i in live_r else 'r%d = 0.0f' % i
                                         for i in range(8)))
    for c in used_cc:
        out.append('    float f%d = %s;' % (c, ('%s_f%d' % (name, c)) if c in live_f else '0.0f'))
    for g in used_g:                                   # the pending bits of the instruction before
        out.append('    float pm%d = 0.0f, %s;' % (g, ', '.join('pv%d = 0.0f' % c for c in used_cc if c >> 2 == g)))
    for a in sorted(touched):
        live = None
        for d in range(1, 64):
            if a - d in touched:
                live = 'D_%03x_%d' % (a - d, d)
                break
        out.append('    float c_%03x = %s;' % (a, live or '0.0f'))
    for k, (units, normal) in enumerate(insns):
        out.append('    /* %d: %s */' % (k, ' '.join('%04x' % w for w in prog.ins[k])))
        out.append('    {')
        written = set()
        for uk, o in units:
            if uk == 'op' and o['kind'] in (K_A, K_B, K_M):
                written |= {int(x) for x in re.findall(r'\bn(\d)\b', unit_code(o))}
            if uk == 'op' and o['kind'] == K_XMEM and o['a'] < 8:
                written.add(o['a'])
        if written:
            out.append('        float %s;' % ', '.join('n%d = r%d' % (i, i) for i in sorted(written)))
        skips = []
        for cc, neg, T, F in normal:
            cnd = '(f%d != %d)' % (cc, int(neg))
            if 1 <= T <= len(units) or 1 <= F <= len(units):
                skips.append((cnd, T, F))
        def guard(i):
            g = []
            for cnd, T, F in skips:
                if T == i + 1:
                    g.append('!%s' % cnd)
                if F == i + 1:
                    g.append('%s' % cnd)
            return ('if (%s) ' % ' && '.join(g)) if g else ''
        newp = {}
        nam = nb = 0
        for i, (uk, o) in enumerate(units):
            if uk == 'ctl':
                out.append('        /* control word %x%x: no effect */' % (o[2], o[3]))
                continue
            kind = o['kind']
            gd = guard(i)
            if kind in (K_A, K_B, K_M):
                txt = unit_code(o)
                if k in overflow_wrap:
                    txt = txt.replace('AWRAP(', 'KOVERFLOW(')
                dest = (o['w'] >> 12) & 7
                grp = None
                if bq2 is not None and not bq2_flagless(o):
                    if kind == K_B:
                        nb += 1
                        grp = 2 + nb if nb <= 2 else None
                    else:
                        nam += 1
                        grp = nam if nam <= 2 else None
                fl = ''
                if grp in used_g:
                    vv = 'u%d_%d' % (k, i)
                    fl = (' %sv = rs ? rvv : n%d; %sz = rs ? rzz : n%d == 0;' % (vv, dest, vv, dest))
                    out.append('        float %sv = 0.0f; int %sz = 0, %se = 0;' % (vv, vv, vv))
                    fl += ' %se = 1;' % vv
                    newp[grp] = vv
                out.append('        %s{ int rs = 0; float rvv = 0; int rzz = 0; (void)rs, (void)rvv, (void)rzz; %s%s }'
                           % (gd, txt, fl))
            elif kind == K_XFLAG:
                raise SystemExit('%s %d: X9bY not in --kernel' % (name, k))
            elif kind == K_XMEM:
                a = o['b']
                if a < 0x2000:
                    if o['a'] < 8:
                        out.append('        %sn%d = c_%03x;' % (gd, o['a'], a & 0x7FF))
                    else:
                        out.append('        %sc_%03x = r%d;' % (gd, a & 0x7FF, o['a'] - 8))
                elif 0xA000 <= a < 0xC000:
                    if o['a'] < 8:
                        src = ('K5_%04x' % a) if a in own5 else 'KIN(0x%04x)' % a
                        out.append('        %sn%d = %s;' % (gd, o['a'], src))
                    else:
                        out.append('        %s{ K5_%04x = r%d; KOUT(0x%04x, r%d); }' % (gd, a, o['a'] - 8, a, o['a'] - 8))
                elif 0xC000 <= a < 0xE000 and o['a'] < 8:
                    out.append('        %sn%d = KP(0x%04x);' % (gd, o['a'], a))
                elif a >= 0xE000 and o['a'] < 8:
                    out.append('        %sn%d = KE(0x%04x);' % (gd, o['a'], a))
                else:
                    warn.append('%s %d: access %04x not modelled' % (name, k, a))
        for g in used_g:                               # pending of the instruction before -> condition bits
            for c in used_cc:
                if c >> 2 == g:
                    out.append('        f%d = pm%d ? pv%d : f%d;' % (c, g, c, c))
        for g in used_g:                               # this instruction's -> pending (a skipped unit sets none)
            if g in newp:
                vv = newp[g]
                bits = {0: '!%sz' % vv, 1: '!(%sv < 0)' % vv, 2: '1', 3: '(%sv < 0 || %sz)' % (vv, vv)}
                out.append('        pm%d = %se; %s' % (g, vv, ' '.join('pv%d = %s;' % (c, bits[c & 3])
                                                                     for c in used_cc if c >> 2 == g)))
            else:
                out.append('        pm%d = 0;' % g)
        for i in sorted(written):
            out.append('        r%d = n%d;' % (i, i))
        out.append('    }')
    for i in sorted(live_r):
        out.append('    %s_r%d = r%d;' % (name, i, i))
    for c in sorted(live_f):
        out.append('    %s_f%d = f%d;' % (name, c, c))
    for a in sorted(need):
        for kk in range(need[a], 1, -1):
            out.append('    D_%03x_%d = D_%03x_%d;' % (a, kk, a, kk - 1))
        out.append('    D_%03x_1 = c_%03x;' % (a, a))
    out.append('}')
    return out


def main():
    ap = argparse.ArgumentParser(description='DSP microcode -> plain C (study tool)')
    ap.add_argument('out', help='the C file to write')
    ap.add_argument('progs', nargs='+', help='NAME=PROGRAM.hex[@ADDRESS]: C function name, the program words, and '
                    '(--bq2) its byte address in program memory')
    ap.add_argument('--skip-lead', action='store_true', help='skip a leading run of 0000 / 0001 data words')
    ap.add_argument('--bq2', action='store_true', help='Boutique BQ2 DSP: implicit flags, control words (§5)')
    ap.add_argument('--check', action='store_true', help='compare the lift with the emulator (random inputs)')
    ap.add_argument('--kernel', action='store_true', help='write s1cc-ready C instead (straight-line programs, §6)')
    ap.add_argument('--split', type=int, default=None, metavar='K',
                    help='--kernel: instructions 0..K-1 as NAME_a (DSP A), the rest as NAME_b (DSP B), §6')
    ap.add_argument('--zero-cells', default='', metavar='X,X,...',
                    help='--split: bank-0 cells (hex) part A may read as 0 (their source is in part B and is 0 in the '
                         'setup used)')
    ap.add_argument('--export-base', type=int, default=0, help='--split: first s1_export slot (0)')
    ap.add_argument('--import-ranges', default=None, metavar='FILE',
                    help='--split: "NAME LO HI" per handed value, measured: constants stay in B, the rest get the range')
    ap.add_argument('--overflow-wrap', default='', metavar='K,K,...',
                    help='--kernel: instructions whose A-unit result reaches +-512 and must wrap as on the DSP (the '
                         'others are plain arithmetic); find them with the lift (§6)')
    ap.add_argument('--samples', type=int, default=64, help='samples per program in --check (64)')
    ap.add_argument('--seed', type=int, default=1, help='random seed of --check (1)')
    a = ap.parse_args()
    global BQ2
    BQ2 = a.bq2
    if a.bq2 and a.check:
        ap.error('--check compares with the emulator, which has no BQ2 model')
    out = ['/* written by sdk/s1cc/tools/dsplift.py%s: %s */' % (' --bq2' if a.bq2 else '', ' '.join(a.progs)),
           '#include "dsplift_rt.h"', '']
    if a.bq2:
        out += ['static void CALL(LiftM *M, int byte_address);', '']
    warn, progs, addr = [], [], {}
    if a.kernel:
        out = ['/* written by sdk/s1cc/tools/dsplift.py --kernel%s: %s */' % (' --bq2' if a.bq2 else '', ' '.join(a.progs)),
               '#include "dsplift_k.h"', '']
        for spec in a.progs:
            name, path = spec.split('=', 1)
            path = path.rsplit('@', 1)[0]
            out += kernel(name, read_words(path, a.skip_lead), warn, 0 if a.bq2 else None,
                          overflow_wrap={int(x) for x in a.overflow_wrap.split(',') if x}, split=a.split,
                          zero_cells={int(x, 16) for x in a.zero_cells.split(',') if x},
                          export_base=a.export_base,
                          ranges={l.split()[0]: (float(l.split()[1]), float(l.split()[2]))
                                  for l in open(a.import_ranges) if l.strip()} if a.import_ranges else None) + ['']
        open(a.out, 'w').write('\n'.join(out) + '\n')
        for w in warn:
            print('warning: ' + w, file=sys.stderr)
        print('%s: %d programs (kernel)' % (a.out, len(a.progs)))
        return
    for spec in a.progs:
        name, path = spec.split('=', 1)
        base = None
        if '@' in path:
            path, b = path.rsplit('@', 1)
            base = int(b, 0)
        if a.bq2 and base is None:
            ap.error('--bq2 needs NAME=FILE@ADDRESS for %s' % name)
        progs.append((name, lift(name, read_words(path, a.skip_lead), out, warn, base if a.bq2 else None)))
        if base is not None:
            addr[base] = name
    if a.bq2:
        out.append('static void CALL(LiftM *M, int byte_address)')
        out.append('{')
        out.append('    switch (byte_address) {')
        for b, nm in sorted(addr.items()):
            out.append('    case 0x%04x: %s(M); break;' % (b, nm))
        out.append('    default: fprintf(stderr, "call to 0x%04x: no program there\\n", byte_address); abort();')
        out.append('    }')
        out.append('}')
    open(a.out, 'w').write('\n'.join(out) + '\n')
    for w in warn:
        print('warning: ' + w, file=sys.stderr)
    print('%s: %d programs' % (a.out, len(progs)))
    if a.check:
        sys.exit(1 if check(a.out, progs, a.samples, a.seed) else 0)


if __name__ == '__main__':
    main()
