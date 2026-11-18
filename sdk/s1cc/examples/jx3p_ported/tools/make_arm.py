"""make_arm.py: the JX-3P port's tables: jx3p_tables.h (the kernel's knob tables and Roland's constant parameter
words) and jx3p_arm.h (SYSTEM-S1CC-BOUTIQUE ARM_CUSTOM: the filter's g, the VCA's envelope, both DCOs' steps per
voice every 1 ms). README.md §3.

    python3 sdk/s1cc/examples/jx3p_ported/tools/make_arm.py

Sources (no hand-picked value):
  work/jx3p/jx_arm.json   what the S8 JX-3P PLUG-OUT 1.01 writes to the DSPs for every value of every parameter
                          (tools/jx_arm.py -> tools/plugout_dsp_tables.py)
  examples/system_1_jx    s1jx.model_writes(default_vals()): every DSP2 / DSP4 word of the default patch (the
                          constants of the words no panel parameter moves: C130 = 0.93, A025 = 8.3e-5, ...)
  jx3p_voice.h            the formulas (lifted MAIN2 instructions 38..74, MAIN4 881..969), cited where used

The patch words (README.md §3.2): Roland's one patch layout of every PLUG-OUT on both synths (word 0x160 + k is the
same panel control on the SYSTEM-1 and the SYSTEM-8), read with the JX-3P's values as Roland's engine reads them,
so Roland's banks need no conversion (2026-11-17). Rows of jx_arm.json:
  0x17D VCF CUTOFF FREQ (C120), 0x17F VCF RESONANCE (C121), 0x181 VCF ENV1 MOD (C127, A066), 0x186 VCF PITCH FOLLOW
  (C128), 0x164 VCF LFO MOD (C126), 0x18B BRILLIANCE (C12D), 0x163 DCO LFO MOD (C026), 0x17A DCO ENV1 MOD (C027,
  A02D), 0x16A / 0x16F DCO1 / DCO2 RANGE (A026 / A027), 0x170 DCO2 FINE TUNE (C021), 0x173 DCO2 TUNE (C020),
  0x169 / 0x16E DCO1 / DCO2 FREQ MOD (C028 / C02A: 0 off, 1 LFO, 2 ENV, >= 3 both), 0x174 / 0x175 DCO1 / DCO2 LEVEL
  (kernel tables), 0x19C VCA LEVEL, 0x193 BEND RANGE; the extended block: 0x1C9 VCA MODE, 0x1CB BEND GAIN,
  0x1CC / 0x1CD BEND SENS DCO / VCF, 0x1CE / 0x1CF MOD SENS DCO / VCF
"""
import json
import math
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EX = os.path.abspath(os.path.join(HERE, '..'))
ROOT = os.path.abspath(os.path.join(EX, '..', '..', '..', '..'))
SRC = os.path.join(ROOT, 'work', 'jx3p', 'jx_arm.json')

Q = 1 << 24
NU, USH = 1024, 18                 # the g table: 1024 steps of 1/64 octave (Q24 >> 18) from U0
U0, ULO, UHI = -3.0, -3.0, 11.5    # u limited to -3 .. 11.5 (instructions 935..941)
LOG2_STEP = math.log2(440.0 / 96000.0) - 4.75      # dt = 2^(pitch + LOG2_STEP): 4.75 = A 440, phase 0..1
# OSC1 / OSC2 COLOR MOD position -> FREQ MOD (bit 0 LFO, bit 1 ENV): MANUAL both (the knobs LFO TO PITCH and PITCH ENV
# AMOUNT work at once), LFO, PITCH ENV / FILTER ENV / AMP ENV = ENV, SUB OSC = off (README.md §3.2)
FMOD = [0, 1, 2, 3, 3, 3]   # DCO1 / DCO2 FREQ MOD 0 off, 1 LFO, 2 ENV, >= 3 both (Roland's rows 0x6d6: C028, C02A)


def q(x):
    return int(round(x * Q))


def rows(J, pid, key):
    out, last = [], None
    for r in J['params'][pid]['rows']:
        v = r.get(key)
        last = last if v is None else v
        out.append(last)
    return out


def arr(name, vals, per=8, ty='int32_t'):
    out = ['static const %s %s[%d] = {' % (ty, name, len(vals))]
    for i in range(0, len(vals), per):
        out.append('    ' + ', '.join('%d' % v for v in vals[i:i + per]) + ',')
    return '\n'.join(out) + '\n};\n'


def fl(v):
    t = '%.9g' % v
    return t if any(c in t for c in '.e') else t + '.0'


def farr(name, vals, per=6):
    out = ['static const float %s[%d] = {' % (name, len(vals))]
    for i in range(0, len(vals), per):
        out.append('    ' + ', '.join(fl(v) + 'f' for v in vals[i:i + per]) + ',')
    return '\n'.join(out) + '\n};\n'


def model_words():
    sys.path.insert(0, os.path.join(ROOT, 'examples', 'system_1_jx'))
    import s1jx
    return s1jx.model_writes(s1jx.default_vals())


def vcf_curve(c):
    """instructions 881..887: the cutoff knob's cubic, limited to 0..1"""
    return min(max(0.65628016 * c + c * c * 0.6643478 + c * c * c * -0.32077295, 0.0), 1.0)


