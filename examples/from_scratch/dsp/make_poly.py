"""Four-voice version of the DSP5 voice (make_voice.py is the one-voice version). Symmetric for both slaves:
each slave's DSP5 runs these four voices, the ARM writes voice k's pitch to bank 5 id 0x60+k and its gate to 0x68+k.

Per voice: AR envelope (rate KA attack / KR release, branch free), SAW at its pitch with the shared LFO
vibrato, times the envelope. The four voices are summed (scale 1/4) into IOMD 6020 (canal 1) and 6021 (canal 0).

    python3 make_poly.py          -> poly.s, poly.hex
    python3 make_poly.py --opt    -> also poly_opt.s / poly_opt.hex: the same voice with ph_wrap (Roland's form)
                                     instead of the manual wraps, scheduled by asm/dspsched.py
                                     (docs/MICROCODE_OPTIMIZATION.md); poly.hex (tested on the device) stays
    python3 make_poly.py --sim    -> four voices in re/dspisa.py: each gate pulse must give its own pitch
"""
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
import asm  # noqa: E402
import dspsched  # noqa: E402

NV = 4
PITCH = [0xA060 + k for k in range(NV)]        # bank 5 ids 0x60+k (ARM: note pitch)
GATE = [0xA068 + k for k in range(NV)]         # bank 5 ids 0x68+k (ARM: gate)
PHASE = [0x0670 + 4 * k for k in range(NV)]    # ring state, store at S, read at S+1 next sample
ENV = [0x0690 + 4 * k for k in range(NV)]
LFO, LFOV = 0x06A0, 0x06B0                     # LFOV: this sample's LFO value (same-sample store and load)
OUT_L, OUT_R = 0x6020, 0x6021                  # canal 1, canal 0 (docs/WAVE_BUS.md)
KA, KR = 0.01, 0.0002
LFO_HZ, DEPTH = 5.5, 0.02
K = 2 * 440.0 / 96000.0


def wrap(reg, tmp):
    L = [f'addxy r{tmp}, r{reg}, #-1.0:32', f'max r{tmp}, r{tmp}, r5']
    L += [f'mul r{tmp}, r{tmp}, #256.0:32', f'min r{tmp}, r{tmp}, r6'] * 4
    L += [f'mul r{tmp}, r{tmp}, #2.0:32', f'subxy r{reg}, r{tmp}, r{reg}']
    return L


def wrap_ph(reg, tmp):
    """ph_wrap (Roland's form, X = r3): the phase into [-1, 1) in one op; tmp unused"""
    return [f'ph_wrap r{reg}, r3, r{reg}']


def source(nv=NV, opt=False):
    """the program text; opt: ph_wrap instead of the manual wraps (lines, one unit each; dspsched packs them)"""
    wrap_ = wrap_ph if opt else wrap
    L = ['; four-voice mono-slave voice, DSP5 DCO1 slot (make_poly.py)',
         'mov r5, #0.0:32', 'mov r6, #1.0:32', 'mov r4, #0.0:32']
    # LFO once per sample; kept in LFOV for the voices
    L += [f'ld r3, [{LFO + 1:04x}]', 'mov r4, r4', f'addxy r3, r3, #{2 * LFO_HZ / 96000!r}:32'] + wrap_(3, 2)
    L += [f'st r3, [{LFO:04x}]', f'st r3, [{LFOV:04x}]', 'mov r4, #0.0:32']
    for k in range(nv):
        # envelope: e' = e + (g - e) * k, k = KR + s * (KA - KR), s = [g > e]
        L += [f'ld r7, [{ENV[k] + 1:04x}]', f'ld r0, [{GATE[k]:04x}]', 'mov r4, r4',
              'subxy r1, r7, r0',
              'mul r2, r1, #256.0:32', 'mul r2, r2, #256.0:32', 'max r2, r2, r5', 'min r2, r2, r6',
              f'mul r2, r2, #{KA - KR!r}:32', f'add r2, #{KR!r}:32',
              'mul r1, r1, r2', 'addxy r7, r7, r1',
              f'st r7, [{ENV[k]:04x}]']
        # pitch: voice pitch + DEPTH * lfo - 4.75 (r4 keeps the mix; r7 the envelope)
        L += [f'ld r0, [{PITCH[k]:04x}]', f'ld r3, [{LFOV:04x}]',
              f'mad r0, r3, #{DEPTH!r}:32', 'add r0, #-4.75:32',
              f'mul r2, r0, #{math.log(2) / 16!r}:32',
              'mov r3, #0.041666666666666664:32',
              f'ymacd r3, r2, #{1 / 6!r}:32', f'ymacd r3, r2, #{0.5!r}:32',
              'ymacd r3, r2, #1.0:32', 'ymacd r3, r2, #1.0:32']
        L += ['mul r3, r3, r3'] * 4
        # saw
        L += [f'ld r1, [{PHASE[k] + 1:04x}]', 'mov r4, r4', f'mul r3, r3, #{K!r}:32', 'addxy r1, r3, r1']
        if opt:
            L += ['ph_wrap r1, r3, r1']
        else:
            L += ['addxy r2, r1, #-1.0:32', 'max r2, r2, r5']
            for _ in range(4):
                L += ['mul r2, r2, #256.0:32', 'min r2, r2, r6']
            L += ['mul r2, r2, #2.0:32', 'subxy r1, r2, r1']
        L += [f'st r1, [{PHASE[k]:04x}]']
        # voice output (saw * envelope) summed into r4
        L += ['mul r1, r1, r7', 'addxy r4, r4, r1']
    L += ['mul r4, r4, #0.25:32', f'st r4, [{OUT_L:04x}]', f'st r4, [{OUT_R:04x}]']
    return '\n'.join(L) + '\n'


