"""C-S1SY: an 8-voice subtractive synth for the SYSTEM-1, all our own microcode (docs/examples_system_1_synth_README.md).

Extends C-S1V8 (examples/system_1): per voice two independent oscillators (wave + level each, DCO2 detuned), an
ADSR VCA, a 4-pole low-pass filter (cutoff, resonance, key track) and a second ADSR for the cutoff.

    slave A  s1m0 @0x0100  per voice: DCO1 + DCO2 (saw / pulse / triangle mix from the ARM) -> A068+v,
                           filter ADSR -> A070+v
    slave B  s1m1 @0x0100  first: sum of last sample's filter outputs x C000 -> 6006 / 6007 (6008..600D = 0);
                           per voice: amp ADSR, x = A068+v x amp env x comp, cutoff (base + amount x A070+v +
                           key track x pitch) -> g = 2 pi f / fs, ladder of 4 one-pole stages with feedback k
    master B s1mm1 @0x1000 6006 / 6007 x volume (C01C) -> A118 / A119 (unchanged from C-S1V4)

Built step by step on the hardware (STEP=n, STEPS below; default 7 = everything up to the waves, packed as Roland
does: pack()). The design above (STEP=0, STAGE=1..4) is the first all-at-once attempt: silent on the device, kept
as reference. The filter ADSR is still to come.

    python3 make_synth.py           -> s1*.s / s1*.hex here, build/s1_programs.h
    python3 make_synth.py --sim     -> also simulate notes through slave A, slave B and master B (emulator C core,
                                       seconds; SIM=py: the Python reference, minutes)
    python3 make_synth.py --sim-check -> C core against the Python reference, sample by sample
"""
import math
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
import asm  # noqa: E402

NV = int(os.environ.get('VOICES', '8'))
STEP = int(os.environ.get('STEP', '8'))     # incremental steps on top of C-S1V8 (STEPS below)
STAGE = int(os.environ.get('STAGE', '4'))   # the first all-at-once design (STEP=0): silent on hw 2026-10-23
AMP_ENV, FILTER, FILT_ENV = STAGE >= 2, STAGE >= 3, STAGE >= 4
FS = 96000.0

# bank 5 of the slave pair (A000 + index; written by the ARM with FD6C sel 0 unless noted)
PITCH = [0xA030 + v for v in range(NV)]     # (note - 12) / 12 octaves
GATE = [0xA038 + v for v in range(NV)]      # 1.0 / 0.0
TRIG = [0xA060 + v for v in range(NV)]      # toggles 0 / 1 at every note start (the envelopes restart)
OSC1, OSC2 = 0xA040, 0xA044                 # 4 words each: saw coef, -pulse threshold, pulse coef, triangle coef
OFF2 = 0xA048                               # DCO2 pitch offset (octaves)
CUT, RES, AMT, KT, COMP = 0xA050, 0xA051, 0xA052, 0xA053, 0xA054
FENV, AENV = 0xA058, 0xA05C                 # 4 words each: sustain, attack, decay, release rate
XV = [0xA068 + v for v in range(NV)]        # written by DSP A: the oscillators of voice v
FEV = [0xA070 + v for v in range(NV)]       # written by DSP A: the filter envelope of voice v
# bank 0 rings (store at S, read S + 1 one sample later), 0x10 words per voice, A and B apart
A_PH1, A_PH2, A_FE, A_FL, A_PT = 0, 2, 4, 6, 8
B_AE, B_AL, B_PT, B_Y1 = 0, 2, 4, 6         # Y1..Y4 at 6, 8, 10, 12
A_BASE = [0x0300 + 0x10 * v for v in range(NV)]
B_BASE = [0x0400 + 0x10 * v for v in range(NV)]
OUT_L, OUT_R = 0x6006, 0x6007
OUT_ZERO = [0x6008, 0x6009, 0x600A, 0x600B, 0x600C, 0x600D]
M_OUT = (0xA118, 0xA119)
M_ZERO = [0xA11A, 0xA11B, 0xA11C, 0xA11D]
M_VOLUME = 0x1C
LEVEL = 1.0 / NV
K_SAW = 2 * 440.0 / FS                      # phase step of A 440 (phase -1..1)
G_440 = 2 * math.pi * 440.0 / FS            # one-pole coefficient of 440 Hz
EXP_IN = math.log(2) / 16                   # 2^c = (e^(c ln2 / 16))^16, e^y by 4 Taylor terms
CUT_LO, CUT_HI = -8.0, 6.5                  # cutoff limits in octaves from 440 Hz (1.7 Hz .. 40 kHz before g clamp)
G_MAX = float(os.environ.get('GMAX', '0.7'))  # filter coefficient limit: k (g / (2 - g))^4 < 1 at Nyquist for
                                              # k = RES_MAX needs < 0.83; higher, the loop oscillates at 48 kHz
                                              # (Nyquist, inaudible; emulator 2026-10-23), not in the audio band
RESCOMP = os.environ.get('RESCOMP', '0') == '1'  # resonance compensated by cutoff, so the filter self-oscillates
# The loop (4 stages y += g (u - y), feedback k y4 of the last sample) oscillates for k > k_crit(g) =
# 4 c(g): 4.01 at 50 Hz, 4.29 at 1 kHz, 5.04 at 3 kHz, 9.6 at 8 kHz (an analog ladder: 4 everywhere). RESCOMP
# multiplies k by c(min(g, RC_GMAX)) (fit of degree 2, 1.2 % off up to g = 0.4); RES_MAX then puts the top of the
# knob RES_MAX / 4 above the threshold. Without it RES_MAX 3.6 < 4: no self-oscillation at any cutoff (emulator).
RC_A, RC_B, RC_GMAX = 0.778126, 2.778432, 0.4
RES_MAX = float(os.environ.get('RES_MAX', '4.4' if RESCOMP else '3.6'))


def a(x):
    return f'{x:04x}'


def exp2_to(dst, src):
    """dst = 2^src (src in r-register, |src| <= 8); uses r2, r3"""
    return [f'mul r2, {src}, #{EXP_IN!r}', 'mov r3, #0.041666666666666664:32',
            'ymacd r3, r2, #0.16666666666666666', 'ymacd r3, r2, #0.5', 'ymacd r3, r2, #1.0', 'ymacd r3, r2, #1.0',
            'mul r3, r3, r3', 'mul r3, r3, r3', 'mul r3, r3, r3', f'mul {dst}, r3, r3']


def osc(v, ph, off, prm):
    """one oscillator of voice v, pitch in r0; adds a*saw + b*pulse + c*tri to r7; uses r1..r6"""
    L = [f'ld r1, [{a(ph + 1)}]']
    if off:
        L += [f'ld r5, [{a(off)}]', 'addxy r2, r0, r5', 'add r2, #-4.75']
        L += exp2_to('r3', 'r2')
    else:
        L += ['addxy r2, r0, #-4.75'] + exp2_to('r3', 'r2')
    L += [f'mul r3, r3, #{K_SAW!r}', 'addxy r1, r3, r1', 'ph_wrap r1, r0, r1', f'st r1, [{a(ph)}]',
          f'ld r4, [{a(prm)}]', f'ld r5, [{a(prm + 1)}]', f'ld r6, [{a(prm + 2)}]',
          'mad r7, r1, r4',                              # saw
          'addxy r2, r1, r5', 'mul r2, r2, #256.0', 'min r2, r2, #1.0', 'max r2, r2, #-1.0',
          f'ld r4, [{a(prm + 3)}]',
          'mad r7, r2, r6',                              # pulse: sign(phase - t)
          'ph_tri r2, r0, -r1',
          'mad r7, r2, r4']                              # triangle
    return L


def adsr(e, latch, pt, gate, trig, prm):
    """ADSR -> r1 (also stored at e); keeps r7, uses r0..r6. latch = 1 once the attack reached 1 (decay / sustain);
    target = 1.25 in attack, S in decay, 0 in release; rate = kA / kD / kR; a change of trig clears the latch."""
    return [f'ld r1, [{a(e + 1)}]', f'ld r2, [{a(latch + 1)}]', f'ld r4, [{a(gate)}]', f'ld r5, [{a(trig)}]',
            f'ld r6, [{a(pt + 1)}]', f'ld r0, [{a(prm)}]',
            f'st r5, [{a(pt)}]',
            'subxy r6, r6, r5', 'mul r6, r6, r6',        # d = (trig - last trig)^2: 1 at a new note
            'mad r2, r2, -r6',                           # latch *= 1 - d
            'addxy r3, r1, #-0.999', 'mul r3, r3, #256.0', 'mul r3, r3, #256.0', 'max r3, r3, #0.0',
            'min r3, r3, #1.0',
            'max r2, r2, r3', 'mul r2, r2, r4', f'st r2, [{a(latch)}]',
            'subxy r6, r4, #1.0',                        # 1 - g
            'subxy r4, r2, r4',                          # attack = g - latch
            'mul r3, r2, r0',                            # target = latch * S
            f'ld r0, [{a(prm + 3)}]', f'ld r5, [{a(prm + 1)}]',
            'mad r3, r4, #1.25',                         #   + 1.25 * attack
            'mul r0, r0, r6',                            # rate = (1 - g) kR
            f'ld r6, [{a(prm + 2)}]',
            'mad r0, r4, r5', 'mad r0, r2, r6',          #   + attack kA + latch kD
            'subxy r3, r1, r3', 'mad r1, r3, r0', 'min r1, r1, #1.0', f'st r1, [{a(e)}]']


