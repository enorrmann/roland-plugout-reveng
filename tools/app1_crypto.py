#!/usr/bin/env python3
"""
app1_crypto.py -- decrypt / encrypt the SYSTEM-8 `App1_Main` OS payload.

    python3 tools/app1_crypto.py info     firmware/sys8_sys_v132/untared/p1y01_m0c0a_up.bin
    python3 tools/app1_crypto.py decrypt  <container> -o out/os.bin
    python3 tools/app1_crypto.py encrypt  out/os.bin -o out/p1y01_m0c0a_up.bin \\
                                          --header-from <original container>
    python3 tools/app1_crypto.py selftest

Only the payload is encrypted: file bytes [0x60 : 0x60 + size@0x44]. The 0x60-byte
clear header and the 0x40-byte clear trailer are never touched, which is what
makes an encrypt -> decrypt round trip exact.

Mechanism (ported from the MC-101 / ESC2 research, a separate repository next to this one:
../mc-101-re/tools/decrypt_c0a.py + ../mc-101-re/docs/core0_boot_and_memory_architecture.md;
in this repository it is specified in docs/vol4_firmware_container_protocol/container_formats.md §1.4.1a):
  * 3-key 3DES, ECB, 64-bit blocks.
  * hardware AXI FIFO word order: each 64-bit block is two 32-bit words that are
    big-endian packed and word-swapped -- [W1:W0] on the way in and on the way out.
  * the key comes from the silicon Boot ROM master key table at 0x012023B0:
        table address = 0x012023B0 + key_idx * 24
    and, in decrypt mode, the hardware key registers load the six 32-bit words in
    inverted stage order:
        Key = pack(">IIIIII", w1, w0, w3, w2, w5, w4)

Key slots found so far (the 24-byte 3DES key is hardware-ordered -- table words
packed as w1,w0,w3,w2,w5,w4):
    0xB6   MC-101 App1_Main / SYSTEM-8 App1_Main
    0xB5   MC-101 App0_UPD
  Slot 0xB6 is the one the SYSTEM-8 container actually names (see `info`), and the
  research above confirms the key table is shared across Roland ESC2 partitions.

The key material is private: it is never stored in this file. Supply it with
--key (hex on the command line) or --key-file (path, or - for stdin), or point
--bootrom at a silicon dump so the table entry is read from there.

Pure stdlib except pycryptodome/pycryptodomex for the DES3 primitive.
"""

from __future__ import annotations

import argparse
import hashlib
import os
import struct
import sys

try:
    from Cryptodome.Cipher import DES3
except ImportError:                                       # pragma: no cover
    try:
        from Crypto.Cipher import DES3
    except ImportError:
        DES3 = None

OS_MAGIC = b"App1_Main"
HEADER_SIZE = 0x60
TRAILER_SIZE = 0x40
BOOTROM_KEY_TABLE = 0x012023B0
KEY_ENTRY_SIZE = 24


# ---------------------------------------------------------------------------
# container header
# ---------------------------------------------------------------------------

def parse_header(data: bytes) -> dict:
    """The 0x60-byte clear header. Word at 0x54 carries the crypto fields."""
    if not data.startswith(OS_MAGIC):
        raise ValueError("not an App1_Main container")
    if len(data) < HEADER_SIZE:
        raise ValueError("file shorter than the 0x60-byte header")

    w_src, w_len, w_dst, w_rsvd, w_chk, w_ctrl, w_pad1, w_pad2 = struct.unpack(
        "<IIIIIIII", data[0x40:0x60])

    # Word 0x54 is 0xKKMM0001 as a big-endian word == bytes 01 00 MM KK LE:
    #   0x54 + 0 = 01  encryption enabled
    #   0x54 + 2 = MM  crypto mode (0 = single DES, 1 = 2-key 3DES, 2 = 3-key 3DES)
    #   0x54 + 3 = KK  boot ROM key slot
    # Verified across SYSTEM-8 v1.30/1.31/1.32 (KK=B6) and both MC-101 partitions
    # (KK=B6 for App1_Main, KK=B5 for App0_UPD), with MM=2 throughout.
    key_idx = data[0x54 + 3]

    return {
        "name": data[:16].split(b"\0", 1)[0].decode("ascii", "replace"),
        "date": data[0x10:0x20].split(b"\0", 1)[0].decode("ascii", "replace"),
        "tool": data[0x20:0x28].split(b"\0", 1)[0].decode("ascii", "replace"),
        "load": struct.unpack_from("<I", data, 0x28)[0],
        "hdr_40": w_src,
        "payload_size": w_len,
        "hdr_48": w_dst,
        "check_50": w_chk,
        "crypto_ctrl": w_ctrl,
        "encrypted": (w_ctrl & 0xFF) == 1,
        "crypto_mode": data[0x54 + 2],           # 0 = single DES, 1 = 2-key, 2 = 3-key 3DES
        "key_index": key_idx,
        "trailer": data[HEADER_SIZE + w_len:],
        "file_size": len(data),
    }


