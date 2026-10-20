#!/usr/bin/env python3
"""
plugout_image.py -- the firmware carried inside a Code*.Dat, and how to rebuild it.

Layers (see docs/CUSTOM_PLUGOUTS.md):

    Code*.Dat (XOR container, plugout.py)
      -> 4096 (SYSTEM-8) / 2048 (SYSTEM-1) records of 512 flash bytes
           first half  = region 0 ("model": SYNINFO name/id/version, panel data)
           second half = region 1 ("engine": the sound engine code)
           unused blocks are sent as filler records and count as 0xFF
      -> region = 0x40-byte "SYSTEM-8:Plugout" header + LZSS stream (lzss.py),
                  padded with 0xFF to 512 bytes
      -> decompressed image, linked at a fixed address per slot

END message data = 16-bit sum of the whole flash image (all records, fillers
as 512 x 0xFF), sent as 4 nibbles. Verified on every shipped container.

Pure stdlib.
"""

from __future__ import annotations

import struct
from dataclasses import dataclass, replace

import lzss
import plugout as P

BLOCK_SIZE = 512
REGION_MAGICS = (b"SYSTEM-8:Plugout", b"SYSTEM-1:Plugout")
PRELOAD_MAGIC = b"PRELOAD_PLUGOUT\0"
SYNINFO_MAGIC = b"SYSTEM8_SYNINFO\0"
REGION_HEADER_SIZE = 0x40
FILL_DATA = b"\x01"                 # data byte of a 70 30 01 09 filler record

# Run addresses of the decompressed regions. On SYSTEM-8 each slot has its own
# build: the three Code8_<n>.Dat differ only in absolute pointers, which move by
# SLOT_STRIDE per slot. Slot 0 is the SYSTEM-8 itself (p1y01_p0m/p0s.bin in
# SYSTEM8_UP.BIN). SYSTEM-1 has a single slot and runs the regions where the
# region header says.
SLOT_STRIDE = 0x200000
LINK_BASE = {
    "SYSTEM-8": (0x61600000, 0x60800000),   # + slot * SLOT_STRIDE
    "SYSTEM-1": (0x60800000, 0x60200000),
}
RECORD_COUNT = {"SYSTEM-8": 4096, "SYSTEM-1": 2048}

CONTAINER_TYPE = b"PLUGOUT_SYS_INF\0"
START_OFFSET, END_OFFSET, DATA_OFFSET = 0x80, 0xA0, 0xC0
SMALL_RECORD, DATA_STRIDE = 0x20, 0x260


def link_base(device: str, region: int, slot: int) -> int:
    base = LINK_BASE[device][region]
    return base + slot * SLOT_STRIDE if device == "SYSTEM-8" else base


def device_for_count(count: int) -> str:
    for dev, n in RECORD_COUNT.items():
        if n == count:
            return dev
    raise ValueError(f"unexpected record count {count}")


# ---------------------------------------------------------------------------
# region header + SYNINFO
# ---------------------------------------------------------------------------

@dataclass
class RegionHeader:
    """0x40 bytes, little endian.

        +0x00 char[16] "SYSTEM-8:Plugout" ("SYSTEM-1:Plugout" in Code1.Dat)
        +0x10 char[16] build stamp, e.g. "25-0509_11:30:38"
        +0x20 char[8]  "0.0114FA" (same in all 2025 builds, "0.01136E" in 2022)
        +0x28 u32 load address (0x60800000 region 0, 0x60200000 region 1)
        +0x2C u32 header size (0x40)
        +0x30 u32 compressed size
        +0x34 u32 32-bit byte sum of the compressed stream
        +0x38 u32 entry (= load address)
        +0x3C u32 decompressed size
    """
    stamp: bytes
    tool: bytes
    load: int
    entry: int
    clen: int = 0
    csum: int = 0
    ulen: int = 0
    magic: bytes = REGION_MAGICS[0]

    @classmethod
    def parse(cls, data: bytes) -> "RegionHeader":
        if data[:16] not in REGION_MAGICS:
            raise ValueError("not a SYSTEM-8/SYSTEM-1:Plugout region")
        load, hsize, clen, csum, entry, ulen = struct.unpack_from("<6I", data, 0x28)
        if hsize != REGION_HEADER_SIZE:
            raise ValueError(f"unexpected region header size {hsize:#x}")
        return cls(stamp=data[16:32], tool=data[32:40], load=load, entry=entry,
                   clen=clen, csum=csum, ulen=ulen, magic=data[:16])

    def pack(self) -> bytes:
        return (self.magic + self.stamp.ljust(16, b"\0")[:16] + self.tool.ljust(8, b"\0")[:8]
                + struct.pack("<6I", self.load, REGION_HEADER_SIZE, self.clen,
                              self.csum, self.entry, self.ulen))


