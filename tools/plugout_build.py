#!/usr/bin/env python3
"""
plugout_build.py -- unpack, modify and rebuild PLUG-OUT firmware (Code*.Dat).

    python3 tools/plugout_build.py unpack plugins/JUNO-60/Script/Code8_1.Dat work/juno \\
            --relocs-from plugins/JUNO-60/Script/Code8_2.Dat
    python3 tools/plugout_build.py show work/juno
    python3 tools/plugout_build.py set-id work/juno --name "JUNO-60 MOD" --model-id 0x40 --version 1.00
    python3 tools/plugout_build.py patch work/juno --region 1 --at 0x60a10800 --hex "00bf00bf"
    python3 tools/plugout_build.py pack work/juno -o out/Code8_2.Dat --slot 2
    python3 tools/plugout_build.py plugin work/juno --template plugins/JUNO-60 -o plugins/JUNO-MOD
    python3 tools/plugout_build.py elf work/juno -o work/juno.elf
    python3 tools/plugout_build.py verify plugins/*/Script/Code*.Dat

A project directory holds plugout.json, region<n>.bin (decompressed images)
and relocs.json (pointer offsets, needed to build for another slot). The
format is described in CUSTOM_PLUGOUTS.md.
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import plugout as P  # noqa: E402
import plugout_image as I  # noqa: E402

MANIFEST = "plugout.json"
RELOCS = "relocs.json"


def int_auto(s: str) -> int:
    return int(s, 0)


def version_arg(s: str) -> int:
    """'1.01' -> 0x0101 (shown by the plugin as %X.%02X), or a plain integer."""
    if "." in s:
        major, minor = s.split(".")
        return int(major, 16) << 8 | int(minor, 16)
    return int(s, 0)


def version_str(v: int) -> str:
    return f"{v >> 8:X}.{v & 0xFF:02X}"


# ---------------------------------------------------------------------------
# project directory
# ---------------------------------------------------------------------------

class Project:
    def __init__(self, path: str, meta: dict, regions: list[bytes], relocs: dict | None):
        self.path, self.meta, self.regions, self.relocs = path, meta, regions, relocs

    @property
    def device(self) -> str:
        return self.meta["device"]

    @property
    def slot(self) -> int:
        return self.meta["slot"]

    def base(self, region: int, slot: int | None = None) -> int:
        return I.link_base(self.device, region, self.slot if slot is None else slot)

    @classmethod
    def from_container(cls, c: P.Container, path: str = "") -> "Project":
        blocks = I.flash_blocks(c)
        device = I.device_for_count(len(blocks))
        regions, rmeta = [], []
        for i, raw in enumerate(I.raw_regions(blocks)):
            r = I.Region.from_raw(raw)
            regions.append(r.plain)
            rmeta.append({"file": f"region{i}.bin",
                          "stamp": r.header.stamp.decode("latin-1"),
                          "magic": r.header.magic.decode("latin-1"),
                          "tool": r.header.tool.decode("latin-1"),
                          "load": f"{r.header.load:#010x}",
                          "entry": f"{r.header.entry:#010x}"})
        slot = I.detect_slot(device, 1, regions[1]) if device == "SYSTEM-8" else 1
        meta = {"format": 1, "device": device, "slot": slot, "name": c.model,
                "model_id": c.model_id, "version": version_str(c.version),
                "records": len(blocks), "regions": rmeta}
        return cls(path, meta, regions, None)

    @classmethod
    def load(cls, path: str) -> "Project":
        with open(os.path.join(path, MANIFEST)) as fh:
            meta = json.load(fh)
        regions = []
        for r in meta["regions"]:
            with open(os.path.join(path, r["file"]), "rb") as fh:
                regions.append(fh.read())
        relocs = None
        rp = os.path.join(path, RELOCS)
        if os.path.exists(rp):
            with open(rp) as fh:
                relocs = {int(k): v for k, v in json.load(fh).items()}
        return cls(path, meta, regions, relocs)

    def save(self, path: str | None = None) -> None:
        path = path or self.path
        os.makedirs(path, exist_ok=True)
        for r, plain in zip(self.meta["regions"], self.regions):
            with open(os.path.join(path, r["file"]), "wb") as fh:
                fh.write(plain)
        with open(os.path.join(path, MANIFEST), "w") as fh:
            json.dump(self.meta, fh, indent=2)
            fh.write("\n")
        if self.relocs is not None:
            with open(os.path.join(path, RELOCS), "w") as fh:
                json.dump({str(k): v for k, v in self.relocs.items()}, fh)
        self.path = path

    # -- build ---------------------------------------------------------------

    def images_for_slot(self, slot: int) -> list[bytes]:
        if slot == self.slot or self.device != "SYSTEM-8":
            return list(self.regions)
        if self.relocs is None:
            raise SystemExit(f"{self.path}: no {RELOCS}; it is needed to build for slot {slot} "
                             "(unpack with --relocs-from, or run 'relocs')")
        delta = (slot - self.slot) * I.SLOT_STRIDE
        return [I.relocate(p, self.relocs[i], delta) for i, p in enumerate(self.regions)]

    def build(self, slot: int | None = None, log=lambda s: None) -> bytes:
        slot = self.slot if slot is None else slot
        name, mid = self.meta["name"], self.meta["model_id"]
        version = version_arg(self.meta["version"])
        raws = []
        for i, (plain, rm) in enumerate(zip(self.images_for_slot(slot), self.meta["regions"])):
            if i == 0 and plain[:16] == I.SYNINFO_MAGIC:
                plain = I.stamp_identity(plain, name, mid, version)
            h = I.RegionHeader(magic=rm["magic"].encode("latin-1"),
                               stamp=rm["stamp"].encode("latin-1"),
                               tool=rm["tool"].encode("latin-1"),
                               load=int(rm["load"], 0), entry=int(rm["entry"], 0))
            log(f"region {i}: compressing {len(plain)} bytes")
            raws.append(I.Region(h, plain).to_raw())
            log(f"region {i}: {len(raws[-1]) // I.BLOCK_SIZE} of "
                f"{self.meta['records'] // 2} blocks")
        blocks = I.blocks_from_regions(raws, self.meta["records"])
        return I.build_container(name, mid, version, blocks)


# ---------------------------------------------------------------------------
# commands
# ---------------------------------------------------------------------------

def say(msg: str) -> None:
    print(msg, flush=True)


def cmd_unpack(args) -> int:
    proj = Project.from_container(P.load(args.dat))
    if args.relocs_from:
        derive_relocs(proj, Project.from_container(P.load(args.relocs_from)))
    proj.save(args.dir)
    say(f"unpacked {args.dat} -> {args.dir}")
    show(proj)
    return 0


def derive_relocs(proj: Project, other: Project) -> None:
    if other.device != proj.device or other.slot == proj.slot:
        raise SystemExit("--relocs-from needs the same PLUG-OUT built for another slot")
    delta = (other.slot - proj.slot) * I.SLOT_STRIDE
    proj.relocs = {i: I.diff_relocs(a, b, delta)
                   for i, (a, b) in enumerate(zip(proj.regions, other.regions))}
    say("relocations: " + ", ".join(f"region {i}: {len(v)}" for i, v in proj.relocs.items()))


def cmd_relocs(args) -> int:
    proj = Project.load(args.dir)
    derive_relocs(proj, Project.from_container(P.load(args.other)))
    proj.save()
    return 0


def show(proj: Project) -> None:
    m = proj.meta
    say(f"device  : {proj.device}, {m['records']} records, built for slot {proj.slot}")
    say(f"plug-out: {m['name']} id {m['model_id']:#06x} version {m['version']}")
    for i, plain in enumerate(proj.regions):
        base = proj.base(i)
        s = I.SynInfo.parse(plain)
        top = base + len(plain)
        if proj.relocs:
            ptrs = [struct.unpack_from("<I", plain, o)[0] for o in proj.relocs[i]]
            top = max([top] + ptrs)
        say(f"region {i}: {len(plain)} bytes at {base:#010x}-{base + len(plain):#010x}"
            f", highest internal pointer {top:#010x}"
            + (f", {len(proj.relocs[i])} relocations" if proj.relocs else ""))
        if s:
            say(f"  SYNINFO {s.name!r} id {s.model_id:#06x} version {version_str(s.version)} "
                f"init {s.init:#010x} ptr {s.ptr:#010x}")


def cmd_show(args) -> int:
    proj = (Project.load(args.target) if os.path.isdir(args.target)
            else Project.from_container(P.load(args.target), args.target))
    show(proj)
    return 0


def cmd_set_id(args) -> int:
    proj = Project.load(args.dir)
    if args.name is not None:
        I.encode_name(args.name)
        proj.meta["name"] = args.name
    if args.model_id is not None:
        if not 0 <= args.model_id < 0xFFFF:
            raise SystemExit("model id must be 0..0xFFFE (0xFFFF means an empty slot)")
        proj.meta["model_id"] = args.model_id
    if args.version is not None:
        proj.meta["version"] = version_str(args.version)
    proj.save()
    show(proj)
    return 0


def cmd_patch(args) -> int:
    proj = Project.load(args.dir)
    r = args.region
    base = proj.base(r)
    if args.hex is not None:
        data = bytes.fromhex(args.hex.replace(" ", ""))
    else:
        with open(args.file, "rb") as fh:
            data = fh.read()
    off = args.at - base
    if off < 0 or off >= I.SLOT_STRIDE:
        raise SystemExit(f"{args.at:#x} is outside region {r} ({base:#x}-{base + I.SLOT_STRIDE:#x})")
    img = bytearray(proj.regions[r])
    if off + len(data) > len(img):
        say(f"region {r} grows from {len(img)} to {off + len(data)} bytes "
            "(zero filled); keep clear of the image's .bss")
        img += bytes(off + len(data) - len(img))
    img[off:off + len(data)] = data
    proj.regions[r] = bytes(img)
    for a in args.reloc or []:
        if proj.relocs is None:
            raise SystemExit(f"--reloc needs {RELOCS}")
        o = a - base
        if o % 4 or not 0 <= o <= len(img) - 4:
            raise SystemExit(f"--reloc {a:#x} is not an aligned word in region {r}")
        if o not in proj.relocs[r]:
            proj.relocs[r] = sorted(proj.relocs[r] + [o])
    proj.save()
    say(f"wrote {len(data)} bytes at {args.at:#010x} (region {r} offset {off:#x})")
    return 0


def cmd_pack(args) -> int:
    proj = Project.load(args.dir)
    slot = proj.slot if args.slot is None else args.slot
    data = proj.build(slot, log=say)
    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    with open(args.output, "wb") as fh:
        fh.write(data)
    say(P.load(args.output).summary())
    return 0


def cmd_plugin(args) -> int:
    """A plugin folder plugout_tool.py can install: Script/Code8_<n>.Dat for every
    slot, plus the template's Script.xml and Patch/ banks for the patch transfer."""
    proj = Project.load(args.dir)
    if proj.device != "SYSTEM-8":
        raise SystemExit("plugin folders are only built for SYSTEM-8 projects")
    script = os.path.join(args.output, "Script")
    os.makedirs(script, exist_ok=True)
    for slot in args.slots:
        out = os.path.join(script, f"Code8_{slot}.Dat")
        say(f"slot {slot}:")
        with open(out, "wb") as fh:
            fh.write(proj.build(slot, log=lambda s: say("  " + s)))
    if args.template:
        shutil.copy2(os.path.join(args.template, "Script", "Script.xml"), script)
        patch_dir = os.path.join(args.template, "Patch")
        if os.path.isdir(patch_dir):
            shutil.copytree(patch_dir, os.path.join(args.output, "Patch"), dirs_exist_ok=True)
    say(f"plugin folder: {args.output}")
    return 0


