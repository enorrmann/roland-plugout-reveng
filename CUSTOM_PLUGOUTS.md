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
| On hardware (SYSTEM-8, OS 1.32, slot 3) | rebuilt JX-3P (identical) ✔ · version changed to 1.02 ✔ · name changed to `JX-3P CUSTOM` ✔ · effect label in region 0 (`JX-3P CH` → `CUSTOM CH`) ✔ · **engine code patch: EFFECT TONE inverted for OD/DS/FZ** ✔ · **init hook reads OS RAM at `0x60000000` into the name** ✔ (§"Reading OS memory") |

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

### JX-3P engine notes (region 1, slot 1 addresses)

* C++ with RTTI: class names come from the typeinfo next to each vtable
  (`CDSPJx3pEfxOd/Ds/Fz/Ch/Cr/Ph`, `CDSPJx3pEfxCmn`, `CDSPJx3pOscVoice`,
  `CPrmDSPJx3pSys8` with 336 parameter methods, ...).
* The ARM code does not render audio: parameter methods turn a 0–255 value
  into a float and send it to a DSP parameter id. Getter `0x60A12688(obj,
  curve, value)` = `curve_table[curve][clamp(value)]` (256 floats per curve);
  sender `0x60A0B41E(_, r1, id, r3, s0)` ignores `r0` and calls the OS table
  (`0x0113FCB0`).
* Effects: `EfxCmn` holds the type-independent parameters (EFFECT LEVEL is
  curve `0x1C`). OD/DS/FZ share methods; their TONE is `0x60A0AC54`
  (curve `0x10`, −1…+1, sent to DSP id `[this+0x14]`).
* Oscillator waveform (`OscVoice` methods 8/9): both DCOs send `value − 3`
  (table at `0x60A44DBC`); "PULSE" vs "NOISE" for positions 3–5 is decided in
  the DSP program, which is not in the PLUG-OUT.
* Region 0 `0x11AB84..`: value-label lists (`<u32 2><u32 0x14><u32 list id>
  <u32 count>…` followed by `<u32 3><u32 0x1C><char[20]>` labels).

Verified patch (EFFECT TONE inverted; 16 bytes at `0x60A0AC62`, slot 1):
`mov r3,r5 · ldr r2,[r4,#0x14] · ldr r1,[r4,#4] · vneg.f32 s0,s0 ·
bl 0x60A0B41E · pop {r4,r5,r6,pc}` — the original tail call
(`pop.w` + `b.w`) becomes `bl` + `pop`, and the dead `mov r0,r4` is dropped,
which frees room for the new instruction without moving anything else.

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
* hardware tests so far: unmodified rebuild, version, name, a region 0 label,
  an engine code patch (all OK). Open: a free model id, code caves.

### Reading OS memory at run time: `JX-3P-MEM` (verified on hardware)

`examples/mem_name/` (`hook.s`, `build.sh`). Region 0's SYNINFO init pointer
(`+0x24`) is redirected to a 76-byte hook at image offset `0x100` (zero area).
At init it writes 7 bytes read from `SRC` as 14 hex characters over the name
(`image + 0x10`), then jumps to the original init (`0x11823C`). The code is
PC-relative, so the same bytes work in every slot; only the init pointer is
relocated (and already is: `0x61C00101` in slot 3).

* Default `SRC=0x60000000` (start of the OS image). `SRC=0x0113FCB0 ./build.sh`
  reads an entry of the OS service table instead (known readable by plugouts).
* Install `plugins/JX-3P-MEM` in slot 3. The slot name becomes 14 hex digits
  instead of `JX-3P MEM`.
* Things this tests: (1) a plugout can read OS RAM at `0x6000_0000`;
  (2) region 0 is writable RAM; (3) the OS reads the name after init. If the
  name stays `JX-3P MEM`, (2) or (3) failed; a crash / freeze on slot select
  means a data abort (the official updater restores slots). Chaining several
  builds with different `SRC` reads 7 more bytes each time — the route the OS
  dump was blocked on in BASE_FIRMWARE.md.

**Result (SYSTEM-8, OS 1.32, slot 3, 2026-10-13):** with `SRC=0x60000000` the
slot name showed `B04801684FF6FF` — the first 7 bytes at `0x60000000`, in
memory order. All three propositions hold. The bytes decode as Thumb-2, so
`App1_Main` is present in RAM in plaintext even though it ships encrypted in
the updater:

```
B0 48        ldr  r0, [pc, #0x2c0]
01 68        ldr  r1, [r0]
4F F6 FF ..  movw r…, #…          (7th byte begins a 32-bit movw)
```

7 bytes per install is slow for a 2.8 MB image; a faster channel (a
parameterised `SRC`, or writing the bytes somewhere the host already reads
back) is the obvious next step.

#### Findings of 2026-10-13 (SYSTEM-8, OS 1.32, slot 3)

**Slot-name channel, timed** (`examples/mem_name/dump_loop.py`, 70 bytes at
`0x60000000`, 10 installs, no errors). Per 7 bytes: build 7.4 s (35 %),
install 12.1 s (56 %), read-back 2.0 s (9 %, of which 2.0 s is the fixed wait
of the script). Floor ≈ 12 s per 7 bytes (0.58 B/s); the whole 2.8 MB OS image
would take weeks, so this channel is for small areas. Using the 3 slots only
saves the `info` part (one `info` returns all slots), not the installs.
Slots 1 and 2 hold factory PLUG-OUTs and would be overwritten.

