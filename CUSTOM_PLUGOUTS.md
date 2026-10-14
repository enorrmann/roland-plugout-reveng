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
  | `+0x28..+0x6C` | DSP microcode registration, 3 lists × 6 DSPs (see "DSP microcode" below) |
  | `+0x70` | swap the program in one slot of a running DSP (see below) |
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
* Oscillator waveform: the sound is DSP microcode that the engine carries
  and loads into the slave, one program per waveform family (see "DSP
  microcode" below). The earlier note that the waveform "is decided in the DSP
  program, which is not in the PLUG-OUT" was wrong: the programs are in the
  PLUG-OUT.
* Region 0 `0x11AB84..`: value-label lists (`<u32 2><u32 0x14><u32 list id>
  <u32 count>…` followed by `<u32 3><u32 0x1C><char[20]>` labels).

Verified patch (EFFECT TONE inverted; 16 bytes at `0x60A0AC62`, slot 1):
`mov r3,r5 · ldr r2,[r4,#0x14] · ldr r1,[r4,#4] · vneg.f32 s0,s0 ·
bl 0x60A0B41E · pop {r4,r5,r6,pc}` — the original tail call
(`pop.w` + `b.w`) becomes `bl` + `pop`, and the dead `mov r0,r4` is dropped,
which frees room for the new instruction without moving anything else.

### DSP microcode in the engine (JX-3P, 2026-10-14, emulation only)

The audio is not produced by ARM code. Each engine image contains **DSP
programs (microcode)** and hands them to the slave BMC, which downloads them
into its six DSPs. Different waveforms are different programs. Found by
emulating region 1 (`examples/from_scratch/re/`); listed by
`dsp_progs.py region1.bin [outdir]`, which also writes the blobs out.

**Load at start-up**: main object (`CPrmDSPJx3pSys8`, vtable `0x60A445CC`)
`vt+8` = `0x60A0FA98`. It reads three values with OS `+0x00`
(`get(0, 4, id)`): `0x6D4` DCO1 WAVEFORM (clamped 0–2), `0x6D9` DCO2
WAVEFORM (0–3), `0x748` (0–5, picks one of six effect programs, so most
likely the effect type), then registers everything with the OS table. Each
OS entry `+0x28..+0x6C` is the same function, `0x01105808` in `s_boot`:
append the argument to one of 18 lists (16-byte headers at
`0x20084050..0x20084160`):

| OS entries | Argument | Meaning |
|---|---|---|
| `+0x28 + 4·n` | DSP address | program **slot** of DSP `n` (`n` = 0–5) |
| `+0x40 + 4·n` | `&{dsp_addr, bytes, bytes, src}` | **program** for one slot |
| `+0x58 + 4·n` | `&{dsp_addr, bytes, bytes, src}` | **data** (8 or 32 bytes, all zero in the JX-3P) |

Layout used by the JX-3P (slot 1 addresses; DSP numbers = units of the
`+0xA0`/`+0xB0` parameter calls):

| DSP | `0x0100` (main) | `0x6000` | `0xB000` | `0x8000` |
|---|---|---|---|---|
| 0, 1 (envelopes/LFO) | `0x13CA` / `0x139A` B | — | — | stub |
| 2, 3 (oscillators, 2 voices each) | `0x814` / `0x7E2` B | **DCO1 program** | **DCO2 program** | — |
| 4 (filter/VCA + effect) | `0x1436` B | stub | **effect program** | — |
| 5 (filter/VCA) | `0x1398` B | stub | — | — |

"Stub" = the same 14 bytes everywhere (`70ff 020a 70ff 020a f800 020b 0000`
at `0x60A1663C`): an empty slot.

**Oscillator programs** (table at `0x60A16844`, 7 pairs `{prog, data}` per
DSP: DCO1 values 0–2, then DCO2 values 0–3; DSP3's table follows at
`0x60A1687C`; effects at `0x60A168B4`, 6 pairs). Panel labels from the
region 0 lists: DCO1 0 SAW, 1 SQR, 2–5 PULSE; DCO2 0 SAW, 1 SQR, 2 PULSE,
3–5 NOISE.

| Wave | DCO1 (slot `0x6000`), src | DCO2 (slot `0xB000`), src | Size |
|---|---|---|---|
| SAW (0), PULSE (2) | `0x60A1B594` | `0x60A1C59C` | `0x1008` B |
| **SQR (1)** | **`0x60A1D5F4`** | **`0x60A1E57C`** | `0xF88` B |
| NOISE (DCO2 3) | — | `0x60A1F554` | `0x180` B |

* SAW and PULSE are **one program**; after loading it the engine sends a
  flag through `+0xB0` (descriptor `0x60A147A8`… for DCO1, `0x60A147C4`… for
  DCO2) with `s0 = 0.0` for SAW and `1.0` for PULSE. SQR and NOISE have
  their own programs. Values above the clamp use the last entry (DCO1 3–5 =
  PULSE, DCO2 4–5 = NOISE).
* DSP2 and DSP3 get byte-identical programs (separate copies in the image:
  SQR for DSP3 is at `0x60A2175C` / `0x60A226E4`).
* The DCO1 and DCO2 copies of the same program differ only in operands: the
  code is made of 16-bit little-endian words, and 304 of the 314 changed
  bytes are `+0x200` on a word (`0x04xx` → `0x06xx`, data memory
  `0x400–0x47F` vs `0x600–0x67F`), the rest small `+2`/`+6` changes
  (`0x2201` → `0x2207`, `0x221B` → `0x221D`): the same code assembled for a
  different memory bank and parameter registers.
* SAW/PULSE vs SQR: same first 78 words and the same tail, otherwise
  different code. Byte layout and execution order: see "Word size and
  structure" below (all programs start `0a02 ff70` and end `0b02 00f8 0001`
  in execution order). The DSP instruction set is still unknown.

