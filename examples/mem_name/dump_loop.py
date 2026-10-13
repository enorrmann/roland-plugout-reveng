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

Cost: ~8 s to build + the transfer + the info query per 7 bytes. A 2.8 MB OS
image is ~400 000 iterations: use --start/--count for the areas you need.
"""
from __future__ import annotations

import argparse
import os
import re
import signal
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import plugout as P            # noqa: E402
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


def build_dat(proj: str, work: str, src: int, slot: int) -> str:
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
                s.install(c, args.slot, progress=None)
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
