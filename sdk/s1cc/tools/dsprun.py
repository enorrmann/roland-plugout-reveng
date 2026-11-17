#!/usr/bin/env python3
"""dsprun.py -- run one DSP program (any, jumps included) sample by sample and record the words it writes.

    python3 sdk/s1cc/tools/dsprun.py PROGRAM.txt -n SAMPLES -o OUT.npy [--set ADDR=V ...] [--setup FILE.json]
            [--at SAMPLE:ADDR=V ...] [--record ADDR[-ADDR],...] [--bank1-ring]

For porting a synth: measuring what a Roland program does with given inputs (an envelope's curve for a knob value, an
LFO's rate, a voice's output for a note) without writing a harness each time. The program is lifted to plain C with
sdk/s1cc/tools/dsplift.py (machine mode: bit-exact with the emulator, docs/vol5_toolchain_compiler_sdk/dsplift.md),
compiled once with a generic driver (cached in work/dsprun/ by the program's hash) and run.

  PROGRAM.txt        the program's words in execution order (hex, as examples/from_scratch/engine/programs.py and
                     dsplift.py read them)
  --set ADDR=V       a memory word before sample 0: parameters C000.., bank 5 A000.., IOMD 6000.., bank 0 / 1 (hex)
  --setup FILE.json  many of them: {"c020": 1.0, "a021": 4.0, ...}
  --at S:ADDR=V      write V at the start of sample S (a gate, a knob move); repeatable
  --record LIST      the words to record (every store to them; the value held until the next store), hex, ranges
                     with '-'; default 8400-841f (the MXM outputs of an S8 main program)
  OUT.npy            float64 array SAMPLES x len(record), column order = the record list
In Python: `import dsprun; y = dsprun.run(words_or_path, n, setup={...}, events=[(s, addr, v)], record=[...])`.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import struct
import subprocess
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
CACHE = os.path.join(ROOT, 'work', 'dsprun')

DRIVER = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lift.c"
/* input: n, nset, (addr, v)*, nev, (sample, addr, v)*, nrec, addr*; output: n x nrec doubles */
static int nrec; static int64_t *rec; static double *cur;
static void onw(LiftM *M, int64_t a, double v) { (void)M; for (int i = 0; i < nrec; i++) if (rec[i] == a) cur[i] = v; }
static void put(LiftM *M, int64_t a, double v)
{
    int64_t off = a & 0x1FFF;
    switch (a >> 13) {
    case 0: M->bank0[off & 0x7FF] = v; break;
    case 1: M->bank1[off & 0x7FF] = v; break;
    case 3: M->iomd[off] = v; break;
    case 5: M->bank5[off & 0x7FF] = v; break;
    case 6: M->params[off] = v; break;
    case 7: M->bank7[off] = v; break;
    default: break;
    }
}
int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "rb"), *o = fopen(argv[2], "wb");
    int n, ns, ne; int64_t a; double v;
    static LiftM M;
    if (fread(&n, 4, 1, f) != 1 || fread(&ns, 4, 1, f) != 1) return 2;
    for (int i = 0; i < ns; i++) { fread(&a, 8, 1, f); fread(&v, 8, 1, f); put(&M, a, v); }
    fread(&ne, 4, 1, f);
    int *es = malloc(4 * (ne + 1)); int64_t *ea = malloc(8 * (ne + 1)); double *ev = malloc(8 * (ne + 1));
    for (int i = 0; i < ne; i++) { fread(&es[i], 4, 1, f); fread(&ea[i], 8, 1, f); fread(&ev[i], 8, 1, f); }
    fread(&nrec, 4, 1, f);
    rec = malloc(8 * (nrec + 1)); cur = calloc(nrec + 1, 8);
    for (int i = 0; i < nrec; i++) fread(&rec[i], 8, 1, f);
    M.onwrite = onw;
    for (int t = 0, k = 0; t < n; t++) {
        for (; k < ne && es[k] <= t; k++) put(&M, ea[k], ev[k]);
        PROG(&M); M.t++;
        fwrite(cur, 8, nrec, o);
    }
    return 0;
}
'''


def _words(prog):
    if isinstance(prog, str):
        return [int(x, 16) for x in open(prog).read().split()]
    return list(prog)


def build(words, bank1_ring=False):
    """lift + compile once per program (cache by hash) -> the binary's path"""
    txt = ' '.join('%04x' % w for w in words) + '\n'
    key = hashlib.sha256((txt + str(bank1_ring)).encode()).hexdigest()[:16]
    d = os.path.join(CACHE, key)
    exe = os.path.join(d, 'run')
    if os.path.exists(exe):
        return exe
    os.makedirs(d, exist_ok=True)
    open(os.path.join(d, 'prog.txt'), 'w').write(txt)
    cmd = [sys.executable, os.path.join(HERE, 'dsplift.py'), os.path.join(d, 'lift.c'), 'prog=' + os.path.join(d, 'prog.txt')]
    if bank1_ring:
        cmd.append('--bank1-ring')
    subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL)
    open(os.path.join(d, 'driver.c'), 'w').write(DRIVER)
    subprocess.run(['gcc', '-O1', '-I', HERE, '-I', d, '-DPROG=prog', '-o', exe, os.path.join(d, 'driver.c'), '-lm'],
                   check=True)
    return exe


