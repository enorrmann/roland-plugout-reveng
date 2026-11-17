"""make_voice.py: the SYSTEM-8 JX-3P PLUG-OUT's voice (audio path) as one DSP program and as s1cc C. README.md §2.

    python3 sdk/s1cc/examples/jx3p_ported/tools/make_voice.py [--dco1 saw|sqr] [--dco2 saw|sqr|noise] [--names]

Source: plugins/JX-3P (the user's JX-3P PLUG-OUT 1.01, Script/Code8_1.Dat region 1), cut by
examples/from_scratch/engine/programs.py (SHA-256 checked). On the SYSTEM-8 one voice runs on three DSPs of a slave:

    DSP0 MAIN0   LFO, ENV1, ENV2, pitch               -> MXM 8404..8416 -> DSP2
    DSP2 MAIN2   relays MAIN0's words to the slots (bank 1 22nn) and on to DSP4; mixer of the DCOs
         DCO1, DCO2 slot programs (oscillators of voices A and B)
    DSP4 MAIN4   VCF, VCA (voices A and B), the pair's sum, output

MAIN0 is not lifted: on the SYSTEM-1 its work is the ARM's (README.md §3). The other four programs become one
program, in the order they run in one sample (DSP2: main first, then the slots; then DSP4), with every literal address
rewritten (the same decoder as the emulator, examples/system_1_jx/dspreloc.py):

    DSP2 (MAIN2, DCO1, DCO2)  bank 1, bank 5 A020.., bank 6 C020..: unchanged; bank 0 0200.. / 0400.. / 0600.. ->
                              0080.. / 0100.. / 01a0.. (dsplift --bank1-ring: bank 0 below 0x400)
    MXM DSP0 -> DSP2  M 8004..8016            -> inputs A0A4..A0B6 (KIN: the ARM's MAIN0 words)
    MXM DSP2 -> DSP4  Z 84xx / M 80xx          -> bank-0 cell 0010 + 2 xx: store X, load X + 1 = one sample later,
                                                  the MXM latency (docs/vol2_dsp_core_isa/core_architecture.md §8.2)
    MXM DSP4 -> DSP2  M 801b..801d / S 861b..  -> bank-0 cells 0050, 0052, 0054 (the same one-sample ring)
    MXM DSP2 -> DSP0  M 8017..8019             -> outputs A1D7..A1D9 (MAIN0's inputs; unused here)
    DSP4 (MAIN4)      bank 0 0200.. -> 0240.., bank 1 + 0x100, bank 5 A0xx -> A0xx + 0x40, bank 6 Cxxx -> Cxxx + 0x100,
                      pair window A400..A403 (voices 2, 3 of DSP5) -> A0F8..A0FB (0), A404.. -> A1F4.. (unused),
                      IOMD 6000..6006 -> inputs A0D0..A0D6, IOMD outputs -> A1E0.. (unused)

Writes work/jx3p/voice_DCO1_DCO2.txt (the program) and, unless --program-only, the lifts (dsplift.py --kernel):
    jx3p_voice.h (the whole voice), jx3p_core_voice.h (no oscillators, g and VCA from the ARM: CORE_CELLS),
    jx3p_dco1_saw.h .. jx3p_dco2_noise.h (Roland's DCO programs alone).
"""
import argparse
import hashlib
import os
import re
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EX = os.path.abspath(os.path.join(HERE, '..'))
ROOT = os.path.abspath(os.path.join(EX, '..', '..', '..', '..'))
WORK = os.path.join(ROOT, 'work', 'jx3p')
for p in ('examples/system_1_jx', 'examples/from_scratch/engine', 'examples/from_scratch/re'):
    sys.path.insert(0, os.path.join(ROOT, p))
import dspreloc as R  # noqa: E402

MX = lambda xx: 0x010 + 2 * xx                       # DSP2 Z 84xx -> DSP4 M 80xx
MXB = lambda xx: 0x050 + 2 * (xx - 0x1B)             # DSP4 M 801b.. -> DSP2 S 861b..
# bank 0 of each program (dsplift --bank1-ring keeps bank 0 below 0x400: bank 1 is cells 0x400..), gaps >= 0x11
B0_SHIFT = {'MAIN2': -0x180, 'DCO1': -0x300, 'DCO2': -0x460, 'MAIN4': +0x40}   # 0080.. 0100.. 01a0.. 0240..
M4_B1, M4_B5, M4_B6 = 0x100, 0x40, 0x100


