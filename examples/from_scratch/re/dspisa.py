"""DSP instruction set (as measured on the master's DSP4, NOTES.md "Formato de instrucción"): decoder,
disassembler and simulator.

    python3 dspisa.py dis prog.bin|prog.txt          # disassemble (blob in memory order, or hex words in execution order)
    python3 dspisa.py sim "0902 3c01" [--regs ...]    # simulate one instruction on r0..r7
    python3 dspisa.py cov work/corpus                # decoding coverage over a directory of .bin programs

An instruction = header (class << 5 | length) + words. The class is a template (TEMPLATES): one letter per
body position:
    A  ALU op (16 bits)          M  MAC first word (w1)        N  MAC second word (w2)
    T  transfer                  R  reference (slot)           I  immediate bits (rest of the instruction)
Slots (for transfers XfeY: slot Y/2) are counted from the first position after the transfers: N words
first, then R words. Operand modes 1/2/3 take 16/24/32 bits from the immediate bit stream: MAC operands in order,
every A/B op from the start of the stream (two A/B ops with immediates share the same bits, measured 2026-10-16).
"""
import argparse, math, os, struct, sys

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
# the condition bits are FLAGS (2026-10-17): X9bY (X = 9/a/b -> group 1/2/3, conds 4g..4g+3) stores the flags of
# the result of unit Y of its instruction; they persist (also across samples) until the next X9bY on that group.
# Per group: c0 = !Z, c1 = !N, c2 = !V (|result| >= 512), c3 = N or Z. Group 0 (c9bY) did not change on the master.
FLAGS = None
JUMP = None                      # set by run() when a jump executes (offset in words from the instruction end)
PENDING = {}                     # one instruction of latency: the next instruction still sees the old flags
FLAG_GROUP = {0x9: 1, 0xA: 2, 0xB: 3, 0xC: 0}
_res = [None]                    # (value, Z) of the op just simulated, for X9bY


def flag_bits(v, z):
    n, ov = v < 0, abs(v) >= 512
    return ''.join('1' if b else '0' for b in (not z, not n, not ov, n or z))
