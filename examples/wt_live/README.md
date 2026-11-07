# C-WTLIVE — live wavetable PLUG-OUT

A dedicated PLUG-OUT that does one thing: receive a WAV wavetable over MIDI and play it on the
SYSTEM-8 slaves. It is separate from the lab plugin `C-DSPT` (`examples/from_scratch/re/dspaudio.py`):
its own build, its own slot content, its own host state, so the lab's ISA tests and this one never
mix.

| Part | Where |
|---|---|
| Driver (build, install, send, restore) | `wtlive.py` (this folder) |
| Build recipe: from-scratch engine with `DSPTEST` + `DSPLIVE` | `build.sh` → `plugins/C-WTLIVE` |
| Test programs of the build (calibration `cal_d2`/`cal_d3`, DCO2 audible) | `build.sh` (paths in `examples/from_scratch/re/dspexp/audio/`) |
| Unused silent test program (kept for reference) | `mute.txt` |
| WAV → table → programs, table transfer, player | `examples/from_scratch/re/wt5.py` (used as is) |
| MIDI protocol (pitch bend + CC1) | `examples/from_scratch/re/dsplive.py` (used as is) |
| Program load over MIDI (empty program first, then the body) | `examples/from_scratch/re/dspown.py` (used as is) |
| Host state and outputs | `work/wtlive/` (the lab uses `work/dsplive`, `work/dspaudio`, `work/wt5`) |

## Commands

Requirements: the SYSTEM-8 on USB (ALSA card `[SYSTEM8`), `arm-none-eabi-gcc`, Python 3 with `numpy`.
Run from `examples/wt_live/`. No other MIDI tool may hold the device while a command runs.

### Install the test waves

```sh
python3 wtlive.py build        # builds plugins/C-WTLIVE (about a minute)
python3 wtlive.py install      # installs it in slot 3 (replaces the PLUG-OUT there), resets the host state
```

After the install the device boots back to SYSTEM-8. Select **PLG3** on the panel (with its password),
as with any PLUG-OUT. Then send a table:

```sh
python3 wtlive.py send '/home/emilio/PRODUCCION/samples/THE_REST/wavetables/Echo Sound Works Core Tables/Basics/PWM Saw 01.wav'
```

Other test waves in the same folder work the same way, for example `Saw Dual 01.wav`,
`Sine to Saw.wav`, `Pulse Acid.wav`, `Sine PWM.wav`.

Options (the same as `wt5.py`):

```sh
python3 wtlive.py info  TABLE.wav                         # frames in the file, what would be used (no hardware)
python3 wtlive.py send  TABLE.wav                         # one frame, up to 437 points, on OSC2 (DCO2)
python3 wtlive.py send  TABLE.wav --osc 1                 # the table on OSC1 (DCO1 slot) instead
python3 wtlive.py send  TABLE.wav --osc both              # the same table on OSC1 and OSC2
python3 wtlive.py send  TABLE.wav --frames 2 --frame 1    # two frames of 218 points, play the second
python3 wtlive.py send  TABLE.wav --frames 8 --frame 3.5  # eight frames of 53, fixed morph between 3 and 4
python3 wtlive.py send  TABLE.wav --frames 8 --scan 0.25  # LFO scans the frames (not verified by ear yet)
python3 wtlive.py send  TABLE.wav --dry                  # only write the programs to work/wtlive/ (no hardware)
python3 wtlive.py restore                                 # back to the silent test program
```

Each `send` takes about 12 s for a single frame (table 6.8 s, player 4–5 s).

## How it works

1. **Host, `wt5.py`.** The WAV is read (a Serum-style `clm ` table or a single cycle, PCM or float).
   Frames are picked evenly over the file, FFT-resampled to N points (band-limited), DC removed,
   and scaled to a common peak of 0.9. The table is laid out in bank 5 at words `0x048…0x1FD`: frame k
   at `0x048 + k*(N+1)`, N points plus a copy of point 0 for the wrap. `K*(N+1) <= 438`.
2. **Transport, `dsplive.py`.** The host sends MIDI on channel 1 through the stock region 0, which
   forwards it to the slaves as parameters:
   - pitch bend carries 14 data bits (voice id `0x465`);
   - CC1 carries the command `seq<<6 | cmd<<4 | ext` (voice id `0x467`); bit 6 toggles on every
     command, and the engine acts when it changes.

   Commands: `ADDR` (where to write: DSP2 or DSP3, a program word index or bank 5, and with ext bit 8
   the OSC1 slot `0x6000` instead of OSC2 `0xB000`), `DATA` (one word or one 18-bit value), `CTRL`
   (restore the built programs of both oscillators, or nothing).
