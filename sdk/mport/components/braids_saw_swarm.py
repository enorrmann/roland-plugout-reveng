"""Braids SAW_SWARM (braids/digital_oscillator.cc, DigitalOscillator::RenderSawSwarm) on our DSP.

Original, per block of 24 samples (control rate) and per sample (audio rate):

    detune = ((p1 + 1024)^2 >> 9);  increments[i] = inc(pitch_ + detune (i - 3) / 65536), i = 0..6
    hp_cutoff = pitch_ + ((p2 - 10922) * 24 >> 5)  (p2 < 10922)  or  ((p2 - 10922) * 12 >> 5);  f = lut_svf_cutoff
    per sample:  phase_i += increments[i];  sample = -28672 + sum(phase_i >> 19)
                 sample = Interpolate88(ws_moderate_overdrive, sample + 32768)        (tanh(2x) table)
                 notch = sample - bp damp;  lp += f bp, CLIP;  hp = notch - lp;  bp += f hp;  out = CLIP(hp)

Port (docs/MUTABLE_SDK.md §7):

    control rate -> Params:  ratio (6 values: increment ratios 2^(k detune) of the saws k = -3..-1, 1..3 to the centre)
                             hp (H = pi 2^(offset / 12) / 2: the high-pass coefficient per unit of increment)
    audio rate   -> parts:   osc(k, inc, ratio, phases) -> x: the 7 saws, x = sum(p_k) / 8 (= sample / 32768
                             exactly), waveshaper = odd polynomial fit of ws_moderate_overdrive (degree 9 by
                             default: 21 LSB worst case); hp(k, x, inc, H, lp, bp) -> y: f = 2 sin(min(u, pi/8)),
                             u = inc H (the Chamberlin coefficient of lut_svf_cutoff), damp = lut_svf_damp[0]

Differences from the original, by design: no strike (the phases are not randomised at a note start; they run free
across notes; Braids randomises 6 of them), no sync input, the increments come from the host's exp2 of the pitch
(make_synth.py's form) instead of lut_oscillator_increments, and the 1/128-semitone interpolation of lut_svf_cutoff
is the exact exponential. Not reproduced either: Braids' int32 overflow in the increment interpolation
((increment_b - increment_a) x detune_fractional, from about MIDI note 92 up: the lower saws get wrong increments;
int_model(wrap=True) has it, bit-exact with the C++). Measured against the original:
`python3 sdk/mport/cli.py check saw_swarm`.
"""
import math

import lut
import mi
from component import Component, Param

KS = (-3, -2, -1, 1, 2, 3)                 # saws with a ratio parameter (k = 0 is the centre: the increment itself)
DAMP = 65534 / 32768.0                     # lut_svf_damp[0] (resonance 0): bp x damp >> 15
F_SCALE = 32767 / 32768.0                  # lut_svf_cutoff stores f x 32767, used as f x bp >> 15
U_MAX = math.pi / 8                        # f = 2 sin(pi fc / fs) with fc / fs <= 1/8 (lookup_tables.py)
U_MIN = math.pi * mi.note_hz(0) / mi.FS    # hp_cutoff >= 0: MIDI note 0 (8.18 Hz)


def detune_units(p1):
    """Braids' detune at parameter p1: (p1 + 1024)^2 >> 9, in 1/65536 of a pitch unit (1/128 semitone)"""
    d = p1 + 1024
    return (d * d) >> 9


def ratios(p1):
    """increment of saw k over the centre's: 2^(k detune / (65536 x 128 x 12)) for k in KS"""
    s = detune_units(p1) / 65536.0 / 128.0          # semitones per saw step
    return tuple(2 ** (k * s / 12.0) for k in KS)


def hp_offset_semitones(p2):
    off = ((p2 - 10922) * 24) >> 5 if p2 < 10922 else ((p2 - 10922) * 12) >> 5
    return off / 128.0


def hp_coef(p2):
    """H = pi 2^(offset / 12) / 2: u = pi fc / fs = H x inc, inc = 2 f_note / fs (our phase step)"""
    return math.pi * 2 ** (hp_offset_semitones(p2) / 12.0) / 2


