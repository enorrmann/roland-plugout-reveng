# data-formats.md — container, flash image and regions

Everything marked **[H]** is reproduced by `test-vectors/check_vectors.py`
(written only from this document and `protocol-spec.md`).
Little-endian integers unless stated. Offsets in hexadecimal.

## 1. `Code*.Dat` container

### REQ-DATA-001 — Cipher **[H]** (EV-01, EV-02)
The file is read as little-endian u32 words `c[0..n-1]`. Plaintext:
`p[i] = c[i] XOR c[i-1]`, with `c[-1] = 0xBACADABE`. Trailing bytes that do not
form a full word do not exist in the observed files (all sizes are a
multiple of 4). To **encrypt**, use the inverse: `c[i] = p[i] XOR c[i-1]`.

### REQ-DATA-002 — Header (plaintext) **[H]**

| Offset | Size | Field | Observed value |
|---|---|---|---|
| 0x00 | 4 | constant | `0x00000080` |
| 0x04 | 16 | signature | `"PLUGOUT_SYS_INF\0"` |
| 0x14 | 16 | model name (ASCII, `\0` padded) | `JX-3P`, `JUNO-60`, `SH-101`, `SYSTEM-8` |
| 0x24 | 2 | model id | JUNO-60 `0x000A`, JX-3P `0x0008`, SH-101 `0x0001` |
| 0x26 | 2 | version (shown as `%X.%02X`) | `0x0101` → 1.01 (JX-3P/JUNO-60); `0x0108` → 1.08 (SH-101 Code8) |
| 0x30 | 4+4 | offset, size of the START record | `0x80`, `0x20` |
| 0x40 | 4+4 | offset, size of the END record | `0xA0`, `0x20` |
| 0x50 | 4+4+4 | offset, stride, number of DATA records | `0xC0`, `0x260`, 4096 (Code8) / 2048 (Code1) |

File size = `0xC0 + count × 0x260` **[H]** (2 490 560 / 1 245 376 B).
Header bytes not listed are zero **[H]** (observed).

### REQ-DATA-003 — Records **[H]**
Each record (START, END, DATA[i]) at `offset (+ i × stride)` is:
`u32 length` + complete SysEx message (`F0 … F7`) + `0x00` padding up to the
slot size. Messages (format in `protocol-spec.md` REQ-PROTO-002):

| Record | SysEx address | Message length |
|---|---|---|
| START | `70 10 00 00` | 14 B (1 data byte) |
| END | `70 10 00 01` | 17 B (4 data bytes) |
| DATA block | `70 30 00 09` | 599 B (586 data) |
| DATA filler | `70 30 01 09` | 14 B (data `01`) |

### REQ-DATA-004 — 7-in-8 block packing **[H]**
The 586-byte data field is split into groups of up to 8: one MSB byte
followed by up to 7 bytes of 7 bits. Bit *j* of the MSB byte is bit 7 of
byte *j* of the group. 73 full groups + 1 one-byte group (586 = 73·8 + 2) →
**512 bytes** of flash. Every SysEx data byte has bit 7 clear.

## 2. Flash image

### REQ-DATA-005 — Layout **[H]**
The DATA records, in order, form a sequence of 512-byte blocks (a filler
record = an erased block, 512 × `0xFF`). There is no block number:
**order is the address**. The sequence is split into two equal halves
(`count/2` blocks): half 0 = region 0 ("model"), half 1 = region 1
("engine"). Each region starts at the first block of its half and occupies
consecutive data blocks; the rest of the half is filler records. A region
fits if it takes ≤ `count/2` blocks (1 MiB in Code8, 512 KiB in Code1).
Observed occupancy: 282–365 blocks (Code8), 252–498 (Code1).

### REQ-DATA-006 — Region header (0x40 B) **[H]**

| Offset | Size | Field |
|---|---|---|
| 0x00 | 16 | signature `"SYSTEM-8:Plugout"` (Code8) or `"SYSTEM-1:Plugout"` (Code1) |
| 0x10 | 16 | build stamp `YY-MMDD_HH:MM:SS` (e.g. `25-0509_11:30:38`) |
| 0x20 | 8 | toolchain tag (`0.0114FA`; `0.015370`/`0.013181`/`0.012265` in Code1/Code.Dat) |
| 0x28 | 4 | load address: `0x60800000` region 0, `0x60200000` region 1 (same in all slots) |
| 0x2C | 4 | header size `0x40` |
| 0x30 | 4 | compressed size `C` |
| 0x34 | 4 | u32 sum (mod 2³²) of the `C` compressed-stream bytes |
| 0x38 | 4 | entry (= load address) |
| 0x3C | 4 | decompressed size `D` |

After the header: an LZSS stream of `C` bytes and `0xFF` padding up to a
whole number of 512-byte blocks. Whether the OS verifies the sum in field
0x34: **[?]**.

### REQ-DATA-007 — LZSS compression **[H]** (decoder)
* 4096-byte ring initialised to `0x00`; initial write pointer 4078.
* A flag byte is read; 8 items are processed, bit 0 first.
  Bit = 1: literal (1 byte, copied to the output and the ring).
  Bit = 0: 2-byte reference `b0,b1`: `pos = b0 | ((b1 & 0xF0) << 4)`,
  `len = (b1 & 0x0F) + 3`; `len` bytes are copied from the ring starting at
  `pos` (modulo 4096), also written back into the ring.
