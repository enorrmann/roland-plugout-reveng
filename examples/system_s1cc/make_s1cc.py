"""SYSTEM-S1CC: a SYSTEM-1 PLUG-OUT that is only an s1cc kernel (docs/examples_system_s1cc_README.md).

No oscillator, envelope, filter or LFO of its own: everything a voice does is the kernel's C (sdk/s1cc). Around it
only what the device needs, in the forms MUTABLE-SYSTEM / C-S1SY13 proved on the hardware:

    slave A  s1m0 @0x0100   per voice v: stage A of the kernel (S1_DSP_A); pitch / gate / trigger read from bank 5
                            (A030+v / A038+v / A060+v, written by engine.c); its output -> A151+v
    slave B  s1m1 @0x0100   first the mix: sum of last sample's voice outputs x gain x C000 -> 6006 / 6007
                            (6008..600D = 0); then per voice v: stage B of the kernel (S1_DSP_B) on x = A151+v ->
                            its output word; without S1_DSP_B the mix reads A151+v
    master B s1mm1 @0x1000  6006 / 6007 x VOLUME (C01C) -> OS effect send 2000 / 2002 (delay, reverb); the effect's
                            return 2001 / 2003 -> A118 / A119 (C-S1SY13's master, unchanged)
    knobs                   the kernel's S1_PARAM tables, by panel id: DSP A parameters from C070, DSP B from C060

    KERNEL=dir (required)   an s1cc build directory (sdk/s1cc/s1cc build ... -o dir)
    VOICES=n                1..8 (default 8): fewer voices, more instructions for each
    BUDGET=n                instructions per sample a slave DSP may have (default 708, the most tried on the device;
                            826 hung it: docs/examples_system_1_synth_README.md §4); above it the build stops

    python3 make_s1cc.py           -> s1*.s / s1*.hex here, build/s1_programs.h
    python3 make_s1cc.py --sim     -> also the whole chain on the emulator (C core) against the kernel's native
                                      reference (sdk/s1cc refdsp), and the silence without notes
"""
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'asm'))
sys.path.insert(0, os.path.join(ROOT, 'sdk', 's1cc', 'tools'))
import asm  # noqa: E402
import dspsched  # noqa: E402
import s1kernel  # noqa: E402

KERNEL = os.environ.get('KERNEL', '')
if not KERNEL or not os.path.exists(os.path.join(KERNEL, 'voice.s1k')):
    raise SystemExit('make_s1cc.py: KERNEL=<s1cc build directory> (with voice.s1k) is required')
NV = int(os.environ.get('VOICES', '8'))
if not 1 <= NV <= 8:
    raise SystemExit('make_s1cc.py: VOICES must be 1..8 (the OS hands notes to 8 voice ids at most)')
BUDGET = int(os.environ.get('BUDGET', '708'))
KDEF = s1kernel.Kernel.read(os.path.join(KERNEL, 'voice.s1k'))
KTAB = s1kernel.read_tables(os.path.join(KERNEL, 'tables.txt'))
HAS_B = 'B' in KDEF.stages

