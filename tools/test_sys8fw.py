"""Tests for sys8fw.py against the shipped firmware/SYSTEM8_UP.BIN."""
import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sys8fw as F  # noqa: E402

UP = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "firmware", "SYSTEM8_UP.BIN")


@unittest.skipUnless(os.path.exists(UP), "firmware/SYSTEM8_UP.BIN not present")
class Sys8Fw(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.raw = open(UP, "rb").read()
        cls.members = F.tar_members(cls.raw)

    def test_tar_roundtrip_is_exact(self):
        self.assertEqual(F.tar_build(self.members), self.raw)

    def test_members(self):
        names = [os.path.basename(m["name"]) for m in self.members]
        self.assertEqual(names, ["p1y01_m0c0a_up.bin", "p1y01_p0m.bin", "p1y01_p0s.bin",
                                 "p1y01_pl1m.bin", "p1y01_pl1s.bin", "p1y01_pl2m.bin"])

    def test_os_header(self):
        o = F.parse_os(self.members[0]["data"])
        self.assertEqual((o["name"], o["load"], o["payload_size"]), ("App1_Main", 0x60000000, 0x2CDA70))
        self.assertIn("VER.1.32", o["trailer_text"])
        self.assertEqual(o["header_size"] + o["payload_size"] + 0x40, o["file_size"])

    def test_plugout_roundtrip_and_symbols(self):
        d = self.members[2]["data"]                      # p0s: base engine
        p = F.parse_plugout(d)
        self.assertEqual(len(p["regions"]), 1)
        meta = {"pre": p["pre"], "regions": [{
            "offset": r["offset"], "stamp": r["header"].stamp.hex(), "tool": r["header"].tool.hex(),
            "load": r["header"].load, "entry": r["header"].entry, "magic": r["header"].magic.hex()}
            for r in p["regions"]]}
        self.assertEqual(F.build_plugout(meta, [p["regions"][0]["plain"]]), d)
        base = F.image_base(p["regions"][0]["plain"], 0)
        self.assertEqual(base, 0x60800000)
        cls = {c["name"] for c in F.find_classes(p["regions"][0]["plain"], base)}
        self.assertTrue({"CDSPSystem8Vco", "CDSPSystem8Vcf", "CPrmDSPSystem8"} <= cls)

    def test_preload_has_two_regions(self):
        p = F.parse_plugout(self.members[5]["data"])    # JX-3P
        self.assertEqual([r["offset"] for r in p["regions"]], [0x40, 0x80000])


if __name__ == "__main__":
    unittest.main()
