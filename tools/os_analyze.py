#!/usr/bin/env python3
"""
os_analyze.py -- static analysis of the decrypted SYSTEM-8 OS image
(dumps/os_full.bin, run address 0x60000000, docs/BASE_FIRMWARE.md §7.14/§8).

Linear Thumb-2 sweep with capstone, then:
  * functions  = targets of BL (and of the scatter-load / reset entry),
                 each function ends where the next one starts;
  * per function: callees, callers, constants loaded from literal pools
    (LDR pc-relative) or built with MOVW/MOVT, split into
    strings (pointer to a NUL-terminated string in the image),
    MMIO (0x40000000-0x5FFFFFFF, 0xE0000000+), RAM and code pointers.

    python3 tools/os_analyze.py dumps/os_full.bin -o work/os
        -> work/os/functions.json, work/os/report.txt, work/os/os_full.elf
    python3 tools/os_analyze.py dumps/os_full.bin -o work/os --parts
        -> also splits the image into the per-CPU programs (PARTS, §8.1) and
           analyses each one at its own link address: work/os/<part>/...

The ELF (one section at 0x60000000, function + string symbols) loads in
Ghidra/IDA as ARM Cortex-M little-endian Thumb.
"""
from __future__ import annotations

import argparse
import bisect
import collections
import json
import os
import re
import struct
import sys

import capstone as cs
from capstone import arm as A

BASE = 0x60000000
# (name, offset in the image, size, link/run address) -- docs/BASE_FIRMWARE.md §8.1
PARTS = [
    ("m0_core0", 0x000000, 0x130E00, 0x60000000),  # master BMC core 0: OS (+ .rodata)
    ("m1_tcm",   0x130E00, 0x042BBC, 0x01100000),  # master BMC core 1: DSP control, code
    ("m1_sdram", 0x173A00, 0x07D74C, 0x61000000),  # master BMC core 1: debug UI / data
    ("s_boot",   0x1F1200, 0x01D328, 0x01100000),  # slave BMCs: 'BOOT' image
    ("s_data",   0x20E600, 0x07D114, 0x60000000),  # slave BMCs: 'DATA' image
    ("subcpu",   0x28B730, 0x004D68, 0x08000000),  # STM32F1 panel/keyboard CPU
]
HERE = os.path.dirname(os.path.abspath(__file__))


def strings_of(img: bytes, base: int) -> dict[int, str]:
    out = {}
    for m in re.finditer(rb"[\x20-\x7e\t\r\n]{4,}\x00", img):
        out[base + m.start()] = m.group()[:-1].decode("latin-1")
    return out


def kind(v: int, base: int, size: int) -> str:
    if base <= v < base + size:
        return "image"
    if 0x40000000 <= v < 0x60000000 or v >= 0xE0000000:
        return "mmio"
    if 0x01000000 <= v < 0x01200000 or 0x20000000 <= v < 0x20200000:
        return "ram"
    if 0x60000000 <= v < 0x70000000:
        return "sdram"
    return "other"


def sweep(img: bytes, base: int):
    md = cs.Cs(cs.CS_ARCH_ARM, cs.CS_MODE_THUMB | cs.CS_MODE_MCLASS)
    md.detail = True
    md.skipdata = True
    calls = []          # (site, target)
    consts = []         # (site, value)
    movw = {}           # reg -> (site, low16)
    for ins in md.disasm(img, base):
        mn = ins.mnemonic
        if mn == ".byte":
            movw.clear()
            continue
        if mn in ("bl", "blx") and ins.operands and ins.operands[0].type == A.ARM_OP_IMM:
            calls.append((ins.address, ins.operands[0].imm))
        elif mn.startswith("ldr") and len(ins.operands) == 2:
            op = ins.operands[1]
            if op.type == A.ARM_OP_MEM and op.mem.base == A.ARM_REG_PC and op.mem.index == 0:
                a = ((ins.address + 4) & ~3) + op.mem.disp
                o = a - base
                if 0 <= o <= len(img) - 4 and mn in ("ldr", "ldr.w"):
                    consts.append((ins.address, struct.unpack_from("<I", img, o)[0]))
        elif mn in ("adr", "adr.w", "addw") and len(ins.operands) >= 2 and ins.operands[-1].type == A.ARM_OP_IMM \
                and (mn != "addw" or ins.operands[1].reg == A.ARM_REG_PC):
            consts.append((ins.address, ((ins.address + 4) & ~3) + ins.operands[-1].imm))
        elif mn in ("movw", "mov.w") and len(ins.operands) == 2 and ins.operands[1].type == A.ARM_OP_IMM:
            movw[ins.operands[0].reg] = (ins.address, ins.operands[1].imm & 0xFFFF)
        elif mn == "movt" and ins.operands[0].reg in movw:
            site, lo = movw.pop(ins.operands[0].reg)
            consts.append((site, (ins.operands[1].imm & 0xFFFF) << 16 | lo))
    return calls, consts


