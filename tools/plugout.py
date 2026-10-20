#!/usr/bin/env python3
"""
plugout.py -- Roland PLUG-OUT firmware containers (Script/Code*.Dat) and the
USB-MIDI install protocol used by the JUNO-60 / JX-3P / SH-101 plugins.

Everything here was taken from the plugins' own code (JUNO-60.vst3 addresses are
given; JX-3P and SH-101 contain the same routines). See docs/PROTOCOL.md.

Pure stdlib. The MIDI transport lives in midi_io.py.
"""

from __future__ import annotations

import os
import struct
from dataclasses import dataclass

# ---------------------------------------------------------------------------
# container
# ---------------------------------------------------------------------------

DELTA_IV = 0xBACADABE           # mov r9d, 0xbacadabe          @ 0x1803564a5
HEADER_SIZE = 0x80              # cmp dword [rcx], 0x80        @ 0x1803564e3
TYPE_NAME = b"PLUGOUT_SYS_INF"

# Plugin model-id words (word at container +0x24, compared @ 0x180356526).
MODEL_IDS = {"SH-101": 0x01, "JX-3P": 0x08, "JUNO-60": 0x0A}


def decode(data: bytes, iv: int = DELTA_IV) -> bytes:
    """Undo the plugin's running XOR: plain[i] = c[i] ^ c[i-1] (32-bit LE words)."""
    n = len(data) & ~3
    words = struct.unpack_from(f"<{n // 4}I", data)
    out, prev = [], iv
    for w in words:
        out.append(w ^ prev)
        prev = w
    return struct.pack(f"<{len(out)}I", *out) + data[n:]


def encode(plain: bytes, iv: int = DELTA_IV) -> bytes:
    """Inverse of decode(): c[i] = p[i] ^ c[i-1]."""
    n = len(plain) & ~3
    words = struct.unpack_from(f"<{n // 4}I", plain)
    out, prev = [], iv
    for w in words:
        prev = w ^ prev
        out.append(prev)
    return struct.pack(f"<{len(out)}I", *out) + plain[n:]


@dataclass
class Container:
    """A decoded Code*.Dat.

    Header (all little endian):
        +0x00 u32  0x80
        +0x04 char[16] "PLUGOUT_SYS_INF"
        +0x14 char[16] model name ("JUNO-60", ...)
        +0x24 u16  model id      (0x0A JUNO-60, 0x08 JX-3P, 0x01 SH-101)
        +0x26 u16  version       (shown by the plugin as "%X.%02X")
        +0x30 u32 offset, u32 slot size   -> START record
        +0x40 u32 offset, u32 slot size   -> END record
        +0x50 u32 offset, u32 stride, u32 count -> DATA records
    Each record is <u32 length><complete SysEx message>.
    """
    path: str
    plain: bytes

    @property
    def model(self) -> str:
        return self.plain[0x14:0x24].split(b"\0")[0].decode("ascii", "replace")

    @property
    def model_id(self) -> int:
        return struct.unpack_from("<H", self.plain, 0x24)[0]

    @property
    def version(self) -> int:
        return struct.unpack_from("<H", self.plain, 0x26)[0]

    def _record(self, offset: int) -> bytes:
        length = struct.unpack_from("<I", self.plain, offset)[0]
        body = self.plain[offset + 4:offset + 4 + length]
        if len(body) != length or body[:1] != b"\xf0" or body[-1:] != b"\xf7":
            raise ValueError(f"{self.path}: bad record at {offset:#x}")
        return body

    @property
    def start_message(self) -> bytes:
        return self._record(struct.unpack_from("<I", self.plain, 0x30)[0])

    @property
    def end_message(self) -> bytes:
        return self._record(struct.unpack_from("<I", self.plain, 0x40)[0])

    @property
    def data_count(self) -> int:
        return struct.unpack_from("<I", self.plain, 0x58)[0]

    def data_message(self, index: int) -> bytes:
        base, stride = struct.unpack_from("<II", self.plain, 0x50)
        return self._record(base + index * stride)

    def data_messages(self):
        for i in range(self.data_count):
            yield self.data_message(i)

    def image_regions(self) -> list[bytes]:
        """Firmware payload, one bytes object per run of data chunks."""
        regions, cur, prev_chunk = [], bytearray(), False
        for m in self.data_messages():
            is_chunk = address_of(m) == ADDR_DATA_CHUNK
            if is_chunk:
                if not prev_chunk:
                    cur = bytearray()
                    regions.append(cur)
                cur += unpack_7in8(sysex_data(m))
            prev_chunk = is_chunk
        return [bytes(r) for r in regions]

    def summary(self) -> str:
        kinds: dict[str, int] = {}
        bad = 0
        for m in [self.start_message, self.end_message, *self.data_messages()]:
            a = address_of(m)
            kinds[addr_str(a)] = kinds.get(addr_str(a), 0) + 1
            bad += not checksum_ok(m)
        return "\n".join([
            f"file        : {self.path}",
            f"model       : {self.model} (id {self.model_id:#04x}, "
            f"version {self.version >> 8:X}.{self.version & 0xFF:02X})",
            f"start msg   : {self.start_message.hex(' ')}",
            f"end msg     : {self.end_message.hex(' ')}",
            f"data records: {self.data_count}",
            "messages    : " + ", ".join(f"{k} x{v}" for k, v in kinds.items()),
            f"bad checksum: {bad}",
        ])


