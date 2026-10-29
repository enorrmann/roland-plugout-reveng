"""SYSTEM-S1CC: a SYSTEM-1 PLUG-OUT that is only an s1cc kernel (docs/examples_system_s1cc_README.md).

No oscillator, envelope, filter or LFO of its own: everything a voice does is the kernel's C (sdk/s1cc). Around it
only what the device needs, in the forms MUTABLE-SYSTEM / C-S1SY13 proved on the hardware:

    slave A  s1m0 @0x0100   per voice v: DSP A of the kernel (S1_DSP_A); pitch / gate / trigger read from bank 5
                            (A030+v / A038+v / A060+v, written by engine.c); its output -> A151+v
    slave B  s1m1 @0x0100   first the mix: sum of last sample's voice outputs x gain x C000 -> 6006 / 6007
                            (6008..600D = 0); then per voice v: DSP B of the kernel (S1_DSP_B) on x = A151+v ->
                            its output word; without S1_DSP_B the mix reads A151+v
    master B s1mm1 @0x1000  6006 / 6007 x VOLUME (C01C) -> OS effect send 2000 / 2002 (delay, reverb); the effect's
                            return 2001 / 2003 -> A118 / A119 (C-S1SY13's master, unchanged)
    knobs                   the kernel's S1_PARAM tables, by panel id: DSP A parameters from C070, DSP B from C060

    KERNEL=dir (required)   an s1cc build directory (sdk/s1cc/s1cc build ... -o dir)
    VOICES=n                1..8 (default 8). build.sh sets it: the most voices that fit (it tries 8, 7, ...)
    BUDGET_A=n, BUDGET_B=n  instructions per sample the slave's DSP A / DSP B may have, measured on the device
                            (docs/examples_system_1_synth_README.md §4): DSP A 703 ran (MP-CSAW 8 voices,
                            2026-10-28), 826 hung; DSP B 708 ran. Defaults 703 / 708. BUDGET=n sets both
    MAIN_BYTES=n            the size each slave main program may have: 6144 bytes, its region 0x0100..0x18FF before
                            section 2 (docs/DSP_PROGRAM_MEMORY.md §3.1); 5854 bytes ran, 6534 and 6740 hung the
                            device (2026-10-28). Above a budget or the size the build stops (exit code 3)
    OUTDIR=dir              where the programs and build/s1_programs.h go (default: this folder)

    python3 make_s1cc.py           -> s1*.s / s1*.hex here, build/s1_programs.h
    python3 make_s1cc.py --sim     -> also the whole chain on the emulator (C core) against the kernel's native
                                      reference (sdk/s1cc refdsp), and the silence without notes
"""
import collections
import math
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'sdk', 's1cc', 'tools'))
import asm  # noqa: E402
import dspsched  # noqa: E402
import s1kernel  # noqa: E402
import s1rules  # noqa: E402  (asm/s1rules.py: the SYSTEM-1 access rules)

KERNEL = os.environ.get('KERNEL', '')
if not KERNEL or not os.path.exists(os.path.join(KERNEL, 'voice.s1k')):
    raise SystemExit('make_s1cc.py: KERNEL=<s1cc build directory> (with voice.s1k) is required')
NV = int(os.environ.get('VOICES', '8'))
if not 1 <= NV <= 8:
    raise SystemExit('make_s1cc.py: VOICES must be 1..8 (the OS hands notes to 8 voice ids at most)')
BUDGET = {'s1m0': int(os.environ.get('BUDGET_A', os.environ.get('BUDGET', '703'))),    # DSP A: 703 ran, 826 hung
          's1m1': int(os.environ.get('BUDGET_B', os.environ.get('BUDGET', '708')))}    # DSP B: 708 ran
MAIN_BYTES = int(os.environ.get('MAIN_BYTES', '6144'))  # 0x0100..0x18FF: 5854 bytes ran, 6534 / 6740 hung
OUTDIR = os.environ.get('OUTDIR', HERE)        # where the .s / .hex and build/s1_programs.h go (tests: elsewhere)
KDEF = s1kernel.Kernel.read(os.path.join(KERNEL, 'voice.s1k'))
KTAB = s1kernel.read_tables(os.path.join(KERNEL, 'tables.txt'))
HAS_B = 'B' in KDEF.dsps
# the host's envelopes (s1_env(0) / s1_env(1) in S1_DSP_B): two ADSRs in assembly per voice on DSP B, C-S1SY's
HOST_ENV_KERNEL = [i for i in (0, 1) if HAS_B and f'IN:env{i}' in KDEF.placeholders('B')]   # the ones it reads
# s1_color(i) in S1_DSP_A: OSC1 / OSC2 COLOR with its COLOR MOD source, made on DSP A (hlfo_voice):
# MANUAL = the knob, else knob x source (0..1): an LFO, a pitch envelope (AD) and a sub oscillator per voice on
# DSP A, the AMP and FILTER envelopes (the host's ADSRs on DSP B, then both are built) copied to bank 5 by DSP B
A_READS = set(KDEF.placeholders('A'))
COLOR_USED = [i for i in (0, 1) if f'IN:color{i}' in A_READS]
CMOD = bool(COLOR_USED)
# the host's LFO (per voice on DSP A, LFO SHAPE / RATE / DELAY TIME): -> PITCH modulates the pitch, s1_dt and s1_rdt
# words the kernel reads (LFO_PITCH=0 leaves them as the ARM writes them); -> FILTER / -> AMP are s1_lfo_filter() /
# s1_lfo_amp(), for the kernel's own filter and VCA; also the LFO source of COLOR MOD
LFO_PITCH = os.environ.get('LFO_PITCH', '1') == '1'
PITCH_MOD = [n for n in ('pitch', 'dt', 'rdt') if LFO_PITCH and f'IN:{n}' in A_READS]
LFO_F, LFO_A = 'IN:lfo_filter' in A_READS, 'IN:lfo_amp' in A_READS
HOST_LFO = bool(CMOD or PITCH_MOD or LFO_F or LFO_A)
# the LFO and the pitch envelope (COLOR MOD source and PITCH ENV AMOUNT -> pitch) run on DSP B (room there; DSP A is
# for the oscillators): lf and pe go to DSP A through bank 5 (LFB, PEB), one sample later
PENV = bool(PITCH_MOD or CMOD)
# the host's mixer on DSP A, after the kernel: voice = kernel + SUB x a square one octave down + NOISE x the LFO's
# noise, before DSP B's filter (SUB 0x176, NOISE 0x177; 0 until the knobs move). MIX=0 leaves it out
HOST_MIX = HOST_LFO and os.environ.get('MIX', '1') == '1'
# inputs of DSP B the host makes on DSP A (-> amp, -> filter, the modulated pitch): the host stores them straight to
# their bank-5 word (INB), and the kernel's copy (ld IN / st INB, at the end of its DSP A code) goes away
HOSTB = [n for n in KDEF.inputs_b if n in ('lfo_amp', 'lfo_filter') or (n == 'pitch' and 'pitch' in PITCH_MOD)]


def _reads(line, reg):
    """does this kernel line read register reg (ld writes only; st, mad, ymacd read every register they name)"""
    m = line.split()
    if not m or m[0] == 'ld' or re.fullmatch(r'\s*mov (r\d), \1\s*', line):    # a load delay filler reads nothing
        return False
    regs = re.findall(r'-?r(\d)', line)
    if m[0] in ('st', 'mad', 'ymacd'):
        return str(reg) in regs
    return str(reg) in regs[1:]


def _strip_copies(lines):
    """the DSP A kernel without its copies of the HOSTB inputs: each `st rX, [{INB:n}]`, and the `ld rX, [{IN:n}]`
    feeding it when nothing else reads rX before it is written again"""
    out = list(lines)
    for n in HOSTB:
        for k, line in enumerate(out):
            m = line and re.match(r'st r(\d), \[\{INB:' + n + r'\}\]', line.strip())
            if not m:
                continue
            reg = int(m.group(1))
            out[k] = None
            j = k - 1
            while j >= 0 and (out[j] is None or not re.match(rf'(\w+) r{reg},', out[j].strip())):
                j -= 1
            if j >= 0 and out[j].strip().startswith(f'ld r{reg}, [{{IN:{n}}}]'):
                live = False
                for line2 in out[j + 1:]:
                    if line2 is None:
                        continue
                    if _reads(line2, reg):
                        live = True
                        break
                    if re.match(rf'(\w+) r{reg},', line2.strip()) and not line2.strip().startswith(('st', 'mad', 'ymacd')):
                        break
                if not live:
                    out[j] = None
            break
    return [x for x in out if x is not None]


if HOSTB:
    KDEF.dsps['A']['lines'] = _strip_copies(KDEF.dsps['A']['lines'])
A_SELF = set(KDEF.placeholders('A'))           # what the kernel's DSP A code itself reads, after the copies went
if A_SELF & {'IN:lfo_amp', 'IN:lfo_filter'}:
    raise SystemExit('make_s1cc.py: s1_lfo_amp / s1_lfo_filter in S1_DSP_A: the host LFO runs on DSP B (use them in '
                     'S1_DSP_B)')
HOST_ENV_USED = [0, 1] if CMOD else HOST_ENV_KERNEL
HOST_ENVS = bool(HOST_ENV_USED)
GT_B = HOST_ENVS or HOST_LFO                  # DSP A copies gate and trigger for DSP B (its envelopes, its LFO)
ENV_TMIN = float(os.environ.get('ENV_TMIN', '0.001'))    # seconds at fader 0 (attack, decay, release)
ENV_TMAX = float(os.environ.get('ENV_TMAX', '10.0'))     # seconds at fader 255; exponential in between (C-S1SY's)

FS = 96000.0
# bank 5 (shared by slave A and B): written by engine.c
PITCH = [0xA030 + v for v in range(NV)]     # (note - 12) / 12 octaves (4.75 = A 440)
GATE = [0xA038 + v for v in range(NV)]      # 1.0 / 0.0
TRIG = [0xA060 + v for v in range(NV)]      # toggles 0 / 1 at every note start
DT = [0xA040 + v for v in range(NV)]        # s1_dt(): f / 96000 of the note (engine.c, per-note table)
RDT = [0xA048 + v for v in range(NV)]       # s1_rdt(): 96000 / f / 16384 (the DSP holds at most 512)
# bank 5, written by DSP A (a DSP stores only to A100..A1FF)
XV = [0xA151 + v for v in range(NV)]        # DSP A output of voice v = the input x of DSP B
X0, INB0 = 0xA188, 0xA1A0                   # exports i: X0 + 8 i + v; inputs B needs from A: INB0 + 8 j + v
# bank 5 A100..A1FF (what a slave DSP may store): A151.. x, A160.. / A168.. gate / trigger copies, A188..A19F export
# slots 0..2, A1A0..A1B7 the inputs for B (pitch, gate, trigger), A1B8..A1FF free: export slots 3..11


def export_addr(i, v):
    """bank-5 word of export slot i, voice v (slots 0..2 where MUTABLE-SYSTEM has them, then the free words)"""
    base = X0 + 8 * i if i < 3 else 0xA1B8 + 8 * (i - 3)
    if i < 0 or base + 7 > 0xA1FF:
        raise SystemExit(f'make_s1cc.py: s1_export({i}): SYSTEM-S1CC has export slots 0..11 (bank 5 A100..A1FF)')
    return base + v
