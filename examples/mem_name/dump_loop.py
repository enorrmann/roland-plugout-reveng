#!/usr/bin/env python3
"""
dump_loop.py -- dump SYSTEM-8 memory 7 bytes at a time with the verified
JX-3P-MEM init hook (hook.s), fully automatically.

One iteration:

  1. assemble hook.s with SRC = current address   (hook.s, as in build.sh)
  2. patch it into the stock JX-3P and pack it for --slot
  3. install it in the slot                       (plugout_tool.py install)
  4. read the slot name back                      (plugout_tool.py info)
  5. the name is 14 hex digits = the 7 bytes at SRC: validate and append them
     to the output file; only then go on with SRC + 7.

Nothing is sent to the device until you type "yes" (or pass --yes). The loop
stops on the first anomaly (install error, name that is not 14 hex digits,
model id changed) and can be resumed: if OUT exists, it continues after the
bytes already in it. Ctrl-C finishes the current iteration, then stops.

    python3 examples/mem_name/dump_loop.py --slot 3 --count 70 -o /tmp/os.bin
    python3 examples/mem_name/dump_loop.py --slot 3 --count 70 --dry-run

Cost: ~1 s to build (after a one-off ~8 s) + the transfer + the info query per 7 bytes. A 2.8 MB OS
image is ~400 000 iterations: use --start/--count for the areas you need.
"""
from __future__ import annotations

import argparse
import os
import re
import signal
import struct
import subprocess
import sys
import tempfile
import time
from dataclasses import replace

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import lzss                    # noqa: E402
import plugout as P            # noqa: E402
import plugout_build as B      # noqa: E402
import plugout_image as I      # noqa: E402
import plugout_tool as T       # noqa: E402

BUILD = [sys.executable, os.path.join(ROOT, "tools", "plugout_build.py")]
STEP = 7                       # bytes per install: 14 hex digits in a 15-char name
BASE, CAVE_OFF, INIT_OFF = 0x61800000, 0x100, 0x11823C   # as in build.sh
HEX14 = re.compile(r"^[0-9A-F]{14}$")
stop = False


def run(cmd: list[str]) -> None:
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode:
        sys.exit(f"command failed: {' '.join(cmd)}\n{r.stdout}{r.stderr}")


def assemble(src: int, work: str) -> str:
    """hook.s with SRC=src -> hook.bin in work (same commands as build.sh)."""
    o, elf, out = (os.path.join(work, n) for n in ("hook.o", "hook.elf", "hook.bin"))
    run(["arm-none-eabi-as", "-mcpu=cortex-a9", "-mthumb", "--defsym", f"SRC={src:#x}",
         "--defsym", f"CAVE_OFF={CAVE_OFF:#x}", "--defsym", f"INIT_OFF={INIT_OFF:#x}",
         os.path.join(HERE, "hook.s"), "-o", o])
    run(["arm-none-eabi-ld", f"-Ttext={BASE + CAVE_OFF:#x}", "--entry=hook", o, "-o", elf])
    run(["arm-none-eabi-objcopy", "-O", "binary", "-j", ".text", elf, out])
    return out


def prepare_project(work: str) -> str:
    """Stock JX-3P, name and version of the verified JX-3P-MEM, region 0 init pointer -> the hook."""
    proj = os.path.join(work, "proj")
    plug = os.path.join(ROOT, "plugins", "JX-3P", "Script")
    run(BUILD + ["unpack", os.path.join(plug, "Code8_1.Dat"), proj,
                 "--relocs-from", os.path.join(plug, "Code8_2.Dat")])
    run(BUILD + ["set-id", proj, "--name", "JX-3P MEM",
                 "--version", "1.06"])
    run(BUILD + ["patch", proj, "--region", "0", "--at", f"{BASE + 0x24:#x}",
                 "--hex", (BASE + CAVE_OFF + 1).to_bytes(4, "little").hex(),
                 "--reloc", f"{BASE + 0x24:#x}"])
    return proj


SENTINEL = 0xA1B2C3D4          # not a modified immediate: forced into the literal pool


