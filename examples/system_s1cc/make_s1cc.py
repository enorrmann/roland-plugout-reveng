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
    VOICES=n                1..8 (default 8): fewer voices, more instructions for each
    BUDGET_A=n, BUDGET_B=n  instructions per sample the slave's DSP A / DSP B may have, measured on the device
                            (docs/examples_system_1_synth_README.md §4): DSP A 693 ran (C-S1SY9), 702 hung
                            (MP-CSAW 7 voices, 2026-10-28), 826 hung; DSP B 708 ran. Defaults 693 / 708; above
                            them the build stops. BUDGET=n sets both
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
BUDGET = {'s1m0': int(os.environ.get('BUDGET_A', os.environ.get('BUDGET', '693'))),    # DSP A: 693 ran, 702 hung
          's1m1': int(os.environ.get('BUDGET_B', os.environ.get('BUDGET', '708')))}    # DSP B: 708 ran
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
HOST_ENV_USED = [0, 1] if CMOD else HOST_ENV_KERNEL
HOST_ENVS = bool(HOST_ENV_USED)
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
H_ENV_WORDS = 6 if HOST_ENVS else 0             # per voice on DSP B: (level, latch, last trigger) x 2
# COLOR MOD (CMOD). DSP B -> DSP A: the envelope levels, A170+v (AMP, env 0) / A178+v (FILTER, env 1), free words of
# bank 5. DSP A parameters of the host's LFO and COLOR MOD (engine.c): C040 LFO rate (phase -1..1 per sample), C041
# S&G coefficient, C042..C047 LFO shape weights (sine, triangle, saw, square, S&G, S&H; 1 for the chosen one), C048 /
# C049 pitch envelope decay / attack rates, C04A LFO fade step (DELAY TIME), C04B -> pitch (octaves), C04C -> filter
# (octaves), C04D / C04E -> amp M / |M|, C050 + 8 i: color i's weights (manual, LFO, pitch env, filter env, amp env,
# sub; LFO and sub are -1..1: with them selected manual = that weight = knob / 2)
ENVX = ([0xA170 + v for v in range(NV)], [0xA178 + v for v in range(NV)])
A6_LRATE, A6_LSG, A6_LWAVE, A6_PENV, A6_COL = 0x40, 0x41, 0x42, 0x48, (0x50, 0x58)
A6_LDELAY, A6_LPITCH, A6_LFILT, A6_LAMP, A6_LAMPABS = 0x4A, 0x4B, 0x4C, 0x4D, 0x4E
# the host LFO's controls, in engine.c's order (hlfo_panel): names, panel ids, defaults until a control moves
HLFO = [('lfo_shape', 0x160, 1), ('lfo_rate', 0x162, 128), ('lfo_delay', 0x161, 0), ('lfo_pitch', 0x163, 128),
        ('lfo_filter', 0x164, 128), ('lfo_amp', 0x165, 128), ('penv_a', 0x17B, 0), ('penv_d', 0x17C, 128),
        ('color0', 0x168, 128), ('color1', 0x16D, 128), ('mod0', 0x169, 0), ('mod1', 0x16E, 0)]
HLFO_DEFAULT = {n: d for n, _, d in HLFO}
NOISE_K = 7.31275                               # the LFO's noise (MUTABLE-SYSTEM step 9): n = wrap(K n + w), w a Weyl
NOISE_W = 166008623 / 2 ** 28                   # counter (odd multiple of 2^-28)
SG_K = 4.0                                      # sample & glide: k = min(1, SG_K x rate) (time constant = period / 8)


def host_a_words():
    """the host's words per voice on DSP A, before the kernel's (ring words): last trigger, LFO phase, S&H, S&G,
    fade; the modulated pitch / dt / rdt and the -> filter / -> amp terms (written and read in the same sample);
    COLOR MOD's pitch envelope (level, latch), sub phase and the colors"""
    w = (['pt', 'lph', 'lsh', 'lsg', 'fade'] if HOST_LFO else []) + [n + 'm' for n in PITCH_MOD]
    w += (['lff'] if LFO_F else []) + (['lfa'] if LFO_A else [])
    return w + (['pe', 'pf', 'sp'] + [f'color{i}' for i in COLOR_USED] if CMOD else [])