# bank 0 (ring, S / S+1): the voice blocks of the kernel, sized by what it needs, in the regions our device-proven
# programs used (C-S1V8 / C-S1SY: slave A 0x0300.., slave B 0x0400..; MUTABLE-SYSTEM's kernel B: 0x0600..)
A_BLOCK0, B_BLOCK0 = 0x0300, 0x0600
B_OUT = [0x0400 + 2 * v for v in range(NV)]  # DSP B output of voice v (ring: read one sample later at S+1)
K_A_PARAM0, K_B_PARAM0 = 0x70, 0x60         # the kernel's knob tables: DSP A C070.., DSP B C060..
OUT_L, OUT_R = 0x6006, 0x6007               # slave B -> master (IOMD)
OUT_ZERO = [0x6008, 0x6009, 0x600A, 0x600B, 0x600C, 0x600D]
GAIN = 2.0 / (1.5 * NV)                     # the mix as C-S1SY13's (one voice at full scale: 1.33 / NV)
M_OUT = (0xA118, 0xA119)                    # master B -> the OS output program on master A
M_ZERO = [0xA11A, 0xA11B, 0xA11C, 0xA11D]
M_VOLUME = 0x1C                             # master B parameter C01C (model.c, VOLUME knob)
FX_SEND = (0x2000, 0x2002)                  # master B bank 1: the OS delay / reverb (system-1/FX_CHAIN.md §4)
FX_RET = (0x2001, 0x2003)
# host envelopes: DSP B parameters (sustain level, attack / decay / release rates), panel ids, defaults until the
# faders move (C-S1SY's amp and filter envelopes); DSP A copies gate and trigger for DSP B (B reading A038+v, which
# the ARM writes, gave silence in C-S1SY step 3)
H6_ENV = (0x40, 0x44)
H_ENV_PANEL = ((0x189, 0x187, 0x188, 0x18A), (0x184, 0x182, 0x183, 0x185))   # (S, A, D, R): AMP, FILTER
H_ENV_DEFAULT = ((255, 0, 128, 60), (0, 0, 128, 60))
GB = [0xA160 + v for v in range(NV)]
TB = [0xA168 + v for v in range(NV)]
# LUT (2026-10-29): fixed tables in bank 5 A080.. (engine.c writes them at load and refresh; the ARM writes A000..A0FF),
# read by an indexed load (hset h6 + ldb5 at 0 and 1: linear interpolation between two points)
#   LFO_LUT=1      the host LFO's shapes from tables (sine, triangle, saw, square; N = 16): lfo = w g + a T[phase]
#   ENV_CURVE=...  one curve for every host envelope (sq, cube, sqrt, smooth, lin; N = 32): level = C(ADSR); the same
#                  table for all of them, each envelope with its own state
LFO_LUT = os.environ.get('LFO_LUT', '0') == '1'
ENV_CURVE = os.environ.get('ENV_CURVE', 'none')
CURVES = {'lin': lambda x: x, 'sq': lambda x: x * x, 'cube': lambda x: x ** 3, 'sqrt': math.sqrt,
          'smooth': lambda x: x * x * (3 - 2 * x)}
if ENV_CURVE != 'none' and ENV_CURVE not in CURVES:
    raise SystemExit(f'make_s1cc.py: ENV_CURVE={ENV_CURVE}: one of none, {", ".join(CURVES)}')
CURVE = ENV_CURVE != 'none' and HOST_ENVS
LUT0, LUT_EN, LUT_LN = 0xA080, 32, 16
LUT_SHAPES = ('sine', 'tri', 'saw', 'square')


def lut_layout():
    """(values, {table: first word}) of the bank-5 tables: the envelope curve (LUT_EN + 1 points of 0..1), then the
    LFO shapes (LUT_LN + 1 points of the phase -1..1 each, as hlfo_voice_b's formulas: triangle 1 - 2 |wrap(p - 1.5)|,
    sine -sin(pi p) (the same peaks), saw p, square clamp(256 p))"""
    vals, at = [], {}
    if CURVE:
        at['env'] = LUT0 + len(vals)
        vals += [CURVES[ENV_CURVE](i / LUT_EN) for i in range(LUT_EN + 1)]
    if LFO_LUT:
        wrap = lambda x: (x + 1) % 2 - 1  # noqa: E731
        f = {'sine': lambda p: -math.sin(math.pi * p), 'tri': lambda p: 1 - 2 * abs(wrap(p - 1.5)),
             'saw': lambda p: p, 'square': lambda p: max(-1.0, min(1.0, 256 * p))}
        for name in LUT_SHAPES:
            at[name] = LUT0 + len(vals)
            vals += [f32(f[name](-1 + 2 * i / LUT_LN)) for i in range(LUT_LN + 1)]
    if len(vals) > 0x80:
        raise SystemExit(f'make_s1cc.py: LUT {len(vals)} words, room for 128 (A080..A0FF)')
    return vals, at


def lut_h(word):
    """h6 of a bank-5 word (hset: address = floor(h 16384) + Y)"""
    return (word & 0x3FF) / 16384


def lut_read(x, t, d, base, n, regs):
    """d = T(x) from the table at base (n intervals, x already in 0..n): fl = floor x, fr = x - fl, h6 = word of fl,
    two indexed loads, d = T0 + fr (T1 - T0). regs = (x, t): x is overwritten (fr), t is a scratch register; base:
    an immediate (float) or a register holding h of the first word"""
    add = f'addxy {t}, {t}, #{base!r}' if isinstance(base, float) else f'addxy {t}, {t}, {base}'
    return [f'floor {t}, {x}', f'subxy {x}, {t}, {x}', f'mul {t}, {t}, #{2.0 ** -14!r}', add, f'hset h6, {t}',
            f'ldb5 {d}, 0', f'ldb5 {t}, 1', f'subxy {t}, {d}, {t}', f'mad {d}, {x}, {t}']


H_ENV_STRIDE = 8 if CURVE else 6                # addresses of one host envelope: level, latch, last trigger (, curved)
H_ENV_WORDS = H_ENV_STRIDE if HOST_ENVS else 0  # per voice on DSP B: (level, latch, last trigger (, C(level))) x 2
# pitch envelope and COLOR MOD (CMOD) on DSP B (DSP A's time is for the oscillators): DSP B parameters C052 / C053
# pitch envelope decay / attack rates, C054 + 6 i: color i's weights (manual, LFO, pitch env, filter env, amp env,
# sub; LFO and sub are -1..1: with them selected manual = that weight = knob / 2). Bank 5, free words: the pitch
# envelope B -> A (PEB, for -> pitch), the sub square A -> B (SUBB), the colors B -> A (COLB), one sample late each
B6_PENV, B6_COL = 0x52, (0x54, 0x5A)
# ... or on DSP A (MOD_DSP=A): DSP A parameters C048 / C049 pitch envelope rates, C050 + 8 i color i's weights; the
# envelope levels DSP B -> DSP A in bank 5 (ENVX: AMP A170+v, FILTER A178+v). MOD_DSP=auto (default): make_s1cc.py
# takes B, and A when B does not fit or fits fewer voices (a kernel with a short DSP A: most of mutable_ported)
A6_PENV, A6_COL = 0x48, (0x50, 0x58)
ENVX = ([0xA170 + v for v in range(NV)], [0xA178 + v for v in range(NV)])
MOD_DSP = os.environ.get('MOD_DSP', 'auto')
if MOD_DSP not in ('A', 'B', 'auto'):
    raise SystemExit('make_s1cc.py: MOD_DSP must be A, B or auto')
MOD_B = MOD_DSP != 'A'                          # where the pitch envelope and COLOR MOD are now (main() may change it)
# COLOR MOD on DSP B by an indexed load (CMOD_IDX=1, default since it ran on the SYSTEM-1, MP-CSAW2-8 8 voices
# 2026-10-29; 0: the weighted sum of the five sources): the ARM writes
# per color w0, w and the offset of the chosen source in the voice block (/16384); per voice h6 = block + offset,
# `ldidx` reads the source, color = w0 + w src. The LFO and the sub are copied into the voice block first (bank 0).
# Indexed transfers: exact on the SYSTEM-8 master (re/dspverify.py --programs, 2026-10-29); none in Roland's
# SYSTEM-1 programs (docs/NATIVE_OPS.md)
CMOD_IDX = os.environ.get('CMOD_IDX', '1') == '1'
CMT0 = 0x0500                                   # DSP B bank 0: the table of COLOR MOD sources (CMOD_IDX), free words


def cmt(s, v):
    """the table word of COLOR MOD source s (1 LFO, 2 pitch env, 3 filter env, 4 amp env, 5 sub) of voice v: one row
    of 16 words per source, voice v at 2 v (ring words S, S + 1), so `ldidx r, 2 v` reads it with h6 at the row"""
    return CMT0 + 16 * s + 2 * v


def cmod_idx():
    """COLOR MOD by an indexed load now (MOD_B can change in main())"""
    return bool(CMOD and MOD_B and CMOD_IDX and HOST_LFO)
PEB = [0xA128 + v for v in range(NV)]
SUBB = [0xA130 + v for v in range(NV)]
COLB = ([0xA138 + v for v in range(NV)], [0xA140 + v for v in range(NV)])
# LFO SHAPE -> C042..C045 (alpha, beta, K, w): lfo = t (alpha + beta t^2) + ph_clamp(K p) + w g, t the triangle of
# the phase p, g the sample & glide value (C041 = its coefficient k; k = 1 is the sample & hold)
LFO_SHAPES = [(1.5, -0.5, 0.0, 0.0), (1.0, 0.0, 0.0, 0.0), (0.0, 0.0, 1.0, 0.0), (0.0, 0.0, 256.0, 0.0),
              (0.0, 0.0, 0.0, 1.0), (0.0, 0.0, 0.0, 1.0)]       # sine, triangle, saw, square, S&G, S&H
if LFO_LUT:                                     # (h of the shape's table, amp, -, w): S&G / S&H only w g
    _AT = {n: LUT0 + k * (LUT_LN + 1) + ((LUT_EN + 1) if CURVE else 0) for k, n in enumerate(LUT_SHAPES)}
    LFO_SHAPES = [(lut_h(_AT[n]), 1.0, 0.0, 0.0) for n in LUT_SHAPES] + [(lut_h(_AT['sine']), 0.0, 0.0, 1.0)] * 2
A6_LPITCH, A6_PAMT, A6_RANGE1 = 0x4B, 0x5E, 0x5F   # DSP A: LFO TO PITCH, PITCH ENV AMOUNT, OSC1 RANGE (octaves)
# DSP B: the LFO (rate, S&G coefficient, shape alpha / beta / K / w, fade step, -> filter, -> amp M / |M|)
B6_LRATE, B6_LSG, B6_LWAVE, B6_LDELAY, B6_LFILT, B6_LAMP, B6_LAMPABS = 0x48, 0x49, 0x4A, 0x4E, 0x4F, 0x50, 0x51
LFB = [0xA120 + v for v in range(NV)]           # bank 5: lf of voice v, DSP B -> DSP A
A6_SUBMIX, A6_NOISEMIX = 0x56, 0x57             # the host mixer's SUB / NOISE levels
# the host LFO's controls, in engine.c's order (hlfo_panel): names, panel ids, defaults until a control moves
HLFO = [('lfo_shape', 0x160, 1), ('lfo_rate', 0x162, 128), ('lfo_delay', 0x161, 0), ('lfo_pitch', 0x163, 128),
        ('lfo_filter', 0x164, 128), ('lfo_amp', 0x165, 128), ('penv_a', 0x17B, 0), ('penv_d', 0x17C, 128),
        ('color0', 0x168, 128), ('color1', 0x16D, 128), ('mod0', 0x169, 0), ('mod1', 0x16E, 0),
        ('sub_mix', 0x176, 0), ('noise_mix', 0x177, 0), ('penv_amt', 0x17A, 128), ('range1', 0x16A, 3)]
HLFO_DEFAULT = {n: d for n, _, d in HLFO}
NOISE_K = 7.31275                               # the LFO's noise (MUTABLE-SYSTEM step 9): n = wrap(K n + w), w a Weyl
NOISE_W = 166008623 / 2 ** 28                   # counter (odd multiple of 2^-28)
SG_K = 4.0                                      # sample & glide: k = min(1, SG_K x rate) (time constant = period / 8)


def host_a_words():
    """the host's words per voice on DSP A, before the kernel's (ring words): the modulated pitch / dt / rdt (written
    and read in the same sample), the sub's phase and its square, the kernel's output for the mixer"""
    w = (['pt', 'pe', 'pf'] if PENV and not MOD_B else []) + [n + 'm' for n in PITCH_MOD if f'IN:{n}' in A_SELF]
    w += ['sp'] if CMOD or HOST_MIX else []
    w += [f'color{i}' for i in COLOR_USED] if not MOD_B else []
    return w + (['xo'] if HOST_MIX else [])


def host_b_words():
    """the host LFO's words per voice on DSP B, after its envelopes': last trigger, phase, S&H, S&G, fade; -> amp
    and -> filter for the kernel (same sample); the note start and the pitch envelope (level, latch)"""
    w = ['bpt', 'lph', 'lsh', 'lsg', 'fade', 'lfa', 'lff'] + (['bd', 'pe', 'pf'] if PENV and MOD_B else []) if HOST_LFO else []
    return w


def set_mod_dsp(on_b):
    """put the pitch envelope and COLOR MOD on DSP B (True) or DSP A: the host's word counts follow"""
    global MOD_B, H_A_WORDS, H_B_WORDS
    MOD_B = on_b
    H_A_WORDS, H_B_WORDS = len(host_a_words()), len(host_b_words())


H_A_WORDS = len(host_a_words())
H_B_WORDS = len(host_b_words())
LFO_WORDS = 2                                   # after the voice blocks: a noise source (noise, Weyl counter)


