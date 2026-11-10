# Volume I — Chapter 1: System Architecture — Hardware, What Code Runs Where, and Our Tools (SYSTEM-8 and SYSTEM-1)

> **Scope:** §1–§5 SYSTEM-8; §6 SYSTEM-1. Common to both: the DSP instruction set ([Vol. II](../vol2_dsp_core_isa/core_architecture.md))
> and the PLUG-OUT model of two regions (region 0 on the master, region 1 on the slaves).

Summary to find one's way. The detail and the tests are in [Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md) §8 (anatomy of the
SYSTEM-8 OS), [Vol. IV ch. 2](../vol4_firmware_container_protocol/plugout_virtual_machine.md) (PLUG-OUT interfaces, microcode). State: 2026-10-15.

## 1. Hardware components (SYSTEM-8)

```
                      USB (to the PC): MIDI 1 / MIDI 2, USB audio 96 kHz, installation of PLUG-OUTs
                                │
 ┌──────────────────────────────┴───────────────────────────────┐
 │ BMC #0 "Master"  (Roland's own chip)                          │
 │  ARM core 0 ── the OS: μT-Kernel, panel, menus, MIDI, USB,   │
 │                SD, patches, sequencer, PLUG-OUT management    │
 │                + REGION 0 of the PLUG-OUT (the "model")       │
 │  ARM core 1 ── DSP control + global effects of the SYSTEM-8   │
 │  DSP0..DSP5 + DSPM ── reverb, delays, chorus, MFX, master     │
 │  SDRAM 0x60000000 (running OS), QSPI flash, SD, DAC/amp       │
 └──────┬──────────────────────────────┬────────────────────────┘
        │ internal USB host "BMCBulk"   │ UART
        │ (boot images,                 │
        │  parameters, notes)           │
 ┌──────┴─────────────────────┐   ┌─────┴──────────────────────┐
 │ BMC #1 "Slave0" (voices 0-3)│   │ STM32F103 "Kbd" (panel)    │
 │ BMC #2 "Slave1" (voices 4-7)│   │  keys, knobs, sliders,     │
 │  ARM: s_boot / s_data       │   │  LEDs, keyboard            │
 │   + REGION 1 of the PLUG-OUT│   └────────────────────────────┘
 │     (the "engine")          │
 │  DSP0..DSP5 + DSPM: VOICES  │
 │  RAM 0x60000000-0x61000000  │
 └─────────────────────────────┘
```

* **BMC**: Roland chip with ARM Cortex-M cores, several own DSPs (6 + DSPM), DMA, USB host and memory controller.
  There are **three**: one master and two slaves, identical in hardware, with different programs.
* **DSP**: Roland's own audio processor, with its own ISA (16-bit microcode, VLIW, numbers in a sign-magnitude RAW
  format with an exponent). It runs its whole program once per sample. **It is not ARM.** The ARM of its BMC loads
  programs and parameters into it by writing to its memory (§4).
* **STM32F1**: microcontroller of the panel. The master's OS reprograms it over UART if needed.
* Audio: the voices come out of the DSPs of the slaves. The effects and the master section are in the DSPs of the
  master, and from there to the DAC (WM2418) and to USB audio. Which link carries the audio of the slaves to the
  master was not investigated.

## 2. What code runs on each processor (SYSTEM-8)

