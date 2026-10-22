"""Our own main programs MAIN0-MAIN5 with Roland's voice layout (docs/VOICE_ALLOCATION.md section 1), no Roland bytes.

Voices of one slave: 0 and 1 on DSP0 / DSP2 / DSP4, voices 2 and 3 on DSP1 / DSP3 / DSP5.
    DSP0 main: pitch of voice 0 (bank 5 id 0x21) -> MXM Z 0404, voice 1 (id 0x33) -> Z 0405   (-> DSP2 M 004, 005)
    DSP1 main: pitch of voice 2 (id 0x21) -> Z 0400, voice 3 (id 0x33) -> Z 0401             (-> DSP3 O 200, 201)
    DSP2 main: M 004 -> DCO2 pitch [2207] (voice A), M 005 -> [2214] (voice B);
               DCO2 slot (saw_dco2): [2221] / [2229] -> Z 0404 / 0405                         (-> DSP4 M 005, 006)
    DSP3 main: O 200 -> [2207], O 201 -> [2214]; [2221] / [2229] -> Z 0401 / 0402             (-> DSP5 O 201, 202)
    DSP4 main: gate of voice 0 = bank 5 id 0x2A, voice 1 = id 0x39 (engine.c note_gate, unit 4):
               (M 005 * g0 + M 006 * g1) -> IOMD 6020 and 6021
    DSP5 main: gate of voice 2 = bank 5 id 0x29, voice 3 = id 0x38 (unit 5):
               (O 201 * g2 + O 202 * g3) -> IOMD 60B0 and 60B1

The DCO2 slot program is re/dspexp/own/saw_dco2 (ours). Gate ids and pitch ids are the ones engine.c writes.

    python3 make_v3.py          -> v3m0..v3m5 .s/.hex
    python3 make_v3.py --sim    -> simulate the chain: voice 2 (DSP1/3/5) at 880 Hz while its gate is on, others silent
"""
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
import asm  # noqa: E402

PITCH_A, PITCH_B = 0xA021, 0xA033          # bank 5 of DSP0 / DSP1: voice 0 / 2 and 1 / 3 (engine.c note_pitch)
GATE = {4: (0xA02A, 0xA039), 5: (0xA029, 0xA038)}   # bank 5 of DSP4 / DSP5: voices (0,1) / (2,3) (engine.c note_gate)
DCO_IN = (0x2207, 0x2214)                  # DCO2 slot pitch inputs, voice A and B (saw_dco2)
DCO_OUT = (0x2221, 0x2229)                 # DCO2 slot outputs, voice A and B (saw_dco2)
K_PHASE = 0x0670                           # DCO2 ring state (saw_dco2), voice B at 0680
IOMD_A, IOMD_B = 0x6020, 0x6021            # DSP4 output (canal 1 / canal 0)
IOMD_C, IOMD_D = 0x60B0, 0x60B1            # DSP5 output


def prog_pitch(name, desc, out0, out1):
    """DSP0 / DSP1: the two pitches of this DSP's voices go to MXM Z (out0, out1)"""
    L = [f'; {name}: {desc}']
    L += [f'ld r1, [{PITCH_A:04x}]', 'mov r4, r4', f'st r1, [{out0:04x}]']
    L += [f'ld r1, [{PITCH_B:04x}]', 'mov r4, r4', f'st r1, [{out1:04x}]']
    return L


def prog_dco_pair(name, desc, pitch_in, out_pair):
    """DSP2 / DSP3: pitch from MXM (pitch_in) to the DCO2 slot inputs; the saw outputs to MXM (out_pair)"""
    L = [f'; {name}: {desc}']
    L += [f'ld r1, [{pitch_in[0]:04x}]', 'mov r4, r4', f'st r1, [{DCO_IN[0]:04x}]']
    L += [f'ld r1, [{pitch_in[1]:04x}]', 'mov r4, r4', f'st r1, [{DCO_IN[1]:04x}]']
    L += [f'ld r1, [{DCO_OUT[0]:04x}]', 'mov r4, r4', f'st r1, [{out_pair[0]:04x}]']
    L += [f'ld r1, [{DCO_OUT[1]:04x}]', 'mov r4, r4', f'st r1, [{out_pair[1]:04x}]']
    return L


def prog_mix(name, desc, audio, gates, outs):
    """DSP4 / DSP5: (audio0 * gate0 + audio1 * gate1) to two IOMD words"""
    L = [f'; {name}: {desc}']
    L += [f'ld r4, [{gates[0]:04x}]', f'ld r1, [{audio[0]:04x}]', 'mov r4, r4', 'mul r1, r1, r4']
    L += [f'ld r4, [{gates[1]:04x}]', f'ld r2, [{audio[1]:04x}]', 'mov r4, r4', 'mul r2, r2, r4']
    L += ['addxy r1, r1, r2', f'st r1, [{outs[0]:04x}]', f'st r1, [{outs[1]:04x}]']
    return L


