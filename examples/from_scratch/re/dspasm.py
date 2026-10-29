"""Minimal assembler for the established DSP subset (DSP_ISA_SPEC.md) + our own SAW oscillator.

    python3 dspasm.py saw dco2 [--fs 48000] [-o prog.txt]   # oscillator for the DCO2 (or dco1) slot
    python3 dspasm.py sim prog.txt --pitch 4.75 [--n 400]    # run it in the C reference (emu/dspref.py), print period

Encoders (each returns one instruction as a list of 16-bit words):
    a(op, d, x, y=None, imm=None)   unit A op (class 0x048); y = register (negative int = negated) or imm (float)
    b(op, d, x, y)                  unit B op (class 0x050), d >= 8 forms (min = op 0, max = op 2), x in r0..r3
    ld(r, addr), st(r, addr)        transfer with one reference (class 0x060)
Every instruction is checked by decoding it again with the C decoder (dspref.template).
"""
import argparse, math, os, struct, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'emu'))
import dspref   # noqa: E402

A_OPS = {'move': (0, 1), 'add3': (1, None), 'mul': (2, None), 'mac': (3, None), 'ymacd': (4, None),
         'ymsubd': (5, None), 'add': (6, None), 'sub': (7, None)}       # name -> (op, forced X sub-code)


def raw(v):
    """float -> RAW word (DSP_ISA_SPEC.md §2: smallest exponent with M < 2^29)"""
    if v == 0:
        return 0
    s = 1 if v < 0 else 0
    m = abs(v)
    for e in range(4):
        M = int(round(m * 2 ** (32 - 4 * e)))
        if M < 2 ** 29:
            return s << 31 | e << 29 | M
    raise ValueError(f'{v} out of range (|v| < 512)')


def imm_words(v, bits):
    w = raw(v)
    if bits == 16:
        if w & 0xFFFF:
            raise ValueError(f'{v} not exact in 16 bits; use 32')
        return [w >> 16]
    return [w >> 16, w & 0xFFFF]


def a(name, d, x=0, y=0, imm=None, bits=32):
    # note: for 'move' the operand is y (register) or imm
    """unit A: rd = op(...); y: register index (negative -> -r|y|), or imm (float)"""
    op, sub = A_OPS[name]
    if sub is not None:
        x = sub
    if imm is not None:
        mode = {16: 1, 32: 3}[bits]
        word = d << 12 | op << 9 | x << 6 | mode << 4
        words = [word] + imm_words(imm, bits)
    else:
        yf = (8 | -y) if y < 0 else y
        if y < 0 and -y > 7 or y > 7:
            raise ValueError('register 0..7')
        words = [d << 12 | op << 9 | x << 6 | yf]
    return [0x048 << 5 | (len(words) + 1)] + words


def b(name, d, x, y=0, imm=None):
    """unit B, measured forms only (DSP_ISA_SPEC.md §6.3), x in r0..r3, y register:
    min / max (rd = min|max(rx, ry), D = 8 + d form; imm: 16-bit immediate instead of ry, as Roland's
    `d550` max(r5, 0)), absadd (rd = |rx| + ry), subabs (rd = ry - |rx|)"""
    if not 0 <= x <= 3:
        raise ValueError('unit B measured with X in r0..r3 only')
    if name in ('min', 'max'):
        w = (8 + d) << 12 | {'min': 0, 'max': 2}[name] << 9 | x << 6
        if imm is not None:
            return [0x050 << 5 | 3, w | 1 << 4] + imm_words(imm, 16)
        return [0x050 << 5 | 2, w | y]
    nib = {'absadd': 0xC, 'subabs': 0xE}[name]
    return [0x050 << 5 | 2, d << 12 | nib << 8 | x << 6 | y]


def aab(a1, a2, bop):
    """class 0x160 (A, A, B in one instruction; DSP_ISA_SPEC.md §4.4): the three ops read their operands before any
    write; at most one of them may take an immediate (every A/B op reads its immediate from the start of the bit
    stream, so two immediates would be the same bits). a1, a2: a(...) results, bop: b(...) result."""
    imms = [ins[2:] for ins in (a1, a2, bop) if len(ins) > 2]
    if len(imms) > 1:
        raise ValueError('only one immediate per A/A/B instruction (they share the bit stream)')
    words = [a1[1], a2[1], bop[1]] + (imms[0] if imms else [])
    return [0x160 << 5 | (len(words) + 1)] + words


