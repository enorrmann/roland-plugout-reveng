#!/usr/bin/env python3
"""
engine_params.py -- list the parameter ids a "flat" PLUG-OUT engine registers
(SYSTEM-100, SH-101: one CPrmDSP* class, no voice classes) and which of them
have a real handler.

    python3 tools/engine_params.py ENGINE.region1.bin CPrmDSPSystem100 [--base 0x60A00000]

The engine's parameter class has a "register all ids" method (the largest
function with no loops, ~5000 instructions) that calls, for every id, a getter
(vtable +0x58/+0x5c) and a setter (+0x30).  The setter dispatch method (the one
with a binary decision tree of `cmp r1, #id`) stores the value at an object
offset and calls one virtual slot per id; a slot that is only `bx lr` is a
no-op in this class.  Both are driven under unicorn with a fake vtable whose
entries are trapped.  Needs plain `python3` (capstone/unicorn), not `-I`.
"""
from __future__ import annotations

import argparse
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import engine_analysis as EA  # noqa: E402
import engine_emu as M        # noqa: E402
import sys8fw as F            # noqa: E402
import unicorn                # noqa: E402

FAKE = 0x400          # fake-vtable entries live at TRAP + FAKE + 4*slot


def ranges(values) -> str:
    v = sorted(set(values))
    out, s, p = [], None, None
    for x in v:
        if s is None:
            s = p = x
        elif x == p + 1:
            p = x
        else:
            out.append((s, p)); s = p = x
    if s is not None:
        out.append((s, p))
    return " ".join(str(a) if a == b else f"{a}-{b}" for a, b in out)


class FlatEngine:
    def __init__(self, path: str, cname: str, base: int = 0x60A00000):
        self.d = open(path, "rb").read()
        self.base = base
        self.img = EA.Image(self.d, base)
        cls = F.find_classes(self.d, base)
        self.cls = next(c for c in cls if c["name"] == cname)
        self.m = self.cls["methods"]
        self.sizes = [len(EA.analyse_function(self.img, a & ~1, limit=20000).insns)
                      for a in self.m]
        self.emu = M.Emu(self.d, base, extra=0x400000)
        self.obj = self.emu.alloc(0x4000)
        self.vt = self.emu.alloc(0x1000)
        for i in range(0x400):
            self.emu.w32(self.vt + 4 * i, (M.TRAP + FAKE + 4 * i) | 1)

    def _run(self, meth, *args, maxinsn=3_000_000):
        e = self.emu
        e.uc.mem_write(self.obj, bytes(0x4000))
        e.w32(self.obj, self.vt)
        try:
            return e.call(self.m[meth] & ~1, self.obj, *args, maxinsn=maxinsn)
        except unicorn.UcError:
            return list(e.calls)

    def register_method(self) -> int:
        """The loop-free method with the most instructions."""
        best, n = None, 0
        for i, a in enumerate(self.m):
            f = EA.analyse_function(self.img, a & ~1, limit=20000)
            if f.loops == 0 and len(f.insns) > n:
                best, n = i, len(f.insns)
        return best

    def setter_method(self, probe_id: int) -> int:
        """Among the methods with the biggest `cmp r1, #imm` decision trees (setter
        and getter look alike), the one that calls a virtual slot for `probe_id`."""
        scored = []
        for i, a in enumerate(self.m):
            f = EA.analyse_function(self.img, a & ~1, limit=20000)
            c = sum(1 for x in f.insns if x.mnemonic.startswith("cmp") and "r1," in x.op_str)
            scored.append((c, len(f.insns), i))
        # the getter shares the decision tree but is shorter than the setter
        top = [t for t in sorted(scored, reverse=True)[:4] if t[0] > 10
               if self._run(t[2], probe_id, 0, 5, maxinsn=200000)]
        if not top:
            raise RuntimeError("no setter found")
        return max(top, key=lambda t: t[1])[2]

    def ids(self) -> list[int]:
        calls = self._run(self.register_method(), 1)
        # getter calls are (r1 = 0, r2 = id)
        offs = {o for o, *_ in calls}
        getter = min(o for o in offs if o != 0x430)
        return [r2 for o, r0, r1, r2, *_ in calls if o == getter]

    def handler(self, pid: int):
        """(slot, instruction count) of the virtual slot the setter calls for `pid`."""
        if not hasattr(self, "_setter"):
            self._setter = self.setter_method(pid)
        calls = self._run(self._setter, pid, 0, 5, maxinsn=200000)
        out = []
        for o, *_ in calls:
            s = (o - FAKE) // 4
            out.append((s, self.sizes[s] if s < len(self.sizes) else None))
        return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("image")
    ap.add_argument("cls")
    ap.add_argument("--base", type=lambda x: int(x, 0), default=0x60A00000)
    a = ap.parse_args()
    fe = FlatEngine(a.image, a.cls, a.base)
    ids = fe.ids()
    real = [p for p in ids if any(sz and sz > 2 for _, sz in fe.handler(p))]
    print(f"{a.cls}: {len(fe.m)} virtual slots, {sum(1 for s in fe.sizes if s > 2)} non-stub")
    print(f"ids registered: {len(ids)}  with a real handler: {len(real)}")
    print("ids:", ranges(ids))
    print("ids with a real handler:", ranges(real))


if __name__ == "__main__":
    main()
