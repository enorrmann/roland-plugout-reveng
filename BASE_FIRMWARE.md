# SYSTEM-8 base firmware (`SYSTEM8_UP.BIN`)

Findings on Roland's official updater (OS 1.32, build 2022/04/28) and the
toolchain built around it: `tools/sys8fw.py`. Companion to
[`CUSTOM_PLUGOUTS.md`](CUSTOM_PLUGOUTS.md) (PLUG-OUT images) and
[`PROTOCOL.md`](PROTOCOL.md) (USB install).

## Status

| Part | State |
|---|---|
| Updater container (tar), headers, sums | fully known; **rebuild is byte-identical** (`sys8fw verify`) |
| Slot-0 model + engine (`p0m`, `p0s`) and the 3 factory PLUG-OUTs (`pl1m`, `pl1s`, `pl2m`) | decoded, patchable, repackable; symbols/ELF export |
| `App1_Main` (the OS, 2.9 MB) | **encrypted in the updater, key not in the file** — but **plaintext in device RAM at `0x60000000`, now shown readable from a plugout** (§5.1) |
| OS start-up code (first 0xEE bytes of the image, literal pool, DMA helpers) | **dumped and disassembled from the device** (§6); memory map and section copies known |
| Recreate the OS from source | still blocked on re-encryption (cipher key); the plaintext is now **fully dumped** (`dumps/os_full.bin`, §7.14) |
| OS structure | **mapped** (§8): 6 programs (master BMC core 0/core 1, slave BMCs, STM32 panel CPU), loader, μT-Kernel, subsystems; tool `tools/os_analyze.py` |

## 1. `SYSTEM8_UP.BIN`

A GNU tar (`ustar  `, blocking factor 20 → 5 560 320 B = 543 × 10 240), six
members under `./_tmp/`, all mode 0777, uid/gid 1000:

| Member | Size | Content |
|---|---|---|
| `p1y01_m0c0a_up.bin` | 2 939 664 | `App1_Main`, the OS (encrypted payload) |
| `p1y01_p0m.bin` | 149 966 | slot 0 *model* region (SYSTEM-8, id 0x0005, v1.20) |
| `p1y01_p0s.bin` | 314 800 | slot 0 *engine* region (id 0x000A, v0.01) |
| `p1y01_pl1m.bin` | 740 646 | preloaded JUPITER-8 v1.06 (slot 1), `PRELOAD_PLUGOUT` |
| `p1y01_pl1s.bin` | 700 695 | preloaded JUNO-106 v1.06 (slot 2) |
| `p1y01_pl2m.bin` | 706 735 | preloaded JX-3P v1.00 (slot 3) |

`p1y01` is the product/updater code; `m`/`s` = model/engine region of slot 0.

### `PRELOAD_PLUGOUT` wrapper (`pl*.bin`), 0x40 bytes

| Offset | Field |
|---|---|
| 0x00 | `"PRELOAD_PLUGOUT\0"` |
| 0x10 | build date `"2022/04/28 09:46"` |
| 0x20 | `"00000000"` |
| 0x28 | u32 0 |
| 0x2C | u32 header size `0x40` |
| 0x30 | u32 payload size (file size − 0x40) |
| 0x34 | u32 byte sum of the payload |
| 0x38 | 2 × u32 0 |

Payload = region 0 at file offset `0x40`, **zero** padding, region 1
(`SYSTEM-8:Plugout`, the engine) at file offset **`0x80000`**. `p0m`/`p0s` are
the bare regions (no wrapper). Region format (header, LZSS, SYNINFO):
`CUSTOM_PLUGOUTS.md` §2–4. All regions here are stamped `22-0331_13:04:49`,
tool tag `0.01136E`.

### Link addresses (from SYNINFO `init`, 2 MiB windows)

| Image | Base |
|---|---|
| slot 0 model (`p0m`) | `0x61600000` |
| slot 0 engine (`p0s`) | `0x60800000` |
| slot 1 / 2 / 3 model (`pl1m`/`pl1s`/`pl2m`) | `0x61800000` / `0x61A00000` / `0x61C00000` |
| slot 1 / 2 / 3 engine | `0x60A00000` / `0x60C00000` / `0x60E00000` |

## 2. `App1_Main` (the OS)

Clear header (0x60 B), encrypted payload, clear trailer (0x40 B):

| Offset | Field |
|---|---|
| 0x00 | `"App1_Main\0…"`; 0x10 `"2022/04/28 09:41"`; 0x20 `"0.010001"` |
| 0x28 | u32 `0x60000000` (load address) |
| 0x30 | u32 `0x00060040`, then `FFFFFFFF` ×3 |
| 0x40 | u32 `0x00060060`; 0x44 u32 payload size `0x2CDA70` (file = 0x60 + payload + 0x40) |
| 0x48 | u32 `0x60000000`; 0x4C 0 |
| 0x50 | u32 `0x81D0EE9E` (probably a plaintext check value; not reproducible on the ciphertext) |
| 0x54 | u32 `0xB6020001`; 0x58 0; 0x5C `FFFFFFFF` |
| trailer | 12 × 00, `F6 96 92 95`, `"Roland P1Y01_C0A"`, `"VER.1.32"`, `"BLD.136E"`, `"22 0428 09 41 54"` |

### The cipher (negative result, with evidence)

* Payload entropy 7.70 bits/byte overall; 7.999 per byte-lane over distinct
  8-byte blocks; every one of the 64 bit positions is 0.50 ± 0.004.
* Identical plaintext blocks give identical ciphertext: one 8-byte block
  occurs **60 889** times (0x956C8…end), others 4 666, 3 350 … → a
  **64-bit block cipher in ECB**. Repeat statistics rule out a 128-bit block
  (8-byte halves repeat independently of their 16-byte neighbours).
* Avalanche is complete: the Hamming distance between consecutive distinct
  blocks averages 31.99; no low-weight differences from the dominant block,
  so it is not linear/affine or an XOR/rotate scheme.
* Not a repeating-XOR key (periods 4/8/16) and not the plug-out
  chained-XOR (`0xBACADABE`).
* Not found with keys taken from the header, trailer and product strings
  (≈ 560 candidates, lengths 4–56) for Blowfish, CAST5, 3DES/DES, IDEA, nor
  with TEA/XTEA (16/32/64 rounds, both endiannesses) using every 16-byte
  window of header+trailer.
* The key therefore lives in the device bootloader, outside this file.

Consequence: the OS cannot be decompiled from the updater alone, and a
modified OS cannot be re-encrypted. Everything else in the updater can.

## 3. What the decoded base engine contains

`sys8fw symbols` recovers Itanium-ABI RTTI → vtable → virtual methods
(names are compiler-mangled type names; method names are positional
`<Class>__v<N>`). For `p0s` (slot-0 engine): **97 classes, 2159 virtual
methods**.

* Voice/DSP blocks: `CDSPSystem8{Vco,Vcf,Vca,Env,Lfo,Efx,Out,Vol,Mst,CvOut}`
  and per-voice `…VcoVoice/EnvVoice/…`, effects `…Efx{Cr,Ds,Fz,Mt,Od,Ph}`,
  jack outputs `…JkOutMst/JkOutSlv`.
* **Filter and oscillator variants** that the factory PLUG-OUTs select:
  `VcfVari{00,01,Jp8,Ju106,Sys1,Fbof,Fmt2,Fmt3}`,
  `VcoVari00..03`, `LfoVari00..02`. Each block class has an `s`-prefixed
  *derived* class (RTTI base pointer) with the same method count: the base
  starts with a `bx lr` stub and aliases the derived functions, so `s…` holds
  the implementation. `_`-prefixed classes also exist (role not confirmed).
  Details: [`base-engine/BASE_ENGINE.md`](base-engine/BASE_ENGINE.md).
* Parameter classes: `CPrmDSPSystem8` (480 methods) and
  `CPrmDSPPlugoutSlave` (481 methods) in `p0s`; `CPrmDSPJp8` (382),
  `CPrmDSPJu106` (352), `CPrmDSPJx3p` (330) in the factory engines.
* **Interface classes** (the answer to "what does the OS call"):
  * every model region has one class, `CAssignSystem8` (slot 0) or
    `CAssignPlugout` (factory/custom slots), with **17 virtual methods**
    (positions 9–12 forward 16-bit arguments to the OS service table;
    others handle value get/set through virtual slots 0x24/0x28/0x30 of a
    helper object).
  * every engine region has `CInterface` with **1 virtual method**: the
    init-descriptor entry the OS invokes.
* Region 0 (model) of slot 0 is 1.16 MB but, like the PLUG-OUT models, almost
  all padding: SYNINFO, 64 factory preset records at `0x8000` (stride
  `0x200`), label tables, and ~20 KB of code at `0x118000`.

Run `sys8fw symbols work/up --json syms.json --elf-dir work/elf` to
regenerate the full tables; ELFs load in Ghidra/IDA as ARM LE Thumb-2.

## 4. Toolchain (`tools/sys8fw.py`)

```bash
S="python3 tools/sys8fw.py"
$S info    firmware/SYSTEM8_UP.BIN            # members, identities, OS header
$S verify  firmware/SYSTEM8_UP.BIN            # unpack + rebuild == original (≈30 s)
$S unpack  firmware/SYSTEM8_UP.BIN work/up    # sys8fw.json + <stem>.region<N>.bin
$S symbols work/up --json work/syms.json --elf-dir work/elf
$S disasm  work/up --out work/asm             # ELF + objdump listing per image
$S patch   work/up --image p1y01_p0s.region0.bin --at 0x60800100 --hex 00bf
$S pack    work/up -o out/SYSTEM8_UP.BIN      # recompress, resum, rewrap, retar
```

* `unpack` writes decompressed images (`*.regionN.bin`; for `pl*` N=1 is the
  engine) and a manifest holding tar headers and region header fields.
  `pack` recompresses with the same LZSS encoder (identical output),
  recomputes region sums, PRELOAD size/sum and tar checksums, and refuses an
  image that no longer fits before the next region.
* Edit images in place with `patch`, or replace `*.regionN.bin` wholesale.
  Anything linked to an absolute address must stay consistent (slot-0 images
  have no per-slot relocation; the slot 1–3 images in `pl*` are linked to
  their slot).
* `--os FILE` swaps the `App1_Main` member for an already-encrypted file.
* Tests: `python3 -m unittest discover -s tools -p "test_sys8fw.py"`.

**Do not install a repacked updater on a device without a recovery plan.**
Whether the bootloader validates anything beyond the sums written here is
unknown; the OS file is the part the bootloader certainly checks.

## 5. Open items / how to unblock the OS

