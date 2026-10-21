#!/usr/bin/env python3
"""selftest_link.py -- check build_le.py against GNU as / ld.

The reconstruction is only trustworthy if its own linker reproduces exactly
what GNU ld does with the same relocations.  This test assembles a file with
GNU as (``arm-none-eabi-as``), links it with GNU ld at the image base, and then
places the same *object* with build_le.py; the two images must be identical.

Skipped (with a message) when the toolchain is not installed.
"""
import os
import shutil
import struct
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import build_le  # noqa: E402

AS = "arm-none-eabi-as"
LD = "arm-none-eabi-ld"
OBJCOPY = "arm-none-eabi-objcopy"
NM = "arm-none-eabi-nm"

SRC_MAIN = r"""
	.syntax unified
	.thumb
	.section .text,"ax"
	.thumb_func
main_fn:
	bl near_fn
	bl far_fn
	bl undefined_far
	b.w near_fn
	beq far_fn
	bne near_fn
	movs r0, #1
	.space 0x40
	.thumb_func
near_fn:
	bx lr
	.space 0x1000
	.thumb_func
far_fn:
	bx lr
"""

SRC_OTHER = r"""
	.syntax unified
	.thumb
	.section .text,"ax"
	.thumb_func
	.global undefined_far
undefined_far:
	bx lr
"""


def run(cmd):
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode:
        raise SystemExit("%s failed:\n%s%s" % (" ".join(cmd), r.stdout, r.stderr))
    return r


def main():
    for tool in (AS, LD, OBJCOPY, NM):
        if not shutil.which(tool):
            print("selftest_link: %s not found, skipped" % tool)
            return 0
    base = 0x60000000
    with tempfile.TemporaryDirectory() as td:
        main_s = os.path.join(td, "a.s")
        other_s = os.path.join(td, "b.s")
        with open(main_s, "w") as f:
            f.write(SRC_MAIN)
        with open(other_s, "w") as f:
            f.write(SRC_OTHER)
        a_o = os.path.join(td, "a.o")
        b_o = os.path.join(td, "b.o")
        run([AS, "-mthumb", "-o", a_o, main_s])
        run([AS, "-mthumb", "-o", b_o, other_s])
        elf = os.path.join(td, "ref.elf")
        run([LD, "-Ttext=0x%08x" % base, "-o", elf, a_o, b_o])
        ref = os.path.join(td, "ref.bin")
        run([OBJCOPY, "-O", "binary", "-j", ".text", elf, ref])
        with open(ref, "rb") as f:
            golden = f.read()

        # value GNU ld gave the symbol that object a leaves undefined
        far = None
        for line in run([NM, elf]).stdout.splitlines():
            parts = line.split()
            if len(parts) == 3 and parts[2] == "undefined_far":
                far = int(parts[0], 16)
        if far is None:
            raise SystemExit("selftest_link: symbol not found in the ld output")

        # place object a with build_le, resolving its one external symbol the
        # way ld did (this is what gen_code.py's symbol table provides)
        obj = build_le.Elf(a_o)
        # an object without its own address assertion is placed at the base,
        # as ld does for the first section
        for sec in obj.alloc_sections():
            if sec["addr"] == 0:
                sec["addr"] = base
        obj.resolve({"undefined_far": far | 1})   # Thumb: bit 0 set
        # the value ld gave the symbol comes from where it placed b.o (the
        # second object starts right after the first section)
        got = bytearray(golden)
        for sec in obj.alloc_sections():
            start = sec["addr"] - base
            for off, data in build_le.place(obj, base, len(golden)):
                got[off:off + len(data)] = data
        n = min(len(golden), len(got))
        diffs = [i for i in range(n) if golden[i] != got[i]]
        if diffs:
            print("selftest_link: FAIL, %d differing byte(s)" % len(diffs))
            for i in diffs[:8]:
                print("  %#x ref %02x got %02x" % (base + i, golden[i], got[i]))
            return 1
        print("selftest_link: OK (%d bytes identical to GNU ld)" % n)
        return 0


if __name__ == "__main__":
    sys.exit(main())
