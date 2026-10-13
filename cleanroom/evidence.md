# evidence.md — evidence register

Confidence levels: **High** = reproducible by a third party with the files +
the verifier; **Medium** = a single real observation or static analysis not
reproduced here; **Low** = inference from names/structure.

## 1. Sources

| ID | Evidence | Provenance | Confidence | Reproduction |
|---|---|---|---|---|
| EV-01 | 12 `Code*.Dat` from `plugins/{JX-3P,JUNO-60,SH-101}/Script/` (SHA-256 in `container-vectors.json`) | the user's Roland Cloud plugin installs (Apr 2025) | High | `check_vectors.py --emit` |
| EV-02 | Independent reproduction of headers, regions, sums and SYNINFO with a verifier written only from the specification | this session | High | `python3 test-vectors/check_vectors.py <root>` → 12/12 OK |
| EV-03 | Install protocol: static analysis of the plugin's x86-64 code + messages extracted from the containers | `PROTOCOL.md` (repo), `protocol-vectors.json` | Medium (host); High (messages) | vectors |
| EV-04 | Tests on a real SYSTEM-8 (OS 1.32, port `hw:3,0,1`), 2026-10-10, slot 3: (1) JX-3P rebuilt identical ✔, (2) version 1.02 ✔, (3) name `JX-3P CUSTOM` ✔, (4) region 0 label `JX-3P CH`→`CUSTOM CH` ✔, (5) code patch: TONE inverted in OD/DS/FZ ✔; (6) curve-0x0e inversion: no perceptible effect | project memory / `CUSTOM_PLUGOUTS.md` | Medium (one person, one device) | requires hardware |
| EV-05 | Differences between slots 1/2/3 per plugin (counts and delta) | `slot-and-preset-vectors.json` | High | `derive_extra_vectors.py` |
| EV-06 | Preset tables in region 0 | `slot-and-preset-vectors.json` | High (structure); [?] mapping to parameters | `derive_extra_vectors.py` |
| EV-07 | References to the `0x0113FC00..FDFF` table in aligned words of the regions | analysis in this session (literal scan) | Medium (does not distinguish literal from data) | u32 scan in [0x0113FC00,0x0113FE00) |
| EV-08 | Type names (RTTI) and engine notes (curves, 336 methods, TONE) | `CUSTOM_PLUGOUTS.md` §7 + `strings` | Low/Medium; **orientation only** | — |
| EV-09 | `SYSTEM8_UP.BIN` (tar of 6 files; OS encrypted/compressed, not decoded) | repo `firmware/` | Medium | `tar tf` |
| EV-10 | `Script/Script.xml` and `Patch/*.bin` (plugin resources, not firmware) | plugins | High | — |
| EV-11 | 33 automatic repository tests against a simulated device; byte-exact rebuild of the 12 containers | `tools/test_plugout*.py` (run: OK) | Medium | `python3 -m unittest discover -s tools` |

## 2. Confirmed facts (summary)

H1 chained-XOR cipher (EV-01/02) · H2 container header and records (EV-02) ·
H3 7-in-8 and 512 B per block (EV-02) · H4 two regions by halves (EV-02) ·
H5 region header and stream sum (EV-02) · H6 LZSS with ring at `0x00`
(EV-02: all 24 regions decode to the exact size with a valid sum) · H7 END
sum (EV-02) · H8 region 0 SYNINFO = container identity (EV-02) · H9 additive
relocation (EV-05) · H10 presets (EV-06) · H11 hardware tests (EV-04).

## 3. Contradictions and resolution

| # | Sources | Contradiction | Resolution |
|---|---|---|---|
| C1 | `README.md` ("patches only tested against simulator") vs. EV-04 (real installs) | was the patch phase transmitted on hardware? | There is no record that phase 4 was verified on a device; treated as **[H] host / not verified on device** (TEST-008 must record it). |
| C2 | `JX-3P/Code1.Dat`: container `SYSTEM-8`/`0x0005`, `SYSTEM-1` signatures, `CDSPSystem8*` classes | model name vs. content | Documented as an anomaly (overview §4); not used. |
| C3 | `Code1` versions (`0x14FA`, `0x0C6D`, `0x08D9`) vs. Code8 (`0x0101`/`0x0108`) | field 0x26/0x22 does not always seem to be a "version" | Treat as opaque `u16` in Code1 [?]. |
| C4 | `PROTOCOL.md` history: "70 30 01 09 = ACK" and "70 10 00 01 at the start" | earlier erroneous version | Already resolved in the repo; the specification uses the corrected version (REQ-PROTO-006/007). |
| C5 | JUNO-60 region 1 reloc count in earlier docs (1474) vs. JX-3P (1473) | none: different plugins | Both match EV-05. |

## 4. Priority unknowns (ordered by impact)

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

## 5. Analysis limitations

* A single test device (SYSTEM-8, OS 1.32); no SYSTEM-1.
* Code analyses (EV-03, EV-07, EV-08) are static and from a single plugin
  version (build `25-0509_11:30:38`).
* Neither audio nor performance MIDI was analysed.
* The table statistics (presets) come from a single build.
