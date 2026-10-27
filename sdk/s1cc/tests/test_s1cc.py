"""s1cc end to end (docs/S1CC.md §13): every example is built (clang -> s1cc -> kernel) and run on the emulator
against its native reference.  python3 -m unittest discover -s sdk/s1cc/tests   (needs clang, LLVM 19, the eurorack
checkout for the Braids examples; about a minute)

The thresholds are the measured results of 2026-10-27 (docs/S1CC.md §11) with a margin: SNR over the first 10 ms (sample by sample:
the port computes what the C computes) and band levels 200 Hz .. 12 kHz within 0.5 dB.
"""
import os
import re
import shutil
import subprocess
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
S1CC = os.path.dirname(HERE)
ROOT = os.path.abspath(os.path.join(S1CC, '..', '..'))
sys.path.insert(0, os.path.join(S1CC, 'tools'))
import numpy as np  # noqa: E402

import s1check  # noqa: E402

EURORACK = os.environ.get('EURORACK', os.path.expanduser('~/git/eurorack'))
BRAIDS = [os.path.join(EURORACK, 'braids', 'resources.cc'), '-I', os.path.join(ROOT, 'sdk', 'mport', 'ref'),
          '-I', EURORACK]
SIMPLE_SAW = os.path.join(S1CC, 'examples', 'simple_saw_wrapped', 'saw.c')
HAVE = shutil.which('clang') and os.path.exists('/usr/lib/llvm-19/bin/llvm-link')

# example, extra build args, [(note, knobs, min SNR A, min SNR B)], max instructions A / B
CASES = [
    ('braids_saw_swarm.cc', BRAIDS, [(57, {}, 50, 40), (45, {'ratio': 200, 'hp_offset': 85}, 50, 35),
                                     (81, {'ratio': 255, 'hp_offset': 100}, 50, 40)], (52, 50)),
    ('braids_saw_swarm_s1.cc', BRAIDS, [(57, {}, 50, 40), (45, {'ratio': 200, 'hp_h': 85}, 50, 35),
                                        (81, {'ratio': 255, 'hp_h': 100}, 50, 40)], (52, 30)),
    ('va_saw_svf.c', [], [(45, {}, 60, 50), (69, {'cutoff': 200, 'damping': 200}, 60, 45),
                          (93, {'cutoff': 60, 'damping': 50}, 60, 60)], (100, 42)),
    # a plain C library and its wrapper, no S1_VOICE_B (min SNR B / max instructions B: None)
    ('simple_saw_wrapped/simple_saw_wrapped.c', [SIMPLE_SAW, '-I', os.path.dirname(SIMPLE_SAW)],
     [(33, {'level': 255}, 90, None), (57, {}, 90, None), (81, {'level': 255}, 90, None)], (22, None)),
]


@unittest.skipUnless(HAVE, 'needs clang and LLVM 19')
class TestExamples(unittest.TestCase):
    def build(self, src, extra):
        out = os.path.join(S1CC, 'build', 'test_' + os.path.splitext(os.path.basename(src))[0])
        cmd = [os.path.join(S1CC, 's1cc'), 'build', os.path.join(S1CC, 'examples', src)] + extra + ['-o', out]
        r = subprocess.run(cmd, capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stderr[-2000:])
        return out

    def test_examples(self):
        for src, extra, runs, (maxa, maxb) in CASES:
            if extra and not os.path.isdir(os.path.join(EURORACK, 'braids')):
                continue
            with self.subTest(src=src):
                out = self.build(src, extra)
                for note, knobs, sa, sb in runs:
                    pa, pb, info = s1check.run_kernel(out, note, 4800, knobs)
                    ra, rb = s1check.run_ref(out, note, 4800, knobs)
                    self.assertGreater(s1check.snr(pa[:960], ra[:960]), sa, (src, note, knobs, 'A'))
                    if sb is not None:
                        self.assertGreater(s1check.snr(pb[:960], rb[:960]), sb, (src, note, knobs, 'B'))
                    for p, r in ((pa, ra), (pb, rb)) if sb is not None else ((pa, ra),):
                        d = np.array(s1check.bands(p)[1:4]) - np.array(s1check.bands(r)[1:4])
                        self.assertTrue(np.all(np.abs(d) < 0.5), (src, note, knobs, d))
                self.assertLessEqual(info['A'][1], maxa, src)
                if maxb is not None:
                    self.assertLessEqual(info['B'][1], maxb, src)


if __name__ == '__main__':
    unittest.main()


class TestErrorCodes(unittest.TestCase):
    """Every error / warning code of the compiler and the driver has its row in docs/S1CC.md §9.2, and back."""

    def test_codes_documented(self):
        src = ''
        for d in (os.path.join(S1CC, 'src'),):
            for f in sorted(os.listdir(d)):
                src += open(os.path.join(d, f)).read()
        src += open(os.path.join(S1CC, 's1cc')).read()
        used = set(re.findall(r'"([EW]\d{3})"', src)) | set(re.findall(r'\bfail ([EW]\d{3})\b', src))
        with open(os.path.join(ROOT, 'docs', 'S1CC.md')) as f:
            doc = f.read()
        documented = set(re.findall(r'^\| `([EW]\d{3})`', doc, re.M)) | set(re.findall(r'^\| `[EW]\d{3}`, `([EW]\d{3})`', doc, re.M))
        self.assertEqual(used - documented, set(), 'codes without a row in docs/S1CC.md §9.2')
        self.assertEqual(documented - used, set(), 'documented codes the compiler never emits')
