#!/usr/bin/env python3
"""Tests for plugout.py / plugout_tool.py against a simulated SYSTEM-8.

    python3 -m unittest discover -s tools -v
"""

from __future__ import annotations

import os
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)
import midi_io  # noqa: E402
import patches as PT  # noqa: E402
import plugout as P  # noqa: E402
import plugout_tool as T  # noqa: E402

DATS = {
    "JUNO-60": os.path.join(ROOT, "JUNO-60", "Script", "Code8_1.Dat"),
    "JX-3P": os.path.join(ROOT, "JX-3P", "Script", "Code8_1.Dat"),
    "SH-101": os.path.join(ROOT, "SH-101", "Script", "Code8_1.Dat"),
}


def nib(value: int, n: int) -> bytes:
    return bytes((value >> (4 * (n - 1 - i))) & 0xF for i in range(n))


class FakeClock:
    def __init__(self):
        self.t = 0.0

    def __call__(self):
        return self.t

    def sleep(self, dt):
        self.t += max(dt, 0)


class FakeSystem8:
    """Device side of the protocol, as the plugin expects it to behave."""

    def __init__(self, slots=None, firmware=0x0200, reject_at=None, end_code=0,
                 silent_flow=False):
        self.slots = slots or {0: ("SYSTEM-8", 0x0000, 0x0200)}
        self.count = 4
        self.firmware = firmware
        self.reject_at = reject_at
        self.end_code = end_code
        self.silent_flow = silent_flow
        self.sent: list[bytes] = []
        self.pending: list[bytes] = []
        self.data_seen = 0
        self.flow_before: list[int] = []
        self.started_with = None
        self.ended = False
        self.patch_writes: list[bytes] = []

    def reply(self, address: int, data: bytes):
        self.pending.append(P.build(P.DT1, address | P.REPLY_BIT, data))

    def send(self, msg: bytes):
        assert P.checksum_ok(msg), msg.hex(" ")
        self.sent.append(msg)
        a, d = P.address_of(msg), P.sysex_data(msg)
        if a == P.ADDR_DEVICE_TYPE:
            self.reply(a, b"\x02")
        elif a == P.ADDR_SLOT_INFO:
            i = d[0]
            name, mid, ver = self.slots.get(i, ("", 0xFFFF, 0xFFFF))
            body = nib(self.firmware if i == 0 else 0, 4) + bytes(4) + nib(mid, 4) + nib(ver, 4)
            body += b"".join(nib(ch, 2) for ch in name.encode().ljust(16, b"\0"))
            self.reply(a, body)
        elif a in (P.ADDR_INFO_101, P.ADDR_INFO_102):
            self.reply(a, b"\x04")
        elif a == P.ADDR_SLOT_COUNT:
            self.reply(a, bytes([self.count]))
        elif a == P.ADDR_START:
            self.started_with = d[0]
            self.reply(a, b"\x00")
        elif a == P.ADDR_FLOW:
            self.flow_before.append(self.data_seen)
            if not self.silent_flow:
                self.reply(a, b"\x00\x00\x00\x00")
        elif a in (P.ADDR_DATA_CHUNK, P.ADDR_DATA_FILL):
            if self.data_seen == self.reject_at:
                self.reply(P.ADDR_DATA_ERROR & ~P.REPLY_BIT, b"\x05")
            self.data_seen += 1
        elif a in (PT.ADDR_PATCH_COUNT, PT.ADDR_PATCH_DONE) or a < 0x0E000000:
            self.patch_writes.append(msg)
        elif a == P.ADDR_END:
            self.ended = True
            self.reply(a, bytes([self.end_code]))

    def receive(self):
        out, self.pending = self.pending, []
        return out

    def close(self):
        pass


def session(dev):
    clock = FakeClock()
    return T.Session(dev, clock=clock, sleep=clock.sleep), clock