def prog_s1m0():
    L = [f'; s1m0: slave DSP A main @0x0100: {NV} voices x (DCO1 + DCO2) -> A068+v'
         + (', filter ADSR -> A070+v' if FILT_ENV else '')]
    for v in range(NV):
        b = A_BASE[v]
        L += [f'; voice {v}', 'mov r7, #0.0', f'ld r0, [{a(PITCH[v])}]']
        L += osc(v, b + A_PH1, None, OSC1)
        L += osc(v, b + A_PH2, OFF2, OSC2)
        L += [f'st r7, [{a(XV[v])}]']
        if FILT_ENV:
            L += adsr(b + A_FE, b + A_FL, b + A_PT, GATE[v], TRIG[v], FENV) + [f'st r1, [{a(FEV[v])}]']
    return L


def prog_s1m1():
    L = [f'; s1m1: slave DSP B main @0x0100: output = sum of the voices of the last sample, then {NV} voices x ('
         + ('amp ADSR' if AMP_ENV else 'gate') + (', 4-pole low-pass' if FILTER else '') + ')',
         'mov r7, #0.0']
    for v in range(NV):
        L += [f'ld r{v & 1}, [{a(B_BASE[v] + B_Y1 + 7)}]', f'addxy r7, r7, r{v & 1}']
    L += [f'ld r6, [c000]', f'mul r7, r7, #{LEVEL}', 'mul r7, r7, r6', f'st r7, [{a(OUT_L)}]', f'st r7, [{a(OUT_R)}]',
          'mov r0, #0.0'] + [f'st r0, [{a(x)}]' for x in OUT_ZERO]
    for v in range(NV):
        b = B_BASE[v]
        y = [b + B_Y1 + 2 * i for i in range(4)]
        L += [f'; voice {v}']
        if AMP_ENV:
            L += adsr(b + B_AE, b + B_AL, b + B_PT, GATE[v], TRIG[v], AENV)
        else:
            L += [f'ld r1, [{a(GATE[v])}]']
        L += [f'ld r7, [{a(XV[v])}]', 'mul r7, r7, r1']
        if not FILTER:
            L += [f'st r7, [{a(y[3])}]']
            continue
        L += [f'ld r6, [{a(COMP)}]', f'ld r0, [{a(CUT)}]', 'mul r7, r7, r6']
        if FILT_ENV:
            L += [f'ld r2, [{a(AMT)}]', f'ld r1, [{a(FEV[v])}]']
        L += [f'ld r3, [{a(PITCH[v])}]', f'ld r4, [{a(KT)}]']
        if FILT_ENV:
            L += ['mad r0, r2, r1']
        L += ['add r3, #-4.75', 'mad r0, r4, r3', f'min r0, r0, #{CUT_HI}', f'max r0, r0, #{CUT_LO}']
        L += exp2_to('r0', 'r0') + [f'mul r0, r0, #{G_440!r}', f'min r0, r0, #{G_MAX}']
        L += [f'ld r1, [{a(y[0] + 1)}]', f'ld r2, [{a(y[1] + 1)}]', f'ld r3, [{a(y[2] + 1)}]', f'ld r4, [{a(y[3] + 1)}]',
              f'ld r6, [{a(RES)}]',
              'mad r7, r6, -r4', 'min r7, r7, #1.5', 'max r7, r7, #-1.5']          # u = x - k y4, limited
        for i, (prev, cur) in enumerate((('r7', 'r1'), ('r1', 'r2'), ('r2', 'r3'), ('r3', 'r4'))):
            L += [f'subxy r5, {cur}, {prev}', f'mad {cur}, r5, r0', f'st {cur}, [{a(y[i])}]']
    return L


def prog_m1():
    L = ['; s1mm1: master DSP B @0x1000: slave 6006 / 6007 x volume (C01C) -> A118 / A119',
         f'ld r0, [{a(OUT_L)}]', f'ld r1, [{a(OUT_R)}]', f'ld r7, [c{M_VOLUME:03x}]',
         'mul r0, r0, r7', 'mul r1, r1, r7', f'st r0, [{a(M_OUT[0])}]', f'st r1, [{a(M_OUT[1])}]', 'mov r2, #0.0']
    L += [f'st r2, [{a(x)}]' for x in M_ZERO]
    return L


def regs_of(line):
    body = line.split(';')[0]
    return set(int(t) for t in __import__('re').findall(r'\br(\d)\b', body))


def schedule(lines):
    """a loaded register is usable one instruction later (DSP_ISA.md section 9): put a harmless move after a load
    when the next instruction uses (or overwrites) the loaded register"""
    out = []
    code = [x for x in lines if x.strip() and not x.startswith(';')]
    for i, x in enumerate(lines):
        out.append(x)
        if not x.startswith('ld '):
            continue
        dst = int(x.split()[1][1])
        rest = [y for y in lines[i + 1:] if not y.startswith(';')]
        if rest and dst in regs_of(rest[0]):
            spare = next(r for r in (5, 6, 4, 3, 2, 1, 0, 7) if r != dst)
            out.append(f'mov r{spare}, r{spare}')
    del code
    return out


def assemble(lines):
    r = asm.Assembler().assemble('\n'.join(lines) + '\n')
    if not r.ok:
        raise SystemExit(r.errors)
    return r


def program(lines):
    """a section program: SYSTEM-1 start marker (0a02 ff71), the code, end sequence"""
    lines = schedule(lines)
    r = assemble(lines)
    return [0x0A02, 0xFF71] + r.words + [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001], r, lines


def close(word):
    r = assemble([f'ld r0, [e000]', f'st r0, [{word:04x}]'])
    w = [0x0000] + r.words
    assert len(w) == 7
    return w, r, ['ld r0, [e000]', f'st r0, [{word:04x}]']


# ---- incremental steps on top of C-S1V8 (STEP=n build.sh; docs/examples_system_1_synth_README.md) ----------------
# Every step adds ONE thing to the last one that played on the hardware, with instructions already proven there.
#   1  DCO2 per voice: a second copy of C-S1V8's saw (same code), +7 cents, level 0.5, fixed in the microcode
#   2  OSC1 / OSC2 levels from the panel (0x174 / 0x175) through bank 5 A040 / A041 (ARM)
#   3  per-voice transport: DSP A stores each voice (DCO1 + DCO2, no gate) in A151+v; DSP B multiplies each by its
#      gate and mixes them (the place where the VCA and the filter will go). First try (B reads the gate A038+v
#      written by the ARM) was silent on hw; now DSP A copies the gates to A160+v
STEP_LEVEL1, STEP_LEVEL2 = 0xA040, 0xA041
STEP_XV = [0xA151 + v for v in range(NV)]
STEP_GB = [0xA160 + v for v in range(NV)]   # gate copied by DSP A for DSP B (B reading A038+v gave silence, step 3)
#   4  amp ADSR per voice on DSP B (faders 0x187..0x18A). As Roland's SH-2: panel values reach a slave DSP as its
#      bank-6 parameters (ARM FCEC(1, id, bits) -> C040..C043 of DSP B: sustain, attack, decay, release rate);
#      DSP A copies the trigger (A060+v -> A168+v) like the gate; B state in its bank 0 ring at 0x0400 + 0x10 v
STEP_TB = [0xA168 + v for v in range(NV)]
STEP_AENV_B = 0xC040
B6_AENV = 0x40
STEP_B_BASE = [0x0400 + (0x20 if STEP >= 8 else 0x10) * v for v in range(NV)]   # step 8: 0x20 (filter ADSR state)
#   5  OSC1 range, OSC2 range (0x16A / 0x16F: 64'..2' = -3..+2 octaves) and OSC2 fine tune (0x170: +-1 semitone)
#      -> parameters C044 / C045 / C046 of DSP A (FCEC(0, id, bits), as Roland); the fixed +7 cents goes away
B6_RANGE1, B6_RANGE2, B6_FINE2 = 0x44, 0x45, 0x46
FINE_SPAN = 1.0 / 12                        # octaves at either end of the fine tune knob
#   6  4-pole low-pass per voice on DSP B (after the amp ADSR): cutoff 0x17D, resonance 0x17F, key track 0x186 ->
#      parameters C048 cutoff (octaves from 440 Hz), C049 k, C04A input gain (1 + k/2) / 2, C04B key track of DSP B
#      (FCEC(1, ...)); DSP A copies the pitch (A030+v -> A178+v). Saturation with ph_clamp (as Roland's SH-2).
#      B outputs the sum of the last sample's filter outputs (one sample later), so r7 is free inside a voice
B6_CUT, B6_RES, B6_GAIN, B6_KT = 0x48, 0x49, 0x4A, 0x4B
STEP_PB = [0xA178 + v for v in range(NV)]
B_Y = [0x6, 0x8, 0xA, 0xC]                  # ring offsets of the 4 stage outputs in a B voice block
#   7  waves: OSC1 / OSC2 wave selector (0x167 / 0x16C) -> coefficients of saw, square, triangle as parameters
#      C050..C052 (OSC1) / C053..C055 (OSC2) of DSP A (FCEC(0, ...)); square = ph_clamp(256 x phase), triangle =
#      ph_tri(phase), both in the form of Roland's SH-2 (X = r3)
B6_WAVE1, B6_WAVE2 = 0x50, 0x53
#   8  filter ADSR per voice on DSP B (faders 0x182..0x185 -> C050..C053 of DSP B: sustain, attack, decay, release;
#      amount 0x181 -> C04C, bipolar: 128 = 0, 0 = -8 .. 255 = +8 octaves): cutoff = base + amount x env + key track; state at B voice block + 0x10
B6_FENV, B6_AMT = 0x50, 0x4C                # parameters of DSP B (DSP A's C050.. are the waves: other DSP)
B_FE = 0x10                                 # filter ADSR state: env +0x10, latch +0x12, last trigger +0x14
STEP_WAVES = [(1, 0, 0), (0, 1, 0), (0, 0, 1), (1, 0, 0), (1, 0, 0), (1, 0, 0)]   # selector 0..5: 2nd square, 3rd tri
V8_PHASE = [0x0300 + 0x10 * v for v in range(NV)]
V8_MIX = 0xA150
#   9  LFO (PARAMLOG 2026-10-23: rate 0x162, shape 0x160, delay / fade 0x161, -> pitch 0x163, -> filter 0x164,
#      -> amp 0x165), all on DSP A (B is at 708, the most tried): one global LFO, phase wrapped with ph_wrap (as
#      Roland), shapes = coefficients of triangle, saw, square, S&H, noise (C060..C064 of DSP A); per voice a fade
#      f (0 -> 1 linear from the gate, rate C059) scales the LFO; pitch + f P lfo (octaves), voice x (1 + f M (lfo - 1))
#      (tremolo down from full level), cutoff = base + f F lfo -> A180+v, which DSP B reads instead of its C048
#      (so the cutoff knob also goes to C05D of DSP A)
A6_LRATE, A6_LDELAY, A6_LPITCH, A6_LFILT, A6_LAMP, A6_CUT, A6_LWAVE = 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x60
LFO_PH, LFO_NZ, LFO_SH, LFO_W = 0x0280, 0x0282, 0x0284, 0x0286   # ring words (state: S written, S+1 read next sample)
LFO_P, LFO_F, LFO_M = 0x0288, 0x028A, 0x028C      # f-less products of this sample (written and read in it)
STEP_CB = [0xA180 + v for v in range(NV)]          # cutoff of voice v, DSP A -> DSP B
V9_FADE = [p + 4 for p in V8_PHASE]                # fade of voice v (ring, S / S+1)
V9_PM = [p + 6 for p in V8_PHASE]                  # modulated pitch of voice v (this sample)
NOISE_K = 7.31275                                  # noise: n = wrap(K n + w), w = wrap(w + NOISE_W) (Weyl counter:
NOISE_W = 166008623 / 2 ** 28                      # odd multiple of 2^-28, period 2^29 samples; n alone cycled after
                                                   # 7194 samples = 13 Hz, emulator 2026-10-23)
