"""Eight SAW voices with note gate in the main program of DSP4, all our own asm (asm/asm.py). Nothing of Roland.

Per voice k (0..7), like make_poly.py without envelope and LFO:
    pitch  bank 5 id 0x70+k ([A070+k]): (note - 12) / 12
    gate   bank 5 id 0x78+k ([A078+k]): 1 while the key is held
    phase  ring state at 0670+4k: stored at S, read at S+1 next sample
Output: (sum of saw_k * gate_k) / 8 in IOMD 6020 (canal 1) and 6021 (canal 0).

The ids of the pitches (0x70..0x77) and of the gates (0x78..0x7F) do not overlap; the engine's note_gate
writes DSP4 id 0x22 only, and the model writes no bank-5 id of DSP4 (checked in engine/build/model.h).

    python3 make_solo.py          -> solo.s, solo.hex
    python3 make_solo.py --sim    -> voice 6 alone: 880 Hz only while its gate is on
"""
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
import asm  # noqa: E402

NV = 8
PITCH = [0xA070 + k for k in range(NV)]
GATE = [0xA078 + k for k in range(NV)]
PHASE = [0x0670 + 4 * k for k in range(NV)]
OUT, OUT2 = 0x6020, 0x6021
K = 2 * 440.0 / 96000.0


def voice(k):
    """one voice: r7 += saw_k * gate_k (r5 = 0, r6 = 1 are constants; r7 is the sum)"""
    L = [f'ld r0, [{PITCH[k]:04x}]', f'ld r4, [{GATE[k]:04x}]',
         'add r0, #-4.75:32',
         f'mul r2, r0, #{math.log(2) / 16!r}:32',
         'mov r3, #0.041666666666666664:32',
         f'ymacd r3, r2, #{1 / 6!r}:32', f'ymacd r3, r2, #{0.5!r}:32',
         'ymacd r3, r2, #1.0:32', 'ymacd r3, r2, #1.0:32']
    L += ['mul r3, r3, r3'] * 4
    L += [f'ld r1, [{PHASE[k] + 1:04x}]', 'mov r4, r4',
          f'mul r3, r3, #{K!r}:32', 'addxy r1, r3, r1', 'addxy r2, r1, #-1.0:32', 'max r2, r2, r5']
    for _ in range(4):
        L += ['mul r2, r2, #256.0:32', 'min r2, r2, r6']
    L += ['mul r2, r2, #2.0:32', 'subxy r1, r2, r1', f'st r1, [{PHASE[k]:04x}]']
    L += ['mul r1, r1, r4', 'addxy r7, r7, r1']
    return L


def source():
    L = ['; solo: eight SAW voices with note gate, DSP4 main (our own, make_solo.py)',
         'mov r5, #0.0:32', 'mov r6, #1.0:32', 'mov r7, #0.0:32']
    for k in range(NV):
        L += voice(k)
    L += [f'mul r7, r7, #{1 / NV!r}:32', f'st r7, [{OUT:04x}]', f'st r7, [{OUT2:04x}]']
    return '\n'.join(L) + '\n'


def words(text):
    r = asm.Assembler().assemble(text)
    if not r.ok:
        raise SystemExit(r.errors)
    return [0x0A02, 0xFF70] + r.words + [0x0B02, 0x00F8, 0x0001], r


def simulate(w, pitch, gate, n):
    import dspdis
    import dspisa
    ins = [i for _, i in dspdis.instructions(w)]
    mem = {}
    out = []
    for t in range(n):
        for k in range(NV):
            mem[PITCH[k]] = pitch[k]
            mem[GATE[k]] = gate[k](t)
            mem[PHASE[k] + 1] = mem.get(PHASE[k], 0.0)
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
        '# solo: eight SAW voices with note gate, DSP4 main (make_solo.py)\n' + '\n'.join('%04x' % x for x in w) + '\n')
    print(r.summary(), 'words', len(w))
    if '--sim' in sys.argv:
        pitch = [4.75] * NV
        pitch[6] = 5.75                                                   # voice 6: one octave up (880 Hz)
        gates = [lambda t: 0.0] * NV                                      # noqa: E731
        gates[6] = lambda t: 1.0 if 2000 <= t < 6000 else 0.0             # noqa: E731
        out = simulate(w, pitch, gates, 8000)
        cross = sum(1 for t in range(2100, 5900) if out[t - 1] < 0 <= out[t])
        print('voice 6 alone (880 Hz expected): crossings', cross, '-> %.0f Hz' % (cross * 96000 / 3800))
        print('silent before gate:', round(max(abs(v) for v in out[:2000]), 6),
              ' after release:', round(max(abs(v) for v in out[6100:8000]), 6))