class CodecTests(unittest.TestCase):
    def test_cipher_roundtrip(self):
        data = os.urandom(4099)
        self.assertEqual(P.decode(P.encode(data)), data)
        self.assertEqual(P.encode(P.decode(data)), data)

    def test_addresses(self):
        self.assertEqual(P.addr_bytes(P.ADDR_START).hex(" "), "70 10 00 00")
        self.assertEqual(P.addr_bytes(P.ADDR_END).hex(" "), "70 10 00 01")
        self.assertEqual(P.addr_bytes(P.ADDR_DATA_CHUNK).hex(" "), "70 30 00 09")
        self.assertEqual(P.addr_bytes(P.ADDR_DATA_FILL).hex(" "), "70 30 01 09")
        self.assertEqual(P.addr_bytes(P.ADDR_FLOW).hex(" "), "70 30 03 00")
        self.assertEqual(P.addr_bytes(P.ADDR_FLOW | P.REPLY_BIT).hex(" "), "71 30 03 00")
        self.assertEqual(P.addr_bytes(P.ADDR_DATA_ERROR).hex(" "), "71 30 00 00")
        self.assertEqual(P.addr_bytes(P.ADDR_DEVICE_TYPE).hex(" "), "70 00 02 00")
        for v in (0, 1, 0x0E0C0180, 0x0FFFFFFF):
            self.assertEqual(P.addr_value(P.addr_bytes(v)), v)

    def test_7in8(self):
        payload = os.urandom(512)
        wire = P.pack_7in8(payload)
        self.assertEqual(len(wire), 586)
        self.assertEqual(P.unpack_7in8(wire), payload)

    def test_build_checksum(self):
        m = P.build(P.DT1, P.ADDR_START, b"\x01")
        self.assertEqual(m.hex(" "), "f0 41 10 00 00 7e 12 70 10 00 00 01 7f f7")
        self.assertTrue(P.checksum_ok(m))

    def test_parser_reassembles_fragments(self):
        p = midi_io.SysexParser()
        m = P.build(P.DT1, P.ADDR_START, b"\x00")
        self.assertEqual(p.feed(m[:5]), [])
        self.assertEqual(p.feed(b"\xf8" + m[5:] + b"\x90\x40"), [m])

    def test_pick_port(self):
        self.assertEqual(midi_io.pick_port(["hw:3,0,0 SYSTEM-8 MIDI 1", "hw:3,0,1 SYSTEM-8 MIDI 2"]),
                         "hw:3,0,1 SYSTEM-8 MIDI 2")
        self.assertEqual(midi_io.pick_port(["SYSTEM-8", "SYSTEM-8 CTRL"]), "SYSTEM-8 CTRL")
        self.assertIsNone(midi_io.pick_port(["Midi Through"]))


@unittest.skipUnless(all(os.path.exists(p) for p in DATS.values()), "plugin files missing")
class ContainerTests(unittest.TestCase):
    def test_shipped_containers(self):
        for model, path in DATS.items():
            c = P.load(path)
            self.assertEqual(c.model, model)
            self.assertEqual(c.model_id, P.MODEL_IDS[model])
            self.assertEqual(c.data_count, 4096)
            self.assertEqual(P.address_of(c.start_message), P.ADDR_START)
            self.assertEqual(P.address_of(c.end_message), P.ADDR_END)
            for m in c.data_messages():
                self.assertTrue(P.checksum_ok(m))
                self.assertIn(P.address_of(m), (P.ADDR_DATA_CHUNK, P.ADDR_DATA_FILL))
            regions = c.image_regions()
            self.assertEqual(len(regions), 2)
            self.assertTrue(regions[0].startswith(b"SYSTEM-8:Plugout"))

    def test_container_path(self):
        self.assertTrue(P.container_path("JUNO-60", 2, 3).endswith(os.path.join("Script", "Code8_3.Dat")))
        self.assertTrue(P.container_path("SH-101", 0, 1).endswith(os.path.join("Script", "Code1.Dat")))

    def test_host_stream(self):
        c = P.load(DATS["JX-3P"])
        msgs = P.host_stream(c, slot=2)
        self.assertEqual(len(msgs), 4098)
        self.assertEqual(P.sysex_data(msgs[0]), b"\x02")
        self.assertTrue(all(P.checksum_ok(m) for m in msgs))
        self.assertEqual(msgs[-1], c.end_message)