FS = 96000.0
# bank 5 (shared by slave A and B): written by engine.c
PITCH = [0xA030 + v for v in range(NV)]     # (note - 12) / 12 octaves (4.75 = A 440)
GATE = [0xA038 + v for v in range(NV)]      # 1.0 / 0.0
TRIG = [0xA060 + v for v in range(NV)]      # toggles 0 / 1 at every note start
# bank 5, written by DSP A (a DSP stores only to A100..A1FF)
XV = [0xA151 + v for v in range(NV)]        # stage A output of voice v = the input x of stage B
X0, INB0 = 0xA188, 0xA1A0                   # exports i: X0 + 8 i + v; inputs B needs from A: INB0 + 8 j + v
# bank 0 (ring, S / S+1): the voice blocks of the kernel, sized by what it needs
A_BLOCK0, B_BLOCK0 = 0x0100, 0x0100
B_OUT = [0x00C0 + 2 * v for v in range(NV)]  # stage B output of voice v (ring: read one sample later at S+1)
K_A_PARAM0, K_B_PARAM0 = 0x70, 0x60         # the kernel's knob tables: DSP A C070.., DSP B C060..
OUT_L, OUT_R = 0x6006, 0x6007               # slave B -> master (IOMD)
OUT_ZERO = [0x6008, 0x6009, 0x600A, 0x600B, 0x600C, 0x600D]
GAIN = 2.0 / (1.5 * NV)                     # the mix as C-S1SY13's (one voice at full scale: 1.33 / NV)
M_OUT = (0xA118, 0xA119)                    # master B -> the OS output program on master A
M_ZERO = [0xA11A, 0xA11B, 0xA11C, 0xA11D]
M_VOLUME = 0x1C                             # master B parameter C01C (model.c, VOLUME knob)
FX_SEND = (0x2000, 0x2002)                  # master B bank 1: the OS delay / reverb (system-1/FX_CHAIN.md §4)
FX_RET = (0x2001, 0x2003)


# ---- the kernel's addresses ---------------------------------------------------------------------------------------
def layout():
    """{param name: (count, A base or None, B base or None)}: a table used by one stage only gets no slot on the other"""
    used = {st: {m.split(':')[1] for m in KDEF.placeholders(st) if m.startswith('P:')} for st in KDEF.stages}
    out, a, b = {}, K_A_PARAM0, K_B_PARAM0
    for p in KDEF.params:
        ab = bb = None
        if p['name'] in used.get('A', ()):
            ab, a = a, a + p['count']
        if p['name'] in used.get('B', ()):
            bb, b = b, b + p['count']
        out[p['name']] = (p['count'], ab, bb)
    return out


def words(stage):
    """bank-0 words a voice of this stage needs: state, scratch (spills), the first-sample flag"""
    st = KDEF.stages[stage]
    return sum(1 for s in KDEF.states if s['stage'] == stage) + st.get('scratch', 0) + bool(st.get('initflag'))


def block(stage, v):
    """bank-0 block of voice v (2 ring words per value: S and S+1)"""
    base = A_BLOCK0 if stage == 'A' else B_BLOCK0
    top = base + 2 * words(stage) * NV
    if top > 0x0800:
        raise SystemExit(f'kernel stage {stage}: {words(stage)} words x {NV} voices do not fit in bank 0 '
                         f'(0x{base:04x}..0x07ff)')
    return base + 2 * words(stage) * v


def addresses(stage, v):
    addr, k, base = {}, 0, block(stage, v)
    for s in KDEF.states:
        if s['stage'] == stage:
            addr[f'S{s["index"]}'] = base + 2 * k
            k += 1
    for j in range(KDEF.stages[stage].get('scratch', 0)):
        addr[f'T{j}'] = base + 2 * k
        k += 1
    if KDEF.stages[stage].get('initflag'):
        addr['I'] = base + 2 * k
    for name, (cnt, ab, bb) in layout().items():
        pb = ab if stage == 'A' else bb
        if pb is not None:
            for i in range(cnt):
                addr[f'P:{name}:{i}'] = 0xC000 + pb + i
    for e in KDEF.exports:
        addr[f'X:{e}'] = X0 + 8 * e + v
    for j, name in enumerate(KDEF.inputs_b):
        addr[f'INB:{name}'] = INB0 + 8 * j + v
        if stage == 'B':
            addr[f'IN:{name}'] = INB0 + 8 * j + v
    if stage == 'A':
        addr.update({'IN:pitch': PITCH[v], 'IN:gate': GATE[v], 'IN:trig': TRIG[v], 'OUT': XV[v]})
    else:
        addr.update({'XIN': XV[v], 'OUT': B_OUT[v]})
    return addr


# ---- programs -------------------------------------------------------------------------------------------------------
def prog_a():
    L = [f'; s1m0: slave DSP A main @0x0100: {NV} voices x s1cc kernel {KERNEL} stage A -> A151+v']
    for v in range(NV):
        L += [f'; voice {v}'] + KDEF.place('A', addresses('A', v))
    return L


