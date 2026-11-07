# SYSTEM-8 as the base of the lab PLUG-OUT (plan, 2026-10-20)

Idea: instead of the JX-3P image (what `examples/from_scratch` builds on), start from the SYSTEM-8 synth itself,
which has more options (for example COLOR, which modifies OSC1 from a MOD source: LFO, ENV, ...), and install that
version in slot 3 with a WAV oscillator to keep testing from there.

## What the firmware provides (`firmware/sys8_sys_v132/SYSTEM8_UP.BIN`, `tools/sys8fw.py info`)

| File | Region | Load | Size (compressed -> expanded) | Identity |
|---|---|---|---|---|
| `p1y01_p0m.bin` | 0 (master, OS side) | `0x60800000` | 149 902 -> 1 166 152 | SYSTEM-8 id `0x0005`, v1.20 |
| `p1y01_p0s.bin` | 1 (slaves) | `0x60200000` | 314 736 -> 536 308 | SYSTEM-8 id `0x000A`, v0.01 |

The JX-3P that the lab uses is built from `plugins/JX-3P`, whose `Script/Code8_*.Dat` hold its region 0/1 and
DSP microcode (`engine/programs.py`, SHA-256 checked). The same kind of container exists for SYSTEM-8 (`p0m`, `p0s`).

## What is needed, in order

1. **Baseline install (no changes).** Build a plugin folder from `p0m`/`p0s` (same `Script/Code8_*.Dat` layout as
   `plugins/JX-3P`) and install it in slot 3. Unknown: whether the device accepts a second copy of the SYSTEM-8
   identity. Risk: low (the regions are the stock slot-0 ones), but it needs one test on the device.
2. **Microcode and parameters.** The SYSTEM-8 DSP microcode differs from the JX-3P one; `programs.py` (SHA-256
   list) and `model_spec.py` (JX-3P parameter model, generated from region 0) do not apply. Either we keep Roland's
   region 0 and patch it (as `JX-3P CUSTOM` was patched), or we rebuild the model from `p0m`.
3. **WAV oscillator.** Needs the streaming design of `docs/OPTIONS.md` (option 1 or 2) inside
   region 1 (`p0s`), which is Roland's code: adding our own code there means patching a larger engine than the
   JX-3P one we use now.

## Assessment

- Step 1 is a short test and tells whether the base is viable.
- Steps 2 and 3 are the bulk: the JX-3P lab took many sessions to reach its current state; the SYSTEM-8 one
  would start from a bigger and different engine (1.1 MB region 0, 536 KB region 1).
- The COLOR/MOD feature is in Roland's code and works without us: if the goal is only to test the modulation of
  OSC1 by LFO/ENV, the stock SYSTEM-8 may already provide it, and the WAV oscillator is the only new piece.

## Next

Step 1 needs your go-ahead (it installs a second copy of the SYSTEM-8 in slot 3 and replaces the current lab).

## Step 1 result: COLOR and MOD in the OS strings (2026-10-20)

Source: `work/os/strings.txt` (decoded OS, `App1_Main`). Labels found, no numeric ids yet:

| Label | Address (OS string table) | Note |
|---|---|---|
| `OSC1_COLOR`, `OSC2_COLOR`, `OSC3_COLOR` | `0x601298C0`, `0x601298CC`, `0x601298D8` | parameter names |
| `COLOR` (panel) | `0x6012E80E` | front-panel label |
| `LED OSC1 COLOR` | `0x6012E814` | LED label |
| `(MOD ID 1..3)`, `(MOD PRM 1..3)` | `0x6012BCF4..0x6012BFA8` | modulation matrix slots: source id and amount |
| `MOD ASIGN` | `0x6012C674` | assign table for the matrix |
| `MOD SENS PITCH`, `MOD SENS FILTER` | `0x6012EEC14`, `0x6012EF37C` | sensitivities |
| `OSC1 X-MOD IN` | `0x6012E404`, `0x6012EEEBC` | cross-mod input of OSC1 |

Still open (step 2): the numeric parameter ids of these labels in region 0 (`p0m`), the mod source list
(LFO, ENV, ...), and the DSP program that implements COLOR. The strings do not contain the ids.

## Step 2 (partial): where the parameter names are (2026-10-20)

- Name pointers: the OS holds the names as an array of 303 pointers to the label strings, starting at
  `0x602AC5E4` (`dumps/os_full.bin`, base `0x60000000`). OSC1_COLOR is at index 125, OSC2_COLOR at 132,
  OSC3_COLOR at 137. The array is NOT indexed by parameter id: index 7 is OCT_UP, not DCO1 WAVE (0x6D4).