# ---- the kernel's addresses ---------------------------------------------------------------------------------------
def layout():
    """{param name: (count, A base or None, B base or None)}: a table used by one DSP only gets no slot on the other"""
    used = {st: {m.split(':')[1] for m in KDEF.placeholders(st) if m.startswith('P:')} for st in KDEF.dsps}
    out, a, b = {}, K_A_PARAM0, K_B_PARAM0
    for p in KDEF.params:
        ab = bb = None
        if p['name'] in used.get('A', ()):
            ab, a = a, a + p['count']
        if p['name'] in used.get('B', ()):
            bb, b = b, b + p['count']
        out[p['name']] = (p['count'], ab, bb)
    return out


def words(dsp):
    """bank-0 words a voice of this DSP needs: state, scratch (spills), the first-sample flag"""
    host = H_ENV_WORDS + H_B_WORDS if dsp == 'B' else H_A_WORDS
    if dsp not in KDEF.dsps:                   # no S1_DSP_B: only the host's envelopes (COLOR MOD)
        return host
    st = KDEF.dsps[dsp]
    return host + sum(1 for s in KDEF.states if s['dsp'] == dsp) + st.get('scratch', 0) + bool(st.get('initflag'))


def block(dsp, v):
    """bank-0 block of voice v (2 ring words per value: S and S+1)"""
    base = A_BLOCK0 if dsp == 'A' else B_BLOCK0
    top = base + 2 * words(dsp) * NV + 2 * LFO_WORDS
    if top > 0x0800:
        raise SystemExit(f'kernel DSP {dsp}: {words(dsp)} words x {NV} voices do not fit in bank 0 '
                         f'(0x{base:04x}..0x07ff)')
    return base + 2 * words(dsp) * v


def lfo_block(dsp='A'):
    """bank-0 words of a DSP's global noise source (after its voice blocks): name -> address. DSP A: the mixer's
    NOISE; DSP B: the LFOs' sample & hold"""
    base = (A_BLOCK0 if dsp == 'A' else B_BLOCK0) + 2 * words(dsp) * NV
    return {n: base + 2 * j for j, n in enumerate(('nz', 'w'))}


def addresses(dsp, v):
    addr, k, base = {}, 0, block(dsp, v)
    if dsp == 'B' and HOST_ENVS:                  # the host's envelopes first: level, latch, last trigger each
        addr['IN:env0'], addr['IN:env1'] = (cmt(4, v), cmt(3, v)) if cmod_idx() else \
            (base + 6, base + 14) if CURVE else (base, base + 6)
        k = H_ENV_WORDS
    if dsp == 'B' and HOST_LFO:                   # then the host LFO's; -> amp / -> filter for the kernel
        hb = {n: base + 2 * (H_ENV_WORDS + j) for j, n in enumerate(host_b_words())}
        k = H_ENV_WORDS + H_B_WORDS
    if dsp == 'A' and HOST_LFO:                   # the host's words: LFO, modulated inputs, COLOR MOD
        h = {n: base + 2 * j for j, n in enumerate(host_a_words())}
        addr.update({f'IN:color{i}': COLB[i][v] if MOD_B else h[f'color{i}'] for i in COLOR_USED})
        addr.update({f'IN:{n}': h[n + 'm'] for n in PITCH_MOD if n + 'm' in h})
        if 'xo' in h:                           # the kernel's output, for the host's mixer
            addr['OUT'] = h['xo']
        k = H_A_WORDS
    for s in KDEF.states:
        if s['dsp'] == dsp:
            addr[f'S{s["index"]}'] = base + 2 * k
            k += 1
    for j in range(KDEF.dsps[dsp].get('scratch', 0)):
        addr[f'T{j}'] = base + 2 * k
        k += 1
    if KDEF.dsps[dsp].get('initflag'):
        addr['I'] = base + 2 * k
    for name, (cnt, ab, bb) in layout().items():
        pb = ab if dsp == 'A' else bb
        if pb is not None:
            for i in range(cnt):
                addr[f'P:{name}:{i}'] = 0xC000 + pb + i
    for e in KDEF.exports:
        addr[f'X:{e}'] = export_addr(e, v)
    for j, name in enumerate(KDEF.inputs_b):
        addr[f'INB:{name}'] = INB0 + 8 * j + v
        if dsp == 'B':
            addr[f'IN:{name}'] = INB0 + 8 * j + v
    if dsp == 'A':
        for key, a in (('IN:pitch', PITCH[v]), ('IN:gate', GATE[v]), ('IN:trig', TRIG[v]), ('IN:dt', DT[v]),
                       ('IN:rdt', RDT[v]), ('OUT', XV[v])):
            addr.setdefault(key, a)                 # the host's modulated words first
    else:
        addr.update({'XIN': XV[v], 'OUT': B_OUT[v]})
        if HOST_LFO:
            addr.update({'IN:lfo_amp': hb['lfa'], 'IN:lfo_filter': hb['lff']})
    return addr


# ---- programs -------------------------------------------------------------------------------------------------------
def hlfo_global(dsp='A'):
    """a DSP's global noise source (MUTABLE-SYSTEM step 11 step_noise, proven on the device): n = wrap(K n + w), w a
    Weyl counter. DSP A: the mixer's NOISE; DSP B: the LFOs' sample & hold"""
    G = lfo_block(dsp)
    return [f'; host: noise n = wrap(K n + w), w = Weyl counter (DSP {dsp})',
            f'ld r2, [{G["nz"] + 1:04x}]', f'ld r0, [{G["w"] + 1:04x}]', 'mov r1, r1', f'add r0, #{NOISE_W!r}:32',
            'ph_wrap r0, r3, r0', f'st r0, [{G["w"]:04x}]', f'mul r2, r2, #{NOISE_K!r}', 'addxy r2, r2, r0',
            'ph_wrap r2, r3, r2', f'st r2, [{G["nz"]:04x}]']


def hlfo_voice_b(v):
    """the host LFO of voice v on DSP B, before its envelopes; four registers (pairs: r4..r7 for the odd voice):
    * note start d: the trigger copy changed
    * LFO (MUTABLE-SYSTEM step 11 step_lfo_voice, proven on the device): phase restarted at the note start, own S&H
      of DSP B's noise at each wrap, own sample & glide g += k (h - g) (k = 1: sample & hold);
      lfo = w g + ph_clamp(K p) + t (alpha + beta t^2), t the triangle of the phase p (B6_LSG .. : k, alpha, beta,
      K, w per shape); fade f 0 -> 1 from the gate copy (DELAY TIME), lf = f lfo -> bank 5 LFB+v for DSP A
    * -> amp: 1 + f (M lf - |M|) (tremolo down from full level; M < 0 the inverted LFO) -> s1_lfo_amp()
    * -> filter: F lf octaves -> s1_lfo_filter()"""
    hb = {n: block('B', v) + 2 * (H_ENV_WORDS + j) for j, n in enumerate(host_b_words())}
    G = lfo_block('B')
    a, b, c, d = 'r0', 'r1', 'r2', 'r3'
    P = lambda k: f'c0{k:02x}'  # noqa: E731
    return [f'; voice {v}: host LFO (DSP B), b = d = 1 at a note start',
            f'ld {a}, [{TB[v]:04x}]', f'ld {b}, [{hb["bpt"] + 1:04x}]', f'mov {a}, {a}', f'st {a}, [{hb["bpt"]:04x}]',
            f'subxy {b}, {b}, {a}', f'mul {b}, {b}, {b}', *([f'st {b}, [{hb["bd"]:04x}]'] if PENV and MOD_B else []),
            f'; voice {v}: LFO phase a, 0 at a note start; c = 2 at a wrap',
            f'ld {a}, [{hb["lph"] + 1:04x}]', f'ld {c}, [{P(B6_LRATE)}]', f'mov {a}, {a}', f'mad {a}, {a}, -{b}',
            f'addxy {c}, {a}, {c}', f'ph_wrap {a}, r3, {c}', f'st {a}, [{hb["lph"]:04x}]', f'subxy {c}, {a}, {c}',
            f'; voice {v}: sample & hold b of the noise at each wrap',
            f'ld {b}, [{hb["lsh"] + 1:04x}]', f'ld {d}, [{G["nz"]:04x}]', f'mov {d}, {d}',
            f'subxy {d}, {b}, {d}', f'mul {d}, {d}, {c}', f'mad {b}, {d}, #0.5', f'st {b}, [{hb["lsh"]:04x}]',
            f'; voice {v}: sample & glide d += k (b - d)',
            f'ld {c}, [{P(B6_LSG)}]', f'ld {d}, [{hb["lsg"] + 1:04x}]', f'mov {d}, {d}',
            f'subxy {b}, {d}, {b}', f'mad {d}, {b}, {c}', f'st {d}, [{hb["lsg"]:04x}]',
            *(lfo_shape_lut(v, a, b, c, d) if LFO_LUT else [
            f'; voice {v}: lfo d = w g + ph_clamp(K p) + t (alpha + beta t^2)',
            f'ld {b}, [{P(B6_LWAVE + 3)}]', f'mov {b}, {b}', f'mul {d}, {d}, {b}',
            f'ld {b}, [{P(B6_LWAVE + 2)}]', f'mov {b}, {b}', f'mul {c}, {a}, {b}', f'ph_clamp {c}, r3, -{c}',
            f'addxy {d}, {d}, {c}', f'ph_tri {a}, r3, -{a}', f'mul {c}, {a}, {a}',
            f'ld {b}, [{P(B6_LWAVE + 1)}]', f'mov {b}, {b}', f'mul {c}, {c}, {b}',
            f'ld {b}, [{P(B6_LWAVE)}]', f'mov {b}, {b}', f'addxy {c}, {c}, {b}', f'mad {d}, {a}, {c}']),
            f'; voice {v}: fade a (0 -> 1 from the gate copy, DELAY TIME), d = lf = lfo x fade -> DSP A',
            f'ld {a}, [{hb["fade"] + 1:04x}]', f'ld {b}, [{P(B6_LDELAY)}]', f'ld {c}, [{GB[v]:04x}]', f'mov {c}, {c}',
            f'addxy {a}, {a}, {b}', f'mul {a}, {a}, {c}', f'min {a}, {a}, #1.0', f'st {a}, [{hb["fade"]:04x}]',
            f'mul {d}, {d}, {a}', f'st {d}, [{LFB[v]:04x}]'] + ([f'st {d}, [{cmt(1, v):04x}]'] if cmod_idx() else []) + [
            f'; voice {v}: -> amp 1 + f (M lf - |M|), -> filter F lf (for the kernel)',
            f'ld {b}, [{P(B6_LAMP)}]', f'ld {c}, [{P(B6_LAMPABS)}]', f'mov {b}, {b}', f'mul {b}, {d}, {b}',
            f'mad {b}, {a}, -{c}', f'add {b}, #1.0', f'st {b}, [{hb["lfa"]:04x}]',
            f'ld {b}, [{P(B6_LFILT)}]', f'mov {b}, {b}', f'mul {b}, {d}, {b}', f'st {b}, [{hb["lff"]:04x}]'] + \
        (penv_b(v, hb) if PENV and MOD_B else [])


