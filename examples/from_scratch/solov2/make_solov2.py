"""Four SAW voices per slave with the Roland voice layout, all our own asm (asm/asm.py). Nothing of Roland.

    DSP2 (DCO1 slot): voices 0 and 1 of the slave. Pitch: bank 5 ids 0x21, 0x22 (ARM). Phase state at 0670 / 0674.
                      Outputs: MXM Z 0404 (voice 0) and Z 0405 (voice 1), read by DSP4 as M 0005 and M 0006.
    DSP3 (DCO1 slot): voices 2 and 3. Same program and ids as DSP2. Outputs: Z 0404 / 0405, read by DSP4 as O 0204 / 0205
                      (routing of docs/DSP_ARM_SIDE.md: Z 0x400+x of DSP n -> O 0x200+x of DSP n+1 for the odd DSP3 pair).
    DSP4 (main):      four gated voices, sum / 4 to IOMD 6020 (canal 1) and 6021 (canal 0). Gates: bank 5 ids 0x78..0x7B.

Roland's DSP0/1 (envelope, LFO) and DSP5 stay empty; DSP2/3 main programs are empty; DSP4 main is ours (docs/WAVE_BUS.md
section 4g: each component keeps the DSP and slot Roland gives it).

    python3 make_solov2.py          -> solov2_osc.s/.hex, solov2_gate.s/.hex
    python3 make_solov2.py --sim    -> voice 2 alone (DSP3) at 880 Hz while its gate is on; the others silent
"""
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
import asm  # noqa: E402

PITCH = [0xA021, 0xA022]                    # DSP2 / DSP3 bank 5 ids 0x21, 0x22 (ARM: note pitch of voices A, B)
PHASE = [0x0670, 0x0674]                    # DSP2 / DSP3 ring state: store at S, read at S+1 next sample
OUT = [0x8404, 0x8405]                      # Z words of DSP2 / DSP3 that DSP4 reads
IN = [0x8005, 0x8006, 0x8204, 0x8205]       # DSP4 M 005/006 (DSP2) and O 204/205 (DSP3)
GATE = [0xA078 + k for k in range(4)]       # DSP4 bank 5 ids 0x78..0x7B (ARM: gate of local voice k)
OUT_L, OUT_R = 0x6020, 0x6021               # IOMD words that reach the output (DSP4 owns IOMD0 0x20-0x24)
K = 2 * 440.0 / 96000.0


def osc(v):
    """saw of voice v (0 or 1) of this DSP; the saw goes to Z OUT[v]"""
    L = [f'ld r0, [{PITCH[v]:04x}]', 'mov r4, r4',
         'add r0, #-4.75:32',                                   # p - 4.75
         f'mul r2, r0, #{math.log(2) / 16!r}:32',               # x = (p - 4.75) ln2 / 16
         'mov r3, #0.041666666666666664:32',
         f'ymacd r3, r2, #{1 / 6!r}:32', f'ymacd r3, r2, #{0.5!r}:32',
         'ymacd r3, r2, #1.0:32', 'ymacd r3, r2, #1.0:32']     # r3 = 2^(x / 16) (Taylor)
    L += ['mul r3, r3, r3'] * 4                                  # r3 = 2^x
    L += [f'ld r1, [{PHASE[v] + 1:04x}]', 'mov r4, r4',
          f'mul r3, r3, #{K!r}:32', 'addxy r1, r3, r1', 'addxy r2, r1, #-1.0:32', 'max r2, r2, r5']
    for _ in range(4):
        L += ['mul r2, r2, #256.0:32', 'min r2, r2, r6']
    L += ['mul r2, r2, #2.0:32', 'subxy r1, r2, r1', f'st r1, [{PHASE[v]:04x}]']   # r1 in [-1, 1): the saw
    L += [f'st r1, [{OUT[v]:04x}]']
    return L


def osc_source():
    L = ['; solov2 SAW, DSP2 and DSP3 DCO1 slot: voices 0 and 1 of each DSP (make_solov2.py)',
         'mov r5, #0.0:32', 'mov r6, #1.0:32']
    L += osc(0) + osc(1)
    return '\n'.join(L) + '\n'