def run(prog, n, setup=None, events=(), record=None, bank1_ring=False):
    """setup {addr: v}, events [(sample, addr, v)], record [addr] -> n x len(record) float64"""
    exe = build(_words(prog), bank1_ring)
    record = list(record) if record is not None else list(range(0x8400, 0x8420))
    setup = setup or {}
    b = struct.pack('<ii', n, len(setup)) + b''.join(struct.pack('<qd', int(a), float(v)) for a, v in setup.items())
    ev = sorted(events)
    b += struct.pack('<i', len(ev)) + b''.join(struct.pack('<iqd', int(s), int(a), float(v)) for s, a, v in ev)
    b += struct.pack('<i', len(record)) + b''.join(struct.pack('<q', a) for a in record)
    d = os.path.dirname(exe)
    inp, out = os.path.join(d, 'in.%d.bin' % os.getpid()), os.path.join(d, 'out.%d.bin' % os.getpid())
    open(inp, 'wb').write(b)
    try:
        subprocess.run([exe, inp, out], check=True)
        return np.fromfile(out, '<f8').reshape(n, len(record))
    finally:
        for p in (inp, out):
            if os.path.exists(p):
                os.remove(p)


def _addrs(s):
    out = []
    for part in s.split(','):
        lo, _, hi = part.partition('-')
        out += list(range(int(lo, 16), int(hi or lo, 16) + 1))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0], formatter_class=argparse.RawDescriptionHelpFormatter,
                                 epilog=__doc__.split('\n\n', 1)[1])
    ap.add_argument('program')
    ap.add_argument('-n', type=int, required=True)
    ap.add_argument('-o', '--out', required=True)
    ap.add_argument('--set', action='append', default=[])
    ap.add_argument('--setup')
    ap.add_argument('--at', action='append', default=[])
    ap.add_argument('--record', default='8400-841f')
    ap.add_argument('--bank1-ring', action='store_true')
    a = ap.parse_args()
    setup = {int(k, 16): v for k, v in json.load(open(a.setup)).items()} if a.setup else {}
    for s in a.set:
        k, v = s.split('=')
        setup[int(k, 16)] = float(v)
    ev = []
    for s in a.at:
        t, rest = s.split(':')
        k, v = rest.split('=')
        ev.append((int(t), int(k, 16), float(v)))
    y = run(a.program, a.n, setup, ev, _addrs(a.record), a.bank1_ring)
    np.save(a.out, y)
    print('%s: %d x %d' % (a.out, *y.shape))


if __name__ == '__main__':
    main()
