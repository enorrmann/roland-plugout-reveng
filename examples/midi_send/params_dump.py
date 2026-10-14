#!/usr/bin/env python3
"""
params_dump.py -- read the OS parameter values the JX-3P engine asks for
(OS +0x00 get on the slave) on the real device, through a region 0 init hook
(init_params.s) that calls the master's get callback and sends the values over
SysEx. Diagnostic for the from-scratch engine (examples/from_scratch/engine).

    python3 examples/midi_send/params_dump.py -o dumps/params.json

The id list = every (group, id) the JX-3P engine reads in a refresh
(vt+0x24(1), emulated), for parts 0, 1 and 2. Installs JX-3P-INIT-PRM in --slot
without patches; reinstall your PLUG-OUT afterwards.
"""
import argparse, json, os, struct, subprocess, sys, tempfile, time
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 'engine'))
from bulk_dump import decode

BASE, CAVE, TAB, VALS, INIT_OFF = 0x61800000, 0x100, 0x1000, 0x4000, 0x11823c


def id_list():
    import run_jx, gen_tables as G
    run_jx.IMG = os.path.join(ROOT, 'examples', 'from_scratch', 'engine', 'build', 'proj', 'region1.bin')
    e = G.start(0)
    e.log.clear()
    e.call(e.u32(e.vt + 0x24), e.o, 1)
    out = []
    for off, r0, r1, r2, *_ in e.log:
        if off == 0 and (r1, r2) not in out:
            out.append((r1, r2))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--slot', type=int, default=3)
    ap.add_argument('--port', default='hw:3,0,0')
    ap.add_argument('--parts', default='0,1,2')
    ap.add_argument('-o', '--out', default=os.path.join(ROOT, 'dumps', 'params.json'))
    args = ap.parse_args()
    ids = id_list()
    ents = [(p, g, i) for p in map(int, args.parts.split(',')) for g, i in ids]
    count = (4 * len(ents) + 31) // 32
    print(f'== {len(ids)} ids x {len(args.parts.split(","))} parts = {len(ents)} values, {count} messages')

    w = tempfile.mkdtemp()
    sh = lambda *a: subprocess.run(a, check=True, cwd=w, stdout=subprocess.DEVNULL)
    sh('arm-none-eabi-as', '-mcpu=cortex-a9', '-mthumb', '--defsym', f'CAVE_OFF={CAVE:#x}',
       '--defsym', f'INIT_OFF={INIT_OFF:#x}', '--defsym', 'ROUTER=0x6000345c', '--defsym', 'DTORW=0x600753f4',
       '--defsym', 'MASK=0x1A20', '--defsym', f'TAB={BASE + TAB:#x}', '--defsym', f'VALS={BASE + VALS:#x}',
       '--defsym', f'NENT={len(ents)}', '--defsym', 'LEN=32', '--defsym', f'COUNT={count}',
       '--defsym', 'CTOR=0x600753cc', '--defsym', 'SETSX=0x600754bc', '--defsym', 'WRAPM=0x600754c6',
       '--defsym', 'DTORM=0x600753e0', os.path.join(HERE, 'init_params.s'), '-o', 'h.o')
    sh('arm-none-eabi-ld', f'-Ttext={BASE + CAVE:#x}', '--entry=hook', 'h.o', '-o', 'h.elf')
    sh('arm-none-eabi-objcopy', '-O', 'binary', '-j', '.text', 'h.elf', 'h.bin')
    assert os.path.getsize(os.path.join(w, 'h.bin')) < TAB - CAVE
    open(os.path.join(w, 'tab.bin'), 'wb').write(b''.join(struct.pack('<I', p << 24 | g << 16 | i) for p, g, i in ents))
    assert 4 * len(ents) < VALS - TAB and VALS + 4 * len(ents) + 32 < 0x8000
    B = [sys.executable, os.path.join(ROOT, 'tools', 'plugout_build.py')]
    jx = os.path.join(ROOT, 'plugins', 'JX-3P', 'Script')
    sh(*B, 'unpack', os.path.join(jx, 'Code8_1.Dat'), 'p', '--relocs-from', os.path.join(jx, 'Code8_2.Dat'))
    sh(*B, 'set-id', 'p', '--name', 'INIT PRM', '--version', '1.22')
    sh(*B, 'patch', 'p', '--region', '0', '--at', hex(BASE + CAVE), '--file', 'h.bin')
    sh(*B, 'patch', 'p', '--region', '0', '--at', hex(BASE + TAB), '--file', 'tab.bin')
    sh(*B, 'patch', 'p', '--region', '0', '--at', hex(BASE + 0x24), '--hex',
       struct.pack('<I', BASE + CAVE + 1).hex(), '--reloc', hex(BASE + 0x24))
    plugin = os.path.join(ROOT, 'plugins', 'JX-3P-INIT-PRM')
    sh(*B, 'plugin', 'p', '--template', os.path.join(ROOT, 'plugins', 'JX-3P-MEM'), '-o', plugin)

    raw = os.path.join(w, 'cap.raw')
    cap = subprocess.Popen(['amidi', '-p', args.port, '-r', raw, '-t', '70'], stderr=subprocess.DEVNULL)
    time.sleep(0.5)
    print(f'== install in slot {args.slot}')
    subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'plugout_tool.py'), 'install', plugin,
                    '--slot', str(args.slot), '--no-patches', '-y'], stdout=subprocess.DEVNULL)
    last, still = -1, 0.0
    while still < 3.0:
        time.sleep(0.5)
        size = os.path.getsize(raw) if os.path.exists(raw) else 0
        still = still + 0.5 if size == last else 0.0
        last = size
    cap.terminate(); cap.wait()
    msgs = decode(open(raw, 'rb').read())
    data = b''.join(d for a, d in msgs)
    print(f'== {len(msgs)}/{count} messages, {len(data)} bytes')
    vals = struct.unpack('<%di' % (len(data) // 4), data[:len(data) // 4 * 4])
    res = [{'part': p, 'group': g, 'id': i, 'value': v} for (p, g, i), v in zip(ents, vals)]
    os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
    json.dump(res, open(args.out, 'w'), indent=0)
    print(f'   wrote {args.out}')
    r = subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'plugout_tool.py'), 'info'],
                       capture_output=True, text=True)
    print('== info:', ([l.strip() for l in r.stdout.splitlines() if f'slot {args.slot}:' in l] or ['NO RESPONSE'])[0])
    return 0 if len(vals) >= len(ents) else 1


if __name__ == '__main__':
    sys.exit(main())