**Live waveform change** (`vt+0x3C` = `0x60A0B658` for DCO1; DCO2 likewise):
ramp the oscillator level ids to 0 (`+0xA0`, DCO1 ids `0x32`/`0x48`, DCO2
`0x33`/`0x49`), `+0xC0` (r0 = 3, r3 = 1; r0 = 4 for the effect; meaning unknown), then for DSP 2 and 3
**`+0x70(dsp, &prog, &data, 0x60A44E18, 0x6000 or 0xB000, 2, 0, 0)`**
(OS `0x011062FA` → `0x01105F94`: replace the program in that slot of the
running DSP), ramp the levels back to 1 and resend the SAW/PULSE flag.
Changing `0x748` does the same on DSP4, slot `0xB000`.

Consequences for a from-scratch PLUG-OUT: a square oscillator needs a DSP
program. Without an assembler for this DSP the only option is to copy
Roland's blobs (e.g. the JX-3P SQR program plus the main programs it runs
under) into the new engine and register them the same way; their parameter
ids and memory layout come with them.

#### Microcode format: what is known (2026-10-14, static analysis only)

**Code, not wavetables.** Read as 16-bit samples, the SQR blob is not two
levels and the SAW blob is not a ramp: the mean step between neighbours is
0.24 of the full range (random data: 0.33; a smooth waveform: well under
0.05), and the 1988 words use only 383 distinct values, a "vocabulary" like
an instruction set. Further evidence: the slave writes the blobs to
**program** memory, the DCO1/DCO2 copies differ only in address operands,
and SAW and PULSE share one blob. The only data the JX-3P sends with them is
8 or 32 zero bytes. The waveforms are computed by the DSP code.

**Where it goes in the slave** (`s_boot`):

* Program memory of DSP `n` (0–5) is mapped at **`0x41480000 + n·0x100000`**,
  DSPM at `0x41260000` (table `0x0111C190`). The program address in the
  descriptor (`0x100`, `0x6000`, `0xB000`, …) is a **byte offset** into it and
  must be even (`0x01110648`).
* Data memory: `0x41440000 + n·0x100000` and `0x41444000 + n·0x100000`
  (also `0x41448000…`), addresses must be multiples of 8 (`0x0111077A`): the
  data word is probably 64 bits.
