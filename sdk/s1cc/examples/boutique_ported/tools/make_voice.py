"""make_voice.py: one Boutique model's voice program as s1cc C (bq_MODEL_voice.h). README.md §2.

    python3 sdk/s1cc/examples/boutique_ported/tools/make_voice.py MODEL OSC1 OSC2

MODEL jp08 / ju06 / jx03; OSC1, OSC2 = the oscillator program variants 0..8 for the two oscillator slots (README.md
§1: the firmware picks them by model and waveform). Needs work/bq1/progs (tools/bq_extract.py) and
work/bq1/MODEL_patches.json + bq_MODEL_tables.h (tools/bq_arm.py).

1. The programs of DSP chip 0 that make voice A, in the order they run: g0_pre (LFO, noise), g0_voiceA (glide,
   envelopes, modulation, mixer, filter, VCA), the OSC1 / OSC2 programs; each with the model's immediate patches
   (what the ARM writes into the programs at set-up).
2. The program's own per-sample constants converted 44.1 -> 96 kHz (RATE_IMM): the gate counter step, the filter's
   frequency scale, the LFO step limit, the shortest attack. (The VCA level smoothing pole 0.0075531 is a MAC
   coefficient, not converted: it only smooths a constant level.)
3. dsplift.py --bq2 --kernel --bank1-ring (bank 1 carries the values between the programs) --kout 203a (the voice
   output, to the post program) --names bq_MODEL*_names.txt --consts (the init words no panel control moves).

    python3 .../make_voice.py MODEL OSC1 OSC2 [--host ...] --names

writes bq_MODEL*_names.txt instead (step 1, README.md §2.1): the seeds (the panel's parameters from the tables,
bq_names*.txt) and sdk/s1cc/tools/dsplift_names.py's automatic cell names; the lift (step 2) only reads it.
"""
import json
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EX = os.path.abspath(os.path.join(HERE, '..'))
ROOT = os.path.abspath(os.path.join(EX, '..', '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
import isa  # noqa: E402

T = isa.Table.load()
R = 44100.0 / 96000.0
BITS = {0: 16, 1: 24, 2: 32}
# the program's per-sample constants: (value as disassembled, its 96 kHz value)
RATE_IMM = [(0.0016275998, 0.0016275998 * R),                        # gate counter step (retrigger timing)
            (0.007836163, 0.007836163 * R),                          # filter: cutoff frequency / sample rate
            (0.0155339998, 0.0155339998 * R),                        # LFO: largest phase step
            (7.927418e-06, 7.927418e-06 * R)]                        # envelopes: shortest attack step


def rd(name):
    return [int(x, 16) for x in open(os.path.join(ROOT, 'work', 'bq1', 'progs', name)).read().split()]


def raw(v, bits):
    return ((T.raw_encode(v) & 0xFFFFFFFF) >> (32 - bits)) if v else 0


def patch(words, base, patches):
    b = bytearray()
    for w in words:
        b += bytes([w >> 8, w & 0xFF])
    n = 0
    for typ, addr, val in patches:
        off, bits = addr - base, BITS[typ]
        if 0 <= off and off + bits // 8 <= len(b):
            x = raw(val, bits)
            for k in range(bits // 8):
                b[off + k] = (x >> (bits - 8 - 8 * k)) & 0xFF
            n += 1
    return [(b[i] << 8) | b[i + 1] for i in range(0, len(b), 2)], n


def rate_patch(words, base):
    """the RATE_IMM immediates, found through the disassembler (value and position), rewritten"""
    import disasm
    text, _ = disasm.program(words, T, 'x')
    out = []
    hw = 0
    for line in text.split('\n'):
        m = re.search(r';\s*\(\w+\)\s+([0-9a-f ]+)$', line)
        if not m or line.startswith(';') or line.startswith('.'):
            continue
        ws = m.group(1).split()
        for val, new in RATE_IMM:
            for im in re.finditer(r'#(-?[0-9.e+-]+)@(\d+):(\d+)', line):
                if abs(float(im.group(1)) - val) <= 1e-4 * abs(val):
                    # the immediate stream starts after the instruction's op / transfer words: find its bytes
                    bits = int(im.group(3))
                    old = raw(val, bits)
                    blob = bytearray()
                    for w in ws:
                        blob += bytes([int(w, 16) >> 8, int(w, 16) & 0xFF])
                    pat = old.to_bytes(bits // 8, 'big')
                    k = blob.find(pat, 2)
                    if k < 0:
                        raise SystemExit('rate patch: %r not found in %s' % (val, line))
                    out.append((BITS_T[bits], base + 2 * hw + k, new))
        hw += len(ws)
    return out


BITS_T = {16: 0, 24: 1, 32: 2}


def instr_bounds(words):
    """the halfword offset of every instruction start (and the end)"""
    import disasm
    text, _ = disasm.program(words, T, 'x')
    out, hw = [0], 0
    for line in text.split('\n'):
        m = re.search(r';\s*\(\w+\)\s+([0-9a-f ]+)$', line)
        if not m or line.startswith(';') or line.startswith('.'):
            continue
        hw += len(m.group(1).split())
        out.append(hw)
    assert hw == len(words), (hw, len(words))
    return out


# the voice program is cut at VOICE_SPLIT (its first read of the oscillators' outputs): DSP A runs the pre program,
# the voice up to there (LFO, glide, envelopes, modulation, the oscillators' pitch) and OSC2 (whose output modulates
# OSC1's pitch: CROSS MOD), DSP B OSC1 and the rest of the voice (mixer, filter, VCA). Every value flows A -> B
# (README.md §2, §5)
VOICE_SPLIT = 185


def arm_fix(src):
    """--arm (with --host --osc2x; SYSTEM-S1CC-BOUTIQUE ARM_CUSTOM, README.md §11): the filter's coefficient g (the
    value at instruction 267) and the VCA's curve (instruction 321) come from the ARM (tools/make_arm.py computes them
    every 1 ms from the knobs, the envelopes, the note and the LFO): KIN(A0F5) = g, KIN(A0F6) = the gain. What only
    fed them leaves the DSP (s1cc removes it)"""
    out, n = [], 0
    for ln in src.split('\n'):
        m = re.match(r'(\s*float \w+ = )\w+ \* 2\.0f;(\s*/\* 267 \*/.*)$', ln)
        if m:
            ln = '%sKIN(0xa0f5);%s' % (m.group(1), m.group(2).replace('*/', ' arm_fix: g from the ARM */'))
            n += 1
        m = re.match(r'(\s*float t321 = )fmaxf\(0\.444274f \* .*;(\s*/\*.*)$', ln)
        if m:
            ln = '%sKIN(0xa0f6);%s' % (m.group(1), m.group(2).replace('*/', ' arm_fix: the VCA gain from the ARM */'))
            n += 1
        out.append(ln)
    if n != 2:
        raise SystemExit('arm_fix: %d of 2 patterns found' % n)
    return '\n'.join(out)


def micro_fix(src):
    """exact algebra on the filter's coefficient (README.md §10): Roland computes g = tan / (tan + 1) as
    s (1 / c) (1 / (s / c + 1)), two reciprocals; it is s / (s + c), one (s = sin, c = cos of the same small angle, both
    > 0). And 1 / (1 + k g^4) (k >= 0) needs no sign or limit: K_RECIP_POS"""
    lines, n = src.split('\n'), 0
    for i in range(len(lines) - 1):
        m = re.match(r'(\s*)float (t\d+) = K_RECIP\((.*)\);(\s*/\*.*)?$', lines[i])
        if not m:
            continue
        t = m.group(2)
        m2 = re.match(r'(\s*float \w+ = )(t\d+) \* %s \* K_RECIP\(%s \* \2 \+ 1\.0f\);(.*)$' % (t, t), lines[i + 1])
        if m2 and src.count(t) == 3:                   # t is used only by the next line
            lines[i] = None
            lines[i + 1] = '%s%s * K_RECIP_POS(%s + (%s));%s' % (m2.group(1), m2.group(2), m2.group(2), m.group(3),
                                                                m2.group(3).replace('*/', ' micro_fix */'))
            n += 1
    lines = [l for l in lines if l is not None]
    for i, l in enumerate(lines):
        m = re.match(r'(\s*float t\d+ = )K_RECIP\((1\.0f \+ (t\d+) \* \((t\d+) \* \4\))\);(.*)$', l)
        if m:
            lines[i] = '%sK_RECIP_POS(%s);%s' % (m.group(1), m.group(2), m.group(5).replace('*/', ' micro_fix */'))
            n += 1
    if n != 2:
        raise SystemExit('micro_fix: %d of 2 patterns found' % n)
    return '\n'.join(lines)


def attack_fix(src):
    """the envelopes (ENV1, ENV2): Roland's program applies the attack step (toward 1.05) and the decay pull (toward
    SUSTAIN x 0.87) in the same sample, and the attack ends only above 0.8925. With a fast DECAY and a low SUSTAIN the
    two balance below 0.8925 (ATTACK 0, DECAY 0, SUSTAIN 0: 0.81 for ever): the attack never ends and the note holds
    as a gate (measured on the lift and heard on the device, 2026-11-11). Here the decay pull is skipped while the
    attack runs (attack flag x gate > 0), and in the release (below). README.md §2."""
    out, n = [], 0
    lines = src.split('\n')
    for i, ln in enumerate(lines):
        m = re.match(r'(\s*float (t\d+) = )fmaxf\(0\.01f \* \((t\d+) \* (t\d+)\) \+ (t\d+) \+ \3 \* 0\.99f, 0\.0f\);(.*)$', ln)
        if m:
            att, tgt = m.group(3), m.group(5)
            # the attack flag: the nearest SEL(K_GT0(flag * key), ...) above (the lift may cut the expression in
            # named parts: the layout depends on the names' lengths)
            f = next((re.search(r'SEL\(K_GT0\((\w+) \* (\w+)\)', l) for l in reversed(lines[max(0, i - 12):i])
                      if re.search(r'SEL\(K_GT0\((\w+) \* (\w+)\)', l)), None)
            prev = att
            if not f:
                raise SystemExit('attack_fix: no attack flag in %r' % prev)
            ln = '%sSEL(K_GT0(%s * %s), %s + %s, fmaxf(0.01f * (%s * %s) + %s + %s * 0.99f, 0.0f));%s' % (
                m.group(1), f.group(1), f.group(2), att, tgt, att, m.group(4), tgt, att, m.group(6).replace('*/', ' attack_fix */'))
            n += 1
        out.append(ln)
    if n != 2:
        raise SystemExit('attack_fix: %d envelopes found, 2 expected' % n)
    # the release: the same balance (release toward ~0.02 against the decay pull toward SUSTAIN x 0.87): SUSTAIN 255,
    # DECAY 0, RELEASE 0 held the note at 0.44 after the key was released (the "stuck note" heard on the device,
    # 2026-11-11). The release starts from the envelope's value of the sample before, without the decay pull
    out2, r = [], 0
    for ln in out:
        m = re.match(r'(\s*float ((\w+)) = SEL\((\w+|K_LE0\(\w+\)), fmaxf\(LERP\((t\d+), )(t\d+)(, 0\.99f\) \+ 0\.01f '
                     r'\* \(\()(t\d+)( - \5\) \* t\d+\), 0\.0f\), \6\);)(.*)$', ln)
        if m and m.group(6) == m.group(8):
            c = re.match(r'c_([0-9a-f]{3})$', m.group(3))
            d = ('D_%s_1' % c.group(1)) if c else m.group(3) + '_z1'      # the cell's static (dsplift_c Names.static)
            ln = '%s%s%s%s%s%s' % (m.group(1), d, m.group(7), d, m.group(9), m.group(10).replace('*/', ' release_fix */'))
            r += 1
        out2.append(ln)
    if r != 2:
        raise SystemExit('release_fix: %d envelopes found, 2 expected' % r)
    return '\n'.join(out2)


# --host: the host's envelopes and LFO (SYSTEM-S1CC ARM_ENV / ARM_LFO, examples/system_s1cc/README.md §3.7) instead of
# Roland's. The pre program (LFO, noise) is left out; the cells the voice keeps its envelopes in (ENV1 07b, ENV2 088)
# and the LFO x fade the pre program leaves (039) become inputs (dsplift --cell-in): KIN(A0F0) = FILTER ENV
# (s1_env(1)), KIN(A0F1) = AMP ENV (s1_env(0)), KIN(A0F2) = s1_lfo_filter() (octaves, the cutoff's unit). The whole
# voice in one function (no --split): SYSTEM-S1CC's fused build places whole voices on DSP A and DSP B. README.md §8
HOST_CELLS = '07b=a0f0,088=a0f1,039=a0f2'
# --osc2x (with --host): the oscillator programs left out too; the voice hands their inputs out (KOUT 200a OSC1 pitch,
# 200f OSC2 pitch, 200b pulse width) and reads their outputs of the sample before (2019 OSC1, 201e OSC2) from the
# caller, who runs bq_osc2x.h (README.md §9)
OSC2X_CELLS = '2019=a0f3,201e=a0f4'


def main():
    host = '--host' in sys.argv
    osc2x = '--osc2x' in sys.argv
    armc = '--arm' in sys.argv
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    model, v1, v2 = args[0], int(args[1]), int(args[2])
    sys.path.insert(0, os.path.join(ROOT, 'asm'))
    pt = [tuple(x) for x in json.load(open(os.path.join(ROOT, 'work', 'bq1', '%s_patches.json' % model)))]
    total, nrate, prog = 0, 0, {}
    for key, name, base in (('pre', 'g0_pre_1000.hex', 0x1000), ('voice', 'g0_voiceA_1900.hex', 0x1900),
                            ('osc1', 'osc%d_3f00.hex' % v1, 0x3F00), ('osc2', 'osc%d_4f00.hex' % v2, 0x4F00)):
        w, n = patch(rd(name), base, pt)
        rp = rate_patch(w, base)
        w, n2 = patch(w, base, rp)
        prog[key] = w
        total += n
        nrate += n2
    vb = instr_bounds(prog['voice'])
    cut = vb[VOICE_SPLIT]
    if host and osc2x:
        words = list(prog['voice'])
    elif host:
        words = prog['voice'] + prog['osc1'] + prog['osc2']
    else:
        words = prog['pre'] + prog['voice'][:cut] + prog['osc2'] + prog['osc1'] + prog['voice'][cut:]
    split = len(instr_bounds(prog['pre'])) - 1 + VOICE_SPLIT + len(instr_bounds(prog['osc2'])) - 1
    sfx = (('_arm' if armc else '_host2x') if osc2x else '_host') if host else ''
    work = os.path.join(ROOT, 'work', 'bq1')
    hexf = os.path.join(work, '%s%s_voice.hex' % (model, sfx))
    open(hexf, 'w').write(' '.join('%04x' % x for x in words) + '\n')
    # the init words no panel control moves are constants of the lift
    # one tables file for every build (tools/bq_arm.py): the exact build reads MODEL_init and the BQ_PANEL block, the
    # host builds MODEL_host_init and BQ_PANEL_HOST
    tab = open(os.path.join(EX, 'bq_%s_tables.h' % model)).read()
    body = tab[tab.index('%s_%sinit[0x70] = {' % (model, 'host_' if host else '')):]
    body = body[body.index('{') + 1:body.index('};')]
    init = [float(x.rstrip('f')) for x in re.findall(r'-?[0-9.]+e[+-]\d+f', body)]
    assert len(init) == 0x70, len(init)
    blk = tab[tab.index('#ifdef %s\n' % ('BQ_PANEL_HOST' if host else 'BQ_PANEL')):]
    blk = blk[:blk.index('#endif')]
    panel = [(int(a, 16), nm, int(i)) for a, nm, i in re.findall(r'case 0x([0-9a-f]+): return s1_param\((\w+), (\d+)\)', blk)]
    moved = {a for a, _, _ in panel}
    consts = os.path.join(work, '%s%s_consts.txt' % (model, sfx))
    with open(consts, 'w') as f:
        for a, v in enumerate(init):
            if a not in moved:
                f.write('C%03X %r\n' % (a, v))
    # the panel's parameter names (the tables' names): the first names file, so that dsplift_c.auto_names names the
    # cells by the controls they come from (README.md §2)
    params = os.path.join(work, '%s%s_params.txt' % (model, sfx))
    with open(params, 'w') as f:
        f.write('# written by tools/make_voice.py from bq_%s_tables.h: the panel words\n' % model)
        for a, nm, i in panel:
            f.write('param C%03X %s%s knob\n' % (a, nm, '' if i == 0 else '_%d' % (i + 1)))
    out = os.path.join(EX, 'bq_%s%s_voice.h' % (model, sfx))
    tmp = out + '.tmp'
    seed = os.path.join(EX, 'bq_names_host.txt' if host else 'bq_names.txt')
    if host and osc2x:
        lift = ['--cell-in', HOST_CELLS + ',' + OSC2X_CELLS]
    elif host:
        lift = ['--cell-in', HOST_CELLS]
    else:
        lift = ['--split', str(split), '--zero-cells', '41b', '--import-ranges',
                os.path.join(EX, 'bq_import_ranges.txt')]
    lift = ['--bq2', '--kernel', '--bank1-ring', '--kout', '203a,200a,200f,200b' if osc2x else '203a'] + lift + \
        ['--consts', consts, '%s%s_voice=%s@0x%04x' % (model, sfx, hexf, 0x1900 if host else 0x1000)]
    tools = os.path.join(ROOT, 'sdk', 's1cc', 'tools')
    # the names: step 1 (--names) writes bq_MODEL*_names.txt from the seeds (the panel's parameters, bq_names*.txt
    # with bq_names_cells.txt) and dsplift_names.py's automatic cell names; step 2 (the lift) only reads it, so it can
    # be read and corrected by hand in between (README.md §2.1)
    gen = os.path.join(EX, 'bq_%s%s_names.txt' % (model, sfx))
    if '--names' in sys.argv:
        subprocess.check_call([sys.executable, os.path.join(tools, 'dsplift_names.py'), gen, '--names', params,
                               '--names', seed] + lift)
        return
    if not os.path.exists(gen):
        raise SystemExit('%s: missing: run make_voice.py with --names first (README.md §2.1)' % gen)
    subprocess.check_call([sys.executable, os.path.join(tools, 'dsplift.py'), tmp, '--names', gen] + lift)
    head = ('/* bq_%s%s_voice.h: the %s voice (Boutique 1.21 PX-11:Appli, DSP chip 0: %svoice A 0x1900, OSC1 '
            'variant %d, OSC2 variant %d) as C, the model\'s %d immediate patches and %d rate conversions (44.1 -> 96 '
            'kHz) applied%s. Written by tools/make_voice.py with sdk/s1cc/tools/dsplift.py; do not edit: README.md §2. */\n'
            % (model, sfx, model.upper(), '' if host else 'pre 0x1000, ', v1, v2, total, nrate,
               '; envelopes and LFO from the host (--host, README.md §8)' if host else ''))
    src = open(tmp).read()
    os.remove(tmp)
    if not host:
        src = attack_fix(src)
    src = micro_fix(src)
    if armc:
        src = arm_fix(src)
    open(out, 'w').write(head + src[src.index('\n') + 1:])
    print('%s: %d words, %d patches, %d rate patches' % (out, len(words), total, nrate))


if __name__ == '__main__':
    main()