def gate_source():
    L = ['; solov2 DSP4 main: four gated voices (two from DSP2, two from DSP3), IOMD 6020 / 6021 (make_solov2.py)',
         'mov r7, #0.0:32']
    for k in range(4):
        L += [f'ld r4, [{GATE[k]:04x}]', f'ld r1, [{IN[k]:04x}]', 'mov r4, r4',
              'mul r1, r1, r4', 'addxy r7, r7, r1']
    L += ['mul r7, r7, #0.25:32', f'st r7, [{OUT_L:04x}]', f'st r7, [{OUT_R:04x}]']
    return '\n'.join(L) + '\n'


def words(text):
    r = asm.Assembler().assemble(text)
    if not r.ok:
        raise SystemExit(r.errors)
    return [0x0A02, 0xFF70] + r.words + [0x0B02, 0x00F8, 0x0001], r


def simulate(w_osc, w_gate, pitch, gate, n):
    """one sample = DSP2 program, DSP3 program, then DSP4 program; DSP4 reads what DSP2 / DSP3 stored in the
    previous sample (MXM double buffer, docs/DSP_HARDWARE.md 8.2). pitch[v], gate[v](t) for v in 0..3"""
    import dspdis
    import dspisa
    ins_osc = [i for _, i in dspdis.instructions(w_osc)]
    ins_gate = [i for _, i in dspdis.instructions(w_gate)]
    mem = [{PITCH[0]: pitch[0], PITCH[1]: pitch[1], PHASE[0] + 1: 0.0, PHASE[1] + 1: 0.0} for _ in range(2)]
    mem4 = {GATE[k]: 0.0 for k in range(4)}
    mem4.update({a: 0.0 for a in IN})
    out = []
    for t in range(n):
        for k in range(4):
            mem4[GATE[k]] = gate[k](t)
        mem4[IN[0]], mem4[IN[1]] = mem[0].get(OUT[0], 0.0), mem[0].get(OUT[1], 0.0)
        mem4[IN[2]], mem4[IN[3]] = mem[1].get(OUT[0], 0.0), mem[1].get(OUT[1], 0.0)
        for d in range(2):
            m = mem[d]
            for v in range(2):
                m[PHASE[v] + 1] = m.get(PHASE[v], 0.0)
            if d == 1:                                             # DSP3 gets its pitches from ids of its own
                m[PITCH[0]], m[PITCH[1]] = pitch[2], pitch[3]
            regs = [0.0] * 8
            for x in ins_osc:
                dspisa.JUMP = None
                regs, m, _ = dspisa.run(list(x), regs, m)
            mem[d] = m                                             # run() returns a new dict
        regs = [0.0] * 8
        for x in ins_gate:
            dspisa.JUMP = None
            regs, mem4, _ = dspisa.run(list(x), regs, mem4)
        out.append(mem4.get(OUT_L, 0.0))
    return out


if __name__ == '__main__':
    osc_text, gate_text = osc_source(), gate_source()
    w_osc, r_osc = words(osc_text)
    w_gate, r_gate = words(gate_text)
    open(os.path.join(HERE, 'solov2_osc.s'), 'w').write(osc_text)
    open(os.path.join(HERE, 'solov2_gate.s'), 'w').write(gate_text)
    open(os.path.join(HERE, 'solov2_osc.hex'), 'w').write(
        '# solov2 SAW, DSP2 and DSP3 DCO1 slot (make_solov2.py)\n' + '\n'.join('%04x' % x for x in w_osc) + '\n')
    open(os.path.join(HERE, 'solov2_gate.hex'), 'w').write(
        '# solov2 gate mix, DSP4 main (make_solov2.py)\n' + '\n'.join('%04x' % x for x in w_gate) + '\n')
    print('solov2_osc:', r_osc.summary(), 'words', len(w_osc))
    print('solov2_gate:', r_gate.summary(), 'words', len(w_gate))
    if '--sim' in sys.argv:
        pitch = [4.75, 4.75, 5.75, 4.75]                           # voice 2 (DSP3, voice 0) one octave up
        gates = [lambda t: 0.0] * 4                                 # noqa: E731
        gates[2] = lambda t: 1.0 if 2000 <= t < 6000 else 0.0       # noqa: E731
        out = simulate(w_osc, w_gate, pitch, gates, 8000)
        cross = sum(1 for t in range(2100, 5900) if out[t - 1] < 0 <= out[t])
        print('voice 2 alone (880 Hz expected): crossings', cross, '-> %.0f Hz' % (cross * 96000 / 3800))
        print('silent before gate:', round(max(abs(v) for v in out[:2000]), 6),
              ' after release:', round(max(abs(v) for v in out[6100:8000]), 6))