3. **Table as data.** `Link.write_b5` sends one `DATA` command per point, 18-bit fixed point
   (2^-17 steps), to bank 5 of DSP2 and DSP3 at once. The C-WTLIVE engine converts each value to the
   DSP's internal RAW format and stores it in bank 5 itself. About 15 ms per command, so 438 points
   take about 6.7 s.
4. **Player, `wt5.py` / `dspown.py`.** A player of about 100 instructions (275 words) is written into
   the slot of the chosen oscillator on DSP2 and DSP3: DCO2 (`--osc 2`, default), DCO1 (`--osc 1`)
   or both. It runs per voice: SAW phase → table position, four bank-5 loads, linear interpolation
   inside the frame and between frames. The load is done as an empty program first, then the body
   (tail first, head last), so the DSP never runs a mix of old and new code. Every program word is
   sent on every `send` (`OWN_FULL=1`), so no host state decides what the device has.
5. **Engine, `examples/from_scratch/engine` built with `DSPTEST DSPLIVE`.** The calibration programs
   `cal_d2`/`cal_d3` are the test programs t0/t1 of the build (DCO2 slot of DSP2/DSP3, audible). The
   DCO1 slot keeps its own saw. `DSPLIVE` needs `DSPTEST`.
6. **Notes and octave.** The sounding pitch of each note is set on the panel (the octave control).
   With the calibration build a note 60 sounds at 130.8 Hz, one octave below the MIDI note; that is
   the panel setting, not the table or the engine.
7. **Restrictions of OSC1.** The DCO1 waveform is switched by the engine when the panel selects another
   DCO1 wave (SAW, SQR, FOLD). That switch writes the DCO1 slot again and replaces the table: keep the
   DCO1 wave on SAW when using `--osc 1` or `both`.

## Indexed reads: the same mechanism on both machines (2026-10-29)

The table is read with the bank-5 indexed transfers (`0da0` etc.: bank 5 at `h6 · 16384 + Y`, h6 / h7 set with
`hset`), the same family of transfers that SYSTEM-S1CC now uses on the **SYSTEM-1** (bank-0 `X28Y` + `hset`, COLOR MOD
with 8 voices, MP-CSAW2-8; docs/DSP_HARDWARE.md §7.4). Measured bit-exact on the SYSTEM-8 master the same day
(`re/dspverify.py --programs`). Room for tables: bank 5 has 1024 words per DSP; on the SYSTEM-8 the free part is
0x048–0x1FF and 0x306–0x3FF (this table uses 0x048–0x1FD, 438 points); on the SYSTEM-1 slave A and B share one bank 5
and only about 200 (ARM-written) + 160 (DSP-written) words are left: single-cycle tables of 64–128 points. Long samples
need the delay memory (EDM, SYSTEM-8 master DSP4 only; docs/DSP_HARDWARE.md §8.4).

## Keeping it apart from the lab

- Slot 3 is the only PLUG-OUT slot the lab uses, so installing C-WTLIVE replaces the lab plugin there
  (`C-DSPT`, or whatever was in slot 3). To use the lab again, reinstall it with
  `examples/from_scratch/re/dspaudio.py install …`.
- `wtlive.py` points `dsplive`, `dspown` and `wt5` at `work/wtlive/` for their state and outputs.
  The lab's `work/dsplive/state.json` is not read or written.
- `build.sh` writes the engine objects into `examples/from_scratch/engine/build/`, which the lab also
  uses. Rebuilding one overwrites the intermediate files of the other, but each plugin folder
  (`plugins/C-WTLIVE`, `plugins/C-DSPT`) is written separately.

## Status (2026-10-20)

- `build`, `install` (slot 3, reads back as `C-WTLIVE`), and
  `send '…/Basics/PWM Saw 01.wav'` run to the end on the device: table 438 words in 6.8 s, player
  275 words loaded into DSP2/DSP3, DCO2 slot of both slaves.
- 2026-10-20: a clean build from the engine source (calibration programs) is installed in slot 3.
  With PLG3 selected, OSC1 plays the standard saw and OSC2 plays the table sent with `wtlive.py send`.
- 2026-10-20: `--osc 1 / 2 / both` and the engine bit (ext 8) were added after that build. Built and
  installed; the OSC1 and `both` options are not verified by ear yet.
- Known gap: `wt5.py`'s `--scan` (LFO over the frames) is not verified by ear.