def prog_b():
    L = [f'; s1m1: slave DSP B main @0x0100: mix of {NV} voices x {GAIN!r} x C000 -> 6006 / 6007'
         + (f'; per voice s1cc kernel stage B' if HAS_B else ''), 'mov r7, #0.0']
    for v in range(NV):                         # B outputs of the sample before (ring S+1); A outputs as they are
        L += [f'ld r{v & 1}, [{(B_OUT[v] + 1) if HAS_B else XV[v]:04x}]', f'addxy r7, r7, r{v & 1}']
    L += ['ld r6, [c000]', f'mul r7, r7, #{GAIN!r}', 'mul r7, r7, r6',
          f'st r7, [{OUT_L:04x}]', f'st r7, [{OUT_R:04x}]', 'mov r0, #0.0']
    L += [f'st r0, [{x:04x}]' for x in OUT_ZERO]
    if HAS_B:
        for v in range(NV):
            L += [f'; voice {v}'] + KDEF.place('B', addresses('B', v))
    return L


def prog_master():
    """C-S1SY13's master B (examples/mutable_system make_mutable.py step_m1, STEP 13)"""
    L = ['; s1mm1: master DSP B @0x1000: 6006 / 6007 x volume (C01C) -> OS effect send 2000 / 2002;'
         ' effect return 2001 / 2003 -> A118 / A119',
         f'ld r0, [{OUT_L:04x}]', f'ld r1, [{OUT_R:04x}]', f'ld r7, [c{M_VOLUME:03x}]', 'mov r4, r4',
         'mul r0, r0, r7', 'mul r1, r1, r7',
         f'st r0, [{FX_SEND[0]:04x}]', f'st r1, [{FX_SEND[1]:04x}]',
         f'ld r2, [{FX_RET[0]:04x}]', f'ld r3, [{FX_RET[1]:04x}]', 'mov r4, r4',
         f'st r2, [{M_OUT[0]:04x}]', f'st r3, [{M_OUT[1]:04x}]', 'mov r2, #0.0']
    return L + [f'st r2, [{x:04x}]' for x in M_ZERO]


_ISA = []


def assemble(lines):
    if not _ISA:
        _ISA.append(asm.Assembler().table)
    r = asm.Assembler(_ISA[0]).assemble('\n'.join(lines) + '\n')
    if not r.ok:
        raise SystemExit(r.errors)
    return r


def section(lines):
    """a section program, scheduled with the op groups proven on the device only (asm/proven_forms.txt): SYSTEM-1
    start marker (0a02 ff71), the code, end sequence"""
    lines = dspsched.schedule(lines, dspsched.load_proven())
    r = assemble(lines)
    return [0x0A02, 0xFF71] + r.words + [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001], r, lines


def close(word):
    """the close program at 0x7FF2: frame counter E000 -> A004 (DSP A) / A005 (DSP B), as every SYSTEM-1 PLUG-OUT"""
    lines = ['ld r0, [e000]', f'st r0, [{word:04x}]']
    w = [0x0000] + assemble(lines).words
    assert len(w) == 7
    return w, None, lines


def instr_count(lines):
    return sum(1 for x in lines if x.strip() and not x.startswith(';'))


def sources():
    return {'s1m0': section(prog_a()), 's1m1': section(prog_b()), 's1mm1': section(prog_master()),
            's1c0': close(0xA004), 's1c1': close(0xA005)}


