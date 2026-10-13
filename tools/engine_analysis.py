"""
engine_analysis.py -- static analysis of SYSTEM-8 engine images (Thumb-2).

Used by base-engine/ReadMe + PLUGOUT_VS_BASE.md. Recovers functions from the
vtable methods (sys8fw symbols), follows direct calls, and classifies what
each function does: parameter translation (reads a value, maps it, sends it
to the DSP through the OS table) versus real computation (loops, float math,
persistent state).

    python3 tools/engine_analysis.py base-engine/up base-engine/report/symbols.json
"""
from __future__ import annotations

import json
import os
import struct
import sys
from collections import Counter, defaultdict

import capstone as cs
from capstone import arm_const as ac

OS_TABLE = 0x0113FC00
MD = cs.Cs(cs.CS_ARCH_ARM, cs.CS_MODE_THUMB | cs.CS_MODE_MCLASS)
MD.detail = True

TERMINAL_BRANCH = {"b", "b.w", "bx"}


class Image:
    def __init__(self, data: bytes, base: int):
        self.d, self.base = data, base
        self.end = base + len(data)

    def u32(self, a):
        o = a - self.base
        return struct.unpack_from("<I", self.d, o)[0] if 0 <= o <= len(self.d) - 4 else None

    def f32(self, a):
        v = self.u32(a)
        return None if v is None else struct.unpack("<f", struct.pack("<I", v))[0]

    def inside(self, a):
        return self.base <= a < self.end


class Func:
    __slots__ = ("addr", "insns", "calls", "indirect", "bl_targets", "size", "lit_refs",
                 "vfp", "vfp_mul_only", "loops", "stores", "loads", "os_refs", "tail")

    def __init__(self, addr):
        self.addr = addr
        self.insns = []
        self.calls = []
        self.indirect = 0
        self.bl_targets = set()
        self.lit_refs = []
        self.vfp = Counter()
        self.loops = 0
        self.stores = 0
        self.loads = 0
        self.os_refs = set()
        self.tail = None
        self.size = 0


def analyse_function(img: Image, entry: int, limit=4000) -> Func:
    """Recursive descent over basic blocks of one function (Thumb)."""
    f = Func(entry)
    seen, work = set(), [entry]
    back_edges = 0
    maxaddr = entry
    while work:
        a = work.pop()
        while a not in seen and img.inside(a) and len(seen) < limit:
            seen.add(a)
            o = a - img.base
            chunk = img.d[o:o + 4]
            ins = next(MD.disasm(chunk, a, 1), None)
            if ins is None:
                break
            f.insns.append(ins)
            maxaddr = max(maxaddr, a + ins.size)
            m = ins.mnemonic
            if m.startswith("v"):
                f.vfp[m.split(".")[0]] += 1
            if ins.id in (ac.ARM_INS_STR, ac.ARM_INS_STRB, ac.ARM_INS_STRH, ac.ARM_INS_VSTR,
                          ac.ARM_INS_STM, ac.ARM_INS_STRD):
                f.stores += 1
            if ins.id in (ac.ARM_INS_LDR, ac.ARM_INS_LDRB, ac.ARM_INS_LDRH, ac.ARM_INS_VLDR,
                          ac.ARM_INS_LDRD, ac.ARM_INS_LDRSH, ac.ARM_INS_LDRSB):
                f.loads += 1
                # literal pool
                for op in ins.operands:
                    if op.type == ac.ARM_OP_MEM and op.mem.base == ac.ARM_REG_PC:
                        la = ((a + 4) & ~3) + op.mem.disp
                        v = img.u32(la)
                        f.lit_refs.append((la, v))
                        if v is not None and OS_TABLE <= v < OS_TABLE + 0x200:
                            f.os_refs.add(v - OS_TABLE)
            if m in ("bl", "blx") and ins.operands and ins.operands[0].type == ac.ARM_OP_IMM:
                t = ins.operands[0].imm
                f.calls.append(t)
            elif m == "blx" or (m == "bx" and ins.operands and ins.operands[0].reg != ac.ARM_REG_LR):
                f.indirect += 1
            # control flow
            if m in ("bx",) and ins.operands and ins.operands[0].reg == ac.ARM_REG_LR:
                break
            if m == "pop" and any(op.reg == ac.ARM_REG_PC for op in ins.operands):
                if ins.cc == ac.ARM_CC_AL or ins.cc == 0:
                    break
            if m.startswith("b") and m not in ("bl", "blx", "bx", "bic", "bics", "bfc", "bfi",
                                               "bkpt") and ins.operands and \
                    ins.operands[0].type == ac.ARM_OP_IMM:
                t = ins.operands[0].imm
                if t < a:
                    back_edges += 1
                if img.inside(t) and entry <= t < entry + 0x4000:
                    work.append(t)
                else:
                    f.tail = t
                if m in ("b", "b.w", "b.n"):
                    break
            if m in ("cbz", "cbnz") and ins.operands:
                t = ins.operands[-1].imm
                if t < a:
                    back_edges += 1
                work.append(t)
            if m in ("tbb", "tbh"):
                break
            a += ins.size
    f.loops = back_edges
    f.size = maxaddr - entry
    f.insns.sort(key=lambda i: i.address)
    return f