def lfo_shape_lut(v, a, b, c, d):
    """LFO_LUT: d = w g + amp T[(p + 1) N / 2], the shape's table chosen by the ARM (B6_LWAVE = h of its first word,
    + 1 = amp, + 3 = w; LFO_SHAPES); a = the phase p (then free), d = the sample & glide value g"""
    P = lambda k: f'c0{k:02x}'  # noqa: E731
    half = float(LUT_LN // 2)
    return [f'; voice {v}: lfo d = w g + amp T[(p + 1) N / 2] (LFO_LUT: the shape table, linear interpolation)',
            f'ld {b}, [{P(B6_LWAVE + 3)}]', f'mov {b}, {b}', f'mul {d}, {d}, {b}',
            f'mul {c}, {a}, #{half!r}', f'addxy {c}, {c}, #{half!r}', f'ld {a}, [{P(B6_LWAVE)}]', f'mov {a}, {a}'] + \
        lut_read(c, b, b, a, LUT_LN, None)[:3] + [f'addxy {b}, {b}, {a}', 'hset h6, ' + b,
                                                   f'ldb5 {a}, 0', f'ldb5 {b}, 1', f'subxy {b}, {a}, {b}',
                                                   f'mad {a}, {c}, {b}',
                                                   f'ld {b}, [{P(B6_LWAVE + 1)}]', f'mov {b}, {b}', f'mad {d}, {a}, {b}']


def penv_core(v, a, b, c, d, pe, pf, prm):
    """pitch envelope AD (the host ADSR's math, adsr(), proven on the device, with the gate at 1 and sustain 0:
    latch cleared at a note start (b = 1), attack flag clamp(256 (1 - latch)), level toward 1.25 attack at rate
    kD + attack (kA - kD)) -> a, stored at word pe; pf = the latch; prm = the decay rate's parameter (attack next)"""
    prm2 = f'c0{int(prm[2:], 16) + 1:02x}'
    return [f'; voice {v}: pitch envelope a: the ADSR\'s latch form with gate 1, sustain 0',
            f'ld {a}, [{pe + 1:04x}]', f'ld {c}, [{pf + 1:04x}]', f'mov {c}, {c}',
            f'mad {c}, {c}, -{b}',                                                # latch *= 1 - d
            # latch = 1 once the level passed 0.999: 256 (256 (e - 0.999)) in two clamped steps (one product would
            # pass the DSP's +-512)
            f'addxy {b}, {a}, #-0.999', f'mul {b}, {b}, #256.0', f'ph_clamp {b}, r3, -{b}', f'mul {b}, {b}, #256.0',
            f'ph_clamp {b}, r3, -{b}', f'max {c}, {c}, {b}', f'st {c}, [{pf:04x}]',
            # attack flag c = clamp(256 (1 - latch)): 1 until the latch is full. With the attack 1 - latch itself a
            # latch caught half way in the 0.999 band (slow attacks) left the target 1.25 (1 - latch) under the
            # level and the envelope stuck there (2026-10-28, on the device: it stayed at a value, never fell to 0)
            f'subxy {c}, {c}, #1.0', f'mul {c}, {c}, #256.0', f'ph_clamp {c}, r3, -{c}',
            f'ld {b}, [{prm}]', f'ld {d}, [{prm2}]', f'mov {d}, {d}',
            f'subxy {d}, {b}, {d}', f'mad {b}, {c}, {d}',                         # rate = kD + attack (kA - kD)
            f'mul {c}, {c}, #1.25',                                               # target 1.25 attack
            f'subxy {c}, {a}, {c}', f'mad {a}, {c}, {b}', f'min {a}, {a}, #1.0', f'st {a}, [{pe:04x}]']


def penv_b(v, hb):
    """the pitch envelope of voice v on DSP B (MOD_DSP=B), after the LFO: penv_core with the note start stored by
    hlfo_voice_b -> word pe (COLOR MOD) and bank 5 PEB+v (-> pitch on DSP A)"""
    a, b, c, d = 'r0', 'r1', 'r2', 'r3'
    pe = cmt(2, v) if cmod_idx() else hb['pe']     # the level in the COLOR MOD table (CMOD_IDX)
    return [f'ld {b}, [{hb["bd"]:04x}]'] + penv_core(v, a, b, c, d, pe, hb['pf'], f'c0{B6_PENV:02x}') + \
        ([f'st {a}, [{PEB[v]:04x}]'] if PITCH_MOD else [])


def colors_b(v):
    """COLOR MOD of voice v on DSP B, after its envelopes: color i = weights (C054 + 6 i ..) x (1, lf, pitch env,
    filter env, amp env, sub) -> bank 5 COLB (s1_color(i) on DSP A, one sample late); the sub square from DSP A.
    Both colors in one pass (a = color 0, c = color 1): each source is loaded once for the two (the loads, not the
    multiplies, set its length: two bank-6 loads and one bank-5 access per instruction)"""
    a, b, c, d = 'r0', 'r1', 'r2', 'r3'
    P = lambda k: f'c0{k:02x}'  # noqa: E731
    hb = {n: block('B', v) + 2 * (H_ENV_WORDS + j) for j, n in enumerate(host_b_words())}
    e = block('B', v)
    acc = dict(zip(COLOR_USED, (a, c)))
    L = [f'; voice {v}: colors ' + ', '.join(f'{i} ({acc[i]})' for i in COLOR_USED) +
         ' = w0 + w1 lf + w2 pe + w3 fe + w4 ae + w5 sub']
    L += [f'ld {acc[i]}, [{P(B6_COL[i])}]' for i in COLOR_USED]
    for w_, src in ((1, LFB[v]), (2, hb['pe']), (3, e + 6), (4, e), (5, SUBB[v])):
        L += [f'ld {b}, [{src:04x}]']
        for i in COLOR_USED:
            L += [f'ld {d}, [{P(B6_COL[i] + w_)}]', f'mov {d}, {d}', f'mad {acc[i]}, {b}, {d}']
    L += [f'st {acc[i]}, [{COLB[i][v]:04x}]' for i in COLOR_USED]
    return L


def colors_idx_all():
    """COLOR MOD of every voice by indexed loads (CMOD_IDX), after all the voices: per color i h6 = the chosen source's
    table row (parameter C054 + 6 i + 2, written by the ARM), w0 and w (C054 + 6 i, + 1); per voice
    color = w0 + w src[h6 + 2 v] (ldidx + ymacd + st). The sources: the envelopes and the pitch envelope keep their
    level in the table (cmt), the LFO is stored there too, the sub is copied (voice_b)"""
    P = lambda k: f'c0{k:02x}'  # noqa: E731
    L = ['; COLOR MOD by indexed loads: per color h6 = the row of the chosen source, each voice w0 + w src[h6 + 2 v]']
    for i in COLOR_USED:
        k = B6_COL[i]
        L += [f'ld r0, [{P(k + 2)}]', 'mov r0, r0', 'hset h6, r0', f'ld r1, [{P(k)}]', f'ld r2, [{P(k + 1)}]',
              'mov r2, r2']
        for v in range(NV):
            r = f'r{3 + v % 4}'
            L += [f'ldidx {r}, {2 * v}', f'ymacd {r}, r2, r1', f'st {r}, [{COLB[i][v]:04x}]']
    return L


def hlfo_voice(v):
    """the host's DSP A part of voice v, before the kernel; four registers (pairs: r4..r7 for the odd voice), longer
    lived values in same-sample words of the voice block:
    * gate and trigger copies for DSP B (its envelopes, LFO and pitch envelope), from their loads
    * -> pitch: y = P lf + E pe + R octaves (LFO TO PITCH, PITCH ENV AMOUNT, OSC1 RANGE 64'..2' = -3..+2; lf and pe
      from DSP B, one sample late); pitch + y,
      dt 2^y = (e^(y ln2 / 8))^8 (Taylor to the 4th power, three squarings: ~2 cents at 6 octaves), rdt 2^-y (to the
      3rd: it only sets a polyBLEP's width) -> the words the kernel reads as s1_pitch / s1_increment,
      s1_dt, s1_rdt; DSP B's pitch straight to its word (HOSTB)
    * sub oscillator (square one octave down) -> bank 5 SUBB+v: COLOR MOD source (DSP B), SUB of the mixer
    The colors (s1_color) come from DSP B (colors_b), the kernel reads them in bank 5 (COLB)"""
    h = {n: block('A', v) + 2 * j for j, n in enumerate(host_a_words())}
    a, b, c, d = 'r0', 'r1', 'r2', 'r3'
    P = lambda k: f'c0{k:02x}'  # noqa: E731

    def put(reg, word, name):                       # the kernel's word (DSP A reads it) and / or DSP B's (INB)
        out = [f'st {reg}, [{h[word]:04x}]'] if word in h else []
        if name in HOSTB:
            out.append(f'st {reg}, [{INB0 + 8 * KDEF.inputs_b.index(name) + v:04x}]')
        return out
    L = [f'; voice {v}: host (DSP A): gate / trigger copies for DSP B']
    if PENV and not MOD_B:                          # MOD_DSP=A: the pitch envelope here (a = its level)
        L += [f'ld {a}, [{TRIG[v]:04x}]', f'ld {c}, [{GATE[v]:04x}]', f'ld {b}, [{h["pt"] + 1:04x}]',
              f'mov {a}, {a}', *([f'st {a}, [{TB[v]:04x}]', f'st {c}, [{GB[v]:04x}]'] if GT_B else []),
              f'st {a}, [{h["pt"]:04x}]', f'subxy {b}, {b}, {a}', f'mul {b}, {b}, {b}']
        L += penv_core(v, a, b, c, d, h['pe'], h['pf'], P(A6_PENV))
    elif GT_B:
        L += [f'ld {a}, [{TRIG[v]:04x}]', f'ld {c}, [{GATE[v]:04x}]', f'mov {a}, {a}',
              f'st {a}, [{TB[v]:04x}]', f'st {c}, [{GB[v]:04x}]']
    if PITCH_MOD:
        L += [f'; voice {v}: -> pitch b = y = P lf + E pe + OSC1 RANGE octaves', f'ld {b}, [{LFB[v]:04x}]',
              f'ld {c}, [{P(A6_LPITCH)}]', *([f'ld {a}, [{PEB[v]:04x}]'] if MOD_B else []), f'mov {c}, {c}',
              f'mul {b}, {b}, {c}',
              f'ld {c}, [{P(A6_PAMT)}]', f'mov {c}, {c}', f'mad {b}, {a}, {c}', f'ld {c}, [{P(A6_RANGE1)}]', f'mov {c}, {c}', f'addxy {b}, {b}, {c}']
        if 'pitch' in PITCH_MOD:
            L += [f'ld {c}, [{PITCH[v]:04x}]', f'mov {c}, {c}', f'addxy {c}, {c}, {b}'] + put(c, 'pitchm', 'pitch')
        # 2^y = (e^(y ln2 / 8))^8: Taylor of e^z on |z| <= 0.52 (y up to +-6 octaves: RANGE, PITCH ENV, LFO), then
        # three squarings; dt to z^4 (~2 cents at 6 octaves), rdt (a polyBLEP's width) to z^3
        poly = {'dt': [f'mov {a}, #0.041666666666666664', f'ymacd {a}, {c}, #0.16666666666666666',
                       f'ymacd {a}, {c}, #0.5', f'ymacd {a}, {c}, #1.0', f'ymacd {a}, {c}, #1.0'],
                'rdt': [f'mov {a}, #0.16666666666666666', f'ymacd {a}, {c}, #0.5', f'ymacd {a}, {c}, #1.0',
                        f'ymacd {a}, {c}, #1.0']}
        for n, src, sign in (('dt', DT, 1.0), ('rdt', RDT, -1.0)):
            if n in PITCH_MOD:
                L += [f'mul {c}, {b}, #{sign * math.log(2) / 8!r}'] + poly[n] + \
                    [f'mul {a}, {a}, {a}', f'mul {a}, {a}, {a}', f'mul {a}, {a}, {a}', f'ld {c}, [{src[v]:04x}]',
                     f'mov {c}, {c}', f'mul {c}, {c}, {a}', f'st {c}, [{h[n + "m"]:04x}]']
    if 'sp' in h:
        L += [f'; voice {v}: sub oscillator: a square one octave down (COLOR MOD source, SUB of the mixer)',
              f'ld {b}, [{h["sp"] + 1:04x}]', f'ld {c}, [{DT[v]:04x}]', f'mov {c}, {c}',
              f'addxy {c}, {b}, {c}', f'ph_wrap {b}, r3, {c}', f'st {b}, [{h["sp"]:04x}]', f'mul {b}, {b}, #256.0',
              f'ph_clamp {b}, r3, -{b}', f'st {b}, [{SUBB[v]:04x}]']
    if not MOD_B:                                   # MOD_DSP=A: the colors here, the envelopes from DSP B (ENVX)
        for i in COLOR_USED:
            k = A6_COL[i]
            L += [f'; voice {v}: color {i} = w0 + w1 lf + w2 pe + w3 fe + w4 ae + w5 sub (c)', f'ld {c}, [{P(k)}]']
            for w_, src in ((1, LFB[v]), (2, h['pe']), (3, ENVX[1][v]), (4, ENVX[0][v]), (5, SUBB[v])):
                L += [f'ld {b}, [{src:04x}]', f'ld {d}, [{P(k + w_)}]', f'mov {d}, {d}', f'mad {c}, {b}, {d}']
            L += [f'st {c}, [{h[f"color{i}"]:04x}]']
    return L


def voice_a(v):
    """DSP A code of voice v: the host's (four registers), the kernel's, and the host's mixer"""
    L = [f'; voice {v}'] + (hlfo_voice(v) if HOST_LFO else []) + KDEF.place('A', addresses('A', v))
    if HOST_MIX:                                # kernel (its output word xo) + SUB x sub + NOISE x noise -> x of DSP B
        h = {n: block('A', v) + 2 * j for j, n in enumerate(host_a_words())}
        L += [f'; voice {v}: mixer', f'ld r0, [{h["xo"]:04x}]', f'ld r1, [{SUBB[v]:04x}]',
              f'ld r2, [c0{A6_SUBMIX:02x}]', 'mov r2, r2', 'mad r0, r1, r2',
              f'ld r1, [{lfo_block("A")["nz"]:04x}]', f'ld r2, [c0{A6_NOISEMIX:02x}]', 'mov r2, r2', 'mad r0, r1, r2',
              f'st r0, [{XV[v]:04x}]']
    if HOST_ENVS and not HOST_LFO:             # gate and trigger for the host's envelopes on DSP B (with the host LFO,
                                                # hlfo_voice stores them from its loads)
        L += [f'ld r0, [{GATE[v]:04x}]', f'st r0, [{GB[v]:04x}]', f'ld r1, [{TRIG[v]:04x}]', f'st r1, [{TB[v]:04x}]']
    return L


def _upper(lines):
    """the same code in r4..r7. The r3 in the middle of ph_wrap / ph_tri / ph_clamp is part of the opcode (Roland's
    `ph_wrap rD, r3, rY`), not a register: it stays (with r7 the DSP does something else, emulator 2026-10-28)"""
    def up(x):
        if x.startswith(';'):
            return x
        m = re.match(r'(\s*ph_\w+\s+)(r\d)(,\s*r3,)(.*)$', x)
        sub = lambda t: re.sub(r'\br([0-3])\b', lambda m_: f'r{int(m_.group(1)) + 4}', t)  # noqa: E731
        return m.group(1) + sub(m.group(2)) + m.group(3) + sub(m.group(4)) if m else sub(x)
    return [up(x) for x in lines]


def _chunks(lines):
    """lines in interleaving units: a hset with the indexed loads that use it stays whole (h6 / h7 are one per DSP: the
    other voice's hset may not come in between)"""
    out, k = [], 0
    while k < len(lines):
        if lines[k].startswith('hset'):
            last = k
            for j in range(k + 1, len(lines)):
                if lines[j].startswith('hset'):
                    break
                if lines[j].startswith(('ldb5', 'ldidx')):
                    last = j
            out.append(lines[k:last + 1])
            k = last + 1
        else:
            out.append(lines[k:k + 1])
            k += 1
    return out


def _interleave(x, y):
    """two voices' code (independent registers), unit by unit: the scheduler fuses across them"""
    cx, cy, L = _chunks(x), _chunks(y), []
    for i in range(max(len(cx), len(cy))):
        L += sum(cx[i:i + 1], []) + sum(cy[i:i + 1], [])
    return L


def prog_a_parts():
    """DSP A as (the head: comment and the global part, [the code of each voice pair, or voice]); prog_a() is all of
    it in one program, sources() may put the last pairs in section 2"""
    head = [f'; s1m0: slave DSP A main @0x0100: {NV} voices x s1cc kernel {KERNEL} DSP A -> A151+v'
            + (' (voice pairs interleaved, PAIRS=1)' if PAIRS else '')]
    if HOST_MIX:
        head += hlfo_global('A')
    units, v = [], 0
    while v < NV:
        if PAIRS and v + 1 < NV:               # two voices, independent registers: the scheduler fuses across them
            units.append(_interleave(voice_a(v), _upper(voice_a(v + 1))))
            v += 2
        else:
            units.append(voice_a(v))
            v += 1
    return head, units


def prog_a():
    head, units = prog_a_parts()
    return head + [x for u in units for x in u]


def prog_b():
    head, units = prog_b_parts()
    return head + [x for u in units for x in u]


def prog_b_parts():
    """DSP B as (the head: the mix, [the code of each voice pair, or voice])"""
    L = [f'; s1m1: slave DSP B main @0x0100: mix of {NV} voices x {GAIN!r} x C000 -> 6006 / 6007'
         + (f'; per voice s1cc kernel DSP B' if HAS_B else ''), 'mov r7, #0.0']
    for v in range(NV):                         # B outputs of the sample before (ring S+1); A outputs as they are
        L += [f'ld r{v & 1}, [{(B_OUT[v] + 1) if HAS_B else XV[v]:04x}]', f'addxy r7, r7, r{v & 1}']
    L += ['ld r6, [c000]', f'mul r7, r7, #{GAIN!r}', 'mul r7, r7, r6',
          f'st r7, [{OUT_L:04x}]', f'st r7, [{OUT_R:04x}]', 'mov r0, #0.0']
    L += [f'st r0, [{x:04x}]' for x in OUT_ZERO]
    if HOST_LFO:
        L += hlfo_global('B')
    units = []
    if HAS_B or HOST_ENVS or HOST_LFO:
        v = 0
        while v < NV:
            if PAIRS_B and v + 1 < NV:          # two voices, independent registers (as DSP A, PAIRS=1)
                units.append(_interleave(voice_b(v), _upper(voice_b(v + 1))))
                v += 2
            else:
                units.append(voice_b(v))
                v += 1
        if cmod_idx():                          # COLOR MOD of all voices, by indexed loads
            units.append(colors_idx_all())
    return L, units


def voice_b(v):
    """DSP B code of voice v: the host's envelopes (four registers) and the kernel's"""
    L = [f'; voice {v}'] + (hlfo_voice_b(v) if HOST_LFO else [])
    if HOST_ENVS:
        b = block('B', v)
        for i in HOST_ENV_USED:
            e = b + H_ENV_STRIDE * i
            out = cmt(4 if i == 0 else 3, v) if cmod_idx() else e + 6 if CURVE else e   # what the kernel reads
            lvl = e if CURVE else out               # the ADSR's level: in the COLOR MOD table (CMOD_IDX) unless curved
            L += [f'; voice {v}: host envelope {i}'] + adsr(lvl, e + 2, e + 4, GB[v], TB[v], 0xC000 + H6_ENV[i])
            if CURVE:                               # level = C(ADSR) from the shared curve table (r0 -> r0)
                L += [f'; voice {v}: envelope {i} curve {ENV_CURVE} (shared table)', f'mul r1, r0, #{float(LUT_EN)!r}'] + \
                    lut_read('r1', 'r2', 'r0', lut_h(lut_layout()[1]['env']), LUT_EN, None) + [f'st r0, [{out:04x}]']
            if CMOD and not MOD_B:                  # its level for COLOR MOD on DSP A
                L += [f'st r0, [{ENVX[i][v]:04x}]']
    if CMOD and MOD_B and not cmod_idx():
        L += colors_b(v)
    if cmod_idx():                                  # the sub square into the COLOR MOD table
        L += [f'ld r1, [{SUBB[v]:04x}]', f'st r1, [{cmt(5, v):04x}]']
    if HAS_B:
        L += KDEF.place('B', addresses('B', v))
    return L


def adsr(e, latch, pt, gate, trig, prm):
    """C-S1SY's ADSR (examples/mutable_system/make_mutable.py step_adsr, proven on the device) in four registers,
    r0..r3 (PAIRS=1 runs the odd voice's copy in r4..r7) -> r0 (stored at e). prm: sustain, attack, decay, release
    rate. latch = 1 once the attack reached 1; target 1.25 (attack) / S (decay) / 0 (release); a change of trig
    clears the latch (new note). The attack is g - latch: it follows the gate itself, so a trigger and a gate that
    arrive apart lose nothing. The same math as step_adsr, except: the end of the attack, 65536 (e - 0.999), is two
    clamped steps of 256 (one product passes the DSP's +-512); the attack is clamp(256 (g - latch)), 1 while the
    latch is not full (with g - latch a latch caught half way in the 0.999 band, slow attacks, kept the envelope
    stuck: 2026-10-28, the pitch envelope on the device); the level is loaded again for the last step instead of
    holding a register"""
    a, b, c, d = 'r0', 'r1', 'r2', 'r3'
    return [f'ld {a}, [{e + 1:04x}]', f'ld {b}, [{latch + 1:04x}]', f'ld {c}, [{trig:04x}]', f'ld {d}, [{pt + 1:04x}]',
            f'mov {d}, {d}', f'st {c}, [{pt:04x}]',
            f'subxy {d}, {d}, {c}', f'mul {d}, {d}, {d}',                       # d = (trig - last trig)^2
            f'subxy {d}, {d}, #1.0', f'mul {b}, {b}, {d}',                      # latch *= 1 - d
            f'addxy {c}, {a}, #-0.999', f'mul {c}, {c}, #256.0', f'ph_clamp {c}, r3, -{c}', f'mul {c}, {c}, #256.0',
            f'ph_clamp {c}, r3, -{c}',                                           # two clamped steps: no overflow
            f'max {b}, {b}, {c}', f'ld {d}, [{gate:04x}]', f'mov {d}, {d}', f'mul {b}, {b}, {d}', f'st {b}, [{latch:04x}]',
            f'subxy {c}, {b}, {d}', f'mul {c}, {c}, #256.0', f'ph_clamp {c}, r3, -{c}',   # attack = clamp(256 (g - latch))
            f'subxy {d}, {d}, #1.0',                                             # 1 - g
            f'ld {a}, [{prm + 3:04x}]', f'mov {a}, {a}', f'mul {d}, {d}, {a}',  # rate = (1 - g) kR
            f'ld {a}, [{prm + 1:04x}]', f'mov {a}, {a}', f'mad {d}, {c}, {a}',  #   + attack kA
            f'ld {a}, [{prm + 2:04x}]', f'mov {a}, {a}', f'mad {d}, {b}, {a}',  #   + latch kD
            f'ld {a}, [{prm:04x}]', f'mov {a}, {a}', f'mul {b}, {b}, {a}', f'mad {b}, {c}, #1.25',   # target
            f'ld {a}, [{e + 1:04x}]', f'mov {a}, {a}', f'subxy {b}, {a}, {b}', f'mad {a}, {b}, {d}',
            f'min {a}, {a}, #1.0', f'st {a}, [{e:04x}]']


def env_time(v):
    """fader 0..255 -> seconds, ENV_TMIN .. ENV_TMAX exponential (C-S1SY: 1 ms .. 10 s)"""
    return ENV_TMIN * (ENV_TMAX / ENV_TMIN) ** (v / 255.0)


def attack_rate(v):
    """rate toward 1.25 that reaches 1.0 in env_time(v): (1 - k)^n = 0.2 (C-S1SY)"""
    return 1 - 0.2 ** (1 / (env_time(v) * FS))


def decay_rate(v):
    """rate that falls to 1/1000 of the step in env_time(v) (C-S1SY)"""
    return 1 - 0.001 ** (1 / (env_time(v) * FS))


def env_params(i, knobs):
    """envelope i's 4 DSP B parameter values (sustain, attack, decay, release rates) for faders (S, A, D, R)"""
    S, A, D, R = knobs
    return [S / 255.0, attack_rate(A), decay_rate(D), decay_rate(R)]


def lfo_hz(v):
    """LFO RATE knob -> Hz: 0.05 .. 40 Hz exponential (MUTABLE-SYSTEM)"""
    return 0.05 * 800 ** (v / 255.0)


def on_grid(x):
    """x on the register grid (multiples of 2^-32 below 0.125, MUTABLE-SYSTEM)"""
    return math.floor(x * 2 ** 32) / 2 ** 32 if abs(x) < 0.125 else x


def lfo_rate(v):
    return on_grid(2 * lfo_hz(v) / FS)          # phase -1..1


def lfo_sg(v):
    return on_grid(min(1.0, SG_K * lfo_rate(v)))


def color_weights(knob, src):
    """color i's 6 DSP A parameters (manual, LFO, pitch env, filter env, amp env, sub) for COLOR knob and MOD 0..5"""
    c, wv = knob / 255.0, [0.0] * 6
    src = min(max(src, 0), 5)
    if src == 0:
        wv[0] = c
    elif src in (1, 5):                            # the LFO and the sub square are -1..1: c (0.5 + 0.5 x)
        wv[0] = wv[src] = c / 2
    else:
        wv[src] = c
    return wv


def bipolar(v):
    """knob 0..255 -> -1..+1, 128 = 0 (MUTABLE-SYSTEM step 11)"""
    return max(-1.0, (v - 128) / 127.0)


def lfo_delay(v):
    """DELAY TIME knob -> fade step per sample: 0 = at once, 255 = 5 s (MUTABLE-SYSTEM)"""
    t = 5.0 * (v / 255.0) ** 2
    return 1.0 if t * FS < 1 else on_grid(1 / (t * FS))


def lfo_pitch(v):
    """LFO TO PITCH -> octaves, bipolar -1 .. +1 as b |b| (fine vibrato near the centre; MUTABLE-SYSTEM step 11)"""
    b = bipolar(v)
    return b * abs(b)


def lfo_filt(v):
    """LFO TO FILTER -> octaves, bipolar -4 .. +4"""
    return 4.0 * bipolar(v)


def lfo_amp(v):
    """LFO TO AMP -> M, bipolar -0.5 .. +0.5: voice x 1 + f (M lfo - |M|)"""
    return 0.5 * bipolar(v)


def penv_amt(v):
    """PITCH ENV AMOUNT -> octaves, bipolar -4 .. +4 as 4 b |b| (fine near the centre, as LFO TO PITCH)"""
    b = bipolar(v)
    return 4.0 * b * abs(b)


def hlfo_params(knobs):
    """the parameters of the host's LFO (DSP B), pitch envelope, COLOR MOD and mixer (DSP A) as engine.c writes
    them -> (DSP A's, DSP B's); knobs: HLFO's names"""
    k = dict(HLFO_DEFAULT, **{n: x for n, x in knobs.items() if n in HLFO_DEFAULT})
    pa = {0xC000 + A6_LPITCH: f32(lfo_pitch(k['lfo_pitch'])), 0xC000 + A6_PAMT: f32(penv_amt(k['penv_amt'])),
          0xC000 + A6_RANGE1: float(min(max(k['range1'], 0), 5) - 3),
          0xC000 + A6_SUBMIX: f32(k['sub_mix'] / 510.0), 0xC000 + A6_NOISEMIX: f32(k['noise_mix'] / 510.0)}
    pb = {0xC000 + B6_LRATE: f32(lfo_rate(k['lfo_rate'])), 0xC000 + B6_LSG: f32(lfo_sg(k['lfo_rate'])),
          0xC000 + B6_LDELAY: f32(lfo_delay(k['lfo_delay'])), 0xC000 + B6_LFILT: f32(lfo_filt(k['lfo_filter'])),
          0xC000 + B6_LAMP: f32(lfo_amp(k['lfo_amp'])), 0xC000 + B6_LAMPABS: f32(abs(lfo_amp(k['lfo_amp']))),
          }
    pm = pb if MOD_B else pa                        # pitch envelope and COLOR MOD: DSP B or A (MOD_DSP)
    pm[0xC000 + (B6_PENV if MOD_B else A6_PENV)] = f32(decay_rate(k['penv_d']))
    pm[0xC000 + (B6_PENV if MOD_B else A6_PENV) + 1] = f32(attack_rate(k['penv_a']))
    for j, x in enumerate(LFO_SHAPES[min(max(k['lfo_shape'], 0), 5)]):
        pb[0xC000 + B6_LWAVE + j] = x
    if k['lfo_shape'] == 5:                         # sample & hold: the glide follows at once
        pb[0xC000 + B6_LSG] = 1.0
    for i in (0, 1):
        wv = color_weights(k[f'color{i}'], k[f'mod{i}'])
        if cmod_idx():                              # w0, w, the chosen source's table row (colors_idx_all)
            src = min(max(k[f'mod{i}'], 0), 5)
            for j, x in enumerate((wv[0], wv[src] if src else 0.0, cmt(src or 4, 0) / 16384)):
                pm[0xC000 + B6_COL[i] + j] = f32(x)
            continue
        for j, x in enumerate(wv):
            pm[0xC000 + (B6_COL if MOD_B else A6_COL)[i] + j] = f32(x)
    return pa, pb


def prog_master():
    """C-S1SY13's master B (examples/mutable_system make_mutable.py step_m1, STEP 13)"""
    L = ['; s1mm1: master DSP B @0x1000: 6006 / 6007 x volume (C01C) -> OS effect send 2000 / 2002;'
         ' effect return 2001 / 2003 -> A118 / A119',
         f'ld r0, [{OUT_L:04x}]', f'ld r1, [{OUT_R:04x}]', f'ld r7, [c{M_VOLUME:03x}]', 'mov r4, r4',
         'mul r0, r0, r7', 'mul r1, r1, r7',
         f'st r0, [{FX_SEND[0]:04x}]', f'st r1, [{FX_SEND[1]:04x}]',
         f'ld r2, [{FX_RET[0]:04x}]', f'ld r3, [{FX_RET[1]:04x}]', 'mov r4, r4',
         f'st r2, [{M_OUT[0]:04x}]', f'st r3, [{M_OUT[1]:04x}]', 'mov r2, #0.0']
    return L + [f'st r2, [{x:04x}]' for x in M_ZERO]


_ISA = []


def assemble(lines):
    if not _ISA:
        _ISA.append(asm.Assembler().table)
    r = asm.Assembler(_ISA[0]).assemble('\n'.join(lines) + '\n')
    if not r.ok:
        raise SystemExit(r.errors)
    return r


# TRY_FORMS: op groups to test on the device before they go into asm/proven_forms.txt (docs/examples_system_s1cc_README.md
# §4.1): 'all' (every encodable group) or 'mad:reg + mul:imm; ...'; the build lists the ones it used in
# build/try_forms.txt, and tools/proven_forms.py promote adds them to the list once the PLUG-OUT worked on the device
TRY_FORMS = os.environ.get('TRY_FORMS', '').strip()
# PAIRS: DSP A runs voices two at a time, the even one in r0..r3 and the odd one in r4..r7, interleaved, so the
# scheduler can put the ops of both in one instruction (one voice is a chain of dependencies). On when the kernel is
# built with --regs-a 0,1,2,3 (and DSP B too with --regs-b 0,1,2,3)
# automatic (PAIRS=0 / 1 forces it): pairs when the kernel's DSP A code is in r0..r3
_A_LOW = not any(re.search(r'\br[4-7]\b', x) for x in KDEF.dsps['A']['lines'])
PAIRS = {'0': False, '1': True}.get(os.environ.get('PAIRS', ''), _A_LOW)
if PAIRS and not _A_LOW:
    raise SystemExit('make_s1cc.py: PAIRS=1 needs the kernel built with --regs-a 0,1,2,3 (its DSP A uses r4..r7)')
# DSP B in pairs too when its kernel is in r0..r3 (--regs-b 0,1,2,3, or --regs 0,1,2,3); else one voice at a time
PAIRS_B = PAIRS and not any(re.search(r'\br[4-7]\b', x) for x in KDEF.dsps.get('B', {}).get('lines', []))
TRIED = collections.Counter()                     # op groups not proven on the device that this build used


def section(name, lines, record=True):
    """a section program, scheduled with the op groups proven on the device only (asm/proven_forms.txt; with
    TRY_FORMS also those), one bank-5 access per instruction (rule 5) and only instruction shapes that are part of a
    proven program's; then checked against every SYSTEM-1 rule (asm/s1rules.py). SYSTEM-1 start marker (0a02 ff71),
    the code, end sequence"""
    dspsched.Ins.one_bank5 = True                  # rule 5: one bank-5 access per instruction (load or store)
    dspsched.Ins.shapes = [(collections.Counter(acc), ops) for acc, ops in s1rules.proven_shapes()]
    proven = dspsched.load_proven()
    allowed = None if TRY_FORMS == 'all' else proven | dspsched.parse_forms(TRY_FORMS)
    lines = dspsched.schedule(lines, allowed)
    new = {s: c for s, c in dspsched.signatures(lines).items() if s not in proven}
    if record:
        TRIED.update(new)
    for w in s1rules.check(name, lines, slave=name != 's1mm1', proven_ops=proven | set(new)):   # a broken rule stops the build
        print('warning:', w)
    r = assemble(lines)
    return [0x0A02, 0xFF71] + r.words + [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001], r, lines


def close(word):
    """the close program at 0x7FF2: frame counter E000 -> A004 (DSP A) / A005 (DSP B), as every SYSTEM-1 PLUG-OUT"""
    lines = ['ld r0, [e000]', f'st r0, [{word:04x}]']
    w = [0x0000] + assemble(lines).words
    assert len(w) == 7
    return w, None, lines


def instr_count(lines):
    return sum(1 for x in lines if x.strip() and not x.startswith(';'))


# Sections: when a DSP's main program (0x0100, 6 KiB) does not hold all its voices, the rest go to its sections, in
# the order the DSP runs them after the main (2, 3, 4, 5), as many voice pairs as each holds. engine.c places each
# with FC40(index, program set, empty data sets) and switches it on with FC44(index, 1), the way the stock synth
# places its voice programs (2026-10-28: a program queued at 0x1900 with FC30 was copied but never ran; with FC40 it
# runs). A section runs in the same sample as the main, with the same registers and memory: more room, not more time
# (the instructions of a DSP's main and sections count together in its BUDGET). docs/DSP_PROGRAM_MEMORY.md §3
# (name, FC40 / FC44 index, start, end) per DSP; the index table is FC40's (system-1/SH101_POLY.md §2)
SECTIONS = {'A': [('s1a2', 0, 0x1900, 0x3100), ('s1a3', 4, 0x3100, 0x4900), ('s1a4', 1, 0x4900, 0x6100),
                  ('s1a5', 5, 0x6100, 0x7FF2)],
            'B': [('s1b2', 2, 0x1900, 0x3100), ('s1b3', 6, 0x3100, 0x4900), ('s1b4', 3, 0x4900, 0x6100),
                  ('s1b5', 7, 0x6100, 0x7FF2)]}
SECTION_OF = {n: (dsp, idx, a, b) for dsp, lst in SECTIONS.items() for n, idx, a, b in lst}


def _fill(main, head, units, dsp):
    """{program name: unscheduled lines}: the main with the head and the most units it holds, then each section in
    turn; leftovers (nothing holds them) go to the last one used, and the size check of main() reports it"""
    flat = lambda us: [x for u in us for x in u]  # noqa: E731
    size = lambda n, lines: 2 * len(section(n, lines, record=False)[0])  # noqa: E731
    regions = [(main, 0x0100, 0x0100 + MAIN_BYTES)] + [(n, a, b) for n, _, a, b in SECTIONS[dsp]]
    out, rest = {}, list(units)
    for name, a, b in regions:
        hd = head if name == main else [f'; {name}: slave DSP {dsp} section @0x{a:04x}, after the main']
        k = len(rest)
        while k > 0 and size(name, hd + flat(rest[:k])) > b - a:
            k -= 1
        if name == main or k:
            out[name] = hd + flat(rest[:k])
        rest = rest[k:]
        if not rest:
            return out
    out[name if name in out else main] += flat(rest)
    return out


def sources():
    raw = _fill('s1m0', *prog_a_parts(), 'A')
    raw.update(_fill('s1m1', *prog_b_parts(), 'B'))
    TRIED.clear()
    progs = {n: section(n, lines) for n, lines in raw.items()}      # the groups the final programs use, once
    progs.update({'s1mm1': section('s1mm1', prog_master()), 's1c0': close(0xA004), 's1c1': close(0xA005)})
    return progs


def dsp_programs(progs, dsp):
    """the programs of one slave DSP in the order it runs them: main, then its sections"""
    return [('s1m0' if dsp == 'A' else 's1m1', progs['s1m0' if dsp == 'A' else 's1m1'])] + \
        [(n, progs[n]) for n, _, _, _ in SECTIONS[dsp] if n in progs]


# ---- ARM header -----------------------------------------------------------------------------------------------------
def float_bits(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


def table(name, vals, n=8):
    rows = [', '.join(f'0x{float_bits(x):08x}u' for x in vals[i:i + n]) for i in range(0, len(vals), n)]
    return f'static const uint32_t {name}[{len(vals)}] = {{\n    ' + ',\n    '.join(rows) + '\n};'


def c_array(name, w):
    """the program as the OS loader reads it: 32-bit words, the first halfword of each pair in the HIGH half
    ((p[i] << 16) | p[i + 1]), an odd count padded with 0 (examples/mutable_system make_mutable.py c_array). The ARM is
    little-endian: a uint16_t array in program order would reach the DSP with every pair swapped (2026-10-27: the
    first SYSTEM-S1CC builds did that and hung the device when the PLUG-OUT was selected)"""
    p = list(w) + [0] * (len(w) & 1)
    words = [(p[i] << 16) | p[i + 1] for i in range(0, len(p), 2)]
    body = ',\n'.join('    ' + ', '.join(f'0x{x:08x}' for x in words[i:i + 6]) for i in range(0, len(words), 6))
    return (f'#define MC_{name.upper()}_BYTES 0x{2 * len(w):x}\n'
            f'static const uint32_t mc_{name.upper()}[{len(words)}] = {{\n{body}\n}};\n')


def note_dt(n):
    """f / 96000 of MIDI note n, as the DSP's float32 pitch word gives it (s1rt.c s1_dt)"""
    return 440.0 * 2.0 ** (f32((n - 12) / 12.0) - 4.75) / 96000.0


def header(progs):
    out = [f'/* generated by make_s1cc.py (KERNEL={KERNEL}, VOICES={NV}): our own DSP programs and tables */',
           '#include <stdint.h>', '']
    out += [c_array(name, w) for name, (w, _, _) in progs.items()]
    sect = [(n, SECTION_OF[n]) for n in progs if n in SECTION_OF]
    out.append(f'/* sections (make_s1cc.py SECTIONS): FC40 / FC44 index and program set of each, placed by engine.c */\n'
               f'#define NSECT {len(sect)}')
    if sect:
        out.append('static const uint8_t sect_index[NSECT] = { ' + ', '.join(str(x[1][1]) for x in sect) + ' };')
        out.append('static struct s1_prog sect_set[NSECT][2] = {\n' + ',\n'.join(
            f'    {{ {{ 0x{a:04x}, MC_{n.upper()}_BYTES, mc_{n.upper()} }}, {{ 0, 0, 0 }} }}' for n, (_, _, a, _) in sect)
            + '\n};')
    out.append('/* pitch word of a MIDI note: (note - 12) / 12 octaves (4.75 = A 440) */')
    out.append(table('pitch_bits', [(n - 12) / 12.0 for n in range(128)]))
    out.append('/* s1_dt / s1_rdt words of a MIDI note: f / 96000 and 96000 / f / 16384 */')
    out.append(table('dt_bits', [note_dt(n) for n in range(128)]))
    out.append(table('rdt_bits', [1.0 / note_dt(n) / 16384.0 for n in range(128)]))
    out.append('/* VOLUME knob -> gain (v / 255)^2 */')
    out.append(table('volume_bits', [(v / 255.0) ** 2 for v in range(256)]))
    idx = lambda x: f'0x{x & 0xFF:02x}'  # noqa: E731
    out.append(f'#define NV {NV}\n#define M_VOLUME 0x{M_VOLUME:02x}\n'
               f'#define F_ONE 0x{float_bits(1.0):08x}u\n#define F_ZERO 0x00000000u\n'
               f'#define B5_PITCH {idx(PITCH[0])}\n#define B5_GATE {idx(GATE[0])}\n#define B5_TRIG {idx(TRIG[0])}\n'
               f'#define B5_DT {idx(DT[0])}\n#define B5_RDT {idx(RDT[0])}')
    out.append(f'#define HOST_ENVS {int(HOST_ENVS)}')
    if HOST_ENVS:
        out.append(f'/* host envelopes (s1_env): DSP B parameters H6_ENV0 / H6_ENV1 + (sustain, attack, decay, release);\n'
                   f'   panel ids (S, A, D, R) of AMP and FILTER; times {ENV_TMIN} .. {ENV_TMAX} s */')
        out.append(f'#define H6_ENV0 0x{H6_ENV[0]:02x}\n#define H6_ENV1 0x{H6_ENV[1]:02x}')
        out.append('static const uint16_t henv_panel[8] = { ' + ', '.join(f'0x{x:03x}' for p in H_ENV_PANEL for x in p) + ' };')
        out.append('static const uint8_t henv_default[8] = { ' + ', '.join(str(x) for d in H_ENV_DEFAULT for x in d) + ' };')

    if HOST_ENVS or HOST_LFO:                      # the envelopes' and the pitch envelope's rates, COLOR levels
        out.append(table('attack_bits', [attack_rate(v) for v in range(256)]))
        out.append(table('decay_bits', [decay_rate(v) for v in range(256)]))
        out.append(table('level_bits', [v / 255.0 for v in range(256)]))
    out.append(f'#define HOST_LFO {int(HOST_LFO)}')
    if HOST_LFO:
        out.append(f'/* the host: LFO (DSP B parameters B6_*), pitch envelope and COLOR MOD (M6_*: DSP A or B, MOD_DSP), mixer (DSP A, A6_*); controls\n'
                   f'   (panel id, default); tables */\n'
                   f'#define B6_LRATE 0x{B6_LRATE:02x}\n#define B6_LSG 0x{B6_LSG:02x}\n#define B6_LWAVE 0x{B6_LWAVE:02x}\n'
                   f'#define B6_LDELAY 0x{B6_LDELAY:02x}\n#define B6_LFILT 0x{B6_LFILT:02x}\n#define B6_LAMP 0x{B6_LAMP:02x}\n'
                   f'#define B6_LAMPABS 0x{B6_LAMPABS:02x}\n'
                   f'#define M6_SEL {int(MOD_B)}  /* pitch envelope and COLOR MOD: DSP B (1) or A (0), MOD_DSP */\n'
                   f'#define M6_PENV 0x{B6_PENV if MOD_B else A6_PENV:02x}\n'
                   f'#define M6_COL0 0x{(B6_COL if MOD_B else A6_COL)[0]:02x}\n'
                   f'#define M6_COL1 0x{(B6_COL if MOD_B else A6_COL)[1]:02x}\n'
                   f'#define CMOD_IDX {int(cmod_idx())}  /* COLOR MOD by an indexed load: w0, w, source row */\n'
                   f'#define A6_LPITCH 0x{A6_LPITCH:02x}\n#define A6_PAMT 0x{A6_PAMT:02x}\n#define A6_RANGE1 0x{A6_RANGE1:02x}\n'
                   f'#define A6_SUBMIX 0x{A6_SUBMIX:02x}\n#define A6_NOISEMIX 0x{A6_NOISEMIX:02x}')
        out.append(f'static const uint16_t hlfo_panel[{len(HLFO)}] = {{ ' + ', '.join(f'0x{p:03x}' for _, p, _ in HLFO) + ' };')
        out.append(f'static const uint8_t hlfo_default[{len(HLFO)}] = {{ ' + ', '.join(str(d) for _, _, d in HLFO) + ' };')
        if cmod_idx():
            out.append('/* COLOR MOD sources (manual, LFO, pitch env, filter env, amp env, sub): the table row / 16384 '
                       '(h6 of the indexed loads) */')
            out.append(table('cmod_off_bits', [cmt(s or 4, 0) / 16384 for s in range(6)]))
        out.append(table('lfo_rate_bits', [lfo_rate(v) for v in range(256)]))
        out.append(table('lfo_sg_bits', [lfo_sg(v) for v in range(256)]))
        out.append(table('penv_amt_bits', [penv_amt(v) for v in range(256)]))
        out.append('/* OSC RANGE 64\'..2\' (0..5) -> octaves -3..+2 */')
        out.append(table('range_bits', [r - 3.0 for r in range(6)]))
        out.append('/* LFO SHAPE -> (alpha, beta, K, w) of DSP B B6_LWAVE.. */')
        out.append(table('lfo_shape_bits', [x for sh in LFO_SHAPES for x in sh], 4))
        out.append(table('lfo_delay_bits', [lfo_delay(v) for v in range(256)]))
        out.append(table('lfo_pitch_bits', [lfo_pitch(v) for v in range(256)]))
        out.append(table('lfo_filt_bits', [lfo_filt(v) for v in range(256)]))
        out.append(table('lfo_amp_bits', [lfo_amp(v) for v in range(256)]))
        out.append(table('lfo_ampabs_bits', [abs(lfo_amp(v)) for v in range(256)]))
        out.append(table('half_bits', [v / 510.0 for v in range(256)]))
    lut, _ = lut_layout()
    out.append(f'/* LUT (LFO_LUT / ENV_CURVE): bank-5 tables from A000 + LUT0_IDX, written at load and refresh */\n'
               f'#define LUT_N {len(lut)}\n#define LUT0_IDX 0x{LUT0 & 0xFF:02x}')
    if lut:
        out.append(table('lut_bits', lut))
    rows, bits, off = [], [], 0
    for name, (cnt, ab, bb) in layout().items():
        rows.append((KTAB[name][0], cnt, 0xFF if ab is None else ab, 0xFF if bb is None else bb, off))
        for kv in range(256):
            bits += KDEF.param_values(name, kv, KTAB)
        off += 256 * cnt
    out.append(f'/* the kernel\'s knob tables (S1_PARAM): panel id, count, DSP A / B parameter base (0xFF = none),\n'
               f'   offset in kparam_bits (256 rows of count values each) */\n#define KPARAMS {len(rows)}')
    if rows:
        out.append('static const uint16_t kparam_panel[KPARAMS] = { ' + ', '.join(f'0x{r[0]:03x}' for r in rows) + ' };')
        out.append('static const uint8_t kparam_count[KPARAMS] = { ' + ', '.join(str(r[1]) for r in rows) + ' };')
        out.append('static const uint8_t kparam_a[KPARAMS] = { ' + ', '.join(f'0x{r[2]:02x}' for r in rows) + ' };')
        out.append('static const uint8_t kparam_b[KPARAMS] = { ' + ', '.join(f'0x{r[3]:02x}' for r in rows) + ' };')
        out.append('static const uint32_t kparam_off[KPARAMS] = { ' + ', '.join(str(r[4]) for r in rows) + ' };')
        out.append(table('kparam_bits', bits))
    return '\n'.join(out) + '\n'


# ---- simulation -----------------------------------------------------------------------------------------------------
def f32(x):
    return float(struct.unpack('<f', struct.pack('<f', x))[0])


def params(knobs):
    """bank-6 parameters of slave A, slave B and master B as engine.c / model.c write them; knobs: {name: 0..255}"""
    pa, pb, pm = {}, {0xC000: 1.0}, {0xC000 + M_VOLUME: 1.0}
    if HOST_ENVS:
        for i in range(2):
            for j, x in enumerate(env_params(i, knobs.get(f'env{i}', H_ENV_DEFAULT[i]))):
                pb[0xC000 + H6_ENV[i] + j] = f32(x)
    if HOST_LFO:
        ha, hb = hlfo_params(knobs)
        pa.update(ha)
        pb.update(hb)
    for name, (cnt, ab, bb) in layout().items():
        vals = KDEF.param_values(name, knobs.get(name.split('~')[0], 128), KTAB)   # name~id: derived from that knob
        for i in range(cnt):
            if ab is not None:
                pa[0xC000 + ab + i] = f32(vals[i])
            if bb is not None:
                pb[0xC000 + bb + i] = f32(vals[i])
    return pa, pb, pm


def simulate(progs, events, n, knobs=None, gate_lag=0):
    """events: {sample: [(voice, note or None for off)]}; returns master A118 per sample (emulator C core, one
    cchain.Slave: DSP 0 = slave A, 1 = slave B, 2 = master B; bank 5 shared by slave A and B; each DSP its own
    bank 6; B's 6006 / 6007 reach the master in the same sample). gate_lag: the gate of a note on comes that many
    samples after its pitch and trigger, as on the device, where engine.c writes them in separate calls"""
    sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 'emu'))
    import chain
    import cchain
    P = {k: chain.Program(progs[k][0], k) for k in progs if k not in ('s1c0', 's1c1')}
    model = chain.Model(order=(0, 1, 2), shared5=((0, 1),), own_params=(0, 1, 2))
    sl = cchain.Slave({0: (P['s1m0'], [P[n] for n, _ in dsp_programs(progs, 'A')[1:]]),
                       1: (P['s1m1'], [P[n] for n, _ in dsp_programs(progs, 'B')[1:]]), 2: (P['s1mm1'], [])}, model)
    for d, prm in zip(sl.d, params(knobs or {})):
        d.params.update(prm)
    a5 = sl.d[0].bank5
    for i, x in enumerate(lut_layout()[0]):           # the tables engine.c writes (LUT)
        a5[(LUT0 & 0x3FF) + i] = f32(x)
    trig = [0.0] * NV
    for v in range(NV):
        a5[PITCH[v] & 0x3FF], a5[GATE[v] & 0x3FF], a5[TRIG[v] & 0x3FF] = f32((69 - 12) / 12.0), 0.0, 0.0
        a5[DT[v] & 0x3FF], a5[RDT[v] & 0x3FF] = f32(note_dt(69)), f32(1.0 / note_dt(69) / 16384.0)
    if gate_lag:                                   # note on = pitch + trigger now, gate up gate_lag samples later
        ev = {}
        for te, lst in events.items():
            for v, note in lst:
                ev.setdefault(te, []).append((v, ('on', note) if note is not None else None))
                if note is not None:
                    ev.setdefault(te + gate_lag, []).append((v, 'gate'))
        events = ev
    else:
        events = {te: [(v, ('on', note) if note is not None else None) for v, note in lst] for te, lst in events.items()}
    out, t, word = [], 0, (2, M_OUT[0])
    for te in sorted(e for e in events if e < n) + [n]:
        if te > t:
            out += sl.run(te - t, (word,))[word]
            t = te
        for v, note in events.get(te, []) if te < n else []:
            if note is None:
                a5[GATE[v] & 0x3FF] = 0.0
            elif note == 'gate':
                a5[GATE[v] & 0x3FF] = 1.0
            else:
                trig[v] = 1.0 - trig[v]
                a5[PITCH[v] & 0x3FF], a5[TRIG[v] & 0x3FF] = f32((note[1] - 12) / 12.0), trig[v]
                a5[DT[v] & 0x3FF] = f32(note_dt(note[1]))
                a5[RDT[v] & 0x3FF] = f32(1.0 / note_dt(note[1]) / 16384.0)
                if not gate_lag:
                    a5[GATE[v] & 0x3FF] = 1.0
    return out


