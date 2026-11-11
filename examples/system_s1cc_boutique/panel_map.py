#!/usr/bin/env python3
"""panel_map.py: the SYSTEM-1 panel lights a SYSTEM-S1CC kernel uses, as a panel map (README.md §1.7).

    python3 examples/system_s1cc/panel_map.py KERNEL_DIR [-o FILE]      draft map inferred from the kernel (stdout)
    python3 examples/system_s1cc/panel_map.py --check FILE              parse a map, print the SYNINFO mask

A panel map is a text file, one light per line: `NAME on|off`, NAME from LIGHTS below (the s1panel.h macro without
`S1_` where the light belongs to a parameter); `#` starts a comment. A light the file does not list is off. The
lights are the 39 entries of the OS's table (SYNINFO ctl[], ids 0xC0..0xE5, 0xEC, in panel order: LIGHTS, measured
2026-11-11): entering the PLUG-OUT the OS sends each one's on / off to the panel (OS 1.30 0x6001E526).

make_s1cc.py only reads the map (PANEL=FILE, default KERNEL_DIR/panel.txt: `s1cc build` copies VOICE.panel from
beside the kernel's source there). This tool writes the first draft from what the kernel reads; edit it from there.
Inference (the host of make_s1cc.py, README.md §3): the kernel's S1_PARAM / S1_VOICE_PARAM panels; s1_color(i) ->
OSCi COLOR + COLOR MOD; s1_pitch / s1_dt / s1_rdt (LFO_PITCH=1) -> LFO -> PITCH, OSC1 RANGE, PITCH ENV and the LFO;
s1_lfo_filter / s1_lfo_amp -> LFO -> FILTER / AMP and the LFO; s1_env(0) / s1_env(1) -> AMP / FILTER ADSR; the host
mixer (MIX=1) -> SUB / NOISE MIX. A COLOR MOD source alone (the LFO, the envelopes) does not light its section: the
MOD selector is lit. OSC1 CROSS MOD, HPF CUTOFF, AMP TONE, CRUSHER, PORTAMENTO: off (SYSTEM-S1CC has none of them).
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'sdk', 's1cc', 'tools'))
import s1kernel  # noqa: E402

# the OS's table = the panel's lights in panel order (SYNINFO ctl[], ids 0xC0..0xE5, 0xEC), measured 2026-11-11 with
# CTL_PROBE_TEST (model.c): (ctl id, light name, the s1panel.h parameter id the light belongs to or None)
LIGHTS = [
    (0xC0, 'LFO_SHAPE', 0x160), (0xC1, 'LFO_TO_PITCH', 0x163), (0xC2, 'LFO_FADE', 0x161),
    (0xC3, 'LFO_TO_FILTER', 0x164), (0xC4, 'LFO_RATE', 0x162), (0xC5, 'LFO_TO_AMP', 0x165),
    (0xC6, 'OSC1_WAVE', 0x167), (0xC7, 'OSC1_RANGE', 0x16A), (0xC8, 'OSC1_COLOR', 0x168),
    (0xC9, 'OSC1_COLOR_MOD', 0x169), (0xCA, 'OSC1_CROSS_MOD', None),
    (0xCB, 'OSC2_WAVE', 0x16C), (0xCC, 'OSC2_RANGE', 0x16F), (0xCD, 'OSC2_COLOR', 0x16D),
    (0xCE, 'OSC2_FINE', 0x170), (0xCF, 'OSC2_COLOR_MOD', 0x16E),
    (0xD0, 'OSC1_MIX', 0x174), (0xD1, 'OSC2_MIX', 0x175), (0xD2, 'SUB_MIX', 0x176), (0xD3, 'NOISE_MIX', 0x177),
    (0xD4, 'PITCH_ENV_AMOUNT', 0x17A), (0xD5, 'PITCH_ENV_ATTACK', 0x17B), (0xD6, 'PITCH_ENV_DECAY', 0x17C),
    (0xD7, 'FILTER_CUTOFF', 0x17D), (0xD8, 'HPF_CUTOFF', 0x180), (0xD9, 'FILTER_RESONANCE', 0x17F),
    (0xDA, 'FILTER_ENV_AMOUNT', 0x181), (0xDB, 'FILTER_KEY_TRACK', 0x186),
    (0xDC, 'FILTER_ATTACK', 0x182), (0xDD, 'FILTER_DECAY', 0x183), (0xDE, 'FILTER_SUSTAIN', 0x184),
    (0xDF, 'FILTER_RELEASE', 0x185),
    (0xE0, 'AMP_TONE', 0x18B), (0xE1, 'CRUSHER', None),
    (0xE2, 'AMP_ATTACK', 0x187), (0xE3, 'AMP_DECAY', 0x188), (0xE4, 'AMP_SUSTAIN', 0x189), (0xE5, 'AMP_RELEASE', 0x18A),
    (0xEC, 'PORTAMENTO', None),
]
CTL_IDS = [c for c, _, _ in LIGHTS]
NAME = {c: n for c, n, _ in LIGHTS}
BY_PANEL = {p: c for c, _, p in LIGHTS if p}


def parse(path):
    """{ctl id: on} of a panel map; unlisted lights off"""
    by_name = {n: c for c, n, _ in LIGHTS}
    on = {c: False for c in CTL_IDS}
    for no, line in enumerate(open(path), 1):
        t = line.split('#')[0].split()
        if not t:
            continue
        if len(t) != 2 or t[1] not in ('on', 'off'):
            raise SystemExit(f'{path}:{no}: expected `NAME on|off`, got: {line.strip()}')
        key = t[0][3:] if t[0].startswith('S1_') else t[0]
        if key not in by_name:
            raise SystemExit(f'{path}:{no}: unknown light {t[0]} (names: examples/system_s1cc/panel_map.py LIGHTS)')
        on[by_name[key]] = t[1] == 'on'
    return on


def mask(on):
    """SYNINFO mask (syninfo.h S1_CTL_ON): bit (ctl id - 0xC0) = light on"""
    return sum(1 << (c - 0xC0) for c, v in on.items() if v)


def infer(kdir):
    """{ctl id: (on, reason)} for every light"""
    k = s1kernel.Kernel.read(os.path.join(kdir, 'voice.s1k'))
    opt = lambda n, d: os.environ.get(n, k.options.get(n, d))  # noqa: E731  (make_s1cc.py opt())
    reads = {m for d in k.dsps for m in k.placeholders(d)}
    why = {}

    def use(p, reason):
        why.setdefault(p, reason)

    tpath = os.path.join(kdir, 'tables.txt')
    for tname, (panel, _, _) in (s1kernel.read_tables(tpath).items() if os.path.exists(tpath) else ()):
        for p in (panel if isinstance(panel, tuple) else (panel,)):
            if p:
                use(p, f'S1_PARAM {tname.split("~")[0]}')
    color = [i for i in (0, 1) if f'IN:color{i}' in reads]
    for i in color:
        use(0x168 + 5 * i, f's1_color({i})')
        use(0x169 + 5 * i, f's1_color({i}): COLOR MOD source')
    pitch = opt('LFO_PITCH', '1') == '1' and any(f'IN:{n}' in reads for n in ('pitch', 'dt', 'rdt'))
    lfo_f, lfo_a = 'IN:lfo_filter' in reads, 'IN:lfo_amp' in reads
    if pitch:
        for p in (0x163, 0x16A, 0x17A, 0x17B, 0x17C):
            use(p, 'host pitch modulation (s1_pitch / s1_dt / s1_rdt, LFO_PITCH=1)')
    if lfo_f:
        use(0x164, 's1_lfo_filter')
    if lfo_a:
        use(0x165, 's1_lfo_amp')
    if pitch or lfo_f or lfo_a:
        for p in (0x160, 0x161, 0x162):
            use(p, 'host LFO')
    for i, ps in ((0, range(0x187, 0x18B)), (1, range(0x182, 0x186))):
        if f'IN:env{i}' in reads:
            for p in ps:
                use(p, f's1_env({i})')
    host_lfo = bool(color or pitch or lfo_f or lfo_a)
    edm_lines = opt('EDM_LINES', '0') == '1' or 'M' in k.dsps
    if host_lfo and opt('MIX', '1') == '1' and not edm_lines:
        use(0x176, 'host mixer (MIX=1)')
        use(0x177, 'host mixer (MIX=1)')
    # region 0 / 1 code that reads a knob itself (model.c EDM forms, engine.c SL_WT)
    forms = (('EDM_STR_BOWED', (0x168, 0x16D)), ('EDM_STR_BLOWN', (0x16D,)), ('EDM_STR_PL', (0x16D, 0x17D)),
             ('EDM_STR_COMB', (0x168, 0x16D)))
    for o, ps in forms:
        if opt(o, '0') == '1':
            for p in ps:
                use(p, f'{o} (model.c)')
    if edm_lines:
        for p in (0x17D, 0x188, 0x18A):
            use(p, 'EDM_LINES string (model.c)')
    if opt('SL_WT', '0') not in ('0', ''):
        use(0x168, 'SL_WT frame (engine.c)')
    lit = {BY_PANEL[p]: r for p, r in why.items() if p in BY_PANEL}
    return {c: (c in lit, lit.get(c, '')) for c in CTL_IDS}


def write(res, kdir):
    w = max(len(n) for n in NAME.values())
    out = [f'# panel map of {os.path.relpath(kdir, ROOT) if kdir.startswith(ROOT) else kdir}: drafted by '
           'examples/system_s1cc/panel_map.py from the kernel',
           '# LIGHT on|off (panel order); unlisted = off', '']
    for c in CTL_IDS:
        on, reason = res[c]
        out.append(f'{NAME[c]:<{w}}  {"on " if on else "off"}' + (f'  # {reason}' if reason else ''))
    return '\n'.join(out) + '\n'


def main(argv):
    if len(argv) == 2 and argv[0] == '--check':
        on = parse(argv[1])
        print(f'S1_CTL_ON 0x{mask(on):012x}  on: ' + ' '.join(NAME[c] for c in CTL_IDS if on[c]))
        return 0
    if not argv or argv[0].startswith('-'):
        print(__doc__.strip(), file=sys.stderr)
        return 2
    kdir, out = argv[0], None
    if argv[1:2] == ['-o'] and len(argv) == 3:
        out = argv[2]
    text = write(infer(kdir), os.path.abspath(kdir))
    if out:
        open(out, 'w').write(text)
    else:
        sys.stdout.write(text)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
