#!/usr/bin/env python3
"""
run_press_test.py -- install JX-3P-PRESS, ask for one knob move, read back.

    python3 examples/preset_fill/run_press_test.py --slot 3 [--mode 0|1]

The engine's parameter sender hook (press.s + fill.c) writes the first 70 bytes
of the OS image at 0x60445957 (the first preset's NAME0) on every parameter
change. Steps: install; RQ1 NAME0 (must still be the original name); you select
the PLUG-OUT slot on the synth and move any knob; RQ1 NAME0 (+ the next leaf
for MODE 1) again; decode and compare with os70_name_method.bin.
"""
from __future__ import annotations

import argparse
import os
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "tools"))
sys.path.insert(0, HERE)
import patches as PT           # noqa: E402
import plugout as P            # noqa: E402
import plugout_tool as T       # noqa: E402
from run_test import rq1       # noqa: E402


def read_span(s, addr: int, n: int) -> bytes:
    out = b""
    while len(out) < n:
        c = min(128, n - len(out))
        d = rq1(s, addr + len(out), c)
        if d is None:
            print(f"  RQ1 +{len(out)} ({c} bytes): no reply")
            break
        out += d
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--slot", type=int, default=3)
    ap.add_argument("--mode", type=int, choices=[0, 1], default=1)
    ap.add_argument("--len", type=int, default=70)
    ap.add_argument("--ref", default=os.path.join(HERE, "os70_name_method.bin"))
    ap.add_argument("-y", "--yes", action="store_true")
    ap.add_argument("--no-install", action="store_true",
                    help="plugout already installed: only wait for the knob and read")
    ap.add_argument("--wait", type=float, default=180.0, help="seconds to wait for the knob")
    ap.add_argument("--backend", choices=["alsa", "rtmidi"])
    ap.add_argument("--port")
    ap.add_argument("--device-id", type=lambda x: int(x, 0), default=P.DEFAULT_DEVICE_ID)
    ap.add_argument("--gap-ms", type=float, default=0.0)
    ap.add_argument("--wire-pacing", action="store_true")
    ap.add_argument("-v", "--verbose", action="store_true")
    a = ap.parse_args()
    if a.slot < 1:
        sys.exit("slot 0 is the synth itself")

    env = dict(os.environ, MODE=str(a.mode), LEN=str(a.len))
    r = subprocess.run([os.path.join(HERE, "make_press_plugout.sh")], env=env,
                       capture_output=True, text=True)
    if r.returncode:
        sys.exit(r.stdout + r.stderr)
    dat = P.load(os.path.join(ROOT, "plugins", "JX-3P-PRESS", "Script", f"Code8_{a.slot}.Dat"))
    print(dat.summary().splitlines()[1])

    layout = PT.ScriptLayout(os.path.join(ROOT, "plugins", "JX-3P", "Script", "Script.xml"))
    leaf = next(x for x in layout.leaves["patch"] if x.path == "NAME0")
    name_addr = PT.patch_address(a.slot, 0, 0) + leaf.offset
    want = a.len * (2 if a.mode else 1)

    s, port = T.open_session(a)
    try:
        T.print_info(s.device_info())
        print(f"\nslot {a.slot} will be overwritten (MODE={a.mode}, LEN={a.len})")
        if not a.yes and not a.no_install and input("type 'yes' to start: ").strip().lower() != "yes":
            return 1
        if not a.no_install:
            s.install(dat, a.slot)
            time.sleep(3.0)
        print("slot:", s.device_info()["slots"][a.slot])
        d = read_span(s, name_addr, want)
        print(f"before knob : {d[:24].hex(' ')} ...")
        print(f"\nselect slot {a.slot} on the SYSTEM-8 and move ANY knob once "
              f"(waiting up to {a.wait:.0f} s for the preset name to change) ...",
              flush=True)
        end = time.monotonic() + a.wait
        base = d[:24]
        while time.monotonic() < end:
            time.sleep(1.0)
            cur = rq1(s, name_addr, 24)
            if cur is not None and cur != base:
                print("change detected")
                break
        else:
            print("no change seen within the time limit; reading anyway")
        time.sleep(0.5)
        d = read_span(s, name_addr, want)
        print(f"after knob  : {len(d)} bytes\n  {d[:48].hex(' ')}{' ...' if len(d) > 48 else ''}")
        if a.mode:
            val = bytes(((d[2 * i] & 15) << 4) | (d[2 * i + 1] & 15) for i in range(len(d) // 2))
        else:
            val = d
        ref = open(a.ref, "rb").read()[:a.len]
        n = min(len(val), len(ref))
        exact = sum(val[i] == ref[i] for i in range(n))
        m7 = sum(val[i] == ref[i] & 0x7F for i in range(n))
        print(f"\ncompare with {os.path.basename(a.ref)}: exact {exact}/{len(ref)}   "
              f"ref & 0x7F {m7}/{len(ref)}")
        print(f"  ref  : {ref[:24].hex(' ')} ...")
        print(f"  read : {val[:24].hex(' ')} ...")
        return 0 if exact == len(ref) else 4
    except T.ProtocolError as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 2
    finally:
        port.close()


if __name__ == "__main__":
    sys.exit(main())
