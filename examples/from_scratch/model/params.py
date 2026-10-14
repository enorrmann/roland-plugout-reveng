"""Region 0 data of the from-scratch PLUG-OUT: parameter list and slot memory.

    python3 params.py <outdir>    -> <outdir>/model_list.h, <outdir>/storage.bin

* Parameter list (SYNINFO +0x28): records {i32 type, u32 size, ...} ending in
  type -1, walked by the OS (0x600525B4 and friends). Our parameters keep the
  JX-3P ids because the engine (region 1) uses them.
* Slot memory: the OS keeps the slot's patch memory inside region 0, at
  base + 0x8000 (table 0x602B9194, accessors 0x6000870E..0x600087A0):

      +0x008000  64 x 0x200   patch  (u32 per id, index = id - 0x6CD)
      +0x010000  64 x 0x898   patch2 (u32 words)
      +0x032600  64 x 0x3710  patch3 (sequencer, u32 words)
      +0x10EA00  0x200 / +0x10EC00 0x898 / +0x10F498 0x3710   edit buffers

  The image carries its initial contents; the plugin overwrites the 64 patches
  with its bank after the install. We fill everything with our own INIT patch.
"""
import os, struct, sys

# --- parameter list -------------------------------------------------------

def P(pid, name, lo=0, hi=0, init=0, flag=1, lst=0, kind=0, pos=None):
    """One parameter record. lo/hi 0/0 = full range. init/flag: meaning not
    verified (init looks like a default, flag is 0 only for Porta Type).
    lst = value-label list id (LISTS), kind = display/control type (0,1,2,4),
    pos = position in the group's panel page (default: next free)."""
    return dict(id=pid, name=name, lo=lo, hi=hi, init=init, flag=flag, lst=lst, kind=kind,
                pos=pos)

GROUPS = [
    ('LFO', [
        P(0x6cd, 'Waveform', lst=3),
        P(0x6ce, 'Delay Time', kind=2),
        P(0x6cf, 'Rate', kind=2),
        P(0x6d3, '', lo=0, hi=1, kind=4),
        P(0x73e, '', kind=4),
    ]),
    ('DCO MODULATOR', [
        P(0x6d0, 'LFO Mod', kind=1),
        P(0x6e7, 'ENV Mod', kind=1),
    ]),
    ('DCO-1', [
        P(0x6d4, 'Waveform', lst=5),
        P(0x6d7, 'Range', lst=7),
        P(0x6d6, 'Freq Mod', lst=4, kind=4),
    ]),
    ('DCO-2', [
        P(0x6d9, 'Waveform', lst=6),
        P(0x6dc, 'Range', lst=7, kind=1),
        P(0x6e0, 'Tune', lo=-128, hi=127, kind=1),
        P(0x6dd, 'FineTune', kind=1),
        P(0x6db, 'Freq Mod', lst=4, kind=4),
        P(0x6df, 'Cross Mod', lo=0, hi=2, lst=10, kind=1, pos=4),   # shares 4 with Freq Mod
    ]),
    ('SOURCE MIX', [
        P(0x6e1, 'DCO-1'),
        P(0x6e2, 'DCO-2'),
    ]),
    ('VCF', [
        P(0x6ed, 'HPF', kind=2),
        P(0x7e3, 'VelocitySens'),
        P(0x6ea, 'CutoffFreq'),
        P(0x6ec, 'Resonance'),
        P(0x6ee, 'ENV Mod', kind=1, pos=5),                   # position 4 left empty
        P(0x6d1, 'LFO Mod', kind=1),
        P(0x6f3, 'PitchFollow', kind=2),
    ]),
    ('VCA', [
        P(0x801, 'VelocitySens'),
        P(0x6f8, 'Brilliance', kind=1),
        P(0x709, 'Level', kind=2),
        P(0x736, 'VCA Mode', lo=0, hi=2, init=1, lst=8, kind=4),
    ]),
    ('ENV-1', [P(i, '', kind=2) for i in range(0x6ef, 0x6f3)]),
    ('ENV-2', [P(i, '', kind=2) for i in range(0x6f4, 0x6f8)]),
    ('EFFECT', [
        P(0x748, '', lst=11),
        P(0x749, ''),
        P(0x6f9, ''),
    ]),
    ('DLY/CH', [P(0x74a, ''), P(0x6fc, ''), P(0x6fb, '')]),
    ('REVERB', [P(0x74b, ''), P(0x74c, ''), P(0x6fa, '')]),
    ('CONTROL', [
        P(0x6fd, ''),                      # portamento time (the assigner reads it)
        P(0x6fe, '', lo=0, hi=1),          # portamento mode (assigner: legato flags)
        P(0x702, '', lo=0, hi=1),
        P(0x6ff, 'Key Assign', lo=0, hi=2, lst=9, kind=4),
        P(0x701, 'Porta Type', lo=0, hi=1, init=1, flag=0, lst=12, kind=2),
    ]),
    ('BEND', [P(0x700, '', kind=2), P(0x738, '', kind=2)]),
    ('BEND SENS', [P(0x739, '', kind=2), P(0x73a, '', kind=2)]),
    ('MOD SENS', [P(0x73b, '', kind=2), P(0x73c, '', kind=2)]),
    ('CIRCUIT MOD', [P(0x744, 'Amount', kind=2)]),
]

