# Volume IV — Chapter 1: Binary Container Formats (`Code*.Dat`, flash image, regions, LZSS, `SYSTEM8_UP.BIN`)

> **Scope:** both: `Code*.Dat`, regions and LZSS are common; `SYSTEM8_UP.BIN` and `App1_Main` are SYSTEM-8 (the SYSTEM-1 images: [ch. 6](base_firmware.md) §9).

> The code and the OS interface (§7 onwards: the JX-3P engine, the DSP microcode in the engine, writing new code,
> reading OS memory at run time, risks) are [Vol. IV ch. 2](plugout_virtual_machine.md). Section 1.4.1a (the
> `App1_Main` cipher) is cited by that number by `tools/sys8fw.py` and [TOOLS.md](../TOOLS.md).


What is inside a `Code*.Dat`, how to rebuild one, and what is known about the
code that runs on the synth. Tools: `tools/plugout_build.py` (CLI),
`tools/plugout_image.py`, `tools/lzss.py`. The USB transfer itself is in
[Vol. IV ch. 4](usb_sysex_protocol_spec.md).

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
Code8_<slot>.Dat     XOR container (Vol. IV ch. 4 §4)
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
| `p1y01_m0c0a_up.bin` | `App1_Main` OS, load `0x60000000`, payload encrypted with 3-key 3DES (§1.4.1a) |
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

---

### 1.4.1a `App1_Main` Payload Cipher: 3-Key 3DES `[VERIFIED]`

Only the **payload** is encrypted. File bytes `[0x60 : 0x60 + size@0x44)`; the `0x60`-byte
clear header and the 0x40-byte clear trailer are always plaintext, which is what makes an
encrypt/decrypt round trip byte-exact.

Container layout:

| Offset | Contents |
|---|---|
| `0x00` | `"App1_Main"` + NUL |
| `0x10` | build date, ASCII (`2022/04/28 09:41`) |
| `0x20` | tool/version tag (`0.010001`) |
| `0x28` | load address (`0x60000000`, master BMC SDRAM) |
| `0x30` | `0x00060040`; `0x34`–`0x40` are `FFFFFFFF` |
| `0x40` | `0x00060060` |
| `0x44` | encrypted payload size (v1.32: `0x2CDA70`) |
| `0x48` | load address again |
| `0x50` | payload check value (not CRC-32/sum32/Adler/SHA of the ciphertext) |
| `0x54` | crypto control: bytes `01 00 MM KK` = enabled / mode / boot ROM key slot |
| `0x60` | **encrypted payload begins** |

Cipher:

- **3-key 3DES in ECB over 64-bit blocks**, operated through the hardware AXI FIFO word
  order: each 64-bit block is two 32-bit words, big-endian packed and word-swapped
  ($[W_1{:}W_0]$) on both ingress and egress.
- **Key source**: the silicon Boot ROM master key table at `0x012023B0`, entry
  `0x012023B0 + key_idx * 24`. In decrypt mode the hardware key registers load the six
  32-bit words in inverted stage order: `Key = pack(">IIIIII", w1, w0, w3, w2, w5, w4)`.

Known key slots (addresses are public; the 24-byte key material is **not**, and is
deliberately not reproduced in this documentation):

| Slot | Address | Protects |
|---|---|---|
| `0xB6` | `0x012034C0` | MC-101 `App1_Main`; SYSTEM-8 `App1_Main` |
| `0xB5` | `0x012034A8` | MC-101 `App0_UPD` recovery updater |

The SYSTEM-8 `App1_Main` container names slot **`0xB6`** and mode `0x02` (3-key 3DES) in
all of v1.30, v1.31 and v1.32. The mechanism and the key table are shared with the
Roland ESC2 firmware family (MC-101).

> **Key handling.** The key material is private. It is never stored in this repository,
> in `tools/`, in the test suite, or in these documents. Supply it at the point of use:
> `--key <48 hex digits>`, `--key-file <path|->`, or `--bootrom <silicon dump>` (which
> reads the table entry directly). `tools/app1_crypto.py` prints only a SHA-256
> fingerprint of the key, never the key itself.

Verification `[VERIFIED]`:

- The ported cipher reproduces the MC-101 `App1_Main` reference plaintext exactly.
- Decrypting the v1.32 payload yields a valid Cortex-M image (`0x680148B0`, `ldr r0,[pc,#…]`)
  whose **code segment matches `dumps/os_full.bin` byte for byte** up to the first
  live-RAM variable; the remaining 2.63 % are runtime-mutated `.data` values.
- Re-encrypting the decrypted payload reproduces Roland's shipped `SYSTEM8_UP.BIN`
  byte for byte (`cmp` clean), so a patched OS can be repackaged and installed.

Tool: [`tools/app1_crypto.py`](../../tools/app1_crypto.py) (`info`, `decrypt`, `encrypt`,
`selftest`), or `tools/sys8fw.py pack --os-plain <decrypted image> --os-key <hex>` to
rebuild the whole updater with a modified OS. `info` needs no key; the other commands
require one.

```bash
# needs no key
python3 tools/app1_crypto.py info firmware/sys8_sys_v132/untared/p1y01_m0c0a_up.bin

# decrypt / re-encrypt (key supplied per invocation, or via $APP1_KEY / a bootrom dump)
python3 tools/app1_crypto.py decrypt <container> -o os.bin --key-file ~/.roland-key
python3 tools/app1_crypto.py encrypt os.bin --header-from <container> \
        -o out.bin --key-file ~/.roland-key
```

> **`info` on v1.32:** `python3 tools/app1_crypto.py info firmware/sys8_sys_v132/untared/p1y01_m0c0a_up.bin`
> prints name `App1_Main`, build date `2022/04/28 09:41`, tool `0.010001`, load address `0x60000000`, payload size
> `0x2CDA70` (2,939,504 B), check `0x81D0EE9E`, crypto control `0xB6020001` (enabled, mode 2, key slot `0xB6`), encrypted
> span `[0x60 : 0x2CDAD0)`, clear trailer of 64 B. `tools/test_app1_crypto.py`: 12 tests, OK without the key (5 of them
> need the key and are skipped).


