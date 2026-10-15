"""Listen to the DSP 'ports' (X9bY writes, Xa9Y reads) on the lab plugin (DCO2 slot of the slaves, live pokes).

    python3 dspports.py write 0 1 2 3 4 5 6 7     # noise -> port Y, outputs silent
    python3 dspports.py read  0 1 2 3 4 5 6 7     # port Y -> outputs
"""
import os, sys, time
import numpy as np
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
import dspown, dspaudio as da

OUTS = ['2221', '221d', '2229', '2225']


def prog(kind, y):
    L = ['0a02 ff70', '0903 0050 0000']
    if kind == 'write':
        L += ['0c03 3fe0 2200', '0001', '0001', f'1803 0040 b9b{y:x}']
        L += [f'0c03 8fe0 {o}' for o in OUTS]                     # outputs = r0 = 0
    elif kind == 'read':
        L += ['0001', f'1803 0040 3a9{y:x}', '0001', '0001']
        L += [f'0c03 bfe0 {o}' for o in OUTS]                     # outputs = r3
    else:                                                          # 'noise': reference, noise to the outputs
        L += ['0c03 3fe0 2200', '0001', '0001'] + [f'0c03 bfe0 {o}' for o in OUTS]
    L += ['0a02 ff70', '0b02 00f8', '0001']
    path = os.path.join(HERE, 'dspexp', 'own', f'port_{kind}_{y}.txt')
    open(path, 'w').write('\n'.join(L) + '\n')
    return path


def main():
    kind, ys = sys.argv[1], [int(a) for a in sys.argv[2:]] or [0]
    for y in ys:
        dspown.load(prog(kind, y))
        time.sleep(0.2)
        p = da.record(f'PORT_{kind}_{y}', 60, 0, 1.5, hold=1.0)
        x = da.load(p)[:, 0]
        on, off = x[35000:110000], x[140000:]
        S = np.abs(np.fft.rfft(on * np.hanning(len(on)))); f = np.fft.rfftfreq(len(on), 1 / 96000)
        print(f'{kind} port {y}: rms during note {on.std():.5f}, after {off.std():.5f}, '
              f'peak {f[np.argmax(S[10:]) + 10]:.0f} Hz', flush=True)


if __name__ == '__main__':
    main()
