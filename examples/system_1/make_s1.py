"""C-S1V4: our own DSP programs for the SYSTEM-1 port of C-V4 (no Roland bytes; docs/examples_system_1_README.md).

The SYSTEM-1 has two DSPs per BMC (A and B, queues 0x0100FC30 / 0x0100FC34), and in PLUG-OUT mode the audio goes
slave DSP A/B -> IOMD 6006..600D -> master DSP B (program at 0x1000) -> bank 5 A118/A119 -> the OS's own output
program of master DSP A (at 0x82: A118/A119 -> DC filter -> x C004 -> 6000/6001). Found in the SH-2 and SH-101
images and the OS 1.30 (study only, nothing copied; docs/examples_system_1_README.md section 2).

C-V4's flow on SYSTEM-8 was DSP0/1 pitch -> DSP2/3 saw slot -> DSP4/5 gate -> 6020. Here one DSP does it all:

    slave A  s1m0 @0x0100: 4 voices: pitch A030+v (ARM, octaves: (note-12)/12), saw (the C-V4 saw, phase in the
                           bank-0 ring: store P, load P+1), x gate A038+v (ARM, 0/1), sum x 0.25 -> A150
    slave B  s1m1 @0x0100: A150 x C000 (the DSP's output level, set by the OS) -> 6006 (L), 6007 (R); 6008..600D = 0
    master B m1   @0x1000: 6006 x C01C -> A118, 6007 x C01C -> A119; A11A..A11D = 0 (C01C = volume, model.c)
    closes   s1c0/s1c1 @0x7FF2 (slave A / B): frame counter E000 -> A004 / A005, as every SYSTEM-1 PLUG-OUT has
             s1e @0x5FF2/0x6FF2 (master B): empty section, as the OS's own PLUG-OUT-mode set has

    python3 make_s1.py            -> s1*.s / s1*.hex here, build/s1_programs.h
    python3 make_s1.py --tone     -> the same, but s1m0 plays voice 0 at 4.75 (A) all the time (bring-up test:
                                     sound without notes or ARM parameters)
    python3 make_s1.py --sim      -> also simulate one chord through slave A, slave B and master B
"""
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
import asm  # noqa: E402

NV = 4
PITCH = [0xA030 + v for v in range(NV)]     # written by the slave ARM (engine.c, FD6C sel 0, idx 0x30 + v)
GATE = [0xA038 + v for v in range(NV)]      # idem, idx 0x38 + v
PHASE = [0x0300 + 0x10 * v for v in range(NV)]   # bank 0 ring: store P, read P + 1 one sample later
MIX = 0xA150                                # slave A -> slave B (bank 5 is shared by A and B)
OUT_L, OUT_R = 0x6006, 0x6007               # slave -> master (IOMD)
OUT_ZERO = [0x6008, 0x6009, 0x600A, 0x600B, 0x600C, 0x600D]
M_OUT = (0xA118, 0xA119)                    # master B -> the OS's output program on master A
M_ZERO = [0xA11A, 0xA11B, 0xA11C, 0xA11D]
M_VOLUME = 0x1C                             # master DSP B parameter for the VOLUME knob (as the SH-2: C01C)
LEVEL = 0.25                                # four voices of amplitude 1 -> at most 1.0


def saw_voice(v, tone=False):
    """C-V4's saw (examples/from_scratch/dsp/saw_dco2.s, f = 440 * 2^(p - 4.75) / 1.032045 at 96 kHz), gated"""
    P = PHASE[v]
    L = [f'; voice {v}']
    L += [f'ld r1, [{P + 1:04x}]']
    if tone:
        L += ['mov r0, #4.75']
    else:
        L += [f'ld r0, [{PITCH[v]:04x}]', 'mov r4, r4']
    L += ['add r0, #-4.75',
          'mul r2, r0, #0.04332169878499658',
          'mov r3, #0.041666666666666664:32',
          'ymacd r3, r2, #0.16666666666666666',
          'ymacd r3, r2, #0.5',
          'ymacd r3, r2, #1.0',
          'ymacd r3, r2, #1.0']
    L += ['mul r3, r3, r3'] * 4
    L += ['mul r3, r3, #0.009166666666666667',
          'addxy r1, r3, r1',
          'addxy r2, r1, #-1.0',
          'max r2, r2, r5']
    L += ['mul r2, r2, #256.0', 'min r2, r2, r6'] * 4
    L += ['mul r2, r2, #2.0',
          'subxy r1, r2, r1',
          f'st r1, [{P:04x}]']
    if tone:
        L += ['mov r4, #1.0' if v == 0 else 'mov r4, #0.0']
    else:
        L += [f'ld r4, [{GATE[v]:04x}]', 'mov r4, r4']
    L += ['mul r1, r1, r4', 'addxy r7, r7, r1']
    return L