def load(path: str) -> Container:
    with open(path, "rb") as fh:
        return load_bytes(fh.read(), path)


def load_bytes(data: bytes, path: str = "<memory>") -> Container:
    plain = decode(data)
    if (struct.unpack_from("<I", plain, 0)[0] != HEADER_SIZE
            or plain[4:4 + len(TYPE_NAME)] != TYPE_NAME):
        raise ValueError(f"{path}: not a PLUGOUT_SYS_INF container")
    return Container(path, plain)


def container_path(plugin_dir: str, device_type: int, slot: int) -> str:
    """The file the plugin picks (path builder @ 0x180353480).

    SYSTEM-1 / SYSTEM-1m (type 0/1) -> Script/Code1.Dat
    SYSTEM-8 (type 2)               -> Script/Code8_<slot>.Dat
    """
    script = os.path.join(plugin_dir, "Script")
    if device_type in (0, 1):
        return os.path.join(script, "Code1.Dat")
    if device_type == 2:
        return os.path.join(script, f"Code8_{slot}.Dat")
    raise ValueError(f"unknown device type {device_type}")


# ---------------------------------------------------------------------------
# Roland SysEx
# ---------------------------------------------------------------------------

ROLAND = 0x41
MODEL_PLUGOUT = bytes([0x00, 0x00, 0x7E])    # <modelID> in Script/Script.xml
RQ1, DT1 = 0x11, 0x12
DEFAULT_DEVICE_ID = 0x10


def addr_bytes(value: int) -> bytes:
    """28-bit address -> 4 x 7-bit bytes (helper @ 0x18026a4d0)."""
    return bytes([(value >> 21) & 0x7F, (value >> 14) & 0x7F,
                  (value >> 7) & 0x7F, value & 0x7F])


def addr_value(b: bytes) -> int:
    """4 x 7-bit bytes -> 28-bit address (helper @ 0x18026a010)."""
    v = 0
    for x in b:
        v = (v << 7) + x
    return v


def addr_str(value: int) -> str:
    return addr_bytes(value).hex(" ")


def checksum(payload: bytes) -> int:
    return (-sum(payload)) & 0x7F


def build(command: int, address: int, data: bytes,
          device_id: int = DEFAULT_DEVICE_ID) -> bytes:
    a = addr_bytes(address)
    return (bytes([0xF0, ROLAND, device_id]) + MODEL_PLUGOUT + bytes([command])
            + a + bytes(data) + bytes([checksum(a + bytes(data)), 0xF7]))


def is_plugout_sysex(m: bytes) -> bool:
    return (len(m) >= 13 and m[0] == 0xF0 and m[1] == ROLAND
            and m[3:6] == MODEL_PLUGOUT and m[-1] == 0xF7)


def address_of(m: bytes) -> int:
    return addr_value(m[7:11])


def sysex_data(m: bytes) -> bytes:
    return m[11:-2]


def checksum_ok(m: bytes) -> bool:
    return checksum(m[7:-2]) == m[-2]


def with_device_id(m: bytes, device_id: int) -> bytes:
    return m[:2] + bytes([device_id]) + m[3:]


def with_data(m: bytes, data: bytes) -> bytes:
    """Replace the data field and recompute the checksum (@ 0x18042f1a0)."""
    a = m[7:11]
    return m[:11] + bytes(data) + bytes([checksum(a + bytes(data)), 0xF7])


def nibbles(b: bytes) -> int:
    """Value sent as one 4-bit nibble per byte, most significant first."""
    v = 0
    for x in b:
        v = (v << 4) | (x & 0x0F)
    return v


def unpack_7in8(wire: bytes) -> bytes:
    """Groups of <msb byte><up to 7 bytes>; bit j of msb is bit 7 of byte j.

    A 586-byte data field = 73 full groups + one group of 1 byte -> 512 bytes.
    """
    out = bytearray()
    for i in range(0, len(wire), 8):
        msb = wire[i]
        for j, b in enumerate(wire[i + 1:i + 8]):
            out.append(b | (((msb >> j) & 1) << 7))
    return bytes(out)


def pack_7in8(payload: bytes) -> bytes:
    out = bytearray()
    for i in range(0, len(payload), 7):
        grp = payload[i:i + 7]
        out.append(sum(((b >> 7) & 1) << j for j, b in enumerate(grp)))
        out += bytes(b & 0x7F for b in grp)
    return bytes(out)


