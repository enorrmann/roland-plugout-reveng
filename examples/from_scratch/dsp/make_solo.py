"""One SAW oscillator with a note gate in the main program of DSP4, all our own asm (asm/asm.py). Nothing of Roland.

    pitch  bank 5 id 0x70 of DSP4 ([A070]): (note - 12) / 12, the ARM writes it on note on
    gate   bank 5 id 0x71 of DSP4 ([A071]): 1 while the key is held, 0 on note off (no envelope)
    output the saw times the gate, in IOMD 6020 (canal 1) and 6021 (canal 0) (docs/WAVE_BUS.md)
    phase  ring state at 0670: stored at S, read at S+1 in the next sample (docs/DSP_ISA_SPEC.md §9)

    python3 make_solo.py          -> solo.s, solo.hex
    python3 make_solo.py --sim    -> run the program in re/dspisa.py: 440 Hz at pitch 4.75, gate pulse
"""
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
import asm  # noqa: E402

PITCH, GATE = 0xA070, 0xA071
PHASE = 0x0670
OUT, OUT2 = 0x6020, 0x6021
K = 2 * 440.0 / 96000.0


def source():
    L = ['; solo: SAW with note gate, DSP4 main program (our own, make_solo.py)',
         'mov r5, #0.0:32', 'mov r6, #1.0:32',
         f'ld r0, [{PITCH:04x}]', f'ld r4, [{GATE:04x}]',
         'add r0, #-4.75:32',
         f'mul r2, r0, #{math.log(2) / 16!r}:32',
         'mov r3, #0.041666666666666664:32',
         f'ymacd r3, r2, #{1 / 6!r}:32', f'ymacd r3, r2, #{0.5!r}:32',
         'ymacd r3, r2, #1.0:32', 'ymacd r3, r2, #1.0:32']
    L += ['mul r3, r3, r3'] * 4
    L += [f'ld r1, [{PHASE + 1:04x}]', 'mov r4, r4',
          f'mul r3, r3, #{K!r}:32', 'addxy r1, r3, r1', 'addxy r2, r1, #-1.0:32', 'max r2, r2, r5']
    for _ in range(4):
        L += ['mul r2, r2, #256.0:32', 'min r2, r2, r6']
    L += ['mul r2, r2, #2.0:32', 'subxy r1, r2, r1', f'st r1, [{PHASE:04x}]']
    L += ['mul r1, r1, r4', f'st r1, [{OUT:04x}]', f'st r1, [{OUT2:04x}]']
    return '\n'.join(L) + '\n'


def words(text):
    r = asm.Assembler().assemble(text)
    if not r.ok:
        raise SystemExit(r.errors)
    return [0x0A02, 0xFF70] + r.words + [0x0B02, 0x00F8, 0x0001], r


def simulate(w, gate, pitch, n):
    import dspdis
    import dspisa
    ins = [i for _, i in dspdis.instructions(w)]
    mem = {PITCH: pitch}
    out = []
    for t in range(n):
        mem[GATE] = gate(t)
        mem[PHASE + 1] = mem.get(PHASE, 0.0)
        regs = [0.0] * 8
        for x in ins:
            dspisa.JUMP = None
            regs, mem, _ = dspisa.run(list(x), regs, mem)
        out.append(mem.get(OUT, 0.0))
    return out


if __name__ == '__main__':
    text = source()
    w, r = words(text)
    open(os.path.join(HERE, 'solo.s'), 'w').write(text)
    open(os.path.join(HERE, 'solo.hex'), 'w').write(
        '# solo: SAW with note gate, DSP4 main (make_solo.py)\n' + '\n'.join('%04x' % x for x in w) + '\n')
    print(r.summary(), 'words', len(w))
    if '--sim' in sys.argv:
        gate = lambda t: 1.0 if 2000 <= t < 6000 else 0.0  # noqa: E731
        out = simulate(w, gate, 4.75, 8000)
        print('gate off at 0:', round(out[0], 4), ' gate on at 2001:', round(out[2001], 4), ' gate off at 6001:', round(out[6001], 4))
        cross = sum(1 for t in range(2100, 5900) if out[t - 1] < 0 <= out[t])
        print('upward zero crossings with gate on:', cross, '->', round(cross * 96000 / 3800, 1), 'Hz (expect ~440)')