1. **Obtain the OS plaintext from a running device.** Custom engine code
   already executes on hardware (`CUSTOM_PLUGOUTS.md`, TONE patch), and the
   OS image is mapped at `0x60000000`. A PLUG-OUT that copies
   `0x60000000…0x602CDA70` somewhere the host can read back (patch memory via
   RQ1, or an OS service that emits MIDI) yields the decrypted OS and, with
   it, known plaintext for the cipher. Needs: a read-back path (the
   service table at `0x0113FC00` is the place to look: `CAssign*` shows
   how the OS exposes it) and a hardware session.

   **Proven on hardware (2026-10-13, `JX-3P-MEM`, `CUSTOM_PLUGOUTS.md`):** a
   plugout init hook read bytes at `0x60000000` and wrote them (as hex) over
   its own name, which the host displays. The first 7 bytes read back as
   `B04801684FF6FF`, valid Thumb-2 — confirming `App1_Main` is present in RAM
   in plaintext and is readable from a plugout. The name field only carries
   7 bytes per install, so the remaining work is a wider read-back path to
   pull the full 2.8 MB image rather than proving access.
   **Update (2026-10-13):** the patch memory is at `0x60445957` in RAM, one
   nibble per byte, and RQ1 reads it back (up to 128 bytes per request, 64
   patches x 3 structs). The OS rewrites it after the PLUG-OUT init, so the
   write must come from a later hook (engine parameter sender). Once a write
   survives, the 2.8 MB image can be read at ~128 B per RQ1 instead of 7 B per
   install. Timings of the slot-name channel are in `CUSTOM_PLUGOUTS.md`.
2. **Bootloader key**: the same dump approach applied to the boot ROM/flash
   region (address unknown; found by probing) would reveal the algorithm
   and key, enabling re-encryption of a rebuilt `App1_Main`.
3. Name methods properly: diff the base engine against the factory engines
   (`CDSPSystem8Vcf` vs `CPrmDSPJp8` …) to label shared code, then export a
   symbol file per image.
4. Check `App1_Main` header fields `0x30/0x40/0x50/0x54` once plaintext is
   available (checksum, flash offset `0x60060`).

## 6. Findings from memory dumps of the running device (2026-10-13)

All of this was read from a real SYSTEM-8 (OS 1.32, slot 3) with
`examples/mem_name/dump_loop.py` (7 bytes per install through the slot name,
see `CUSTOM_PLUGOUTS.md`). Nothing below comes from the encrypted file.

### 6.1 Dump files (repository root, untracked)

| File | Bytes | Start address | Content |
|---|---|---|---|
| `os_dump.bin` | 238 | `0x60000000` | start of the OS image (reset handler); grows with each run |
| `os_helpers.bin` | 196 | `0x600001A0` | DMA init + copy + zero-fill routines |
| `os_pool.bin` | 161 | `0x600002C0` | literal pool of the start-up code |
| `os_dma.bin` | 42 | `0x600003D0` | end of the pool: VTOR, DMA register addresses |
| `mem_0000.bin` | 70 | `0x00000000` | vector table (a copy, see 6.2) |
| `rom_58c0.bin` | 70 | `0x010058C0` | fault handlers, PendSV (kernel code, RAM) |
| `rom_59a0.bin` | 70 | `0x010059A0` | BASEPRI helpers, kernel variables |

### 6.2 Memory map and start-up

* The core is a **Cortex-M** (`msr BASEPRI`, `PSP`, `PendSV`, SCB registers).
  Reset vector = `0x60000001`: the OS image runs in place at `0x60000000`.
* **Vector table at `0x00000000`** (aliased with `0x01000000`): SP =
  `0x01050800`, reset `0x60000001`; NMI/HardFault/MemManage/BusFault/
  UsageFault/DebugMon and IRQ0 point at `b .` stubs (`0x010058C0…CE`), i.e.
  **any fault freezes the synth** (no reset); SVCall `0x01005A1D`, PendSV
  `0x010058F1`, SysTick `0x01005AB5`.
* `0x0100xxxx` is **not a separate ROM**: it is RAM the start-up code fills
  from the OS image by DMA. The "kernel" code seen at `0x010058C0` is part of
  `App1_Main`.

Start-up (`0x60000000`): read-modify-write `SCB->AIRCR` (`0x05FA0300`,
`PRIGROUP = 3`), `BASEPRI = 0x10`, write `0x800` to `0x40026434`, init the DMA
(`0x600001A0`), then a scatter-load of seven copy/zero pairs:

| Op | Destination | Length | Source |
|---|---|---|---|
| copy | `0x01000000` | `0x400` | `0x602B9374` (vector table) |
| copy | `0x01000400` | `0x13FAC` | `0x602B9774` (kernel code and data) |
| zero | `0x010143AC` | `0x565C` | (`.bss`, ends `0x01019A08`) |
| copy | `0x01100000` | `0` | `0x602CD720` (empty) |
| zero | `0x01100000` | `0` | empty |
| copy | `0x01140000` | `0` | `0x602CD720` (empty) |
| zero | `0x01140000` | `0` | empty |
| copy | `0x20000000` | `0x308` | `0x602CD720` |
| zero | `0x20000308` | `0x8740` | zero word at `0x20027FFC` |
| copy/zero | `0x20020000`, `0x20080000`, `0x200A0000`… | `0` | `0x602CDA28` (empty) |

Source ranges end at `0x602CD720 + 0x308 = 0x602CDA28`, consistent with the
`App1_Main` payload (`0x2CDA70` bytes). The image therefore holds: code
(`0x60000000…0x602B9374`), then the data images above.

### 6.3 DMA engine

`0x600001A0` clears `0x40001000`, `0x40001020`, `0x40001024`, `0x40001028`,
`0x4000102C`, then sets bit 31 of `0x40001000`. `0x600001C2` (copy):
`[0x40001028] = src`, `[0x4000102C] = dst`, then chunks of 64-byte units
(`len >> 6`, at most 4096 per chunk), writing control words `0x18000000` /
`0x08000000` / `0xA0000000` to `0x40001024` and polling until
`(status & 0x70000) == 0x50000`; the tail is done in 4-byte units.
`0x6000023E` (zero fill): same engine, with source = a word zeroed in RAM.
It is a plain memory-to-memory DMA: no cipher constants, S-boxes or key
material appear in it or in the pool.

### 6.4 What this means for the cipher

* The OS is **plaintext in RAM** and nothing at or after `0x60000000` decrypts
  it: decryption happens earlier, in a loader (boot ROM / flash) not visible
  from a plugout. The `0x0100xxxx` region does not hide it.
* **Known plaintext**: the first 70 bytes of the OS against the first 70 bytes
  of the encrypted payload (`0x60` offset in the file). Facts derived from it
  and from the encrypted file (all three updater versions 1.30/1.31/1.32):
  * identical start-up ciphertext in 1.30, 1.31 and 1.32, i.e. no per-version
    tweak, no IV, no position dependence → ECB-like;
  * block of 8 bytes, **full diffusion** inside the block (a block differing
    from the most frequent one changes ~32 of 64 bits, never fewer than 16):
    XOR / byte substitution / permutation are ruled out; it is a real block
    cipher (Feistel or SPN);
  * the most frequent ciphertext block `6B4C9A852C732831` is 60 843 times
    (16.5 % of the payload), in long runs: the encryption of an all-zero
    block (padding / `.bss` images). `93B181AF5FD8EEC9` (4 666 times) is most
    likely `FF*8`. That gives ~10 known (P, C) pairs to test a candidate
    algorithm and key.
* `tools/crypto_scan.py` looks for TEA/XTEA/Blowfish/DES/AES/CAST/MD5/SHA/CRC
  constants and 256-byte permutation tables in a raw dump; nothing found in
  the dumps so far (they are too small to cover the code of any cipher).

### 6.5 Corrections to earlier notes

* An early reading of 7 bytes at `0x00000000` (`00 08 05 01 01 00 00`) looked
  like data; the 70-byte read shows a vector table.
* Sweeping `0x01000000+` to find the decryptor (suggested earlier) is not
  useful: it is a copy of the OS image.
* The only way to read the full 2.8 MB is a faster read-back path
  (§5.1: engine hook + RQ1 on the patch memory). See `CUSTOM_PLUGOUTS.md`,
  "Stepping through memory": `JX-3P-MEM-STEP`.


## 7. Looking for a SysEx send function in the OS (2026-10-14)

Goal: find OS code a PLUG-OUT hook can call to send arbitrary memory out as
SysEx (a much faster exfil path than 24 bytes per install). All reads on a
real SYSTEM-8 (OS 1.32, slot 3) with `examples/mem_names/dump_loop.py`
(~13 s per 24 bytes) and the new on-device search `examples/mem_scan/scan.py`.

### 7.1 New tool: on-device pattern search (`examples/mem_scan/`)

`scan.c` is a region 0 init hook (stock JX-3P, same base as JX-3P-MEM-ALL)
that scans `LO..HI` (default the whole OS image `0x60000000..0x602CDA70`) for
a masked byte pattern of up to 8 bytes (`?` = wildcard nibble), optionally
requiring a second pattern within `±WIN` bytes, and reports
`count | match1 .. match5` (6 x u32) over the three slot names. One install
(~15 s) searches the whole 2.8 MB image; `--skip N` pages through matches.
Every search is appended to `examples/mem_scan/scan_log.txt`.

    python3 examples/mem_scan/scan.py --pat "F0 41 ?? 00 00"
    python3 examples/mem_scan/scan.py --pat "?? 5A 00 60" --skip 5
    python3 examples/mem_scan/scan.py --pat "F0 2?" --step 2 --near "F7 2?" --win 64 --qstep 2

### 7.2 Dump files (`dumps/`, untracked)

| File | Start | Bytes | Content |
|---|---|---|---|
| `vec.bin` (+`vec.log`) | `0x01000040` | 960 | IRQ part of the vector table (IRQ0..IRQ239) |
| `tmpl_602c4d00.bin` | `0x602C4D00` | 72 | MIDI receive buffer (last DT1 received) |
| `tmpl_602ad770.bin` | `0x602AD770` | 48 | constant `F0 41 7E 00 00 00 00 F7` |
| `idreply_60005ab0.bin` | `0x60005AB0` | 72 | Identity Reply constant + MIDI class constructor |
| `vtbl_601182a0.bin` | `0x601182A0` | 96 | vtable of the MIDI class |
| `midi_cls_60005990.bin` | `0x60005990` | 288 | methods of the MIDI class |
| `thunks_600753c8.bin` | `0x600753C8` | 264 | veneers OS image → kernel copy at `0x0100xxxx` |
| `send_6000345c.bin` | `0x6000345C` | 552 | MIDI output router (start) |
| `send_pool_6000385c.bin` | `0x6000385C` | 48 | literal pool of the router |
| `router_cont.bin` | `0x60003684` | 528 | router, continued |
| `router_tail_60003894.bin` | `0x60003894` | 456 | router end (bits 6–13, 19) + pool `0x60003930` |
| `helper_600033b0.bin` | `0x600033B0` | 192 | helper `0x600033CC` (MIDI mode setting) |
| `thunks_60075390.bin` | `0x60075390` | 72 | veneers before `0x600753C8` (param access, wrap getters) |
| `bulk.bin` | `0x60000000` | 2048 | first bulk SysEx dump (7.13) |
| `bulk_2.bin` | `0x60000800` | 16384 | bulk, 512 messages (7.13) |
| `bulk_2048.bin` | `0x60000800` | 65536 | bulk, 2048 messages in one install (7.13) |
| `os_full.bin` | `0x60000000` | 0x2CDA70 | whole OS image, `bulk_loop.py` (7.14) |
| `kernel_01000000.bin` | `0x01000000` | 0x19A08 | live kernel RAM of master core 0: vector table, μT-Kernel + runtime, `.bss` (8.11) |
| `bulk_kernel.log` | | | log of the kernel dump |
| `ram_20000000.bin` | `0x20000000` | 0x8A48 | live `.data`/`.bss` of master core 0 (8.12) |
| `bulk_ram20.log` | | | log of that dump |
| `bulk_loop.log` | | | log of the last `bulk_loop.py` run |