# ---------------------------------------------------------------------------
# install protocol (JUNO-60.vst3 @ 0x1803547c0 and 0x180352b50)
# ---------------------------------------------------------------------------

# Every host write goes to a 0x0E0xxxxx address ("70 ..") and the device
# answers on the same address with bit 21 set (0x0E2xxxxx, "71 ..").
REPLY_BIT = 0x00200000

ADDR_START = 0x0E040000        # 70 10 00 00  data: [slot | flags]   -> 71 10 00 00 (1 byte, 0 = ok)
ADDR_END = 0x0E040001          # 70 10 00 01  data: from container  -> 71 10 00 01 (1 byte, 0 = ok)
ADDR_DATA_CHUNK = 0x0E0C0009   # 70 30 00 09  586 bytes = 512 bytes 7-in-8
ADDR_DATA_FILL = 0x0E0C0089    # 70 30 01 09  data 01 (filler record)
ADDR_FLOW = 0x0E0C0180         # 70 30 03 00  data 00 -> 71 30 03 00 (4 bytes)
ADDR_DATA_ERROR = 0x0E2C0000   # 71 30 00 00  (1 byte) device rejects a data record

ADDR_SLOT_INFO = 0x0E000080    # 70 00 01 00  data [slot] -> 0x30 bytes
ADDR_DEVICE_TYPE = 0x0E000100  # 70 00 02 00  -> 1 byte: 0 SYSTEM-1, 1 SYSTEM-1m, 2 SYSTEM-8
ADDR_INFO_101 = 0x0E000101     # 70 00 02 01  -> 1 byte (patch banks?)
ADDR_INFO_102 = 0x0E000102     # 70 00 02 02  -> 1 byte (patches per bank?)
ADDR_SLOT_COUNT = 0x0E000103   # 70 00 02 03  -> 1 byte

FLOW_EVERY = 32                # flow query before data record i when i % 32 == 31
START_TIMEOUT = 5.0            # session timeout 0x1388 ms, 3 attempts
END_TIMEOUT = 20.0             # 0x4e20 ms for the END record
RETRIES = 3
FLAG_NOT_AUTHORIZED = 0x40     # set by the plugin when Roland Cloud says "Not Authorized."

DEVICE_TYPES = {0: "SYSTEM-1", 1: "SYSTEM-1m", 2: "SYSTEM-8"}
MIN_FIRMWARE = {0: 0x130, 1: 0x130, 2: 0x131}

# Slot states, as computed @ 0x1803532b6
SLOT_EMPTY, SLOT_SAME, SLOT_OTHER_VERSION, SLOT_OTHER_MODEL = 0, 1, 2, 3


@dataclass
class SlotInfo:
    index: int
    model_id: int
    version: int
    name: str

    raw_name: bytes = b""

    @property
    def empty(self) -> bool:
        return self.model_id == 0xFFFF

    def state_for(self, c: Container) -> int:
        if self.model_id == c.model_id and self.raw_name == c.plain[0x14:0x24]:
            return SLOT_SAME if self.version == c.version else SLOT_OTHER_VERSION
        if self.model_id in (0xFFFF, c.model_id):
            return SLOT_EMPTY
        return SLOT_OTHER_MODEL

    def __str__(self) -> str:
        if self.empty:
            return f"slot {self.index}: (none)"
        return (f"slot {self.index}: {self.name or '?'} id {self.model_id:#06x} "
                f"version {self.version >> 8:X}.{self.version & 0xFF:02X}")


def parse_slot_info(index: int, data: bytes) -> SlotInfo:
    """0x30-byte answer to ADDR_SLOT_INFO (@ 0x1803531e0).

    bytes 0-3 version of the device firmware (slot 0 only), 8-11 model id,
    12-15 version, 16-47 name: 16 characters, each sent as two nibbles
    (decoder @ 0x180355d30).
    """
    raw = bytes(nibbles(data[i:i + 2]) for i in range(16, 48, 2))
    return SlotInfo(index=index,
                    model_id=nibbles(data[8:12]),
                    version=nibbles(data[12:16]),
                    name=raw.split(b"\0")[0].decode("ascii", "replace").strip(),
                    raw_name=raw)


def wire_seconds(nbytes: int) -> float:
    """Time on a 31250-baud MIDI cable (the plugin's pacing uses len*1000/3125 ms)."""
    return nbytes / 3125.0


def host_stream(c: Container, slot: int, flags: int = 0,
                device_id: int | None = None) -> list[bytes]:
    """Every message the host writes, in order (without the flow queries)."""
    start = with_data(c.start_message, bytes([(flags & 0x70) | (slot & 0x0F)]))
    msgs = [start, *c.data_messages(), c.end_message]
    if device_id is not None:
        msgs = [with_device_id(m, device_id) for m in msgs]
    return msgs


if __name__ == "__main__":
    import sys
    for p in sys.argv[1:]:
        print(load(p).summary(), end="\n\n")
