# `examples/from_scratch/re/` — what each script does and what it needs

This folder is the reverse-engineering lab used to (a) work out what the JX-3P
PLUG-OUT engine and its VST3 actually do and (b) measure the **DSP microcode
ISA of the SYSTEM-8** by running experiments on real hardware.

Two very different halves share the folder, and they have **different
prerequisites**:

| Half | Runs on | Needs |
|---|---|---|
| **Host-side / static** — emulation, decompilation, fitting, conversion | Your computer | Python 3 + `numpy`; `unicorn` / `capstone` / `pefile` for a few; the JX-3P plugin folder |
| **Hardware / lab** — DSP probes, wavetables, audio | A SYSTEM-8 over USB | The **`plugins/C-DSPT` lab plugin installed in slot 3 ("PLG3")**, built from `plugins/JX-3P` |

---

## 0. Required PLUG-OUTs (read this first)

**Stock PLUG-OUT images — only used as data, never installed by these scripts:**

* **JX-3P** — `plugins/JX-3P/` (`Script/Script.xml`, `JX-3P.vst3`) plus a dump
  of its ARM *region 1* (`region1.bin`). Used by the engine/VST half
  (`run_jx`, `emu1`, `measure`, `dsp_progs`, `vst_*`, `jxpatch`).
* **SYSTEM-1 / JUNO-60 / JUPITER-8 / SYSTEM-8 base firmware images** — only
  used as *corpus* for the ISA work (`dsp_corpus.py`, `dsp_corpus_all.py`,
  `dsp_s1.py`). Their blobs are expected in `work/corpus/` at the repo root.

**The lab PLUG-OUT — must be installed on the synth:**

Almost every hardware script talks to the **`C-DSPT`** plugin, which
`dspaudio.py install` / `dsplive.py install` build from `plugins/JX-3P` with
`engine/build.sh`:

```bash
DSPTEST=1 MC_TEST=b11.batch engine/build.sh plugins/JX-3P plugins/C-DSPT
python3 tools/plugout_tool.py install plugins/C-DSPT --slot 3 --yes --force
```

* It is installed **in slot 3** and must then be password-**selected as PLG3**
  on the panel (after an install the device boots back on SYSTEM-8).
* Branches: `DSPTEST` (test programs in the DCO2 slot), `DSPLIVE` (live pokes
  over MIDI) and `PEEK` (region-0 peek/poke transport). `dsplive.py`/`dsppeek.py`
  need a lab built **after 2026-10-17** for the both-DSP and bank-5 commands.
* No other MIDI tool may be holding the port while a script runs.

So, in short: **the only PLUG-OUT these scripts require you to install is the
in-repo lab plugin `C-DSPT` in slot 3 (PLG3)**; the stock JX-3P / JUNO-60 /
JUPITER-8 / SYSTEM-1 files are inputs for the analysis, not something you
install to run the lab.

---

## 1. Engine / VST analysis (needs the JX-3P plugin files)