# shape 0..5 -> (triangle, saw, square, S&H, noise) [inferido: order of the SYSTEM-1 panel not checked]
#  10  the saw of C-S1V8 with ph_wrap (Roland's form: ph_wrap rD, r3, rY) instead of the 11-op manual wrap
#  11  LFO per voice: each voice its own phase (restarted at its note start) and S&H; the noise source stays global.
#      -> filter / -> amp bipolar: 128 = 0, below = inverted LFO. Voice x (1 + f (M lfo - |M|)): M > 0 = tremolo
#      down from full level, M < 0 the same on the inverted LFO (C05E = |M|). Per voice ring block V8_PHASE[v]:
#      +0/+2 DCO phases, +4 fade, +6 modulated pitch, +8 LFO phase, +10 S&H, +12 last trigger, +14 amp term
A6_LAMPABS = 0x5E
V11_LPH, V11_SH, V11_TRIG, V11_AMP = 8, 10, 12, 14   # offsets in the voice block
# step 11: shape 0..5 = the SYSTEM-1 panel (user, 2026-10-23): sine, triangle, saw, square, sample & glide,
# sample & hold -> coefficients C060..C065 of (sine, tri, saw, square, S&G, S&H). Sine = t (1.5 - 0.5 t^2) of the
# triangle t; S&G glides to the S&H value, g += k (h - g), k = min(1, SG_K x rate) (time constant = period / 8)
LFO_WAVES11 = [tuple(1 if i == k else 0 for i in range(6)) for k in range(6)]
SG_K = 4.0
V11_SG = [0x0240 + 2 * v for v in range(NV)]          # S&G state of voice v (ring, S / S+1)
LFO_WAVES = [(1, 0, 0, 0, 0), (0, 1, 0, 0, 0), (0, -1, 0, 0, 0), (0, 0, 1, 0, 0), (0, 0, 0, 1, 0), (0, 0, 0, 0, 1)]


def v8_saw(P):
    """C-S1V8's saw (examples/system_1/make_s1.py saw_voice) on r0 = pitch - 4.75 (+ offset), phase ring P:
    the new phase is left in r1; uses r2, r3 (steps <= 9 also r5 = 0, r6 = 1)"""
    L = [f'ld r1, [{P + 1:04x}]', 'mov r4, r4',
         f'mul r2, r0, #{EXP_IN!r}', 'mov r3, #0.041666666666666664:32',
         'ymacd r3, r2, #0.16666666666666666', 'ymacd r3, r2, #0.5', 'ymacd r3, r2, #1.0', 'ymacd r3, r2, #1.0']
    L += ['mul r3, r3, r3'] * 4
    L += ['mul r3, r3, #0.009166666666666667', 'addxy r1, r3, r1']
    if STEP >= 10:                               # step 10: one ph_wrap (Roland's form, X = r3) instead of 11 ops
        return L + ['ph_wrap r1, r3, r1', f'st r1, [{P:04x}]']
    L += ['addxy r2, r1, #-1.0', 'max r2, r2, r5']           # steps <= 9 as installed: manual wrap (lint warns)
    L += ['mul r2, r2, #256.0', 'min r2, r2, r6'] * 4
    L += ['mul r2, r2, #2.0', 'subxy r1, r2, r1', f'st r1, [{P:04x}]']
    return L


def step_wave(prm):
    """step 7: r1 = phase (= saw) -> r2 = a saw + b square + c triangle, a b c = parameters prm..prm+2; uses r3, r4.
    Each load has one instruction before its use (no filler)."""
    return [f'ld r4, [c0{prm:02x}]', 'mul r3, r1, #256.0', 'mul r2, r1, r4',
            f'ld r4, [c0{prm + 1:02x}]', 'ph_clamp r3, r3, -r3', 'mad r2, r3, r4',
            f'ld r4, [c0{prm + 2:02x}]', 'ph_tri r3, r3, -r1', 'mad r2, r3, r4']


def step_lfo():
    """step 9: the global LFO on DSP A -> LFO_P / LFO_F / LFO_M (lfo x pitch depth, x filter depth, x amp depth / 2
    x (lfo - 1)); uses r0..r4, r7 (before r5 / r6 / r7 are set for the voices)"""
    w = lambda i: f'ld r4, [c0{A6_LWAVE + i:02x}]'  # noqa: E731
    return ['; LFO: phase += rate, wrapped; r7 = 2 at a wrap (S&H takes a new noise value)',
            f'ld r1, [{LFO_PH + 1:04x}]', f'ld r4, [c0{A6_LRATE:02x}]', f'ld r2, [{LFO_NZ + 1:04x}]', 'mov r0, r0',
            'addxy r7, r1, r4', 'ph_wrap r1, r3, r7', f'st r1, [{LFO_PH:04x}]', 'subxy r7, r1, r7',
            f'ld r0, [{LFO_W + 1:04x}]', 'mov r0, r0', f'add r0, #{NOISE_W!r}:32', 'ph_wrap r0, r3, r0',
            f'st r0, [{LFO_W:04x}]',
            f'mul r2, r2, #{NOISE_K!r}', 'addxy r2, r2, r0', 'ph_wrap r2, r3, r2', f'st r2, [{LFO_NZ:04x}]',
            f'ld r3, [{LFO_SH + 1:04x}]', 'mov r0, r0', 'subxy r0, r3, r2', 'mul r0, r0, r7', 'mad r3, r0, #0.5',
            f'st r3, [{LFO_SH:04x}]',
            '; LFO: shape = tri, saw, square, S&H, noise x C060..C064',
            w(3), 'mov r0, r0', 'mul r7, r3, r4', w(4), 'mov r0, r0', 'mad r7, r2, r4', w(1), 'mov r0, r0',
            'mad r7, r1, r4', w(2), 'mul r3, r1, #256.0', 'ph_clamp r3, r3, -r3', 'mad r7, r3, r4',
            w(0), 'ph_tri r3, r3, -r1', 'mad r7, r3, r4',
            '; LFO: x depths',
            f'ld r4, [c0{A6_LPITCH:02x}]', 'mov r0, r0', 'mul r0, r7, r4', f'st r0, [{LFO_P:04x}]',
            f'ld r4, [c0{A6_LFILT:02x}]', 'mov r0, r0', 'mul r0, r7, r4', f'st r0, [{LFO_F:04x}]',
            f'ld r4, [c0{A6_LAMP:02x}]', 'add r7, #-1.0', 'mul r0, r7, r4', f'st r0, [{LFO_M:04x}]']


