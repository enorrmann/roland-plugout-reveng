"""dsplift (docs/vol5_toolchain_compiler_sdk/dsplift.md §3): the lifted C against the emulator on Roland programs of
work/corpus_all (not in git: skipped when missing). The programs are the ones that exposed lifter bugs while it was
written (tap requests, loops with conditional jumps) and a long voice program.
    python3 -m unittest sdk/s1cc/tests/test_dsplift.py      (about 20 s)"""
import os
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
CORPUS = os.path.join(ROOT, 'work', 'corpus_all')
PROGS = ['dsp4dump_0000fdc', 'p0s_005085c', 'p0s_0051140', 'pl2m_0015c2c', 'JUPITER-8_1_0017d80']


@unittest.skipUnless(os.path.isdir(CORPUS), 'work/corpus_all missing')
class TestDsplift(unittest.TestCase):
    def test_check(self):
        out = os.path.join(tempfile.mkdtemp(prefix='dsplift_test_'), 'lift.c')
        args = ['%s=%s' % (p.replace('-', '_'), os.path.join(CORPUS, p + '.txt')) for p in PROGS]
        r = subprocess.run([sys.executable, os.path.join(ROOT, 'sdk', 's1cc', 'tools', 'dsplift.py'), out, *args,
                            '--check', '--samples', '16'], capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertEqual(r.stdout.count(' OK '), len(PROGS), r.stdout)


if __name__ == '__main__':
    unittest.main()
