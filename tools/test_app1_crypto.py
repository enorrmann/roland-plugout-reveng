"""Tests for app1_crypto.py -- the App1_Main 3DES payload cipher.

The cipher is checked three ways:
  * against the MC-101 reference plaintext (../mc-101-re), if that tree is present
  * against dumps/os_full.bin, the SYSTEM-8 OS as it runs in BMC SDRAM
  * by a full encrypt -> decrypt round trip of the shipped Roland container

The 3DES key is PRIVATE and is never stored in this repository. Tests that need
key material read it from the environment and skip when it is absent:

    APP1_KEY_0xB6   24-byte key, hex, for slot 0xB6 (System-8 App1_Main)
    APP1_KEY_0xB5   24-byte key, hex, for slot 0xB5
    APP1_BOOTROM    path to a silicon Boot ROM dump, used instead of the above

accept: with keys supplied, the ported cipher reproduces the MC-101 reference
pair exactly; the SYSTEM-8 payload decrypts to a valid ARM image whose text
matches live SDRAM; and re-encrypting the decrypted payload reproduces Roland's
file byte for byte. Without keys, the structural tests still run.
"""
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import app1_crypto as C  # noqa: E402

_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.join(_HERE, "..")
UN = os.path.join(_ROOT, "firmware", "sys8_sys_v132", "untared", "p1y01_m0c0a_up.bin")
DUMP = os.path.join(_ROOT, "dumps", "os_full.bin")
MC101 = os.path.join(_ROOT, "..", "mc-101-re", "vendor", "mc101_sys_v182",
                     "extracts", "RPG69_C0A")


def _key_for(idx: int):
    """(key, source) from the environment, or (None, reason)."""
    bootrom = os.environ.get("APP1_BOOTROM")
    if bootrom and os.path.exists(bootrom):
        with open(bootrom, "rb") as fh:
            return C.key_from_bootrom(fh.read(), idx), f"APP1_BOOTROM {bootrom}"
    env = os.environ.get(f"APP1_KEY_0x{idx:02X}")
    if env:
        return C.parse_key_hex(env), f"APP1_KEY_0x{idx:02X}"
    return None, f"set APP1_KEY_0x{idx:02X} or APP1_BOOTROM to run this test"


needs_key_8 = unittest.skipIf(_key_for(0xB6)[0] is None, _key_for(0xB6)[1])
needs_key_ref = unittest.skipIf(_key_for(0xB6)[0] is None, _key_for(0xB6)[1])
needs_crypto = unittest.skipIf(C.DES3 is None, "pycryptodome not installed")


@needs_crypto
class KeyHandling(unittest.TestCase):
    """The key must be a parameter, never a built-in."""

    def test_no_builtin_key_table(self):
        self.assertFalse(hasattr(C, "KEY_SLOTS"),
                         "the key table must not be embedded in the tool")

    def test_resolve_requires_a_key(self):
        with self.assertRaises(SystemExit):
            C.resolve_key(None, 0xB6, None, None)

    def test_key_parsing_accepts_separators(self):
        raw = "27" * 24
        spaced = " ".join(raw[i:i + 2] for i in range(0, 48, 2))
        self.assertEqual(C.parse_key_hex(raw), C.parse_key_hex(spaced))
        self.assertEqual(C.parse_key_hex(raw.upper()), C.parse_key_hex(raw))

    def test_key_parsing_rejects_wrong_length(self):
        for bad in ("00" * 8, "00" * 16, "00" * 32):
            with self.assertRaises(ValueError):
                C.parse_key_hex(bad)

    def test_bootrom_table_layout(self):
        # addresses are public: they are offsets in the documented key table
        self.assertEqual(C.bootrom_addr(0xB5), 0x012034A8)
        self.assertEqual(C.bootrom_addr(0xB6), 0x012034C0)