def step_noise():
    """step 11: the global noise source (as step 9) -> LFO_NZ; uses r0, r2"""
    return ['; noise: n = wrap(K n + w), w = Weyl counter',
            f'ld r2, [{LFO_NZ + 1:04x}]', f'ld r0, [{LFO_W + 1:04x}]', 'mov r1, r1', f'add r0, #{NOISE_W!r}:32',
            'ph_wrap r0, r3, r0', f'st r0, [{LFO_W:04x}]', f'mul r2, r2, #{NOISE_K!r}', 'addxy r2, r2, r0',
            'ph_wrap r2, r3, r2', f'st r2, [{LFO_NZ:04x}]']


def step_lfo_voice(v):
    """step 11: LFO of voice v (own phase restarted at the note start, own S&H), fade, cutoff -> A180+v, amp term ->
    block + 14, pitch -> r0 (and block + 6, A178+v unmodulated); also the gate / trigger copies for DSP B.
    Uses r0..r4, r7 (r5 / r6 kept); ends with r7 = 0."""
    P = V8_PHASE[v]
    w = lambda i: f'ld r4, [c0{A6_LWAVE + i:02x}]'  # noqa: E731
    return [f'; voice {v}: LFO, phase restarted when the trigger changes',
            f'ld r1, [{P + V11_LPH + 1:04x}]', f'ld r4, [c0{A6_LRATE:02x}]', f'ld r3, [{TRIG[v]:04x}]',
            f'ld r2, [{P + V11_TRIG + 1:04x}]', 'mov r0, r0', f'st r3, [{P + V11_TRIG:04x}]',
            f'st r3, [{STEP_TB[v]:04x}]',
            'subxy r2, r2, r3', 'mul r2, r2, r2', 'mad r1, r1, -r2',        # d = 1 at a new note: phase 0
            'addxy r0, r1, r4', 'ph_wrap r1, r3, r0', f'st r1, [{P + V11_LPH:04x}]', 'subxy r0, r1, r0',   # 2 at a wrap
            f'ld r3, [{P + V11_SH + 1:04x}]', f'ld r2, [{LFO_NZ:04x}]', 'mov r4, r4',
            'subxy r4, r3, r2', 'mul r4, r4, r0', 'mad r3, r4, #0.5', f'st r3, [{P + V11_SH:04x}]',
            f'; voice {v}: sample & glide g += k (h - g)', f'ld r4, [c0{A6_LRATE:02x}]', f'ld r2, [{V11_SG[v] + 1:04x}]',
            'mov r0, r0', f'mul r4, r4, #{SG_K!r}', 'subxy r0, r2, r3', 'min r4, r4, r6', 'mad r2, r0, r4',
            f'st r2, [{V11_SG[v]:04x}]',
            f'; voice {v}: shape = sine, tri, saw, square, S&G, S&H x C060..C065',
            w(5), 'mov r0, r0', 'mul r7, r3, r4', w(4), 'mov r0, r0', 'mad r7, r2, r4', w(2), 'mov r0, r0',
            'mad r7, r1, r4', w(3), 'mul r3, r1, #256.0', 'ph_clamp r3, r3, -r3', 'mad r7, r3, r4',
            w(1), 'ph_tri r3, r3, -r1', 'mad r7, r3, r4',
            'mul r0, r3, r3', w(0), 'mul r0, r0, #-0.5', 'add r0, #1.5', 'mul r0, r0, r3', 'mad r7, r0, r4',
            f'; voice {v}: fade (0 -> 1 from the gate), lf = lfo x fade',
            f'ld r2, [{V9_FADE[v] + 1:04x}]', f'ld r4, [c0{A6_LDELAY:02x}]', f'ld r3, [{GATE[v]:04x}]', 'mov r0, r0',
            f'st r3, [{STEP_GB[v]:04x}]', 'addxy r2, r2, r4', 'mul r2, r2, r3', 'min r2, r2, r6',
            f'st r2, [{V9_FADE[v]:04x}]', 'mul r1, r7, r2',
            f'; voice {v}: amp term f (M lfo - |M|), cutoff = C05D + F lf, pitch + P lf',
            f'ld r4, [c0{A6_LAMP:02x}]', f'ld r3, [c0{A6_LAMPABS:02x}]', 'mov r0, r0', 'mul r0, r1, r4',
            'mad r0, r2, -r3', f'st r0, [{P + V11_AMP:04x}]',
            f'ld r4, [c0{A6_LFILT:02x}]', f'ld r3, [c0{A6_CUT:02x}]', 'mov r0, r0', 'mad r3, r1, r4',
            f'st r3, [{STEP_CB[v]:04x}]',
            f'ld r0, [{PITCH[v]:04x}]', f'ld r4, [c0{A6_LPITCH:02x}]', 'mov r7, r7', f'st r0, [{STEP_PB[v]:04x}]',
            'mad r0, r1, r4', f'st r0, [{V9_PM[v]:04x}]', 'mov r7, #0.0']


def step_s1m0():
    per_voice = STEP >= 3
    L = [f'; s1m0 (STEP {STEP}): slave DSP A main @0x0100: {NV} voices x (DCO1 + DCO2 saws) '
         + ('-> A151+v' if per_voice else 'x gate -> A150'),
         'mov r5, #0.0', 'mov r6, #1.0', 'mov r7, #0.0']
    if STEP >= 11:
        L += step_noise()
    elif STEP >= 9:
        L += step_lfo() + ['mov r7, #0.0']
    gate = lambda v: [] if per_voice else [f'ld r4, [{GATE[v]:04x}]', 'mov r4, r4', 'mul r1, r1, r4']  # noqa: E731
    for v in range(NV):
        P = V8_PHASE[v]
        if STEP >= 11:
            L += step_lfo_voice(v) + [f'; voice {v}: DCO1']
        else:
            L += [f'; voice {v}: DCO1'] + (['mov r7, #0.0'] if per_voice and v else [])
        if STEP >= 11:
            pass
        elif STEP >= 9:                                    # fade -> r2, pitch + f P lfo -> r0 and V9_PM
            L += [f'ld r2, [{V9_FADE[v] + 1:04x}]', f'ld r4, [c0{A6_LDELAY:02x}]', f'ld r3, [{GATE[v]:04x}]',
                  'mov r5, r5', 'addxy r2, r2, r4', 'mul r2, r2, r3', 'min r2, r2, r6', f'st r2, [{V9_FADE[v]:04x}]',
                  f'ld r0, [{PITCH[v]:04x}]', f'ld r4, [{LFO_P:04x}]', 'mov r5, r5', f'st r0, [{STEP_PB[v]:04x}]',
                  'mad r0, r4, r2', f'st r0, [{V9_PM[v]:04x}]']
        else:
            L += [f'ld r0, [{PITCH[v]:04x}]', 'mov r4, r4']
            if STEP >= 6:
                L += [f'st r0, [{STEP_PB[v]:04x}]']
        L += ['add r0, #-4.75']
        if STEP >= 5:
            L += [f'ld r4, [c0{B6_RANGE1:02x}]', 'mov r4, r4', 'addxy r0, r0, r4']
        L += v8_saw(P)
        if STEP >= 7:
            L += step_wave(B6_WAVE1) + [f'ld r4, [{STEP_LEVEL1:04x}]', 'mov r4, r4', 'mul r1, r2, r4']
        elif STEP >= 2:
            L += [f'ld r4, [{STEP_LEVEL1:04x}]', 'mov r4, r4', 'mul r1, r1, r4']
        L += gate(v) + ['addxy r7, r7, r1']
        if STEP >= 5:
            L += [f'; voice {v}: DCO2', f'ld r0, [{(V9_PM if STEP >= 9 else PITCH)[v]:04x}]',
                  f'ld r4, [c0{B6_RANGE2:02x}]', 'mov r4, r4',
                  'addxy r0, r0, r4', f'ld r4, [c0{B6_FINE2:02x}]', 'mov r4, r4', 'addxy r0, r0, r4', 'add r0, #-4.75']
        else:
            L += [f'; voice {v}: DCO2', f'add r0, #{DCO2_DETUNE!r}']
        L += v8_saw(P + 2)
        if STEP >= 7:
            L += step_wave(B6_WAVE2) + [f'ld r4, [{STEP_LEVEL2:04x}]', 'mov r4, r4', 'mul r1, r2, r4']
        elif STEP >= 2:
            L += [f'ld r4, [{STEP_LEVEL2:04x}]', 'mov r4, r4', 'mul r1, r1, r4']
        else:
            L += ['mul r1, r1, #0.5']
        L += gate(v) + ['addxy r7, r7, r1']
        if STEP >= 11:                                   # tremolo (amp term of this sample)
            L += [f'ld r4, [{V8_PHASE[v] + V11_AMP:04x}]', 'mov r5, r5', 'mad r7, r7, r4']
        elif STEP >= 9:                                  # tremolo, cutoff -> A180+v
            L += [f'; voice {v}: LFO to amp and cutoff', f'ld r3, [{V9_FADE[v]:04x}]', f'ld r4, [{LFO_M:04x}]',
                  'mov r5, r5', 'mul r4, r3, r4', 'mad r7, r7, r4', f'ld r4, [{LFO_F:04x}]',
                  f'ld r0, [c0{A6_CUT:02x}]', 'mov r5, r5', 'mad r0, r3, r4', f'st r0, [{STEP_CB[v]:04x}]']
        if STEP >= 11:                                   # the gate / trigger copies are in step_lfo_voice
            L += [f'st r7, [{STEP_XV[v]:04x}]']
            continue
        if per_voice:
            L += [f'st r7, [{STEP_XV[v]:04x}]', f'ld r4, [{GATE[v]:04x}]', 'mov r5, r5', f'st r4, [{STEP_GB[v]:04x}]']
        if STEP >= 4:
            L += [f'ld r4, [{TRIG[v]:04x}]', 'mov r5, r5', f'st r4, [{STEP_TB[v]:04x}]']
    if not per_voice:
        L += [f'mul r7, r7, #{1.0 / (1.5 * NV)!r}', f'st r7, [{V8_MIX:04x}]']
    pad = int(os.environ.get('PAD', '0'))       # budget test: harmless fillers at the end of DSP A's program
    L += ['; PAD: budget test fillers'] * bool(pad) + ['mov r4, r4'] * pad
    return L


