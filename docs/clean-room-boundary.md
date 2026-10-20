# clean-room-boundary.md — team separation and authorised material

This is not legal advice. This policy describes an **operational, verifiable**
separation; whether it is legally sufficient depends on the jurisdiction and
the purpose (interoperability, research), which a lawyer must assess.

## 1. Roles

| Team | Does | Receives | Delivers |
|---|---|---|---|
| **A** (analysis) | examines `Code*.Dat`, plugins, `SYSTEM8_UP.BIN`, the real device | everything | only the approved content of `cleanroom/` |
| **Reviewer** (independent of A and B) | filters A's delivery | A's draft | approved list; rejects contaminated material |
| **B** (implementation) | designs/implements/tests | only approved material (§3) | own code + decision log |

**Current situation (honest):** this repository has a single author, and
Team A (this specification) and any future implementation share history,
tools and disassembly notes. **Real separation does not exist yet**: it is
achieved only when B is another person/team working from a copy that
contains only §3. Renaming things or rewriting code is not a substitute.

## 2. Material excluded from the transfer (contaminated)

| Material | Reason |
|---|---|
| `plugins/**/*.vst3`, `*.dat`, `unins000.exe`, proprietary images/`Script*.xml` | Roland works; see note 1 |
| `plugins/**/Script/Code*.Dat`, `firmware/SYSTEM8_UP.BIN` and their decompressed regions, `.elf`, `.bin` | original firmware |
| `docs/CUSTOM_PLUGOUTS.md` §7 (engine notes, function addresses, instruction listings, TONE patch) and the code-address columns of `docs/PROTOCOL.md` | derived from disassembly |
| `examples/cave_tone/*` (`engine.s`, `hook.c`, `hook.o`, `*.elf`) | assembly/code tied to the original implementation |
| `tools/*.py` | written while disassembling (comments carry plugin addresses); B does not consult them |
| Pseudocode or Ghidra/IDA captures, A's internal notes | by definition |
| Internal class/method names (RTTI) | accidental details; quoted in `cleanroom_architecture.md` only as **non-normative** orientation; the Reviewer decides whether to remove them |

Note 1: the plugins and packages are third-party and are not redistributed;
B's tests compare against vectors (JSON), not against the binaries.

## 3. Material authorised for B

1. `docs/overview.md`, `cleanroom_architecture.md`, `functional-spec.md`,
   `protocol-spec.md`, `data-formats.md`, `state-machines.md`,
   `test-plan.md`, `evidence.md`, this document.
2. `cleanroom/test-vectors/*.json` (numeric values, hashes, preset names,
   24 header bytes of one record).
3. `cleanroom/test-vectors/check_vectors.py` and `derive_extra_vectors.py`:
   **verification references** written only from the specification; B may
   run them but must implement its own version.
4. Devices: access to its own SYSTEM-8/-1 and to the public output of the
   protocol (black-box tests).
5. The `Code*.Dat` files themselves: B may **use them as test input** (it
   lawfully holds them through the plugin install) but must **not**
   disassemble the code content of region 1 or decompress with A's tools.
   The Reviewer decides whether B receives the files or only the vectors.

## 4. Procedural rules

1. Two separate repositories/copies; B has no read permission on A's.
2. Every question from B to A: in writing, through the Reviewer; answers are
   published in `evidence.md`/`functional-spec.md` (not answered with code).
3. B records in its own repository: design decisions, sources consulted,
   date and who; no links to A's notes.
4. Contamination review: automatic comparison of B with the §2 material
   (strings, non-trivial constants, structure) before release.
5. If B finds it needs a **[?]** datum, it files an experiment request
   (real device or measurement), not a code-reading request.
6. Auditable A→B and B→A access log (hash of each delivery).

## 5. What the specification does **not** transfer

* Algorithms or code of the engine or the DSP; nor how the original LZSS is
  implemented (only its decoding contract).
* Any internal function address of the plugin or the firmware.

## 6. Operational safety risks

* Installing a malformed package into a slot can make it unusable until
  reinstalling via the official updater **[I]**; always test in a test slot,
  with `--dry-run`.
* Never overwrite slot 0.
* No firmware authentication is known: the security impact of installing
  unofficial firmware has not been assessed; this documentation does not
  aim to circumvent Roland Cloud licences (the `0x40` authorisation flag is
  described only as part of the protocol).