class SawSwarm(Component):
    name = 'saw_swarm'
    source = 'braids/digital_oscillator.cc DigitalOscillator::RenderSawSwarm'
    params = (Param('ratio', lambda v: ratios(mi.param(v)), count=len(KS),
                    doc='TIMBRE (detune): increment ratios of the saws k = -3..-1, 1..3'),
              Param('hp', lambda v: hp_coef(mi.param(v)), doc='COLOR: high-pass coefficient H'))
    state = ('p-3', 'p-2', 'p-1', 'p0', 'p1', 'p2', 'p3', 'lp', 'bp')
    inputs = ('inc',)

    def __init__(self, ws_degree=9):
        self.ws_degree = ws_degree
        self.ws, self.ws_err = lut.fit_odd(lut.interp88(lut.table('braids', 'ws_moderate_overdrive')), ws_degree,
                                           x_max=0.875)

    def osc(self, k, inc, ratio, phases):
        """the 7 saws and the waveshaper. inc: Value (phase step of the centre saw); ratio(i): Value of
        ratio i (KS order); phases: 7 Rings (k = -3..3) -> Value in -1..1"""
        x = None
        for j, kk in enumerate(range(-3, 4)):
            step = inc if kk == 0 else k.mul(inc, ratio(KS.index(kk)), f'inc{kk:+d}')
            p = k.wrap(k.add(phases[j].value, step), f'p{kk:+d}')
            phases[j].write(p)
            x = k.mul(p, 0.125, 'x') if x is None else k.mad(x, p, 0.125, 'x')   # x = sum / 8 = sample / 32768
        k.comment('waveshaper: odd polynomial of ws_moderate_overdrive')
        return k.poly_odd(x, self.ws, 'ws')

    def hp_f(self, k, inc, H):
        """the Chamberlin coefficient f = 2 sin(u) (x 32767 / 32768), u = clamp(inc H, U_MIN, U_MAX)"""
        u = k.max(k.min(k.mul(inc, H, 'u'), U_MAX), U_MIN)
        return k.poly_odd(u, (2 * F_SCALE, -F_SCALE / 3), 'f')

    def hp(self, k, x, f, lp, bp):
        """one sample of Braids' high-pass (damp lut_svf_damp[0]): x, f Values; lp, bp Rings -> Value"""
        b = bp.value
        notch = k.mad(x, b, -DAMP, 'notch')
        lpn = k.clamp(k.mad(lp.value, f, b, 'lp'))
        lp.write(lpn)
        h = k.sub(notch, lpn, 'hp')
        bp.write(k.mad(b, f, h, 'bp'))
        return k.clamp(h, 'out')

    def kernel(self, k, ins, prm, st):
        x = self.osc(k, ins['inc'], prm['ratio'], [st[s] for s in self.state[:7]])
        f = self.hp_f(k, ins['inc'], prm['hp'](0))
        return {'out': self.hp(k, x, f, st['lp'], st['bp'])}

    def reference(self, note, knobs, n):
        import reference
        return reference.braids('saw_swarm', note, mi.param(knobs.get('ratio', 128)), mi.param(knobs.get('hp', 0)),
                                n, strike=False)

    def model(self, note, knobs, n, wrap=True):
        """int_model() with knob values"""
        return int_model(note, mi.param(knobs.get('ratio', 128)), mi.param(knobs.get('hp', 0)), n, wrap)


def int_model(note, p1, p2, n, wrap=True):
    """the original algorithm in Python integers, bit-exact with the C++ render (reference.braids(strike=False)):
    the third leg of the check (C++ = this model, then this model against the port). wrap=False drops Braids' int32
    overflow of (increment_b - increment_a) x detune_fractional (from about MIDI note 92 up, depending on the
    detune: the lower saws get a wrong increment) -> the behaviour the port implements"""
    inc_t = lut.table('braids', 'lut_oscillator_increments')
    cut, ws = lut.table('braids', 'lut_svf_cutoff'), lut.table('braids', 'ws_moderate_overdrive')

    def s32(x):
        return ((x + 2 ** 31) % 2 ** 32) - 2 ** 31

    def increment(p):                                   # DigitalOscillator::ComputePhaseIncrement
        p = min(p, 128 * 128 - 1)
        r, s = p - 128 * 128, 0
        while r < 0:
            r, s = r + 1536, s + 1
        a, b = inc_t[r >> 4], inc_t[(r >> 4) + 1]
        return ((a + (s32((b - a) * (r & 15)) >> 4)) & 0xFFFFFFFF) >> s

    pitch, d = int(round(note * 128)), detune_units(p1)
    incs = []
    for i in range(7):
        sd = d * (i - 3)
        a, b = increment(pitch + (sd >> 16)), increment(pitch + (sd >> 16) + 1)
        pr = (b - a) * (sd & 0xFFFF)
        incs.append((a + ((s32(pr) if wrap else pr) >> 16)) & 0xFFFFFFFF)
    hpc = pitch + (((p2 - 10922) * 24) >> 5 if p2 < 10922 else ((p2 - 10922) * 12) >> 5)
    ph = (max(0, min(32767, hpc)) << 17) & 0xFFFFFFFF
    i8 = ph >> 24
    f = cut[i8] + (((cut[i8 + 1] - cut[i8]) * ((ph >> 8) & 0xFFFF)) >> 16)
    phs, lp, bp, out = [0x80000000] * 7, 0, 0, []
    for _ in range(n):
        s = -28672
        for i in range(7):
            phs[i] = (phs[i] + incs[i]) & 0xFFFFFFFF
            s += phs[i] >> 19
        idx = (s + 32768) & 0xFFFF
        a, b = ws[idx >> 8], ws[(idx >> 8) + 1]
        smp = a + (((b - a) * (idx & 255)) >> 8)
        notch = smp - (s32(bp * 65534) >> 15)
        lp = max(-32767, min(32767, lp + ((f * bp) >> 15)))
        hp = notch - lp
        bp += (f * hp) >> 15
        out.append(max(-32767, min(32767, hp)) / 32768.0)
    return out
