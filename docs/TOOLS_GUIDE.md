# Tools: use and parameters

This is the hand-written part of [TOOLS.md](TOOLS.md) (translated from Spanish on 2026-11-10). It contains the
workflows and the tools of daily use with all their parameters. Below it, `tools/tools_usage.py` generates the
reference of **all** the tools of the repository (docstring and options of each script). After adding or changing a
tool:

```sh
python3 tools/tools_usage.py            # regenerates docs/TOOLS.md (this guide + the reference)
python3 tools/tools_usage.py --check    # exit code 1 if docs/TOOLS.md is out of date
```

State: 2026-10-23. Chapters of the documentation suite: [index.md](index.md).

## 1. Flow of an own PLUG-OUT (SYSTEM-1, C-S1SY)

```sh
STEP=11 python3 examples/system_1_synth/make_synth.py --sim          # 1. test in the emulator (≈ 1.4 s)
STEP=11 python3 examples/system_1_synth/make_synth.py --pack-check   #    + bit-exact optimization (≈ 3 s)
STEP=11 NAME=C-S1SY11 sh examples/system_1_synth/build.sh           # 2. build (+ check_optimized)
amidi -l                                                             # 3. is the device there? (SYSTEM-1 MIDI 1/2)
python3 tools/plugout_tool.py install plugins/C-S1SY11 --slot 1 --force --no-patches -y   # 4. install
```

`--sim-check` (≈ 25 s, with the Python reference) is **not** part of the daily flow. It is only run when something
changes that the reference validates and `--pack-check` does not:
* the emulator's core (`examples/from_scratch/emu/`: `dspcore.*`, `dspjit.py`, `dspcomp.py`, `cchain.py`, `chain.py`);
* `simulate()` / `simulate_py()` or the memory model of the simulation;
* an instruction form that our programs did not use (a new op or mode).

To test waves, filters and modulations `simulate()` and own measurements on the C core are used, which runs ≈ 3.5×
faster than real time.

After installing, the PLUG-OUT has to be selected on the panel. Tools that talk MIDI with the device: never two at
once.

## 2. `examples/system_1_synth/make_synth.py`: programs, simulation and validation

```sh
python3 examples/system_1_synth/make_synth.py                # s1*.s / s1*.hex, build/s1_programs.h
python3 examples/system_1_synth/make_synth.py --sim          # also: three sound tests in the emulator
python3 examples/system_1_synth/make_synth.py --pack-check   # also: packing / fusion bit for bit, C only (≈ 3 s)
python3 examples/system_1_synth/make_synth.py --sim-check    # also: C core against the Python reference (§2.2, ≈ 25 s)
```

### 2.1 Environment variables

| variable | default | what it does |
|---|---|---|
| `STEP` | 8 | incremental step: 1–11 (list in `sh examples/system_1_synth/build.sh --help`); 0 = the first design |
| `STAGE` | 4 | only with `STEP=0` |
| `VOICES` | 8 | voices |
| `PACK` | 1 | packs `ld`/`st` into the instructions of the ops (`pack()`); 0 = one unit per instruction |
| `SCHED` | 1 from `STEP=12`, otherwise 0 | scheduler `asm/dspsched.py` instead of `pack()` |
| `FUSE` | 1 from `STEP=11`, otherwise 0 | fuses independent ops in one instruction (`op1 \| op2`) |
| `RESCOMP` | 0 | resonance compensated by the cutoff (self-oscillation) |
| `RES_MAX` | 3.6 (4.4 with `RESCOMP=1`) | filter feedback with the resonance at maximum |
| `GMAX` | 0.7 | cap of the filter coefficient |
| `PAD` | 0 | budget test: n fillers at the end of DSP A |
| `SIM` | `c` | `py`: `--sim` with the flat-memory machine (`emu/dspref.Machine`, slower) instead of the slave of `cchain` (1 s of audio ≈ 0.4 s) |
| `SIM_JOBS` | cores of the machine | independent simulations of `--sim` / `--sim-check` at once, one per process; 1 = serial |
| `DSP_CC` | `clang` if present, otherwise `cc` | compiler of the C core and of the native code (`gcc`, `clang` or a path) |
| `SIM_N` | 9600 | samples of the A 440 test of `--sim` |
| `SIM_CHECK_N` | 600 | samples of the C core / Python comparison of `--sim-check` |
| `SIM_PACK_N` | 19200 | samples of the packed / unpacked comparison of `--sim-check` |

### 2.2 `--sim-check` and `--pack-check`

`--pack-check` only does comparison 2 (without the Python reference). `--sim-check` does both. Two comparisons, sample
by sample. Each one says `identical` or the number of different samples, the first different sample and the two
values:

