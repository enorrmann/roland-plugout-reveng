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
   output, to the post program) --names bq_names.txt --consts (the init words no panel control moves).
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


def main():
    model, v1, v2 = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
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
    words = prog['pre'] + prog['voice'][:cut] + prog['osc2'] + prog['osc1'] + prog['voice'][cut:]
    split = len(instr_bounds(prog['pre'])) - 1 + VOICE_SPLIT + len(instr_bounds(prog['osc2'])) - 1
    work = os.path.join(ROOT, 'work', 'bq1')
    hexf = os.path.join(work, '%s_voice.hex' % model)
    open(hexf, 'w').write(' '.join('%04x' % x for x in words) + '\n')
    # the init words no panel control moves are constants of the lift
    tab = open(os.path.join(EX, 'bq_%s_tables.h' % model)).read()
    body = tab[tab.index('%s_init[0x70] = {' % model):]
    body = body[body.index('{') + 1:body.index('};')]
    init = [float(x.rstrip('f')) for x in re.findall(r'-?[0-9.]+e[+-]\d+f', body)]
    assert len(init) == 0x70, len(init)
    moved = {int(a, 16) for a in re.findall(r'0x([0-9a-f]+)', ' '.join(re.findall(r'_addr\[\d+\] = \{([^}]*)\}', tab)))}
    consts = os.path.join(work, '%s_consts.txt' % model)
    with open(consts, 'w') as f:
        for a, v in enumerate(init):
            if a not in moved:
                f.write('C%03X %r\n' % (a, v))
    out = os.path.join(EX, 'bq_%s_voice.h' % model)
    tmp = out + '.tmp'
    subprocess.check_call([sys.executable, os.path.join(ROOT, 'sdk', 's1cc', 'tools', 'dsplift.py'), tmp, '--bq2',
                           '--kernel', '--split', str(split), '--zero-cells', '41b', '--import-ranges', os.path.join(EX, 'bq_import_ranges.txt'), '--bank1-ring', '--kout', '203a', '--names', os.path.join(EX, 'bq_names.txt'),
                           '--consts', consts, '%s_voice=%s@0x1000' % (model, hexf)])
    head = ('/* bq_%s_voice.h: the %s voice (Boutique 1.21 PX-11:Appli, DSP chip 0: pre 0x1000, voice A 0x1900, OSC1 '
            'variant %d, OSC2 variant %d) as C, the model\'s %d immediate patches and %d rate conversions (44.1 -> 96 '
            'kHz) applied. Written by tools/make_voice.py with sdk/s1cc/tools/dsplift.py; do not edit: README.md §2. */\n'
            % (model, model.upper(), v1, v2, total, nrate))
    src = open(tmp).read()
    os.remove(tmp)
    open(out, 'w').write(head + src[src.index('\n') + 1:])
    print('%s: %d words, %d patches, %d rate patches' % (out, len(words), total, nrate))


if __name__ == '__main__':
    main()