@dataclass
class SynInfo:
    """Start of a decompressed region.

        +0x00 "SYSTEM8_SYNINFO\\0"
        +0x10 char[16] name     (region 0: the PLUG-OUT name the device reports)
        +0x20 u16 model id, u16 version (region 0: same as the container header)
        +0x24 u32 init function (Thumb, returns a descriptor table)
        +0x28 u32 pointer into the image (end of the init data)
        +0x2C "SYSTEM8_SYNINFO\\0" again
    """
    name: str
    model_id: int
    version: int
    init: int
    ptr: int

    @classmethod
    def parse(cls, plain: bytes) -> "SynInfo | None":
        if plain[:16] != SYNINFO_MAGIC:
            return None
        model_id, version, init, ptr = struct.unpack_from("<HHII", plain, 0x20)
        name = plain[16:32].split(b"\0")[0].decode("ascii", "replace")
        return cls(name, model_id, version, init, ptr)


def stamp_identity(plain: bytes, name: str, model_id: int, version: int) -> bytes:
    """Write name / model id / version into a region's SYNINFO."""
    if plain[:16] != SYNINFO_MAGIC:
        raise ValueError("region has no SYNINFO block")
    b = bytearray(plain)
    b[16:32] = encode_name(name)
    struct.pack_into("<HH", b, 0x20, model_id, version)
    return bytes(b)


def encode_name(name: str) -> bytes:
    raw = name.encode("ascii")
    if len(raw) > 15:
        raise ValueError(f"name {name!r} is longer than 15 characters")
    return raw.ljust(16, b"\0")


# ---------------------------------------------------------------------------
# regions
# ---------------------------------------------------------------------------

@dataclass
class Region:
    header: RegionHeader
    plain: bytes

    @classmethod
    def from_raw(cls, raw: bytes) -> "Region":
        h = RegionHeader.parse(raw)
        comp = raw[REGION_HEADER_SIZE:REGION_HEADER_SIZE + h.clen]
        if len(comp) != h.clen:
            raise ValueError("region truncated")
        if sum(comp) & 0xFFFFFFFF != h.csum:
            raise ValueError("region byte sum mismatch")
        plain = lzss.decode(comp, h.ulen)
        if len(plain) != h.ulen:
            raise ValueError("region decompresses to the wrong size")
        return cls(h, plain)

    @property
    def syninfo(self) -> SynInfo | None:
        return SynInfo.parse(self.plain)

    def to_raw(self) -> bytes:
        comp = lzss.encode(self.plain)
        h = replace(self.header, clen=len(comp), csum=sum(comp) & 0xFFFFFFFF,
                    ulen=len(self.plain))
        raw = h.pack() + comp
        return raw + b"\xff" * (-len(raw) % BLOCK_SIZE)


def split_preload(data: bytes) -> bytes:
    """SYSTEM8_UP.BIN pl*.bin files wrap one region in a PRELOAD_PLUGOUT header."""
    if data[:16] == PRELOAD_MAGIC:
        return data[REGION_HEADER_SIZE:]
    return data


# ---------------------------------------------------------------------------
# flash image <-> container
# ---------------------------------------------------------------------------

def flash_blocks(c: P.Container) -> list[bytes | None]:
    """512-byte blocks in record order; None for a filler record."""
    out = []
    for m in c.data_messages():
        a = P.address_of(m)
        if a == P.ADDR_DATA_CHUNK:
            block = P.unpack_7in8(P.sysex_data(m))
            if len(block) != BLOCK_SIZE:
                raise ValueError("data record is not 512 bytes")
            out.append(block)
        elif a == P.ADDR_DATA_FILL:
            out.append(None)
        else:
            raise ValueError(f"unexpected record {P.addr_str(a)}")
    return out