def step_adsr(e, latch, pt, gate, trig, prm):
    """ADSR -> r1 (stored at e) with instructions proven on the SYSTEM-1 (min / max with registers only);
    r5 = 0, r6 = 1, r7 kept; uses r0..r4. prm: sustain, attack, decay, release rate. latch = 1 once the attack
    reached 1; target 1.25 (attack) / S (decay) / 0 (release); a change of trig clears the latch (new note)."""
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


def step_filter_voice(v):
    """step 6: voice v on DSP B: amp ADSR, x = A151+v x env x gain, cutoff -> g, 4 one-pole stages with feedback"""
    b = STEP_B_BASE[v]
    y = [b + o for o in B_Y]
    L = [f'; voice {v}: amp ADSR'] + step_adsr(b, b + 2, b + 4, STEP_GB[v], STEP_TB[v], STEP_AENV_B)
    if STEP >= 8:                                        # x = A151+v x amp env, then the filter ADSR -> r1
        L += [f'ld r7, [{STEP_XV[v]:04x}]', 'mul r7, r7, r1']
        L += [f'; voice {v}: filter ADSR'] + step_adsr(b + B_FE, b + B_FE + 2, b + B_FE + 4, STEP_GB[v], STEP_TB[v],
                                                       0xC000 + B6_FENV)
        L += [f'; voice {v}: low-pass', f'ld r6, [c0{B6_GAIN:02x}]',
              f'ld r0, [{STEP_CB[v]:04x}]' if STEP >= 9 else f'ld r0, [c0{B6_CUT:02x}]',
              f'ld r2, [c0{B6_AMT:02x}]', f'ld r3, [{STEP_PB[v]:04x}]', f'ld r4, [c0{B6_KT:02x}]',
              'mul r7, r7, r6', 'mad r0, r2, r1']
    else:
        L += [f'; voice {v}: low-pass', f'ld r7, [{STEP_XV[v]:04x}]', f'ld r6, [c0{B6_GAIN:02x}]',
              f'ld r0, [c0{B6_CUT:02x}]', f'ld r3, [{STEP_PB[v]:04x}]', f'ld r4, [c0{B6_KT:02x}]',
              'mul r7, r7, r1', 'mul r7, r7, r6']
    L += ['add r3, #-4.75', 'mad r0, r4, r3', f'min r0, r0, #{CUT_HI}', f'max r0, r0, #{CUT_LO}']
    L += exp2_to('r0', 'r0') + [f'mul r0, r0, #{G_440!r}', f'min r0, r0, #{G_MAX}']
    if RESCOMP:                                          # r5 = c(g) = 1 + g (A + B g), g clamped to RC_GMAX
        L += [f'min r2, r0, #{RC_GMAX}', f'mov r5, #{RC_A}', f'mad r5, r2, #{RC_B}', 'mul r5, r5, r2',
              'add r5, #1.0']
    L += [f'ld r1, [{y[0] + 1:04x}]', f'ld r2, [{y[1] + 1:04x}]', f'ld r3, [{y[2] + 1:04x}]', f'ld r4, [{y[3] + 1:04x}]',
          f'ld r6, [c0{B6_RES:02x}]']
    if RESCOMP:
        L += ['mul r6, r6, r5']
    L += ['mad r7, r6, -r4',                             # u = x - k y4
          'ph_clamp r7, r3, -r7']                        # saturate to [-1, 1]
    for i, (prev, cur) in enumerate((('r7', 'r1'), ('r1', 'r2'), ('r2', 'r3'), ('r3', 'r4'))):
        L += [f'subxy r5, {cur}, {prev}', f'mad {cur}, r5, r0', f'st {cur}, [{y[i]:04x}]']
    return L


def step_s1m1():
    """C-S1V8's slave B program (steps 1, 2); from step 3 the per-voice mix"""
    if STEP >= 6:
        L = [f'; s1m1 (STEP {STEP}): slave DSP B main @0x0100: out = sum of the last sample\'s filter outputs x C000;'
             ' per voice amp ADSR + 4-pole low-pass', 'mov r7, #0.0']
        for v in range(NV):
            L += [f'ld r{v & 1}, [{STEP_B_BASE[v] + B_Y[3] + 1:04x}]', f'addxy r7, r7, r{v & 1}']
        L += ['ld r6, [c000]', f'mul r7, r7, #{2.0 / (1.5 * NV)!r}', 'mul r7, r7, r6',
              f'st r7, [{OUT_L:04x}]', f'st r7, [{OUT_R:04x}]', 'mov r0, #0.0']
        L += [f'st r0, [{x:04x}]' for x in OUT_ZERO]
        L += ['mov r5, #0.0', 'mov r6, #1.0']
        for v in range(NV):
            L += step_filter_voice(v)
            L += ['mov r5, #0.0', 'mov r6, #1.0']        # the ADSR of the next voice needs them again
        return schedule(L)
    if STEP >= 3:
        L = [f'; s1m1 (STEP {STEP}): slave DSP B main @0x0100: sum of A151+v x '
             + ('amp ADSR' if STEP >= 4 else 'gate') + ' -> x C000 -> 6006 / 6007',
             'mov r7, #0.0']
        if STEP >= 4:
            L += ['mov r5, #0.0', 'mov r6, #1.0']
        for v in range(NV):
            if STEP >= 4:
                b = STEP_B_BASE[v]
                L += [f'; voice {v}: amp ADSR'] + step_adsr(b, b + 2, b + 4, STEP_GB[v], STEP_TB[v], STEP_AENV_B)
                L += [f'ld r0, [{STEP_XV[v]:04x}]', 'mov r5, r5', 'mul r0, r0, r1', 'addxy r7, r7, r0']
                continue
            L += [f'ld r0, [{STEP_XV[v]:04x}]', f'ld r4, [{STEP_GB[v]:04x}]', 'mov r5, r5', 'mul r0, r0, r4',
                  'addxy r7, r7, r0']
        L += ['ld r6, [c000]', f'mul r7, r7, #{1.0 / (1.5 * NV)!r}', 'mul r7, r7, r6',
              f'st r7, [{OUT_L:04x}]', f'st r7, [{OUT_R:04x}]', 'mov r0, #0.0']
        L += [f'st r0, [{x:04x}]' for x in OUT_ZERO]
        return L
    L = ['; s1m1: slave DSP B main @0x0100: A150 x C000 -> 6006 / 6007, the other outputs 0',
         'ld r7, [c000]', f'ld r0, [{V8_MIX:04x}]', 'mov r4, r4', 'mul r1, r0, r7',
         f'st r1, [{OUT_L:04x}]', f'st r1, [{OUT_R:04x}]', 'mov r0, #0.0']
    L += [f'st r0, [{x:04x}]' for x in OUT_ZERO]
    return L


def step_m1():
    """C-S1V8's master B program, unchanged"""
    L = ['; s1mm1: master DSP B @0x1000: slave 6006 / 6007 x volume (C01C) -> A118 / A119 (the OS output program reads them)',
         f'ld r0, [{OUT_L:04x}]', f'ld r1, [{OUT_R:04x}]', f'ld r7, [c{M_VOLUME:03x}]', 'mov r4, r4',
         'mul r0, r0, r7', 'mul r1, r1, r7',
         f'st r0, [{M_OUT[0]:04x}]', f'st r1, [{M_OUT[1]:04x}]', 'mov r2, #0.0']
    L += [f'st r2, [{x:04x}]' for x in M_ZERO]
    return L


# ---- packing (step 8): transfers in the same instruction as an op, as Roland's programs do ----------------------
# Roland's SH-2 (system-1/s1_progs.py --listing) puts up to 3 loads / stores in one instruction with an op
# ("mul r1, r0, r7 | st r1, [6006] | ld r0, [a100]") and uses a loaded register in the very next instruction. All
# units of an instruction read before they write. pack() merges our one-unit lines that way and drops the
# "mov rN, rN" load fillers; make_synth.py --sim-check compares the packed programs with the unpacked ones sample by
# sample on the emulator's C core (sim_check).
import re as _re
_XFER = _re.compile(r'^(ld|st) r(\d), \[([0-9a-f]{4})\]$')


def _filler(x):
    m = _re.match(r'^mov r(\d), r(\d)$', x)
    return bool(m) and m.group(1) == m.group(2)


def _bank(addr):
    return addr >> 12


