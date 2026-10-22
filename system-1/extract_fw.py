#!/usr/bin/env python3
"""
extract_fw.py -- extract the SYSTEM-1 OS images (master app, slave app, updater).

Two sources, both with the same region format as the PLUG-OUTs ("SYSTEM-1:..." 0x40-byte
header + LZSS, docs/CUSTOM_PLUGOUTS.md §2-3):

  * firmware/sys1_sys_v130/SYSTEM1_UPD.BIN  Roland's updater, 4 MiB flash image, NOT encrypted:
        0x000000 loader "ESC91.000" (raw)
        0x008000 "SYSTEM-1:Updater"  (2014-03-03)
        0x030000 "SYSTEM-1:Appli"    master app 1.30 (2016-10-26)
        0x130000 "SYSTEM-1:Appli"    slave app 1.30
  * plugins/SYSTEM-1/Script/Code1.Dat       the SYSTEM-1 plug-in's OS update 1.20 (2015-09-24),
        "UPD_POUT_INFO" container, START 70 20 00 00 (PLUG-OUTs use 70 10 00 00):
        flash blocks 0..839 master app, 2048..2312 slave app

Output (work/ is not in git):
    work/sys1_v130/{updater,app_master,app_slave}.bin
    work/sys1_v120/{app_master,app_slave}.bin

    python3 system-1/extract_fw.py
"""
from __future__ import annotations

import os
import re
import struct
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, os.path.join(ROOT, "tools"))
import lzss                      # noqa: E402
import plugout as P              # noqa: E402
import plugout_image as I        # noqa: E402

HDR = re.compile(rb"SYSTEM-1:(Appli  |Updater)")


def regions(img: bytes):
    """(offset, header bytes, decompressed image) for every region header in img"""
    for m in HDR.finditer(img):
        o = m.start()
        h = img[o:o + 0x40]
        load, hsize, csize, csum, entry, dsize = struct.unpack_from("<6I", h, 0x28)
        if hsize != 0x40:
            continue
        comp = img[o + 0x40:o + 0x40 + csize]
        if sum(comp) & 0xFFFFFFFF != csum:
            raise ValueError(f"region at {o:#x}: bad byte sum")
        out = lzss.decode(comp, dsize)
        if len(out) != dsize:
            raise ValueError(f"region at {o:#x}: size {len(out)} != {dsize}")
        yield o, h, out


def save(path: str, data: bytes, what: str) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as fh:
        fh.write(data)
    print(f"  {os.path.relpath(path, ROOT)}: {len(data)} bytes  ({what})")


def main() -> int:
    upd = os.path.join(ROOT, "firmware", "sys1_sys_v130", "SYSTEM1_UPD.BIN")
    if not os.path.exists(upd):
        z = os.path.join(ROOT, "firmware", "sys1_sys_v130.zip")
        with zipfile.ZipFile(z) as zf:
            zf.extract("sys1_sys_v130/SYSTEM1_UPD.BIN", os.path.join(ROOT, "firmware"))
    img = open(upd, "rb").read()
    print(f"{os.path.relpath(upd, ROOT)}: {len(img)} bytes")
    names = {0x8000: "updater", 0x30000: "app_master", 0x130000: "app_slave"}
    for o, h, out in regions(img):
        save(os.path.join(ROOT, "work", "sys1_v130", names[o] + ".bin"), out,
             h[:0x28].decode("ascii", "replace").replace("\0", " ").strip())

    c = P.load(os.path.join(ROOT, "plugins", "SYSTEM-1", "Script", "Code1.Dat"))
    img = b"".join(b if b is not None else b"\xff" * 512 for b in I.flash_blocks(c))
    print(f"plugins/SYSTEM-1/Script/Code1.Dat: {c.model} {c.version >> 8:X}.{c.version & 0xFF:02X}")
    for (o, h, out), name in zip(regions(img), ("app_master", "app_slave")):
        save(os.path.join(ROOT, "work", "sys1_v120", name + ".bin"), out,
             h[:0x28].decode("ascii", "replace").replace("\0", " ").strip())
    return 0


if __name__ == "__main__":
    sys.exit(main())
