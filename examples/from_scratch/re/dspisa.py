"""DSP instruction set (as measured on the master's DSP4, NOTES.md "Formato de instrucción"): decoder,
disassembler and simulator.

    python3 dspisa.py dis prog.bin|prog.txt          # disassemble (blob in memory order, or hex words in execution order)
    python3 dspisa.py sim "0902 3c01" [--regs ...]    # simulate one instruction on r0..r7

An instruction = header (class << 5 | length) + words. The class is a template (TEMPLATES): one letter per
body position:
    A  ALU op (16 bits)          M  MAC first word (w1)        N  MAC second word (w2)
    T  transfer                  R  reference (slot)           I  immediate bits (rest of the instruction)
Slots (for transfers XfeY: slot Y/2) are counted from the first position after the transfers: N words
first, then R words. Operand modes 1/2/3 take 16/24/32 bits from the immediate bit stream, in order.
"""
import argparse, os, struct, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

# class -> template for the body (positions 1..); '*' = repeat the last letter for the remaining words
TEMPLATES = {
    0x000: '',
    0x048: 'AI*',
    0x0A8: 'AAI*',
    0x040: 'MNI*',
    0x060: 'TR',
    0x0F0: 'TTRR',
    0x1F0: 'TTTRRR',
    0x3C0: 'TTTTRRRR',
    0x720: 'TTTTTRRRRR',
    0x722: 'TTTTTTRRRRRR',
    0x0A0: 'MTNRI*',
    0x088: 'MANI*',
    0x158: 'MTTNRRI*',
}
IMMBITS = {1: 16, 2: 24, 3: 32}
# context condition bits (condition 0 first) tested by C words; default = slave DCO slots (measured by audio)
CONTEXT_BITS = {'dco': '0011011001100110', 'master_dsp3': '0111111011101110', 'master_dsp4_efdc': '0111101111100111'}
CONTEXT = 'dco'


def raw_value(w):
    s = (w >> 29) & 3
    v = (w & 0x1FFFFFFF) / 2 ** (32 - 4 * s)
    return -v if w >> 31 else v


MEASURED = os.path.join(HERE, 'dsp_templates.txt')        # dsptemplates.py output, copied into the repo


def load_measured():
    """templates inferred from the position maps (dsptemplates.py); other lengths of a class differ in the
    number of trailing immediate words"""
    if not os.path.exists(MEASURED):
        return
    ab = {}
    abpath = os.path.join(HERE, 'dsp_units_ab.txt')             # dspslots.py ab output
    if os.path.exists(abpath):
        for line in open(abpath):
            w = line.split()
            if w and w[0].startswith('0x'):
                ab[int(w[0], 16)] = {int(x.split(':')[0]): x.split(':')[1] for x in w[3:] if ':' in x}
    for line in open(MEASURED):
        w = line.split()
        if len(w) >= 4:
            cls, t = int(w[0], 16), list(w[3])
            for p, u in ab.get(cls, {}).items():
                if u == 'B' and t[p - 1] == 'A':
                    t[p - 1] = 'B'
            core = ''.join(t).rstrip('I')
            TEMPLATES.setdefault(cls, core + 'I*')


def template(ins):
    cls, n = ins[0] >> 5, len(ins) - 1
    if not getattr(template, 'loaded', False):
        load_measured()
        template.loaded = True
    t = TEMPLATES.get(cls)
    if t is None:
        return None
    if t.endswith('*'):
        t = t[:-1] + t[-2] * max(0, n - len(t) + 1)
        t = t[:n] if len(t) >= n else t
    return t if len(t) == n else None


class Imm:
    """immediate bit stream: the words of the I positions, then (as the hardware does) whatever follows"""
    def __init__(self, words):
        self.bits = ''.join(f'{w:016b}' for w in words) + '0' * 64
        self.pos = 0

    def take(self, mode):
        n = IMMBITS[mode]
        v = int(self.bits[self.pos:self.pos + n], 2) << (32 - n)
        self.pos += n
        return v