# hidden registers (DSP_ISA_SPEC.md §9.0), loaded by C-position words 40TY: h4 = delay base (f6c, A op 0 sub 4),
# h5 = tap-result address (A op 0 sub 5), h6 = index of mode-28 transfers, h7 = unknown. Defaults: master DSP4.
HIDDEN = {4: 288.0, 5: 0x105 / 16384, 6: 0.0, 7: 0.0}


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
    if cls == 0x060 and n == 1:          # `0c02 X28Y` / `0c02 3a90`: a transfer without a reference (measured)
        return 'T'
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
    if d >= 8:                    # r(D-8) = X + Y + r[op] (exact in the single-op class, DSP_ISA.md §4)
        out[d - 8] = X + Y + regs[op]
        _res[0] = (out[d - 8], out[d - 8] == 0)
        return f'r{d - 8} = r{x} + {ys} + r{op}'
    if op == 0:
        if x == 1:
            out[d] = Y; return f'r{d} = {ys}'
        if x == 2:
            out[d] = D * regs[d + 1]; out[d + 1] = 2 - regs[d + 1] * regs[y & 7]; out[y & 7] = regs[d + 1] * regs[y & 7]
            return f'newton r{d} r{d + 1} {ys}'
        if x in (6, 7):
            out[d] = D + Y; return f'r{d} = r{d} + {ys}'
        if x in (4, 5):
            out[d] = D + Y + HIDDEN[x]
            return f'r{d} = r{d} + {ys} + h{x}'
        return 'nop'
    s = {1: f'r{d} + r{x} + {ys}', 2: f'r{x} * {ys}', 3: f'r{d} + r{x} * {ys}', 4: f'{ys} + r{x} * r{d}',
         5: f'{ys} - r{x} * r{d}', 6: f'r{x} + {ys}', 7: f'{ys} - r{x}'}[op]
    out[d] = {1: D + X + Y, 2: X * Y, 3: D + X * Y, 4: Y + X * D, 5: Y - X * D, 6: X + Y, 7: Y - X}[op]
    if abs(out[d]) >= 512:        # wraps: magnitude mod 512, sign kept (measured r2 + r2: 600 -> 88, 1022 -> 510)
        _res[0] = (out[d], False)
        out[d] = math.copysign(abs(out[d]) % 512, out[d])
        _res[0] = (_res[0][0], out[d] == 0)
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
    if mode == 1 and not y & 8:                     # sqrt split: |Y| = m 4^-k, m in [1, 4) (DSP_ISA_SPEC.md §6.4)
        if Y:
            m, k = abs(Y), 0
            while m >= 4: m, k = m / 4, k - 1
            while m < 1: m, k = m * 4, k + 1
            out[y & 7], out[(d + 1) & 7], out[7] = m, 1 / math.sqrt(m), math.copysign(2.0 ** -k, Y)
        return f'r{y & 7}, r{d + 1}, r7 = sqrt_split(r{y & 7})  (m, ~1/sqrt m, sign 2^-k)'
    if mode == 2 and not y & 8:                     # log2 split: Y = m 2^e, m in [0.75, 1.5); Y < 0: e = 9, Y/512
        if Y > 0:
            e = math.floor(math.log2(Y / 0.75))
            out[d], out[y & 7] = float(e), Y / 2.0 ** e
        elif Y < 0:
            out[d], out[y & 7] = 9.0, Y / 512
        return f'r{d}, r{y & 7} = log2_split(r{y & 7})  (e, m)'
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
    if d >= 8 and b == 0xF and (w >> 6) & 3 == 1:    # bits 7-6 = 01: sub-op in bits 5-4 (measured 2026-10-17)
        sub, yy = (w >> 4) & 3, w & 15
        if sub == 0:                                  # f4Y: rd = r(Y & 7) (sign bit ignored)
            out[d & 7] = regs[yy & 7]
            return f'r{d & 7} = r{yy & 7}  (f4)'
        if sub == 2 and yy >= 12:                     # f6Y: rd = (rd + h(Y-8)) mod 512 (f6c = delay base h4)
            out[d & 7] = (regs[d & 7] + HIDDEN[yy - 8]) % 512
            return f'r{d & 7} = (r{d & 7} + h{yy - 8}) mod 512'
        if sub == 1 and yy == 12:                     # f5c: delay-ring write position (counts down 1/16384 per sample)
            return f'r{d & 7} = delay_write_pos  (time-varying, mod 512)'
        if sub == 1:
            out[d & 7] = 0.0
            return f'r{d & 7} = counter f5{yy:x}  (reads 0 on the master; slave meaning unknown)'
        if sub == 3:
            return f'B:{w:04x} (r{d & 7}: no visible effect, program-start form)'
    if d >= 8 and b == 0xF:                          # ff70 / ef75 / ef74 (program start): no visible effect
        return f'B:{w:04x} (r{d & 7}: no visible effect, program-start form)'
    Y = raw_value(imm.take(mode)) if mode else (-regs[y & 7] if y & 8 else regs[y & 7])
    ys = f'#{Y:.7g}' if mode else ('-' if y & 8 else '') + f'r{y & 7}'
    X = regs[x]
    if d >= 8 and b == 0xE:              # 2026-10-17, bits 7-6: 0 = flags of r(d) + Y only (CMP/TST, no write),
        D = regs[d & 7]                  # 1 = r(d) = Y, 3 = r(d) = r(d) + Y, 2 = writes r(d) and r(d+1) (unknown)
        sub = (w >> 6) & 3
        if sub == 0:
            _res[0] = (D + Y, D + Y == 0)
            return f'cmp r{d & 7} + {ys}'
        if sub == 1:
            out[d & 7] = Y
            return f'r{d & 7} = {ys}'
        if sub == 3:
            out[d & 7] = D + Y
            return f'r{d & 7} = r{d & 7} + {ys}'
        return f'B:{w:04x} (r{d & 7}, r{(d + 1) & 7}: unknown)'
    if d >= 8 and b & 0xE in (0, 2, 4, 6):   # min/max: Z = operands equal, N = sign of the result (measured max)
        _res[0] = None
    known = {
        (0, 0xc): (abs(X) + Y, f'|r{x}| + {ys}'), (0, 0xe): (Y - abs(X), f'{ys} - |r{x}|'),
        (1, 0x0): (min(X, Y), f'min(r{x}, {ys})'), (1, 0x4): (max(X, Y), f'max(r{x}, {ys})'),
        (1, 0x2): (min(-X, Y), f'min(-r{x}, {ys})'), (1, 0x6): (max(-X, Y), f'max(-r{x}, {ys})'),   # 2026-10-16
        (1, 0x8): (abs(X), f'|r{x}|'), (1, 0xa): (abs(Y), f'|{ys}|'), (1, 0xe): (Y, ys),
    }
    k = (d >> 3, b & 0xE)                # odd b = same op with X in r4..r7 (measured: d150 d550 e190 e590)
    if k == (1, 0xC):                    # frexp: X = m 2^e, m in [0.5, 1): r(d) = m, rY = e (0 -> 0, -31) [2026-10-16]
        import math
        m, e = math.frexp(X) if X else (0.0, -31)
        out[d & 7] = m
        if not mode:
            out[y & 7] = float(e)
        return f'r{d & 7}, {ys} = frexp(r{x})  (m in [0.5, 1), exponent)'
    if k in known:
        out[d & 7], txt = known[k]
        if k[0] == 1 and k[1] in (0, 2, 4, 6):
            _res[0] = (out[d & 7], (-X if k[1] & 2 else X) == Y)
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
    if op2 == 3:
        return f'r{d & 7} = mac op2=011 (unknown)'
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
    out[d & 7] = r / 2 if d >= 8 else r            # D >= 8: r(D-8) = result / 2 (measured)
    terms = [('-' if op1 & 1 else '') + p1s] + ([ws] if op1 & 2 else []) + ([p2s] if op1 & 4 else [])
    body = ' + '.join(terms).replace('+ -', '- ')
    return f'r{d - 8} = ({body}) / 2' if d >= 8 else f'r{d} = {body}'


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
    # jumps (2026-10-17): a word 009Y in a C position is a unit: jump by the signed 16-bit word number Y >> 1 of
    # the immediate stream / free slots, counted in words from the end of the instruction
    t = ''.join('J' if k == 'C' and w & 0xFFF0 == 0x0090 else k for w, k in zip(body, t)) + t[len(body):]
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
    units = [i for i, k in enumerate(t) if k in 'ABMJ'] + [i for i, k in enumerate(t) if k == 'T']
    global FLAGS
    if FLAGS is None:
        FLAGS = CONTEXT_BITS[CONTEXT]
    bits = FLAGS
    unit_res = {}
    upd = {}
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
        _res[0] = None
        if k == 'A':
            imm.pos = 0           # every A/B op reads its immediate from the start of the stream (measured
            text.append(a_op(w, regs, imm, out))   # 2026-10-16: two ops with imm16/imm32 get the same bits)
        elif k == 'M':
            text.append(mac(w, next(n_words), regs, imm, out))
        elif k == 'B':
            imm.pos = 0
            text.append(b_op(w, regs, imm, out))
        if k in 'ABM':
            if _res[0] is None:          # flags of the op's destination r(D & 7) (MAC measured 2026-10-17)
                dest = (w >> 12) & 7
                _res[0] = (out[dest], out[dest] == 0)
            unit_res[units.index(pos) + 1] = _res[0]
        elif k == 'J':
            n = imm_words[(w & 15) >> 1] if (w & 15) >> 1 < len(imm_words) else 0
            n = n - 0x10000 if n & 0x8000 else n
            global JUMP
            JUMP = n
            text.append(f'jump {n:+d} words')
        elif k == 'C' and w >> 8 == 0x40:     # 40TY: hidden register load (DSP_ISA_SPEC.md §9.0)
            tg, m, y = 4 + ((w >> 6) & 3), (w >> 4) & 3, w & 15
            if m == 0:
                HIDDEN[tg] = regs[y & 7] % 512
                text.append(f'h{tg} = r{y & 7} mod 512')
            elif m == 3:
                text.append(f'h{tg} = imm32 (reads 0)')
            else:
                rest = list(body[pos + 1 + y:]) + [0, 0]
                v = (rest[0] << 16 | rest[1]) >> (32 - IMMBITS[m])
                HIDDEN[tg] = v / 16384
                text.append(f'h{tg} = {v:#x}/16384 = {v / 16384:g}')
        elif k == 'C':
            cc, neg, a_, b_ = (w >> 9) & 15, (w >> 8) & 1, (w >> 4) & 15, w & 15
            text.append(f'C: if {"!" if neg else ""}cond{cc} skip unit {a_ or "-"} else skip unit {b_ or "-"}')
        elif k == 'T':
            reg, slot, mode = w >> 12, (w & 15) >> 1, (w >> 4) & 0xFF
            addr = slots[slot] if slot < len(slots) else None
            if mode == 0x28:                  # indexed: bank0[floor(h6 * 16384) + Y], ring-relative (§9.0)
                ia = (int(HIDDEN[6] * 16384) + (w & 15)) & 0x7FF
                if reg < 8:
                    out[reg] = mem.get(ia, 0.0); text.append(f'r{reg} <- [h6+{w & 15}]')
                else:
                    mem[ia] = regs[reg - 8]; text.append(f'[h6+{w & 15}] <- r{reg - 8}')
                continue
            if mode == 0x9B:                  # set flags of group X from unit Y (2026-10-17)
                g, u = FLAG_GROUP.get(reg), w & 15
                if g is None or u not in unit_res:
                    text.append(f'flags.{reg:x} <- unit {u}  (no effect measured)')
                else:
                    upd[g] = flag_bits(*unit_res[u])
                    text.append(f'flags{g} <- unit {u}')
                continue
            if mode == 0xA9:                  # delay-tap request at address rX, results to bank 2 [h5] (§9.1)
                text.append(f'tap_request(r{reg & 7}) -> bank2[h5..h5+2]')
                continue
            if mode != 0xFE or addr is None:
                text.append(f'xfer.{mode:02x} r{reg & 7}{"->" if reg >= 8 else "<-"} slot {slot}'
                            + (f' [{addr:04x}]' if addr is not None else '') + '  (mode unknown, no effect on r0-r7)')
                continue
            if reg < 8:
                out[reg] = mem.get(addr, 0.0); text.append(f'r{reg} <- [{addr:04x}]')
            else:
                mem[addr] = regs[reg - 8]; text.append(f'[{addr:04x}] <- r{reg - 8}')
    global PENDING
    f = list(FLAGS)
    for g, b4 in PENDING.items():
        f[4 * g:4 * g + 4] = b4
    FLAGS, PENDING = ''.join(f), upd
    return out, mem, '; '.join(text)


