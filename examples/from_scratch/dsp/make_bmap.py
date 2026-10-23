"""Channel map of the slave output words (MAIN5 copies [2204]-[221D] into IOMD B0-B7 at the end of each sample,
see docs/WAVE_BUS.md). The DSP5 slot writes one fixed sawtooth per word: B0 = 110 Hz ... B7 = 495 Hz, 55 Hz steps.
A single capture then shows which word reaches which output channel (both slaves run the same program).

    python3 make_bmap.py      -> bmap.s, bmap.hex
"""
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import make_iomd_test as m  # noqa: E402

FREQS = [110 + 55 * k for k in range(8)]
# words to test: MAIN5 overwrites B0-B7 at the end of each sample (docs/log/WAVE_BUS_LOG.md §2), so the default set
# below is the DSP4 output words that MAIN5 does not write. Set BMAP_SET=b to test B0-B7 again.
OUTS = [0x6020, 0x6021, 0x6022, 0x6023, 0x6024, 0x6060, 0x6061, 0x6062]
if os.environ.get('BMAP_WORDS'):           # e.g. BMAP_WORDS=60b0,60b1,...: exactly eight words
    OUTS = [int(t, 16) for t in os.environ['BMAP_WORDS'].split(',')]
assert len(OUTS) == 8
m.VOICES = [(0x0670 + 4 * k, OUTS[k], 4.75 + math.log2(f / 440)) for k, f in enumerate(FREQS)]
text = m.source()
w, r = m.words(text)
open(os.path.join(HERE, 'bmap.s'), 'w').write(text)
open(os.path.join(HERE, 'bmap.hex'), 'w').write(
    '# channel map: B0..B7 = 110..495 Hz (make_bmap.py)\n' + '\n'.join('%04x' % x for x in w) + '\n')
print(r.summary(), 'words', len(w), 'frequencies', FREQS, 'outputs', [hex(o) for o in OUTS])