def g_of(u):
    """instructions 935..969: g = tan(w) / (1 + tan(w)), w = 2^u e^-3.292449 x 0.014398932 (Roland's sin and cos
    polynomial: the same within float precision), limited to 1"""
    u = min(max(u, ULO), UHI)
    w = math.exp(0.6931472 * u - 3.292449) * 0.014398932
    t = math.tan(w)
    return min(t / (1.0 + t), 1.0)


def main():
    if not os.path.exists(SRC):
        subprocess.run([sys.executable, os.path.join(HERE, 'jx_arm.py')], check=True)
    J = json.load(open(SRC))
    M = model_words()
    P = lambda d, i: M.get(('P', d, i), 0.0)
    B = lambda d, i: M.get(('B', d, i), 0.0)

    # ---- the kernel: DCO levels and resonance as knob tables, the other parameter words as constants ----
    lvl1, lvl2 = rows(J, '0x6e1', '2:C:034'), rows(J, '0x6e2', '2:C:035')
    reso = rows(J, '0x6ec', '4:C:021')
    kp = {}
    for i in range(0x20, 0x50):
        kp[0xC000 + i] = P(2, i)
        kp[0xC100 + i] = P(4, i)
    k = ['/* jx3p_tables.h: the JX-3P kernel\'s knob tables and Roland\'s constant parameter words. Written by',
         '   sdk/s1cc/examples/jx3p_ported/tools/make_arm.py from work/jx3p/jx_arm.json and s1jx.model_writes; do not',
         '   edit. */', '#ifndef JX3P_TABLES_H', '#define JX3P_TABLES_H', '']
    k.append(farr('jx_lvl1', lvl1) + farr('jx_lvl2', lvl2) + farr('jx_reso', reso)
             + farr('jx_hpf', rows(J, '0x6ed', '4:C:048')))                       # HPF CUTOFF (C148)
    k.append('S1_PARAM(lvl1, S1_OSC1_MIX, 1) { out[0] = jx_lvl1[knob]; }          /* DCO1 LEVEL (C034) */')
    k.append('S1_PARAM(lvl2, S1_OSC2_MIX, 1) { out[0] = jx_lvl2[knob]; }          /* DCO2 LEVEL (C035) */')
    k.append('S1_PARAM(reso, S1_FILTER_RESONANCE, 1) { out[0] = jx_reso[knob]; }  /* VCF RESONANCE (C121) */')
    k.append('/* KP(a) of the lifted voice: the knob tables, else the default patch\'s word (DSP2 C0xx, DSP4 C1xx) */')
    k.append('static inline __attribute__((always_inline)) float jx_kp(int a)\n{\n    switch (a) {')
    k.append('    case 0xc034: return s1_param(lvl1, 0);\n    case 0xc035: return s1_param(lvl2, 0);')
    k.append('    case 0xc121: return s1_param(reso, 0);')
    for a, v in sorted(kp.items()):
        if v != 0.0 and a not in (0xC034, 0xC035, 0xC121):
            k.append('    case 0x%04x: return %sf;' % (a, fl(v)))
    k.append('    default: return 0.0f;\n    }\n}\n#endif\n')
    open(os.path.join(EX, 'jx3p_tables.h'), 'w').write('\n'.join(k))

    # ---- the ARM ----
    cut = rows(J, '0x6ea', '4:C:020')
    base = [q(11.6 * vcf_curve(c) - 0.7 + P(4, 0x2B)) for c in cut]               # 934: 11.6 vcf - 0.7 + C12B
    rterm = [q(min((1.0 - r) * 0.5, 0.075)) for r in reso]                         # 934
    bril = [q(x) for x in rows(J, '0x6f8', '4:C:02d')]                              # 934: + BRILLIANCE (C12D)
    envm = [q(x) for x in rows(J, '0x6ee', '4:C:027')]                              # 916
    envp = [int(x) for x in rows(J, '0x6ee', '4:A:026')]
    kf = [q(x) for x in rows(J, '0x6f3', '4:C:028')]                                # 918 (bipolar, Roland's word)
    lfof = [q(x) for x in rows(J, '0x6d1', '4:C:026')]                              # 918 (bipolar, as the S1 knob)
    lfop = [q(x) for x in rows(J, '0x6d0', '2:C:026')]                              # 50 (bipolar, as the S1 knob)
    denv = [q(x) for x in rows(J, '0x6e7', '2:C:027')]                              # 50
    dpol = [int(x) for x in rows(J, '0x6e7', '2:A:02d')]
    fine = [q(x) for x in rows(J, '0x6dd', '2:C:021')]                              # 65
    rng1 = [q(x) for x in rows(J, '0x6d7', '2:A:026')[:6]]                          # 64
    rng2 = [q(x) for x in rows(J, '0x6dc', '2:A:027')[:6]]                          # 65
    tune = rows(J, '0x6e0', '2:C:020')                                              # 0..1 octave: k / 127
    assert abs(tune[127] - 1.0) < 1e-6 and abs(tune[64] - 64 / 127) < 1e-5
    tuneq = [q(x) for x in tune]                                                    # DCO2 TUNE (0x173) row, octaves
    g = [q(g_of(U0 + i / 64.0)) for i in range(NU + 1)]
    mant = [int(round((2.0 ** (i / 1024.0) - 1.0) * (1 << 23))) for i in range(1025)]
    d = J['params']
    dflt = lambda pid: d[pid]['default']
    init = [dflt('0x6ea'), dflt('0x6ec'), dflt('0x6ee'), dflt('0x6f3'), dflt('0x6d1'), dflt('0x6f8'), dflt('0x6d0'), dflt('0x6e7'), dflt('0x6d7'),
            dflt('0x6dc'), dflt('0x6dd'), (dflt('0x6e0') - 11) & 255, 0, 0, 1, dflt('0x709'), dflt('0x700'), 0, 255, 255, 128, 128]
    # 11 DCO2 TUNE: the patch holds word 0x173 (the SYSTEM-1's OSC2 COARSE) as the bank's value - 11 (Roland's SH-101
    # image, the .PRM; PARAMLOG 2026-11-18: 117 for String 1's 128), the ARM adds 11 back; the OS's range of that word is
    # widened to -11..244 while the PLUG-OUT is selected (model.c / engine.c coarse_wide); the engine reads it from
    # EXTEND30 0x1D5, where model.c coarse_copy puts it (the slave still cut 0x173), 12 / 13 FREQ MOD, 14 VCA MODE (Script.xml default 1), 15 VCA LEVEL, 16 BEND RANGE, 17 BEND GAIN,
    # 18 / 19 BEND SENS DCO / VCF, 20 / 21 MOD SENS DCO / VCF (Script.xml defaults)

    a = ['/* jx3p_arm.h: the JX-3P voice\'s per-voice values on the ARM (SYSTEM-S1CC-BOUTIQUE ARM_CUSTOM, README.md §3).',
         '   Written by sdk/s1cc/examples/jx3p_ported/tools/make_arm.py; do not edit. */', '',
         '#ifdef ARM_CUSTOM_CC_TABLE',
         '#define ARM_CUSTOM_XMOD 1     /* model.c: OSC2 SYNC / RING buttons = DCO2 CROSS MOD off / SYNC / METAL (§4.3) */',
         '#define ARM_CUSTOM_XMOD_WORD 1 /* ... in one word, 0x172 = 0 / 1 / 2, as Roland\'s patch (§4.3) */',
         '/* model.c (region 0): MIDI CC -> the patch word of a JX-3P parameter with no SYSTEM-1 control, Roland\'s value',
         '   (def 0: model.c stores (value - def) & 0xFF). These CCs take precedence over the test map (model.c cc_panel) */',
         'static const struct { uint8_t cc; uint16_t id; uint8_t steps, max, def; } arm_custom_cc[] = {',
         '    { 109, 0x1C9, 3, 2, 0 },     /* EXTEND18: VCA MODE 0 ENV-1, 1 ENV-2, 2 GATE */',
         '    { 110, 0x19C, 0, 255, 0 },   /* VCA LEVEL (the patch word of PAT_LEV) */',
         '};',
         '#else',
         '#define ARM_CUSTOM_NK 22',
         '/* the patch words of Roland\'s JX-3P patch (the one layout of every PLUG-OUT on both synths, README.md §3.2),',
         '   Roland\'s values: 0 VCF CUTOFF FREQ, 1 RESONANCE, 2 VCF ENV1 MOD, 3 VCF PITCH FOLLOW, 4 VCF LFO MOD, 5 BRILLIANCE,',
         '   6 DCO LFO MOD, 7 DCO ENV1 MOD, 8 DCO1 RANGE, 9 DCO2 RANGE, 10 DCO2 FINE TUNE, 11 DCO2 TUNE, 12 DCO1 FREQ MOD,',
         '   13 DCO2 FREQ MOD, 14 VCA MODE (EXTEND18), 15 VCA LEVEL, 16 BEND RANGE, 17 BEND GAIN (EXTEND20), 18 / 19 BEND',
         '   SENS DCO / VCF (EXTEND21 / 22), 20 / 21 MOD SENS DCO / VCF (EXTEND23 / 24) (README.md §3.7) */',
         'static const uint16_t arm_custom_id[ARM_CUSTOM_NK] = { 0x17D, 0x17F, 0x181, 0x186, 0x164, 0x18B, 0x163,',
         '                                                        0x17A, 0x16A, 0x16F, 0x170, 0x1D5, 0x169, 0x16E,',
         '                                                        0x1C9, 0x19C, 0x193, 0x1CB, 0x1CC, 0x1CD, 0x1CE,',
         '                                                        0x1CF };',
         '/* the JX-3P defaults (jx_arm.json), as SYSTEM-1 knob values */',
         '#define ARM_CUSTOM_INIT { %s }' % ', '.join(str(x) for x in init),
         '#define ARM_U0 %d                 /* the g table: u from U0 (Q24), steps of 2^-%d (1/64 octave) */' % (q(U0), 24 - USH),
         '#define ARM_USH %d' % USH, '#define ARM_NU %d' % NU,
         '#define ARM_LOG2_STEP %d          /* log2(440 / 96000) - 4.75: dt = 2^(pitch + this) */' % q(LOG2_STEP),
         '/* the SYSTEM-1 plays every RANGE position one octave above the S8 JX-3P\'s scale (measured 2026-11-17, note 60, the',
         '   same panel: stock SYSTEM-1 524.0 Hz, JX-3P-S1 without this 261.5 Hz; work/jx3p/ref_capture/system1_saw_*.raw,',
         '   jx3p_s1_saw_*.raw). tools/arm_check.c sets it 0: Roland\'s S8 scale */',
         '#ifndef ARM_S1_OCT',
         '#define ARM_S1_OCT %d             /* 1 octave (Q24) */' % Q,
         '#endif',
         '#define ARM_A024 %d               /* DCO1 / DCO2 detune words (A024 / A025 of the default patch) */' % q(B(2, 0x24)),
         '#define ARM_A025 %d' % q(B(2, 0x25)),
         '#define ARM_VCA 218175            /* 0.013004484 (instruction 1050): the VCA one-pole\'s input factor */',
         ]
    vl = rows(J, '0x709', '4:C:04b')                                                # VCA LEVEL (C14B), 128 = 1
    a.append(arr('arm_vlev', [q(x / vl[128]) for x in vl]))
    a.append(arr('arm_base', base) + arr('arm_rterm', rterm) + arr('arm_bril', bril) + arr('arm_envm', envm)
             + arr('arm_envp', envp, 32, 'uint8_t') + arr('arm_kf', kf) + arr('arm_lfof', lfof) + arr('arm_lfop', lfop)
             + arr('arm_denv', denv) + arr('arm_dpol', dpol, 32, 'uint8_t') + arr('arm_fine', fine) + arr('arm_tune', tuneq)
             + arr('arm_rng1', rng1) + arr('arm_rng2', rng2)
             + arr('arm_fmod', FMOD, 6, 'uint8_t') + arr('arm_g', g) + arr('arm_mant', mant))
    # the envelopes (MAIN0 instructions 155..253, README.md §3.4): ENV1 and ENV2 have the same rows (checked)
    for k1, k2 in (('0:C:02f', '0:C:033'), ('0:C:030', '0:C:034'), ('0:C:031', '0:C:035'), ('0:C:032', '0:C:036')):
        pa = {'0:C:02f': '0x6ef', '0:C:030': '0x6f0', '0:C:031': '0x6f1', '0:C:032': '0x6f2'}[k1]
        assert rows(J, pa, k1) == rows(J, hex(int(pa, 16) + 5), k2), k1
    ea = [q(x) for x in rows(J, '0x6ef', '0:C:02f')]                                # attack step per tick
    ed = [q(1.0 - x) for x in rows(J, '0x6f0', '0:C:030')]                          # decay factor 1 - D
    es = [q(x) for x in rows(J, '0x6f1', '0:C:031')]                                # sustain level
    er = [q(1.0 - x) for x in rows(J, '0x6f2', '0:C:032')]                          # release factor 1 - R
    a.append(arr('arm_env_a', ea) + arr('arm_env_d1', ed) + arr('arm_env_s', es) + arr('arm_env_r1', er))
    a.append(ARM_ENV_C)
    # the LFO (MAIN0 instructions 66..147, README.md §3.5): rate, steps, delay from Roland's rows
    lr = rows(J, '0x6cf', '0:C:026')
    finc = [min(max(0.46623794 * math.exp(5.28342146 * min(max(k, 0.0), 1.0)) - 0.413383555, 0.0) * 2 ** -16,
                0.0155343) for k in lr]                                            # 92, 100..101: phase step (span 2)
    a02c = [rows(J, '0x6cd', '0:A:02c')[w] for w in range(3)]                     # x 1, 1, 2 (RANDOM: twice)
    assert a02c == [1.0, 1.0, 2.0], a02c
    jinc = [int(round(x * 2 ** 31)) for x in finc]                                 # uint32 phase: 2^32 = span 2
    jstep = [q(max(4 * x, 0.0010212418)) for x in finc]                           # 129: the steps' counter
    dl = rows(J, '0x6ce', '0:A:02d')
    jfade = [int(round(min(x, 1.0) * 2 ** 31)) for x in dl]                        # 103: delay ramp per sample, Q31
    jc027 = [int(x) for x in rows(J, '0x6ce', '0:C:027')]                          # 137: 0 = no delay (x 2)
    a.append(arr('arm_jl_inc', jinc, 8, 'uint32_t') + arr('arm_jl_step', jstep) + arr('arm_jl_fade', jfade, 8, 'uint32_t')
             + arr('arm_jl_c027', jc027, 32, 'uint8_t') + arr('arm_jl_wave', JL_WAVE, 6, 'uint8_t'))
    a.append(ARM_LFO_C)
    a.append(ARM_C)
    a.append('#endif\n')
    open(os.path.join(EX, 'jx3p_arm.h'), 'w').write('\n'.join(a))
    print('jx3p_tables.h, jx3p_arm.h written')
    if '--vectors' in sys.argv:
        vectors(J, P, B)


