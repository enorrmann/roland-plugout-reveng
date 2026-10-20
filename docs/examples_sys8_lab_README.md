# S8LAB — SYSTEM-8 lab PLUG-OUT (PLG1)

The SYSTEM-8 v1.32 firmware, rebuilt as a PLUG-OUT named `S8LAB` (id `0x20`, version 0.01), installed in **PLG1**
(slot 1). Its only change is the identity; the engine is Roland's, so the tests below probe the stock SYSTEM-8
behaviour and the audio/MIDI tools around it. Plan and history: `docs/PLAN.md`.

## Commands (run from `examples/sys8_lab/`; the device on USB, no other MIDI tool running)

```sh
./build_s8lab.sh            # rebuild plugins/S8LAB from work/sys8_proj and check it (no hardware)
./build_s8lab.sh install    # also send it to PLG1 (firmware only; the patches in PLG1 stay)
python3 check_lab.py        # preflight: PLG1 holds S8LAB 0.01, patch 0 OSC1 reads back
```

Select **PLG1** on the panel (with its password) to hear it. After an install the device boots to SYSTEM-8.

## Tests

Each test plays a held note on PLG1 patch 0 (`arecord` on `hw:SYSTEM8`, note sent with `amidi`), measures the
spectral centroid and level, and restores the patch values it changed.

| Script | What it does | Writes | Status |
|---|---|---|---|
| `color_test.py 0 255 93` | OSC1 COLOR (`0x6D5`) at the given values, one note each | DT1 to COLOR, restored at the end | works; COLOR changes the timbre, not monotonic |
| `cords_test.py 0 2 3` | `OSC1 X-MOD IN` cord source (`PAT_NAME1 + 104`) and `OSC1 CROSS MOD` | patch, restored | cord has no audible effect; CROSS MOD does |
| `patch_write_test.py 13` | whole 640-byte patch written in 128-byte blocks, live and after PC 1 / PC 0 | patch, restored | cord write accepted, no effect |
| `mod_scan.py 0 16` | EXTEND `MOD ID` 0..15 with `MOD PRM` 255, level and centroid per 50 ms | EXTEND, restored | no audible change found |
| `host_lfo_color.py [period] [rate]` | host triangle 0 → 255 → 0 on COLOR while a note plays | DT1 stream | inconclusive by measurement; judge by ear |

Measurement notes:
- The centroid of a single note is noisy. Use the same value twice to see the spread before reading a change.
- Level is low (RMS about 0.003 in the smoke test); the centroid of COLOR 93 measured 2212 Hz now against 4677 Hz
  in PLAN.md (step 2). The difference is not explained yet; compare runs made in the same session only.
- `dspaudio.record` and `amidi` are the same helpers the other labs use (`examples/from_scratch/re/dspaudio.py`).

## Layout facts used by the tests

- Patch addresses come from the SYSTEM-8 script, `plugins/SYSTEM-8/Script/Script.xml` (`tools/patches.py`).
- OSC1 leaf (10 bytes at patch offset 14): WAVE, COLOR, MOD, RANGE, CROSS MOD, two nibble bytes each.
- Patch cords are in `PAT_NAME1` (patch offset 256), 8 bytes per destination, values 0..13.
- The `EXTEND` block (offset 108) holds `MOD ID 1..3` and `MOD PRM 1..3`.

## Rules

- One MIDI client at a time: the scripts open the device; a second tool collides with them.
- Every test restores what it changed. If a run is killed, a program change (PC 1, then PC 0) discards the live
  edits and returns PLG1 to its stored patch.
- The source of the image is `work/sys8_proj`, unpacked from `firmware/sys8_sys_v132/SYSTEM8_UP.BIN`
  (`tools/sys8fw.py`). `build_s8lab.sh` refuses to build if those regions change.
