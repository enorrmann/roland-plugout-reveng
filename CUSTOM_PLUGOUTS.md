# Custom PLUG-OUTs

What is inside a `Code*.Dat`, how to rebuild one, and what is known about the
code that runs on the synth. Tools: `tools/plugout_build.py` (CLI),
`tools/plugout_image.py`, `tools/lzss.py`. The USB transfer itself is in
[`PROTOCOL.md`](PROTOCOL.md).

## Status

| | |
|---|---|
| Container, flash image, region header, LZSS, END checksum | fully known; **every shipped `Code*.Dat` (12) is rebuilt byte for byte** |
| Per-slot builds | relocation table derived from two slot builds; slot 1 + relocs → slot 3 equals Roland's `Code8_3.Dat` byte for byte (JUNO-60, JX-3P, SH-101) |
| Authentication | none found: the only integrity checks are byte sums (region header, END message) |
| Name / model id / version | editable (container header + SYNINFO) |
| Code | ARM Thumb-2 with VFP; OS services through a fixed table at `0x0113FC00` (partially mapped, §7) |
| On hardware | **a modified image has not been installed on a real SYSTEM-8 yet** |

## 1. Layers

```
Code8_<slot>.Dat     XOR container (PROTOCOL.md §4)
 └ 4096 records      one 512-byte flash block each (2048 on SYSTEM-1)
    ├ blocks    0..2047   region 0 "model"   + filler records
    └ blocks 2048..4095   region 1 "engine"  + filler records
       └ region   = 0x40 header + LZSS stream, padded with 0xFF to 512
          └ image = decompressed bytes, linked at a fixed address per slot
```

* A data record (`70 30 00 09`) carries one block (512 bytes, 7-in-8). There
  is no block number: blocks are written in order.
* A filler record (`70 30 01 09`, data `01`) stands for an erased block
  (512 × `0xFF`). Each region starts at the first block of its half; the rest
  of the half is fillers.
* A compressed region must fit its half: 2048 blocks = 1 MiB (512 KiB on
  SYSTEM-1). The shipped ones use 280–370 blocks.

### END message

The 4 data bytes of `70 10 00 01` are the **16-bit sum of the whole flash
image** (all 4096 blocks, fillers counted as 512 × `0xFF`), one nibble per
byte, most significant first. `plugout_image.end_checksum()`.

### Container header (decrypted)

`0x00` u32 `0x80` · `0x04` `"PLUGOUT_SYS_INF\0"` · `0x14` name[16] ·
`0x24` u16 model id · `0x26` u16 version ·
`0x30` START (offset `0x80`, size `0x20`, count 1) ·
`0x40` END (`0xA0`, `0x20`, 1) · `0x50` DATA (`0xC0`, stride `0x260`, count).
Each record: u32 length + SysEx, zero padded to its slot.

## 2. Region header (0x40 bytes, little endian)

| Offset | Field |
|---|---|
| `0x00` | `"SYSTEM-8:Plugout"` (`"SYSTEM-1:Plugout"` in `Code1.Dat`) |
| `0x10` | build stamp, e.g. `"25-0509_11:30:38"` |
| `0x20` | `"0.0114FA"` (all 2025 builds; `"0.01136E"` in the 2022 updater) |
| `0x28` | u32 load address: `0x60800000` region 0, `0x60200000` region 1 |
| `0x2C` | u32 header size `0x40` |
| `0x30` | u32 compressed size |
| `0x34` | u32 32-bit byte sum of the compressed stream |
| `0x38` | u32 entry (= load address) |
| `0x3C` | u32 decompressed size |

On SYSTEM-8 the load/entry fields are the same in every slot build; the image
itself is linked per slot (§4). On SYSTEM-1 they are the run addresses.

## 3. LZSS

Okumura's LZSS (1989): 4096-byte ring, max match 18, threshold 2, flag byte
LSB first (1 = literal), reference `pos = b0 | (b1 & 0xF0) << 4`,
`len = (b1 & 0x0F) + 3`, first byte at ring position 4078. **The ring is
initialised to `0x00`** (Okumura uses spaces). Okumura's binary-tree encoder
reproduces all 29 regions (plugins + `SYSTEM8_UP.BIN`) exactly.

