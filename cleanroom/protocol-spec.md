# protocol-spec.md — PLUG-OUT firmware transfer over USB-MIDI

Sources: static analysis of the plugins' *host* code (x86-64) and the
device's replies (EV-03), queries and 5 real installs on a SYSTEM-8
(EV-04), and a simulated device (EV-11). No device-side code has been
analysed (OS encrypted, EV-09): **every device behaviour not observed is
marked [?]**.

## 1. Transport

### REQ-PROTO-001 — Port **[H]** (EV-03, EV-04)
* USB-MIDI. The synthesizer exposes two ports; the transfer port is the
  **second**, named `SYSTEM-8 CTRL` (on Linux/ALSA typically `hw:N,0,1`). The
  first does not answer these queries.
* No hardware MIDI flow control; flow control is application-level
  (REQ-PROTO-006).

### REQ-PROTO-002 — Message **[H]**
```
F0 41 <dev> 00 00 7E <cmd> <a1 a2 a3 a4> <data…> <cs> F7
```
* `41` Roland; `<dev>` Device ID, default `0x10` (adjustable on the device);
  `00 00 7E` = PLUG-OUT model id; `<cmd>` = `0x12` (DT1), also for queries
  (1 data byte).
* Address: 4 bytes of 7 bits (28 bits). `cs = (−(a1+a2+a3+a4+Σdata)) & 0x7F`.
  START example: `F0 41 10 00 00 7E 12 70 10 00 00 00 00 F7`.
  END example (JX-3P slot 1): `… 70 10 00 01 06 02 03 06 6E F7`
  (`protocol-vectors.json`).
* **Replies:** the device answers on the **same address with bit 21 set**
  (`70 xx xx xx` → `71 xx xx xx`). The host accepts a reply if address and
  data length match; the `cmd` byte is not checked **[H]** (host).
* Messages with an invalid checksum: device reaction **[?]**.

## 2. Device and slot query

### REQ-PROTO-003 **[H]**
All are DT1 with 1 data byte (`00` unless noted):

| Send (address) | Reply | Content |
|---|---|---|
| `70 00 02 00` | `71 00 02 00`, 1 B | type: 0 SYSTEM-1, 1 SYSTEM-1m, 2 SYSTEM-8 |
| `70 00 01 00` (data `00`) | `71 00 01 00`, 48 B | slot 0: see below |
| `70 00 01 00` (data `i`) | 48 B | slot *i* |
| `70 00 02 01` | 1 B | factor A |
| `70 00 02 02` | 1 B | factor B |
| `70 00 02 03` | 1 B | number of slots |

* Patches per slot = A × B (64 on the tested SYSTEM-8).
* 48-byte slot block (one nibble per byte, MSB first): bytes 0–3 firmware
  version (slot 0) / bytes 8–11 model id, 12–15 version, 16–47 name
  (16 characters × 2 nibbles). Id `0xFFFF` = empty slot **[H]**.
* Minimum firmware: SYSTEM-1/1m ≥ 1.30, SYSTEM-8 ≥ 1.31 (plugin rule).
* Additional reads (`00 00 68 07`, `…68 16`, `…68 13..15`, RQ1): purely
  informational; they do not affect the transfer **[I]**.

### REQ-PROTO-004 — Host rules before installing **[H]** (host)
1. Refuse if the same model id is already in **another** slot.
2. If the slot already has the same id and version: skip firmware, send only
   patches (after confirmation).
3. If the slot holds another PLUG-OUT: confirm with the user.
4. Slot 0 (the synthesizer) is never a target.
5. File: SYSTEM-8 → `Code8_<slot>.Dat`; SYSTEM-1/1m → `Code1.Dat`.
   Whether the **device** also enforces these rules: **[?]**.

## 3. Install sequence

### REQ-PROTO-005 — START **[H]**
Send the START record with its data byte replaced by `slot | flags` and the
checksum recomputed. `flags = 0x40` if the plugin is not authorized by
Roland Cloud; `0x00` if it is. Expected reply `71 10 00 00` (1 B), value
`00` = accepted; other value = rejected (codes **[?]**). Timing: 5 s per
attempt, 3 attempts (same message resent). Functional difference between
`flags 0x40` and `0x00` on the device **[?]**; hardware tests used `0x40`.

### REQ-PROTO-006 — DATA **[H]**
For `i = 0 … count−1`, in order:
1. If `i % 32 == 31`: send `70 30 03 00` (data `00`) and wait for
   `71 30 03 00` with 4 bytes (content not checked) before continuing.
2. Send record `i` **exactly as stored** (block `70 30 00 09` or filler
   `70 30 01 09`).
3. If `71 30 00 00` (1 B) arrives at any time: the device rejected a record
   → abort (cause and code **[?]**).
There is no per-record ACK or sequence number; order is address
(REQ-DATA-005). Filler records **are sent** (not skipped).