- Before the array, records that look like parameter definitions (ranges and defaults: 0x64, 0x4B, 0x1E, ...),
  starting around `0x602AC5B4`. Their size and the field that holds the id are not known yet.
- Not found: the numeric id of OSC1_COLOR and the modulation source list. Without them the COLOR parameter cannot
  be written or tested from our code.
- Next: parse the records before the array (find the stride from the repeated fields) and check the candidate id
  against a known parameter (DCO1 WAVE = 0x6D4, DCO2 WAVE = 0x6D9, DELAY TIME = 0x6CE from the JX-3P model).

## Step 2 result: OSC1 COLOR id and writes (2026-10-20)

- **COLOR = id 0x6D5.** Evidence: the JX-3P-MIDI-SEND `Script.xml` lists the OSC1 struct as DCO1 WAVEFORM,
  (OSC1 COLOR), DCO1 FREQ MOD, DCO1 RANGE, (OSC1 CROSS MOD); the model in `examples/from_scratch/model/params.py`
  has DCO1 Waveform at 0x6D4 and DCO2 Waveform at 0x6D9 (the struct starts there), and Freq Mod at 0x6D6, which is
  exactly where the struct order puts it.
- **Patch layout checked on the device:** RQ1 of patch 0, slot 1, OSC1 leaf (10 bytes): WAVE 00 01, COLOR 05 0D
  (= 93), FREQ MOD 00 01, RANGE 00 03 (= the Script default 3), CROSS MOD 00 00.
- **Writes work:** DT1 to `patch_address(1, 0, 0) + 14 + 2` with two nibble bytes (hi, lo) changes COLOR; restored
  afterwards (`examples/sys8_lab/color_test.py`).
- **Audio:** one held note per value, 3 s: centroid 4677 Hz (93), 4737 (0), 4967 (64), 4995 (128), 4707 (192),
  4786 (255). Control: DCO1 WAVE 0 / 3 / 5 gives centroids 4276 / 4987 / 4350 Hz, so the measurement sees timbre
  changes of the same size. COLOR changes the timbre but not monotonically (single-note measurement, noisy).
- **Open (step 3):** the modulation matrix. `MOD ID 1..3` (0..255) and `MOD PRM 1..3` are in the EXTEND struct; the
  meaning of the MOD ID values (LFO, ENV, ...) and the target of each slot (MOD ASIGN) are not in the JX-3P script or
  in the OS string table. Next: find the source enumeration in region 0 or test MOD ID values against audio.

## Step 3 results: modulation paths of SYSTEM-8 (2026-10-20)