def ld(r, addr):
    return [0x060 << 5 | 3, r << 12 | 0xFE0, addr]


def st(r, addr):
    return [0x060 << 5 | 3, (8 + r) << 12 | 0xFE0, addr]


SLOTS = {   # pitch in, outputs, state memory  (from Roland's SAW programs, NOTES.md)
    'dco2': {'A': (0x2207, (0x2221, 0x221D), 0x0670), 'B': (0x2214, (0x2229, 0x2225), 0x0680)},
    'dco1': {'A': (0x2201, (0x221F, 0x221B), 0x0470), 'B': (0x220E, (0x2227, 0x2223), 0x0480)},
}
# Data memory is a ring addressed relative to a pointer that advances one word per sample (DSP_ISA_SPEC.md §9):
# a value stored at reference S is read at S + 1 in the next sample. A loaded register can be used by an ALU op
# only from the second instruction after the load.


def saw(slot, fs=96000.0, level=1.0):
    """naive SAW: phase += 2 * 440 * 2^(p - 4.75) / fs, wrap at 1 -> -1; out = level * phase"""
    K = 2 * 440.0 / fs
    P = [ [0x0A02, 0xFF70] ]                         # slot program start, as Roland's
    P.append(a('move', 5, imm=0.0, bits=16))         # r5 = 0
    P.append(a('move', 6, imm=1.0, bits=16))         # r6 = 1
    for voice in 'AB':
        pin, outs, state = SLOTS[slot][voice]
        P += [ld(1, state + 1),                             # r1 = phase of the previous sample (ring: S + 1)
              ld(0, pin),                                   # r0 = pitch p
              a('add', 0, 0, imm=-4.75),                    # r0 = x = p - 4.75
              a('mul', 2, 0, imm=math.log(2) / 16),         # r2 = y = x ln2 / 16
              a('move', 3, imm=1 / 24),                     # r3 = 1/24
              a('ymacd', 3, 2, imm=1 / 6),                  # r3 = 1/6 + y r3
              a('ymacd', 3, 2, imm=0.5),                    # r3 = 1/2 + y r3
              a('ymacd', 3, 2, imm=1.0),                    # r3 = 1 + y r3
              a('ymacd', 3, 2, imm=1.0)]                    # r3 = 1 + y r3  = e^y
        P += [a('mul', 3, 3, 3)] * 4                        # r3 = (e^y)^16 = 2^x
        P += [a('mul', 3, 3, imm=K),                        # r3 = increment
              a('add', 1, 3, 1),                            # r1 = phase + inc
              a('add', 2, 1, imm=-1.0, bits=16),            # r2 = phase - 1
              b('max', 2, 2, 5)]                            # r2 = max(r2, 0)
        for _ in range(4):
            P += [a('mul', 2, 2, imm=256.0, bits=16),       # r2 = r2 * 256
                  b('min', 2, 2, 6)]                        # r2 = min(r2, 1)  -> exactly 0 or 1 at the end
        P += [a('mul', 2, 2, imm=2.0, bits=16),             # r2 = 2 * wrap
              a('sub', 1, 2, 1),                            # r1 = r1 - r2
              st(1, state)]
        if level != 1.0:
            P.append(a('mul', 1, 1, imm=level))
        P += [st(1, outs[0]), st(1, outs[1])]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [0x0001]]
    for ins in P:                                    # every instruction must decode with its template
        if dspref.template(ins) is None:
            raise AssertionError(f'no template for {ins}')
    return P


def osc_core(pin, state):
    """common part: r1 = phase in [-1, 1) advanced by 2 * 440 * 2^(p - 4.75) / fs (needs r5 = 0, r6 = 1; uses r0-r3)"""
    return [], pin, state


def wrap(reg, tmp):
    """reg -= 2 if reg >= 1 (branch-free, exact; reg < 3); needs r5 = 0, r6 = 1; tmp in r0..r3"""
    P = [a('add', tmp, reg, imm=-1.0, bits=16), b('max', tmp, tmp, 5)]
    for _ in range(4):
        P += [a('mul', tmp, tmp, imm=256.0, bits=16), b('min', tmp, tmp, 6)]
    P += [a('mul', tmp, tmp, imm=2.0, bits=16), a('sub', reg, tmp, reg)]
    return P


