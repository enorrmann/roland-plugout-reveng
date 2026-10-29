"""Read C-S1RAMP's reports (model.c ramp_report: SysEx F0 7D 07) and show what master DSP B measured of each FCE4
ramp (docs/examples_system_1_ramp_README.md).

    python3 examples/system_1_ramp/ramp.py -t 300 --log work/captures/ramp.txt   listen (C-S1RAMP selected)
    python3 examples/system_1_ramp/ramp.py --file work/captures/ramp.txt         decode a saved log again

Report: case, y, z, direction; the ARM's own timing of the ramp (the live value, parameter block +8, polled with
the Cortex-M3 cycle counter): time until it crossed 0.01 0.1 0.25 0.5 0.75 0.9 0.99 0.999 (down: 1 - those);
parameter block C030 +0 control, +4 target, +8 value, +C; end-of-ramp table entry; SysTick, CPUID, DWT. Uses `amidi -d`; no other MIDI tool at the same time.
"""
import argparse
import os
import re
import struct
import subprocess
import sys

TH = (0.01, 0.1, 0.25, 0.5, 0.75, 0.9, 0.99, 0.999)
FS = 96000


def f32(u):
    return struct.unpack('<f', struct.pack('<I', u & 0xFFFFFFFF))[0]


def messages(tokens):
    cur = None
    for t in tokens:
        b = int(t, 16)
        if b == 0xF0:
            cur = [b]
        elif cur is not None:
            cur.append(b)
            if b == 0xF7:
                yield cur
                cur = None


def decode(m):
    if m[:3] != [0xF0, 0x7D, 0x07] or len(m) != 9 + 5 * 23:
        return None
    case, y, z, down, how = m[3:8]
    w = []
    for k in range(23):
        p = m[8 + 5 * k:13 + 5 * k]
        w.append(p[0] << 28 | p[1] << 21 | p[2] << 14 | p[3] << 7 | p[4])
    return {'case': case, 'y': y, 'z': z, 'down': down, 'how': how, 'b5': w[:11], 'blk': w[11:15], 'cb': w[15],
            'marker': w[16:21], 'audio': w[21:23]}


def show(r):
    """the ARM's timing (model.c ramp_start): cycles until each crossing; SysTick LOAD + 1 = cycles per ms (1 kHz tick)"""
    w, x = r['b5'], r['marker'] + r['audio']
    per_ms = (w[10] & 0xFFFFFF) + 1
    ms = lambda c: 'never' if c == 0xFFFFFFFF else f'{c / per_ms:.2f}'
    print(f"case {r['case']:3d} y {r['y']:2d} z {r['z']} {'down' if r['down'] else 'up  '}  now {f32(w[0]):.6f}  "
          f"polled {ms(w[1])} ms ({x[3]} polls, first change {ms(x[4])} ms)  {per_ms} cycles/ms")
    print('    crossing ms: ' + ' '.join(f'{th}:{ms(c)}' for th, c in zip(TH, w[2:10])))
    print(f"    block {' '.join('%08x' % v for v in r['blk'])} (+8 = {f32(r['blk'][2]):.6f})  end table {r['cb']:08x}  "
          f"SysTick CSR {x[0]:08x} CPUID {x[1]:08x} DWT {x[2]:08x}")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('-t', '--time', type=int, default=300)
    ap.add_argument('--file')
    ap.add_argument('--log', help='keep the raw amidi output here')
    a = ap.parse_args()
    if a.file:
        for m in messages(open(a.file).read().split()):
            r = decode(m)
            if r:
                show(r)
        return
    out = subprocess.run(['amidi', '-l'], capture_output=True, text=True).stdout
    ps = [mm.group(1) for mm in re.finditer(r'^IO\s+(hw:\S+)\s+(.*)$', out, re.M) if 'SYSTEM-1' in mm.group(2)]
    if not ps:
        sys.exit('no SYSTEM-1 MIDI port (amidi -l)')
    log = open(a.log, 'w') if a.log else None
    procs = [subprocess.Popen(['timeout', str(a.time), 'amidi', '-p', p, '-d'], stdout=subprocess.PIPE,
                              stderr=subprocess.DEVNULL, text=True) for p in ps]
    print(f'listening {a.time} s on {", ".join(ps)}', flush=True)
    import selectors
    sel = selectors.DefaultSelector()
    bufs = {}
    for p in procs:
        sel.register(p.stdout, selectors.EVENT_READ, p)
        bufs[p] = []
    live = len(procs)
    while live:
        for key, _ in sel.select():
            line = key.fileobj.readline()
            if not line:
                sel.unregister(key.fileobj)
                live -= 1
                continue
            if log:
                log.write(line)
                log.flush()
            toks = bufs[key.data]
            toks += line.split()
            if 'F7' in toks:
                i = len(toks) - 1 - toks[::-1].index('F7')
                for m in messages(toks[:i + 1]):
                    r = decode(m)
                    if r:
                        show(r)
                        sys.stdout.flush()
                del toks[:i + 1]


if __name__ == '__main__':
    main()