Source: the new `plugins/SYSTEM-8/Script/Script.xml` (the SYSTEM-8's own script, with the real names).

- **COLOR confirmed in the SYSTEM-8 script:** the OSC1 struct is `OSC1 WAVE, OSC1 COLOR, OSC1 MOD, OSC1 RANGE,
  OSC1 CROSS MOD` (offsets 14, 16, 18, 20, 22 in the patch). COLOR (`0x6D5`) changes the timbre live (`color_test.py`).
- **Patch cords** are NOT the `EXTEND` mod matrix. They are in `PAT_NAME1` (patch offset 256): each destination is an
  `int8x4` (8 bytes, value 0..13). Sources are the `matrixOut` list (CV OUT, GATE OUT, LFO OUT, LFO A/B OUT, P/F/A ENV
  OUT, OSC1/OSC1 SYNC/OSC2/OSC3 OUT, MIX OUT); destinations are `matrixIn` (LFO CV/RING/PITCH/FILTER/AMP IN, OSC1
  X-MOD IN, OSC2 SYNC/RING IN, OSC3 EXT IN, P/F/A ENV IN). `OSC1 X-MOD IN` is at patch offset 360.
  - The device accepts the write (read-back shows the value).
  - No audible effect: all values 0..13 with `OSC1 CROSS MOD = 255` give the same spectral spread (`cords_test.py`).
  - `OSC1 CROSS MOD` itself does act live: 255 makes the sound much noisier (~5 kHz spread) with the cord off.
- **EXTEND `MOD ID` / `MOD PRM`:** scans 0..11 with amount 255 show no audible change; `MOD ID` = 0xD5 (COLOR's
  low byte) with amount 255 shows no change either. Its meaning is still open.
- **Reload:** a program change (PC 1, then PC 0) discards the live edits (the sound returns to the stored patch).
  So the live tests above are of the edit buffer.

Still open: how an LFO or ENV reaches COLOR (or OSC1 X-MOD) on the SYSTEM-8. Candidates: the matrix needs the cord
stored in the patch (save path), the cord only works in a different mode, or the matrix value encodes the target
differently. Next: test with the cord stored (after a save) or find the code that reads `PAT_NAME1` in region 0.

## Step 4 (2026-10-20): cords as a whole patch, and a host LFO on COLOR

- **Read/write by leaf.** The device answers an RQ1 only up to the end of the leaf (for example OSC1 gives 10 bytes
  at offset 14). `patch_write_test.py` reads and writes the 640-byte patch leaf by leaf.
- **Whole-patch write of the cord** (`OSC1 X-MOD IN` = 13, patch offset 360): accepted (read back), but the sound
  does not change, live or after a program change (PC 1, PC 0), compared with cord 0 (458 Hz vs 458 Hz after PC).
  So the cords have no audible effect on this path.
- **Host LFO on COLOR** (`host_lfo_color.py`): a triangle 0 -> 255 -> 0 is sent as DT1 to `OSC1 COLOR` from one
  open MIDI session at 20 steps/s while a note plays (period 4 s). The device accepts the writes. The spectral
  centroid per 0.25 s window does not clearly follow COLOR in this measurement (183–245 Hz, weak correlation); the
  level decays during the note. The effect has to be judged by ear: run the script and listen.

## Step 5 (2026-10-20): COLOR in the DSP microcode of the SYSTEM-8 saw

- The user's direction: COLOR is a DSP parameter; look at the saw microcode of the SYSTEM-8 itself, not at the MIDI
  path. The earlier live-write tests were a dead end (the device does not change the sound with DT1 writes to the
  patch memory, while the panel does).
- The JX-3P saw (`dco1_saw`, SHA `d54b9f85a1bde4cc`, `engine/programs.py`) is NOT the SYSTEM-8 saw: the 4104-byte
  block at the best candidate position in the SYSTEM-8 region 1 (`work/sys8_proj/region1.bin`, offset 154076) matches
  in 5.7 % of the bytes. The 32-byte match was a coincidence.
- Next: locate the DCO program descriptors of the SYSTEM-8 region 1 (the table that gives run address, size and program
  for each slot, as `engine/programs.py` does for the JX-3P), extract the SAW slot program, and find the instruction that
  reads the COLOR parameter (the parameter enters as a value in the program's state or as a word loaded from the
  parameter area). Verify the candidate by changing the word on the lab, not by the MIDI path.

## Step 6 (2026-10-20): the DCO descriptor table of the SYSTEM-8 (region 1)

Source: `work/sys8_proj/region1.bin` (decoded region 1 of the stock SYSTEM-8, `p0s`). Program addresses are
`0x60800000 + file offset`.

- **Descriptor record (16 bytes):** `{ u32 slot, u32 size_words, u32 size_words, u32 program_address }`, the same
  shape as the `dspdesc_t` entries of `engine/engine.c` (slot, size, size, pointer). Slot `0x6000` = DCO1 slot,
  `0xB000` = DCO2 slot.
- **85 records (file offsets `0x026608` to `0x0532CC`), 83 distinct programs** (by SHA-256 of their words).
- **Families (DCO1 slot `0x6000`):** size 3614 (6 programs), 2758 (2), 2410 (2), 2362 (2), 2306 (2), 2190 (2),
  2126 (2), 2106 (2), 2046 (2), 1950 (2), 1918 (2), 1854 (2), 1822 (2), 1794 (2), 1614 (2), 1366 (2), 1294 (4),
  1238 (2), 1166 (2), and 4 programs of 14 words (empty / placeholder).
- **Families (DCO2 slot `0xB000`):** the same pattern with sizes 3618 (6 programs), 2414 (2), 2110 (2), 2050 (2),
  1954 (2), 1858 (2), 1618 (2), 1370 (2), 1330, 1298 (4), 1242 (2), 1220, 1170 (2), 1068, 974, 784, 736, and 3
  programs of 14 words.
- **Reading:** the 6-program family (one per OSC WAVE value) suggests that each DCO program is selected by the
  pair (WAVE, COLOR band). COLOR therefore selects a DIFFERENT PROGRAM, not a register inside one program.
- **Why the writes did nothing:** the engine swaps the DCO program when the panel changes WAVE or COLOR (the
  parameter callback, like `swap_dco` in `engine/engine.c`). A DT1 write into the patch memory does not call that
  callback, so the sound stays on the program that was loaded.

Still open: the code in region 0 (`p0m`) that maps (WAVE, COLOR) to a descriptor index, and the instruction-level
decoding of the programs (which words read the parameter area). Next: find the table that selects the descriptor
(a reference to the 16-byte records from region 0 or from the parameter callback), then decode the SAW program.

## Step 7 (2026-10-20): the SAW of the SYSTEM-8 is the JUPITER-8 family

- The JUPITER-8 script names the wave of its VCO: `oscWave = SAW, PWM, TRI, SIN, SQR, NOISE` (index 0 = SAW).
- The SYSTEM-8 DCO2 programs are the same code as the JUPITER-8 ones (90 % shared 32-byte fragments), with the size
  changed by +14 words (for example JUPITER-8 770 words <-> SYSTEM-8 784 words at `0x60851A20`).
- The JUPITER-8 DCO2 table has six descriptors without a DCO1 pair, in this order: 770, 722, 1316, 1054, 976, 1236
  words, one per wave. Taking that order as the wave index: index 0 (SAW) = 770 words = SYSTEM-8 **784 words at
  `0x60851A20`** (candidate, 90 % match); index 1 (PWM) = 722 words = SYSTEM-8 736 words at `0x60851D58`.
- Status: candidate identified statically; not yet confirmed by audio. The file is `examples/sys8_lab/progs/`.

### Step 7 results (audio on the lab, PLG3)

- The lab reference saw (`dspexp/own/saw_dco2.txt`) gives harmonics 1, 0.50, 0.33, 0.25, 0.20, 0.17: a correct saw.
- **Descriptor sizes are in bytes**, not words. The first extraction read them as words and doubled the programs
  (the extra words were the next record). Corrected: the candidate SAW is **392 words** (`progs/d2_60851a20_392w.txt`),
  ending with `0b02 00f8 0001` like the reference. 79 distinct programs in `progs/`.
- The candidate loads (392 words in DCO2 of both slaves) but produces **no sound** (RMS 0.0). So it is NOT confirmed
  as the SYSTEM-8 SAW in this context. Open: the program probably depends on state that only the SYSTEM-8 main
  program sets up (pitch, gate or slot memory), or the JUPITER-8 index order is not the wave order.
- Next: load the SYSTEM-8 DCO1 SAW family (same method) and check which candidates output audio; or find the
  SYSTEM-8 main program in region 1 and load it first.

### Step 7b: SYSTEM-8 main programs (2026-10-20)

- Main-slot (`0x0100`) descriptors in region 1: six programs, in table order DSP0..DSP5 (file offsets
  `0x023EE0` 5952 bytes, `0x025480` 5438, `0x0265F8` 4382, `0x027654` 4034, `0x02832C` 3136, `0x029668` 4836).
  The slave main programs are probably index 2 and 3 (4382 and 4034 bytes); not verified.
- The lab (`dsplive` / `dspown`) can only write the DCO slots. It cannot replace the main program, so the SYSTEM-8
  candidate SAW cannot be tested against the SYSTEM-8 main in the current lab. It runs with the lab's own main.
- Conclusion: the candidate (`progs/d2_60851a20_392w.txt`) stays unconfirmed. Validation needs a lab built on the
  SYSTEM-8 image (main + DCO slot programs + loader), which is the same work as the S8LAB lab.

### Step 8 (2026-10-20): static check of the candidate and scope of the SYSTEM-8 lab

- The reference saw uses the slot pin/state/output addresses of the JX-3P layout (`0x2207`, `0x0670`, `0x0680`,
  `0x2221`, `0x221D`, `0x2214`). The candidate (392 words) contains none of them as literal words. The SYSTEM-8 may
  use a different slot memory map, so this does not decide anything yet.
- Scope of a lab on the SYSTEM-8 image (not started): (1) map the SYSTEM-8 slot memory (pins, state, outputs) from the
  main programs of region 1; (2) replace or extend the main program of the slave DSPs, which needs a region 1 build
  with our loader (relocations for each slot, as the JX-3P lab does with `relocs.json`); (3) test the candidate saw
  and the other DCO families with audio. Each step needs its own hardware checks.

## Status (2026-10-20): S8LAB in PLG1, lab ready for tests

- The lab is `S8LAB` (id 0x20, 0.01) in **PLG1**: the SYSTEM-8 v1.32 regions with only the identity changed.
  Rebuild and check: `build_s8lab.sh` (the repack is byte-identical to the installed image).
- Install is firmware only (no Script.xml, no Patch bank): the patches already in PLG1 are used.
- `check_lab.py` is the preflight; `README.md` lists the tests, their status and the rules.
- Open items: the COLOR/MOD/cord results above; the level of the test note is low; the centroid of COLOR 93 differs
  from step 2 (2212 vs 4677 Hz), not explained.