Disassemble with
`arm-none-eabi-objdump -D -b binary -marm -Mforce-thumb --adjust-vma=<start> dumps/<file>`.

### 7.3 Vector table (IRQs)

Default handler `0x010058CF` (`b .`). Not default:

| Handler | IRQs |
|---|---|
| `0x010059C9` (common dispatcher, kernel) | 2, 6, 11, 20, 23, 27, 34, 47, 51, 52, 94, 96, 98, 107–109, 163, 176, 187, 188, 190, 191, 200, 201, 204, 208, 209 |
| `0x0100D039` | 3 |
| `0x60046D01`, `0x60045E1D`, `0x60046E81`, `0x6004713D` (OS image) | 5, 12, 13, 101 |
| `0x01010293` / `0x01010343` / `0x01011F3D` (one group, three times) | 24/25/26, 28/29, 205/206/207 |
| `0x0100CF11`, `0x0100CF1F`, `0x0100CF2D`, `0x0100CF3B` | 87, 88, 89, 90 |
| `0x010058D1` | 183 |

The SoC is unidentified, so the IRQ numbers do not name the UART/USB yet;
the four handlers in the OS image (IRQs 5, 12, 13, 101) are the first
candidates for the MIDI drivers.

### 7.4 SysEx in the image (scan results)

| Pattern | Count | Where |
|---|---|---|
| `F0 41` | 763 | mostly Thumb code (`orr.w` …) |
| `F0 41 10 00 00` | 1 | `0x602C4D1D` |
| `F0 41 ?? 00 00` | 2 | `0x602AD784`, `0x602C4D1D` |
| `F0 41 7F`, `F0 7E 7F 06 01` | 0 | |
| `00 00 7E 12` | 1 | `0x602C4D20` (same buffer) |
| `F0 7E ?? 06 02 41` | 1 | `0x60005ACC` |
| `?? 5A 00 60` (pointers to `0x60005Axx`) | 18 | `0x60005C50`, vtable `0x601182C0..F4`, `0x6027FEC4/E0/FC` |

* `0x602C4D1C`: `F0 41 10 00 00 7E 12 70 10 00 01 05 06 0E 04 62 F7` = the
  END message of the PLUG-OUT that was just installed (data = its byte sum),
  preceded by `0x60126BBC` and `1`. It is the **SysEx receive buffer**,
  i.e. `0x602B9374..0x602CDA70` (the data images already copied by the
  start-up code) is reused as RAM after boot.
* `0x602AD784`: `F0 41 7E 00 00 00 00 F7`, followed by pointers
  `0x60C3ED64`, `0x602ADD74`: a constant message template; no literal
  pointing at it (`84 D7 2A 60`: 0 matches), so it is reached through a base
  pointer.
* `0x60005ACC`: the complete **Identity Reply**
  `F0 7E 10 06 02 41 2E 03 00 00 00 03 00 00 F7` (family `0x032E`, member
  `0x0000`, version `00 03 00 00`).

### 7.5 The MIDI "system" class and its send path

Constructor `0x60005ABC`: `*(u32 *)0x60290580 = 0x601182B4` (vtable),
`*(u8 *)0x60290584 = 0x10` (device ID). The vtable at `0x601182B4` has 17
methods (`0x600059BD` … `0x60005AB9`); the interesting ones:

| Method | Address | What |
|---|---|---|
| helper | `0x600059AA` | `dev == 0x7F || dev == this->dev_id` |
| `+0x04` | `0x600059C2` | if `r3 == 0x27E` or `0x32E` and dev matches → `0x600754A8(0x2000000C, …)` |
| `+0x08` | `0x600059F2` | same filter → `0x600754B2(0x2000000C, …)` |
| `+0x2C` | `0x60005A32` | **Identity Request handler** (builds and sends the reply) |

`0x2000000C` (literal at `0x60005AC8`) is the object those calls receive.
The values `0x27E` / `0x32E` are model IDs (`0x32E` = SYSTEM-8 family).

Identity Request handler, `this`, `r1` = source port, `r2` = device ID:

```
buf[16] = Identity Reply (copied from 0x60005ACC); buf[2] = this->dev_id
Msg m;   0x600753CC(&m)                  ctor        -> kernel 0x01007629
m[6] = port & 15
         0x600754BC(&m, buf, 15)         set SysEx   -> kernel 0x010076E5
Wrap w;  0x600754C6(&w, &m)              wrap (12 B) -> kernel 0x01007595
         0x6000345C(port == 9 ? 0x800 : 0x1000, &w)  MIDI output router
         0x600753F4(&w)                  dtor        -> kernel 0x01007781
         0x600753E0(&m)                  dtor        -> kernel 0x0100768F
```

`0x600753xx–0x600754xx` are 10-byte veneers (`movw/movt ip; bx ip`) into the
kernel copy at `0x0100xxxx`; the full list is in `thunks_600753c8.bin`.

**`0x6000345C` = MIDI output router.** `r0` = destination bit mask, `r1` =
wrapped message. Type byte `[r1+7] >> 1` (3 or 4 take a special path). Each
bit hands the message to one output object (literal pool `0x6000385C`):
bit 4 → `0x6003F526(0x60CC4370)`, bit 2 → `0x60057CB8(0x60E69448, msg)`,
bit 3 → `0x6003F37C(0x60CC4370, msg)`, bit 0 → `0x60007794(0x60AADC8C, msg)`,
bit 1 → `0x6003E32A(0x60C43030, msg)`, bit 5 → `0x60052B8A(0x60E65810, msg)`,
then `0x600753C2(0x602C3DEC, msg)` gated by system parameters read through
`0x600753A4(obj, id, 1)` (ids `0x22C`, `0x22`, `0x1E`, `0x2A`, `0x2B`, `0x0C`
…: MIDI settings such as Rx/Tx switches).

