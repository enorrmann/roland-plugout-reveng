#!/usr/bin/env python3
"""
bulk_dump.py -- dump SYSTEM-8 OS memory over SysEx with the JX-3P-INIT-BULK
PLUG-OUT (init_bulk.s, region 0 init hook).

Per run: build the plugin for SRC/COUNT/LEN, start a raw capture of USB1
(`SYSTEM-8 MIDI 1`; the installer uses MIDI 2), install it, wait until the
port is idle, then decode every `F0 7D 01 <addr 8 nibbles> <data nibbles> F7`
message, check that the addresses are contiguous from SRC, and write the
bytes to OUT (an existing OUT is extended: resumable with --resume).

    python3 examples/midi_send/bulk_dump.py --start 0x60000000 --count 64 -o dumps/bulk.bin
    python3 examples/midi_send/bulk_dump.py --decode cap.raw       # decode only
"""
from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
MSG = re.compile(rb"\xf0\x7d\x01([\x00-\x0f]{8})([\x00-\x0f]*)\xf7")


def decode(raw: bytes) -> list[tuple[int, bytes]]:
    out = []
    for m in MSG.finditer(raw):
        a = 0
        for n in m.group(1):
            a = a << 4 | n
        nib = m.group(2)
        if len(nib) % 2:
            print(f"  WARNING: odd nibble count at {a:#010x}, message dropped")
            continue
        out.append((a, bytes(nib[i] << 4 | nib[i + 1] for i in range(0, len(nib), 2))))
    return out


def check(msgs: list[tuple[int, bytes]], start: int | None) -> tuple[bytes, int]:
    """Concatenate contiguous messages; report gaps. Returns (data, first addr)."""
    if not msgs:
        return b"", start or 0
    first = msgs[0][0] if start is None else start
    data = bytearray()
    expect = first
    for a, d in msgs:
        if a != expect:
            print(f"  GAP/ORDER: expected {expect:#010x}, got {a:#010x} -- stopping here")
            break
        data += d
        expect += len(d)
    return bytes(data), first


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--start", type=lambda x: int(x, 0), default=0x60000000)
    ap.add_argument("--count", type=int, default=64, help="messages per install")
    ap.add_argument("--len", type=int, default=32, help="bytes per message (<= 32)")
    ap.add_argument("--slot", type=int, default=3)
    ap.add_argument("--port", default="hw:3,0,0", help="capture port (USB1)")
    ap.add_argument("--idle", type=float, default=3.0, help="stop capture after this many idle s")
    ap.add_argument("-o", "--out", default=os.path.join(ROOT, "dumps", "bulk.bin"))
    ap.add_argument("--decode", help="only decode a raw capture file")
    args = ap.parse_args()

    if args.decode:
        msgs = decode(open(args.decode, "rb").read())
        data, first = check(msgs, None)
        print(f"{len(msgs)} messages, {len(data)} contiguous bytes from {first:#010x}")
        return 0

    env = dict(os.environ, SRC=hex(args.start), COUNT=str(args.count), LEN=str(args.len))
    print(f"== build SRC={args.start:#010x} COUNT={args.count} LEN={args.len}")
    r = subprocess.run([os.path.join(HERE, "make_init_bulk.sh")], env=env,
                       stdout=subprocess.DEVNULL)
    if r.returncode:
        return r.returncode

    raw_path = os.path.join(HERE, "cap_bulk.raw")
    cap = subprocess.Popen(["amidi", "-p", args.port, "-r", raw_path, "-t", str(args.idle + 60)],
                           stderr=subprocess.DEVNULL)
    time.sleep(0.5)
    t0 = time.time()
    print(f"== install in slot {args.slot}")
    r = subprocess.run([sys.executable, os.path.join(ROOT, "tools", "plugout_tool.py"),
                        "install", os.path.join(ROOT, "plugins", "JX-3P-INIT-BULK"),
                        "--slot", str(args.slot), "--no-patches", "-y"],
                       stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
    if r.returncode:
        print(r.stderr[-500:])
    # wait until the capture file stops growing
    last, still = -1, 0.0
    while still < args.idle:
        time.sleep(0.5)
        size = os.path.getsize(raw_path) if os.path.exists(raw_path) else 0
        still = still + 0.5 if size == last else 0.0
        last = size
    cap.terminate()
    cap.wait()
    print(f"== install + capture {time.time() - t0:.1f} s, {last} raw bytes")

    msgs = decode(open(raw_path, "rb").read())
    data, first = check(msgs, args.start)
    want = args.count * args.len
    print(f"   {len(msgs)}/{args.count} messages, {len(data)}/{want} contiguous bytes "
          f"from {first:#010x}")
    if data:
        os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
        with open(args.out, "wb") as f:
            f.write(data)
        print(f"   wrote {args.out}")

    r = subprocess.run([sys.executable, os.path.join(ROOT, "tools", "plugout_tool.py"), "info"],
                       capture_output=True, text=True)
    slot = [ln for ln in r.stdout.splitlines() if f"slot {args.slot}:" in ln]
    print("== info: " + (slot[0].strip() if slot else "NO RESPONSE"))
    return 0 if len(data) == want else 1


if __name__ == "__main__":
    sys.exit(main())
