"""The model of one BMC slave (six DSPs) and of the two slaves, for the voice chain studies of docs/VOICE_ALLOCATION.md:
Model (the hardware hypotheses), Program (a decoded program), access_violations, helpers. The emulator itself is the
C core (cchain.py / dspcore.c); the Python Slave of this file was archived 2026-10-29 (archive/chain.py).

The banks, per DSP (dspcore.h rd / wr):

    bank 0 (0nnn)  ring of 0x800 words: what is stored at R in sample n is read at R+1 in sample n+1
                   [medido, DSP_HARDWARE.md 7.2]
    bank 1 (2nnn)  0x800 words; a ring like bank 0 when Model.ring1 (hypothesis of question D), else plain memory;
                   Model.shared1 = pairs of DSPs with one bank 1 (question E)
    bank 3 (6nnn)  IOMD: one block for the six DSPs; writes persist [medido, 8.3]. Ownership is not modelled.
    bank 4 (8nnn)  MXM: point-to-point, latched once per sample, 1 sample latency, no hold (a word nobody wrote
                   in the previous sample reads 0) [medido en el maestro, 8.2]. Inputs of DSP r (r >= 2):
                   M[i] <- Z[0x400 + i - Model.mxm_shift] of the even DSP of the previous pair, O[i] <- Z of the
                   odd one; S[0x600 + i] of DSP r <- writes of DSP r+2 to M (r even) or O (r odd) [listado].
    bank 5 (Annn)  0x000-0x3FF: own persistent memory (DPC) [medido]. 0x400-0x7FF: Model.dp_pairs -> one window
                   per pair (0,1), (2,3), (4,5) (DP01 / DP23 / DP45), latched like MXM when Model.dp_delay = 1
                   (hypothesis of question B); otherwise own memory.
    bank 6 (Cnnn)  parameters, read only: Slave.params (default 0).

Inside one sample the DSPs run in Model.order; in DSP2 / DSP3 the main program runs before the slot programs
when Model.main_first (question F). Registers, flags and hidden registers persist per DSP.

What is NOT modelled: the ARM timing (engine writes are injected between samples), the effect slots, bank 2
(delay lines), the wave bus beyond the two-slave link of Model.link (question C), the real IOMD ownership.

The emulator contains no Roland code: the programs are passed in by the caller. Today the tests pass ours (v3/make_v3.py,
v4/make_v4.py); running Roland's programs, loaded at run time from the local copy, is allowed for study (the
no-Roland-microcode rule applies only to our own plug-outs, docs/log/VOICE_ALLOCATION_LOG.md §7).
"""
import os
import sys
from dataclasses import dataclass, field

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
for p in (os.path.join(ROOT, 'examples', 'from_scratch', 're'), os.path.join(ROOT, 'asm')):
    if p not in sys.path:
        sys.path.insert(0, p)

import dspdis  # noqa: E402
import dspref  # noqa: E402

FS = 96000.0
RING = 0x800


@dataclass
class Model:
    """the hardware hypotheses; the defaults are the model that agrees with every measurement (docs/log/VOICE_ALLOCATION_LOG.md §6)"""
    ring1: bool = True                 # D: bank 1 is a ring like bank 0
    main_first: bool = True            # F: main program before the slot programs, in the same DSP
    mxm_shift: int = 0                 # G: Z[0x400+i] -> M[i + shift]
    mxm_delay: int = 1                 # MXM latency in samples (1 measured on the master)
    shared1: tuple = ()                # E: pairs of DSPs with one bank 1, e.g. ((2, 3),)
    shared5: tuple = ()                # pairs of DSPs with one bank 5 0x000-0x3FF (SYSTEM-1 slave A / B: ((0, 1),))
    edm: tuple = ()                    # the delay line: (dsp with the window, base 324, latency 2, words 983040)
    own_params: tuple = ()             # DSPs with their own bank 6 (SYSTEM-1: FCEC(sel, ...) per DSP)
    dp_pairs: bool = True              # B: bank 5 0x400-0x7FF shared by the pair
    dp_delay: int = 1                  # B: latency of the pair window (1 = latched like MXM)
    order: tuple = (0, 1, 2, 3, 4, 5)  # DSP order inside the sample
    link: dict = field(default_factory=dict)   # C: {slave-1 IOMD word: slave-0 IOMD word}, 1 sample late


STRICT = True     # Program refuses code that breaks the hardware's per-instruction access limits (access_violations)


