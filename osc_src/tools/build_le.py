#!/usr/bin/env python3
"""build_le.py -- place generated objects at fixed addresses and emit the image.

This is the "linker" of the reconstruction.  Every object the generators
produce (``data/*.o`` and ``code/build/*.o``) carries its sections at absolute
addresses through ``. = <addr>;`` in its assembly source, so placing an object
is just copying each ``SHF_ALLOC`` section's bytes to its virtual address and
applying the ELF relocations.  Every relocation of the reconstructed objects is
resolved internally -- the code layer resolves its own cross references through
its symbol table (``gen_code.py``) -- so an unresolved or out-of-range
relocation is a hard error.

    python3 tools/build_le.py -o build/os_full.bin data/data.o code/build/code.o
    python3 tools/build_le.py --selftest     # check against GNU as/ld

Relocations implemented (all that the reconstruction emits):

    R_ARM_NONE (0)         nothing
    R_ARM_ABS32 (2)        S + A                     (word)
    R_ARM_ABS16 (5)        S + A                     (halfword)
    R_ARM_THM_CALL (10)    BL    -> (S + A) - (P + 4)
    R_ARM_THM_JUMP24 (30)  B.W   -> (S + A) - (P + 4)

The Thumb branch encoding is checked bit for bit against GNU ld by
``selftest_link.py``; the details that matter are documented at the encoder.
"""
from __future__ import annotations

import argparse
import struct
import sys

EM_ARM = 40
SHT_SYMTAB = 2
SHT_REL = 9
SHT_RELA = 4
SHT_NOBITS = 8
SHF_ALLOC = 0x2
STT_SECTION = 3

R_NONE, R_ABS32, R_ABS16, R_THM_CALL, R_THM_JUMP24 = 0, 2, 5, 10, 30
R_THM_JUMP19 = 51


def _u16(b, o):
    return struct.unpack_from("<H", b, o)[0]


def _u32(b, o):
    return struct.unpack_from("<I", b, o)[0]


def write_thm_imm(hi, lo, value):
    """Write the signed 24-bit, even offset ``value`` into a BL / B.W pair.

    Layout (ARM ARM: encoding T1 of B.W / T1 of BL), checked against GNU ld's
    output in ``selftest_link.py``::

        hi: 11110 S | imm10          imm10 = displacement[21:12]
        lo: 11 J1 1 J2 | imm11       imm11 = displacement[11:1]
        I1 = NOT(J1 XOR S),  I2 = NOT(J2 XOR S)

    A displacement inside +-2 MiB has zero in S:I1:I2 (bit 23..21), so the
    canonical encoding is J1 = J2 = 1 and only imm10:imm11 carry the offset --
    exactly what GNU as writes.  The general form is kept for completeness.
    """
    if value >= (1 << 31):                # accept the unsigned form too
        value -= 1 << 32
    if value & 1:
        raise ValueError("odd Thumb branch offset %#x" % value)
    if not -(1 << 24) <= value < (1 << 24):
        raise ValueError("Thumb branch out of range: %#x" % value)
    v = value & 0xFFFFFF                  # 24-bit two's complement
    s = (v >> 23) & 1
    i1 = (v >> 22) & 1
    i2 = (v >> 21) & 1
    imm10 = (v >> 12) & 0x3FF
    imm11 = (v >> 1) & 0x7FF
    j1 = (1 ^ i1) ^ s                     # J1 = NOT(I1) XOR S
    j2 = (1 ^ i2) ^ s                     # J2 = NOT(I2) XOR S
    hi = 0xF000 | (s << 10) | imm10
    lo = 0xC000 | (j1 << 13) | (j2 << 11) | imm11
    # A displacement inside +-2 MiB has S = I1 = I2 = 0, so J1 = J2 = 1 and
    # the three fixed bits of ``lo`` (15..14 and 12) read back as 111 -- the
    # canonical form GNU as emits.
    if not (v >> 21):
        lo = 0xF800 | imm11
    return hi, lo


