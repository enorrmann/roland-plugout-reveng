#!/usr/bin/env python3
"""
midi_io.py -- minimal SysEx-capable MIDI ports.

Backends:
  alsa    Linux raw MIDI through libasound (ctypes, no Python packages needed).
          Port names are ALSA raw MIDI names: "hw:3,0,1".
  rtmidi  python-rtmidi (pip install python-rtmidi): Linux, Windows, macOS.
          Port names are the rtmidi port names (substring match accepted).

The SYSTEM-8 shows two USB MIDI ports. The plugins talk to the second one,
"SYSTEM-8 CTRL" (Script/Script.xml <midiOut><name>); on Linux it usually
appears as subdevice 1 ("hw:N,0,1", sometimes named "SYSTEM-8 MIDI 2").
"""

from __future__ import annotations

import ctypes
import ctypes.util
import errno
import re
import shutil
import subprocess
import time

DEVICE_HINTS = ("SYSTEM-8", "SYSTEM-1")


class MidiError(RuntimeError):
    pass


class SysexParser:
    """Reassemble complete F0..F7 messages from a raw byte stream."""

    def __init__(self):
        self._buf: bytearray | None = None

    def feed(self, data: bytes) -> list[bytes]:
        out = []
        for b in data:
            if b >= 0xF8:                    # real-time bytes may appear anywhere
                continue
            if b == 0xF0:
                self._buf = bytearray([b])
            elif self._buf is not None:
                if b == 0xF7:
                    self._buf.append(b)
                    out.append(bytes(self._buf))
                    self._buf = None
                elif b & 0x80:               # any other status aborts the SysEx
                    self._buf = None
                else:
                    self._buf.append(b)
        return out


def pick_port(names: list[str]) -> str | None:
    """Choose the SYSTEM-8/SYSTEM-1 control port from a list of port names."""
    dev = [n for n in names if any(h in n.upper() for h in DEVICE_HINTS)]
    for n in dev:
        if "CTRL" in n.upper():
            return n
    return dev[1] if len(dev) >= 2 else (dev[0] if dev else None)


# ---------------------------------------------------------------------------
# ALSA raw MIDI (ctypes)
# ---------------------------------------------------------------------------

SND_RAWMIDI_NONBLOCK = 0x0002


def alsa_ports() -> list[tuple[str, str]]:
    """[(hw name, description)] from `amidi -l`."""
    if not shutil.which("amidi"):
        raise MidiError("amidi not found (install alsa-utils) -- or pass --port hw:X,Y,Z")
    out = subprocess.run(["amidi", "-l"], capture_output=True, text=True).stdout
    ports = []
    for line in out.splitlines():
        m = re.match(r"\s*\S+\s+(hw:\d+,\d+(?:,\d+)?)\s+(.*)$", line)
        if m:
            ports.append((m.group(1), m.group(2).strip()))
    return ports