def access_violations(ins_list):
    """[(instruction index, what)] for instructions that break the per-instruction memory-access limits of the DSP
    (docs/DSP_HARDWARE.md 7.3): bank-0 loads <= 2 (more: only the last is valid), bank-0 stores <= 2 (more never
    appears in Roland's code; 3 hung the SYSTEM-1, C-S1JXH MAIN3 instruction 55, 2026-10-23), bank-6 loads <= 2 (the
    same: more never appears; 3 hung the SYSTEM-1, C-S1JXH1 MAIN1 instruction 48), bank-5 loads <= 1 and
    bank-5 stores <= 1 (more: none is done; on the SYSTEM-1 a program with such instructions hung the device, C-S1JXQ
    2026-10-22, docs/examples_system_1_jx_README.md section 10), transfers <= 6. The interpreter does not model these limits (it
    performs every access), so a program that breaks them would pass the emulator and fail on the device: hence the
    check. Literal (mode fe) and indexed transfers (X28Y bank 0, XdaY/Xd0Y.. bank 5) are counted."""
    out = []
    for k, ins in enumerate(ins_list):
        try:
            L = dspref.layout(list(ins))
        except ValueError:
            continue
        slot_pos = [i for i, t in enumerate(L.t) if t in 'NS']
        n0 = s0 = l5 = s5 = l6 = nt = 0
        for w, t in zip(L.body, L.t):
            if t != 'T':
                continue
            mode, store = (w >> 4) & 0xFF, w >> 12 >= 8
            if mode == 0x9B:                                  # condition flags: no memory
                continue
            nt += 1
            if mode == 0xFE:
                s = (w & 15) >> 1
                bank = L.body[slot_pos[s]] >> 12 if s < len(slot_pos) else None
            elif mode == 0x28:
                bank = 0x0
            elif mode in (0xDA, 0xDE, 0xDB, 0xDF) or 0xD0 <= mode <= 0xD7:
                bank = 0xA
            else:
                continue
            if bank == 0x0 and not store:
                n0 += 1
            elif bank == 0x0:
                s0 += 1
            elif bank == 0xC and not store:
                l6 += 1
            elif bank == 0xA:
                if store:
                    s5 += 1
                else:
                    l5 += 1
        bad = [m for m, c in (('bank-0 loads %d > 2' % n0, n0 > 2), ('bank-0 stores %d > 2' % s0, s0 > 2),
                              ('bank-5 loads %d > 1' % l5, l5 > 1), ('bank-6 loads %d > 2' % l6, l6 > 2),
                              ('bank-5 stores %d > 1' % s5, s5 > 1), ('transfers %d > 6' % nt, nt > 6)) if c]
        if bad:
            out.append((k, ', '.join(bad)))
    return out


class Program:
    """a decoded program: instruction list and word offsets (for jumps). With STRICT (default), a program that breaks
    the per-instruction access limits of the hardware is refused (access_violations)."""

    def __init__(self, words, name='', strict=None):
        self.name = name
        self.ins = [i for _, i in dspdis.instructions(words)]
        if STRICT if strict is None else strict:
            bad = access_violations(self.ins)
            if bad:
                raise ValueError(f'{name or "program"}: breaks the DSP access limits at {len(bad)} instructions, '
                                 f'first {bad[:3]}')
        self.pos = {}
        p = 0
        for k, i in enumerate(self.ins):
            self.pos[p] = k
            p += len(i)
        self.start = sorted(self.pos)


class Bmc2:
    """two slaves; Model.link carries IOMD words of slave 1 into slave 0, one sample late (question C). The slaves
    are cchain.Slave (cchain.Bmc2 builds them)"""

    def step(self):
        out1 = {a: self.s[1].iomd.get(a, 0.0) for a in self.m.link}
        for s in self.s:
            s.step()
        for a, b in self.m.link.items():
            self.s[0].iomd[b] = out1[a]


# ---------------- helpers ----------------
AUDIBLE = (0x6020, 0x6021)          # the IOMD words heard at the output (slave 0, docs/WAVE_BUS.md 4b) [medido]


def freq(seq, fs=FS):
    """frequency from the upward zero crossings (mean period, linear interpolation); 0 if fewer than 3"""
    m = sum(seq) / len(seq) if seq else 0.0
    xs = []
    for t in range(1, len(seq)):
        a, b = seq[t - 1] - m, seq[t] - m
        if a < 0 <= b and b != a:
            xs.append(t - 1 + (-a) / (b - a))
    if len(xs) < 3:
        return 0.0
    return (len(xs) - 1) * fs / (xs[-1] - xs[0])


def note_hz(note):
    """the pitch the engine writes, (note - 12) / 12, as the saw plays it: 440 Hz at 4.75 (A4 = note 69)"""
    return 440.0 * 2 ** ((note - 69) / 12.0)


def peak(seq):
    return max((abs(x) for x in seq), default=0.0)