def patch_branch(buf, off, disp, hi_j="canonical", lo_j="canonical"):
    """Encode a branch displacement into ``buf`` at ``off`` in place.

    The object's own bytes decide the opcode bits and, optionally, J1/J2: GNU
    as writes J1 = J2 = 1 for a normal branch but leaves them at the
    placeholder value (J1 = 0, J2 = 0, i.e. ``f7ff fffe``) when the target is
    not yet known, and GNU ld keeps whatever J1/J2 byte 1 of the instruction
    holds.  Reproducing that keeps the rebuild byte-identical.
    """
    hi, lo = _u16(buf, off), _u16(buf, off + 2)
    hi2, lo2 = write_thm_imm(hi, lo, disp)
    if hi_j != "canonical":
        hi2 = (hi2 & ~0x400) | ((int(hi_j) & 1) << 10)
    if lo_j != "canonical":
        lo2 = (lo2 & ~0x2800) | ((int(lo_j) & 1) << 13) | ((int(lo_j) & 1) << 11)
    struct.pack_into("<HH", buf, off, hi2, lo2)


def write_thm19(hi, lo, value):
    """Write a signed 20-bit, even offset into a conditional B pair.

    Layout (ARM ARM: encoding T3 of B<c>)::

        hi: 1110 S cond imm6      imm6 = displacement[19:14]
        lo: 10 J1 0 J2 imm11      imm11 = displacement[12:1]
    """
    if value >= (1 << 31):               # accept the unsigned form too
        value -= 1 << 32
    if value & 1:
        raise ValueError("odd Thumb branch offset %#x" % value)
    if not -(1 << 19) <= value < (1 << 19):
        raise ValueError("Thumb conditional branch out of range: %#x" % value)
    v = value & 0xFFFFF                  # 20-bit two's complement field
    s = (v >> 19) & 1
    j1 = (v >> 18) & 1
    j2 = (v >> 17) & 1
    imm6 = (v >> 12) & 0x3F
    imm11 = (v >> 1) & 0x7FF
    hi = (hi & 0xFBC0) | (s << 10) | imm6
    lo = (lo & 0xD000) | 0x8000 | (j1 << 13) | (j2 << 11) | imm11
    return hi, lo


def read_thm_imm(hi, lo):
    """Signed 24-bit offset of a BL / B.W halfword pair (inverse of above)."""
    s = (hi >> 10) & 1
    i1 = 1 ^ (((lo >> 13) & 1) ^ s)
    i2 = 1 ^ (((lo >> 11) & 1) ^ s)
    v = (s << 23) | (i1 << 22) | (i2 << 21) | ((hi & 0x3FF) << 12) | ((lo & 0x7FF) << 1)
    if s:
        v -= 1 << 24
    return v