H_A_WORDS = len(host_a_words())
LFO_WORDS = 2 if HOST_LFO else 0                # after the voice blocks: the S&H noise (noise, Weyl counter)


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
    host = H_ENV_WORDS if dsp == 'B' else H_A_WORDS
    if dsp not in KDEF.dsps:                   # no S1_DSP_B: only the host's envelopes (COLOR MOD)
        return host
    st = KDEF.dsps[dsp]
    return host + sum(1 for s in KDEF.states if s['dsp'] == dsp) + st.get('scratch', 0) + bool(st.get('initflag'))


def block(dsp, v):
    """bank-0 block of voice v (2 ring words per value: S and S+1)"""
    base = A_BLOCK0 if dsp == 'A' else B_BLOCK0
    top = base + 2 * words(dsp) * NV + (2 * LFO_WORDS if dsp == 'A' else 0)
    if top > 0x0800:
        raise SystemExit(f'kernel DSP {dsp}: {words(dsp)} words x {NV} voices do not fit in bank 0 '
                         f'(0x{base:04x}..0x07ff)')
    return base + 2 * words(dsp) * v


def lfo_block():
    """DSP A bank-0 words of the global LFO (after the voice blocks): name -> address"""
    base = A_BLOCK0 + 2 * words('A') * NV
    return {n: base + 2 * j for j, n in enumerate(('nz', 'w'))}


def addresses(dsp, v):
    addr, k, base = {}, 0, block(dsp, v)
    if dsp == 'B' and HOST_ENVS:                  # the host's envelopes first: level, latch, last trigger each
        addr['IN:env0'], addr['IN:env1'] = base, base + 6
        k = H_ENV_WORDS
    if dsp == 'A' and HOST_LFO:                   # the host's words: LFO, modulated inputs, COLOR MOD
        h = {n: base + 2 * j for j, n in enumerate(host_a_words())}
        addr.update({f'IN:{n}': h[n] for n in [f'color{i}' for i in COLOR_USED]})
        addr.update({f'IN:{n}': h[n + 'm'] for n in PITCH_MOD})
        addr.update({k_: h[n] for k_, n in (('IN:lfo_filter', 'lff'), ('IN:lfo_amp', 'lfa')) if n in h})
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
    return addr


# ---- programs -------------------------------------------------------------------------------------------------------
def hlfo_global():
    """the noise source of the voices' S&H LFOs (MUTABLE-SYSTEM step 11 step_noise, proven on the device)"""
    G = lfo_block()
    return ['; host LFO: noise n = wrap(K n + w), w = Weyl counter',
            f'ld r2, [{G["nz"] + 1:04x}]', f'ld r0, [{G["w"] + 1:04x}]', 'mov r1, r1', f'add r0, #{NOISE_W!r}:32',
            'ph_wrap r0, r3, r0', f'st r0, [{G["w"]:04x}]', f'mul r2, r2, #{NOISE_K!r}', 'addxy r2, r2, r0',
            'ph_wrap r2, r3, r2', f'st r2, [{G["nz"]:04x}]']


