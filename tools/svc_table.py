#!/usr/bin/env python3
"""
svc_table.py -- decode a dump of the OS service table (0x0113FC00) and list the
functions it points to.

    python3 tools/svc_table.py os_svc.bin [--base 0x0113FC00]

Every aligned u32 is classified:
  * odd value inside the kernel image (0x01000400..0x01019A08): Thumb function
    (the code at 0x01000400 is a RAM copy of image 0x602B9774..., see
    BASE_FIRMWARE.md section 6)
  * value inside the OS image (0x60000000..0x602CDA70): pointer into the image
  * value in RAM (0x01xxxxxx / 0x20xxxxxx): data pointer
  * small values: counters / flags
It also prints the dump_loop.py commands that read the first 0x20 bytes of every
distinct function, to be analysed with arm-none-eabi-objdump.
"""
import argparse
import struct
import sys

KERNEL = (0x01000400, 0x0101A000)
IMAGE = (0x60000000, 0x602CDA70)
RAM = (0x01000000, 0x01200000)
FAST = (0x20000000, 0x20030000)


def classify(v: int) -> str:
    if v == 0:
        return "null"
    if KERNEL[0] <= v < KERNEL[1] and v & 1:
        return "kernel function (Thumb)"
    if KERNEL[0] <= v < KERNEL[1]:
        return "kernel data/code (even)"
    if IMAGE[0] <= v < IMAGE[1]:
        return "OS image" + (" function (Thumb)" if v & 1 else "")
    if RAM[0] <= v < RAM[1]:
        return "RAM"
    if FAST[0] <= v < FAST[1]:
        return "fast RAM"
    if v < 0x10000:
        return "small value"
    return "?"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("file")
    ap.add_argument("--base", type=lambda x: int(x, 0), default=0x0113FC00)
    ap.add_argument("--slot", type=int, default=3)
    a = ap.parse_args()
    d = open(a.file, "rb").read()
    funcs = []
    for o in range(0, len(d) - 3, 4):
        v = struct.unpack_from("<I", d, o)[0]
        kind = classify(v)
        print(f"{a.base + o:#010x} +{o:#04x}  {v:#010x}  {kind}")
        if "function" in kind and v not in funcs:
            funcs.append(v)
    if len(d) % 4:
        print(f"({len(d) % 4} trailing bytes ignored)")
    if funcs:
        print("\n# read the first 0x20 bytes of each function (7 bytes per install):")
        for f in funcs:
            print(f"python3 examples/mem_name/dump_loop.py --slot {a.slot} "
                  f"--start {f & ~1:#x} --count 0x20 -o fn_{f & ~1:08x}.bin")
    return 0


if __name__ == "__main__":
    sys.exit(main())
