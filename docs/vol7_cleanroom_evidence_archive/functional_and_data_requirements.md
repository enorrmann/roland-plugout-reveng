# Volume VII — Chapter 2: Clean-Room Specification — Overview, Logical Architecture, Functional Requirements, Data Formats

> The protocol and the state machines are [ch. 5](cleanroom_protocol_spec.md); the test plan and the evidence
> register, [ch. 3](test_vectors_verification_plan.md).

---

## Overview — PLUG-OUT firmware (`Code1.Dat`, `Code8_1/2/3.Dat`)

Clean-Room transfer document (Team A → Team B).
Certainty marks used in every document:

| Mark | Meaning |
|---|---|
| **[H]** Fact | backed by reproducible evidence (see [Vol. VII ch. 3](test_vectors_verification_plan.md), vectors in `test-vectors/`) |
| **[I]** Inference | reasonable deduction from evidence, no direct proof |
| **[Hyp]** Hypothesis | pending validation; do not use as a firm requirement |
| **[?]** Unknown | cannot be determined with current evidence |

### 1. Scope

The Roland Cloud plugins *JUNO-60*, *JX-3P* and *SH-101* install a
**PLUG-OUT** into a SYSTEM-8 (or SYSTEM-1): a firmware package contained in a
`Code*.Dat` file that the plugin transfers over USB-MIDI (SysEx) into the
synthesizer's flash memory, where the device OS runs it.

| File | Target | Present in |
|---|---|---|
| `Code8_1.Dat`, `Code8_2.Dat`, `Code8_3.Dat` | SYSTEM-8, PLUG-OUT slot 1, 2, 3 | JUNO-60, JX-3P, SH-101 |
| `Code1.Dat` | SYSTEM-1 / SYSTEM-1m | JX-3P (see anomaly §4), SH-101 |
| `Code.Dat` | not used by the plugin | SH-101 |

**In scope:** (a) how the firmware is delivered, validated and stored
(container, protocol, flash image); (b) what the firmware declares and
exposes (identity, data tables, OS interface); (c) what a third party needs
to generate containers that the device accepts.

**Out of scope / not determinable today:** the synthesis algorithm. The
PLUG-OUT's ARM code **does not render audio**: it configures a DSP whose
program is not in these files **[I]** (EV-07, EV-08). This specification
therefore **does not allow** reproducing the instruments' sound; see §5.

### 2. System summary

```
 Host (plugin / tool)                        SYSTEM-8 / SYSTEM-1
 ┌───────────────────────────┐   SysEx    ┌──────────────────────────────┐
 │ Code8_<slot>.Dat (cipher) │ ─────────▶ │ Device OS (App1_Main)        │
 │  START, N×DATA, END       │ ◀───────── │  writes image into flash     │
 └───────────────────────────┘   replies  │  slot k: region 0 + region 1 │
                                          │  region 0 "model" (tables)   │
                                          │  region 1 "engine" (code)    │
                                          └──────────────────────────────┘
```

* The container is an ordered dump of SysEx messages encrypted with chained
  XOR **[H]**. Each data message carries 512 bytes of flash.
* The flash image holds **two regions** compressed with LZSS, each linked to
  a fixed address **per slot** **[H]**.
* Identity (name, model id, version) is in the container and in the SYNINFO
  block of region 0; the device reports it via a SysEx query **[H]**.

### 3. Reference figures (all **[H]**, `test-vectors/container-vectors.json`)

| Plugin | Container | Records | Data blocks | Region 0 plain | Region 1 plain |
|---|---|---|---|---|---|
| JX-3P | Code8_1..3 | 4096 | 651 | 1 159 752 B | 282 172 B |
| JUNO-60 | Code8_1..3 | 4096 | 640 | 1 158 192 B | 265 024 B |
| SH-101 | Code8_1..3 | 4096 | 588 | 1 158 616 B | 211 212 B |
| SH-101 | Code1 | 2048 | 560 | 215 012 B | 148 132 B |
| JX-3P | Code1 | 2048 | 934 | 370 308 B | 448 832 B |

### 4. Observed anomalies

* `JX-3P/Script/Code1.Dat` **[H]**: the container header says `SYSTEM-8`,
  id `0x0005`, version `0x0C6D`; its region signature is `SYSTEM-1:Plugout`,
  stamped `16-0310_12:05:02` (2016), with `CDSPSystem8*` classes. It is not a
  JX-3P. Function **[?]**; hypothesis: build of the synthesizer's own base
  engine for SYSTEM-1 **[Hyp]**. Not usable as a JX-3P PLUG-OUT
  ([Vol. IV ch. 4](../vol4_firmware_container_protocol/usb_sysex_protocol_spec.md) §3).