## 4. Images, SYNINFO and slots

Each image starts with a SYNINFO block:

| Offset | Field |
|---|---|
| `0x00` | `"SYSTEM8_SYNINFO\0"` |
| `0x10` | name[16] |
| `0x20` | u16 model id, u16 version |
| `0x24` | u32 init function (Thumb, odd) |
| `0x28` | u32 pointer into the image |
| `0x2C` | `"SYSTEM8_SYNINFO\0"` again |

* Region 0 SYNINFO = the PLUG-OUT identity (`JUNO-60`, `0x000A`, `1.01`); the
  same values as the container header, and what the device reports for the
  slot (`70 00 01 00`). The SYSTEM-8 itself is `SYSTEM-8`, `0x0005`, `1.20`.
* Region 1 SYNINFO is always `SYSTEM-8`, `0x000A`, `0.01` on SYSTEM-8 builds
  (probably the engine interface version).

Run addresses (from the SYNINFO pointers and the slot diffs):

| | slot 0 (synth) | slot 1 | slot 2 | slot 3 |
|---|---|---|---|---|
| region 0 (model) | `0x61600000` | `0x61800000` | `0x61A00000` | `0x61C00000` |
| region 1 (engine) | `0x60800000` | `0x60A00000` | `0x60C00000` | `0x60E00000` |

Each slot window is 2 MiB (`SLOT_STRIDE = 0x200000`). The image is followed by
its `.bss`: the highest internal pointer is past the end of the image (JUNO-60
engine: image ends `0x60A40B40`, pointers reach `0x60A4F29C`).

Region 0 is ~1.1 MiB but almost all zeros: the SYNINFO, then a zero area, then
the model's tables (parameter names such as `LFO`, `ENV1+`, `64FEET`) and a
little code at the end. Region 1 is the sound engine (200–280 KiB of code).

### Relocation

`Code8_1/2/3.Dat` differ only in aligned 32-bit absolute pointers that move by
`0x200000` per slot (JUNO-60: 52 in region 0, 1474 in region 1; no `movw/movt`
pairs). Diffing two slot builds gives the full list; applying it builds any
other slot. `plugout_build.py unpack --relocs-from` stores it in
`relocs.json`.

## 5. `SYSTEM8_UP.BIN` (official updater, tar)

| File | Contents |
|---|---|
| `p1y01_m0c0a_up.bin` | `App1_Main` OS, load `0x60000000`, encrypted or compressed differently (entropy 8.0); not decoded |
| `p1y01_p0m.bin` / `p0s.bin` | slot 0 = the SYSTEM-8 itself, regions 0 / 1 in the format above |
| `p1y01_pl1m.bin` | `PRELOAD_PLUGOUT` header + region 0 of JUPITER-8, linked for slot 1 |
| `p1y01_pl1s.bin` | same, JUNO-106, slot 2 |
| `p1y01_pl2m.bin` | same, JX-3P, slot 3 |