def payload_slice(data: bytes) -> bytes:
    info = parse_header(data)
    end = HEADER_SIZE + info["payload_size"]
    if end > len(data):
        raise ValueError(f"payload 0x{info['payload_size']:x} runs past EOF")
    return data[HEADER_SIZE:end]


# ---------------------------------------------------------------------------
# key derivation
# ---------------------------------------------------------------------------

def key_from_bootrom(rom: bytes, key_idx: int) -> bytes:
    """Pull a 24-byte entry out of the silicon master key table."""
    off = BOOTROM_KEY_TABLE - 0x01200000 + key_idx * KEY_ENTRY_SIZE
    if off + KEY_ENTRY_SIZE > len(rom):
        raise ValueError(f"key index 0x{key_idx:02X} outside the dump (rom 0x{len(rom):x} B)")
    w = struct.unpack("<6I", rom[off:off + KEY_ENTRY_SIZE])
    # decrypt-mode key register order: K1=[w4,w5] K2=[w2,w3] K3=[w0,w1]
    return struct.pack(">6I", w[1], w[0], w[3], w[2], w[5], w[4])


def parse_key_hex(text: str) -> bytes:
    """A 24-byte 3-key 3DES key from hex, ignoring spaces, colons, dashes."""
    k = bytes.fromhex(text.replace(" ", "").replace(":", "").replace("-", "").strip())
    if len(k) != 24:
        raise ValueError(f"a 3-key 3DES key is 24 bytes, got {len(k)}")
    return k


def read_key_file(path: str) -> bytes:
    if path == "-":
        return parse_key_hex(sys.stdin.read())
    with open(path) as fh:
        return parse_key_hex(fh.read())


def resolve_key(key_hex: str | None, key_idx: int, bootrom: str | None,
                key_file: str | None = None) -> tuple[bytes, str]:
    """Return (key, provenance). The key is never taken from a built-in table."""
    if key_hex and key_file:
        raise SystemExit("pass only one of --key / --key-file")
    if key_hex:
        return parse_key_hex(key_hex), "command line"
    if key_file:
        return read_key_file(key_file), f"file {key_file}"
    if bootrom:
        if not os.path.exists(bootrom):
            raise SystemExit(f"--bootrom not found: {bootrom}")
        with open(bootrom, "rb") as fh:
            return (key_from_bootrom(fh.read(), key_idx),
                    f"bootrom {bootrom} slot 0x{key_idx:02X} @ 0x{bootrom_addr(key_idx):08X}")
    raise SystemExit(
        f"the 3DES key for slot 0x{key_idx:02X} is not stored in this tool.\n"
        f"  supply it with --key <48 hex digits>, --key-file <path|->, "
        f"or --bootrom <silicon dump>")


def bootrom_addr(key_idx: int) -> int:
    return BOOTROM_KEY_TABLE + key_idx * KEY_ENTRY_SIZE


# ---------------------------------------------------------------------------
# the cipher
# ---------------------------------------------------------------------------

def _require() -> None:
    if DES3 is None:
        raise SystemExit("need pycryptodome: pip install pycryptodome")


def transform(data: bytes, key: bytes, decrypt: bool) -> bytes:
    """3DES-ECB over the hardware's word-swapped block representation."""
    _require()
    n = len(data) // 8
    if len(data) % 8:
        raise ValueError("payload must be a multiple of 8 bytes for 3DES")
    cipher = DES3.new(key, DES3.MODE_ECB)
    flip = cipher.decrypt if decrypt else cipher.encrypt

    w = struct.unpack(f"<{n * 2}I", data[:n * 8])
    # host little-endian words -> AXI FIFO big-endian, word pairs swapped
    swapped = [x for i in range(0, len(w), 2) for x in (w[i + 1], w[i])]
    out = flip(struct.pack(f">{n * 2}I", *swapped))
    ow = struct.unpack(f">{n * 2}I", out)
    back = [x for i in range(0, len(ow), 2) for x in (ow[i + 1], ow[i])]
    return struct.pack(f"<{n * 2}I", *back)


