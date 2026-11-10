# Volume VI — Chapter 5: Wavetable Engines

> The wavetable work of both devices, with pointers to the details; each fact names its source. SYSTEM-8:
> [ch. 6](custom_osc.md) (sections "Wavetable from a single-cycle WAV" and "Real wavetable in bank 5") and
> [examples/wt_pwm/README.md](../../examples/wt_pwm/README.md),
> [examples/wt_morph/README.md](../../examples/wt_morph/README.md) (+ [OPTIONS.md](../../examples/wt_morph/OPTIONS.md)),
> [examples/wt_live/README.md](../../examples/wt_live/README.md). SYSTEM-1: [ch. 7](mutable_memory.md) §3.3.8 and §4.

## 1. SYSTEM-8

| PLUG-OUT | where the table lives | how it is read | state (from its README) |
|---|---|---|---|
| C-WAVE | in the DSP microcode itself: one A/A/B instruction per point (`re/wav2dco.py`, `re/dspasm.py wavetable()`) | the program is the table | [ch. 6](custom_osc.md) "Wavetable from a single-cycle WAV" (2026-10-16) |
| **C-PWM** (`examples/wt_pwm`) | six frames of `80's PWM.wav` (frames 0, 6, 12, 18, 24, 30) compiled into DCO1 microcode at 128 points each (`prog/dco1_w0..5.txt`, ~345 instructions each); DCO1's waveform parameter (0..5) selects the frame | microcode | the README gives no hardware result; limits: six fixed positions (no morph), 128 points per frame (the budget allows ~290 points for one DCO, but the two DCOs share it), frames fixed at build time |
| **C-MORPH** (`examples/wt_morph`) | six frames of `80's PWM.wav` (72 points each, 438 words) written by the engine into bank 5 of DSP2/DSP3 at start (`engine.c -DMORPH`, `morph_load()`) | a 114-instruction DSP player, a triangle LFO sweeps frame 0 → 5 → 0, one period every 4 s (`MORPH_HZ`, 0.25 Hz) | **not verified by audio yet**; limits: 438 words of bank 5, LFO rate fixed at build time |
| **C-WTLIVE** (`examples/wt_live`) | a WAV sent over MIDI (`wtlive.py send`) into bank 5 words `0x048…0x1FD` of DSP2 and DSP3 (`K·(N+1) ≤ 438`) | a ~100-instruction player (275 words) loaded live into the chosen oscillator slot; bank-5 indexed loads, interpolation inside and between frames | 2026-10-20: build, install and `send` run on the device (table 438 words in 6.8 s); OSC2 plays the table; the OSC1 and `both` options and `wt5.py --scan` are not verified by ear |

Transport of C-WTLIVE (its "How it works"): the host sends MIDI on channel 1 through the stock region 0, which forwards it
to the slaves as parameters: pitch bend carries 14 data bits (voice id `0x465`), CC1 carries the command
`seq<<6 | cmd<<4 | ext` (voice id `0x467`); commands `ADDR`, `DATA`, `CTRL`; one `DATA` per point, 18-bit fixed point,
about 15 ms per command. Programs are loaded as an empty program first, then the body (tail first, head last).

Room for tables on the SYSTEM-8 (C-WTLIVE README; [Vol. II ch. 1](../vol2_dsp_core_isa/core_architecture.md) §7.4): bank 5
has 1024 words per DSP; the free part in general is `0x048`–`0x1FF` and `0x306`–`0x3FF`. Long samples need the delay
memory (EDM), on the SYSTEM-8 only reachable from master DSP4 slot `0xF7DC`. The options considered for a 4–5 s morph
are in [OPTIONS.md](../../examples/wt_morph/OPTIONS.md).

## 2. SYSTEM-1 (SYSTEM-S1CC)

From [ch. 7](mutable_memory.md) §4.0, the practical rule:

| case | plug-out | example kernel | measured |
|---|---|---|---|
| single-cycle oscillator (one wave or a few frames) | slave, **fixed frame in bank 5** (`SL_WT=b5`) | `sdk/s1cc/examples/wavetable/jp8_saw.c`, `wt_slave.c` (WT-SLAVE5) | 8 voices; 127 points (63 harmonics) at `A080`–`A0FF`; harmonics within 0.5 dB of the ideal saw, inter-harmonic floor ~−161 dB (2026-11-06) |
| long multi-frame table with morph | **master's EDM** (`EDM_WT=1`) | `sdk/s1cc/examples/wavetable/wt_edm_cmod.c` (WT-LINES8) | ~6 voices, fills in the first ~13 notes, 17–22 dB more noise than the ideal read; morph by ⅛ frame works (2026-11-06) |

Other variants measured on the device (ch. 7 §4.1, §4.4): WT-SLAVE768 (a 768-point loop in bank 0 of slave DSP A, refilled
by the ARM in blocks of 64 points every 12 ms: works, 8 voices, but measured worse than bank 5: 3.7 dB error, floor
−91 dB); WT-EDM and WT-EDM-CMOD (the whole Sys8.wav in an EDM loop read by master B with one `tapreq` per voice, sent to
the slave through `600E + v`; works with 6 voices and per-voice COLOR MOD); MP-EDMWT (first EDM table, 2026-11-05).
Rules learnt on the way: only `A000`–`A1FF` of bank 5 is reliable on the S1 slave (`A200` is corrupt), bank 0 of DSP A
wraps at `0x0400`, at most 6 own `tapreq` per sample on master B, master A needs an own empty main program (`MA_MAIN`)
for `600E`–`6021` to reach the slave, and non-round immediates run differently on master B than in the emulator (ch. 7
§4.2, §4.3, §4.4).

Loading a frame live over USB (`SL_LOAD`, `tools/wt_load.py`): built, **untested on the device** as of 2026-11-06 (ch. 7
§4.5; the same statement is in the docstring of `tools/wt_load.py`).
