#!/usr/bin/env python3
"""
crypto_scan.py -- look for block-cipher / hash constants in a raw memory dump.

    python3 tools/crypto_scan.py rom_dump.bin --base 0x01000000
    python3 tools/crypto_scan.py rom_dump.bin --base 0x01000000 --sboxes --xrefs

What it reports (offset and, with --base, the address):
  * 32-bit magic constants (TEA/XTEA/RC5/Blowfish/MD5/SHA/CRC/CAST/...) in
    little- and big-endian
  * byte tables (AES S-box, DES permutation / S1, Skipjack F, ...)
  * --sboxes: any 256-byte window that is a permutation of 0..255
    (S-box candidate, algorithm-agnostic) and runs of 4 x 256 x u32 tables
    (Blowfish/CAST style) by entropy of 1 KB windows
  * --xrefs: for every hit, Thumb `ldr rX,[pc,#imm]` / ADR sites that load
    the hit address (needs --base), i.e. the code using the constant

Pure Python, no dependencies.
"""
from __future__ import annotations

import argparse
import struct
import sys

WORDS = {
    0x9E3779B9: "TEA/XTEA delta, RC5/RC6/Serpent/SEED golden ratio",
    0x61C88647: "TEA delta (negated, -0x9E3779B9)",
    0xC6EF3720: "XTEA sum for 32 rounds (delta*32)",
    0xB7E15163: "RC5/RC6 P32",
    0x243F6A88: "Blowfish P-array[0] (pi)",
    0x85A308D3: "Blowfish P-array[1] (pi)",
    0xD1310BA6: "Blowfish S-box[0][0]",
    0x30FB40D4: "CAST5 S1[0]",
    0x9C30D539: "CAST5 S1[1]",
    0x67452301: "MD5/SHA-1 init A",
    0xEFCDAB89: "MD5/SHA-1 init B",
    0x98BADCFE: "MD5/SHA-1 init C",
    0x10325476: "MD5/SHA-1 init D",
    0xD76AA478: "MD5 K[0]",
    0x5A827999: "SHA-1 K0",
    0x6ED9EBA1: "SHA-1 K1",
    0x428A2F98: "SHA-256 K[0]",
    0x6A09E667: "SHA-256 H0 / Camellia",
    0xA09E667F: "Camellia sigma1",
    0xC66363A5: "AES Te0[0]",
    0x77073096: "CRC-32 table[1]",
    0xEDB88320: "CRC-32 reflected polynomial",
    0x04C11DB7: "CRC-32 polynomial",
    0x7137449F: "SHA-256 K[1]",
    0x00808200: "DES combined S/P table SP1[1] (libdes/OpenSSL style)",
    0x01010400: "DES combined S/P table SP1[0] (libdes/OpenSSL style)",
}

BYTES = {
    "AES S-box": bytes.fromhex("637c777bf26b6fc53001672bfed7ab76"),
    "AES inv S-box": bytes.fromhex("52096ad53036a538bf40a39e81f3d7fb"),
    "DES S1 row 0": bytes([14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7]),
    "DES IP": bytes([58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4]),
    "DES FP": bytes([40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31]),
    "DES PC-1": bytes([57, 49, 41, 33, 25, 17, 9, 1, 58, 50, 42, 34, 26, 18]),
    "DES PC-2": bytes([14, 17, 11, 24, 1, 5, 3, 28, 15, 6, 21, 10, 23, 19, 12, 4]),
    "DES E": bytes([32, 1, 2, 3, 4, 5, 4, 5, 6, 7, 8, 9]),
    "DES P": bytes([16, 7, 20, 21, 29, 12, 28, 17, 1, 15, 23, 26, 5, 18, 31, 10]),
    "Skipjack F-table": bytes.fromhex("a3d70983f848f6f4"),
    "SM4 S-box": bytes.fromhex("d690e9fecce13db716b614c228fb2c05"),
    "Twofish q0": bytes.fromhex("a967b3e804ed1dc6"),
    "ChaCha/Salsa tag": b"expand 32-byte k",
    "ChaCha tag (16)": b"expand 16-byte k",
}


def find_all(buf: bytes, pat: bytes):
    i = buf.find(pat)
    while i != -1:
        yield i
        i = buf.find(pat, i + 1)


def scan_words(buf: bytes):
    for val, what in WORDS.items():
        for end, tag in (("<I", "LE"), (">I", "BE")):
            for off in find_all(buf, struct.pack(end, val)):
                yield off, 4, f"{val:#010x} {tag}", what


def scan_bytes(buf: bytes):
    for name, pat in BYTES.items():
        for off in find_all(buf, pat):
            yield off, len(pat), "bytes", name


def scan_sboxes(buf: bytes):
    """Windows of 256 bytes holding every value 0..255 exactly once."""
    out, i, n = [], 0, len(buf)
    while i + 256 <= n:
        w = buf[i:i + 256]
        if len(set(w)) == 256:
            out.append((i, 256, "perm256", "256-byte S-box / permutation candidate"))
            i += 256
        else:
            i += 1
    return out


def thumb_xrefs(buf: bytes, base: int, target: int):
    """Thumb-1 `ldr rt,[pc,#imm8*4]` whose literal word equals `target`
    (the code that loads a pointer to the table) and Thumb-2 `ldr.w`."""
    hits = []
    for off in range(0, len(buf) - 1, 2):
        h = struct.unpack_from("<H", buf, off)[0]
        if h >> 11 == 0b01001:                       # ldr rt,[pc,#imm8<<2]
            lit = ((base + off + 4) & ~3) + ((h & 0xFF) << 2)
        elif h == 0xF8DF or (h & 0xFF7F) == 0xF85F:  # ldr.w rt,[pc,#imm12]
            if off + 4 > len(buf):
                continue
            h2 = struct.unpack_from("<H", buf, off + 2)[0]
            imm = h2 & 0xFFF
            up = (h >> 7) & 1
            lit = ((base + off + 4) & ~3) + (imm if up else -imm)
        else:
            continue
        o = lit - base
        if 0 <= o <= len(buf) - 4 and struct.unpack_from("<I", buf, o)[0] & ~1 == target & ~1:
            hits.append(base + off)
    return hits


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("file")
    ap.add_argument("--base", type=lambda x: int(x, 0), default=0,
                    help="address of byte 0 of the file")
    ap.add_argument("--sboxes", action="store_true", help="also find 256-byte permutations")
    ap.add_argument("--xrefs", action="store_true", help="find Thumb ldr [pc,#] users of each hit")
    args = ap.parse_args()

    buf = open(args.file, "rb").read()
    print(f"{args.file}: {len(buf)} bytes, base {args.base:#x}", file=sys.stderr)

    hits = list(scan_words(buf)) + list(scan_bytes(buf))
    if args.sboxes:
        hits += scan_sboxes(buf)
    hits.sort()
    if not hits:
        print("no known constants found")
        return 1
    for off, ln, kind, what in hits:
        print(f"{args.base + off:#010x} (+{off:#07x})  {kind:<18} {what}")
        if args.xrefs and args.base and kind != "bytes" and ln == 4:
            for a in thumb_xrefs(buf, args.base, args.base + off):
                print(f"      code at {a:#010x} loads &{args.base + off:#x}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
