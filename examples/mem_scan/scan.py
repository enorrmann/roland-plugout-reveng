#!/usr/bin/env python3
"""
scan.py -- search SYSTEM-8 memory on the device for a byte pattern (scan.c,
region 0 init hook of a stock JX-3P) and print the number of matches and the
first five addresses, read back through the three slot names (as
../mem_names/dump_loop.py). One install per search (~20 s).

Pattern syntax: hex bytes separated by spaces, '?' = wildcard nibble
("F? 2?" = Thumb `movs rX, #0xF?`). Optional --near: second pattern that must
occur within --win bytes (before or after) of each match.

    python3 examples/mem_scan/scan.py --pat "F0 41 10 00 00"
    python3 examples/mem_scan/scan.py --pat "F0 2?" --step 2 --near "F7 2?" --win 64
    python3 examples/mem_scan/scan.py --pat "..." --skip 5      # next five matches
"""
from __future__ import annotations

import argparse
import importlib.util
import os
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location(
    "names_dump_loop", os.path.join(HERE, "..", "mem_names", "dump_loop.py"))
M = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(M)
D, P, T = M.D, M.P, M.T
BASE, CAVE_OFF, INIT_OFF = D.BASE, D.CAVE_OFF, D.INIT_OFF
OS_LO, OS_HI = 0x60000000, 0x602CDA70          # App1_Main image in RAM


def parse_pattern(text: str) -> tuple[bytes, bytes]:
    pat, msk = bytearray(), bytearray()
    for tok in text.split():
        if len(tok) != 2:
            sys.exit(f"bad pattern byte {tok!r}")
        p = m = 0
        for ch in tok:
            p, m = p << 4, m << 4
            if ch != "?":
                p |= int(ch, 16)
                m |= 0xF
        pat.append(p)
        msk.append(m)
    if not 1 <= len(pat) <= 8:
        sys.exit("pattern must be 1..8 bytes")
    return bytes(pat), bytes(msk)


def words(b: bytes) -> tuple[int, int]:
    b = b.ljust(8, b"\0")
    return int.from_bytes(b[:4], "little"), int.from_bytes(b[4:], "little")


def run(cmd: list[str]) -> None:
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode:
        sys.exit(f"command failed: {' '.join(cmd)}\n{r.stdout}{r.stderr}")


def build_hook(args, work: str) -> str:
    pat, msk = parse_pattern(args.pat)
    p0, p1 = words(pat)
    m0, m1 = words(msk)
    defs = {"LO": args.lo, "HI": args.hi, "STEP": args.step, "SKIP": args.skip,
            "P0": p0, "P1": p1, "M0": m0, "M1": m1, "PLEN": len(pat)}
    if args.near:
        q, qm = parse_pattern(args.near)
        q0, q1 = words(q)
        qm0, qm1 = words(qm)
        defs.update({"Q0": q0, "Q1": q1, "QM0": qm0, "QM1": qm1, "QLEN": len(q),
                     "WIN": args.win, "QSTEP": args.qstep})
    o = lambda n: os.path.join(work, n)                       # noqa: E731
    run(["arm-none-eabi-gcc", "-mcpu=cortex-a9", "-mthumb", "-Os", "-ffreestanding",
         "-fno-builtin", "-fno-jump-tables", "-fno-stack-protector", "-fno-pic",
         "-mno-unaligned-access", "-nostdlib"]
        + [f"-D{k}={v:#x}u" for k, v in defs.items()]
        + ["-c", os.path.join(HERE, "scan.c"), "-o", o("scan.o")])
    run(["arm-none-eabi-as", "-mcpu=cortex-a9", "-mthumb", "--defsym", f"CAVE_OFF={CAVE_OFF:#x}",
         "--defsym", f"INIT_OFF={INIT_OFF:#x}", os.path.join(HERE, "entry.s"), "-o", o("entry.o")])
    run(["arm-none-eabi-ld", f"-Ttext={BASE + CAVE_OFF:#x}", "--entry=entry",
         o("entry.o"), o("scan.o"), "-o", o("hook.elf")])
    sec = subprocess.run(["arm-none-eabi-objdump", "-h", o("hook.elf")],
                         capture_output=True, text=True).stdout
    for bad in (".rodata", ".data", ".bss"):
        if bad in sec:
            sys.exit(f"hook has a {bad} section (slot-relative data): not allowed")
    run(["arm-none-eabi-objcopy", "-O", "binary", "-j", ".text", o("hook.elf"), o("hook.bin")])
    if args.verbose:
        print(subprocess.run(["arm-none-eabi-objdump", "-d", o("hook.elf")],
                             capture_output=True, text=True).stdout)
    print(f"hook: {os.path.getsize(o('hook.bin'))} bytes, " + " ".join(
        f"{k}={v:#x}" for k, v in defs.items()))
    return o("hook.bin")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--pat", required=True)
    ap.add_argument("--near")
    ap.add_argument("--win", type=lambda x: int(x, 0), default=64)
    ap.add_argument("--qstep", type=int, default=1)
    ap.add_argument("--lo", type=lambda x: int(x, 0), default=OS_LO)
    ap.add_argument("--hi", type=lambda x: int(x, 0), default=OS_HI)
    ap.add_argument("--step", type=int, default=1, help="alignment of P matches (2 = Thumb)")
    ap.add_argument("--skip", type=int, default=0)
    ap.add_argument("--slot", type=int, default=3)
    ap.add_argument("--read-tries", type=int, default=5)
    ap.add_argument("--read-wait", type=float, default=0.1)
    ap.add_argument("--log", default=os.path.join(HERE, "scan_log.txt"))
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--backend", choices=["alsa", "rtmidi"])
    ap.add_argument("--port")
    ap.add_argument("--device-id", type=lambda x: int(x, 0), default=P.DEFAULT_DEVICE_ID)
    ap.add_argument("--gap-ms", type=float, default=0.0)
    ap.add_argument("--wire-pacing", action="store_true")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()

    with tempfile.TemporaryDirectory() as work:
        hook = build_hook(args, work)
        proj = D.prepare_project(work)
        run(D.BUILD + ["patch", proj, "--region", "0", "--at", f"{BASE + CAVE_OFF:#x}",
                       "--file", hook])
        dat = os.path.join(work, f"Code8_{args.slot}.Dat")
        run(D.BUILD + ["pack", proj, "-o", dat, "--slot", str(args.slot)])
        if args.dry_run:
            print(P.load(dat).summary())
            return 0
        c = P.load(dat)
        s, port = T.open_session(args)
        try:
            t0 = time.time()
            M.install_records(s, c, args.slot, list(range(c.data_count)), lambda i, n: None)
            names = M.read_names(s, args.slot, c.model_id, args.read_tries, args.read_wait)
        finally:
            port.close()
        if not all(M.HEX16.match(n) for n in names):
            sys.exit(f"names {names!r}: the hook did not run")
        r = [int(n[i:i + 8], 16) for n in names for i in (0, 8)]
        count, hits = r[0], [a for a in r[1:] if a]
        line = (f"pat={args.pat!r}" + (f" near={args.near!r}/{args.win}" if args.near else "")
                + f" step={args.step} {args.lo:#x}..{args.hi:#x} skip={args.skip}: "
                f"count={count} " + " ".join(f"{a:#010x}" for a in hits))
        print(line + f"   ({time.time() - t0:.0f} s)")
        with open(args.log, "a") as fh:
            fh.write(line + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
