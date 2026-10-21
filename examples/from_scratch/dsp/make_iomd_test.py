"""Hardware test of the slave's output path (docs/MINI_PLUGOUT.md §4).

Writes three fixed-pitch sawtooths, each sample, to three candidate IOMD words of the slave's output bus:
  60B0 = 440 Hz (IOMD0 B0, DSP5 store), 60B1 = 660 Hz (IOMD0 B1), 6020 = 880 Hz (IOMD0 20, DSP4 store).
The program is our own asm (asm/asm.py), registered in the DSP5 DCO1 slot (engine.c -DIOMDTEST).

    python3 make_iomd_test.py          -> iomd_test.s (source), iomd_test.hex (words, execution order)
    python3 make_iomd_test.py --sim    -> also runs the words in re/dspisa.py for 300 samples and prints the periods
"""
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
import asm  # noqa: E402

VOICES = [  # (state address, output address, pitch p with f = 440 * 2^(p - 4.75))
    (0x0670, 0x60B0, 4.75),
    (0x0680, 0x60B1, 4.75 + math.log2(660 / 440)),
    (0x0690, 0x6020, 4.75 + math.log2(880 / 440)),
]


def source():
    L = ['; IOMD output test for the slave DSP5 (own asm, see make_iomd_test.py)',
         'mov r5, #0.0:32', 'mov r6, #1.0:32']
    for n, (st, out, p) in enumerate(VOICES):
        k = 2 * 440.0 / 96000.0
        L += [f'; voice {n}: f = 440 * 2^(p - 4.75)',
              f'ld r1, [{st + 1:04x}]',
              f'mov r0, #{p!r}:32', 'add r0, #-4.75:32',
              f'mul r2, r0, #{math.log(2) / 16!r}:32',
              'mov r3, #0.041666666666666664:32',
              f'ymacd r3, r2, #{1 / 6!r}:32', f'ymacd r3, r2, #{0.5!r}:32',
              'ymacd r3, r2, #1.0:32', 'ymacd r3, r2, #1.0:32']
        L += ['mul r3, r3, r3'] * 4
        L += [f'mul r3, r3, #{k!r}:32', 'addxy r1, r3, r1', 'addxy r2, r1, #-1.0:32', 'max r2, r2, r5']
        for _ in range(4):
            L += ['mul r2, r2, #256.0:32', 'min r2, r2, r6']
        L += ['mul r2, r2, #2.0:32', 'subxy r1, r2, r1', f'st r1, [{st:04x}]', f'st r1, [{out:04x}]']
    return '\n'.join(L) + '\n'


def words(text):
    r = asm.Assembler().assemble(text)
    if not r.ok:
        raise SystemExit(r.errors)
    return [0x0A02, 0xFF70] + r.words + [0x0B02, 0x00F8, 0x0001], r


def simulate(w, n=300):
    import dspdis
    import dspisa
    ins = [i for _, i in dspdis.instructions(w)]
    mem = {}
    ring = [st for st, _, _ in VOICES]
    out = {o: [] for _, o, _ in VOICES}
    for _ in range(n):
        for st in ring:
            if st in mem:
                mem[st + 1] = mem[st]
        regs = [0.0] * 8
        for x in ins:
            dspisa.JUMP = None
            regs, mem, _ = dspisa.run(list(x), regs, mem)
        for _, o, _ in VOICES:
            out[o].append(mem.get(o, 0.0))
    return out


if __name__ == '__main__':
    text = source()
    w, r = words(text)
    open(os.path.join(HERE, 'iomd_test.s'), 'w').write(text)
    open(os.path.join(HERE, 'iomd_test.hex'), 'w').write(
        '# IOMD test program, execution order (make_iomd_test.py)\n' + '\n'.join('%04x' % x for x in w) + '\n')
    print(r.summary(), 'words', len(w))
    if '--sim' in sys.argv:
        out = simulate(w)
        for _, o, p in VOICES:
            s = out[o][-200:]
            zc = sum(1 for a, b in zip(s, s[1:]) if a < 0 <= b)     # rising zero-crossings per 200 samples
            print('%#06x p=%.3f  min %.3f max %.3f  wraps/200 samples %d (expect %.1f)'
                  % (o, p, min(s), max(s), zc, 200 * 440 * 2 ** (p - 4.75) / 96000))