def raw_regions(blocks: list[bytes | None]) -> list[bytes]:
    """Each half of the flash image holds one region, from its first block."""
    half = len(blocks) // 2
    out = []
    for i in range(2):
        part = blocks[i * half:(i + 1) * half]
        n = next((k for k, b in enumerate(part) if b is None), len(part))
        if any(b is not None for b in part[n:]):
            raise ValueError(f"region {i}: data after a filler record")
        out.append(b"".join(part[:n]))
    return out


def end_checksum(blocks: list[bytes | None]) -> int:
    return sum(0xFF * BLOCK_SIZE if b is None else sum(b) for b in blocks) & 0xFFFF


def nibbles4(value: int) -> bytes:
    return bytes([(value >> 12) & 0xF, (value >> 8) & 0xF, (value >> 4) & 0xF, value & 0xF])


def blocks_from_regions(raws: list[bytes], count: int) -> list[bytes | None]:
    half = count // 2
    blocks: list[bytes | None] = []
    for i, raw in enumerate(raws):
        if len(raw) % BLOCK_SIZE:
            raise ValueError(f"region {i} is not padded to {BLOCK_SIZE} bytes")
        n = len(raw) // BLOCK_SIZE
        if n > half:
            raise ValueError(f"region {i} needs {n} blocks, only {half} fit "
                             f"({n * BLOCK_SIZE - half * BLOCK_SIZE} bytes too many after compression)")
        blocks += [raw[k * BLOCK_SIZE:(k + 1) * BLOCK_SIZE] for k in range(n)]
        blocks += [None] * (half - n)
    return blocks


def _record(msg: bytes, size: int) -> bytes:
    rec = struct.pack("<I", len(msg)) + msg
    if len(rec) > size:
        raise ValueError("record does not fit its slot")
    return rec.ljust(size, b"\0")


def build_container(name: str, model_id: int, version: int,
                    blocks: list[bytes | None]) -> bytes:
    """Encoded Code*.Dat, laid out like Roland's (byte-identical on rebuild)."""
    count = len(blocks)
    start = P.build(P.DT1, P.ADDR_START, b"\x00")
    end = P.build(P.DT1, P.ADDR_END, nibbles4(end_checksum(blocks)))
    hdr = bytearray(START_OFFSET)
    struct.pack_into("<I", hdr, 0, P.HEADER_SIZE)
    hdr[4:20] = CONTAINER_TYPE
    hdr[0x14:0x24] = encode_name(name)
    struct.pack_into("<HH", hdr, 0x24, model_id, version)
    struct.pack_into("<3I", hdr, 0x30, START_OFFSET, SMALL_RECORD, 1)
    struct.pack_into("<3I", hdr, 0x40, END_OFFSET, SMALL_RECORD, 1)
    struct.pack_into("<3I", hdr, 0x50, DATA_OFFSET, DATA_STRIDE, count)
    fill = _record(P.build(P.DT1, P.ADDR_DATA_FILL, FILL_DATA), DATA_STRIDE)
    parts = [bytes(hdr), _record(start, SMALL_RECORD), _record(end, SMALL_RECORD)]
    for b in blocks:
        parts.append(fill if b is None else
                     _record(P.build(P.DT1, P.ADDR_DATA_CHUNK, P.pack_7in8(b)), DATA_STRIDE))
    return P.encode(b"".join(parts))


# ---------------------------------------------------------------------------
# relocation
# ---------------------------------------------------------------------------

def diff_relocs(a: bytes, b: bytes, delta: int) -> list[int]:
    """Offsets of the 32-bit words that move by `delta` between two builds.

    Every difference must be such a word; anything else means the builds are
    not the same code linked at two addresses.
    """
    if len(a) != len(b):
        raise ValueError("images differ in size")
    out, i, n = [], 0, len(a)
    while i < n:
        if a[i] == b[i]:
            i += 1
            continue
        o = i & ~3
        if o + 4 > n:
            raise ValueError(f"difference at {i:#x} past the last word")
        x, y = struct.unpack_from("<II", a[o:o + 4] + b[o:o + 4])
        if (y - x) & 0xFFFFFFFF != delta & 0xFFFFFFFF:
            raise ValueError(f"word at {o:#x} changes by {(y - x) & 0xFFFFFFFF:#x}, "
                             f"not {delta & 0xFFFFFFFF:#x}")
        out.append(o)
        i = o + 4
    return out


