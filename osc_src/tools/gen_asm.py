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
import bisect
import os
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
# arm-none-eabi-as over the whole image, of the narrow instructions only ``adr``
# comes back wider (as picks ADD.W); every other narrow form (``ldm``, ``stm``,
# ``movs``, ``b<cond>`` ...) assembles back to its original size when its own
# text is used.
WIDENING = ("adr",)


def widening(ins):
    """True when the assembler would not reproduce this instruction's bytes."""
    if ins.size != 2:
        return False
    return ins.mnemonic.split(".")[0] in WIDENING


# Inside an IT block Capstone prints a 16-bit data-processing instruction as
# ``subgt r1, r1, #4``.  The assembler accepts that text but encodes the
# two-operand thumb form (``subgt r1, #4``) -- a different instruction, with
# different flags.  Such a line is emitted as its bytes instead.
def simplified(ins):
    """True when as would fold a three-operand text into a two-operand one."""
    parts = [p.strip() for p in ins.op_str.split(",")]
    return len(parts) == 3 and parts[0] == parts[1] and len(ins.bytes) == 2


def render(ins, force_bytes=False):
    """Assembly text for one instruction, with cross references symbolic."""
    m, o = ins.mnemonic, ins.op_str
    if force_bytes:
        return "\t.byte " + ", ".join("0x%02x" % b for b in ins.bytes)
    if simplified(ins):
        return "\t.byte " + ", ".join("0x%02x" % b for b in ins.bytes)
    if m in ("bl", "blx") and ins.operands and ins.operands[0].type == cs.arm.ARM_OP_IMM:
        return "\tbl fn_%08x" % (ins.operands[0].imm & ~1)
    if m in ("cbz", "cbnz") and ins.operands:
        # the assembler places a cbz relocation at the wrong word: keep bytes
        return "\t.byte " + ", ".join("0x%02x" % b for b in ins.bytes)
    if (m[0] == "b" and m not in ("bkpt", "bfc", "bfi") and not m.startswith("bl")
            and m not in ("blx",)
            and ins.operands and ins.operands[0].type == cs.arm.ARM_OP_IMM):
        # as widens a branch whose target is not yet defined, and a target is
        # only known after linking -- so the bytes are kept, with the target
        # declared as a symbol so the link-time symbol table has it even when
        # the target sits inside another unit.
        return "\t.globl fn_%08x\n\t.byte %s" % (
            ins.operands[0].imm & ~1,
            ", ".join("0x%02x" % b for b in ins.bytes))
    return ("\t" + m + " " + o).rstrip()


def unit(fn, lines):
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
    ap.add_argument("--stats", action="store_true",
                    help="print the first failures (diagnostics)")
    ap.add_argument("--limit", type=int, default=0)
    args = ap.parse_args(argv)

    with open(args.image, "rb") as f:
        img = f.read()
    all_funcs = sorted(set(int(x, 16) for x in open(args.functions) if x.strip()))
    funcs = all_funcs
    if args.limit:
        funcs = all_funcs[:args.limit]
        # with a limit, a unit still ends where the next listed unit starts
        all_funcs = funcs
    os.makedirs(args.out, exist_ok=True)
    os.makedirs(args.work, exist_ok=True)

    asm_ok = byte_ok = failed = 0
    for i, fn in enumerate(funcs):
        end = funcs[i + 1] if i + 1 < len(funcs) else BASE + len(img)
        seg = img[fn - BASE:end - BASE]
        if not seg:
            continue
        insns = list(MD.disasm(seg, fn))
        # An IT block whose body is not emitted as text would leave the
        # assembler's IT state open and break every instruction after it, so
        # the whole group is emitted as bytes in that case.
        forced = set()
        it_lines = []
        for k, x in enumerate(insns):
            if x.mnemonic.startswith("it") and len(x.mnemonic) <= 4:
                # the mask is one letter per instruction; a trailing "e" (the
                # last letter) ends the block early, so ``ittt`` guards 3 and
                # ``itte`` honours the "e" at its position
                mask = x.op_str.strip()
                body = mask.index("e") if "e" in mask else len(mask)
                it_lines.append((k, body))
        for k, body in it_lines:
            group = insns[k:k + 1 + body]
            if any(widening(x) or simplified(x) for x in group) or \
                    any(x.mnemonic.startswith("it") for x in group[1:]):
                forced.update(id(x) for x in group)
        decoded = (sum(x.size for x in insns) == len(seg)
                   and not any(widening(x) or simplified(x) or id(x) in forced
                               for x in insns))
        data = None
        lines = [render(x, id(x) in forced) for x in insns]
        if decoded:
            data, _err = assemble(args.work, fn, end, unit(fn, lines))
            if data is not None and data != seg[:len(data)]:
                data = None
        if data is not None and len(data) == len(seg):
            asm_ok += 1
            with open(os.path.join(args.out, "fn_%08x.s" % fn), "w") as f:
                f.write(unit(fn, lines))
            continue
        # Byte-exact fallback.  Any function entry that falls inside this unit
        # gets a global label, so a call to it resolves to the right address
        # even though the unit is a plain byte image.
        lo = bisect.bisect_right(all_funcs, fn)
        hi = bisect.bisect_left(all_funcs, end)
        nested = set(all_funcs[lo:hi])
        lines = []
        for off in range(0, len(seg), 2):
            here = fn + off
            if here in nested:
                lines.append("\t.global fn_%08x" % here)
                lines.append("fn_%08x:" % here)
            lines.append("\t.byte " + ", ".join("0x%02x" % b
                                                 for b in seg[off:off + 2]))
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