The updater rewrites slot 0 and the factory PLUG-OUTs: it is the recovery path
if a custom PLUG-OUT misbehaves (follow Roland's update procedure).

## 6. Workflow

```bash
B="python3 tools/plugout_build.py"

# unpack slot 1, with relocations taken from the slot 2 build
$B unpack plugins/SH-101/Script/Code8_1.Dat work/sh --relocs-from plugins/SH-101/Script/Code8_2.Dat
$B show work/sh

# change things
$B set-id work/sh --name "SH-101 MOD" --version 1.09
$B patch  work/sh --region 1 --at 0x60a10800 --hex "00bf00bf"         # run address, slot 1
$B patch  work/sh --region 1 --at 0x60a60000 --file hook.bin --reloc 0x60a60010

# disassemble: load work/sh.elf in Ghidra / IDA as ARM LE, Thumb-2
$B elf work/sh -o work/sh.elf

# build: one file, or a plugin folder with Code8_1..3 + Script.xml + Patch/
$B pack   work/sh -o out/Code8_2.Dat --slot 2
$B plugin work/sh --template plugins/SH-101 -o plugins/SH-101-MOD

# install (always try --dry-run first)
python3 tools/plugout_tool.py install plugins/SH-101-MOD --slot 2 --dry-run
python3 tools/plugout_tool.py install plugins/SH-101-MOD --slot 2

# sanity check of the tool chain: rebuild Roland's files byte for byte
$B verify plugins/*/Script/Code*.Dat
```

Project directory: `plugout.json` (device, slot it was built for, identity,
region header fields), `region0.bin` / `region1.bin` (decompressed images,
edit freely), `relocs.json` (pointer offsets per region). `pack` stamps the
identity into the container header and region 0's SYNINFO, recompresses,
recomputes every sum, and relocates when `--slot` differs.

Compression is pure Python: ~6 s for region 0, ~1 s for region 1.

## 7. The code and the OS interface (partial)

* Both regions are ARM **Thumb-2** with VFP (`vldr`, `vmul`, `vmla`, …). The
  exact core is not confirmed.
* `init` (SYNINFO `+0x24`) fills a descriptor in the image's `.bss` with
  function pointers and returns a pointer to it (JUNO-60 engine `0x60A107E4`
  → descriptor `0x60A4F280`; model `0x6191823C` → `0x6191AD68`). These are the
  PLUG-OUT's exports.
* The engines call the OS through a **fixed table at `0x0113FC00`**, the same
  address in every slot and in the synth's own engine (`p0s`). Typical import
  stub: `ldr r1, =0x0113FCB0; ldr r3, [r1]; mov r1, r2; bx r3`.

  | Entry | Use seen |
  |---|---|
  | `+0x00` | read at init next to the constants `4` and `0x748` |
  | `+0x28..+0x40` | read in an init loop, one entry per object |
  | `+0xA0`, `+0xA4`, `+0xB0` | functions, called with `(obj, …)` from import stubs |
  | `+0xBC` | function taking a float in `s0` |
  | `+0xC0` | function (`[r4+4]` object as first argument) |
  | `+0xF0` | function called first in the engine start routine; result tested for 0 |

  Used by all three engines and by `p0s`, so it is a stable interface.
* No absolute OS addresses (`0x600xxxxx`) appear in the engines.

### Writing new code

What works today, without the full ABI:

1. **Modify an existing PLUG-OUT** — tables, constants, identity, or code.
   Edit `region<n>.bin` (or `patch`), `pack`, done. Pointers you change inside
   the image keep relocating as long as they stay at listed offsets.
2. **Add code** — build position-dependent Thumb-2 for the slot-1 address
   (e.g. `arm-none-eabi-gcc -mthumb -mfloat-abi=hard -mfpu=vfpv3 -ffreestanding
   -nostdlib -Wl,-Ttext=0x60a60000`, objcopy `-O binary`), `patch --file` it
   past the `.bss` (`show` prints the highest internal pointer), list every
   absolute pointer you emit with `--reloc`, then redirect a call or a
   descriptor entry to it. `plugin` / `pack --slot` relocate it for other
   slots.

A PLUG-OUT written from scratch needs the rest of the interface: the meaning
of each `0x0113FC00` entry, the descriptor layouts, and what region 0's tables
must contain. Next steps:

* load `elf` exports of the three PLUG-OUTs and `p0m`/`p0s` in Ghidra, label
  the import stubs, and compare call sites across engines;
* decode `App1_Main` (the OS) to see who fills `0x0113FC00`;
* first hardware test: an unmodified rebuild (`pack` output equals Roland's
  file), then an identity change only, then code.

### Risks

* Not yet tried on hardware. A broken engine may crash the synth when the slot
  is selected; the official updater rewrites slots 0–3.
* Model id: the device and the plugin use it (with the name) to decide whether
  a slot already holds this PLUG-OUT. Whether the OS also keys other behaviour
  (panel, patch format) on it is unknown; the safest first test keeps the
  original id.
* `plugout_tool.py install` refuses slot 0 (the synth itself).
