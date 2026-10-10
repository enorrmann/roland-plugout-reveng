#!/usr/bin/env python3
"""
patches.py -- the patch bank the plugins write into a PLUG-OUT slot after the
firmware (JUNO-60.vst3 @ 0x1803551a5..0x1803552d0 and 0x180355af0).

* Memory layout of a patch: Script/Script.xml, struct types "patch",
  "patch2", "patch3" (fm.usrX, fm.usr2X, fm.usr3X). Every leaf struct is written
  as one DT1 message.
* Patch data: Patch/*.bin ("KoaBankFile0000N"). Each record holds the leaf
  memory concatenated in Script.xml order.
* Device addresses: computed per slot / patch / struct (@ 0x18036d270).

See PROTOCOL.md section 6.
"""

from __future__ import annotations

import os
import re
import xml.etree.ElementTree as ET
from dataclasses import dataclass

import plugout as P

PATCH_TYPES = ("patch", "patch2", "patch3")      # fm.usrX, fm.usr2X, fm.usr3X

ADDR_PATCH_COUNT = 0x0E100081    # 70 20 01 01  data: number of patches
ADDR_PATCH_DONE = 0x0E100082     # 70 20 01 02  data: 1 if the firmware was already there

# @ 0x180993f88, indexed by slot + (struct - 1) * 4
_PATCH_BASE = [0x5000000, 0x6000000, 0x7000000, 0x0D000000,
               0x9000000, 0xA000000, 0xB000000, 0x0C000000]


def a7(text: str) -> int:
    """Script.xml address/size "01 70" -> 7-bit packed integer."""
    v = 0
    for x in text.split():
        v = (v << 7) + int(x, 16)
    return v


@dataclass
class Leaf:
    path: str
    offset: int          # 7-bit packed, relative to the struct
    size: int
    default: bytes


