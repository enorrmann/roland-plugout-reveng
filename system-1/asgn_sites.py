#!/usr/bin/env python3
"""
asgn_sites.py -- list the places in the master app's voice allocator that encode "4 voices".

Disassembles the allocator range of the SYSTEM-1 master app 1.30 (0x60009500-0x6000A400:
the code behind the allocator object at 0x604A26B0, called from the CAssign-like object
0x600A195C, vtable 0x60081008) with arm-none-eabi-objdump and prints every instruction that
uses one of the constants that a 6-voice patch has to revisit:

    #4    voice count in loops / compares (cmp rX, #4; movs rX, #4)
    #3    "& 3" round robin index and "+ 3 & 3" (previous voice)
    #288  0x120 + voice = per-voice parameter id (group 2), sent to the slave

    python3 system-1/asgn_sites.py [work/sys1_v130/app_master.bin] [START END]
"""
from __future__ import annotations

import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

KEEP = re.compile(r"\b(cmp|movs|mov\.w|and\.w|add\.w|adds)\b.*#(4|3|288)\b")
SKIP = re.compile(r"lsl #|\[r\d+, #[34]\]|sp, #")


def main() -> int:
    args = sys.argv[1:]
    path = args[0] if args else os.path.join(ROOT, "work", "sys1_v130", "app_master.bin")
    start = int(args[1], 16) if len(args) > 1 else 0x60009500
    end = int(args[2], 16) if len(args) > 2 else 0x6000A400
    dis = subprocess.run(
        ["arm-none-eabi-objdump", "-D", "-b", "binary", "-m", "arm", "-M", "force-thumb",
         "--adjust-vma=0x60000000", f"--start-address={start:#x}", f"--stop-address={end:#x}", path],
        capture_output=True, text=True, check=True).stdout
    n = 0
    for line in dis.splitlines():
        if KEEP.search(line) and not SKIP.search(line):
            print(line.strip())
            n += 1
    print(f"{n} sites in {start:#x}-{end:#x}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