* `SH-101/Script/Code.Dat` **[H]**: 2015 build (`15-0924_14:45:20`), unused.

### 5. Analysis limitations

1. Hardware verification exists only on one SYSTEM-8 (OS 1.32), and only with
   JX-3P variants in slot 3 (EV-04). JUNO-60, SH-101 and SYSTEM-1 have not
   been tested on a real device.
2. The OS (`App1_Main`, entropy ≈ 8.0) has not been decoded: what the device
   actually validates (only the sums? a signature?) is unknown.
3. The DSP program and the complete *parameter → DSP id* mapping are unknown;
   without them there is no audio specification.
4. The OS service table (`0x0113FC00`) is only partially mapped
   (REQ-ARCH-004).

### 6. Document map

| Document | Content |
|---|---|
| [Vol. VII ch. 2](functional_and_data_requirements.md) | components, execution model, OS interface |
| [Vol. VII ch. 2](functional_and_data_requirements.md) | `REQ-FUNC-*` requirements and acceptance criteria |
| [Vol. VII ch. 5](cleanroom_protocol_spec.md) | `REQ-PROTO-*`: SysEx, install sequence |
| [Vol. VII ch. 2](functional_and_data_requirements.md) | `REQ-DATA-*`: container, image, regions, LZSS, SYNINFO |
| [Vol. VII ch. 5](cleanroom_protocol_spec.md) | host, transfer, slot |
| [Vol. VII ch. 3](test_vectors_verification_plan.md) | `TEST-*` and conformance matrix |
| `test-vectors/` | vectors and reproducible verifier |
| [Vol. VII ch. 3](test_vectors_verification_plan.md) | `EV-*` register, confidence, open items |
| [Vol. VII ch. 1](cleanroom_boundary_contract.md) | team separation rules |

---

## [Vol. VII ch. 2](functional_and_data_requirements.md) — logical architecture

Separates what is needed for compatibility (the **contract**) from accidents
of the original implementation (**non-normative**). Marks: see [Vol. VII ch. 2](functional_and_data_requirements.md).

### 1. Components

| ID | Component | Responsibility | Certainty |
|---|---|---|---|
| C1 | Host installer | queries, checks rules, transfers the package | [H] |
| C2 | `Code*.Dat` package | dump of encrypted SysEx messages (START/DATA/END) | [H] |
| C3 | Device-side receiver | writes blocks to flash in order, validates | [I] (OS encrypted; EV-09) |
| C4 | Per-slot flash image | region 0 + region 1, linked to the slot window | [H] |
| C5 | Region 0 "model" | SYNINFO (identity), data tables (presets, parameter names, labels), little code | [H]/[I] |
| C6 | Region 1 "engine" | ARM code that translates parameters into DSP commands; exports a descriptor | [I] |
| C7 | Device OS (`App1_Main`) | loads/runs regions, exposes services in a fixed table | [I] |
| C8 | DSP | generates audio; its program is not in the package | [I] |

#### REQ-ARCH-001 — Packaging layers **[H]**
`Code*.Dat` ⊃ SysEx records ⊃ 512-byte blocks ⊃ regions (0x40 header +
LZSS) ⊃ image (SYNINFO + data + code). Detail in [Vol. VII ch. 2](functional_and_data_requirements.md).

#### REQ-ARCH-002 — One image per slot **[H]** (EV-05)
Each PLUG-OUT slot has its own **2 MiB window** per region
(REQ-DATA-010). The package for slot *k* contains an image linked to that
slot's addresses. A package for one slot is not valid in another without
relocating the absolute pointers listed in REQ-DATA-010. Relocation is
additive and uniform (adds `0x200000 × Δslot`); no movw/movt or other forms
are observed **[H]** (scope: the 3 plugins).

#### REQ-ARCH-003 — Duplicated identity **[H]**
Name, model id and version appear in the container header and in the SYNINFO
of region 0; the host uses the container, the device publishes what is in
the slot. Whether the device derives identity from one or the other: **[?]**
(in tests 2–3 of EV-04 both were changed together).

#### REQ-ARCH-004 — OS interface: service table **[I]** (EV-07)
* The engine (region 1) invokes services through pointers read from a table
  at the fixed address **`0x0113FC00`**, identical in all slots and in the
  synthesizer's own engine (`p0s`).
* Region 0 does not reference the table (0 occurrences) **[H]**.
* Entries referenced by the three Code8 engines:

