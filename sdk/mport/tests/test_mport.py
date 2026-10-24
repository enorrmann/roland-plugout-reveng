"""Tests of sdk/mport (docs/MUTABLE_SDK.md §9): python3 -m unittest discover -s sdk/mport/tests

  * the kernel's register allocation gives the same numbers as the same expressions in Python (emulator, C core);
  * the scheduled bench program equals the plain one sample by sample;
  * the integer model of SAW_SWARM equals the original C++ render bit by bit (needs g++ and the eurorack checkout);
  * the port stays within the measured distance to the original (SNR over the first 10 ms, band levels).
"""
import os
import shutil
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
MPORT = os.path.dirname(HERE)
sys.path.insert(0, MPORT)
sys.path.insert(0, os.path.join(MPORT, 'components'))

import numpy as np  # noqa: E402

import bench  # noqa: E402
import compare  # noqa: E402
import lut  # noqa: E402
from braids_saw_swarm import SawSwarm  # noqa: E402
from component import Component  # noqa: E402
from kernel import Kernel, KernelError  # noqa: E402

HAVE_REF = shutil.which('g++') and os.path.isdir(os.path.join(lut.EURORACK, 'braids'))


class Expr(Component):
    """a component made of arbitrary expressions, to test the allocator against Python"""
    name = 'expr'
    state = ('acc',)
    inputs = ('inc',)

    def kernel(self, k, ins, prm, st):
        a = ins['inc']
        b = k.mul(a, 3.0)
        c = k.add(b, 0.25)
        d = k.sub(c, a)                       # 2 inc + 0.25
        e = k.mad(d, b, c)                    # d + b c
        f = k.ymac(k.mov(0.5), a, -0.125)     # -0.125 + a 0.5
        g = k.max(k.min(e, 0.9), -0.9)
        h = k.wrap(k.add(st['acc'].value, a))
        st['acc'].write(h)
        y = k.add(k.add(g, f), k.mul(h, 0.001))
        return {'out': y}


class TestKernel(unittest.TestCase):
    def test_expressions(self):
        out, _ = bench.run(Expr(), note=69, knobs={}, n=50)
        inc = 2 * 440 / 96000.0
        b, c = 3 * inc, 3 * inc + 0.25
        e = c - inc + b * c
        g = max(min(e, 0.9), -0.9)
        acc = 0.0
        for t in range(50):
            acc = ((acc + inc + 1) % 2) - 1
            self.assertAlmostEqual(out[t], g + (-0.125 + inc * 0.5) + 0.001 * acc, places=6)

    def test_out_of_registers(self):
        k = Kernel('tiny', regs=(0, 1))
        a, b = k.load(0xA030), k.load(0xA031)
        c = k.load(0xA032)
        k.store(k.add(k.add(a, b), c), 0xA100)
        with self.assertRaises(KernelError):
            k.lines()

    def test_accumulator_copy(self):
        k = Kernel('acc', regs=(0, 1, 2, 3))
        a = k.load(0xA030)
        b = k.mad(a, a, 2.0)                  # a is read again: works on a copy
        k.store(k.add(a, b), 0xA100)
        text = '\n'.join(k.lines())
        self.assertIn('mov r', text)

    def test_sched_equals_plain(self):
        c = SawSwarm()
        for knobs in ({'ratio': 128, 'hp': 128}, {'ratio': 255, 'hp': 0}):
            a, ia = bench.run(c, 57, knobs, 2000, sched=True)
            b, ib = bench.run(c, 57, knobs, 2000, sched=False)
            self.assertEqual(a, b)
            self.assertLess(ia['instructions'], ib['instructions'])


class TestLut(unittest.TestCase):
    def test_overdrive_fit(self):
        f = lut.interp88(lut.table('braids', 'ws_moderate_overdrive'))
        _, err = lut.fit_odd(f, 9, x_max=0.875)
        self.assertLess(err, 25)


@unittest.skipUnless(HAVE_REF, 'needs g++ and the eurorack checkout')
class TestSawSwarm(unittest.TestCase):
    CASES = [(45, {'ratio': 200, 'hp': 85}), (69, {'ratio': 128, 'hp': 128}), (81, {'ratio': 255, 'hp': 100}),
             (96, {'ratio': 128, 'hp': 128})]

    def test_int_model_is_the_cpp(self):
        c = SawSwarm()
        for note, knobs in self.CASES:
            self.assertTrue(np.array_equal(np.array(c.model(note, knobs, 2000)), c.reference(note, knobs, 2000)),
                            (note, knobs))

    def test_port_close_to_original(self):
        c = SawSwarm()
        for note, knobs in self.CASES:
            port, info = bench.run(c, note, knobs, 4800)
            target = np.array(c.model(note, knobs, 4800, wrap=False))    # the original without its int32 overflow
            self.assertGreater(compare.snr_db(port[:960], target[:960]), 40, (note, knobs))
            self.assertTrue(all(abs(x) < 0.5 for x in compare.band_levels(port)[1:4]
                                - np.array(compare.band_levels(target))[1:4]), (note, knobs))
            self.assertLessEqual(info['instructions'], 50)


if __name__ == '__main__':
    unittest.main()
