"""First DSP microcode disassembler: instruction boundaries + annotations.

    python3 dspdis.py prog.bin [dsp]          # dsp = unit for C0nn/A0nn names (model_spec.py)
    python3 dspdis.py --fields prog.bin       # split into ops / transfers / bridge / immediates

Known structure (NOTES.md "ISA del DSP"):
* words in execution order (dspmc.py); an instruction is `w[0] & 0x1F` words
  long (all 84 unique programs of the corpus tile exactly with this rule)
* operand words `C0nn` = parameter block id nn (+0xA0), `A0nn` = bank 5 id nn
  (+0xB0), `02nn`/`04nn`/`06nn` = local data, `22nn` = mailbox to the slot
  programs
* some instructions end in a 32-bit immediate in the DSP's own number format
  (dspnum.py): 30000000 = 1.0, 01555555 = 1/192 (envelope tick), 2775b5ea /
  45488e4f / a69d3813 = the LFO rate curve of the VST (0.466 e^(5.28 x) - 0.413).
  `imm?` marks the last two words decoded that way (a guess per instruction)
"""
import os, re, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'engine'))
import dspmc, dspnum


REFS = {0x02, 0x04, 0x06, 0x22, 0x80, 0x84, 0xA0, 0xC0}   # high bytes of memory references


def instructions(w):
    i = 0
    while i < len(w):
        n = w[i] & 0x1F
        if n == 0:
            raise ValueError(f'length 0 at word {i}')
        yield i, w[i:i + n]
        i += n


def is_move(w):
    return (w >> 4) & 0xFF == 0xFE


def fields(ins):
    """split an instruction: (ops, moves [(reg, write?, ref)], bridge words, rest)"""
    body = ins[1:]
    ms = [k for k, w in enumerate(body) if is_move(w)]
    if not ms:
        return body, [], [], []
    ops = body[:ms[0]]
    mv = [body[k] for k in ms]
    nm = len(mv)
    after = body[ms[-1] + 1:]
    # refs = the nm words that end the run of operand-looking words; bridges before them
    best = 0
    for gap in range(0, 5):
        refs = after[gap:gap + nm]
        if len(refs) == nm and all(r >> 8 in REFS or r >> 11 == 0 or r >> 8 in (0x41, 0x43, 0x82, 0x86, 0x60, 0xa1, 0xa2, 0xa3, 0xc1) for r in refs):
            best = gap
            break
    refs = after[best:best + nm]
    moves = [(w >> 12, w >> 12 >= 8, r) for w, r in zip(mv, refs)]
    return ops, moves, after[:best], after[best + nm:]


def show(ins):
    ops, moves, bridge, rest = fields(ins)
    parts = [f'{ins[0]:04x}']
    if ops:
        parts.append('ops ' + ' '.join(f'{w:04x}' for w in ops))
    for reg, wr, ref in moves:
        parts.append(f'[{ref:04x}]<-w{reg}' if wr else f'r{reg}<-[{ref:04x}]')
    if bridge:
        parts.append('br ' + ' '.join(f'{w:04x}' for w in bridge))
    if len(rest) >= 2:
        parts.append(f'imm {dspnum.decode(rest[0] << 16 | rest[1]):.7g}' + ('' if len(rest) == 2 else ' +' + ' '.join(f'{w:04x}' for w in rest[2:])))
    elif rest:
        parts.append('tail ' + ' '.join(f'{w:04x}' for w in rest))
    return '  '.join(parts)


def param_names(dsp):
    """{('P'|'B', id): 'source ids'} from the model's targets on this DSP"""
    try:
        import model_spec as M
    except Exception:
        return {}
    out = {}
    for src, targets, _e, _w in M.OUT:
        for t in targets:
            m = re.match(r'([PB])(\d):(0x[0-9a-f]+)', t)
            if m and int(m.group(2)) == dsp:
                k = (m.group(1), int(m.group(3), 16))
                out.setdefault(k, set()).add('const' if src is None else f'{src:#x}')
    return {k: '/'.join(sorted(v)) for k, v in out.items()}


def main():
    if '--fields' in sys.argv:
        sys.argv.remove('--fields')
        for a, ins in instructions(dspmc.blob_words(sys.argv[1])):
            print(f'{a:5d}  {show(ins)}')
        return
    w = dspmc.blob_words(sys.argv[1])
    dsp = int(sys.argv[2]) if len(sys.argv) > 2 else None
    names = param_names(dsp) if dsp is not None else {}
    for a, ins in instructions(w):
        notes = []
        for x in ins[1:]:
            if x >> 8 == 0xC0 and ('P', x & 0xFF) in names:
                notes.append(f'P{x & 0xff:#x}<-{names[("P", x & 0xff)]}')
            if x >> 8 == 0xA0 and ('B', x & 0xFF) in names:
                notes.append(f'B{x & 0xff:#x}<-{names[("B", x & 0xff)]}')
        if len(ins) >= 4 and ins[-2] >> 8 not in REFS:
            q = ins[-2] << 16 | ins[-1]
            notes.append(f'imm? {dspnum.decode(q):.7g}')
        print(f'{a:5d}  ' + ' '.join(f'{x:04x}' for x in ins).ljust(64) + ('; ' + ', '.join(notes) if notes else ''))


if __name__ == '__main__':
    main()
