"""DSP ISA test bench on the hardware: test programs in the DCO2 slot, recorded by USB audio.

    python3 dspaudio.py install b11.batch       # build plugins/C-DSPT, install in slot 3 (--force)
        (then select PLG3 on the panel: after an install the device is back on SYSTEM-8)
    python3 dspaudio.py recall b11              # eight notes, one per voice (VOICE_NOTES)
    python3 dspaudio.py rec name [--note 60]    # one note, one recording
    python3 dspaudio.py show work/dspaudio/x.raw

A batch maps names to test programs (`t0=x.txt`, `t1=`, and program-name
prefixes such as `dco1_=z_dco1.txt`; files in re/dspexp/audio): 16-bit words in
execution order (hex, '#' comments), as printed by dspdis.py. engine/build.sh
with DSPTEST=1 MC_TEST=<spec> puts them in place (programs.py): the DCO2 slot of
DSP2 runs t0 and the one of DSP3 t1, loaded at start-up (engine.c -DDSPTEST).
Each test program has a voice A path (outputs [2221] [221d]) and a voice B path
([2229] [2225]), so one install carries four tests; the assigner gives eight
different notes eight voices (2 slaves x DSP2/3 x A/B; map: calibration batch
b11). Live program swaps (+0x70) from note events were tried and were not
reliable (programs silent or the previous one kept).
Recordings: work/dspaudio/<name>.raw (float32 interleaved L R, 96 kHz).
"""
import argparse, os, subprocess, sys, threading, time
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
PLUGIN = os.path.join(ROOT, 'plugins', 'C-DSPT')
OUT = os.path.join(ROOT, 'work', 'dspaudio')
RATE = 96000


def build(spec):
    env = dict(os.environ, DSPTEST='1', MC_TEST=spec)
    subprocess.run([os.path.join(HERE, '..', 'engine', 'build.sh'), os.path.join(ROOT, 'plugins', 'JX-3P'), PLUGIN],
                   check=True, env=env, stdout=subprocess.DEVNULL)


def install(slot=3):
    subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'plugout_tool.py'), 'install', PLUGIN,
                    '--slot', str(slot), '--yes', '--force'], check=True, stdout=subprocess.DEVNULL)


def midi_port():
    out = subprocess.run(['amidi', '-l'], capture_output=True, text=True).stdout
    for line in out.splitlines():
        if 'SYSTEM-8 MIDI 1' in line:
            return line.split()[1]
    raise SystemExit('SYSTEM-8 MIDI 1 not found')


def record(name, note=45, ch=0, secs=2.0, on=0.3, hold=1.0, vel=100):
    os.makedirs(OUT, exist_ok=True)
    path = os.path.join(OUT, name + '.raw')
    port = midi_port()

    def play():
        time.sleep(on)
        subprocess.run(['amidi', '-p', port, '-S', f'{0x90 | ch:02x} {note:02x} {vel:02x}'])
        time.sleep(hold)
        subprocess.run(['amidi', '-p', port, '-S', f'{0x80 | ch:02x} {note:02x} 00'])
    t = threading.Thread(target=play)
    t.start()
    subprocess.run(['arecord', '-q', '-D', 'hw:SYSTEM8,0', '-d', str(int(np.ceil(secs))), '-f', 'FLOAT_LE',
                    '-r', str(RATE), '-c', '2', '-t', 'raw', path], check=True)
    t.join()
    return path


def load(path):
    return np.fromfile(path, '<f4').reshape(-1, 2)


def show(path):
    a = load(path)
    print(f'{os.path.basename(path)}: {len(a)} frames')
    for c, n in enumerate('LR'):
        x = a[:, c]
        nz = np.nonzero(x)[0]
        if not len(nz):
            print(f'  {n}: silent')
            continue
        v, k = np.unique(x[nz[0]:nz[-1] + 1], return_counts=True)
        top = sorted(zip(k, v), reverse=True)[:6]
        print(f'  {n}: nonzero {nz[0]}..{nz[-1]}  min {x.min():.6g} max {x.max():.6g}  rms {np.sqrt(np.mean(x[nz[0]:nz[-1]+1] ** 2)):.6g}'
              f'  distinct {len(v)}  most common ' + ', '.join(f'{val:.7g}x{cnt}' for cnt, val in top))


VOICE_NOTES = [60, 62, 64, 65, 67, 69, 71, 72]   # eight different notes: the assigner gives each a new voice
# which test path plays each of them (calibration b11, measured gains 1/8 1/4 1/2 1, twice: both slaves)
PATH_OF = ['d3b', 'd3a', 'd2b', 'd2a'] * 2
GAIN_B11 = {'d2a': 1.0, 'd2b': 0.5, 'd3a': 0.25, 'd3b': 0.125}
OUTS = {'a': '2221 221d', 'b': '2229 2225'}


