"""Memory references of a DSP program and their relocation (same decoder as the emulator: emu/dspref.layout, the C decoder).

    import dspreloc as R
    p = R.Prog(words, 'MAIN2')          # words in execution order
    p.refs                              # [Ref(ins, op 'ld'/'st', reg, addr, pos)]; pos = index of the address word
    p.footprint()                       # {bank: (loads, stores)} of literal addresses
    p.ring_reach(0x0)                   # largest k such that a load at A reads what the program stored at A-k
    q = p.relocate(lambda r: new_addr)  # new word list: only the address words change (same lengths, jumps kept)

Every memory transfer of the JX-3P programs is a literal one (mode fe, address in a slot word); other transfer modes
(X28Y ring-relative, bank-5 indexed, hidden registers) are reported by `Prog.other` and refused by relocate().
"""
import os
import sys
from dataclasses import dataclass

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 'emu'))
import dspdis  # noqa: E402
import dspref  # noqa: E402

BANKS = {0x0: 'b0', 0x2: 'b1', 0x6: 'iomd', 0x8: 'mxm', 0xA: 'b5', 0xC: 'b6', 0xE: 'b7'}


@dataclass
class Ref:
    ins: int        # instruction index
    op: str         # 'ld' / 'st'
    reg: int
    addr: int
    pos: int        # word index of the address inside the program


class Prog:
    def __init__(self, words, name=''):
        self.name, self.words = name, list(words)
        self.refs, self.other, self.starts = [], [], []
        for k, (off, ins) in enumerate(dspdis.instructions(self.words)):
            self.starts.append(off)
            try:
                L = dspref.layout(list(ins))
            except ValueError:
                self.other.append((k, 'undecodable'))
                continue
            slot_pos = [i for i, t in enumerate(L.t) if t in 'NS'] if any(t == 'T' for t in L.t) else \
                [i for i, t in enumerate(L.t) if t in 'NR']
            for i, (w, t) in enumerate(zip(L.body, L.t)):
                if t != 'T':
                    continue
                mode = (w >> 4) & 0xFF
                if mode == 0x9B:                       # condition flags: no memory
                    continue
                if mode != 0xFE:
                    self.other.append((k, f'transfer mode {mode:02x}'))
                    continue
                s = (w & 15) >> 1
                pos = off + 1 + slot_pos[s]
                self.refs.append(Ref(k, 'st' if w >> 12 >= 8 else 'ld', (w >> 12) & 7, self.words[pos], pos))

    def footprint(self):
        out = {}
        for r in self.refs:
            b = out.setdefault(r.addr >> 12, (set(), set()))
            b[0 if r.op == 'ld' else 1].add(r.addr)
        return out

    def ring_reach(self, bank):
        """largest k >= 0 over the loads of `bank` such that the nearest store at or below the load address is k words
        lower (k = 0: stored in the same sample; k = 1: the usual one-sample delay); None if a load has no store
        below it (it reads something the program never wrote)"""
        ld, st = self.footprint().get(bank, (set(), set()))
        worst = 0
        for a in ld:
            ks = [a - s for s in st if 0 <= a - s < 0x800]
            if not ks:
                return None
            worst = max(worst, min(ks))
        return worst

    def relocate(self, fn):
        """fn(Ref) -> new address (or the same). Two transfers sharing one address word must agree."""
        if self.other:
            raise ValueError(f'{self.name}: non-literal transfers {self.other[:3]}')
        w = list(self.words)
        done = {}
        for r in self.refs:
            new = fn(r)
            if r.pos in done and done[r.pos] != new:
                raise ValueError(f'{self.name}: word {r.pos} ({r.addr:04x}) mapped to {done[r.pos]:04x} and {new:04x}')
            done[r.pos] = new
            w[r.pos] = new
        return w


def concat(*progs):
    """one program from several: each one's end (0b02 00f8 0001, return to the scheduler) is dropped but the last.
    Jumps are relative to the instruction end, so they keep their targets."""
    out = []
    for i, w in enumerate(progs):
        w = list(w)
        if i < len(progs) - 1:
            if w[-3:] != [0x0B02, 0x00F8, 0x0001]:
                raise ValueError('program does not end with 0b02 00f8 0001')
            w = w[:-3]
        out += w
    return out
