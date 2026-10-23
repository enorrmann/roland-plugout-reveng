# plugout-rev

Reverse engineering of the Roland SYSTEM-8 PLUG-OUT system, with three results:

1. **An install tool** (`tools/plugout_tool.py`) that sends PLUG-OUT firmware and patch banks to a SYSTEM-8 or
   SYSTEM-1 over USB, replicating the plugins' own install routine, without the VST plugin.
2. **A tool chain for the firmware** (`tools/plugout_build.py`, `tools/sys8fw.py`, `asm/`): unpack, patch and
   rebuild the PLUG-OUT containers and the SYSTEM-8 updater, and assemble and disassemble the DSP microcode.
3. **Our own PLUG-OUTs and lab experiments** (`examples/`): a from-scratch C engine, its own DSP microcode for
   an oscillator and a wavetable player, and a SYSTEM-8 image used as a lab.

All documents are in [`docs/`](docs/INDEX.md); start with [`docs/INDEX.md`](docs/INDEX.md).

## Scope

What this project is:

* A research record of how the PLUG-OUT firmware, the SYSTEM-8 OS and the DSP microcode work, with the evidence
  behind each claim (`docs/`).
* Tools that work against a real SYSTEM-8 on USB, plus offline tools for the containers and the firmware.
* Experiments that run on the device: custom PLUG-OUTs built from our own code, and one lab image.

What this project is not:

* Not a replacement of Roland's firmware or plugins. The tools install only into PLUG-OUT slots, the one the user
  chooses (`--slot`); what was there is overwritten, so reinstall the stock plugout to get it back.
* Not a complete DSP toolchain. The DSP instruction set is decoded for the parts the experiments use; see
  `docs/DSP_ISA_DISCREPANCIES.md` for what is still open.
* Not a copy of the plugins. The plugin folders (`plugins/`) and the decoded firmware (`work/`, `dumps/`) are
  local and ignored by git; the firmware updater images are in `firmware/`.

### Verified on a real SYSTEM-8 (firmware 1.32)

| Area | Status | Where |
|---|---|---|
| Install of firmware and patch banks, `info`, `banks`, `inspect` | works | `tools/plugout_tool.py`, [`docs/PROTOCOL.md`](docs/PROTOCOL.md) |
| Rebuilt JX-3P (byte-identical), renamed version, name change, effect label change | works | [`docs/CUSTOM_PLUGOUTS.md`](docs/CUSTOM_PLUGOUTS.md) |
| Code patch in the engine (EFFECT TONE inverted for OD/DS/FZ) | works | `examples/` + `docs/CUSTOM_PLUGOUTS.md` |
| Region 0 init hook reading OS RAM and the SD card | works | `examples/mem_name`, `examples/sd_txt` |
| Own C engine and own region 0 (parameter model), own packaging | plays | `examples/from_scratch/`, `docs/examples_from_scratch_engine_README.md` |
| Own DSP microcode: sawtooth oscillator at 96 kHz (±1.2 cents) | works | `docs/CUSTOM_OSC.md` |
| `C-V4`: fully own PLUG-OUT (own region 0, engine and all six main DSP programs, no Roland microcode), eight voices on both slaves | works (sawtooth with gate only) | [`docs/FROM_SCRATCH.md`](docs/FROM_SCRATCH.md), [`docs/VOICE_ALLOCATION.md`](docs/VOICE_ALLOCATION.md) §7 |
| Wavetable player (WAV → bank 5 → DSP) and `C-WTLIVE` live table | works | `examples/wt_live/`, `examples/from_scratch/re/wt5.py` |
| Live DSP peek and poke from the host | works | `docs/DSP_HARDWARE.md`, `examples/from_scratch/re/dsplive.py` |
| S8LAB lab image in PLG1: OSC1 COLOR changes the timbre | works (not monotonic) | `examples/sys8_lab/`, `docs/PLAN.md` |

### Not done or not verified