| Offset | JX-3P | JUNO-60 | SH-101 | Inferred use [I] |
|---|---|---|---|---|
| `+0x00` | yes | yes | – | read at init next to constants |
| `+0x28` | – | – | yes | read at init |
| `+0xA0` | yes | yes | yes | function `(obj, …)` |
| `+0xA4` | yes | yes | yes | function `(obj, …)` |
| `+0xB0` | yes | yes | yes | **send command/parameter to the DSP**: `(…, id, float value)` |
| `+0xBC` | – | yes | yes | function taking a float |
| `+0xC0` | – | yes | yes | function taking an object |
| `+0xF0` | yes | yes | yes | start-up function; result compared with 0 |

  Common to all three: `+0xA0, +0xA4, +0xB0, +0xF0` **[H]**. Exact
  signatures and semantics: **[?]** (requires decoding the OS or on-device
  experiments, TEST-020).
* A PLUG-OUT from scratch additionally needs: the layout of the descriptor
  returned by `init` and the minimum content required in region 0 **[?]**.

#### REQ-ARCH-005 — Export (`init`) **[I]**
SYNINFO+0x24 points to a Thumb function that initialises a descriptor in
`.bss` with function pointers and returns its address; that descriptor is
the interface the OS calls (interface class `CPlugoutInterface` in the type
strings, **no normative value**).

#### REQ-ARCH-006 — Execution model **[Hyp]/[?]**
Whether the OS calls the engine through tasks, interrupts or callbacks, and
its deadlines, has not been determined. The only observable: the device
panel (parameter values 0–255) causes engine calls towards the DSP (EV-04,
test 5: a code patch that inverts the sign of the value sent changes the
sound of the TONE control in OD/DS/FZ).

### 2. Engine internals (non-normative) [I]

For orientation only; derived from type names (EV-08), **do not use as a
requirement**:

* One parameter object per model (336 parameter methods in JX-3P). Each
  method converts a 0–255 value into a float via a curve table (256 floats
  per curve) and sends it to the DSP with an id.
* Per-voice groups (oscillator, filter, envelope, LFO, amp) and effect
  groups (JX-3P: Ch, Cr, Ds, Fz, Od, Ph; JUNO-60: Ce, Ch, Ds, Fz, Mt, Od,
  Ph) plus "noise", "CV out" and mix.
* SH-101 (Code8): a much smaller engine — the type strings only show
  `CPrmDSPSh101`, `CPrmDSPSynCtrl`, `CPlugoutInterface` **[H]**.

### 3. SYSTEM-1 variant (Code1)

* Signature `SYSTEM1_SYNINFO`; regions with load `0x60800000`/`0x60200000`;
  classes `CPrmDSPPlugoutMaster` (region 0) and `CPrmDSPPlugoutSlave`
  (region 1) in SH-101 **[H]** → two processors/contexts, master and slave
  **[Hyp]**.
* No evidence of multiple slots: a single install.

### 4. Hardware dependencies / required abstractions

* ARM Thumb-2 + VFP family (exact core **[?]**).
* Memory map: flash (windows `0x6xxxxxxx`) and service table at `0x0113FC00`.
* An alternative host-side implementation needs only USB-MIDI + SysEx (see
  [Vol. VII ch. 5](cleanroom_protocol_spec.md)); nothing from the CPU is required.

---

## [Vol. VII ch. 2](functional_and_data_requirements.md) — functional requirements and acceptance criteria

Each requirement: *Description — Certainty — Evidence — Acceptance criterion
— Tests*. It describes **what** the firmware/package does observably, not how
it is written. Tests are in [Vol. VII ch. 3](test_vectors_verification_plan.md).

### A. Package and delivery

#### REQ-FUNC-001 — Self-contained package per slot
A `Code8_<k>.Dat` contains everything needed to leave slot *k* of the
SYSTEM-8 with a working PLUG-OUT, with no other files. **[H]** EV-01, EV-04.
*Acceptance:* a package generated independently, installed in slot *k*, is
listed by the device with the expected identity. **TEST-001, 008**.

#### REQ-FUNC-002 — Distinct packages per slot
Content differs between slots only in absolute pointers
(`+0x200000` per slot) (REQ-DATA-010). **[H]** EV-05.
*Acceptance:* for each plugin, `Code8_2` and `Code8_3` are obtained from
`Code8_1` by adding the delta at the listed positions and recompressing.
**TEST-005**.

#### REQ-FUNC-003 — Selection by device type
SYSTEM-8 uses `Code8_<slot>`; SYSTEM-1/1m uses `Code1`. **[H]** (host).
*Acceptance:* TEST-009.

