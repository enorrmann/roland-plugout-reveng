"""Reading an s1cc kernel (.s1k, docs/vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md §8) and placing it at real addresses: the glue between s1cc (C++)
and the repo's Python assembler / scheduler / emulator. Used by s1check.py and by examples that embed a kernel
(examples/mutable_system OSC=...).

    k = Kernel.read('voice.s1k')
    lines_a = k.place('A', {'IN:pitch': 0xA030, 'S0': 0x0100, ..., 'OUT': 0xA151})   # one-unit lines
    k.param_values('ratio', knob=128, tables=read_tables('tables.txt'))             # floats the ARM writes
"""
import math
import re

PH = re.compile(r'\{([^}]+)\}')


VSTEPS = 8                                       # S1_VOICE_PARAM: steps of each knob (knob >> 5)


def read_tables(path):
    """{name: (panel_id, count, [256 x count floats])} from `PROG tables`; an S1_VOICE_PARAM:
    {name: ((panel_a, panel_b), count, [128 notes x 8 x 8 steps x count floats])}"""
    out, f = {}, open(path).read().split('\n')
    i = 0
    while i < len(f):
        p = f[i].split()
        if p and p[0] == 'p':
            name, panel, count = p[1], int(p[2]), int(p[3])
            rows = [list(map(float, f[i + 1 + v].split())) for v in range(256)]
            out[name] = (panel, count, [x for r in rows for x in r])
            i += 257
        elif p and p[0] == 'v':
            name, pa, pb, count = p[1], int(p[2]), int(p[3]), int(p[4])
            n = 128 * VSTEPS * VSTEPS
            rows = [list(map(float, f[i + 1 + r].split())) for r in range(n)]
            out[name] = ((pa, pb), count, [x for r in rows for x in r])
            i += 1 + n
        else:
            i += 1
    return out


def on_grid(x):
    return math.floor(x * 2 ** 32) / 2 ** 32 if abs(x) < 0.125 else x


class Kernel:
    def __init__(self):
        self.states, self.params, self.exports, self.inputs_b, self.dsps = [], [], {}, [], {}
        # S1_MASTER (master DSP B, dsps['M']): its states, its S1_PARAMs, its S1_VOICE_PARAMs and lines per voice,
        # apart from the slaves' (states / params are the slaves' only)
        self.mstates, self.mparams, self.vparams, self.lines = [], [], [], 0
        self.options, self.compile = {}, []     # S1_BUILD (.option KEY VALUE), S1_COMPILE (.compile ...)
        self.fused = False                      # .fused: dsps['A'] is the whole voice, S1_DSP_B(S1_DSP_A())

    @classmethod
    def read(cls, path):
        k, cur = cls(), None
        for line in open(path).read().split('\n'):
            p = line.split()
            if not p or line.startswith(';') and cur is None:
                continue
            if p[0] == '.state':
                (k.mstates if p[2] == 'M' else k.states).append(
                    {'index': int(p[1]), 'dsp': p[2], 's': float(p[3]), 'o': float(p[4]), 'name': p[5]})
            elif p[0] == '.param':
                (k.mparams if p[4:5] == ['M'] else k.params).append(
                    {'name': p[1], 'count': int(p[2]), 'scale': float(p[3])})
            elif p[0] == '.vparam':                 # name count scale0 .. scale(count - 1) M
                k.vparams.append({'name': p[1], 'count': int(p[2]),
                                  'scales': [float(x) for x in p[3:3 + int(p[2])]]})
            elif p[0] == '.lines':
                k.lines = int(p[1])
            elif p[0] == '.export':
                k.exports[int(p[1])] = (float(p[2]), float(p[3]))
            elif p[0] == '.option' and cur is None:
                k.options[p[1]] = line.split(None, 2)[2] if len(p) > 2 else ''
            elif p[0] == '.fused' and cur is None:
                k.fused = True
            elif p[0] == '.compile' and cur is None:
                k.compile = p[1:]
            elif p[0] == '.inputb':
                k.inputs_b.append(p[1])
            elif p[0] == '.dsp':
                cur = p[1]
                k.dsps[cur] = {'units': int(p[2]), 'lines': [], 'scratch': 0}
            elif p[0] == '.scratch':
                k.dsps[cur]['scratch'] = int(p[1])
            elif p[0] == '.initflag':
                k.dsps[cur]['initflag'] = True
            elif p[0] == '.init':             # a state word the first-sample flag corrects (s1cc init uses it)
                k.dsps[cur].setdefault('init', []).append(int(p[1]))
            elif p[0] == '.trace':
                k.dsps[cur].setdefault('trace', []).append((int(p[1]), float(p[2]), ' '.join(p[3:])))
            elif p[0] == '.end':
                cur = None
            elif cur:
                k.dsps[cur]['lines'].append(line)
        return k

    def placeholders(self, dsp):
        return sorted({m for x in self.dsps[dsp]['lines'] for m in PH.findall(x)})

    def place(self, dsp, addr):
        """lines of a DSP with every {placeholder} replaced: addr maps 'IN:pitch', 'S3', 'P:ratio:2', 'X:0',
        'INB:pitch', 'XIN', 'OUT' to addresses ('S3+1' is the S3 address + 1)"""
        def sub(m):
            key = m.group(1)
            plus = 0
            if key.endswith('+1'):
                key, plus = key[:-2], 1
            if key not in addr:
                raise KeyError(f'kernel placeholder {{{key}}} has no address')
            return f'{addr[key] + plus:04x}'
        return [PH.sub(sub, x) for x in self.dsps[dsp]['lines']]

    def param_values(self, name, knob, tables):
        """the DSP words of a knob table at a knob value: value / scale, on the register grid"""
        p = next(q for q in self.params if q['name'] == name)
        _, count, vals = tables[name]
        return [on_grid(vals[knob * count + i] / p['scale']) for i in range(count)]