* `VerifyProg` (`0x011026CA`) compares the blob with program memory word by
  word, without any transformation: the bytes sit in DSP memory exactly as
  in the image (no packing, no compression). For a length ≡ 2 mod 4 it
  compares only the upper halfword of the last 32-bit word (`bfc r0,#0,#16`);
  why the upper half is not clear yet. `VerifyDrProg` (`0x01102752`) does the
  same for the data lists.
* Built-in program lists of the slave itself: `0x6000F7D0` in `s_data`
  (16-byte headers, two per DSP 0–6, count at `+2`, array at `+4`).

**Corpus**: `examples/from_scratch/re/dsp_corpus.py outdir name=image …`
finds programs through their descriptors in any image (the load address is
guessed). All stock PLUG-OUT engines, `p0s`, `pl1m/pl1s/pl2m`, `s_data`,
`m1_tcm` and `m1_sdram` together: **199 unique programs, 495 450 bytes**
(`p0s`, the SYSTEM-8 engine, alone has 99 descriptors). `os_full.bin` as a
whole gives none because its parts have different load addresses.

**Word size and structure** (word lists below are in **execution order**,
`examples/from_scratch/re/dspmc.py`):

* Unit = **16-bit instruction**; two per 32-bit little-endian word, **upper
  half first**. Evidence, all from the live dump of the master DSP4 (below)
  plus the corpus: (1) after loading a 0x256-byte blob over a longer
  program, the blob's last halfword (`0000`, at +0x254) was **not** written —
  memory still holds the old program's value — while the halfword that
  follows the blob in the image (`0001`, +0x256) **was**; `VerifyProg` checks
  only that upper half for a 4n+2 length. So for a 4n+2 blob the image
  supplies one extra halfword `0001`, which is the real last instruction, and
  the blob's final `0000` is a filler that is never loaded. (2) In this order
  every one of the 199 programs starts with `0a02 ff70` and ends with
  `0b02 00f8 0001` (in memory byte order there were two different endings).
  (3) The stream becomes much more predictable: conditional entropy of the
  next word 3.27 bits instead of 4.32.
* No 32-bit instruction alignment: even- and odd-position words have the same
  distribution (entropy 9.82/9.84 bits, Jensen–Shannon divergence 0.025), and
  no frequent word prefers a parity (`1e05` 2879/2787, `8fe0` 2933/2717, …).
  A 4n+2-byte program is just an odd number of instructions.
* The 14-byte empty-slot stub is, in execution order, `0a02 ff70 0a02 ff70
  0b02 00f8 0001`: `0a02 ff70` is a prologue/sync, `0b02 00f8 0001` the end
  of a task.
* **Operand words**: all 304 relocated words between the DCO1 and DCO2
  copies have the form `0x04nn` → `0x06nn` (data address `0x400–0x47F` vs
  `0x600–0x67F`). They are separate 16-bit words, not fields inside an
  instruction. A second group, `0x22nn`, moves by +2/+6 (`2201` → `2207`,
  `2204/2205` → `220a/220b`).
* Frequent sequences (corpus, execution order): `<ref> 1e05 0fe0 1fe2`
  (2169×), `1e05 8fe0 9fe2`, `1e05 8fe0 0fe2`, `1e05 2fe0 3fe2`, `<ref> 0c03
  8fe0`, `<ref> 3e07 8fe0`. `1e05`, `0c03` and `3e07` are **preceded** by a
  high-entropy word (a memory reference; entropy of the previous word
  8.8/7.9/7.4 bits) and followed by `Xfe0`, which is usually followed by
  `(X+1)fe2` — pairs of a 4-bit field X, X+1 (register pairs?). `XfeY`
  (X = 0…F, Y = 0/2/4/6/8) makes up 17.6 % of all words.
* Repeated blocks such as `1cb6 3cca 0807 … 2222 2222 aaaa 2aaa` appear about
  once per program, but not at a fixed offset; the `2222`/`aaaa` words look
  like bit masks (a field repeated per slot), not instructions.

