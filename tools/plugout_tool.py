#!/usr/bin/env python3
"""
plugout_tool.py -- install a Roland PLUG-OUT (JUNO-60, JX-3P, SH-101) into a
SYSTEM-8 / SYSTEM-1 over USB MIDI, without the VST plugin.

    python3 tools/plugout_tool.py ports
    python3 tools/plugout_tool.py info
    python3 tools/plugout_tool.py install JUNO-60 --slot 1
    python3 tools/plugout_tool.py inspect JUNO-60/Script/Code8_1.Dat

Replicates the plugin's install routine (JUNO-60.vst3 @ 0x1803547c0); see
PROTOCOL.md for the reverse-engineered details.
"""

from __future__ import annotations

import argparse
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import midi_io  # noqa: E402
import patches as PT  # noqa: E402
import plugout as P  # noqa: E402


class ProtocolError(RuntimeError):
    pass


class Session:
    """Request/reply engine with the plugin's pacing and retry rules."""

    def __init__(self, port, device_id: int = P.DEFAULT_DEVICE_ID,
                 gap: float = 0.0, wire_pacing: bool = False, verbose: bool = False,
                 clock=time.monotonic, sleep=time.sleep):
        self.port = port
        self.device_id = device_id
        self.gap = gap
        self.wire_pacing = wire_pacing
        self.verbose = verbose
        self.clock, self.sleep = clock, sleep
        self._next_send = 0.0
        self._inbox: list[bytes] = []

    # -- transport ---------------------------------------------------------

    def send(self, msg: bytes) -> None:
        """Send one message. By default there is no extra delay: the device
        paces the firmware transfer with its flow-control replies and the port
        write blocks while the USB buffer is full. Optionally wait
        `gap (+ len/3125 s)` between messages, like the plugin's MIDI out
        (interval setting 10010 = 10 ms + wire time, @ 0x1804364c0).
        """
        wait = self._next_send - self.clock()
        if wait > 0:
            self.sleep(wait)
        self._next_send = self.clock() + self.gap + (
            P.wire_seconds(len(msg)) if self.wire_pacing else 0.0)
        if self.verbose:
            print(f"  -> {msg[:16].hex(' ')}{' ...' if len(msg) > 16 else ''}")
        self.port.send(msg)

    def _pump(self) -> None:
        for m in self.port.receive():
            if self.verbose:
                print(f"  <- {m.hex(' ')}")
            if P.is_plugout_sysex(m):
                self._inbox.append(m)

    def _take(self, address: int, size: int) -> bytes | None:
        for i, m in enumerate(self._inbox):
            if P.address_of(m) == address and len(P.sysex_data(m)) == size:
                del self._inbox[i]
                return P.sysex_data(m)
        return None

    def check_data_error(self, index: int) -> None:
        """The device reports a rejected data record on 71 30 00 00."""
        self._pump()
        err = self._take(P.ADDR_DATA_ERROR, 1)
        if err is not None:
            raise ProtocolError(f"device rejected data record {index} (code {err[0]:#04x})")

    def request(self, msg: bytes, reply_address: int, reply_size: int,
                timeout: float = P.START_TIMEOUT, retries: int = P.RETRIES) -> bytes:
        """Send `msg` and wait for a message on `reply_address` carrying
        `reply_size` data bytes; resend on timeout (@ 0x180356f00)."""
        self._pump()
        self._inbox = [m for m in self._inbox if P.address_of(m) == P.ADDR_DATA_ERROR]
        for _ in range(retries):
            self.send(msg)
            deadline = self.clock() + timeout
            while True:
                self._pump()
                data = self._take(reply_address, reply_size)
                if data is not None:
                    return data
                if self.clock() >= deadline:
                    break
                self.sleep(0.002)
        raise ProtocolError(f"no reply on {P.addr_str(reply_address)} "
                            f"after {retries} x {timeout:g} s")

    def write(self, address: int, data: bytes, reply_size: int, **kw) -> bytes:
        msg = P.build(P.DT1, address, data, self.device_id)
        return self.request(msg, address | P.REPLY_BIT, reply_size, **kw)

    # -- device queries (@ 0x180352b50) -------------------------------------

    def device_info(self) -> dict:
        info = {"type": self.write(P.ADDR_DEVICE_TYPE, b"\0", 1)[0]}
        info["firmware"] = P.nibbles(self.write(P.ADDR_SLOT_INFO, b"\0", 0x30)[0:4])
        banks = self.write(P.ADDR_INFO_101, b"\0", 1)[0]
        per_bank = self.write(P.ADDR_INFO_102, b"\0", 1)[0]
        info["patches"] = banks * per_bank
        count = self.write(P.ADDR_SLOT_COUNT, b"\0", 1)[0]
        info["slots"] = [P.parse_slot_info(i, self.write(P.ADDR_SLOT_INFO, bytes([i]), 0x30))
                         for i in range(count)]
        return info

    # -- install (@ 0x1803547c0) ----------------------------------------------

    def install(self, c: P.Container, slot: int, flags: int = 0, progress=None) -> None:
        start = c.start_message
        start = P.with_data(start, bytes([(flags & 0x70) | (slot & 0x0F)]))
        start = P.with_device_id(start, self.device_id)
        code = self.request(start, P.ADDR_START | P.REPLY_BIT, 1)[0]
        if code != 0:
            raise ProtocolError(f"device refused the transfer (start reply {code:#04x})")

        total = c.data_count
        for i in range(total):
            if i % P.FLOW_EVERY == P.FLOW_EVERY - 1:
                self.write(P.ADDR_FLOW, b"\0", 4)
            self.send(P.with_device_id(c.data_message(i), self.device_id))
            self.check_data_error(i)
            if progress:
                progress(i + 1, total)

        end = P.with_device_id(c.end_message, self.device_id)
        code = self.request(end, P.ADDR_END | P.REPLY_BIT, 1, timeout=P.END_TIMEOUT)[0]
        if code != 0:
            raise ProtocolError(f"device reported an error at the end (reply {code:#04x})")

    # -- patch bank (@ 0x180355183..0x1803552d0) --------------------------------

    def send_patches(self, msgs: list[bytes], progress=None) -> None:
        """Plain DT1 writes; the plugin waits for no reply here."""
        for i, m in enumerate(msgs):
            self.send(m)
            self.check_data_error(i)
            if progress:
                progress(i + 1, len(msgs))


# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------

def version_str(v: int) -> str:
    return f"{v >> 8:X}.{v & 0xFF:02X}"


def print_info(info: dict) -> None:
    t = info["type"]
    print(f"device   : {P.DEVICE_TYPES.get(t, f'unknown type {t}')}")
    print(f"firmware : {version_str(info['firmware'])}   patches per slot: {info['patches']}")
    for s in info["slots"]:
        print(f"  {'(base) ' if s.index == 0 else 'PLUG-OUT'} {s}")


def estimate_seconds(msgs: list[bytes], gap: float, wire: bool) -> float:
    return sum(gap + (P.wire_seconds(len(m)) if wire else 0) for m in msgs)


def progress_bar(done: int, total: int, t0=[None]) -> None:
    if t0[0] is None or done == 1:
        t0[0] = time.monotonic()
    el = time.monotonic() - t0[0]
    eta = el / done * (total - done)
    bar = "#" * (30 * done // total)
    end = "\n" if done == total else ""
    print(f"\r  [{bar:<30}] {done}/{total}  {el:5.0f}s  eta {eta:4.0f}s", end=end, flush=True)


def int_auto(s: str) -> int:
    return int(s, 0)


def open_session(args) -> tuple[Session, object]:
    backend = args.backend or midi_io.default_backend()
    name = args.port or midi_io.find_port(backend)
    print(f"port     : {name} ({backend})")
    port = midi_io.open_port(backend, name)
    s = Session(port, device_id=args.device_id, gap=args.gap_ms / 1000.0,
                wire_pacing=args.wire_pacing, verbose=args.verbose)
    return s, port


# ---------------------------------------------------------------------------
# commands
# ---------------------------------------------------------------------------

def cmd_ports(args) -> int:
    backend = args.backend or midi_io.default_backend()
    ports = midi_io.list_ports(backend)
    for name, desc in ports:
        print(f"{name:<12} {desc}")
    try:
        print(f"\nauto-selected: {midi_io.find_port(backend)}")
    except midi_io.MidiError as e:
        print(f"\n{e}")
    return 0


def cmd_inspect(args) -> int:
    c = P.load(args.dat)
    print(c.summary())
    for path in (args.image, args.syx):
        if path and os.path.dirname(path):
            os.makedirs(os.path.dirname(path), exist_ok=True)
    if args.image:
        for i, region in enumerate(c.image_regions()):
            path = f"{args.image}{i}.bin"
            with open(path, "wb") as fh:
                fh.write(region)
            print(f"wrote {path} ({len(region)} bytes)")
    if args.syx:
        msgs = P.host_stream(c, args.slot, args.flags, args.device_id)
        with open(args.syx, "wb") as fh:
            fh.write(b"".join(msgs))
        print(f"wrote {args.syx}: {len(msgs)} messages, slot {args.slot} "
              "(without flow-control queries; not suitable for blind replay)")
    return 0


def cmd_info(args) -> int:
    s, port = open_session(args)
    try:
        print_info(s.device_info())
    finally:
        port.close()
    return 0


def resolve_container(args, device_type: int) -> P.Container:
    if args.dat:
        return P.load(args.dat)
    if not args.plugin:
        raise SystemExit("give a plugin folder (JUNO-60, JX-3P, SH-101) or --dat")
    return P.load(P.container_path(args.plugin, device_type, args.slot))


def patch_plan(args, device_type: int, count: int, already: bool) -> tuple[str, list[bytes]]:
    if not args.plugin:
        raise SystemExit("the patch bank needs the plugin folder (or use --no-patches)")
    layout = PT.ScriptLayout(os.path.join(args.plugin, "Script", "Script.xml"))
    bank = args.bank or PT.default_bank(args.plugin)
    msgs = PT.patch_messages(layout, PT.load_bank(bank, layout), args.slot, count,
                             device_type, already, args.device_id)
    return bank, msgs


def cmd_install(args) -> int:
    if args.no_patches and args.patches_only:
        raise SystemExit("--no-patches and --patches-only exclude each other")
    if args.slot < 1:
        raise SystemExit("slot 0 is the synth itself; PLUG-OUT slots start at 1")
    gap, wire = args.gap_ms / 1000.0, args.wire_pacing
    if args.dry_run:
        dtype = 2 if args.device_type is None else args.device_type
        if not args.patches_only:
            c = resolve_container(args, dtype)
            print(c.summary())
            fw = P.host_stream(c, args.slot)
            pauses = estimate_seconds(fw, gap, wire)
            print(f"firmware: {len(fw)} messages, {sum(map(len, fw))} bytes, slot {args.slot}"
                  + (f", {pauses:.0f} s of pauses" if pauses else ""))
        if not args.no_patches:
            bank, msgs = patch_plan(args, dtype, 64, False)
            print(f"patches : {bank}: {len(msgs)} messages, {sum(map(len, msgs))} bytes")
        return 0

    s, port = open_session(args)
    try:
        state = None
        if args.skip_check:
            dtype = 2 if args.device_type is None else args.device_type
            info = {"patches": 64}
        else:
            info = s.device_info()
            print_info(info)
            dtype = info["type"]
            if dtype not in P.DEVICE_TYPES:
                raise SystemExit(f"unknown device type {dtype}")
            if info["firmware"] < P.MIN_FIRMWARE[dtype]:
                raise SystemExit(f"device firmware {version_str(info['firmware'])} is too old: "
                                 f"the plugin requires {version_str(P.MIN_FIRMWARE[dtype])}+")

        c = resolve_container(args, dtype)
        print()
        print(c.summary())

        do_firmware = not args.patches_only
        if "slots" in info:
            slots = info["slots"]
            if not 1 <= args.slot < len(slots):
                raise SystemExit(f"slot must be 1..{len(slots) - 1}")
            others = [x for x in slots if x.index != args.slot
                      and x.state_for(c) in (P.SLOT_SAME, P.SLOT_OTHER_VERSION)]
            if others and not args.force:
                raise SystemExit(f"{c.model} is already installed in slot {others[0].index} "
                                 "(the plugin refuses this; --force to continue)")
            state = slots[args.slot].state_for(c)
            if state == P.SLOT_SAME and not args.force:
                print(f"\n{c.model} {version_str(c.version)} is already in slot {args.slot}: "
                      "the firmware is skipped, as the plugin does (--force to resend it)")
                do_firmware = False
            if state == P.SLOT_OTHER_MODEL:
                print(f"\nslot {args.slot} holds {slots[args.slot].name}; it will be overwritten")
        if args.patches_only and state not in (None, P.SLOT_SAME, P.SLOT_OTHER_VERSION):
            raise SystemExit(f"slot {args.slot} does not hold {c.model}: install the firmware first")

        bank, pmsgs = (None, [])
        if not args.no_patches:
            bank, pmsgs = patch_plan(args, dtype, info["patches"], state == P.SLOT_SAME)
        if not do_firmware and not pmsgs:
            print("nothing to do")
            return 0

        print()
        if do_firmware:
            print(f"firmware: {c.model} {version_str(c.version)} -> slot {args.slot}")
        if pmsgs:
            print(f"patches : {os.path.basename(bank)} ({info['patches']} patches, "
                  f"{len(pmsgs)} messages) -> slot {args.slot}")
        print("do not disconnect or power off the device during the transfer")
        if not args.yes and input("type 'yes' to start: ").strip().lower() != "yes":
            print("aborted")
            return 1

        bar = None if args.verbose else progress_bar
        if do_firmware:
            print("firmware:")
            s.install(c, args.slot, flags=args.flags, progress=bar)
            print("done: the device accepted the PLUG-OUT")
        if pmsgs:
            print("patches:")
            s.send_patches(pmsgs, progress=bar)
            print("done: patches sent")
    except ProtocolError as e:
        print(f"\nERROR: {e}", file=sys.stderr)
        return 2
    finally:
        port.close()
    return 0


def main(argv=None) -> int:
    common = argparse.ArgumentParser(add_help=False)
    g = common.add_argument_group("MIDI")
    g.add_argument("--backend", choices=["alsa", "rtmidi"],
                   help="default: alsa on Linux, rtmidi elsewhere")
    g.add_argument("--port", help="MIDI port (default: auto-detect the SYSTEM-8 CTRL port)")
    g.add_argument("--device-id", type=int_auto, default=P.DEFAULT_DEVICE_ID,
                   help="SysEx device id (default 0x10 = 17)")
    g.add_argument("--gap-ms", type=float, default=0.0,
                   help="extra pause between messages (default 0; the plugin uses 10)")
    g.add_argument("--wire-pacing", action="store_true",
                   help="also wait the 31250-baud wire time of each message, as the plugin")
    g.add_argument("-v", "--verbose", action="store_true", help="print every message")

    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("ports", parents=[common], help="list MIDI ports")
    p.set_defaults(func=cmd_ports)

    p = sub.add_parser("info", parents=[common], help="query the device and its slots")
    p.set_defaults(func=cmd_info)

    p = sub.add_parser("install", parents=[common], help="install a PLUG-OUT")
    p.add_argument("plugin", nargs="?", help="plugin folder: JUNO-60, JX-3P or SH-101")
    p.add_argument("--slot", type=int, default=1, help="PLUG-OUT slot (default 1)")
    p.add_argument("--dat", help="use this Code*.Dat instead of picking one from the plugin")
    p.add_argument("--flags", type=int_auto, default=0,
                   help="bits 4-6 of the start byte (the plugin sends 0x40 when not authorized)")
    p.add_argument("--device-type", type=int, choices=[0, 1, 2],
                   help="with --skip-check/--dry-run: 0 SYSTEM-1, 1 SYSTEM-1m, 2 SYSTEM-8 (default)")
    p.add_argument("--skip-check", action="store_true", help="do not query the device first")
    p.add_argument("--force", action="store_true", help="install even if the plugin would refuse")
    p.add_argument("-y", "--yes", action="store_true", help="do not ask for confirmation")
    p.add_argument("--dry-run", action="store_true", help="show what would be sent, no MIDI")
    p.add_argument("--bank", help="patch bank to send (default: the first file in <plugin>/Patch)")
    p.add_argument("--no-patches", action="store_true", help="send only the firmware")
    p.add_argument("--patches-only", action="store_true", help="send only the patch bank")
    p.set_defaults(func=cmd_install)

    p = sub.add_parser("inspect", help="decode a Code*.Dat")
    p.add_argument("dat")
    p.add_argument("--image", metavar="PREFIX", help="write firmware regions to PREFIX<n>.bin")
    p.add_argument("--syx", help="write the host message stream to a .syx file")
    p.add_argument("--slot", type=int, default=1, help="slot written into the start message")
    p.add_argument("--flags", type=int_auto, default=0)
    p.add_argument("--device-id", type=int_auto, default=None)
    p.set_defaults(func=cmd_inspect)

    args = ap.parse_args(argv)
    try:
        return args.func(args)
    except (midi_io.MidiError, ValueError, OSError) as e:
        print(f"error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
