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
    VOICES=n|auto           1..8, or auto (default): the most voices that fit, computed before the build (choose_voices:
                            each DSP's program scheduled whole, without assembly or sections, a lower bound of what
                            the build counts; a binary search on it, then one build, and one voice less only if that
                            build still does not fit)
    BUDGET_A=n, BUDGET_B=n  instructions per sample the slave's DSP A / DSP B may have, measured on the device
                            (docs/examples_system_1_synth_README.md §4): DSP A 703 ran (MP-CSAW 8 voices,
                            2026-10-28), 826 hung; DSP B 708 ran. Defaults 703 / 708. BUDGET=n sets both
    MAIN_BYTES=n            the size each slave main program may have: 6144 bytes, its region 0x0100..0x18FF before
                            section 2 (docs/DSP_PROGRAM_MEMORY.md §3.1); 5854 bytes ran, 6534 and 6740 hung the
                            device (2026-10-28). Above a budget or the size the build stops (exit code 3)
    SPLIT=auto|n            a fused kernel (s1cc --fused): the kernel of voices 0..n-1 on DSP A, the rest on DSP B;
                            auto: the n that leaves the busier DSP least loaded (docs/examples_system_s1cc_README.md §1.3)
    OUTDIR=dir              where everything generated goes: the programs (.s / .hex), s1_programs.h,
                            try_forms.txt (default: build/ next to this file, build.sh's folder)

    The other variables (BUDGET*, MAIN_BYTES, LFO_PITCH, ARM_ENV, ...: OPTIONS below) may also come from the kernel:
    S1_BUILD("ARM_ENV=1 LFO_LUT=1") in its C source -> .option lines in voice.s1k. An environment variable wins.

    python3 make_s1cc.py           -> build/s1*.s, build/s1*.hex, build/s1_programs.h
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
KDEF = s1kernel.Kernel.read(os.path.join(KERNEL, 'voice.s1k'))
# the options a kernel may set with S1_BUILD("KEY=VALUE ...") (its .option lines): the defaults of these variables;
# an environment variable wins (docs/examples_system_s1cc_README.md §1). NAME / MODEL_ID / PARAMLOG are build.sh's
OPTIONS = ('NAME', 'MODEL_ID', 'PARAMLOG', 'BUDGET', 'BUDGET_A', 'BUDGET_B', 'MAIN_BYTES', 'LFO_PITCH', 'MIX',
           'ENV_TMIN', 'ENV_TMAX', 'LFO_LUT', 'ENV_CURVE', 'ARM_ENV', 'ARM_LFO', 'ARM_RAMP', 'ARM_FADE', 'ARM_DIAG', 'ARM_PROBE', 'ARM_P0', 'ARM_RAMPTEST',
           'ARM_TICK', 'EDM_KS', 'EDM_TAP', 'EDM_GLIDE', 'EDM_CALL', 'EDM_ECF', 'EDM_BASE', 'EDM_RBASE', 'EDM_ENTRY', 'EDM_STEP', 'EDM_MAIN', 'MOD_DSP', 'CMOD_IDX', 'TRY_FORMS', 'PAIRS', 'SPLIT')
_bad = sorted(set(KDEF.options) - set(OPTIONS))
if _bad:
    raise SystemExit(f'make_s1cc.py: S1_BUILD option(s) {", ".join(_bad)} unknown here (known: {" ".join(OPTIONS)})')
if KDEF.options:
    print('kernel options (S1_BUILD): ' + ' '.join(f'{k}={v}' + (f' (environment: {os.environ[k]})' if k in os.environ
                                                                 else '') for k, v in KDEF.options.items()))


def opt(name, default):
    """an option: the environment variable, else the kernel's S1_BUILD value, else the default"""
    return os.environ.get(name, KDEF.options.get(name, default))


SPLIT_FIXED = None if opt('SPLIT', 'auto') == 'auto' else int(opt('SPLIT', 'auto'))   # FUSED: NA by hand


NV_MAX = 8                                     # the OS hands notes to 8 voice ids at most
VOICES = os.environ.get('VOICES', 'auto')
NV = NV_MAX if VOICES == 'auto' else int(VOICES)   # set_voices() changes it (VOICES=auto: choose_voices)
if not 1 <= NV <= NV_MAX:
    raise SystemExit('make_s1cc.py: VOICES must be 1..8 or auto (the OS hands notes to 8 voice ids at most)')


def _budget(name, default):
    """BUDGET_A / BUDGET_B, else BUDGET: from the environment first, then from the kernel"""
    for src in (os.environ, KDEF.options):
        if name in src or 'BUDGET' in src:
            return int(src.get(name, src.get('BUDGET')))
    return default


BUDGET = {'s1m0': _budget('BUDGET_A', 703),     # DSP A: 703 ran, 826 hung
          's1m1': _budget('BUDGET_B', 708)}     # DSP B: 708 ran
MAIN_BYTES = int(opt('MAIN_BYTES', '6144'))  # 0x0100..0x18FF: 5854 bytes ran, 6534 / 6740 hung
OUTDIR = os.environ.get('OUTDIR', os.path.join(HERE, 'build'))   # everything generated (tests: a temporary folder)
KTAB = s1kernel.read_tables(os.path.join(KERNEL, 'tables.txt'))
HAS_B = 'B' in KDEF.dsps
# FUSED (a kernel built with s1cc --fused, .fused): dsps['A'] is the whole voice, S1_DSP_B(S1_DSP_A()). The host
# stays as it is (its DSP A part on DSP A, its DSP B part on DSP B, for every voice); the kernel of voices 0..NA-1
# runs on DSP A, that of NA..NV-1 on DSP B, and the inputs it reads on the other DSP come through bank 5 (KX)
FUSED = KDEF.fused
K_DSP = 'A' if FUSED else 'B'                  # the kernel code that reads s1_env / s1_lfo_*
# the host's envelopes (s1_env(0) / s1_env(1) in S1_DSP_B): two ADSRs in assembly per voice on DSP B, C-S1SY's
HOST_ENV_KERNEL = [i for i in (0, 1) if K_DSP in KDEF.dsps and f'IN:env{i}' in KDEF.placeholders(K_DSP)]
# s1_color(i) in S1_DSP_A: OSC1 / OSC2 COLOR with its COLOR MOD source, made on DSP A (hlfo_voice):
# MANUAL = the knob, else knob x source (0..1): an LFO, a pitch envelope (AD) and a sub oscillator per voice on
# DSP A, the AMP and FILTER envelopes (the host's ADSRs on DSP B, then both are built) copied to bank 5 by DSP B
A_READS = set(KDEF.placeholders('A'))
COLOR_USED = [i for i in (0, 1) if f'IN:color{i}' in A_READS]
CMOD = bool(COLOR_USED)
# the host's LFO (per voice on DSP A, LFO SHAPE / RATE / DELAY TIME): -> PITCH modulates the pitch, s1_dt and s1_rdt
# words the kernel reads (LFO_PITCH=0 leaves them as the ARM writes them); -> FILTER / -> AMP are s1_lfo_filter() /
# s1_lfo_amp(), for the kernel's own filter and VCA; also the LFO source of COLOR MOD
LFO_PITCH = opt('LFO_PITCH', '1') == '1'
PITCH_MOD = [n for n in ('pitch', 'dt', 'rdt') if LFO_PITCH and f'IN:{n}' in A_READS]
LFO_F, LFO_A = 'IN:lfo_filter' in A_READS, 'IN:lfo_amp' in A_READS
HOST_LFO = bool(CMOD or PITCH_MOD or LFO_F or LFO_A)
# the LFO and the pitch envelope (COLOR MOD source and PITCH ENV AMOUNT -> pitch) run on DSP B (room there; DSP A is
# for the oscillators): lf and pe go to DSP A through bank 5 (LFB, PEB), one sample later
PENV = bool(PITCH_MOD or CMOD)
# the host's mixer on DSP A, after the kernel: voice = kernel + SUB x a square one octave down + NOISE x the LFO's
# noise, before DSP B's filter (SUB 0x176, NOISE 0x177; 0 until the knobs move). MIX=0 leaves it out
HOST_MIX = HOST_LFO and opt('MIX', '1') == '1'   # FUSED: the kernel reads it as s1_host_mix() (s1cc fuse)
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
if A_SELF & {'IN:lfo_amp', 'IN:lfo_filter'} and not FUSED:
    raise SystemExit('make_s1cc.py: s1_lfo_amp / s1_lfo_filter in S1_DSP_A: the host LFO runs on DSP B (use them in '
                     'S1_DSP_B)')
HOST_ENV_USED = [0, 1] if CMOD else HOST_ENV_KERNEL
HOST_ENVS = bool(HOST_ENV_USED)
GT_B = HOST_ENVS or HOST_LFO                  # DSP A copies gate and trigger for DSP B (its envelopes, its LFO)
ENV_TMIN = float(opt('ENV_TMIN', '0.001'))    # seconds at fader 0 (attack, decay, release)
ENV_TMAX = float(opt('ENV_TMAX', '10.0'))     # seconds at fader 255; exponential in between (C-S1SY's)

FS = 96000.0
# bank 5 (shared by slave A and B): written by engine.c
PITCH = [0xA030 + v for v in range(NV_MAX)]     # (note - 12) / 12 octaves (4.75 = A 440)
GATE = [0xA038 + v for v in range(NV_MAX)]      # 1.0 / 0.0
TRIG = [0xA060 + v for v in range(NV_MAX)]      # toggles 0 / 1 at every note start
DT = [0xA040 + v for v in range(NV_MAX)]        # s1_dt(): f / 96000 of the note (engine.c, per-note table)
RDT = [0xA048 + v for v in range(NV_MAX)]       # s1_rdt(): 96000 / f / 16384 (the DSP holds at most 512)
# bank 5, written by DSP A (a DSP stores only to A100..A1FF)
XV = [0xA151 + v for v in range(NV_MAX)]        # DSP A output of voice v = the input x of DSP B
X0, INB0 = 0xA188, 0xA1A0                   # exports i: X0 + 8 i + v; inputs B needs from A: INB0 + 8 j + v
# bank 5 A100..A1FF (what a slave DSP may store): A151.. x, A160.. / A168.. gate / trigger copies, A188..A19F export
# slots 0..2, A1A0..A1B7 the inputs for B (pitch, gate, trigger), A1B8..A1FF free: export slots 3..11


KX0 = 0xA1A0                                   # FUSED: bank-5 slots of the copied inputs, KX0 + 8 j + v (no exports,
                                                # no INB in a fused kernel: A1A0..A1FF are free), 12 slots
KV0, KV_TOP = 0x0180, {'A': 0x0300, 'B': 0x0400}    # FUSED: the kernel's voice blocks in each DSP's bank 0, below the
                                                # host's blocks (A 0x0300.., B 0x0400..; the OS has 0x0000..0x0017)
NA = 0                                          # FUSED: voices 0..NA-1 run the kernel on DSP A (set_split)
A_IN = ('IN:pitch', 'IN:gate', 'IN:trig', 'IN:dt', 'IN:rdt', 'IN:color0', 'IN:color1',   # made / written for DSP A
        'IN:host_mix')
B_IN = ('IN:env0', 'IN:env1', 'IN:lfo_amp', 'IN:lfo_filter')                            # made on DSP B


def export_addr(i, v):
    """bank-5 word of export slot i, voice v (slots 0..2 where MUTABLE-SYSTEM has them, then the free words)"""
    base = X0 + 8 * i if i < 3 else 0xA1B8 + 8 * (i - 3)
    if i < 0 or base + 7 > 0xA1FF:
        raise SystemExit(f'make_s1cc.py: s1_export({i}): SYSTEM-S1CC has export slots 0..11 (bank 5 A100..A1FF)')
    return base + v
# bank 0 (ring, S / S+1): the voice blocks of the kernel, sized by what it needs, in the regions our device-proven
# programs used (C-S1V8 / C-S1SY: slave A 0x0300.., slave B 0x0400..; MUTABLE-SYSTEM's kernel B: 0x0600..)
A_BLOCK0, B_BLOCK0 = 0x0300, 0x0600
B_OUT = [0x0400 + 2 * v for v in range(NV_MAX)]  # DSP B output of voice v (ring: read one sample later at S+1)
K_A_PARAM0, K_B_PARAM0 = 0x70, 0x60         # the kernel's knob tables: DSP A C070.., DSP B C060..
OUT_L, OUT_R = 0x6006, 0x6007               # slave B -> master (IOMD)
OUT_ZERO = [0x6008, 0x6009, 0x600A, 0x600B, 0x600C, 0x600D]
GAIN = 2.0 / (1.5 * NV)                     # the mix as C-S1SY13's (one voice at full scale: 1.33 / NV); set_voices
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
GB = [0xA160 + v for v in range(NV_MAX)]
TB = [0xA168 + v for v in range(NV_MAX)]
# LUT (2026-10-29): fixed tables in bank 5 A080.. (engine.c writes them at load and refresh; the ARM writes A000..A0FF),
# read by an indexed load (hset h6 + ldb5 at 0 and 1: linear interpolation between two points)
#   LFO_LUT=1      the host LFO's shapes from tables (sine, triangle, saw, square; N = 16): lfo = w g + a T[phase]
#   ENV_CURVE=...  one curve for every host envelope (sq, cube, sqrt, smooth, lin; N = 32): level = C(ADSR); the same
#                  table for all of them, each envelope with its own state
LFO_LUT = opt('LFO_LUT', '0') == '1'
ENV_CURVE = opt('ENV_CURVE', 'none')
CURVES = {'lin': lambda x: x, 'sq': lambda x: x * x, 'cube': lambda x: x ** 3, 'sqrt': math.sqrt,
          'smooth': lambda x: x * x * (3 - 2 * x)}
if ENV_CURVE != 'none' and ENV_CURVE not in CURVES:
    raise SystemExit(f'make_s1cc.py: ENV_CURVE={ENV_CURVE}: one of none, {", ".join(CURVES)}')
CURVE = ENV_CURVE != 'none' and HOST_ENVS
LUT0, LUT_EN, LUT_LN = 0xA080, 32, 16
LUT_SHAPES = ('sine', 'tri', 'saw')             # the square is clamp(256 saw) (hlfo: amp 256, then the clamp)


def lut_layout():
    """(values, {table: first word}) of the bank-5 tables: the envelope curve (LUT_EN + 1 points of 0..1), then the
    LFO shapes (LUT_LN + 1 points of the phase -1..1 each, as hlfo_voice_b's formulas in the emulator: triangle =
    ph_tri (-1 at p = -0.5, +1 at 0.5), sine = sin(pi p) (the true sine; the formula's is t (1.5 - 0.5 t^2)), saw p,
    the square = clamp(256 saw) in the code)"""
    vals, at = [], {}
    if CURVE:
        at['env'] = LUT0 + len(vals)
        vals += [CURVES[ENV_CURVE](i / LUT_EN) for i in range(LUT_EN + 1)]
    if LFO_LUT:
        f = {'sine': lambda p: math.sin(math.pi * p), 'tri': lambda p: 2 / math.pi * math.asin(math.sin(math.pi * p)),
             'saw': lambda p: p}
        for name in LUT_SHAPES:
            at[name] = LUT0 + len(vals)
            vals += [f32(f[name](-1 + 2 * i / LUT_LN)) for i in range(LUT_LN + 1)]
    if len(vals) > 0x80:
        raise SystemExit(f'make_s1cc.py: LUT {len(vals)} words, room for 128 (A080..A0FF)')
    return vals, at


def lut_h(word):
    """h6 of a bank-5 word (hset: address = floor(h 16384) + Y)"""
    return (word & 0x3FF) / 16384


def env_out(i, v):
    """where host envelope i of voice v leaves the level the kernel and COLOR MOD read (C(level) with ENV_CURVE)"""
    return cmt(4 if i == 0 else 3, v) if cmod_idx() else block('B', v) + H_ENV_STRIDE * i + (6 if CURVE else 0)


def luts_all():
    """LFO_LUT / ENV_CURVE: every table lookup of the sample in one unit after the voices (h6 and h7 are one each per
    DSP: inside a voice the lookups were a serial chain; here two run at a time, h6 on r0..r2 and h7 on r4..r6,
    and the scheduler overlaps them): the envelopes' C(level) -> env_out (the kernel reads it one sample late), the
    LFO's T[x] -> ly (its voice reads it one sample late)"""
    jobs = []
    for v in range(NV):
        if CURVE:
            for i in HOST_ENV_USED:
                jobs.append((block('B', v) + H_ENV_STRIDE * i, [env_out(i, v)] + ([ENVX[i][v]] if CMOD and not MOD_B else []),
                             lut_h(lut_layout()[1]['env']), float(LUT_EN)))
        if LFO_LUT:
            hb = host_b_block(v)
            jobs.append((hb['lx'], [hb['ly']], 'lfo', None))
    if not jobs:
        return []
    chains = []
    for k, (src, dsts, base, scale) in enumerate(jobs):         # microcode/lut.s
        x, t, d = ('r0', 'r1', 'r2') if k % 2 == 0 else ('r4', 'r5', 'r6')
        chains.append(frag('lut_job', x=x, t=t, d=d, src=src, dsts=dsts, scale=scale, h=6 + k % 2,
                           base=base if base != 'lfo' else ('r3' if k % 2 == 0 else 'r7')))
    out = frag('lut_head', n=len(jobs))
    for k in range(0, len(chains), 2):
        x, y = chains[k], chains[k + 1] if k + 1 < len(chains) else []
        for i in range(max(len(x), len(y))):
            out += x[i:i + 1] + y[i:i + 1]
    return out


# ARM_ENV=1 / ARM_LFO=1 (2026-10-30): the host's ADSRs / LFO run on the slave ARM (engine.c tick), one value per
# voice in a DSP B parameter that the hardware ramps (FCE4, 4.00 ms per ramp with y = 0 whatever the distance); a
# hook in the slave OS's 1 ms cyclic handler advances each voice every 4 ms (docs/examples_system_s1cc_README.md
# §3.7). DSP B only reads the parameter.
ARM_ENV = opt('ARM_ENV', '1') == '1' and HOST_ENVS   # default since 2026-11-03 (FCEC every 1 ms, measured)
ARM_LFO = opt('ARM_LFO', '1') == '1' and HOST_LFO
if ARM_LFO and LFO_LUT:
    raise SystemExit('make_s1cc.py: ARM_LFO and LFO_LUT together: the LFO is either on the ARM or on DSP B')
# ARM_RAMP=0 (default): the value written at every 1 ms tick (FCEC), steps of 1 ms: <= 1 ms late, attacks down to
# ~1 ms, ids C080.. (room for more voices). ARM_RAMP=1: one point every 4 ms, the hardware ramps (FCE4) linearly
# between points (its own steps of 1/3 ms), ~4 ms late. Measured on the slave's DSP B (2026-11-03, ramp_probe): ramps
# reach the program only for ids below 0x80 (C061..C07F: yes; C080..C0FF: no); FCEC reaches C060..C0FF; ids from
# 0x100 are not plain parameters (writing C100..C13F left the slave silent)
ARM_RAMP = opt('ARM_RAMP', '0') == '1'
ARM_SEG = 384 if ARM_RAMP else 96               # samples per step: a 4 ms ramp (y = 0), or the 1 ms tick
ARM_FADE = opt('ARM_FADE', '1') == '1'   # 0: no DELAY TIME (the LFO's fade at 1)
# diagnostics, heard on the device (2026-10-30): ARM_DIAG=1 DSP B adds 0.1 x C11F to the output and engine.c
# toggles C11F at every note on / off (a click: the positive check); ARM_TICK=1 the 1 ms tick also toggles it every
# 4 ticks (a 125 Hz buzz while the tick runs)
ARM_DIAG = opt('ARM_DIAG', '0') == '1'
ARM_TICK = opt('ARM_TICK', '0') == '1'
# ARM_PROBE=1 (2026-11-03, with ARM_ENV and the kernel sdk/s1cc/examples/arm_probe): telemetry of the ARM's time base
# through voice NV-1's envelope parameters, heard as tones: env 0 = (calls of tick() mod 64) / 64, env 1 = (envelope
# steps mod 64) / 64. Leave voice NV-1 unplayed
ARM_PROBE = opt('ARM_PROBE', '0') == '1'
# ARM_RAMPTEST=1 (with sdk/s1cc/examples/arm_probe/ramp_probe.c): the ARM tests every kernel table of DSP B in turn,
# an immediate write (FCEC) and a hardware ramp (FCE4) per 128 ms slot (docs/S1_ARM_CLOCK.md)
ARM_RAMPTEST = opt('ARM_RAMPTEST', '0') == '1'
# DSP B parameters: envelope i of voice v at P_ENV0 + 8 i + v; lf, fade at P_LFO0 / P_FADE0 + v. With ramps they must
# be below 0x80: C060..C07F, and the kernel's DSP B tables move to C080.. (written with FCEC, on a knob move only)
P_ENV0, P_LFO0, P_FADE0 = [int(x, 0) for x in opt('ARM_P0', '0x60,0x70,0x78' if ARM_RAMP else '0x80,0x90,0x98').split(',')]
if (ARM_ENV or ARM_LFO) and P_ENV0 <= K_B_PARAM0 < P_FADE0 + NV_MAX:
    K_B_PARAM0 = P_FADE0 + NV_MAX                   # the kernel's DSP B tables after the ARM's parameters


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
ENVX = ([0xA170 + v for v in range(NV_MAX)], [0xA178 + v for v in range(NV_MAX)])
MOD_DSP = opt('MOD_DSP', 'auto')
if MOD_DSP not in ('A', 'B', 'auto'):
    raise SystemExit('make_s1cc.py: MOD_DSP must be A, B or auto')
MOD_B = MOD_DSP != 'A'                          # where the pitch envelope and COLOR MOD are now (main() may change it)
# COLOR MOD on DSP B by an indexed load (CMOD_IDX=1, default since it ran on the SYSTEM-1, MP-CSAW2-8 8 voices
# 2026-10-29; 0: the weighted sum of the five sources): the ARM writes
# per color w0, w and the offset of the chosen source in the voice block (/16384); per voice h6 = block + offset,
# `ldidx` reads the source, color = w0 + w src. The LFO and the sub are copied into the voice block first (bank 0).
# Indexed transfers: exact on the SYSTEM-8 master (re/dspverify.py --programs, 2026-10-29); none in Roland's
# SYSTEM-1 programs (docs/NATIVE_OPS.md)
CMOD_IDX = opt('CMOD_IDX', '1') == '1'
CMT0 = 0x0500                                   # DSP B bank 0: the table of COLOR MOD sources (CMOD_IDX), free words


def cmt(s, v):
    """the table word of COLOR MOD source s (1 LFO, 2 pitch env, 3 filter env, 4 amp env, 5 sub) of voice v: one row
    of 16 words per source, voice v at 2 v (ring words S, S + 1), so `ldidx r, 2 v` reads it with h6 at the row"""
    return CMT0 + 16 * s + 2 * v


def cmod_idx():
    """COLOR MOD by an indexed load now (MOD_B can change in main())"""
    return bool(CMOD and MOD_B and CMOD_IDX and HOST_LFO)
PEB = [0xA128 + v for v in range(NV_MAX)]
SUBB = [0xA130 + v for v in range(NV_MAX)]
COLB = ([0xA138 + v for v in range(NV_MAX)], [0xA140 + v for v in range(NV_MAX)])
# LFO SHAPE -> C042..C045 (alpha, beta, K, w): lfo = t (alpha + beta t^2) + ph_clamp(K p) + w g, t the triangle of
# the phase p, g the sample & glide value (C041 = its coefficient k; k = 1 is the sample & hold)
LFO_SHAPES = [(1.5, -0.5, 0.0, 0.0), (1.0, 0.0, 0.0, 0.0), (0.0, 0.0, 1.0, 0.0), (0.0, 0.0, 256.0, 0.0),
              (0.0, 0.0, 0.0, 1.0), (0.0, 0.0, 0.0, 1.0)]       # sine, triangle, saw, square, S&G, S&H
if LFO_LUT:                                     # (h of the shape's table, amp, -, w): S&G / S&H only w g
    _AT = {n: LUT0 + k * (LUT_LN + 1) + ((LUT_EN + 1) if CURVE else 0) for k, n in enumerate(LUT_SHAPES)}
    LFO_SHAPES = [(lut_h(_AT[n]), 1.0, 0.0, 0.0) for n in LUT_SHAPES] + [(lut_h(_AT['saw']), 256.0, 0.0, 0.0)] + \
        [(lut_h(_AT['sine']), 0.0, 0.0, 1.0)] * 2                  # sine, triangle, saw, square, S&G, S&H
A6_LPITCH, A6_PAMT, A6_RANGE1 = 0x4B, 0x5E, 0x5F   # DSP A: LFO TO PITCH, PITCH ENV AMOUNT, OSC1 RANGE (octaves)
# OSC1 RANGE 64'..2' (0..5) -> RANGE_64FT .. RANGE_64FT + 5 octaves. -3 (8' = 0) sounded two octaves under the stock
# SYSTEM-1 at every position (2026-11-03, MP-CSAW2 on the device: its 4' = the stock 16', its 2' = the stock 8'): -1
RANGE_64FT = -1.0
# DSP B: the LFO (rate, S&G coefficient, shape alpha / beta / K / w, fade step, -> filter, -> amp M / |M|)
B6_LRATE, B6_LSG, B6_LWAVE, B6_LDELAY, B6_LFILT, B6_LAMP, B6_LAMPABS = 0x48, 0x49, 0x4A, 0x4E, 0x4F, 0x50, 0x51
LFB = [0xA120 + v for v in range(NV_MAX)]           # bank 5: lf of voice v, DSP B -> DSP A
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
    # 'xo': the kernel's output for the mixer; FUSED: 'hm' = the mixer's SUB / NOISE term the kernel adds itself
    return w + (['xo'] if HOST_MIX and not FUSED else []) + (['hm'] if FUSED else [])


def host_b_words():
    """the host LFO's words per voice on DSP B, after its envelopes': last trigger, phase, S&H, S&G, fade; -> amp
    and -> filter for the kernel (same sample); the note start and the pitch envelope (level, latch)"""
    w = ['bpt', 'lph', 'lsh', 'lsg', 'fade', 'lfa', 'lff'] + (['bd', 'pe', 'pf'] if PENV and MOD_B else []) + \
        (['lx', 'ly'] if LFO_LUT else []) if HOST_LFO else []      # LFO_LUT: the table position, its T (luts_all)
    return w


def set_mod_dsp(on_b):
    """put the pitch envelope and COLOR MOD on DSP B (True) or DSP A: the host's word counts follow"""
    global MOD_B, H_A_WORDS, H_B_WORDS
    MOD_B = on_b
    H_A_WORDS, H_B_WORDS = len(host_a_words()), len(host_b_words())


def set_voices(n):
    """build n voices (the address lists hold all NV_MAX, the programs use the first n)"""
    global NV, GAIN
    NV, GAIN = n, 2.0 / (1.5 * n)
    set_split(min(NA, n))


def set_split(na):
    """FUSED: the kernel of voices 0..na-1 on DSP A, the others on DSP B"""
    global NA
    NA = na


def kdsp(v):
    """FUSED: the DSP that runs the kernel of voice v"""
    return 'A' if v < NA else 'B'


def kv_words():
    """FUSED: bank-0 words of the kernel per voice: state, scratch, the first-sample flag"""
    st = KDEF.dsps['A']
    return len(KDEF.states) + st.get('scratch', 0) + bool(st.get('initflag'))


def kv_fits():
    return all(KV0 + 2 * kv_words() * n <= KV_TOP[d] for d, n in (('A', NA), ('B', NV - NA)))


def kv_block(v):
    """FUSED: the kernel's bank-0 block of voice v on its DSP"""
    if not kv_fits():
        raise SystemExit(f'make_s1cc.py: fused kernel: {kv_words()} words x {max(NA, NV - NA)} voices do not fit in '
                         f'bank 0 0x{KV0:04x}..')
    return KV0 + 2 * kv_words() * (v if v < NA else v - NA)


def mix_src(v):
    """the word DSP B's mix reads for voice v: its DSP B output (ring S+1), or DSP A's output"""
    if FUSED:
        return B_OUT[v] + 1 if kdsp(v) == 'B' else XV[v]
    return B_OUT[v] + 1 if HAS_B else XV[v]


H_A_WORDS = len(host_a_words())
H_B_WORDS = len(host_b_words())
LFO_WORDS = 2                                   # after the voice blocks: a noise source (noise, Weyl counter)


# ---- the kernel's addresses ---------------------------------------------------------------------------------------
def layout():
    """{param name: (count, A base or None, B base or None)}: a table used by one DSP only gets no slot on the other"""
    used = {st: {m.split(':')[1] for m in KDEF.placeholders(st) if m.startswith('P:')} for st in KDEF.dsps}
    if FUSED:                                    # the whole voice may run on either DSP
        used['B'] = used['A']
    out, a, b = {}, K_A_PARAM0, K_B_PARAM0
    for p in KDEF.params:
        ab = bb = None
        if p['name'] in used.get('A', ()):
            ab, a = a, a + p['count']
        if p['name'] in used.get('B', ()):
            bb, b = b, b + p['count']
        out[p['name']] = (p['count'], ab, bb)
    return out


def check_arm_ids():
    """the kernel's DSP B parameters: below 0x100 (ids from 0x100 are not plain parameters, 2026-11-03), and apart
    from the ARM's (ARM_ENV / ARM_LFO, P_ENV0 .. P_FADE0 + NV_MAX)"""
    used = [(bb, bb + cnt) for cnt, ab, bb in layout().values() if bb is not None]
    top = max([b for _, b in used] + [0])
    if top > 0x100:
        raise SystemExit(f'make_s1cc.py: the kernel\'s DSP B parameters reach C{top:03x}, past C0FF')
    if (ARM_ENV or ARM_LFO) and any(a < P_FADE0 + NV_MAX and b > P_ENV0 for a, b in used):
        raise SystemExit(f'make_s1cc.py: the kernel\'s DSP B parameters overlap the ARM\'s C{P_ENV0:03x}..'
                         f'C{P_FADE0 + NV_MAX - 1:03x}')


def words(dsp):
    """bank-0 words a voice of this DSP needs: state, scratch (spills), the first-sample flag"""
    host = H_ENV_WORDS + H_B_WORDS if dsp == 'B' else H_A_WORDS
    if dsp not in KDEF.dsps or FUSED:          # FUSED: the kernel's words are in kv_block                   # no S1_DSP_B: only the host's envelopes (COLOR MOD)
        return host
    st = KDEF.dsps[dsp]
    return host + sum(1 for s in KDEF.states if s['dsp'] == dsp) + st.get('scratch', 0) + bool(st.get('initflag'))


def bank0_top(dsp):
    """the first bank-0 word after a DSP's voice blocks and its noise source (it must be <= 0x0800)"""
    return (A_BLOCK0 if dsp == 'A' else B_BLOCK0) + 2 * words(dsp) * NV + 2 * LFO_WORDS


def block(dsp, v):
    """bank-0 block of voice v (2 ring words per value: S and S+1)"""
    base = A_BLOCK0 if dsp == 'A' else B_BLOCK0
    if bank0_top(dsp) > 0x0800:
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
        addr['IN:env0'], addr['IN:env1'] = (env_out(0, v) + int(CURVE), env_out(1, v) + int(CURVE))
        # (CURVE: luts_all writes C(level) after the voices, the kernel reads it one sample later: ring word + 1)
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


def k_inputs():
    """FUSED: the inputs the kernel reads"""
    return [k for k in KDEF.placeholders('A') if k in A_IN + B_IN]


def host_view(v):
    """FUSED: where each input of voice v is, on the DSP that makes it -> {key: (dsp, address)}"""
    a = addresses('A', v)
    out = {k: ('A', a[k]) for k in A_IN if k in a}
    out['IN:host_mix'] = ('A', host_a_block(v)['hm'])     # microcode/split.s mix_term
    if HOST_ENVS:
        for i in (0, 1):
            out[f'IN:env{i}'] = ('B', env_out(i, v) + int(CURVE))
    if HOST_LFO:
        hb = host_b_block(v)
        out.update({'IN:lfo_amp': ('B', hb['lfa']), 'IN:lfo_filter': ('B', hb['lff'])})
    if CMOD and MOD_B:                          # the colors are made on DSP B (COLB, bank 5): both DSPs read them
        for i in COLOR_USED:
            out[f'IN:color{i}'] = ('AB', COLB[i][v])
    if GT_B:                                    # DSP A's copies of gate and trigger, for DSP B
        out['IN:gate'], out['IN:trig'] = ('AB', GB[v]), ('AB', TB[v])
    return out


def k_copies(dsp, v):
    """FUSED: the (source, bank-5 slot) copies DSP dsp makes for voice v's kernel on the other DSP"""
    if kdsp(v) == dsp:
        return []
    view = host_view(v)
    return [(view[k][1], KX0 + 8 * j + v) for j, k in enumerate(k_inputs()) if view[k][0] == dsp]


def kaddr(v):
    """FUSED: the kernel's addresses for voice v on its DSP"""
    dsp, view = kdsp(v), host_view(v)
    addr = {}
    for j, k in enumerate(k_inputs()):
        if k not in view:
            raise SystemExit(f'make_s1cc.py: fused kernel reads {k}, which the host does not make')
        where, a = view[k]
        addr[k] = a if dsp in where else KX0 + 8 * j + v
    if KX0 + 8 * len(k_inputs()) > 0xA200:
        raise SystemExit('make_s1cc.py: fused kernel: more inputs than bank-5 slots A1A0..A1FF')
    base, k = kv_block(v), 0
    for s in KDEF.states:
        addr[f'S{s["index"]}'] = base + 2 * k
        k += 1
    for j in range(KDEF.dsps['A'].get('scratch', 0)):
        addr[f'T{j}'] = base + 2 * k
        k += 1
    if KDEF.dsps['A'].get('initflag'):
        addr['I'] = base + 2 * k
    for name, (cnt, ab, bb) in layout().items():
        pb = ab if dsp == 'A' else bb
        for i in range(cnt):
            addr[f'P:{name}:{i}'] = 0xC000 + pb + i
    addr['OUT'] = XV[v] if dsp == 'A' else B_OUT[v]
    return addr


def k_voice(dsp, v):
    """FUSED: what DSP dsp runs for voice v's kernel: the copies for the other DSP, or the kernel itself"""
    cp = k_copies(dsp, v)
    L = frag('kcopy', v=v, pairs=cp) if cp else []
    return L + (KDEF.place('A', kaddr(v)) if kdsp(v) == dsp else [])


# ---- microcode templates --------------------------------------------------------------------------------------------
# The host's assembly is in microcode/*.s, as fragments; the Python here only chooses them and fills them in.
#   .fragment NAME ARG ...         starts fragment NAME (until the next .fragment); frag(NAME, ARG=value, ...)
#   {expr} {expr:04x} {expr!r}     a Python expression over the ARGs and this module's globals, formatted
#   .if EXPR / .elif EXPR / .else / .endif, .for NAMES in EXPR / .endfor     (they nest)
#   .include NAME ARG=expr, ...    the lines of another fragment
#   ;; ...                         a comment of the file only; `; ...` lines go into the program
# Indentation is for the reader only (every line is stripped).
MICROCODE = os.path.join(HERE, 'microcode')
_FIELD = re.compile(r'\{([^{}]+?)(?:!([rs]))?(?::([^{}!]*))?\}')
_FRAGS = {}                                         # name -> (args, parsed body)


def _parse(body, k, stop):
    """body lines [(where, text)] from k into nodes, up to a directive in stop -> (nodes, index of that line)"""
    nodes = []
    while k < len(body):
        where, s = body[k]
        word, _, rest = s.partition(' ')
        if word in stop:
            return nodes, k
        if word == '.if':
            arms = []
            while True:
                sub, k = _parse(body, k + 1, ('.elif', '.else', '.endif'))
                arms.append((rest.strip(), where, sub))
                if k >= len(body):
                    raise SystemExit(f'{where}: .if without .endif')
                where, s = body[k]
                word, _, rest = s.partition(' ')
                if word == '.elif':
                    continue
                if word == '.else':
                    sub, k = _parse(body, k + 1, ('.endif',))
                    if k >= len(body):
                        raise SystemExit(f'{where}: .else without .endif')
                    arms.append(('True', where, sub))
                break
            nodes.append(('if', arms))
        elif word == '.for':
            names, sep, expr = rest.partition(' in ')
            if not sep:
                raise SystemExit(f'{where}: .for NAMES in EXPR')
            sub, k = _parse(body, k + 1, ('.endfor',))
            if k >= len(body):
                raise SystemExit(f'{where}: .for without .endfor')
            nodes.append(('for', where, [n.strip() for n in names.split(',')], expr.strip(), sub))
        elif word == '.include':
            name, _, args = rest.strip().partition(' ')
            nodes.append(('include', where, name, args))
        elif word in ('.slot', '.class'):             # assembler directives (asm/asm.py), kept as lines
            nodes.append(('line', where, s))
        elif word.startswith('.'):
            raise SystemExit(f'{where}: unexpected {word}')
        else:
            nodes.append(('line', where, s))
        k += 1
    return nodes, k


def _load_frags():
    for fn in sorted(os.listdir(MICROCODE)):
        if not fn.endswith('.s'):
            continue
        frags, name = {}, None
        for no, line in enumerate(open(os.path.join(MICROCODE, fn)), 1):
            s, where = line.strip(), f'microcode/{fn}:{no}'
            if not s or s.startswith(';;'):
                continue
            if s.startswith('.fragment'):
                name, *args = s.split()[1:]
                if name in _FRAGS or name in frags:
                    raise SystemExit(f'{where}: fragment {name} twice')
                frags[name] = (args, [])
            elif name is None:
                raise SystemExit(f'{where}: code before the first .fragment')
            else:
                frags[name][1].append((where, s))
        for name, (args, body) in frags.items():
            _FRAGS[name] = (args, _parse(body, 0, ())[0])


_CODE = {}


def _eval(expr, ns, where):
    try:
        if expr not in _CODE:
            _CODE[expr] = compile(expr, where, 'eval')
        return eval(_CODE[expr], ns)
    except Exception as e:
        raise SystemExit(f'{where}: {expr}: {type(e).__name__}: {e}')


def _run(nodes, ns, out):
    for n in nodes:
        if n[0] == 'line':
            def field(m, where=n[1]):
                x = _eval(m.group(1), ns, where)
                return format(repr(x) if m.group(2) == 'r' else x, m.group(3) or '')
            out.append(_FIELD.sub(field, n[2]))
        elif n[0] == 'if':
            for cond, where, sub in n[1]:
                if _eval(cond, ns, where):
                    _run(sub, ns, out)
                    break
        elif n[0] == 'for':
            _, where, names, expr, sub = n
            for x in _eval(expr, ns, where):
                inner = dict(ns)
                inner.update({names[0]: x} if len(names) == 1 else dict(zip(names, x)))
                _run(sub, inner, out)
        else:
            _, where, name, args = n
            out += frag(name, **_eval(f'dict({args})', ns, where))


def frag(name, /, **kw):
    """the lines of microcode fragment name with its ARGs kw"""
    if not _FRAGS:
        _load_frags()
    if name not in _FRAGS:
        raise SystemExit(f'make_s1cc.py: no microcode fragment {name} (microcode/*.s)')
    args, nodes = _FRAGS[name]
    if set(args) != set(kw):
        raise SystemExit(f'make_s1cc.py: fragment {name} takes {", ".join(args) or "nothing"}, given {", ".join(kw)}')
    out = []
    _run(nodes, dict(globals(), **kw), out)
    return out


# ---- programs -------------------------------------------------------------------------------------------------------
def host_b_block(v):
    """the host LFO's words of voice v on DSP B (host_b_words): name -> address"""
    return {n: block('B', v) + 2 * (H_ENV_WORDS + j) for j, n in enumerate(host_b_words())}


def host_a_block(v):
    """the host's words of voice v on DSP A (host_a_words): name -> address"""
    return {n: block('A', v) + 2 * j for j, n in enumerate(host_a_words())}


def hlfo_global(dsp='A'):
    """a DSP's global noise source (microcode/lfo.s noise)"""
    return frag('noise', dsp=dsp, G=lfo_block(dsp))


def hlfo_voice_b(v):
    """the host LFO of voice v on DSP B, before its envelopes; with the pitch envelope (MOD_DSP=B) after it
    (microcode/lfo.s lfo_b)"""
    return frag('lfo_b', v=v, hb=host_b_block(v), G=lfo_block('B'))


def colors_b(v):
    """COLOR MOD of voice v on DSP B, after its envelopes (microcode/color.s colors_b): color 0 in r0, color 1 in r2"""
    return frag('colors_b', v=v, hb=host_b_block(v), e=block('B', v), acc=dict(zip(COLOR_USED, ('r0', 'r2'))))


def colors_idx_all():
    """COLOR MOD of every voice by indexed loads (CMOD_IDX), after all the voices (microcode/color.s colors_idx)"""
    return frag('colors_idx')


def hlfo_voice(v):
    """the host's DSP A part of voice v, before the kernel (microcode/host_a.s host_a)"""
    return frag('host_a', v=v, h=host_a_block(v))


def voice_a(v):
    """DSP A code of voice v: the host's (four registers), the kernel's, and the host's mixer"""
    L = [f'; voice {v}'] + (hlfo_voice(v) if HOST_LFO else []) + \
        (frag('mix_term', v=v, h=host_a_block(v)) + k_voice('A', v) if FUSED else KDEF.place('A', addresses('A', v)))
    if HOST_MIX and not FUSED:                  # kernel (its output word xo) + SUB x sub + NOISE x noise -> x of DSP B
        L += frag('mixer', v=v, h=host_a_block(v))
    if HOST_ENVS and not HOST_LFO:             # gate and trigger for the host's envelopes on DSP B (with the host LFO,
                                                # hlfo_voice stores them from its loads)
        L += frag('gate_copy', v=v)
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
    head = frag('head_a')
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
    """DSP B as (the head: the mix, microcode/host_b.s head_b; [the code of each voice pair, or voice])"""
    L = frag('head_b')
    units = []
    if HAS_B or HOST_ENVS or HOST_LFO or FUSED:
        v = 0
        while v < NV:
            if PAIRS_B and v + 1 < NV:          # two voices, independent registers (as DSP A, PAIRS=1)
                units.append(_interleave(voice_b(v), _upper(voice_b(v + 1))))
                v += 2
            else:
                units.append(voice_b(v))
                v += 1
        if LFO_LUT or CURVE:                    # the table lookups of all voices (before COLOR MOD reads the levels)
            units.append(luts_all())
        if cmod_idx():                          # COLOR MOD of all voices, by indexed loads
            units.append(colors_idx_all())
    return L, units


def voice_b(v):
    """DSP B code of voice v: the host's envelopes (four registers, microcode/host_b.s host_env) and the kernel's"""
    L = [f'; voice {v}'] + (hlfo_voice_b(v) if HOST_LFO else [])
    if HOST_ENVS:
        b = block('B', v)
        for i in HOST_ENV_USED:
            e = b + H_ENV_STRIDE * i
            lvl = e if CURVE else env_out(i, v)     # the ADSR's level (CURVE: luts_all makes C(level) from it)
            L += frag('host_env', v=v, i=i, e=e, lvl=lvl)
    if CMOD and MOD_B and not cmod_idx():
        L += colors_b(v)
    if cmod_idx():                                  # the sub square into the COLOR MOD table
        L += frag('sub_to_cmt', v=v)
    if FUSED:
        L += k_voice('B', v)
    elif HAS_B:
        L += KDEF.place('B', addresses('B', v))
    return L


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
          0xC000 + A6_RANGE1: RANGE_64FT + min(max(k['range1'], 0), 5),
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


EDM_TAP = opt('EDM_TAP', '0') == '1'           # master B taps of the delay memory (microcode/edm.s, MUTABLE_MEMORY.md §3.3)
EDM_RES = (0x402E, 0x4031)                    # bank-2 words of the taps' results (the OS uses 4004..402D; 4040: no echo)
EDM_POS = 0x0200                              # master B bank 0: the position counter (the OS effect uses 0000..0033)


EDM_ENTRY = opt('EDM_ENTRY', '0') == '1'       # our master B program at 0x0 (microcode/edm.s edm_entry)
if EDM_ENTRY:                                 # kept as the record of the 2026-11-03 probes, refused for builds
    raise SystemExit('make_s1cc.py: EDM_ENTRY queues 0x0000, which a PLUG-OUT must never do (system-1/WAVE_BUS.md '
                     '§5.3: it hung / muted the device in the 2026-11-03 probes, docs/MUTABLE_MEMORY.md §3.3)')
EDM_CALL_MODE = opt('EDM_CALL', '0')          # '1': our tap of the OS line; 'write': our own line (edm_call.s)
EDM_CALL = EDM_CALL_MODE in ('1', 'write')    # the requests from inside the OS effect at 0x0 (microcode/edm_call.s)
if EDM_CALL:
    EDM_TAP = True                            # the main reads the results (master_edm without its own requests)
EDM_ARGS = dict(EDM_RES=EDM_RES, EDM_POS=EDM_POS, EDM_AGE=48000 / 16384, EDM_LEN=576000 / 16384, EDM_GAIN=0.7,
                EDM_REQ=not EDM_CALL)
# EDM_ECF=1: two lines of our own in the delay memory through master B's map table (MUTABLE_MEMORY.md §3.3.4): the
# main stores the send to each line's write word and adds its read word (the next one) to the output; region 0
# (model.c edm_ecf) maps the words. Word 4034..4037: past everything the OS effect and EDM_RES use
EDM_ECF = opt('EDM_ECF', '0') == '1'
ECF_WORDS = (0x4034, 0x4036)
if EDM_ECF:
    EDM_TAP = True                            # the main as written, one line per instruction (section())
EDM_STEP = opt('EDM_STEP', 'pass')               # edm_entry's body: 'pass' (no tapreq) or 'taps'


def edm_entry():
    """master B 0x0 (microcode/edm.s edm_entry): written whole in the fragment, with its markers and the entry's
    calls (cond / raw words: assembled with allow_inferred); the SYSTEM-1 rules checked on the body"""
    lines = frag('edm_entry', EDM_STEP=EDM_STEP, **EDM_ARGS)
    # the rules on the body only: the head stores E021 to A000 twice in one instruction, as every stock master B entry
    body = lines[lines.index('f7 r7, r1') + 1:lines.index('f7 r7, r0')] if 'f7 r7, r1' in lines else []
    for w in s1rules.check('s1mb0', body, slave=False):
        print('warning:', w)
    r = asm.assemble('\n'.join(lines) + '\n', _ISA[0] if _ISA else None, allow_inferred=True)
    if not r.ok:
        raise SystemExit(r.errors)
    return r.words, r, lines


def edm_call_header(path):
    """EDM_CALL: microcode/edm_call.s assembled into the halfword arrays model.c writes into master B (edm_call.h)"""
    out = ['/* generated by make_s1cc.py (EDM_CALL) from microcode/edm_call.s */']
    block = (('edm_call_block_w', dict(EDM_RES=EDM_RES, EDM_AGE=EDM_ARGS['EDM_AGE'], EDM_BASE=int(opt('EDM_BASE', '296')), EDM_WR=0x4034,
                                             EDM_RBASE=int(opt('EDM_RBASE', '256'))))
             if EDM_CALL_MODE == 'write' else ('edm_call_block', dict(EDM_RES=EDM_RES, EDM_AGE=EDM_ARGS['EDM_AGE'])))
    for name, kw in (block, ('edm_call_site', {})):
        r = asm.assemble('\n'.join(frag(name, **kw)) + '\n', _ISA[0] if _ISA else None)
        if not r.ok:
            raise SystemExit(r.errors)
        name = 'edm_call_block' if name.startswith('edm_call_block') else name
        out.append(f'static const uint16_t {name}[{len(r.words)}] = {{ ' + ', '.join('0x%04x' % w for w in r.words) + ' };')
    open(path, 'w').write('\n'.join(out) + '\n')


def prog_master():
    """C-S1SY13's master B (microcode/master.s master), or with EDM_TAP its taps of the delay memory (microcode/edm.s),
    or with EDM_ECF its own delay-memory lines (microcode/edm_ecf.s)"""
    if EDM_ECF:
        return frag('master_ecf', ECF_W1=ECF_WORDS[0], ECF_W2=ECF_WORDS[1], ECF_GAIN=0.7)
    if EDM_TAP:
        return frag('master_edm', **EDM_ARGS)
    if EDM_ENTRY and opt('EDM_MAIN', '') == 'direct':
        return frag('master_direct')
    return frag('master')


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
TRY_FORMS = opt('TRY_FORMS', '').strip()
# PAIRS: DSP A runs voices two at a time, the even one in r0..r3 and the odd one in r4..r7, interleaved, so the
# scheduler can put the ops of both in one instruction (one voice is a chain of dependencies). On when the kernel is
# built with --regs-a 0,1,2,3 (and DSP B too with --regs-b 0,1,2,3)
# automatic (PAIRS=0 / 1 forces it): pairs when the kernel's DSP A code is in r0..r3
_A_LOW = not any(re.search(r'\br[4-7]\b', x) for x in KDEF.dsps['A']['lines'])
PAIRS = {'0': False, '1': True}.get(opt('PAIRS', ''), _A_LOW)
if PAIRS and not _A_LOW:
    raise SystemExit('make_s1cc.py: PAIRS=1 needs the kernel built with --regs-a 0,1,2,3 (its DSP A uses r4..r7)')
# DSP B in pairs too when its kernel is in r0..r3 (--regs-b 0,1,2,3, or --regs 0,1,2,3); else one voice at a time
PAIRS_B = PAIRS and not any(re.search(r'\br[4-7]\b', x) for x in KDEF.dsps.get('B', {}).get('lines', []))
TRIED = collections.Counter()                     # op groups not proven on the device that this build used


def sched_setup():
    """the scheduler as the SYSTEM-1 needs it -> (proven op groups, op groups it may use: None = all)"""
    dspsched.Ins.one_bank5 = True                  # rule 5: one bank-5 access per instruction (load or store)
    dspsched.Ins.shapes = [(collections.Counter(acc), ops) for acc, ops in s1rules.proven_shapes()]
    proven = dspsched.load_proven()
    return proven, None if TRY_FORMS == 'all' else proven | dspsched.parse_forms(TRY_FORMS)


def section(name, lines, record=True):
    """a section program, scheduled with the op groups proven on the device only (asm/proven_forms.txt; with
    TRY_FORMS also those), one bank-5 access per instruction (rule 5) and only instruction shapes that are part of a
    proven program's; then checked against every SYSTEM-1 rule (asm/s1rules.py). SYSTEM-1 start marker (0a02 ff71),
    the code, end sequence"""
    proven, allowed = sched_setup()
    if name == 's1mm1' and EDM_TAP:          # tapreq / f6 / hset: one line per instruction, as written (microcode/edm.s)
        for w in s1rules.check(name, lines, slave=False, proven_ops=proven):
            print('warning:', w)
        r = assemble(lines)
        return [0x0A02, 0xFF71] + r.words + [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001], r, lines
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
    lines = frag('close', word=word)
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
        hd = head if name == main else frag('section_head', name=name, dsp=dsp, a=a)
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
    if EDM_ENTRY and EDM_STEP == 'head':     # control: the head alone, nothing else of the set changes
        progs['s1mb0'] = edm_entry()
    elif EDM_ENTRY:                          # our master B entry and the three ends it calls / jumps to (as the
        progs['s1mb0'] = edm_entry()         # stock PLUG-OUTs' master B sets carry them: 0x5FF2 / 0x6FF2 empty,
        empty = [0x0A02, 0xFF71, 0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001]   # 0x7FF2 the frame counter)
        progs['s1mb5'] = (empty, None, [])
        progs['s1mb6'] = (list(empty), None, [])
        progs['s1mb7'] = close(0xA000)
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
        out.append(f'/* OSC RANGE 64\'..2\' (0..5) -> octaves {RANGE_64FT:+g}..{RANGE_64FT + 5:+g} */')
        out.append(table('range_bits', [RANGE_64FT + r for r in range(6)]))
        out.append('/* LFO SHAPE -> (alpha, beta, K, w) of DSP B B6_LWAVE.. */')
        out.append(table('lfo_shape_bits', [x for sh in LFO_SHAPES for x in sh], 4))
        out.append(table('lfo_delay_bits', [lfo_delay(v) for v in range(256)]))
        out.append(table('lfo_pitch_bits', [lfo_pitch(v) for v in range(256)]))
        out.append(table('lfo_filt_bits', [lfo_filt(v) for v in range(256)]))
        out.append(table('lfo_amp_bits', [lfo_amp(v) for v in range(256)]))
        out.append(table('lfo_ampabs_bits', [abs(lfo_amp(v)) for v in range(256)]))
        out.append(table('half_bits', [v / 510.0 for v in range(256)]))
    check_arm_ids()
    out.append(f'#define ARM_ENV {int(ARM_ENV)}\n#define ARM_LFO {int(ARM_LFO)}\n'
               f'#define ARM_DIAG {int(ARM_DIAG)}\n#define ARM_TICK {int(ARM_TICK)}\n#define ARM_PROBE {int(ARM_PROBE)}\n'
               f'#define ARM_RAMPTEST {int(ARM_RAMPTEST)}\n'
               f'#define ARM_RAMP {int(ARM_RAMP)}\n#define ARM_FADE {int(ARM_FADE)}\n'
               f'#define ARM_SEG_MS {ARM_SEG // 96}  /* one step: a 4 ms ramp or the 1 ms tick */')
    if ARM_ENV or ARM_LFO:
        tabs = arm_tables()
        out.append(f'/* ARM_ENV / ARM_LFO: DSP B parameters (envelope i of voice v: P_ENV0 + 8 i + v; lf, fade: P_LFO0 /\n'
                   f'   P_FADE0 + v); per step of {ARM_SEG} samples ({ARM_SEG // 96} ms), Q24 (1 = 1 << 24) */\n'
                   f'#define P_ENV0 0x{P_ENV0:03x}\n#define P_LFO0 0x{P_LFO0:03x}\n#define P_FADE0 0x{P_FADE0:03x}')
        itab = lambda name, vals, t='uint32_t': (f'static const {t} {name}[{len(vals)}] = {{\n    ' +  # noqa: E731
                                                  ',\n    '.join(', '.join(f'{x}u' for x in vals[i:i + 8])
                                                                 for i in range(0, len(vals), 8)) + '\n};')
        for name in (('env_att_seg', 'env_dec_seg') if ARM_ENV else ()) + \
                (('lfo_seg_inc', 'lfo_fade_seg', 'lfo_sg_seg') if ARM_LFO else ()):
            out.append(itab(name, tabs[name]))
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


def arm_tables():
    """engine.c's ARM_ENV / ARM_LFO tables (Q24, per step of ARM_SEG samples): the one-pole's factor over a step
    (1 - rate)^ARM_SEG, the LFO's phase step (2^32 = one cycle), fade step and S&G factor"""
    q24 = lambda x: max(0, min(1 << 24, round(x * (1 << 24))))  # noqa: E731
    return {'env_att_seg': [q24((1 - attack_rate(v)) ** ARM_SEG) for v in range(256)],
            'env_dec_seg': [q24((1 - decay_rate(v)) ** ARM_SEG) for v in range(256)],
            'lfo_seg_inc': [round(lfo_hz(v) * ARM_SEG / FS * 2 ** 32) & 0xFFFFFFFF for v in range(256)],
            'lfo_fade_seg': [q24(min(1.0, lfo_delay(v) * ARM_SEG)) for v in range(256)],
            'lfo_sg_seg': [q24(1 - (1 - lfo_sg(v)) ** ARM_SEG) for v in range(256)]}


class ArmModel:
    """engine.c's ARM_ENV / ARM_LFO (env_step, lfo_step) for --sim: the same Q24 integer arithmetic, one step per
    ARM_SEG samples (ARM_RAMP=0: the 1 ms tick, a value written at once), into slave B's parameters"""
    Q1 = 1 << 24

    def __init__(self, params_b, knobs):
        self.p, self.t = params_b, arm_tables()
        self.env = [[255, 0, 128, 60], [0, 0, 128, 60]]          # S, A, D, R (henv_knob)
        for i in range(2):
            if f'env{i}' in knobs:
                self.env[i] = list(knobs[f'env{i}'])
            elif HOST_ENVS:
                self.env[i] = list(H_ENV_DEFAULT[i])
        k = dict(HLFO_DEFAULT, **{n: x for n, x in knobs.items() if n in HLFO_DEFAULT})
        self.sh, self.rate, self.delay = min(k['lfo_shape'], 5), k['lfo_rate'], k['lfo_delay']
        self.st = [[0] * NV for _ in range(2)]                 # 0 idle, 1 attack, 2 decay, 3 release
        self.lvl = [[0] * NV for _ in range(2)]
        self.ph, self.h, self.g, self.fade = [0] * NV, [0] * NV, [0] * NV, [0] * NV
        self.on, self.restart, self.rnd = [0] * NV, [0] * NV, 0x12345678
        self.ramps = {}                                # parameter -> (start, end, steps done)

    @staticmethod
    def qmul(a, b):
        return (a * b) >> 24

    @staticmethod
    def s32(x):
        x &= 0xFFFFFFFF
        return x - (1 << 32) if x & 0x80000000 else x

    def gate(self, v, on):
        """arm_gate"""
        if ARM_LFO:
            if on:
                self.restart[v] = 1
            self.on[v] = on
        if ARM_ENV:
            for i in range(2):
                self.st[i][v] = 1 if on else (0 if self.st[i][v] == 0 else 3)

    RAMP_STEPS = 12                                 # a 4 ms ramp of the hardware: steps of 1/3 ms (32 samples)

    def write(self, pid, q):
        """FCEC: at once; ARM_RAMP: FCE4, a linear ramp from the parameter's value now to q over ARM_SEG samples"""
        if ARM_RAMP:
            self.ramps[pid] = (self.p[0xC000 + pid], q / self.Q1, 0)
        else:
            self.p[0xC000 + pid] = f32(q / self.Q1)

    def sub(self):
        """one hardware ramp step (every 32 samples)"""
        for pid, (a, b, j) in list(self.ramps.items()):
            j += 1
            self.p[0xC000 + pid] = f32(a + (b - a) * min(1.0, j / self.RAMP_STEPS))
            self.ramps[pid] = (a, b, j)

    def env_step(self, i, v):
        Q1, top = self.Q1, self.Q1 + self.Q1 // 4
        S, A, D, Rl = self.env[i]
        sl = (S << 24) // 255
        l, st = self.lvl[i][v], self.st[i][v]
        if st == 1:
            l = top - self.qmul(top - l, self.t['env_att_seg'][A])
            if l >= Q1:
                l, self.st[i][v] = Q1, 2
        elif st == 2:
            l = sl + self.qmul(l - sl, self.t['env_dec_seg'][D])
        elif st == 3:
            l = self.qmul(l, self.t['env_dec_seg'][Rl])
            if l < Q1 >> 14:
                l, self.st[i][v] = 0, 0
        else:
            return
        self.lvl[i][v] = l
        self.write(P_ENV0 + 8 * i + v, l)

    def lfo_shape(self, sh, ph, v):
        Q1, p = self.Q1, self.s32(ph) >> 7
        t = 2 * Q1 - 2 * p if p > Q1 // 2 else -2 * Q1 - 2 * p if p < -Q1 // 2 else 2 * p
        if sh == 0:
            return self.qmul(t, Q1 + Q1 // 2 - self.qmul(self.qmul(t, t), Q1 // 2))
        if sh == 1:
            return t
        if sh == 2:
            return p
        if sh == 3:
            return Q1 if p > 0 else -Q1 if p < 0 else 0
        return self.g[v]

    def lfo_step(self, v, m=16):    # m sixteenths of a step
        Q1, r = self.Q1, self.rate
        if self.restart[v]:
            self.restart[v], self.ph[v] = 0, 0
        old = self.ph[v]
        self.ph[v] = (old + (self.t['lfo_seg_inc'][r] >> 4) * m) & 0xFFFFFFFF
        if self.s32(old) >= 0 and self.s32(self.ph[v]) < 0:
            self.rnd = (self.rnd * 1664525 + 1013904223) & 0xFFFFFFFF
            self.h[v] = self.s32(self.rnd) >> 7
        if self.sh == 5:
            self.g[v] = self.h[v]
        else:
            k = (self.t['lfo_sg_seg'][r] * m) >> 4
            self.g[v] += self.qmul(self.h[v] - self.g[v], min(k, Q1))
        if not ARM_FADE:
            self.fade[v] = Q1
        elif self.on[v]:
            self.fade[v] = min(Q1, self.fade[v] + (self.t['lfo_fade_seg'][self.delay] >> 4) * m)
        else:
            self.fade[v] = 0
        self.write(P_FADE0 + v, self.fade[v])
        self.write(P_LFO0 + v, self.qmul(self.lfo_shape(self.sh, self.ph[v], v), self.fade[v]))

    def step(self):
        """one tick step (tick: lfo_step, then both envelopes, every voice)"""
        for v in range(NV):
            if ARM_LFO:
                self.lfo_step(v)
            if ARM_ENV:
                self.env_step(0, v)
                self.env_step(1, v)


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
    arm = ArmModel(sl.d[1].params, knobs or {}) if ARM_ENV or ARM_LFO else None
    grid = 32 if ARM_RAMP else ARM_SEG              # the hardware ramp's step, or the 1 ms tick
    out, t, word = [], 0, (2, M_OUT[0])
    for te in sorted(e for e in events if e < n) + [n]:
        while te > t:                               # the ARM's step every ARM_SEG samples (4 ms ramps or 1 ms)
            k = min(te, (t // grid + 1) * grid) - t
            out += sl.run(k, (word,))[word]
            t += k
            if arm and ARM_RAMP and t % grid == 0:
                arm.sub()
            if arm and t % ARM_SEG == 0:
                arm.step()
        for v, note in events.get(te, []) if te < n else []:
            if note is None:
                a5[GATE[v] & 0x3FF] = 0.0
                if arm:
                    arm.gate(v, 0)
            elif note == 'gate':
                a5[GATE[v] & 0x3FF] = 1.0
                if arm:
                    arm.gate(v, 1)
            else:
                trig[v] = 1.0 - trig[v]
                a5[PITCH[v] & 0x3FF], a5[TRIG[v] & 0x3FF] = f32((note[1] - 12) / 12.0), trig[v]
                a5[DT[v] & 0x3FF] = f32(note_dt(note[1]))
                a5[RDT[v] & 0x3FF] = f32(1.0 / note_dt(note[1]) / 16384.0)
                if not gate_lag:
                    a5[GATE[v] & 0x3FF] = 1.0
                    if arm:
                        arm.gate(v, 1)
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
        ref = (rb if HAS_B or FUSED else ra) * GAIN
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
    names = [('x', XV[:NV])] + [(f'export {e}', [export_addr(e, v) for v in range(NV)]) for e in KDEF.exports] + \
            [(f'input {n}', [INB0 + 8 * j + v for v in range(NV)]) for j, n in enumerate(KDEF.inputs_b)] + \
            ([('gate / trigger copies', GB[:NV])] if HOST_ENVS else [])
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


def estimate():
    """{program: instructions} of each slave DSP with NV voices, each scheduled whole (no assembly, no sections: a
    section adds its head and the scheduler cannot fuse across its start, so the build counts as many or more), or
    None when the voice blocks do not fit in bank 0. A fraction of a second, against seconds for sources()"""
    if bank0_top('A') > 0x0800 or bank0_top('B') > 0x0800 or (FUSED and not kv_fits()):
        return None
    _, allowed = sched_setup()
    out = {}
    for name, parts in (('s1m0', prog_a_parts), ('s1m1', prog_b_parts)):
        head, units = parts()
        out[name] = instr_count(dspsched.schedule(head + [x for u in units for x in u], allowed))
    return out


def fits_estimate(n):
    """n voices may fit (estimate() within BUDGET, with some MOD_DSP placement) -> True / False"""
    set_voices(n)
    for on_b in ((True, False) if MOD_DSP == 'auto' and PENV else (MOD_B,)):
        set_mod_dsp(on_b)
        choose_split()
        e = estimate()
        if e and all(e[k] <= BUDGET[k] for k in BUDGET):
            return True
    return False


def choose_split():
    """FUSED: NA (voices whose kernel runs on DSP A) with the lowest load of the busier DSP, by the estimate;
    SPLIT=n fixes it"""
    if not FUSED:
        return
    if SPLIT_FIXED is not None:
        set_split(min(SPLIT_FIXED, NV))
        return
    best = None
    for na in range(NV + 1):
        set_split(na)
        e = estimate()
        load = max(e[k] / BUDGET[k] for k in BUDGET) if e else 99.0
        if best is None or load < best[0]:
            best = (load, na)
    set_split(best[1])


def choose_voices():
    """VOICES=auto: the most voices whose estimate fits, by binary search (estimate() grows with the voices)"""
    lo, hi = 1, NV_MAX                              # the answer is in lo..hi; 1 even when nothing fits (the build says)
    while lo < hi:
        mid = (lo + hi + 1) // 2
        if fits_estimate(mid):
            lo = mid
        else:
            hi = mid - 1
    set_voices(lo)
    return lo


def build():
    """sources() with the MOD_DSP placement -> (programs, the limits they break)"""
    if MOD_DSP == 'auto' and PENV:                  # the pitch envelope and COLOR MOD on the DSP with room for them
        tried = []
        for on_b in (True, False):
            set_mod_dsp(on_b)
            choose_split()
            e = estimate()                          # a placement whose estimate is already over cannot fit
            if e is None or any(e[k] > BUDGET[k] for k in BUDGET):
                tried.append((True, max(e[k] / BUDGET[k] for k in BUDGET) if e else 99.0, on_b))
                continue
            over, load = overruns(sources())
            tried.append((bool(over), load, on_b))
        _, _, on_b = min(tried)
        set_mod_dsp(on_b)
        print(f'MOD_DSP=auto: pitch envelope and COLOR MOD on DSP {"B" if on_b else "A"}')
    choose_split()
    if FUSED:
        print(f'fused kernel: voices 0..{NA - 1} on DSP A, {NA}..{NV - 1} on DSP B' if 0 < NA < NV else
              f'fused kernel: all {NV} voices on DSP {"A" if NA else "B"}')
    progs = sources()                               # (again after auto: TRIED and the sections are this placement's)
    return progs, overruns(progs)[0]


def main():
    if '--forms' in sys.argv:
        forms_report()
        return
    if VOICES == 'auto':
        n = choose_voices()
        print(f'VOICES=auto: {n} voices by the estimate (budget A {BUDGET["s1m0"]}, B {BUDGET["s1m1"]})')
        progs, over = build()
        while over and NV > 1:                      # the estimate is a lower bound: the sections may cost one voice
            print(f'make_s1cc.py: {NV} voices do not fit ({", ".join(over)}), one less')
            set_voices(NV - 1)
            progs, over = build()
    else:
        progs, over = build()
    os.makedirs(OUTDIR, exist_ok=True)
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
    open(os.path.join(OUTDIR, 's1_programs.h'), 'w').write(header(progs))
    if EDM_CALL:
        edm_call_header(os.path.join(OUTDIR, 'edm_call.h'))
    tried = os.path.join(OUTDIR, 'try_forms.txt')
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