FOLD_SLOTS = {   # DCO1 only: phase state S, LFO state L (ring: load X+1, store X)
    'dco1': {'A': (0x2201, (0x221F, 0x221B), 0x0470, 0x0474), 'B': (0x220E, (0x2227, 0x2223), 0x0480, 0x0484)},
    'dco2': {'A': (0x2207, (0x2221, 0x221D), 0x0670, 0x0674), 'B': (0x2214, (0x2229, 0x2225), 0x0680, 0x0684)},
}


def fold(slot, fs=96000.0, lfo_hz=0.25, gmin=1.0, gmax=4.5, level=0.9):
    """wavefolder with motion: triangle at the note's pitch, gain g = gmin + (gmax - gmin) * |lfo| swept by an
    internal triangle LFO (lfo_hz), folded twice by reflections at +-1:  x <- 1 - |x - 1| ; x <- |x + 1| - 1"""
    K = 2 * 440.0 / fs
    P = [[0x0A02, 0xFF70],
         a('move', 5, imm=0.0, bits=16), a('move', 6, imm=1.0, bits=16), a('move', 7, imm=-1.0, bits=16)]
    for voice in 'AB':
        pin, outs, state, lstate = FOLD_SLOTS[slot][voice]
        P += [ld(1, state + 1),                             # r1 = phase (previous sample)
              ld(0, pin),                                   # r0 = pitch p
              ld(4, lstate + 1),                            # r4 = LFO phase
              a('add', 0, 0, imm=-4.75),
              a('mul', 2, 0, imm=math.log(2) / 16),
              a('move', 3, imm=1 / 24),
              a('ymacd', 3, 2, imm=1 / 6), a('ymacd', 3, 2, imm=0.5),
              a('ymacd', 3, 2, imm=1.0), a('ymacd', 3, 2, imm=1.0)]
        P += [a('mul', 3, 3, 3)] * 4
        P += [a('mul', 3, 3, imm=K), a('add', 1, 3, 1)]     # r1 = phase + inc
        P += wrap(1, 2)
        P += [st(1, state)]
        # LFO: r4 += 2 * lfo_hz / fs, wrapped (tmp r2)
        P += [a('add', 4, 4, imm=2 * lfo_hz / fs)]
        P += wrap(4, 2)
        P += [st(4, lstate)]
        # triangle from the phase: tri = 2|phase| - 1
        P += [b('absadd', 3, 1, 7),                         # r3 = |phase| - 1
              a('mul', 3, 3, imm=2.0, bits=16),
              a('add', 3, 3, 6)]                            # r3 = tri in [-1, 1]
        # gain from the LFO: g = gmin + (gmax - gmin) * |lfo|
        P += [a('move', 2, y=4),
              b('absadd', 2, 2, 5),                         # r2 = |lfo|
              a('mul', 2, 2, imm=gmax - gmin),
              a('add', 2, 2, imm=gmin),
              a('mul', 0, 2, 3)]                            # r0 = g * tri
        for _ in range(2):                                  # two fold stages: |x| up to 5
            P += [a('add', 2, 0, 7),                        # r2 = x - 1
                  b('subabs', 0, 2, 6),                     # x = 1 - |x - 1|
                  a('add', 3, 0, 6),                        # r3 = x + 1
                  b('absadd', 0, 3, 7)]                     # x = |x + 1| - 1
        if level != 1.0:
            P.append(a('mul', 0, 0, imm=level))
        P += [st(0, outs[0]), st(0, outs[1])]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [0x0001]]
    for ins in P:
        if dspref.template(ins) is None:
            raise AssertionError(f'no template for {ins}')
    return P


