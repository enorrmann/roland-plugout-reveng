# Volume I — Chapter 2: Memory Maps (ARM address spaces and DSP windows)

> **Scope:** both (each table says which chip and device).

> The addresses of both devices in one place, each with its source. The same address means different things on
> different chips: every table says which chip it is for. Sources: [ch. 1](soc_overview.md) §2, §4,
> [Vol. IV ch. 1](../vol4_firmware_container_protocol/container_formats.md) §4,
> [Vol. IV ch. 2](../vol4_firmware_container_protocol/plugout_virtual_machine.md) §7,
> [Vol. IV ch. 6](../vol4_firmware_container_protocol/base_firmware.md) §6.2, §8,
> [Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §1 and
> [Vol. I ch. 1](soc_overview.md) §6.4, §4.

## 1. The trap: addresses are per chip

`0x60000000` is the OS on the SYSTEM-8 master and `s_data` on a slave; `0x01100000` is `m1_tcm` on the master's core 1
and `s_boot` on a slave. That is why the engine (region 1) cannot call the master's OS: it jumps into other code and
hangs; it can call the slave's services (`0x0113FC00`) ([ch. 1](soc_overview.md) §2). The same holds on the SYSTEM-1
for `0x60000000` and `0x0100FC00` ([Vol. I ch. 1](soc_overview.md) §6.4).

## 2. SYSTEM-8, ARM address spaces

### 2.1 Master BMC

| address | what | source |
|---|---|---|
| `0x00000000` (aliased with `0x01000000`) | vector table: SP `0x01050800`, reset `0x60000001`; faults and IRQ0 point at `b .` stubs (any fault freezes the synth) | base_firmware §6.2 |
| `0x01000000` – `0x01019A08` | "kernel": RAM filled at start-up from the OS image (vector table `0x400` B, code and data from `0x01000400`, `.bss` from `0x010143AC`); not a separate ROM | base_firmware §6.2, §8.11 |
| `0x01000400` | parameter kernel (`set(part, group, id, value)`) | ch. 1 §2 |
| `0x01100000` | `m1_tcm` (core 1 TCM, window up to `0x01144000`) | base_firmware §8.3; ch. 1 §2 |
| `0x20000000` | `.data` / `.bss` of core 0 (`0x308` copied, `0x8740` zeroed) | base_firmware §6.2, §8.12 |
| `0x60000000` | the OS (`m0_core0`, `App1_Main` running in place) | ch. 1 §2; base_firmware §8.1 |
| `0x61000000` – `0x61600000` | `m1_sdram` (core 1) | base_firmware §8.3 |
| `0x61600000 + 0x200000·slot` | PLUG-OUT region 0 (model), slot 0 = the SYSTEM-8 itself … slot 3 = `0x61C00000` | Vol. IV ch. 1 §4 |
| `0x40001000` | memory-to-memory DMA | base_firmware §6.3, §8.8 |
| `0x40060000`, `0x40064000` | USB host EHCI / OHCI (links to the slaves) | base_firmware §8.8 |
| `0x4010B000` | SD card controller | base_firmware §8.8 |
| `0x41100000` | DSP system / register block (`+0x2000·n` per DSP) | base_firmware §8.5, §8.8 |

The other peripheral pages seen in the code (`0x40026000`, `0x40031000`, `0x40054000`, `0x40062000`, `0x40080000`,
`0xE000E000`, `0xE0043000`) and their partial identification are in base_firmware §8.8.

### 2.2 Slave BMCs

| address | what | source |
|---|---|---|
| `0x01100000` | `s_boot` (`BOOT` image sent by the master) | ch. 1 §2; base_firmware §8.3 |
| `0x0113FC00` | OS service table for the engine ([Vol. IV ch. 3](../vol4_firmware_container_protocol/os_service_table.md)) | ch. 1 §2 |
| `0x01103F2C` | `UpdateSynthParam` (reception of parameters) | ch. 1 §2 |
| `0x60000000` – `0x60800000` | `s_data` (`DATA` image) | base_firmware §8.3 |
| `0x60800000 + 0x200000·slot` | PLUG-OUT region 1 (engine), slot 3 = `0x60E00000`; RAM `0x60000000`–`0x61000000` | Vol. IV ch. 1 §4; ch. 1 §1 |

### 2.3 PLUG-OUT windows (both regions)

From Vol. IV ch. 1 §4 (run addresses from the SYNINFO pointers and the slot diffs; each window 2 MiB,
`SLOT_STRIDE = 0x200000`; the image is followed by its `.bss`):

| | slot 0 (synth) | slot 1 | slot 2 | slot 3 |
|---|---|---|---|---|
| region 0 (model), master | `0x61600000` | `0x61800000` | `0x61A00000` | `0x61C00000` |
| region 1 (engine), slaves | `0x60800000` | `0x60A00000` | `0x60C00000` | `0x60E00000` |

The region header's load/entry fields are the same in every slot build (`0x60800000` region 0, `0x60200000` region 1);
the image itself is linked per slot (Vol. IV ch. 1 §2, §4).

## 3. SYSTEM-8, DSP windows seen by the ARM

Each DSP's ARM-side windows (DSP n = 0..5), from [ch. 1](soc_overview.md) §4 and the slave map of
[Vol. IV ch. 2](../vol4_firmware_container_protocol/plugout_virtual_machine.md) §7 ("Memory map of a slave DSP",
from `s_boot`; base `0x41400000 + n·0x100000`, DSPM `0x41200000`):

| address | content | the ARM can |
|---|---|---|
| `0x41400000 + n·0x100000` (+`0x2000` SD31, +`0x4000` RAW) | bank 0 (DT0), a ring | read (writing does not stay) |
| `… + 0x10000` | bank 1 (DSP0 and DSP1 share `0x41510000`) | — |
| `… + 0x20000` | bank 2 | — |
| `… + 0x30000` | bank 4 (DSP0/1: `0x411D0000` / `0x411D8000`) | read |
| `… + 0x40000`, `+0x44000`, `+0x48000` | data RAM: the `+0x58` "data" lists, 8-byte aligned | — |
| `0x41480000 + n·0x100000` (= `… + 0x80000`) | **program memory** (+ slot address in bytes; 64 KiB; main `0x0100`, slots `0x6000` / `0x8000` / `0xB000`) | read and write (32-bit stores) |
| `… + 0xA0000` | ramped parameter block (`C0nn`), written by `+0xA0(dsp, id, ramp, flag)`: `[+0]` current value, `[+8]` control `0x08000000 \| ramp<<20 \| flag<<24 \| mode<<28`, `[+0xC]` target float | write via the OS |
| `0x41104318 + n·0x1000` | control and task bit of each slot | write |
| `0x41114000 + n·0x4000` | bank 5 / DPC (0x400 words; DSPM `0x41110000`; IEEE view, SD31 at `+0x1000`, RAW at `+0x2000`) | read and write (`+0xB0`) |
| `0x4113C000` (IEEE) / `0x4113E000` (RAW) | bank 3, IOMD, 0x100 words common to the BMC | read |
| `0x411D0000`, `0x411D8000` | MXM0 / MXM1 windows | read |
| `0x41300000` | WMEM (reading all of it with a test program in slot `0xF7DC` hung the master once) | — |
| `0x41158000`, sys + `0x40` / `+0x44` | indirect port of the EDM (delay memory, 0x200000 words per BMC) | read (ARM write not achieved) |

The complete table of DSP windows of the debug console (`CEZUtilDSPDump`, 129 entries, `m1_sdram` `0x61014BB0`) is in
[Vol. II ch. 6](../vol2_dsp_core_isa/memory_banks_dma.md) §1. Two readings of the bank-5 RAW view coexist in the
sources: `0x41114000 + n·0x4000 + 0x2000` (DSP_ARM_SIDE, the C code `engine.c live_bank5`) and "without an offset"
(the slave map above); [Vol. II ch. 11](../vol2_dsp_core_isa/slave_dsp_readback.md) §2 item 3 lists it as unresolved
for the slaves. The EDM and WMEM rows come from [Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §8.4 and
[Vol. II ch. 4](../vol2_dsp_core_isa/instruction_set_reference.md) §9.1.

## 4. SYSTEM-1, ARM address spaces

From [Vol. I ch. 1](soc_overview.md) §6.4:

| chip | address | what |
|---|---|---|
| master | `0x60000000` | OS (`"SYSTEM-1:Appli"`), from flash `SYSTEM1_UPD.BIN` + `0x30000` |
| master | `0x01000300` | fast code |
| master | `0x0100FC00` | services for PLUG-OUTs (162 entries) |
| master | `0x60800000` | region 0 (SYNINFO, assigner `+0x2C`, model object `+0x30`, 64 patches at `+0x1000`) |
| slave | `0x60000000` | slave OS, from flash + `0x130000`, sent by the master |
| slave | `0x01000300`, `0x0100FC00` | fast code, services (the same addresses as on the master, other code) |
| slave | `0x60200000` | region 1 (engine) |

A single slot, without relocation ([ch. 1](soc_overview.md) §6).

## 5. SYSTEM-1, DSP windows seen from the slave ARM [code, partial]

Translated from [Vol. I ch. 1](soc_overview.md) §6.6 (tags of that document):

| address | what | state |
|---|---|---|
| `0x40101000 + 4·i` | bank 5 (`A000+i`), IEEE view; `+0x400` = `A100+i` (`FD6C`, `sel` 1) | `A0xx` [measured with C-S1V4]; `A1xx` [inferred] |
| `0x4014C000 + 16·id` / `0x4016C000 + 16·id` | parameters (bank 6, `C0id`) of DSP A / B (`FCEC` immediate, `FCE4` with ramp: `mode<<28 \| …`) | [code]; `FCEC` on the master [measured: VOLUME] |
| `0x40140000`, `0x40160000` (+`0xE000`, +`0x10000`) | blocks of each DSP; `+0x10000` is the destination of DMA copies in blocks of `0x200` (program or data memory) | [open] |
| `0x40100000` | common control | [open] |

Measurements on the parameter block (2026-11-03, [ch. 6](s1_arm_clock.md) §2): `FCEC` writes
`[+0] = 0xE0000000, [+4] = [+8] = value, [+0xC] = 0`; `C060`–`C0FF` of the slave's DSP B arrive exactly; the hardware
ramp (`FCE4`) only exists for ids < `0x80`; writes to `C100`… are not parameters. `FCE4` writes
`[+0] = 0x80000000 | z << 28 | y << 24`, `[+4]` = target ([Vol. IV ch. 3](../vol4_firmware_container_protocol/os_service_table.md)).
On the master, all four tried paths to read bank 5 return 0 ([examples/system_1_ramp/README.md](../../examples/system_1_ramp/README.md) §1).