LISTS = {
    1: ['OFF', 'ON'],
    2: ['ON', 'OFF'],
    3: ['SIN', 'SQR', 'RND', 'RND', 'RND', 'RND'],
    4: ['OFF', 'LFO', 'ENV', 'LFO+ENV', 'LFO+ENV', 'LFO+ENV'],
    5: ['SAW', 'SQR', 'PULSE', 'PULSE', 'PULSE', 'PULSE'],
    6: ['SAW', 'SQR', 'PULSE', 'NOISE', 'NOISE', 'NOISE'],
    7: ['64FEET', '32FEET', '16FEET', '8FEET', '4FEET', '2FEET'],
    8: ['ENV-1', 'ENV-2', 'GATE'],
    9: ['POLY', 'MONO', 'UNISON'],
    10: ['OFF', 'SYNC', 'METAL'],
    11: ['OVERDRIVE', 'DISTORTION', 'CHORUS', 'FUZZ', 'CRUSHER', 'PHASER'],
    12: ['1', '2'],
}

# records after the lists; meaning unknown (type 7 names the VCA Mode id)
TAIL = [(4, [0]), (7, [0x736, 0, 1, 0, 0])]


def _name(s, n=20):
    b = s.encode('ascii')
    assert len(b) < n, s
    return b.ljust(n, b'\0')


def record_list():
    out = bytearray()
    def rec(t, body):
        out.extend(struct.pack('<iI', t, 8 + len(body)) + body)
    for gname, params in GROUPS:
        rec(0, struct.pack('<I', len(params)) + _name(gname))
        pos = 0
        for p in params:
            pos = pos if p['pos'] is None else p['pos']
            rec(1, struct.pack('<12i', p['id'], 1, p['lo'], p['hi'], p['init'], p['flag'],
                               p['lst'], p['kind'], pos, 0, 0, 0) + _name(p['name']))
            pos += 1
    for lid, labels in LISTS.items():
        # labels for values 0..n-1; the count is the number of distinct choices
        rec(2, struct.pack('<3I', lid, len(dict.fromkeys(labels)), 0))
        for s in labels:
            rec(3, _name(s))
    for t, words in TAIL:
        rec(t, struct.pack('<%di' % len(words), *words))
    out.extend(struct.pack('<i', -1))
    return bytes(out)

# --- slot memory ----------------------------------------------------------