def hlfo_voice(v):
    """the host's LFO of voice v and what it drives, before the kernel (uses r0..r7):
    * LFO (MUTABLE-SYSTEM step 11 step_lfo_voice, proven on the device): phase restarted at the note start, own S&H
      of the global noise at each wrap, own S&G; shapes x C042..C047 -> lfo -1..1; fade f 0 -> 1 from the gate
      (DELAY TIME), lf = f lfo
    * -> amp: 1 + f (M lf - |M|) (tremolo down from full level; M < 0 the inverted LFO) -> s1_lfo_amp()
    * -> filter: F lf octaves -> s1_lfo_filter()
    * -> pitch: y = P lf octaves; pitch + y, dt 2^y, rdt 2^-y (2^z: Taylor of e^(z ln2 / 2) to z^4, squared; 0.6
      cents at 0.7 octaves) -> the words the kernel reads as s1_pitch / s1_increment, s1_dt, s1_rdt
    * COLOR MOD (if the kernel reads s1_color): pitch envelope AD (the host ADSR's math, adsr(), proven on the
      device, with the gate at 1 and sustain 0: latch cleared at a note start, attack = 1 - latch, level toward
      1.25 attack at rate attack kA + latch kD), sub oscillator (square one octave down), the AMP / FILTER envelope
      levels from DSP B (A170+v / A178+v); color i = weights (C050 + 8 i ..) x (1, lf, pitch env, filter env,
      amp env, sub)"""
    h = {n: block('A', v) + 2 * j for j, n in enumerate(host_a_words())}
    G = lfo_block()
    w = lambda i: f'ld r4, [c0{A6_LWAVE + i:02x}]'  # noqa: E731
    L = [f'; voice {v}: host LFO, d = 1 at a note start (the trigger changed)',
         f'ld r3, [{TRIG[v]:04x}]', f'ld r0, [{h["pt"] + 1:04x}]', 'mov r3, r3', f'st r3, [{h["pt"]:04x}]',
         'subxy r0, r0, r3', 'mul r0, r0, r0',
         f'; voice {v}: LFO, phase 0 at a note start; r7 = 2 at a wrap',
         f'ld r1, [{h["lph"] + 1:04x}]', f'ld r4, [c0{A6_LRATE:02x}]', 'mov r3, r3', 'mad r1, r1, -r0',
         'addxy r7, r1, r4', 'ph_wrap r1, r3, r7', f'st r1, [{h["lph"]:04x}]', 'subxy r7, r1, r7',
         f'ld r3, [{h["lsh"] + 1:04x}]', f'ld r2, [{G["nz"]:04x}]', 'mov r4, r4',
         'subxy r4, r3, r2', 'mul r4, r4, r7', 'mad r3, r4, #0.5', f'st r3, [{h["lsh"]:04x}]',
         f'; voice {v}: sample & glide g += k (h - g)', f'ld r4, [c0{A6_LSG:02x}]', f'ld r2, [{h["lsg"] + 1:04x}]',
         'mov r7, r7', 'subxy r7, r2, r3', 'mad r2, r7, r4', f'st r2, [{h["lsg"]:04x}]',
         f'; voice {v}: shape = sine, tri, saw, square, S&G, S&H x C042..C047',
         w(5), 'mov r7, r7', 'mul r7, r3, r4', w(4), 'mov r3, r3', 'mad r7, r2, r4', w(2), 'mov r3, r3',
         'mad r7, r1, r4', w(3), 'mul r3, r1, #256.0', 'ph_clamp r3, r3, -r3', 'mad r7, r3, r4',
         w(1), 'ph_tri r3, r3, -r1', 'mad r7, r3, r4',
         'mul r2, r3, r3', w(0), 'mul r2, r2, #-0.5', 'add r2, #1.5', 'mul r2, r2, r3', 'mad r7, r2, r4',
         f'; voice {v}: fade (0 -> 1 from the gate, DELAY TIME), r7 = lf = lfo x fade',
         f'ld r2, [{h["fade"] + 1:04x}]', f'ld r4, [c0{A6_LDELAY:02x}]', f'ld r3, [{GATE[v]:04x}]', 'mov r1, r1',
         'addxy r2, r2, r4', 'mul r2, r2, r3', 'min r2, r2, #1.0', f'st r2, [{h["fade"]:04x}]', 'mul r7, r7, r2']
    if LFO_A:
        L += [f'; voice {v}: -> amp 1 + f (M lf - |M|)', f'ld r4, [c0{A6_LAMP:02x}]', f'ld r3, [c0{A6_LAMPABS:02x}]',
              'mov r1, r1', 'mul r5, r7, r4', 'mad r5, r2, -r3', 'add r5, #1.0', f'st r5, [{h["lfa"]:04x}]']
    if LFO_F:
        L += [f'; voice {v}: -> filter F lf', f'ld r4, [c0{A6_LFILT:02x}]', 'mov r1, r1', 'mul r5, r7, r4',
              f'st r5, [{h["lff"]:04x}]']
    if PITCH_MOD:
        L += [f'; voice {v}: -> pitch y = P lf octaves', f'ld r4, [c0{A6_LPITCH:02x}]', 'mov r1, r1', 'mul r4, r7, r4']
        if 'pitch' in PITCH_MOD:
            L += [f'ld r3, [{PITCH[v]:04x}]', 'mov r1, r1', 'addxy r3, r3, r4', f'st r3, [{h["pitchm"]:04x}]']
        for n, src, sign in (('dt', DT, 1.0), ('rdt', RDT, -1.0)):
            if n in PITCH_MOD:
                L += [f'mul r3, r4, #{sign * math.log(2) / 2!r}', 'mov r5, #0.041666666666666664',
                      'ymacd r5, r3, #0.16666666666666666', 'ymacd r5, r3, #0.5', 'ymacd r5, r3, #1.0',
                      'ymacd r5, r3, #1.0', 'mul r5, r5, r5', f'ld r3, [{src[v]:04x}]', 'mov r1, r1',
                      'mul r3, r3, r5', f'st r3, [{h[n + "m"]:04x}]']
    if not CMOD:
        return L
    L += [f'; voice {v}: COLOR MOD pitch envelope: the ADSR\'s latch form with gate 1, sustain 0',
          f'ld r1, [{h["pe"] + 1:04x}]', f'ld r2, [{h["pf"] + 1:04x}]', 'mov r5, #0.0', 'mov r6, #1.0',
          'mad r2, r2, -r0',                                                     # latch *= 1 - d
          'addxy r4, r1, #-0.999', 'mul r4, r4, #256.0', 'mul r4, r4, #256.0', 'max r4, r4, r5', 'min r4, r4, r6',
          'max r2, r2, r4', f'st r2, [{h["pf"]:04x}]', 'subxy r3, r2, #1.0',     # attack = 1 - latch
          f'ld r4, [c0{A6_PENV + 1:02x}]', 'mov r0, r0', 'mul r0, r3, r4',      # rate = attack kA + latch kD
          f'ld r4, [c0{A6_PENV:02x}]', 'mov r0, r0', 'mad r0, r2, r4',
          'mul r4, r3, #1.25', 'subxy r4, r1, r4', 'mad r1, r4, r0', 'min r1, r1, r6', f'st r1, [{h["pe"]:04x}]',
          f'; voice {v}: COLOR MOD sub oscillator',
          f'ld r2, [{h["sp"] + 1:04x}]', f'ld r0, [{DT[v]:04x}]', 'mov r3, r3',
          'addxy r0, r2, r0', 'ph_wrap r2, r3, r0', f'st r2, [{h["sp"]:04x}]', 'mul r2, r2, #256.0',
          'ph_clamp r2, r3, -r2',
          f'; voice {v}: COLOR MOD envelopes from DSP B', f'ld r4, [{ENVX[0][v]:04x}]', f'ld r0, [{ENVX[1][v]:04x}]']
    for i in COLOR_USED:
        c = A6_COL[i]
        L += [f'; voice {v}: color {i}', f'ld r5, [c0{c:02x}]', f'ld r6, [c0{c + 1:02x}]', 'mov r3, r3',
              'mad r5, r7, r6', f'ld r6, [c0{c + 2:02x}]', 'mov r3, r3', 'mad r5, r1, r6',
              f'ld r6, [c0{c + 3:02x}]', 'mov r3, r3', 'mad r5, r0, r6', f'ld r6, [c0{c + 4:02x}]', 'mov r3, r3',
              'mad r5, r4, r6', f'ld r6, [c0{c + 5:02x}]', 'mov r3, r3', 'mad r5, r2, r6',
              f'st r5, [{h[f"color{i}"]:04x}]']
    return L


