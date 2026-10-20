"""Dedicated live wavetable PLUG-OUT (C-WTLIVE): build, install and send WAV tables, nothing else.

    python3 wtlive.py build                              # plugins/C-WTLIVE (build.sh)
    python3 wtlive.py install                            # slot 3, then select PLG3 on the panel
    python3 wtlive.py send 'PWM Saw 01.wav'              # the table, live (see wt5.py for the options)
    python3 wtlive.py send TABLE.wav --frames 4 --frame 1.5
    python3 wtlive.py send TABLE.wav --osc 1             # OSC1 (DCO1 slot); --osc 2 (default) or --osc both
    python3 wtlive.py info TABLE.wav                     # no hardware
    python3 wtlive.py restore                            # silent test program again

It runs the send code of re/wt5.py (the protocol is documented in re/dsplive.py) but keeps its own
host state, in work/wtlive/ instead of work/dsplive/ and work/wt5/, and only ever installs
plugins/C-WTLIVE, so the lab plugin C-DSPT and its state are not touched. Only one MIDI tool may use
the device at a time. See docs/examples_wt_live_README.md.
"""
import os, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
RE = os.path.join(ROOT, 'examples', 'from_scratch', 're')
PLUGIN = os.path.join(ROOT, 'plugins', 'C-WTLIVE')
WORK = os.path.join(ROOT, 'work', 'wtlive')
SLOT = 3                                # the lab uses slot 3 too: whichever plugout is installed there is the one that runs
sys.path.insert(0, RE)

import dsplive as L                     # noqa: E402  (protocol: MIDI pitch bend + CC1, see dsplive.py)
import dspown                           # noqa: E402  (player load, restore)
import wt5                              # noqa: E402  (WAV -> frames -> programs and bank-5 table)

# Own host state: nothing written by the lab (work/dsplive, work/dspaudio, work/wt5) is read or overwritten.
L.OUT, L.STATE = WORK, os.path.join(WORK, 'state.json')
wt5.OUT = WORK


def build():
    subprocess.run([os.path.join(HERE, 'build.sh')], check=True)


def install():
    """plugins/C-WTLIVE into slot SLOT (replaces what is there). The host state is cleared: the device
    holds the installed mute programs, so the next send writes every word."""
    if not os.path.isdir(PLUGIN):
        raise SystemExit('no plugins/C-WTLIVE: run build first')
    subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'plugout_tool.py'), 'install', PLUGIN,
                    '--slot', str(SLOT), '--yes', '--force'], check=True, stdout=subprocess.DEVNULL)
    os.makedirs(WORK, exist_ok=True)               # empty state: dspown.load expects one, and sends all words
    L.save_state({'paths': {}, 'words': {}, 'installed': {}, 'installed_paths': {}})
    print(f'installed C-WTLIVE in slot {SLOT}: select PLG3 on the panel, then send')


def main():
    os.environ.setdefault('OWN_FULL', '1')      # every program word on every send: no host state to go stale
    if len(sys.argv) < 2 or sys.argv[1] not in ('build', 'install', 'send', 'info', 'restore'):
        raise SystemExit(__doc__)
    cmd, rest = sys.argv[1], sys.argv[2:]
    if cmd == 'build':
        build()
    elif cmd == 'install':
        install()
    elif cmd == 'restore':
        sys.argv = ['dspown.py', 'restore']
        dspown.main()
    else:                               # send / info: the wt5 command line, unchanged
        sys.argv = ['wt5.py', cmd] + rest
        wt5.main()


if __name__ == '__main__':
    main()