def sim_report(progs):
    import numpy as np
    import s1check
    n = int(os.environ.get('SIM_N', '9600'))
    if HOST_ENV_KERNEL:                            # the reference has the gate for s1_env: no sample comparison
        print('host envelopes on DSP B: the native reference uses the gate for s1_env; checked by shape instead')
        out = np.abs(np.array(simulate(progs, {0: [(0, 57)], n // 2: [(0, None)]}, n,
                                        {'env0': (128, 60, 128, 60), 'env1': (0, 0, 128, 60)})))
        w = int(0.005 * FS)
        seg = [out[k:k + w].max() for k in range(0, n, w)]
        print('amp envelope (A 60, D 128, S 128, R 60), peak per 5 ms: ' + ' '.join('%.3f' % x for x in seg[:20]))
    for note in (45, 69) if not HOST_ENV_KERNEL else ():
        out = np.array(simulate(progs, {0: [(0, note)]}, n))
        ra, rb = s1check.run_ref(KERNEL, note, n, {})
        ref = (rb if HAS_B else ra) * GAIN
        best = None
        for lag in range(0, 6):                    # the chain's latency in samples (ring words, IOMD, master)
            e = np.mean((out[lag:] - ref[:n - lag]) ** 2)
            if best is None or e < best[1]:
                best = (lag, e)
        lag = best[0]

        def snr(m):                                # 10 ms: sample by sample; all: the phases drift apart (float32)
            e = np.mean((out[lag:lag + m] - ref[:m]) ** 2)
            return 10 * np.log10(np.mean(ref[:m] ** 2) / e) if e > 0 else float('inf')
        print(f'note {note}, voice 0: peak {np.max(np.abs(out)):.3f} (reference x gain {np.max(np.abs(ref)):.3f}), '
              f'latency {lag} samples, SNR against the native reference {snr(960):.1f} dB (10 ms), '
              f'{snr(n - lag):.1f} dB (all)')
    chord = np.array(simulate(progs, {0: [(v, 48 + 3 * v) for v in range(NV)]}, n // 2))
    print(f'{NV} notes: peak {np.max(np.abs(chord)):.3f}')
    # on the device engine.c writes the trigger and the gate in separate calls: they can reach the DSP samples apart.
    # A kernel that needs both in the same sample loses its note starts there (2026-10-27: the ADSR never attacked)
    a = np.array(simulate(progs, {0: [(0, 57)]}, 4800))
    b = np.array(simulate(progs, {0: [(0, 57)]}, 4800, gate_lag=3))
    pa, pb = np.sqrt(np.mean(a ** 2)), np.sqrt(np.mean(b ** 2))        # the first 50 ms
    print(f'gate 3 samples after the trigger: rms {pb:.4f} (together: {pa:.4f})')
    if pa > 1e-4 and abs(pb / pa - 1) > 0.1:
        print('warning: the kernel loses a note start when the gate comes after the trigger (as engine.c can send '
              'them on the device): keep the start until the gate is up')
    quiet = max(abs(x) for x in simulate(progs, {}, 2000))
    print('no notes: peak %.6f' % quiet)
    if quiet > 1e-3:                               # no host VCA here: the kernel's own gate / envelope is the only one
        print('warning: the kernel sounds without notes: SYSTEM-S1CC has no VCA of its own, the kernel must apply '
              's1_gate() or an envelope (the comparison above then includes the other voices)')


def crossings(progs):
    """when each value DSP A hands to DSP B arrives (x, the inputs B needs, the exports, the gate / trigger copies).
    The two DSPs of a chip run at the same time and share bank 5 (docs/examples_system_1_jx_README.md §13): B reads
    A's word of this sample only if A's store comes before B's load, in cycles; otherwise the word of the sample
    before. The emulator runs A, then B, and cannot show it. Cycle = the instruction's place in its program (no jumps);
    both programs are assumed to start the sample together [inferred, as timing.py]; within 8 cycles: a race."""
    import re
    def at(lines, kind):
        pos, k = {}, 0
        for line in lines:
            if not line.strip() or line.startswith(';'):
                continue
            for a in re.findall(kind + r' r\d, \[(a[0-9a-f]{3})\]', line):
                pos.setdefault(int(a, 16), []).append(k)
            k += 1
        return pos
    st, ld = at(progs['s1m0'][2], 'st'), at(progs['s1m1'][2], 'ld')
    out = {}
    for a, sts in st.items():
        if a not in ld:
            continue
        s_, l_ = max(sts), min(ld[a])
        out[a] = 'same sample' if s_ + 8 < l_ else 'one sample late' if s_ > l_ + 8 else 'race'
    return out


def crossing_report(progs):
    c = crossings(progs)
    names = [('x', XV)] + [(f'export {e}', [export_addr(e, v) for v in range(NV)]) for e in KDEF.exports] + \
            [(f'input {n}', [INB0 + 8 * j + v for v in range(NV)]) for j, n in enumerate(KDEF.inputs_b)] + \
            ([('gate / trigger copies', GB)] if HOST_ENVS else [])
    for name, addrs in names:
        per = [c.get(a, '-') for a in addrs]
        if all(p == '-' for p in per):
            continue
        groups = {}
        for v, p in enumerate(per):
            groups.setdefault(p, []).append(str(v))
        print(f'A -> B {name}: ' + '; '.join(f'{p} (voices {",".join(vs)})' for p, vs in groups.items()))


def forms_report():
    """--forms: the op groups not proven on the device that would shorten the slave programs, and by how much"""
    dspsched.Ins.one_bank5 = True
    dspsched.Ins.shapes = [(collections.Counter(acc), ops) for acc, ops in s1rules.proven_shapes()]
    proven = dspsched.load_proven()
    for name, lines in (('s1m0', prog_a()), ('s1m1', prog_b())):
        cands, base, free = dspsched.candidate_forms(lines, proven)
        print(f'{name}: {base} instructions with the proven groups, {free} with every encodable group')
        # Roland's own use first (asm/roland_forms.py): a group it runs on the SYSTEM-1 is the safer test
        ev = {sig: dspsched.evidence_text(sig) for sig, _, _ in cands}
        cands.sort(key=lambda c: (' S1 -' in ev[c[0]], -c[1], -c[2]))
        for sig, saved, used in cands:
            print(f'  {saved:4d} saved alone, used x{used:<4d} {dspsched.form_text(sig):40s} {ev[sig]}')
    print("test: TRY_FORMS='GROUP; GROUP' (or TRY_FORMS=all) sh examples/system_s1cc/build.sh with NAME=..., install, "
          'play; if it works: python3 tools/proven_forms.py promote examples/system_s1cc/build/try_forms.txt --by NAME')


def overruns(progs):
    """the limits a build passes: instructions per DSP (main and sections), bytes per region -> [text]; and the
    largest share of a DSP's budget it uses"""
    over, load = [], 0.0
    for name, (w, _, lines) in progs.items():
        if name in BUDGET:
            parts = dsp_programs(progs, 'A' if name == 's1m0' else 'B')
            n_ins = sum(instr_count(x[1][2]) for x in parts)
            load = max(load, n_ins / BUDGET[name])
            if n_ins > BUDGET[name]:
                over.append(f'{name}: {n_ins} instructions{" (with its sections)" if len(parts) > 1 else ""}, '
                            f'budget {BUDGET[name]}')
            if 2 * len(w) > MAIN_BYTES:
                over.append(f'{name}: {2 * len(w)} bytes, its region has {MAIN_BYTES}')
        if name in SECTION_OF and 2 * len(w) > SECTION_OF[name][3] - SECTION_OF[name][2]:
            over.append(f'{name}: {2 * len(w)} bytes, its section has {SECTION_OF[name][3] - SECTION_OF[name][2]}')
    return over, load


def main():
    if '--forms' in sys.argv:
        forms_report()
        return
    if MOD_DSP == 'auto' and PENV:                  # the pitch envelope and COLOR MOD on the DSP with room for them
        tried = []
        for on_b in (True, False):
            set_mod_dsp(on_b)
            progs = sources()
            over, load = overruns(progs)
            tried.append((bool(over), load, on_b, progs))
        _, _, on_b, progs = min(tried, key=lambda t: (t[0], t[1]))
        set_mod_dsp(on_b)
        progs = sources()                           # again: TRIED and the sections are this placement's
        print(f'MOD_DSP=auto: pitch envelope and COLOR MOD on DSP {"B" if on_b else "A"}')
    else:
        progs = sources()
    over = []
    for n in ['s1s0'] + list(SECTION_OF):             # sections this build does not use: no stale files
        if n not in progs:
            for ext in ('.s', '.hex'):
                if os.path.exists(os.path.join(OUTDIR, n + ext)):
                    os.remove(os.path.join(OUTDIR, n + ext))
    for name, (w, _, lines) in progs.items():
        open(os.path.join(OUTDIR, name + '.s'), 'w').write('\n'.join(lines) + '\n')
        open(os.path.join(OUTDIR, name + '.hex'), 'w').write(
            f'# {name}, own SYSTEM-1 program (make_s1cc.py KERNEL={KERNEL} VOICES={NV})\n'
            + '\n'.join('%04x' % x for x in w) + '\n')
        print(f'{name:6s} {instr_count(lines):4d} instructions, {len(w)} halfwords')
        if name in BUDGET:                             # a DSP's time: its main and its sections
            parts = dsp_programs(progs, 'A' if name == 's1m0' else 'B')
            n_ins = sum(instr_count(x[1][2]) for x in parts)
            if n_ins > BUDGET[name]:
                over.append(f'{name}: {n_ins} instructions{" (with its sections)" if len(parts) > 1 else ""}, '
                            f'budget {BUDGET[name]}')
            if 2 * len(w) > MAIN_BYTES:
                over.append(f'{name}: {2 * len(w)} bytes, its region has {MAIN_BYTES}')
        if name in SECTION_OF and 2 * len(w) > SECTION_OF[name][3] - SECTION_OF[name][2]:
            over.append(f'{name}: {2 * len(w)} bytes, its section has {SECTION_OF[name][3] - SECTION_OF[name][2]}')
    print(f'kernel per voice: A {len(KDEF.dsps["A"]["lines"])} lines, {words("A")} words'
          + (f'; B {len(KDEF.dsps["B"]["lines"])} lines, {words("B")} words' if HAS_B else '; no DSP B'))
    crossing_report(progs)
    for st in KDEF.dsps:                       # spills, and r5 / r6 left unused: built for MUTABLE-SYSTEM's registers
        lines = KDEF.dsps[st]['lines']
        if KDEF.dsps[st].get('scratch') and not PAIRS and not any(re.search(r'\br[56]\b', x) for x in lines):
            print(f'note: DSP {st} has {KDEF.dsps[st]["scratch"]} spill words and does not use r5 / r6: SYSTEM-S1CC '
                  'has no host keeping them, build the kernel with --regs 0,1,2,3,4,5,6,7 (docs/S1CC.md §1.1)')
    if over:
        print(f'make_s1cc.py: {NV} voices do not fit ({", ".join(over)})')
        sys.exit(3)
    os.makedirs(os.path.join(OUTDIR, 'build'), exist_ok=True)
    open(os.path.join(OUTDIR, 'build', 's1_programs.h'), 'w').write(header(progs))
    tried = os.path.join(OUTDIR, 'build', 'try_forms.txt')
    if TRIED:                                      # what this build tests on the device (tools/proven_forms.py promote)
        open(tried, 'w').write(f'# op groups not proven on the device used by make_s1cc.py KERNEL={KERNEL} VOICES={NV}'
                               f' TRY_FORMS={TRY_FORMS}\n'
                               + ''.join(f'{dspsched.form_text(s)}    # x{c}\n' for s, c in TRIED.most_common()))
        print(f'TRY_FORMS: {len(TRIED)} op groups not proven on the device, {sum(TRIED.values())} instructions '
              f'({tried}): test this build on the device, then tools/proven_forms.py promote {tried} --by NAME')
    elif os.path.exists(tried):
        os.remove(tried)
    if '--sim' in sys.argv:
        sim_report(progs)


if __name__ == '__main__':
    main()
