# Volume VII — Chapter 3: Clean-Room Test Plan and Evidence Register

> **Scope:** both.

> The test vectors and their checker are described in
> [cleanroom/test-vectors/README.md](../../cleanroom/test-vectors/README.md).

---

## [Vol. VII ch. 3](test_vectors_verification_plan.md) — validation and conformance matrix

Status: **A** = automatable today without hardware (`test-vectors/`);
**H** = requires a real SYSTEM-8; **P** = pending (no verifiable reference;
no expected result is invented).

All **A** tests run with
`python3 test-vectors/check_vectors.py <repo-root>` (reference) or against
Team B's implementation by comparing with the JSON in `test-vectors/`. The
`<root>` paths point to the official `Code*.Dat` (material **excluded** from
the transfer; see [Vol. VII ch. 1](cleanroom_boundary_contract.md): Team B uses them only through the
vector JSON).

### 1. Tests

| ID | Objective | Preconditions | Inputs | Procedure | Expected result | Tolerance | Evidence | Status |
|---|---|---|---|---|---|---|---|---|
| TEST-001 | Decode official container | 12 files | `Code*.Dat` | decrypt (REQ-DATA-001), read header | fields = `container-vectors.json` (`name`,`model_id`,`version`,`records`) | exact | EV-01, EV-02 | A |
| TEST-002 | SysEx checksums | TEST-001 | all records | `cs=(-Σ addr+data)&7F` | 100 % valid (`protocol-vectors.json: checksums_ok`) | exact | EV-03 | A |
| TEST-003 | Compressed-stream sum | TEST-001 | regions | Σ bytes mod 2³² vs `0x34` | `header_sum_ok = true` in 24/24 regions | exact | EV-02 | A |
| TEST-004 | END sum | TEST-001 | blocks | REQ-DATA-008 | `end_sum_computed == end_sum_in_message` | exact | EV-02 | A |
| TEST-005 | Per-slot relocation | Code8_1,2,3 | decompressed | word-by-word diff | counts and delta of `slot-and-preset-vectors.json:relocation`; 0 "other" differences | exact | EV-05 | A |
| TEST-006 | Preset table | region 0 | image | read 0x8000 + i·0x200 + 0x100, reverse 4-byte groups | `presets` of the JSON | exact | EV-06 | A |
| TEST-007 | Byte-exact build | TEST-001..006 | decompressed image of an official file | recompress, 7-in-8 pack, sums, encrypt | file identical to the official one (SHA-256 in the JSON) | exact; if the LZSS encoder differs, compare the **decompressed** image and flag | EV-01, EV-11 | A |
| TEST-008 | Install an independent package | SYSTEM-8, free slot | Team B package (rebuild from TEST-007) | START…END (REQ-PROTO-005..007) | replies `00`; slot shown with correct identity | exact | EV-04 | H |
| TEST-009 | File selection by device | SYSTEM-1 and SYSTEM-8 | device type | query `70 00 02 00` | SYSTEM-8→Code8_k; SYSTEM-1→Code1 | exact | EV-03 | A (logic) / H |
| TEST-010 | Reinstall over occupied slot | TEST-008 | two packages with different versions | install A then B | `info` shows B | exact | EV-04 | H |
| TEST-011 | Identity change | TEST-008 | name/version modified in container **and** SYNINFO | install, query | new name and version (≤16 chars) | exact | EV-04 | H |
| TEST-012 | Acceptance of another LZSS encoder | TEST-008 | same content with a different LZSS stream | install | works like the official one | exact | — | P |
| TEST-013 | Device rejections | free slot | (a) wrong END sum, (b) wrong region sum, (c) wrong SysEx checksum | install | record replies/state | n/a | — | P (risk of leaving the slot inconsistent) |
| TEST-014 | Persistence | TEST-008 | — | power off and on; query slot | slot keeps identity | exact | — | P |
| TEST-015 | Flow control | TEST-008 | omit `70 30 03 00` queries | install | record whether the device loses data | n/a | — | P |
| TEST-016 | Region 0 label | TEST-008 | modify a list text | install; look at the panel | label changed | exact | EV-04 test 4 | H |
| TEST-017 | Parameter sweep | TEST-008 | values 0,64,128,192,255 of a control | send patch/parameter, record audio/CV | monotonic curve; magnitude ± 1 dB (measurement) | ± 1 dB / ± 5 cents | EV-04 test 5 | P (no numeric reference) |
| TEST-018 | New model id | test slot | free id `0x00xx` | install | accepted/rejected and what the panel does | n/a | — | P |
| TEST-019 | Tool interoperability | — | package B | install with the repo tool and with the official plugin | both accept | exact | EV-11 | H |
| TEST-020 | Service-table semantics | decode OS or experiments | entries `+0xA0…+0xF0` | instrument calls | documented signatures | n/a | EV-07 | P |