1. **`sim check`: the slave of the C core against the flat-memory machine.** It runs `simulate()` (slave of
   `examples/from_scratch/emu/cchain.py`) and `simulate_py()` (`emu/dspref.Machine`, one DSP per program, rings moved by
   hand) with the same programs and events: it checks the slave's model of banks and rings: A 440 on voice 0 at t = 0,
   E on voice 1 at t = 50, voice 0 released halfway. It lasts `SIM_CHECK_N` samples.
2. **`pack check`: packed / fused programs against the same unpacked** (`sources(True)` against `sources(False)`), in
   the C core. 8 notes, half of them released halfway, `SIM_PACK_N` samples. It validates `pack()` and `FUSE`: the
   optimization cannot change a single bit.

Panels tested in both comparisons:

| panel | what it enables |
|---|---|
| `defaults` | saw + saw/2, filter open |
| `filter + envelopes` | square + triangle, cutoff 120, resonance 200, both ADSRs, key track |
| `LFO sine` / `LFO S&G` / `LFO S&H` (step ≥ 11) | the previous one + LFO with all the depths; at step 9: `LFO triangle` / `LFO S&H` |

A difference in `sim check` is an error of the C core, or of a semantics that differs between the two simulators. A
difference in `pack check` is an error of `pack()`/`FUSE`, or a parameter off the registers' grid (`on_grid()`,
[microcode optimization](vol3_voice_allocation_scheduling/microcode_optimization.md) rule 6).

### 2.3 `--sim`

| line | what it measures |
|---|---|
| `A 440 …: peak, f` | one note during `SIM_N` samples: peak of the master's output (`A118`) and frequency by zero crossings |
| `8 notes: peak` | 8 notes at once |
| `no notes: peak` | 2000 samples without notes (must be 0) |

## 3. `examples/system_1_synth/build.sh`

```sh
sh examples/system_1_synth/build.sh --help       # all the options
```

It takes the variables of §2.1 plus `NAME` (name of the PLUG-OUT and folder `plugins/<NAME>`, at most 16
characters), `MODEL_ID` and `PARAMLOG=1` (region 0 sends each parameter to the PC). At the end it runs
`tools/check_optimized.py` on the programs.

## 4. `tools/check_optimized.py`

```sh
python3 tools/check_optimized.py FILE...            # .s, .hex or .txt; warnings per file
python3 tools/check_optimized.py -q FILE...         # one line per file
python3 tools/check_optimized.py --strict FILE...   # exit code 1 if there are warnings
```

Warnings: `manual-wrap`, `filler`, `transfer-only` and `fusable`. They are explained in
[microcode optimization](vol3_voice_allocation_scheduling/microcode_optimization.md) §2.

Many SYSTEM-1 `fusable` are groups of ops that are not in `asm/proven_forms.txt` (rule R8). To test them on the device
and add them to the list: `make_s1cc.py --forms`, `TRY_FORMS=...` and `tools/proven_forms.py promote`
([examples/system_s1cc/README.md](../examples/system_s1cc/README.md) §4.1).

```sh
python3 tools/proven_forms.py list                                         # the proven groups
python3 tools/proven_forms.py promote BUILD/try_forms.txt --by PLUGOUT     # add the ones that ran on the device
```

## 5. Device

