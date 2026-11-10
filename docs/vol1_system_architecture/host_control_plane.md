# Volume I — Chapter 4: The ARM Side — Processors, Programs, RTOS, FPU and Panel CPU

> **Scope:** both (each row says which device).

> What is known of the ARM side of both devices; every row names its source. The full analyses are
> [Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md) §8 (anatomy of the SYSTEM-8 OS),
> [ch. 1](soc_overview.md) §2 and [Vol. I ch. 1](soc_overview.md) §6 (SYSTEM-1). The exact Cortex-M
> core of the BMC is not known (§4).

## 1. SYSTEM-8: the six programs of `App1_Main`

`App1_Main` is not one program but a bundle with its own loader, six programs for four kinds of CPU (base_firmware
§8.1, from the dump `dumps/os_full.bin` analysed with `tools/os_analyze.py`):

| part | in the image | size | runs at | CPU | content |
|---|---|---|---|---|---|
| `m0_core0` | `0x60000000` | `0x130E00` | in place | master BMC, core 0 | the OS |
| `m1_tcm` | `0x60130E00` | `0x42BBC` | `0x01100000` | master BMC, core 1 | DSP control + global effects |
| `m1_sdram` | `0x60173A00` | `0x7D74C` | `0x61000000` | master BMC, core 1 | debug console screens (`CEZUtil*`), MIDI→DSP command parser |
| `s_boot` | `0x601F1200` | `0x1D328` | `0x01100000` (on the slave) | slave BMCs | `BOOT` image, DSP control |
| `s_data` | `0x6020E600` | `0x7D114` | `0x60000000` (on the slave) | slave BMCs | `DATA` image, debug console screens |
| `subcpu` | `0x6028B730` | `0x4D68` | `0x08000000` | STM32F1 | panel/keyboard controller firmware |

Who runs what, including the PLUG-OUT regions, is the table of [ch. 1](soc_overview.md) §2: region 0 (model) on the
master's core 0 at `0x61600000 + 0x200000·slot`, region 1 (engine) on the slave BMCs only at
`0x60800000 + 0x200000·slot` (proven in base_firmware §8.13).

**Start-up** (base_firmware §8.3): core 0 copies `m1_sdram` to `0x61000000` and `m1_tcm` to `0x01100000`, then sends the
slave images through the internal USB host, each as a 16-byte header `{'BUDB', length, load address, 'DATA'|'BOOT'}`
plus the data (kernel function `0x010125C1`): `DATA` → slave `0x60000000`, `BOOT` → slave `0x01100000`; it checks the
panel CPU's version and reflashes it through the STM32 UART system bootloader if needed.

## 2. RTOS and the roles of each core (SYSTEM-8)

* **Core 0, the OS proper** (base_firmware §8.4): RTOS **μT-Kernel** (T-Engine), seen in
  `## MemPool Usage Report uTKernel Task ##` and `## Kernel Resource Usage Report ##` with `tsk sem flg mbx mtx mbf por
  mpl mpf cyc alm`. Kernel services are reached through veneers `0x600753xx` → `0x0100xxxx`. Task/object names seen:
  `System`, `Idle`, `MidiIn`, `MidiOut`, `MidiOutUsb`, `KbdRx`, `KbdWdg`, `Calb`, `QSPIDlyP`. Also: table-driven UI,
  SD card (FAT, drive `A:`), sequencer/arpeggiator, panel monitor, PLUG-OUT management, MIDI router `0x6000345C`,
  test/factory menus with analog calibration. The kernel RAM (`0x01000400–0x010143AC`) was dumped live
  (base_firmware §8.11).
* **Core 1** (base_firmware §8.5): `CBmcDspMgr`, one `CFSPDrv*` driver per DSP pair, inter-processor communication
  (`CPIPCDev/Drv`), the internal MIDI/parameter stream "UMDW", the SYSTEM-8 global effects and master section
  (`CDSPSystem8Rev`, `…Dly*`, `CDSPSystem8Mst`, parameter class `CPrmDSPSystem8`), DAC/amp enable and mute. DSP register
  block `0x41100000` (`+0x2000…+0x9000` per DSP).
* **Slaves** (base_firmware §8.6): same software family as core 1 but with generic per-DSP parameter classes
  (`CDSPPrm`, `CDSP0Prm…CDSP5Prm`, `CDSPMPrm`) and texts `UpdateSynthParam: no plugout.`: the slaves produce the voices,
  with the engine supplied by the PLUG-OUT. Services for region 1 at `0x0113FC00`
  ([Vol. IV ch. 3](../vol4_firmware_container_protocol/os_service_table.md)).
* **Master ↔ slaves**: internal USB, the master is the host (EHCI/OHCI pages `0x40060000`/`0x40064000`), the slaves
  appear as `BMCBulk` devices (base_firmware §8.2). MIDI and SysEx over that link: [ch. 5](midi_sysex_routing.md).

The cyclic handler used for a 1 ms tick was found and measured on the **SYSTEM-1** slave ([ch. 6](s1_arm_clock.md));
on the SYSTEM-8 it has not been looked for.

## 3. Panel CPU (SYSTEM-8): STM32F1 (`subcpu`)

