"""When each bank-5 mailbox word between the two DSPs of a SYSTEM-1 chip is written and read inside the sample, against
the latency the SYSTEM-8 had (docs/examples_system_1_jx_README.md section 13).

    python3 timing.py [samples]     # default 300 samples of the emulated SYSTEM-1 layout, random inputs

The two DSPs of a chip (master A/B, slave A/B) run AT THE SAME TIME on the device and share bank 5. A word that one
writes and the other reads is seen in the same sample only if the write comes before the read in cycles (1 cycle per
instruction, docs/DSP_HARDWARE.md). The emulator runs the DSPs one after the other, so it cannot show this: here the
executed instruction sequence of every DSP comes from the emulator's trace, and the cycle of each instruction is its
position in that sequence plus the programs the OS runs before ours (frame header at 0x0000; master A: the output
program at 0x82). [inferred] both DSPs of a chip start the sample on the same cycle; the exact visibility edge
(pipeline) is a few cycles, hence the margin.

Latency the S8 had, per mailbox kind (s1jx.py Plan keys):
  MZ / MS  MXM: 1 sample (latched bus, DSP_HARDWARE.md)
  DCO      main -> its DCO slot: 0 (same S8 DSP, the main ran first); DCO -> main: 1 (the main reads it at +1 of the
           bank-1 ring, the value of the sample before)
  REG      r2 / r3 handoff (main -> DCO): 0
Verdict per word: OK (S1 latency = S8), LATE (S1 one sample later), EARLY (S1 sees this sample's value, S8 the one
before) or RACE (writes and reads interleave, or the margin is under MARGIN cycles: depends on the exact timing).
"""
import os
import random
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import s1jx as J  # noqa: E402
import dspreloc as R  # noqa: E402
import chain  # noqa: E402
import cchain  # noqa: E402

MARGIN = 8
FRAME_HEADER = 4            # s1jx.frame_header: 4 instructions at 0x0000 on every DSP
OS_OUTPUT = 40              # master A, 0x82 (test_s1jx.py BUDGET: 925 - 40)


def s8_latency(key, writer_unit):
    kind = key[0]
    if kind in ('MZ', 'MS'):
        return 1
    if kind == 'DCO':
        return 1 if writer_unit.startswith('DCO') else 0
    if kind == 'REG':
        return 0
    return None


def schedule(pl, samples, seed=1):
    """{s1 dsp: {(program index, instruction): (min cycle, max cycle)}} over `samples` samples"""
    s1w = J.emu_map_s1(pl, pl.programs())
    progs = {J.MA: (chain.Program(s1w[(J.MA, 0x100)], 'MA'), []), J.MB: (chain.Program(s1w[(J.MB, 0x1000)], 'MB'), []),
             J.SA: (chain.Program(s1w[(J.SA, 0x100)], 'SA'), [chain.Program(s1w[(J.SA, 0x1900)], 'SA2')]),
             J.SB: (chain.Program(s1w[(J.SB, 0x100)], 'SB'), [chain.Program(s1w[(J.SB, 0x1900)], 'SB2')])}
    cur = {d: [] for d in range(4)}

    def tr(dsp, i, regs, f, pm, pv):
        cur[dsp].append(i)
    s = cchain.Slave(progs, native=False, threads=False, trace=tr)
    rng = random.Random(seed)
    seen = {d: {} for d in range(4)}
    base = {d: FRAME_HEADER + (OS_OUTPUT if d == J.MA else 0) for d in range(4)}
    for t in range(samples):
        if t % 50 == 0:
            for d in range(4):
                for i in range(0x200):
                    s.p_bank(d, i, rng.choice([0.0, 1.0, rng.uniform(-1, 1)]))
            for i in range(0x800):
                s.params[0xC000 + i] = rng.choice([0.0, 1.0, rng.uniform(0, 1)])
        for d in cur:
            cur[d] = []
        s.step()
        for d, seq in cur.items():
            prog, last = 0, -1
            for c, i in enumerate(seq):
                if i == 0 and last >= 0:       # instruction 0 again: the next program of this DSP
                    prog += 1
                last = i
                k = (prog, i)
                lo, hi = seen[d].get(k, (1 << 30, -1))
                seen[d][k] = (min(lo, base[d] + c), max(hi, base[d] + c))
    return seen


def accesses(pl):
    """{chip: {bank-5 index: [(s1 dsp, program index, instruction, op)]}} for the words used by both DSPs of a chip"""
    out = {0: {}, 1: {}}
    per = {}
    for s1, addr, units, w in pl.programs():
        per.setdefault(s1, []).append(w)
    for s1, ws in per.items():
        chip = J.CHIP[s1]
        for k, w in enumerate(ws):
            for r in R.Prog(w).refs:
                i = r.addr & 0x1FF
                if r.addr >> 12 == 0xA and (r.addr & 0xFFF) < 0x200 and len(pl.users[chip].get(i, ())) > 1:
                    out[chip].setdefault(i, []).append((s1, k, r.ins, r.op))
    return out


def unit_at(pl, s1, prog, ins):
    """the unit an instruction of a location belongs to (by instruction counts of the concatenated units)"""
    locs = [(a, u) for d, a, u in J.LOCATIONS if d == s1]
    a, units = locs[prog]
    n = 0
    for u in units:
        w = pl.tail() if u == 'TAIL' else pl.words[u]
        m = len(list(R.dspdis.instructions(w))) - (2 if u != units[-1] else 0)
        if ins < n + m:
            return u
        n += m
    return units[-1]


def main(argv):
    n = int(argv[0]) if argv and argv[0].isdigit() else 300
    pl = J.Plan(J.roland_programs())
    sch = schedule(pl, n)
    acc = accesses(pl)
    bad = 0
    print(f'{n} samples; cycle = executed position + {FRAME_HEADER} (+{OS_OUTPUT} on master A); margin {MARGIN}')
    for chip in (0, 1):
        print(f'== {"master" if chip == 0 else "slave"}')
        for i, lst in sorted(acc[chip].items()):
            key = pl.b5_owner[chip][i]
            wr = [(d, p, k) for d, p, k, op in lst if op == 'st']
            rd = [(d, p, k) for d, p, k, op in lst if op == 'ld']
            if not wr or not rd:
                print(f'   A{i:03x} {str(key):24} only {"stores" if wr else "loads"}')
                continue
            wc = [sch[d].get((p, k)) for d, p, k in wr]
            rc = [sch[d].get((p, k)) for d, p, k in rd]
            if None in wc or None in rc:
                print(f'   A{i:03x} {str(key):24} not executed in the run')
                continue
            w_lo, w_hi = min(c[0] for c in wc), max(c[1] for c in wc)
            r_lo, r_hi = min(c[0] for c in rc), max(c[1] for c in rc)
            if w_hi + MARGIN <= r_lo:
                lat = 0
            elif r_hi + MARGIN <= w_lo:
                lat = 1
            else:
                lat = None
            wu = unit_at(pl, *wr[0])
            want = s8_latency(key, wu)
            v = 'RACE' if lat is None else 'OK' if want is None or lat == want else 'LATE' if lat > want else 'EARLY'
            bad += v != 'OK'
            ru = unit_at(pl, *rd[0])
            print(f'   A{i:03x} {str(key):24} {wu:>7} st {w_lo:4}-{w_hi:<4} -> {ru:7} ld {r_lo:4}-{r_hi:<4} '
                  f'S1 latency {"?" if lat is None else lat}, S8 {"?" if want is None else want}: {v}')
    print(f'{bad} words differ from the SYSTEM-8 timing')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