| Script | What it does | Useful for | Needs |
|---|---|---|---|
| `run_jx.py` | Boots the emulated JX-3P engine in `emu1` and sets patch parameters through the engine vtable `+0x30`. | Shared bootstrap for the engine scripts. | JX-3P `region1.bin`; `IMG` at line 4 is a hardcoded path — edit it. `unicorn`. |
| `emu0.py` | Unicorn ARM/Thumb-M emulator of region 0 (base `0x61800000`). | Emulating the SYSTEM-8 base region. | `unicorn`. |
| `emu1.py` | Unicorn emulator of region 1 (engine), trapping the OS table at `0x0113FC00`. | Engine emulation underneath `run_jx`. | `unicorn`. |
| `jxpatch.py` | Lists the JX-3P patch parameters (`id = 0x6CD + nibble_offset/2`) out of `Script.xml`. | Enumerating parameters. | JX-3P `Script/Script.xml` (default path). |
| `measure.py` | Sweeps 0..255 over every patch parameter and every external get, records all OS-table writes. | Ground-truth reference data for the parameter model. | `engine/build/proj/region1.bin`; `numpy`. |
| `deps.py` | Boots the engine, sweeps parameters and records cross-parameter dependencies → JSON. | Seeing which parameters interact. | Same as `measure`. |
| `draft_model.py` | Turns `measure.py` JSON into expressions for `engine/model_spec.py`. | Auto-drafting the parameter model. | Pure Python + `numpy`. |
| `probe.py` | Helper: writes of one parameter in a given context (engine cached per slave). | Used by `spec_blocks` / `sweepfit`. | JX-3P `region1.bin`. |
| `sweepfit.py` | Sweeps one parameter id in a context and prints fitted curves. | Adding one curve to the model. | Engine image. |
| `spec_blocks.py` | Measures the per-slave / per-state blocks (PART table `0x22c`, `0x737`, effect curves `0x6f9/0x749`). | Regenerating model tables. | Engine image. |
| `knotfit.py` | Least-squares piecewise-linear (optionally log2) knot fit of a measured curve. | Fitting awkward curves. | `numpy`. |
| `fitcurves.py` | Classifies every measured curve (step / const / lin / bip / exp2 / pow / table). | Quick survey of a measurement JSON. | `numpy`. |
| `tdis.py` | Thumb disassembler (capstone) that resolves `ldr rX,[pc,#n]` literals. | Reading the ARM regions. | `capstone`. |
| `r0list.py` | Dumps the region-0 record list (GROUP/PARAM/LABEL entries). | Inspecting a stock PLUG-OUT's region 0. | A `region0.bin` dump. |
| `vst_rtti.py` | MSVC x64 RTTI walk over a VST3 DLL → class → vtables → methods. | Mapping the plugin's classes. Static only. | JX-3P `JX-3P.vst3`; `pefile`. |
| `vst_sim.py` | Extracts `CJx3pSim`'s registered parameter names/offsets and who uses them. | Named reference model for the ISA. | JX-3P `.vst3`; `capstone`. |
| `vst_anchor.py` | Matches constants from an annotated Ghidra `init.c` against instructions in corpus programs. | Locating VST constants inside the microcode. | `init.c` + `work/corpus/JX_*.bin`. |
| `ghidra/Decompile.java` | Ghidra headless post-script: decompiles functions at given addresses. | Decompiling the VST3. | Ghidra 11.3.2 + JX-3P `.vst3`. |
| `ghidra/annotate.py` | Renames parameter/field accesses in decompiled C to `P_<param>` / `st_<off>`. | Making the decompilation readable. | JX-3P `.vst3`. |
| `ghidra/README.md` | How to run Ghidra on the JX-3P VST3 (with the address table). | — | — |

---

## 2. Wavetables and our own oscillators (needs the lab + SYSTEM-8)

| Script | What it does | Useful for | Needs |
|---|---|---|---|
| `wav2dco.py` | Converts a single-cycle WAV into an oscillator program for a DCO slot (FFT-resampled, points as immediates). | Building a custom oscillator. | SYSTEM-8 DSP; only the **DCO2** slot is live-pokable. |
| `wt5.py` | All-in-one: multi-frame WAV → K frames × N points → sent live to **bank 5** (DCO2 slot) and the player left running. `info`/`--dry` need no hardware. | Loading a Serum-style wavetable onto the device. | Lab `C-DSPT` in **slot 3**, **PLG3 selected**. |
| `wt5_diag.py` | Host-side checks of what `wt5.py send` sends (simulator, shadow of `dspown.load`, bank-5 stream, optional `--peek`). | Debugging a wavetable that does not sound. | No hardware (except `--peek`, which needs the lab + `dsppeek`). |
| `wt5_isolate.py` | Hardware A/B/C test isolating program writes (DATA) vs bank-5 writes. | Diagnosing "the DSPs went silent after a bank-5 write". | Lab in slot 3, PLG3. Contains a hardcoded WAV path (line 22). |
| `wtcheck.py` | Plays a scale and compares the measured harmonics of each note with the table's. | Verifying a `wav2dco` program on the device. | SYSTEM-8 + lab + USB audio (`hw:SYSTEM8,0`). |
| `works.sh`, `works_code.sh` | Recorded working invocations of `wt5.py` (the second with `--code`). | Reference commands. | Lab plugin; hardcoded absolute WAV path. |
| `dspasm.py` | Minimal assembler for the established DSP subset + our own SAW oscillator. `saw` / `sim` subcommands. | Writing our own microcode. | Pure Python (the C reference `emu/dspref.py` for verification). |
| `dspflagsaw.py` | SAW variants using flags, C selects, jumps and loops (`sel` / `jmp` / `loop`). | Checking flags and jumps by ear. | Lab + SYSTEM-8 for the audio run. |
| `prog.txt` | Example output of `wav2dco` (a 32-point saw for the DCO2 slot). | Sample data. | — |