def vectors(J, P, B):
    """work/jx3p/arm_vectors.txt for tools/arm_check.c: knob settings as the ARM reads them (k[0..13], note, ENV1,
    LFO) and the S8 PLUG-OUT's words for the same settings (the same rows, the default patch elsewhere)"""
    import random
    rnd = random.Random(3)
    R = lambda pid, key: rows(J, pid, key)
    cut, reso, bril = R('0x6ea', '4:C:020'), R('0x6ec', '4:C:021'), R('0x6f8', '4:C:02d')
    envm, envp = R('0x6ee', '4:C:027'), R('0x6ee', '4:A:026')
    kf, lfof, lfop = R('0x6f3', '4:C:028'), R('0x6d1', '4:C:026'), R('0x6d0', '2:C:026')
    denv, dpol, fine = R('0x6e7', '2:C:027'), R('0x6e7', '2:A:02d'), R('0x6dd', '2:C:021')
    rng1, rng2 = R('0x6d7', '2:A:026'), R('0x6dc', '2:A:027')
    out = []
    # bend / mod (README.md §3.7): bend -8192..8192, mod 0..127, BEND RANGE, BEND GAIN, SENS DCO / VCF, MOD SENS
    # DCO / VCF as the JX-3P values (the EXTEND words carry value - default)
    cases = [[255, 0, 128, 0, 0, 128, 0, 128, 3, 3, 128, 0, 0, 0, 60, 0.0, 0.0, 0, 0, 2, 0, 255, 255, 128, 128]]
    for i in range(60):
        k = [rnd.randrange(256) for _ in range(8)] + [rnd.randrange(6), rnd.randrange(6), rnd.randrange(256),
                                                      rnd.randrange(256), rnd.randrange(6), rnd.randrange(6)]
        bm = ([0, 0, 2, 0, 255, 255, 128, 128] if i < 20 else
              [rnd.randrange(-8192, 8193), rnd.randrange(128), rnd.randrange(24), rnd.randrange(4)]
              + [rnd.randrange(256) for _ in range(4)])
        cases.append(k + [rnd.randrange(24, 100), rnd.random(), rnd.uniform(-1, 1)] + bm)
    for c in cases:
        k, note, e1, lf = c[:14], c[14], c[15], c[16]
        bend, mod, brange, bgain, sd, sv, md, mv = c[17:25]
        w = {}
        for i in range(0x20, 0x50):
            w[0xC000 + i], w[0xC100 + i] = P(2, i), P(4, i)
            w[0xA000 + i], w[0xA040 + i] = B(2, i), B(4, i)
        f1, f2 = FMOD[k[12]], FMOD[k[13]]
        w.update({0xC120: cut[k[0]], 0xC121: reso[k[1]], 0xC127: envm[k[2]], 0xA066: envp[k[2]], 0xC128: kf[k[3]],
                  0xC126: lfof[k[4]], 0xC12D: bril[k[5]], 0xC026: lfop[k[6]], 0xC027: denv[k[7]], 0xA02D: dpol[k[7]],
                  0xA026: rng1[k[8]], 0xA027: rng2[k[9]], 0xC021: fine[k[10]], 0xC020: R('0x6e0', '2:C:020')[k[11]],
                  0xC028: f1 & 1, 0xC02A: f1 >> 1, 0xC029: f2 & 1, 0xC02B: f2 >> 1,
                  0xA0A5: lf, 0xA0A7: (note - 12) / 12.0, 0xA0AA: e1})
        bw, mw, bs = bend / 8192.0, mod / 127.0, (brange + 1) / 12.0 * (bgain + 1)     # the engine's words (§3.7)
        w.update({0xC022: bw, 0xC023: sd / 255.0 * bs, 0xC024: mw, 0xC025: md / 255.0, 0xA0A6: lf,
                  0xC122: bw, 0xC123: sv / 255.0 * bs, 0xC124: mw, 0xC125: mv / 255.0 * 10})
        ext = [brange, bgain, sd, sv, md, mv]                                          # Roland's values (raw)
        kk = list(k)
        kk[11] = (k[11] - 11) & 255                                                      # word 0x173 as the S1 holds it
        out.append('K ' + ' '.join(str(x) for x in kk) + ' %d %d %d %d %d ' % (note, q(e1), q(lf), bend, mod)
                   + ' '.join(str(x) for x in ext))
        out += ['W %04x %.9g' % (a, v) for a, v in sorted(w.items()) if v]
        out.append('E')
    path = os.path.join(ROOT, 'work', 'jx3p', 'arm_vectors.txt')
    open(path, 'w').write('\n'.join(out) + '\n')
    print(path, len(cases), 'cases')


