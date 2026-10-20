#!/usr/bin/env python3
"""
patch_read.py -- try to read patch names from a SYSTEM-8 slot with RQ1.

    python3 tools/patch_read.py --slot 3 --patches 0-7 [--plugin plugins/JX-3P]

The plugin protocol only documents DT1 writes of the patch bank (docs/PROTOCOL.md
sec. 6); no read has been seen. This sends the standard Roland read request
(RQ1 = address + 4-byte size) for the NAME0 leaf of `fm.usrX` of each patch and
prints whatever comes back (decoded as 2 nibbles per character). Only RQ1 is
used: nothing is written to the device.
"""
from __future__ import annotations

import argparse
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import patches as PT          # noqa: E402
import plugout as P           # noqa: E402
import plugout_tool as T      # noqa: E402


def decode_name(data: bytes) -> str:
    chars = bytes(((data[i] & 0xF) << 4) | (data[i + 1] & 0xF)
                  for i in range(0, len(data) - 1, 2))
    return chars.split(b"\0")[0].decode("ascii", "replace").rstrip()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--slot", type=int, default=3)
    ap.add_argument("--patches", default="0-3", help="0-based range, e.g. 0-7 or 5")
    ap.add_argument("--plugin", default=os.path.join(os.path.dirname(__file__), "..", "plugins", "JX-3P"))
    ap.add_argument("--leaf", default="NAME0")
    ap.add_argument("--timeout", type=float, default=1.5)
    ap.add_argument("--backend", choices=["alsa", "rtmidi"])
    ap.add_argument("--port")
    ap.add_argument("--device-id", type=lambda x: int(x, 0), default=P.DEFAULT_DEVICE_ID)
    ap.add_argument("--gap-ms", type=float, default=0.0)
    ap.add_argument("--wire-pacing", action="store_true")
    ap.add_argument("-v", "--verbose", action="store_true")
    a = ap.parse_args()

    layout = PT.ScriptLayout(os.path.join(a.plugin, "Script", "Script.xml"))
    leaf = next(x for x in layout.leaves["patch"] if x.path == a.leaf)
    lo, _, hi = a.patches.partition("-")
    rng = range(int(lo), int(hi or lo) + 1)

    s, port = T.open_session(a)
    try:
        for p in rng:
            addr = PT.patch_address(a.slot, p, 0) + leaf.offset
            size = P.addr_bytes(leaf.size)             # 4 x 7-bit size field
            rq = P.build(P.RQ1, addr, size, a.device_id)
            s._pump()
            s._inbox.clear()
            s.send(rq)
            got, end = [], time.monotonic() + a.timeout
            while time.monotonic() < end and not got:
                s._pump()
                got, s._inbox = s._inbox, []
                time.sleep(0.01)
            print(f"patch {p:2d}  addr {P.addr_str(addr)}  RQ1 {rq.hex(' ')}")
            if not got:
                print("    no reply")
            for m in got:
                d = P.sysex_data(m)
                print(f"    reply @ {P.addr_str(P.address_of(m))} cmd {m[6]:#04x} "
                      f"{len(d)} bytes: {d[:32].hex(' ')}{' ...' if len(d) > 32 else ''}")
                print(f"    name: {decode_name(d)!r}")
    finally:
        port.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
