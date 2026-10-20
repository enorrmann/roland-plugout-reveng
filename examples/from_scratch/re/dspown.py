"""Load an own program into the DCO2 slot of the slaves (DSP2 and DSP3) through dsplive, and measure it by
USB audio.

    python3 dspown.py load dspexp/own/saw_dco2.txt      # needs the DSPLIVE plugin installed + PLG3 selected
    python3 dspown.py notes NAME [--notes 48 60 72]       # record one note each, print the measured frequency
    python3 dspown.py restore                            # back to the installed test programs
"""
import argparse, json, os, sys, time
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dsplive as L     # noqa: E402
import dspaudio as da   # noqa: E402


def words_of(path):
    return [int(t, 16) for line in open(path) for t in line.split('#')[0].split()]


EMPTY = [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001, 0x0000]     # slot program that returns at once


def load(path, osc=2):
    """the program of OSC osc (1 DCO1 slot, 2 DCO2 slot) on DSP2 and DSP3; the state keeps only OSC2"""
    w = words_of(path)
    if len(w) > 2 * 0x800:
        raise SystemExit('program too long for the DSPLIVE shadow')
    st = L.load_state() or {'paths': {}, 'words': {}, 'installed': {}, 'installed_paths': {}}
    link = L.Link()
    # both DSPs with one command per word (needs a lab built 2026-10-17 or later; OWN_BOTH=0 for older ones).
    # An empty program first (start + return), then the body tail first, the head last: the DSP never runs a
    # mix of old and new code. (2026-10-17: a torn program with a backward jump hung the slaves until a power
    # cycle; forward-only code only glitches.)
    both = os.environ.get('OWN_BOTH', '1') == '1'
    for d in ((2,) if both else (2, 3)):
        link.write(d, 0, EMPTY, both, osc)
        # body: only the runs that differ from what is loaded (the head is the empty program meanwhile)
        old = st['words'].get(str(d), []) if osc == 2 else []
        if osc != 2 or os.environ.get('OWN_FULL') == '1' or (both and st['words'].get('2') != st['words'].get('3')):
            old = []                                 # OSC1 and OWN_FULL=1: send every word (state unsure)
        old = old[:len(EMPTY)] and ([None] * len(EMPTY) + old[len(EMPTY):])
        new = [None] * len(EMPTY) + w[len(EMPTY):]
        for start, chunk in reversed(L.runs(old, new)):
            if start < len(EMPTY):
                chunk, start = chunk[len(EMPTY) - start:], len(EMPTY)
            for k in range(len(chunk) - len(chunk) % 32, -1, -32):
                if chunk[k:k + 32]:
                    link.write(d, start + k, chunk[k:k + 32], both, osc)
        for i in range(len(EMPTY) - 2, -1, -2):       # head: one 32-bit word at a time, the start word last
            link.write(d, i, w[i:i + 2], both, osc)
    link.center()
    if osc == 2:
        for d in (2, 3):
            st['words'][str(d)] = w
        st['own'] = os.path.abspath(path)
        L.save_state(st)
    print(f'loaded {len(w)} words into DSP2/DSP3 DCO2 of both slaves')


def freq(x, rate=da.RATE):
    """fundamental by autocorrelation (refined with parabolic interpolation)"""
    x = x - x.mean()
    n = len(x)
    f = np.fft.rfft(x, 2 * n)
    ac = np.fft.irfft(f * np.conj(f))[:n]
    lo = int(rate / 4000)
    seg = ac[lo:n // 2]
    # first local maximum within 90 % of the best one (argmax alone can pick 2 periods)
    peaks = [i for i in range(1, len(seg) - 1) if seg[i] >= seg[i - 1] and seg[i] >= seg[i + 1] and seg[i] > 0.9 * seg.max()]
    k = lo + (peaks[0] if peaks else int(np.argmax(seg)))
    if 1 <= k < n - 1:
        a, b, c = ac[k - 1], ac[k], ac[k + 1]
        k = k + 0.5 * (a - c) / (a - 2 * b + c)
    return rate / k


def notes(name, ns):
    for n in ns:
        path = da.record(f'{name}_n{n}', n, 0, 2.0)
        x = da.load(path)[40000:150000, 0]
        f = freq(x)
        exp = 440 * 2 ** ((n - 69) / 12)
        print(f'note {n}: {f:9.3f} Hz (equal temperament {exp:8.3f}, ratio {f / exp:.4f}), rms {x.std():.4f}',
              flush=True)
        time.sleep(0.4)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['load', 'notes', 'restore'])
    ap.add_argument('arg', nargs='?')
    ap.add_argument('--notes', type=int, nargs='+', default=[48, 60, 69, 72])
    a = ap.parse_args()
    if a.cmd == 'load':
        load(a.arg)
    elif a.cmd == 'notes':
        notes(a.arg, a.notes)
    else:
        L.Link().cmd(2, 0)
        st = L.load_state()
        st['words'] = json.loads(json.dumps(st['installed']))
        L.save_state(st)
        print('restored')


if __name__ == '__main__':
    main()