def to_elf(img: bytes, base: int, syms: list[tuple[str, int, bool]], entry: int) -> bytes:
    sys.path.insert(0, HERE)
    import plugout_image as I
    return I.to_elf([(".os", base, img)], syms, entry)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("image", nargs="?", default=os.path.join(HERE, "..", "dumps", "os_full.bin"))
    ap.add_argument("--base", type=lambda x: int(x, 0), default=BASE)
    ap.add_argument("-o", "--out", default=os.path.join(HERE, "..", "work", "os"))
    ap.add_argument("--parts", action="store_true", help="also analyse the per-CPU programs")
    args = ap.parse_args()

    img = open(args.image, "rb").read()
    if args.parts:
        for name, off, size, base in PARTS:
            sub = os.path.join(args.out, name)
            os.makedirs(sub, exist_ok=True)
            path = os.path.join(sub, name + ".bin")
            with open(path, "wb") as fh:
                fh.write(img[off:off + size])
            print(f"== {name}: image {BASE + off:#010x} +{size:#x} -> run {base:#010x}")
            analyse(open(path, "rb").read(), base, sub, name)
    print("== whole image")
    analyse(img, args.base, args.out, "os_full")
    return 0


def analyse(img: bytes, base: int, out: str, name: str) -> None:
    end = base + len(img)
    os.makedirs(out, exist_ok=True)
    strs = strings_of(img, base)
    print(f"{len(img)} bytes, {len(strs)} strings; sweeping ...", flush=True)
    calls, consts = sweep(img, base)

    # functions: BL targets inside the image, called from >= 1 site (and the reset entry)
    tgt = collections.Counter(t for _, t in calls if base <= t < end)
    starts = sorted(set(tgt) | {base})
    callers = collections.defaultdict(set)
    callees = collections.defaultdict(set)

    def owner(a: int) -> int | None:
        i = bisect.bisect_right(starts, a) - 1
        return starts[i] if i >= 0 else None

    for site, t in calls:
        f = owner(site)
        if f is not None:
            callees[f].add(t)
            if base <= t < end:
                callers[t].add(f)
    refs = collections.defaultdict(lambda: collections.defaultdict(set))
    for site, v in consts:
        f = owner(site)
        if f is None:
            continue
        k = kind(v, base, len(img))
        if k == "image":
            if v in strs:
                k = "string"
            elif v & 1 and (v & ~1) in tgt:
                k = "func_ptr"
        refs[f][k].add(v)

    funcs = []
    for i, f in enumerate(starts):
        nxt = starts[i + 1] if i + 1 < len(starts) else end
        r = refs.get(f, {})
        funcs.append({
            "addr": f, "size": nxt - f, "ncallers": len(callers[f]),
            "callers": sorted(callers[f]), "callees": sorted(callees[f]),
            "strings": [strs[s] for s in sorted(r.get("string", ()))],
            "mmio": sorted(r.get("mmio", ())), "ram": sorted(r.get("ram", ())),
            "func_ptrs": sorted(r.get("func_ptr", ())),
            "image_refs": sorted(r.get("image", ())),
        })
    with open(os.path.join(out, "functions.json"), "w") as fh:
        json.dump(funcs, fh)

    # text report
    with open(os.path.join(out, "report.txt"), "w") as fh:
        for f in funcs:
            fh.write(f"{f['addr']:#010x} size {f['size']:#x} callers {f['ncallers']} "
                     f"callees {len(f['callees'])}\n")
            for s in f["strings"][:12]:
                fh.write(f"    str  {s[:70]!r}\n")
            if f["mmio"]:
                fh.write("    mmio " + " ".join(f"{m:#010x}" for m in f["mmio"][:12]) + "\n")
    syms = [(f"fn_{f['addr']:08x}", f["addr"] | 1, True) for f in funcs]
    syms += [(f"s_{a:08x}", a, False) for a in strs]
    with open(os.path.join(out, name + ".elf"), "wb") as fh:
        fh.write(to_elf(img, base, syms, base | 1))
    print(f"   {len(funcs)} functions, {len(calls)} calls, {len(consts)} constants -> {out}")


if __name__ == "__main__":
    sys.exit(main())
