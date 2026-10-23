"""C-S1JX: the JX-3P's own DSP microcode (one DCO) on the SYSTEM-1, 4 voices (system-1/DSP_FIT.md, docs in
docs/examples_system_1_jx_README.md).

The JX-3P's six SYSTEM-8 slave DSPs become the four DSPs of the SYSTEM-1, one chain of two voices per chip:

    S8 DSP  program            SYSTEM-1
    DSP0    MAIN0  LFO/env v0,1  master B @0x1000  MAIN0 + DCO1(v0,1) + tail (output x VOLUME -> A118/A119)
    DSP2    MAIN2  + DCO1        master A @0x0100  MAIN2 + MAIN4 (one program)
    DSP4    MAIN4  VCF/VCA, mix
    DSP1    MAIN1  LFO/env v2,3  slave B  @0x0100  MAIN1;  section 0x1900: DCO1(v2,3)
    DSP3    MAIN3  + DCO1        slave A  @0x0100  MAIN5;  section 0x1900: MAIN3
    DSP5    MAIN5  VCF/VCA v2,3
    DCO2 is not used: its outputs read as 0 (the OSC2 controls do nothing).

Every literal address of the Roland programs is rewritten (dspreloc.py), nothing else changes:
  * bank 0 / bank 1 rings: each program gets its own range on its chip (disjoint even if A and B share them);
  * bank 5 (shared by A and B of a chip): each program's parameter block, and mailbox words for what went through
    MXM (Z 84nn -> M 80nn / O 82nn, S 86nn back), through the DSP2/3 <-> DCO bank-1 interface, and for constants
    (ZERO: read 0, DUMP: written, never read);
  * pair window A4xx (DSP0->1, DSP4<->5) and the S8 IOMD inputs/outputs: chip to chip by IOMD 600E.. (down) and
    6006.. (up); MAIN4's 6020/6021 -> OUT_L/OUT_R; the unknown S8 inputs (60e0-60e2, 6000-6006, 6010-6017) -> ZERO;
  * bank 6 (per DSP): an id offset where two programs share a DSP.
The ARM side (model.c, engine.c) writes the JX-3P model's parameters through the tables of build/s1jx.h.

    python3 s1jx.py                 -> build/s1jx.h (programs + tables), build/plan.txt
    python3 s1jx.py --check [n]     -> also: emulate the SYSTEM-1 layout next to the SYSTEM-8 chain (C emulator)

The microcode is read at build time from plugins/JX-3P (engine/programs.py); build/ is not committed.
"""
import os
import random
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
BUILD = os.path.join(HERE, 'build')
for p in ('asm', 'examples/from_scratch/re', 'examples/from_scratch/emu', 'examples/from_scratch/engine',
          'examples/system_1'):
    sys.path.insert(0, os.path.join(ROOT, p))
import dspreloc as R  # noqa: E402
sys.path.insert(0, HERE)
import model_full  # noqa: E402  the complete parameter model (VCF and envelopes included; see its docstring)
sys.modules['model_spec'] = model_full

MA, MB, SA, SB = range(4)                   # SYSTEM-1 DSPs: master A/B, slave A/B (also the emulator's DSP0-3)
S1_NAMES = ('master A', 'master B', 'slave A', 'slave B')
CHIP = (0, 0, 1, 1)
# unit -> (S8 DSP, Roland program); DCO1 units take the DCO1 waveform program (SAW by default)
UNITS = {'MAIN0': 0, 'MAIN1': 1, 'MAIN2': 2, 'MAIN3': 3, 'MAIN4': 4, 'MAIN5': 5, 'DCO1@2': 2, 'DCO1@3': 3}
# where each unit runs: S1 DSP, program location (address), order inside the location
PLACE = {
    'MAIN0': (MA, 0x100), 'MAIN2': (MA, 0x100),
    'MAIN4': (MB, 0x1000), 'DCO1@2': (MB, 0x1000),
    'MAIN5': (SA, 0x100), 'MAIN3': (SA, 0x1900),
    'MAIN1': (SB, 0x100), 'DCO1@3': (SB, 0x1900),
}
LOCATIONS = [(MA, 0x100, ['MAIN0', 'MAIN2']), (MB, 0x1000, ['MAIN4', 'DCO1@2', 'TAIL']),
             (SA, 0x100, ['MAIN5']), (SA, 0x1900, ['MAIN3']), (SB, 0x100, ['MAIN1']), (SB, 0x1900, ['DCO1@3'])]
DCO2_OUT = {0x221D, 0x2221, 0x2225, 0x2229}  # DCO2 slot outputs (docs/CUSTOM_OSC.md); MAIN2/3 read them at +1
B5_RESERVED = [set(range(0x000, 0x020)) | set(range(0x118, 0x120)),       # master: OS words, A118-A11F output
               set(range(0x000, 0x020))]                                  # slave
B5_TOP = 0x200
B5_DSP = 0x100     # bank-5 words a DSP writes go to A100-A1FF: every Roland SYSTEM-1 slave program stores only there
                   # (A000-A0FF is what the ARM writes, FD6C sel 0); our stores to A0xx hung the slave (C-S1JXS...,
                   # 2026-10-22). With the read-only parameters in bank 6, bank 5 only holds words some DSP writes
ZERO_ID = 0xFE                              # bank-6 id read as 0 (the ARM writes 0.0 to it on every DSP)
B0_START, B0_TOP = 0x020, 0x400            # bank 0 used by our programs per chip (the SYSTEM-1 corpus: 0018-0377)                              # the ARM writes A000-A1FF (FD6C sel 0 / 1)
B6_RESERVED = {MA: set(range(0x00, 0x10)) | {ZERO_ID}, MB: {0x1C, ZERO_ID}, SA: {ZERO_ID}, SB: {0x00, ZERO_ID}}   # OS (C002-C004, slave C000), VOLUME
IOMD_DOWN = list(range(0x600E, 0x6022))     # master -> slave
IOMD_UP = list(range(0x6006, 0x600E))       # slave -> master
HANDOFF = {'DCO1@2': 'MAIN2', 'DCO1@3': 'MAIN3'}   # slot program: the main that ran before it on the S8
HANDOFF_REGS = (2, 3)                       # the registers the DCO programs read before writing them
M_OUT, M_ZERO, M_VOLUME = (0xA118, 0xA119), (0xA11A, 0xA11B, 0xA11C, 0xA11D), 0x1C