def opnd(regs, mode, field, imm, neg_bit=True):
    """an operand: register (bit 3 = negate) or immediate"""
    if mode:
        return raw_value(imm.take(mode)), f'#{raw_value(imm.cur):.7g}' if False else None
    v = regs[field & 7]
    return (-v if neg_bit and field & 8 else v), None


def a_op(w, regs, imm, out):
    d, op, x, mode, y = w >> 12, (w >> 9) & 7, (w >> 6) & 7, (w >> 4) & 3, w & 15
    if mode:
        Y, ys = raw_value(imm.take(mode)), None
    else:
        Y = -regs[y & 7] if y & 8 else regs[y & 7]
    ys = f'r{y & 7}' if not mode else f'#{Y:.7g}'
    if y & 8 and not mode:
        ys = '-' + ys
    D, X = regs[d & 7], regs[x]
    if d >= 8:
        return f'r{d - 8} ?= (D>=8, op {op}) unknown'
    if op == 0:
        if x == 1:
            out[d] = Y; return f'r{d} = {ys}'
        if x == 2:
            out[d] = D * regs[d + 1]; out[d + 1] = 2 - regs[d + 1] * regs[y & 7]; out[y & 7] = regs[d + 1] * regs[y & 7]
            return f'newton r{d} r{d + 1} {ys}'
        if x in (6, 7):
            out[d] = D + Y; return f'r{d} = r{d} + {ys}'
        if x in (4, 5):
            return f'r{d} = r{d} + {ys} + h{x}  (hidden)'
        return 'nop'
    s = {1: f'r{d} + r{x} + {ys}', 2: f'r{x} * {ys}', 3: f'r{d} + r{x} * {ys}', 4: f'{ys} + r{x} * r{d}',
         5: f'{ys} - r{x} * r{d}', 6: f'r{x} + {ys}', 7: f'{ys} - r{x}'}[op]
    out[d] = {1: D + X + Y, 2: X * Y, 3: D + X * Y, 4: Y + X * D, 5: Y - X * D, 6: X + Y, 7: Y - X}[op]
    return f'r{d} = {s}'


def sfu(w, regs, imm, out):
    """unit B, D >= 8 and nibble f: special functions selected by the mode bits (DSP_ISA_SPEC.md §6.4)"""
    d, mode, y = (w >> 12) - 8, (w >> 4) & 3, w & 15
    Y = regs[y & 7]
    import math
    if mode == 0 and not y & 8:                     # reciprocal seed: r(d) = sign * 2^-e, r(d+1) ~ 1/m (|Y| = 2^e m)
        if Y:
            m, e = math.frexp(abs(Y))
            out[d] = math.copysign(2.0 ** -e, Y)
            out[(d + 1) & 7] = 1 / m                 # hardware seed is within ~1 %
        return f'r{d}, r{d + 1} = recip_seed(r{y & 7})'
    if mode == 1:                                   # exp split: r(d) = e^floor(Y), Y = frac(Y)
        fl = math.floor(Y)
        out[d] = math.exp(fl)
        out[y & 7] = Y - fl
        return f'r{d} = e^floor(r{y & 7}); r{y & 7} = frac(r{y & 7})'
    if mode == 2 and y & 8:
        out[d] = math.copysign(1.0, Y) if Y else 0.0
        return f'r{d} = sign(r{y & 7})'
    if mode == 3 and y & 8:
        out[d] = -1.0 if Y < 0 else 0.0
        return f'r{d} = -(r{y & 7} < 0)'
    if mode == 3 and not y & 8:
        out[d] = float(math.ceil(Y))
        return f'r{d} = ceil(r{y & 7})'
    return f'SFU:{w:04x} (r{d}: unknown)'