def prog_s1m0(tone=False):
    L = ['; s1m0: slave DSP A main @0x0100: four gated saws -> A150' + (' (TONE test: voice 0 always on)' if tone else ''),
         'mov r5, #0.0', 'mov r6, #1.0', 'mov r7, #0.0']
    for v in range(NV):
        L += saw_voice(v, tone)
    L += [f'mul r7, r7, #{LEVEL}', f'st r7, [{MIX:04x}]']
    return L


def prog_s1m1():
    L = ['; s1m1: slave DSP B main @0x0100: A150 x C000 -> 6006 / 6007, the other outputs 0',
         'ld r7, [c000]', f'ld r0, [{MIX:04x}]', 'mov r4, r4', 'mul r1, r0, r7',
         f'st r1, [{OUT_L:04x}]', f'st r1, [{OUT_R:04x}]', 'mov r0, #0.0']
    L += [f'st r0, [{a:04x}]' for a in OUT_ZERO]
    return L


def prog_m1():
    L = ['; s1mm1: master DSP B @0x1000: slave 6006 / 6007 x volume (C01C) -> A118 / A119 (the OS output program reads them)',
         f'ld r0, [{OUT_L:04x}]', f'ld r1, [{OUT_R:04x}]', f'ld r7, [c{M_VOLUME:03x}]', 'mov r4, r4',
         'mul r0, r0, r7', 'mul r1, r1, r7',
         f'st r0, [{M_OUT[0]:04x}]', f'st r1, [{M_OUT[1]:04x}]', 'mov r2, #0.0']
    L += [f'st r2, [{a:04x}]' for a in M_ZERO]
    return L


def assemble(lines):
    r = asm.Assembler().assemble('\n'.join(lines) + '\n')
    if not r.ok:
        raise SystemExit(r.errors)
    return r


def program(lines):
    """a section program: SYSTEM-1 start marker (0a02 ff71), the code, end sequence"""
    r = assemble(lines)
    return [0x0A02, 0xFF71] + r.words + [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001], r


def close(word):
    """the 7 halfwords at 0x7FF2 (the end of program memory): frame counter E000 -> word"""
    r = assemble([f'ld r0, [e000]', f'st r0, [{word:04x}]'])
    w = [0x0000] + r.words
    assert len(w) == 7
    return w, r


def empty_section():
    """the 7 halfwords at the end of an unused section (0x5FF2, 0x6FF2): an empty program"""
    return [0x0000, 0x0A02, 0xFF71, 0x0A02, 0xFF70, 0x0B02, 0x00F8], None


def sources(tone=False):
    return {
        's1m0': program(prog_s1m0(tone)) + (prog_s1m0(tone),),
        's1m1': program(prog_s1m1()) + (prog_s1m1(),),
        's1mm1': program(prog_m1()) + (prog_m1(),),
        's1c0': close(0xA004) + (['ld r0, [e000]', 'st r0, [a004]'],),
        's1c1': close(0xA005) + (['ld r0, [e000]', 'st r0, [a005]'],),
        's1e': empty_section() + ([],),
    }


def c_array(name, w):
    """32-bit words, the first halfword in the high half (the packing of every SYSTEM-1 program set)"""
    p = list(w) + [0] * (len(w) & 1)
    words = [(p[i] << 16) | p[i + 1] for i in range(0, len(p), 2)]
    body = ',\n'.join('    ' + ', '.join(f'0x{x:08x}' for x in words[i:i + 6]) for i in range(0, len(words), 6))
    return (f'#define MC_{name.upper()}_BYTES 0x{2 * len(w):x}\n'
            f'static const uint32_t mc_{name.upper()}[{len(words)}] = {{\n{body}\n}};\n')