def relocate(plain: bytes, offsets: list[int], delta: int) -> bytes:
    b = bytearray(plain)
    for o in offsets:
        v = struct.unpack_from("<I", b, o)[0]
        struct.pack_into("<I", b, o, (v + delta) & 0xFFFFFFFF)
    return bytes(b)


def detect_slot(device: str, region: int, plain: bytes) -> int:
    """Slot a SYSTEM-8 region was linked for, from its SYNINFO init pointer."""
    if device != "SYSTEM-8":
        return 1
    s = SynInfo.parse(plain)
    if s is None:
        raise ValueError(f"region {region} has no SYNINFO; give the slot explicitly")
    slot, rest = divmod(s.init - LINK_BASE[device][region], SLOT_STRIDE)
    if not 0 <= slot < 8 or rest >= len(plain):
        raise ValueError(f"region {region}: init {s.init:#x} is outside every slot window")
    return slot


# ---------------------------------------------------------------------------
# ELF export (for Ghidra / IDA / objdump)
# ---------------------------------------------------------------------------

def to_elf(segments: list[tuple[str, int, bytes]], symbols: list[tuple[str, int, bool]],
           entry: int = 0) -> bytes:
    """Minimal ELF32 ARM (EABI5) with one PT_LOAD + section per segment.

    segments: (section name, address, bytes); symbols: (name, address, is_func).
    """
    shstr = bytearray(b"\0")
    strtab = bytearray(b"\0")

    def sname(s: str) -> int:
        shstr.extend(s.encode() + b"\0")
        return len(shstr) - len(s) - 1

    ehsize, phsize, shsize = 52, 32, 40
    nseg = len(segments)
    off = ehsize + phsize * nseg
    data_offs = []
    for _, _, data in segments:
        off += -off % 16
        data_offs.append(off)
        off += len(data)

    sym = bytearray(16)             # null symbol
    for name, addr, func in symbols:
        strtab.extend(name.encode() + b"\0")
        shndx = next((k + 1 for k, (_, a, d) in enumerate(segments)
                      if a <= (addr & ~1) < a + len(d)), 0xFFF1)
        info = 0x12 if func else 0x11              # GLOBAL FUNC / GLOBAL OBJECT
        sym += struct.pack("<IIIBBH", len(strtab) - len(name) - 1, addr, 0, info, 0, shndx)

    sec_names = [sname(n) for n, _, _ in segments]
    n_symtab, n_strtab, n_shstr = sname(".symtab"), sname(".strtab"), sname(".shstrtab")
    sym_off = off + (-off % 4)
    str_off = sym_off + len(sym)
    shstr_off = str_off + len(strtab)
    sh_off = shstr_off + len(shstr)
    sh_off += -sh_off % 4
    nsec = nseg + 4

    out = bytearray(struct.pack("<4sBBBB8sHHIIIIIHHHHHH", b"\x7fELF", 1, 1, 1, 0, b"",
                                2, 40, 1, entry, ehsize, sh_off, 0x05000000, ehsize,
                                phsize, nseg, shsize, nsec, nsec - 1))
    for (_, addr, data), o in zip(segments, data_offs):
        out += struct.pack("<8I", 1, o, addr, addr, len(data), len(data), 7, 4)
    for (_, _, data), o in zip(segments, data_offs):
        out += b"\0" * (o - len(out)) + data
    out += b"\0" * (sym_off - len(out)) + sym + strtab + shstr
    out += b"\0" * (sh_off - len(out))
    out += b"\0" * shsize
    for (_, addr, data), o, n in zip(segments, data_offs, sec_names):
        out += struct.pack("<10I", n, 1, 7, addr, o, len(data), 0, 0, 16, 0)
    out += struct.pack("<10I", n_symtab, 2, 0, 0, sym_off, len(sym), nseg + 2, 1, 4, 16)
    out += struct.pack("<10I", n_strtab, 3, 0, 0, str_off, len(strtab), 0, 0, 1, 0)
    out += struct.pack("<10I", n_shstr, 3, 0, 0, shstr_off, len(shstr), 0, 0, 1, 0)
    return bytes(out)