def b_op(w, regs, imm, out):
    """unit B: same word layout; only the ops below are known"""
    d, b, x, mode, y = w >> 12, (w >> 8) & 15, (w >> 6) & 7, (w >> 4) & 3, w & 15
    if d >= 8 and b == 0xF and (w >> 6) & 3 == 0:     # special-function unit (measured forms only)
        return sfu(w, regs, imm, out)
    if d >= 8 and b == 0xF:                          # ff70 / ef75 / ef74 (program start): no visible effect
        return f'B:{w:04x} (r{d & 7}: no visible effect, program-start form)'
    Y = raw_value(imm.take(mode)) if mode else (-regs[y & 7] if y & 8 else regs[y & 7])
    ys = f'#{Y:.7g}' if mode else ('-' if y & 8 else '') + f'r{y & 7}'
    X = regs[x]
    known = {
        (0, 0xc): (abs(X) + Y, f'|r{x}| + {ys}'), (0, 0xe): (Y - abs(X), f'{ys} - |r{x}|'),
        (1, 0x0): (min(X, Y), f'min(r{x}, {ys})'), (1, 0x4): (max(X, Y), f'max(r{x}, {ys})'),
        (1, 0x8): (abs(X), f'|r{x}|'), (1, 0xa): (abs(Y), f'|{ys}|'), (1, 0xe): (Y, ys),
    }
    k = (d >> 3, b & 0xE)                # odd b = same op with X in r4..r7 (measured: d150 d550 e190 e590)
    if k in known:
        out[d & 7], txt = known[k]
        return f'r{d & 7} = {txt}'
    return f'B:{w:04x} (r{d & 7}, op {b:x}: unknown)'


def mac(w1, w2, regs, imm, out):
    d, op1, x, ym, y = w1 >> 12, (w1 >> 9) & 7, (w1 >> 6) & 7, (w1 >> 4) & 3, w1 & 15
    z, op2, c, wm, wf = w2 >> 12, (w2 >> 9) & 7, (w2 >> 6) & 7, (w2 >> 4) & 3, w2 & 15
    Y = raw_value(imm.take(ym)) if ym else regs[y & 7]
    cm = op2 >> 1
    C = raw_value(imm.take(cm)) if cm else regs[c]
    W = raw_value(imm.take(wm)) if wm else (-regs[wf & 7] if wf & 8 else regs[wf & 7])
    Z = -regs[z & 7] if z & 8 else regs[z & 7]
    if op1 < 2:
        return 'mac nop'
    ys = f'#{Y:.7g}' if ym else f'r{y & 7}'
    cs = f'#{C:.7g}' if cm else f'r{c}'
    ws = f'#{W:.7g}' if wm else ('-' if wf & 8 else '') + f'r{wf & 7}'
    zs = ('-' if z & 8 else '') + f'r{z & 7}'
    P1 = Y * regs[x]
    P2 = C * Z
    p1s, p2s = f'r{x}*{ys}', f'{cs}*{zs}'
    if op2 == 0 and op1 >= 6:
        P1, P2, p1s, p2s = regs[x], Z, f'r{x}', zs
    r = (-P1 if op1 & 1 else P1) + (W if op1 & 2 else 0) + (P2 if op1 & 4 else 0)
    out[d & 7] = r
    terms = [('-' if op1 & 1 else '') + p1s] + ([ws] if op1 & 2 else []) + ([p2s] if op1 & 4 else [])
    return f'r{d} = ' + ' + '.join(terms).replace('+ -', '- ')


