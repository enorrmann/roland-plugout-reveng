"""Minimal assembler for the established DSP subset (DSP_ISA_SPEC.md) + our own SAW oscillator.

    python3 dspasm.py saw dco2 [--fs 48000] [-o prog.txt]   # oscillator for the DCO2 (or dco1) slot
    python3 dspasm.py sim prog.txt --pitch 4.75 [--n 400]    # run it in dspisa's simulator, print period

Encoders (each returns one instruction as a list of 16-bit words):
    a(op, d, x, y=None, imm=None)   unit A op (class 0x048); y = register (negative int = negated) or imm (float)
    b(op, d, x, y)                  unit B op (class 0x050), d >= 8 forms (min = op 0, max = op 2), x in r0..r3
    ld(r, addr), st(r, addr)        transfer with one reference (class 0x060)
Every instruction is checked by decoding it again with dspisa.
"""
import argparse, math, os, struct, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dspisa   # noqa: E402

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


def b(name, d, x, y):
    """unit B, measured forms only: min / max into rd (d = 0..7), x in r0..r3, y register"""
    op = {'min': 0, 'max': 2}[name]
    if not 0 <= x <= 3:
        raise ValueError('unit B measured with X in r0..r3 only')
    return [0x050 << 5 | 2, (8 + d) << 12 | op << 9 | x << 6 | y]


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
        if dspisa.template(ins) is None:
            raise AssertionError(f'no template for {ins}')
    return P


def text(P, comment=''):
    return (f'# {comment}\n' if comment else '') + '\n'.join(' '.join(f'{w:04x}' for w in ins) for ins in P) + '\n'


def simulate(words, pitch, n, slot='dco2'):
    """run the program n times (one execution per sample) in dspisa; returns the voice A outputs"""
    ins_list = [ins for _, ins in __import__('dspdis').instructions(words)]
    mem = {SLOTS[slot]['A'][0]: pitch, SLOTS[slot]['B'][0]: pitch}
    out = []
    for _ in range(n):
        regs = [0.0] * 8
        for ins in ins_list:
            if tuple(ins) in ((0x0A02, 0xFF70), (0x0B02, 0x00F8), (0x0001,)):
                continue                      # program start / end words (unit-B / C forms we do not simulate)
            regs, mem, _ = dspisa.run(list(ins), regs, mem)
        out.append(mem.get(SLOTS[slot]['A'][1][0], 0.0))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['saw', 'sim'])
    ap.add_argument('arg')
    ap.add_argument('--fs', type=float, default=96000.0)
    ap.add_argument('--level', type=float, default=1.0)
    ap.add_argument('--pitch', type=float, default=4.75)
    ap.add_argument('--n', type=int, default=400)
    ap.add_argument('--slot', default='dco2')
    ap.add_argument('-o')
    a_ = ap.parse_args()
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
