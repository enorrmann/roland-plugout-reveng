#!/usr/bin/env python3
"""
run_test.py -- install the JX-3P-FILL hook, then check what it did.

    python3 examples/preset_fill/run_test.py --slot 3            # raw bytes
    python3 examples/preset_fill/run_test.py --slot 3 --mode 1   # nibbles

  1. build plugins/JX-3P-FILL (make_plugout.sh, MODE from --mode)
  2. install it in --slot (asks for "yes" unless -y)
  3. `info` name = addr[4] count[2] form[1]: where the first preset name is
  4. RQ1 read of that preset's NAME0 leaf (+ the bytes after it) through the
     normal patch protocol
  5. compare with os70_name_method.bin: the same 70 OS bytes obtained earlier,
     7 per install, through the slot name (dump_loop.py)

The preset is restored by reinstalling the JX-3P bank (plugout_tool.py install
plugins/JX-3P --slot N --patches-only) or the whole plugin.
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
sys.path.insert(0, os.path.join(ROOT, "tools"))
import patches as PT           # noqa: E402
import plugout as P            # noqa: E402
import plugout_tool as T       # noqa: E402

HEX14 = re.compile(r"^[0-9A-F]{14}$")


def rq1(s: T.Session, addr: int, size: int, timeout: float = 2.0) -> bytes | None:
    """RQ1 read; data of the DT1 reply at the same address, or None."""
    msg = P.build(P.RQ1, addr, P.addr_bytes(size), s.device_id)
    s._pump()
    s._inbox.clear()
    s.send(msg)
    end = time.monotonic() + timeout
    while time.monotonic() < end:
        s._pump()
        for i, m in enumerate(s._inbox):
            if P.address_of(m) == addr:
                del s._inbox[i]
                return P.sysex_data(m)
        time.sleep(0.01)
    return None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--slot", type=int, default=3)
    ap.add_argument("--mode", type=int, choices=[0, 1], default=0,
                    help="0 raw bytes at the name, 1 each byte as two nibbles")
    ap.add_argument("--len", type=int, default=70)
    ap.add_argument("--ref", default=os.path.join(HERE, "os70_name_method.bin"))
    ap.add_argument("-y", "--yes", action="store_true")
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
    env.setdefault("REPORT", "0")
    r = subprocess.run([os.path.join(HERE, "make_plugout.sh")], env=env,
                       capture_output=True, text=True)
    if r.returncode:
        sys.exit(r.stdout + r.stderr)
    dat = P.load(os.path.join(ROOT, "plugins", "JX-3P-FILL", "Script", f"Code8_{a.slot}.Dat"))
    print(dat.summary().splitlines()[1])

    layout = PT.ScriptLayout(os.path.join(ROOT, "plugins", "JX-3P", "Script", "Script.xml"))
    leaf = next(x for x in layout.leaves["patch"] if x.path == "NAME0")
    name_addr = PT.patch_address(a.slot, 0, 0) + leaf.offset

    s, port = T.open_session(a)
    try:
        info = s.device_info()
        T.print_info(info)
        before = rq1(s, name_addr, 128)
        if before:
            print(f"\npreset 1 before : {before[:64].hex(' ')}")
        print(f"\nslot {a.slot} will be overwritten (MODE={a.mode}, LEN={a.len})")
        if not a.yes and input("type 'yes' to start: ").strip().lower() != "yes":
            return 1
        s.install(dat, a.slot)
        name = ""
        for i in range(8):
            time.sleep(2.0)
            name = s.device_info()["slots"][a.slot].name
            if HEX14.match(name):
                break
        print(f"slot name       : {name}")
        if not HEX14.match(name):
            print("hook did not report (not 14 hex digits)")
            return 2
        raw = bytes.fromhex(name)
        addr, count, form = int.from_bytes(raw[:4], "big"), int.from_bytes(raw[4:6], "big"), raw[6]
        print(f"first match     : {addr:#010x}  count {count}  form {form} "
              f"({'ASCII' if form == 1 else 'nibble per byte' if form == 2 else 'none'})")
        if not addr:
            return 3

        want = a.len * (2 if a.mode else 1)
        got = bytearray()
        while len(got) < want:
            n = min(128, want - len(got))
            d = rq1(s, name_addr + len(got), n)
            if d is None:
                print(f"RQ1 at +{len(got)}: no reply")
                break
            got += d
        print(f"RQ1 read back   : {len(got)} bytes\n  {bytes(got[:48]).hex(' ')}"
              f"{' ...' if len(got) > 48 else ''}")

        ref = open(a.ref, "rb").read()[:a.len]
        if a.mode:
            val = bytes(((got[2 * i] & 15) << 4) | (got[2 * i + 1] & 15)
                        for i in range(len(got) // 2))
        else:
            val = bytes(got)
        n = min(len(val), len(ref))
        exact = sum(val[i] == ref[i] for i in range(n))
        m7 = sum(val[i] == ref[i] & 0x7F for i in range(n))
        print(f"\ncompare with {os.path.basename(a.ref)} ({len(ref)} bytes):")
        print(f"  exact {exact}/{len(ref)}   ref & 0x7F {m7}/{len(ref)}")
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
