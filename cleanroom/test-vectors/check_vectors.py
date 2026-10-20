#!/usr/bin/env python3
"""
Reference checker written ONLY from docs/data-formats.md and
docs/protocol-spec.md (no import of the tools/ package).

It re-derives every field listed in container-vectors.json from the raw
Code*.Dat files. A conforming independent implementation must obtain the same
values. Usage:

    python3 check_vectors.py <repo-root>            # compare with container-vectors.json
    python3 check_vectors.py <repo-root> --emit     # print fresh vectors (JSON)
"""
import hashlib, json, os, struct, sys

IV = 0xBACADABE


def decrypt(data):
    """DATA-1: chained XOR over little-endian 32-bit words."""
    n = len(data) // 4
    words = struct.unpack("<%dI" % n, data[: n * 4])
    out, prev = [], IV
    for w in words:
        out.append(w ^ prev)
        prev = w
    return struct.pack("<%dI" % n, *out) + data[n * 4:]


def unpack_7in8(wire):
    out = bytearray()
    for i in range(0, len(wire), 8):
        msb = wire[i]
        for j, b in enumerate(wire[i + 1:i + 8]):
            out.append(b | (((msb >> j) & 1) << 7))
    return bytes(out)


def lzss_decode(src, size):
    ring = bytearray(4096)          # ring initialised to 0x00
    r, out, i = 4096 - 18, bytearray(), 0
    while i < len(src) and len(out) < size:
        flags = src[i]; i += 1
        for bit in range(8):
            if i >= len(src) or len(out) >= size:
                break
            if flags >> bit & 1:
                c = src[i]; i += 1
                out.append(c); ring[r] = c; r = (r + 1) & 4095
            else:
                b0, b1 = src[i], src[i + 1]; i += 2
                pos = b0 | (b1 & 0xF0) << 4
                for k in range((b1 & 0x0F) + 3):
                    c = ring[(pos + k) & 4095]
                    out.append(c); ring[r] = c; r = (r + 1) & 4095
    return bytes(out[:size])


def sysex_data(m):
    # F0 41 dev 00 00 7E cmd a1 a2 a3 a4 <data...> cs F7
    return m[11:-2]


def sysex_addr(m):
    return bytes(m[7:11])


def analyse(path):
    raw = open(path, "rb").read()
    p = decrypt(raw)
    assert struct.unpack_from("<I", p, 0)[0] == 0x80
    assert p[4:19] == b"PLUGOUT_SYS_INF"
    name = p[0x14:0x24].split(b"\0")[0].decode()
    model_id, version = struct.unpack_from("<HH", p, 0x24)
    d_off, stride, count = struct.unpack_from("<III", p, 0x50)
    e_off, e_size = struct.unpack_from("<II", p, 0x40)
    blocks, nfill = [], 0
    for i in range(count):
        o = d_off + i * stride
        ln = struct.unpack_from("<I", p, o)[0]
        m = p[o + 4:o + 4 + ln]
        a = sysex_addr(m)
        if a == bytes([0x70, 0x30, 0x00, 0x09]):
            blocks.append(unpack_7in8(sysex_data(m)))
        elif a == bytes([0x70, 0x30, 0x01, 0x09]):
            blocks.append(None); nfill += 1
        else:
            raise ValueError("unexpected record at %d" % i)
    end_msg = p[e_off + 4:e_off + 4 + struct.unpack_from("<I", p, e_off)[0]]
    end_sum = sum(0xFF * 512 if b is None else sum(b) for b in blocks) & 0xFFFF
    end_nib = 0
    for x in sysex_data(end_msg):
        end_nib = end_nib << 4 | (x & 15)
    half = count // 2
    regions = []
    for r in range(2):
        part = blocks[r * half:(r + 1) * half]
        n = next((k for k, b in enumerate(part) if b is None), len(part))
        rawr = b"".join(part[:n])
        hdr = rawr[:0x40]
        csize, csum, entry, dsize = struct.unpack_from("<IIII", hdr, 0x30)
        load = struct.unpack_from("<I", hdr, 0x28)[0]
        comp = rawr[0x40:0x40 + csize]
        plain = lzss_decode(comp, dsize)
        regions.append({
            "magic": hdr[:16].decode("latin1"),
            "stamp": hdr[16:32].decode("latin1"),
            "tool": hdr[32:40].rstrip(b"\0").decode("latin1"),
            "load": hex(load), "entry": hex(entry),
            "blocks": n, "compressed_size": csize, "plain_size": dsize,
            "header_sum_ok": sum(comp) & 0xFFFFFFFF == csum,
            "plain_sha256": hashlib.sha256(plain).hexdigest(),
            "syninfo_signature": plain[:16].rstrip(b"\0").decode("latin1"),
            "syninfo_name": plain[16:32].split(b"\0")[0].decode("latin1"),
            "syninfo_model_id": hex(struct.unpack_from("<H", plain, 0x20)[0]),
            "syninfo_version": hex(struct.unpack_from("<H", plain, 0x22)[0]),
            "syninfo_init": hex(struct.unpack_from("<I", plain, 0x24)[0]),
        })
    return {
        "file_sha256": hashlib.sha256(raw).hexdigest(),
        "file_size": len(raw),
        "name": name, "model_id": hex(model_id), "version": hex(version),
        "records": count, "data_records": count - nfill, "filler_records": nfill,
        "end_sum_computed": hex(end_sum), "end_sum_in_message": hex(end_nib),
        "regions": regions,
    }


FILES = [
    "plugins/JX-3P/Script/Code1.Dat", "plugins/JX-3P/Script/Code8_1.Dat",
    "plugins/JX-3P/Script/Code8_2.Dat", "plugins/JX-3P/Script/Code8_3.Dat",
    "plugins/JUNO-60/Script/Code8_1.Dat", "plugins/JUNO-60/Script/Code8_2.Dat",
    "plugins/JUNO-60/Script/Code8_3.Dat",
    "plugins/SH-101/Script/Code1.Dat", "plugins/SH-101/Script/Code8_1.Dat",
    "plugins/SH-101/Script/Code8_2.Dat", "plugins/SH-101/Script/Code8_3.Dat",
    "plugins/SH-101/Script/Code.Dat",
]


def main():
    root = sys.argv[1]
    got = {f: analyse(os.path.join(root, f)) for f in FILES}
    if "--emit" in sys.argv:
        json.dump(got, sys.stdout, indent=1, sort_keys=True); print(); return
    here = os.path.dirname(os.path.abspath(__file__))
    ref = json.load(open(os.path.join(here, "container-vectors.json")))
    bad = 0
    for f in FILES:
        ok = got[f] == ref[f]
        bad += not ok
        print("%-45s %s" % (f, "OK" if ok else "MISMATCH"))
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
