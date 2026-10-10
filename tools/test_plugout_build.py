#!/usr/bin/env python3
"""Tests for lzss.py / plugout_image.py / plugout_build.py.

    python3 -m unittest discover -s tools -v

The rebuild tests compress a full region in pure Python and take ~15 s.
"""

from __future__ import annotations

import os
import random
import struct
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
PLUGINS = os.path.join(os.path.dirname(HERE), "plugins")
sys.path.insert(0, HERE)
import lzss  # noqa: E402
import plugout as P  # noqa: E402
import plugout_build as B  # noqa: E402
import plugout_image as I  # noqa: E402


def dat(model: str, name: str) -> str:
    return os.path.join(PLUGINS, model, "Script", name)


HAVE_PLUGINS = all(os.path.exists(dat(m, "Code8_1.Dat")) for m in ("JUNO-60", "JX-3P", "SH-101"))


class LzssTests(unittest.TestCase):
    def test_roundtrip(self):
        rnd = random.Random(1)
        samples = [b"", b"a", bytes(5000), b"abcabcabcabd" * 400,
                   bytes(rnd.randrange(4) for _ in range(20000)),
                   bytes(rnd.randrange(256) for _ in range(3000))]
        for s in samples:
            self.assertEqual(lzss.decode(lzss.encode(s)), s)

    def test_zero_fill(self):
        # a reference into the untouched ring buffer yields 0x00, not spaces
        self.assertEqual(lzss.decode(bytes([0x00, 0xEE, 0xF0]), 3), b"\0\0\0")


class ImageTests(unittest.TestCase):
    def test_region_header(self):
        h = I.RegionHeader(stamp=b"25-0509_11:30:38", tool=b"0.0114FA", load=0x60800000,
                           entry=0x60800000, clen=10, csum=20, ulen=30)
        self.assertEqual(I.RegionHeader.parse(h.pack()), h)
        self.assertEqual(len(h.pack()), I.REGION_HEADER_SIZE)

    def test_region_roundtrip(self):
        plain = I.SYNINFO_MAGIC + b"TEST".ljust(16, b"\0") + struct.pack(
            "<HHII", 0x42, 0x0100, 0x60a00011, 0x60a00100) + bytes(3000)
        h = I.RegionHeader(stamp=b"x", tool=b"y", load=0x60200000, entry=0x60200000)
        raw = I.Region(h, plain).to_raw()
        self.assertEqual(len(raw) % I.BLOCK_SIZE, 0)
        r = I.Region.from_raw(raw)
        self.assertEqual(r.plain, plain)
        self.assertEqual(r.syninfo.name, "TEST")
        self.assertEqual(I.detect_slot("SYSTEM-8", 1, plain), 1)

    def test_relocs(self):
        a = struct.pack("<4I", 0x60a00010, 7, 0x60a00020, 9)
        b = struct.pack("<4I", 0x60c00010, 7, 0x60c00020, 9)
        self.assertEqual(I.diff_relocs(a, b, I.SLOT_STRIDE), [0, 8])
        self.assertEqual(I.relocate(a, [0, 8], I.SLOT_STRIDE), b)
        with self.assertRaises(ValueError):
            I.diff_relocs(a, struct.pack("<4I", 0x60c00010, 8, 0x60c00020, 9), I.SLOT_STRIDE)

    def test_container_layout(self):
        blocks = [bytes(range(256)) * 2, None, None, None]
        c = P.load_bytes(I.build_container("TEST", 0x42, 0x0100, blocks), "test")
        self.assertEqual((c.model, c.model_id, c.version, c.data_count), ("TEST", 0x42, 0x100, 4))
        self.assertEqual(I.flash_blocks(c), blocks)
        self.assertEqual(P.nibbles(P.sysex_data(c.end_message)), I.end_checksum(blocks))

    def test_elf(self):
        elf = I.to_elf([(".r0", 0x61800000, b"\0" * 64)], [("init", 0x61800011, True)], 0x61800011)
        self.assertEqual(elf[:4], b"\x7fELF")
        self.assertEqual(struct.unpack_from("<HHI", elf, 16), (2, 40, 1))


@unittest.skipUnless(HAVE_PLUGINS, "plugin files missing")
class ShippedTests(unittest.TestCase):
    def test_end_checksum(self):
        for model in ("JUNO-60", "JX-3P", "SH-101"):
            for name in sorted(os.listdir(os.path.join(PLUGINS, model, "Script"))):
                if not name.endswith(".Dat") or not name.startswith("Code"):
                    continue
                c = P.load(dat(model, name))
                self.assertEqual(P.nibbles(P.sysex_data(c.end_message)),
                                 I.end_checksum(I.flash_blocks(c)), name)

    def test_regions_and_slots(self):
        for slot in (1, 2, 3):
            proj = B.Project.from_container(P.load(dat("JUNO-60", f"Code8_{slot}.Dat")))
            self.assertEqual(proj.slot, slot)
            s0, s1 = (I.SynInfo.parse(r) for r in proj.regions)
            self.assertEqual((s0.name, s0.model_id, s0.version), ("JUNO-60", 0x0A, 0x0101))
            self.assertEqual(s1.name, "SYSTEM-8")
            self.assertTrue(proj.base(0) <= s0.init < proj.base(0) + len(proj.regions[0]))
            self.assertTrue(proj.base(1) <= s1.init < proj.base(1) + len(proj.regions[1]))

    def test_rebuild_and_retarget(self):
        """unpack slot 1 + relocs from slot 2 -> pack for slot 3 == Roland's Code8_3.Dat."""
        proj = B.Project.from_container(P.load(dat("SH-101", "Code8_1.Dat")))
        B.derive_relocs(proj, B.Project.from_container(P.load(dat("SH-101", "Code8_2.Dat"))))
        for slot in (1, 3):
            with open(dat("SH-101", f"Code8_{slot}.Dat"), "rb") as fh:
                self.assertEqual(proj.build(slot), fh.read(), f"slot {slot}")

    def test_project_files(self):
        with tempfile.TemporaryDirectory() as tmp:
            work = os.path.join(tmp, "p")
            self.assertEqual(B.main(["unpack", dat("SH-101", "Code8_1.Dat"), work,
                                     "--relocs-from", dat("SH-101", "Code8_2.Dat")]), 0)
            self.assertEqual(B.main(["set-id", work, "--name", "SH-101 MOD",
                                     "--version", "1.09"]), 0)
            proj = B.Project.load(work)
            self.assertEqual(proj.meta["version"], "1.09")
            self.assertEqual(len(proj.relocs[1]), 605)
            self.assertEqual(B.main(["patch", work, "--region", "1", "--at", "0x60a00100",
                                     "--hex", "00bf"]), 0)
            self.assertEqual(B.Project.load(work).regions[1][0x100:0x102], b"\x00\xbf")


if __name__ == "__main__":
    unittest.main()
