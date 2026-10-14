#!/usr/bin/env python3
"""
dump_loop.py (mem_names) -- dump SYSTEM-8 memory 24 bytes at a time through the
names of the three PLUG-OUT slots (hook.s of this directory).

Each install of the hook writes 8 bytes of OS memory as 16 hex digits over each
slot name: slot 1 = SRC.., slot 2 = SRC+8.., slot 3 = SRC+16.. . The names are
read back with the device info query. It reuses the build / install machinery
of examples/mem_name/dump_loop.py (fast sentinel build, same JX-3P-MEM base).

Stops on the first anomaly (a name that is not 16 hex digits, slot 3 model id
changed, install error); in particular if slots 1/2 do not change, their names
are not read from the RAM the hook writes. Resumable (OUT is appended).

    python3 examples/mem_names/dump_loop.py --slot 3 --count 240 -o /tmp/os.bin
    python3 examples/mem_names/dump_loop.py --slot 3 --count 240 --dry-run
"""
from __future__ import annotations

import argparse
import importlib.util
import os
import re
import signal
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location(
    "mem_dump_loop", os.path.join(HERE, "..", "mem_name", "dump_loop.py"))
D = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(D)
D.HERE = HERE                  # assemble() takes hook.s from this directory
P, T = D.P, D.T

PHASES = ("build", "start", "data", "filler", "end", "readback")
STEP = 24                      # 3 slots x 8 bytes
HEX16 = re.compile(r"^[0-9A-F]{16}$")
stop = False


def read_names(s, install_slot: int, model_id: int, tries: int, wait: float) -> list[str]:
    """Names of slots 1..3, retried until all are 16 hex digits (or tries run out)."""
    names: list[str] = []
    for i in range(tries):
        time.sleep(wait)
        try:
            sl = s.device_info()["slots"]
            if sl[install_slot].model_id != model_id:
                raise T.ProtocolError(f"slot {install_slot} model id "
                                      f"{sl[install_slot].model_id:#06x}, expected {model_id:#06x}")
            names = [sl[k].name for k in (1, 2, 3)]
        except (T.ProtocolError, TimeoutError) as e:
            print(f"    info attempt {i + 1}/{tries}: {e}")
            continue
        if all(HEX16.match(n) for n in names):
            return names
        print(f"    info attempt {i + 1}/{tries}: names {names!r} not all 16 hex digits yet")
    return names


def records_to_send(c, skip: str) -> list[int]:
    """Record indices to send. Data records carry no index: the device counts
    them in order, so only filler after the last data record can be dropped
    without moving region 1 ('tail'); 'all' drops every filler (experiment,
    moves region 1 to the block right after region 0)."""
    is_data = [len(c.data_message(i)) > 100 for i in range(c.data_count)]
    if skip == "none":
        return list(range(c.data_count))
    last = max(i for i, d in enumerate(is_data) if d)
    if skip == "tail":
        return list(range(last + 1))
    return [i for i, d in enumerate(is_data) if d]


def install_records(s, c, slot: int, idx: list[int], progress, tolerate_end: bool = False) -> int:
    """T.Session.install, sending only the records in `idx` (flow control
    keyed on the original record number, END as usual)."""
    start = P.with_data(c.start_message, bytes([slot & 0x0F]))
    start = P.with_device_id(start, s.device_id)
    code = s.request(start, P.ADDR_START | P.REPLY_BIT, 1)[0]
    if code != 0:
        raise T.ProtocolError(f"device refused the transfer (start reply {code:#04x})")
    for n, i in enumerate(idx):
        if i % P.FLOW_EVERY == P.FLOW_EVERY - 1:
            s.write(P.ADDR_FLOW, b"\0", 4)
        s.send(P.with_device_id(c.data_message(i), s.device_id))
        s.check_data_error(i)
        progress(n + 1, len(idx))
    end = P.with_device_id(c.end_message, s.device_id)
    code = s.request(end, P.ADDR_END | P.REPLY_BIT, 1, timeout=P.END_TIMEOUT)[0]
    if code != 0 and not tolerate_end:
        raise T.ProtocolError(f"device reported an error at the end (reply {code:#04x})")
    return code


