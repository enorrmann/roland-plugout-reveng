#!/usr/bin/env python3
"""Derives slot-relocation and factory-preset vectors (see data-formats.md
DATA-6, DATA-7). Uses check_vectors.py only for container decoding."""
import json, os, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import check_vectors as C


def regions(path):
    raw = C.decrypt(open(path, "rb").read())
    d_off, stride, count = struct.unpack_from("<III", raw, 0x50)
    blocks = []
    for i in range(count):
        o = d_off + i * stride
        ln = struct.unpack_from("<I", raw, o)[0]
        m = raw[o + 4:o + 4 + ln]
        blocks.append(None if m[7:11] == bytes([0x70, 0x30, 1, 9]) else C.unpack_7in8(C.sysex_data(m)))
    half, out = count // 2, []
    for r in range(2):
        part = blocks[r * half:(r + 1) * half]
        n = next((k for k, b in enumerate(part) if b is None), len(part))
        rr = b"".join(part[:n])
        csize, _, _, dsize = struct.unpack_from("<IIII", rr, 0x30)
        out.append(C.lzss_decode(rr[0x40:0x40 + csize], dsize))
    return out


def rev4(b):
    return b"".join(b[i:i + 4][::-1] for i in range(0, len(b), 4))


root = sys.argv[1]
res = {"relocation": {}, "presets": {}}
for pl, files in (("JX-3P", 3), ("JUNO-60", 3), ("SH-101", 3)):
    R = [regions(os.path.join(root, "plugins", pl, "Script", "Code8_%d.Dat" % s)) for s in (1, 2, 3)]
    for reg in (0, 1):
        for a, b, delta in ((0, 1, 0x200000), (0, 2, 0x400000)):
            x, y = R[a][reg], R[b][reg]
            assert len(x) == len(y)
            words = diff_other = 0
            for o in range(0, len(x), 4):
                u, v = struct.unpack_from("<I", x, o)[0], struct.unpack_from("<I", y, o)[0]
                if u != v:
                    if v - u == delta: words += 1
                    else: diff_other += 1
            diff_bytes = sum(1 for i in range(len(x)) if x[i] != y[i])
            res["relocation"]["%s region%d slot%d->slot%d" % (pl, reg, a + 1, b + 1)] = {
                "words_shifted_by_delta": words, "words_other_difference": diff_other,
                "delta": hex(delta), "differing_bytes": diff_bytes}
    z = R[0][0]
    names = []
    for i in range(64):
        o = 0x8000 + i * 0x200 + 0x100
        t = rev4(z[o:o + 16])
        if not all(32 <= c < 127 for c in t) or not t.strip():
            break
        names.append(t.decode().rstrip())
    ph = sum(1 for n in names if set(n) == {"-"})
    names = [n for n in names if set(n) != {"-"}]
    res["presets"][pl] = {"table_offset": "0x8000", "stride": "0x200", "name_offset_in_record": "0x100",
                           "slots": 64, "named": len(names), "placeholder_slots": ph, "names": names}
json.dump(res, sys.stdout, indent=1)
print()