def roland(name):
    """execution-order words of one program of the user's JX-3P PLUG-OUT"""
    import programs as P
    os.makedirs(WORK, exist_ok=True)
    img = P.unpack(os.path.join(ROOT, 'plugins', 'JX-3P'), WORK)
    n, addr, sha = P.MICROCODE[name]
    blob = img[addr - P.BASE: addr - P.BASE + ((n + 3) & ~3)]
    if hashlib.sha256(blob).hexdigest()[:16] != sha:
        raise SystemExit(f'{name}: not the JX-3P 1.01 PLUG-OUT')
    h = struct.unpack('<%dH' % (len(blob) // 2), blob)
    w = []
    for i in range(0, len(h), 2):
        w += [h[i + 1], h[i]]
    return w[:n // 2]


def map_dsp2(r, unit):
    a = r.addr
    if a >> 12 == 0x0:
        return a + B0_SHIFT[unit]
    if a >> 12 == 0x8:
        x = a & 0x7FF
        if r.op == 'ld' and 0x004 <= x <= 0x016:
            return 0xA0A0 + x
        if r.op == 'ld' and 0x61B <= x <= 0x61D:
            return MXB(x - 0x600) + 1
        if r.op == 'st' and 0x404 <= x <= 0x41A:
            return MX(x - 0x400)
        if r.op == 'st' and 0x017 <= x <= 0x019:
            return 0xA1C0 + x
        raise SystemExit(f'DSP2: MXM {r.op} {a:04x}?')
    if a >> 12 in (0x2, 0xA, 0xC):
        return a
    raise SystemExit(f'DSP2: {r.op} {a:04x}?')


def map_dsp4(r):
    a = r.addr
    b = a >> 12
    if b == 0x0:
        return a + B0_SHIFT['MAIN4']
    if b == 0x2:
        return a + M4_B1
    if b == 0x8:
        x = a & 0x7FF
        if r.op == 'ld' and 0x005 <= x <= 0x01A:
            return MX(x) + 1
        if r.op == 'st' and 0x01B <= x <= 0x01D:
            return MXB(x)
        raise SystemExit(f'DSP4: MXM {r.op} {a:04x}?')
    if b == 0x6:
        if r.op == 'ld':
            return 0xA0D0 + (a & 0xF)
        return 0xA1E0 + (a & 0x7F) % 0x20 if (a & 0xFF) < 0x40 else 0xA1E0 + 0x8 + (a & 0xF)
    if b == 0xA:
        x = a & 0xFFF
        if x < 0x400:
            return a + M4_B5
        if r.op == 'ld':
            return 0xA0F8 + (x & 3)
        return 0xA1F4 + (x & 3)
    if b == 0xC:
        return a + M4_B6
    raise SystemExit(f'DSP4: {r.op} {a:04x}?')


def voice_program(dco1='saw', dco2='saw'):
    m2 = R.Prog(roland('main2'), 'MAIN2').relocate(lambda r: map_dsp2(r, 'MAIN2'))
    d1 = R.Prog(roland('dco1_' + dco1), 'DCO1').relocate(lambda r: map_dsp2(r, 'DCO1'))
    d2 = R.Prog(roland('dco2_' + dco2), 'DCO2').relocate(lambda r: map_dsp2(r, 'DCO2'))
    m4 = R.Prog(roland('main4'), 'MAIN4').relocate(map_dsp4)
    return R.concat(m2, d1, d2, m4)


LIFT = os.path.join(ROOT, 'sdk', 's1cc', 'tools', 'dsplift.py')
NAMES = os.path.join(EX, 'jx3p_names.txt')
# the core voice (README.md §2): what the ARM computes instead (jx3p_arm.h) and the oscillators (jx3p_osc.h) become
# inputs of the lift (dsplift --cell-in): DCO1 / DCO2 outputs and their sync words, the filter's coefficient g (cell
# 025f, MAIN4 instructions 881..969: cutoff knob curve, envelope, LFO, key follow -> exp -> tan / (1 + tan)) and the
# VCA's input term (cell 028e = 0.013 x the envelope mix, MAIN4 1047..1050)
# (bank 1 is a ring, --bank1-ring: a word stored at X is read at X + 1 in the next sample, so the cells cut are the
# read addresses: DCO1 out 221f -> 2220, its sync 221b -> 221c, DCO2 out 2221 -> 2222, its sync 221d -> 221e)
# and the two slew limiters of the VCA's level (instructions 1015..1035, cells 0283 / 0286): they move constant targets
# (C130 = 0.93 and the gate word A02A = 1, README.md §2.2) and only in the first ~3 ms a voice plays after loading;
# the kernel gives the targets (2026-11-17: ~10 s1cc units per voice, the 8 voices with CROSS MOD)
CORE_CELLS = '2220=a0c0,221c=a0c1,2222=a0c2,221e=a0c3,025f=a0c4,028e=a0c5,0283=a0c6,0286=a0c7'
# a DCO program alone: its mailbox inputs (pitch, wave flags, sync) as KIN(A0D0..A0D4)
DCO_IN = {'DCO1': '2201=a0d0,2203=a0d1,2204=a0d2,2205=a0d3,2206=a0d4',
          'DCO2': '2207=a0d0,2209=a0d1,220a=a0d2,220b=a0d3,220c=a0d4'}


def params_seed():
    """work/jx3p/params.txt: the parameter words the panel moves, named after Roland's patch parameters
    (work/jx3p/jx_arm.json): DSP2 C0xx as they are, DSP4 C0xx -> C1xx and A0xx -> A0xx + 0x40 (map_dsp4)"""
    import json
    src = os.path.join(WORK, 'jx_arm.json')
    if not os.path.exists(src):
        subprocess.run([sys.executable, os.path.join(HERE, 'jx_arm.py')], check=True)
    J = json.load(open(src))
    out, seen = [], set()
    for pid, p in J['params'].items():
        nm = re.sub(r'[^a-z0-9]+', '_', p['name'].lower()).strip('_')
        for i, k in enumerate(p['diff']):
            d, kind, idx = k.split(':')
            if d not in '24':
                continue
            idx = int(idx, 16)
            if kind == 'C':
                a = 'C%03X' % (idx + (M4_B6 if d == '4' else 0))
                line = 'param %s %s%s knob' % (a, nm, '' if i == 0 else '_%d' % i)
            else:
                a = 'A%03X' % (idx + (M4_B5 if d == '4' else 0))
                line = 'in %s %s%s' % (a, nm, '' if i == 0 else '_%d' % i)
            if a not in seen:
                seen.add(a)
                out.append(line)
    path = os.path.join(WORK, 'params.txt')
    open(path, 'w').write('# written by tools/make_voice.py from work/jx3p/jx_arm.json\n' + '\n'.join(out) + '\n')
    return path


def lift(out, name, words, *opts, names=False):
    """the readable lift in two steps (README.md §2.1): step 1 (make_voice.py --names, or when the names file is
    missing) writes jx3p_NAME_names.txt with sdk/s1cc/tools/dsplift_names.py from the seeds; step 2 lifts with it"""
    path = os.path.join(WORK, name + '.txt')
    open(path, 'w').write(' '.join('%04x' % x for x in words) + '\n')
    args = ['--kernel', '--bank1-ring', *opts, '%s=%s' % (name, path)]
    gen = os.path.join(EX, name + '_names.txt')
    if names or not os.path.exists(gen):
        subprocess.run([sys.executable, os.path.join(os.path.dirname(LIFT), 'dsplift_names.py'), gen,
                        '--names', params_seed(), '--names', NAMES] + args, check=True)
    subprocess.run([sys.executable, LIFT, os.path.join(EX, out), '--names', gen] + args, check=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--dco1', default='saw', choices=('saw', 'sqr'))
    ap.add_argument('--dco2', default='saw', choices=('saw', 'sqr', 'noise'))
    ap.add_argument('--program-only', action='store_true')
    ap.add_argument('--names', action='store_true', help='rewrite the jx3p_*_names.txt files (step 1) before the lifts')
    ap.add_argument('--check', action='store_true', help='dsplift --check of the joined program against the emulator')
    a = ap.parse_args()
    w = voice_program(a.dco1, a.dco2)
    path = os.path.join(WORK, f'voice_{a.dco1}_{a.dco2}.txt')
    open(path, 'w').write(' '.join('%04x' % x for x in w) + '\n')
    print(f'{path}: {len(w)} words')
    if a.check:
        subprocess.run([sys.executable, LIFT, os.path.join(WORK, 'lift_check.c'), f'jx3p_voice={path}', '--check',
                        '--samples', '16'], check=True)
    if a.program_only:
        return
    # 1. the whole voice as Roland runs it (one voice: output 231e; voice B's oscillators drop out): the original in C
    lift('jx3p_voice.h', 'jx3p_voice', w, '--kout', '231e', names=a.names)
    # 2. the core: filter, VCA, brilliance of Roland; oscillators and modulation from outside
    lift('jx3p_core_voice.h', 'jx3p_core', w, '--kout', '231e', '--cell-in', CORE_CELLS, names=a.names)
    # 3. Roland's DCO programs alone (the originals our oscillators replace, jx3p_osc.h)
    for p in ('dco1_saw', 'dco1_sqr', 'dco2_saw', 'dco2_sqr', 'dco2_noise'):
        u = p[:4].upper()
        dw = R.concat(R.Prog(roland(p), u).relocate(lambda r, u=u: map_dsp2(r, u)))
        lift('jx3p_%s.h' % p, 'jx3p_' + p, dw, '--kout', '221f,221b,2221,221d', '--cell-in', DCO_IN[u], names=a.names)


if __name__ == '__main__':
    main()