def source_opt(nv=NV, scheduled=True):
    """the optimized voice: ph_wrap, then (scheduled) dspsched.schedule"""
    lines = source(nv, opt=True).splitlines()
    return '\n'.join(dspsched.schedule(lines) if scheduled else lines) + '\n'


def words(text):
    r = asm.Assembler().assemble(text)
    if not r.ok:
        raise SystemExit(r.errors)
    return [0x0A02, 0xFF70] + r.words + [0x0B02, 0x00F8, 0x0001], r


def simulate(w, gates, pitches, n):
    import dspdis
    import dspisa
    ins = [i for _, i in dspdis.instructions(w)]
    mem = {}
    for k in range(NV):
        mem[PITCH[k]] = pitches[k]
    out = []
    for t in range(n):
        for k in range(NV):
            mem[GATE[k]] = gates[k](t)
        for st in list(PHASE) + list(ENV) + [LFO]:
            if st in mem:
                mem[st + 1] = mem[st]
        regs = [0.0] * 8
        for x in ins:
            dspisa.JUMP = None
            regs, mem, _ = dspisa.run(list(x), regs, mem)
        out.append(mem.get(OUT_L, 0.0))
    return out


if __name__ == '__main__':
    text = source()
    w, r = words(text)
    open(os.path.join(HERE, 'poly.s'), 'w').write(text)
    open(os.path.join(HERE, 'poly.hex'), 'w').write(
        '# four-voice DSP5 voice (make_poly.py)\n' + '\n'.join('%04x' % x for x in w) + '\n')
    print(r.summary(), 'words', len(w))
    if '--opt' in sys.argv:
        t2 = source_opt()
        w2, r2 = words(t2)
        open(os.path.join(HERE, 'poly_opt.s'), 'w').write('; poly with ph_wrap, scheduled (make_poly.py --opt)\n' + t2)
        open(os.path.join(HERE, 'poly_opt.hex'), 'w').write(
            '# four-voice DSP5 voice, optimized (make_poly.py --opt)\n' + '\n'.join('%04x' % x for x in w2) + '\n')
        print('opt:', r2.summary(), 'words', len(w2))
    if '--sim' in sys.argv:
        # voice 1 at note 69 (440 Hz) from t=100, voice 3 at note 57 (220 Hz) from t=100, both held
        gates = [lambda t: 0.0, lambda t: 1.0 if t >= 100 else 0.0,
                 lambda t: 0.0, lambda t: 1.0 if t >= 100 else 0.0]
        pitches = [0, 4.75, 0, 3.75]
        out = simulate(w, gates, pitches, 5000)
        s = out[1000:5000]
        import numpy as np
        f = np.fft.rfftfreq(len(s), 1 / 96000)
        S = np.abs(np.fft.rfft(np.array(s) * np.hanning(len(s))))
        for q in (220.0, 440.0):
            m = (f > q * 0.98) & (f < q * 1.02)
            print('sim: %.0f Hz peak level %.1f dB' % (q, 20 * np.log10(S[m].max() / (len(s) / 2) + 1e-12)))
