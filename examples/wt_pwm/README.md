# C-PWM — wavetable PLUG-OUT with a WAV inside

DCO1 is a wavetable made of six frames of `80's PWM.wav` (Echo Sound Works Core Tables / Analog, 31 frames of 2048
samples). The frames 0, 6, 12, 18, 24 and 30 are compiled into the DSP microcode at 128 points each
(`prog/dco1_w0..5.txt`, from `re/wav2dco.py --cycle 2048 --frame F -n 128 --slot dco1`, ~345 instructions each).
DCO1's waveform parameter (0..5) selects the frame. DCO2 keeps the six AKWF programs of C-WAVE.

Nothing is transferred over MIDI: the table is part of the plugout. This is the baseline to compare with the
streaming design (WAV in region 1 → bank 5 → DSP) for longer samples.

    examples/wt_pwm/build.sh                                  # -> plugins/C-PWM
    python3 tools/plugout_tool.py install plugins/C-PWM --slot 3 --yes --force

After the install, select **PLG3** on the panel and set DCO1's waveform to 0..5.

Limits: six fixed positions (no morph between frames), 128 points per frame (the budget allows up to ~290 points for one
DCO, but the two DCOs share it), and the frames are fixed at build time.
