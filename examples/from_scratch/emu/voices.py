"""Program sets (ours only) and the ARM-side voice writes of v3/engine_v3.c, for chain.py.

    python3 voices.py v3|v4 [SAMPLES]        one voice at a time on slave 0: level and frequency at 6020
"""
import os
import sys

import chain

HERE = os.path.dirname(os.path.abspath(__file__))
FS_DIR = os.path.abspath(os.path.join(HERE, '..'))
for p in (os.path.join(FS_DIR, 'v3'), os.path.join(FS_DIR, 'v4')):
    if p not in sys.path:
        sys.path.insert(0, p)

import make_v3  # noqa: E402
import make_v4  # noqa: E402


def hex_words(path):
    """words of a .hex/.txt program file ('#' starts a comment)"""
    out = []
    for line in open(path):
        out += [int(t, 16) for t in line.split('#')[0].split()]
    return out


def slot_programs():
    """DCO1 slot = our null program (gate/dco2_null.hex), DCO2 slot = our saw (v3saw), as engine_v3.c loads them.
    The DCO1 slot runs first (task 2 before task 3) [inferido: if the null ran after the saw it would overwrite the
    saw's voice A phase word 0670, and voice A sounds on the device]"""
    null = chain.Program(hex_words(os.path.join(FS_DIR, 'gate', 'dco2_null.hex')), 'dco1-null')
    saw = chain.Program(make_v3.saw_dco2_words(), 'dco2-saw')
    return [null, saw]


def programs(which='v3', sources=None):
    """{dsp: (main, [slots])} for v3 (make_v3.sources) or v4 (make_v4.sources); `sources` replaces the mains"""
    src = sources or (make_v3.sources() if which == 'v3' else make_v4.sources())
    slots = slot_programs()
    out = {}
    for d, text in src.items():
        w, _ = make_v3.words(text)
        out[d] = (chain.Program(w, f'{which}m{d}'), slots if d in (2, 3) else [])
    return out


class Engine:
    """engine_v3.c voice writes for one slave (lv = local voice 0..3):
    voice_pitch: bank 5 of DSP lv>>1, id 0x21 + 0x12*(lv&1), value (note - 12) / 12
    voice_gate:  bank 5 of DSP 4 + (lv>>1), id 0x2A + 0xF*(lv&1) - (lv>>1); on: pitch first, then 1.0; off: 0.0"""

    def __init__(self, slave):
        self.s = slave
        self.note = [60] * 4
        for lv in range(4):
            self.gate(lv, 0)

    def pitch(self, lv, note):
        self.note[lv] = note
        self.s.p_bank(lv >> 1, 0x21 + 0x12 * (lv & 1), (note - 12) / 12.0)

    def gate(self, lv, vel):
        u, k = lv >> 1, lv & 1
        if vel > 0:
            self.pitch(lv, self.note[lv])
        self.s.p_bank(4 + u, 0x2A + 0xF * k - u, 1.0 if vel > 0 else 0.0)

    def note_on(self, lv, note):
        self.note[lv] = note
        self.gate(lv, 100)

    def note_off(self, lv):
        self.gate(lv, 0)


def play(slave, events, n, words=chain.AUDIBLE):
    """run n samples; events = {t: [(lv, note or None)]} (None = off). Returns {word: [values]}"""
    eng = Engine(slave)
    out = {w: [] for w in words}
    for t in range(n):
        for lv, note in events.get(t, []):
            if note is None:
                eng.note_off(lv)
            else:
                eng.note_on(lv, note)
        slave.step()
        for w in words:
            out[w].append(slave.iomd.get(w, 0.0))
    return out


def one_voice(which, lv, model=None, note=69, n=1500, other=57, on=200):
    """voice lv alone (the others hold `other` as pitch, gates off): level and frequency at 6020"""
    s = chain.Slave(programs(which), model)
    eng = Engine(s)
    for k in range(4):
        eng.pitch(k, other)
    out = []
    for t in range(n):
        if t == on:
            eng.note_on(lv, note)
        s.step()
        out.append(s.iomd.get(0x6020, 0.0))
    seg = out[on + 100:]
    return {'peak': chain.peak(seg), 'freq': chain.freq(seg), 'silent_before': chain.peak(out[:on])}


if __name__ == '__main__':
    which = sys.argv[1] if len(sys.argv) > 1 else 'v3'
    n = int(sys.argv[2]) if len(sys.argv) > 2 else 1500
    for lv in range(4):
        r = one_voice(which, lv, n=n)
        print(f'{which} voice {lv}: peak {r["peak"]:.4f}  freq {r["freq"]:7.1f} Hz (note 69 = 440.0; others 220.0)'
              f'  before gate {r["silent_before"]:.4f}', flush=True)
