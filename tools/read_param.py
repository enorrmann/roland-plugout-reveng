#!/usr/bin/env python3
"""
read_param.py -- read the current value of a patch parameter by name with RQ1.

    python3 tools/read_param.py --slot 3 --patch 0                  # EFFECT TONE
    python3 tools/read_param.py --slot 3 --param "EFFECT TYPE" --watch
    python3 tools/read_param.py --list "TONE"                       # find names
    python3 tools/read_param.py --edit --watch                      # the live edit buffer
    python3 tools/read_param.py --edit --leaf NAME0 --watch         # current preset name
    python3 tools/read_param.py --edit --set 200                    # like a knob move
    python3 tools/read_param.py --write "VCF CUTOFF FREQ=100" --write "VCF RESONANCE=40"
    python3 tools/read_param.py --edit --param "VCF CUTOFF FREQ H"       # 32-bit (float bits)
    python3 tools/read_param.py --write "VCF CUTOFF FREQ H=0x3F000000"   # = 0.5f
    python3 tools/read_param.py --write "VCF CUTOFF FREQ H=0.5f"         # same, as a float

The parameter's address comes from the JX-3P Script.xml (the patch struct the
value lives in + its offset). Values are `int8x4`: 8 nibbles, one per byte on
the wire, most significant first. Only RQ1 is sent: nothing is written.

--watch polls and prints on every change (turn the knob on the panel: this
shows whether RQ1 follows the live edit buffer or only the stored patch).
"""
from __future__ import annotations

import argparse
import os
import struct
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import patches as PT          # noqa: E402
import plugout as P           # noqa: E402
import plugout_tool as T      # noqa: E402


GROUPS = {"patch": ("03 00 00 00", 0), "patch2": ("05 00 00 00", 1),
          "patch3": ("06 00 00 00", 2)}   # fm.PATCH/2/3 edit buffers, usrX/usr2X/usr3X index