* Decoding continues until `D` bytes are produced.
* The **encoder** is not normative: any stream that the decoder above
  expands to the image is acceptable for a tool; whether the *device*
  accepts streams from another encoder is **[?]** (hardware tests used the
  classic Okumura encoder, TEST-012).

### REQ-DATA-008 — END message sum **[H]**
Let `S` = sum of all bytes of the `count` blocks (a filler counts as
512 × `0xFF`) modulo 65 536. The 4 data bytes of END are `S` with one nibble
per byte, MSB first (e.g. `S = 0x6236` → `06 02 03 06`).
Vectors: `container-vectors.json` fields `end_sum_*`; examples in
`protocol-vectors.json`.

## 3. Region contents

### REQ-DATA-009 — SYNINFO (start of every image, 0x30 B) **[H]**

| Offset | Field |
|---|---|
| 0x00 | `"SYSTEM8_SYNINFO\0"` (Code8) / `"SYSTEM1_SYNINFO\0"` (Code1) |
| 0x10 | name (16 B) |
| 0x20 | u16 model id, u16 version |
| 0x24 | u32 address of the init function (bit 0 = 1: Thumb) |
| 0x28 | u32 pointer inside the image |
| 0x2C | Code8: the signature again (16 B); Code1: 2 pointers, `0x000100C0`, `0x00000005` |

* Region 0: name/id/version **equal** to the container's **[H]** (all 12
  files). It is what the device reports for the slot query **[H]** (EV-04).
* Region 1 (Code8): name `SYSTEM-8`, id `0x000A`, version `0x0001` in all
  three plugins **[H]**; meaning **[?]** (hyp.: engine interface version).
* Code1 SH-101: region 1 id `0x0001`, version `0x0000`.

### REQ-DATA-010 — Per-slot linking and relocation **[H]** (EV-05)
Code8 images are linked to fixed per-slot addresses (2 MiB window):

| | slot 0 (synth) | slot 1 | slot 2 | slot 3 |
|---|---|---|---|---|
| region 0 | `0x61600000` | `0x61800000` | `0x61A00000` | `0x61C00000` |
| region 1 | `0x60800000` | `0x60A00000` | `0x60C00000` | `0x60E00000` |

`Code8_1/2/3` differ **only** in aligned u32 words that add up
`0x200000 × (dest_slot − src_slot)` (no other differences):

| Plugin | region 0 | region 1 |
|---|---|---|
| JX-3P | 52 words | 1473 |
| JUNO-60 | 52 | 1474 |
| SH-101 | 53 | 605 |

Exact data: `slot-and-preset-vectors.json` (`relocation`). The list of
positions is not a container table: it is obtained by comparing builds
**[H]**. How the OS decides the physical placement **[?]**.

### REQ-DATA-011 — Factory preset table, region 0 **[H]** (EV-06)
* At `0x8000`, records of `0x200` B. 64 slots in JX-3P and JUNO-60 (all 64
  named); SH-101: 48 named + 16 slots named `----------------`.
* Record = 128 32-bit words. Words 64–67 (bytes `0x100..0x10F`): a
  16-character ASCII name; **each 4-byte group is stored in reverse order**
  (the word is big-endian with respect to the text). Space padded.
* Remaining words: parameter values; in JUNO-60 all 0–255; in JX-3P and
  SH-101 negative signed values also appear (e.g. `0xFFFFFFF6`).
  Word→parameter mapping **[?]** (hyp.: matches the `patch` leaves of
  `Script.xml`, EV-10; not verified).
* Vector: `slot-and-preset-vectors.json` (`presets`, full names).

### REQ-DATA-012 — Label lists, region 0 **[H]/[I]**
Near the end of region 0 (JX-3P, ~`0x11AB84`) there are structures
`<u32 2><u32 0x14><u32 list id><u32 count>…` followed by entries
`<u32 3><u32 0x1C><char[20]>` (e.g. `OFF`, `ON`). They are value texts shown
on the panel **[I]**. Changing a text here changed the label visible on
hardware (EV-04, test 4) **[H]**. Full structure **[?]**.

### REQ-DATA-013 — Region 1 (engine) **[I]** (EV-07, EV-08)
ARM Thumb-2 code with VFP; it contains an initialisation function
(SYNINFO+0x24) that fills in and returns an exports descriptor; it reaches
the OS through a table of function pointers at `0x0113FC00`
(`cleanroom_architecture.md` REQ-ARCH-004). Past the image there is a `.bss` not
present in the file (internal pointers reach ~60 KiB past the end of the
image in JUNO-60) **[I]**.

## 4. Associated formats (outside `Code*.Dat`)

### REQ-DATA-014 — Patch bank `Patch/*.bin` **[H]** (EV-10)
Plugin file (not firmware), used in the patch phase (REQ-PROTO-010):
* 16 B `KoaBankFile0000N` (N = format version: 2 or 3) + tag
  (`PG-JU60`, `PG-JX3P`, `PG-SH101`) + 64 records.
* Record: name (16 B) + header (12 B in v3, 27 B in v2) + leaf memory
  concatenated in `Script.xml` order.
* v3 (JUNO-60, JX-3P): `patch`+`patch2`+`patch3` = 20 223 B per record;
  v2 (SH-101): only `patch` = 683 B (`patch2/3` take the `Script.xml`
  defaults). All bytes are 7-bit.
* Memory structures: `patch` 640 B (14 leaves), `patch2` 3975 B (17),
  `patch3` 15 580 B (66, sequencer).