# SYSTEM-1 LFO SHAPE (0 sine, 1 triangle, 2 saw, 3 square, 4 sample & glide, 5 sample & hold) -> the JX-3P's LFO
# WAVEFORM (0 SINE, 1 SQUARE, 2 RANDOM): a panel choice (README.md §3.5)
JL_WAVE = [0, 1, 2, 2, 2, 2]   # LFO WAVEFORM 0 SINE, 1 SQUARE, >= 2 RANDOM (Roland's rows 0x6cd: C02C..C02E)

ARM_LFO_C = r'''
/* the JX-3P's LFO (MAIN0 instructions 66..147, README.md §3.5): one for all voices. Phase -1..1 (uint32, 2^32 = span 2)
   += inc per sample, inc = min((0.46623794 e^(5.28342146 RATE) - 0.413383555) 2^-16, 0.0155343) (x 2 for RANDOM).
   SINE is held and taken again when a counter (+ max(4 inc, 0.0010212418) per sample) reaches 1 or the phase
   changes sign (every half cycle, measured: lfo_check.c): 8 steps per cycle when fast, one per ~979 samples when slow (the JX-3P's CPU LFO); SQUARE is not held. SINE = 1.3674 t +
   1.8136 t^2 - 1.181 t^3 - 1, t = |wrap(phase + 1)|; SQUARE = sign(phase); RANDOM = a new value at each wrap (Roland's
   is the master's noise word; ours an LCG). Per voice x (delay + 1 - C027): the delay ramps by A02D per sample from
   the note on to 1; C027 = 0 for DELAY 0 (the LFO at x 2), else 1 */
#define ARM_CUSTOM_TICK 1
#define ARM_CUSTOM_GATE 1
static uint32_t jl_p, jl_fade[NV], jl_rnd = 0x1234567u;
static int32_t jl_c, jl_held;
static int32_t jl_sample(int w)
{
    int32_t p = (int32_t)jl_p;
    if (w == 1)
        return p > 0 ? Q1 : p < 0 ? -Q1 : 0;
    if (w == 2) {
        jl_rnd = jl_rnd * 1664525u + 1013904223u;
        return (int32_t)jl_rnd >> 7;                   /* -1..1, Q24 */
    }
    int32_t q = (int32_t)(jl_p + 0x80000000u);          /* wrap(phase + 1) */
    int32_t t = (q < 0 ? -(q + 1) : q) >> 7;            /* |.|, Q24 */
    int32_t t2 = qmul(t, t), t3 = qmul(t2, t);
    return qmul(t, 22941165) + qmul(t2, 30427159) - qmul(t3, 19813892) - Q1;   /* 1.3674 1.8136 1.181, Q24 */
}
static void arm_custom_tick(int n)
{
    int w = arm_jl_wave[hlfo_knob[C_LFO_SHAPE] > 5 ? 5 : hlfo_knob[C_LFO_SHAPE]];
    int r = hlfo_knob[C_LFO_RATE];
    uint32_t inc = arm_jl_inc[r] << (w == 2);
    int32_t step = arm_jl_step[r];
    if (w == 2)
        step = step * 2 > 17134 ? step * 2 : 17134;    /* max(4 inc, 0.0010212418) of the doubled step */
    for (int i = 0; i < n; i++) {
        uint32_t old = jl_p;
        jl_p += inc;
        int wrapped = (int32_t)old >= 0 && (int32_t)jl_p < 0;     /* +1 -> -1 */
        int half = (int32_t)(old ^ jl_p) < 0;                     /* the sign changes: every half cycle */
        jl_c += step;
        if (jl_c >= Q1 || half) {                       /* SINE: held at the steps and half cycles */
            jl_c = 0;
            if (w == 0 || (w == 2 && wrapped))          /* RANDOM: a new value at each wrap */
                jl_held = jl_sample(w);
        }
    }
    uint32_t f = arm_jl_fade[hlfo_knob[C_LFO_DELAY]] * (uint32_t)n;
    for (int v = 0; v < NV; v++)
        jl_fade[v] = jl_fade[v] >= 0x80000000u - f ? 0x80000000u : jl_fade[v] + f;
}
static uint8_t jx_gate[NV];                           /* VCA MODE GATE: the voice's key */
static void arm_custom_gate(int v, int on)
{
    jx_gate[v] = (uint8_t)(on != 0);
    if (!on)
        return;
    jl_fade[v] = 0;                                    /* the delay from the note on */
    if (lfo_kt) {                                      /* LFO KEY TRIG (the SYSTEM-1 button): the phase from 0, */
        jl_p = 0;                                      /* the first value taken at once (Roland: at sample 1) */
        jl_c = Q1;
    }
}
static int32_t jl_raw(void)                            /* MAIN0's A0A6: the LFO's wave alone, -1..1, no delay, no x 2
                                                          (work/jx3p/lfo_ref.npz: A0A6 = A0A5 / 2 at DELAY 0, full
                                                          from the first sample at any DELAY) */
{
    int w = arm_jl_wave[hlfo_knob[C_LFO_SHAPE] > 5 ? 5 : hlfo_knob[C_LFO_SHAPE]];
    return w == 1 ? jl_sample(1) : jl_held;
}
static int32_t jl_lfo(int v)                           /* the voice's LFO (A0A5), Q24, -2..2 */
{
    int32_t g = (int32_t)(jl_fade[v] >> 7) + (arm_jl_c027[hlfo_knob[C_LFO_DELAY]] ? 0 : Q1);
    int w = arm_jl_wave[hlfo_knob[C_LFO_SHAPE] > 5 ? 5 : hlfo_knob[C_LFO_SHAPE]];
    int32_t x = w == 1 ? jl_sample(1) : jl_held;      /* SQUARE: the phase's sign at every sample, not held */
    return qmul(x, g);
}
'''