def wavetable(slot, table, fs=96000.0, level=1.0, chunk=64, pad_mid=0, fused=True, pad_mid_aab=0):
    """single-cycle wavetable oscillator with the table hardcoded in the program (no indexed reads exist):
    linear interpolation between the N points (v[N] = v[0]), every point evaluated each sample.
    u = N * (phase + 1) / 2 in [0, N). The points are split in chunks [a, b) of <= `chunk` points; chunk value
        w = clamp(u - a, 0, b - a),   h(w) = cst + sum_{k in chunk} s_k * max(w, k - a)
    = interp(clamp(u, a, b)) - v[a] (s_k = change of slope at k inside the chunk, slope 0 before a), and
    out = v[0] + sum of the chunk values. Two instructions per point (unit B max(r2, imm16) and unit A op 3
    acc += r2 * imm32), ~5 per chunk; intermediate values stay below 4 * chunk + 4 (format limit 512).
    fused (default): ONE instruction per point, class 0x160 A/A/B, software-pipelined on relu(w - i):
        r1 = w - j (counter), r2 = relu(w - (j - 1)), r3 = chunk sum;
        instruction j: A r1 += r7 (-1) | A r3 += r2 * s[j-1] (imm32) | B r2 = max(r1, r5 = 0)
    (all read before writing). No lower clamp needed (w < 0 gives relu 0), the upper clamp min(w, L) remains,
    ~4 instructions per chunk; partial sums stay below 2 + 2 * chunk.
    pad_mid_aab: filler A/A/B instructions there (cost of a 3-op instruction).
    pad_mid: filler ALU instructions (r7 = r5) between the voice A and the voice B code (time budget probe)."""
    N = len(table)
    if N < 2 or N > 4096:
        raise ValueError('table of 2..4096 points')
    K = 2 * 440.0 / fs
    v = [float(x) for x in table]
    d = [v[(k + 1) % N] - v[k] for k in range(N)]
    chunks = []
    for a_ in range(0, N, chunk):
        b_ = min(N, a_ + chunk)
        sl = [d[k] - (d[k - 1] if k > a_ else 0.0) for k in range(a_, b_)]
        if any(sl):
            chunks.append((a_, b_, sl, -sum(x * i for i, x in enumerate(sl))))
    P = [[0x0A02, 0xFF70],
         a('move', 5, imm=0.0, bits=16), a('move', 6, imm=1.0, bits=16), a('move', 7, imm=-1.0, bits=16)]
    for voice in 'AB':
        if voice == 'B':
            P += [a('move', 4, y=5)] * pad_mid               # r4 = 0: one cheap ALU instruction each
            P += [aab(a('move', 4, y=5), a('move', 3, y=5), b('max', 2, 1, 5))] * pad_mid_aab   # 3-op fillers
        pin, outs, state = SLOTS[slot][voice]
        P += [ld(1, state + 1), ld(0, pin),
              a('add', 0, 0, imm=-4.75), a('mul', 2, 0, imm=math.log(2) / 16), a('move', 3, imm=1 / 24),
              a('ymacd', 3, 2, imm=1 / 6), a('ymacd', 3, 2, imm=0.5),
              a('ymacd', 3, 2, imm=1.0), a('ymacd', 3, 2, imm=1.0)]
        P += [a('mul', 3, 3, 3)] * 4
        P += [a('mul', 3, 3, imm=K), a('add', 1, 3, 1)]
        P += wrap(1, 2)
        P += [st(1, state)]
        P += [a('mul', 0, 1, imm=N / 2.0),                  # r0 = u = N/2 * phase + N/2
              a('add', 0, 0, imm=N / 2.0),
              a('move', 4, imm=v[0])]                       # r4 = output accumulator
        for a_, b_, sl, cst in chunks if fused else ():
            P += [a('add', 1, 0, imm=-float(a_)) if a_ else a('move', 1, y=0),   # r1 = w = u - a
                  b('min', 1, 1, imm=float(b_ - a_)),       # w = min(w, L)
                  aab(a('add', 1, 1, 7), a('move', 3, imm=0.0, bits=16), b('max', 2, 1, 5))]  # r1 = w-1, r3 = 0, r2 = relu(w)
            for sk in sl:
                P.append(aab(a('add', 1, 1, 7), a('mac', 3, 2, imm=sk), b('max', 2, 1, 5)))
            P.append(a('add', 4, 3, 4))                     # r4 += chunk value
        for a_, b_, sl, cst in chunks if not fused else ():
            P += [a('add', 1, 0, imm=-float(a_)),           # r1 = w = clamp(u - a, 0, b - a)
                  b('max', 1, 1, 5),
                  b('min', 1, 1, imm=float(b_ - a_)),
                  a('move', 3, imm=cst)]                    # r3 = chunk accumulator
            for i, sk in enumerate(sl):
                if sk == 0:
                    continue
                if i == 0:
                    P.append(a('mac', 3, 1, imm=sk))        # max(w, 0) = w
                    continue
                P += [b('max', 2, 1, imm=float(i)),         # r2 = max(w, i)
                      a('mac', 3, 2, imm=sk)]               # r3 += s * r2
            P.append(a('add', 4, 3, 4))                     # r4 += chunk value
        if level != 1.0:
            P.append(a('mul', 4, 4, imm=level))
        P += [st(4, outs[0]), st(4, outs[1])]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [0x0001]]
    for ins in P:
        if dspref.template(ins) is None:
            raise AssertionError(f'no template for {ins}')
    return P