---

## 3. Hardware transport and audio bench

| Script | What it does | Needs |
|---|---|---|
| `dspaudio.py` | **The bench.** `install <batch>` builds `plugins/C-DSPT` from `plugins/JX-3P` and installs it **in slot 3**; `recall` records eight notes (one per voice); `rec`/`show`/`report` record and print spectra. Test programs run in the **DCO2 slot of DSP2 (`t0`) / DSP3 (`t1`)**. | SYSTEM-8; `amidi`, `arecord`, `hw:SYSTEM8,0`; `numpy`. This is what builds the lab plugin. |
| `dsplive.py` | Live microcode pokes over MIDI (pitch-bend + CC1), no reinstall needed. `install <batch>`, `poke`, `restore`, `recall`. State in `work/dsplive/state.json`. | Lab built with **`DSPLIVE`**, in slot 3, PLG3; a card named `[SYSTEM8` in `/proc/asound/cards`. |
| `dsppeek.py` | Live peek/poke of **master** memory (incl. master DSPs) through region 0. `ping`, `read`, `write`. | Lab built with **`PEEK`**. Note: slave DSPs are *not* reachable this way. |
| `dspown.py` | Loads our own program into the DCO2 slot of DSP2/DSP3 and measures it by USB audio. `load`, `notes`, `restore`. | Lab + PLG3 (`DSPLIVE`); `OWN_BOTH` / `OWN_FULL` env vars. |
| `dspharm.py` | Records notes and prints f0 plus harmonics 1..8 relative to h1. | SYSTEM-8 + lab + USB audio. |
| `dspmaster.py` | Runs a test program on a **master** DSP and reads its bank 0 (`dsp3` or `efdc` context), restoring the original. | Lab + PLG3 (`PEEK`); `DSPCTX` env var. Replaces the global effect while it runs. |
| `dsprun.py` | Same idea but through the `examples/midi_send` bulk-dump hooks; experiment files define `PROG`/`DATA`/`READ` (or `DATA5`/`READ5`). | SYSTEM-8 + `examples/midi_send/bulk_dump.py`. |
| `dspports.py` | Writes/reads the DSP "ports" (`X9bY` / `Xa9Y`) in the DCO2 slot and listens to the audio. | Lab + PLG3, DCO2 slot. |

---

## 4. DSP ISA — decoders, simulators, arithmetic

