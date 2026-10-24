"""What a ported component is (docs/MUTABLE_SDK.md §5).

A Mutable Instruments render function splits in two:

  control rate  what Render() computes once per block from pitch_ and parameter_[] (increments, coefficients,
                table reads at a knob position). Here: Python functions of the knob value (0..255), evaluated at
                build time into ARM tables; the ARM writes the result as DSP parameters C0xx (FCEC) when the knob
                moves. -> Param
  audio rate    the per-sample loop. Here: microcode written with a Kernel (kernel.py), with the state of the loop
                in bank-0 ring words (load S+1, store S) and the inputs / outputs as values. -> Component.kernel()

    class MyOsc(Component):
        name, source = 'my_osc', 'braids/digital_oscillator.cc RenderMyOsc'
        params = (Param('k', lambda v: ..., count=1),)
        state = ('phase',)
        inputs, outputs = ('inc',), ('out',)
        def kernel(self, k, ins, prm, st):          # ins / outputs: {name: Value}; prm[name](i) -> Value
            ...                                     # st[name] -> kernel.Ring
            return {'out': y}

A component may also give parts (methods that take the same arguments) so that a host puts them on different DSPs:
Braids' SAW_SWARM runs its seven saws and the waveshaper on slave DSP A and its high-pass on slave DSP B in the
MUTABLE-SYSTEM example (examples/mutable_system).
"""
from mi import on_grid


class Param:
    """a control-rate value the ARM writes: fn(knob 0..255) -> float, or a tuple of `count` floats"""

    def __init__(self, name, fn, count=1, doc=''):
        self.name, self.fn, self.count, self.doc = name, fn, count, doc

    def values(self, v):
        x = self.fn(v)
        x = tuple(x) if isinstance(x, (tuple, list)) else (x,)
        assert len(x) == self.count, (self.name, len(x), self.count)
        return tuple(on_grid(float(e)) for e in x)

    def table(self):
        """the 256 x count floats of the ARM table, knob-major"""
        return [e for v in range(256) for e in self.values(v)]


class Component:
    name = ''
    source = ''
    params = ()
    state = ()
    inputs = ()
    outputs = ('out',)

    def param(self, name):
        return next(p for p in self.params if p.name == name)

    def kernel(self, k, ins, prm, st):
        raise NotImplementedError

    def reference(self, note, knobs, n):
        """the original C++ render (reference.py) for a note and knob values {param name: 0..255}; None if absent"""
        return None
