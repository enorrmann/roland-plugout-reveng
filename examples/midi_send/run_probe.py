#!/usr/bin/env python3
"""
run_probe.py -- install the JX-3P-MIDI-SEND probe and map each router bit to a
physical MIDI port.

Steps:
  1. build the plugin (make_plugout.sh) and install it in --slot
  2. capture every MIDI input (USB1, USB2, DIN via the CH345 adapter) for
     --window seconds; during that window SELECT the PLUG-OUT and move a knob
     or button once -- the probe then emits one SysEx per router bit 0..NBITS-1
  3. decode the captured SysEx (F0 7D 01 <bit> .. F7) and print, per bit, which
     port(s) received it -> the bit->port map
  4. query the device (info) to confirm it did not freeze

Nothing risky is sent from here; the only firmware change is the engine hook,
which on fault freezes the synth (power cycle; slots survive).

    python3 examples/midi_send/run_probe.py --slot 3
    python3 examples/midi_send/run_probe.py --slot 3 --window 25 --nbits 6
    PROBE=cc python3 examples/midi_send/run_probe.py --slot 3   # CC probe (cc.s)
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

# (label, ALSA rawmidi port). DIN is the CH345 USB-MIDI adapter on the DIN jacks.
PORTS = [("USB1", "hw:3,0,0"), ("USB2", "hw:3,0,1"), ("DIN", "hw:4,0,0")]
SYSEX = re.compile(rb"\xf0\x7d\x01(.)[\x00-\x7f]*?\xf7")


def run(cmd: list[str], **kw) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, text=True, **kw)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--slot", type=int, default=3)
    ap.add_argument("--nbits", type=int, default=6)
    ap.add_argument("--window", type=float, default=20.0, help="capture seconds")
    ap.add_argument("--no-install", action="store_true", help="skip build+install")
    ap.add_argument("--plugin", default=os.path.join(ROOT, "plugins", "JX-3P-MIDI-SEND"))
    args = ap.parse_args()

    if not args.no_install:
        print(f"== build (NBITS={args.nbits})")
        env = dict(os.environ, NBITS=str(args.nbits))
        r = run([os.path.join(HERE, "make_plugout.sh")], env=env)
        if r.returncode:
            return r.returncode
        print(f"== install in slot {args.slot}")
        r = run([sys.executable, os.path.join(ROOT, "tools", "plugout_tool.py"),
                 "install", args.plugin, "--slot", str(args.slot), "--no-patches", "-y"])
        if r.returncode:
            return r.returncode

    print(f"\n*** SELECT the PLUG-OUT in slot {args.slot} and move ONE knob/button now ***")
    print(f"    capturing {len(PORTS)} ports for {args.window:.0f} s ...")
    procs = []
    for label, port in PORTS:
        out = os.path.join(HERE, f"cap_{label}.bin")
        f = open(out, "wb")
        p = subprocess.Popen(["amidi", "-p", port, "-d"], stdout=f, stderr=subprocess.DEVNULL)
        procs.append((label, port, p, f, out))
    try:
        time.sleep(args.window)
    finally:
        for _, _, p, f, _ in procs:
            p.terminate()
            f.close()

    print("\n== captured (bit -> port):")
    seen: dict[int, list[str]] = {}
    for label, port, _, _, out in procs:
        text = open(out, "rb").read().decode("ascii", "ignore")
        # amidi -d prints received bytes as space-separated hex text
        raw = bytes(int(h, 16) for h in re.findall(r"[0-9A-Fa-f]{2}", text))
        for m in SYSEX.finditer(raw):
            seen.setdefault(m.group(1)[0], []).append(label)
        if raw:
            print(f"  {label} ({port}): {len(raw)} bytes  {raw.hex(' ')[:96]}")
        ncc = raw.count(bytes([0xBF, 0x66]))      # cc.s probe: BF 66 01
        if ncc:
            print(f"  {label}: probe CC 'BF 66 01' received {ncc}x")
    if not seen:
        print("  (no tagged SysEx on any port)")
    else:
        for bit in sorted(seen):
            print(f"  router bit {bit} (mask {1 << bit:#05x}) -> {', '.join(seen[bit])}")

    print("\n== liveness (device info):")
    r = run([sys.executable, os.path.join(ROOT, "tools", "plugout_tool.py"), "info"],
            capture_output=True)
    print("  " + ("RESPONDS (not frozen)" if r.returncode == 0 else "NO RESPONSE -- likely frozen, power-cycle"))
    if r.stdout:
        print("\n".join("  " + ln for ln in r.stdout.strip().splitlines()[:6]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