class _Ins:
    def __init__(self):
        self.op, self.xf = None, []           # op text; transfers (kind, reg, addr)
        self.more = []                        # further ops fused into this instruction (FUSE)

    def ops(self):
        return ([self.op] if self.op else []) + self.more

    def text(self):
        parts = self.ops() + [f'{k} r{r}, [{a:04x}]' for k, r, a in self.xf]
        return ' | '.join(parts)

    def loads(self):
        return {r for k, r, _ in self.xf if k == 'ld'}

    def stores(self):
        return {r for k, r, _ in self.xf if k == 'st'}

    def op_writes(self):
        return {_op_regs(o)[0] for o in self.ops()}

    def op_reads(self):
        return set().union(*[_op_regs(o)[1] for o in self.ops()]) if self.ops() else set()

    def ports_ok(self, kind, addr):
        if len(self.xf) >= 3 or any(a == addr for _, _, a in self.xf):
            return False
        same = sum(1 for k, _, a in self.xf if k == kind and _bank(a) == _bank(addr))
        return same < (2 if _bank(addr) == 0 else 1)


# op fusion (step 11, FUSE=1): independent ops share one instruction, as Roland ("addxy r2, r2, #0 | ph_wrap r0,
# r3, r0", SH-2 on the SYSTEM-1); all units read before they write. Only ops with one destination register.
FUSE = os.environ.get('FUSE', '1' if STEP >= 11 else '0') == '1'
_FUSABLE = {'mul', 'mad', 'addxy', 'subxy', 'add', 'min', 'max', 'mov', 'ph_wrap', 'ph_tri', 'ph_clamp', 'ymacd'}
_READS_DEST = {'mad', 'ymacd', 'add'}


def _op_regs(op):
    """(destination, registers read) of a one-destination op"""
    regs = [int(t) for t in _re.findall(r'\br(\d)\b', op)]
    if not regs:
        return None, set()
    reads = set(regs[1:]) | ({regs[0]} if op.split()[0] in _READS_DEST else set())
    return regs[0], reads


def _assembles(text):
    return asm.Assembler().assemble(text + '\n').ok


def pack(lines):
    out, cur = [], None
    for x in lines:
        if x.startswith(';') or not x.strip():
            continue
        if _filler(x):
            continue
        m = _XFER.match(x)
        if m:
            kind, r, addr = m.group(1), int(m.group(2)), int(m.group(3), 16)
            ok = cur is not None and cur.ports_ok(kind, addr) and r not in cur.op_writes() and r not in cur.loads()
            if ok and kind == 'ld':
                ok = r not in cur.stores()
            if ok:
                cur.xf.append((kind, r, addr))
                if _assembles(cur.text()):
                    continue
                cur.xf.pop()
            cur = _Ins()
            cur.xf.append((kind, r, addr))
            out.append(cur)
            continue
        # an op: joins an instruction that has no op yet if it does not touch what that instruction loads
        regs = set(int(t) for t in _re.findall(r'\br(\d)\b', x))
        if cur is not None and cur.op is None and not (regs & cur.loads()):
            cur.op = x
            if _assembles(cur.text()):
                continue
            cur.op = None
        # FUSE: or joins its ops if it neither reads nor rewrites what they write (and keeps off the loads); a store
        # already in the instruction reads before x writes, as in the unfused order
        if FUSE and cur is not None and cur.op is not None and x.split()[0] in _FUSABLE \
                and all(o.split()[0] in _FUSABLE for o in cur.ops()) and len(cur.ops()) < 3:
            d, rd = _op_regs(x)
            if d is not None and not (rd & cur.op_writes()) and d not in cur.op_writes() \
                    and not (regs & cur.loads()):
                cur.more.append(x)
                if _assembles(cur.text()):
                    continue
                cur.more.pop()
        cur = _Ins()
        cur.op = x
        out.append(cur)
    return [i.text() for i in out]


PACK = os.environ.get('PACK', '1') == '1'   # pack() the step programs (Roland's packing); PACK=0 = one unit per line


def step_program(lines, packed=None):
    """as program(), without the automatic scheduling (the step code places its own load delays as C-S1V8);
    PACK=1 packs transfers into the ops' instructions"""
    if packed if packed is not None else PACK:
        lines = pack(lines)
    r = assemble(lines)
    return [0x0A02, 0xFF71] + r.words + [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001], r, lines


def sources(packed=None):
    """packed: as step_program (None = PACK)"""
    if STEP == 0:
        return {
            's1m0': program(prog_s1m0()),
            's1m1': program(prog_s1m1()),
            's1mm1': program(prog_m1()),
            's1c0': close(0xA004),
            's1c1': close(0xA005),
        }
    return {
        's1m0': step_program(step_s1m0(), packed),
        's1m1': step_program(step_s1m1(), packed),
        's1mm1': step_program(step_m1(), packed),
        's1c0': close(0xA004),
        's1c1': close(0xA005),
    }


def instr_count(lines):
    return sum(1 for x in lines if x.strip() and not x.startswith(';'))


def c_array(name, w):
    p = list(w) + [0] * (len(w) & 1)
    words = [(p[i] << 16) | p[i + 1] for i in range(0, len(p), 2)]
    body = ',\n'.join('    ' + ', '.join(f'0x{x:08x}' for x in words[i:i + 6]) for i in range(0, len(words), 6))
    return (f'#define MC_{name.upper()}_BYTES 0x{2 * len(w):x}\n'
            f'static const uint32_t mc_{name.upper()}[{len(words)}] = {{\n{body}\n}};\n')