#### REQ-FUNC-004 — Integrity consistency
A package is internally consistent if: SysEx checksums are valid; the
compressed-stream sum equals the one in the region header; the END sum
equals the sum of the blocks. **[H]** (12/12 official packages comply).
*Acceptance:* TEST-002, 003, 004.

### B. Identity and lifecycle

#### REQ-FUNC-010 — Published identity
After a complete, error-free install, the slot query returns the package's
name (≤16 chars), model id (u16) and version (u16). **[H]** EV-04 (name
`JX-3P CUSTOM`, version 1.02 verified).
*Acceptance:* TEST-010, 011.

#### REQ-FUNC-011 — Reinstall over an occupied slot
Installing over an already occupied slot replaces it **[H]** (successive
EV-04 tests in slot 3). The host must ask for confirmation
(REQ-PROTO-004). *Acceptance:* TEST-010.

#### REQ-FUNC-012 — Model uniqueness
The same model id must not be installed in two slots (host rule). Whether
the **device** also prevents it: **[?]**. Whether a free id not used by any
official PLUG-OUT is accepted: **[?]** → TEST-018 (pending).

#### REQ-FUNC-013 — Persistence
The installed PLUG-OUT remains after the USB session ends **[H]** (later
queries) and after power cycling **[Hyp]** (not recorded). TEST-014 pending.

#### REQ-FUNC-014 — Recovery
A faulty PLUG-OUT can be recovered by reinstalling a valid one or with the
official updater `SYSTEM8_UP.BIN` (rewrites slot 0 and the factory
PLUG-OUTs). **[I]** EV-09. *Acceptance:* cannot be safely verified; see
[Vol. VII ch. 1](cleanroom_boundary_contract.md) §6.

### C. Model content (region 0)

#### REQ-FUNC-020 — Factory presets
Region 0 contains the per-model preset table (REQ-DATA-011): JX-3P 64,
JUNO-60 64, SH-101 48 (+16 empty). **[H]** EV-06.
*Acceptance:* TEST-006 (names). That the device **displays** those presets:
[?] not observed in a controlled way (the plugin sends its own bank in the
patch phase).

#### REQ-FUNC-021 — Value labels
The panel's value texts (OFF/ON, waveform names, effect types…) come from
lists in region 0. **[H]** for the existence and change of one label (EV-04
test 4); the full catalogue **[?]**. *Acceptance:* TEST-016.

### D. Engine

#### REQ-FUNC-030 — Parameter → DSP translation
The engine converts each parameter value (0–255 observed) through a curve
into a float sent to the DSP with a parameter id. **[I]** EV-04 test 5,
EV-08. Complete parameter→id→curve mapping: **[?]**.
*Acceptance:* TEST-017 (pending) measures via MIDI/audio the effect of
extreme values.

#### REQ-FUNC-031 — OD/DS/FZ effect TONE curve (JX-3P)
The TONE value is mapped to [−1, +1] and sent; inverting the sign inverts
the audible direction of the control. **[H]** (hardware EV-04 test 5). The
remaining curves are unstudied: [?].

#### REQ-FUNC-032 — Waveform
In JX-3P both DCOs send `value − 3` as the waveform selection; the
PULSE/NOISE distinction for positions 3–5 is decided by the DSP. **[I]**
(EV-08). NOISE could not be enabled natively (EV-04 note).

#### REQ-FUNC-033 — Audio
Audio generation (oscillators, filters, effects): **not specifiable** with
current evidence **[?]**. A Clean-Room implementation of the *sound*
requires measuring the real device (controlled input/output recordings);
this has not been done.

### E. Errors

#### REQ-FUNC-040 — Rejections
The device may reject START (`71 10 00 00` ≠ 0), a DATA record
(`71 30 00 00`) or END (`71 10 00 01` ≠ 0). **[H]** (the paths exist in the
host). Concrete causes observed on a device: none recorded **[?]**.
*Acceptance:* TEST-013 (pending).

#### REQ-FUNC-041 — Transport fault tolerance
The host retries each message that expects a reply up to 3 times (5 s /
20 s). **[H]**.

### Traceability

| Requirement | Evidence | Tests |
|---|---|---|
| FUNC-001 | EV-01, EV-04 | TEST-001, 008 |
| FUNC-002 | EV-05 | TEST-005 |
| FUNC-004 | EV-01, EV-02 | TEST-002–004 |
| FUNC-010 | EV-04 | TEST-010, 011 |
| FUNC-020 | EV-06 | TEST-006 |
| FUNC-021 | EV-04 | TEST-016 |
| FUNC-030/031 | EV-04, EV-08 | TEST-017 |

---

## [Vol. VII ch. 2](functional_and_data_requirements.md) — container, flash image and regions