UNDECODED = ('unknown', '??', 'no visible effect', 'reads 0')


def coverage(directory):
    """fraction of corpus instructions whose disassembly has no undecoded part; top undecoded forms"""
    import collections, glob, dspmc, dspdis
    tot = ok = 0
    bad = collections.Counter()
    for f in sorted(glob.glob(os.path.join(directory, '*.bin'))):
        for pos, ins in dspdis.instructions(dspmc.blob_words(f)):
            tot += 1
            try:
                _, _, text = run(ins, [0.0] * 8)
            except Exception as e:
                text = f'?? {e}'
            parts = [p for p in text.split('; ') if any(u in p for u in UNDECODED)]
            if parts:
                for p in parts:
                    key = p.split('(')[0].strip()
                    key = key if key.startswith('B:') else key.split(':')[0][:40]
                    bad[key] += 1
            else:
                ok += 1
    print(f'{ok} / {tot} instructions fully decoded ({100 * ok / tot:.1f} %)')
    for k, n in bad.most_common(25):
        print(f'  {n:6d}  {k}')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['dis', 'sim', 'cov'])
    ap.add_argument('arg')
    ap.add_argument('--regs', default='0.5,0.25,0.125,-0.375,0.0625,-0.75,0.875,-0.0625')
    a = ap.parse_args()
    if a.cmd == 'cov':
        coverage(a.arg)
        return
    if a.cmd == 'sim':
        ins = [int(x, 16) for x in a.arg.split()]
        regs = [float(x) for x in a.regs.split(',')]
        out, mem, text = run(ins, regs)
        print(text)
        print(' '.join(f'r{i}={v:+.6g}' for i, v in enumerate(out) if v != regs[i]) or 'no change')
        run([0x0001], out)              # one more instruction (a no-op) for the flag latency
        print('flags (cond 0..15):', ' '.join(FLAGS[i:i + 4] for i in range(0, 16, 4)))
        return
    import dspmc, dspdis
    w = dspmc.blob_words(a.arg) if a.arg.endswith('.bin') else [int(t, 16) for l in open(a.arg) for t in l.split('#')[0].split()]
    for pos, ins in dspdis.instructions(w):
        regs = [0.0] * 8
        try:
            _, _, text = run(ins, regs)
        except Exception as e:
            text = f'?? {e}'
        global JUMP
        if JUMP is not None:
            text += f'  -> {pos + len(ins) + JUMP}'
            JUMP = None
        print(f'{pos:5d}  {" ".join(f"{x:04x}" for x in ins):48s} {text}')


if __name__ == '__main__':
    main()