class FastBuild:
    """Builds Code8_<slot>.Dat for any SRC without recompressing.

    The LZSS encoder (pure Python) takes ~7 s. Only the 4-byte literal SRC of
    the hook changes between iterations, and a unique sentinel value is never
    matched by a back-reference, so it is stored as 4 literal bytes in the
    compressed stream. The stream is encoded once with the sentinel; each
    iteration patches those 4 bytes (+ the byte sum in the region header).
    """

    def __init__(self, proj_dir: str, work: str, slot: int):
        run(BUILD + ["patch", proj_dir, "--region", "0", "--at", f"{BASE + CAVE_OFF:#x}",
                     "--file", assemble(SENTINEL, work)])
        proj = B.Project.load(proj_dir)
        self.slot, self.meta = slot, proj.meta
        self.name, self.mid = proj.meta["name"], proj.meta["model_id"]
        self.version = B.version_arg(proj.meta["version"])
        images = proj.images_for_slot(slot)
        plain0 = I.stamp_identity(images[0], self.name, self.mid, self.version)
        self.hdrs = []
        for rm in proj.meta["regions"]:
            self.hdrs.append(I.RegionHeader(
                magic=rm["magic"].encode("latin-1"), stamp=rm["stamp"].encode("latin-1"),
                tool=rm["tool"].encode("latin-1"), load=int(rm["load"], 0),
                entry=int(rm["entry"], 0)))
        self.raw1 = I.Region(self.hdrs[1], images[1]).to_raw()
        self.comp0 = lzss.encode(plain0)
        self.ulen0 = len(plain0)
        pat = struct.pack("<I", SENTINEL)
        at = plain0.find(pat)
        if at < 0 or plain0.find(pat, at + 1) >= 0:
            raise SystemExit("fast build: sentinel not unique in region 0")
        self.offs = self._literal_offsets(at, 4)
        if self.offs is None:
            raise SystemExit("fast build: SRC literal is not stored as literals")
        check = self.compressed0(0x60000000)
        want = bytearray(plain0)
        want[at:at + 4] = struct.pack("<I", 0x60000000)
        if lzss.decode(check, self.ulen0) != bytes(want):
            raise SystemExit("fast build: self-check failed")

    def _literal_offsets(self, at: int, n: int):
        """Compressed-stream offsets of output bytes at..at+n-1, if all literals."""
        src, i, out, found = self.comp0, 0, 0, {}
        while i < len(src) and len(found) < n:
            flags = src[i]
            i += 1
            for bit in range(8):
                if i >= len(src):
                    break
                if flags >> bit & 1:
                    if at <= out < at + n:
                        found[out - at] = i
                    i += 1
                    out += 1
                else:
                    ln = (src[i + 1] & 0x0F) + lzss.THRESHOLD + 1
                    if out < at + n and out + ln > at:
                        return None
                    i += 2
                    out += ln
        return [found[k] for k in range(n)] if len(found) == n else None

    def compressed0(self, src: int) -> bytes:
        comp = bytearray(self.comp0)
        for o, b in zip(self.offs, struct.pack("<I", src)):
            comp[o] = b
        return bytes(comp)

    def build(self, src: int) -> bytes:
        comp = self.compressed0(src)
        h = replace(self.hdrs[0], clen=len(comp), csum=sum(comp) & 0xFFFFFFFF, ulen=self.ulen0)
        raw0 = h.pack() + comp
        raw0 += b"\xff" * (-len(raw0) % I.BLOCK_SIZE)
        blocks = I.blocks_from_regions([raw0, self.raw1], self.meta["records"])
        return I.build_container(self.name, self.mid, self.version, blocks)


_fast: FastBuild | None = None


def build_dat(proj: str, work: str, src: int, slot: int) -> str:
    """Code8_<slot>.Dat reading `src` (fast path; see FastBuild)."""
    global _fast
    dat = os.path.join(work, f"Code8_{slot}.Dat")
    if _fast is None:
        print("compressing the base image once ...", flush=True)
        _fast = FastBuild(proj, work, slot)
    with open(dat, "wb") as fh:
        fh.write(_fast.build(src))
    return dat


def build_dat_slow(proj: str, work: str, src: int, slot: int) -> str:
    hook = assemble(src, work)
    run(BUILD + ["patch", proj, "--region", "0", "--at", f"{BASE + CAVE_OFF:#x}",
                 "--file", hook])
    dat = os.path.join(work, f"Code8_{slot}.Dat")
    run(BUILD + ["pack", proj, "-o", dat, "--slot", str(slot)])
    return dat


def slot_name(s: T.Session, slot: int, model_id: int) -> str:
    info = s.device_info()
    sl = info["slots"][slot]
    if sl.model_id != model_id:
        raise T.ProtocolError(f"slot {slot} reports model id {sl.model_id:#06x}, "
                              f"expected {model_id:#06x}")
    return sl.name


