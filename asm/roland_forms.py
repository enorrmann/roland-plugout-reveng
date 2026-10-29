"""Which op groups Roland's own programs put in one instruction (the local corpora: work/corpus_s1 for the SYSTEM-1,
work/corpus_all for the SYSTEM-8), to weigh a group before it is tried on the device (TRY_FORMS): a group Roland
runs on the SYSTEM-1 is a much safer test than one it never uses (2026-10-29: the MAC with an immediate coefficient
beside an A op hung the SYSTEM-1; Roland's SYSTEM-1 code never has it).

    python3 asm/roland_forms.py 'mac_p1:reg + mad:reg' ...    # evidence for groups
    python3 asm/roland_forms.py --list s1 [N]                 # the most used groups of a corpus
    import roland_forms; roland_forms.evidence(sig)           # -> {'s1': (exact, similar), 's8': (exact, similar)}

Exact: the same signature (asm/dspsched.py op_signature: mnemonic and operand kind). Similar: the same mnemonics
whatever the operand kinds. The index is built once from the corpora (asm/disasm.py listing of every instruction)
and kept in work/roland_forms.json (local: it comes from Roland's code, it is not copied into the repository).
"""
import collections
import glob
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
CORPORA = {'s1': os.path.join(ROOT, 'work', 'corpus_s1'), 's8': os.path.join(ROOT, 'work', 'corpus_all')}
INDEX = os.path.join(ROOT, 'work', 'roland_forms.json')
_IX = []


def _signatures(path):
    """Counter of the op signatures sharing an instruction in every program of a corpus directory"""
    sys.path.insert(0, HERE)
    sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
    import disasm
    import dspdis
    import dspsched
    import isa
    table = isa.Table.load()
    out = collections.Counter()
    for f in sorted(glob.glob(os.path.join(path, '*.txt'))):
        words = [int(x, 16) for x in open(f).read().split()]
        for _, ins in dspdis.instructions(words):
            try:
                text = disasm.disassemble_insn(table, list(ins)).text
            except Exception:                                   # noqa: BLE001  (an undecodable instruction)
                continue
            ops = [u.strip() for u in text.split(';')[0].split('|') if u.strip()]
            ops = [u for u in ops if u.split()[0] not in ('ld', 'st', 'raw', '.free', '.slot')]
            if len(ops) > 1:
                out['|'.join(dspsched.op_signature(ops))] += 1
    return out


def index(rebuild=False):
    """{corpus: {signature text 'a:reg|b:imm': count}}"""
    if _IX and not rebuild:
        return _IX[0]
    if os.path.exists(INDEX) and not rebuild:
        ix = json.load(open(INDEX))
    else:
        ix = {name: dict(_signatures(p)) for name, p in CORPORA.items() if os.path.isdir(p)}
        os.makedirs(os.path.dirname(INDEX), exist_ok=True)
        json.dump(ix, open(INDEX, 'w'), indent=0)
    _IX[:] = [ix]
    return ix


def _mnems(sig):
    return tuple(sorted(t.split(':')[0] for t in sig))


def evidence(sig):
    """sig: a tuple of 'mnemonic:kind' (op_signature) -> {corpus: (exact count, similar count)}"""
    ix = index()
    key, mn = '|'.join(sorted(sig)), _mnems(sig)
    out = {}
    for name, sigs in ix.items():
        exact = sigs.get(key, 0)
        similar = sum(c for s, c in sigs.items() if _mnems(s.split('|')) == mn)
        out[name] = (exact, similar)
    return out


def text(sig):
    """'Roland S1 x3 (similar x5), S8 -' style summary"""
    ev = evidence(sig)
    parts = []
    for name in ('s1', 's8'):
        if name not in ev:
            continue
        e, s = ev[name]
        parts.append(f'{name.upper()} ' + (f'x{e}' if e else '-') + (f' (similar x{s})' if s > e else ''))
    return 'Roland ' + ', '.join(parts)


def main(argv):
    if argv[:1] == ['--rebuild']:
        index(rebuild=True)
        argv = argv[1:]
    if argv[:1] == ['--list']:
        name = argv[1] if len(argv) > 1 else 's1'
        n = int(argv[2]) if len(argv) > 2 else 40
        for s, c in sorted(index().get(name, {}).items(), key=lambda kv: -kv[1])[:n]:
            print(f'{c:6d}  {" + ".join(s.split("|"))}')
        return
    for g in argv:
        sig = tuple(sorted(t.strip() for t in g.split('+')))
        print(f'{" + ".join(sig)}: {text(sig)}')


if __name__ == '__main__':
    main(sys.argv[1:])
