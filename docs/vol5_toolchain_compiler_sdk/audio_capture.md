# Volume V — Chapter 6: Capturing Audio and Sending Notes to the SYSTEM-1

> **Scope:** SYSTEM-1.

> The SYSTEM-8 counterpart (USB capture with `arecord -D hw:SYSTEM8,0 -f FLOAT_LE -r 96000 -c 2`, `re/dspaudio.py`)
> is described in [Vol. I ch. 3](../vol1_system_architecture/bus_interconnect.md) §4.7 and
> [Vol. II ch. 11](../vol2_dsp_core_isa/slave_dsp_readback.md).

How to play notes on the SYSTEM-1 over USB and record its output, to verify a PLUG-OUT on the device (the rig we use, for
example, to check the wavetable: send a frame and classify the captured wave).

Tool ready: **`tools/s1_capture.py`** (it does all of this in one command). This document explains the method in case it
has to be done by hand or understood.

## Devices (this setup)

| What | Value | How to find it |
|---|---|---|
| MIDI port | `hw:3,0,0` (SYSTEM-1 MIDI 1) | `amidi -l` → line with `SYSTEM-1 MIDI 1` |
| Audio device | `hw:3,0` (card `SYSTEM1`) | `/proc/asound/cards`, or `arecord -l` |
| Audio format | **S32_LE, 96000 Hz, 2 channels** | `cat /proc/asound/card3/stream0` |
| **MIDI channel** | **5** (nibble 4: note-on `0x94`) | ⚠️ it is set on the device; if it changes, nothing sounds |

The card number (3) can change between boots; take it from `/proc/asound/cards` instead of fixing it. **The
synthesizer's output appears on the USB audio input only while the device is sounding.**

## Rules

- **Only one MIDI client at a time** (project rule): do not run another SYSTEM-1 tool (e.g. `wt_load.py`) while
  capturing. `amidi` opens and closes on each call, so it can be interleaved with other tools, but not simultaneously.
- **Do not touch the keyboard while recording**: your notes add to the capture and dirty the spectrum (it happened to
  us: "saw" came out where there was a square). The tool sends the note by itself.
- The MIDI channel belongs to the device, not to the tool: if nothing sounds and the audio is silent, it is almost
  always the channel.

## The method by hand

Note (channel 5 = nibble 4): note-on `0x94 note vel`, note-off `0x84 note 00`. CC = `0xB4 cc val`, pitch bend =
`0xE4 lsb msb`.

```sh
# 1. play a note (A = 0x45 no; 0x3c = middle C), and record 2 s in parallel
amidi -p hw:3,0,0 -S '94 3c 64' &            # note-on, note 60, vel 100
arecord -q -D hw:3,0 -d 2 -f S32_LE -r 96000 -c 2 -t raw out.raw
amidi -p hw:3,0,0 -S '84 3c 00'              # note-off
```

Decoding the raw (interleaved stereo S32_LE) in Python:

```python
import numpy as np
x = np.fromfile('out.raw', '<i4').reshape(-1, 2).astype(np.float64) / 2147483648.0
mono = x[:, 0] + x[:, 1]
print('peak', np.abs(mono).max())            # 0.0 = silence (channel? PLUG-OUT selected?)
```

### Classifying the wave (saw / square / triangle) by harmonics

Take the **sustained stretch** (skip attack and release), Hann window, FFT, measure h2..h6 relative to h1:

```python
nz = np.nonzero(np.abs(mono) > mono.std() * 0.1)[0]
a, b = nz[0], nz[-1]
mid = mono[a + (b - a)//4 : b - (b - a)//4]; mid -= mid.mean()
X = np.abs(np.fft.rfft(mid * np.hanning(len(mid))))
f = np.fft.rfftfreq(len(mid), 1/96000.)
f0 = f[1 + np.argmax(X[1:])]
h = lambda k: X[(f >= f0*k*0.97) & (f <= f0*k*1.03)].max()
h1 = h(1)
# saw: strong even harmonics (h2/h1 ~0.5). square: even ~0, h3/h1 ~0.33. triangle: h3/h1 ~0.11 (1/9).
```

Rules: `(h2+h4)/2 > 0.25` → **saw**; otherwise if `h3/h1 > 0.22` → **square**; otherwise if `h3/h1 > 0.03` →
**triangle**; otherwise → sine. (Note 60 should give f0 ≈ 261 Hz; if ~1 Hz comes out, the capture is silent or the
detection caught a subharmonic of a transient.)

## The tool

```sh
python3 tools/s1_capture.py                         # note 60, channel 5, 1.5 s -> summary of harmonics
python3 tools/s1_capture.py --note 67 --secs 3
python3 tools/s1_capture.py --cmd "python3 tools/wt_load.py --square"   # runs that, then captures
python3 tools/s1_capture.py --raw cap.raw           # keeps the raw (S32_LE 96k 2ch)
python3 tools/s1_capture.py --list                  # shows the devices and exits
python3 tools/s1_capture.py --channel 1             # another MIDI channel (1..16)
```

To send a wavetable frame and see it change: `tools/wt_load.py` ([Vol. VI ch. 7](../vol6_reference_designs_applications/mutable_memory.md) §4.5); it also sends on channel
5 by default (`--channel`).
