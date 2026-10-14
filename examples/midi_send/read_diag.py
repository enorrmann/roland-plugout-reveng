#!/usr/bin/env python3
"""Read the 128 nibble bytes at NAME0 of preset 1 (slot 3) and decode them as
64 bytes, showing the Msg/Wrap structures written by diag.s."""
import os, sys, struct, time
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "tools"))
sys.path.insert(0, os.path.join(ROOT, "examples", "preset_fill"))
import patches as PT, plugout as P, plugout_tool as T
from run_test import rq1

class Args:
    backend = None; port = None; device_id = P.DEFAULT_DEVICE_ID
    gap_ms = 0.0; wire_pacing = False; verbose = False; slot = 3
a = Args()
s, port = T.open_session(a)
# NAME0 of preset 1 (patch 0), slot 3
layout = PT.ScriptLayout(os.path.join(ROOT, "plugins", "JX-3P", "Script", "Script.xml"))
name0 = next(l for l in layout.leaves["patch"] if l.path == "NAME0")
addr = PT.patch_address(a.slot, 0, 0) + name0.offset  # 0x1E00080
print(f"reading NAME0 at {addr:#x} ...")
raw = rq1(s, addr, 128)
port.close()
if raw is None:
    sys.exit("RQ1 of NAME0 got no reply")
if len(raw) < 128:
    sys.exit(f"NAME0 too short: {len(raw)} bytes")
decoded = bytes((raw[2*i] << 4) | raw[2*i+1] for i in range(64))
print(f"NAME0 raw (first 32): {raw[:32].hex(' ')}")
print(f"decoded (64 bytes):   {decoded.hex(' ')}")
print()
print(f"  buf[0..14] (SysEx):   {decoded[0:15].hex(' ')}")
print(f"  buf[15]    (pad):     {decoded[15]:02x}")
print(f"  Msg[0..7]:            {decoded[16:24].hex(' ')}")
print(f"  Wrap[0..11]:          {decoded[24:36].hex(' ')}")
marker = struct.unpack_from("<I", decoded, 36)[0]
print(f"  marker:               {marker:#010x}  {'OK' if marker == 0xDEADBEEF else 'WRONG (hook did not run or corrupted)'}")
print(f"  pad[40..63]:          {decoded[40:64].hex(' ')}")
