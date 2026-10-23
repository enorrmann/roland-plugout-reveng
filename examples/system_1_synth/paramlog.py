"""Listen to a PARAMLOG build and list the parameters the device hands to the PLUG-OUT model (docs/examples_system_1_README.md §6).

The PARAMLOG build of C-S1V4 (PARAMLOG=1 build.sh) sends one SysEx per call of the model's vt+0x30:
    F0 7D 06 <id >> 7> <id & 7F> <flag> <value: bits 31-28, 27-21, 20-14, 13-7, 6-0> F7
Everything else the device sends (CC echoes of the panel, ...) is shown too, interleaved, so a knob's id can be
matched with its MIDI CC.

    python3 examples/system_1/paramlog.py                 listen 60 s on every SYSTEM-1 port, then summarize
    python3 examples/system_1/paramlog.py -t 120 --raw    also print every message as it arrives
    python3 examples/system_1/paramlog.py --file LOG...   decode saved `amidi -d` output instead
    python3 examples/system_1/paramlog.py --device SYSTEM-8   the same on a SYSTEM-8 (a build that sends F0 7D 06)

Uses `amidi -d` (alsa-utils). Do not run it while another tool talks to the device.
"""
import argparse
import collections
import os
import re
import subprocess
import sys
import tempfile


def ports(hint='SYSTEM-1'):
    out = subprocess.run(['amidi', '-l'], capture_output=True, text=True).stdout
    return [m.group(1) for m in re.finditer(r'^IO\s+(hw:\S+)\s+(.*)$', out, re.M) if hint in m.group(2)]


def decode(lines):
    """[(kind, data)]: ('param', (id, flag, value)) or ('midi', 'B4 10 42')"""
    out = []
    for line in lines:
        t = line.split()
        if not t:
            continue
        if len(t) == 12 and t[:3] == ['F0', '7D', '06'] and t[-1] == 'F7':
            b = [int(x, 16) for x in t]
            pid = b[3] << 7 | b[4]
            v = b[6] << 28 | b[7] << 21 | b[8] << 14 | b[9] << 7 | b[10]
            out.append(('param', (pid, b[5], v - (1 << 32) if v >= 1 << 31 else v)))
        else:
            out.append(('midi', ' '.join(t)))
    return out


def summary(events):
    vals = collections.defaultdict(list)
    cc_after = collections.defaultdict(collections.Counter)    # id -> MIDI messages that follow it
    last = None
    for kind, d in events:
        if kind == 'param':
            vals[d[0]].append(d[2])
            last = d[0]
        elif last is not None and d[:1] in 'B':
            cc_after[last][' '.join(d.split()[:2])] += 1
    print('%-6s %6s %8s %8s  %-24s %s' % ('id', 'calls', 'min', 'max', 'first values', 'MIDI that follows'))
    for pid in sorted(vals):
        v = vals[pid]
        cc = ', '.join('%s x%d' % kv for kv in cc_after[pid].most_common(2))
        print('0x%03x %6d %8d %8d  %-24s %s' % (pid, len(v), min(v), max(v), str(v[:4]), cc))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('-t', '--time', type=int, default=60, help='seconds to listen (default 60)')
    ap.add_argument('--raw', action='store_true', help='print every message too')
    ap.add_argument('--file', nargs='+', help='decode these amidi -d logs instead of listening')
    ap.add_argument('--device', default='SYSTEM-1', help='port name to listen on (SYSTEM-8 for the SYSTEM-8)')
    a = ap.parse_args()
    if a.file:
        lines = [l for f in a.file for l in open(f)]
    else:
        ps = ports(a.device)
        if not ps:
            sys.exit(f'no {a.device} MIDI port (amidi -l)')
        tmp = tempfile.mkdtemp(prefix='paramlog')
        procs = [subprocess.Popen(['timeout', str(a.time), 'amidi', '-p', p, '-d'],
                                  stdout=open(os.path.join(tmp, f'{i}.txt'), 'w'), stderr=subprocess.DEVNULL)
                 for i, p in enumerate(ps)]
        print(f'listening {a.time} s on {", ".join(ps)}: move the controls now', flush=True)
        for p in procs:
            p.wait()
        lines = [l for i in range(len(ps)) for l in open(os.path.join(tmp, f'{i}.txt'))]
        print(f'logs kept in {tmp}')
    ev = decode(lines)
    if a.raw:
        for kind, d in ev:
            print(('param id 0x%03x flag %d value %d' % d) if kind == 'param' else d)
    summary(ev)


if __name__ == '__main__':
    main()