def prog_a():
    L = [f'; s1m0: slave DSP A main @0x0100: {NV} voices x s1cc kernel {KERNEL} DSP A -> A151+v']
    if HOST_LFO:
        L += hlfo_global()
    for v in range(NV):
        L += [f'; voice {v}'] + (hlfo_voice(v) if HOST_LFO else []) + KDEF.place('A', addresses('A', v))
        if HOST_ENVS:                               # gate and trigger for the host's envelopes on DSP B
            L += [f'ld r0, [{GATE[v]:04x}]', f'st r0, [{GB[v]:04x}]', f'ld r1, [{TRIG[v]:04x}]', f'st r1, [{TB[v]:04x}]']
    return L


def prog_b():
    L = [f'; s1m1: slave DSP B main @0x0100: mix of {NV} voices x {GAIN!r} x C000 -> 6006 / 6007'
         + (f'; per voice s1cc kernel DSP B' if HAS_B else ''), 'mov r7, #0.0']
    for v in range(NV):                         # B outputs of the sample before (ring S+1); A outputs as they are
        L += [f'ld r{v & 1}, [{(B_OUT[v] + 1) if HAS_B else XV[v]:04x}]', f'addxy r7, r7, r{v & 1}']
    L += ['ld r6, [c000]', f'mul r7, r7, #{GAIN!r}', 'mul r7, r7, r6',
          f'st r7, [{OUT_L:04x}]', f'st r7, [{OUT_R:04x}]', 'mov r0, #0.0']
    L += [f'st r0, [{x:04x}]' for x in OUT_ZERO]
    if HAS_B or HOST_ENVS:
        for v in range(NV):
            L += [f'; voice {v}']
            if HOST_ENVS:
                b = block('B', v)
                L += ['mov r5, #0.0', 'mov r6, #1.0']
                for i in HOST_ENV_USED:
                    e = b + 6 * i
                    L += [f'; voice {v}: host envelope {i}'] + adsr(e, e + 2, e + 4, GB[v], TB[v], 0xC000 + H6_ENV[i])
                    if CMOD:                        # its level for COLOR MOD on DSP A
                        L += [f'st r1, [{ENVX[i][v]:04x}]']
            if HAS_B:
                L += KDEF.place('B', addresses('B', v))
    return L


