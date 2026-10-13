# test-plan.md — validation and conformance matrix

Status: **A** = automatable today without hardware (`test-vectors/`);
**H** = requires a real SYSTEM-8; **P** = pending (no verifiable reference;
no expected result is invented).

All **A** tests run with
`python3 test-vectors/check_vectors.py <repo-root>` (reference) or against
Team B's implementation by comparing with the JSON in `test-vectors/`. The
`<root>` paths point to the official `Code*.Dat` (material **excluded** from
the transfer; see `clean-room-boundary.md`: Team B uses them only through the
vector JSON).

## 1. Tests

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

## 2. Quantifiable compatibility criteria

| Level | Definition | Threshold |
|---|---|---|
| C0 Container | TEST-001…006 | 100 % exact on the 12 packages |
| C1 Rebuild | TEST-007 | 12/12 identical files (or identical decompressed image) |
| C2 Install | TEST-008, 010, 011 | 3/3 on a real SYSTEM-8, ≥ 1 per plugin |
| C3 Data | TEST-016 | modified labels/identity visible |
| C4 Audio | TEST-017 | **not defined**: requires measured references (pending) |

"Compatible" in this document means C0–C3. **C4 is not reachable with the
current evidence** (REQ-FUNC-033).

## 3. Differential / regression tests

* Each package produced by Team B is installed in a test slot and `info` is
  compared with the expected value; the official one is then reinstalled
  and TEST-010/008 repeated (round-trip regression).
* A **P** test is never declared passed without a real reference.

## 4. Requirement → test matrix

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