def float_bits(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


def table(name, vals, n=8):
    return (f'static const uint32_t {name}[{len(vals)}] = {{\n' + ',\n'.join(
        '    ' + ', '.join(f'0x{float_bits(x):08x}' for x in vals[i:i + n]) for i in range(0, len(vals), n)) + '\n};')


# ---- panel value (0..255) -> DSP word ----------------------------------------------------------------------------
def env_time(v):
    """fader -> seconds, 1 ms .. 10 s exponential"""
    return 0.001 * 10000 ** (v / 255.0)


def attack_rate(v):
    """rate toward 1.25 that reaches 1.0 in env_time(v): (1 - k)^n = 0.2"""
    return 1 - 0.2 ** (1 / (env_time(v) * FS))


def decay_rate(v):
    """rate that falls to 1/1000 of the step in env_time(v)"""
    return 1 - 0.001 ** (1 / (env_time(v) * FS))


def cutoff(v):
    """knob -> octaves from 440 Hz: -5 (13.75 Hz) .. +5.5 (19.9 kHz)"""
    return -5 + 10.5 * v / 255.0


def resonance(v):
    return RES_MAX * v / 255.0


def lfo_hz(v):
    """LFO rate knob -> Hz: 0.05 .. 40 Hz exponential"""
    return 0.05 * 800 ** (v / 255.0)


def on_grid(x):
    """x on the register grid (dspisa.quant: multiples of 2^-32 below 0.125), so a parameter used by the very next
    instruction after its load (packed) gives the same bits as after a filler (unpacked)"""
    return math.floor(x * 2 ** 32) / 2 ** 32 if abs(x) < 0.125 else x


def lfo_rate(v):
    return on_grid(2 * lfo_hz(v) / FS)          # phase -1..1


def lfo_delay(v):
    """delay / fade knob -> fade step per sample: 0 = at once, 255 = 5 s"""
    t = 5.0 * (v / 255.0) ** 2
    return 1.0 if t * FS < 1 else on_grid(1 / (t * FS))


def lfo_pitch(v):
    """-> pitch depth in octaves: up to 1 (squared: fine vibrato at the low end); step 11: bipolar, 128 = 0,
    -1 .. +1 octave as b |b| (b = bipolar(v))"""
    if STEP >= 11:
        b = bipolar(v)
        return b * abs(b)
    return (v / 255.0) ** 2


def bipolar(v):
    """knob 0..255 -> -1..+1, 128 = 0 (step 11)"""
    return max(-1.0, (v - 128) / 127.0)


def lfo_filt(v):
    """-> cutoff depth in octaves: up to 4 (step 11: bipolar, -4 .. +4, 128 = 0)"""
    return 4.0 * (bipolar(v) if STEP >= 11 else v / 255.0)


def lfo_amp(v):
    """-> M: voice x 1 + f M (lfo - 1) (1 - 2M .. 1, full knob 0 .. 1); step 11: bipolar -0.5 .. +0.5, voice x
    1 + f (M lfo - |M|)"""
    return 0.5 * (bipolar(v) if STEP >= 11 else v / 255.0)


def env_amount(v):
    """filter env amount knob: bipolar, 128 = 0, 0 = -8 octaves (inverted envelope), 255 = +8 octaves"""
    return 8.0 * max(-1.0, (v - 128) / 127.0)


# wave selector 0..5 -> (saw, pulse threshold, pulse, triangle), all at level 1
WAVES = [(1, 0, 0, 0), (0, 0, 1, 0), (0, 0.5, 1, 0), (0, 0, 0, 1), (0.5, 0, 0.5, 0), (0, 0.75, 1, 0)]
DCO2_DETUNE = 0.006                          # octaves (7 cents) until the OSC2 tune control is mapped


def header(progs):
    out = ['/* generated by make_synth.py (STEP=%d, STAGE=%d): our own DSP programs and tables (no Roland bytes) */' % (STEP, STAGE),
           '#include <stdint.h>', '']
    for name, (w, _, _) in progs.items():
        out.append(c_array(name, w))
    out.append('/* pitch word of a MIDI note: (note - 12) / 12 octaves (4.75 = A 440) */')
    out.append(table('pitch_bits', [(n - 12) / 12.0 for n in range(128)]))
    out.append('/* VOLUME knob -> gain (v / 255)^2 */')
    out.append(table('volume_bits', [(v / 255.0) ** 2 for v in range(256)]))
    out.append('/* envelope faders -> rates per sample at 96 kHz; sustain -> level */')
    out.append(table('attack_bits', [attack_rate(v) for v in range(256)]))
    out.append(table('decay_bits', [decay_rate(v) for v in range(256)]))
    out.append(table('level_bits', [v / 255.0 for v in range(256)]))
    out.append('/* filter knobs -> cutoff (octaves from 440 Hz), resonance k, compensation 1 + k/2, env amount */')
    out.append(table('cutoff_bits', [cutoff(v) for v in range(256)]))
    out.append(table('res_bits', [resonance(v) for v in range(256)]))
    out.append(table('comp_bits', [1 + resonance(v) / 2 for v in range(256)]))
    out.append(table('amount_bits', [env_amount(v) for v in range(256)]))
    out.append('/* waves 0..5: saw, square, pulse 25 %, triangle, saw + square, pulse 12.5 %:\n'
               '   coefficient (x 256) of saw, pulse, triangle; -threshold of the pulse as float bits */')
    out.append('static const uint16_t wave_coef[6][3] = {\n' + ',\n'.join(
        '    { %d, %d, %d }' % (round(w[0] * 256), round(w[2] * 256), round(w[3] * 256)) for w in WAVES) + '\n};')
    out.append(table('wave_thr_bits', [-w[1] for w in WAVES]))
    out.append(f'#define DCO2_OFF_BITS 0x{float_bits(DCO2_DETUNE):08x}u')
    out.append(f'#define M_VOLUME 0x{M_VOLUME:02x}')
    out.append(f'#define F_ONE 0x{float_bits(1.0):08x}u\n#define F_ZERO 0x00000000u')
    out.append(f'#define NV {NV}\n#define STAGE {STAGE}\n#define STEP {STEP}')
    idx = lambda x: f'0x{x & 0xFF:02x}'  # noqa: E731
    out.append(f'#define B5_PITCH {idx(PITCH[0])}\n#define B5_GATE {idx(GATE[0])}\n#define B5_TRIG {idx(TRIG[0])}')
    out.append(f'#define B5_OSC1 {idx(OSC1)}\n#define B5_OSC2 {idx(OSC2)}\n#define B5_OFF2 {idx(OFF2)}')
    out.append(f'#define B5_CUT {idx(CUT)}\n#define B5_RES {idx(RES)}\n#define B5_AMT {idx(AMT)}\n'
               f'#define B5_KT {idx(KT)}\n#define B5_COMP {idx(COMP)}')
    out.append(f'#define B5_FENV {idx(FENV)}\n#define B5_AENV {idx(AENV)}')
    out.append(f'#define B5_LEVEL1 {idx(STEP_LEVEL1)}\n#define B5_LEVEL2 {idx(STEP_LEVEL2)}')
    out.append(f'#define B6_AENV 0x{B6_AENV:02x}')
    out.append(f'#define B6_CUT 0x{B6_CUT:02x}\n#define B6_RES 0x{B6_RES:02x}\n#define B6_GAIN 0x{B6_GAIN:02x}\n#define B6_KT 0x{B6_KT:02x}')
    out.append(f'#define B6_FENV 0x{B6_FENV:02x}\n#define B6_AMT 0x{B6_AMT:02x}')
    out.append(f'#define B6_WAVE1 0x{B6_WAVE1:02x}\n#define B6_WAVE2 0x{B6_WAVE2:02x}')
    out.append('/* wave selector 0..5 -> saw, square, triangle coefficients */')
    out.append(table('step_wave_bits', [c for w in STEP_WAVES for c in w], 3))
    out.append(table('gain_bits', [(1 + resonance(v) / 2) / 2 for v in range(256)]))
    out.append(f'#define B6_RANGE1 0x{B6_RANGE1:02x}\n#define B6_RANGE2 0x{B6_RANGE2:02x}\n#define B6_FINE2 0x{B6_FINE2:02x}')
    out.append('/* OSC range 0..5 (64\' .. 2\') -> octaves; OSC2 fine tune 0..255 -> octaves (128 = 0) */')
    out.append(table('range_bits', [r - 3.0 for r in range(6)]))
    out.append(table('fine_bits', [(v - 128) / 128.0 * FINE_SPAN for v in range(256)]))
    out.append(f'#define A6_LRATE 0x{A6_LRATE:02x}\n#define A6_LDELAY 0x{A6_LDELAY:02x}\n#define A6_LPITCH 0x{A6_LPITCH:02x}\n'
               f'#define A6_LFILT 0x{A6_LFILT:02x}\n#define A6_LAMP 0x{A6_LAMP:02x}\n#define A6_CUT 0x{A6_CUT:02x}\n'
               f'#define A6_LWAVE 0x{A6_LWAVE:02x}')
    out.append('/* step 9, LFO knobs -> DSP A parameters: phase step, fade step, depths; shape -> 5 coefficients */')
    out.append(table('lfo_rate_bits', [lfo_rate(v) for v in range(256)]))
    out.append(table('lfo_delay_bits', [lfo_delay(v) for v in range(256)]))
    out.append(table('lfo_pitch_bits', [lfo_pitch(v) for v in range(256)]))
    out.append(table('lfo_filt_bits', [lfo_filt(v) for v in range(256)]))
    out.append(table('lfo_amp_bits', [lfo_amp(v) for v in range(256)]))
    out.append(f'#define A6_LAMPABS 0x{A6_LAMPABS:02x}')
    out.append(table('lfo_ampabs_bits', [abs(lfo_amp(v)) for v in range(256)]))
    lw = LFO_WAVES11 if STEP >= 11 else LFO_WAVES
    out.append(f'#define LFO_NW {len(lw[0])}')
    out.append(table('lfo_wave_bits', [c for w in lw for c in w], len(lw[0])))
    return '\n'.join(out) + '\n'


# ---- simulation ----------------------------------------------------------------------------------------------------
def panel_defaults():
    return dict(osc1_wave=0, osc1_level=255, osc2_wave=0, osc2_level=128, cutoff=255, res=0, amount=0, kt=0,
                fenv=(0, 128, 0, 60), aenv=(0, 128, 255, 60),
                lfo_rate=128, lfo_wave=0, lfo_delay=0, lfo_pitch=128 if STEP >= 11 else 0, lfo_filt=128 if STEP >= 11 else 0,
                lfo_amp=128 if STEP >= 11 else 0)


def f32(x):
    return float(struct.unpack('<f', struct.pack('<f', x))[0])


def b5_from_panel(p):
    """what engine.c writes, computed in Python"""
    b5 = {}
    for base, wave, lev in ((OSC1, p['osc1_wave'], p['osc1_level']), (OSC2, p['osc2_wave'], p['osc2_level'])):
        w = WAVES[wave]
        g = 0.5 * lev / 255.0
        b5[base], b5[base + 1], b5[base + 2], b5[base + 3] = w[0] * g, -w[1], w[2] * g, w[3] * g
    b5[OFF2] = DCO2_DETUNE
    b5[STEP_LEVEL1], b5[STEP_LEVEL2] = p['osc1_level'] / 255.0, p['osc2_level'] / 255.0
    b5[CUT], b5[RES], b5[COMP] = cutoff(p['cutoff']), resonance(p['res']), 1 + resonance(p['res']) / 2
    b5[AMT], b5[KT] = env_amount(p['amount']), p['kt'] / 255.0
    for base, (A, D, S, R) in ((FENV, p['fenv']), (AENV, p['aenv'])):
        b5[base], b5[base + 1], b5[base + 2], b5[base + 3] = S / 255.0, attack_rate(A), decay_rate(D), decay_rate(R)
    return {k: float(struct.unpack('<f', struct.pack('<f', x))[0]) for k, x in b5.items()}


def sim_params(b5, panel):
    """bank-6 parameters (FCEC from the ARM) of DSP A, DSP B and master B, as engine.c writes them"""
    pa = {0xC000: 1.0, 0xC000 + B6_RANGE1: 0.0, 0xC000 + B6_RANGE2: 0.0, 0xC000 + B6_FINE2: 0.0}   # 8', fine centre
    for base, wv in ((B6_WAVE1, panel['osc1_wave']), (B6_WAVE2, panel['osc2_wave'])):
        for i in range(3):
            pa[0xC000 + base + i] = float(STEP_WAVES[wv][i])
    if STEP >= 9:
        pa.update({0xC000 + A6_LRATE: f32(lfo_rate(panel['lfo_rate'])), 0xC000 + A6_LDELAY: f32(lfo_delay(panel['lfo_delay'])),
                   0xC000 + A6_LPITCH: f32(lfo_pitch(panel['lfo_pitch'])), 0xC000 + A6_LFILT: f32(lfo_filt(panel['lfo_filt'])),
                   0xC000 + A6_LAMP: f32(lfo_amp(panel['lfo_amp'])), 0xC000 + A6_CUT: b5[CUT],
                   0xC000 + A6_LAMPABS: f32(abs(lfo_amp(panel['lfo_amp'])))})
        for i, c in enumerate((LFO_WAVES11 if STEP >= 11 else LFO_WAVES)[panel['lfo_wave']]):
            pa[0xC000 + A6_LWAVE + i] = float(c)
    pb = {0xC000: 1.0, 0xC000 + B6_CUT: b5[CUT], 0xC000 + B6_RES: b5[RES], 0xC000 + B6_KT: b5[KT],
          0xC000 + B6_GAIN: b5[COMP] / 2}
    for i in range(4):
        pb[0xC000 + B6_AENV + i] = b5[AENV + i]
        if STEP >= 8:
            pb[0xC000 + B6_FENV + i] = b5[FENV + i]
    if STEP >= 8:
        pb[0xC000 + B6_AMT] = b5[AMT]
    return pa, pb, {0xC000 + M_VOLUME: 1.0}


def sim_b5(panel):
    b5 = b5_from_panel(panel)
    for v in range(NV):
        b5[PITCH[v]], b5[GATE[v]], b5[TRIG[v]] = (69 - 12) / 12.0, 0.0, 0.0
    return b5


def sim_event(b5, v, note):
    if note is None:
        b5[GATE[v]] = 0.0
    else:
        b5[PITCH[v]] = (note - 12) / 12.0
        b5[GATE[v]] = 1.0
        b5[TRIG[v]] = 1.0 - b5[TRIG[v]]


def simulate(progs, events, n, panel=None):
    """events: {sample: [(voice, note or None for off)]}; returns master A118 per sample. On the C core of the
    emulator (examples/from_scratch/emu/cchain.py, native code), one cchain.Slave: DSP 0 = slave A, DSP 1 = slave B,
    DSP 2 = master B. Model.shared5 = ((0, 1),): one bank 5 for slave A and B (as simulate_py); Model.own_params:
    each DSP its own bank 6 (FCEC(sel, ...)); B's 6006 / 6007 reach the master through the IOMD in the same sample.
    Banks 0 / 1 are whole rings (S+1 per sample). The samples between two events run in one C call. SIM=py:
    simulate_py (re/dspisa.py, slow); --sim-check compares the two."""
    if os.environ.get('SIM') == 'py':
        return simulate_py(progs, events, n, panel)
    sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 'emu'))
    import chain
    import cchain
    P = {k: chain.Program(progs[k][0], k) for k in ('s1m0', 's1m1', 's1mm1')}
    model = chain.Model(order=(0, 1, 2), shared5=((0, 1),), own_params=(0, 1, 2))
    sl = cchain.Slave({0: (P['s1m0'], []), 1: (P['s1m1'], []), 2: (P['s1mm1'], [])}, model)
    panel = panel or panel_defaults()
    b5 = sim_b5(panel)
    for d, prm in zip(sl.d, sim_params(b5, panel)):
        d.params.update(prm)
    a5 = sl.d[0].bank5
    for k, v in b5.items():
        a5[k & 0x3FF] = v
    out, t, word = [], 0, (2, M_OUT[0])
    for te in sorted(e for e in events if e < n) + [n]:
        if te > t:
            out += sl.run(te - t, (word,))[word]
            t = te
        for v, note in events.get(te, []) if te < n else []:
            sim_event(b5, v, note)
            for k in (PITCH[v], GATE[v], TRIG[v]):
                a5[k & 0x3FF] = b5[k]
    return out