def cmd_elf(args) -> int:
    proj = Project.load(args.dir)
    segs, syms = [], []
    for i, plain in enumerate(proj.regions):
        base = proj.base(i)
        segs.append((f".region{i}", base, plain))
        syms.append((f"region{i}_syninfo", base, False))
        s = I.SynInfo.parse(plain)
        if s:
            syms.append((f"region{i}_init", s.init, True))
    s1 = I.SynInfo.parse(proj.regions[-1])
    with open(args.output, "wb") as fh:
        fh.write(I.to_elf(segs, syms, s1.init if s1 else 0))
    say(f"wrote {args.output}: load as ARM little endian (Thumb-2), symbols region<n>_init")
    return 0


def cmd_verify(args) -> int:
    """Unpack and rebuild each container; the result must be byte-identical."""
    bad = 0
    for path in args.dat:
        with open(path, "rb") as fh:
            original = fh.read()
        proj = Project.from_container(P.load(path), path)
        ok = proj.build() == original
        bad += not ok
        say(f"{'ok  ' if ok else 'FAIL'} {path}")
    return 1 if bad else 0


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("unpack", help="Code*.Dat -> project directory")
    p.add_argument("dat")
    p.add_argument("dir")
    p.add_argument("--relocs-from", metavar="DAT",
                   help="the same PLUG-OUT built for another slot (derives relocs.json)")
    p.set_defaults(func=cmd_unpack)

    p = sub.add_parser("relocs", help="derive relocs.json from another slot's build")
    p.add_argument("dir")
    p.add_argument("other", metavar="DAT")
    p.set_defaults(func=cmd_relocs)

    p = sub.add_parser("show", help="describe a project directory or a Code*.Dat")
    p.add_argument("target")
    p.set_defaults(func=cmd_show)

    p = sub.add_parser("set-id", help="change name / model id / version")
    p.add_argument("dir")
    p.add_argument("--name", help="up to 15 ASCII characters")
    p.add_argument("--model-id", type=int_auto)
    p.add_argument("--version", type=version_arg, help="e.g. 1.02")
    p.set_defaults(func=cmd_set_id)

    p = sub.add_parser("patch", help="write bytes into a region at a run address")
    p.add_argument("dir")
    p.add_argument("--region", type=int, choices=[0, 1], required=True)
    p.add_argument("--at", type=int_auto, required=True, help="address in the project's slot")
    g = p.add_mutually_exclusive_group(required=True)
    g.add_argument("--hex")
    g.add_argument("--file")
    p.add_argument("--reloc", type=int_auto, action="append",
                   help="address of a new absolute pointer word (repeatable)")
    p.set_defaults(func=cmd_patch)

    p = sub.add_parser("pack", help="project directory -> Code*.Dat")
    p.add_argument("dir")
    p.add_argument("-o", "--output", required=True)
    p.add_argument("--slot", type=int, help="build for this slot (default: the project's)")
    p.set_defaults(func=cmd_pack)

    p = sub.add_parser("plugin", help="build a plugin folder for plugout_tool.py install")
    p.add_argument("dir")
    p.add_argument("-o", "--output", required=True)
    p.add_argument("--template", help="plugin folder to take Script.xml and Patch/ from")
    p.add_argument("--slots", type=int, nargs="+", default=[1, 2, 3])
    p.set_defaults(func=cmd_plugin)

    p = sub.add_parser("elf", help="export the regions as an ELF for disassemblers")
    p.add_argument("dir")
    p.add_argument("-o", "--output", required=True)
    p.set_defaults(func=cmd_elf)

    p = sub.add_parser("verify", help="check that unpack + pack reproduces each file")
    p.add_argument("dat", nargs="+")
    p.set_defaults(func=cmd_verify)

    args = ap.parse_args(argv)
    try:
        return args.func(args)
    except (ValueError, OSError) as e:
        print(f"error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