| Script | What it does | Notes |
|---|---|---|
| `dspnum.py` | The DSP's internal "RAW" number format (sign, 2-bit exponent, 29-bit mantissa) encode/decode. | Pure Python. |
| `dspmc.py` | Blob bytes → 16-bit words in execution order. | Pure Python, import-only helper. |
| `dspdis.py` | Instruction-boundary disassembler with annotations (`--fields`). | Pure Python. |
| `dspisa.py` | **Archived 2026-10-29** (`archive/dspisa.py`); importing it fails. The reference is `emu/dspref.py` (C). | — |
| `dspbudget.py` | Time budget of an idle master DSP and the cost of A/A/B vs single-op fillers, jumps, loops. | Lab + PLG3 (master DSP3). |
| `dspprobe.py` | Exact register before→after probe of instruction lines. | Lab + PLG3 (master). |
| `dspfit.py` | Least-squares formula for one instruction over D/X/Z/W (degree ≤ 3). | Lab + PLG3. |
| `dspmacfit.py` | Formula over `r0..r7` (degree ≤ 2) for one instruction. | Lab + PLG3. |
| `dspnlfit.py` | Nonlinear exact fit (abs / min / max / sign / step atoms). | `numpy` only, consumes `dspclass --raw`. |
| `dspclass.py` | Measures the most frequent corpus instructions per class and fits them. | Lab + PLG3; `work/corpus`. |
| `dspslots.py` | Maps which body position of a class is A / M / T / reference / illegal (`ab` mode). | Lab + PLG3. |
| `dsptemplates.py` | Infers the class templates from the slots map + corpus → `dsp_templates.txt`. | Lab + PLG3 (via `dspslots` output). |
| `dspvar.py` | Finds operand positions by diffing aligned copies of the same code. | `work/corpus` (no hardware). |
| `dspverify.py` | Verifies instruction semantics exactly against the C reference, master DSP3 flag context (`--inferred` uses `dsp_templates.txt`). | Lab + PLG3 (master DSP3). |
| `dspjump.py` | Verifies `0090 nnnn` = forward jump of nnnn words, and its conditional form. | Lab + PLG3. |
| `dspcond.py`, `dspcondsnap.py` | Snapshot the 16 condition bits of a DSP context, after a prefix instruction. | Lab + PLG3; `DSPCTX`. |
| `dspcreal.py`, `dspcsweep.py` | Which ops of a real instruction run for chosen flag values / for every C-word target byte. | Lab + PLG3. |
| `dspbits.py` | Unit-B `D < 8` bitwise ops (Q24, results modulo 512). | Lab + PLG3. |
| `dspshape.py` | Unit-B phase-shaping candidates (wrap / tri / ph). | Lab + PLG3. |
| `dspexact.py` | Bit-exact RAW arithmetic, compared with exact rational rounding models. | Lab + PLG3 (reads bank 5 of master DSP3). |
| `dspseed.py` | SFU `recip` / `rsqrt` seed tables. | Lab + PLG3. |
| `dspx9b.py`, `dspx9bflags.py`, `dspx9bscan.py` | What `X9bY` does: condition bits, flag groups, where it writes. | Lab + PLG3. |
| `dsphops.py`, `dsphreg.py` | Unit-B nibble-`f` forms and the hidden registers `h4..h7`. | Lab + PLG3 (master DSP4 `0xF7DC` for `dsphreg`). |
| `dspf5.py` | `{8+d}f5Y` returns a time-varying value; samples it and fits a slope. | Lab + PLG3. |

---

## 5. Bus, memory and delay-memory experiments (master DSP3 ↔ DSP4 `0xF7DC`)

| Script | What it does |
|---|---|
| `dsptap.py` | Own delay line through the delay-memory DMA (counter, base, request, destination). Only works on master **DSP4 slot `0xF7DC`**. |
| `dsprdly.py` | Instruments Roland's own delay program at specific halfwords to measure the tap. Needs `work/corpus/m1_tcm_…f7dc….bin`. |
| `dspedm.py` | Reads / scans external delay memory (EDM) through the ARM indirect port. |
| `dspedmloop.py` | Uses the delay line as loop memory and checks tap(a) vs tap(2a). |
| `dspmtap.py` | N delay-tap requests per sample (measured: 24 taps OK). |
| `dspidx.py`, `dspidxring.py` | Indexed transfers (mode `28`, `h6`) against bank-0 markers; is the addressing ring-relative? |
| `dspmodescan.py` | Sweeps transfer mode bytes; maps `da` / `db` / `dN` to `bank5[h6+Y]`, `bank5[h7+Y]`, `bank5[N*16+Y]`. |
| `dspb5alias.py`, `dspb5idx.py`, `dspb5map.py` | Bank-5 aliasing, indexed-store wrap/latency, words beyond `0x3FF` and the time-varying modes. |
| `dspb0fix.py` | Fixed table in bank 0 switched off by a jump (shows one plain bank-0 store wipes it). |
| `dspbus.py`, `dspbus2.py` | Where do bank-3 (IOMD) and bank-4 (MXM) stores land, and how is MXM routed between DSPs. |
| `dspbusclk.py`, `dspbuslat.py`, `dspbussync.py` | MXM/IOMD latency and latching, using a shared clock or a sample counter. |
| `dspmxm.py`, `dspmxsum.py` | MXM routing in both directions; is MXM a mixing bus? |
| `dspiomd.py` | Does IOMD (bank 3) link the master DSPs, or only go out? |
| `dspb0dly.py` | **Slave** test: `(saw + saw delayed k)/2` via indexed bank-0 delay. DCO2 slot; load with `dspown.py`, listen with `dspharm.py`. |
| `dspb5osc.py` | **Slave** oscillator reading a bank-5 table through `0daY`. DCO2 slot, same workflow. |