def path_body(f):
    """a single-path test file (dspexp/audio/n_*.txt style: body, then `7809 Xfe0 .. 2221 221d 2229 2225`)
    -> (body lines, register X stored to the outputs)"""
    body, reg = [], None
    for line in open(f):
        w = line.split('#')[0].split()
        if not w or w in (['0a02', 'ff70'], ['0b02', '00f8'], ['0001']):
            continue
        if w[0] == '7809' and w[5:] == ['2221', '221d', '2229', '2225']:
            reg = w[1][0]
            continue
        body.append(' '.join(w))
    if reg is None:
        raise SystemExit(f'{f}: no output store line')
    return body, reg


def combine(paths, out):
    """two single-path tests -> one DCO2 program: voice A path, then voice B path"""
    L = ['0a02 ff70']
    for v in 'ab':
        f = paths[v]
        body, reg = path_body(f)
        L += [f'# voice {v.upper()}: {os.path.basename(f)}'] + body + [f'1e05 {reg}fe0 {reg}fe2 {OUTS[v]}']
    L += ['0a02 ff70', '0b02 00f8', '0001']
    open(out, 'w').write('\n'.join(L) + '\n')
    return out


def spec_of(arg):
    """'dco2_saw=a.txt,dco2_sqr=b.txt' (prefix=file, relative to re/dspexp/audio) or a .batch file of such lines"""
    if arg.endswith('.batch'):
        arg = ','.join(l.split('#')[0].strip() for l in open(arg) if l.split('#')[0].strip())
    out, paths = [], {}
    for t in arg.split(','):
        k, f = t.split('=')
        if not os.path.exists(f):
            f = os.path.join(HERE, 'dspexp', 'audio', f)
        if k in GAIN_B11:                      # d2a= d2b= d3a= d3b=: combined into t0 (DSP2), t1 (DSP3)
            paths[k] = os.path.abspath(f)
        else:
            out.append((k, os.path.abspath(f)))
    os.makedirs(OUT, exist_ok=True)
    for t, d in (('t0', 'd2'), ('t1', 'd3')):
        if d + 'a' in paths:
            out.append((t, combine({v: paths[d + v] for v in 'ab'}, os.path.join(OUT, f'prog_{d}.txt'))))
    return out


def report(batch, cal='b11'):
    """level and spectrum of each voice note relative to the calibration batch (same note, gain-normalized)"""
    def bands(path):
        x = load(path)[60000:120000, 0]
        S = np.abs(np.fft.rfft(x))
        f = np.fft.rfftfreq(len(x), 1 / RATE)
        return x.std(), np.array([np.sqrt((S[(f >= a) & (f < b)] ** 2).mean()) for a, b in
                                  ((0, 300), (300, 1000), (1000, 3000), (3000, 8000))])
    spec = {}
    bf = batch if os.path.exists(batch) else os.path.join(HERE, 'dspexp', 'audio', batch + '.batch')
    for line in open(bf):
        w = line.split('#')[0].strip()
        if '=' in w:
            spec[w.split('=')[0]] = w.split('=')[1]
    for i, n in enumerate(VOICE_NOTES):
        p = PATH_OF[i]
        r, rb = bands(os.path.join(OUT, f'{cal}_n{n}.raw'))
        x, xb = bands(os.path.join(OUT, f'{os.path.basename(batch).removesuffix(".batch")}_n{n}.raw'))
        g = GAIN_B11[p]
        print(f'{n} {p} {spec.get(p, "?"):<22} gain {x / r * g:7.4f}   bands ' + ' '.join(f'{v:.3f}' for v in xb / rb * g))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['build', 'install', 'rec', 'recall', 'show', 'report'])
    ap.add_argument('arg', help='build/install: spec or .batch; rec: name; recall: batch name; show: .raw')
    ap.add_argument('--note', type=int, default=60)
    ap.add_argument('--ch', type=int, default=0)
    ap.add_argument('--secs', type=float, default=2.0)
    a = ap.parse_args()
    if a.cmd == 'show':
        return show(a.arg)
    if a.cmd == 'report':
        return report(a.arg)
    if a.cmd in ('build', 'install'):
        spec = spec_of(a.arg)
        build(','.join(f'{k}={f}' for k, f in spec))
        if a.cmd == 'install':
            install()
            print('installed: select PLG3 on the panel, then: dspaudio.py recall <batch name>')
        return
    if a.cmd == 'recall':                      # one note per voice, right after PLG3 was selected
        for n in VOICE_NOTES:
            show(record(f'{a.arg}_n{n}', n, a.ch, a.secs))
            time.sleep(0.5)
        return
    show(record(a.arg, a.note, a.ch, a.secs))


if __name__ == '__main__':
    main()