@needs_crypto
@unittest.skipUnless(os.path.exists(UN), "firmware/sys8_sys_v132/untared not present")
class HeaderOnly(unittest.TestCase):
    """Structural facts that hold without any key material."""

    @classmethod
    def setUpClass(cls):
        with open(UN, "rb") as fh:
            cls.info = C.parse_header(fh.read())

    def test_header_fields(self):
        i = self.info
        self.assertEqual((i["name"], i["load"]), ("App1_Main", 0x60000000))
        self.assertEqual(i["payload_size"], 0x2CDA70)
        self.assertTrue(i["encrypted"])
        self.assertEqual(i["crypto_mode"], 2)          # 3-key 3DES
        self.assertEqual(i["key_index"], 0xB6)
        self.assertEqual(len(i["trailer"]), 0x40)

    def test_payload_is_block_aligned(self):
        self.assertEqual(self.info["payload_size"] % 8, 0)


@needs_crypto
@unittest.skipUnless(os.path.exists(MC101) and os.path.exists(os.path.join(MC101, "header.bin")),
                     "mc-101-re not present")
class PortedMechanism(unittest.TestCase):
    """The ported cipher must reproduce the MC-101 reference plaintext exactly."""

    @needs_key_ref
    def test_matches_mc101_reference(self):
        with open(os.path.join(MC101, "header.bin"), "rb") as fh:
            idx = C.parse_header(fh.read())["key_index"]
        key, src = _key_for(idx)
        with open(os.path.join(MC101, "App1_Main.enc"), "rb") as fh:
            ct = fh.read(1 << 20)
        with open(os.path.join(MC101, "App1_Main_decrypted.bin"), "rb") as fh:
            want = fh.read(1 << 20)
        self.assertEqual(C.transform(ct, key, decrypt=True), want, f"key from {src}")


@needs_crypto
@unittest.skipUnless(os.path.exists(UN), "firmware/sys8_sys_v132/untared not present")
class System8Container(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with open(UN, "rb") as fh:
            cls.blob = fh.read()
        cls.info = C.parse_header(cls.blob)
        cls.key, cls.key_src = _key_for(cls.info["key_index"])
        cls.plain = C.decrypt_payload(cls.blob, cls.key) if cls.key else None

    @needs_key_8
    def test_entry_point(self):
        ok, why = C.looks_like_arm(self.plain)
        self.assertTrue(ok, why)
        self.assertEqual(self.plain[:4].hex(), "b0480168")   # ldr r0,[pc,#0x...]

    @needs_key_8
    def test_matches_live_sdram_dump(self):
        """dumps/os_full.bin is the same image after running: identical up to the
        first live-RAM variable."""
        if not os.path.exists(DUMP):
            self.skipTest("dumps/os_full.bin not present")
        with open(DUMP, "rb") as fh:
            live = fh.read()
        self.assertEqual(len(self.plain), len(live))
        first = next(i for i, (a, b) in enumerate(zip(self.plain, live)) if a != b)
        self.assertEqual(self.plain[:first], live[:first])
        # the code segment must match completely -- no differences inside .text
        self.assertGreater(first, 0x12C000, f"text mismatch from 0x{first:X}")

    @needs_key_8
    def test_reencrypt_reproduces_shipped_file(self):
        self.assertEqual(C.rebuild_container(self.blob, self.plain, self.key), self.blob)

    @needs_key_8
    def test_modified_payload_roundtrip(self):
        """A patched image survives encrypt -> decrypt intact."""
        mod = bytearray(self.plain)
        mod[0x1000:0x1010] = bytes(range(16))
        container = C.rebuild_container(self.blob, bytes(mod), self.key)
        self.assertEqual(len(container), len(self.blob))
        self.assertEqual(container[:0x60], self.blob[:0x60])          # clear header kept
        self.assertEqual(container[0x60 + len(mod):], self.blob[0x60 + len(self.plain):])
        self.assertEqual(C.decrypt_payload(container, self.key), bytes(mod))
        # the patch must not appear in cleartext inside the encrypted span
        self.assertNotEqual(container[0x60:0x60 + len(mod)],
                            self.blob[0x60:0x60 + len(self.plain)])


if __name__ == "__main__":
    unittest.main()