**Names from the OS** (debug screens `CEZUtilDSPProbe/DSPDump`, m1 strings
at `0x60170A80…`): memory banks `dt0`, `dt1`, `pdb`, `ecf`, `iom`, `dpm`,
`drm`, `mxm`, `ccf`, `usb`, `tbw`; modes `mute_mode`, `sat_mode`,
`dex_mode`, `inh_proc`, `int_mask`; trace events `jmp/bra/jsr`, `tsksw`
(task switch), `renderer`, `eram/iram writeback to dsp`; errors
`Ill Inst %-4s Idx%2d`, `Zero Divide`, `PROC OV`, `PDH/TBW Access`. So the
DSP has jumps, branches and subroutine calls, several tasks per DSP, and
named data banks; `DSP96K` suggests a 96 kHz rate.

#### Memory map of a slave DSP and operand spaces (2026-10-14)

From `s_boot` (`+0xA0` → `0x0111178A`, `+0xB0` → `0x011111FC`, bank tables
`0x0111C2B0` and `0x0111C2E0`). DSP `n` = 0–5, base `0x41400000 + n·0x100000`
(DSPM: `0x41200000`):

| Offset | Size (entries) | Contents / writer |
|---|---|---|
| `+0x00000` | 0x800 × 32 bit | bank 0 (float; u32 aliases at `+0x2000`, `+0x4000`) |
| `+0x10000` | 0x800 | bank 1 (same layout; DSP0 and DSP1 share `0x41510000`) |
| `+0x20000` | 0x600 | bank 2 |
| `+0x30000` | 0x200 | bank 4 (DSP0/1: `0x411D0000`/`0x411D8000`) |
| `+0x40000`, `+0x44000`, `+0x48000` | | data RAM: the `+0x58` "data" lists, 8-byte aligned |
| `+0x80000` | | **program memory** |
| `+0xA0000` | 0x800 (DSP0/1) / 0x400 × 16 B | **parameter block**, written by `+0xA0(dsp, id, ramp, flag)`: `[+0]` current value, `[+8]` control `0x08000000 \| ramp<<20 \| flag<<24 \| mode<<28`, `[+0xC]` target float. Looks like a hardware ramp per id |

Shared: bank 5 at `0x41114000 + n·0x4000` (0x400 floats; DSPM `0x41110000`),
written by **`+0xB0(dsp, &id, s0)`** (`bank 5, id`); bank 3 at `0x4113C000`
(0x100, common to all).

**Operand spaces.** The main DSP2 program (`0x814` B at `0x100`) holds two
copies of the same code, one per voice (split at DSP address `0x4DC`).
Aligning the halves (`dsp_wdiff.py prog.bin --halves`): 314 of 494 words
are identical — the operations — and the changed words fall into classes
by high byte, each with its own voice stride:

| Word | Voice stride | Meaning (evidence) |
|---|---|---|
| `C0nn` | +0x16 | **parameter block id `nn`** (`+0xA0`): voice A uses `C020…C035` = exactly the `+0xA0` ids of unit 2 (tune `0x20`, fine `0x21`, bend `0x23`, LFO `0x26`, ENV `0x27`, mute `0x32/0x33`, level `0x34/0x35`); voice B `C036…C04B`. `+0xA0` voice stride is 0x16 too |
| `A0nn` | +0x0E | **bank 5 id `nn`** (`+0xB0`): voice A `A020…A02D`, including the SAW/PULSE flags `A02A` (DCO1) and `A02B` (DCO2); voice B uses `A038/A039`, the ids the engine sends for voice B |
| `02nn` | +0x38 | per-voice local data (0x38 words per voice) |
| `22nn` | +0x0D (and +6/+7/+8) | state shared with the slot programs (below) |
| `80nn`, `84nn` | +6 | another space, unidentified |