def adsr(e, latch, pt, gate, trig, prm):
    """C-S1SY's ADSR, copied unchanged from examples/mutable_system/make_mutable.py step_adsr (proven on the device):
    -> r1 (stored at e); r5 = 0, r6 = 1; uses r0..r4. prm: sustain, attack, decay, release rate. latch = 1 once the
    attack reached 1; target 1.25 (attack) / S (decay) / 0 (release); a change of trig clears the latch (new note).
    The attack is g - latch: it follows the gate itself, so a trigger and a gate that arrive apart lose nothing."""
    return [f'ld r1, [{e + 1:04x}]', f'ld r2, [{latch + 1:04x}]', f'ld r4, [{gate:04x}]', f'ld r3, [{trig:04x}]',
            f'ld r0, [{pt + 1:04x}]', 'mov r5, r5',
            f'st r3, [{pt:04x}]',
            'subxy r0, r0, r3', 'mul r0, r0, r0',        # d = (trig - last trig)^2
            'subxy r0, r0, #1.0', 'mul r2, r2, r0',      # latch *= 1 - d
            'addxy r3, r1, #-0.999', 'mul r3, r3, #256.0', 'mul r3, r3, #256.0', 'max r3, r3, r5', 'min r3, r3, r6',
            'max r2, r2, r3', 'mul r2, r2, r4', f'st r2, [{latch:04x}]',
            'subxy r3, r2, r4',                          # attack = g - latch
            'subxy r4, r4, #1.0',                        # 1 - g
            f'ld r0, [{prm + 3:04x}]', 'mov r5, r5', 'mul r0, r0, r4',            # rate = (1 - g) kR
            f'ld r4, [{prm + 1:04x}]', 'mov r5, r5', 'mad r0, r3, r4',            #   + attack kA
            f'ld r4, [{prm + 2:04x}]', 'mov r5, r5', 'mad r0, r2, r4',            #   + latch kD
            f'ld r4, [{prm:04x}]', 'mov r5, r5', 'mul r4, r4, r2', 'mad r4, r3, #1.25',   # target
            'subxy r4, r1, r4', 'mad r1, r4, r0', 'min r1, r1, r6', f'st r1, [{e:04x}]']


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


