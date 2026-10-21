"""Mono voice for the DSP5 DCO1 slot, all our own asm (asm/asm.py), no Roland code.

    saw with pitch from the ARM (bank 5 id 0x21, read [A021]) and gate (bank 5 id 0x22, [A022]):
      envelope  AR: rising toward the gate with rate KA, falling with KR (branch free, min/max)
      LFO       sawtooth at LFO_HZ, vibrato of DEPTH (pitch units: 1 = 12 semitones)
      VCA       output = saw * envelope, stored in IOMD 6020 (the word that reaches the audio output)

    python3 make_voice.py          -> voice.s (source) and voice.hex (words, execution order)
    python3 make_voice.py --sim    -> run the words in re/dspisa.py with a gate pulse and print the result
"""
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
import asm  # noqa: E402

PITCH, GATE = 0xA021, 0xA022              # bank 5 ids written by the ARM
ENV, LFO, PHASE = 0x0690, 0x06A0, 0x0670  # ring state (store at S, read at S + 1 next sample)
OUT = 0x6020                              # IOMD0 word that reaches the output (slave 0)
OUT2 = 0x6021                             # second output word: canal 0 (docs/WAVE_BUS.md, 2026-10-21)
KA, KR = 0.01, 0.0002                     # attack / release rate per sample
LFO_HZ, DEPTH = 5.5, 0.02
K = 2 * 440.0 / 96000.0


def wrap(reg, tmp):
    """reg in [-1, 1) after reg += inc: reg -= 2 * max(reg - 1, 0) (saturating steps, no branches)"""
    L = [f'addxy r{tmp}, r{reg}, #-1.0:32', f'max r{tmp}, r{tmp}, r5']
    L += [f'mul r{tmp}, r{tmp}, #256.0:32', f'min r{tmp}, r{tmp}, r6'] * 4
    L += [f'mul r{tmp}, r{tmp}, #2.0:32', f'subxy r{reg}, r{tmp}, r{reg}']
    return L


def source():
    L = ['; mono voice, DSP5 DCO1 slot. Own asm: see make_voice.py',
         'mov r5, #0.0:32', 'mov r6, #1.0:32']
    # envelope: e' = e + (g - e) * k, k = KR + s * (KA - KR), s = 1 while g > e
    if os.environ.get('DRONE'):                      # diagnostic build: envelope fixed at 1, no gate
        L += ['mov r7, #1.0:32']
    else:
        L += [f'ld r7, [{ENV + 1:04x}]', f'ld r4, [{GATE:04x}]', 'mov r4, r4',
              'subxy r1, r7, r4',                        # r1 = g - e
              'mul r2, r1, #256.0:32', 'mul r2, r2, #256.0:32', 'max r2, r2, r5', 'min r2, r2, r6',  # s = [d > 1.5e-5]
              f'mul r2, r2, #{KA - KR!r}:32', f'add r2, #{KR!r}:32',
              'mul r1, r1, r2', 'addxy r7, r7, r1',       # r7 = envelope (kept for the VCA)
              f'st r7, [{ENV:04x}]']
    # LFO: r3 = saw at LFO_HZ
    L += [f'ld r3, [{LFO + 1:04x}]', 'mov r4, r4',
          f'addxy r3, r3, #{2 * LFO_HZ / 96000!r}:32'] + wrap(3, 2) + [f'st r3, [{LFO:04x}]']
    # pitch: p + DEPTH * lfo - 4.75
    L += [f'ld r0, [{PITCH:04x}]', 'mov r4, r4',
          f'mad r0, r3, #{DEPTH!r}:32', 'add r0, #-4.75:32',
          f'mul r2, r0, #{math.log(2) / 16!r}:32',
          'mov r3, #0.041666666666666664:32',
          f'ymacd r3, r2, #{1 / 6!r}:32', f'ymacd r3, r2, #{0.5!r}:32',
          'ymacd r3, r2, #1.0:32', 'ymacd r3, r2, #1.0:32']
    L += ['mul r3, r3, r3'] * 4
    # saw (same algorithm as saw_dco2.s, voice A)
    L += [f'ld r1, [{PHASE + 1:04x}]', 'mov r4, r4',
          f'mul r3, r3, #{K!r}:32', 'addxy r1, r3, r1', 'addxy r2, r1, #-1.0:32', 'max r2, r2, r5']
    for _ in range(4):
        L += ['mul r2, r2, #256.0:32', 'min r2, r2, r6']
    L += ['mul r2, r2, #2.0:32', 'subxy r1, r2, r1', f'st r1, [{PHASE:04x}]']
    # VCA and output
    L += ['mul r1, r1, r7', f'st r1, [{OUT:04x}]', f'st r1, [{OUT2:04x}]']   # OUT2: second route, tested per slave
    if os.environ.get('ALLOUT'):           # diagnostic: the same signal in every IOMD word (slave 1 search)
        # ALLOUT=1 -> 6020-60FF; ALLOUT=6000-601F -> that range (hex, inclusive)
        if '-' in os.environ['ALLOUT']:
            lo, hi = (int(t, 16) for t in os.environ['ALLOUT'].split('-'))
        else:
            lo, hi = 0x6020, 0x60FF
        L +=[f'st r1, [{w:04x}]' for w in range(lo, hi + 1) if w not in (OUT, OUT2)]
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
    out, env, lfo = [], [], []
    for t in range(n):
        mem[GATE] = gate(t)
        for st in (ENV, LFO, PHASE):
            if st in mem:
                mem[st + 1] = mem[st]
        regs = [0.0] * 8
        for x in ins:
            dspisa.JUMP = None
            regs, mem, _ = dspisa.run(list(x), regs, mem)
        out.append(mem.get(OUT, 0.0))
        env.append(mem.get(ENV, 0.0))
        lfo.append(mem.get(LFO, 0.0))
    return out, env, lfo


if __name__ == '__main__':
    text = source()
    w, r = words(text)
    open(os.path.join(HERE, 'voice.s'), 'w').write(text)
    open(os.path.join(HERE, 'voice.hex'), 'w').write(
        '# mono voice, DSP5 DCO1 slot (make_voice.py)\n' + '\n'.join('%04x' % x for x in w) + '\n')
    print(r.summary(), 'words', len(w))
    if '--sim' in sys.argv:
        pitch = 4.75                                    # note 69 = 440 Hz
        gate = lambda t: 1.0 if 100 <= t < 2000 else 0.0  # noqa: E731
        out, env, lfo = simulate(w, gate, pitch, 3000)
        for t in (0, 99, 101, 110, 150, 300, 1000, 1999, 2000, 2100, 2500):
            print('t=%4d gate=%d env=%.4f lfo=%+.3f out=%+.4f' % (t, gate(t), env[t], lfo[t], out[t]))