The same spaces in the slot programs: DCO1 vs DCO2 copies change
`04nn` → `06nn` (+0x200: local area of the slot, DCO1 `0x400–0x481`,
DCO2 `0x600–0x67F`) and `22nn` (+6 / +2). **The oscillator programs never use
`A0nn`/`C0nn`**: they do not read parameters. They use their local area and
`22nn`, which the main program also uses heavily (`2200…2238`). So the main
program of each DSP reads the parameters and passes values to the
oscillator task through the `22nn` space; the slot code only computes the
waveform. Within `22nn` the DCO1 copy uses `2201–2206` (voice A) and
`220E–2213` (voice B), DCO2 the same +6: six words per oscillator, 13 per
voice. SAW/PULSE uses `2203/2206/2210/2213` that SQR does not, a candidate
for the PULSE flag and pulse width.

So a 16-bit word is either an **operation** (never changes between copies:
`XfeY`, `0c03`, `1e05`, `3e07`, …) or a **memory reference** "space +
address" (`0nnn` local, `22nn`, `80nn/84nn`, `A0nn` bank 5, `C0nn`
parameters). The references stand where an operation needs them; they are
not fields inside the operation word. Corpus shares: `XfeY` 17.6 % of all
words (Y = 0/2/4/6/8 in falling order, X = 0…F, `8` most common).

**Pipeline (main DSP2 program, parameter copy loop at `0x108`).** In
execution order the loop is 7 words per element:

    8004 3e07 8fe0 8fe2 0fe4 2200 8404
    8005 3e07 8fe0 8fe2 0fe4 222b 8405
    8006 3e07 8fe0 8fe2 0fe4 222c 8406   … up to 800b/840b

Element k reads `80k`, runs `3e07 8fe0 8fe2 0fe4`, writes **the previous
element's** result to `22nn` and reads `84k`. The first write (`2200`) comes
before any result exists (pipeline fill). So each word is one issue slot,
results come out with a fixed latency, and references belong to the data
flow of operations issued earlier: software pipelining.

**No absolute program addresses.** The master loads five effect programs
(`m1_tcm`, sizes `0x2DE/0x256/0x352/0x3B4/0x670`) three times each, into slots
`0xEFDC` and `0xF7DC` (0x800 bytes apart). No word differs by 0x800 or 0x400
between the `0xEFDC` and `0xF7DC` copies: either there are no jumps, or they
are relative. What does differ is per-instance data: `04nn`→`06nn` (+0x200),
parameter ids `C17x`→`C1Fx` (+0x80 per instance, so the id field is at
least 9 bits), data words `0000/0001`→`0008/0009` after `1e05`, and in a third
copy `43nn`→`41nn`, `03nn`→`01nn`, `5xnn`→`6xnn`. A simple "bit 14 =
read/write" rule for `0nnn`/`4nnn` does not hold (the two forms share no more
addresses than unrelated pairs).

#### Live dump of the master's DSP program memory (2026-10-14, hardware)

The master BMC has the same DSP block (core 1, `m1_tcm`, runs the global
effects), and core 0 can read it: `bulk_loop.py` (region 0 init hook, slot 3)
read it without problems, `BULK OK` every time, no hang.

    python3 examples/midi_send/bulk_loop.py --start 0x41480000 --end 0x41490000 -o dumps/dsp0_prog_master.bin
    python3 examples/midi_send/bulk_loop.py --start 0x4158e000 --end 0x41590000 -o dumps/dsp1_prog_master_e000.bin   # and DSP2..5

| DSP | Program memory contents |
|---|---|
| 0 | `0x0000–0x002F` (system boot code, 46 B) and `0xFFDC–0xFFFF` (system stub, 36 B), rest zero |
| 1, 2, 3 | only the `0xFFDC` stub in `0xE000–0xFFFF` |
| 4 | `0xEFDC` = `m1_tcm` blob `0x01132DA8` (0x256 B), `0xF7DC` = `0x01134238` (0x256 B), stub |
| 5 | `0xE7DC` = `0x01135388` (0xFD0 B), `0xF7DC` = `0x01136388` (0x2DE B), stub |

* The blobs are in program memory **byte for byte** as in the image (no
  packing, no compression), at the descriptor address = byte offset.
