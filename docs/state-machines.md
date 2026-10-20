# state-machines.md — states and transitions

Only states **observable** from the USB-MIDI interface are modelled. The
OS's internal state is unknown [?].

## 1. Slot state (per the `70 00 01 00` query)

```
                 START ok                 END ok
   ┌──────────┐ ────────▶ ┌────────────┐ ─────────▶ ┌─────────────┐
   │ EMPTY    │           │ WRITING    │            │ OCCUPIED(M,V)│
   │ id=0xFFFF│ ◀──────── │ (not obs.) │            │ id,ver,name  │
   └──────────┘  abort/?  └────────────┘            └─────┬────────┘
        ▲                                                  │ START
        └────── (uninstall: not observed) [?]              ▼ (reinstall)
                                                        WRITING
```

| Transition | Trigger | Evidence |
|---|---|---|
| EMPTY → WRITING | START with reply 0 | [I] |
| WRITING → OCCUPIED | END with reply 0 | [H] (EV-04) |
| OCCUPIED → WRITING | new START to the same slot | [H] (EV-04) |
| WRITING → ? | interrupted transfer | [?] — see REQ-PROTO-009 |
| `71 30 00 00` during DATA | rejection | [H] host path; resulting state [?] |

State visible during WRITING (slot reported as empty, previous, or
inconsistent?): **[?]**; an `info` issued right after installing once
showed an old name (EV-04 note), consistent with the query happening before
the device updates **[Hyp]**.

## 2. Host machine (install)

```
 IDLE ─ query type/slots ─▶ CHECK
 CHECK ─ REQ-PROTO-004 rules violated ─▶ ABORT
 CHECK ─ slot same version ─▶ PATCHES_ONLY
 CHECK ─ ok ─▶ START_SENT
 START_SENT ─ 71 10 00 00 = 0 ─▶ SENDING(i = 0)
 START_SENT ─ ≠0 / 3 timeouts ─▶ ABORT
 SENDING(i) ─ i%32==31 ─▶ WAIT_FLOW ─ 71 30 03 00 ─▶ SENDING(i)
 SENDING(i) ─ sends record i ─▶ SENDING(i+1)
 SENDING(*) ─ 71 30 00 00 ─▶ ABORT
 SENDING(count) ─▶ END_SENT
 END_SENT ─ 71 10 00 01 = 0 ─▶ PATCHES
 END_SENT ─ ≠0 / 3×20 s ─▶ ABORT
 PATCHES ─ 70 20 01 01, leaves…, 70 20 01 02 ─▶ DONE
```

Notes: `WAIT_FLOW` happens before sending the record with `i%32==31`
(REQ-PROTO-006). `PATCHES_ONLY` skips START/DATA/END. **[H]** (host).

## 3. Load and run on the device

Not directly observed. **Hypothetical [Hyp]** sequence consistent with the
data structure: slot selected on the panel → the OS maps the slot's regions
→ calls the SYNINFO `init` function (REQ-ARCH-005) → uses the returned
descriptor. Pending validation (TEST-020).