ARM_ENV_C = r'''
/* the JX-3P's envelopes (MAIN0 instructions 155..253, the same for ENV1 and ENV2; README.md §3.4): computed once per
   192 samples (a counter of 1/192 per sample; 2 steps of 1 ms here), the first at the note on, held in between.
   Per tick: attack x = min(x + A, 1), then decay; decay x = max(S + (x - S)(1 - D) - 1.49999e-5 (0.5 - x), S);
   release x = max(x (1 - R) - 1.49999e-5 (0.5 - x), 0). A, D, S, R: Roland's rows of ENV ATTACK / DECAY / SUSTAIN /
   RELEASE (0..255 = the SYSTEM-1 fader). i = 0 AMP faders = ENV2 (the VCA), 1 FILTER faders = ENV1 */
#define ARM_CUSTOM_ENV 1
#define ARM_ENV_C (-252)                               /* -1.49999e-5 (-0x1.f75p-17), Q24 */
static uint8_t arm_env_ph[2][NV], arm_env_prev[2][NV];
static int arm_custom_env(int i, int v, int st, int32_t *l, const uint8_t *k, int n)
{
    int32_t x = *l;
    if (st == ST_ATT && arm_env_prev[i][v] != ST_ATT)
        arm_env_ph[i][v] = 0;                          /* a note on: the first tick now */
    for (int j = 0; j < n && st != ST_IDLE; j++) {
        if (arm_env_ph[i][v]++ & 1)                    /* one tick every 2 steps (192 samples) */
            continue;
        int32_t t = qmul(Q1 / 2 - x, ARM_ENV_C);
        if (st == ST_ATT) {
            x += arm_env_a[k[1]];
            if (x >= Q1) {
                x = Q1;
                st = ST_DEC;
            }
        } else if (st == ST_DEC) {
            int32_t s = arm_env_s[k[0]];
            x = s + qmul(x - s, arm_env_d1[k[2]]) + t;
            if (x < s)
                x = s;
        } else if (st == ST_REL) {
            x = qmul(x, arm_env_r1[k[3]]) + t;
            if (x <= 0) {
                x = 0;
                st = ST_IDLE;
            }
        }
    }
    arm_env_prev[i][v] = (uint8_t)st;
    *l = x;
    return st;
}
'''

