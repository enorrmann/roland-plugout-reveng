# Volume VI — Chapter 3: CZ-SYSTEM — Casio CZ Phase Distortion as an s1cc Kernel for SYSTEM-S1CC

> **Scope:** SYSTEM-1.

> The full description (Spanish) is [sdk/s1cc/examples/casio_cz/README.md](../../sdk/s1cc/examples/casio_cz/README.md);
> section numbers in parentheses are those of the README. Source of the description of the CZ used by the example:
> Michael Rickard, *Casio CZ-1 Spelunking* (https://www.kasploosh.com/cz/).

## 1. What it is and its state

An s1cc kernel ([Vol. V ch. 1](../vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md)) for the SYSTEM-S1CC PLUG-OUT
([examples/system_s1cc/README.md](../../examples/system_s1cc/README.md)). It reproduces the phase-distortion synthesis of
the Casio CZ: the DCO waveforms, the DCW as an amount of distortion with its envelope, the two lines with detune and the
ring and noise modulations. What the CZ has and the SYSTEM-S1CC host already gives (envelopes, LFO, filter) is supplied by
the host.

**State (2026-11-03): verified against its native reference (`s1cc check`) and in the complete emulator chain
(`make_s1cc.py --sim`). 8 voices. Not tested on the device.**

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/casio_cz/cz.c --check          # S1_COMPILE already sets --regs 0,1,2,3
KERNEL=sdk/s1cc/build/cz sh examples/system_s1cc/build.sh            # -> plugins/CZ-SYSTEM (S1_BUILD: NAME, MIX=0)
python3 tools/plugout_tool.py install plugins/CZ-SYSTEM --slot 1 --force --no-patches -y
```

## 2. Panel (§2)

| CZ | SYSTEM-1 control | what it does |
|---|---|---|
| DCO1, waveform of line 1 | OSC1 WAVE | positions 0…4 = WAVE 1…5 of the CZ; position 5 = WAVE 6 (resonance, saw window) |
| DCO2, waveform of line 2 | OSC2 WAVE | positions 0…4 = WAVE 1…5; position 5 = WAVE 7 (resonance, triangle window) |
| DCW of each line | OSC1 / OSC2 COLOR | distortion 0…1: 0 = sine (WAVE 1…5) |
| DCW envelope | OSC1 / OSC2 COLOR MOD = FILTER ENV | DCW = COLOR × FILTER envelope (A / D / S / R); also LFO, PITCH ENV, AMP ENV, SUB OSC |
| LINE SELECT 1+2' | OSC1 MIX / OSC2 MIX | level of each line, 0…0.5 |
| OCTAVE | OSC1 RANGE | both lines (the host applies it to the pitch) |
| octave and DETUNE of line 2 | OSC2 RANGE, OSC2 FINE | line 2 = line 1 × 2^(OSC2 RANGE − OSC1 RANGE) × 2^(±1/12) (FINE 128 = in tune) |
| RING modulation | SUB | line 2' × ((1 − m) + m · line 1), m = SUB / 255 |
| NOISE modulation | NOISE | line 2' × ((1 − m) + m · noise), m = NOISE / 255 |

The rest of the panel table (envelopes, filter) is in §2 of the README.

## 3. One formula for all the waveforms (§3)

The DSP does not jump: each branch of an `if` is paid whole. Instead of computing six shapes and choosing one, the voice
computes a single phase-distortion formula and the WAVE selector changes its coefficients; each selector is an
`S1_PARAM` table of 6 words. With p the oscillator's phase (0…1) and d the DCW (0…1) (§3.2):

```text
k   = (1 + a·d)^4, a = 32^(1/4) − 1          slope of the fast segment: 1…32; at most rdt/2
r   ≈ 1/k                                    cubic of 1/(1 + a·d), 0.5 %, to the fourth
B   ≈ 1/(2 − r)                              cubic in r, 0.2 %: slope of the slow segment of the saw
R   = 1 + SQ + 15·F·d                        at most rdt/4
q   = R·p + O·(1 − r),   n = floor(q),   h = q − n
phi = min((KA·k + F)·h,  1 − (BG + BE)·B + (BG·B + BE)·h,  1)
y   = −(1 − F·win(p) − 2·SQ·n) · cos(2π·(1 − SQ/2)·phi)
win(p) = p (OSC1: saw window 1 − p)  or  |2p − 1| (OSC2: triangle window)
```

| position | WAVE | SQ | F | O | KA | BG | BE |
|---|---|---|---|---|---|---|---|
| 0 | 1 saw | 0 | 0 | 0 | 1 | 1 | 0 |
| 1 | 2 square | 1 | 0 | 0 | 1 | 0 | 0 |
| 2 | 3 pulse | 0 | 0 | −1/2 | 1 | 0 | 0 |
| 3 | 4 sine-pulse | 0 | 0 | 0 | 1 | 0 | 0 |
| 4 | 5 saw-pulse | 0 | 0 | 0 | 2 | 0 | 1 |
| 5 | 6 / 7 resonance | 0 | 1 | 0 | 0 | 0 | 0 |

phi is a minimum of lines: continuous for any coefficient. DCW keyboard tracking: k ≤ rdt/2 (the fast segment lasts at
least 2 samples) and R ≤ rdt/4 (the resonance's sine stays below fs/4). Line 2: its step is dt × ratio and its 1/dt comes
from the inverses of the same tables, without division. Noise: n = wrap(K·n + W), K = 7.31275, W = 0.618…, two ops (§3.2).

## 4. Budget (§4)

Fused kernel (the default): the whole voice in one program, voices 0…3 on DSP A and 4…7 on DSP B. Measured 2026-11-03
(in the emulator's build accounting), with the complete host (`ARM_ENV=1 ARM_LFO=1`, `MOD_DSP=B`):

| | instructions |
|---|---|
| kernel per voice (oscillators, ring / noise, SVF, VCA) | 179 |
| DSP A: `s1m0` + section `s1a2` | 284 + 373 = 657 (budget 703) |
| DSP B: `s1m1` + section `s1b2` | 387 + 311 = 698 (budget 708) |

How it went from 6 to 8 voices: generic formula with a 13-word table (233 per voice, 6 voices) → 6-word table (215) →
`s1_recip` instead of the second `s1_exp2` (208) → k = (1 + a·d)^4 and r by polynomial (186, 7 voices) → noise in two ops
and ring / noise with (1 − m) in the table (179, 8 voices). With 8 registers (`--regs 0,…,7`, without voices in pairs) 8
voices do not fit (A 858, B 890).

## 5. Verification (§5)

* **Native reference** (`s1cc check`, fused kernel, line 1 alone): SNR 75…76 dB (10 ms) in positions 0…4 and 65 dB in the
  resonance. Line 2 alone: 31…44 dB (phase drift of dt × ratio against the reference's float; MP-CSAW2, which works on the
  device, gives 39 dB with its line 2 on the same bench).
* **Complete chain** (`make_s1cc.py --sim`, 8 voices): DCW 0 gives a sine (harmonics above 1 kHz at −80 dB); the AMP
  envelope does attack, decay, release; 8 notes peak 0.508; no notes peak 0; COLOR 255 with COLOR MOD = FILTER ENV moves
  the spectral centroid 3695 → 441 Hz in 0.5 s; COLOR MOD = LFO makes it oscillate between 1460 and 3940 Hz.
* `sdk/s1cc/tests/test_s1cc.py` (`TestExamples`, case `casio_cz/cz.c`) checks line 1 (saw, square, resonance) and line 2
  (saw-pulse) against the reference.

## 6. Differences with the CZ (§6)

| CZ | here | why |
|---|---|---|
| 8-stage envelopes (DCO, DCW, DCA) per line | the host's ADSRs: AMP = DCA, FILTER = DCW (with COLOR MOD), PITCH ENV (AD) = DCO; the same for both lines | the host's (request: use the host's envelopes and filters) |
| combined waves (two shapes alternated cycle by cycle) | no | the panel has one selector per line |
| WAVE 8 (trapezoid window) | no position | 6 positions per selector |
| ring / noise: on or off | continuous amount (SUB, NOISE) | crossfade between line 2 unmodulated and modulated |
| 4 voices in 1+2' (8 with one line) | 8 voices with both lines always | budget §4 |
| no filter | the host's SVF | request; CUTOFF 255, RESONANCE 0 leaves it open |
| exact shape of the DCW as a function of the value | k = (1 + a·d)^4, fast segment down to 1/32 of the cycle | chosen curve, not measured on a CZ |
