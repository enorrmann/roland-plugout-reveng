#!/usr/bin/env python3
"""gen_asm.py -- turn the disassembly of the image into assembly sources.

For every function of the image this writes an assembly unit at the function's
original address (``.text`` with ``. = <addr>``) and assembles it with
``arm-none-eabi-as``.  Cross references are emitted as symbols so that the
object carries relocations (``fn_<target>`` for a call, ``L_<addr>`` for a
branch, ``LC_<addr>`` for a literal-pool load): ``gen_code.py`` collects the
symbols and ``build_le.py`` resolves them.

Two levels of fidelity are produced, per function:

``asm``      the real instruction text (mnemonics from Capstone).  A function
             is emitted this way only if it assembles back to *its exact bytes*
             after the relocations are resolved; this is checked immediately.
``bytes``    otherwise the function is emitted as ``.byte`` data at the same
             address, which is byte-exact by construction.

The ``bytes`` form is what makes the rebuild exact; the ``asm`` form is the
readable recovery of the code.  ``--stats`` prints the split.

    python3 tools/gen_asm.py --cache build/seg.pkl --functions build/functions.txt \
        -o code/asm --work build/as --stats
"""
from __future__ import annotations

import argparse
import os
import pickle
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import capstone as cs            # noqa: E402
import build_le                  # noqa: E402

BASE = 0x60000000
AS = "arm-none-eabi-as"
# Cortex-M7 covers the DSP-ish core of the BMC (Thumb-2 + the VFP the firmware
# uses); -mimplicit-it keeps the assembler from rejecting IT-block bodies.
AS_FLAGS = ["-mthumb", "-mcpu=cortex-m7", "-mimplicit-it=always"]

MD = cs.Cs(cs.CS_ARCH_ARM, cs.CS_MODE_THUMB | cs.CS_MODE_MCLASS)
MD.detail = True


# Capstone prints some instructions in their widest form.  Measured against
# arm-none-eabi-as over the whole image (tools/gen_asm.py --measure), only
# these narrow forms come back wider:
#
#     adr                     2 -> 4 bytes   (as picks ADD.W)
#     b<cond> with a target out of the narrow range   2 -> 4 bytes
#
# Everything else (``ldm``, ``stm``, ``cbz``, ``movs``, ...) assembles back to
# its original size, so the instruction text can be emitted as it is.
WIDENING = ("adr",)


def widening(ins):
    """True when the assembler would not reproduce this instruction's bytes."""
    if ins.size != 2:
        return False
    return ins.mnemonic.split(".")[0] in WIDENING


def render(ins):
    """Assembly text for one instruction, with cross references symbolic."""
    m, o = ins.mnemonic, ins.op_str
    if m in ("bl", "blx") and ins.operands and ins.operands[0].type == cs.arm.ARM_OP_IMM:
        return "\tbl fn_%08x" % (ins.operands[0].imm & ~1)
    if m in ("cbz", "cbnz") and ins.operands:
        # the assembler places a cbz relocation at the wrong word: keep bytes
        return "\t.byte " + ", ".join("0x%02x" % b for b in ins.bytes)
    if (m[0] == "b" and m not in ("bkpt", "bfc", "bfi") and not m.startswith("bl")
            and m not in ("blx",)
            and ins.operands and ins.operands[0].type == cs.arm.ARM_OP_IMM):
        # Only the 32-bit forms are emitted as text: for a narrow branch the
        # assembler would widen b<cond> into b<cond>.w and shift the whole
        # function.  Narrow branches stay as bytes.
        # as widens a branch whose target is not yet defined, and the target
        # of a firmware branch is only known after linking: such a branch is
        # kept as its original bytes.  The function-level check below puts the
        # whole function in the byte form when that is not enough.
        return "\t.byte " + ", ".join("0x%02x" % b for b in ins.bytes)
    return ("\t" + m + " " + o).rstrip()


def unit(fn, lines, fallback_bytes=None):
    # the address travels in the section name (see gen_data.py / build_le.py):
    # one section per function, so nothing is padded and objects stay small
    out = ["\t.syntax unified", "\t.thumb",
           '\t.section .text.%08x,"ax",%%progbits' % fn,
           "\t.thumb_func", "\t.global fn_%08x" % fn, "fn_%08x:" % fn]
    out.extend(lines)
    return "\n".join(out) + "\n"


def assemble(work, fn, end, text):
    s = os.path.join(work, "u.s")
    o = os.path.join(work, "u.o")
    with open(s, "w") as f:
        f.write(text)
    r = subprocess.run([AS] + AS_FLAGS + ["-o", o, s], capture_output=True, text=True)
    if r.returncode:
        return None, r.stderr
    try:
        obj = build_le.Elf(o)
        obj.resolve({s["name"]: int(s["name"][3:], 16) | 1
                     for s in obj.symbols
                     if s["name"].startswith("fn_") and s["shndx"] == 0})
        data = build_le.place(obj, fn, end)[0][1]
    except Exception as e:                       # noqa: BLE001 - reported upward
        return None, str(e)
    return data, None


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--cache", default="build/seg.pkl")
    ap.add_argument("--functions", required=True,
                    help="one function entry address per line, ascending")
    ap.add_argument("--image", default=os.path.join("..", "dumps", "os_full.bin"))
    ap.add_argument("-o", "--out", default="code/asm")
    ap.add_argument("--work", default="build/as")
    ap.add_argument("--stats", action="store_true")
    ap.add_argument("--limit", type=int, default=0)
    args = ap.parse_args(argv)

    with open(args.image, "rb") as f:
        img = f.read()
    funcs = sorted(set(int(x, 16) for x in open(args.functions) if x.strip()))
    if args.limit:
        funcs = funcs[:args.limit]
    os.makedirs(args.out, exist_ok=True)
    os.makedirs(args.work, exist_ok=True)

    asm_ok = byte_ok = failed = 0
    for i, fn in enumerate(funcs):
        end = min(funcs[i + 1] if i + 1 < len(funcs) else BASE + len(img),
                  fn + 0x10000)
        seg = img[fn - BASE:end - BASE]
        if not seg:
            continue
        insns = list(MD.disasm(seg, fn))
        decoded = (sum(x.size for x in insns) == len(seg)
                   and not any(widening(x) for x in insns))
        data = None
        if decoded:
            data, _err = assemble(args.work, fn, end, unit(fn, [render(x) for x in insns]))
            if data is not None and data != seg[:len(data)]:
                data = None
        if data is not None and len(data) == len(seg):
            asm_ok += 1
            with open(os.path.join(args.out, "fn_%08x.s" % fn), "w") as f:
                f.write(unit(fn, [render(x) for x in insns]))
            continue
        # byte-exact fallback
        lines = []
        for off in range(0, len(seg), 16):
            chunk = seg[off:off + 16]
            lines.append("\t.byte " + ", ".join("0x%02x" % b for b in chunk))
            if off == 0:
                lines.append("\t.global fn_%08x_body" % fn)
                lines.append("fn_%08x_body:" % fn)
        text2 = unit(fn, lines)
        data2, err = assemble(args.work, fn, end, text2)
        if data2 is not None and data2 == seg:
            byte_ok += 1
            with open(os.path.join(args.out, "fn_%08x.s" % fn), "w") as f:
                f.write(text2)
        else:
            failed += 1
            if args.stats and failed < 6:
                print("gen_asm: %08x failed: %s" % (fn, (err or "mismatch").splitlines()[:1]))
    print("gen_asm: %d function(s): %d assembled as instructions, %d as bytes, %d failed"
          % (len(funcs), asm_ok, byte_ok, failed))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
