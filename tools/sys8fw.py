#!/usr/bin/env python3
"""
sys8fw.py -- unpack, inspect, patch and rebuild the SYSTEM-8 updater (SYSTEM8_UP.BIN).

    python3 tools/sys8fw.py info    firmware/SYSTEM8_UP.BIN
    python3 tools/sys8fw.py unpack  firmware/SYSTEM8_UP.BIN work/up
    python3 tools/sys8fw.py pack    work/up -o out/SYSTEM8_UP.BIN
    python3 tools/sys8fw.py verify  firmware/SYSTEM8_UP.BIN        # unpack + pack == original
    python3 tools/sys8fw.py symbols work/up --elf-dir work/elf       # RTTI classes, vtables, ELF

What is decoded: the tar container, the PRELOAD_PLUGOUT wrapper, the
"SYSTEM-8:Plugout" regions (LZSS) of slot 0 and the factory PLUG-OUTs.
What is NOT: the `App1_Main` OS file, which is encrypted with a 64-bit block
cipher whose key is not in the updater (docs/BASE_FIRMWARE.md). It is carried
through unchanged unless you supply a replacement with `--os`.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import lzss  # noqa: E402
import plugout_image as I  # noqa: E402

TAR_BLOCK = 512
TAR_PAD = 10240                      # GNU tar blocking factor 20
OS_MAGIC = b"App1_Main"
PRELOAD_HEADER = 0x40
REGION_HEADER = I.REGION_HEADER_SIZE


def say(*a):
    print(*a, file=sys.stderr)


# ---------------------------------------------------------------------------
# tar (headers are kept verbatim so a rebuild is byte-exact)
# ---------------------------------------------------------------------------

def tar_checksum(h: bytes) -> int:
    return sum(h[:148]) + 8 * 0x20 + sum(h[156:])


def tar_members(raw: bytes) -> list[dict]:
    out, off = [], 0
    while off + TAR_BLOCK <= len(raw):
        h = raw[off:off + TAR_BLOCK]
        if not any(h):
            break
        size = int(h[124:135].split(b"\0")[0].strip() or b"0", 8)
        chk = int(h[148:154].strip() or b"0", 8)
        if chk != tar_checksum(h):
            raise ValueError(f"tar header at {off:#x}: bad checksum")
        data = raw[off + TAR_BLOCK:off + TAR_BLOCK + size]
        out.append({"name": h[:100].split(b"\0")[0].decode(), "header": h, "data": data})
        off += TAR_BLOCK + (size + TAR_BLOCK - 1) // TAR_BLOCK * TAR_BLOCK
    return out


def tar_build(members: list[dict]) -> bytes:
    out = bytearray()
    for m in members:
        h = bytearray(m["header"])
        h[124:136] = b"%011o\0" % len(m["data"])
        h[148:156] = b"        "
        h[148:156] = b"%06o\0 " % tar_checksum(bytes(h))
        out += h + m["data"] + b"\0" * (-len(m["data"]) % TAR_BLOCK)
    out += b"\0" * (TAR_BLOCK * 2)
    out += b"\0" * (-len(out) % TAR_PAD)
    return bytes(out)


# ---------------------------------------------------------------------------
# App1_Main (encrypted OS) header / trailer
# ---------------------------------------------------------------------------

def parse_os(data: bytes) -> dict:
    """0x60-byte clear header + encrypted payload + 0x40-byte clear trailer."""
    if not data.startswith(OS_MAGIC):
        raise ValueError("not an App1_Main file")
    payload = struct.unpack_from("<I", data, 0x44)[0]
    return {
        "name": data[:16].split(b"\0")[0].decode(),
        "date": data[0x10:0x20].decode("ascii", "replace"),
        "tool": data[0x20:0x28].decode("ascii", "replace"),
        "load": struct.unpack_from("<I", data, 0x28)[0],
        "field_30": struct.unpack_from("<I", data, 0x30)[0],
        "field_40": struct.unpack_from("<I", data, 0x40)[0],
        "payload_size": payload,
        "field_48": struct.unpack_from("<I", data, 0x48)[0],
        "check_50": struct.unpack_from("<I", data, 0x50)[0],
        "field_54": struct.unpack_from("<I", data, 0x54)[0],
        "header_size": 0x60,
        "trailer": data[0x60 + payload:].hex(),
        "trailer_text": data[0x60 + payload:].replace(b"\0", b" ").decode("ascii", "replace").strip(),
        "file_size": len(data),
        "payload_sha256": hashlib.sha256(data[0x60:0x60 + payload]).hexdigest(),
    }


# ---------------------------------------------------------------------------
# PLUG-OUT style components (p0m, p0s, pl*)
# ---------------------------------------------------------------------------

def parse_plugout(data: bytes) -> dict:
    """One file = [PRELOAD_PLUGOUT header 0x40]? + region at offset A [+ region at offset B]."""
    pre = None
    off = 0
    if data[:16] == I.PRELOAD_MAGIC:
        f = struct.unpack_from("<4I", data, 0x28)
        pre = {"text": data[:16].decode(), "date": data[0x10:0x20].decode(),
               "ver": data[0x20:0x28].decode(), "f28": f[0], "hsize": f[1],
               "f38": struct.unpack_from("<2I", data, 0x38)}
        if f[1] != PRELOAD_HEADER:
            raise ValueError("unexpected PRELOAD header size")
        if sum(data[0x40:]) & 0xFFFFFFFF != f[3]:
            raise ValueError("PRELOAD payload sum mismatch")
        if f[2] != len(data) - 0x40:
            raise ValueError("PRELOAD payload size mismatch")
        off = 0x40
    regions = []
    pos = off
    while pos < len(data):
        if data[pos:pos + 16] not in I.REGION_MAGICS:
            nxt = None
            # skip zero padding up to the next region magic (or file end)
            for m in I.REGION_MAGICS:
                j = data.find(m, pos)
                if j >= 0 and (nxt is None or j < nxt):
                    nxt = j
            if nxt is None:
                if any(data[pos:]):
                    raise ValueError(f"unexpected data at {pos:#x}")
                break
            if any(data[pos:nxt]):
                raise ValueError(f"non-zero padding at {pos:#x}")
            pos = nxt
        r = I.Region.from_raw(data[pos:])
        regions.append({"offset": pos, "header": r.header, "plain": r.plain,
                        "raw_len": REGION_HEADER + r.header.clen})
        pos += REGION_HEADER + r.header.clen
    return {"pre": pre, "regions": regions, "size": len(data)}


def build_plugout(meta: dict, plains: list[bytes]) -> bytes:
    pre = meta.get("pre")
    start = PRELOAD_HEADER if pre else 0
    body = bytearray()
    for r, plain in zip(meta["regions"], plains):
        comp = lzss.encode(plain)
        hdr = I.RegionHeader(stamp=bytes.fromhex(r["stamp"]), tool=bytes.fromhex(r["tool"]),
                             load=r["load"], entry=r["entry"], clen=len(comp),
                             csum=sum(comp) & 0xFFFFFFFF, ulen=len(plain),
                             magic=bytes.fromhex(r["magic"]))
        blob = hdr.pack() + comp
        at = r["offset"] - start
        if at < len(body):
            raise ValueError(f"region at {r['offset']:#x} overlaps the previous one "
                             f"({len(body) + start:#x}); the image no longer fits")
        body += b"\0" * (at - len(body)) + blob
    if pre:
        h = bytearray(PRELOAD_HEADER)
        h[0:16] = pre["text"].encode().ljust(16, b"\0")
        h[0x10:0x20] = pre["date"].encode().ljust(16, b"\0")
        h[0x20:0x28] = pre["ver"].encode().ljust(8, b"\0")
        struct.pack_into("<4I", h, 0x28, pre["f28"], PRELOAD_HEADER, len(body),
                         sum(body) & 0xFFFFFFFF)
        struct.pack_into("<2I", h, 0x38, *pre["f38"])
        return bytes(h) + bytes(body)
    return bytes(body)


# ---------------------------------------------------------------------------
# project directory
# ---------------------------------------------------------------------------

def stem(name: str) -> str:
    return os.path.basename(name).rsplit(".", 1)[0]


def cmd_info(args) -> int:
    raw = open(args.up, "rb").read()
    for m in tar_members(raw):
        d = m["data"]
        print(f"{m['name']}  {len(d)} bytes  sha256 {hashlib.sha256(d).hexdigest()[:16]}")
        if d.startswith(OS_MAGIC):
            o = parse_os(d)
            print(f"    OS image {o['name']} built {o['date']} tool {o['tool']} load {o['load']:#x}"
                  f" payload {o['payload_size']:#x} (encrypted)")
            print(f"    trailer: {o['trailer_text']!r}")
        else:
            p = parse_plugout(d)
            if p["pre"]:
                print(f"    {p['pre']['text']} built {p['pre']['date']}")
            for i, r in enumerate(p["regions"]):
                h, s = r["header"], I.SynInfo.parse(r["plain"])
                ident = f"{s.name} id {s.model_id:#06x} v{s.version >> 8:X}.{s.version & 0xFF:02X}" if s else "?"
                print(f"    region {i} @{r['offset']:#x}: load {h.load:#x} stamp {h.stamp.decode()} "
                      f"{h.clen}->{h.ulen} bytes  {ident}")
    return 0


def cmd_unpack(args) -> int:
    raw = open(args.up, "rb").read()
    os.makedirs(args.dir, exist_ok=True)
    manifest = {"format": 1, "source_sha256": hashlib.sha256(raw).hexdigest(), "members": []}
    for m in tar_members(raw):
        d, n = m["data"], stem(m["name"])
        ent = {"name": m["name"], "header": m["header"].hex(), "stem": n}
        if d.startswith(OS_MAGIC):
            o = parse_os(d)
            ent.update(kind="os", os=o, file=f"{n}.enc")
            open(os.path.join(args.dir, f"{n}.enc"), "wb").write(d)
        else:
            p = parse_plugout(d)
            ent.update(kind="plugout", pre=p["pre"], regions=[])
            for i, r in enumerate(p["regions"]):
                h = r["header"]
                fn = f"{n}.region{i}.bin"
                open(os.path.join(args.dir, fn), "wb").write(r["plain"])
                ent["regions"].append({"file": fn, "offset": r["offset"], "stamp": h.stamp.hex(),
                                       "tool": h.tool.hex(), "load": h.load, "entry": h.entry,
                                       "magic": h.magic.hex()})
        manifest["members"].append(ent)
    with open(os.path.join(args.dir, "sys8fw.json"), "w") as fh:
        json.dump(manifest, fh, indent=1)
    say(f"unpacked {len(manifest['members'])} members into {args.dir}")
    return 0


def load_project(path: str) -> dict:
    with open(os.path.join(path, "sys8fw.json")) as fh:
        return json.load(fh)


def build_up(path: str, os_override: str | None = None) -> bytes:
    man = load_project(path)
    members = []
    for ent in man["members"]:
        if ent["kind"] == "os":
            data = open(os_override or os.path.join(path, ent["file"]), "rb").read()
        else:
            plains = [open(os.path.join(path, r["file"]), "rb").read() for r in ent["regions"]]
            data = build_plugout(ent, plains)
        members.append({"name": ent["name"], "header": bytes.fromhex(ent["header"]), "data": data})
    return tar_build(members)


def cmd_pack(args) -> int:
    out = build_up(args.dir, args.os)
    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    open(args.output, "wb").write(out)
    say(f"wrote {args.output} ({len(out)} bytes, sha256 {hashlib.sha256(out).hexdigest()[:16]})")
    return 0


def cmd_verify(args) -> int:
    import tempfile
    raw = open(args.up, "rb").read()
    with tempfile.TemporaryDirectory() as td:
        class A:  # noqa: D401
            up, dir = args.up, td
        cmd_unpack(A)
        out = build_up(td)
    ok = out == raw
    say("ok   rebuild is byte-identical" if ok else "FAIL rebuild differs")
    return 0 if ok else 1


# ---------------------------------------------------------------------------
# symbols: RTTI classes, vtables, virtual methods -> ELF for Ghidra / IDA
# ---------------------------------------------------------------------------

def image_base(plain: bytes, region: int) -> int | None:
    s = I.SynInfo.parse(plain)
    return None if s is None else s.init & 0xFFE00000


def find_classes(plain: bytes, base: int) -> list[dict]:
    """Itanium-ABI RTTI: name string <- typeinfo <- vtable[1]; then method pointers."""
    import re
    end = base + len(plain)
    words = struct.unpack_from(f"<{len(plain) // 4}I", plain)
    pos_of = {}
    for i, w in enumerate(words):
        if base <= w < end:
            pos_of.setdefault(w, []).append(i)
    names = {}
    for m in re.finditer(rb"(?<![0-9A-Za-z_])(\d{1,3})([A-Za-z_][A-Za-z0-9_]*)\0", plain):
        if int(m.group(1)) == len(m.group(2)):
            names[base + m.start()] = m.group(2).decode()
    classes = []
    for naddr, cname in names.items():
        for ti_idx in pos_of.get(naddr, []):
            ti = base + (ti_idx - 1) * 4          # typeinfo = {vptr, name, [bases]}
            if not (0 <= ti_idx - 1):
                continue
            for vt_idx in pos_of.get(ti, []):
                if vt_idx == 0 or words[vt_idx - 1] != 0:
                    continue
                vt = base + vt_idx * 4 - 4        # vtable address of the offset-to-top slot
                methods, j = [], vt_idx + 1
                while j < len(words):
                    w = words[j]
                    if (w & 1) and base <= (w & ~1) < end:
                        methods.append(w)
                        j += 1
                    elif w == 0 and methods == [] and False:
                        j += 1
                    else:
                        break
                base_ti = words[ti_idx + 1] if ti_idx + 1 < len(words) else 0
                classes.append({"name": cname, "typeinfo": ti, "vtable": vt + 8, "methods": methods,
                                "base_typeinfo": base_ti})
    by_ti = {c["typeinfo"]: c["name"] for c in classes}
    for c in classes:
        c["base"] = by_ti.get(c.pop("base_typeinfo"))
    return sorted(classes, key=lambda c: c["vtable"])


def find_labels(plain: bytes) -> list[dict]:
    """Value-label lists of a model region.

    Header (5 words) <2><0x14><list id><count><0>, then `count` entries of
    7 words: <3><0x1C><char[20]>.
    """
    w = struct.unpack_from(f"<{len(plain) // 4}I", plain)
    out, i = [], 0
    while i < len(w) - 12:
        if w[i] == 2 and w[i + 1] == 0x14 and w[i + 4] == 0 and 0 < w[i + 3] < 256:
            j, ents = i + 5, []
            while j + 7 <= len(w) and w[j] == 3 and w[j + 1] == 0x1C:
                raw = plain[(j + 2) * 4:(j + 2) * 4 + 20].split(b"\0")[0]
                ents.append(raw.decode("latin1"))
                j += 7
            if ents:
                out.append({"offset": i * 4, "id": w[i + 2], "declared": w[i + 3], "labels": ents})
                i = j
                continue
        i += 1
    return out


def find_presets(plain: bytes) -> list[str]:
    """Factory preset names: 16 chars at +0x100 of each 0x200 record from 0x8000,
    each 4-byte group stored reversed."""
    names = []
    for k in range(256):
        o = 0x8000 + k * 0x200 + 0x100
        if o + 16 > len(plain):
            break
        raw = b"".join(plain[o + g:o + g + 4][::-1] for g in range(0, 16, 4))
        if not all(32 <= c < 127 for c in raw) or not raw.strip(b" -"):
            break
        names.append(raw.decode().rstrip())
    return names


def _report_os(args, ent: dict) -> tuple:
    """Report for the encrypted OS member: clear header/trailer + payload statistics."""
    import collections
    import math
    data = open(os.path.join(args.dir, ent["file"]), "rb").read()
    o = parse_os(data)
    pay = data[0x60:0x60 + o["payload_size"]]
    blocks = [pay[k:k + 8] for k in range(0, len(pay) - 7, 8)]
    cnt = collections.Counter(blocks)
    n = len(pay)
    ent_bits = -sum(v / n * math.log2(v / n) for v in collections.Counter(pay).values())
    top = cnt.most_common(5)
    first = {b: next(k * 8 for k, x in enumerate(blocks) if x == b) for b, _ in top}
    last = {}
    for k, x in enumerate(blocks):
        if x in first:
            last[x] = k * 8
    h = data[:0x60]
    md = [f"# {ent['stem']} (App1_Main, the OS) - NOT DECODED", "",
          "Encrypted payload; only the clear header and trailer can be read. No classes, vtables, "
          "presets or labels can be extracted until the payload is decrypted (docs/BASE_FIRMWARE.md section 2).", "",
          "## Clear header (0x60 bytes)", "",
          "| offset | value | note |", "|---|---|---|",
          f"| 0x00 | `{o['name']}` | image name |",
          f"| 0x10 | `{o['date']}` | build date |",
          f"| 0x20 | `{o['tool']}` | tool/version tag |",
          f"| 0x28 | `{o['load']:#010x}` | load address |",
          f"| 0x30 | `{o['field_30']:#010x}` | then 3 x `FFFFFFFF` |",
          f"| 0x40 | `{o['field_40']:#010x}` | |",
          f"| 0x44 | `{o['payload_size']:#x}` | payload size |",
          f"| 0x48 | `{o['field_48']:#010x}` | |",
          f"| 0x50 | `{o['check_50']:#010x}` | probable plaintext check value |",
          f"| 0x54 | `{o['field_54']:#010x}` | |", "",
          "Raw header:", "", "```", h.hex(" ", 4), "```", "",
          "## Clear trailer (0x40 bytes)", "",
          f"`{o['trailer']}`", "", f"Text: `{o['trailer_text']}`", "",
          "## Payload statistics", "",
          f"* file {o['file_size']} bytes = 0x60 + {o['payload_size']:#x} payload + 0x40 trailer",
          f"* payload SHA-256 `{o['payload_sha256']}`",
          f"* byte entropy {ent_bits:.3f} bits/byte; 8-byte blocks: {len(blocks)} total, "
          f"{len(cnt)} distinct",
          "* most repeated 8-byte ciphertext blocks (identical plaintext under ECB):", "",
          "| block (hex) | count | first offset | last offset |", "|---|---|---|---|"]
    for b_, c_ in top:
        md.append(f"| `{b_.hex()}` | {c_} | `{first[b_] + 0x60:#x}` | `{last[b_] + 0x60:#x}` |")
    md += ["", "(offsets are file offsets)", "",
           "## Cipher conclusions", "",
           "* 64-bit block cipher in ECB mode (equal plaintext blocks give equal ciphertext; 8-byte "
           "halves repeat independently of their 16-byte neighbours).",
           "* Full diffusion; no XOR/rotate/linear structure; not the plug-out chained XOR.",
           "* Keys derived from header, trailer and product strings failed for Blowfish, CAST5, "
           "DES/3DES, IDEA, TEA and XTEA. The key is in the device bootloader."]
    tag = ent["stem"]
    open(os.path.join(args.out, f"{tag}.md"), "w").write("\n".join(md) + "\n")
    return (tag, f"{o['name']} (encrypted OS) built {o['date']}", None, o["load"], 0, 0, 0)


def cmd_report(args) -> int:
    man = load_project(args.dir)
    os.makedirs(args.out, exist_ok=True)
    allsyms = {}
    index = []
    for ent in man["members"]:
        if ent["kind"] == "os":
            index.append(_report_os(args, ent))
            continue
        if ent["kind"] != "plugout":
            continue
        for i, r in enumerate(ent["regions"]):
            plain = open(os.path.join(args.dir, r["file"]), "rb").read()
            sinfo = I.SynInfo.parse(plain)
            base = image_base(plain, i)
            tag = r["file"][:-4]
            ident = (f"{sinfo.name} id {sinfo.model_id:#06x} v{sinfo.version >> 8:X}."
                     f"{sinfo.version & 0xFF:02X}" if sinfo else "?")
            kinds = []
            cls = find_classes(plain, base) if sinfo and base else []
            lab = find_labels(plain)
            pre = find_presets(plain)
            allsyms[r["file"]] = {"base": base, "classes": cls}
            # function start addresses for approximate sizes
            starts = sorted({m & ~1 for c in cls for m in c["methods"]} | {sinfo.init & ~1} if sinfo else set())
            def size_of(m):
                a = m & ~1
                k = starts.index(a) if a in starts else -1
                nxt = starts[k + 1] if 0 <= k + 1 < len(starts) else base + len(plain)
                return nxt - a
            md = [f"# {tag}", "", f"* image `{r['file']}`, {len(plain)} bytes, base `{base:#x}`",
                  f"* identity: {ident}; init `{sinfo.init:#x}`" if sinfo else "",
                  f"* classes: {len(cls)}, virtual methods: {sum(len(c['methods']) for c in cls)}", ""]
            if cls:
                md += ["## Classes", "", "| vtable | class | base | virtual methods | first method |",
                       "|---|---|---|---|---|"]
                for c in cls:
                    md.append(f"| `{c['vtable']:#x}` | `{c['name']}` | `{c['base'] or ''}` | "
                              f"{len(c['methods'])} | `{c['methods'][0]:#x}` |" if c["methods"] else
                              f"| `{c['vtable']:#x}` | `{c['name']}` | `{c['base'] or ''}` | 0 | |")
                md += ["", "## Virtual methods (address, upper-bound size)", ""]
                for c in cls:
                    md += [f"### `{c['name']}` @ `{c['vtable']:#x}`", ""]
                    md.append(", ".join(f"v{k} `{m:#x}` ({size_of(m)} B)" for k, m in enumerate(c["methods"])))
                    md.append("")
            if pre:
                md += [f"## Factory presets ({len(pre)})", ""] + [f"{k + 1:2d}. {n}" for k, n in enumerate(pre)] + [""]
            if lab:
                md += [f"## Value-label lists ({len(lab)})", ""]
                for L in lab:
                    md.append(f"* offset `{L['offset']:#x}` id {L['id']} ({len(L['labels'])} labels): "
                              + ", ".join(f"`{x}`" for x in L["labels"]))
                md.append("")
            open(os.path.join(args.out, f"{tag}.md"), "w").write("\n".join(md))
            index.append((tag, ident, base, len(cls), sum(len(c["methods"]) for c in cls), len(pre), len(lab)))
    with open(os.path.join(args.out, "symbols.json"), "w") as fh:
        json.dump({k: {"base": v["base"], "classes": v["classes"]} for k, v in allsyms.items()}, fh, indent=1)
    with open(os.path.join(args.out, "index.md"), "w") as fh:
        fh.write("| image | identity | base | classes | virtual methods | presets | label lists |\n|---|---|---|---|---|---|---|\n")
        for t, ident, b, nc, nm, npz, nl in index:
            fh.write(f"| [{t}]({t}.md) | {ident} | `{b:#x}` | {nc} | {nm} | {npz} | {nl} |\n"
                     if b is not None else
                     f"| [{t}]({t}.md) | {ident} | `{nc:#x}` load | not decoded | - | - | - |\n")
    say(f"wrote report for {len(index)} images into {args.out}")
    return 0


def cmd_symbols(args) -> int:
    man = load_project(args.dir)
    out = {}
    for ent in man["members"]:
        if ent["kind"] != "plugout":
            continue
        for i, r in enumerate(ent["regions"]):
            plain = open(os.path.join(args.dir, r["file"]), "rb").read()
            base = image_base(plain, i)
            if base is None:
                continue
            cls = find_classes(plain, base)
            key = r["file"]
            out[key] = {"base": base, "classes": [
                {"name": c["name"], "vtable": c["vtable"], "typeinfo": c["typeinfo"],
                 "base": c["base"], "methods": c["methods"]} for c in cls]}
            say(f"{key}: base {base:#x}, {len(cls)} classes, "
                f"{sum(len(c['methods']) for c in cls)} virtual methods")
            if args.elf_dir:
                os.makedirs(args.elf_dir, exist_ok=True)
                syms, seen = [], set()
                s = I.SynInfo.parse(plain)
                syms.append((f"{ent['stem']}_r{i}_syninfo", base, False))
                syms.append((f"{ent['stem']}_r{i}_init", s.init, True))
                for c in cls:
                    syms.append((f"vt_{c['name']}", c["vtable"], False))
                    for k, mth in enumerate(c["methods"]):
                        if mth not in seen:
                            seen.add(mth)
                            syms.append((f"{c['name']}__v{k}", mth, True))
                elf = I.to_elf([(".image", base, plain)], syms, s.init)
                open(os.path.join(args.elf_dir, f"{ent['stem']}.region{i}.elf"), "wb").write(elf)
    if args.json:
        with open(args.json, "w") as fh:
            json.dump(out, fh, indent=1)
    return 0


def _image_file(man: dict, name: str) -> tuple[dict, dict]:
    for ent in man["members"]:
        for r in ent.get("regions", []):
            if r["file"] == name:
                return ent, r
    raise SystemExit(f"no image {name!r}; see sys8fw.json")


def cmd_patch(args) -> int:
    man = load_project(args.dir)
    _, r = _image_file(man, args.image)
    path = os.path.join(args.dir, r["file"])
    plain = bytearray(open(path, "rb").read())
    base = image_base(bytes(plain), 0)
    if base is None:
        raise SystemExit("image has no SYSTEM8_SYNINFO; use --offset")
    off = args.offset if args.offset is not None else args.at - base
    if args.at is not None and args.offset is not None:
        raise SystemExit("use --at or --offset, not both")
    blob = bytes.fromhex(args.hex) if args.hex else open(args.file, "rb").read()
    if not 0 <= off <= len(plain):
        raise SystemExit(f"offset {off:#x} is outside the image (size {len(plain):#x}, base {base:#x})")
    if off + len(blob) > len(plain):
        plain.extend(b"\0" * (off + len(blob) - len(plain)))
    old = bytes(plain[off:off + len(blob)])
    plain[off:off + len(blob)] = blob
    open(path, "wb").write(bytes(plain))
    say(f"{args.image}: {len(blob)} bytes at {base + off:#x} (offset {off:#x}); was {old.hex()}")
    return 0


def cmd_disasm(args) -> int:
    import subprocess
    import tempfile
    cmd_symbols(argparse.Namespace(dir=args.dir, elf_dir=args.out, json=None))
    for fn in sorted(os.listdir(args.out)):
        if not fn.endswith(".elf"):
            continue
        txt = os.path.join(args.out, fn[:-4] + ".S")
        with open(txt, "w") as fh:
            subprocess.run([args.objdump, "-d", "-M", "force-thumb", os.path.join(args.out, fn)],
                           stdout=fh, check=True)
        say(f"wrote {txt}")
    return 0


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("info"); p.add_argument("up"); p.set_defaults(func=cmd_info)
    p = sub.add_parser("unpack"); p.add_argument("up"); p.add_argument("dir"); p.set_defaults(func=cmd_unpack)
    p = sub.add_parser("pack"); p.add_argument("dir"); p.add_argument("-o", "--output", required=True)
    p.add_argument("--os", help="replacement App1_Main file (already encrypted)"); p.set_defaults(func=cmd_pack)
    p = sub.add_parser("verify"); p.add_argument("up"); p.set_defaults(func=cmd_verify)
    p = sub.add_parser("symbols"); p.add_argument("dir"); p.add_argument("--elf-dir")
    p.add_argument("--json"); p.set_defaults(func=cmd_symbols)
    p = sub.add_parser("report", help="per-image class/preset/label tables (markdown + JSON)")
    p.add_argument("dir"); p.add_argument("--out", required=True); p.set_defaults(func=cmd_report)
    p = sub.add_parser("patch", help="write bytes into an unpacked image at a run address")
    p.add_argument("dir"); p.add_argument("--image", required=True, help="file name, e.g. p1y01_p0s.region0.bin")
    p.add_argument("--at", type=lambda x: int(x, 0)); p.add_argument("--offset", type=lambda x: int(x, 0))
    g = p.add_mutually_exclusive_group(required=True)
    g.add_argument("--hex"); g.add_argument("--file"); p.set_defaults(func=cmd_patch)
    p = sub.add_parser("disasm", help="symbols + objdump listings (needs arm-none-eabi-objdump)")
    p.add_argument("dir"); p.add_argument("--out", required=True)
    p.add_argument("--objdump", default="arm-none-eabi-objdump"); p.set_defaults(func=cmd_disasm)
    args = ap.parse_args(argv)
    if getattr(args, "cmd", "") == "patch" and (args.at is None) == (args.offset is None):
        ap.error("patch needs exactly one of --at / --offset")
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
