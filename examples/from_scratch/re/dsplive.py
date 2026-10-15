"""Live DSP microcode pokes over MIDI (engine.c -DDSPLIVE): change the test programs without reinstalling.

    python3 dsplive.py install b14.batch     # build C-DSPT with DSPTEST + DSPLIVE, install in slot 3
        (then select PLG3 on the panel, as with dspaudio.py)
    python3 dsplive.py poke d2a=r_08.txt d3b=n_pass.txt   # rewrite those test paths in the running DSPs
    python3 dsplive.py restore               # back to the programs as installed
    python3 dsplive.py recall NAME           # dspaudio recall (eight notes) + report against b11

Transport (no region 0 changes): pitch bend carries 14 data bits (voice id 0x465), then CC1 carries
the command (voice id 0x467); the stock-path region 0 forwards both to the slaves, whose engine acts
when 0x467 changes (bit 6 toggles every command):
    CC1 = seq<<6 | cmd<<4 | ext     cmd 0 ADDR (dsp 2 + ext, word index = bend)
                                    cmd 1 DATA (word = bend | ext<<14, stored at the index, index++)
                                    cmd 2 CTRL (ext 0 restore, ext 1 nop)
Words = 16-bit microcode words in execution order, written into the DCO2 slot of DSP2 (t0, paths
d2a/d2b) or DSP3 (t1, d3a/d3b). Both slaves receive the same pokes. Only changed words are sent;
the state of what is loaded is kept in work/dsplive/state.json.
"""
import argparse, json, os, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dspaudio as da   # noqa: E402

OUT = os.path.join(da.ROOT, 'work', 'dsplive')
STATE = os.path.join(OUT, 'state.json')
DELAY = 0.015            # seconds between commands: the master sends parameter changes in batches


def midi_dev():
    for line in open('/proc/asound/cards'):
        w = line.split()
        if len(w) > 1 and w[1] == '[SYSTEM8':
            return f'/dev/snd/midiC{w[0]}D0'
    raise SystemExit('SYSTEM-8 not found')


class Link:
    def __init__(self, ch=0, delay=DELAY):
        self.f = open(midi_dev(), 'wb', buffering=0)
        self.ch, self.delay, self.seq = ch, delay, 0
        self.cmd(2, 1)                       # two nops: afterwards every command is a change
        self.cmd(2, 1)

    def cmd(self, cmd, ext, data=0x2000):
        assert 0 <= data < 0x4000 and 0 <= ext < 16
        self.seq ^= 1
        self.f.write(bytes([0xE0 | self.ch, data & 0x7F, data >> 7,
                            0xB0 | self.ch, 1, self.seq << 6 | cmd << 4 | ext]))
        time.sleep(self.delay)

    def write(self, dsp, start, words):
        self.cmd(0, dsp - 2, start)
        for w in words:
            self.cmd(1, w >> 14, w & 0x3FFF)

    def center(self):
        self.f.write(bytes([0xE0 | self.ch, 0, 0x40]))


def words_of(path):
    return [int(t, 16) for line in open(path) for t in line.split('#')[0].split()]


def program(paths, d):
    """the DCO2 program of DSP d (2/3) for the test paths {a: file, b: file}"""
    os.makedirs(OUT, exist_ok=True)
    out = os.path.join(OUT, f'prog_d{d}.txt')
    return words_of(da.combine(paths, out))


def load_state():
    try:
        return json.load(open(STATE))
    except (OSError, ValueError):
        return None


def save_state(st):
    os.makedirs(OUT, exist_ok=True)
    json.dump(st, open(STATE, 'w'), indent=1)


def state_from_batch(batch):
    paths = {}
    for k, f in ((k, f) for k, f in (t.split('=') for t in
                 (l.split('#')[0].strip() for l in open(batch)) if t) if k in da.GAIN_B11):
        paths[k] = f if os.path.exists(f) else os.path.join(HERE, 'dspexp', 'audio', f)
    st = {'paths': {k: os.path.abspath(v) for k, v in paths.items()}, 'words': {}}
    for d in (2, 3):
        if f'd{d}a' in paths:
            st['words'][str(d)] = program({v: paths[f'd{d}{v}'] for v in 'ab'}, d)
    return st


def runs(old, new):
    """(start, words) runs of new that differ from old"""
    i, n, out = 0, len(new), []
    while i < n:
        if i < len(old) and old[i] == new[i]:
            i += 1
            continue
        j = i
        while j < n and not (j < len(old) and old[j] == new[j]):
            j += 1
        out.append((i, new[i:j]))
        i = j
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['install', 'poke', 'restore', 'recall', 'report'])
    ap.add_argument('args', nargs='*')
    ap.add_argument('--full', action='store_true', help='poke: send every word, not only changed ones')
    ap.add_argument('--delay', type=float, default=DELAY)
    a = ap.parse_args()
    if a.cmd == 'install':
        batch = a.args[0]
        if not os.path.exists(batch):
            batch = os.path.join(HERE, 'dspexp', 'audio', batch)
        os.environ['DSPLIVE'] = '1'
        spec = da.spec_of(batch)
        da.build(','.join(f'{k}={f}' for k, f in spec))
        da.install()
        st = state_from_batch(batch)
        st['installed'] = json.loads(json.dumps(st['words']))
        save_state(st)
        print('installed: select PLG3 on the panel, then poke / recall')
        return
    st = load_state()
    if st is None:
        raise SystemExit('no state: run install first')
    if a.cmd == 'restore':
        Link(delay=a.delay).cmd(2, 0)
        st['words'] = json.loads(json.dumps(st['installed']))
        save_state(st)
        return
    if a.cmd == 'poke':
        for t in a.args:
            k, f = t.split('=')
            if k not in da.GAIN_B11:
                raise SystemExit(f'{k}: expected d2a d2b d3a d3b')
            st['paths'][k] = os.path.abspath(f if os.path.exists(f) else os.path.join(HERE, 'dspexp', 'audio', f))
        link = Link(delay=a.delay)
        for d in (2, 3):
            new = program({v: st['paths'][f'd{d}{v}'] for v in 'ab'}, d)
            old = [] if a.full else st['words'].get(str(d), [])
            if len(new) > 2 * 0x800:
                raise SystemExit(f'd{d}: program too long')
            # the tail first, the start last: the old end marker stays valid until the new code reaches it
            for start, w in reversed(runs(old, new)):
                link.write(d, start, w)
                print(f'd{d}: {len(w)} words at {start}')
            st['words'][str(d)] = new
        link.center()
        save_state(st)
        return
    name = a.args[0]
    batch = os.path.join(OUT, name + '.batch')
    if a.cmd == 'recall':
        with open(batch, 'w') as f:      # what was loaded, for report
            for k, p in sorted(st['paths'].items()):
                f.write(f'{k}={os.path.basename(p)}\n')
        for n in da.VOICE_NOTES:
            da.show(da.record(f'{name}_n{n}', n, 0, 2.0))
            time.sleep(0.5)
    da.report(batch)


if __name__ == '__main__':
    main()
