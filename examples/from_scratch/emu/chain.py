"""Emulator of one BMC slave (six DSPs) and of the two slaves, for the voice chain studies of docs/VOICE_ALLOCATION.md.

Every memory access of re/dspisa.py goes through dspisa.ACCESS, so the banks are modelled here, per DSP:

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
import dspisa  # noqa: E402

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
            L = dspisa.Layout(list(ins))
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


class Dsp:
    def __init__(self, n):
        self.n = n
        self.regs = [0.0] * 8
        self.flags = None                  # dspisa.FLAGS / PENDING / HIDDEN of this DSP
        self.pending = {}
        self.hidden = dict(dspisa.HIDDEN)
        self.bank0 = [0.0] * RING
        self.bank1 = [0.0] * RING          # replaced by the partner's list when shared
        self.bank5 = {}                    # 0x000-0x3FF
        self.own_hi = {}                   # 0x400-0x7FF when dp_pairs is off, and own writes inside the sample
        self.mxm_in = {}                   # latched inputs for this sample: address -> value (M, O, S zones)
        self.mxm_out = {}                  # writes of this sample: address (8nnn) -> value
        self.progs = []                    # [Program], in execution order (main first or slots first)
        self.reads = set()                 # every address read (for coverage checks)


class Slave:
    """one BMC: six DSPs. programs = {dsp: (main Program, [slot Programs])}"""

    def __init__(self, programs, model=None, index=0):
        self.m = model or Model()
        self.index = index
        self.d = [Dsp(n) for n in range(6)]
        for a, b in self.m.shared1:
            self.d[b].bank1 = self.d[a].bank1
        for n, (main, slots) in programs.items():
            self.d[n].progs = [main] + list(slots) if self.m.main_first else list(slots) + [main]
        self.iomd = {}
        self.params = {}
        self.dp = {k: {} for k in range(3)}         # pair window as seen this sample
        self.dp_next = {k: {} for k in range(3)}    # writes of this sample (dp_delay 1)
        self.t = 0
        self.cur = None

    # ---------------- ARM side ----------------
    def p_bank(self, dsp, ident, value):
        """OS +0xB0: bank 5 word `ident` of DSP `dsp` (engine.c p_bank)"""
        self.d[dsp].bank5[ident & 0x3FF] = float(value)

    # ---------------- memory ----------------
    def _ring(self, a):
        return (a - self.t) % RING

    def _access(self, op, a, _mem, v=None):
        d = self.cur
        bank = a >> 13
        off = a & 0x1FFF
        if op == 'r':
            d.reads.add(a)
        if bank == 0:
            i = self._ring(off & 0x7FF)
            if op == 'r':
                return d.bank0[i]
            d.bank0[i] = v
        elif bank == 1:
            i = self._ring(off & 0x7FF) if self.m.ring1 else off & 0x7FF
            if op == 'r':
                return d.bank1[i]
            d.bank1[i] = v
        elif bank == 3:
            if op == 'r':
                return self.iomd.get(a, 0.0)
            self.iomd[a] = v
        elif bank == 4:
            if op == 'r':
                return d.mxm_in.get(a, d.mxm_out.get(a, 0.0) if 0x8400 <= a < 0x8600 else 0.0)
            d.mxm_out[a] = v
        elif bank == 5:
            i = off & 0x7FF
            if i < 0x400:
                if op == 'r':
                    return d.bank5.get(i, 0.0)
                d.bank5[i] = v
            elif not self.m.dp_pairs:
                if op == 'r':
                    return d.own_hi.get(i, 0.0)
                d.own_hi[i] = v
            else:
                pair = d.n >> 1
                if op == 'r':
                    if i in d.own_hi:                       # own write earlier in this sample
                        return d.own_hi[i]
                    return self.dp[pair].get(i, 0.0)
                d.own_hi[i] = v
                if self.m.dp_delay:
                    self.dp_next[pair][i] = v
                else:
                    self.dp[pair][i] = v
        elif bank == 6:
            if op == 'r':
                return self.params.get(a, 0.0)
        else:
            if op == 'r':
                return 0.0
        return None

    # ---------------- MXM latch ----------------
    def _latch(self):
        """inputs of the next sample from the writes of this one (delay 1)"""
        new = {n: {} for n in range(6)}
        for w in range(6):
            for a, v in self.d[w].mxm_out.items():
                z = a & 0x7FF
                if 0x400 <= z < 0x600:                  # Z -> M (even writer) / O (odd writer) of the next pair
                    i = (z - 0x400 + self.m.mxm_shift) & 0x1FF
                    for r in (2 * (w // 2 + 1), 2 * (w // 2 + 1) + 1):
                        if r < 6:
                            new[r][0x8000 + (0x200 if w & 1 else 0) + i] = v
                elif z < 0x400:                          # M / O -> S of the previous pair
                    r = (w // 2 - 1) * 2 + (1 if z >= 0x200 else 0)
                    if r >= 0:
                        new[r][0x8600 + (z & 0x1FF)] = v
        return new

    # ---------------- one sample ----------------
    def _run_prog(self, d, p):
        i, steps = 0, 0
        while i < len(p.ins):
            ins = p.ins[i]
            steps += 1
            if steps > 100000:
                raise RuntimeError('endless loop in ' + p.name)
            dspisa.JUMP = None
            d.regs, _, _ = dspisa.run(list(ins), d.regs, {})
            if dspisa.JUMP is not None:
                i = p.pos[p.start[i] + len(ins) + dspisa.JUMP]
            else:
                i += 1

    def step(self):
        dspisa.ACCESS = self._access
        if self.m.mxm_delay == 0:
            for d in self.d:
                d.mxm_in = {}
        for n in self.m.order:
            d = self.d[n]
            if self.m.mxm_delay == 0:                     # same-sample MXM: what earlier DSPs wrote now
                d.mxm_in = self._latch()[n]
            self.cur = d
            dspisa.FLAGS, dspisa.PENDING = d.flags, d.pending
            dspisa.HIDDEN.clear()
            dspisa.HIDDEN.update(d.hidden)
            for p in d.progs:
                self._run_prog(d, p)
            d.flags, d.pending, d.hidden = dspisa.FLAGS, dspisa.PENDING, dict(dspisa.HIDDEN)
        if self.m.mxm_delay:
            new = self._latch()
            for n in range(6):
                self.d[n].mxm_in = new[n]
        for n in range(6):
            self.d[n].mxm_out = {}
            self.d[n].own_hi = {}
        if self.m.dp_pairs and self.m.dp_delay:
            self.dp = self.dp_next
            self.dp_next = {k: {} for k in range(3)}
        dspisa.ACCESS = None
        self.t += 1


class Bmc2:
    """two slaves; Model.link carries IOMD words of slave 1 into slave 0, one sample late (question C)"""

    def __init__(self, programs, model=None):
        self.m = model or Model()
        self.s = [Slave(programs, self.m, 0), Slave(programs, self.m, 1)]

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
