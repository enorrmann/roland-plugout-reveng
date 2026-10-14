"""Generate tables.h for the C engine from the user's own JX-3P PLUG-OUT.

usage: python3 gen_tables.py plugins/JX-3P outdir

Unpacks Code8_1.Dat (relocations from Code8_2.Dat) into outdir/proj, then runs
the JX-3P engine (region 1) in emu1 with its default patch and records:

* the DSP program registration of vt+8 (OS +0x28..+0x6C), with the SAW
  program in both oscillator slots (DCO1/DCO2 waveform 0);
* the DSP state after start-up (parameter block via +0xA0/+0xA4, bank 5 via
  +0xB0, +0xBC), for slave 0 and slave 1;
* the oscillator mute / unmute call lists (vt+0x10 / vt+0x18);
* the velocity curve.

The microcode blobs are copied into tables.h; nothing of Roland's is kept in
the repository.
"""
import os, sys, struct, subprocess
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, '..', '..', '..')
sys.path.insert(0, os.path.join(HERE, '..', 're'))
import run_jx
from jxpatch import patch_params

BASE = 0x60A00000


def unpack(plugin, out):
    proj = os.path.join(out, 'proj')
    if not os.path.exists(os.path.join(proj, 'region1.bin')):
        subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'plugout_build.py'), 'unpack',
                        os.path.join(plugin, 'Script', 'Code8_1.Dat'), proj,
                        '--relocs-from', os.path.join(plugin, 'Script', 'Code8_2.Dat')], check=True)
    return proj


def start(slave, regs=None):
    """Boot the engine, load the default patch; returns the emulator."""
    e = run_jx.boot(slave)
    for i, n, d in patch_params():
        e.vals[i] = d
    e.log.clear()
    e.call(e.u32(e.vt + 8), e.o)
    if regs is not None:
        regs.extend(e.log)
    e.call(e.u32(e.vt + 0xC), e.o)
    for i, n, d in patch_params():
        run_jx.setp(e, i, d)
    e.call(e.u32(e.vt + 0x24), e.o, 1)
    return e


def state(e, log):
    """Final DSP state from a call log: param block and bank 5, in first-write order."""
    A, B, BC = {}, {}, []
    for off, r0, r1, r2, r3, s0, st in log:
        if off == 0xA4:
            A[(r0, r1)] = s0
        elif off == 0xA0:
            A[(r0, r1)] = s0
        elif off == 0xB0:
            B[(r0, e.u32(r1))] = s0
        elif off == 0xBC:
            BC.append(s0)
    return A, B, BC


def capture_state(slave, extra=None):
    e = run_jx.boot(slave)
    for i, n, d in patch_params():
        e.vals[i] = d
    e.vals.update(extra or {})
    e.log.clear()
    e.call(e.u32(e.vt + 8), e.o)
    e.call(e.u32(e.vt + 0xC), e.o)
    for i, n, d in patch_params():
        run_jx.setp(e, i, d)
    e.call(e.u32(e.vt + 0x24), e.o, 1)
    return state(e, e.log)


def external_gets(slave):
    """(group, id) the engine reads with OS +0x00 during a refresh, outside the patch."""
    e = start(slave)
    patch = {i for i, n, d in patch_params()}
    e.log.clear()
    e.call(e.u32(e.vt + 0x24), e.o, 1)
    seen = []
    for off, r0, r1, r2, r3, s0, st in e.log:
        if off == 0 and r2 not in patch and (r1, r2) not in seen:
            seen.append((r1, r2))
    return seen


NOTE_IDS = set(range(0x429, 0x453)) | {0x45C}


def responses(slave):
    """For every id outside the patch that the engine reads (OS +0x00), the
    JX-3P's DSP writes for values 0..255, other parameters at their defaults.
    Returns [(group, id, [(kind, dsp, dsp_id, curve[256] or None-filled)])].
    The emulator's OS returns 0 for these ids; the device its real values, which
    the engine reads at run time and maps through these curves."""
    e = start(slave)
    out = []
    for grp, i in external_gets(slave):
        if i in NOTE_IDS:
            continue
        curves, bad = {}, False
        for v in range(256):
            e.log.clear()
            run_jx.setp(e, i, v)
            for off, r0, r1, r2, r3, s0, st in e.log:
                if off in (0xA0, 0xA4):
                    key = ('P', r0, r1)
                elif off == 0xB0:
                    key = ('B', r0, e.u32(r1))
                elif off in (0x00, 0xF0):
                    continue
                else:
                    bad = True          # program swaps etc. (effect type): not handled
                    continue
                curves.setdefault(key, [None] * 256)[v] = s0
        run_jx.setp(e, i, 0)
        if bad:
            print(f'  {i:#x}: response has other OS calls, skipped')
            continue
        if curves:
            out.append((grp, i, [(k[0], k[1], k[2], c) for k, c in curves.items()]))
    return out


