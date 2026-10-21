# osc_src — decompiled reconstruction of the SYSTEM-8 OS (`os_full.bin`)

`os_full.bin` (2 939 504 B, `0x60000000`–`0x602CDA6F`) is the SYSTEM-8 OS as it
sits in the master BMC's SDRAM while the synth runs.  The copy in the updater
(`App1_Main`) is encrypted and the key is held by the BMC, so this dump is the
only plaintext source of the operating system; everything here is derived from
it.  See [`../docs/BASE_FIRMWARE.md`](../docs/BASE_FIRMWARE.md) §7.14 for how
the dump was obtained.

This folder reconstructs the image from source:

```bash
make            # assemble + link every unit, then verify against the dump
make verify     # re-check the rebuilt image
make clean
```

**Result: the rebuilt image is byte-identical to `../dumps/os_full.bin`**
(sha256 `2600b866f751f38ac9d96fc771c9cd550e2e01d654c16f3277452214f3471f40`,
`cmp` exits 0).

Nothing outside `osc_src/` is written: the tools read `../dumps/os_full.bin` and
emit into `osc_src/`.

```
osc_src/
├── Makefile              build + verify
├── README.md             this file
├── NOTES.md              method, measurements, limitations
├── tools/
│   ├── segment.py        Thumb-2 sweep: code vs data (and the shared cache)
│   ├── gen_asm.py        code layer  -> code/asm/*.s   (9 373 units)
│   ├── gen_data.py       data layer  -> data/*.s + *.c (16 734 runs)
│   ├── build_le.py       linker: places objects, applies relocations
│   ├── rebuild.py        driver used by make
│   ├── verify.py         byte/SHA-256 comparison
│   └── selftest_link.py  checks build_le against GNU as/ld
├── ghidra/               headless scripts used for the analysis
├── code/asm/             generated code (per function)
├── data/                 generated data (per run)
└── build/, code/build/   cache, objects and the rebuilt image (not tracked)
```

## The reconstruction in three layers

The image holds 2 897 218 B of Thumb-2 code (98.56 %, 1 339 349 instructions)
and 42 286 B of data in 16 734 runs; `tools/segment.py` computes and caches
that split.  Each part is reconstructed the way that keeps the bytes exact:

**1. Data (`tools/gen_data.py` → `data/`)**
Every non-code run is emitted as a named C array (`d_<address>`, in `const`
arrays with a comment giving the run's size and kind — `fill`, `zero`, `text`,
`pointers` or `bytes`) plus the assembly that gives it its address.  The
address travels in the section name (`.data.600002f0`), because `as` always
lays an object's sections end to end; `build_le.py` reads it back.

**2. Code (`tools/gen_asm.py` → `code/asm/`)**
One unit per function, at the function's own address, with `bl`, `b.w`,
literals and calls as *symbols*: `fn_<target>` for code, and the object carries
`R_ARM_THM_CALL` / `R_ARM_THM_JUMP24` / `R_ARM_THM_JUMP19` relocations.  3 074
functions (35 %) assemble straight from their instruction text, producing the
original bytes.  The rest cannot be reproduced that way — see *limitations* —
and are emitted as their exact bytes, still with a global label at every
function entry that falls inside them, so call targets resolve.

**3. Link (`tools/build_le.py`)**
Places every object at its address, resolves the cross references against the
link-wide symbol table and writes the image.  Its Thumb-2 relocation encoding is
checked against GNU as/ld by `tools/selftest_link.py` (4 190/4 190 bytes
identical).

## Limitations — read this before trusting the C

* **The code layer is assembly, not C.** The instruction text is recovered
  (mnemonics and operands for every instruction), but no decompiler ran on the
  code: this is not compilable, readable C for the OS.  `ghidra/` holds the
  headless scripts used to analyse the image (import at `0x60000000`,
  Cortex-M, seed functions, export), and `tools/gen_decomp.py` wraps the
  decompiler — that is the starting point if you want per-function C.
* **A third of the functions come out byte-exact from instruction text; the
  rest are byte-exact as raw bytes.**  The reasons are assembler-encoding
  details, documented in `NOTES.md`: GNU `as` picks a different (equivalent)
  encoding for some instructions (`adds r0, r0, #1` → `adds r0, #1`,
  `ldm.w` without writeback → `ldmia.w`) and widens branches whose target is
  not yet resolved.  Their *addresses and bytes* are exact; their *text* is
  only recovered up to the point where the encoding diverges.
* **The code/data split is heuristic.** It is a linear Thumb-2 sweep with
  re-synchronisation; call-target cross-checks show 0.8 % of calls landing
  inside a decoded instruction (a signal of local drift, `NOTES.md`).
  Everything the sweep calls data is still reproduced byte for byte, so the
  rebuild is unaffected by misclassification — only the presentation is.
* **This is not the updater file.** Rebuilding `SYSTEM8_UP.BIN` additionally
  needs the `App1_Main` container and the encrypted payload; that is
  `../tools/sys8fw.py`.  What this folder proves is that the plaintext OS can
  be expressed as sources that rebuild it exactly.

## Verifying

```bash
make            # end to end
python3 tools/selftest_link.py     # the linker against GNU ld
python3 tools/segment.py           # code/data split
python3 tools/verify.py code/build/os_full.bin ../dumps/os_full.bin
```

Requirements: `arm-none-eabi-as` (binutils-arm-none-eabi) and `capstone`
(5.x, for `tools/segment.py` only; the generated sources and the build do not
need it).
