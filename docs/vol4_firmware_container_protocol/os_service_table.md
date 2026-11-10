# Volume IV — Chapter 3: OS Service Tables Used by PLUG-OUT Code

> **Scope:** both: the SYSTEM-8 table at `0x0113FC00` and the SYSTEM-1 table at `0x0100FC00`, each marked.

> Every OS service that a PLUG-OUT of this repository calls or that the documentation describes, with the place
> where each fact comes from; each row was checked against the call sites in the code. "Use seen" is the static
> reading of Roland's code, "our call" is how our code calls it. Nothing here is a symbol table published by Roland:
> the names in the "name" column are the typedef names of our code, or the offsets.

## 1. What the tables are

The tables are **arrays of function pointers** at a fixed address; a PLUG-OUT calls entry `off` as
`(*(void **)(BASE + off))(…)`. The code of this repository wraps that in a macro:

```c
/* SYSTEM-8, slave (examples/from_scratch/engine/engine.c, comment: "s_boot, copied to 0x0113FC00") */
#define OS(off)  (*(void *volatile *)(0x0113FC00u + (off)))
/* SYSTEM-1, master and slave (examples/system_1/syninfo.h) */
#define SVC(off) (*(void *volatile *)(0x0100FC00u + (off)))
```

`SVC` is only the macro's name: these are plain indirect calls, not the ARM `SVC` instruction. On the SYSTEM-1 the
entries are named in the documents by their address (`FC30` = `0x0100FC30`, `FD6C` = `0x0100FD6C`, …).

| | SYSTEM-8 | SYSTEM-1 |
|---|---|---|
| base | `0x0113FC00` | `0x0100FC00` |
| where | slave BMCs (`s_boot`); used by region 1 and by the synth's own engine `p0s` ([ch. 2](plugout_virtual_machine.md) §7) | the same address on the master and on the slave, with different code behind it; 162 entries ([Vol. I ch. 1](../vol1_system_architecture/soc_overview.md) §6.4) |
| who may call it | region 1 (engine) on the slaves; it **cannot** call the master's OS ([Vol. I ch. 1](../vol1_system_architecture/soc_overview.md) §2) | region 0 uses the master's table, region 1 the slave's ([Vol. I ch. 1](../vol1_system_architecture/soc_overview.md) §6.4) |

## 2. SYSTEM-8 (`0x0113FC00`, slave)

