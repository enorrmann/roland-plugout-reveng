"""Run one Roland main program (engine/build/programs.h, copied from the JX-3P) sample by sample on the C reference
(emu/dspref.Machine, with this file's access hook).

Only what is measured is simulated. Anything else stops the run with an error that names the missing piece:
  - the condition-flag context of the DSP (--context; only the measured ones, DSP_HARDWARE.md §6)
  - a hidden register h4-h7 the program reads before loading it with 40TY (--hidden)
  - a bank-0, bank-5 or bank-6 word that was never given (--input); the ARM writes banks 5 and 6
  - reads of banks that depend on other DSPs or on the ARM: 1, 2, 3 (IOMD), 4 (MXM), 7
  - bank-5 words above 0x3FF, bank-0 references above 0x7FF, jumps into the middle of an instruction
  - undecoded ops, and ops whose effect is not measured (dspref.check)
Writes to banks 3 and 4 are collected per sample (outputs to the wave bus and the other DSPs), not read back.
Registers r0-r7 at power-up are not measured: they start at --regs (default 0). --check-init reruns the program
with other random register values and reports the samples where a recorded word changes.

    python3 sim_main.py MAIN0 --context <ctx> --input init.txt --n 1500 --record 0261,025B

Input file: one "t addr value" per line (addr hex, absolute; value = the DSP's RAW value, as the reference uses it).
  t = 0 is the state before sample 0 (bank 0, 5 and 6 words); t > 0 is written before sample t (banks 5 and 6 only).
Ring: bank 0 is a 0x800-word ring. Between samples every word moves +1 (mod 0x800): stored at S in sample n, read at
S+1 in sample n+1 (DSP_HARDWARE.md §7.2). Nothing else is reset between samples: flags, hidden registers, r0-r7 and
memory carry over (measured, DSP_HARDWARE.md §2).
"""
import argparse, os, random, re, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 're'))
sys.path.insert(0, os.path.join(HERE, '..', 'emu'))
import dspdis, dspref   # noqa: E402

PROGRAMS = os.path.join(HERE, '..', 'engine', 'build', 'programs.h')
RING = 0x800
NOOP_WORD = 0xFF70        # program-start form, the only "no visible effect" word measured in MAIN0


class Refused(Exception):
    """the simulation needs something that is not measured"""


def program_words(name, path=PROGRAMS):
    src = open(path).read()
    m = re.search(r'mc_%s\[(\d+)\].*?=\s*\{(.*?)\};' % name, src, re.S)
    words = []
    for t in re.findall(r'0x([0-9a-f]+)', m.group(2)):
        v = int(t, 16); words += [v >> 16, v & 0xFFFF]          # the high half of each 32-bit word comes first
    n = int(re.search(r'MC_%s_BYTES (0x[0-9a-f]+)' % name, src).group(1), 16) // 2
    if n > len(words):
        raise ValueError(f'{name}: {n} words declared, {len(words)} in the table')
    return words[:n]


def read_inputs(path):
    events = {}
    for ln, line in enumerate(open(path), 1):
        s = line.split('#')[0].split()
        if not s:
            continue
        if len(s) != 3:
            raise SystemExit(f'{path}:{ln}: expected "t addr value"')
        t, a, v = int(s[0]), int(s[1], 16), float(s[2])
        if a < RING and t != 0:
            raise SystemExit(f'{path}:{ln}: bank 0 words are only given at t = 0')
        if not (a < RING or 0xA000 <= a < 0xA400 or 0xC000 <= a < 0xE000):
            raise SystemExit(f'{path}:{ln}: address {a:04x} can not be given as input (banks 0, 5, 6 only)')
        events.setdefault(t, []).append((a, v))
    return events


def make_hook(io):
    def access(op, a, mem, v=None):
        bank = a >> 13
        if bank == 0:                                  # 0000-1FFF: ring 0x000-0x7FF
            if a >= RING:
                raise Refused(f'bank-0 reference {a:04x} above the 0x800-word ring: not measured')
            if op == 'w':
                mem[a] = v
                return None
            if a not in mem:
                raise Refused(f'bank-0 word {a:04x} read before it was written: give it with --input at t = 0')
            return mem[a]
        if a >> 12 == 0xA:                             # bank 5, persistent
            if a - 0xA000 >= 0x400:
                raise Refused(f'bank-5 index {a - 0xA000:#x} above 0x3FF: not measured')
            if op == 'w':
                mem[a] = v
                return None
            if a not in mem:
                raise Refused(f'bank-5 word {a:04x} read before it was given: the ARM writes it (--input)')
            return mem[a]
        if a >> 12 == 0xC:                             # bank 6, written by the ARM
            if op == 'w':
                raise Refused(f'write to bank 6 {a:04x}: the ARM owns it')
            if a not in mem:
                raise Refused(f'bank-6 word {a:04x} read before it was given: the ARM writes it (--input)')
            return mem[a]
        if bank == 3 and a >> 12 == 0x6:               # IOMD: outputs, not read back (DSP_ISA_SPEC §9)
            if op == 'w':
                io[a] = v
                return None
            raise Refused(f'IOMD read {a:04x}: not measured')
        if bank == 4:                                  # MXM: outputs to the other DSPs
            if op == 'w':
                io[a] = v
                return None
            raise Refused(f'MXM read {a:04x}: depends on the other DSPs, not modelled')
        raise Refused(f'bank {bank} access {a:04x}: not measured')
    return access