def decrypt_payload(container: bytes, key: bytes) -> bytes:
    return transform(payload_slice(container), key, decrypt=True)


def encrypt_payload(plain: bytes, key: bytes) -> bytes:
    return transform(plain, key, decrypt=False)


def rebuild_container(header_blob: bytes, plain: bytes, key: bytes) -> bytes:
    """Return a full App1_Main file with `plain` re-encrypted inside it."""
    info = parse_header(header_blob)
    if len(plain) % 8:
        raise ValueError(f"plaintext is {len(plain)} B, not a multiple of 8")
    cipher = encrypt_payload(plain, key)

    h = bytearray(header_blob[:HEADER_SIZE])
    struct.pack_into("<I", h, 0x44, len(cipher))
    return bytes(h) + cipher + header_blob[HEADER_SIZE + info["payload_size"]:]


# ---------------------------------------------------------------------------
# validation
# ---------------------------------------------------------------------------

def looks_like_arm(plain: bytes) -> tuple[bool, str]:
    """Cheap sanity check for a Cortex-M image: the first word is a valid `ldr`/`b`."""
    if len(plain) < 8:
        return False, "too short"
    w0 = struct.unpack_from("<I", plain, 0)[0]
    half = w0 & 0xFFFF
    kinds = {0x48: "ldr rX,[pc,#imm]", 0x4F: "ldr rX,[pc,#imm]", 0xE0: "b"}
    kind = kinds.get(half >> 8)
    zero = plain.count(0) / len(plain)
    ok = kind is not None or (half >> 11) in (0x1D, 0x1E, 0x1F)
    return ok, (f"entry 0x{w0:08X} ({kind or 'thumb-2 branch'}), "
                f"{zero * 100:.1f}% zero bytes")


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def cmd_info(args) -> int:
    data = open(args.container, "rb").read()
    i = parse_header(data)
    print(f"file            {args.container}  ({i['file_size']:,} B)")
    print(f"name            {i['name']}")
    print(f"build date      {i['date']}")
    print(f"tool            {i['tool']}")
    print(f"load address    0x{i['load']:08X}")
    print(f"payload size    0x{i['payload_size']:X}  ({i['payload_size']:,} B)  "
          f"8-byte aligned: {i['payload_size'] % 8 == 0}")
    print(f"check 0x50      0x{i['check_50']:08X}")
    print(f"crypto ctrl     0x{i['crypto_ctrl']:08X}  "
          f"enabled={i['encrypted']} mode={i['crypto_mode']} "
          f"key slot=0x{i['key_index']:02X}")
    print(f"encrypted span  [0x{HEADER_SIZE:X} : 0x{HEADER_SIZE + i['payload_size']:X})")
    print(f"clear trailer   [0x{HEADER_SIZE + i['payload_size']:X} : 0x{i['file_size']:X})  "
          f"{len(i['trailer'])} B")
    print(f"key slot        0x{i['key_index']:02X}  (key material not stored; pass --key/--key-file)")
    return 0


def _key(args) -> bytes:
    k, src = resolve_key(args.key, args.key_index, args.bootrom, args.key_file)
    # never echo the key itself
    print(f"[*] key slot 0x{args.key_index:02X} from {src} "
          f"(sha256 of key {hashlib.sha256(k).hexdigest()[:16]}…)")
    return k


def cmd_decrypt(args) -> int:
    data = open(args.container, "rb").read()
    i = parse_header(data)
    key = _key(args)
    if not i["encrypted"]:
        print("[*] container is not encrypted; extracting payload as-is")
        plain = payload_slice(data)
    else:
        plain = decrypt_payload(data, key)
    ok, why = looks_like_arm(plain)
    print(f"[{'✓' if ok else '!'}] {why}")
    if args.output:
        os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
        open(args.output, "wb").write(plain)
        print(f"[✓] wrote {args.output} ({len(plain):,} B)")
        print(f"    sha256 {hashlib.sha256(plain).hexdigest()}")
    return 0 if ok else 2