| offset | name in our code | our call (type) | use seen in Roland's code / notes | sources |
|---|---|---|---|---|
| `+0x00` | `get_fn` | `int (*)(int part, int grp, int id)`; `os_get(grp, id)` = `get(0, grp, id)` | read at init next to the constants `4` and `0x748`; the JX-3P reads DCO1 WAVEFORM `0x6D4`, DCO2 WAVEFORM `0x6D9` and `0x748` (effect type) with `get(0, 4, id)` | `examples/from_scratch/engine/engine.c:34,43`; [ch. 2](plugout_virtual_machine.md) §7 |
| `+0x28 + 4·n` | `reg_fn` | `void (*)(const void *)` | append to one of 18 lists (`s_boot 0x01105808`, list headers `0x20084050..0x20084160`): **slot** address of DSP n | `engine.c:29,479`; ch. 2 §7 "DSP microcode in the engine" |
| `+0x40 + 4·n` | `reg_fn` | same | **program** for one slot: `&{dsp_addr, bytes, bytes, src}` | `engine.c:485`; ch. 2 §7 |
| `+0x58 + 4·n` | `reg_fn` | same | **data** for one slot (8 or 32 bytes, all zero in the JX-3P) | `engine.c:492`; ch. 2 §7 |
| `+0x70` | `swap_fn` | `void (*)(int dsp, const void *prog, const void *data, void *ctx, void *ctx2, uint32_t slot, uint32_t task, uint32_t x0, uint32_t x1)` | replace the program in one slot of a running DSP (OS `0x011062FA` → `0x01105F94`); JX-3P: `+0x70(dsp, &prog, &data, 0x60A44E18, 0x6000 or 0xB000, 2, 0, 0)` | `engine.c:36,273`; ch. 2 §7 |
| `+0xA0` | `ramp_fn` | `void (*)(int dsp, int id, int ramp, int flag, int sp, float v)` | parameter `C0id` (bank 6) with a hardware ramp; `ramp` = encoded duration, `flag` = wait for the end | `engine.c:30,39`; [Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §4.6 |
| `+0xA4` | `set_fn` | `void (*)(int dsp, int id, float v)` | parameter `C0id`, immediate | `engine.c:31,40`; Vol. I ch. 3 §4.6 |
| `+0xB0` | `bank_fn` | `void (*)(int dsp, const uint32_t *id, float v)` | bank-5 word `A000 + *id` of DSP `dsp`; the OS converts the float to RAW; the JX-3P sends its SAW/PULSE flag this way | `engine.c:32,41`; Vol. I ch. 3 §4.5; ch. 2 §7 |
| `+0xBC` | — | not called by our code | a function taking a float in `s0` | ch. 2 §7 |
| `+0xC0` | `sync_fn` | `void (*)(int a, int b, int c, int d)`; called `(3, 0, 0, 1)` before swapping the DCOs and `(4, 0, 3, 1)` before swapping the effect | Roland calls it in the same place (r0 = 3, r3 = 1; r0 = 4 for the effect): "meaning unknown" (ch. 2 §7). Our code's comment says "wait for the DSP frame": an interpretation, not a measurement | `engine.c:35,290,303`; ch. 2 §7 |
| `+0xF0` | `slave_fn` | `int (*)(void)`; `slave_index() = fn() ? 1 : 0` | called first in the engine start routine; result tested for 0 | `engine.c:33,42`; `examples/from_scratch/v3/engine_v3.c:425` (`first = 4 · slave`: the voices 4–7 of C-V4 on slave 1 are measured, [Vol. III ch. 1](../vol3_voice_allocation_scheduling/system8_voice_allocation.md)) ; ch. 2 §7 |

### 2.1 What the slave calls in the engine

The other direction of the interface: `init` (SYNINFO `+0x24` of region 1) returns a pointer `D` into the engine's
descriptor, and the slave's `UpdateSynthPlugout` (`s_boot` `0x01104226`) runs `D = init(); D[1](); D[5]->vt[0](obj);
D[0] = slot`.

| `D[n]` | content | | `vt` offset of the main object `D[5]` | called for (`examples/from_scratch/engine/engine.c`) |
|---|---|---|---|---|
| 0 | slot, written by the slave | | `+0x00` | init: the engine asks `+0xF0` which slave it is on |
| 1 | `create()` | | `+0x08` | load: register slots, programs and data (`+0x28` / `+0x40` / `+0x58`) |
| 2 | `destroy()` | | `+0x0C` | start: `+0xC0`, one `+0x70` per slot program, ramps (a slot program only runs once `+0x70` has enabled its task) |
| 3, 4 | 0 | | `+0x10` / `+0x18` | mute / unmute |
| 5 | main object (the JX-3P's is `CPrmDSPJx3pSys8`, vtable `0x60A445CC`) | | `+0x24` | refresh (argument 1 = send the whole state) |
| | | | `+0x30` | parameter: `vt[0x30](obj, id, 0, value)`, from `UpdateSynthParam` (`0x01103F2C`), for the patch ids `0x6CD…`, the voice ids `0x429…` and `0x43A…0x44A` |

The vtable has 28 entries; the ones not listed are no-ops in our engine, which runs on the device with them
(`+0x28` is also called by the slave, from `0x01105B6E`). The parameter list of region 1 (SYNINFO `+0x28`) is empty: it
points at a word `0xFFFFFFFF`. Program and data descriptors are **six words**, `{dsp_addr, bytes, bytes, src, 0, 0}`:
the OS uses the last two, so each descriptor serves one DSP and one program only (the JX-3P keeps separate tables for
DSP2 and DSP3, `0x60A0B878` / `0x60A0B87C`, and a data descriptor after each program).

The kernel's `UpdateSynthParam` at `0x01103F2C` receives the parameters on the slave
([Vol. I ch. 1](../vol1_system_architecture/soc_overview.md) §2). The master sends ranges of changed ids (the kernel
keeps the lowest and highest changed id, `0x01011B7E`) and the slave notifies every id of the range; the kernel clips
each value to the id's minimum and maximum (`0x01010C24`).

## 3. SYSTEM-1 (`0x0100FC00`)

| entry | name in our code | our call (type) | what it does | sources |
|---|---|---|---|---|
| `FC00` | `get_fn` | `int (*)(int group, int id)` | read a parameter (master: groups 1 part, 2 voice ids, 4 patch; engine `refresh` reads the current values) | `examples/system_1/model.c:27`, `engine.c:27` |
| `FC04` | `set_fn` | `void (*)(int group, int id, int value)` | set a parameter (master; the assigner sends `0x110+v` note, `0x120+v` velocity with group 2) | `examples/system_1/model.c:28` and its header comment |
| `FC08` | `flush_fn` | `void (*)(int group)` | send the modified ids of a group | `model.c:29`; [Vol. I ch. 1](../vol1_system_architecture/soc_overview.md) §6.5 (`flush(3)`) |
| `FC0C` | `touch_fn` | `void (*)(int group, int id)` | mark an id as modified (`touch(3, id)` of `0x160`–`0x18F` in `load` makes the OS resend the panel values) | `model.c:30`; [Vol. I ch. 1](../vol1_system_architecture/soc_overview.md) §6.5 |
| `FC30` | `s1_queue_fn` | `void (*)(struct s1_prog *set)`, `struct s1_prog { uint32_t addr, bytes; const void *src; }`, zero-terminated | queue a program set for **DSP A** of the calling chip | `examples/system_1/syninfo.h:64,67`; `engine.c:81` |
| `FC34` | `s1_queue_fn` | same | queue a program set for **DSP B** | `engine.c:82`; `model.c:261` |
| `FC38` / `FC3C` | `s1_queue_fn` (`FC3C`) | same | data queues; Roland's SH-101 / SH-2 / PROMARS call two `FC38` and three `FC3C`; what they do "is not studied" ([Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §15.3 table note). SYSTEM-S1CC uses `FC3C` to load the master-B data it needs (the EDM map table, `model.c` comment "the OS's data queue for master B") | `examples/system_s1cc/model.c:351,383`; [Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) Part B |
| `FC40` | `place_fn` | `void (*)(int index, const struct s1_prog *prog, const struct s1_prog *d1, const struct s1_prog *d2, const struct s1_prog *d3)` | place a voice **section** the way the stock synth does (`FC40` = `0x60003F22`, own queue `0x600602d8`); a program queued with `FC30` at `0x1900` was copied but did not run (2026-10-28) | `examples/system_s1cc/engine.c:506–511` |
| `FC44` | `slot_fn` | `void (*)(int index, int on)` | clear (`on = 1`) or set the "off" bit of section `index` (`0x60003F96`); our engines switch the OS's voice sections off with `FC44(i, 0)`, i = 0..7 | `examples/system_1/engine.c:80`; `examples/system_s1cc/engine.c:516–522` |
| `FCE4` | `ramp_fn` | `void (*)(int sel, int id, uint32_t target, int y, int z)` | hardware ramp of parameter `C0id` of DSP A (`sel 0`) / B (`sel 1`); block `0x4014C000` / `0x4016C000` + 16·id: `[+0] = 0x80000000 \| z << 28 \| y << 24`, `[+4]` = target; with z ≠ 0 the OS first puts its end-of-ramp function in `0x20003534 + 4 (+0x404 for DSP B) + 4·id`. Master 1.30 `0x01008CD4`, slave `0x0100976C`. Linear, one step every 1/3 ms, 4–97 ms from 0 to 1 by `y` | `examples/system_1_ramp/model.c:257–259`; [examples/system_1_ramp/README.md](../../examples/system_1_ramp/README.md) §2 |
| `FCEC` | `env_fn` / `param_fn` | `void (*)(int sel, int id, uint32_t bits)` | parameter `C0id`, immediate: `[+0] = 0xE0000000, [+4] = [+8] = value, [+0xC] = 0` | `examples/system_1/model.c:229`; `examples/system_s1cc/engine.c:42`; [Vol. I ch. 6](../vol1_system_architecture/s1_arm_clock.md) §2 |
| `FD6C` | `bank_fn` | `void (*)(int sel, const uint32_t *obj, uint32_t value)`, word index in `obj[2]` | bank-5 word `A000 + idx` (`sel 0`) or `A100 + idx` (`sel 1`) at `0x40101000 + 4·idx` / `0x40101400 + 4·idx`; other `sel`: nothing. Master `0x010094B6`, slave `0x01009F4E` | `examples/system_1/engine.c:25,32`; [Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §14.2 |
| `FD74` / `FD78` | `b5rd_fn` | `uint32_t (*)(int sel, const uint32_t *obj)` | bank-5 read: `FD74` through the converter `0x40040550`, `FD78` direct. On the master all four tried paths read 0 [measured] | `examples/system_1_ramp/model.c:263–272`; examples/system_1_ramp/README.md §1 |
| `FE3C` | `cbreg_fn` | `int (*)(int sel, int id, void (*fn)(int))` | put an own end-of-ramp function in the table (only if the entry is free; returns 1 / 0); the interrupt handler (`0x0100B008` for DSP B) calls `table[id](id)` and does not free it. Master 1.30 `0x01008CBC`. Measured 2026-10-30: 6225 calls, one every 4.00 ms | `examples/system_1_ramp/model.c:362–368`; [Vol. II ch. 10](../vol2_dsp_core_isa/native_ops.md) §5 |
| `FE40` | `cbfree_fn` | `int (*)(int sel, int id)` | free the entry | same |

The SYSTEM-1 send of MIDI from region 0 uses an OS function by absolute address, not this table: `0x600065B0`,
object `0x605F20D4`, cable 3, mask 1 ([Vol. I ch. 1](../vol1_system_architecture/soc_overview.md) §6.5).