def classify(f: Func) -> str:
    """translator | computing | stub | other"""
    n = len(f.insns)
    if n <= 2:
        return "stub"
    arith = sum(v for k, v in f.vfp.items()
                if k in ("vadd", "vsub", "vmul", "vdiv", "vmla", "vmls", "vsqrt", "vneg", "vabs",
                         "vfma", "vcmp", "vcvt", "vmax", "vmin", "vnmul", "vfnma", "vnmla"))
    if f.loops or arith >= 6 or n > 60:
        return "computing"
    if f.os_refs or f.tail is not None or f.calls or f.indirect:
        return "translator"
    return "other"


# ---------------------------------------------------------------------------
# normalised function hashes (for cross-engine comparison)
# ---------------------------------------------------------------------------
import hashlib
import re

_HEX = re.compile(r"0x[0-9a-f]+|#-?\d+")


def norm_insn(ins) -> str:
    m, o = ins.mnemonic, ins.op_str
    if "[pc" in o:                       # literal pool load: value is address-dependent
        return m + " pc-rel"
    if m.startswith("b") and m not in ("bic", "bics", "bfc", "bfi", "bx", "bkpt") and ins.operands \
            and ins.operands[0].type == ac.ARM_OP_IMM:
        return m + " target"
    def keep(mo):
        t = mo.group(0)
        v = int(t.lstrip("#"), 0) if not t.startswith("0x") else int(t, 16)
        return t if abs(v) < 0x100 else "#big"
    return m + " " + _HEX.sub(keep, o)


def func_hash(f: Func) -> str:
    h = hashlib.sha1()
    for i in f.insns:
        h.update(norm_insn(i).encode())
        h.update(b";")
    return h.hexdigest()[:16]


def all_functions(img: Image, entries, depth=1):
    """Analyse vtable entries and the functions they call directly (depth levels)."""
    funcs, todo = {}, [(e & ~1, 0) for e in entries]
    while todo:
        a, dp = todo.pop()
        if a in funcs or not img.inside(a):
            continue
        f = analyse_function(img, a)
        funcs[a] = f
        if dp < depth:
            for t in f.calls:
                todo.append((t & ~1, dp + 1))
    return funcs


def find_constructors(img: Image, vtable: int) -> list[int]:
    """Functions that load `vtable` from a literal pool (C++ constructors)."""
    lits = [img.base + o for o in range(0, len(img.d) - 3, 4)
            if struct.unpack_from("<I", img.d, o)[0] == vtable]
    out = []
    for L in lits:
        for A in range(L - 0x800, L, 2):
            if A < img.base:
                continue
            o = A - img.base
            ins = next(MD.disasm(img.d[o:o + 4], A, 1), None)
            if ins is None or ins.id != ac.ARM_INS_LDR:
                continue
            ops = ins.operands
            if len(ops) == 2 and ops[1].type == ac.ARM_OP_MEM and ops[1].mem.base == ac.ARM_REG_PC:
                if ((A + 4) & ~3) + ops[1].mem.disp == L:
                    # walk back to a push {..., lr}
                    s = A
                    while s > A - 0x80:
                        o2 = s - img.base
                        i2 = next(MD.disasm(img.d[o2:o2 + 4], s, 1), None)
                        if i2 and i2.mnemonic in ("push", "push.w", "stmdb") and "lr" in i2.op_str:
                            out.append(s)
                            break
                        s -= 2
    return sorted(set(out))
