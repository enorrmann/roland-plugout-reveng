"""Two-DSP chain for the gate plug-out, all our own asm (asm/asm.py), no Roland code and no envelope or LFO.

    DSP2 (DCO1 slot): SAW at the pitch the ARM writes to bank 5 id 0x21 ([A021]); the phase is the only state.
                      Output -> MXM Z 0x404, which DSP4 reads as its M 0x005 (routing of docs/DSP_ARM_SIDE.md).
    DSP4 (DCO1 slot): output = SAW from DSP2 times the gate the ARM writes to bank 5 id 0x22 ([A022]),
                      stored in IOMD 6020 (canal 1) and 6021 (canal 0) (docs/WAVE_BUS.md).

The gate is 0 or 1 with no smoothing: the note starts and stops at once.

    python3 make_chain.py          -> dsp2_osc.s/.hex, dsp4_gate.s/.hex
    python3 make_chain.py --sim    -> run both programs in re/dspisa.py: pitch 440 Hz, gate pulse on DSP4
"""
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
import asm  # noqa: E402

PITCH = 0xA021                  # DSP2 bank 5 id 0x21 (ARM: note pitch)
GATE = 0xA022                   # DSP4 bank 5 id 0x22 (ARM: gate)
PHASE = 0x0670                  # DSP2 ring state: store at S, read at S+1 next sample
MXM_OUT = 0x8404                # DSP2 Z 0x404 -> DSP4 M 0x005
MXM_IN = 0x8005                 # DSP4 M 0x005
OUT, OUT2 = 0x6020, 0x6021      # IOMD words that reach the output (DSP4 owns IOMD0 0x20-0x24)
K = 2 * 440.0 / 96000.0


def wrap(reg, tmp):
    """reg in [-1, 1) after reg += inc: reg -= 2 * max(reg - 1, 0) (saturating steps, no branches)"""
    L = [f'addxy r{tmp}, r{reg}, #-1.0:32', f'max r{tmp}, r{tmp}, r5']
    L += [f'mul r{tmp}, r{tmp}, #256.0:32', f'min r{tmp}, r{tmp}, r6'] * 4
    L += [f'mul r{tmp}, r{tmp}, #2.0:32', f'subxy r{reg}, r{tmp}, r{reg}']
    return L


def osc_source():
    L = ['; SAW, DSP2 DCO1 slot. Pitch from the ARM, output to MXM 8404 (DSP4). Own asm: see make_chain.py',
         'mov r5, #0.0:32', 'mov r6, #1.0:32',
         f'ld r0, [{PITCH:04x}]', 'mov r4, r4',
         'add r0, #-4.75:32',                                   # p - 4.75
         f'mul r2, r0, #{math.log(2) / 16!r}:32',               # x = (p - 4.75) ln2 / 16
         'mov r3, #0.041666666666666664:32',
         f'ymacd r3, r2, #{1 / 6!r}:32', f'ymacd r3, r2, #{0.5!r}:32',
         'ymacd r3, r2, #1.0:32', 'ymacd r3, r2, #1.0:32']     # r3 = 2^(x / 16) (Taylor)
    L += ['mul r3, r3, r3'] * 4                                  # r3 = 2^x
    L += [f'ld r1, [{PHASE + 1:04x}]', 'mov r4, r4',
          f'mul r3, r3, #{K!r}:32', 'addxy r1, r3, r1', 'addxy r2, r1, #-1.0:32', 'max r2, r2, r5']
    for _ in range(4):
        L += ['mul r2, r2, #256.0:32', 'min r2, r2, r6']
    L += ['mul r2, r2, #2.0:32', 'subxy r1, r2, r1', f'st r1, [{PHASE:04x}]']
    L += [f'st r1, [{MXM_OUT:04x}]']                             # the saw goes to DSP4
    return '\n'.join(L) + '\n'


def gate_source():
    L = ['; gate, DSP4 DCO1 slot. Saw from DSP2 (MXM 8005) times the note gate, to IOMD 6020 / 6021. Own asm: see make_chain.py',
         f'ld r4, [{GATE:04x}]',
         f'ld r1, [{MXM_IN:04x}]',
         'mov r4, r4',
         'mul r1, r1, r4',
         f'st r1, [{OUT:04x}]', f'st r1, [{OUT2:04x}]']
    return '\n'.join(L) + '\n'


def words(text):
    r = asm.Assembler().assemble(text)
    if not r.ok:
        raise SystemExit(r.errors)
    return [0x0A02, 0xFF70] + r.words + [0x0B02, 0x00F8, 0x0001], r


def simulate(w_osc, w_gate, pitch, gate, n):
    """one sample = DSP2 program, then DSP4 program; DSP4 reads the saw DSP2 stored in the previous sample"""
    import dspdis
    import dspisa
    ins_osc = [i for _, i in dspdis.instructions(w_osc)]
    ins_gate = [i for _, i in dspdis.instructions(w_gate)]
    mem2 = {PITCH: pitch, PHASE + 1: 0.0}
    mem4 = {GATE: 0.0, MXM_IN: 0.0}
    out = []
    for t in range(n):
        mem4[GATE] = gate(t)
        mem4[MXM_IN] = mem2.get(MXM_OUT, 0.0)                    # MXM latch: last sample's DSP2 store
        mem2[PHASE + 1] = mem2.get(PHASE, 0.0)
        regs = [0.0] * 8
        for x in ins_osc:
            dspisa.JUMP = None
            regs, mem2, _ = dspisa.run(list(x), regs, mem2)
        regs = [0.0] * 8
        for x in ins_gate:
            dspisa.JUMP = None
            regs, mem4, _ = dspisa.run(list(x), regs, mem4)
        out.append(mem4.get(OUT, 0.0))
    return out


if __name__ == '__main__':
    osc, gate = osc_source(), gate_source()
    w_osc, r_osc = words(osc)
    w_gate, r_gate = words(gate)
    open(os.path.join(HERE, 'dsp2_osc.s'), 'w').write(osc)
    open(os.path.join(HERE, 'dsp4_gate.s'), 'w').write(gate)
    open(os.path.join(HERE, 'dsp2_osc.hex'), 'w').write(
        '# SAW, DSP2 DCO1 slot (make_chain.py)\n' + '\n'.join('%04x' % x for x in w_osc) + '\n')
    open(os.path.join(HERE, 'dsp4_gate.hex'), 'w').write(
        '# gate, DSP4 DCO1 slot (make_chain.py)\n' + '\n'.join('%04x' % x for x in w_gate) + '\n')
    print('dsp2_osc:', r_osc.summary(), 'words', len(w_osc))
    print('dsp4_gate:', r_gate.summary(), 'words', len(w_gate))
    if '--sim' in sys.argv:
        gate_pulse = lambda t: 1.0 if 2000 <= t < 6000 else 0.0  # noqa: E731
        out = simulate(w_osc, w_gate, 4.75, gate_pulse, 8000)     # note 69 = 440 Hz
        for t in (0, 1999, 2000, 2001, 3000, 5999, 6000, 6001, 7000):
            print('t=%4d gate=%d out=%+.4f' % (t, gate_pulse(t), out[t]))
        peaks = sum(1 for t in range(2100, 5900) if out[t - 1] < 0 <= out[t])
        print('upward zero crossings in 3800 samples with gate on:', peaks,
              '-> %.1f Hz' % (peaks * 96000 / 3800))
