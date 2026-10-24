"""Reference renders from the original Mutable Instruments C++ (docs/MUTABLE_SDK.md §6).

The eurorack checkout (EURORACK, default ~/git/eurorack) is compiled as it is, with the small stmlib stand-ins of
sdk/mport/ref/stmlib (the submodule is empty there). The binary goes to sdk/mport/build/ (not in git) and is rebuilt
when a source is newer.

    import reference
    x = reference.braids(shape=1, note=69, p1=16000, p2=10922, n=9600)    # numpy float array, int16 / 32768
"""
import os
import subprocess

import numpy as np

from lut import EURORACK

HERE = os.path.dirname(os.path.abspath(__file__))
BUILD = os.path.join(HERE, 'build')

BRAIDS_SHAPES = {'saw_swarm': 1}       # braids::DigitalOscillatorShape (digital_oscillator.h)


def _build(name, main, sources):
    exe = os.path.join(BUILD, name)
    deps = [main] + sources + [os.path.join(dp, f) for dp, _, fs in os.walk(os.path.join(HERE, 'ref', 'stmlib'))
                               for f in fs]
    if os.path.exists(exe) and all(os.path.getmtime(exe) >= os.path.getmtime(d) for d in deps):
        return exe
    os.makedirs(BUILD, exist_ok=True)
    cmd = ['g++', '-O2', '-w', '-I' + os.path.join(HERE, 'ref'), '-I' + EURORACK, main] + sources + ['-o', exe]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode:
        raise RuntimeError('reference build failed:\n' + ' '.join(cmd) + '\n' + r.stderr[-3000:])
    return exe


def braids_exe():
    b = os.path.join(EURORACK, 'braids')
    return _build('braids_render', os.path.join(HERE, 'ref', 'braids_render.cc'),
                  [os.path.join(b, 'digital_oscillator.cc'), os.path.join(b, 'resources.cc')])


def braids(shape, note, p1, p2, n, seed=1, block=24, strike=True):
    """n samples of braids::DigitalOscillator (shape: index or name), pitch = MIDI note << 7, parameters 0..32767;
    strike=False: the oscillator state starts at 0 (as a port's bank-0 state) instead of the random strike phases"""
    shape = BRAIDS_SHAPES.get(shape, shape)
    out = subprocess.run([braids_exe(), str(int(shape)), str(int(round(note * 128))), str(int(p1)), str(int(p2)),
                          str(int(n)), str(seed), str(block), str(int(strike))], capture_output=True, check=True).stdout
    return np.frombuffer(out, dtype='<i2').astype(float) / 32768.0