@unittest.skipUnless(os.path.exists(DATS["JUNO-60"]), "plugin files missing")
class ProtocolTests(unittest.TestCase):
    def setUp(self):
        self.c = P.load(DATS["JUNO-60"])

    def test_device_info(self):
        dev = FakeSystem8(slots={0: ("SYSTEM-8", 0, 0x200), 2: ("JUNO-60", 0x0A, 0x0101)})
        s, _ = session(dev)
        info = s.device_info()
        self.assertEqual(info["type"], 2)
        self.assertEqual(info["firmware"], 0x200)
        self.assertEqual(info["patches"], 16)
        self.assertEqual(len(info["slots"]), 4)
        self.assertTrue(info["slots"][1].empty)
        self.assertEqual(info["slots"][2].name, "JUNO-60")
        self.assertEqual(info["slots"][2].state_for(self.c), P.SLOT_SAME)
        self.assertEqual(info["slots"][1].state_for(self.c), P.SLOT_EMPTY)

    def test_install_sequence(self):
        dev = FakeSystem8()
        s, clock = session(dev)
        s.install(self.c, slot=3)
        self.assertEqual(dev.started_with, 3)
        self.assertEqual(dev.data_seen, 4096)
        self.assertTrue(dev.ended)
        self.assertEqual(dev.flow_before, list(range(31, 4096, 32)))
        # data records go out byte-for-byte as stored in the container
        data = [m for m in dev.sent if P.address_of(m) in (P.ADDR_DATA_CHUNK, P.ADDR_DATA_FILL)]
        self.assertEqual(data, list(self.c.data_messages()))
        self.assertEqual(dev.sent[-1], self.c.end_message)
        # no extra delays by default: only the device's replies pace the transfer
        self.assertLess(clock.t, 1)

    def test_plugin_pacing_option(self):
        dev = FakeSystem8()
        clock = FakeClock()
        s = T.Session(dev, gap=0.010, wire_pacing=True, clock=clock, sleep=clock.sleep)
        s.install(self.c, slot=1)
        self.assertGreater(clock.t, 170)          # 10 ms + wire time per message

    def test_install_rejected_record(self):
        dev = FakeSystem8(reject_at=100)
        s, _ = session(dev)
        with self.assertRaisesRegex(T.ProtocolError, "record 100"):
            s.install(self.c, slot=1)
        self.assertFalse(dev.ended)

    def test_install_end_error(self):
        s, _ = session(FakeSystem8(end_code=1))
        with self.assertRaisesRegex(T.ProtocolError, "end"):
            s.install(self.c, slot=1)

    def test_flow_timeout_retries(self):
        dev = FakeSystem8(silent_flow=True)
        s, _ = session(dev)
        with self.assertRaisesRegex(T.ProtocolError, "71 30 03 00"):
            s.install(self.c, slot=1)
        flows = [m for m in dev.sent if P.address_of(m) == P.ADDR_FLOW]
        self.assertEqual(len(flows), P.RETRIES)
        self.assertEqual(dev.data_seen, 31)

    def test_device_id_override(self):
        dev = FakeSystem8()
        clock = FakeClock()
        s = T.Session(dev, device_id=0x11, clock=clock, sleep=clock.sleep)
        s.install(self.c, slot=1)
        self.assertTrue(all(m[2] == 0x11 for m in dev.sent))


@unittest.skipUnless(os.path.exists(os.path.join(ROOT, "SH-101", "Patch")), "plugin files missing")
class PatchTests(unittest.TestCase):
    def test_banks(self):
        for model in DATS:
            layout = PT.ScriptLayout(os.path.join(ROOT, model, "Script", "Script.xml"))
            self.assertEqual([layout.size(t) for t in PT.PATCH_TYPES], [640, 3975, 15580])
            bank = PT.load_bank(PT.default_bank(os.path.join(ROOT, model)), layout)
            self.assertEqual(len(bank), 64)
            for p in bank:
                for t in PT.PATCH_TYPES:
                    self.assertEqual(len(p.data[t]), layout.size(t))
                    self.assertTrue(all(b < 0x80 for b in p.data[t]))

    def test_sh101_defaults(self):
        layout = PT.ScriptLayout(os.path.join(ROOT, "SH-101", "Script", "Script.xml"))
        bank = PT.load_bank(PT.default_bank(os.path.join(ROOT, "SH-101")), layout)
        self.assertEqual(bank[0].name, "LD the real 101")
        self.assertEqual(bank[0].data["patch2"], layout.default("patch2"))
        # JUNO-60 "LFO RATE H" default 1058115986 = 0x3F119192, as 8 nibbles
        juno = PT.ScriptLayout(os.path.join(ROOT, "JUNO-60", "Script", "Script.xml"))
        self.assertEqual(juno.default("patch2")[:8].hex(" "), "03 0f 01 01 09 01 09 02")

    def test_addresses(self):
        a = lambda *x: P.addr_str(PT.patch_address(*x))
        self.assertEqual(a(1, 0, 0), "40 00 00 00")
        self.assertEqual(a(1, 9, 0), "41 01 00 00")
        self.assertEqual(a(2, 9, 0), "0e 11 00 00")
        self.assertEqual(a(3, 0, 0), "0f 00 00 00")
        self.assertEqual(a(1, 0, 1), "30 00 00 00")
        self.assertEqual(a(3, 1, 2), "60 10 00 00")

    def test_messages_and_session(self):
        model = "JUNO-60"
        layout = PT.ScriptLayout(os.path.join(ROOT, model, "Script", "Script.xml"))
        bank = PT.load_bank(PT.default_bank(os.path.join(ROOT, model)), layout)
        msgs = PT.patch_messages(layout, bank, slot=2, count=64, device_type=2,
                                 already_installed=False)
        self.assertEqual(len(msgs), 2 + 64 * (14 + 17 + 66))
        self.assertEqual(P.sysex_data(msgs[0]), bytes([64]))
        self.assertEqual(P.address_of(msgs[-1]), PT.ADDR_PATCH_DONE)
        self.assertTrue(all(P.checksum_ok(m) for m in msgs))
        only1 = PT.patch_messages(layout, bank, 1, 64, device_type=0, already_installed=True)
        self.assertEqual(len(only1), 2 + 64 * 14)
        self.assertEqual(P.sysex_data(only1[-1]), b"\x01")
        dev = FakeSystem8()
        s, _ = session(dev)
        s.send_patches(msgs)
        self.assertEqual(dev.patch_writes, msgs)


if __name__ == "__main__":
    unittest.main()