# ---- ARM header -----------------------------------------------------------------------------------------------------
def float_bits(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


def table(name, vals, n=8):
    rows = [', '.join(f'0x{float_bits(x):08x}u' for x in vals[i:i + n]) for i in range(0, len(vals), n)]
    return f'static const uint32_t {name}[{len(vals)}] = {{\n    ' + ',\n    '.join(rows) + '\n};'


def c_array(name, w):
    rows = [', '.join(f'0x{x:04x}' for x in w[i:i + 12]) for i in range(0, len(w), 12)]
    return (f'#define MC_{name.upper()}_BYTES {2 * len(w)}\nstatic const uint16_t mc_{name.upper()}[{len(w)}] = {{\n    '
            + ',\n    '.join(rows) + '\n};')


def header(progs):
    out = [f'/* generated by make_s1cc.py (KERNEL={KERNEL}, VOICES={NV}): our own DSP programs and tables */',
           '#include <stdint.h>', '']
    out += [c_array(name, w) for name, (w, _, _) in progs.items()]
    out.append('/* pitch word of a MIDI note: (note - 12) / 12 octaves (4.75 = A 440) */')
    out.append(table('pitch_bits', [(n - 12) / 12.0 for n in range(128)]))
    out.append('/* VOLUME knob -> gain (v / 255)^2 */')
    out.append(table('volume_bits', [(v / 255.0) ** 2 for v in range(256)]))
    idx = lambda x: f'0x{x & 0xFF:02x}'  # noqa: E731
    out.append(f'#define NV {NV}\n#define M_VOLUME 0x{M_VOLUME:02x}\n'
               f'#define F_ONE 0x{float_bits(1.0):08x}u\n#define F_ZERO 0x00000000u\n'
               f'#define B5_PITCH {idx(PITCH[0])}\n#define B5_GATE {idx(GATE[0])}\n#define B5_TRIG {idx(TRIG[0])}')
    rows, bits, off = [], [], 0
    for name, (cnt, ab, bb) in layout().items():
        rows.append((KTAB[name][0], cnt, 0xFF if ab is None else ab, 0xFF if bb is None else bb, off))
        for kv in range(256):
            bits += KDEF.param_values(name, kv, KTAB)
        off += 256 * cnt
    out.append(f'/* the kernel\'s knob tables (S1_PARAM): panel id, count, DSP A / B parameter base (0xFF = none),\n'
               f'   offset in kparam_bits (256 rows of count values each) */\n#define KPARAMS {len(rows)}')
    if rows:
        out.append('static const uint16_t kparam_panel[KPARAMS] = { ' + ', '.join(f'0x{r[0]:03x}' for r in rows) + ' };')
        out.append('static const uint8_t kparam_count[KPARAMS] = { ' + ', '.join(str(r[1]) for r in rows) + ' };')
        out.append('static const uint8_t kparam_a[KPARAMS] = { ' + ', '.join(f'0x{r[2]:02x}' for r in rows) + ' };')
        out.append('static const uint8_t kparam_b[KPARAMS] = { ' + ', '.join(f'0x{r[3]:02x}' for r in rows) + ' };')
        out.append('static const uint32_t kparam_off[KPARAMS] = { ' + ', '.join(str(r[4]) for r in rows) + ' };')
        out.append(table('kparam_bits', bits))
    return '\n'.join(out) + '\n'


# ---- simulation -----------------------------------------------------------------------------------------------------
def f32(x):
    return float(struct.unpack('<f', struct.pack('<f', x))[0])


def params(knobs):
    """bank-6 parameters of slave A, slave B and master B as engine.c / model.c write them; knobs: {name: 0..255}"""
    pa, pb, pm = {}, {0xC000: 1.0}, {0xC000 + M_VOLUME: 1.0}
    for name, (cnt, ab, bb) in layout().items():
        vals = KDEF.param_values(name, knobs.get(name, 128), KTAB)
        for i in range(cnt):
            if ab is not None:
                pa[0xC000 + ab + i] = f32(vals[i])
            if bb is not None:
                pb[0xC000 + bb + i] = f32(vals[i])
    return pa, pb, pm


def simulate(progs, events, n, knobs=None):
    """events: {sample: [(voice, note or None for off)]}; returns master A118 per sample (emulator C core, one
    cchain.Slave: DSP 0 = slave A, 1 = slave B, 2 = master B; bank 5 shared by slave A and B; each DSP its own
    bank 6; B's 6006 / 6007 reach the master in the same sample)"""
    sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 'emu'))
    import chain
    import cchain
    P = {k: chain.Program(progs[k][0], k) for k in ('s1m0', 's1m1', 's1mm1')}
    model = chain.Model(order=(0, 1, 2), shared5=((0, 1),), own_params=(0, 1, 2))
    sl = cchain.Slave({0: (P['s1m0'], []), 1: (P['s1m1'], []), 2: (P['s1mm1'], [])}, model)
    for d, prm in zip(sl.d, params(knobs or {})):
        d.params.update(prm)
    a5 = sl.d[0].bank5
    trig = [0.0] * NV
    for v in range(NV):
        a5[PITCH[v] & 0x3FF], a5[GATE[v] & 0x3FF], a5[TRIG[v] & 0x3FF] = f32((69 - 12) / 12.0), 0.0, 0.0
    out, t, word = [], 0, (2, M_OUT[0])
    for te in sorted(e for e in events if e < n) + [n]:
        if te > t:
            out += sl.run(te - t, (word,))[word]
            t = te
        for v, note in events.get(te, []) if te < n else []:
            if note is None:
                a5[GATE[v] & 0x3FF] = 0.0
            else:
                trig[v] = 1.0 - trig[v]
                a5[PITCH[v] & 0x3FF], a5[GATE[v] & 0x3FF], a5[TRIG[v] & 0x3FF] = f32((note - 12) / 12.0), 1.0, trig[v]
    return out


