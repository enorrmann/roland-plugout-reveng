"""Check that our own DSP microcode uses the optimized forms (docs/MICROCODE_OPTIMIZATION.md); warn where it does not.

    python3 tools/check_optimized.py FILE...          .s (our assembly) or .hex / .txt (program words, '#' comments)
    python3 tools/check_optimized.py --strict FILE... exit code 1 when there is any warning
    python3 tools/check_optimized.py -q FILE...       one summary line per file

Same DSP ISA on the SYSTEM-1 and the SYSTEM-8, so the same checks for both. Every warning is an instruction that
costs a cycle of the per-sample budget for nothing:

  manual-wrap   a phase wrap / step built from `mul rX, rX, #256` + `min rX, rX, ..` pairs: one ph_wrap (or
                ph_clamp / ph_tri) does it (Roland: `ph_wrap rD, r3, rY`, SH-2 and the SYSTEM-8 oscillators)
  filler        `mov rN, rN` alone in an instruction: a load delay that packing did not absorb
  transfer-only an instruction with loads / stores and no op: the transfers fit into an op's instruction (pack())
  fusable       two adjacent single-op instructions, independent (the second does not read or write the first's
                result) and encodable as one instruction `op1 | op2` (A+B, A+A, as Roland). An estimate: the
                assembler decides the encoding; the cost of each fused class is in docs/MICROCODE_OPTIMIZATION.md

Build scripts run it on the programs they generate (examples/system_1_synth/build.sh, ...).
"""
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
import asm      # noqa: E402
import disasm   # noqa: E402
import isa      # noqa: E402

XFER = re.compile(r'^(ld|st)\s')
REG = re.compile(r'\br(\d)\b')
_TABLE = None
_ASM = None


def table():
    global _TABLE
    if _TABLE is None:
        _TABLE = isa.Table.load()
    return _TABLE


def assembles(text):
    global _ASM
    if _ASM is None:
        _ASM = asm.Assembler()
    try:
        return asm.Assembler(_ASM.table).assemble(text + '\n').ok
    except Exception:
        return False


def clean(unit):
    """'#0.5@2:24' -> '#0.5' (disassembler immediate annotations)"""
    return re.sub(r'@\d+:\d+', '', unit).strip()


def instructions_of(path):
    """[[unit text]] per instruction"""
    text = open(path).read()
    if path.endswith('.s'):
        lines = text.splitlines()
    else:
        words = [int(t, 16) for line in text.splitlines() for t in line.split('#')[0].split()]
        lines, _ = disasm.program(words, table(), os.path.basename(path))
        lines = lines.splitlines()
    out = []
    for line in lines:
        line = line.split(';')[0].strip()
        if not line or line.startswith('.') or line.startswith('raw'):
            continue
        out.append([clean(u) for u in line.split('|') if u.strip()])
    return out


def ops(ins):
    return [u for u in ins if not XFER.match(u)]


def dest_srcs(op):
    regs = [int(r) for r in REG.findall(op)]
    if not regs:
        return None, set()
    mn = op.split()[0]
    d = regs[0]
    srcs = set(regs[1:])
    if mn in ('mad', 'ymacd') or mn.startswith('mac') or mn == 'add':      # accumulate: the destination is read too
        srcs.add(d)
    return d, srcs


def check(path):
    ins = instructions_of(path)
    w = {'manual-wrap': [], 'filler': [], 'transfer-only': [], 'fusable': []}
    for k, i in enumerate(ins):
        o = ops(i)
        if not o:
            w['transfer-only'].append(k)
        elif len(i) == 1 and re.fullmatch(r'mov r(\d), r\1', o[0]):
            w['filler'].append(k)
    # manual wrap: (mul rX, rX, #256 ; min rX, rX, ..) twice in a row
    seq = [ops(i)[0] if len(ops(i)) == 1 else '' for i in ins]
    k = 0
    while k + 3 < len(seq):
        m = re.fullmatch(r'mul r(\d), r\1, #256(\.0)?', seq[k])
        if m and seq[k + 1].startswith(f'min r{m.group(1)}, r{m.group(1)},') and seq[k + 2] == seq[k] \
                and seq[k + 3] == seq[k + 1]:
            w['manual-wrap'].append(k)
            k += 4
            while k + 1 < len(seq) and seq[k] == seq[k - 2] and seq[k + 1] == seq[k - 1]:
                k += 2
            continue
        k += 1
    # fusable adjacent single-op instructions
    k = 0
    while k + 1 < len(ins):
        a, b = ins[k], ins[k + 1]
        oa, ob = ops(a), ops(b)
        if len(oa) == 1 and len(ob) == 1 and not re.fullmatch(r'mov r(\d), r\1', oa[0]) \
                and not re.fullmatch(r'mov r(\d), r\1', ob[0]):
            da, sa = dest_srcs(oa[0])
            db, sb = dest_srcs(ob[0])
            if da is not None and db is not None and da not in sb and da != db and db not in sa \
                    and assembles(' | '.join([oa[0], ob[0]] + [u for u in a + b if XFER.match(u)])):
                w['fusable'].append(k)
                k += 2
                continue
        k += 1
    return len(ins), w


HINT = {
    'manual-wrap': 'use ph_wrap rD, r3, rY (one op) instead',
    'filler': 'load delays packing did not absorb',
    'transfer-only': 'transfers that fit into an op instruction',
    'fusable': 'adjacent independent ops encodable as one instruction (op fusion, as Roland)',
}


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('-')]
    strict, quiet = '--strict' in sys.argv, '-q' in sys.argv
    if not args:
        print(__doc__)
        return 2
    total = 0
    for path in args:
        n, w = check(path)
        nw = sum(len(v) for v in w.values())
        total += nw
        name = os.path.relpath(path, ROOT) if os.path.isabs(path) else path
        if quiet or not nw:
            parts = ', '.join(f'{k} {len(v)}' for k, v in w.items() if v) or 'optimized'
            print(f'check_optimized: {name}: {n} instructions: {parts}')
            continue
        for kind, at in w.items():
            if at:
                first = ', '.join(str(x) for x in at[:6]) + (' ...' if len(at) > 6 else '')
                print(f'check_optimized: warning: {name}: {len(at)} x {kind} ({HINT[kind]}); instructions {first}')
        print(f'check_optimized: {name}: {n} instructions, {nw} warnings')
    return 1 if strict and total else 0


if __name__ == '__main__':
    sys.exit(main())
