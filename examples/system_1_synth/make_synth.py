"""C-S1SY: an 8-voice subtractive synth for the SYSTEM-1, all our own microcode (docs/examples_system_1_synth_README.md).

Extends C-S1V8 (examples/system_1): per voice two independent oscillators (wave + level each, DCO2 detuned), an
ADSR VCA, a 4-pole low-pass filter (cutoff, resonance, key track) and a second ADSR for the cutoff.

    slave A  s1m0 @0x0100  per voice: DCO1 + DCO2 (saw / pulse / triangle mix from the ARM) -> A068+v,
                           filter ADSR -> A070+v
    slave B  s1m1 @0x0100  first: sum of last sample's filter outputs x C000 -> 6006 / 6007 (6008..600D = 0);
                           per voice: amp ADSR, x = A068+v x amp env x comp, cutoff (base + amount x A070+v +
                           key track x pitch) -> g = 2 pi f / fs, ladder of 4 one-pole stages with feedback k
    master B s1mm1 @0x1000 6006 / 6007 x volume (C01C) -> A118 / A119 (unchanged from C-S1V4)

The VCA is applied before the filter (the filter is linear; the resonance is limited so that it rings out).
Stages (STAGE=1..4 build.sh), one hardware test each:
    1  DCO1 + DCO2 with wave / level from the panel, gate as VCA, no filter
    2  + amp ADSR
    3  + low-pass filter (cutoff, resonance, key track)
    4  + filter ADSR with amount

    python3 make_synth.py           -> s1*.s / s1*.hex here, build/s1_programs.h
    python3 make_synth.py --sim     -> also simulate notes through slave A, slave B and master B
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
STEP = int(os.environ.get('STEP', '1'))     # incremental steps on top of C-S1V8 (STEPS below)
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
G_MAX = 0.95
RES_MAX = 3.6


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
STEP_B_BASE = [0x0400 + 0x10 * v for v in range(NV)]
#   5  OSC1 range, OSC2 range (0x16A / 0x16F: 64'..2' = -3..+2 octaves) and OSC2 fine tune (0x170: +-1 semitone)
#      -> parameters C044 / C045 / C046 of DSP A (FCEC(0, id, bits), as Roland); the fixed +7 cents goes away
B6_RANGE1, B6_RANGE2, B6_FINE2 = 0x44, 0x45, 0x46
FINE_SPAN = 1.0 / 12                        # octaves at either end of the fine tune knob
V8_PHASE = [0x0300 + 0x10 * v for v in range(NV)]
V8_MIX = 0xA150


def v8_saw(P):
    """C-S1V8's saw (examples/system_1/make_s1.py saw_voice) on r0 = pitch - 4.75 (+ offset), phase ring P:
    the new phase is left in r1; uses r2, r3, r5 = 0, r6 = 1"""
    L = [f'ld r1, [{P + 1:04x}]', 'mov r4, r4',
         f'mul r2, r0, #{EXP_IN!r}', 'mov r3, #0.041666666666666664:32',
         'ymacd r3, r2, #0.16666666666666666', 'ymacd r3, r2, #0.5', 'ymacd r3, r2, #1.0', 'ymacd r3, r2, #1.0']
    L += ['mul r3, r3, r3'] * 4
    L += ['mul r3, r3, #0.009166666666666667', 'addxy r1, r3, r1', 'addxy r2, r1, #-1.0', 'max r2, r2, r5']
    L += ['mul r2, r2, #256.0', 'min r2, r2, r6'] * 4
    L += ['mul r2, r2, #2.0', 'subxy r1, r2, r1', f'st r1, [{P:04x}]']
    return L


def step_s1m0():
    per_voice = STEP >= 3
    L = [f'; s1m0 (STEP {STEP}): slave DSP A main @0x0100: {NV} voices x (DCO1 + DCO2 saws) '
         + ('-> A151+v' if per_voice else 'x gate -> A150'),
         'mov r5, #0.0', 'mov r6, #1.0', 'mov r7, #0.0']
    gate = lambda v: [] if per_voice else [f'ld r4, [{GATE[v]:04x}]', 'mov r4, r4', 'mul r1, r1, r4']  # noqa: E731
    for v in range(NV):
        P = V8_PHASE[v]
        L += [f'; voice {v}: DCO1'] + (['mov r7, #0.0'] if per_voice and v else [])
        L += [f'ld r0, [{PITCH[v]:04x}]', 'mov r4, r4', 'add r0, #-4.75']
        if STEP >= 5:
            L += [f'ld r4, [c0{B6_RANGE1:02x}]', 'mov r4, r4', 'addxy r0, r0, r4']
        L += v8_saw(P)
        if STEP >= 2:
            L += [f'ld r4, [{STEP_LEVEL1:04x}]', 'mov r4, r4', 'mul r1, r1, r4']
        L += gate(v) + ['addxy r7, r7, r1']
        if STEP >= 5:
            L += [f'; voice {v}: DCO2', f'ld r0, [{PITCH[v]:04x}]', f'ld r4, [c0{B6_RANGE2:02x}]', 'mov r4, r4',
                  'addxy r0, r0, r4', f'ld r4, [c0{B6_FINE2:02x}]', 'mov r4, r4', 'addxy r0, r0, r4', 'add r0, #-4.75']
        else:
            L += [f'; voice {v}: DCO2', f'add r0, #{DCO2_DETUNE!r}']
        L += v8_saw(P + 2)
        if STEP >= 2:
            L += [f'ld r4, [{STEP_LEVEL2:04x}]', 'mov r4, r4', 'mul r1, r1, r4']
        else:
            L += ['mul r1, r1, #0.5']
        L += gate(v) + ['addxy r7, r7, r1']
        if per_voice:
            L += [f'st r7, [{STEP_XV[v]:04x}]', f'ld r4, [{GATE[v]:04x}]', 'mov r5, r5', f'st r4, [{STEP_GB[v]:04x}]']
        if STEP >= 4:
            L += [f'ld r4, [{TRIG[v]:04x}]', 'mov r5, r5', f'st r4, [{STEP_TB[v]:04x}]']
    if not per_voice:
        L += [f'mul r7, r7, #{1.0 / (1.5 * NV)!r}', f'st r7, [{V8_MIX:04x}]']
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


def step_s1m1():
    """C-S1V8's slave B program (steps 1, 2); from step 3 the per-voice mix"""
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