From base_firmware §8.7: vector table at `0x6028B730` (SP `0x200023B8`, reset `0x0800035D`). Peripherals used: GPIOA–E
(`0x40010800…0x40011800`, so a 100-pin package), ADC1 `0x40012400`, TIM1 `0x40012C00`, TIM8 `0x40013400` (TIM8 →
high-density F103), TIM4 `0x40000800`, **USART1 `0x40013800` (link to the master)**, RCC `0x40021000`, FLASH
`0x40022000`. 133 functions, almost no strings. It scans switches/encoders/sliders, drives LEDs and reads the keyboard;
the master side runs the tasks `KbdRx`/`KbdWdg` and can reflash it. The packet format on the UART is not documented.

SYSTEM-1: the updater brings no image of another micro; how its panel is read is open
([Vol. I ch. 1](soc_overview.md) §6.1).

## 4. Floating point: the SYSTEM-8 has an FPU, the SYSTEM-1 does not

Translated from [Vol. I ch. 1](soc_overview.md) §6.3 ([code]):

The ARMs of the SYSTEM-8 BMCs have a floating-point unit (VFP, single precision) and all their code uses it; those of the
SYSTEM-1 do not use it in any binary. Count of VFP instructions (`python3 system-1/vfp_count.py`; random data gives ≈ 1
per 10 KiB, which is the noise):

| image | VFP instructions | per 10 KiB |
|---|---|---|
| SYSTEM-8, OS (master + slaves, `dumps/os_full.bin`) | 3054 | 10.6 |
| SYSTEM-8, JX-3P region 1 (engine of the slaves) | 679 | 24.6 |
| SYSTEM-8, SH-2 region 1 | 992 | 47.5 |
| SYSTEM-8, JX-3P region 0 (master) | 0 | 0 (it does no floating-point arithmetic, but the chip has an FPU) |
| SYSTEM-1, OS 1.30 master | 3 | 0.0 |
| SYSTEM-1, OS 1.30 slave | 24 | 0.7 |
| SYSTEM-1, SH-2 / PROMARS regions 0 and 1 | 5–8 | 0.2–0.4 |

The exact chip of the BMCs is not known (they are Roland's); that is why one speaks of "with VFP" and "without VFP",
and not of Cortex-M4 versus Cortex-M3. That the SYSTEM-1 has no FPU is **[inferred]**: no code uses it, but executing a
VFP instruction on the device has not been tried (if there is no FPU, it would cause a usage fault and probably hang
the chip).

Consequences for our PLUG-OUTs:

* Build for the SYSTEM-1 with `-mcpu=cortex-m3 -mfloat-abi=soft` (`examples/system_1/build.sh`); for the SYSTEM-8 with
  `-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16` (`examples/from_scratch/engine/build.sh`). These are our build
  flags, chosen to match "with / without VFP"; they do not identify the core.
* On the SYSTEM-1 the values that go to the DSPs are still IEEE `float`s (the services `FD6C`, `FCEC` receive them as bit
  patterns in integer registers); C-S1V4 precomputes them on the PC (`make_s1.py`). Doing `float` arithmetic on the
  SYSTEM-1 ARM is possible with gcc's software emulation (`libgcc`: `__aeabi_fmul`, …), slower.
* The ARM code of a SYSTEM-8 PLUG-OUT (e.g. the JX-3P engine) **does not run on the SYSTEM-1** as it is: besides
  another interface and other services, it uses VFP. It has to be ported or rewritten ([Vol. III ch. 9](../vol3_voice_allocation_scheduling/system1_polyphony_studies.md) §3).
* The DSPs are not affected: they do their own arithmetic ([Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md)),
  the same on both devices.

## 5. SYSTEM-1: code on each processor

Translated from [Vol. I ch. 1](soc_overview.md) §6.4:

| processor | code | where it lives |
|---|---|---|
| master, ARM | **OS** (`"SYSTEM-1:Appli"`, `0x60000000`), fast code at `0x01000300`, **services for PLUG-OUTs at `0x0100FC00`** (162 entries) | flash `SYSTEM1_UPD.BIN` + `0x30000` |
| master, ARM | **region 0 of the PLUG-OUT**: SYNINFO, assigner (`+0x2C`), model object (`+0x30`: loads the master's DSPs, receives the parameters), 64 patches at `+0x1000` | **`0x60800000`** |
| master, DSP A/B | per-voice modulation, mixer, output (`0x82` in PLUG-OUT mode); in PLUG-OUT mode DSP B is set by the PLUG-OUT (`0x1000`) | loaded by the OS / region 0 |
| slave, ARM | slave OS (`0x60000000` of the slave), fast code `0x01000300`, **services `0x0100FC00`** (the same addresses as on the master, other code) | flash + `0x130000`, sent by the master |
| slave, ARM | **region 1 of the PLUG-OUT**: SYNINFO, engine (`+0x30`; vtable of 22: `load`, `refresh`, `param`) | **`0x60200000`** of the slave |
| slave, DSP A/B | voices: stock main + 4 sections per DSP; PLUG-OUTs their own (C-S1V4: everything in the main of A, output in B) | loaded by the slave's ARM |

**The same trap as on the SYSTEM-8**: `0x60000000` and `0x0100FC00` exist on both chips with different content; region
1 can only call the slave's services, and only region 0 (master) can send MIDI.

The SYSTEM-1 slave ARM's clock: the cycles per ms read from the OS's SysTick (`LOAD + 1` = 156 000 per ms) give an ARM of
156 MHz, assuming the SysTick runs at 1 kHz [open] ([examples/system_1_ramp/README.md](../../examples/system_1_ramp/README.md)
§1). Its 1 ms tick: [ch. 6](s1_arm_clock.md).