* The system pieces come from `m1_sdram` `0x6100F000` (46 B → DSP address
  `0x0000`) and `0x6100F030` (36 B → `0xFFDC`), descriptors at `0x6100F054`;
  `s_data` has the same stub at `0x6000F770`. They do not start with
  `0a02 ff70`, so the corpus does not contain them. Slots are counted down
  from that stub: `0xF7DC`, `0xEFDC`, `0xE7DC` (0x800 apart).
* Program memory is plain readable memory from the ARM side, so DSP memory of
  the master can be dumped at run time; the slave's (the voices) is not
  reachable from region 0.

Open: the operations themselves (what `XfeY`, `0c03`, `1e05`, `3e07`, `0fe4`
do) and the role of `80nn/84nn`. The debug screens in `m1_sdram`
(`CEZUtilDSPDump/Probe`) have no disassembler: only bank names and
checksums. Next step that can settle opcodes: since the master's program
memory is plain memory, a region 0 hook could **patch single words of a
running effect program** (e.g. DSP5 `0xF7DC`) and the result could be heard
or recorded — a controlled experiment per opcode.

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

#### Names of all three slots: `JX-3P-MEM-ALL` (verified on hardware)

`examples/mem_names/` (`hook.s`, `make_plugout.sh`, `dump_loop.py`). The init
hook (84 bytes at image offset `0x100`) writes 8 bytes of OS memory as 16 hex
digits over each slot name, absolute: slot 1 (`0x61800010`) = `SRC..+7`, slot 2
(`0x61A00010`) = `SRC+8..+15`, slot 3 (`0x61C00010`) = `SRC+16..+23`; 24 bytes per
install. 16 chars fill the 16-byte name field (no NUL; the id follows).
`dump_loop.py` reuses the fast build of `examples/mem_name/dump_loop.py` and
reads the three names back with the device info query (24 bytes per install,
stops if a name is not 16 hex digits). Untested assumptions: slots 1/2 are
mapped at their region 0 base when slot 3 initialises, and the OS reports the
name from that RAM copy. A freeze on install would mean a data abort there.

```bash
examples/mem_names/make_plugout.sh
python3 tools/plugout_tool.py install plugins/JX-3P-MEM-ALL --slot 3 --no-patches
python3 examples/mem_names/dump_loop.py --slot 3 --count 240 -o os_names.bin
```

**Result (SYSTEM-8, OS 1.32, 2026-10-14):** works on hardware. The install and
`examples/mem_names/dump_loop.py` run, the three slot names change, and the
dump is identical to the one produced by the old `examples/mem_name/dump_loop.py`
(7 bytes per install). So slots 1/2 are mapped at their region 0 base while
slot 3 initialises, the OS reports the names from that RAM, and 16 characters
without NUL are read back correctly. 24 bytes per install instead of 7.

Still unmeasured: throughput (seconds per install and per byte). The TONE
register route (`examples/cave_tone/make_mem.sh`, CUTH `0x60452C4F`) remains
unconfirmed.

**Install timing (SYSTEM-8, slot 3, 2026-10-14, `mem_names/dump_loop.py`,
3 iterations, identical each time):** build 0.2 s, START + first record < 0.1 s,
651 data records 2.3 s (3.6 ms each), 3445 filler records 3.8 s (1.11 ms each),
END message and reply 6.1 s, name read-back 2.0 s (the `--read-wait` sleep) =
15 s per iteration, 24 bytes per iteration (1.6 B/s). Install alone is 12.2 s,
of which only 19 % depends on the payload. Consequences:

* A mini PLUG-OUT can save at most the 2.3 s of data records (about 15 % of an
  iteration); the 3.8 s of filler and the 6.1 s END are fixed costs of the
  4096-record container. Not worth the hang risk of
  [[minimal-plugout-hangs]] (memory note); dropped.
* The whole 2.8 MB OS at 24 B / 15 s would be ~20 days: this channel is for
  targeted areas. Next candidates: a host-written address (install once, change
  `SRC` by MIDI and re-trigger the hook), or the TONE-register route.
* Cheap tuning: `--read-wait 0.5` (the info answer is usually ready sooner).