* **Own microcode beyond the oscillators above.** `C-V4` has no Roland microcode, but its voice is only a sawtooth
  with a gate: filters, envelopes, LFO and effects are not written yet (Roland's are used only in the older builds).
* **Noise on OSC1.** The stock engine has no native path for it, and the DSP decides the sound; any version needs
  a code cave (see `docs/CUSTOM_PLUGOUTS.md`).
* **Wavetable budget.** A table that exceeds the DSP cycle budget can need a power cycle of the device
  (`examples/from_scratch/re/wav2dco.py`, `docs/CUSTOM_OSC.md`, `docs/OPTIONS.md`).
* **The SYSTEM-8 modulation paths.** How an LFO or envelope reaches COLOR or OSC1 cross mod is not found: the
  patch cords and the EXTEND matrix did not change the sound in the tests (`docs/PLAN.md`, steps 3 to 8).
* **Bootloader and updater.** Both are encrypted (ECB); the key is in the BMC chip. The OS image is decoded
  (local dump, not in git).
* **Long samples.** A write to the EDM memory from ARM code is still open; not yet written up in `docs/`.
* **Emulation.** There is no full emulator of the device. The partial emulators (`tools/engine_emu.py`,
  `examples/from_scratch/re/emu0.py`, `emu1.py`) cover parts of the engine, and `examples/from_scratch/emu/` emulates
  the six DSPs of the slaves running our programs (tested against the `C-V4` hardware result). The DSP ISA was checked
  on the device by audio.
* **Other PLUG-OUTs.** The install code accepts the JUNO-60 and SH-101 folders, but the hardware records in
  `docs/` cover the JX-3P family and our own builds.

## Repository layout

| Folder | Contents |
|---|---|
| `tools/` | install tool, container and firmware tools, device access, and their tests |
| `asm/` | DSP assembler and disassembler (`asm/asm.py`, `asm/disasm.py`), with tests |
| `firmware/` | official SYSTEM-8 updater images, v1.30 to v1.32 |
| `base-engine/` | decoded base engine of the stock plugouts (reports and ELF exports) |
| `cleanroom/` | clean-room specification test vectors and their checker |
| `examples/` | experiments that run on the device (one folder each, see below) |
| `docs/` | all documents; index in [`docs/INDEX.md`](docs/INDEX.md) |
| `plugins/`, `work/`, `dumps/` | local, not in git: plugin folders, decoded images, device dumps |

### Examples

| Folder | What it is | Status |
|---|---|---|
| `examples/from_scratch/` | own PLUG-OUT in C (`engine/`, `model/`, `v3/`, `v4/`), the slave DSP emulator (`emu/`) and the reverse-engineering scripts (`re/`) | `C-V4` plays eight voices with only our code and microcode |
| `examples/sys8_lab/` | S8LAB, the SYSTEM-8 image in PLG1, with the COLOR, cord, MOD and LFO tests | installed; tests run |
| `examples/wt_live/` | `C-WTLIVE`: send a WAV wavetable to the slaves live (slot 3) | works |
| `examples/wt_morph/` | `C-MORPH`: one wavetable position morphing with an LFO | design only; no hardware run recorded; options in `docs/OPTIONS.md` |
| `examples/wt_pwm/` | `C-PWM`: wavetable PLUG-OUT with a WAV inside | built; no hardware run recorded in `docs/` |
| `examples/sd_txt/` | read a file from the SD card in a region 0 init hook | works |
| `examples/mem_name/`, `examples/mem_names/`, `examples/mem_scan/` | region 0 hooks that read OS RAM through the plugout name, and the dump loops | works |
| `examples/midi_send/` | captures and bulk dump / bulk loop of the device's SysEx | captures |
| `examples/cave_tone/`, `examples/preset_fill/` | code-cave hooks (TONE, preset fill and press) | experiments |

## Tools and firmware

**Usage and parameters of every tool: [`docs/TOOLS.md`](docs/TOOLS.md)** (workflows and daily tools by hand in
[`docs/TOOLS_GUIDE.md`](docs/TOOLS_GUIDE.md), the full reference generated by `python3 tools/tools_usage.py`).

| Path | What |
|---|---|
| `tools/plugout_tool.py` | CLI: `ports`, `info`, `install`, `banks`, `inspect` |
| `tools/plugout.py` | container format, Roland SysEx, protocol constants |
| `tools/patches.py` | `Script.xml` structures, `Patch/*.bin` banks, patch addresses |
| `tools/midi_io.py` | MIDI ports (ALSA raw MIDI via ctypes, python-rtmidi) |
| `tools/plugout_build.py` | CLI to unpack, modify and rebuild `Code*.Dat` ([`docs/CUSTOM_PLUGOUTS.md`](docs/CUSTOM_PLUGOUTS.md)) |
| `tools/plugout_image.py` | flash image, region header, SYNINFO, relocation, container builder |
| `tools/lzss.py` | LZSS codec of the firmware regions |
| `tools/sys8fw.py` | unpack / patch / rebuild `SYSTEM8_UP.BIN`, RTTI symbols, ELF ([`docs/BASE_FIRMWARE.md`](docs/BASE_FIRMWARE.md)) |
| `tools/os_analyze.py`, `tools/svc_table.py`, `tools/crypto_scan.py` | analysis of the decoded OS and its service table |
| `tools/engine_*.py`, `tools/read_param.py`, `tools/patch_read.py` | engine analysis and parameter reads |
| `tools/check_optimized.py` | warns about unoptimized DSP microcode; every build runs it ([`docs/MICROCODE_OPTIMIZATION.md`](docs/MICROCODE_OPTIMIZATION.md)) |
| `tools/tools_usage.py` | regenerates [`docs/TOOLS.md`](docs/TOOLS.md) |
| `tools/test_sys8fw.py`, `tools/test_plugout.py`, `tools/test_plugout_build.py` | tests (`python3 -m unittest discover -s tools`) |
| `asm/` | DSP assembler and disassembler ([`docs/DSP_ISA_SPEC.md`](docs/DSP_ISA_SPEC.md), [`docs/REVIEW.md`](docs/REVIEW.md)) |
| `firmware/sys8_sys_v132/SYSTEM8_UP.BIN` | official SYSTEM-8 updater (v1.32); see [`docs/VERSIONS.md`](docs/VERSIONS.md) |

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

## Limitations

* The patch transfer has been tested against a simulated device and on the real SYSTEM-8; it follows the plugin's
  code ([`docs/PROTOCOL.md`](docs/PROTOCOL.md) §6).
* Patches are sent without flow control (as the plugin does) and, by default, without delays. If the device drops
  data, use `--gap-ms 10 --wire-pacing`.
* Do not run two tools against the device at once: they share one MIDI port and their replies mix.