### 2. Quantifiable compatibility criteria

| Level | Definition | Threshold |
|---|---|---|
| C0 Container | TEST-001…006 | 100 % exact on the 12 packages |
| C1 Rebuild | TEST-007 | 12/12 identical files (or identical decompressed image) |
| C2 Install | TEST-008, 010, 011 | 3/3 on a real SYSTEM-8, ≥ 1 per plugin |
| C3 Data | TEST-016 | modified labels/identity visible |
| C4 Audio | TEST-017 | **not defined**: requires measured references (pending) |

"Compatible" in this document means C0–C3. **C4 is not reachable with the
current evidence** (REQ-FUNC-033).

### 3. Differential / regression tests

* Each package produced by Team B is installed in a test slot and `info` is
  compared with the expected value; the official one is then reinstalled
  and TEST-010/008 repeated (round-trip regression).
* A **P** test is never declared passed without a real reference.

### 4. Requirement → test matrix

| Requirement | Tests |
|---|---|
| REQ-DATA-001..004 | TEST-001, 007 |
| REQ-DATA-005..008 | TEST-003, 004, 007 |
| REQ-DATA-009..011 | TEST-001, 006, 011 |
| REQ-DATA-010 | TEST-005 |
| REQ-PROTO-002 | TEST-002 |
| REQ-PROTO-003..004 | TEST-009 |
| REQ-PROTO-005..008 | TEST-008, 013, 015 |
| REQ-FUNC-010..013 | TEST-010, 011, 014, 018 |
| REQ-FUNC-021 | TEST-016 |
| REQ-FUNC-030..033 | TEST-017 |
| REQ-ARCH-004 | TEST-020 |

---

## [Vol. VII ch. 3](test_vectors_verification_plan.md) — evidence register

Confidence levels: **High** = reproducible by a third party with the files +
the verifier; **Medium** = a single real observation or static analysis not
reproduced here; **Low** = inference from names/structure.

### 1. Sources