def step_program(lines):
    """as program(), without the automatic scheduling (the step code places its own load delays as C-S1V8)"""
    r = assemble(lines)
    return [0x0A02, 0xFF71] + r.words + [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001], r, lines


def sources():
    if STEP == 0:
        return {
            's1m0': program(prog_s1m0()),
            's1m1': program(prog_s1m1()),
            's1mm1': program(prog_m1()),
            's1c0': close(0xA004),
            's1c1': close(0xA005),
        }
    return {
        's1m0': step_program(step_s1m0()),
        's1m1': step_program(step_s1m1()),
        's1mm1': step_program(step_m1()),
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


def env_amount(v):
    return 8.0 * v / 255.0


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
    out.append(f'#define B6_RANGE1 0x{B6_RANGE1:02x}\n#define B6_RANGE2 0x{B6_RANGE2:02x}\n#define B6_FINE2 0x{B6_FINE2:02x}')
    out.append('/* OSC range 0..5 (64\' .. 2\') -> octaves; OSC2 fine tune 0..255 -> octaves (128 = 0) */')
    out.append(table('range_bits', [r - 3.0 for r in range(6)]))
    out.append(table('fine_bits', [(v - 128) / 128.0 * FINE_SPAN for v in range(256)]))
    return '\n'.join(out) + '\n'


# ---- simulation ----------------------------------------------------------------------------------------------------
def panel_defaults():
    return dict(osc1_wave=0, osc1_level=255, osc2_wave=0, osc2_level=128, cutoff=255, res=0, amount=0, kt=0,
                fenv=(0, 128, 0, 60), aenv=(0, 128, 255, 60))


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


def simulate(progs, events, n, panel=None):
    """events: {sample: [(voice, note or None for off)]}; returns (master A118 per sample, slave B count)"""
    import dspdis
    import dspisa
    code = {k: [i for _, i in dspdis.instructions(progs[k][0])] for k in ('s1m0', 's1m1', 's1mm1')}
    b5 = b5_from_panel(panel or panel_defaults())
    for v in range(NV):
        b5[PITCH[v]], b5[GATE[v]], b5[TRIG[v]] = (69 - 12) / 12.0, 0.0, 0.0
    rings = {'s1m0': [x + o for x in A_BASE for o in range(0, 16, 2)],
             's1m1': [x + o for x in B_BASE for o in range(0, 16, 2)]}
    mem = {'s1m0': {}, 's1m1': {}}
    mem_m = {0xC000 + M_VOLUME: 1.0}
    out = []
    for t in range(n):
        for v, note in events.get(t, []):
            if note is None:
                b5[GATE[v]] = 0.0
            else:
                b5[PITCH[v]] = (note - 12) / 12.0
                b5[GATE[v]] = 1.0
                b5[TRIG[v]] = 1.0 - b5[TRIG[v]]
        for key in ('s1m0', 's1m1'):
            m = mem[key]
            for s in sorted(rings[key], reverse=True):
                m[s + 1] = m.get(s, 0.0)
            m[0xC000] = 1.0
            if key == 's1m1':                            # bank-6 parameters of DSP B (FCEC from the ARM)
                for i in range(4):
                    m[0xC000 + B6_AENV + i] = b5[AENV + i]
            else:                                        # DSP A: ranges 8', fine tune centre
                m[0xC000 + B6_RANGE1] = m[0xC000 + B6_RANGE2] = m[0xC000 + B6_FINE2] = 0.0
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


def sim_report(progs):
    n = int(os.environ.get('SIM_N', '9600'))
    seq = simulate(progs, {0: [(0, 69)]}, n)
    print('A 440, DCO1 saw + DCO2 saw/2: peak %.3f, f %.2f Hz' % (max(abs(x) for x in seq[200:]), freq(seq[n // 2:])))
    seq = simulate(progs, {0: [(v, 60 + 2 * v) for v in range(NV)]}, n // 2)
    print('8 notes: peak %.3f' % max(abs(x) for x in seq[200:]))
    seq = simulate(progs, {}, 2000)
    print('no notes: peak %.6f' % max(abs(x) for x in seq))


if __name__ == '__main__':
    main()