def hlfo_params(knobs):
    """DSP A parameters of the host's LFO and COLOR MOD as engine.c writes them; knobs: HLFO's names"""
    k = dict(HLFO_DEFAULT, **{n: x for n, x in knobs.items() if n in HLFO_DEFAULT})
    pa = {0xC000 + A6_LRATE: f32(lfo_rate(k['lfo_rate'])), 0xC000 + A6_LSG: f32(lfo_sg(k['lfo_rate'])),
          0xC000 + A6_PENV: f32(decay_rate(k['penv_d'])), 0xC000 + A6_PENV + 1: f32(attack_rate(k['penv_a'])),
          0xC000 + A6_LDELAY: f32(lfo_delay(k['lfo_delay'])), 0xC000 + A6_LPITCH: f32(lfo_pitch(k['lfo_pitch'])),
          0xC000 + A6_LFILT: f32(lfo_filt(k['lfo_filter'])), 0xC000 + A6_LAMP: f32(lfo_amp(k['lfo_amp'])),
          0xC000 + A6_LAMPABS: f32(abs(lfo_amp(k['lfo_amp'])))}
    for j in range(6):
        pa[0xC000 + A6_LWAVE + j] = 1.0 if j == k['lfo_shape'] else 0.0
    for i in (0, 1):
        for j, x in enumerate(color_weights(k[f'color{i}'], k[f'mod{i}'])):
            pa[0xC000 + A6_COL[i] + j] = f32(x)
    return pa


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
TRIED = collections.Counter()                     # op groups not proven on the device that this build used


def section(name, lines):
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


