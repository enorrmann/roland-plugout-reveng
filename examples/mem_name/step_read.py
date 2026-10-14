#!/usr/bin/env python3
"""
step_read.py -- collect the OS memory chunks that the JX-3P-MEM-STEP engine hook
leaves in the preset name (NAME0 of preset 1, RQ1 readable), one per panel
parameter change.

    python3 examples/mem_name/step_read.py --slot 3 -o os_step.bin

Each knob / button / switch move makes the hook write 128 nibble bytes at NAME0:
4 bytes big endian address + 60 data bytes. This script polls NAME0, decodes a
new chunk when it changes and appends it to OUT (resumes: the file length must
be a multiple of 60 and the hook's counter must be at that chunk, otherwise it
warns). Ctrl-C stops. Nothing is sent to the synth except RQ1 reads.
"""
from __future__ import annotations

import argparse
import os
import struct
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "tools"))
sys.path.insert(0, os.path.join(ROOT, "examples", "preset_fill"))
import patches as PT           # noqa: E402
import plugout as P            # noqa: E402
import plugout_tool as T       # noqa: E402
from run_test import rq1       # noqa: E402

CHUNK = 60


def decode(raw: bytes):
    if len(raw) < 128 or any(b > 15 for b in raw[:128]):
        return None
    v = bytes((raw[2 * i] << 4) | raw[2 * i + 1] for i in range(64))
    return struct.unpack(">I", v[:4])[0], v[4:]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--slot", type=int, default=3)
    ap.add_argument("--start", type=lambda x: int(x, 0), default=0x60000000,
                    help="SRC the plugin was built with (default 0x60000000)")
    ap.add_argument("-o", "--out", default="os_step.bin")
    ap.add_argument("--poll", type=float, default=0.3, help="seconds between reads")
    ap.add_argument("--backend", choices=["alsa", "rtmidi"])
    ap.add_argument("--port")
    ap.add_argument("--device-id", type=lambda x: int(x, 0), default=P.DEFAULT_DEVICE_ID)
    ap.add_argument("--gap-ms", type=float, default=0.0)
    ap.add_argument("--wire-pacing", action="store_true")
    ap.add_argument("-v", "--verbose", action="store_true")
    a = ap.parse_args()
    if a.slot < 1:
        sys.exit("slot 0 is the synth itself")

    layout = PT.ScriptLayout(os.path.join(ROOT, "plugins", "JX-3P", "Script", "Script.xml"))
    leaf = next(x for x in layout.leaves["patch"] if x.path == "NAME0")
    addr = PT.patch_address(a.slot, 0, 0) + leaf.offset

    s, port = T.open_session(a)
    try:
        base = rq1(s, addr, 128)
        if base is None:
            sys.exit("RQ1 of NAME0 got no reply")
        print(f"NAME0 of preset 1, slot {a.slot} ({addr:#x}): {base[:32].hex(' ')} ...")
        print("move any knob / button; one chunk per parameter change. Ctrl-C to stop.")
        last, got = base, 0
        have = os.path.getsize(a.out) // CHUNK * CHUNK if os.path.exists(a.out) else 0
        while True:
            time.sleep(a.poll)
            cur = rq1(s, addr, 128)
            if cur is None or cur == last:
                continue
            last = cur
            d = decode(cur)
            if d is None:
                print(f"  changed but not nibbles: {cur[:24].hex(' ')} ...")
                continue
            at, data = d
            want = a.start + have
            note = "" if at == want else f"   (expected {want:#x}: missed or repeated)"
            if at == want:
                with open(a.out, "ab") as fh:
                    fh.write(data)
                have += CHUNK
            got += 1
            print(f"  {at:#010x}: {data[:16].hex(' ')} ...{note}")
    except KeyboardInterrupt:
        pass
    except T.ProtocolError as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 2
    finally:
        port.close()
    print(f"\n{have} bytes in {a.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