class ScriptLayout:
    def __init__(self, script_xml: str):
        root = ET.parse(script_xml).getroot()
        self.types = {e.findtext("type"): e for e in root.findall("structType")}
        self.leaves = {t: list(self._leaves(t, 0, "")) for t in PATCH_TYPES}

    @staticmethod
    def _value_size(v) -> int:
        m = re.match(r"int(\d+)x(\d+)$", v.findtext("type"))
        if m:
            return int(m.group(1))
        return a7(v.findtext("size"))

    @staticmethod
    def _encode(v, size: int) -> bytes:
        t, d = v.findtext("type"), v.findtext("default")
        if d is None:
            return bytes(size)
        m = re.match(r"int(\d+)x(\d+)$", t)
        if m:
            n, bits = int(m.group(1)), int(m.group(2))
            x = int(d) & ((1 << (n * bits)) - 1)
            return bytes((x >> (bits * (n - 1 - i))) & ((1 << bits) - 1) for i in range(n))
        if t == "stringNx4":
            s = d.encode("ascii").ljust(size // 2)[:size // 2]
            return bytes(b for ch in s for b in (ch >> 4, ch & 0xF))
        raise ValueError(f"unsupported value type {t}")

    def _leaf_memory(self, e) -> tuple[int, bytes]:
        mem = bytearray()
        off = 0
        for v in e.findall("value"):
            size = self._value_size(v)
            addrs = [a7(a.text) for a in v.findall("address")] or [off]
            enc = self._encode(v, size)
            for a in addrs:
                if len(mem) < a + size:
                    mem += bytes(a + size - len(mem))
                mem[a:a + size] = enc
            if not v.findall("address"):
                off += size
        if e.findtext("size"):
            n = a7(e.findtext("size"))
            mem = (mem + bytes(n))[:n]
        return len(mem), bytes(mem)

    def _leaves(self, t: str, base: int, path: str):
        e = self.types[t]
        subs = e.findall("struct")
        if not subs:
            size, default = self._leaf_memory(e)
            yield Leaf(path or t, base, size, default)
            return
        for c in subs:
            name = c.findtext("name") or c.findtext("type")
            yield from self._leaves(c.findtext("type"), base + a7(c.findtext("address")),
                                    f"{path}.{name}" if path else name)

    def size(self, t: str) -> int:
        return sum(x.size for x in self.leaves[t])

    def default(self, t: str) -> bytes:
        return b"".join(x.default for x in self.leaves[t])


@dataclass
class Patch:
    name: str
    data: dict           # struct type -> leaf memory, concatenated


def load_bank(path: str, layout: ScriptLayout) -> list[Patch]:
    """Read a KoaBankFile: 16-byte magic, model tag, 64 records.

    Record = name[16] + header + leaf memory. Version 3 (JUNO-60, JX-3P) has a
    12-byte header and patch+patch2+patch3; version 2 (SH-101) a 27-byte header
    and only "patch" (the others take the Script.xml defaults).
    """
    with open(path, "rb") as fh:
        d = fh.read()
    magic = d[:16]
    if magic == b"KoaBankFile00003":
        header, types = 12, PATCH_TYPES
    elif magic == b"KoaBankFile00002":
        header, types = 27, ("patch",)
    else:
        raise ValueError(f"{path}: unsupported bank {magic!r}")
    rec = 16 + header + sum(layout.size(t) for t in types)
    count = 64
    tag = len(d) - 16 - count * rec
    if not 0 < tag <= 16 or not d[16:16 + tag].startswith(b"PG-"):
        raise ValueError(f"{path}: record size {rec} does not match Script.xml")
    patches = []
    for i in range(count):
        r = d[16 + tag + i * rec:16 + tag + (i + 1) * rec]
        pos, data = 16 + header, {}
        for t in PATCH_TYPES:
            if t in types:
                data[t] = r[pos:pos + layout.size(t)]
                pos += layout.size(t)
            else:
                data[t] = layout.default(t)
        patches.append(Patch(r[:16].decode("ascii", "replace").rstrip(), data))
    return patches


def patch_address(slot: int, patch: int, struct: int) -> int:
    """Device address of fm.usrX / usr2X / usr3X for a patch (@ 0x18036d270)."""
    if struct == 0:
        bank, num = patch >> 3, patch & 7
        if slot < 2:
            return (((bank << 7) + num) << 14) + (0x4000000 if slot == 0 else 0x8000000)
        return (((bank << 4) + num) << 14) + (0x1C00000 if slot == 2 else 0x1E00000)
    return (patch << 18) + _PATCH_BASE[slot + (struct - 1) * 4]


def patch_messages(layout: ScriptLayout, bank: list[Patch], slot: int, count: int,
                   device_type: int, already_installed: bool,
                   device_id: int = P.DEFAULT_DEVICE_ID) -> list[bytes]:
    """All DT1 writes of the patch transfer, in the plugin's order."""
    def dt1(addr, data):
        return P.build(P.DT1, addr, data, device_id)

    structs = PATCH_TYPES if device_type == 2 else PATCH_TYPES[:1]
    msgs = [dt1(ADDR_PATCH_COUNT, bytes([count & 0x7F]))]
    for p in range(min(count, len(bank))):
        for s, t in enumerate(structs):
            base = patch_address(slot, p, s)
            mem, pos = bank[p].data[t], 0
            for leaf in layout.leaves[t]:
                msgs.append(dt1(base + leaf.offset, mem[pos:pos + leaf.size]))
                pos += leaf.size
    msgs.append(dt1(ADDR_PATCH_DONE, bytes([1 if already_installed else 0])))
    return msgs


def default_bank(plugin_dir: str) -> str:
    d = os.path.join(plugin_dir, "Patch")
    files = sorted(f for f in os.listdir(d) if f.lower().endswith(".bin"))
    if not files:
        raise ValueError(f"no patch banks in {d}")
    return os.path.join(d, files[0])