### REQ-PROTO-007 — END **[H]**
Send the END record unmodified (4 data bytes: REQ-DATA-008). Reply
`71 10 00 01` (1 B), `00` = OK. Timing: 20 s per attempt, 3 attempts.
Observable effect: after OK the slot appears in the query with the package's
name/id/version (EV-04). Whether the device recomputes and compares the sum,
and what happens on mismatch: **[?]** (TEST-013 pending).

### REQ-PROTO-008 — Pacing **[H]/[I]**
The plugin waits `10 ms + length × 0.32 ms` between messages. The
independent tools in this repository add no pauses and the device sets the
pace with the flow query every 32 records (the send blocks when the USB
buffer fills) **[H]** (real installs without pauses, EV-04). Total size:
4096 × 599 B ≈ 2.4 MB per Code8 install.

### REQ-PROTO-009 — Power-off/disconnect **[?]**
Device behaviour if the transfer is interrupted (half-written slot, state
after reboot) is untested; the repository guide recommends the official
updater as the recovery path **[I]**.

## 4. Patch phase (after END)

### REQ-PROTO-010 **[H]** (host; actual transmission unverified, see EV-11)
No replies (plain DT1):
1. `70 20 01 01` [`n`] with `n = A × B` (64).
2. For each patch `p = 0..n−1` and each structure (`usrX`, `usr2X`, `usr3X`
   on SYSTEM-8; only `usrX` on SYSTEM-1), one DT1 per leaf of `Script.xml`
   (EV-10), at `base(slot,p,structure) + leaf offset`.
3. `70 20 01 02` [`1` if the slot already had the same version, else `0`].

Addresses (28-bit integers, 7 bits per byte): for `usrX` with
`b = p / 8`, `i = p % 8`:
* slot 0/1: `((b<<7)+i)<<14` + `0x4000000` (slot 0) / `0x8000000` (slot 1);
* slot 2/3: `((b<<4)+i)<<14` + `0x1C00000` (slot 2) / `0x1E00000` (slot 3).

For `usr2X`/`usr3X` (k = 1/2): `(p<<18) + T[slot + (k−1)·4]`,
`T = 0x5000000, 0x6000000, 0x7000000, 0xD000000, 0x9000000, 0xA000000,
0xB000000, 0xC000000`. Values: `intNxM` = N bytes of M bits, MSB first;
`stringNx4` = 2 nibbles per character.

`Patch/*.bin` bank format: `data-formats.md` REQ-DATA-014 (outside the
firmware; EV-10). Independent of the contents of `Code*.Dat`.

## 5. Address summary

| Address | Dir. | Data | Reply |
|---|---|---|---|
| `70 00 02 00` | H→D | `00` | `71 00 02 00` type |
| `70 00 01 00` | H→D | slot | `71 00 01 00` 48 B |
| `70 00 02 01/02/03` | H→D | `00` | 1 B |
| `70 10 00 00` | H→D | slot\|flags | `71 10 00 00` 1 B, 0=OK |
| `70 30 00 09` | H→D | 586 B (512 B 7-in-8) | `71 30 00 00` only on error |
| `70 30 01 09` | H→D | `01` | same |
| `70 30 03 00` | H→D | `00` | `71 30 03 00` 4 B |
| `70 10 00 01` | H→D | 4 nibbles | `71 10 00 01` 1 B, 0=OK |
| `70 20 01 01/02` | H→D | n / flag | none |

## 6. Sequence diagram

```
Host                                   SYSTEM-8
 │── 70 00 02 00 ───────────────────▶ │
 │◀────────────────── 71 00 02 00 [2] │   type
 │── 70 00 01 00 [0..N] ─────────────▶ │
 │◀────────────── 71 00 01 00 [48 B]  │   slot identities
 │── 70 10 00 00 [slot|flags] ────────▶ │   START
 │◀────────────────── 71 10 00 00 [00] │
 │── DATA 0 … DATA 30 ────────────────▶ │
 │── 70 30 03 00 [00] ────────────────▶ │   before DATA 31
 │◀────────────── 71 30 03 00 [4 B]    │
 │── DATA 31 … (every 32) ────────────▶ │
 │── END 70 10 00 01 [sum] ───────────▶ │   up to 20 s
 │◀────────────────── 71 10 00 01 [00] │
 │── 70 20 01 01 … patches … 70 20 01 02 ─▶  (no replies)
```

## 7. Non-ideal cases (summary)

| Case | Host behaviour | Device behaviour |
|---|---|---|
| No reply in 5/20 s | resends up to 3 times, then aborts | — |
| `71 10 00 00` ≠ 0 | aborts ("device refused") | [?] |
| `71 30 00 00` | aborts ("rejected data record") | [?] |
| Duplicate / out-of-order message | untested | [?] |
| Different Device ID | queries get no reply | [I] |