def simulate(words, events, regs0, hidden, context, n, record):
    if context not in dspref.CONTEXT_BITS:
        raise Refused(f'condition-flag context {context!r} is not measured for this DSP; '
                      f'the measured ones are {", ".join(dspref.CONTEXT_BITS)}')
    ins = [i for _, i in dspdis.instructions(words)]
    starts, p = [], 0
    for i in ins:
        starts.append(p); p += len(i)
    if p != len(words):
        raise ValueError('instructions do not cover the program')
    pos = {s: k for k, s in enumerate(starts)}
    why = [dspref.check(x, NOOP_WORD) for x in ins]     # a unit with no measured effect (refused if it runs)
    use = [dspref.hidden_use(x) for x in ins]

    # every run starts from the same measured-or-given state
    m = dspref.Machine(context=context, hidden={k: hidden.get(k) or 0.0 for k in (4, 5, 6, 7)})
    known = {k for k in (4, 5, 6, 7) if hidden.get(k) is not None}
    io, mem, err = {}, {}, []
    hook = make_hook(io)

    def access(op, a, v):
        try:
            return hook(op, a, mem, v)
        except Refused as e:                           # raised again after the instruction
            err.append(e)
            return 0.0
    m.set_access(access)
    m.regs = list(regs0)
    out = {a: [] for a in record}
    io_last = {}
    try:
        for t in range(n):
            if t > 0:
                mem = {((a + 1) % RING if a < RING else a): v for a, v in mem.items()}
            for a, v in events.get(t, []):
                mem[a] = v
            io.clear()
            i, steps = 0, 0
            while i < len(ins):
                steps += 1
                if steps > 200000:
                    raise Refused('endless loop in one sample')
                if why[i]:
                    raise Refused(f'instruction {i}: not modelled: {why[i]}')
                reads, loads = use[i]
                if reads - known:
                    raise Refused(f'instruction {i}: reads h{min(reads - known)} before it is loaded (--hidden)')
                try:
                    jump = m.run(list(ins[i]))
                except (ValueError, IndexError) as e:
                    raise Refused(f'instruction {i} ({" ".join(f"{w:04x}" for w in ins[i])}): {e!r}')
                if err:
                    raise err[0]
                known |= loads
                if jump is not None:
                    tgt = starts[i] + len(ins[i]) + jump
                    if tgt == len(words):
                        i = len(ins)
                    elif tgt in pos:
                        i = pos[tgt]
                    else:
                        raise Refused(f'jump from instruction {i} to word {tgt}: not an instruction start')
                else:
                    i += 1
            io_last.update(io)
            for a in record:
                out[a].append(mem.get(a))
    finally:
        m.set_access(None)
    return out, io_last


def run_checked(name, words, events, regs0, hidden, context, n, record, seed):
    out, io = simulate(words, events, regs0, hidden, context, n, record)
    if seed is None:
        return out, io, None
    rnd = random.Random(seed)
    other = [rnd.uniform(-1, 1) for _ in range(8)]
    out2, _ = simulate(words, events, other, hidden, context, n, record)
    first = {}
    for a in record:
        for t, (x, y) in enumerate(zip(out[a], out2[a])):
            if x != y:
                first[a] = (t, abs(x - y))
                break
    return out, io, first


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('name')
    ap.add_argument('--context', default=None, help='condition-flag context (dspref.CONTEXT_BITS key)')
    ap.add_argument('--input', default=None, help='"t addr value" lines (see the docstring)')
    ap.add_argument('--n', type=int, default=1500)
    ap.add_argument('--regs', default=None, help='r0..r7 at power-up, comma separated (default 0)')
    ap.add_argument('--hidden', default='', help='h4..h7 start values, e.g. 4=288,5=0.0039')
    ap.add_argument('--record', default='0261,025B', help='bank-0 addresses printed per sample (hex)')
    ap.add_argument('--check-init', type=int, default=None, metavar='SEED',
                    help='rerun with random r0-r7 and report the samples where a recorded word differs')
    a = ap.parse_args()

    if a.name.endswith('.txt'):                  # 16-bit words in execution order (dspexp format, '#' comments)
        words = [int(t, 16) for line in open(a.name) for t in line.split('#')[0].split()]
    else:
        words = program_words(a.name)
    events = read_inputs(a.input) if a.input else {}
    regs0 = [float(x) for x in a.regs.split(',')] if a.regs else [0.0] * 8
    if a.regs is None:
        print('note: r0-r7 start at 0 (power-up state not measured; see --check-init)', file=sys.stderr)
    hidden = {int(k): float(v) for k, v in (x.split('=') for x in a.hidden.split(',') if x)}
    record = [int(x, 16) for x in a.record.split(',')]
    try:
        out, io, first = run_checked(a.name, words, events, regs0, hidden, a.context, a.n, record, a.check_init)
    except Refused as e:
        print(f'REFUSED: {e}')
        sys.exit(2)
    for t in [x for x in (0, 1, 99, 101, 150, 300, 600, 1000, 1499) if x < a.n]:
        print('t=%d ' % t + ' '.join('[%04x]=%s' % (k, '%.5f' % out[k][t] if out[k][t] is not None else 'unwritten')
                                    for k in record))
    if io:
        print('bank 3/4 writes in the last sample: ' + ' '.join(f'{k:04x}={v:.5f}' for k, v in sorted(io.items())))
    if first is not None:
        if not first:
            print('check-init: recorded words do not depend on the power-up registers')
        for k, (t, d) in first.items():
            print('check-init: [%04x] depends on power-up registers: first differs at t=%d (|diff| %.3g)' % (k, t, d))