def sources():
    return {'s1m0': section('s1m0', prog_a()), 's1m1': section('s1m1', prog_b()), 's1mm1': section('s1mm1', prog_master()),
            's1c0': close(0xA004), 's1c1': close(0xA005)}


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
        out.append(table('attack_bits', [attack_rate(v) for v in range(256)]))
        out.append(table('decay_bits', [decay_rate(v) for v in range(256)]))
        out.append(table('level_bits', [v / 255.0 for v in range(256)]))
    out.append(f'#define HOST_LFO {int(HOST_LFO)}')
    if HOST_LFO:
        out.append(f'/* the host LFO and COLOR MOD: DSP A parameters; controls (panel id, default); tables */\n'
                   f'#define A6_LRATE 0x{A6_LRATE:02x}\n#define A6_LSG 0x{A6_LSG:02x}\n#define A6_LWAVE 0x{A6_LWAVE:02x}\n'
                   f'#define A6_PENV 0x{A6_PENV:02x}\n#define A6_COL0 0x{A6_COL[0]:02x}\n#define A6_COL1 0x{A6_COL[1]:02x}\n'
                   f'#define A6_LDELAY 0x{A6_LDELAY:02x}\n#define A6_LPITCH 0x{A6_LPITCH:02x}\n'
                   f'#define A6_LFILT 0x{A6_LFILT:02x}\n#define A6_LAMP 0x{A6_LAMP:02x}\n#define A6_LAMPABS 0x{A6_LAMPABS:02x}')
        out.append('static const uint16_t hlfo_panel[12] = { ' + ', '.join(f'0x{p:03x}' for _, p, _ in HLFO) + ' };')
        out.append('static const uint8_t hlfo_default[12] = { ' + ', '.join(str(d) for _, _, d in HLFO) + ' };')
        out.append(table('lfo_rate_bits', [lfo_rate(v) for v in range(256)]))
        out.append(table('lfo_sg_bits', [lfo_sg(v) for v in range(256)]))
        out.append(table('lfo_delay_bits', [lfo_delay(v) for v in range(256)]))
        out.append(table('lfo_pitch_bits', [lfo_pitch(v) for v in range(256)]))
        out.append(table('lfo_filt_bits', [lfo_filt(v) for v in range(256)]))
        out.append(table('lfo_amp_bits', [lfo_amp(v) for v in range(256)]))
        out.append(table('lfo_ampabs_bits', [abs(lfo_amp(v)) for v in range(256)]))
        out.append(table('half_bits', [v / 510.0 for v in range(256)]))
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
        pa.update(hlfo_params(knobs))
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
    P = {k: chain.Program(progs[k][0], k) for k in ('s1m0', 's1m1', 's1mm1')}
    model = chain.Model(order=(0, 1, 2), shared5=((0, 1),), own_params=(0, 1, 2))
    sl = cchain.Slave({0: (P['s1m0'], []), 1: (P['s1m1'], []), 2: (P['s1mm1'], [])}, model)
    for d, prm in zip(sl.d, params(knobs or {})):
        d.params.update(prm)
    a5 = sl.d[0].bank5
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
        for sig, saved, used in cands:
            print(f'  {saved:4d} saved alone, used x{used:<4d} {dspsched.form_text(sig)}')
    print("test: TRY_FORMS='GROUP; GROUP' (or TRY_FORMS=all) sh examples/system_s1cc/build.sh with NAME=..., install, "
          'play; if it works: python3 tools/proven_forms.py promote examples/system_s1cc/build/try_forms.txt --by NAME')


def main():
    if '--forms' in sys.argv:
        forms_report()
        return
    progs = sources()
    over = []
    for name, (w, _, lines) in progs.items():
        open(os.path.join(OUTDIR, name + '.s'), 'w').write('\n'.join(lines) + '\n')
        open(os.path.join(OUTDIR, name + '.hex'), 'w').write(
            f'# {name}, own SYSTEM-1 program (make_s1cc.py KERNEL={KERNEL} VOICES={NV})\n'
            + '\n'.join('%04x' % x for x in w) + '\n')
        print(f'{name:6s} {instr_count(lines):4d} instructions, {len(w)} halfwords')
        if name in BUDGET and instr_count(lines) > BUDGET[name]:
            over.append(f'{name}: {instr_count(lines)} instructions, budget {BUDGET[name]}')
    print(f'kernel per voice: A {len(KDEF.dsps["A"]["lines"])} lines, {words("A")} words'
          + (f'; B {len(KDEF.dsps["B"]["lines"])} lines, {words("B")} words' if HAS_B else '; no DSP B'))
    crossing_report(progs)
    for st in KDEF.dsps:                       # spills, and r5 / r6 left unused: built for MUTABLE-SYSTEM's registers
        lines = KDEF.dsps[st]['lines']
        if KDEF.dsps[st].get('scratch') and not any(re.search(r'\br[56]\b', x) for x in lines):
            print(f'note: DSP {st} has {KDEF.dsps[st]["scratch"]} spill words and does not use r5 / r6: SYSTEM-S1CC '
                  'has no host keeping them, build the kernel with --regs 0,1,2,3,4,5,6,7 (docs/S1CC.md §1.1)')
    if over:
        raise SystemExit(f'make_s1cc.py: over the budget ({", ".join(over)}): fewer VOICES, a smaller kernel, or '
                         f'BUDGET_A / BUDGET_B=n at your own risk (DSP A: 702 hung the device)')
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