def read_back(s: T.Session, slot: int, model_id: int, tries: int, wait: float) -> str:
    """Slot name after the install, retried while the device finishes."""
    name = ""
    for i in range(tries):
        time.sleep(wait)
        try:
            name = slot_name(s, slot, model_id)
        except (T.ProtocolError, TimeoutError) as e:
            print(f"    info attempt {i + 1}/{tries}: {e}")
            continue
        if HEX14.match(name):
            return name
        print(f"    info attempt {i + 1}/{tries}: name {name!r} is not 14 hex digits yet")
    return name


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--slot", type=int, default=3, help="PLUG-OUT slot (default 3)")
    ap.add_argument("--start", type=lambda x: int(x, 0), default=0x60000000,
                    help="first address (default 0x60000000, the OS image)")
    ap.add_argument("--count", type=lambda x: int(x, 0), required=True,
                    help="bytes to read (rounded up to a multiple of 7)")
    ap.add_argument("-o", "--out", default="mem_dump.bin")
    ap.add_argument("--no-resume", action="store_true", help="start over, overwrite OUT")
    ap.add_argument("--read-tries", type=int, default=5)
    ap.add_argument("--read-wait", type=float, default=2.0, help="seconds before each info query")
    ap.add_argument("-y", "--yes", action="store_true")
    ap.add_argument("--dry-run", action="store_true", help="build the first file only, no MIDI")
    ap.add_argument("--backend", choices=["alsa", "rtmidi"])
    ap.add_argument("--port")
    ap.add_argument("--device-id", type=lambda x: int(x, 0), default=P.DEFAULT_DEVICE_ID)
    ap.add_argument("--gap-ms", type=float, default=0.0)
    ap.add_argument("--wire-pacing", action="store_true")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    if args.slot < 1:
        sys.exit("slot 0 is the synth itself")

    total = -(-args.count // STEP) * STEP
    done = 0
    if os.path.exists(args.out) and not args.no_resume:
        done = os.path.getsize(args.out) // STEP * STEP
        if done:
            print(f"resuming: {done} bytes already in {args.out}")
    if done >= total:
        print("nothing to do")
        return 0
    iters = (total - done) // STEP

    with tempfile.TemporaryDirectory() as work:
        print(f"building the base project ...")
        proj = prepare_project(work)

        if args.dry_run:
            dat = build_dat(proj, work, args.start + done, args.slot)
            print(P.load(dat).summary())
            print(f"dry run: would read {args.start + done:#x}..{args.start + total:#x} "
                  f"in {iters} installs of slot {args.slot}")
            return 0

        # compress the base image and build the first file BEFORE touching MIDI:
        # the one-off compression takes ~8 s, long enough for the device to
        # time out an open session
        print("building the first file ...", flush=True)
        build_dat(proj, work, args.start + done, args.slot)

        s, port = T.open_session(args)
        try:
            info = s.device_info()
            T.print_info(info)
            if args.slot >= len(info["slots"]):
                sys.exit(f"slot must be 1..{len(info['slots']) - 1}")
            if info["slots"][args.slot].index == 0:
                sys.exit("refusing slot 0")
            print(f"\nslot {args.slot} ({info['slots'][args.slot].name or 'empty'}) will be "
                  f"OVERWRITTEN {iters} times, reading {args.start + done:#x}..{args.start + total:#x}")
            print("do not disconnect or power off the device during the transfers")
            if not args.yes and input("type 'yes' to start: ").strip().lower() != "yes":
                print("aborted")
                return 1

            def on_sigint(*_):
                global stop
                stop = True
                print("\nstopping after the current iteration ...")
            signal.signal(signal.SIGINT, on_sigint)

            prev = info["slots"][args.slot].name
            phase = {"build": 0.0, "install": 0.0, "readback": 0.0}
            t0 = time.time()
            for n in range(iters):
                if stop:
                    break
                src = args.start + done
                print(f"[{n + 1}/{iters}] {src:#010x}: build + install ...", flush=True)
                t_a = time.time()
                dat = build_dat(proj, work, src, args.slot)
                c = P.load(dat)
                t_b = time.time()
                t_s = time.time()
                seen = []

                def first_record(i, total, t_s=t_s, seen=seen):
                    if not seen:
                        seen.append(1)
                        print(f"    START acknowledged after {time.time() - t_s:.1f} s, "
                              f"sending {total} records ...", flush=True)
                print(f"    file ready ({t_b - t_a:.1f} s build); sending START ...", flush=True)
                s.install(c, args.slot, progress=first_record)
                t_c = time.time()
                name = read_back(s, args.slot, c.model_id, args.read_tries, args.read_wait)
                t_d = time.time()
                for k, v in zip(("build", "install", "readback"),
                                (t_b - t_a, t_c - t_b, t_d - t_c)):
                    phase[k] += v
                print(f"    phases: build {t_b - t_a:.1f} s, install {t_c - t_b:.1f} s, "
                      f"readback {t_d - t_c:.1f} s (of which wait {args.read_wait:.1f} s)")
                if not HEX14.match(name):
                    sys.exit(f"stop: slot name {name!r} is not 14 hex digits; the hook did not "
                             f"run, or {src:#x} is unreadable. {done} bytes are in {args.out}")
                if name == prev:
                    print("    note: same 14 digits as the previous read (identical data)")
                data = bytes.fromhex(name)
                with open(args.out, "ab" if done else "wb") as fh:
                    fh.write(data)
                prev = name
                done += STEP
                el = time.time() - t0
                print(f"    {src:#010x}: {name}   ({el / (n + 1):.0f} s/iteration, "
                      f"{(iters - n - 1) * el / (n + 1) / 60:.0f} min left)")
            tot = sum(phase.values()) or 1.0
            print("time per phase: " + ", ".join(
                f"{k} {v:.0f} s ({100 * v / tot:.0f} %)" for k, v in phase.items()))
            print(f"done: {done} bytes in {args.out}" + (" (interrupted)" if stop else ""))
        except T.ProtocolError as e:
            print(f"\nERROR: {e}", file=sys.stderr)
            return 2
        finally:
            port.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
