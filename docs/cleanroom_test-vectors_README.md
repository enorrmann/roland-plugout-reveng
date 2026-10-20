# test-vectors

| File | Content | How it is generated |
|---|---|---|
| `container-vectors.json` | for each of the 12 `Code*.Dat`: SHA-256, identity, record count, END sum (computed and in message), per region: stamp, load, sizes, sum, image SHA-256 and SYNINFO | `python3 check_vectors.py <root> --emit` |
| `protocol-vectors.json` | START/END/first DATA/filler messages in hex and checksum check (3 containers) | see `evidence.md` EV-03 |
| `slot-and-preset-vectors.json` | relocation counts between slots and factory preset names | `python3 derive_extra_vectors.py <root>` |
| `check_vectors.py` | reference verifier (only `data-formats.md` + `protocol-spec.md`) | `python3 -I check_vectors.py <root>` → 12 × `OK` |
| `derive_extra_vectors.py` | generates `slot-and-preset-vectors.json` | — |

`<root>` = repository directory containing `plugins/*/Script/Code*.Dat`.
The vectors contain **no** firmware code or images; only hashes, headers and
table data (see `clean-room-boundary.md` §3).

Reference results available: 12/12 containers (24 regions, 24 valid stream
sums, 12 consistent END sums). **There are no audio vectors** (pending,
`test-plan.md` TEST-017).