def simulate_py(progs, events, n, panel=None):
    """the reference in Python (re/dspisa.py, minutes): dict memories, the ring words of the voice blocks moved by
    hand (S -> S+1), registers 0 at the start of each program"""
    import dspdis
    import dspisa
    code = {k: [i for _, i in dspdis.instructions(progs[k][0])] for k in ('s1m0', 's1m1', 's1mm1')}
    panel = panel or panel_defaults()
    b5 = sim_b5(panel)
    pa, pb, pm = sim_params(b5, panel)
    span = 0x20 if STEP >= 8 else 0x10
    rings = {'s1m0': [x + o for x in A_BASE for o in range(0, 16, 2)] + [LFO_PH, LFO_NZ, LFO_SH, LFO_W] + V11_SG,
             's1m1': [x + o for x in (STEP_B_BASE if STEP else B_BASE) for o in range(0, span, 2)]}
    mem = {'s1m0': {}, 's1m1': {}}
    mem_m = dict(pm)
    out = []
    for t in range(n):
        for v, note in events.get(t, []):
            sim_event(b5, v, note)
        for key in ('s1m0', 's1m1'):
            m = mem[key]
            for s in sorted(rings[key], reverse=True):
                m[s + 1] = m.get(s, 0.0)
            m.update(pb if key == 's1m1' else pa)
            m.update(b5)
            regs = [0.0] * 8
            for x in code[key]:
                dspisa.JUMP = None
                regs, m, _ = dspisa.run(list(x), regs, m)
            for k, val in m.items():
                if 0xA000 <= k < 0xC000:
                    b5[k] = val
            mem[key] = m
        m = dict(mem_m)
        m[OUT_L], m[OUT_R] = mem['s1m1'].get(OUT_L, 0.0), mem['s1m1'].get(OUT_R, 0.0)
        regs = [0.0] * 8
        for x in code['s1mm1']:
            dspisa.JUMP = None
            regs, m, _ = dspisa.run(list(x), regs, m)
        mem_m = m
        out.append(m.get(M_OUT[0], 0.0))
    return out


def freq(seq):
    cross = [t for t in range(1, len(seq)) if seq[t - 1] < 0 <= seq[t]]
    return (len(cross) - 1) * FS / (cross[-1] - cross[0]) if len(cross) > 2 else 0.0


def main():
    progs = sources()
    for name, (w, r, lines) in progs.items():
        open(os.path.join(HERE, name + '.s'), 'w').write('\n'.join(lines) + '\n')
        open(os.path.join(HERE, name + '.hex'), 'w').write(
            f'# {name}, own SYSTEM-1 program (make_synth.py STEP={STEP})\n' + '\n'.join('%04x' % x for x in w) + '\n')
        print(f'{name:6s} {instr_count(lines):4d} instructions, {len(w)} halfwords')
    os.makedirs(os.path.join(HERE, 'build'), exist_ok=True)
    open(os.path.join(HERE, 'build', 's1_programs.h'), 'w').write(header(progs))
    if '--sim' in sys.argv:
        sim_report(progs)
    if '--sim-check' in sys.argv:
        sim_check(progs)


def sim_report(progs):
    n = int(os.environ.get('SIM_N', '9600'))
    seq = simulate(progs, {0: [(0, 69)]}, n)
    print('A 440, DCO1 saw + DCO2 saw/2: peak %.3f, f %.2f Hz' % (max(abs(x) for x in seq[200:]), freq(seq[n // 2:])))
    seq = simulate(progs, {0: [(v, 60 + 2 * v) for v in range(NV)]}, n // 2)
    print('8 notes: peak %.3f' % max(abs(x) for x in seq[200:]))
    seq = simulate(progs, {}, 2000)
    print('no notes: peak %.6f' % max(abs(x) for x in seq))


def sim_check(progs):
    """the C core against the Python reference, sample by sample (SIM_CHECK_N samples, 2 notes, one released); then
    (steps) the packed programs against the unpacked ones on the C core (SIM_PACK_N samples, 8 notes, released)"""
    n = int(os.environ.get('SIM_CHECK_N', '600'))
    ev = {0: [(0, 69)], 50: [(1, 64)], n // 2: [(0, None)]}
    filt = dict(panel_defaults(), osc1_wave=1, osc2_wave=2, cutoff=120, res=200, amount=220, kt=128,
                fenv=(10, 90, 60, 40), aenv=(5, 100, 200, 30))
    panels = [('defaults', None), ('filter + envelopes', filt)]
    if STEP >= 9:
        lfo = dict(filt, lfo_rate=230, lfo_delay=40, lfo_pitch=160 if STEP < 11 else 60, lfo_filt=150 if STEP < 11 else 40,
                   lfo_amp=200 if STEP < 11 else 60)
        if STEP >= 11:
            panels += [('LFO sine', dict(lfo, lfo_wave=0)), ('LFO S&G', dict(lfo, lfo_wave=4)),
                       ('LFO S&H', dict(lfo, lfo_wave=5))]
        else:
            panels += [('LFO triangle', dict(lfo, lfo_wave=0)), ('LFO S&H', dict(lfo, lfo_wave=4))]
    os.environ['SIM'] = 'c'
    for name, panel in panels:
        c = simulate(progs, ev, n, panel)
        py = simulate_py(progs, ev, n, panel)
        bad = [t for t in range(n) if c[t] != py[t]]
        print('sim check, %s, %d samples: %s (peak %.4f)' % (name, n, 'identical' if not bad else
              '%d differ, first at %d: C %r, Python %r' % (len(bad), bad[0], c[bad[0]], py[bad[0]]), max(map(abs, c))))
    if STEP == 0:
        return
    n = int(os.environ.get('SIM_PACK_N', '19200'))
    ev = {0: [(v, 48 + 3 * v) for v in range(NV)], n // 2: [(v, None) for v in range(0, NV, 2)]}
    packed, unpacked = sources(True), sources(False)
    for name, panel in panels:
        a = simulate(packed, ev, n, panel)
        b = simulate(unpacked, ev, n, panel)
        bad = [t for t in range(n) if a[t] != b[t]]
        print('pack check, %s, %d samples: %s (peak %.4f)' % (name, n, 'identical' if not bad else
              '%d differ, first at %d: packed %r, unpacked %r' % (len(bad), bad[0], a[bad[0]], b[bad[0]]),
              max(map(abs, a))))


if __name__ == '__main__':
    main()