def install_phases(c, t_begin: float, stamps: list[float], t_end: float,
                   idx: list[int] | None = None) -> dict:
    """Split one install into: start (START reply + first record), data records,
    filler records, end (END message and its reply). `stamps[i]` is the time
    record i was sent (and its error check done). A data record is one that
    carries a block (the 14-byte filler records are the unused flash blocks)."""
    idx = list(range(c.data_count)) if idx is None else idx
    is_data = [len(c.data_message(i)) > 100 for i in idx]
    out = {"start": 0.0, "data": 0.0, "filler": 0.0, "end": 0.0,
           "n_data": float(sum(is_data)), "n_fill": float(len(is_data) - sum(is_data))}
    if len(stamps) != len(is_data):
        return out
    out["start"] = stamps[0] - t_begin
    for i in range(1, len(stamps)):
        out["data" if is_data[i] else "filler"] += stamps[i] - stamps[i - 1]
    out["end"] = t_end - stamps[-1]
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--slot", type=int, default=3, help="slot to install the hook in (default 3)")
    ap.add_argument("--start", type=lambda x: int(x, 0), default=0x60000000)
    ap.add_argument("--count", type=lambda x: int(x, 0), required=True,
                    help="bytes to read (rounded up to a multiple of 24)")
    ap.add_argument("-o", "--out", default="mem_dump.bin")
    ap.add_argument("--no-resume", action="store_true")
    ap.add_argument("--skip-filler", choices=["none", "tail", "all"], default="none",
                    help="EXPERIMENT: do not send filler records: 'tail' = those after the last "
                         "data record (END right after region 1), 'all' = every one (region 1 "
                         "shifts). Default none (full container)")
    ap.add_argument("--read-tries", type=int, default=5)
    ap.add_argument("--read-wait", type=float, default=2.0)
    ap.add_argument("-y", "--yes", action="store_true")
    ap.add_argument("--dry-run", action="store_true")
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
        print("building the base project ...")
        proj = D.prepare_project(work)
        if args.dry_run:
            dat = D.build_dat(proj, work, args.start + done, args.slot)
            print(P.load(dat).summary())
            print(f"dry run: would read {args.start + done:#x}..{args.start + total:#x} "
                  f"in {iters} installs of slot {args.slot}")
            return 0
        print("building the first file ...", flush=True)
        D.build_dat(proj, work, args.start + done, args.slot)

        s, port = T.open_session(args)
        try:
            info = s.device_info()
            T.print_info(info)
            if args.slot >= len(info["slots"]):
                sys.exit(f"slot must be 1..{len(info['slots']) - 1}")
            print(f"\nslot {args.slot} will be OVERWRITTEN {iters} times and the names of "
                  f"slots 1, 2 and 3 are rewritten in RAM, reading "
                  f"{args.start + done:#x}..{args.start + total:#x}")
            print("do not disconnect or power off the device during the transfers")
            if not args.yes and input("type 'yes' to start: ").strip().lower() != "yes":
                print("aborted")
                return 1

            def on_sigint(*_):
                global stop
                stop = True
                print("\nstopping after the current iteration ...")
            signal.signal(signal.SIGINT, on_sigint)

            total_ph: dict[str, float] = {}
            t0 = time.time()
            for n in range(iters):
                if stop:
                    break
                src = args.start + done
                print(f"[{n + 1}/{iters}] {src:#010x}: build + install ...", flush=True)
                t_a = time.time()
                dat = D.build_dat(proj, work, src, args.slot)
                c = P.load(dat)
                t_b = time.time()
                # per-record timestamps (progress is called after each record + its error check)
                stamps: list[float] = []
                idx = records_to_send(c, args.skip_filler)
                end_code = install_records(s, c, args.slot, idx,
                                           lambda i, total: stamps.append(time.time()),
                                           tolerate_end=args.skip_filler != "none")
                if end_code:
                    print(f"    END reply {end_code:#04x}: reading the names anyway "
                          f"(--skip-filler {args.skip_filler})")
                t_c = time.time()
                names = read_names(s, args.slot, c.model_id, args.read_tries, args.read_wait)
                t_d = time.time()
                ph = install_phases(c, t_b, stamps, t_c, idx)
                ph["build"], ph["readback"] = t_b - t_a, t_d - t_c
                for k, v in ph.items():
                    total_ph[k] = total_ph.get(k, 0.0) + v
                print("    phases: " + ", ".join(f"{k} {ph[k]:.1f} s" for k in PHASES)
                      + f"  [{ph['n_data']:.0f} data / {ph['n_fill']:.0f} filler records]")
                if not all(HEX16.match(x) for x in names):
                    sys.exit(f"stop: slot names {names!r} are not 16 hex digits each; the hook "
                             f"did not run, slots 1/2 are not displayed from the written RAM, "
                             f"or {src:#x}..{src + STEP:#x} is unreadable. "
                             f"{done} bytes are in {args.out}")
                data = b"".join(bytes.fromhex(x) for x in names)
                with open(args.out, "ab" if done else "wb") as fh:
                    fh.write(data)
                done += STEP
                el = time.time() - t0
                print(f"    {src:#010x}: {' '.join(names)}   ({el / (n + 1):.0f} s/iteration, "
                      f"{(iters - n - 1) * el / (n + 1) / 60:.0f} min left)")
            keys = PHASES
            tot = sum(total_ph.get(k, 0.0) for k in keys) or 1.0
            if total_ph:
                print("time per phase: " + ", ".join(
                    f"{k} {total_ph.get(k, 0.0):.0f} s ({100 * total_ph.get(k, 0.0) / tot:.0f} %)"
                    for k in keys))
                nd, nf = total_ph.get("n_data", 0.0), total_ph.get("n_fill", 0.0)
                if nd and nf:
                    print(f"per record: data {1000 * total_ph['data'] / nd:.1f} ms, "
                          f"filler {1000 * total_ph['filler'] / nf:.2f} ms")
            print(f"done: {done} bytes in {args.out}" + (" (interrupted)" if stop else ""))
        except T.ProtocolError as e:
            print(f"\nERROR: {e}", file=sys.stderr)
            return 2
        finally:
            port.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