class Elf:
    """Minimal little-endian ELF32 reader for the objects we generate."""

    def __init__(self, path):
        self.path = path
        with open(path, "rb") as f:
            self.buf = f.read()
        b = self.buf
        if b[:4] != b"\x7fELF" or b[4] != 1 or b[5] != 1:
            raise ValueError("%s: not a 32-bit little-endian ELF" % path)
        if _u16(b, 18) != EM_ARM:
            raise ValueError("%s: not ARM" % path)
        shoff = _u32(b, 32)
        shentsize, shnum, shstrndx = _u16(b, 46), _u16(b, 48), _u16(b, 50)
        self.sections = []
        for i in range(shnum):
            o = shoff + i * shentsize
            (name, typ, flags, addr, off, size, link, info, align,
             entsize) = struct.unpack_from("<IIIIIIIIII", b, o)
            self.sections.append(dict(i=i, name=name, type=typ, flags=flags, addr=addr,
                                      off=off, size=size, link=link, info=info,
                                      align=align, entsize=entsize))
        shstr = self.sections[shstrndx]
        for s in self.sections:
            s["sname"] = self._str(shstr, s["name"])
        self.symbols = []
        for s in self.sections:
            if s["type"] == SHT_SYMTAB:
                self._read_symbols(s)

    def _str(self, sec, off):
        start = sec["off"] + off
        end = self.buf.index(b"\0", start)
        return self.buf[start:end].decode("latin-1")

    def _read_symbols(self, symtab):
        strtab = self.sections[symtab["link"]]
        for i in range(symtab["size"] // 16):
            o = symtab["off"] + i * 16
            name, value, size, info, other, shndx = struct.unpack_from("<IIIBBH", self.buf, o)
            self.symbols.append(dict(name=self._str(strtab, name), value=value, size=size,
                                     info=info, shndx=shndx, i=i))

    def alloc_sections(self):
        # empty sections (e.g. .data of a read-only object) carry no bytes and
        # may sit at a placeholder address; skip them
        out = [s for s in self.sections
               if s["flags"] & SHF_ALLOC and s["type"] != SHT_NOBITS and s["size"]]
        for s in out:
            # The address of a generated section travels in its name, e.g.
            # ``.data.600002f0``: as always lays the sections of an object end
            # to end, so an absolute address can only be applied here.
            if s["addr"] == 0:
                tail = s["sname"].rsplit(".", 1)[-1]
                try:
                    v = int(tail, 16)
                except ValueError:
                    v = -1
                if len(tail) == 8 and v >= 0:
                    s["addr"] = v
        return out

    def resolve(self, mapping):
        """Give undefined symbols the value the symbol table assigns them.

        A Thumb code symbol keeps bit 0 set in its value (the ABI's marker);
        the relocation code strips it again for a branch.  A data symbol (a
        literal pool word, a string) is given its plain address.
        """
        for sym in self.symbols:
            if sym["shndx"] == 0 and sym["name"] in mapping:
                sym["shndx"] = 1                 # "some section": value is final
                sym["value"] = mapping[sym["name"]]


def global_symbols(elf, base):
    """Defined symbols of one object, as a {name: value} map.

    Every generated unit is one function (or one data run) at a fixed address,
    so the union of the objects' symbols is the link's symbol table and a
    cross reference resolves exactly as it did in the firmware.  A global
    ``fn_<addr>`` or ``d_<addr>`` symbol names its own address, which is where
    the unit was placed -- not the section-relative value as stores it.
    """
    out = {}
    for sym in elf.symbols:
        if sym["shndx"] in (0xFFF1,) or not sym["name"]:
            continue
        if (sym["info"] & 0xF) == STT_SECTION:
            continue
        name = sym["name"]
        value = sym["value"]
        if sym["shndx"] == 0:
            # an undefined symbol here is a *declared* call target (see
            # gen_asm.render): its name carries the address
            for prefix in ("fn_", "d_"):
                if name.startswith(prefix) and len(name) == len(prefix) + 8:
                    try:
                        value = int(name[len(prefix):], 16) | 1
                    except ValueError:
                        value = None
                    break
            else:
                value = None
            if value is None:
                continue
        for prefix in ("fn_", "d_"):
            if name.startswith(prefix) and len(name) == len(prefix) + 8:
                try:
                    value = int(name[len(prefix):], 16) | (value & 1 or 1)
                except ValueError:
                    pass
        out[name] = value
    return out


def place(elf, base, size, table=None):
    """Return [(image_offset, bytes)] for one object, relocations applied."""
    out = []
    allsec = {s["i"]: s for s in elf.sections}
    for sec in elf.alloc_sections():
        start = sec["addr"] - base
        if start < 0 or start + sec["size"] > size:
            raise ValueError("%s: section %s at %#x outside the image"
                             % (elf.path, sec["sname"], sec["addr"]))
        data = bytearray(elf.buf[sec["off"]:sec["off"] + sec["size"]])
        for rsec in elf.sections:
            if rsec["type"] not in (SHT_REL, SHT_RELA) or rsec["info"] != sec["i"]:
                continue
            rela = rsec["type"] == SHT_RELA
            step = 12 if rela else 8
            for k in range(rsec["size"] // step):
                o = rsec["off"] + k * step
                if rela:
                    r_off, r_info, r_add = struct.unpack_from("<IIi", elf.buf, o)
                else:
                    r_off, r_info = struct.unpack_from("<II", elf.buf, o)
                    r_add = 0
                symi, rtype = r_info >> 8, r_info & 0xFF
                sym = dict(elf.symbols[symi])
                if sym["shndx"] == 0:
                    if table is not None and sym["name"] in table:
                        sym["shndx"] = 1
                        sym["value"] = table[sym["name"]]
                    else:
                        raise ValueError("%s: symbol %r is undefined"
                                         % (elf.path, sym["name"]))
                if (sym["info"] & 0xF) == STT_SECTION:
                    sval = allsec[sym["shndx"]]["addr"] + r_add
                else:
                    sval = sym["value"] + r_add
                p = sec["addr"] + r_off          # place
                if rtype == R_NONE:
                    continue
                if rtype == R_ABS32:
                    struct.pack_into("<I", data, r_off, sval & 0xFFFFFFFF)
                elif rtype == R_ABS16:
                    struct.pack_into("<H", data, r_off, sval & 0xFFFF)
                elif rtype in (R_THM_CALL, R_THM_JUMP24):
                    # BL / B.W: the 24-bit even displacement (S+A)-((P+4)&~1)
                    # is stored as S imm10 / 1 J1 1 J2 imm11 with
                    # I1 = NOT(J1 XOR S), I2 = NOT(J2 XOR S).
                    #
                    # GNU ld keeps J1/J2 as the assembler left them (as' REL
                    # placeholder for an unknown target is ``f7ff fffe``, whose
                    # J1/J2 are 1/0 while a canonical encoding of -4 is
                    # ``f7ff fffc``) and patches only S, imm10 and imm11.
                    disp = ((sval & ~1) - ((p + 4) & ~1)) & 0xFFFFFFFF
                    patch_branch(data, r_off, disp)
                elif rtype == R_THM_JUMP19:
                    # conditional B: 1110 S cond imm6 / 10 J1 0 J2 imm11
                    X = sval & ~1
                    disp = (X - ((p + 4) & ~1)) & 0xFFFFFFFF
                    hi, lo = _u16(data, r_off), _u16(data, r_off + 2)
                    hi2, lo2 = write_thm19(hi, lo, disp)
                    # the object's bytes keep the opcode's constant bits
                    lo2 = (lo2 & 0x2BFF) | (lo & 0x2BFF)
                    struct.pack_into("<HH", data, r_off, hi2, lo2)
                else:
                    raise ValueError("%s: unsupported relocation %d (%s) in %s"
                                     % (elf.path, rtype, sym["name"], sec["sname"]))
        out.append((start, bytes(data)))
    return out


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("objects", nargs="*")
    ap.add_argument("-o", "--out")
    ap.add_argument("--base", type=lambda s: int(s, 0), default=0x60000000)
    ap.add_argument("--size", type=lambda s: int(s, 0), default=0x2CDA70)
    ap.add_argument("--fill", type=lambda s: int(s, 0), default=0xFF)
    ap.add_argument("--allow-holes", action="store_true")
    ap.add_argument("--selftest", action="store_true", help="run the GNU ld check")
    args = ap.parse_args(argv)
    if args.selftest:
        import selftest_link
        return selftest_link.main()
    if not args.objects or not args.out:
        ap.error("objects and -o are required")

    image = bytearray([args.fill]) * args.size
    covered = bytearray(args.size)
    objs = [Elf(p) for p in args.objects]
    table = {}
    for elf in objs:
        table.update(global_symbols(elf, args.base))
    for path, elf in zip(args.objects, objs):
        for off, data in place(elf, args.base, args.size, table):
            for i, b in enumerate(data):
                if covered[off + i]:
                    # an overlapping unit (a call target inside another
                    # function) must agree with what is already placed
                    if image[off + i] != b:
                        raise SystemExit("%s: conflicting byte at %#x (%02x vs %02x)"
                                         % (path, args.base + off + i, image[off + i], b))
                    continue
                image[off + i] = b
                covered[off + i] = 1
    holes = args.size - sum(covered)
    if holes:
        first = next(i for i in range(args.size) if not covered[i])
        print("build_le: %d byte(s) not covered by any object (first at %#x)"
              % (holes, args.base + first), file=sys.stderr)
    with open(args.out, "wb") as f:
        f.write(image)
    print("build_le: %s: %d bytes, %d covered" % (args.out, args.size, args.size - holes))
    return 0 if (not holes or args.allow_holes) else 1


if __name__ == "__main__":
    sys.exit(main())
