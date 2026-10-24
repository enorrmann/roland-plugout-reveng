"""sdk/mport command line (docs/MUTABLE_SDK.md §8).

    python3 sdk/mport/cli.py list                         ported components
    python3 sdk/mport/cli.py check saw_swarm              port vs the original C++ (and the integer model)
    python3 sdk/mport/cli.py lines saw_swarm [--plain]    the bench program: scheduled (default) or one unit per line
    python3 sdk/mport/cli.py lut braids ws_moderate_overdrive --odd 9 [--xmax 0.875]
                                                          polynomial fit of a resources.cc table, error in LSB
"""
import argparse
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, 'components'))

import bench  # noqa: E402
import compare  # noqa: E402
import lut  # noqa: E402
from components import REGISTRY  # noqa: E402

# (note, knobs) cases of `check`: knob values 0..255 of the component's params
CASES = {
    'saw_swarm': [(45, {'ratio': 200, 'hp': 85}), (57, {'ratio': 30, 'hp': 170}), (69, {'ratio': 128, 'hp': 128}),
                  (69, {'ratio': 128, 'hp': 0}), (81, {'ratio': 255, 'hp': 100}), (33, {'ratio': 255, 'hp': 255}),
                  (96, {'ratio': 128, 'hp': 128})],
}


def cmd_check(a):
    import numpy as np
    comp = REGISTRY[a.component]()
    n, m = a.n, a.window
    print(f'{comp.name}: {comp.source}')
    first = True
    for note, knobs in CASES.get(a.component, [(69, {})]):
        port, info = bench.run(comp, note, knobs, n)
        if first:
            print(f'bench program: {info["units"]} units -> {info["instructions"]} instructions (scheduled)')
            print('note knobs                      C++=model  SNR 10ms vs C++ / vs model(no overflow)   bands dB '
                  '(20-200-1k-4k-12k-40k, port - C++)')
            first = False
        ref = comp.reference(note, knobs, n)
        exact = ''
        if hasattr(comp, 'model'):
            mod = np.array(comp.model(note, knobs, n, wrap=True))
            exact = 'yes' if np.array_equal(mod, ref) else 'NO'
            mod_nw = np.array(comp.model(note, knobs, n, wrap=False))
            snr2 = compare.snr_db(port[:m], mod_nw[:m])
        else:
            snr2 = float('nan')
        r = compare.report(port, ref, m)
        print(f'{note:4d} {str(knobs):28s} {exact:9s} {r["snr_db"]:6.1f} / {snr2:6.1f}' + ' ' * 22
              + ' '.join(f'{x:+6.2f}' for x in r['bands_db']))


def cmd_lines(a):
    comp = REGISTRY[a.component]()
    src = bench.lines(comp)
    if a.plain:
        print('\n'.join(src))
        return
    _, code = bench.program(src)
    print('\n'.join(code))
    print(f'; {len(code)} instructions')


def cmd_lut(a):
    f = lut.interp88(lut.table(a.module, a.table)) if a.kind == '88' else lut.interp824(lut.table(a.module, a.table))
    if a.odd:
        c, err = lut.fit_odd(f, a.odd, x_max=a.xmax)
    else:
        c, err = lut.fit(f, a.deg, x_min=a.xmin, x_max=a.xmax)
    print('coefficients:', ', '.join(repr(x) for x in c))
    print(f'max error: {err:.2f} LSB (int16)')


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest='cmd', required=True)
    sub.add_parser('list', help='ported components')
    c = sub.add_parser('check', help='port vs original C++ render')
    c.add_argument('component')
    c.add_argument('-n', type=int, default=9600, help='samples per case (default 9600 = 100 ms)')
    c.add_argument('--window', type=int, default=960, help='samples of the sample-by-sample SNR (default 960)')
    ln = sub.add_parser('lines', help='bench program of a component')
    ln.add_argument('component')
    ln.add_argument('--plain', action='store_true', help='one unit per line, unscheduled')
    t = sub.add_parser('lut', help='fit a resources.cc table')
    t.add_argument('module')
    t.add_argument('table')
    t.add_argument('--kind', choices=('88', '824'), default='88', help='Interpolate88 (x in -1..1) or 824 (0..1)')
    t.add_argument('--odd', type=int, help='odd polynomial of this degree')
    t.add_argument('--deg', type=int, default=5, help='full polynomial degree (without --odd)')
    t.add_argument('--xmin', type=float, default=-1.0)
    t.add_argument('--xmax', type=float, default=1.0)
    a = p.parse_args(argv)
    if a.cmd == 'list':
        for name, cls in REGISTRY.items():
            print(f'{name:12s} {cls.source}')
    else:
        {'check': cmd_check, 'lines': cmd_lines, 'lut': cmd_lut}[a.cmd](a)


if __name__ == '__main__':
    main()