---

## 6. Corpus extraction and JX-3P program dumping

| Script | What it does | Needs |
|---|---|---|
| `dsp_corpus.py` | Finds DSP program blobs in PLUG-OUT images via the descriptor table and writes them to `work/corpus/`. | Stock PLUG-OUT **images only** (SYSTEM-8 / JUNO-60 / JX-3P family), no install. |
| `dsp_corpus_all.py` | Same plus a signature scan; reports which programs are new. | Same + an existing `work/corpus`. |
| `dsp_progs.py` | Runs the JX-3P engine's DSP-load vtable call for every DCO1/DCO2 waveform and effect type, dumping the programs it hands to the slave. | **JX-3P** `region1.bin`; `unicorn`. |
| `dsp_s1.py` | Extracts DSP programs from **SYSTEM-1** images (which have no descriptor table). | SYSTEM-1 images only. |
| `dsp_wdiff.py` | Aligns two word streams and classifies the differing words (operands vs operations). | Corpus blobs. |
| `dspexp/*.py` | Master-DSP experiment files (`PROG` / `DATA` / `READ`) run by `dsprun.py`. | SYSTEM-8 + the `examples/midi_send` hooks. |
| `dspexp/audio/live/mk.py`, `probe.py`, `probe3.py` | Builders for single-path audio test programs (constant and 3-operand probes). | Lab + PLG3 for the audio run. |
| `dspexp/audio/live/runq.sh` | Pokes four probe files at once, records, prints `gain-1` per file for both slaves. Calls `re/dsplive.py`, so run it from the repo root. | Lab in slot 3, PLG3. |

---

## 7. Non-script files

| Path | What it is |
|---|---|
| `dsp_templates.txt` | Measured/inferred class → template table (consumed by `emu/dspdec.h`, `dspverify.py --inferred`). |
| `dsp_units_ab.txt` | Unit A vs B position map per class (output of `dspslots.py ab`). |
| `dspexp/audio/*.batch` | Batches mapping test names to programs (`b11.batch` is the voice/gain calibration used by `dspaudio.py report`). |
| `dspexp/audio/*.txt`, `dspexp/audio/live/*.txt` | The 16-bit test programs and probe builders themselves. |
| `dspexp/master/*.txt`, `dspexp/own/*.txt`, `wt/`, `wt2/`, `lab_pad*.txt` | Master-DSP tests, own oscillators and generated wavetable programs. |

---

## 8. External tools some scripts call

* `tools/plugout_tool.py` — installing the lab plugin (`--slot 3 --yes --force`).
* `examples/../engine/build.sh` — building `plugins/C-DSPT` from `plugins/JX-3P`
  (`DSPTEST`, `DSPLIVE`, `PEEK`, `MC_TEST`).
* `examples/midi_send/bulk_dump.py` — the region-0 hook used by `dsprun.py`.
* `amidi` / `arecord` (alsa-utils) and a SYSTEM-8 USB audio device named
  `hw:SYSTEM8,0`.
* Ghidra 11.3.2 for the `ghidra/` scripts.

Scripts write their outputs to `work/…` (and `dumps/`) at the repo root, so run
them from the repository root.

## Documentation checks

| command | what |
|---|---|
| `python3 tools/doccheck.py FILE.md…` | assembles every fenced `dsp` code block of the technical documents (WAVE_BUS, VOICE_ALLOCATION of both devices); exit 1 if one fails |