ARM_C = r'''
static int32_t arm_lerp(const int32_t *t, int n, int32_t x, int sh)    /* x: index in Q(sh), clamped to the table */
{
    if (x <= 0)
        return t[0];
    int i = x >> sh;
    if (i >= n)
        return t[n];
    int32_t f = x & ((1 << sh) - 1);
    return t[i] + (int32_t)(((int64_t)(t[i + 1] - t[i]) * f) >> sh);
}
static uint32_t arm_mant2(int32_t x)                   /* 2^frac(x) - 1, x Q24: 23-bit mantissa (linear in arm_mant) */
{
    uint32_t f = (uint32_t)x & 0xFFFFFFu, i = f >> 14, r = f & 0x3FFFu;
    return (uint32_t)arm_mant[i] + (uint32_t)(((uint64_t)(uint32_t)(arm_mant[i + 1] - arm_mant[i]) * r) >> 14);
}
static uint32_t arm_exp2_bits(int32_t x)               /* 2^x (x Q24) as IEEE float bits */
{
    int32_t e = x >> 24;
    if (e < -126)
        return 0;
    return ((uint32_t)(e + 127) << 23) | (arm_mant2(x) & 0x7FFFFFu);
}
static int32_t arm_exp2_q24(int32_t x)                 /* 2^x, x Q24 <= 0 -> Q24 */
{
    int32_t n = x >> 24;
    if (n < -24)
        return 0;
    return (int32_t)(((1u << 24) + (arm_mant2(x) << 1)) >> -n);
}
static int32_t arm_floor24(int32_t x) { return x & ~0xFFFFFF; }
/* instructions 64..74: Roland's stretch of a DCO's pitch t (octaves): t + max(min(0.0004 floor(t) - 0.0016, 0.001),
   -1e-4), then limited to -4 .. 10 */
static int32_t arm_stretch(int32_t t)
{
    int32_t s = (int32_t)(((int64_t)arm_floor24(t) * 6711) >> 24) - 26844;   /* 0.0004 floor(t) - 0.0016 */
    s = s > 16777 ? 16777 : s < -1678 ? -1678 : s;
    t += s;
    return t > (10 << 24) ? (10 << 24) : t < -(4 << 24) ? -(4 << 24) : t;
}
/* the DCO pitches (instructions 38..74): note (n - 12) / 12 (+ ARM_S1_OCT), + A024 / A025, + RANGE, DCO2 + TUNE + FINE TUNE, + the
   modulation by FREQ MOD: LFO x DCO LFO MOD, ENV1 term x DCO ENV1 MOD (env x m (1 - 2 pol) + m pol); bend and
   modulation (jx_bend / jx_mod, below). Writes s1_dt / s1_rdt = DCO1's step and its inverse
   (/ 16384), s1_pitch = DCO2's step (a float word: exact at any distance from DCO1) and s1_arm(0) = dt1 / dt2 / 256
   (Q24, DCO2 within 8 octaves of DCO1: only the width of DCO2's BLEP, rd2 = rd1 x 256 s1_arm(0)). The kernel gives
   s1cc's native reference the same meanings (S1_REF_INPUTS, README.md §3.1) */
/* bend and modulation (README.md §3.7; Roland's engine, measured in unicorn: tools/plugout_dsp_tables.py's Engine):
   lever words C022 / C122 = bend / 8192 (-1..1), C024 / C124 = mod; sensitivity words
   C023 (DCO) / C123 (VCF) = SENS / 255 x (BEND RANGE + 1) / 12 x (BEND GAIN + 1), C025 (DCO) = MOD SENS / 255,
   C125 (VCF) = MOD SENS / 255 x 10. The voice (jx3p_voice.h 56, 57, 58: both DCOs; 916, 920: the filter's sum):
   + C022 C023 and + A0A6 C024 C025 to the pitch, + C122 C123 and + A0A6 C124 C125 to the cutoff (A0A6 = jl_raw).
   mod: the SYSTEM-1's CC 1 (model.c ID_MOD 0x131, 0..127) / 127 */
/* no 64-bit division in region 1 (no libgcc): the constant divisors as Q24 / Q32 reciprocals (relative error < 1e-5) */
static int32_t jx_bend(const uint8_t *k, int sens)     /* the lever word x the sensitivity word, Q24 */
{
    int b = cust_bend < -8192 ? -8192 : cust_bend > 8192 ? 8192 : cust_bend;
    int32_t w = sens * (k[16] + 1) * ((k[17] & 3) + 1);   /* <= 255 x 24 x 4 */
    return (int32_t)(((int64_t)(b * w) * 11228673) >> 24);  /* x Q1 / (8192 x 255 x 12) = x 2048 / 3060 */
}
static int32_t jx_mod(int32_t raw, int sens, int x)    /* A0A6 x mod / 127 x sens / 255 x x, Q24 */
{
    int m = cust_mod < 0 ? 0 : cust_mod > 127 ? 127 : cust_mod;
    int32_t c = (int32_t)(((int64_t)(m * sens * x) * 132622) >> 8);              /* x Q1 / (127 x 255): 2^32 / 32385 */
    return qmul(raw, c);
}
static void arm_custom_pitch(int v, int32_t e1, int32_t lf, int note, const uint8_t *k)
{
    lf = jl_lfo(v);                                    /* the JX-3P's LFO (§3.5), not the host's */
    int32_t n = (int32_t)(((note & 127) - 12) * 1398101) + ARM_S1_OCT + jx_bend(k, k[18])
                + jx_mod(jl_raw(), k[20], 1);
    int32_t pl = qmul(lf, arm_lfop[k[6]]);
    int32_t m = arm_denv[k[7]], pe = arm_dpol[k[7]] ? m - qmul(e1, m) : qmul(e1, m);
    int f1 = arm_fmod[k[12] > 5 ? 5 : k[12]], f2 = arm_fmod[k[13] > 5 ? 5 : k[13]];
    int r1 = k[8] > 5 ? 5 : k[8], r2 = k[9] > 5 ? 5 : k[9];
    int32_t o1 = n + ARM_A024 + arm_rng1[r1] + ((f1 & 1) ? pl : 0) + ((f1 & 2) ? pe : 0);
    int32_t o2 = n + ARM_A025 + arm_rng2[r2] + arm_tune[(uint8_t)(k[11] + 11)] + arm_fine[k[10]] + ((f2 & 1) ? pl : 0)
                 + ((f2 & 2) ? pe : 0);
    o1 = arm_stretch(o1);
    o2 = arm_stretch(o2);
    int32_t d = o2 - o1;
    d = d > (8 << 24) ? (8 << 24) : d < -(8 << 24) ? -(8 << 24) : d;
    b5(B5_DT + v, arm_exp2_bits(o1 + ARM_LOG2_STEP));
    b5(B5_RDT + v, arm_exp2_bits(-o1 - ARM_LOG2_STEP - (14 << 24)));
    b5(B5_PITCH + v, arm_exp2_bits(o2 + ARM_LOG2_STEP));
    ramp(P_ARM0 + v, arm_exp2_q24(-d - (8 << 24)));
}
#define ARM_CUSTOM_PITCH 1
/* per voice, every step: e0 / e1 the AMP / FILTER envelope (Q24), lf the LFO x fade, note, k[] the knobs.
   The filter (instructions 881..969): u = 11.6 vcf(CUTOFF) - 0.7 + min((1 - RES) 0.5, 0.075) + BRILLIANCE
   + LFO x VCF LFO MOD + ENV1 term x VCF ENV1 MOD + (note - 60) / 12 x PITCH FOLLOW; g = arm_g(u) -> s1_env(1).
   The VCA (1047..1050): its source (VCA MODE) x 0.013004484 x VCA LEVEL -> s1_env(0) (the kernel's one-pole) */
static void arm_custom_voice(int v, int32_t e0, int32_t e1, int32_t lf, int note, const uint8_t *k, int lfo_k)
{
    (void)lfo_k;
    lf = jl_lfo(v);                                    /* the JX-3P's LFO (§3.5), not the host's */
    arm_custom_pitch(v, e1, lf, note, k);
    int32_t m = arm_envm[k[2]], et = arm_envp[k[2]] ? m - qmul(e1, m) : qmul(e1, m);
    int32_t u = arm_base[k[0]] + arm_rterm[k[1]] + arm_bril[k[5]] + qmul(lf, arm_lfof[k[4]]) + et
              + qmul(arm_kf[k[3]], (int32_t)(((note & 127) - 60) * 1398101))
              + jx_bend(k, k[19]) + jx_mod(jl_raw(), k[21], 10);
    ramp(P_ENV0 + 8 + v, arm_lerp(arm_g, ARM_NU, u - ARM_U0, ARM_USH));
    /* VCA MODE (examples/from_scratch/engine/model_spec.py 0x736: 0 -> A02B ENV-1, 1 -> A02C ENV-2, 2 -> A02D gate;
       3 as 0): the VCA's source, x VCA LEVEL (C14B, Roland's row over its 128) */
    int mode = k[14];
    int32_t src = mode == 1 ? e0 : mode == 2 ? (jx_gate[v] ? Q1 : 0) : e1;
    ramp(P_ENV0 + v, qmul(qmul(src, ARM_VCA), arm_vlev[k[15]]));
}
'''

if __name__ == '__main__':
    main()