| ID | Evidence | Provenance | Confidence | Reproduction |
|---|---|---|---|---|
| EV-01 | 12 `Code*.Dat` from `plugins/{JX-3P,JUNO-60,SH-101}/Script/` (SHA-256 in `container-vectors.json`) | the user's Roland Cloud plugin installs (Apr 2025) | High | `check_vectors.py --emit` |
| EV-02 | Independent reproduction of headers, regions, sums and SYNINFO with a verifier written only from the specification | this session | High | `python3 test-vectors/check_vectors.py <root>` → 12/12 OK |
| EV-03 | Install protocol: static analysis of the plugin's x86-64 code + messages extracted from the containers | [Vol. IV ch. 4](../vol4_firmware_container_protocol/usb_sysex_protocol_spec.md) (repo), `protocol-vectors.json` | Medium (host); High (messages) | vectors |
| EV-04 | Tests on a real SYSTEM-8 (OS 1.32, port `hw:3,0,1`), 2026-10-10, slot 3: (1) JX-3P rebuilt identical ✔, (2) version 1.02 ✔, (3) name `JX-3P CUSTOM` ✔, (4) region 0 label `JX-3P CH`→`CUSTOM CH` ✔, (5) code patch: TONE inverted in OD/DS/FZ ✔; (6) curve-0x0e inversion: no perceptible effect | project memory / [Vol. IV ch. 2](../vol4_firmware_container_protocol/plugout_virtual_machine.md) | Medium (one person, one device) | requires hardware |
| EV-05 | Differences between slots 1/2/3 per plugin (counts and delta) | `slot-and-preset-vectors.json` | High | `derive_extra_vectors.py` |
| EV-06 | Preset tables in region 0 | `slot-and-preset-vectors.json` | High (structure); [?] mapping to parameters | `derive_extra_vectors.py` |
| EV-07 | References to the `0x0113FC00..FDFF` table in aligned words of the regions | analysis in this session (literal scan) | Medium (does not distinguish literal from data) | u32 scan in [0x0113FC00,0x0113FE00) |
| EV-08 | Type names (RTTI) and engine notes (curves, 336 methods, TONE) | [Vol. IV ch. 2](../vol4_firmware_container_protocol/plugout_virtual_machine.md) §7 + `strings` | Low/Medium; **orientation only** | — |
| EV-09 | `SYSTEM8_UP.BIN` (tar of 6 files; OS encrypted/compressed, not decoded) | repo `firmware/` | Medium | `tar tf` |
| EV-10 | `Script/Script.xml` and `Patch/*.bin` (plugin resources, not firmware) | plugins | High | — |
| EV-11 | 33 automatic repository tests against a simulated device; byte-exact rebuild of the 12 containers | `tools/test_plugout*.py` (run: OK) | Medium | `python3 -m unittest discover -s tools` |

### 2. Confirmed facts (summary)

H1 chained-XOR cipher (EV-01/02) · H2 container header and records (EV-02) ·
H3 7-in-8 and 512 B per block (EV-02) · H4 two regions by halves (EV-02) ·
H5 region header and stream sum (EV-02) · H6 LZSS with ring at `0x00`
(EV-02: all 24 regions decode to the exact size with a valid sum) · H7 END
sum (EV-02) · H8 region 0 SYNINFO = container identity (EV-02) · H9 additive
relocation (EV-05) · H10 presets (EV-06) · H11 hardware tests (EV-04).

### 3. Contradictions and resolution

| # | Sources | Contradiction | Resolution |
|---|---|---|---|
| C1 | `README.md` ("patches only tested against simulator") vs. EV-04 (real installs) | was the patch phase transmitted on hardware? | There is no record that phase 4 was verified on a device; treated as **[H] host / not verified on device** (TEST-008 must record it). |
| C2 | `JX-3P/Code1.Dat`: container `SYSTEM-8`/`0x0005`, `SYSTEM-1` signatures, `CDSPSystem8*` classes | model name vs. content | Documented as an anomaly (overview §4); not used. |
| C3 | `Code1` versions (`0x14FA`, `0x0C6D`, `0x08D9`) vs. Code8 (`0x0101`/`0x0108`) | field 0x26/0x22 does not always seem to be a "version" | Treat as opaque `u16` in Code1 [?]. |

### 4. Priority unknowns (ordered by impact)

1. Real device validation: does it check sums, version, id, signature?
   (TEST-013)
2. Semantics of the `0x0113FC00` service table and the layout of the `init`
   descriptor (TEST-020) — blocks a PLUG-OUT from scratch.
3. Minimum/mandatory content of region 0 (not just presets).
4. DSP program and complete parameter→id→curve mapping (blocks audio, C4).
5. Effect of the START `0x40` flag on the device.
6. Meaning of opaque fields: SYNINFO+0x28, region 1 `0x000A/0x0001`, region
   `0x20` (tool), Code1 versions.
7. Slot state after an interrupted transfer.
8. Persistence after power-off; acceptance of new model ids.
9. SYSTEM-1/1m: no hardware tests at all.

### 5. Analysis limitations

* A single test device (SYSTEM-8, OS 1.32); no SYSTEM-1.
* Code analyses (EV-03, EV-07, EV-08) are static and from a single plugin
  version (build `25-0509_11:30:38`).
* Neither audio nor performance MIDI was analysed.
* The table statistics (presets) come from a single build.
