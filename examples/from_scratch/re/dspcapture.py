"""One device session that captures every pending measurement of the DSP's ops, then offline checks against the C
reference (emu/dspref.py) as often as needed, without the device (docs/NATIVE_OPS.md).

    python3 dspcapture.py run [-o work/captures/NAME] [-n 96]   # SYSTEM-8, lab PEEK in slot 3, PLG3 selected
    python3 dspcapture.py check [-o work/captures/NAME]          # no device: the reference against the capture

What `run` captures (raw words, JSON lines), one file per part:
    native.jsonl   re/dspnative.py, every test (-n cases per test, with and without an empty instruction)
    insns.jsonl    re/dspverify.py on Roland instructions listed in work/cond_insns.txt (conditions, X9bY) and
                   work/dualmac_p2.txt (two MACs with P2 in classes with transfers) and work/mac_alu_insns.txt (one
                   MAC beside A / B ops, by class), local lists (Roland's code stays in work/); plus one instance of
                   every class marked inferred
    jumps.jsonl    re/dspverify.py --jumps: unconditional, skipped by conditions 4-19 and their negations, a loop
    programs.jsonl re/dspverify.py --programs: indexed transfers (hset from a register, bank 5 / bank 0), load latency
Each part is a separate process; a part that fails does not stop the others. Only one MIDI tool at a time.
"""
import argparse
import os
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
LISTS = [os.path.join(ROOT, 'work', f) for f in ('cond_insns.txt', 'dualmac_p2.txt', 'mac_alu_insns.txt')]


def parts(out, n, tests=(), lists_only=None):
    """(name, file, capture command, check command)"""
    py = [sys.executable]
    lists = []
    for f in LISTS if lists_only is None else lists_only:
        if os.path.exists(f):
            lists += ['--list-file', f]
    nat, ins, jmp, prg = (os.path.join(out, f) for f in ('native.jsonl', 'insns.jsonl', 'jumps.jsonl', 'programs.jsonl'))
    return [
        ('native', nat, py + ['dspnative.py', '-n', str(n), '--save', nat] + list(tests),
         py + ['dspnative.py', '--check', nat]),
        ('insns', ins, py + ['dspverify.py', '-n', '4'] + ([] if lists_only else ['--inferred']) + ['--save', ins] + lists,
         py + ['dspverify.py', '--check', ins]),
        ('jumps', jmp, py + ['dspverify.py', '-n', '4', '--jumps', '--save', jmp], py + ['dspverify.py', '--check', jmp]),
        ('programs', prg, py + ['dspverify.py', '-n', '8', '--programs', '--save', prg],
         py + ['dspverify.py', '--check', prg]),
    ]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['run', 'check'])
    ap.add_argument('-o', default=os.path.join(ROOT, 'work', 'captures', time.strftime('%Y-%m-%d')))
    ap.add_argument('-n', type=int, default=96)
    ap.add_argument('--only', nargs='*', help='parts to run (native, insns, jumps, programs)')
    ap.add_argument('--tests', nargs='*', default=(), help='native: only these dspnative tests')
    ap.add_argument('--lists', nargs='*', help='insns: only these instruction lists (no --inferred)')
    a = ap.parse_args()
    os.makedirs(a.o, exist_ok=True)
    bad = 0
    for name, path, cap, chk in parts(a.o, a.n, a.tests, a.lists):
        if a.only and name not in a.only:
            continue
        if a.cmd == 'run':
            if os.path.exists(path):
                os.replace(path, path + '.old')            # a new capture: keep the previous one aside
            t0 = time.time()
            log = open(os.path.join(a.o, name + '.log'), 'w')
            rc = subprocess.call(cap, cwd=HERE, stdout=log, stderr=subprocess.STDOUT)
            print(f'{name}: capture exit {rc}, {time.time() - t0:.0f} s, {path}', flush=True)
            bad += rc != 0
        else:
            if not os.path.exists(path):
                print(f'{name}: no capture ({path})')
                continue
            print(f'== {name}', flush=True)
            bad += subprocess.call(chk, cwd=HERE) != 0
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