| Processor | Code | Where it lives / where it comes from |
|---|---|---|
| Master, ARM core 0 | **OS** (`m0_core0`): μT-Kernel, UI, MIDI (router `0x6000345C`), USB, SD, installation and loading of PLUG-OUTs, parameter kernel `0x01000400` | in place at `0x60000000`; kernel at `0x01000000` |
| Master, ARM core 0 | **Region 0 of the PLUG-OUT** ("model": parameter list, note assigner, slot memory). Ours: `examples/from_scratch/model/model.c` | `0x61600000 + 0x200000·slot` (slot 3 = `0x61C00000`) |
| Master, ARM core 1 | `m1_tcm` + `m1_sdram`: control of the master's DSPs, global effects (`CDSPSystem8Rev`, `…Dly…`, `…Mst`), DAC/amp | `0x01100000` (TCM of core 1) and `0x61000000` |
| Master, DSP0..5 | microcode of the global effects (e.g. DSP4 slot `0xEFDC`) | loaded by core 1 |
| Slave0 / Slave1, ARM | `s_boot` + `s_data`: DSP control, reception of parameters (`UpdateSynthParam` `0x01103F2C`), services `0x0113FC00` for the engine | sent by the master at boot: `s_boot` → `0x01100000`, `s_data` → `0x60000000` **of the slave** |
| Slave0 / Slave1, ARM | **Region 1 of the PLUG-OUT** ("engine": from the parameter to the DSP values, loading of microcode). Ours: `examples/from_scratch/engine/engine.c` | `0x60800000 + 0x200000·slot` **of the slave** (slot 3 = `0x60E00000`) |
| Slave, DSP0..5 | microcode of the voices that the engine brings: DSP0/1 envelopes and LFO, DSP2/3 oscillators (slots DCO1 `0x6000`, DCO2 `0xB000`), DSP4/5 filter and VCA (DSP4 slot `0xB000` = effect of the PLUG-OUT) | copied by the slave's ARM to program memory |
| STM32F1 | `subcpu` (panel) | `0x08000000` |

**Frequent trap: the same address means different things on each chip.** `0x60000000` is the OS on the master and
`s_data` on a slave; `0x01100000` is `m1_tcm` on the master's core 1 and `s_boot` on a slave. That is why the engine
(region 1) **cannot call the master's OS**: it jumps into other code and hangs. It can, however, call the slave's
services (`0x0113FC00`). Each slave runs its own copy of the engine.

## 3. How they connect (flows) (SYSTEM-8)

| Flow | Path |
|---|---|
| Installation of a PLUG-OUT | PC → USB MIDI 2 → master's OS → QSPI. On load: region 1 → internal USB → RAM of each slave; region 0 → master's SDRAM; then `init` (SYNINFO `+0x24`) |
| MIDI note or key | master's OS → assigner of region 0 (`note_on`) → `set(part, group, id, value)` of the kernel (voice ids `0x429+v`, `0x43A+v`) → the master sends ranges of modified ids → the slave calls the engine's `vt+0x30(id, value)` → the engine writes DSP parameters (`+0xA0`/`+0xA4`/`+0xB0`) |
| Panel knob | STM32 → OS → parameter kernel (patch ids `0x6CD…`) → slave → engine `vt+0x30` |
| Microcode of the voices | engine `vt+8`: registers programs (`+0x28/+0x40/+0x58`); the slave copies them to the program memory of its DSPs; `+0x70` changes the program of a slot while running |
| Audio | DSPs of the slaves (voices) → master (effects, master) → DAC / USB audio (`arecord -D hw:SYSTEM8,0`). Slave 1 arrives through slave 0: its DSP4 adds the IOMD inputs `6000`–`6003` to its output `6020`/`6021` [measured 2026-10-22, [S8 voice-allocation log](../vol7_cleanroom_evidence_archive/historical_lab_logs/s8_voice_allocation_log.md) §6.7] |
| MIDI to the PC | only from the master: region 0 or hooks calling the OS router. The slaves have **no** output |

## 4. Memory map of a DSP (SYSTEM-8; the same on master and slaves, each sees its own)