Everything marked **[H]** is reproduced by `test-vectors/check_vectors.py`
(written only from this document and [Vol. VII ch. 5](cleanroom_protocol_spec.md)).
Little-endian integers unless stated. Offsets in hexadecimal.

### 1. `Code*.Dat` container

#### REQ-DATA-001 — Cipher **[H]** (EV-01, EV-02)
The file is read as little-endian u32 words `c[0..n-1]`. Plaintext:
`p[i] = c[i] XOR c[i-1]`, with `c[-1] = 0xBACADABE`. Trailing bytes that do not
form a full word do not exist in the observed files (all sizes are a
multiple of 4). To **encrypt**, use the inverse: `c[i] = p[i] XOR c[i-1]`.

#### REQ-DATA-002 — Header (plaintext) **[H]**

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

#### REQ-DATA-003 — Records **[H]**
Each record (START, END, DATA[i]) at `offset (+ i × stride)` is:
`u32 length` + complete SysEx message (`F0 … F7`) + `0x00` padding up to the
slot size. Messages (format in [Vol. VII ch. 5](cleanroom_protocol_spec.md) REQ-PROTO-002):

| Record | SysEx address | Message length |
|---|---|---|
| START | `70 10 00 00` | 14 B (1 data byte) |
| END | `70 10 00 01` | 17 B (4 data bytes) |
| DATA block | `70 30 00 09` | 599 B (586 data) |
| DATA filler | `70 30 01 09` | 14 B (data `01`) |

#### REQ-DATA-004 — 7-in-8 block packing **[H]**
The 586-byte data field is split into groups of up to 8: one MSB byte
followed by up to 7 bytes of 7 bits. Bit *j* of the MSB byte is bit 7 of
byte *j* of the group. 73 full groups + 1 one-byte group (586 = 73·8 + 2) →
**512 bytes** of flash. Every SysEx data byte has bit 7 clear.

### 2. Flash image

#### REQ-DATA-005 — Layout **[H]**
The DATA records, in order, form a sequence of 512-byte blocks (a filler
record = an erased block, 512 × `0xFF`). There is no block number:
**order is the address**. The sequence is split into two equal halves
(`count/2` blocks): half 0 = region 0 ("model"), half 1 = region 1
("engine"). Each region starts at the first block of its half and occupies
consecutive data blocks; the rest of the half is filler records. A region
fits if it takes ≤ `count/2` blocks (1 MiB in Code8, 512 KiB in Code1).
Observed occupancy: 282–365 blocks (Code8), 252–498 (Code1).

#### REQ-DATA-006 — Region header (0x40 B) **[H]**

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

#### REQ-DATA-007 — LZSS compression **[H]** (decoder)
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

#### REQ-DATA-008 — END message sum **[H]**
Let `S` = sum of all bytes of the `count` blocks (a filler counts as
512 × `0xFF`) modulo 65 536. The 4 data bytes of END are `S` with one nibble
per byte, MSB first (e.g. `S = 0x6236` → `06 02 03 06`).
Vectors: `container-vectors.json` fields `end_sum_*`; examples in
`protocol-vectors.json`.

### 3. Region contents

#### REQ-DATA-009 — SYNINFO (start of every image, 0x30 B) **[H]**

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

#### REQ-DATA-010 — Per-slot linking and relocation **[H]** (EV-05)
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

#### REQ-DATA-011 — Factory preset table, region 0 **[H]** (EV-06)
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

#### REQ-DATA-012 — Label lists, region 0 **[H]/[I]**
Near the end of region 0 (JX-3P, ~`0x11AB84`) there are structures
`<u32 2><u32 0x14><u32 list id><u32 count>…` followed by entries
`<u32 3><u32 0x1C><char[20]>` (e.g. `OFF`, `ON`). They are value texts shown
on the panel **[I]**. Changing a text here changed the label visible on
hardware (EV-04, test 4) **[H]**. Full structure **[?]**.

#### REQ-DATA-013 — Region 1 (engine) **[I]** (EV-07, EV-08)
ARM Thumb-2 code with VFP; it contains an initialisation function
(SYNINFO+0x24) that fills in and returns an exports descriptor; it reaches
the OS through a table of function pointers at `0x0113FC00`
([Vol. VII ch. 2](functional_and_data_requirements.md) REQ-ARCH-004). Past the image there is a `.bss` not
present in the file (internal pointers reach ~60 KiB past the end of the
image in JUNO-60) **[I]**.

### 4. Associated formats (outside `Code*.Dat`)

#### REQ-DATA-014 — Patch bank `Patch/*.bin` **[H]** (EV-10)
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