def run(ins, regs, mem=None):
    """simulate one instruction; returns (new regs, new mem, text)"""
    t = template(ins)
    if t is None:
        raise ValueError(f'unknown template for class {ins[0] >> 5:#x} len {len(ins)}')
    body = ins[1:]
    # slot array = every word after the last transfer; the first ones are MAC second words (N), the slots that
    # a memory transfer (mode fe) points to are references, the remaining slot words are the immediate bit stream
    # (in order). Transfers with another mode byte do not use a slot. (DSP_ISA_SPEC.md §4.3)
    last_t = max([i for i, k in enumerate(t) if k == 'T'], default=-1)
    if last_t >= 0:
        nN = t.count('M')
        tail = list(t[last_t + 1:])
        for i in range(len(tail)):
            tail[i] = 'N' if i < nN else 'S'
        t = t[:last_t + 1] + ''.join(tail)
    slot_words = [w for w, k in zip(body, t) if k in 'NS']
    used = set()
    for w, k in zip(body, t):
        if k == 'T' and (w >> 4) & 0xFF == 0xFE:
            used.add((w & 15) >> 1)
    nN = t.count('N')
    imm_words = [w for i, w in enumerate(slot_words) if i >= nN and i not in used] if last_t >= 0 else \
        [w for w, k in zip(body, t) if k == 'I']
    imm = Imm(imm_words)
    out, mem = list(regs), dict(mem or {})
    slots = slot_words if last_t >= 0 else [w for w, k in zip(body, t) if k in 'NR']
    n_words = iter([w for w, k in zip(body, t) if k == 'N'])
    text = []
    # predication: units = ops (A/B/M) then transfers, numbered from 1 in position order (DSP_ISA_SPEC.md §8)
    units = [i for i, k in enumerate(t) if k in 'ABM'] + [i for i, k in enumerate(t) if k == 'T']
    bits = CONTEXT_BITS[CONTEXT]
    skip = set()
    for w, k in zip(body, t):
        if k == 'C':
            cc, neg, ua, ub = (w >> 9) & 15, (w >> 8) & 1, (w >> 4) & 15, w & 15
            cond = (bits[cc] == '1') != bool(neg)
            u = ua if cond else ub
            if 1 <= u <= len(units):
                skip.add(units[u - 1])
    for pos, (w, k) in enumerate(zip(body, t)):
        if pos in skip:
            text.append(f'(skipped by C) {k}:{w:04x}')
            if k == 'M':
                next(n_words)
            continue
        if k == 'A':
            text.append(a_op(w, regs, imm, out))
        elif k == 'M':
            text.append(mac(w, next(n_words), regs, imm, out))
        elif k == 'B':
            text.append(b_op(w, regs, imm, out))
        elif k == 'C':
            cc, neg, a_, b_ = (w >> 9) & 15, (w >> 8) & 1, (w >> 4) & 15, w & 15
            text.append(f'C: if {"!" if neg else ""}cond{cc} skip unit {a_ or "-"} else skip unit {b_ or "-"}')
        elif k == 'T':
            reg, slot, mode = w >> 12, (w & 15) >> 1, (w >> 4) & 0xFF
            addr = slots[slot] if slot < len(slots) else None
            if mode != 0xFE or addr is None:
                text.append(f'xfer.{mode:02x} r{reg & 7}{"->" if reg >= 8 else "<-"} slot {slot}'
                            + (f' [{addr:04x}]' if addr is not None else '') + '  (mode unknown, no effect on r0-r7)')
                continue
            if reg < 8:
                out[reg] = mem.get(addr, 0.0); text.append(f'r{reg} <- [{addr:04x}]')
            else:
                mem[addr] = regs[reg - 8]; text.append(f'[{addr:04x}] <- r{reg - 8}')
    return out, mem, '; '.join(text)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['dis', 'sim'])
    ap.add_argument('arg')
    ap.add_argument('--regs', default='0.5,0.25,0.125,-0.375,0.0625,-0.75,0.875,-0.0625')
    a = ap.parse_args()
    if a.cmd == 'sim':
        ins = [int(x, 16) for x in a.arg.split()]
        regs = [float(x) for x in a.regs.split(',')]
        out, mem, text = run(ins, regs)
        print(text)
        print(' '.join(f'r{i}={v:+.6g}' for i, v in enumerate(out) if v != regs[i]) or 'no change')
        return
    import dspmc, dspdis
    w = dspmc.blob_words(a.arg) if a.arg.endswith('.bin') else [int(t, 16) for l in open(a.arg) for t in l.split('#')[0].split()]
    for pos, ins in dspdis.instructions(w):
        regs = [0.0] * 8
        try:
            _, _, text = run(ins, regs)
        except Exception as e:
            text = f'?? {e}'
        print(f'{pos:5d}  {" ".join(f"{x:04x}" for x in ins):48s} {text}')


if __name__ == '__main__':
    main()
