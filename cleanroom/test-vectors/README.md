> [!NOTE]
> **TRM Technical Reference Manual**: This implementation example is documented normatively in [Volume VI: Reference Designs & Applications](../../docs/vol7_cleanroom_evidence_archive/test_vectors_verification_plan.md).

# test-vectors

| File | Content | How it is generated |
|---|---|---|
| `container-vectors.json` | for each of the 12 `Code*.Dat`: SHA-256, identity, record count, END sum (computed and in message), per region: stamp, load, sizes, sum, image SHA-256 and SYNINFO | `python3 check_vectors.py <root> --emit` |
| `protocol-vectors.json` | START/END/first DATA/filler messages in hex and checksum check (3 containers) | see [Vol. VII ch. 3](../../docs/vol7_cleanroom_evidence_archive/test_vectors_verification_plan.md) EV-03 |
| `slot-and-preset-vectors.json` | relocation counts between slots and factory preset names | `python3 derive_extra_vectors.py <root>` |
| `check_vectors.py` | reference verifier (only [Vol. VII ch. 2](../../docs/vol7_cleanroom_evidence_archive/functional_and_data_requirements.md) + [Vol. VII ch. 5](../../docs/vol7_cleanroom_evidence_archive/cleanroom_protocol_spec.md)) | `python3 -I check_vectors.py <root>` → 12 × `OK` |
| `derive_extra_vectors.py` | generates `slot-and-preset-vectors.json` | — |

`<root>` = repository directory containing `plugins/*/Script/Code*.Dat`.
The vectors contain **no** firmware code or images; only hashes, headers and
table data (see [Vol. VII ch. 1](../../docs/vol7_cleanroom_evidence_archive/cleanroom_boundary_contract.md) §3).

Reference results available: 12/12 containers (24 regions, 24 valid stream
sums, 12 consistent END sums). **There are no audio vectors** (pending,
[Vol. VII ch. 3](../../docs/vol7_cleanroom_evidence_archive/test_vectors_verification_plan.md) TEST-017).