def roland_programs(dco='saw'):
    """unpatched Roland words (execution order) from the user's plugins/JX-3P (engine/programs.py does the cutting)"""
    import programs as P
    img = P.unpack(os.path.join(ROOT, 'plugins', 'JX-3P'), BUILD)
    out = {}
    for name in ('main0', 'main1', 'main2', 'main3', 'main4', 'main5', 'dco1_' + dco):
        n, addr, sha = P.MICROCODE[name]
        import hashlib
        blob = img[addr - P.BASE: addr - P.BASE + ((n + 3) & ~3)]
        if hashlib.sha256(blob).hexdigest()[:16] != sha:
            raise SystemExit(f'{name}: not the JX-3P 1.01 PLUG-OUT')
        h = struct.unpack('<%dH' % (len(blob) // 2), blob)
        w = []
        for i in range(0, len(h), 2):
            w += [h[i + 1], h[i]]
        out[name.upper() if name.startswith('main') else 'DCO1'] = w[:n // 2]
    return out


class Plan:
    def __init__(self, words):
        """the plan; parameters stay in bank 5 (keep_b5) where moving them to bank 6 would put more than two bank-6
        loads in one instruction of a program with jumps (more never appears in Roland's code; three hung the
        SYSTEM-1, C-S1JXH1 MAIN1 instruction 48, 2026-10-23)"""
        self.keep_b5 = set()
        while True:
            self._build(words)
            more = self._bank6_overload()
            if not more:
                break
            self.keep_b5 |= more

    def _bank6_overload(self):
        """parameters to put back in bank 5: one per instruction with more than 2 bank-6 loads"""
        out = set()
        for u, w in self.words.items():
            per = {}
            for r in R.Prog(w).refs:
                if r.addr >> 12 == 0xC and r.op == 'ld':
                    per.setdefault(r.ins, []).append(r.addr & 0xFFF)
            s1 = PLACE[u][0]
            for ids in per.values():
                if len(ids) > 2:
                    moved = [self.p6_ids[s1][i] for i in ids if i in self.p6_ids[s1]]
                    if not moved:
                        raise SystemExit(f'{u}: more than 2 bank-6 loads in one instruction, none of them a moved parameter')
                    out.add(moved[0])
        return out

    def _build(self, words):
        self.p = {u: R.Prog(words['DCO1' if u.startswith('DCO1') else u], u) for u in UNITS}
        self.fp = {u: p.footprint() for u, p in self.p.items()}
        self.b5 = [dict(), dict()]              # chip -> {key: index 0x000-0x1FF}
        self.b5_owner = [dict(), dict()]        # chip -> {index: key}
        self.iomd = {}                          # key -> IOMD word
        self.b0off, self.b1off, self.b6off = {}, {}, {}
        self.ringmail = {}                      # chip -> {MXM key: bank-0 store address} (same-DSP MXM links)
        self.dump0 = {}                         # chip -> bank-0 address of DUMP (stores nobody reads)
        self.users = [dict(), dict()]           # chip -> {index: {S1 DSPs}} (to know cross-DSP words)
        self._allocate()

    # ---- allocation ----
    def word(self, chip, key):
        if key not in self.b5[chip]:
            lo = 0x020 if key[0] == 'P' and key[1:] in self.keep_b5 else B5_DSP   # read-only parameters: A0xx
            free = next(i for i in range(lo, B5_TOP) if i not in B5_RESERVED[chip] and i not in self.b5_owner[chip])
            self.b5[chip][key] = free
            self.b5_owner[chip][free] = key
        return self.b5[chip][key]

    def io(self, key, up):
        if key not in self.iomd:
            pool = IOMD_UP if up else IOMD_DOWN
            used = set(self.iomd.values())
            self.iomd[key] = next(a for a in pool if a not in used)
        return self.iomd[key]

    def _allocate(self):
        # rings: per chip, one contiguous block of bank 0 per unit: its bank-0 range, then its internal bank-1 range
        # (moved to bank 0: the SYSTEM-1 programs only use bank 1 at 2000-2023, and our relocated bank-1 words made the
        # device hang, 2026-10-22; bank 0 is the same kind of ring and is measured on the SYSTEM-1). Gaps of GAP words
        # (ring reach of these programs <= 16).
        GAP = 0x11                              # > 16, the largest ring reach of these programs
        for chip in (0, 1):
            nxt = B0_START
            for u in sorted(UNITS, key=lambda u: (PLACE[u][0], PLACE[u][1])):
                if CHIP[PLACE[u][0]] != chip:
                    continue
                for bank, attr in ((0x0, 'b0off'), (0x2, 'b1off')):
                    if bank == 0x0:
                        a = sorted(r.addr & 0xFFF for r in self.p[u].refs if r.addr >> 12 == 0)
                    else:
                        a = sorted(r.addr & 0xFFF for r in self.p[u].refs if r.addr >> 12 == 2
                                   and not self.b1_interface(u, r))
                    if not a:
                        continue
                    getattr(self, attr)[u] = nxt - a[0]
                    nxt += a[-1] - a[0] + 1 + GAP
            self.b0_end = getattr(self, 'b0_end', {})
            self.b0_end[chip] = nxt
            self.dump0[chip] = nxt                  # the DUMP word: alone, GAP words before the MXM ring mailboxes
            assert nxt <= B0_TOP, (chip, hex(nxt))
        # bank 6: per S1 DSP, programs in order; the first keeps its ids, the next ones move past the previous ones
        for s1 in range(4):
            used = set(B6_RESERVED[s1])
            for u in [u for u in UNITS if PLACE[u][0] == s1 and not u.startswith('DCO')]:
                ids = {a & 0xFFF for a in self.fp[u].get(0xC, (set(), set()))[0]} | self.model_p6(UNITS[u])
                off = 0
                while any(i + off in used for i in ids):
                    off += 0x10
                self.b6off[u] = off
                used |= {i + off for i in ids}
        # ARM-written parameters that the program only reads -> bank 6 (one id per word on the program's DSP). Bank 5
        # then only carries the mailboxes between the two DSPs of a chip and the words a program writes itself, so no
        # instruction combines a bank-5 load with a bank-5 store (never seen in Roland's SYSTEM-8 programs; two such
        # instructions hung the SYSTEM-1, C-S1JXH 2026-10-22, docs/examples_system_1_jx_README.md section 10)
        self.p6, self.p6_ids = {}, {s1: {} for s1 in range(4)}
        for s1 in range(4):
            used = set(B6_RESERVED[s1])
            for u in [u for u in UNITS if PLACE[u][0] == s1 and not u.startswith('DCO')]:
                used |= {(a & 0xFFF) + self.b6off[u] for a in self.fp[u].get(0xC, (set(), set()))[0]}
                used |= {i + self.b6off[u] for i in self.model_p6(UNITS[u])}
            free = [i for i in range(0x10, 0x100) if i not in used]
            for u in [u for u in UNITS if PLACE[u][0] == s1 and not u.startswith('DCO')]:
                d = UNITS[u]
                ld, st = self.fp[u].get(0xA, (set(), set()))
                own_st = {a & 0x3FF for a in st if (a & 0xFFF) < 0x400}
                ids = {a & 0x3FF for a in ld if (a & 0xFFF) < 0x400} | self.model_b5(d)
                for i in sorted(ids - own_st):
                    if (d, i) in self.keep_b5:
                        continue
                    if not free:
                        raise SystemExit(f'{S1_NAMES[s1]}: no free bank-6 id for P{d}:{i:#x}')
                    six = free.pop(0)
                    self.p6[(d, i)] = (s1, six)
                    self.p6_ids[s1][six] = (d, i)
        # bank 5 parameter blocks first (in id order), then everything else as relocate() meets it
        for u in ('MAIN0', 'MAIN2', 'MAIN4', 'MAIN1', 'MAIN3', 'MAIN5'):
            d, chip = UNITS[u], CHIP[PLACE[u][0]]
            ids = {a & 0x3FF for a in self.fp[u].get(0xA, (set(), set()))[0] | self.fp[u].get(0xA, (set(), set()))[1]
                   if (a & 0xFFF) < 0x400} | self.model_b5(d)
            for i in sorted(ids):
                if (d, i) not in self.p6:
                    self.word(chip, ('P', d, i))
        self.words = {}
        for loc in LOCATIONS:
            for u in loc[2]:
                if u in self.p:
                    self.words[u] = self.p[u].relocate(lambda r, u=u: self.map(u, r))
        self.reloc = dict(self.words)           # the pure relocation, before our glue (test_s1jx.py test_units)
        self._handoff()
        for u in self.words:
            self.words[u] = split_bank5(self.words[u], u, self.dump0[CHIP[PLACE[u][0]]] + 1)

    def _handoff(self):
        """the DCO slot programs start from the r2 / r3 their main left (on the SYSTEM-8 they ran right after it on
        the same DSP; test_s1jx.py test_scramble): ours, the main stores them at its end, the DCO loads them first"""
        import asm
        for dco, main in HANDOFF.items():
            d, chip = UNITS[dco], CHIP[PLACE[dco][0]]
            ws = {n: 0xA000 | self.mail(chip, ('REG', d, n), PLACE[main][0]) for n in HANDOFF_REGS}
            for n in HANDOFF_REGS:
                self.mail(chip, ('REG', d, n), PLACE[dco][0])
            st = asm.Assembler().assemble(''.join(f'st r{n}, [{ws[n]:04x}]\n' for n in HANDOFF_REGS))
            ld = asm.Assembler().assemble(''.join(f'ld r{n}, [{ws[n]:04x}]\n' for n in HANDOFF_REGS) + 'mov r4, r4\n')
            assert st.ok and ld.ok, (st.errors, ld.errors)
            w = self.words[main]
            assert w[-5:] == [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001]
            self.words[main] = w[:-5] + st.words + w[-5:]
            w = self.words[dco]
            assert w[:2] == [0x0A02, 0xFF70]
            self.words[dco] = w[:2] + ld.words + w[2:]

    @staticmethod
    def model_targets():
        import re
        import model_spec as M
        out = set()
        for _s, targets, _e, _w in M.OUT:
            for t in targets:
                m = re.match(r'([PB])(\d):(0x[0-9a-f]+)', t)
                out.add((m.group(1), int(m.group(2)), int(m.group(3), 16)))
        return out

    def model_b5(self, d):
        return {i for k, dd, i in self.model_targets() if k == 'B' and dd == d and i < 0x100} | \
            ({0x21, 0x22, 0x33, 0x34, 0x29, 0x3b} if d < 2 else {0x22, 0x29, 0x2a, 0x31, 0x38, 0x39} if d == 4 else
             {0x21, 0x28, 0x29, 0x30, 0x37, 0x38} if d == 5 else set())

    def model_p6(self, d):
        return {i for k, dd, i in self.model_targets() if k == 'P' and dd == d and i < 0x100}

    # ---- the address map ----
    def map(self, u, r):
        a, op = r.addr, r.op
        d, s1 = UNITS[u], PLACE[u][0]
        chip = CHIP[s1]
        bank = a >> 12
        dco = u.startswith('DCO')
        main_of_dco = 'MAIN%d' % d
        if bank == 0x0:
            return (a + self.b0off[u]) & 0xFFFF
        if bank == 0x2:
            if dco:                                         # DCO1 <-> its main: mailbox words
                return 0xA000 | self.mail(chip, ('DCO', d, a), s1)
            if d in (2, 3):
                dl, ds = self.fp['DCO1@%d' % d].get(0x2, (set(), set()))
                if op == 'st' and a in dl:
                    return 0xA000 | self.mail(chip, ('DCO', d, a), s1)
                if op == 'ld' and a - 1 in ds:
                    return 0xA000 | self.mail(chip, ('DCO', d, a - 1), s1)
                if op == 'ld' and a - 1 in DCO2_OUT:
                    return 0xC000 | ZERO_ID
            return (a & 0xFFF) + self.b1off[u]            # internal bank-1 ring -> bank 0 (see _allocate)
        if bank == 0x6:
            if op == 'st' and d == 4 and a in (0x6020, 0x6021):
                return 0xA000 | self.mail(chip, ('OUT', a), s1)
            if op == 'st':
                return self.dump0[chip]
            return 0xC000 | ZERO_ID                        # 60e0.., 6000.., 6010..: written outside the S8 slave
        if bank == 0x8:
            z = a & 0xFFF
            if op == 'st' and 0x400 <= z < 0x600:            # Z: to DSP d+2
                return self.mxm(chip, ('MZ', d, z & 0x1FF), d, d + 2, 'st', s1)
            if op == 'ld' and z < 0x400:                     # M (even) / O (odd): from DSP d-2's Z
                return self.mxm(chip, ('MZ', d - 2, z & 0x1FF), d - 2, d, 'ld', s1)
            if op == 'st' and z < 0x400:                     # M / O stored: back to DSP d-2's S
                return self.mxm(chip, ('MS', d, z & 0x1FF), d, d - 2, 'st', s1)
            if op == 'ld' and 0x600 <= z < 0x800:            # S: from DSP d+2
                return self.mxm(chip, ('MS', d + 2, z & 0x1FF), d + 2, d, 'ld', s1)
            raise ValueError(f'{u}: MXM {op} {a:04x}')
        if bank == 0xA:
            i = a & 0xFFF
            if i < 0x400:                                    # own bank 5: the program's block
                if (d, i) in self.p6:                        # a parameter the program only reads: bank 6
                    return 0xC000 | self.p6[(d, i)][1]
                return 0xA000 | self.word(chip, ('P', d, i))
            # pair window: partner d ^ 1 is on the other chip -> IOMD
            nn = i & 0x3FF
            partner = 'MAIN%d' % (d ^ 1)
            pl, ps = self.fp[partner].get(0xA, (set(), set()))
            if op == 'st':
                if a in pl:
                    return self.io(('PW', d, nn), up=bool(d & 1))
                return self.dump0[chip]
            if a in ps:
                return self.io(('PW', d ^ 1, nn), up=not (d & 1))
            return 0xC000 | ZERO_ID
        if bank == 0xC:
            return 0xC000 | ((a & 0xFFF) + self.b6off[u])
        raise ValueError(f'{u}: bank {bank:x} address {a:04x}')

    def b1_interface(self, u, r):
        """a bank-1 ref of u that is the DCO <-> main interface (mapped to bank-5 mailboxes, not a ring)"""
        if u.startswith('DCO'):
            return True
        d = UNITS[u]
        if d not in (2, 3):
            return False
        dl, ds = self.fp['DCO1@%d' % d].get(0x2, (set(), set()))
        a = r.addr
        return (r.op == 'st' and a in dl) or (r.op == 'ld' and (a - 1 in ds or a - 1 in DCO2_OUT))

    def mxm(self, chip, key, writer, reader, op, s1):
        """an MXM word of the SYSTEM-8. Writer and reader on the same SYSTEM-1 DSP: the bank-0 ring with stride 2 (store
        at R, load at R + 1: exactly MXM's one-sample latency, whatever the order); otherwise a bank-5 mailbox word.
        The ring keeps bank-5 accesses at one load and one store per instruction (DSP_HARDWARE.md 7.3; more hung the
        SYSTEM-1, C-S1JXQ 2026-10-22)."""
        if PLACE['MAIN%d' % writer][0] == PLACE['MAIN%d' % reader][0]:
            ring = self.ringmail.setdefault(chip, {})
            if key not in ring:
                ring[key] = self.dump0[chip] + 0x11 + 2 * len(ring)
                assert ring[key] + 2 <= B0_TOP, (chip, hex(ring[key]))
            return ring[key] + (1 if op == 'ld' else 0)
        return 0xA000 | self.mail(chip, key, s1)

    def mail(self, chip, key, s1):
        i = self.word(chip, key)
        self.users[chip].setdefault(i, set()).add(s1)
        return i

    # ---- programs per location ----
    def location_words(self, s1, addr, units, tail=True):
        parts = []
        for u in units:
            if u == 'TAIL':
                parts.append(self.tail())
            else:
                parts.append(truncate(nojump(self.words[u], u), u))
        w = R.concat(*parts)
        if w[:2] == [0x0A02, 0xFF70]:
            w[1] = 0xFF71                   # SYSTEM-1 start marker
        return w

    def tail(self):
        """ours: OUT_L / OUT_R (MAIN4's 6020 / 6021) x VOLUME (C01C) -> A118 / A119; A11A-A11D = 0"""
        import asm
        ol, orr = (0xA000 | self.b5[0][('OUT', 0x6020)], 0xA000 | self.b5[0][('OUT', 0x6021)])
        L = [f'ld r0, [{ol:04x}]', f'ld r1, [{orr:04x}]', f'ld r7, [c{M_VOLUME:03x}]', 'mov r4, r4',
             'mul r0, r0, r7', 'mul r1, r1, r7', f'st r0, [{M_OUT[0]:04x}]', f'st r1, [{M_OUT[1]:04x}]',
             'mov r2, #0.0'] + [f'st r2, [{a:04x}]' for a in M_ZERO]
        r = asm.Assembler().assemble('\n'.join(L) + '\n')
        if not r.ok:
            raise SystemExit(r.errors)
        return [0x0A02, 0xFF70] + r.words + [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001]

    def test_tail(self):
        """build SLAVE_TEST=1 (C-S1JXS): master B only: the slave's voices 2/3 (MAIN5 -> IOMD, the pair-window words
        MAIN4 sums) x VOLUME -> A118 / A119; nothing on master A. Splits a hang between the master programs and the rest"""
        import asm
        a, b = self.iomd[('PW', 5, 0)], self.iomd[('PW', 5, 1)]
        L = [f'ld r0, [{a:04x}]', f'ld r1, [{b:04x}]', f'ld r7, [c{M_VOLUME:03x}]', 'mov r4, r4',
             'addxy r0, r0, r1', 'mov r4, r4', 'mul r0, r0, r7', f'st r0, [{M_OUT[0]:04x}]', f'st r0, [{M_OUT[1]:04x}]',
             'mov r2, #0.0'] + [f'st r2, [{x:04x}]' for x in M_ZERO]
        r = asm.Assembler().assemble('\n'.join(L) + '\n')
        if not r.ok:
            raise SystemExit(r.errors)
        return [0x0A02, 0xFF71] + r.words + [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001]

    def programs(self):
        return [(s1, addr, units, self.location_words(s1, addr, units)) for s1, addr, units in LOCATIONS]

    # ---- ARM tables ----
    def b5_index(self, d, i):
        """where the ARM writes S8 bank-5 word (d, i): a bank-5 word index of d's chip (< 0x200), or 0x8000 | sel << 8 |
        id for a bank-6 id of DSP sel (parameters the program only reads), or None"""
        if (d, i) in self.p6:
            s1, six = self.p6[(d, i)]
            return 0x8000 | (s1 & 1) << 8 | six
        return self.b5[CHIP[PLACE['MAIN%d' % d][0]]].get(('P', d, i))

    def report(self):
        L = []
        for s1, addr, units, w in self.programs():
            L.append(f'{S1_NAMES[s1]} @{addr:#06x}: {" + ".join(units)}  {2 * len(w)} bytes')
        for u in UNITS:
            L.append(f'  {u:7} b0 {self.b0off.get(u, 0):+#x}  b1 {self.b1off.get(u, 0):+#x}  b6 {self.b6off.get(u, 0):+#x}')
        for chip in (0, 1):
            used = sorted(self.b5_owner[chip])
            L.append(f'{"master" if chip == 0 else "slave"} bank 5: {len(used)} words, A{used[0]:03x}-A{used[-1]:03x}')
            for i in used:
                L.append(f'    A{i:03x} {self.b5_owner[chip][i]}')
        L.append('IOMD: ' + ', '.join(f'{k}->{v:04x}' for k, v in sorted(self.iomd.items(), key=lambda x: x[1])))
        return '\n'.join(L)


def c_array(name, w):
    p = list(w) + [0] * (len(w) & 1)
    words = [(p[i] << 16) | p[i + 1] for i in range(0, len(p), 2)]
    body = ',\n'.join('    ' + ', '.join(f'0x{x:08x}' for x in words[i:i + 6]) for i in range(0, len(words), 6))
    return (f'#define MC_{name}_BYTES 0x{2 * len(w):x}\n'
            f'static const uint32_t mc_{name}[{len(words)}] = {{\n{body}\n}};\n')


def header(plan):
    import make_s1
    out = ['/* generated by s1jx.py: the JX-3P microcode relocated for the SYSTEM-1 (Roland bytes: build/ only, '
           'never commit) and the parameter tables */', '#include <stdint.h>', '']
    names = {(MA, 0x100): 'MA_MAIN', (MB, 0x1000): 'MB_MAIN', (SA, 0x100): 'SA_MAIN', (SA, 0x1900): 'SA_SEC',
             (SB, 0x100): 'SB_MAIN', (SB, 0x1900): 'SB_SEC'}
    for s1, addr, units, w in plan.programs():
        out.append(f'/* {S1_NAMES[s1]} @{addr:#x}: {" + ".join(units)} */')
        out.append(c_array(names[(s1, addr)], w))
    out.append('/* SLAVE_TEST build: master B only (Plan.test_tail) */')
    out.append(c_array('MB_TEST', plan.test_tail()))
    out.append(c_array('END', make_s1.empty_section()[0]))
    out.append('/* an empty main program (diagnostic builds) */')
    out.append(c_array('EMPTY', [0x0A02, 0xFF71, 0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001]))
    for word in (0xA004, 0xA005, 0xA01D):
        out.append(c_array(f'HDR_{word:04X}', frame_header(word)))
        out.append(c_array(f'CLOSE_{word:04X}', make_s1.close(word)[0]))
    # bank 5: S8 (dsp, id 0x20-0x4F) -> index 0x000-0x1FF on that DSP's chip (0xFFFF: not read by any program)
    out.append('/* bank 5 of the S8: DSP d, id 0x20 + k -> on the chip of d (even d: master, odd d: slave): a bank-5 word '
               'index (< 0x200, FD6C sel = index >> 8), or 0x8000 | sel << 8 | id = a bank-6 id of DSP sel (FCEC; '
               'parameters the program only reads); 0xFFFF = dropped */')
    out.append('static const uint16_t b5_index[6][0x30] = {')
    for d in range(6):
        row = [plan.b5_index(d, 0x20 + k) for k in range(0x30)]
        out.append('    { ' + ', '.join('0x%04x' % (x if x is not None else 0xFFFF) for x in row) + ' },')
    out.append('};')
    out.append('/* bank 6: S8 DSP d -> SYSTEM-1 DSP (0 A, 1 B of its chip) and id offset; ids >= 0x100 dropped */')
    sel = [PLACE['MAIN%d' % d][0] & 1 for d in range(6)]
    off = [plan.b6off['MAIN%d' % d] for d in range(6)]
    out.append('static const uint8_t b6_sel[6] = { ' + ', '.join(map(str, sel)) + ' };')
    out.append('static const uint16_t b6_off[6] = { ' + ', '.join('0x%02x' % x for x in off) + ' };')
    out.append(f'#define M_VOLUME 0x{M_VOLUME:02x}')
    for chip in (0, 1):
        rows = state_table(plan, chip)
        out.append(f'/* start-up state of the {"master" if chip == 0 else "slave"}, computed here from the model '
                   f'(default_vals + ENGINE_EXTRA): kind 0 = bank-5 word idx (FD6C), 1 = bank-6 id of DSP sel (FCEC) */')
        out.append(f'static const struct {{ uint8_t kind, sel; uint16_t idx; uint32_t bits; }} jx_state{chip}[] = {{\n' +
                   ',\n'.join(f'    {{ {k}, {sl}, 0x{i:03x}, 0x{b:08x} }}' for k, sl, i, b in rows) + '\n};')
    out.append('/* the INIT PATCH (Script.xml defaults) and 0 for the part / system ids: the parameter values */')
    vals = default_vals()
    out.append('static const struct { uint16_t id; int32_t v; } jx_defaults[] = {\n' + ',\n'.join(
        f'    {{ 0x{i:03x}, {v if v < 0x80000000 else v - 0x100000000} }}' for i, v in sorted(vals.items())) + '\n};')
    return '\n'.join(out) + '\n'


def state_table(plan, chip):
    """[(kind, sel, idx, bits)]: every bank-5 / bank-6 word of `chip` after a full refresh with default_vals(), the
    voices excluded (jx.h writes them); what jx.h's runtime model would write, without float maths on the ARM"""
    rows = {}
    for (kind, d, i), v in model_writes(default_vals()).items():
        if d & 1 != chip:
            continue
        if kind == 'B':
            idx = plan.b5_index(d, i)
            if idx is not None and idx & 0x8000:
                rows[(1, (idx >> 8) & 1, idx & 0xFF)] = v
            elif idx is not None:
                rows[(0, 0, idx)] = v
        elif i < 0x100:
            rows[(1, PLACE['MAIN%d' % d][0] & 1, i + plan.b6off['MAIN%d' % d])] = v
    for sel in (0, 1):
        rows[(1, sel, ZERO_ID)] = 0.0             # ZERO (bank-6 word every relocated program may read as 0)
    return [(k, sl, i, float_bits(v)) for (k, sl, i), v in sorted(rows.items())]


def nojump(words, name):
    """diagnostic (NOJUMP=UNIT[,UNIT]): every jump word (009Y in a C position) of that unit becomes 0000, a condition
    word that skips nothing: both branches run one after the other (wrong results, same lengths). To test whether the
    SYSTEM-1 DSP runs jumps (no SYSTEM-1 program uses one)"""
    if name not in os.environ.get('NOJUMP', '').split(','):
        return words
    out, n = [], 0
    for _o, ins in R.dspdis.instructions(words):
        ins = list(ins)
        L = R.dspisa.Layout(ins)
        for pos, t in enumerate(L.t):
            if t == 'J':
                ins[1 + pos] = 0x0000
                n += 1
        out += ins
    print(f'NOJUMP {name}: {n} jump words neutralized', file=sys.stderr)
    return out


def truncate(words, name):
    """diagnostic (TRUNC=UNIT:lo:hi, e.g. TRUNC=MAIN3:1:95): keep only instructions lo..hi-1 of that unit (index 0 =
    its start marker, always kept) and its end; to find the instruction that hangs the device by halving"""
    spec = os.environ.get('TRUNC', '')
    if not spec or spec.split(':')[0] != name:
        return words
    lo, hi = (int(x) for x in spec.split(':')[1:])
    ins = [list(i) for _o, i in R.dspdis.instructions(words)]
    body = ins[1:-2] if ins[-2:] == [[0x0B02, 0x00F8], [0x0001]] else ins[1:]
    if body and body[-1] == [0x0A02, 0xFF70]:
        body = body[:-1]
    keep = [w for i in body[lo - 1:hi - 1] for w in i]
    print(f'TRUNC {name}: instructions {lo}..{hi - 1} of {len(body)}', file=sys.stderr)
    return ins[0] + keep + [0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001]


def split_bank5(words, name, dummy0=None):
    """keep every instruction within the per-instruction access limits (chain.access_violations): bank 0 at most 2
    loads and 2 stores (more than 2 stores never appears in Roland's code, and 3 hung the SYSTEM-1: C-S1JXH, MAIN3
    instruction 55, 2026-10-23), and ONE bank-5 access (two loads, two stores, or a load with a store, hung the
    SYSTEM-1 or never appear in Roland's code). Relocation creates such instructions when bank-1 or MXM words move to
    bank 0 / bank 5. They are pure transfers here; each becomes: the extra stores (one instruction each, before, so
    they store the same registers), the instruction itself, the extra loads (one each, after, so the next original
    instruction sees every loaded value, as before). Only for programs without jumps."""
    import asm
    import disasm
    import isa
    table = isa.Table.load()
    out, changed = [], 0
    for _off, ins in R.dspdis.instructions(words):
        p = R.Prog(list(ins))
        b5 = [r for r in p.refs if r.addr >> 12 == 0xA]
        s0 = [r for r in p.refs if r.addr >> 12 == 0x0 and r.op == 'st']
        l0 = [r for r in p.refs if r.addr >> 12 == 0x0 and r.op == 'ld']
        if len(b5) <= 1 and len(s0) <= 2 and len(l0) <= 2:
            out += list(ins)
            continue
        text, _ = disasm.program(list(ins), table, 'x')
        parts = [x.strip() for line in text.splitlines() if line.strip() and not line.startswith((';', '.'))
                 for x in line.split(';')[0].split('|')]
        parts = [re.sub(r'@\S*$', '', x) for x in parts]
        if not all(x.startswith(('ld ', 'st ')) for x in parts):
            raise ValueError(f'{name}: instruction over the access limits is not pure transfers: {parts}')
        bank = lambda x: int(re.search(r'\[([0-9a-f]{4})\]', x).group(1), 16) >> 12
        st5 = [x for x in parts if x.startswith('st') and bank(x) == 0xA]
        ld5 = [x for x in parts if x.startswith('ld') and bank(x) == 0xA]
        st0 = [x for x in parts if x.startswith('st') and bank(x) == 0x0]
        ld0 = [x for x in parts if x.startswith('ld') and bank(x) == 0x0]
        rest = [x for x in parts if bank(x) not in (0x0, 0xA)]
        if st5:                       # one bank-5 access stays: a store (the bank-5 loads go after)
            k5, before, after = st5[:1], st5[1:], ld5
        else:
            k5, before, after = ld5[:1], [], ld5[1:]
        before = st0[2:] + before
        after = after + ld0[2:]
        core = st0[:2] + k5 + rest + ld0[:2]
        lines = before + [' | '.join(core)] + after
        r = asm.Assembler().assemble('\n'.join(lines) + '\n')
        if not r.ok:
            raise SystemExit(f'{name}: {lines}: {r.errors}')
        out += r.words
        changed += 1
    if changed and any(t == 'J' for _o, i in R.dspdis.instructions(words)
                       for t in R.dspisa.Layout(list(i)).t):
        raise ValueError(f'{name}: has jumps; inserting instructions would move their targets')
    return out


def frame_header(word):
    """program memory 0x0000 of a DSP, as every SYSTEM-1 PLUG-OUT loads it: the frame counter E021 into `word`
    when `word` is 0 (our assembly; same words as theirs)"""
    import asm
    r = asm.Assembler().assemble(f'ld r0, [{word:04x}]\ncmp r0, r0, #0\nld r0, [e021]\n'
                                 f'cond !12, 1, 0 | st r0, [{word:04x}] | st r0, [{word:04x}]\n')
    if not r.ok:
        raise SystemExit(r.errors)
    return r.words


def gen_model():
    """build/model.h from model_full.py with engine/gen_model.py (unchanged; it imports model_spec)"""
    import gen_model as G
    assert G.M is model_full
    G.main(BUILD)


# ---------------------------------------------------------------------------------------------------------- check
def emu_map_s1(plan, words_by_loc):
    """emulator view of the SYSTEM-1 programs: bank-5 words used by both DSPs of a chip -> pair window (A400 +),
    bank 6 per DSP -> C000 + 0x200 * DSP + id (the emulator has one bank 6 for all DSPs)"""
    out = {}
    for s1, addr, units, w in words_by_loc:
        chip = CHIP[s1]
        p = R.Prog(w, f'{s1}@{addr:x}')

        def f(r, s1=s1, chip=chip):
            a = r.addr
            if a >> 12 == 0xA and (a & 0xFFF) < 0x200 and len(plan.users[chip].get(a & 0x1FF, ())) > 1:
                return 0xA400 | (a & 0x1FF)
            if a >> 12 == 0xC:
                return 0xC000 + 0x200 * s1 + (a & 0x1FF)
            return a
        out[(s1, addr)] = p.relocate(f)
    return out


def emu_map_s8(words):
    """the S8 reference: bank 6 per DSP the same way (C000 + 0x200 * dsp + id)"""
    out = {}
    for u, d in UNITS.items():
        p = R.Prog(words['DCO1' if u.startswith('DCO') else u], u)
        out[u] = p.relocate(lambda r, d=d: 0xC000 + 0x200 * d + (r.addr & 0x1FF) if r.addr >> 12 == 0xC else r.addr)
    return out


NULL_DCO2 = [0x0A02, 0xFF70, 0x0903, 0x0050, 0x0000, 0x7809, 0x8FE0, 0x8FE2, 0x8FE4, 0x8FE6, 0x2221, 0x221D,
             0x2229, 0x2225, 0x0A02, 0xFF70, 0x0B02, 0x00F8, 0x0001]     # [2221] [221d] [2229] [2225] <- 0


def default_vals():
    """the INIT PATCH of the JX-3P (Script.xml defaults of the patch struct, ids 0x6CD-0x70C); every other model
    source (part, system, panel offsets, bend / modulation) 0"""
    sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 're'))
    import jxpatch
    import model_spec as M
    vals = {i: 0 for i in M.REFRESH_ORDER}
    vals.update({i: d for i, _n, d in jxpatch.patch_params()})
    # float parameters the SYSTEM-8 OS sends (group 5): 0x7E4 = the cutoff the JX-3P writes last (0 closes the
    # filter), 0x74D = the LFO rate; here both follow the patch values
    vals[0x7E4] = float_bits(vals[0x6EA] / 255)
    vals[0x74D] = float_bits(vals[0x6CF] / 255)
    return vals


def float_bits(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


# what engine.c writes outside the model: the output level of MAIN4 (e_unmute, P4:0x4a)
ENGINE_EXTRA = [('P', 4, 0x4A, 1.0)]


def model_writes(vals):
    import test_model as T
    w = T.model_state(vals, 0)
    for k, d, i, v in ENGINE_EXTRA:
        w[(k, d, i)] = v
    return w


def note_writes(voice, note, vel):
    """engine.c note_pitch / note_gate for S8 slave 0, local voice `voice`"""
    sys.path.insert(0, os.path.join(ROOT, 'examples', 'from_scratch', 'engine'))
    import re
    src = open(os.path.join(ROOT, 'examples', 'from_scratch', 'engine', 'build', 'model.h')).read()
    vc = [float(x.rstrip('f')) for x in re.search(r'vel_curve\[128\] = \{(.*?)\}', src).group(1).split(',')]
    u, k = voice >> 1, voice & 1
    d, s = 4 + u, 0xF * k - u
    w = [('B', u, 0x21 + 0x12 * k, (note - 12) / 12.0)]
    if vel > 0:
        w += [('B', d, 0x2a + s, 1.0), ('B', d, 0x29 + s, vc[vel]), ('B', d, 0x22 + s, vel / 127.0),
              ('B', u, 0x22 + 0x12 * k, 1.0)]
    else:
        w += [('B', u, 0x22 + 0x12 * k, 0.0)]
    return w


def check(plan, words, n_states=2, seconds=0.25):
    import chain
    import cchain
    s8w = emu_map_s8(words)
    s1w = emu_map_s1(plan, plan.programs())
    P = lambda w, name: chain.Program(w, name)
    ref_progs = {d: (P(s8w['MAIN%d' % d], 'MAIN%d' % d), []) for d in range(6)}
    for d in (2, 3):
        ref_progs[d] = (ref_progs[d][0], [P(s8w['DCO1@%d' % d], 'DCO1'), P(NULL_DCO2, 'NULL2')])
    s1_progs = {MA: (P(s1w[(MA, 0x100)], 'MA'), []), MB: (P(s1w[(MB, 0x1000)], 'MB'), []),
                SA: (P(s1w[(SA, 0x100)], 'SA'), [P(s1w[(SA, 0x1900)], 'SA2')]),
                SB: (P(s1w[(SB, 0x100)], 'SB'), [P(s1w[(SB, 0x1900)], 'SB2')])}
    import test_model as T
    rng = random.Random(5)
    n = int(96000 * seconds)
    ok = True                       # True / False / None (inconclusive)
    for it in range(n_states):
        vals = default_vals()
        if it:
            vals.update({i: v for i, v in T.rand_state(rng).items() if i not in (0x748, 0x6d9)})
        ref = cchain.Slave(ref_progs, native=False, threads=False)
        s1 = cchain.Slave(s1_progs, native=False, threads=False)

        def write_ref(kind, d, i, v):
            if kind == 'B':
                ref.p_bank(d, i, v)
            elif i < 0x200:
                ref.params[0xC000 + 0x200 * d + i] = v

        def write_s1(kind, d, i, v):
            s1d = PLACE['MAIN%d' % d][0]
            if kind == 'B':
                idx = plan.b5_index(d, i)
                if idx is None:
                    return
                chip = CHIP[s1d]
                if len(plan.users[chip].get(idx, ())) > 1:
                    raise SystemExit(f'parameter word A{idx:03x} is also a mailbox')
                s1.p_bank(s1d, idx, v)
            elif i < 0x100:
                s1.params[0xC000 + 0x200 * s1d + i + plan.b6off['MAIN%d' % d]] = v
        for (kind, d, i), v in model_writes(vals).items():
            write_ref(kind, d, i, v)
            write_s1(kind, d, i, v)
        s1.params[0xC000 + 0x200 * MB + M_VOLUME] = 1.0
        notes = [(0, 60, 100), (1, 64, 100), (2, 67, 100), (3, 71, 100)]
        for v, note, vel in notes:
            for kind, d, i, x in note_writes(v, note, vel):
                write_ref(kind, d, i, x)
                write_s1(kind, d, i, x)
        a = ref.run(n, (0x6020, 0x6021))
        ol = 0x400 | plan.b5[0][('OUT', 0x6020)]
        out = []
        for _ in range(n):
            s1.step()
            out.append(s1.d[MB].bank5[0x118])
        r = a[0x6020]
        best = max(range(0, 8), key=lambda lag: corr(r[2000:n - 8], out[2000 + lag:n - 8 + lag]))
        c = corr(r[2000:n - 8], out[2000 + best:n - 8 + best])
        pr, po = max(map(abs, r)), max(map(abs, out))
        print(f'  state {it}: S8 peak {pr:.3f}, S1 peak {po:.3f}, best lag {best}, correlation {c:.4f}')
        if not pr > 0.01 or pr > 4:     # silent, NaN or blown up: the emulator's filter (docs, section 5)
            print('    inconclusive: the SYSTEM-8 reference itself is not usable audio')
            ok = None if ok is True else ok
            continue
        ok = (ok is not False) and c > 0.95 and po > 0.01
    print('check:', {True: 'OK', False: 'DIFFERENT', None: 'INCONCLUSIVE'}[ok])
    return ok is not False


def corr(x, y):
    n = min(len(x), len(y))
    x, y = x[:n], y[:n]
    mx, my = sum(x) / n, sum(y) / n
    sxy = sum((a - mx) * (b - my) for a, b in zip(x, y))
    sxx = sum((a - mx) ** 2 for a in x)
    syy = sum((b - my) ** 2 for b in y)
    return sxy / (sxx * syy) ** 0.5 if sxx and syy else 0.0


def main():
    os.makedirs(BUILD, exist_ok=True)
    words = roland_programs(os.environ.get('DCO1', 'saw'))
    plan = Plan(words)
    open(os.path.join(BUILD, 's1jx.h'), 'w').write(header(plan))
    gen_model()
    open(os.path.join(BUILD, 'plan.txt'), 'w').write(plan.report() + '\n')
    print(plan.report().split('\n  ')[0])
    for s1, addr, units, w in plan.programs():
        print(f'  {S1_NAMES[s1]:9} @{addr:#06x} {" + ".join(units):24} {2 * len(w):5d} bytes')
    if '--check' in sys.argv:
        k = sys.argv.index('--check')
        n = int(sys.argv[k + 1]) if len(sys.argv) > k + 1 and sys.argv[k + 1].isdigit() else 2
        return 0 if check(plan, words, n) else 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
