"""Mutable Instruments' fixed-point conventions -> the number model of our DSP (docs/MUTABLE_SDK.md §2).

Our DSP computes with signed numbers up to +-512 (29-bit mantissa, results truncated toward zero; DSP_HARDWARE.md),
at 96 kHz like Braids. Each MI quantity has one place in that model:

  MI (Braids / Plaits)                          here
  sample int16 (-32768..32767)                  float -1..1 (x = s / 32768)
  phase uint32 (0..2^32), saw = phase >> 19     phase -1..1 wrapped by ph_wrap: p = phase / 2^31 - 1
  phase increment uint32                        2 f / fs (the step of p)
  pitch int16, MIDI note << 7 (1/128 semitone)  our pitch word: (note - 12) / 12 octaves (4.75 = A 440)
  parameter (TIMBRE / COLOR) 0..32767           SYSTEM-1 knob 0..255 -> param(v) = v x 32767 / 255
  a * b >> 15 (Q15 product)                     a x b with a, b as floats (/ 32768)
  CLIP(x) to int16                              ph_clamp (to -1..1)
  table lookups                                 lut.py: polynomial (audio rate) or ARM table (control rate)

Control-rate code of a Render() call (what Braids computes once per block of 24 samples from pitch_ and parameter_)
runs at build time in Python, at every knob value, and reaches the DSP as parameters C0xx from ARM tables.
"""
import math

FS = 96000.0
A4_PITCH_WORD = 4.75                     # our pitch word of A 440 ((69 - 12) / 12)


def param(v):
    """SYSTEM-1 knob 0..255 -> MI parameter 0..32767"""
    return int(round(v * 32767 / 255))


def knob(p):
    """MI parameter 0..32767 -> nearest knob value 0..255"""
    return int(round(p * 255 / 32767))


def braids_pitch(note):
    """MIDI note -> Braids pitch_ (1/128 semitone)"""
    return int(round(note * 128))


def pitch_word(note):
    """MIDI note -> our pitch word (octaves; engine.c pitch_bits)"""
    return (note - 12) / 12.0


def increment(hz):
    """frequency -> our phase step (phase -1..1)"""
    return 2 * hz / FS


def note_hz(note):
    return 440.0 * 2 ** ((note - 69) / 12.0)


def phase_from_u32(w):
    """MI phase word -> our phase in [-1, 1)"""
    return (w & 0xFFFFFFFF) / 2 ** 31 - 1.0


def semitones_from_pitch(units):
    """Braids pitch units (1/128 semitone) -> semitones"""
    return units / 128.0


def ratio(semitones):
    return 2 ** (semitones / 12.0)


def on_grid(x):
    """x on the register grid (multiples of 2^-32 below 0.125, dspisa.quant), so a parameter used by the instruction
    right after its load gives the same bits packed and unpacked (MICROCODE_OPTIMIZATION.md rule 6)"""
    return math.floor(x * 2 ** 32) / 2 ** 32 if abs(x) < 0.125 else x


class Lcg:
    """stmlib::Random (the reference renderer's copy, sdk/mport/ref/stmlib/utils/random.h)"""

    def __init__(self, seed=1):
        self.s = seed & 0xFFFFFFFF

    def word(self):
        self.s = (self.s * 1664525 + 1013904223) & 0xFFFFFFFF
        return self.s