def cond_probe(path_conds, weights=(0.5, 0.25, 0.125, 0.0625)):
    """DCO2 test program (DSPTEST bench, re/dspaudio.py): for voice A (outputs [2221] [221d]) and B ([2229] [2225])
    out = noise[2200] * sum(w_j for the context conditions k_j that hold). path_conds = (conds_A, conds_B).
    Predication: `2d06 2050 1052 <2k>12 0000 <w>`: op 1 r2 = 0 (always), op 2 r1 = w only if condition k."""
    P = [[0x0A02, 0xFF70]]
    for conds, outs in zip(path_conds, ((0x2221, 0x221D), (0x2229, 0x2225))):
        P.append(a('move', 4, imm=0.0, bits=16))                      # acc = 0
        for k, wgt in zip(conds, weights):
            P += [a('move', 1, imm=0.0, bits=16),
                  [0x168 << 5 | 6, 0x2050, 0x1052, (2 * k) << 8 | 0x12, 0x0000, raw(wgt) >> 16],
                  a('add', 4, 1, 4)]                                  # acc += r1
        P += [ld(0, 0x2200), [0x0001], a('mul', 0, 0, 4), st(0, outs[0]), st(0, outs[1])]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [0x0001]]
    for ins in P:
        if dspref.template(ins) is None:
            raise AssertionError(f'no template for {ins}')
    return P


def text(P, comment=''):
    return (f'# {comment}\n' if comment else '') + '\n'.join(' '.join(f'{w:04x}' for w in ins) for ins in P) + '\n'


def simulate(words, pitch, n, slot='dco2'):
    """run the program n times (one execution per sample) in the C reference; returns the voice A outputs"""
    ins_list = [ins for _, ins in __import__('dspdis').instructions(words)]
    m = dspref.Machine()
    mem = m.mem
    mem[SLOTS[slot]['A'][0]] = mem[SLOTS[slot]['B'][0]] = pitch
    ring = [SLOTS[slot][v][2] for v in 'AB'] + [FOLD_SLOTS.get(slot, {}).get(v, (0, 0, 0, None))[3] for v in 'AB']
    pid = m.add(ins_list)
    out = []
    for _ in range(n):
        for st_ in ring:                      # ring memory: what was stored at S is read at S + 1 next sample
            if st_ is not None:
                mem[st_ + 1] = mem.get(st_)
        m.regs = [0.0] * 8
        m.run_program(pid)                    # every instruction runs, jumps followed
        out.append(mem.get(SLOTS[slot]['A'][1][0], 0.0))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['saw', 'fold', 'sim'])
    ap.add_argument('arg')
    ap.add_argument('--fs', type=float, default=96000.0)
    ap.add_argument('--level', type=float, default=1.0)
    ap.add_argument('--pitch', type=float, default=4.75)
    ap.add_argument('--n', type=int, default=400)
    ap.add_argument('--slot', default='dco2')
    ap.add_argument('-o')
    a_ = ap.parse_args()
    if a_.cmd == 'fold':
        P = fold(a_.arg, a_.fs)
        t = text(P, f'own wavefold (moving) for the {a_.arg} slot, fs {a_.fs:g} (re/dspasm.py)')
        (open(a_.o, 'w').write(t) if a_.o else print(t, end=''))
        return
    if a_.cmd == 'saw':
        P = saw(a_.arg, a_.fs, a_.level)
        t = text(P, f'own naive SAW for the {a_.arg} slot, fs {a_.fs:g}, level {a_.level:g} (re/dspasm.py)')
        (open(a_.o, 'w').write(t) if a_.o else print(t, end=''))
        return
    words = [int(x, 16) for l in open(a_.arg) for x in l.split('#')[0].split()]
    out = simulate(words, a_.pitch, a_.n, a_.slot)
    wraps = [i for i in range(1, len(out)) if out[i] < out[i - 1]]
    per = (wraps[-1] - wraps[0]) / (len(wraps) - 1) if len(wraps) > 1 else float('nan')
    f = 440 * 2 ** (a_.pitch - 4.75)
    print(f'pitch {a_.pitch}: period {per:.3f} samples (expected {a_.fs / f:.3f} at fs {a_.fs:g}); '
          f'min {min(out):+.4f} max {max(out):+.4f}; first {", ".join(f"{v:+.4f}" for v in out[:5])}')


if __name__ == '__main__':
    main()
