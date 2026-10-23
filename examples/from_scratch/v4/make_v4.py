"""v4: our own MAIN0-MAIN5 with the data flow found in docs/log/VOICE_ALLOCATION_LOG.md §6 (no Roland bytes).

Changes against v3 (make_v3.py), one per question of the study:
  A / D  the DCO2 slot writes [2221] / [2229]; bank 1 is a ring, so the main program reads them one sample later at
         [2222] / [222A] (as Roland's MAIN2 reads [2220]-[222A]). v3 read [2221] / [2229]: stale words.
  A / G  MXM keeps the word number: DSP2 Z 404 / 405 arrive at DSP4 M 004 / 005 (v3 read M 005 / 006).
  B      voices 2 and 3 go from DSP5 to DSP4 through the pair window of bank 5: DSP5 stores the gated voices in
         [A400] / [A401], DSP4 adds them (Roland: MAIN5 stores A400-A403, MAIN4 loads them). 60B0 is not audible.
  C      DSP4 also adds the IOMD inputs 6000-6003 (Roland's MAIN4 mixes them; hypothesis: slave 1 arrives there)
         and writes its four voices to 6060-6063, as Roland's MAIN4 does (6060 = voice 0 ... 6063 = voice 3).

Voices of one slave: 0, 1 on DSP0 -> DSP2 -> DSP4; 2, 3 on DSP1 -> DSP3 -> DSP5 -> (A400/A401) -> DSP4.
    DSP0: pitch ids 0x21 / 0x33 -> Z 404 / 405                      (DSP2 M 004 / 005)
    DSP1: pitch ids 0x21 / 0x33 -> Z 400 / 401                      (DSP3 O 200 / 201)
    DSP2: M 004 / 005 -> [2207] / [2214]; [2222] / [222A] -> Z 404 / 405   (DSP4 M 004 / 005)
    DSP3: O 200 / 201 -> [2207] / [2214]; [2222] / [222A] -> Z 400 / 401   (DSP5 O 200 / 201)
    DSP4: v0 = M004 * g(0x2A), v1 = M005 * g(0x39); out = v0 + v1 + A400 + A401 + 6000..6003 -> 6020, 6021;
          v0, v1, A400, A401 -> 6060..6063
    DSP5: O200 * g(0x29) -> A400, O201 * g(0x38) -> A401

    python3 make_v4.py           -> v4m0..v4m5 .s / .hex (the DCO slots stay v3's: our saw, our null)
    python3 make_v4.py --no-ring -> v4bm0..v4bm5: the same, but DSP2/DSP3 read [2221] / [2229] (fallback if the
                                    bank-1 ring of question D is wrong: then v4 is silent and v4b is not)
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'v3'))
import make_v3 as v3  # noqa: E402

PITCH_A, PITCH_B = v3.PITCH_A, v3.PITCH_B
GATE = v3.GATE
DCO_IN = v3.DCO_IN
DCO_READ = (0x2222, 0x222A)                # slot outputs [2221] / [2229], one sample later (bank 1 ring)
PAIR = (0xA400, 0xA401)                    # bank 5 pair window: DSP5 -> DSP4
LINK_IN = (0x6000, 0x6001, 0x6002, 0x6003)  # IOMD inputs mixed by DSP4
VOICE_OUT = (0x6060, 0x6061, 0x6062, 0x6063)
IOMD_A, IOMD_B = v3.IOMD_A, v3.IOMD_B


def prog_dco_pair(name, desc, pitch_in, out_pair, read=DCO_READ):
    L = [f'; {name}: {desc}']
    L += [f'ld r1, [{pitch_in[0]:04x}]', 'mov r4, r4', f'st r1, [{DCO_IN[0]:04x}]']
    L += [f'ld r1, [{pitch_in[1]:04x}]', 'mov r4, r4', f'st r1, [{DCO_IN[1]:04x}]']
    L += [f'ld r1, [{read[0]:04x}]', 'mov r4, r4', f'st r1, [{out_pair[0]:04x}]']
    L += [f'ld r1, [{read[1]:04x}]', 'mov r4, r4', f'st r1, [{out_pair[1]:04x}]']
    return L


def prog_dsp4(name, desc):
    """v0 = M004 * g0, v1 = M005 * g1; out = v0 + v1 + A400 + A401 + IOMD 6000..6003"""
    g0, g1 = GATE[4]
    L = [f'; {name}: {desc}']
    L += [f'ld r4, [{g0:04x}]', 'ld r1, [8004]', 'mov r4, r4', 'mul r1, r1, r4', f'st r1, [{VOICE_OUT[0]:04x}]']
    L += [f'ld r4, [{g1:04x}]', 'ld r2, [8005]', 'mov r4, r4', 'mul r2, r2, r4', f'st r2, [{VOICE_OUT[1]:04x}]']
    L += ['addxy r1, r1, r2']
    for k, a in enumerate(PAIR):
        L += [f'ld r2, [{a:04x}]', 'mov r4, r4', f'st r2, [{VOICE_OUT[2 + k]:04x}]', 'addxy r1, r1, r2']
    for a in LINK_IN:
        L += [f'ld r2, [{a:04x}]', 'mov r4, r4', 'addxy r1, r1, r2']
    L += [f'st r1, [{IOMD_A:04x}]', f'st r1, [{IOMD_B:04x}]']
    return L


def prog_dsp5(name, desc):
    """O200 * g2 -> A400, O201 * g3 -> A401 (one bank-5 store per instruction)"""
    g2, g3 = GATE[5]
    L = [f'; {name}: {desc}']
    L += [f'ld r4, [{g2:04x}]', 'ld r1, [8200]', 'mov r4, r4', 'mul r1, r1, r4', f'st r1, [{PAIR[0]:04x}]']
    L += [f'ld r4, [{g3:04x}]', 'ld r2, [8201]', 'mov r4, r4', 'mul r2, r2, r4', f'st r2, [{PAIR[1]:04x}]']
    return L


def sources(ring=True):
    """the six programs as assembler text, keyed by DSP; ring=False: the v4b fallback (slot outputs read as written)"""
    tag, rd = ('v4', DCO_READ) if ring else ('v4b', v3.DCO_OUT)
    progs = {
        0: v3.prog_pitch(f'{tag}m0', 'voices 0 and 1 pitch -> MXM Z 404, 405 (DSP2 M 004, 005)', 0x8404, 0x8405),
        1: v3.prog_pitch(f'{tag}m1', 'voices 2 and 3 pitch -> MXM Z 400, 401 (DSP3 O 200, 201)', 0x8400, 0x8401),
        2: prog_dco_pair(f'{tag}m2', f'DCO2 pitch from M 004/005; saw [{rd[0]:04x}]/[{rd[1]:04x}] -> Z 404/405 '
                         '(DSP4 M 004/005)', (0x8004, 0x8005), (0x8404, 0x8405), rd),
        3: prog_dco_pair(f'{tag}m3', f'DCO2 pitch from O 200/201; saw [{rd[0]:04x}]/[{rd[1]:04x}] -> Z 400/401 '
                         '(DSP5 O 200/201)', (0x8200, 0x8201), (0x8400, 0x8401), rd),
        4: prog_dsp4(f'{tag}m4', 'voices 0,1 gated + A400/A401 (DSP5) + IOMD 6000-6003 -> 6020/6021; voices -> 6060-6063'),
        5: prog_dsp5(f'{tag}m5', 'voices 2,3 gated -> bank 5 pair window A400/A401 (DSP4)'),
    }
    return {k: '\n'.join(v) + '\n' for k, v in progs.items()}


if __name__ == '__main__':
    ring = '--no-ring' not in sys.argv
    for k, text in sources(ring).items():
        name = ('v4m%d' if ring else 'v4bm%d') % k
        open(os.path.join(HERE, name + '.s'), 'w').write(text)
        w, r = v3.words(text)
        open(os.path.join(HERE, name + '.hex'), 'w').write(
            '# %s, own main program (make_v4.py)\n' % name + '\n'.join('%04x' % x for x in w) + '\n')
        print(name, r.summary(), 'words', len(w))