PATCH_BASE = 0x6CD
INIT_NAME = 'INIT PATCH'
# our INIT patch (u32 value per id; ids not listed are 0). Signed parameters
# (DCO-2 Tune) are stored as signed values: 0 = centre.
INIT = {
    0x6cf: 175, 0x6d0: 128, 0x6d1: 128, 0x6d2: 128, 0x6d7: 3, 0x6dc: 3, 0x6dd: 128,
    0x6e1: 128, 0x6e6: 1, 0x6e7: 128, 0x6ea: 255, 0x6ee: 128, 0x6f1: 255, 0x6f3: 128,
    0x6f6: 255, 0x6f8: 128, 0x6fc: 128, 0x700: 2, 0x709: 128, 0x70a: 1, 0x734: 128,
    0x735: 1, 0x736: 1, 0x737: 128, 0x739: 255, 0x73a: 255, 0x73b: 128, 0x73c: 128,
    0x740: 11, 0x741: 128, 0x743: 1, 0x745: 128, 0x749: 128, 0x74b: 2, 0x74c: 128,
}
# patch2 default (word index: value); same in every patch of the stock SH-101
# image, so it is the OS's neutral record. 0x3F008081 / 0x3F800000 look like floats.
PATCH2 = {
    0: 0x3F008081, 151: 0x3F800000, 300: 50, 301: 153, 302: 7, 303: 255, 332: 20, 333: 2,
    334: 13, 336: 64, 337: 128, 364: 128, 365: 230, 366: 128, 367: 2, 369: 64, 370: 128,
    399: 255, 402: 255, 404: 64, 405: 128, 445: 20, 446: 3, 447: 13, 448: 10, 449: 255,
    478: 10, 479: 60, 480: 67, 481: 72, **{i: -1 for i in range(482, 495)},
}


def patch_rec(name=INIT_NAME):
    w = [0] * 128
    for pid, v in INIT.items():
        w[pid - PATCH_BASE] = v
    nm = name.encode('ascii').ljust(16)[:16]
    for k in range(4):                  # 16 characters, 4 per word, first in the high byte
        w[64 + k] = struct.unpack('>i', nm[4 * k:4 * k + 4])[0]
    return struct.pack('<128i', *w)


def patch2_rec():
    w = [0] * (0x898 // 4)
    for i, v in PATCH2.items():
        w[i] = v
    return struct.pack('<%di' % len(w), *w)


def patch3_rec():
    """Sequencer: a header and 64 steps of 54 words (8 notes C2, 8 values 49)."""
    w = [0] * (0x3710 // 4)
    w[0], w[2], w[3], w[5], w[6] = 15, 15, 1, 128, 128
    for s in range(64):
        o = 38 + 54 * s
        w[o:o + 8] = [36] * 8
        w[o + 16:o + 24] = [49] * 8
    return struct.pack('<%di' % len(w), *w)


STORAGE = 0x8000                       # offset in region 0
STORAGE_END = 0x10F498 + 0x3710


def storage():
    m = bytearray(STORAGE_END - STORAGE)
    def put(off, b):
        m[off - STORAGE:off - STORAGE + len(b)] = b
    a, b, c = patch_rec(), patch2_rec(), patch3_rec()
    for p in range(64):
        put(0x8000 + p * 0x200, a)
        put(0x10000 + p * 0x898, b)
        put(0x32600 + p * 0x3710, c)
    put(0x10EA00, a)
    put(0x10EC00, b)
    put(0x10F498, c)
    return bytes(m)


def main(out):
    os.makedirs(out, exist_ok=True)
    lst = record_list()
    words = struct.unpack('<%dI' % (len(lst) // 4), lst)
    with open(os.path.join(out, 'model_list.h'), 'w') as f:
        f.write('/* generated by model/params.py: parameter record list */\n')
        f.write('static const unsigned int model_list[%d] = {\n' % len(words))
        for i in range(0, len(words), 8):
            f.write('    ' + ', '.join('%#010x' % x for x in words[i:i + 8]) + ',\n')
        f.write('};\n')
    with open(os.path.join(out, 'storage.bin'), 'wb') as f:
        f.write(storage())
    print(f'parameter list: {len(lst)} bytes; slot memory: {STORAGE_END - STORAGE:#x} bytes')


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), 'build'))
