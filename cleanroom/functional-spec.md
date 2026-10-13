# functional-spec.md — functional requirements and acceptance criteria

Each requirement: *Description — Certainty — Evidence — Acceptance criterion
— Tests*. It describes **what** the firmware/package does observably, not how
it is written. Tests are in `test-plan.md`.

## A. Package and delivery

### REQ-FUNC-001 — Self-contained package per slot
A `Code8_<k>.Dat` contains everything needed to leave slot *k* of the
SYSTEM-8 with a working PLUG-OUT, with no other files. **[H]** EV-01, EV-04.
*Acceptance:* a package generated independently, installed in slot *k*, is
listed by the device with the expected identity. **TEST-001, 008**.

### REQ-FUNC-002 — Distinct packages per slot
Content differs between slots only in absolute pointers
(`+0x200000` per slot) (REQ-DATA-010). **[H]** EV-05.
*Acceptance:* for each plugin, `Code8_2` and `Code8_3` are obtained from
`Code8_1` by adding the delta at the listed positions and recompressing.
**TEST-005**.

### REQ-FUNC-003 — Selection by device type
SYSTEM-8 uses `Code8_<slot>`; SYSTEM-1/1m uses `Code1`. **[H]** (host).
*Acceptance:* TEST-009.

### REQ-FUNC-004 — Integrity consistency
A package is internally consistent if: SysEx checksums are valid; the
compressed-stream sum equals the one in the region header; the END sum
equals the sum of the blocks. **[H]** (12/12 official packages comply).
*Acceptance:* TEST-002, 003, 004.

## B. Identity and lifecycle

### REQ-FUNC-010 — Published identity
After a complete, error-free install, the slot query returns the package's
name (≤16 chars), model id (u16) and version (u16). **[H]** EV-04 (name
`JX-3P CUSTOM`, version 1.02 verified).
*Acceptance:* TEST-010, 011.

### REQ-FUNC-011 — Reinstall over an occupied slot
Installing over an already occupied slot replaces it **[H]** (successive
EV-04 tests in slot 3). The host must ask for confirmation
(REQ-PROTO-004). *Acceptance:* TEST-010.

### REQ-FUNC-012 — Model uniqueness
The same model id must not be installed in two slots (host rule). Whether
the **device** also prevents it: **[?]**. Whether a free id not used by any
official PLUG-OUT is accepted: **[?]** → TEST-018 (pending).

### REQ-FUNC-013 — Persistence
The installed PLUG-OUT remains after the USB session ends **[H]** (later
queries) and after power cycling **[Hyp]** (not recorded). TEST-014 pending.

### REQ-FUNC-014 — Recovery
A faulty PLUG-OUT can be recovered by reinstalling a valid one or with the
official updater `SYSTEM8_UP.BIN` (rewrites slot 0 and the factory
PLUG-OUTs). **[I]** EV-09. *Acceptance:* cannot be safely verified; see
`clean-room-boundary.md` §6.

## C. Model content (region 0)

### REQ-FUNC-020 — Factory presets
Region 0 contains the per-model preset table (REQ-DATA-011): JX-3P 64,
JUNO-60 64, SH-101 48 (+16 empty). **[H]** EV-06.
*Acceptance:* TEST-006 (names). That the device **displays** those presets:
[?] not observed in a controlled way (the plugin sends its own bank in the
patch phase).

### REQ-FUNC-021 — Value labels
The panel's value texts (OFF/ON, waveform names, effect types…) come from
lists in region 0. **[H]** for the existence and change of one label (EV-04
test 4); the full catalogue **[?]**. *Acceptance:* TEST-016.

## D. Engine

### REQ-FUNC-030 — Parameter → DSP translation
The engine converts each parameter value (0–255 observed) through a curve
into a float sent to the DSP with a parameter id. **[I]** EV-04 test 5,
EV-08. Complete parameter→id→curve mapping: **[?]**.
*Acceptance:* TEST-017 (pending) measures via MIDI/audio the effect of
extreme values.

### REQ-FUNC-031 — OD/DS/FZ effect TONE curve (JX-3P)
The TONE value is mapped to [−1, +1] and sent; inverting the sign inverts
the audible direction of the control. **[H]** (hardware EV-04 test 5). The
remaining curves are unstudied: [?].

### REQ-FUNC-032 — Waveform
In JX-3P both DCOs send `value − 3` as the waveform selection; the
PULSE/NOISE distinction for positions 3–5 is decided by the DSP. **[I]**
(EV-08). NOISE could not be enabled natively (EV-04 note).

### REQ-FUNC-033 — Audio
Audio generation (oscillators, filters, effects): **not specifiable** with
current evidence **[?]**. A Clean-Room implementation of the *sound*
requires measuring the real device (controlled input/output recordings);
this has not been done.

## E. Errors

### REQ-FUNC-040 — Rejections
The device may reject START (`71 10 00 00` ≠ 0), a DATA record
(`71 30 00 00`) or END (`71 10 00 01` ≠ 0). **[H]** (the paths exist in the
host). Concrete causes observed on a device: none recorded **[?]**.
*Acceptance:* TEST-013 (pending).

### REQ-FUNC-041 — Transport fault tolerance
The host retries each message that expects a reply up to 3 times (5 s /
20 s). **[H]**.

## Traceability

| Requirement | Evidence | Tests |
|---|---|---|
| FUNC-001 | EV-01, EV-04 | TEST-001, 008 |
| FUNC-002 | EV-05 | TEST-005 |
| FUNC-004 | EV-01, EV-02 | TEST-002–004 |
| FUNC-010 | EV-04 | TEST-010, 011 |
| FUNC-020 | EV-06 | TEST-006 |
| FUNC-021 | EV-04 | TEST-016 |
| FUNC-030/031 | EV-04, EV-08 | TEST-017 |
