# SD-TXT: read a file from the SD card in a region 0 init hook

A JX-3P-based PLUG-OUT whose region 0 init hook opens a file on the SD card
with the OS's own file functions, reads it, and reports the bytes over the MIDI
router, so they can be read from the USB MIDI port during the install. This is
the first step toward using a WAV from the card as an oscillator (see
"Next steps").

## Files

| File | |
|---|---|
| `sd_txt.s` | the hook (Thumb, region 0 cave at `0x61800100`); path, retries and read size are at the top and the bottom of the file |
| `make_sd_txt.sh` | builds `plugins/SD-TXT` (stock JX-3P-MEM template + hook, like `midi_send/make_init_cc.sh`) |
| `decode.py` | rebuilds the bytes from a MIDI capture (`--out` writes a binary) |
| `cap_saw_header.txt` | capture of the first 96 bytes of `A:/wav/saw.wav` (for `decode.py`) |
| `README.md` | this file |

## Build and install

The card must be in the SYSTEM-8 when you install. Start from a device that
answers (`tools/plugout_tool.py info`).

```sh
examples/sd_txt/make_sd_txt.sh                        # -> plugins/SD-TXT (about 1 s)
# 1. stop any old capture first, or the port is busy and the capture stays empty
for p in $(pgrep -x amidi); do kill $p; done
# 2. capture USB1 in the background (the hook answers during the install)
(timeout 400 amidi -p hw:3,0,0 -d > /tmp/..._cap.txt 2>/dev/null &)
# 3. install into slot 3 (--force: the same name and version would be skipped)
python3 tools/plugout_tool.py install /home/emilio/git/roland-plugout-reveng/plugins/SD-TXT --slot 3 --no-patches -y --force
# 4. after about 30 s
python3 examples/sd_txt/decode.py /tmp/..._cap.txt
```

Use absolute paths in `plugout_tool.py install` (a relative path fails when the
shell is not in the repo). Do not use `pkill -f amidi`: it matches the shell
running the command and kills it (exit 144).

## Witnesses (what the capture contains)

All are CC on channel 16 (`BF cc value`), through the router with mask `0x1A20`:

| CC | Value | Meaning |
|---|---|---|
| `0x69` | attempts | how many `fopen` retries before success (0 = first try) |
| `0x67` | bytes read | `0x7F` = `fopen` failed every retry |
| `0x68`, `0x6A` | nibbles | for each byte, in order: `0x68` = low nibble, then `0x6A` = high nibble |

The hook sends `0x69` first, then `0x67`, then the bytes. If the capture has only
`BF 67 7F`, the card was not readable at that moment.

## OS functions used (from `work/os/m0_core0`)

| Function | Address | Use |
|---|---|---|
| `fopen(path, mode)` | `0x60057E54` | returns `FILE*` (0 = error) |
| `fread(buf, size, count, fp)` | `0x60057EA8` | the arguments pass through the thunk unchanged; the result is the number of items read |
| `fclose(fp)` | `0x60057E28` | |
| MIDI router | `0x6000345C` | `wrap_packet` `0x600753EA`, `Wrap dtor` `0x600753F4` (same as `init_cc.s`) |

The ABI is confirmed by their call sites: `0x6000CD28` (read, compares the
count with `n`), `0x6000CD02` (close), `0x6000CCDC` (open, `"wb"` at
`0x6000D01C`). The backup writer `0x6001D70C` → `0x6000C7B0` → `fopen("wb")`
does not mount the volume itself.

## Results so far

- **Card in the device**: `fopen` works on the first try, `fread` returns the
  requested bytes. `A:/ROLAND/SYSTEM-8/BACKUP/sys8_bak.bin` (made from the panel
  backup) starts with `SYS8`.
- **Earlier failures** (`A:/hola.txt`, 1027 CC `0x7F` in one capture): the user
  confirmed the problem was the card state at the time, not the path. Not
  re-tested with the card in place.
- **`A:/wav/saw.wav`** (44 100 Hz float, `clm ` wavetable, 16 512 B RIFF):

  | Offset | Bytes | Meaning |
  |---|---|---|
  | 0 | `RIFF` size `0x4080` `WAVE` | file ~16.5 KB |
  | 12 | `JUNK` 28 B | padding, skip |
  | 48 | `fmt ` 16 B: tag `3`, mono, `44100`, byte rate `176400`, block align `4`, `32` bits | IEEE float |
  | 72 | `clm ` 48 B: `<!>2048 20000000` | Serum table header, 2048 samples per frame |

  The float data starts after the `clm ` chunk. `wt5.py` on the PC already
  reads this format.

## Limits and warnings

- **Busy waits in the init hook can hang or fail the installer.** `RETRIES=40`
  with `DELAY=2000000` (several seconds) made the installer fail and the device
  hang. Current values: `RETRIES=3`, `DELAY=20000`. Keep init hooks short.
- **Stack**: the buffer is 96 bytes on the stack (`sub sp, #100`). Reading a
  whole WAV this way is not possible; read in blocks or stream.
- **Calls into the OS from the hook run on the OS CPU (region 0).** Region 1
  (the engine) cannot call them (BASE_FIRMWARE §7.10).
- **Installing with the same name and version is skipped** unless `--force`.
- **One MIDI client at a time** (see the memory note). A stale `amidi` on the
  port makes the capture empty without an error.

## Next steps

1. Read the first frame of `saw.wav` (2048 floats, after the `clm ` header) in
   blocks, and send the floats as witnesses (nibbles or 18-bit fixed point).
2. Reduce 2048 points to the 437 that `wt_live` accepts (band-limited
   resampling, as `wt5.py` does), and send them like `dsplive.py` does
   (`ADDR` / `DATA` commands on CC1 and pitch bend).
3. Check the oscillator sound on the device (`wt_live` path).

Current state of the device: slot 3 holds this hook (`SD TXT 0.01`), which
replaced `C-WTLIVE`. Restore `C-WTLIVE` with `examples/wt_live/wtlive.py install`
when you need it back.
