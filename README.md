# plugout-rev

Install Roland PLUG-OUTs (JUNO-60, JX-3P, SH-101) into a SYSTEM-8 (or SYSTEM-1)
connected over USB, **without the VST plugin**. The tools replicate the plugins'
own install routine; the protocol is documented in [`PROTOCOL.md`](PROTOCOL.md).

Only the plugin folders' `Script/` (`Code*.Dat`, `Script.xml`) and `Patch/`
files are needed.

## Requirements

* Python 3.8+.
* Linux: nothing else (uses `libasound` through ctypes; `amidi` from
  `alsa-utils` to detect the port).
* Windows / macOS: `pip install python-rtmidi` (the `rtmidi` backend, which
  also works on Linux).

## Usage

```bash
# 1. Is the synth visible? (the "CTRL" port is selected automatically)
python3 tools/plugout_tool.py ports

# 2. Device and slot status
python3 tools/plugout_tool.py info

# 3. Install firmware + patch bank
#    (Code8_<slot>.Dat is picked from the slot; bank = first file in Patch/)
python3 tools/plugout_tool.py install JUNO-60 --slot 1

# list the patches of the plugin's banks
python3 tools/plugout_tool.py banks plugins/JUNO-60

# resend only the patches: pick ranges (1-based) from one or more banks;
# they fill the slot's patches in order (bank 1-1, 1-2, ... 8-8)
python3 tools/plugout_tool.py install plugins/JUNO-60 --slot 1 --patches-only \
    --bank "2 Preset.bin:1-32" --bank "3 Preset.bin:1-32"
```

The slot holds 64 patches (`info` shows the number). A selection longer than
that is cut with a warning; a shorter one writes only those patches, leaving
the rest of the slot unwritten (as the plugin does with a short bank). Banks
saved by the plugin with more than 64 patches are read too.

Example `info` output (real SYSTEM-8):

```
port     : hw:3,0,1 (alsa)
device   : SYSTEM-8
firmware : 1.32   patches per slot: 64
  (base)  slot 0: SYSTEM-8 id 0x0005 version 1.20
  PLUG-OUT slot 1: JUPITER-8 id 0x0006 version 1.07
  PLUG-OUT slot 2: JUPITER-4 id 0x000b version 1.00
  PLUG-OUT slot 3: JUNO-60 id 0x000a version 1.01
```

Before sending, `install` makes the same checks as the plugin:

* refuses if the model is already installed in another slot (unless `--force`);
* if the slot already holds the same version, skips the firmware and sends only
  the patches (`--force` resends the firmware too);
* warns when another PLUG-OUT is about to be overwritten;
* asks you to type `yes` (`-y` skips this).

After the firmware it sends the patch bank (64 patches, ~6200 messages). No
delays are added: the SYSTEM-8 sets the pace. Do not disconnect or power off the
device during the transfer.

### Options

| Option | Use |
|---|---|
| `--port hw:3,0,1` | explicit port (for rtmidi: the name or part of it) |
| `--backend rtmidi` | use python-rtmidi instead of raw ALSA |
| `--device-id 0x11` | if the synth's Device ID was changed (default 17 = 0x10) |
| `--dat PATH` | send a specific `Code*.Dat` |
| `--dry-run` | show what would be sent, without MIDI |
| `--skip-check` | do not query the device first (assumes SYSTEM-8; `--device-type` for others) |
| `--bank FILE[:RANGES]` | patches to send, e.g. `"2 Preset.bin:1-8,17-24"`; repeatable (default: first file in `<plugin>/Patch`) |
| `--no-patches` / `--patches-only` | firmware only / patches only |
| `--gap-ms 10 --wire-pacing` | pause between messages like the plugin (default: none) |
| `-v` | print every message sent and received |

### Inspecting a container

```bash
python3 tools/plugout_tool.py inspect JX-3P/Script/Code8_1.Dat
python3 tools/plugout_tool.py inspect JX-3P/Script/Code8_1.Dat --image out/jx3p_ --syx out/jx3p.syx
```

`--image` extracts the two firmware regions; `--syx` saves the messages the host
sends. That `.syx` is **not** suitable for blind replay with `amidi -s`: the
transfer must wait for the flow-control replies every 32 messages, which is what
`install` does.

## Troubleshooting

* **`no SYSTEM-8 / SYSTEM-1 port found`**: check `amidi -l` and pass `--port`.
  The right port is the device's second one (`SYSTEM-8 CTRL`, usually
  `hw:N,0,1` on Linux). The first port does not answer.
* **`no reply on 71 00 02 00`**: wrong port, different device ID, or the synth
  is not in normal mode.
* **`device refused the transfer`** / **`device rejected data record`**: the
  device rejected the start or a block; check that the file matches the slot
  (`Code8_<slot>.Dat`).
* Use `-v` to see the full traffic.

## Files

| Path | What |
|---|---|
| `tools/plugout_tool.py` | CLI: `ports`, `info`, `install`, `inspect` |
| `tools/plugout.py` | container format, Roland SysEx, protocol constants |
| `tools/patches.py` | `Script.xml` structures, `Patch/*.bin` banks, patch addresses |
| `tools/midi_io.py` | MIDI ports (ALSA raw MIDI via ctypes, python-rtmidi) |
| `tools/plugout_build.py` | CLI to unpack, modify and rebuild `Code*.Dat` ([`CUSTOM_PLUGOUTS.md`](CUSTOM_PLUGOUTS.md)) |
| `tools/plugout_image.py` | flash image, region header, SYNINFO, relocation, container builder |
| `tools/lzss.py` | LZSS codec of the firmware regions |
| `tools/sys8fw.py` | unpack / patch / rebuild `SYSTEM8_UP.BIN` (base firmware), RTTI symbols, ELF ([`BASE_FIRMWARE.md`](BASE_FIRMWARE.md)) |
| `tools/test_sys8fw.py` | tests for `sys8fw.py` against the shipped updater |
| `tools/test_plugout.py` | tests against a simulated SYSTEM-8: `python3 -m unittest discover -s tools` |
| `tools/test_plugout_build.py` | rebuild / relocation tests against the shipped files |
| `PROTOCOL.md` | protocol extracted from the plugins, with code addresses |
| `CUSTOM_PLUGOUTS.md` | firmware format, slots, relocation, OS interface; building custom PLUG-OUTs |
| `firmware/SYSTEM8_UP.BIN` | official SYSTEM-8 updater (analysed in `CUSTOM_PLUGOUTS.md` §5) |

## Limitations

* `info` has been run against a real SYSTEM-8 (firmware 1.32), and after a
  firmware install it reported the JUNO-60 1.01 in the target slot. The patch
  transfer has only been tested against a simulated device; it follows the
  plugin's code (`PROTOCOL.md` §6).
* Patches are sent without flow control (as the plugin does) and, by default,
  without delays. If the device drops data, use `--gap-ms 10 --wire-pacing`.
