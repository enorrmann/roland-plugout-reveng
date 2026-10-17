"""SAW variants that use flags (X9bY + cmp), C selects, jumps and loops, for the audio check on the slaves
(2026-10-17). Same pitch formula as dspasm.saw(); only the phase update changes:

    sel   wrap: cmp r1 - 1 -> flags3; r1 -= 2 under a C word (runs only if r1 >= 1)
    jmp   wrap: cmp r1 - 1 -> flags3; conditional jump over r1 -= 2 (taken if r1 < 1)
    loop  increment / 4 added 4 times in a loop (backward conditional jump), then the jmp wrap

    python3 dspflagsaw.py sel|jmp|loop [-o file]
"""
import argparse, math, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import dspasm as A

NOP = [0x0001]


def imm16(v):
    return A.raw(v) >> 16


def cmp_imm(r, v):              # unit B nibble e, bits 7-6 = 00: flags of r + imm16, flags3 <- unit 1
    return [0x0D8 << 5 | 4, (8 + r) << 12 | 0xE10, 0xB9B1, imm16(v)]


def add_if(r, v):               # r = r + imm16, skipped unless cond13 (r_cmp >= 0) holds: C 1b10 = if !cond13 skip 1
    w = A.a('add', r, x=r, imm=v, bits=16)
    return [0x0B8 << 5 | 4, w[1], 0x1B10, w[2]]


def jump_unless_ge(off):        # jump `off` words if cond13 is false (C 1a10: if cond13 skip unit 1 = the jump)
    return [0x0E0 << 5 | 4, 0x0090, 0x1A10, off & 0xFFFF]


def build(slot, mode, fs=96000.0, iters=4):
    K = 2 * 440.0 / fs / (iters if mode == 'loop' else 1)
    P = [[0x0A02, 0xFF70]]
    for voice in 'AB':
        pin, outs, state = A.SLOTS[slot][voice]
        P += [A.ld(1, state + 1), A.ld(0, pin), A.a('add', 0, 0, imm=-4.75), A.a('mul', 2, 0, imm=math.log(2) / 16),
              A.a('move', 3, imm=1 / 24), A.a('ymacd', 3, 2, imm=1 / 6), A.a('ymacd', 3, 2, imm=0.5),
              A.a('ymacd', 3, 2, imm=1.0), A.a('ymacd', 3, 2, imm=1.0)]
        P += [A.a('mul', 3, 3, 3)] * 4 + [A.a('mul', 3, 3, imm=K)]
        if mode == 'loop':
            body = [A.a('add', 1, 3, 1), A.a('add', 2, 2, imm=1 / 16, bits=16), cmp_imm(2, -iters / 16), NOP]   # r2: r4 is read by the main program after the slot
            n = sum(len(i) for i in body) + 4
            P += [A.a('move', 2, imm=0.0, bits=16)] + body + [jump_unless_ge(-n)]
        else:
            P += [A.a('add', 1, 3, 1)]
        P += [cmp_imm(1, -1.0), NOP]
        if mode == 'sel':
            P += [add_if(1, -2.0)]
        else:
            sub = A.a('add', 1, 1, imm=-2.0, bits=16)
            P += [jump_unless_ge(len(sub)), sub]
        P += [A.st(1, state), A.st(1, outs[0]), A.st(1, outs[1])]
    P += [[0x0A02, 0xFF70], [0x0B02, 0x00F8], [0x0001]]
    return P


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('mode', choices=['sel', 'jmp', 'loop'])
    ap.add_argument('--slot', default='dco2')
    ap.add_argument('--iters', type=int, default=4)
    ap.add_argument('-o')
    a = ap.parse_args()
    t = A.text(build(a.slot, a.mode, iters=a.iters), f'SAW {a.mode} (flags / C / jumps) for the {a.slot} slot (re/dspflagsaw.py)')
    (open(a.o, 'w').write(t) if a.o else print(t, end=''))


if __name__ == '__main__':
    main()


def build_group(slot, xword, cc, v, fs=96000.0):
    """reference SAW (jmp wrap) whose output level is chosen by condition cc after `cmp r5 + v` stored with the
    transfer xword (e.g. 0xC9B1 = group 0?): cond true -> out = r1 / 4, false -> out = r1 (audio test of a group)"""
    P = build(slot, 'jmp', fs)
    out = []
    for ins in P[:-3]:
        if ins == A.st(1, A.SLOTS[slot]['A'][1][0]) or ins == A.st(1, A.SLOTS[slot]['B'][1][0]):
            r = 'A' if ins == A.st(1, A.SLOTS[slot]['A'][1][0]) else 'B'
            o = A.SLOTS[slot][r][1]
            out += [A.a('move', 5, imm=0.0, bits=16), A.a('move', 7, imm=0.25, bits=16),
                    [0x0D8 << 5 | 4, 0xDE10, xword, imm16(v)], NOP, NOP,
                    [0x168 << 5 | 4, 0x2041, 0x2447, cc << 9 | 0x12],      # cond ? r2 = r1 * r7 : r2 = r1
                    A.st(2, o[0]), A.st(2, o[1])]
        elif ins == A.st(1, A.SLOTS[slot]['A'][1][1]) or ins == A.st(1, A.SLOTS[slot]['B'][1][1]):
            continue
        else:
            out.append(ins)
    return out + P[-3:]