**Early END (no filler after the last data record), 2026-10-14:** `dump_loop.py
--skip-filler tail` sends records 0..2412 (everything up to the end of region
1, 2413 of 4096) and then END. Repeated cleanly (the first attempt overlapped
with another dump loop on the same device and is void): START and all records
are accepted, the END reply is `0x03` and arrives immediately (0.0 s, against
6.1 s of processing for a full container), and the init did NOT run: after the
call the three names are still the ones of the previous install, not the bytes
at `SRC`. So the device validates the record count at END and discards the
transfer; the filler cannot be left out. `--skip-filler all` was not tried (the
data records carry no index, so it would also move region 1 from block 2048 to
286). The 3.8 s of filler and 6.1 s of END are fixed. Use `--skip-filler` only
for this experiment: with an END error the names read back are stale, not data.
Never run two loops on the same device: the names are shared state.

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

#### Update 2026-10-13: tools and negative results

* **Faster build**: `dump_loop.py` no longer recompresses the image each
  iteration (LZSS in Python was 7 of 7.4 s). The hook's `SRC` is encoded once
  as a unique sentinel (stored as 4 literal bytes); each build patches those
  bytes and the region byte sum. Build 7.4 s → 0.2 s; output verified
  byte-identical to the slow path. Floor stays ~12 s install + 2 s read-back
  per 7 bytes.
* **Minimal PLUG-OUTs hang the device** (`examples/mem_name/make_min.py`):
  region 0 of 180 B with a stub descriptor, with and without a stub engine
  (`MIN`, `MIN-E`), and region 0 zero-padded to its stock size with the stock
  SYNINFO pointer (`MIN-P`) all froze the SYSTEM-8 at "receiving plg3"
  (power cycle needed). Cause is the descriptor returned by region 0's init
  (stock init returns a table with real callbacks at `+4`, `+8`, `+0x18`);
  size and SYNINFO pointer are not it. Keep the stock image and patch it.
  The container always carries 4096 records; filler records are cheap on the
  wire, so a smaller image only saves payload, not messages.
* **Name channel capacity** (not implemented): the slot info answer carries
  the name as 16 characters of two nibbles each, so up to 16 raw bytes could be
  read instead of 14 hex digits (7 bytes), plus the 2-byte version.
  Untested: whether the OS truncates at NUL, passes bytes >= 0x80, and whether
  version is taken from the runtime image. Do not use the model id (the OS may
  key behaviour on it).
* **`JX-3P-MEM-STEP` reworked** (`press.s`, `step_read.py`): the engine hook
  no longer writes the PLUG-OUT name (the slot always showed `MEM STEP`); on
  every panel parameter change it writes 128 nibble bytes into the RAM image of
  `NAME0` of preset 1 (`0x60445957`, slot 3 only), which `RQ1` reads back:
  4 bytes big endian address + 60 bytes of OS memory (`SRC + 60 * counter`).
  `NAME0` is a 128-byte leaf of which a name uses 32, so the write stays inside
  the leaf. The counter lives in region 0 (`name + 0x70`, zero area, known
  writable) instead of the engine image; the region 0 init hook is only
  installed with `INIT=1`. Host side: `python3 examples/mem_name/step_read.py
  --slot 3 -o os_step.bin`. Verified only on the built slot 3 file (relocated
  pointers aside, only the 108-byte hook at `0x60E07000` and the `b.w` at the
  sender entry differ from stock). **Not yet run on hardware**; open points:
  whether the hook runs on selecting the slot, whether the OS restores NAME0
  after the write (as it did after init), whether `DEST` is the same cell
  without sending the patch bank (`--no-patches`), and whether MIDI-driven
  parameter changes also reach the sender (that would remove the manual knob).

### Risks

* Identity, data and small in-place code changes work on hardware. A broken
  engine may crash the synth when the slot is selected; the official updater
  rewrites slots 0–3.
* Model id: the device and the plugin use it (with the name) to decide whether
  a slot already holds this PLUG-OUT. Whether the OS also keys other behaviour
  (panel, patch format) on it is unknown; the safest first test keeps the
  original id.
* `plugout_tool.py install` refuses slot 0 (the synth itself).
