#!/usr/bin/env python3
"""
make_min.py -- build JX-3P-MEM-MIN, a deliberately incomplete PLUG-OUT that
holds only what the memory dump needs.

    region 0 = SYNINFO + a ~100-byte init hook (min_hook0.s)   -> 1 block
    region 1 = SYNINFO + a stub engine init       (min_hook1.s) -> 1 block

Everything else of the stock JX-3P is gone: no panel tables, no DSP code. The
other 4094 records of the container are filler records (a few bytes on the
wire), so the install is shorter than a full PLUG-OUT. Region 1 still starts at
record 2048: the flash layout is fixed.

    python3 examples/mem_name/make_min.py [--src 0x60000000] [-o plugins/JX-3P-MEM-MIN]
    python3 tools/plugout_tool.py install plugins/JX-3P-MEM-MIN --slot 3 --dry-run

--keep-engine keeps the stock engine (region 1) to tell a bad region 0 from a
bad stub engine when the device hangs.

EXPERIMENT: whether the OS accepts an engine without code is not known. The
install should work (the device only flashes the blocks); selecting the slot
may hang or reset the synth. The official updater restores slots 0-3.
"""
import argparse
import os
import shutil
import struct
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import plugout_build as B   # noqa: E402
import plugout_image as I   # noqa: E402

CODE_OFF = 0x40             # code follows the SYNINFO block (0x30) at this image offset


def assemble(src_file: str, base: int, work: str, **sym) -> bytes:
    stem = os.path.join(work, os.path.basename(src_file))
    cmd = ["arm-none-eabi-as", "-mcpu=cortex-a9", "-mthumb"]
    for k, v in sym.items():
        cmd += ["--defsym", f"{k}={v:#x}"]
    subprocess.run(cmd + [src_file, "-o", stem + ".o"], check=True)
    subprocess.run(["arm-none-eabi-ld", f"-Ttext={base + CODE_OFF:#x}", "-e", "0",
                    stem + ".o", "-o", stem + ".elf"], check=True)
    subprocess.run(["arm-none-eabi-objcopy", "-O", "binary", "-j", ".text",
                    stem + ".elf", stem + ".bin"], check=True)
    return open(stem + ".bin", "rb").read()


def image(name: str, model_id: int, version: int, base: int, code: bytes,
          size: int = 0, ptr: int = 0) -> bytes:
    """SYNINFO block (docs/CUSTOM_PLUGOUTS.md section 4) + code, linked at base.
    size/ptr: zero-pad the image to `size` bytes and use `ptr` (image offset)
    as the SYNINFO pointer, like the stock region 0."""
    end = base + (ptr or CODE_OFF + len(code))
    syn = bytearray(I.SYNINFO_MAGIC)
    syn += I.encode_name(name)
    syn += struct.pack("<HHII", model_id, version, base + CODE_OFF + 1, end)
    syn += I.SYNINFO_MAGIC
    return (bytes(syn).ljust(CODE_OFF, b"\0") + code).ljust(size, b"\0")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--src", type=lambda x: int(x, 0), default=0x60000000,
                    help="address read at install (default 0x60000000)")
    ap.add_argument("-o", "--out", default=os.path.join(ROOT, "plugins", "JX-3P-MEM-MIN"))
    ap.add_argument("--name", default="JX-3P MEM MIN")
    ap.add_argument("--keep-engine", action="store_true",
                    help="keep the stock JX-3P region 1 (engine); only region 0 is minimal")
    ap.add_argument("--pad-region0", action="store_true",
                    help="zero-pad region 0 to the stock size and use the stock SYNINFO "
                         "pointer (tests whether the OS needs that range)")
    ap.add_argument("--slots", type=int, nargs="+", default=[1, 2, 3])
    args = ap.parse_args()

    plug = os.path.join(ROOT, "plugins", "JX-3P")
    with tempfile.TemporaryDirectory() as work:
        subprocess.run([sys.executable, os.path.join(ROOT, "tools", "plugout_build.py"),
                        "unpack", os.path.join(plug, "Script", "Code8_1.Dat"),
                        os.path.join(work, "p"), "--relocs-from",
                        os.path.join(plug, "Script", "Code8_2.Dat")],
                       check=True, capture_output=True)
        proj = B.Project.load(os.path.join(work, "p"))
        b0, b1 = proj.base(0, 1), proj.base(1, 1)
        code0 = assemble(os.path.join(HERE, "min_hook0.s"), b0, work,
                         SRC=args.src, CAVE_OFF=CODE_OFF)
        code1 = assemble(os.path.join(HERE, "min_hook1.s"), b1, work)
        r1, rel1 = ((proj.regions[1], proj.relocs[1]) if args.keep_engine
                    else (image("SYSTEM-8", 0x000A, 0x0001, b1, code1), [0x24, 0x28]))
        stock0 = proj.regions[0]
        pad = ({"size": len(stock0), "ptr": I.SynInfo.parse(stock0).ptr - b0}
               if args.pad_region0 else {})
        proj.regions = [image(args.name, 0x0008, B.version_arg("1.06"), b0, code0, **pad), r1]
        proj.relocs = {0: [0x24, 0x28], 1: rel1}
        proj.meta["name"], proj.meta["version"] = args.name, "1.06"
        for i, r in enumerate(proj.regions):
            print(f"region {i}: {len(r)} bytes at {proj.base(i, 1):#x} (code {len(r) - CODE_OFF})")

        script = os.path.join(args.out, "Script")
        os.makedirs(script, exist_ok=True)
        for slot in args.slots:
            with open(os.path.join(script, f"Code8_{slot}.Dat"), "wb") as fh:
                fh.write(proj.build(slot))
        tpl = os.path.join(ROOT, "plugins", "JX-3P-MEM")
        shutil.copy2(os.path.join(tpl, "Script", "Script.xml"), script)
        shutil.copytree(os.path.join(tpl, "Patch"), os.path.join(args.out, "Patch"),
                        dirs_exist_ok=True)
    print(f"plugin folder: {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
