# C-MORPH — one wavetable position that morphs with an LFO

DCO1 has one sound: six frames of `80's PWM.wav` (frames 0, 6, 12, 18, 24, 30 of 31, 72 points each) read by a
triangle LFO, frame 0 → 5 → 0, one period every 4 s (`MORPH_HZ`, default 0.25 Hz). DCO2 keeps the six AKWF
programs of C-WAVE.

- `make.py` converts the WAV with `re/wt5.py` (`convert`, `words_of_table`, `program(..., scan=0.25)`):
  - `build/morph_table.h`: the 438-word table (18-bit values) that the engine writes into bank 5 of DSP2/DSP3 at start
    (`engine.c -DMORPH`, `morph_load()` from `e_start`).
  - `prog/morph_dco1.txt`: the DSP player (114 instructions) for DCO1 position 0..5 (all six are the same program).
- `build.sh` → `plugins/C-MORPH`.

    examples/wt_morph/build.sh
    python3 tools/plugout_tool.py install plugins/C-MORPH --slot 3 --yes --force

After the install select **PLG3** on the panel.

Not verified by audio yet (the `--scan` player was never heard before). Limits: 438 words of bank 5 (6 × 73), so 72
points per frame; the LFO rate is fixed at build time; the table is written when the plugout starts.
