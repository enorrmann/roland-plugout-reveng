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
| Recreate the OS from source | still blocked on re-encryption (cipher key), but the plaintext is now obtainable from hardware; a full read-back path is the remaining step |

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
2. **Bootloader key**: the same dump approach applied to the boot ROM/flash
   region (address unknown; found by probing) would reveal the algorithm
   and key, enabling re-encryption of a rebuilt `App1_Main`.
3. Name methods properly: diff the base engine against the factory engines
   (`CDSPSystem8Vcf` vs `CPrmDSPJp8` …) to label shared code, then export a
   symbol file per image.
4. Check `App1_Main` header fields `0x30/0x40/0x50/0x54` once plaintext is
   available (checksum, flash offset `0x60060`).