Capacity of each DSP: ≈ 925–945 instructions per sample (1 cycle each, whatever their content; going over hangs the
slave), up to 3 ops and 6 transfers per instruction, program of up to 64 KiB in main + slots ([Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §1,
[Vol. III ch. 3](../vol3_voice_allocation_scheduling/slot_program_model.md) §2). SYSTEM-1: §6.

| Address (DSP n = 0..5) | Content | The ARM can |
|---|---|---|
| `0x41400000 + n·0x100000` (+`0x2000` SD31, +`0x4000` RAW) | bank 0 (DT0): data RAM of the program (refs `02nn`/`04nn`/`06nn`), a **ring** that rotates one word per sample (the DSPs run at 96 kHz) | read (writing does not stay) |
| `0x41480000 + n·0x100000` | **program memory** (+ slot address in bytes; 64 KiB; main `0x0100`, slots `0x6000`/`0x8000`/`0xB000`: [Vol. III ch. 3](../vol3_voice_allocation_scheduling/slot_program_model.md) §2) | read and write (32-bit stores) |
| `0x41104318 + n·0x1000` | control and task bit of each slot | write |
| `0x41114000 + n·0x4000` | bank 5 / DPC (refs `A0nn`, `+0xB0`) | read and write |
| `0x41400000 + n·0x100000 + 0xA0000` | ramped parameter block (refs `C0nn`, `+0xA0`) | write via the OS |

## 5. Our tools and what they touch (SYSTEM-8)

| Tool | Runs on | What it touches | What for |
|---|---|---|---|
| `tools/plugout_tool.py install` | PC | master's OS (USB MIDI 2) | install PLUG-OUTs |
| `examples/midi_send/bulk_dump.py` / `bulk_loop.py` | hook in the `init` of region 0 (master core 0) | reads memory **of the master** and sends it by SysEx (one installation per dump) | dumps of the OS, flash, the master's DSPs |
| `re/dspaudio.py` (`examples/from_scratch`) | PC + engine `-DDSPTEST` on the **slaves** | test programs in the DCO2 slot of DSP2/DSP3 of the slaves; measurement by USB audio | DSP ISA by audio (one installation per batch) |
| `re/dsplive.py` | PC + engine `-DDSPLIVE` on the **slaves** | writes live the program memory of DSP2/DSP3 of the slaves (pitch bend + CC1 → ids `0x465`/`0x467` → engine) | ISA by audio without reinstalling |
| `re/dsppeek.py` | PC + region 0 `-DPEEK` on the **master** (core 0) | reads and writes the master's memory live, including its DSPs; answers by SysEx | exact register values (master's DSPs) |
| `re/dspmaster.py`, `re/dspprobe.py`, `re/dspfit.py`, `re/dspclass.py`, `re/dspslots.py`, `re/dspcount.py` | PC + region 0 `-DPEEK` | test programs on the **master's DSP3** (idle; default) or in slot `0xEFDC` of DSP4 (shared with the effects: touching it can silence the output until a restart), results read from DT0 | exact ISA ([Vol. II ch. 7](../vol2_dsp_core_isa/isa_overview.md)) |
| `emu/dspref.py` (+ `emu/dspdec.h`, `emu/dspcore.h`) | PC | nothing (the C reference: decoder with `re/dsp_templates.txt` and simulator; disassembler: `asm/disasm.py`) | read and simulate the microcode |

The important distinction: **dsplive** acts on the DSPs of the **slaves** (voices, observed by audio) and **dsppeek**
on those of the **master** (effects, read directly). There is no known way to read the memory of a slave from the PC.

## 6. SYSTEM-1

State: 2026-10-22, capacity figures 2026-10-28. Sources: OS 1.30 ([Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md)
§9), the SYSTEM-1 PLUG-OUTs SH-2 / SH-101 / PROMARS (read only) and what was measured with C-S1V4
([examples/system_1/README.md](../../examples/system_1/README.md)). Voices: [Vol. III ch. 2](../vol3_voice_allocation_scheduling/system1_voice_allocation.md);
audio path: [ch. 3](bus_interconnect.md) Part B; program memory: [Vol. III ch. 3](../vol3_voice_allocation_scheduling/slot_program_model.md).
Tags: **[measured]** · **[code]** · **[inferred]** · **[open]**.

### 6.1 Hardware components

```
                 USB (to the PC): MIDI 1 (hw:N,0,0) / MIDI 2 (hw:N,0,1), PLUG-OUT install
                                │
 ┌──────────────────────────────┴───────────────────────────────┐
 │ "master" BMC                                                  │
 │  ARM (Cortex-M, no FPU) ── the OS: panel, MIDI, USB, patches, │
 │      PLUG-OUT management + PLUG-OUT REGION 0 ("model")        │
 │  DSP A, DSP B ── per-voice modulation, mix, output (+effects) │
 └──────┬────────────────────────────────────────────────────────┘
        │ slave image, parameters, notes; wave bus (IOMD 6006–6021)
 ┌──────┴─────────────────────────────┐
 │ "slave" BMC (the 4 voices)         │
 │  ARM (no FPU): slave OS            │
 │   + PLUG-OUT REGION 1 ("engine")   │
 │  DSP A, DSP B: VOICES              │
 └────────────────────────────────────┘
```

