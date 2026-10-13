# architecture.md — logical architecture

Separates what is needed for compatibility (the **contract**) from accidents
of the original implementation (**non-normative**). Marks: see `overview.md`.

## 1. Components

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

### REQ-ARCH-001 — Packaging layers **[H]**
`Code*.Dat` ⊃ SysEx records ⊃ 512-byte blocks ⊃ regions (0x40 header +
LZSS) ⊃ image (SYNINFO + data + code). Detail in `data-formats.md`.

### REQ-ARCH-002 — One image per slot **[H]** (EV-05)
Each PLUG-OUT slot has its own **2 MiB window** per region
(REQ-DATA-010). The package for slot *k* contains an image linked to that
slot's addresses. A package for one slot is not valid in another without
relocating the absolute pointers listed in REQ-DATA-010. Relocation is
additive and uniform (adds `0x200000 × Δslot`); no movw/movt or other forms
are observed **[H]** (scope: the 3 plugins).

### REQ-ARCH-003 — Duplicated identity **[H]**
Name, model id and version appear in the container header and in the SYNINFO
of region 0; the host uses the container, the device publishes what is in
the slot. Whether the device derives identity from one or the other: **[?]**
(in tests 2–3 of EV-04 both were changed together).

### REQ-ARCH-004 — OS interface: service table **[I]** (EV-07)
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

### REQ-ARCH-005 — Export (`init`) **[I]**
SYNINFO+0x24 points to a Thumb function that initialises a descriptor in
`.bss` with function pointers and returns its address; that descriptor is
the interface the OS calls (interface class `CPlugoutInterface` in the type
strings, **no normative value**).

### REQ-ARCH-006 — Execution model **[Hyp]/[?]**
Whether the OS calls the engine through tasks, interrupts or callbacks, and
its deadlines, has not been determined. The only observable: the device
panel (parameter values 0–255) causes engine calls towards the DSP (EV-04,
test 5: a code patch that inverts the sign of the value sent changes the
sound of the TONE control in OD/DS/FZ).

## 2. Engine internals (non-normative) [I]

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

## 3. SYSTEM-1 variant (Code1)

* Signature `SYSTEM1_SYNINFO`; regions with load `0x60800000`/`0x60200000`;
  classes `CPrmDSPPlugoutMaster` (region 0) and `CPrmDSPPlugoutSlave`
  (region 1) in SH-101 **[H]** → two processors/contexts, master and slave
  **[Hyp]**.
* No evidence of multiple slots: a single install.

## 4. Hardware dependencies / required abstractions

* ARM Thumb-2 + VFP family (exact core **[?]**).
* Memory map: flash (windows `0x6xxxxxxx`) and service table at `0x0113FC00`.
* An alternative host-side implementation needs only USB-MIDI + SysEx (see
  `protocol-spec.md`); nothing from the CPU is required.
