# Wavetable space: options (2026-10-20)

Goal: a morph through the frames of a WAV (4–5 s, or 31 frames of 2048 samples) with a smooth LFO sweep.

## Where the memory is

| Memory | Size | Who can read it | Status |
|---|---|---|---|
| Bank 5 of slave DSP2/DSP3 | 1024 words; free 0x048–0x1FD (438), 0x306–0x3FF (~250, unverified) | slave DCO program (`da`/`db` indexed) | works; written by region 1 (`morph_load`, engine.c -DMORPH) |
| Bank 0 of slave DSPs | not measured | `X28Y` indexed | unknown size and use |
| Slave EDM / delay line | 0x200000 words per BMC (cleared by s_boot) | taps on the slave DSP | exists; slave tap window not verified |
| Master EDM, line 0xF7DC | 983 040 words of the delay line | master DSP4 taps | works for DSP-written data; ARM writes blocked |
| Effect slot 0xB000 (DSP4) and delay (DSP4) | effects only | master DSP4 | not reachable from slave DCO; no gain |

## Options

1. **Master EDM written by the ARM.** Blocked: the port write sequence of the driver lands nothing (tested with the
   DSP idle, with the DSP as observer, and with the IDM → DMA path). Not solved.
2. **Bank 5 as it is (438 words).** Works (C-MORPH). 6 frames × 72 points. Not enough for fluid morph.
3. **Bank 5 in two blocks (≈690 words).** Needs a new player that computes the frame base with a jump between
   blocks. Gain ~1.6×. The 0x306–0x3FF block is not verified by audio.
4. **Effect space (master DSP4/DSP5).** Not usable: the slave oscillator cannot read another BMC's memory.
5. **Slave delay line as the table store.** The slave EDM exists, but the slave DSPs have not been checked for a
   tap window that the DCO program can read at a computed address. Needs an observer test on DSP2/3 (by audio).
   This is the only option that would give hundreds of frames inside the DSP, if it works.
6. **ARM-driven frame window (streaming).** Region 1 (the slave ARM) writes bank 5 at memory speed (already done by
   `morph_load`). The WAV lives in region 1; the ARM keeps two frames (k and k+1) in bank 5 and swaps them when the
   LFO crosses a frame boundary. The DSP reads the window with `da` as now. The frame count is limited only by the
   WAV size (region 1 holds ~1 MiB compressed). The LFO position must be read back by the ARM from bank 5 (the DSP
   writes it, coherent read). Latency: about one frame of 72 points per boundary crossing.

## Measured on C-MORPH (2026-10-20)

Recording of one held note (7 s, `re/dspaudio.py` helper): spectral centroid goes low → high → low with a period of
~4 s (triangle at 0.25 Hz), so the sweep runs. Output level is low: RMS 0.0017, peak 0.003, with the RMS dip when the
centroid peaks (possibly the patch filter). Level not compared with other plugouts yet.

## Recommendation

Test option 5 first: an observer test on DSP2/3 (a tap to a known value, read by audio), because it would give the
large table with no new ARM code. If it fails, option 6 is the path that works with the parts already verified.

## Option 5 test on the bench (2026-10-20): inconclusive

Minimal DCO2 program on both slaves: input 0.25 written to `[4300]`, tap age 1 (`0b03 4050`, request `0c02 3a90`,
optional `1d06` base 324), tap output to the voice-A outputs (`0x2221`, `0x221D`). Result: silence in all three
variants, including a control with no tap and a constant 0.25 output. The lab with its own programs is audible
(peak 0.0027), so the problem is the minimal program (it lacks the voice structure the mixer expects), not the delay
line. Option 5 is still open: the test has to start from a DCO2 program that is known to sound (the C-MORPH player)
and add the tap ops to it.

## Streaming: investigation of the three ways to get a periodic ARM update (2026-10-20)

Goal: the ARM of the slaves (region 1) must refill a bank-5 window every LFO cycle while the sound plays.

### 1. OS cyclic timer (μT-Kernel `CEZUtilSTimer` / `CEZTask`)

- The strings show C++ classes `CEZUtilSTimer` and `CEZTask` in the OS image, but the function symbols are all
  `fn_…` (no names), so the methods cannot be identified by name.
- The engine calls the OS through a table at `0x0113FC00` (`OS(off)`). In the master image (`work/os/m1_tcm`) that
  address holds ASCII strings, not function pointers: the table is filled at run time. It cannot be read from the
  static dump. A live dump of `0x0113FC00` (the way `kernel_01000000.bin` was taken) would give the entries.
- No SysTick literal (`0xE000E010`) appears in the images (checked as literal words; the compiler can also build it
  with `movw/movt`, which this check does not see).
- **Status: not found.** Next step: live dump of the slave OS table and of the timer registers, then find which
  entry starts a periodic task (a function that registers a handler and a period).

### 2. Advance the window on each note (event-driven)

- The engine receives every note-on and note-off through `e_param` (`note_pitch`, `note_gate`). Each note-on can
  advance a window pair of the WAV (frames j, j+1) in bank 5 by the ARM.
- Feasible with what is verified: the bank-5 writes from region 1 land (C-MORPH), and `e_param` runs on every note.
- It does not give the LFO sweep (the sweep would be one step per key press). It does give the whole WAV, with
  the DSP reading two frames at a time as in `wt5`.
- **Status: feasible, not built.**

### 3. Region 0 (our own C code, `model/`)

- Region 0 runs the OS side and the parameter model. Its callbacks are in `model/model.c`; none is periodic
  (checked by reading the entry points in the model sources; the stock callbacks at `0x01000400` are not in our
  build).
- **Status: no periodic entry found in our code.**

### Recommendation

Option 2 can be built now and demonstrates the whole-WAV streaming. The LFO sweep needs option 1: take a live dump of
the slave OS table at `0x0113FC00` while a PLUG-OUT runs, and look for the timer entry.