def sources():
    """the six programs as assembler text, keyed by DSP"""
    progs = {
        0: prog_pitch('v3m0', 'voices 0 and 1 pitch -> MXM Z 404, 405 (DSP2 M 004, 005)', 0x8404, 0x8405),
        1: prog_pitch('v3m1', 'voices 2 and 3 pitch -> MXM Z 400, 401 (DSP3 O 200, 201)', 0x8400, 0x8401),
        2: prog_dco_pair('v3m2', 'DCO2 pitch from M 004/005; saw out -> Z 404/405 (DSP4 M 005/006)',
                         (0x8004, 0x8005), (0x8404, 0x8405)),
        3: prog_dco_pair('v3m3', 'DCO2 pitch from O 200/201; saw out -> Z 401/402 (DSP5 O 201/202)',
                         (0x8200, 0x8201), (0x8401, 0x8402)),
        4: prog_mix('v3m4', 'voices 0,1 gated mix -> IOMD 6020 / 6021', (0x8005, 0x8006), GATE[4],
                    (IOMD_A, IOMD_B)),
        5: prog_mix('v3m5', 'voices 2,3 gated mix -> IOMD 60B0 / 60B1', (0x8201, 0x8202), GATE[5],
                    (IOMD_C, IOMD_D)),
    }
    return {k: '\n'.join(v) + '\n' for k, v in progs.items()}


def words(text):
    r = asm.Assembler().assemble(text)
    if not r.ok:
        raise SystemExit(r.errors)
    return [0x0A02, 0xFF70] + r.words + [0x0B02, 0x00F8, 0x0001], r


def saw_dco2_words():
    """the DCO2 slot program of the own SAW (re/dspexp/own/saw_dco2.txt: 16-bit words, '#' lines are comments)"""
    path = os.path.join(ROOT, 'examples', 'from_scratch', 're', 'dspexp', 'own', 'saw_dco2.txt')
    out = []
    for line in open(path):
        if line.startswith('#'):
            continue
        out += [int(t, 16) for t in line.split()]
    return out


def simulate(progs, pitch, gate, n):
    """one sample: DSP0, DSP1, DSP2 (main, DCO2 saw), DSP3 (main, saw), DSP4, DSP5. MXM is one sample late, as on the
    device (docs/DSP_HARDWARE.md 8.2). pitch[v] for voices 0..3, gate[v](t). Returns the IOMD words per sample."""
    import dspdis
    import dspisa
    code = {}
    for k, text in progs.items():
        w, _ = words(text)
        code[k] = [i for _, i in dspdis.instructions(w)]
    saw_words = saw_dco2_words()
    code_saw = [i for _, i in dspdis.instructions(saw_words)]
    mem = {k: {} for k in range(6)}
    prev = {k: {} for k in range(6)}
    iomd = []
    for t in range(n):
        for v in range(2):                                    # bank 5: pitch of DSP0 / DSP1 voices
            mem[0][PITCH_A if v == 0 else PITCH_B] = pitch[v]
            mem[1][PITCH_A if v == 0 else PITCH_B] = pitch[2 + v]
        mem[4][GATE[4][0]], mem[4][GATE[4][1]] = gate[0](t), gate[1](t)
        mem[5][GATE[5][0]], mem[5][GATE[5][1]] = gate[2](t), gate[3](t)
        # MXM: each DSP reads what its partner wrote in the previous sample
        mem[2][0x8004], mem[2][0x8005] = prev[0].get(0x8404, 0.0), prev[0].get(0x8405, 0.0)
        mem[3][0x8200], mem[3][0x8201] = prev[1].get(0x8400, 0.0), prev[1].get(0x8401, 0.0)
        mem[4][0x8005], mem[4][0x8006] = prev[2].get(0x8404, 0.0), prev[2].get(0x8405, 0.0)
        mem[5][0x8201], mem[5][0x8202] = prev[3].get(0x8401, 0.0), prev[3].get(0x8402, 0.0)
        for d in range(6):
            m = mem[d]
            if d in (2, 3):
                for p in (0x0670, 0x0680):
                    m[p + 1] = m.get(p, 0.0)
            for prog in ([code[d], code_saw] if d in (2, 3) else [code[d]]):
                regs = [0.0] * 8
                for x in prog:
                    dspisa.JUMP = None
                    regs, m, _ = dspisa.run(list(x), regs, m)
            mem[d] = m
        prev = {d: dict(mem[d]) for d in range(6)}
        iomd.append((mem[4].get(IOMD_A, 0.0), mem[5].get(IOMD_C, 0.0)))
    return iomd


if __name__ == '__main__':
    progs = sources()
    for k, text in progs.items():
        name = 'v3m%d' % k
        open(os.path.join(HERE, name + '.s'), 'w').write(text)
        w, r = words(text)
        open(os.path.join(HERE, name + '.hex'), 'w').write(
            '# %s, own main program (make_v3.py)\n' % name + '\n'.join('%04x' % x for x in w) + '\n')
        print(name, r.summary(), 'words', len(w))
    if '--sim' in sys.argv:
        pitch = [4.75, 4.75, 5.75, 4.75]               # voice 2 one octave up (DSP1 -> DSP3 -> DSP5)
        gates = [lambda t: 0.0] * 4                    # noqa: E731
        gates[2] = lambda t: 1.0 if 2000 <= t < 6000 else 0.0   # noqa: E731
        out = simulate(progs, pitch, gates, 8000)
        seq = [o[1] for o in out]
        cross = sum(1 for t in range(2100, 5900) if seq[t - 1] < 0 <= seq[t])
        print('voice 2 alone (880 Hz expected): crossings', cross, '-> %.0f Hz' % (cross * 96000 / 3800))
        print('silent before gate:', round(max(abs(v) for v in seq[:2000]), 6),
              ' after release:', round(max(abs(v) for v in seq[6100:8000]), 6))