def cmd_encrypt(args) -> int:
    plain = open(args.plain, "rb").read()
    hdr_src = args.header_from
    if not hdr_src:
        raise SystemExit("--header-from is required: the clear header/trailer carry the "
                         "load address and build tag and must be preserved")
    blob = open(hdr_src, "rb").read()
    key = _key(args)
    out = rebuild_container(blob, plain, key)
    print(f"[*] plaintext {len(plain):,} B -> container {len(out):,} B")

    # immediate proof: decrypt what we just built
    back = decrypt_payload(out, key)
    same = back == plain
    print(f"[{'✓' if same else '!'}] re-decrypt round trip {'exact' if same else 'MISMATCH'}")
    if args.output:
        os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
        open(args.output, "wb").write(out)
        print(f"[✓] wrote {args.output}")
    return 0 if same else 2


def cmd_selftest(args) -> int:
    """Prove the ported mechanism on the MC-101 reference pair, if it is around."""
    here = os.path.dirname(os.path.abspath(__file__))
    ref = os.path.join(here, "..", "..", "mc-101-re", "vendor", "mc101_sys_v182",
                       "extracts", "RPG69_C0A")
    enc = os.path.join(ref, "App1_Main.enc")
    dec = os.path.join(ref, "App1_Main_decrypted.bin")
    if not (os.path.exists(enc) and os.path.exists(dec)):
        print(f"[!] reference pair not found under {ref}; nothing to compare")
        return 3
    head = open(os.path.join(ref, "header.bin"), "rb").read()
    i = parse_header(head)
    key, src = _key(args)
    ct = open(enc, "rb").read()
    want = open(dec, "rb").read()
    got = transform(ct[:1 << 20], key, decrypt=True)
    same = got == want[:1 << 20]
    print(f"[{'+' if same else '-'}] MC-101 key slot 0x{i['key_index']:02X} (from {src}): "
          f"1 MiB {'matches' if same else 'DIFFERS'} the reference plaintext")
    return 0 if same else 2


def main(argv=None) -> int:
    p = argparse.ArgumentParser(description=__doc__.split("\n")[1].strip())
    sub = p.add_subparsers(dest="cmd", required=True)

    def common(sp):
        sp.add_argument("--key", help="24-byte 3-key 3DES key as hex (48 digits)")
        sp.add_argument("--key-file", help="read the key from a file, or '-' for stdin")
        sp.add_argument("--key-index", type=lambda s: int(s, 0), default=None,
                        help="boot ROM key slot (default: taken from the container header)")
        sp.add_argument("--bootrom", help="silicon Boot ROM dump to read the key table from")

    a = sub.add_parser("info", help="show the container header and crypto fields")
    a.add_argument("container")
    a.set_defaults(func=cmd_info)

    a = sub.add_parser("decrypt", help="decrypt the payload")
    a.add_argument("container")
    a.add_argument("-o", "--output")
    common(a)
    a.set_defaults(func=cmd_decrypt)

    a = sub.add_parser("encrypt", help="re-encrypt a modified payload into a container")
    a.add_argument("plain")
    a.add_argument("--header-from", required=True,
                   help="original container to take the clear header/trailer from")
    a.add_argument("-o", "--output")
    common(a)
    a.set_defaults(func=cmd_encrypt)

    a = sub.add_parser("selftest", help="verify the ported cipher against the MC-101 pair")
    common(a)
    a.set_defaults(func=cmd_selftest)

    args = p.parse_args(argv)
    if (getattr(args, "key_index", None) is None
            and args.cmd in ("decrypt", "encrypt", "selftest")):
        # default to whatever the container header names
        src = {"decrypt": lambda: args.container,
               "encrypt": lambda: args.header_from,
               "selftest": lambda: os.path.join(
                   os.path.dirname(os.path.abspath(__file__)), "..", "..", "mc-101-re",
                   "vendor", "mc101_sys_v182", "extracts", "RPG69_C0A", "header.bin")}[args.cmd]()
        try:
            args.key_index = parse_header(open(src, "rb").read())["key_index"]
        except OSError:
            args.key_index = 0xB6          # documented default slot for App1_Main
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
