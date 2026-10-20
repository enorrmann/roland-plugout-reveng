# Overview — PLUG-OUT firmware (`Code1.Dat`, `Code8_1/2/3.Dat`)

Clean-Room transfer document (Team A → Team B).
Certainty marks used in every document:

| Mark | Meaning |
|---|---|
| **[H]** Fact | backed by reproducible evidence (see `evidence.md`, vectors in `test-vectors/`) |
| **[I]** Inference | reasonable deduction from evidence, no direct proof |
| **[Hyp]** Hypothesis | pending validation; do not use as a firm requirement |
| **[?]** Unknown | cannot be determined with current evidence |

## 1. Scope

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

## 2. System summary

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

## 3. Reference figures (all **[H]**, `test-vectors/container-vectors.json`)

| Plugin | Container | Records | Data blocks | Region 0 plain | Region 1 plain |
|---|---|---|---|---|---|
| JX-3P | Code8_1..3 | 4096 | 651 | 1 159 752 B | 282 172 B |
| JUNO-60 | Code8_1..3 | 4096 | 640 | 1 158 192 B | 265 024 B |
| SH-101 | Code8_1..3 | 4096 | 588 | 1 158 616 B | 211 212 B |
| SH-101 | Code1 | 2048 | 560 | 215 012 B | 148 132 B |
| JX-3P | Code1 | 2048 | 934 | 370 308 B | 448 832 B |

## 4. Observed anomalies

* `JX-3P/Script/Code1.Dat` **[H]**: the container header says `SYSTEM-8`,
  id `0x0005`, version `0x0C6D`; its region signature is `SYSTEM-1:Plugout`,
  stamped `16-0310_12:05:02` (2016), with `CDSPSystem8*` classes. It is not a
  JX-3P. Function **[?]**; hypothesis: build of the synthesizer's own base
  engine for SYSTEM-1 **[Hyp]**. Not usable as a JX-3P PLUG-OUT
  (`docs/PROTOCOL.md` §3).
* `SH-101/Script/Code.Dat` **[H]**: 2015 build (`15-0924_14:45:20`), unused.

## 5. Analysis limitations

1. Hardware verification exists only on one SYSTEM-8 (OS 1.32), and only with
   JX-3P variants in slot 3 (EV-04). JUNO-60, SH-101 and SYSTEM-1 have not
   been tested on a real device.
2. The OS (`App1_Main`, entropy ≈ 8.0) has not been decoded: what the device
   actually validates (only the sums? a signature?) is unknown.
3. The DSP program and the complete *parameter → DSP id* mapping are unknown;
   without them there is no audio specification.
4. The OS service table (`0x0113FC00`) is only partially mapped
   (REQ-ARCH-004).

## 6. Document map

| Document | Content |
|---|---|
| `cleanroom_architecture.md` | components, execution model, OS interface |
| `functional-spec.md` | `REQ-FUNC-*` requirements and acceptance criteria |
| `protocol-spec.md` | `REQ-PROTO-*`: SysEx, install sequence |
| `data-formats.md` | `REQ-DATA-*`: container, image, regions, LZSS, SYNINFO |
| `state-machines.md` | host, transfer, slot |
| `test-plan.md` | `TEST-*` and conformance matrix |
| `test-vectors/` | vectors and reproducible verifier |
| `evidence.md` | `EV-*` register, confidence, open items |
| `clean-room-boundary.md` | team separation rules |
