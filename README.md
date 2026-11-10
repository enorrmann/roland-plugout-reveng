# Roland Custom Plug-Out Ecosystem & BMC DSP SDK

[![Docs](https://img.shields.io/badge/Docs-TRM%20Suite%20(Vols%20I--VII)-blue.svg)](docs/index.md)
[![Clean-room vectors](https://img.shields.io/badge/Cleanroom-test%20vectors-brightgreen.svg)](cleanroom/test-vectors/)
[![Doccheck](https://img.shields.io/badge/Doccheck-dsp%20blocks%20assemble-success.svg)](tools/doccheck.py)

**Build and run your own synthesizer engines on Roland hardware.**

Reverse engineering of the Roland SYSTEM-8 and SYSTEM-1 PLUG-OUT system, and the tools that came out of it: an install
tool that replaces the VST plugin's installer, a tool chain for the firmware containers and the DSP microcode (assembler,
disassembler, scheduler), a bit-exact emulator of the DSP, a C / C++ compiler to the DSP microcode (`s1cc`), and our own
PLUG-OUTs that run on the devices. The documentation is the 7-volume suite in [`docs/`](docs/index.md); start with
[`docs/index.md`](docs/index.md).

---

## 1. The objective: custom PLUG-OUTs

```
   ┌─────────────────────────────────────────────────────────────────────────┐
   │                       CUSTOM PLUG-OUT WORKFLOW                          │
   │                                                                         │
   │   1. DEVELOP                  2. SIMULATE                 3. DEPLOY     │
   │  ┌───────────────┐           ┌───────────────┐          ┌────────────┐  │
   │  │ C/C++ code or │  s1cc /   │ bit-exact C   │          │ SYSTEM-8 / │  │
   │  │ DSP assembly  │──────────►│ emulator      │─────────►│ SYSTEM-1   │  │
   │  │ (sdk/, asm/)  │  asm/     │ (emu/)        │ USB-MIDI │ PLUG-OUT   │  │
   │  └───────────────┘           └───────────────┘          └────────────┘  │
   └─────────────────────────────────────────────────────────────────────────┘
```

Engines of our own that ran on the devices (details and evidence in the documents named):

* **C-V4 (SYSTEM-8)**: own region 0, engine and all six main DSP programs, no Roland microcode; eight voices on both
  slaves, a sawtooth with a gate ([Vol. VI ch. 1](docs/vol6_reference_designs_applications/c_v4_system8_reference.md)).
* **C-S1V4 / C-S1V8 (SYSTEM-1)**: the same design ported; C-S1V8 plays 8 voices, twice the stock 4
  ([Vol. VI ch. 2](docs/vol6_reference_designs_applications/c_s1v8_system1_reference.md)).
* **C-S1SY (SYSTEM-1)**: a synth built step by step on the device; C-S1SY13F is the last build that works completely
  ([examples/system_1_synth/README.md](examples/system_1_synth/README.md)).
* **SYSTEM-S1CC kernels (SYSTEM-1)**: voices written in C and compiled with s1cc; MP-CSAW and MP-CSAW2 (Mutable
  Instruments' CSAW rewritten for the DSP) and the EDM strings MP-EDMSTR (6 voices) were tested on the device
  ([Vol. VI ch. 4](docs/vol6_reference_designs_applications/mutable_instruments_ports.md),
  [ch. 7](docs/vol6_reference_designs_applications/mutable_memory.md)).
* **Wavetables**: C-WTLIVE sends a WAV table to the SYSTEM-8 slaves live; on the SYSTEM-1, WT-SLAVE5 (8 voices) and
  WT-EDM / WT-LINES8 (6 voices) ([Vol. VI ch. 5](docs/vol6_reference_designs_applications/wavetable_engines.md)).

Built and checked in the emulator only, not on the device yet: the Casio CZ phase-distortion kernel (CZ-SYSTEM) and most
of the other Mutable kernels.

## 2. The developer toolkit

* **`s1cc`** ([Vol. V ch. 1](docs/vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md)): clang compiles the C / C++ of
  a voice to LLVM IR, s1cc (C++, LLVM 19 API) translates it to DSP microcode, and `s1cc check` compares it with the same
  source compiled natively. A minimal voice with the SDK header `s1dsp.h`
  ([Vol. V ch. 2](docs/vol5_toolchain_compiler_sdk/s1dsp_api_reference.md)):

  ```c
  #include "s1dsp.h"
  static float ph;
  S1_DSP_A {                                /* slave DSP A, every sample, every voice */
      float p = ph + s1_dt();               /* phase step of the note */
      p -= s1_floor(p);                     /* frac in one op */
      ph = p;
      return 2.0f * p - 1.0f;               /* a naive saw */
  }
  ```
* **`sdk/mport`** ([Vol. V ch. 3](docs/vol5_toolchain_compiler_sdk/mport_mutable_sdk.md)): hand ports of Mutable
  Instruments components with a C++ reference and a bit-exact integer model (Braids SAW_SWARM).
* **Emulator** (`examples/from_scratch/emu/`, [Vol. V ch. 5](docs/vol5_toolchain_compiler_sdk/emulator.md)): the C
  reference of the DSP (`dspdec.h`, `dspcore.h`), native translation and threads; one JX-3P slave ran at 1.23× real time
  when measured; `test_core.py` checks the 15 756 distinct instructions of the local corpus against a golden file.
* **Microcode toolchain** (`asm/`, [Vol. V ch. 4](docs/vol5_toolchain_compiler_sdk/assembler_disassembler_tools.md)):
  assembler and disassembler with a round trip on Roland's corpus, the scheduler `dspsched.py`, the SYSTEM-1 access
  rules `s1rules.py`, and `tools/check_optimized.py`.
* **Install and firmware tools** (`tools/`): `plugout_tool.py` installs PLUG-OUT firmware and patch banks over USB-MIDI
  (§10), `plugout_build.py` unpacks and rebuilds `Code*.Dat`, `sys8fw.py` and `app1_crypto.py` the SYSTEM-8 updater.
  Usage of every tool: [`docs/TOOLS.md`](docs/TOOLS.md).

## 3. Scope

What this project is:

* A research record of how the PLUG-OUT firmware, the SYSTEM-8 OS and the DSP microcode work, with the evidence
  behind each claim (`docs/`).
* Tools that work against a real SYSTEM-8 on USB, plus offline tools for the containers and the firmware.
* Experiments that run on the device: custom PLUG-OUTs built from our own code, and one lab image.

What this project is not:

* Not a replacement of Roland's firmware or plugins. The tools install only into PLUG-OUT slots, the one the user
  chooses (`--slot`); what was there is overwritten, so reinstall the stock plugout to get it back.
* Not a complete DSP toolchain. The DSP instruction set is decoded for the parts the experiments use; see
  [Vol. II ch. 8](docs/vol2_dsp_core_isa/isa_discrepancies.md) for what is still open.
* Not a copy of the plugins. The plugin folders (`plugins/`) and the decoded firmware (`work/`, `dumps/`) are
  local and ignored by git; the firmware updater images are in `firmware/`.

### 3.1 Verified on a real SYSTEM-8 (firmware 1.32), as of the original README

| Area | Status | Where |
|---|---|---|
| Install of firmware and patch banks, `info`, `banks`, `inspect` | works | `tools/plugout_tool.py`, [`docs/vol4_firmware_container_protocol/usb_sysex_protocol_spec.md`](docs/vol4_firmware_container_protocol/usb_sysex_protocol_spec.md) |
| Rebuilt JX-3P (byte-identical), renamed version, name change, effect label change | works | [`docs/vol4_firmware_container_protocol/plugout_virtual_machine.md`](docs/vol4_firmware_container_protocol/plugout_virtual_machine.md) |
| Code patch in the engine (EFFECT TONE inverted for OD/DS/FZ) | works | `examples/` + [Vol. IV ch. 2](docs/vol4_firmware_container_protocol/plugout_virtual_machine.md) |
| Region 0 init hook reading OS RAM and the SD card | works | `examples/mem_name`, `examples/sd_txt` |
| Own C engine and own region 0 (parameter model), own packaging | plays | `examples/from_scratch/`, [examples/from_scratch/engine/README.md](examples/from_scratch/engine/README.md) |
| Own DSP microcode: sawtooth oscillator at 96 kHz (±1.2 cents) | works | [Vol. VI ch. 6](docs/vol6_reference_designs_applications/custom_osc.md) |
| `C-V4`: fully own PLUG-OUT (own region 0, engine and all six main DSP programs, no Roland microcode), eight voices on both slaves | works (sawtooth with gate only) | [`docs/vol6_reference_designs_applications/c_v4_system8_reference.md`](docs/vol6_reference_designs_applications/c_v4_system8_reference.md), [`docs/vol3_voice_allocation_scheduling/system8_voice_allocation.md`](docs/vol3_voice_allocation_scheduling/system8_voice_allocation.md) §7 |
| Wavetable player (WAV → bank 5 → DSP) and `C-WTLIVE` live table | works | `examples/wt_live/`, `examples/from_scratch/re/wt5.py` |
| Live DSP peek and poke from the host | works | [Vol. II ch. 1](docs/vol2_dsp_core_isa/core_architecture.md), `examples/from_scratch/re/dsplive.py` |
| S8LAB lab image in PLG1: OSC1 COLOR changes the timbre | works (not monotonic) | `examples/sys8_lab/`, [PLAN.md](docs/vol7_cleanroom_evidence_archive/PLAN.md) |

### 3.2 Not done or not verified (as of the original README)

* **Own microcode beyond the oscillators above.** `C-V4` has no Roland microcode, but its voice is only a sawtooth
  with a gate: filters, envelopes, LFO and effects are not written yet (Roland's are used only in the older builds).
* **Noise on OSC1.** The stock engine has no native path for it, and the DSP decides the sound; any version needs
  a code cave (see [Vol. IV ch. 2](docs/vol4_firmware_container_protocol/plugout_virtual_machine.md)).
* **Wavetable budget.** A table that exceeds the DSP cycle budget can need a power cycle of the device
  (`examples/from_scratch/re/wav2dco.py`, [Vol. VI ch. 6](docs/vol6_reference_designs_applications/custom_osc.md), [examples/wt_morph/OPTIONS.md](examples/wt_morph/OPTIONS.md)).
* **The SYSTEM-8 modulation paths.** How an LFO or envelope reaches COLOR or OSC1 cross mod is not found: the
  patch cords and the EXTEND matrix did not change the sound in the tests ([PLAN.md](docs/vol7_cleanroom_evidence_archive/PLAN.md), steps 3 to 8).
* **Bootloader and updater.** Both are encrypted (ECB); the key is in the BMC chip. The OS image is decoded
  (local dump, not in git).
* **Long samples.** A write to the EDM memory from ARM code is still open; not yet written up in `docs/`.
* **Emulation.** There is no full emulator of the device. The partial emulators (`tools/engine_emu.py`,
  `examples/from_scratch/re/emu0.py`, `emu1.py`) cover parts of the engine, and `examples/from_scratch/emu/` emulates
  the six DSPs of the slaves running our programs (tested against the `C-V4` hardware result). The DSP ISA was checked
  on the device by audio.
* **Other PLUG-OUTs.** The install code accepts the JUNO-60 and SH-101 folders, but the hardware records in
  `docs/` cover the JX-3P family and our own builds.

## 4. The documentation

The 7-volume suite is indexed in [`docs/index.md`](docs/index.md), with a table that maps every old document name to its
new chapter. The SYSTEM-1's own documents stay in [`system-1/`](system-1/README.md) (Spanish); each example has its
README in its folder.

## 5. Quick start

```bash
python3 tools/plugout_tool.py ports            # is the synth visible?
python3 tools/plugout_tool.py info             # device, firmware and slots

# SYSTEM-8: build C-V4 and install it into slot 3, then select PLG3 on the panel
cd examples/from_scratch && VOICELOG=1 NAME=C-V4 sh v4/build_v4.sh && cd ../..
python3 tools/plugout_tool.py install plugins/C-V4 --slot 3 --force --no-patches -y

# SYSTEM-1: build C-S1V8 and install it into slot 1 (it replaces what is there)
VOICES=8 NAME=C-S1V8 sh examples/system_1/build.sh
python3 tools/plugout_tool.py install plugins/C-S1V8 --slot 1 --force --no-patches -y

# checks without hardware
python3 examples/from_scratch/emu/test_core.py
python3 examples/from_scratch/emu/test_chain.py
python3 tools/doccheck.py docs/vol*/*.md system-1/*.md
python3 cleanroom/test-vectors/check_vectors.py .
```

Only one tool may talk to the device at a time (MIDI).

## 6. Repository layout

| Folder | Contents |
|---|---|
| `tools/` | install tool, container and firmware tools, device access, and their tests |
| `asm/` | DSP assembler, disassembler, scheduler and rules, with tests |
| `emu` (in `examples/from_scratch/emu/`) | the DSP emulator (C reference, native translation) |
| `sdk/` | `s1cc` (C / C++ → DSP microcode) and `mport` (Mutable ports) |
| `firmware/` | official updater images (SYSTEM-8 v1.30 to v1.32, SYSTEM-1 v1.30) |
| `base-engine/` | decoded base engine of the stock plugouts (reports and ELF exports) |
| `cleanroom/` | clean-room specification test vectors and their checker |
| `system-1/` | SYSTEM-1 analysis documents and scripts |
| `examples/` | our PLUG-OUTs and experiments (one folder each, each with a README where there is one) |
| `web/` | the browser installer (Web MIDI) |
| `docs/` | the documentation suite; index in [`docs/index.md`](docs/index.md) |
| `plugins/`, `work/`, `dumps/` | local, not in git: plugin folders, decoded images, device dumps |

### 6.1 Examples (SYSTEM-8 experiments of the original README)

| Folder | What it is | Status |
|---|---|---|
| `examples/from_scratch/` | own PLUG-OUT in C (`engine/`, `model/`, `v3/`, `v4/`), the slave DSP emulator (`emu/`) and the reverse-engineering scripts (`re/`) | `C-V4` plays eight voices with only our code and microcode |
| `examples/sys8_lab/` | S8LAB, the SYSTEM-8 image in PLG1, with the COLOR, cord, MOD and LFO tests | installed; tests run |
| `examples/wt_live/` | `C-WTLIVE`: send a WAV wavetable to the slaves live (slot 3) | works |
| `examples/wt_morph/` | `C-MORPH`: one wavetable position morphing with an LFO | design only; no hardware run recorded; options in [examples/wt_morph/OPTIONS.md](examples/wt_morph/OPTIONS.md) |
| `examples/wt_pwm/` | `C-PWM`: wavetable PLUG-OUT with a WAV inside | built; no hardware run recorded in `docs/` |
| `examples/sd_txt/` | read a file from the SD card in a region 0 init hook | works |
| `examples/mem_name/`, `examples/mem_names/`, `examples/mem_scan/` | region 0 hooks that read OS RAM through the plugout name, and the dump loops | works |
| `examples/midi_send/` | captures and bulk dump / bulk loop of the device's SysEx | captures |
| `examples/cave_tone/`, `examples/preset_fill/` | code-cave hooks (TONE, preset fill and press) | experiments |

## 7. Clean-room boundary

The clean-room specification of the PLUG-OUT firmware and protocol is a separate track, written from the evidence
without the tools' code; its team separation and authorised material are in
[Vol. VII ch. 1](docs/vol7_cleanroom_evidence_archive/cleanroom_boundary_contract.md) (which states that it is not legal
advice). Roland's plugin binaries and decoded firmware are local and ignored by git; our own PLUG-OUTs contain no Roland
microcode (`emu/sigcheck.py` checks the builds).

## 8. Tools and firmware

**Usage and parameters of every tool: [`docs/TOOLS.md`](docs/TOOLS.md)** (workflows and daily tools by hand in
[`docs/TOOLS_GUIDE.md`](docs/TOOLS_GUIDE.md), the full reference generated by `python3 tools/tools_usage.py`).

| Path | What |
|---|---|
| `tools/plugout_tool.py` | CLI: `ports`, `info`, `install`, `banks`, `inspect` |
| `tools/plugout.py` | container format, Roland SysEx, protocol constants |
| `tools/patches.py` | `Script.xml` structures, `Patch/*.bin` banks, patch addresses |
| `tools/midi_io.py` | MIDI ports (ALSA raw MIDI via ctypes, python-rtmidi) |
| `tools/plugout_build.py` | CLI to unpack, modify and rebuild `Code*.Dat` ([`docs/vol4_firmware_container_protocol/plugout_virtual_machine.md`](docs/vol4_firmware_container_protocol/plugout_virtual_machine.md)) |
| `tools/plugout_image.py` | flash image, region header, SYNINFO, relocation, container builder |
| `tools/lzss.py` | LZSS codec of the firmware regions |
| `tools/sys8fw.py` | unpack / patch / rebuild `SYSTEM8_UP.BIN`, RTTI symbols, ELF ([`docs/vol4_firmware_container_protocol/base_firmware.md`](docs/vol4_firmware_container_protocol/base_firmware.md)) |
| `tools/os_analyze.py`, `tools/svc_table.py`, `tools/crypto_scan.py` | analysis of the decoded OS and its service table |
| `tools/engine_*.py`, `tools/read_param.py`, `tools/patch_read.py` | engine analysis and parameter reads |
| `tools/check_optimized.py` | warns about unoptimized DSP microcode; every build runs it ([`docs/vol3_voice_allocation_scheduling/microcode_optimization.md`](docs/vol3_voice_allocation_scheduling/microcode_optimization.md)) |
| `tools/tools_usage.py` | regenerates [`docs/TOOLS.md`](docs/TOOLS.md) |
| `tools/test_sys8fw.py`, `tools/test_plugout.py`, `tools/test_plugout_build.py` | tests (`python3 -m unittest discover -s tools`) |
| `asm/` | DSP assembler and disassembler ([`docs/vol2_dsp_core_isa/instruction_set_reference.md`](docs/vol2_dsp_core_isa/instruction_set_reference.md), [`docs/vol7_cleanroom_evidence_archive/REVIEW.md`](docs/vol7_cleanroom_evidence_archive/REVIEW.md)) |
| `firmware/sys8_sys_v132/SYSTEM8_UP.BIN` | official SYSTEM-8 updater (v1.32); see [`docs/vol4_firmware_container_protocol/firmware_versions.md`](docs/vol4_firmware_container_protocol/firmware_versions.md) |

## 9. Requirements

* Python 3.8+.
* Linux: nothing else (uses `libasound` through ctypes; `amidi` from
  `alsa-utils` to detect the port).
* Windows / macOS: `pip install python-rtmidi` (the `rtmidi` backend, which
  also works on Linux).

## 10. Usage of the install tool

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

## 11. Troubleshooting

* **`no SYSTEM-8 / SYSTEM-1 port found`**: check `amidi -l` and pass `--port`.
  The right port is the device's second one (`SYSTEM-8 CTRL`, usually
  `hw:N,0,1` on Linux). The first port does not answer.
* **`no reply on 71 00 02 00`**: wrong port, different device ID, or the synth
  is not in normal mode.
* **`device refused the transfer`** / **`device rejected data record`**: the
  device rejected the start or a block; check that the file matches the slot
  (`Code8_<slot>.Dat`).
* Use `-v` to see the full traffic.

## 12. Limitations

* The patch transfer has been tested against a simulated device and on the real SYSTEM-8; it follows the plugin's
  code ([`docs/vol4_firmware_container_protocol/usb_sysex_protocol_spec.md`](docs/vol4_firmware_container_protocol/usb_sysex_protocol_spec.md) §6).
* Patches are sent without flow control (as the plugin does) and, by default, without delays. If the device drops
  data, use `--gap-ms 10 --wire-pacing`.
* Do not run two tools against the device at once: they share one MIDI port and their replies mix.