The rest of the router is now dumped (`dumps/router_cont.bin`, `0x60003684..
0x60003894`; full router = that concatenated after `send_6000345c.bin`).
**Bits `0x800` (11) / `0x1000` (12) are NOT physical ports** — the router
never tests them as direct outputs (the only direct output bits are 0–5, plus
16/17/18 for the soft-thru / param paths). They are **logical port tags**: in
the lower layer a decoded cable/port number `r6` (from helper `0x600033CC` +
the message's port nibble) does `r6==8 → orr 0x800` (`0x60003804`), `r6==9 →
orr 0x1000` (`0x6000380A`) onto a base mask and then **recurses into the
router** (`bl 0x6000345C` at `0x60003778`, `0x600036E8`, `0x6000374A`). The
recursion's base mask is the literal **`0x00081603`** at `0x60003880` (bits
0,1,9,10,12,19); the other base mask is **`0x00080603`** at `0x60003874`. So
the resolved physical transmit is through the **low bits 0/1** objects
(`0x60007794@0x60AADC8C`, `0x6003E32A@0x60C43030`), chosen against the MIDI
routing params `0x20`/`0x22`/`0x23`/`0x2A`/`0x2B`. New literals:
`0x60003874 = 0x00080603`, `0x60003878 = 0x60CC40D4`,
`0x6000387C = 0x602AE5A2` (object for the `0x60041F88(obj,n,0)` calls),
`0x60003880 = 0x00081603`.

**Correction (later the same day, see 7.7):** bits `0x800`/`0x1000` *are* tested
as direct outputs at the end of the router (part not dumped yet when the
paragraph above was written).

Test on USB (2026-10-14): Identity Request `F0 7E 7F 06 01 F7` / `F0 7E 10
06 01 F7` sent on `SYSTEM-8 MIDI 1` and `MIDI 2` gets **no reply on either USB
port** (the PLUG-OUT query on MIDI 2 does reply, so capture works).

Test with the CH345 USB-MIDI adapter on the DIN ports (2026-10-14): Identity
Request on the DIN IN (CH345 OUT) gets **no reply on the DIN OUT** (CH345 IN)
and none on either USB port; and vice-versa. So the SYSTEM-8 does not answer
Identity Request on any transport in its current state — consistent with the
router finding above: emission is gated by the MIDI Tx/routing params (or the
Identity handler is not reached at all). Next: confirm by emitting a SysEx
from a hook with an explicit single router bit and watching the ports.

### 7.6 How a hook could use it (not tried)

A PLUG-OUT hook can rebuild the same sequence with absolute OS addresses
(constant for OS 1.32): build `F0 41 10 00 00 7E 12 <addr4> <data as
nibbles> <sum> F7` on the stack from OS memory, then `ctor`, `set SysEx`,
`wrap`, `router(mask, &w)`, `dtors`. Open points before trying it:
1. which router bit is USB `MIDI 2` (the host port) — see the test above;
2. call it from a later hook (engine sender / press hook), not from the
   init hook, which runs while the OS is still receiving the transfer;
3. size of `Msg`/`Wrap` (8 and 12 bytes on the handler's stack) and maximum
   SysEx length accepted by `set SysEx` (`0x010076E5`, to read);
4. any fault freezes the synth (power cycle; slots survive).

### 7.7 Full router, Wrap types and the CC path (2026-10-14)

With the end of the router (`router_tail_60003894.bin`), the helper
`0x600033CC` and the veneers `0x60075390..0x600753C8` dumped, the send path
is complete.

**Kernel veneers used by the MIDI code** (`0x6007539A..0x600754C6`):

| Veneer | Kernel | Role (from the call sites) |
|---|---|---|
| `0x6007539A` | `0x01007AF9` | `param_open(&acc, group)` (8-byte accessor on the stack) |
| `0x600753A4` | `0x01011A87` | `param_get(&acc, id, 1)` → value |
| `0x600753AE` | `0x01011A79` | `param_close(&acc)` |
| `0x600753B8` | `0x010075CB` | `wrap_get_packet(&w, &pkt4)` |
| `0x600753C2` | `0x01008DE7` | `queue_post(obj, &w)` (output objects of bits 5/9/16/17) |
| `0x600753CC` | `0x01007629` | `Msg` ctor |
| `0x600753D6` | `0x010075A9` | `wrap_get_msg(&w, &m)` |
| `0x600753E0` | `0x0100768F` | `Msg` dtor |
| `0x600753EA` | `0x010075B7` | **`wrap_packet(&w, &pkt4)`** (4-byte USB-MIDI packet) |
| `0x600753F4` | `0x01007781` | `Wrap` dtor |
| `0x600753FE` | `0x01008EFD` | post to another queue type (bits 7/8) |
| `0x600754BC` | `0x010076E5` | `Msg` set SysEx (`&m, buf, len`) |
| `0x600754C6` | `0x01007595` | `wrap_msg(&w, &m)` |

**Wrap** (12 bytes): `w[7] >> 1` is the type: **3 = SysEx `Msg`**, **4 = one
USB-MIDI event packet** `{cable<<4 | CIN, status, d1, d2}`. For type 4 the
router reads the packet back with `wrap_get_packet` and uses `pkt[0] >> 4`
(cable) and `pkt[0] & 15` (CIN: 8/9/B/E = note off/on/CC/bend); for type 3 it
uses `Msg[6]` (port) as the cable. So **CC / notes go out as type-4 wraps**,
no `Msg` needed.

**Router `0x6000345C(mask, &w)`, every bit** (pools `0x6000385C`, `0x60003930`):

| Bit | Mask | Target |
|---|---|---|
| 0 | `0x1` | `0x60007794(0x60AADC8C, w)` |
| 1 | `0x2` | `0x6003E32A(0x60C43030, w)` |
| 2 | `0x4` | `0x60057CB8(0x60E69448, w)` |
| 3 | `0x8` | `0x6003F37C(0x60CC4370, w)` |
| 4 | `0x10` | `0x6003F526(0x60CC4370)` |
| 5 | `0x20` | `0x60052B8A(0x60E65810, w)`, then like bit 16 |
| 6 | `0x40` | `0x6000EFC8(0x60BB3A20, w)` |
| 7 | `0x80` | `queue2_post(0x60445918, w)` |
| 8 | `0x100` | `queue2_post(0x604456E4, w)` |
| 9 | `0x200` | `queue_post(0x602C3DB4, w)` |
| 10 | `0x400` | `0x60051834(0x60DE09F0, w)` |
| 11 | `0x800` | `0x60004DA0(0x60445520, w)` |
| 12 | `0x1000` | `0x60004DA0(0x60445558, w)` |
| 13 | `0x2000` | `0x60004DA0(0x60445590, w)` |
| 16 | `0x10000` | `queue_post(0x602C3DEC, w)` if helper mode is 0, or 0x23–0x25 with a channel match (params `0x22`/`0x23`) |
| 17 | `0x20000` | `queue_post(0x602C3DEC, w)` if helper mode 0x26/0x27 and param `0x22C` and channel match |
| 18 | `0x40000` | "local/panel" path, below |
| 19 | `0x80000` | tail call `0x60004A64(0x60290514, w)` |

Bits 11/12/13 are three instances of one class (`0x60004DA0`, objects
`0x38` bytes apart) — the best candidates for three physical MIDI outputs.
The identity handler's `0x800`/`0x1000` choice is therefore "answer on output
A or B", not a logical tag.

**Helper `0x600033CC`** (no args) reads param `0x22C` (group 12) and `0x526`
(group 24); if both are set it reads `0x598`/`0x599` (group 27), else
`0xB4`/`0xB5` (group 3); it returns the second value, or 40 if the first is
1. A MIDI mode/channel enum (values 0, 1–17, 18–34, 35–39) used to gate bits
16/17.

**Bit 18 = how the panel transmits** (`0x60003578..0x6000377E`). For a type-4
wrap with cable 8 (the panel/local source) and param `0x1E` set: notes go to
the engine (`0x6003F1D2`/`0x6003F22E` on `0x60CC40D4`), CC 64 to
`0x6003E3E4`; then it reads params `0x0C`, `0x22`, `0x23`, `0x20`, **rewrites
the packet** (`pkt[0]` cable := 4; `pkt[1]` channel := param − 1), calls
`0x60041F88(0x602AE5A2, n, 0)` (n = 2/4/1, likely the MIDI-out indicator),
`wrap_packet`, and **`router(0x1A20, &w)`** (bits 5, 9, 11, 12). This is the
path that produced the `B3 47 xx` CCs (Tx channel 4) captured on USB1 and DIN
during the `midi_send` probe run (`examples/midi_send/cap_*.bin`).

### 7.8 Recipe: send a custom CC / SysEx from a hook (not tried yet)

CC (copy of what the panel path does, Tx-independent):

```
u8  pkt[4] = { 0x4B, 0xB0 | ch, cc, val };   // cable 4, CIN B
u8  w[12];                                   // Wrap
0x600753EA(w, pkt);                          // wrap_packet
0x6000345C(0x1A20, w);                       // router: bits 5,9,11,12
0x600753F4(w);                               // Wrap dtor
```

Variant honouring the user's Tx channel settings: `pkt[0] = 0x8B` (cable 8)
and `router(0x40000, w)` — the OS then does the rewrite itself (note: for
CIN 8/9 this also plays the note locally).

SysEx: same sequence as 7.6 but set `Msg[6] = 4` (the cable the panel path
uses) and use mask `0x1A20`. The `midi_send` probe v2 used `Msg[6]` = 0 / 9
with masks `0x1000`/`0x800`/`0x1A20` and produced no SysEx on USB1/USB2/DIN
(only the normal panel CCs), so the port number is the first suspect; it is
also not yet proven that the probe hook ran (`diag.s` / `read_diag.py`
exists to check it).

Suggested order: (1) CC with cable 4 + `0x1A20` from the engine hook (one
packet per panel move, distinct CC number, e.g. `B0 66 nn`); (2) if it shows
up, SysEx with `Msg[6] = 4`; (3) bisect `0x1A20` into bits 5/9/11/12 to name
the physical ports.

### 7.9 Hardware results of the CC send (2026-10-14)

* Engine sender hook (`midi_send/cc.s` on `0x60A0B41E`, cable 4 + `0x1A20`):
  no `BF 66` on any port, no freeze — but that hook was never proven to run
  (only the EFFECT TONE cave hook is a proven witness).
* `midi_send/cc_tone.s` in the verified EFFECT TONE cave (`make_cc_tone.sh`):
  * v1.08, ungated: the SYSTEM-8 **hangs as soon as slot 3 is selected**
    (preset load calls the TONE method → the send runs at load).
  * v1.09, send only when TONE == 255: selection, sound and inverted tone OK;
    **hangs when TONE reaches 255**, nothing on USB1/USB2/DIN. Moving TONE emits
    no MIDI of its own, so this is not router re-entry from the panel path.
  * Hypothesis: the engine parameter methods run in a context where the OS
    queues (`queue_post` `0x01008DE7`) must not be used (high-priority task /
    critical section / ISR → kernel assert loop), or the stack there is too
    small for the router. Next: `STAGE=1` (wrap + dtor, no router), then the
    router with single bits (`0x1000`, `0x200`).
* v1.11 (`STAGE=1`, wrap_packet + Wrap dtor, **no router**): **hangs at 255
  too** (a held note keeps sounding: the DSP runs on, the control CPU is
  stuck). So the router is not the (only) cause: already the first kernel call
  (`0x600753EA` → `0x010075B7`) hangs from the engine context. Next:
  `STAGE=0` (v1.10) only calls the pure OS-image function `0x600059AA(sp,
  0x7F)`; if that hangs too, engine code cannot execute OS code at all (MPU /
  privilege / XN), and the send must be done from a region 0 callback
  (OS context, where the init hook already reads OS memory) instead.
* v1.10 (`STAGE=0`, only a call to the pure OS function `0x600059AA`):
  **hangs too**. While the sound is hung, `plugout_tool.py info` still
  answers (slot 3 = `JX-3P CC TONE` 1.10): the OS CPU is alive, the processor
  running region 1 is stuck.

### 7.10 Two processors: region 0 = OS CPU, region 1 = engine CPU

Conclusion of 7.9 (matches the `CPrmDSPPlugoutMaster` / `CPrmDSPPlugoutSlave`
split of the SYSTEM-1 SH-101, `cleanroom/architecture.md`):

* **Region 1 (engine, `0x60A00000`)** runs on a separate processor. Any jump
  into OS addresses (`0x600xxxxx`, kernel `0x0100xxxx`) from there hangs it
  (different memory map or no execute right); the OS CPU keeps answering MIDI.
  So no OS MIDI function can be called from an engine hook.
* **Region 0 (`0x61800000`)** runs on the OS CPU: its init hook reads OS
  memory and writes the slot names (`mem_names`, verified). Every OS call must
  be made from region 0.

What region 0 of the JX-3P offers (slot 1 addresses, `tools/plugout_build.py
unpack`):

* SYNINFO `init` `0x6191823C` fills a descriptor at `0x6191B390` (`.bss`):
  `+4` = `0x619181B8` (lazily constructs two `CAssignJx3p` objects of 0xA4
  bytes at `0x6191B248` / `0x6191B2EC`, ctor `0x619196D4`, C++ guards at
  `0x6191B240/44`, pointers kept at `0x6191B234/38`), `+8` = `0x61918214`
  (clears those pointers), `+0xC..+0x14` = copies of `0x6191B234..3C`,
  `+0x18` = `0x61918228(sel, mode)` → `0x61919618` (set assign mode; calls
  vtable `+0x3C` with 4 or 8).
* Classes (RTTI): `CAssignJx3p` : `CAssignPlugout` : `CAssign` — the **voice
  assigner**. Vtable `0x6191977C`:

| Slot | Address | Role |
|---|---|---|
| `+0x00` | `0x6191941A` | calls `+0x38(obj[1])` (reset) |
| `+0x04` | `0x61919458` | `(r1 == 4)` → `0x61918472`, `0x619184B8` |
| `+0x08` | `0x61919470` | **note off** `(note)`: clears bit in bitmap `+0x4C`, dispatch by mode `[+0xC]` (0/1/2) to `0x61918DBA` / `0x61918FE4` / `0x61918EC6` with vel 0 |
| `+0x0C` | `0x619194AC` | **note on** `(note, vel)` (vel 0 = off), same dispatch |
| `+0x10` | `0x61919506` | **control change** `(cc, val)`: 1, 11, 64, 120, 121–127 handled, 7/10/65 ignored, others → `+0x34` |
| `+0x18` | `0x61919576` | **pitch bend** → `+0x24(obj, 2, 0x465, value)` |
| `+0x3C` | `0x61919434` | set mode |

  These run on the OS CPU on every note / CC, so they are run-time hook points
  for MIDI output (after the init test below succeeds).

### 7.11 First region 0 send test: `JX-3P-INIT-CC` — WORKS (2026-10-14)

`examples/midi_send/init_cc.s` + `make_init_cc.sh`: the init hook (cave
`0x61800100`, SYNINFO init → `0x61800101`, as `mem_names`) sends one
`BF 66 01` through `router(0x1A20)`, then writes `INIT CC OK` over its own
name and runs the stock init. Expected: the CC on USB1/DIN during the install,
`info` shows `INIT CC OK`. Risks: init runs while the OS finishes receiving
the transfer (the hangs of the minimal PLUG-OUTs were there); unknown whether
init also runs at boot. If it works, the next step is a SysEx memory dump at
init (`Msg` + set SysEx, `Msg[6] = 4`) — kilobytes per install instead of 24
bytes — and note-on hooks for run-time sending.

**Result on hardware (2026-10-14): it works.** Captured during the install
(`amidi -d` on USB1 `hw:3,0,0` and the CH345 on DIN; USB2 is used by the
installer): `B3 00 55 · B3 20 00 · C3 00` (bank/program change the OS sends
after the install) followed by **`BF 66 01` on both USB1 and DIN**; `info`
then shows slot 3 as `INIT CC OK` (router returned), no hang. So:

* region 0 code can call the OS MIDI output: `wrap_packet` `0x600753EA` →
  `router(0x1A20, &w)` `0x6000345C` → `Wrap` dtor `0x600753F4`;
* mask `0x1A20` with cable 4 reaches **USB1 (`SYSTEM-8 MIDI 1`) and DIN OUT**;
* the init context (end of the install transfer) is safe for it.

Captures: `examples/midi_send/cap_init_USB1.txt`, `cap_init_DIN.txt`.

### 7.12 SysEx from region 0: `JX-3P-INIT-SX` — WORKS (2026-10-14)

`examples/midi_send/init_sx.s` + `make_init_sx.sh` (`SRC`, `LEN` ≤ 32,
`MASK`): the init hook builds
`F0 7D 01 <SRC as 8 nibbles> <LEN bytes as 2 nibbles each> F7` on the stack
and sends it with the Identity-handler sequence:

```
Msg m;  0x600753CC(&m)            // ctor
m[6] = 4                          // port/cable 4 (as the panel path)
        0x600754BC(&m, buf, len)  // set SysEx (len includes F0 and F7)
Wrap w; 0x600754C6(&w, &m)        // wrap_msg
        0x6000345C(0x1A20, &w)    // router -> USB1 + DIN
        0x600753F4(&w); 0x600753E0(&m)   // dtors
```

Result on hardware (`SRC=0x60000000`, `LEN=16`): received on **USB1 and DIN**
right after the post-install bank/program change:
`F0 7D 01 06 00 00 00 00 00 00 00 0B 00 04 08 … 0E 0A F7` → address
`0x60000000`, data `b0 48 01 68 4f f6 ff 02 01 ea 02 01 ae 4a 41 ea`,
identical to `os_dump.bin`. Name `INIT SX OK`, no hang. Captures:
`examples/midi_send/cap_sx_USB1.txt`, `cap_sx_DIN.txt`.

Why the earlier attempts failed: they ran in region 1 (engine CPU, 7.10);
the `midi_send/probe.s` hook on the DSP sender most likely never ran an OS
call successfully for the same reason.

### 7.13 Bulk dump over SysEx: `JX-3P-INIT-BULK` — WORKS (2026-10-14)

`examples/midi_send/init_bulk.s` (224-byte init hook: the 7.12 sequence in a
loop, `COUNT` messages of `LEN` ≤ 32 bytes from `SRC`, then `BULK OK` over the
name), `make_init_bulk.sh` (`SRC`, `LEN`, `COUNT`, `MASK`) and the host driver
`bulk_dump.py` (builds, raw-captures USB1 `hw:3,0,0` with `amidi -r` while
installing, waits for the port to go idle, decodes, checks address
continuity, writes `-o`; `--decode FILE` decodes an existing capture).

    python3 examples/midi_send/bulk_dump.py --start 0x60000000 --count 64 -o dumps/bulk.bin

First run (64 × 32 bytes): **64/64 messages, 2048/2048 contiguous bytes from
`0x60000000`**, 16 s for install + capture (4872 raw bytes), slot 3 shows
`BULK OK`, no hang → `dumps/bulk.bin`. That is ~85× the 24 bytes per install
of the slot-name channel.

**COUNT = 2048 in one install (2026-10-14):**

    python3 examples/midi_send/bulk_dump.py --start 0x60000800 --count 2048 -o dumps/bulk_2048.bin

2048/2048 messages, **65536/65536 contiguous bytes** from `0x60000800`,
16.4 s install + capture (155656 raw bytes), `BULK OK`, no hang. The install
time does not depend on `COUNT` (16 s for 64 and for 2048 messages): the
sends run inside the init hook while the install is processed. About 24000 of
the 65536 bytes are `>= 0x80`; they come through unchanged (nibble encoding).

**Verification against the slow, proven channels** (all identical, 0 diffs):

| Bulk data | Compared with | Range |
|---|---|---|
| `bulk.bin` | `os_dump.bin`, `os_dump_2.bin`, `os_helpers.bin`, `os_pool.bin`, `os_dma.bin` (§6, `mem_name/dump_loop.py`, 7 B/install) | `0x60000000`–`0x600003FA` |
| `bulk_2048.bin` | `bulk_2.bin` (separate install, 512 messages) | `0x60000800`–`0x60004800` |
| `bulk_2048.bin` | new checkpoints with `mem_names/dump_loop.py --count 48` (24 B/install) | `0x60000800`, `0x60008400`, `0x600107D0` (last 48 B), 48 B each |

Checkpoint command (one per address, ~30 s each):

    python3 examples/mem_names/dump_loop.py --slot 3 --start 0x60008400 --count 48 --no-resume -y --read-wait 1.0 -o chk.bin

Still not measured: `COUNT` above 2048 and `LEN` above 32 (the stack buffer
is 80 bytes). Not needed: 64 KiB per install already makes a full OS dump a
matter of minutes (7.14).

### 7.14 Full OS dump: `bulk_loop.py` (2026-10-14)

`examples/midi_send/bulk_loop.py` runs `bulk_dump.py` repeatedly and builds
**one** image, resumable like `examples/mem_names/dump_loop.py`:

* `-o` (default `dumps/os_full.bin`) always holds the bytes from `--start`
  contiguously. If it exists, the loop continues after the bytes already in
  it; `--no-resume` starts over.
* Each chunk (`--count` × `--len`, default 2048 × 32 = 64 KiB) is dumped to
  `<out>.part` and appended to the image only when it is complete; the last
  chunk is rounded up to whole messages and cut back to `--end`.
* A failed chunk is retried (`--retries`, default 2); the loop stops if it
  still fails, or at once if the device does not answer the info query
  (power-cycle, then rerun to resume).
* Ctrl-C finishes the current chunk, then stops (the child runs in its own
  session, so the install is never cut in half).
* Default range: the OS image, `0x60000000` + `0x2CDA70` (App1_Main payload
  size, §6) = 45 chunks.

```bash
python3 examples/midi_send/bulk_loop.py > dumps/bulk_loop.log 2>&1   # whole OS / resume
python3 examples/midi_send/bulk_loop.py --start 0x60000000 --end 0x60100000 -o part.bin
```

Timing: ~40 s per 64 KiB chunk (build + 16.5 s install/capture incl. 3 s idle
wait + info query), ~1.6 KB/s overall, ~30 min for the whole OS.

**Early stop of the capture — tried and REVERTED (2026-10-14):** a version
of `bulk_dump.py` stopped the capture as soon as all `COUNT` messages were
decoded (raw size ≥ `COUNT × (12 + 2·LEN)`, plus 8 bytes of CC/PC
`B3 00 55  B3 20 00  C3 00` that the install puts on USB1), skipping the
3 s `--idle` wait (16.5 s → 13.6 s). One chunk later the next install
stuck on the device display at "receiving plg 3" (`no reply on 71 30 03 00
after 3 x 5 s`, no info answer, power-cycle needed). The device seems to need
the pause after an install before the next one; the 3 s idle wait is kept
(25 chunks in a row without problems with it).
After the power-cycle the same chunk `0x60190000` dumped cleanly with the
idle wait back, so the address was not the cause. `bulk_loop.py` also waits
`--settle` s (default 3) before every install, as an extra margin.

Pitfall: killing `bulk_loop.py`/`bulk_dump.py` from outside (not Ctrl-C) can
leave an `amidi -r` capture running for up to 63 s, holding the USB1 port;
wait for it to exit (or kill it) before the next run.

**Run (2026-10-14):** a first version wrote one file per chunk; 6 chunks
(`0x60000000`–`0x6005FFFF`) were merged into `os_full.bin` (the overlap with
`bulk_2048.bin` is identical) and the new loop resumed from 384 KiB.
Stopped at 55.7 % by the hang above: `os_full.bin` = 1638400 bytes
(`0x60000000`–`0x6018FFFF`, complete chunks only); resume from `0x60190000`.

**Result: COMPLETE.** After the power-cycle the loop resumed and ran the
remaining 20 chunks without a single retry (859 s, ~43 s per chunk with
idle wait + settle). `dumps/os_full.bin` = **2939504 bytes (`0x2CDA70`),
`0x60000000`–`0x602CDA6F`**, sha256 starts `2600b866f751f38a`.

Verification against every earlier dump (slow channels and bulk):

| Files | Result |
|---|---|
| `os_dump.bin`, `os_dump_2.bin`, `os_helpers.bin`, `os_pool.bin`, `os_dma.bin` (§6) | identical |
| `bulk.bin`, `bulk_2.bin`, `bulk_2048.bin` (7.13) | identical |
| 3 checkpoints `mem_names/dump_loop.py` (7.13) | identical |
| all `dumps/*.bin` of 7.2 (router, thunks, vtable, MIDI class, Identity Reply, `tmpl_602ad770`, …) | identical |
| `tmpl_602c4d00.bin` (MIDI receive buffer) | 5 bytes differ at `0x602C4D28`–`0x602C4D2C`: data + checksum of the last DT1 received (`F0 41 10 00 00 7E 12 70 10 00 01 xx xx xx xx sum F7`); live RAM, expected |

Note: the range is the mapped OS area at run time, not only the code: it
also holds live RAM (e.g. the MIDI buffer at `0x602C4D00`), so two dumps
can differ in such variables.

## 8. Anatomy of the decrypted OS image (2026-10-14)

Source: `dumps/os_full.bin` (§7.14), the whole `App1_Main` as it sits in the
master's SDRAM at `0x60000000` while the synth runs. Tool:
`tools/os_analyze.py` (capstone linear sweep; functions = `BL` targets;
per-function callees/callers, strings, MMIO, RAM; ELF with `fn_*`/`s_*`
symbols for Ghidra/IDA):

```bash
python3 tools/os_analyze.py dumps/os_full.bin -o work/os --parts
#   work/os/functions.json, report.txt, os_full.elf          (whole image)
#   work/os/<part>/<part>.bin/.elf, functions.json, report.txt (each program at its own link address)
```

### 8.1 The image is six programs for four kinds of CPU

`App1_Main` is not one program. It is a bundle with its own loader. Proof:
four copies of the C library tables (`0123456789abcdef@0x` at `0x60115D50`,
`0x6016EAA0`, `0x6020A8C4`, `0x602B8B08`); the code blocks call almost
only themselves (`BL` graph); each block has its own NVIC accesses
(`0xE000E000`); the loader at `0x60001E58`/`0x60001EBE` copies or sends the
blocks to other addresses/CPUs (8.3); and each extracted part points at its
own strings only when placed at that run address.

| Part (`--parts` name) | In the image | Size | Runs at | CPU | Content |
|---|---|---|---|---|---|
| `m0_core0` | `0x60000000` | `0x130E00` | in place | **master BMC, core 0** | the OS: code `0x60000000–~0x60094800`, then `.rodata`/tables, menu strings (`0x60128000+`), `.data` (`0x60130Bxx`) |
| `m1_tcm` | `0x60130E00` | `0x42BBC` | `0x01100000` | **master BMC, core 1** | DSP control + global effects (8.5) |
| `m1_sdram` | `0x60173A00` | `0x7D74C` | `0x61000000` | master BMC, core 1 | debug console screens (`CEZUtil*`), MIDI→DSP command parser |
| `s_boot` | `0x601F1200` | `0x1D328` | `0x01100000` (on the slave) | **slave BMCs** | `BOOT` image, DSP control (8.6) |
| `s_data` | `0x6020E600` | `0x7D114` | `0x60000000` (on the slave) | slave BMCs | `DATA` image, debug console screens |
| `subcpu` | `0x6028B730` | `0x4D68` (u32 at `0x6028B724`) | `0x08000000` | **STM32F1** | panel/keyboard controller firmware (8.7) |
| — | `0x60290498` | | | | records with 64 names + `----------------`, period `0x6D0`, the 16-bit units of each name scrambled ("t FaliSp" = "Split Fat"); partly modified at run time; probably the performance list. Not decoded |
| — | `0x602A8000` | | in place | master core 0 | variables (modified at run time), then from `0x602AF008` **boot/link code**: `This is Master BMC.` / `This is Slave BMC 0.`, `BOOT CORE1`, `CORE1 READY`, `RAM Clear`, USB host stack (EHCI/OHCI, `BMCBulk%d Attached`) |
| — | `0x602B9374` | `0x146FC` | | | scatter-load sources (vector table, kernel for `0x01000000`, `.data` for `0x20000000`) — **all zeros in the dump**: after start-up the OS reuses this SDRAM (e.g. the MIDI receive buffer at `0x602C4D00`). The kernel (`0x01000400–0x010143AC`) is therefore *not* in `os_full.bin`; it was dumped live separately (8.11) |

Counts (`os_analyze`): `m0_core0` 4065 called functions; `m1_tcm` 520;
`m1_sdram` 254; `s_boot` 540; `s_data` 264; `subcpu` 133.

### 8.2 Hardware picture derived from the code

```
                 USB host (EHCI/OHCI), "BMCBulk" links
   ┌──────────────────────────┐ ───────────────► ┌──────────────────────┐
   │ BMC #0 "Master"          │ ───────────────► │ BMC #1 "Slave0"      │
   │  core 0: OS (m0_core0)   │                  │ BMC #2 "Slave1"      │
   │   μT-Kernel, UI, MIDI,   │                  │  core 1: s_boot      │
   │   SD, USB, PLUG-OUT mgmt │                  │  DATA: s_data        │
   │  core 1: m1_tcm/m1_sdram │                  │  DSP0..5 + DSPM      │
   │   DSP control, effects   │                  │  (voices, PLUG-OUT   │
   │  DSPs, SDRAM 0x60000000  │                  │   engine region 1?)  │
   └──────────┬───────────────┘                  └──────────────────────┘
              │ USART (STM32 system bootloader for updates)
   ┌──────────┴───────────────┐
   │ STM32F1 "Kbd"/panel CPU   │  switches, encoders, sliders (A/D), LEDs,
   │ (subcpu, 19.8 KB)         │  keyboard ("CMSK261WithFM3Drv")
   └──────────────────────────┘
```

* **BMC** = Roland's custom chip; three of them: `Master`, `Slave0`,
  `Slave1` (`BMC #%d(%s) CORE#%1d`, `Test BMC0/1/2`, `BMC Slave Adrs = %d`,
  `Check BMCChipID`). Each has a core 0 and a core 1 (`Check CORE0 Exists`,
  `CORE1 Build #`, `<CORE1_SELF_BOOT>`) and several DSPs (`DSP%c`, classes
  `CDSP0Prm`…`CDSP5Prm`, `CDSPMPrm`, `DSP96K`, error texts
  `PDH/TBW Access (DSP …)`, `Ill Inst`, `Zero Divide`).
* Master → slaves: USB host on the master (`0x40060000`/`0x40064000`
  EHCI/OHCI register pages), the slaves appear as `BMCBulk` devices; the
  boot images go over this link (8.3).
* This explains §7.10 (region 1 code "runs on another CPU"): on a slave, the
  address `0x60000000` holds `s_data`, not the master's OS, so an engine hook
  that calls a master OS address jumps into unrelated code. Where exactly
  each slot's engine (`0x60800000 + 0x200000·slot`) is executed: on the
  slaves only, not on master core 0 or core 1 (proven in 8.13).

### 8.3 Start-up and loading (master core 0)

1. Reset `0x60000000` (§6.2): AIRCR, BASEPRI, DMA init, scatter-load of
   vector table/kernel/`.data` from `0x602B9374…` (now zero, see 8.1).
2. `0x60001E58`: copies `0x7D74C` bytes `0x60173A00 → 0x61000000`
   (`m1_sdram`, checked against the window `0x61000000–0x61600000`), then
   `0x42BBC` bytes `0x60130E00 → 0x01100000` (`m1_tcm`, window up to
   `0x01144000`); then loops on two kernel calls (`0x0100C173`, `0x0100C1A1`)
   with a call `0x600751F6(10)` between tries (probably a 10 ms delay; the
   kernel is not dumped, so the meaning — most likely "core 1 is up" — is
   inferred), and continues at `0x60001560`.
3. `0x60001EBE`: sends the slave images, each as a 16-byte header
   `{'BUDB', length, load address, 'DATA'|'BOOT'}` followed by the data, through
   kernel function `0x010125C1` (header buffer `0x602BB600`):
   `DATA` = `0x6020E600`, `0x7D114` bytes → slave `0x60000000` (window
   `0x60000000–0x60800000`); `BOOT` = `0x601F1200`, `0x1D328` bytes → slave
   `0x01100000`.
4. `0x600021D0` & co.: **STM32 update** — reads the panel CPU version
   (`version %d : %d  build %d : %d`) and, if needed, rewrites it through
   the STM32 UART system bootloader (`Error UnProtectReadOut`,
   `Error SelectUART`, `ERASE ERROR.`, `SUB CPU UPDATE: Writing now...`,
   `Comparing.....`, `Completed.`) with the image at `0x6028B730`.
5. Boot modes (`0x60001C9E`): `BOOTMODE_NORMAL`, `BOOT_MODE_FACTORY_RESET`.

The version screen (`0x60076130`) names the parts the way Roland does:
`Ap0` (application), `Dt0` (data), `Ld0` (loader), `Up0` (updater), `Kbd`
(panel CPU).

### 8.4 Master core 0: the OS proper

* **RTOS: μT-Kernel** (T-Engine) — `## MemPool Usage Report uTKernel Task ##`,
  `## Kernel Resource Usage Report ##` with `tsk sem flg mbx mtx mbf por mpl
  mpf cyc alm` (`0x6004AAC4`). Kernel services are reached through veneers
  `0x600753xx` → `0x0100xxxx` (§7.5). Object names seen: `System`, `Idle`,
  `MidiIn`, `MidiOut`, `MidiOutUsb`, `KbdRx`, `KbdWdg`, `Calb`, `QSPIDlyP`.
* **UI / menus**: table-driven, with the item macros kept as strings
  (`XITEM_CHAR_VALUE_BOX(…)`, `XITEM_LIST_PRM`, `XITEM_EDIT_PRM`); edit marks
  `PATCH_EDIT_MARK`, `PERF_EDIT_MARK`…; write/exchange/copy dialogs
  (`PATCH_WRITE_NUM`, `PATCH_EXCHANGE_SRC/DST`, `SEQ_COPY_SRC_NUM`), name entry
  (`ABCDEFGHIJ…` charset at `0x60017214`). Display: LCD + 7-segment LEDs
  (`LCD Check`, `Contrast Check`, `Debug LED 7Seg`).
* **Storage (SD card, FAT12/16/32, drive `A:`)** (`0x6000B652…0x6001D1C8`):
  `A:/ROLAND/SYSTEM-8/BACKUP/BACKUP%03d.bin`,
  `…/DUMP/PATCH|PERFORM/DUMP%03d.json`,
  `…/EXPORT/PATCH/EXPORT%03d.s8p`, `…/EXPORT/PERFORM/*.s8f`,
  `sys8_bak.bin`, `sys8_prf.s8f`, `sys8_pat.tsv`; INI-like text sections
  `[SYSTEM-8]`, `[PATCH]`, `[PERFORMANCE]`, `Version = %X.%X%X`. SD
  controller page `0x4010B000` (`SDC Attached`). USB mass-storage wrapper
  `UhMscWrp`.
* **Sequencer/arpeggiator** (`0x60011678`): step sequencer (`STEP SEQ`,
  `STEPLOOP`, `REAL REC`, `FIRST/LAST STEP`, 64 steps in 4 pages,
  `GATE Time`), chord memory (`0x6001A3B4`), arpeggio destination
  (`ARP_DST`), vocoder input (`IN_VOC`).
* **Panel / controls**: `SLIDER1…8`, `CTRL2…8`, `FUNC1…8`, `TEMPO`, `PLAY`,
  `STOP`, `RESET`; panel monitor (`Switch %3d [%-16s] ON/OFF`,
  `Encoder %2d`, `A/D %3d`, `Bender`, `Modulation`, `Aftertouch`, `Hold`,
  `Pedal%d`); keyboard driver class `CMSK261WithFM3Drv` /
  `CKeyboardDrvInterface` (the only RTTI class of core 0, `0x60115F5F`).
* **PLUG-OUT management**: slots `SYS8`, `PLG1…3` (the SYSTEM-8 itself is
  slot 0), `PLG%d:%11s`, `PLGOUT_REMOVE_TARGET`, preload checks
  `PRE1…3: Ver. %X.%X%X / Error! / OK`, `PLGREADY_0007`, `PLG0: Error!`,
  `JUNO-106 / Coming soon!`. The USB install protocol (`PROTOCOL.md`) and the
  MIDI router (§7.5–7.7) are in this program.
* **MIDI**: router `0x6000345C` (§7.7), SysEx `SendDT1`, USB-MIDI out
  (`MidiOutUsb`), Identity Reply (§7.2).
* **Test / factory menus** (`0x600365DC…0x6003CA24`, `0x60075C62…`):
  `TEST MENU`, `Test SD/Sw/Enc/Kbd/BMC0-2`, `FX-RAM`, `SlaveBMC`,
  `USB Func`, `SD CARD`, **calibration** of the analog I/O
  (`CALIB_CV_OUT_max`, `CALIB_GATE_OUT_min/max`, `CALIB_TRIG_IN_min/max`,
  `CALIB_BENDER_min/max`, `CALIB_LFO_RATE_min/max`, `CALIB_CUTOFF_min/max`,
  `CV OUT 0V/10V`, `GATE OUT 0V/10V`, `TRIGGER IN`), `Factory Reset`,
  `QC Test Mode`, audio test tones (`OUTPUT :SINE`, `PHONES :SINE`), CORE1
  debug console (`<CORE1 Debug Console (Adrs=%08Xh)>`).
* Signals: `SIGABRT`, `SIGPVFN` (pure virtual) handlers at `0x60074178`.

### 8.5 Master core 1: DSP control and global effects (`m1_tcm` + `m1_sdram`)

48 RTTI classes / 1417 virtual methods in `m1_tcm`, 19 in `m1_sdram`:

* `CBmcDspMgr` (67 methods), `CFSPDrv*` (`Sys`, `M`, `01`, `23`, `45`: one
  driver per DSP pair), `CPIPCDev/Drv` (inter-processor communication),
  `CMIDIRx`, `CUMDWRx/Tx`, `CMIDIToUMDW` (internal MIDI/parameter stream
  "UMDW"), `CASGNUMDWRx` (assigner input), `CVPXMonIn`.
* **The SYSTEM-8 global effects and master section**: `CDSPSystem8Rev`,
  `…DlyCh`, `…DlyDly`, `…DlyPan`, `…DlyChSt`, `…DlyFlSt`, `…DlyMfx1`,
  `CDSPSystem8Mst`, each with `s`-prefixed implementations per part
  `Upr`/`Lwr`/`Ext` (upper part, lower part, external input), plus the
  parameter class `CPrmDSPSystem8` (326 methods). Same naming family as the
  base engine `p0s` (§3).
* Start-up/log texts: `BMCInit`, `BMCInitUDL()`, `CORE I/O Init`,
  `ThreadMain`, `CORE1 DSP Ready`, `EnableDAC (P1Y01)`, `EnableAmp`,
  `MuteTotalOn/Off` — core 1 owns the DAC/amp enable and mute.
* DSP driver `..\..\..\modules\dsp_api\bmc\bmc_dsp_drv.cpp`,
  `App\dsp_config\dsp_common_memcpy.cpp`; DSP register block `0x41100000`
  (`+0x2000…+0x9000` per DSP: `StartTrig(MEVF ID=%d)`).
* `m1_sdram`: debug console screens (`CEZUtilFIFO/Thread/STimer/DSPInt/
  DSPDump/DSPInfo/DSPProbe/MXM/IDMEMSum/Ver/Console`), `CMidiDSPControl`
  with `…Command`/`…XMLTag` (a MIDI-driven DSP control protocol), and the
  version screen (`Appli Version`, `Max Voice`, `Total Voice`).

### 8.6 Slave BMCs (`s_boot` + `s_data`)

Same software family as master core 1 (424 of ~440 strings shared), but:
no SYSTEM-8 effect classes; instead generic per-DSP parameter classes
`CDSPPrm`, `CDSP0Prm…CDSP5Prm`, `CDSPMPrm`; texts
`UpdateSynthPatch: no plugout.`, `UpdateSynthParam: no plugout.`,
`BMCInitDAC()`, `Check BMCChipID`, `DSP%d: Stop ENV`, and source file
`App\src_app\dspc\dspc_thread.cpp`. So the **slaves produce the voices**,
with the synth engine supplied by the PLUG-OUT (`no plugout` when a slot is
empty); master core 1 adds the effects and the master section.

### 8.7 Panel CPU: STM32F1 (`subcpu`)

Vector table at `0x6028B730`: SP `0x200023B8`, reset `0x0800035D`.
Peripherals used: GPIOA–E (`0x40010800…0x40011800`, so a 100-pin package),
ADC1 `0x40012400`, TIM1 `0x40012C00`, TIM8 `0x40013400` (TIM8 → high-density
F103), TIM4 `0x40000800`, USART1 `0x40013800` (link to the master), RCC
`0x40021000`, FLASH `0x40022000`. 133 functions, almost no strings. It scans
switches/encoders/sliders, drives LEDs and reads the keyboard; the master
side runs tasks `KbdRx`/`KbdWdg` and can reflash it (8.3, `Update Panel.`).

### 8.8 Peripheral pages of the BMC (from code use)

| Page | Used by | Identification |
|---|---|---|
| `0x40001000` | core 0 start-up | memory-to-memory DMA (§6.3) |
| `0x40026000` | all BMC programs | system/chip control (most used; BMC init) — unconfirmed |
| `0x40031000` | cores 0/1 | PDMAC (`CPDMACXfer`) |
| `0x40054000` | cores 0/1 | inter-core (`Check CORE0 Exists`) |
| `0x40060000`, `0x40064000` | core 0 boot/link | USB host EHCI / OHCI |
| `0x40062000` | core 0 | USB host (`USBH Attach`) |
| `0x40080000` | core 0 | heavily used, not identified (USB device / MIDI?) |
| `0x4010B000` | core 0 | SD card controller |
| `0x41100000` + `0x2000·n` | core 1 / slaves | DSP blocks |
| `0xE000E000`, `0xE0043000` | all | Cortex-M SCS (NVIC/SysTick), debug |

### 8.9 Where things live, in short

| Synth function | Where |
|---|---|
| Panel, display, menus, patches/performances, sequencer/arp, SD, USB, MIDI, PLUG-OUT install | master BMC core 0 (`m0_core0`, run in place at `0x60000000`) |
| PLUG-OUT *model* region (region 0, `0x61600000 + 0x200000·slot`) | master core 0 address space (§7.10: region 0 hooks can call the OS) |
| Effects (reverb, delays, chorus, MFX) and master out, DAC/amp | master BMC core 1 (`0x01100000` + `0x61000000`) |
| Voices / PLUG-OUT *engine* (region 1, `0x60800000 + 0x200000·slot`) | slave BMCs (`s_boot` + `s_data`) only (8.13) |
| Keys, knobs, sliders, LEDs | STM32F1 panel CPU, reflashed by the OS |
| CV/Gate out, trigger in, bender/LFO/cutoff calibration | master core 0 (test menu), via the BMC A/D–D/A |

### 8.10 Open items for this analysis

1. ~~Dump the kernel `0x01000000–0x01019A08`~~ (8.11) and ~~`0x20000000–0x20008A48`~~
   (8.12) — done. Still to do: name the remaining kernel/runtime functions
   behind the 247 veneers (`work/os/kernel/veneers.txt`).
2. Ghidra: load `work/os/<part>/<part>.elf` (Cortex-M, LE); the `fn_*`
   symbols come from `BL` targets only, so functions reached only through
   pointers (vtables, task entries) need the RTTI/vtable lists
   (`sys8fw.find_classes`) or Ghidra's own analysis.
3. ~~Prove where region 1 runs~~ — slaves only (8.13). Old idea: a region 1 hook
   that only writes a marker to its own memory, then read that address with
   the bulk dump from the master.
4. Dump the boot loader from QSPI flash (8.14).
5. Decode the record layout at `0x60290498` (name scrambling, `0x6D0`
   stride) and the master `.rodata` tables (`0x60094800–0x60128000`).

### 8.11 Kernel RAM of master core 0 (`dumps/kernel_01000000.bin`, 2026-10-14)

```bash
python3 examples/midi_send/bulk_loop.py --start 0x01000000 --end 0x01019A08 -o dumps/kernel_01000000.bin
python3 tools/os_analyze.py dumps/kernel_01000000.bin --base 0x01000000 -o work/os/kernel
```

2 installs (64 KiB + 38.5 KiB), 85 s, no retry, `BULK OK`. The range is the
scatter-load target of §6.2: vector table (`0x01000000`, `0x400`), code +
data (`0x01000400`, `0x13FAC`), `.bss` (`0x010143AC–0x01019A08`).

Verification: identical to every earlier slow read of this area —
`mem_0000.bin` (70 B at `0x01000000`), `dumps/vec.bin` (960 B at
`0x01000040`), `rom_58c0.bin`, `rom_59a0.bin`. Vector table: SP
`0x01050800`, reset `0x60000001`, faults → `0x010058C1…`, SVCall
`0x01005A1D`, PendSV `0x010058F1`, SysTick `0x01005AB5` (as in §6.2). Code
part 8 % zeros, `.bss` 82 % zeros.

Content (619 functions):

* **micro T-Kernel 2.00.01** (`micro T-Kernel Version 2.00.01` at
  `0x01005C24`, `!ERROR! init_kernel`, `!ERROR! start_system`,
  `<< SYSTEM SHUTDOWN >>`, `<< SYSTEM REBOOT >>`, `tk_ext_tsk/tk_exd_tsk was
  called in the task independent…` at `0x01003970`, object classes `tkdev
  task semaphore eventflag mailbox messagebuffer` at `0x01005E74`).
  Identified: **`0x01004381` = `tk_dly_tsk`** (returns `E_PAR` = -17 for a
  negative timeout, `E_CTX` = -25 when it may not wait; the loader's
  `0x600751F6(10)` in 8.3 is therefore a 10 ms delay).
* The **USB host driver** for the inter-BMC link (`BMCBulk%d Detached`,
  EHCI/OHCI attach/reset texts at `0x0100DE2E`) and an `Assigner` object.
* **Roland's C++ runtime** shared by the OS: MIDI message / Wrap classes of
  §7.7/§7.12 (`0x010075B7` wrap_packet, `0x01007781` Wrap dtor,
  `0x01007629` Msg ctor, `0x010076E5` set_sysex, `0x01007595` wrap_msg) and
  parameter/object helpers such as `0x01011A79`/`0x01011A87` (312 call
  sites each), `0x01011B29`/`0x01011B7F` (~215), `0x0100B39D` (213),
  `0x01008E91` (197). The slave send `0x010125C1` (8.3) is here too.

**Veneers** (`movw ip / movt ip / bx ip`, 527 in the image, table in
`work/os/kernel/veneers.txt`): they are far-call bridges between the fast
RAM and the SDRAM part of the *same* program, not calls into one shared
kernel:

| Veneers in | Count | Target |
|---|---|---|
| `m0_core0` (`0x600751BA–0x60075B56`) | 247 | `0x0100xxxx` / `0x0101xxxx`: this kernel + runtime |
| `m1_sdram` (`0x6017…`) | 114 | `0x0110xxxx`/`0x0111xxxx` = its own `m1_tcm` |
| `m1_tcm` (`0x6014…`) | 8 | `0x6100xxxx` = its own `m1_sdram` |
| `s_data` (`0x6021…`) | 121 | `0x0110xxxx`/`0x0111xxxx` = its own `s_boot` |
| `s_boot` (`0x6020…`) | 37 | `0x6000xxxx` = its own `s_data` |

So master core 1 and the slaves run their own thread system inside their
TCM images (`ThreadMain`, `CEZUtilThread`), not the master's μT-Kernel.

### 8.12 `.data`/`.bss` of master core 0 (`dumps/ram_20000000.bin`, 2026-10-14)

```bash
python3 examples/midi_send/bulk_loop.py --start 0x20000000 --end 0x20008A48 -o dumps/ram_20000000.bin
```

1 install (1107 messages), 42 s, `BULK OK`. Range = scatter-load targets of
§6.2: `.data` `0x20000000` (`0x308` bytes, initialised from the image) and
`.bss` `0x20000308–0x20008A48`. This is **live RAM**: values change between
reads.

Checkpoints with the slow channel (`mem_names/dump_loop.py`, 48 B each):
`0x20000000` identical; `0x200002F0` identical except the word at
`0x200002F0` (`0x20002EE3` slow vs `0x20002F4C` bulk), a buffer pointer that
moves — expected for live data.

What is in it (first look):

| Address | Content |
|---|---|
| `0x2000002C`, `0x20000030` | pointers into the OS (`0x60008473`, `0x6000847D`): callbacks |
| `0x20000210–0x2000025C` | small tables (`01 02 03 04 05 06 07`, flags), kernel addresses |
| `0x20000270`, `0x20000278`, `0x200002E0/E4` | kernel/runtime function pointers (`0x0100C4F7`, `0x0100CAB1`, `0x01011DDB`, `0x01011DE1`) |
| `0x2000027C–0x20000288` | pointers to `0x61E02524…0x61E02E0C`: SDRAM at `0x61E00000`, after the four model windows (`0x61600000…0x61C00000`); not a slot base — possibly install staging (unverified) |
| `0x200002B0/B4` | peripheral instances `0x40068080`, `0x4006A080` |
| `0x200002F0`, `0x20000300` | buffer pointers (`0x20002F4C`, `0x20004110`) |
| `0x20000308…` | MIDI receive buffer: a DT1 being received (`F0 41 10 00 00 7E 12 71 00 01 …`) — the install itself |
| `0x20001A72–0x200029EE` | **value-label lists of the installed PLUG-OUT** (slot 3, JX-3P model: `UNISON`, `METAL`, `OVERDRIVE`, `JX-3P CH`, `CRUSHER`, `PHASER`, `64FEET`…`2FEET`, `LFO+ENV`, `CIRCUIT MOD`, `Key Assign`, `Porta Type`, `BEND SENS`…) — the OS keeps the labels of the current model in RAM |
| `0x20002C4A–0x200033D1` | **LZSS stream of the PLUG-OUT being installed** (text broken by flag bytes: `p11CAss·ignJx3p`, `14<tPlugo/ut`, `DREOVERB`, `DCO MODU_LATOR`): installs are decompressed through this RAM buffer |
| `0x20007090`, `0x200088F8` | the same 128-byte table twice (ramp `0x20…0x3F…0x20`, then pair-swapped) — lookup tables, role unknown |

### 8.13 Where region 1 (the engine) runs: on the slave BMCs only (2026-10-14)

Static proof, `LoadPlugout` in master core 0 (`0x60051BC6` for flash slots,
`0x60051D32` for a USB install):

1. Per slot, a table at `0x602B9140` (RAM, `0x602B90CC + 0x74 + 16·slot`)
   holds `{model window, engine window, 0x200000, 0x200000}`:
   `{0x61600000, 0x60800000}`, `{0x61800000, 0x60A00000}`,
   `{0x61A00000, 0x60C00000}`, `{0x61C00000, 0x60E00000}`.
2. The **engine** image is read from QSPI flash (or from the install buffer)
   and LZSS-decoded (`0x600759A8` → kernel `0x0101294D`) into the **model**
   window of the master, checked for `SYSTEM8_SYNINFO` (`0x6005234E`), then
   sent with `0x60051B54(load = engine window, src = model window, len)`:
   16-byte header `{'BUDB', len, load, 'BOOT'}` + data through kernel
   `0x010125C1` → `0x0100D548` (USB host bulk to the `BMCBulk` slaves),
   the same path as the `s_boot`/`s_data` images (8.3).
3. Only then the **model** image is decoded into the same model window
   (overwriting the engine copy), checked, and its SYNINFO `+0x24` (init)
   is called on the master.
4. An empty slot gets a 0x200-byte image of `0x55` sent to its engine
   window instead.
5. `s_boot` (`0x01102562`) runs a RAM test over `0x60800000–0x61000000`
   (write address, read back) at slave start-up: the slaves have RAM at
   exactly the four engine windows (their `s_data` occupies
   `0x60000000–0x60800000`). No code in `m0_core0`, `m1_tcm` or `m1_sdram`
   uses an engine window address except the table above.

Hardware check (bulk dump from region 0, 2 KiB at the start of each engine
window of the **master** SDRAM, shared by core 0 and core 1):

| Master address | Content |
|---|---|
| `0x60800000` | OS data: a repeating table of u32 `0x31`/`0x24` (period 0xD8), no code, no `SYSTEM8_SYNINFO` |
| `0x60A00000` | same table, other phase |
| `0x60C00000`, `0x60E00000` | all zeros (slot 3 had just received the dump PLUG-OUT, whose engine went to the slaves) |

So the engine code is not present in the master's address space; region 1
runs on the slave BMCs. This is why any call from an engine hook into a
master OS address hangs (§7.10): on a slave `0x60000000` is `s_data`.

### 8.14 Boot loader: not in the dumps; it is in QSPI flash at offset 0

None of the dumps (`os_full.bin`, `kernel_01000000.bin`,
`ram_20000000.bin`) contains the loader: no `Roland P1Y01` image trailer,
no cipher code (§6.4). The OS knows where it is. Flash partition table
(names at `0x6009B750`, offsets from `0x60078AB8(idx)`):

| # | Name | QSPI offset |
|---|---|---|
| 0 | `loader` (Ld0) | `0x000000` |
| 1 | `updater` (Up0) | `0x010000` |
| 2 | `app` (Ap0, `App1_Main`) | `0x060000` |
| 3–6 | `plugout 0…3` (engine at +0, model at +0x100000) | `0x800000`, `0xA00000`, `0xC00000`, `0xE00000` |
| 7–8 | `plugout p1/p2` (preload images, 0x40-byte `PRELOAD_PLUGOUT` header) | `0x1000000`, `0x1200000` |
| 9–16 | sequencer / perform / patch / patch2 / manual patch / system / test mode / reserved parameters | `0x1400000`, `0x1780000`, `0x17A0000`, `0x17D0000`, `0x1860000`, `0x18B0000`, `0x18F0000`, `0x1910000` |

The version screen reads each image's trailer from flash: `app` = size at
flash `0x6004C` (fallback `0x60044`, i.e. `App1_Main` header `+0x44`),
trailer at `0x60000 + ((size + 0x6F) & ~0xF)`, version `+0x24`, build
`+0x2C` — so the app is stored in flash in the updater file format
(header + encrypted payload + trailer). `updater`: same, base `0x10000`.
`loader` (`0x60000EBA`): size = u32 at flash `0x0C`, trailer at
`(size + 0x2F) & ~0xF` — a different, 0x20-byte header.

Flash read function: **`0x6004306A(0, 0, &{offset, length, dst})`**
(QSPI controller `0x40010000`, guarded by a semaphore; 29 callers, used by
`LoadPlugout` and the version screen). A region 0 hook can therefore read
flash `0x0–0x60000` (loader + updater) into a buffer and send it with the
bulk SysEx path (§7.13). The loader is what decrypts `App1_Main`; if it is
stored in clear, it holds the cipher and the key (§2, §5).

**Dumped 2026-10-14** (`dumps/flash_loader.bin`, flash `0x0–0x10000`, one
install, 43 s, `FLASH OK`):

```bash
python3 examples/midi_send/bulk_loop.py --flash --start 0 --end 0x10000 -o dumps/flash_loader.bin
```

PLUG-OUT `plugins/JX-3P-INIT-FLASH` (`init_flash.s`, `make_init_flash.sh`;
`bulk_dump.py`/`bulk_loop.py --flash`): per message the region 0 init hook
reads 32 B with `0x6004306A` into the stack and sends them in the §7.13 format
(address field = flash offset). Result:

* `0x00`: `"BMC 0.10"`, `0x0C` u32 size `0x3D30`, `0x10` `0x21E`,
  `0x14` `0x00B40201`, `0x18` `FF`×8; payload `0x20–0x3D50`; trailer at
  `0x3D50`: 12 × 00, `CB F6 F5 C9`, `"Roland P1Y01_C0L"`, `"VER.1.01"`,
  `"BLD.11E7"`, `"18 0730 10 18 47"`. Rest of the 64 KiB is `FF`.
* **The loader is encrypted too**: entropy 7.3–7.96, the same 64-bit ECB
  signature as `App1_Main` (block `7999E9A16AA6D322` 120×, mostly in 16-byte
  pairs). Its dominant block differs from the app's (`6B4C9A852C732831`).
  So the key is not in the loader: the BMC decrypts the loader itself (boot
  ROM / on-chip key).

**Updater dumped 2026-10-14** (`dumps/flash_updater.bin`, flash
`0x10000–0x60000`, 5 installs):

```bash
python3 examples/midi_send/bulk_loop.py --flash --start 0x10000 --end 0x60000 -o dumps/flash_updater.bin
```

(Never run two `bulk_loop.py` at once: a second one sharing the MIDI port made
the device look hung — all-zero capture, `NO RESPONSE`.)

* Header `0x10000–0x10060` (u32 LE): `"App0_UPD"`, UTF-16 `"ern"`,
  `"2018/07/30 10:19"`, `"0.010001"`; `+0x28` `0x60000000` (load address);
  `+0x30` `0x10040`; `+0x40` `0x10060` (payload flash offset), `+0x44`
  `0x40080` (payload size), `+0x48` `0x60000000`, `+0x50` `0x49A54D00`,
  `+0x54` `0xB5020001`.
* Payload `0x10060–0x500E0` (exactly `0x40080` B), trailer `0x500E0–0x50120`:
  `00`×12, `6A E1 9F 04`, `"Roland P1Y01_C0U"`, `"VER.1.02"`, `"BLD.11E7"`,
  `"18 0730 10 19 32"`. Rest is `FF`.
* **Encrypted too**: entropy 7.96–7.99, 64-bit ECB signature again; dominant
  block `9E57B0A3DCD3B80B` (302×), second `7D7F03CBC8FF560D` (116×). It differs
  from both the loader (`7999E9A16AA6D322`) and `App1_Main`
  (`6B4C9A852C732831`). So all three flash images are encrypted, and the key
  is held by the BMC, not by any of them.