| | SYSTEM-8 | SYSTEM-1 |
|---|---|---|
| BMC | master + 2 slaves | **master + 1 slave** (one slave image, one region 1) [code] |
| DSPs per BMC | 6 + DSPM | **2 (A, B)** (`FC30`/`FC34`; accesses with `sel` 0/1) [code] |
| ARM | Cortex-M with VFP (core 0/1 on the master) | **Cortex-M without FPU**: no binary uses VFP [code]; one core on the master [inferred: a single application program] |
| voices of the stock synth | 8 | 4 (own PLUG-OUTs: 8, C-S1V8) |
| PLUG-OUT slots | 3 | 1 |
| sampling | 96 kHz | 96 kHz [inferred: the saw calibrated at 96 kHz sounds in tune, by ear] |
| panel | STM32F103 over UART | [open]: the updater brings no image of another micro ([Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md) §9.2) |

### 6.2 Capacity of the slave DSPs (what fits in a PLUG-OUT)

| resource | DSP A | DSP B | certainty / detail |
|---|---|---|---|
| time per sample (96 kHz) | **703** instructions ran, 826 hung | **708** ran (no limit searched) | [measured], [examples/system_1_synth/README.md](../../examples/system_1_synth/README.md) §4. 1 cycle per instruction, whatever its content (measured on the SYSTEM-8, [Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §1) |
| main program | **6 KiB** (`0x0100`–`0x18FF`); going over hangs | same | [measured 2026-10-28], [Vol. III ch. 3](../vol3_voice_allocation_scheduling/slot_program_model.md) §3 |
| total program memory | 32 KiB: main + sections 2–5 (~6 KiB each) + closing. An own section is placed with `FC40` (with `FC30` it is copied but does not run) and switched on with `FC44(i, 1)`; it runs after the main program and adds to its time | same | [measured 2026-10-28], [Vol. III ch. 3](../vol3_voice_allocation_scheduling/slot_program_model.md) §3.2 |
| size of an instruction | 2–62 bytes; ≈ 8–10 in ours: 6 KiB are ≈ 600–750 instructions | | [measured] |
| ops per instruction | up to 3 (only the groups of `asm/proven_forms.txt`) and 3 transfers, with the rules R2–R8 | | [measured], `asm/s1rules.py`, [Vol. III ch. 6](../vol3_voice_allocation_scheduling/microcode_optimization.md) |
| registers | r0–r7 shared by all the programs of the DSP; two halves r0–r3 / r4–r7 (two voices in pairs, [examples/system_s1cc/README.md](../../examples/system_s1cc/README.md) §4.2) | | [measured] |
| data memory | bank 0 (ring, 0x800 words), 1, 5 (0x400, the one the ARM and the DSPs write: `A100`–`A1FF` for the DSPs), 6 (parameters, read-only) | | [measured], [Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §7, [Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §7 |

The build of SYSTEM-S1CC applies these limits (`make_s1cc.py`: `BUDGET_A` / `BUDGET_B` / `MAIN_BYTES`) and chooses by
itself how many voices fit.

### 6.3 Floating point: the SYSTEM-8 has an FPU, the SYSTEM-1 does not [code]

The ARMs of the SYSTEM-8 BMCs have a floating-point unit (VFP, single precision) and all their code uses it; those of
the SYSTEM-1 do not use it in any binary. Count of VFP instructions (`python3 system-1/vfp_count.py`; data gives by
chance ≈ 1 per 10 KiB, which is the noise):

| image | VFP instructions | per 10 KiB |
|---|---|---|
| SYSTEM-8, OS (master + slaves, `dumps/os_full.bin`) | 3054 | 10.6 |
| SYSTEM-8, JX-3P region 1 (engine of the slaves) | 679 | 24.6 |
| SYSTEM-8, SH-2 region 1 | 992 | 47.5 |
| SYSTEM-8, JX-3P region 0 (master) | 0 | 0 (no floating-point arithmetic, but the chip has an FPU) |
| SYSTEM-1, OS 1.30 master | 3 | 0.0 |
| SYSTEM-1, OS 1.30 slave | 24 | 0.7 |
| SYSTEM-1, SH-2 / PROMARS regions 0 and 1 | 5–8 | 0.2–0.4 |

The exact chip of the BMCs is not known (they are Roland's); hence "with VFP" and "without VFP", not Cortex-M4 versus
Cortex-M3. That the SYSTEM-1 has no FPU is **[inferred]**: no code uses it, but executing a VFP instruction on the
device has not been tried (without an FPU it would raise a usage fault and probably hang the chip).

Consequences for our PLUG-OUTs:

* Compile for the SYSTEM-1 with `-mcpu=cortex-m3 -mfloat-abi=soft` (`examples/system_1/build.sh`); for the SYSTEM-8
  with `-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16` (`examples/from_scratch/engine/build.sh`).
* On the SYSTEM-1 the values that go to the DSPs are still IEEE `float` (the services `FD6C`, `FCEC` take them as bit
  patterns in integer registers); C-S1V4 precomputes them on the PC (`make_s1.py`). `float` arithmetic on the
  SYSTEM-1 ARM is possible with gcc's software emulation (`libgcc`: `__aeabi_fmul`, …), slower.
* The ARM code of a SYSTEM-8 PLUG-OUT (e.g. the JX-3P engine) **does not run on the SYSTEM-1** as is: besides another
  interface and other services, it uses VFP. It has to be ported or rewritten
  ([Vol. III ch. 9](../vol3_voice_allocation_scheduling/system1_polyphony_studies.md) §3).
* The DSPs are not affected: they do their own arithmetic ([Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md)), the same on both devices.

### 6.4 What code runs on each processor

| Processor | Code | Where it lives |
|---|---|---|
| master, ARM | **OS** (`"SYSTEM-1:Appli"`, `0x60000000`), fast code at `0x01000300`, **services for PLUG-OUTs at `0x0100FC00`** (162 entries) | flash `SYSTEM1_UPD.BIN` + `0x30000` |
| master, ARM | **PLUG-OUT region 0**: SYNINFO, assigner (`+0x2C`), model object (`+0x30`: loads the master's DSPs, receives the parameters), 64 patches at `+0x1000` | **`0x60800000`** |
| master, DSP A/B | per-voice modulation, mixer, output (`0x82` in PLUG-OUT mode); in PLUG-OUT mode DSP B is set by the PLUG-OUT (`0x1000`) | loaded by the OS / region 0 |
| slave, ARM | slave OS (`0x60000000` of the slave), fast code `0x01000300`, **services `0x0100FC00`** (the same addresses as on the master, other code) | flash + `0x130000`, sent by the master |
| slave, ARM | **PLUG-OUT region 1**: SYNINFO, engine (`+0x30`; vtable of 22: `load`, `refresh`, `param`) | **`0x60200000`** of the slave |
| slave, DSP A/B | voices: stock main + 4 sections per DSP; PLUG-OUTs their own (C-S1V4: everything in the main program of A, output in B) | loaded by the slave's ARM |

**The same trap as on the SYSTEM-8**: `0x60000000` and `0x0100FC00` exist on both chips with different contents;
region 1 can only call the slave's services, and only region 0 (master) can send MIDI. Region 0 has a model object
that receives the parameters (`vt+0x30`); on the SYSTEM-8 that point does not exist, which matters for capturing
parameters ([examples/system_1/README.md](../../examples/system_1/README.md) §6.3). There is a single slot, so there is
no relocation.

### 6.5 How they connect (flows)

| flow | path |
|---|---|
| install | PC → USB MIDI 2 → master OS → flash (`Code1.Dat`, 2048 records, `70 10 00 00`). On load: region 1 → slave, region 0 → `0x60800000`; the OS checks SYNINFO (`+0x000`, `+0x1B4`) and calls `create` (`+0x24`) on both [measured: C-S1V4 accepted] |
| switching to the PLUG-OUT | master: `get_b()` → `vt[0]`, `get_a()` = assigner; master queues emptied, the OS's PLUG-OUT-mode sets, `model->vt[2]` (load). Slave: `get_b()` → `vt[0]`, queues emptied, sections switched off, `engine->vt[2]` [code, measured] |
| note | OS → assigner `note_on` → `set(2, 0x110+v / 0x120+v)` → master (its DSP) and slave (`engine vt+0x30(id, 0, value)`) → the engine writes bank 5 (`FD6C`) [measured] |
| panel control | OS → `model vt+0x30(id, flag, value)` on the master (VOLUME = `0x100`) and, for groups 0–3, `engine vt+0x30` on the slave [measured with PARAMLOG] |
| panel values when entering the PLUG-OUT | the OS does not resend them; the model does `touch(3, id)` of `0x160`–`0x18F` and `flush(3)` in `load` and the OS sends them to the engine (`examples/system_s1cc/model.c resend_panel`) [measured 2026-10-28: the synth's controls are group 3] |
| audio | slave A → B (bank 5) → IOMD `6006`–`600D` → master B → `A118`/`A119` → master A (`0x82`) → `6000`/`6001` → output [measured] |
| modulation (stock synth) | master (program of 696 instructions) → IOMD `600E`–`6021` → slave A [code] |
| MIDI to the PC | only from the master: region 0 can use the OS's send (`0x600065B0`, object `0x605F20D4`, cable 3, mask 1) → goes out on MIDI 1 [measured] |

### 6.6 DSP memory seen from the slave's ARM [code, partial]

| address | what | state |
|---|---|---|
| `0x40101000 + 4·i` | bank 5 (`A000+i`), IEEE view; `+0x400` = `A100+i` (`FD6C`, `sel` 1) | `A0xx` [measured with C-S1V4]; `A1xx` [inferred] |
| `0x4014C000 + 16·id` / `0x4016C000 + 16·id` | parameters (bank 6, `C0id`) of DSP A / B (`FCEC` immediate, `FCE4` with ramp: `mode<<28 \| …`) | [code]; `FCEC` on the master [measured: VOLUME] |
| `0x40140000`, `0x40160000` (+`0xE000`, +`0x10000`) | blocks of each DSP; `+0x10000` is the destination of DMA copies in blocks of `0x200` (program or data memory) | [open] |
| `0x40100000` | common control | [open] |

Program memory (sections, closings): [Vol. III ch. 3](../vol3_voice_allocation_scheduling/slot_program_model.md) §3.

### 6.7 Tools for the SYSTEM-1

| tool | runs on | touches | for |
|---|---|---|---|
| `tools/plugout_tool.py info / install` | PC | master OS (MIDI 2) | install in slot 1 (`--slot 1 --force --no-patches`) |
| `tools/plugout_build.py unpack / pack / show` | PC | `Code1.Dat` | unpack and build (exact round trip with the SH-2) |
| `examples/system_1/build.sh` (`TONE=1`, `PARAMLOG=1`) | PC | builds C-S1V4 / C-S1TONE / C-S1LOG | own PLUG-OUT and tests |
| `examples/system_1/make_s1.py --sim` | PC | nothing (simulates slave A, B and master B with the C reference of the DSP, `emu/dspref.py`) | check the programs before installing |
| `examples/system_1/paramlog.py` | PC + `PARAMLOG` build on the master | listens on MIDI 1/2 | which id each control sends ([examples/system_1/README.md](../../examples/system_1/README.md) §6) |
| `tools/s1_capture.py` | PC | USB audio + MIDI | capture the output and send notes ([Vol. V ch. 6](../vol5_toolchain_compiler_sdk/audio_capture.md)) |
| `system-1/extract_fw.py`, `voice_sets.py`, `fnmatch.py`, `asgn_sites.py`, `s1_progs.py`, `dsp_fit.py`, `vfp_count.py` | PC | OS and PLUG-OUT images | static study ([system-1/README.md](../../system-1/README.md)) |

Not available on the SYSTEM-1: reading DSP memory from the PC (like the SYSTEM-8's `dsppeek`) and live writing
(`dsplive`).

> Paths `re/…` and `emu/…` in §5 are relative to `examples/from_scratch/`.