| tool | use |
|---|---|
| `tools/plugout_tool.py ports` / `info` | MIDI port and state of the slots |
| `tools/plugout_tool.py install <plugin> --slot N [--force] [--no-patches] [-y]` | installs `Code*.Dat` (+ patches); `--force` without checking the version, `--no-patches` only the firmware |
| `tools/plugout_tool.py install --code1 FILE …` | installs that file as `Script/Code1.Dat` (SYSTEM-1); `--dat` is the `Code*.Dat` chosen by hand |
| `python3 tools/plugout.py [--json] FILE.Dat` | summary of a container; `--json`: header, START / END, records and regions (used by `web/test/test-protocol.js`) |
| `python3 -m http.server 8000 --bind 127.0.0.1 --directory web` | web installer at `http://localhost:8000/` (Chrome / Edge); [web installer](vol4_firmware_container_protocol/web_installer.md) |
| `node web/test/test-protocol.js [FILE.Dat...]` / `sh web/test/run-page-test.sh` | tests of the web installer without hardware: library against `plugout.py`, whole page in headless Chrome |
| `examples/system_1/paramlog.py -t SEC [--raw] [--file LOG...]` | listens to a `PARAMLOG=1` build: ids of the panel's controls (with the PLUG-OUT selected) |
| `examples/system_1_ramp/ramp.py -t SEC [--log F] [--file F]` | listens to C-S1RAMP: times of the `FCE4` ramps (`F0 7D 07`) and the end-of-ramp chain (`F0 7D 08`) |
| `python3 examples/from_scratch/re/dsplive.py install l00.batch` | SYSTEM-8: installs in slot 3 the lab with the master's probe (`PEEK`); select PLG3. `python3 examples/from_scratch/re/dsppeek.py ping` answers `pong 0x1234` when it is active |
| `python3 examples/from_scratch/re/dspnative.py [TEST...] [-n 24] [--list] [--sim]` | SYSTEM-8 with the probe: native ops (`shl`, SFU, `newton`, `add3`, `sum8`, two-product MAC) on the master's DSP3, exact against the C reference (`emu/dspref.py`), with and without an empty instruction before reading; `--sim` only the emulator ([s1cc](vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md) §7.6) |
| `KERNEL=sdk/s1cc/build/native_check NAME=NATIVE-CHECK sh examples/system_s1cc/build.sh` | SYSTEM-1: PLUG-OUT that tests by ear the native ops that s1cc uses (clean saw = fine; OSC1 COLOR chooses the op; [s1cc](vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md) §7.6) |

## 6. Emulator (`examples/from_scratch/emu/`)

| tool | use |
|---|---|
| `cchain.Slave(programs, model, native=True, threads=True)` | one BMC: up to 6 DSPs; `step(n)`, `run(n, words)`, `d[n].bank5`, `params`, `iomd`; see the docstring of `cchain.py` |
| `python3 examples/from_scratch/emu/test_core.py [name...]` | tests of the C core: corpus against the local golden file, native against interpreter, threads, `quant` (all or those containing the name; `--update-golden` only after a measured change) |
| `python3 examples/from_scratch/emu/test_chain.py` | tests of the model of the SYSTEM-8 chain |

Speed and state: [emulator](vol5_toolchain_compiler_sdk/emulator.md) §3.6.

## 7. Documentation

| tool | use |
|---|---|
| `python3 tools/doccheck.py FILE.md...` | all the ```` ```dsp ```` blocks of the documents must assemble |
| `python3 tools/tools_usage.py [--check]` | regenerates / checks this document |

## 8. `sdk/mport`: porting Mutable Instruments components

Specification: [mport](vol5_toolchain_compiler_sdk/mport_mutable_sdk.md). Example that uses it: `examples/mutable_system`
([examples/mutable_system/README.md](../examples/mutable_system/README.md)).

```sh
python3 sdk/mport/cli.py list                                     # ported components
python3 sdk/mport/cli.py check saw_swarm                          # port vs original C++ (≈ 1.4 s; builds the reference the 1st time)
python3 sdk/mport/cli.py lines saw_swarm [--plain]                # microcode of the bench
python3 sdk/mport/cli.py lut braids ws_moderate_overdrive --odd 9 --xmax 0.875   # table -> polynomial, error in LSB
python3 -m unittest discover -s sdk/mport/tests                   # tests (≈ 1.2 s)
python3 examples/mutable_system/make_mutable.py --sim --pack-check
sh examples/mutable_system/build.sh                               # -> plugins/MUTABLE-SYSTEM
```

`EURORACK` = path of the Mutable checkout (by default `~/git/eurorack`); the reference needs g++.

## 9. `sdk/s1cc`: compiling a voice in C / C++ to microcode

Specification: [s1cc](vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md).

```sh
make -C sdk/s1cc                                                   # the compiler (LLVM 19)
sdk/s1cc/s1cc build VOICE.cc [MORE.cc ...] [-I DIR] [-o OUTPUT] [--tol X] [--tollog X] [--maxdeg N] [--regs LIST]
                                                                   # MORE.cc: only those that are not neighbours of an included header (--no-auto)
sdk/s1cc/s1cc build VOICE.cc ... --debug                           # which C functions it does not compile and why (s1cc §9.1)
sdk/s1cc/s1cc build VOICE.cc ... --check                           # build + check (instructions, SNR)
sdk/s1cc/s1cc check OUTPUT [--note N] [--n SAMPLES] [--all-forms] [name=knob ...]
KERNEL=OUTPUT NAME=MUTABLE-S1CC sh examples/mutable_system/build.sh
KERNEL=OUTPUT sh examples/system_s1cc/build.sh                     # SYSTEM-S1CC: only the kernel (examples/system_s1cc/README.md)
python3 -m unittest discover -s sdk/s1cc/tests                     # ≈ 8 s
```