The first 70 bytes of the OS (`examples/preset_fill/os70_name_method.bin`):

```
B0 48 01 68 4F F6 FF 02 01 EA 02 01 AE 4A 41 EA 02 01 01 60 4F F0 10 00
80 F3 11 88 00 F0 C0 F8 AA 48 4F F4 00 61 01 60 A9 48 AA 49 AA 4A 00 F0
C8 F8 AA 48 AA 49 AB 4A 00 F0 C3 F8 AA 48 AB 49 AB 4A 00 F0 FC F8
```

Thumb-2 start-up code: `ldr r0,[pc]; ldr r1,[r0]; movw r2,#0xFFFF; …;
msr cpsr/…` (`80 F3 11 88` = `msr basepri/cpsr, r0`) followed by `bl`s to
init helpers; i.e. the reset path of `App1_Main`.

**Reading patches from the device: RQ1 works** (`tools/patch_read.py`). The
plugin only ever writes the patch bank (DT1), but the SYSTEM-8 answers the
standard Roland read `RQ1 (0x11)` = address + 4 x 7-bit size with a `DT1`
at the same address. `NAME0` of patch `p` in slot `s` is at
`patch_address(s, p, 0) + 0x80` (128 bytes, 2 nibbles per character). All 64
names of slot 3 read back and match `plugins/JX-3P/Patch/1 Preset.bin` (63/64
byte exact; #38 differs only in the quote character of my printing). So any
leaf of `usrX/usr2X/usr3X` can be read from the device. Note: a request of
size 70 at NAME0 returned 64 bytes and the follow-up at +64 got no reply
(unexplained); 128 returns 128.

**Where the preset lives in RAM** (`examples/preset_fill/`: `scanfill.c`,
`entry.s`, `build.sh`, `make_plugout.sh`, `run_test.py`). An init hook in
region 0 scans `0x602CE000..0x60800000` (past the OS image, below the slot-0
engine; both known mapped) for `"String 1"`, the first preset of the JX-3P
bank, as ASCII or as one nibble per byte, and reports `addr[4] count[2]
form[1]` as 14 hex digits in the slot name. The needle is stored XOR `0xA5`
and loaded from immediates, so the image never holds it in plain and has no
slot-relative pointer (the 288-byte hook relocates with only the init
pointer).

| Result | |
|---|---|
| first match | `0x60445957` |
| matches | 1 |
| form | 2: **one nibble per byte**, identical to the RQ1 wire form (`05 03 07 04 …`) |
| writable at init | yes: with `REPORT=1` the 7 bytes read back right after the fill were `B04801684FF6FF` |
| persists | **no**: afterwards RQ1 of NAME0 still returns the original name, and a second install finds the original text at the same address again (count 1) |

So the patch memory the host reads is a RAM image at `0x60445957` (the
address looks stable across installs), and the OS reloads / re-initialises it
after the PLUG-OUT's `init` has run (at install end or at slot activation).
Writes done *at init* are therefore lost. Comparison with the name channel
therefore could not be completed through RQ1: read-back 4/70 bytes only match
by chance (the original name), the 70 OS bytes never reach the host this way.
Next step: do the same write from a hook that runs *later* — the engine's
parameter sender hook (`examples/mem_name/press.s`, runs on the first knob
move) with `DEST = 0x60445957` — then RQ1 NAME0. Mode 1 of `scanfill.c`
(each byte as two nibbles, 140 bytes) is there for that: it survives the 7-bit
SysEx data and decodes exactly. Raw bytes `>= 0x80` may be masked by the
device.

#### Stepping through memory with the panel: `JX-3P-MEM-STEP` (built, not yet tested on hardware)

`examples/mem_name/press.s` + `engine_build.sh` + `make_plugout.sh`. The
init hook above is kept; additionally the entry of the DSP parameter sender
(`0x60A0B41E`, reached by every knob / button / switch of the panel) is
replaced by a `B.W` to a 112-byte hook in the region 1 cave (`0x60A07000`, slot
1). Each call increments a counter stored in the cave, reads 7 bytes at
`SRC + 7·counter`, writes them as 14 hex digits over the name in region 0
(`image + 0x10`, found PC-relative) and runs the 3 displaced instructions
(`ldr r0,[pc,#0x50]`, `push {r3,lr}`, `ldr r0,[r0]`) before continuing at
`0x60A0B424`. Arguments, `s0` and flags the sender needs are preserved.

```bash
examples/mem_name/make_plugout.sh                  # SRC=0x60000000
examples/mem_name/make_plugout.sh 0x0113FC00       # other start address
python3 tools/plugout_tool.py install plugins/JX-3P-MEM-STEP --slot 3 --dry-run
python3 tools/plugout_tool.py install plugins/JX-3P-MEM-STEP --slot 3
```

Notes: the first press shows `SRC+7`; the name at slot selection is `SRC`. It
advances on any parameter change (it may also fire on non-panel parameter
traffic, e.g. notes), and whether the OS re-reads the name after init is the
open question this build tests — if the display does not change, the next step
is a different sink (a parameter label or the patch name).

### Risks

* Identity, data and small in-place code changes work on hardware. A broken
  engine may crash the synth when the slot is selected; the official updater
  rewrites slots 0–3.
* Model id: the device and the plugin use it (with the name) to decide whether
  a slot already holds this PLUG-OUT. Whether the OS also keys other behaviour
  (panel, patch format) on it is unknown; the safest first test keeps the
  original id.
* `plugout_tool.py install` refuses slot 0 (the synth itself).
