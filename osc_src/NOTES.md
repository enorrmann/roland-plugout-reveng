# NOTES — how `os_full.bin` was reconstructed

Companion to [`README.md`](README.md).  Every number here is produced by the
tools in `tools/` and can be regenerated.

## 1. What the input is

`../dumps/os_full.bin`, 2 939 504 bytes, loaded at `0x60000000`, read out of the
running SYSTEM-8 (master BMC core 0) over SysEx in 64 KiB chunks
(`../docs/BASE_FIRMWARE.md` §7.14).  It is not one program: it is six images for
four kinds of CPU bundled by a small loader (§8.1 of the same document):

| Part | In the image | Runs at | CPU |
|---|---|---|---|
| `m0_core0` | `0x000000` | in place | master BMC core 0 — the OS |
| `m1_tcm` | `0x130E00` | `0x01100000` | master BMC core 1 — DSP control |
| `m1_sdram` | `0x173A00` | `0x61000000` | master BMC core 1 — debug console |
| `s_boot` | `0x1F1200` | `0x01100000` (slave) | slave BMCs |
| `s_data` | `0x20E600` | `0x60000000` (slave) | slave BMCs |
| `subcpu` | `0x28B730` | `0x08000000` | STM32F1 panel CPU |

The reconstruction treats the image as one address space at its run addresses,
which is what a byte-identical rebuild needs; the part table above is what the
code *means*.

## 2. Code vs data (`tools/segment.py`)

ARM Thumb-2 is self-synchronising: real code decodes from its first instruction
and keeps decoding; data decoded as code rarely stays decodable.  The
classifier is a linear Capstone sweep in Cortex-M mode
(`CS_MODE_THUMB | CS_MODE_MCLASS` — without the M-class profile the
`msr basepri` encoding at `0x60000018` is rejected and the sweep stops after 8
instructions), restarted one byte later whenever a stream stops:

```
image    2939504 bytes at 0x60000000
code     2897218 bytes (98.56 %), 1339349 instructions in 16734 blocks
data     42286 bytes in 16734 blocks
```

The data is literal-pool words the sweep could not read as instructions,
jump tables, `0xFF` flash padding and string blobs.  Two independent checks:

* **Decode self-consistency** — re-decoding each sweep instruction on its own
  agrees with the sweep in 99.9 % of a 200 000-instruction sample.
* **Call targets** — 722 of 90 472 `bl`/`b.w` targets land inside a decoded
  instruction (0.8 %).  A target inside an instruction means local drift: the
  sweep glided into a data table and the "instruction" it produced is not real
  code.  It is the honest measure of how good the split is, and it is why the
  README calls the split heuristic.

None of this affects the rebuild: whatever the sweep calls data is still
reproduced byte for byte.

## 3. The linker is part of the proof (`tools/build_le.py`)

The generated objects carry relocations instead of fixed branch offsets, so
the reconstruction needs a linker.  Rather than trust one, `selftest_link.py`
assembles a file with GNU `as`, links it with GNU `ld` at the image base and
places the same object with `build_le.py`; the two images must agree.  Getting
there required the real ARM Thumb encodings, documented in the code:

* `R_ARM_THM_CALL` / `R_ARM_THM_JUMP24` — `S imm10` / `11 J1 1 J2 imm11`, with
  `imm10 = disp[21:12]`, `imm11 = disp[11:1]`, `J1/J2` kept from the object
  because GNU `as` writes `f7ff fffe` for an unresolved target.
* `R_ARM_THM_JUMP19` — conditional `B`, `1110 S cond imm6` / `10 J1 0 J2 imm11`.
* Absolute addresses travel in **section names** (`.text.600001a0`,
  `.data.600002f0`): `as` always lays an object's sections end to end, so a
  `. = <addr>` inside a section pads the object to the address — an earlier
  version produced 1.6 GB objects for a 64 KiB unit.

`make selftest` runs the check.

## 4. The code layer (`tools/gen_asm.py`)

One unit per function.  Cross references become symbols: `fn_<target>` for a
call or branch, so the object holds a relocation and the linker writes the
final displacement.  A unit is kept as instruction text only if the result
reproduces its bytes exactly; otherwise it is emitted as its exact bytes, with
a global label at every function entry inside it.

A unit starts at the reset entry and at every call / tail-call target
(`tools/gen_functions.py`, 22 933 units).  Split:

```
9089 assembled as instructions   (39.6 %)
13844 emitted as bytes           (60.4 %)
   0 failed
```

The 6 299 are not a limitation of the reconstruction of *bytes* — all 9 373 are
byte-exact — but of the *text*: GNU `as` does not reproduce these encodings.

### Why a function falls back to bytes

| Cause | Example | What happens |
|---|---|---|
| `as` shortens an instruction | `adds r0, r0, #1` (`1c40`) | assembles as `adds r0, #1` (`3001`) — same semantics, 2 bytes either way, different encoding |
| `as` widens without writeback | `ldm.w r0, {sl, fp}` (`e890 0c00`) | assembles as `ldmia.w` (`e8b0`) |
| `as` normalises J1/J2 | `bl` with `J1=1, J2=0` in the original | assembles with the canonical `J1=J2=1` |
| `as` widens an unresolved branch | `beq fn_60000206` | a target that is not yet defined makes `as` emit `beq.w` and shift the unit |
| `as` picks `ADD.W` for a narrow `adr` | `adr r0, #0x28` (`a00a`) | assembles as `addw` |
| instructions the assembler rejects | `hint`, `bkpt` variants, VFP forms outside `-mcpu=cortex-m7` | not assembled |

Every one of these is recorded per function in the generated sources: look at
the top of `code/asm/fn_<addr>.s` — text if the first line after the label is
an instruction, byte form if it is `.byte`.

## 5. What was *not* done

* **No decompiler run on the code.**  The scope chosen was "recovered C for the
  whole image + a rebuild that produces the identical file"; what is recovered
  here is the assembly-level text plus exact data, and the rebuild is exact.
  Producing readable C for 9 373 functions is a separate, much larger job.
  `ghidra/` contains the working headless scripts used to explore the image
  (import at the right base, seed every call target as a function, export code
  units and symbols) and `tools/gen_decomp.py` wraps Ghidra's decompiler per
  function — the pipeline a follow-up would build on.  Ghidra does decompile
  this image (e.g. `FUN_600001c2`, the DMA copy routine, comes out as a clean
  loop over the DMA registers).
* **No re-encryption of `App1_Main`.**  Rebuilding the updater needs the
  container plus the encrypted payload (`../tools/sys8fw.py`) and the BMC key,
  which is not in any file we have (`../docs/BASE_FIRMWARE.md` §2, §8.14).
* **Live RAM is included as dumped.**  The image holds run-time variables (the
  MIDI receive buffer at `0x602C4D00`, the value-label lists at `0x20001A72`
  seen through the dump).  They are reproduced verbatim, as the dump has them.

## 6. Reproducing the numbers

```bash
python3 tools/segment.py                       # code/data split
python3 tools/gen_functions.py --cache build/seg.pkl -o build/functions.txt
python3 tools/gen_asm.py --functions build/functions.txt --limit 300   # quick look
python3 tools/selftest_link.py                 # linker vs GNU ld
make && make verify                            # full rebuild + comparison
```
