"""The SYSTEM-1 access rules, checked on scheduled programs (one instruction per line, units joined by ' | ').

Breaking one of them hung the device (docs/examples_system_1_jx_README.md §5, measured by on-device bisection):
    R2  bank 5: at most 1 load and 1 store per instruction
    R3  bank 0: at most 2 loads and 2 stores per instruction
    R4  bank 6: at most 2 loads per instruction
    R5  one bank-5 access per instruction (load or store)                         [our rule, inferred]
    R6  at most 2 loads of banks 5 + 6 + IOMD per instruction (the S1 never has more)  [inferred]
    R7  a slave DSP stores to bank 5 only in A100..A1FF (A000..A0FF is what the ARM writes)
    R8  ops sharing an instruction: only the groups proven on the device (asm/proven_forms.txt)
and, as a warning: an instruction shape (accesses per bank, number of ops) that is not part of a shape of a program
that ran on the device (PROVEN_PROGRAMS: C-S1V8, C-S1SY13).

    import s1rules
    s1rules.check('s1m0', lines)              -> list of warnings; SystemExit on a broken rule
    python3 asm/s1rules.py FILE.s ...           the same from the command line
"""
import collections
import glob
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
# programs in git that ran on the device (C-S1V8, C-S1SY13): not the generated files of examples whose build options
# change (MUTABLE-SYSTEM's s1m*.s are whatever its last build made)
PROVEN_PROGRAMS = ['examples/system_1/s1m*.s', 'examples/system_1_synth/s1m*.s']
_ADDR = re.compile(r'\[([0-9a-fA-F]{4})\]')


def bank(addr):
    return {0x0: '0', 0x1: '1', 0x3: 'io', 0x5: '5', 0x6: '6', 0x7: 'e'}.get(addr >> 13, f'{addr >> 13:x}')


def units(line):
    return [u.strip() for u in line.split(';')[0].split('|') if u.strip()]


def shape(line):
    """(sorted accesses like 'ld5', number of ops) of one instruction"""
    acc, ops = [], 0
    for u in units(line):
        m = u.split()[0]
        if m in ('ld', 'st'):
            a = _ADDR.search(u)
            acc.append(m + (bank(int(a.group(1), 16)) if a else '?'))
        else:
            ops += 1
    return tuple(sorted(acc)), ops


def op_signature(line):
    import dspsched
    ops = [u for u in units(line) if u.split()[0] not in ('ld', 'st')]
    return dspsched.op_signature(ops) if len(ops) > 1 else None


_PROVEN = []


def proven_shapes():
    if not _PROVEN:
        s = set()
        for pat in PROVEN_PROGRAMS:
            for p in glob.glob(os.path.join(ROOT, pat)):
                for line in open(p):
                    if line.strip() and not line.startswith(';'):
                        s.add(shape(line))
        _PROVEN.append(s)
    return _PROVEN[0]


def covered(sh):
    """the shape is part of a proven one: no more accesses of any kind, no more ops"""
    acc, ops = collections.Counter(sh[0]), sh[1]
    return any(ops <= p_ops and not (acc - collections.Counter(p_acc)) for p_acc, p_ops in proven_shapes())


def check(name, lines, slave=True, proven_ops=None):
    """SystemExit listing every broken rule; returns warnings (shapes never run on the device)"""
    sys.path.insert(0, HERE)
    import dspsched
    proven_ops = proven_ops if proven_ops is not None else dspsched.load_proven()
    errors, warnings, new = [], [], collections.Counter()
    code = [x for x in lines if x.strip() and not x.startswith(';')]
    for k, line in enumerate(code):
        n = collections.Counter()
        for u in units(line):
            m = u.split()[0]
            if m in ('ld', 'st'):
                a = _ADDR.search(u)
                if a:
                    addr = int(a.group(1), 16)
                    n[(m, bank(addr))] += 1
                    if slave and m == 'st' and bank(addr) == '5' and not 0xA100 <= addr <= 0xA1FF:
                        errors.append(f'R7 instruction {k}: a slave DSP stores to {addr:04x} (only A100..A1FF): {line}')
        broken = []
        if n[('ld', '5')] > 1 or n[('st', '5')] > 1:
            broken.append('R2')
        if n[('ld', '0')] > 2 or n[('st', '0')] > 2:
            broken.append('R3')
        if n[('ld', '6')] > 2:
            broken.append('R4')
        if n[('ld', '5')] + n[('st', '5')] > 1:
            broken.append('R5')
        if n[('ld', '5')] + n[('ld', '6')] + n[('ld', 'io')] > 2:
            broken.append('R6')
        sig = op_signature(line)
        if sig is not None and sig not in proven_ops:
            broken.append('R8')
        errors += [f'{r} instruction {k}: {line}' for r in broken]
        sh = shape(line)
        if not covered(sh):
            new[sh] += 1
    for (acc, ops), c in sorted(new.items()):
        warnings.append(f'{name}: {c} x {" + ".join(acc) or "no transfer"} with {ops} op(s): a shape no program that '
                        'ran on the device has (allowed by the rules)')
    if errors:
        raise SystemExit(f'{name}: SYSTEM-1 access rules broken (asm/s1rules.py):\n  ' + '\n  '.join(errors[:20])
                         + (f'\n  ... {len(errors) - 20} more' if len(errors) > 20 else ''))
    return warnings


def main(argv):
    bad = 0
    for p in argv:
        try:
            for w in check(os.path.basename(p), open(p).read().splitlines(), slave='mm' not in os.path.basename(p)):
                print('warning:', w)
            print(f'{p}: rules ok')
        except SystemExit as e:
            print(e)
            bad = 1
    return bad


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
