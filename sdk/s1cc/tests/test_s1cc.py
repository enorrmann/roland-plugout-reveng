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
HAVE = shutil.which('clang') and os.path.exists('/usr/lib/llvm-19/bin/llvm-link')

# example, extra build args, [(note, knobs, min SNR A, min SNR B)], max instructions A / B
CASES = [
    ('braids_saw_swarm.cc', BRAIDS, [(57, {}, 50, 40), (45, {'ratio': 200, 'hp_offset': 85}, 50, 35),
                                     (81, {'ratio': 255, 'hp_offset': 100}, 50, 40)], (52, 52)),   # B: --tollog 0.0005
    ('braids_saw_swarm_s1.cc', BRAIDS, [(57, {}, 50, 40), (45, {'ratio': 200, 'hp_h': 85}, 50, 35),
                                        (81, {'ratio': 255, 'hp_h': 100}, 50, 40)], (52, 30)),
    ('va_saw_svf.c', [], [(45, {}, 60, 50), (69, {'cutoff': 200, 'damping': 200}, 60, 45),
                          # B 55: since float phases are held in [-1, 1) (one ph_wrap) they start at half a cycle; at
                          # this low cutoff the reference's output is 0.021 instead of 0.034 for the same error
                          (93, {'cutoff': 60, 'damping': 50}, 60, 54)], (100, 42)),
    # Plaits as it is, with the real stmlib (s1cc deps): skipped without lib/stmlib; W908 (a branch never profiled)
    ('plaits_variable_shape.cc', ['-I', EURORACK], [(45, {}, 60, None), (69, {'pw': 200, 'shape': 50}, 70, None),
                                                    (81, {'shape': 255}, 75, None)], (330, None)),
    # mutable_ported: C written for this DSP (s1_dt / s1_rdt, branch-free polyBLEP, the shared SVF + envelopes in B);
    # the instruction counts are what lets 8 voices fit (docs/examples_mutable_ported_README.md)
    ('mutable_ported/mp_saw.c', ['--regs', '0,1,2,3,4,5,6,7'], [(45, {}, 65, 50), (81, {'cutoff': 200}, 60, 50)], (27, 45)),
    ('mutable_ported/mp_csaw.c', ['--regs', '0,1,2,3,4,5,6,7'], [(57, {'width': 40, 'depth': 200}, 65, 50)], (68, 45)),
    # cutoff open: with the filter at 128 the 1-4 kHz band of B is ~100 dB down, where only the filter's noise floor is
    ('mutable_ported/mp_harmonic.c', ['--regs', '0,1,2,3,4,5,6,7'], [(57, {'amp': 200, 'cutoff': 255}, 75, 50)], (86, 45)),
    ('mutable_ported/mp_filter_ladder.c', ['--regs', '0,1,2,3,4,5,6,7'], [(45, {'reso': 200}, 65, 60)], (27, 46)),
    # s1_export / s1_import: DSP A's note frequency to DSP B's key-tracked low-pass
    ('simple_saw_wrapped/simple_saw_export.c', ['--regs', '0,1,2,3,4,5,6,7'],
     [(45, {'track': 100}, 90, 85), (69, {'track': 200}, 90, 85)], (31, 17)),
    # a plain C library, two instances per voice, and its wrapper; no S1_DSP_B (min SNR B / max instructions B: None)
    # saw.c comes in by itself (the source next to saw.h: s1cc build, automatic sources)
    ('simple_saw_wrapped/simple_saw_wrapped.c', [],
     [(33, {'level': 255, 'detune': 255}, 90, None), (57, {'detune': 0}, 90, None),
      (81, {'level': 255, 'detune': 255}, 90, None)], (33, None)),
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
            if src.startswith('plaits') and not os.path.isdir(os.path.join(S1CC, 'lib', 'stmlib', 'dsp')):
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
                        # bands 1..3 that carry sound: within 60 dB of the loudest (an empty band shows only the
                        # DSP's numeric floor: the additive oscillator above its last harmonic)
                        bp, br = np.array(s1check.bands(p)), np.array(s1check.bands(r))
                        live = br[1:4] > br.max() - 60
                        d = (bp[1:4] - br[1:4])[live]
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


@unittest.skipUnless(HAVE, 'needs clang and LLVM 19')
class TestSystemS1cc(unittest.TestCase):
    """examples/system_s1cc (SYSTEM-S1CC): the PLUG-OUT that is only the kernel. The whole chain on the emulator
    (slave A, slave B, master) against the kernel's native reference, and the instruction budget guard."""

    def setUp(self):
        import tempfile
        self.out = tempfile.mkdtemp()
        kernel = os.path.join(S1CC, 'build', 'test_simple_saw_env')
        r = subprocess.run([os.path.join(S1CC, 's1cc'), 'build',
                            os.path.join(S1CC, 'examples', 'simple_saw_wrapped', 'simple_saw_env.c'), '-o', kernel,
                            '--regs', '0,1,2,3,4,5,6,7'],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stderr[-2000:])
        self.env = dict(os.environ, KERNEL=kernel, OUTDIR=self.out)
        self.gen = os.path.join(ROOT, 'examples', 'system_s1cc', 'make_s1cc.py')

    def tearDown(self):
        shutil.rmtree(self.out, ignore_errors=True)

    def test_chain(self):
        for voices in ('8', '2'):                      # simple_saw_env: saws on DSP A (29), the ADSR on DSP B (46)
            r = subprocess.run([sys.executable, self.gen, '--sim'], env=dict(self.env, VOICES=voices),
                               capture_output=True, text=True)
            self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
            snrs = [float(x) for x in re.findall(r'SNR against the native reference ([-0-9.]+) dB \(10 ms\)', r.stdout)]
            self.assertEqual(len(snrs), 2, r.stdout)
            # 88: the saw's phase starts at half a cycle since float phases are held in [-1, 1) (10 ms 89.7 dB
            # instead of 91.8, and over the whole run 72 dB instead of 24)
            self.assertTrue(all(x > 88 for x in snrs), (voices, r.stdout))
            self.assertIn('no notes: peak 0.000000', r.stdout)
            self.assertNotIn('warning', r.stdout)

    def test_header_layout(self):
        # the OS loader reads 32-bit words with the first halfword in the high half; a uint16_t array would reach the
        # DSP with every pair swapped (the first SYSTEM-S1CC builds: the device hung when the PLUG-OUT was selected)
        r = subprocess.run([sys.executable, self.gen], env=self.env, capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        h = open(os.path.join(self.out, 'build', 's1_programs.h')).read()
        for name in ('S1M0', 'S1M1', 'S1MM1'):
            m = re.search(r'static const uint32_t mc_%s\[\d+\] = \{\s*(0x[0-9a-f]{8})' % name, h)
            self.assertIsNotNone(m, name)
            self.assertEqual(m.group(1), '0x0a02ff71', name)        # the section start marker 0a02 ff71

    def test_budget_guard(self):
        r = subprocess.run([sys.executable, self.gen], env=dict(self.env, BUDGET='100'), capture_output=True, text=True)
        self.assertNotEqual(r.returncode, 0)
        self.assertIn('over the budget of 100 instructions', r.stdout + r.stderr)


@unittest.skipUnless(HAVE, 'needs clang and LLVM 19')
class TestSystemS1ccHostEnvelopes(unittest.TestCase):
    """SYSTEM-S1CC with the host's envelopes (s1_env): the assembly ADSRs on DSP B, by shape (the native reference
    has the gate for s1_env), and a gate that comes after the trigger"""

    def test_host_envelopes(self):
        import tempfile
        out = tempfile.mkdtemp()
        try:
            kernel = os.path.join(S1CC, 'build', 'test_simple_saw_hostenv')
            r = subprocess.run([os.path.join(S1CC, 's1cc'), 'build',
                                os.path.join(S1CC, 'examples', 'simple_saw_wrapped', 'simple_saw_hostenv.c'),
                                '-o', kernel, '--regs', '0,1,2,3,4,5,6,7'], capture_output=True, text=True)
            self.assertEqual(r.returncode, 0, r.stderr[-2000:])
            r = subprocess.run([sys.executable, os.path.join(ROOT, 'examples', 'system_s1cc', 'make_s1cc.py'), '--sim'],
                               env=dict(os.environ, KERNEL=kernel, OUTDIR=out), capture_output=True, text=True)
            self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
            self.assertNotIn('warning', r.stdout)
            self.assertIn('no notes: peak 0.000000', r.stdout)
            seg = [float(x) for x in re.search(r'peak per 5 ms: ([0-9. ]+)', r.stdout).group(1).split()]
            top = max(seg)
            self.assertGreater(top, 0.03)                       # it attacks
            self.assertLess(seg[9], 0.6 * top)                  # decays towards the sustain (S 128) while held
            self.assertLess(seg[-1], 0.01 * top)                # and releases after the gate (at 50 ms)
        finally:
            shutil.rmtree(out, ignore_errors=True)
