#!/usr/bin/env python3
"""
bulk_loop.py -- dump a whole memory range into ONE image with repeated
bulk_dump.py runs (COUNT x LEN bytes per install, default 2048 x 32 = 64 KiB).

The image -o always holds the bytes from --start contiguously. If it exists,
the loop resumes after the bytes already in it (as examples/mem_names/
dump_loop.py does); --no-resume starts over. Each chunk is dumped to a
temporary file and only appended to the image when it is complete. A failed
chunk is retried (--retries); the loop stops if it still fails or the device
does not answer the info query. Ctrl-C finishes the current chunk, then stops.

    python3 examples/midi_send/bulk_loop.py                       # whole OS
    python3 examples/midi_send/bulk_loop.py --count 2048 -o dumps/os_full.bin
    python3 examples/midi_send/bulk_loop.py --start 0x60000000 --end 0x60100000

Default range: the OS image, 0x60000000 + 0x2CDA70 (App1_Main payload size).
"""
from __future__ import annotations

import argparse
import os
import signal
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
OS_BASE, OS_SIZE = 0x60000000, 0x2CDA70
stop = False


def on_sigint(*_):
    global stop
    stop = True
    print("\n-- Ctrl-C: stopping after the current chunk", flush=True)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--start", type=lambda x: int(x, 0), default=OS_BASE)
    ap.add_argument("--end", type=lambda x: int(x, 0), default=OS_BASE + OS_SIZE)
    ap.add_argument("--count", type=int, default=2048, help="messages per install")
    ap.add_argument("--len", type=int, default=32, help="bytes per message (<= 32)")
    ap.add_argument("--slot", type=int, default=3)
    ap.add_argument("--retries", type=int, default=2)
    ap.add_argument("--no-resume", action="store_true")
    ap.add_argument("-o", "--out", default=os.path.join(ROOT, "dumps", "os_full.bin"))
    args = ap.parse_args()

    total = args.end - args.start
    done = 0
    if os.path.exists(args.out) and not args.no_resume:
        done = min(os.path.getsize(args.out), total)
        print(f"resume: {done} bytes already in {args.out}")
    else:
        os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
        open(args.out, "wb").close()
    tmp = args.out + ".part"
    signal.signal(signal.SIGINT, on_sigint)

    t0 = time.time()
    step = args.count * args.len
    while done < total and not stop:
        addr = args.start + done
        size = min(step, total - done)
        count = -(-size // args.len)
        for attempt in range(args.retries + 1):
            el = time.time() - t0
            print(f"== {addr:#010x} +{size:#x}  {done}/{total} B ({100 * done / total:.1f} %), "
                  f"try {attempt + 1}, {el:.0f} s", flush=True)
            if os.path.exists(tmp):
                os.remove(tmp)
            r = subprocess.run([sys.executable, os.path.join(HERE, "bulk_dump.py"),
                                "--start", hex(addr), "--count", str(count),
                                "--len", str(args.len), "--slot", str(args.slot),
                                "-o", tmp], capture_output=True, text=True,
                               start_new_session=True)   # Ctrl-C must not kill the install
            print(r.stdout.rstrip(), flush=True)
            if "NO RESPONSE" in r.stdout:
                print("device does not answer: stopping (power-cycle, then rerun to resume)")
                return 1
            data = open(tmp, "rb").read() if os.path.exists(tmp) else b""
            if len(data) >= size:
                with open(args.out, "ab") as f:
                    f.write(data[:size])      # last chunk is rounded up to whole messages
                done += size
                break
        else:
            print(f"chunk {addr:#010x} failed {args.retries + 1} times: stopping (rerun to resume)")
            return 1
    if os.path.exists(tmp):
        os.remove(tmp)

    print(f"{'stopped' if done < total else 'done'}: {done}/{total} bytes from "
          f"{args.start:#010x} in {args.out} ({time.time() - t0:.0f} s this run)")
    return 0 if done == total else 1


if __name__ == "__main__":
    sys.exit(main())