class AlsaPort:
    def __init__(self, name: str):
        libname = ctypes.util.find_library("asound")
        if not libname:
            raise MidiError("libasound not found")
        lib = self._lib = ctypes.CDLL(libname)
        lib.snd_rawmidi_open.argtypes = [ctypes.POINTER(ctypes.c_void_p),
                                         ctypes.POINTER(ctypes.c_void_p),
                                         ctypes.c_char_p, ctypes.c_int]
        lib.snd_rawmidi_write.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t]
        lib.snd_rawmidi_write.restype = ctypes.c_ssize_t
        lib.snd_rawmidi_read.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t]
        lib.snd_rawmidi_read.restype = ctypes.c_ssize_t
        lib.snd_rawmidi_drain.argtypes = [ctypes.c_void_p]
        lib.snd_rawmidi_close.argtypes = [ctypes.c_void_p]
        lib.snd_strerror.restype = ctypes.c_char_p

        self._in, self._out = ctypes.c_void_p(), ctypes.c_void_p()
        rc = lib.snd_rawmidi_open(ctypes.byref(self._in), ctypes.byref(self._out),
                                  name.encode(), SND_RAWMIDI_NONBLOCK)
        if rc < 0:
            raise MidiError(f"cannot open {name}: {lib.snd_strerror(rc).decode()}")
        # In and out share one file descriptor, so both stay non-blocking;
        # send() retries on EAGAIN instead.
        self.name = name
        self._parser = SysexParser()
        self._rbuf = ctypes.create_string_buffer(4096)

    def send(self, msg: bytes) -> None:
        view = memoryview(bytes(msg))
        while view:
            n = self._lib.snd_rawmidi_write(self._out, bytes(view), len(view))
            if n < 0:
                if -n in (errno.EAGAIN, errno.EINTR):
                    time.sleep(0.001)
                    continue
                raise MidiError(f"write: {self._lib.snd_strerror(int(n)).decode()}")
            view = view[n:]
        self._lib.snd_rawmidi_drain(self._out)

    def receive(self) -> list[bytes]:
        msgs = []
        while True:
            n = self._lib.snd_rawmidi_read(self._in, self._rbuf, len(self._rbuf))
            if n == -errno.EAGAIN or n == 0:
                return msgs
            if n < 0:
                raise MidiError(f"read: {self._lib.snd_strerror(int(n)).decode()}")
            msgs += self._parser.feed(self._rbuf.raw[:n])

    def close(self) -> None:
        for h in (self._in, self._out):
            if h:
                self._lib.snd_rawmidi_close(h)
        self._in = self._out = ctypes.c_void_p()


# ---------------------------------------------------------------------------
# python-rtmidi
# ---------------------------------------------------------------------------

def _rtmidi():
    try:
        import rtmidi
    except ImportError:
        raise MidiError("python-rtmidi not installed (pip install python-rtmidi)") from None
    return rtmidi


def rtmidi_ports() -> list[tuple[str, str]]:
    rtmidi = _rtmidi()
    ins = rtmidi.MidiIn().get_ports()
    return [(p, "in+out" if p in ins else "out") for p in rtmidi.MidiOut().get_ports()]


class RtMidiPort:
    def __init__(self, name: str):
        rtmidi = _rtmidi()
        self._out, self._in = rtmidi.MidiOut(), rtmidi.MidiIn()
        self.name = name
        self._out.open_port(self._find(self._out.get_ports(), name))
        self._in.open_port(self._find(self._in.get_ports(), name))
        self._in.ignore_types(sysex=False, timing=True, active_sense=True)
        self._parser = SysexParser()

    @staticmethod
    def _find(ports: list[str], name: str) -> int:
        if name in ports:
            return ports.index(name)
        hits = [i for i, p in enumerate(ports) if name.lower() in p.lower()]
        if len(hits) != 1:
            raise MidiError(f"port {name!r}: {len(hits)} matches in {ports}")
        return hits[0]

    def send(self, msg: bytes) -> None:
        self._out.send_message(list(msg))

    def receive(self) -> list[bytes]:
        msgs = []
        while True:
            item = self._in.get_message()
            if item is None:
                return msgs
            msgs += self._parser.feed(bytes(item[0]))

    def close(self) -> None:
        self._in.close_port()
        self._out.close_port()


# ---------------------------------------------------------------------------

def default_backend() -> str:
    return "alsa" if ctypes.util.find_library("asound") else "rtmidi"


def list_ports(backend: str) -> list[tuple[str, str]]:
    return alsa_ports() if backend == "alsa" else rtmidi_ports()


def find_port(backend: str) -> str:
    ports = list_ports(backend)
    if backend == "alsa":
        name = pick_port([f"{hw} {desc}" for hw, desc in ports])
        name = name.split()[0] if name else None
    else:
        name = pick_port([p for p, _ in ports])
    if not name:
        raise MidiError("no SYSTEM-8 / SYSTEM-1 port found; connect it or pass --port")
    return name


def open_port(backend: str, name: str):
    return AlsaPort(name) if backend == "alsa" else RtMidiPort(name)