def fit(c):
    """('lin', a, b) if every defined point is a*v + b, else ('tab', c)."""
    pts = [(v, y) for v, y in enumerate(c) if y is not None]
    (v0, y0), (v1, y1) = pts[0], pts[-1]
    a = (y1 - y0) / (v1 - v0) if v1 != v0 else 0.0
    b = y0 - a * v0
    if len(pts) == 256 and all(abs(a * v + b - y) <= 1e-5 * max(1.0, abs(y)) for v, y in pts):
        return ('lin', a, b)
    return ('tab', tuple(c))


def swap_ops(add_desc):
    """Calls the JX-3P makes when the patch's waveforms and the effect type
    (0x748 = 0) arrive, as {type, a, b, c, d, e, f, g, h, v}:
    0 = +0xA0(a, b, c, d, [e], v), 1 = +0xC0, 2 = +0x70(a, &descs[b], &descs[c],
    ctx, ctx, e, f, g, h)."""
    import unicorn as U
    from unicorn import arm_const as A
    e = start(0)
    rec = []

    def hook(uc, addr, size, ud):
        if (addr - run_jx.TRAP) % 16:
            return
        off = (addr - run_jx.TRAP) // 4
        if off in (0x70, 0xA0, 0xC0):
            sp = uc.reg_read(A.UC_ARM_REG_SP)
            st = struct.unpack('<5I', uc.mem_read(sp, 20))
            r = [uc.reg_read(getattr(A, f'UC_ARM_REG_R{i}')) for i in range(4)]
            rec.append((off, r, st, e.s(0)))
    e.uc.hook_add(U.UC_HOOK_CODE, hook, begin=run_jx.TRAP, end=run_jx.TRAP + 0x400)
    ops = []
    for i, v in [(i, d) for i, n, d in patch_params()] + [(0x748, 0)]:
        rec.clear()
        run_jx.setp(e, i, v)
        if not any(c[0] == 0x70 for c in rec):
            continue
        for off, r, st, s0 in rec:
            if off == 0xA0:
                ops.append((0, r[0], r[1], r[2], r[3], st[0], 0, 0, 0, float(s0)))
            elif off == 0xC0:
                ops.append((1, 0, 0, 0, 0, 0, 0, 0, 0, 0.0))
            else:
                assert r[3] == st[0], (r, st)
                ops.append((2, r[0], add_desc(r[1]), add_desc(r[2]), 0, st[1], st[2], st[3], st[4], 0.0))
    return ops


def f(x):
    return f'{x!r}f' if '.' in repr(x) or 'e' in repr(x) else f'{x!r}.0f'