def float_bits(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


def header(progs, tone):
    out = ['/* generated by make_s1.py%s: our own DSP programs and the pitch table (no Roland bytes) */'
           % (' --tone' if tone else ''), '#include <stdint.h>', '']
    for name, (w, _, _) in progs.items():
        out.append(c_array(name, w))
    out.append('/* pitch word of a MIDI note: (note - 12) / 12 octaves as an IEEE float (4.75 = A 440) */')
    tab = [float_bits((n - 12) / 12.0) for n in range(128)]
    out.append('static const uint32_t pitch_bits[128] = {\n' + ',\n'.join(
        '    ' + ', '.join(f'0x{x:08x}' for x in tab[i:i + 8]) for i in range(0, 128, 8)) + '\n};')
    out.append('/* master volume: VOLUME knob (id 0x100, 0..255) -> gain (v / 255)^2 as an IEEE float */')
    vol = [float_bits((v / 255.0) ** 2) for v in range(256)]
    out.append('static const uint32_t volume_bits[256] = {\n' + ',\n'.join(
        '    ' + ', '.join(f'0x{x:08x}' for x in vol[i:i + 8]) for i in range(0, 256, 8)) + '\n};')
    out.append(f'#define M_VOLUME 0x{M_VOLUME:02x}')
    out.append(f'#define F_ONE 0x{float_bits(1.0):08x}u\n#define F_ZERO 0x00000000u')
    out.append(f'#define B5_PITCH 0x{PITCH[0] & 0xFF:02x}\n#define B5_GATE 0x{GATE[0] & 0xFF:02x}')
    return '\n'.join(out) + '\n'


def simulate(progs, notes, n=9600):
    """per sample: slave A, slave B, master B. Bank 5 is shared by A and B of one BMC; the slave's IOMD words are
    read by the master in the same sample (the latency does not matter here). Returns master A118 per sample."""
    import dspdis
    import dspisa
    code = {k: [i for _, i in dspdis.instructions(progs[k][0])] for k in ('s1m0', 's1m1', 's1mm1')}
    sb5, mb5 = {}, {}                                   # bank 5 of the slave pair / of the master pair
    for v in range(NV):
        sb5[PITCH[v]] = (notes[v] - 12) / 12.0 if v < len(notes) else 0.0
        sb5[GATE[v]] = 1.0 if v < len(notes) else 0.0
    mem_a, mem_b, mem_m = {}, {}, {}
    out = []
    for _ in range(n):
        for p in PHASE:                                  # bank 0 ring: R written now is read at R + 1 next sample
            mem_a[p + 1] = mem_a.get(p, 0.0)
        mem_b[0xC000] = 1.0
        mem_m[0xC000 + M_VOLUME] = 1.0
        for key, mem, b5 in (('s1m0', mem_a, sb5), ('s1m1', mem_b, sb5)):
            m = dict(mem)
            m.update(b5)
            regs = [0.0] * 8
            for x in code[key]:
                dspisa.JUMP = None
                regs, m, _ = dspisa.run(list(x), regs, m)
            for k, val in m.items():
                if 0xA000 <= k < 0xC000:
                    b5[k] = val
            mem.clear()
            mem.update(m)
        m = dict(mem_m)
        for a in [OUT_L, OUT_R]:
            m[a] = mem_b.get(a, 0.0)
        regs = [0.0] * 8
        for x in code['s1mm1']:
            dspisa.JUMP = None
            regs, m, _ = dspisa.run(list(x), regs, m)
        mem_m = m
        out.append(m.get(M_OUT[0], 0.0))
    return out


def main():
    tone = '--tone' in sys.argv
    progs = sources(tone)
    for name, (w, r, lines) in progs.items():
        if lines:
            open(os.path.join(HERE, name + '.s'), 'w').write('\n'.join(lines) + '\n')
        open(os.path.join(HERE, name + '.hex'), 'w').write(
            f'# {name}, own SYSTEM-1 program (make_s1.py)\n' + '\n'.join('%04x' % x for x in w) + '\n')
        print(name, r.summary() if r else 'empty section', 'words', len(w))
    os.makedirs(os.path.join(HERE, 'build'), exist_ok=True)
    open(os.path.join(HERE, 'build', 's1_programs.h'), 'w').write(header(progs, tone))
    if '--sim' in sys.argv:
        for notes in ([69], [57, 69], [60, 64, 67, 72]):
            seq = simulate(progs, notes)
            cross = sum(1 for t in range(1, len(seq)) if seq[t - 1] < 0 <= seq[t])
            print('notes', notes, 'peak %.3f' % max(abs(x) for x in seq[200:]),
                  'zero crossings/s %.0f' % (cross * 96000 / len(seq)))
        seq = simulate(progs, [])
        print('no notes: peak %.6f' % max(abs(x) for x in seq))


if __name__ == '__main__':
    main()
