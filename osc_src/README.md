# `osc_src` — decompiled reconstruction of `os_full.bin`

`os_full.bin` (2 939 504 B, `0x60000000`–`0x602CDA6F`) is the SYSTEM-8 OS as
it sits in the master BMC's SDRAM while the synth runs
([`../docs/BASE_FIRMWARE.md`](../docs/BASE_FIRMWARE.md) §7.14, §8).  Its
upstream copy in the updater (`App1_Main`) is encrypted, so this image is the
only plaintext source: every file here is derived from the dump.

This folder is a **self-contained C (and assembly) reconstruction** of that
image:

```bash
make            # rebuild ../dumps/os_full.bin from the sources, then verify it
make verify     # compare the rebuilt image with the dump (sha256 + byte diff)
make clean
```

The rebuilt file is written to `code/build/os_full.bin`; the makefile refuses to
finish unless it is identical to `../dumps/os_full.bin`.

Nothing outside `osc_src/` is written by these tools: inputs are read from
`../dumps/`, `../tools/` and `../work/`, outputs stay here.

## Status

| Stage | State |
|---|---|
| Analysis (`ghidra/`, `tools/`) | Ghidra 11.3.2 headless, ARM:LE:32:Cortex; 281 456 code units, 9 000+ functions, 57 000 symbols |
| Data layer (`data/`) | generated: literal pools, tables, string/rodata blobs and padding emitted as C objects at fixed addresses |
| Code layer (`code/`) | per-function C recovered from Ghidra, assembled through a symbol table so every `bl`/branch/literal-pool reference re-resolves to the original address |
| Byte-exact rebuild | `make verify` |

## Method

See [`NOTES.md`](NOTES.md) for the full write-up: segmentation, the Ghidra
pipeline, how the code layer is made link-exact and what is still open.