def main():
    plugin, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    proj = unpack(plugin, out)
    run_jx.IMG = os.path.join(proj, 'region1.bin')
    img = open(run_jx.IMG, 'rb').read()
    L = []

    # --- program registration (vt+8), in call order --------------------------
    regs = []
    e = start(0, regs)
    blobs, descs, calls, descidx = {}, [], [], {}

    def add_desc(ptr):
        """index of the JX-3P descriptor at ptr in our descs[], adding it if new"""
        if ptr not in descidx:
            addr, n, n2, src = struct.unpack('<4I', e.uc.mem_read(ptr, 16))
            nb = (n + 3) & ~3                   # includes the 0x0001 tail of 4n+2 blobs
            blobs.setdefault(src, img[src - BASE: src - BASE + nb])
            descs.append((addr, n, n2, src))
            descidx[ptr] = len(descs) - 1
        return descidx[ptr]

    for off, r0, *_ in regs:
        if not 0x28 <= off <= 0x6C:
            continue
        kind, dsp = divmod((off - 0x28) // 4, 6)
        if kind == 0:
            calls.append((off, ('addr', r0)))
            continue
        calls.append((off, ('desc', add_desc(r0))))

    # --- slot program start (+0xC0 sync, +0x70 swap) ----------------------------
    # The JX-3P starts the oscillator and effect slot programs only when the
    # waveform / effect parameters arrive: mute ramps, +0xC0, +0x70 per DSP, unmute.
    # +0x70 enables the slot's task bit; without it the slot programs never run.
    swaps = swap_ops(add_desc)
    names = {src: f'blob_{src:08x}' for src in blobs}
    L.append('/* generated by gen_tables.py from the JX-3P PLUG-OUT; do not commit */')
    L.append('#include <stdint.h>')
    L.append('#define NAN __builtin_nanf("")')
    for src, b in blobs.items():
        words = struct.unpack('<%dI' % (len(b) // 4), b)
        L.append(f'static const uint32_t {names[src]}[{len(words)}] __attribute__((aligned(4))) = {{')
        for i in range(0, len(words), 8):
            L.append('  ' + ', '.join(f'0x{w:08x}' for w in words[i:i + 8]) + ',')
        L.append('};')
    L.append('static dspdesc_t descs[] = {')
    for addr, n, n2, src in descs:
        L.append(f'  {{0x{addr:04x}, 0x{n:x}, 0x{n2:x}, {names[src]}}},')
    L.append('};')
    L.append('static const swapop_t swap_ops[] = {')
    for op in swaps:
        L.append('  {' + ', '.join(str(x) if not isinstance(x, float) else f(x) for x in op) + '},')
    L.append('};')
    L.append('static const regcall_t regcalls[] = {')
    for off, (k, v) in calls:
        L.append(f'  {{0x{off:02x}, {1 if k == "desc" else 0}, {v:#x}}},')
    L.append('};')

    # --- start-up state per slave ---------------------------------------------
    for slave in (0, 1):
        A, B, BC = capture_state(slave)
        L.append(f'static const pset_t state_a{slave}[] = {{')
        L += [f'  {{{u}, 0x{i:x}, {f(v)}}},' for (u, i), v in A.items()]
        L.append('};')
        L.append(f'static const pset_t state_b{slave}[] = {{')
        L += [f'  {{{u}, 0x{i:x}, {f(v)}}},' for (u, i), v in B.items()]
        L.append('};')
        L.append(f'static const float state_bc{slave}[] = {{' + ', '.join(f(v) for v in BC) + '};')

    # --- learned responses to ids outside the patch -----------------------------
    curves, tabs, outs = [], [], []
    cidx, tidx = {}, {}
    for slave in (0, 1):
        resp = responses(slave)
        L.append(f'static const ext_t ext{slave}[] = {{')
        for grp, i, lst in resp:
            first = len(outs)
            for kind, dsp, did, c in lst:
                key = fit(c)
                if key not in cidx:
                    if key[0] == 'lin':
                        curves.append((key[1], key[2], -1))
                    else:
                        if key[1] not in tidx:
                            tidx[key[1]] = len(tabs)
                            tabs.append(key[1])
                        curves.append((0.0, 0.0, tidx[key[1]]))
                    cidx[key] = len(curves) - 1
                outs.append((1 if kind == 'B' else 0, dsp, did, cidx[key]))
            L.append(f'  {{{grp}, 0x{i:x}, {first}, {len(outs) - first}}},')
        L.append('};')
        print(f'slave {slave}: {len(resp)} ids outside the patch with learned responses')
    L.append('static const extout_t extout[] = {')
    L += [f'  {{{k}, {d}, 0x{i:x}, {c}}},' for k, d, i, c in outs]
    L.append('};')
    L.append('static const curve_t curves[] = {')
    L += [f'  {{{f(a)}, {f(b)}, {t}}},' for a, b, t in curves]
    L.append('};')
    L.append(f'static const float curve_tab[{max(1, len(tabs))}][256] = {{')
    for t in tabs:
        L.append('  {' + ', '.join('NAN' if y is None else f(y) for y in t) + '},')
    if not tabs:
        L.append('  {0},')
    L.append('};')
    print(f'{len(outs)} outputs, {len(curves)} curves ({len(tabs)} tables)')

    # --- oscillator mute / unmute (vt+0x10 / vt+0x18) --------------------------
    for name, off in (('mute', 0x10), ('unmute', 0x18)):
        e.log.clear()
        e.call(e.u32(e.vt + off), e.o, 0)
        L.append(f'static const rampset_t osc_{name}[] = {{')
        for o, r0, r1, r2, r3, s0, st in e.log:
            assert o == 0xA0, hex(o)
            L.append(f'  {{{r0}, 0x{r1:x}, {r2}, {r3}, {st}, {f(s0)}}},')
        L.append('};')

    # --- velocity curve (value sent to id 0x29 of DSP 4, voice 0) -------------
    curve = []
    for v in range(128):
        e.log.clear()
        run_jx.setp(e, 0x429, 60)
        run_jx.setp(e, 0x43A, v)
        got = [s0 for o, r0, r1, r2, r3, s0, st in e.log if o == 0xB0 and r0 == 4 and e.u32(r1) == 0x29]
        curve.append(got[0] if got else 0.0)
        run_jx.setp(e, 0x43A, 0)
    L.append('static const float vel_curve[128] = {')
    for i in range(0, 128, 8):
        L.append('  ' + ', '.join(f(x) for x in curve[i:i + 8]) + ',')
    L.append('};')

    with open(os.path.join(out, 'tables.h'), 'w') as fh:
        fh.write('\n'.join(L) + '\n')
    print(f'tables.h: {len(blobs)} blobs ({sum(map(len, blobs.values()))} B), '
          f'{len(descs)} descriptors, {len(calls)} registrations')


if __name__ == '__main__':
    main()