def sim_report(progs):
    import numpy as np
    import s1check
    n = int(os.environ.get('SIM_N', '9600'))
    for note in (45, 69):
        out = np.array(simulate(progs, {0: [(0, note)]}, n))
        ra, rb = s1check.run_ref(KERNEL, note, n, {})
        ref = (rb if HAS_B else ra) * GAIN
        best = None
        for lag in range(0, 6):                    # the chain's latency in samples (ring words, IOMD, master)
            e = np.mean((out[lag:] - ref[:n - lag]) ** 2)
            if best is None or e < best[1]:
                best = (lag, e)
        lag, e = best
        snr = 10 * np.log10(np.mean(ref[:n - lag] ** 2) / e) if e > 0 else float('inf')
        print(f'note {note}, voice 0: peak {np.max(np.abs(out)):.3f} (reference x gain {np.max(np.abs(ref)):.3f}), '
              f'latency {lag} samples, SNR against the native reference {snr:.1f} dB')
    chord = np.array(simulate(progs, {0: [(v, 48 + 3 * v) for v in range(NV)]}, n // 2))
    print(f'{NV} notes: peak {np.max(np.abs(chord)):.3f}')
    print('no notes: peak %.6f' % max(abs(x) for x in simulate(progs, {}, 2000)))


def main():
    progs = sources()
    over = []
    for name, (w, _, lines) in progs.items():
        open(os.path.join(HERE, name + '.s'), 'w').write('\n'.join(lines) + '\n')
        open(os.path.join(HERE, name + '.hex'), 'w').write(
            f'# {name}, own SYSTEM-1 program (make_s1cc.py KERNEL={KERNEL} VOICES={NV})\n'
            + '\n'.join('%04x' % x for x in w) + '\n')
        print(f'{name:6s} {instr_count(lines):4d} instructions, {len(w)} halfwords')
        if name in ('s1m0', 's1m1') and instr_count(lines) > BUDGET:
            over.append(f'{name}: {instr_count(lines)} instructions')
    print(f'kernel per voice: A {len(KDEF.stages["A"]["lines"])} lines, {words("A")} words'
          + (f'; B {len(KDEF.stages["B"]["lines"])} lines, {words("B")} words' if HAS_B else '; no stage B'))
    if over:
        raise SystemExit(f'make_s1cc.py: over the budget of {BUDGET} instructions per sample ({", ".join(over)}): '
                         f'fewer VOICES, a smaller kernel, or BUDGET=n at your own risk (826 hung the device)')
    os.makedirs(os.path.join(HERE, 'build'), exist_ok=True)
    open(os.path.join(HERE, 'build', 's1_programs.h'), 'w').write(header(progs))
    if '--sim' in sys.argv:
        sim_report(progs)


if __name__ == '__main__':
    main()