def find_values(layout: PT.ScriptLayout):
    """(name, leaf, offset in leaf, size, range, default, group) for each addressable value."""
    out = []
    for group in GROUPS:
      for leaf in layout.leaves[group]:
        e = layout.types.get(leaf.path)
        if e is None:
            continue
        off = 0
        for v in e.findall("value"):
            if v.findtext("type") is None:
                continue
            size = layout._value_size(v)
            addrs = [PT.a7(a.text) for a in v.findall("address")]
            at = addrs[0] if addrs else off
            if not addrs:
                off += size
            out.append((v.findtext("name"), leaf, at, size,
                        v.findtext("range"), v.findtext("default"), group))
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--slot", type=int, default=3)
    ap.add_argument("--patch", type=int, default=0, help="0-based preset number")
    ap.add_argument("--param", default=None, help='parameter name (default "EFFECT TONE")')
    ap.add_argument("--group", choices=sorted(GROUPS),
                    help="disambiguate names that exist in several groups (default: first match)")
    ap.add_argument("--write", action="append", default=[], metavar="NAME=VALUE",
                    help='write one or more parameters into the EDIT BUFFER, e.g. '
                         '--write "VCF CUTOFF FREQ=100" --write "VCF RESONANCE=200"; '
                         "values outside the declared range are refused unless --force")
    ap.add_argument("--force", action="store_true", help="allow values outside the declared range")
    ap.add_argument("--list", metavar="TEXT", help="list parameters whose name contains TEXT")
    ap.add_argument("--edit", action="store_true",
                    help="read the current (edit) patch buffer `fm.PATCH` (0x03000000) "
                         "instead of the stored patch `fm.usrX`; --slot/--patch are ignored")
    ap.add_argument("--leaf", help="read a whole leaf (e.g. NAME0) raw instead of one value; "
                    "printed as nibbles and decoded as 2 nibbles per character")
    ap.add_argument("--set", type=int, metavar="VALUE",
                    help="WRITE the value into the EDIT BUFFER (needs --edit): the panel "
                         "parameter changes like a knob move (temporary, not stored)")
    ap.add_argument("--watch", action="store_true", help="poll and print on every change")
    ap.add_argument("--poll", type=float, default=0.2)
    ap.add_argument("--plugin", default=os.path.join(os.path.dirname(__file__), "..", "plugins", "JX-3P"))
    ap.add_argument("--timeout", type=float, default=1.5)
    ap.add_argument("--backend", choices=["alsa", "rtmidi"])
    ap.add_argument("--port")
    ap.add_argument("--device-id", type=lambda x: int(x, 0), default=P.DEFAULT_DEVICE_ID)
    ap.add_argument("--gap-ms", type=float, default=0.0)
    ap.add_argument("--wire-pacing", action="store_true")
    ap.add_argument("-v", "--verbose", action="store_true")
    a = ap.parse_args()

    layout = PT.ScriptLayout(os.path.join(a.plugin, "Script", "Script.xml"))
    vals = find_values(layout)
    if a.list is not None:
        for n, leaf, at, size, rng, dflt, grp in vals:
            if a.list.upper() in n.upper():
                print(f"{n:<28} {grp:<6} leaf {leaf.path:<12} +{leaf.offset + at:#06x}  "
                      f"size {size}  range {rng}  default {dflt}")
        return 0
    def resolve(pname: str):
        hits = [v for v in vals if v[0] == pname and (a.group is None or v[6] == a.group)]
        if not hits:
            sys.exit(f"no parameter {pname!r}; try --list")
        if len({h[6] for h in hits}) > 1 and a.group is None:
            print(f"note: {pname!r} exists in groups {sorted({h[6] for h in hits})}; "
                  f"using {hits[0][6]} (--group to choose)", file=sys.stderr)
        return hits[0]

    def declared_range(rng):
        try:
            lo, hi = (int(x) for x in rng.split(","))
            return lo, hi
        except (AttributeError, ValueError):
            return None

    if a.write:
        # edit-buffer writes of named parameters, then exit (no knob needed)
        s, port = T.open_session(a)
        try:
            for item in a.write:
                pname, sep, vtxt = item.rpartition("=")
                if not sep:
                    sys.exit(f"--write wants NAME=VALUE, got {item!r}")
                name, leaf, at, size, rng, dflt, grp = resolve(pname)
                if vtxt.endswith("f"):          # float literal -> IEEE-754 bits, e.g. 0.5f
                    val = struct.unpack("<I", struct.pack("<f", float(vtxt[:-1])))[0]
                else:
                    val = int(vtxt, 0)
                lim = declared_range(rng)
                if lim and not (lim[0] <= val <= lim[1]) and not a.force:
                    sys.exit(f"{name}: {val} is outside the declared range {rng} (--force)")
                if val >> (4 * size):
                    sys.exit(f"{name}: {val} does not fit {size} nibbles")
                addr = PT.a7(GROUPS[grp][0]) + leaf.offset + at
                data = bytes((val >> (4 * (size - 1 - i))) & 15 for i in range(size))

                def rd(addr=addr, size=size):
                    s._pump()
                    s._inbox.clear()
                    s.send(P.build(P.RQ1, addr, P.addr_bytes(size), a.device_id))
                    end = time.monotonic() + a.timeout
                    while time.monotonic() < end:
                        s._pump()
                        for i, m in enumerate(s._inbox):
                            if P.address_of(m) == addr:
                                del s._inbox[i]
                                d = P.sysex_data(m)
                                return int("".join(f"{b & 15:x}" for b in d[:size]) or "0", 16)
                        time.sleep(0.01)
                    return None

                before = rd()
                s.send(P.build(P.DT1, addr, data, a.device_id))
                time.sleep(0.3)
                after = rd()
                def show(v):
                    if v is None or size != 8:
                        return str(v)
                    f = struct.unpack("<f", struct.pack("<I", v))[0]
                    return f"{v:#010x} (float {f:.6g})"

                print(f"{name:<24} {grp:<6} {P.addr_str(addr)}\n"
                      f"    before    {show(before)}\n    wrote     {show(val)}\n"
                      f"    read back {show(after)}")
        finally:
            port.close()
        return 0
    a.param = a.param or "EFFECT TONE"
    if a.leaf:
        lf = next((x for x in layout.leaves["patch"] if x.path == a.leaf), None)
        if lf is None:
            sys.exit(f"no leaf {a.leaf!r}: " + ", ".join(x.path for x in layout.leaves["patch"]))
        name, leaf, at, size, rng, dflt, grp = a.leaf, lf, 0, lf.size, "-", "-", "patch"
    else:
        name, leaf, at, size, rng, dflt, grp = resolve(a.param)
    if a.set is not None and not a.edit:
        sys.exit("--set only writes the edit buffer: add --edit")
    if a.edit:
        base, where = PT.a7(GROUPS[grp][0]), f"edit buffer fm.{grp.upper()}"   # Script.xml, struct fm
    else:
        base, where = (PT.patch_address(a.slot, a.patch, GROUPS[grp][1]),
                       f"slot {a.slot} patch {a.patch} ({grp})")
    addr = base + leaf.offset + at
    print(f"{name}: {where}  address {P.addr_str(addr)}  "
          f"({size} nibbles, range {rng}, default {dflt})")

    s, port = T.open_session(a)

    def read() -> bytes | None:
        rq = P.build(P.RQ1, addr, P.addr_bytes(size), a.device_id)
        s._pump()
        s._inbox.clear()
        s.send(rq)
        end = time.monotonic() + a.timeout
        while time.monotonic() < end:
            s._pump()
            for i, m in enumerate(s._inbox):
                if P.address_of(m) == addr:
                    del s._inbox[i]
                    return P.sysex_data(m)
            time.sleep(0.01)
        return None

    def text(d: bytes) -> str:
        c = bytes(((d[i] & 15) << 4) | (d[i + 1] & 15) for i in range(0, len(d) - 1, 2))
        return c.split(b"\0")[0].decode("latin-1")

    def value(d: bytes) -> int:
        v = 0
        for b in d[:size]:
            v = (v << 4) | (b & 15)
        return v

    try:
        d = read()
        if d is None:
            print("no reply")
            return 1
        if a.leaf:
            print(f"{name} = {text(d)!r}   raw {d[:48].hex(' ')}{' ...' if len(d) > 48 else ''}")
        else:
            extra = ""
            if size == 8:
                extra = f"   float {struct.unpack('<f', struct.pack('<I', value(d)))[0]:.6g}"
            print(f"{name} = {value(d)}  ({value(d):#04x}){extra}   raw {d.hex(' ')}")
        if a.set is not None:
            v = a.set & ((1 << (4 * size)) - 1)
            data = bytes((v >> (4 * (size - 1 - i))) & 15 for i in range(size))
            s.send(P.build(P.DT1, addr, data, a.device_id))
            time.sleep(0.3)
            d = read()
            print(f"wrote {a.set}; read back {value(d) if d else None}")
        if a.watch:
            print("watching; move the knob. Ctrl-C to stop.")
            last = d
            while True:
                time.sleep(a.poll)
                cur = read()
                if cur is not None and cur != last:
                    last = cur
                    shown = repr(text(cur)) if a.leaf else f"{value(cur)}  ({value(cur):#04x})"
                    print(f"{time.strftime('%H:%M:%S')}  {name} = {shown}")
    except KeyboardInterrupt:
        pass
    finally:
        port.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
