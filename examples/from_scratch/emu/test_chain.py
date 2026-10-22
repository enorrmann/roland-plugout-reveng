"""Tests of the voice-chain study (docs/VOICE_ALLOCATION.md §6), one group per question A-H, plus regressions.

    python3 test_chain.py              all tests (about 5 minutes: the DSP simulator is pure Python)
    python3 test_chain.py A G          only the tests whose name contains test_A / test_G

Each test can fail: it checks either a fact read from Roland's listings (study only, needs the local
engine/build/programs.h; skipped without it) or a prediction of the emulator (chain.py) for a hardware model.
Where the prediction depends on the model, the test checks both the chosen model and the alternative, so that the
measurement that tells them apart is explicit. Only our programs run in the emulator.
"""
import os
import sys
import traceback

import chain
import listing
import sigcheck
import voices

FS = voices.FS_DIR
HI, LO = 81, 69                     # note 81 = 880 Hz, note 69 = 440 Hz (pitch 5.75 / 4.75)


class Skip(Exception):
    pass


def need_listing():
    if not listing.available():
        raise Skip('engine/build/programs.h missing (run engine/build.sh)')


def cents(f, ref):
    import math
    return 1200 * math.log2(f / ref) if f > 0 else float('inf')


def run(which, model, pitches, gates, n=900, sources=None, words=chain.AUDIBLE):
    """one slave; pitches[lv] = note, gates = local voices on from sample 100. Returns {word: samples after 300}"""
    s = chain.Slave(voices.programs(which, sources), model)
    e = voices.Engine(s)
    for lv, note in enumerate(pitches):
        e.pitch(lv, note)
    out = {w: [] for w in words}
    for t in range(n):
        if t == 100:
            for lv in gates:
                e.note_on(lv, pitches[lv])
        s.step()
        for w in words:
            out[w].append(s.iomd.get(w, 0.0))
    return {w: v[300:] for w, v in out.items()}


def heard(out):
    """(peak, frequency) of the audible words 6020 + 6021 (they carry the same signal in v3 / v4)"""
    seq = out[0x6020]
    return chain.peak(seq), chain.freq(seq)


# ---------------------------------------------------------------- regressions
def test_regression_v3_programs_unchanged():
    """make_v3.sources() still assembles to the v3m*.hex files of the C-V3 build that was measured"""
    for d, text in voices.make_v3.sources().items():
        w, _ = voices.make_v3.words(text)
        assert w == voices.hex_words(os.path.join(FS, 'v3', f'v3m{d}.hex')), f'v3m{d} changed'


def test_regression_no_roland_bytes():
    if not os.path.exists(sigcheck.PROGRAMS_H):
        raise Skip('programs.h missing')
    rol, own = sigcheck.roland_programs(), sigcheck.own_word_lists()
    for d in ('v3', 'v4'):
        for f in sorted(os.listdir(os.path.join(FS, d))):
            if f.endswith('.hex'):
                p = os.path.join(FS, d, f)
                assert not sigcheck.check_words(p, voices.hex_words(p), rol, own), p


def test_regression_v4_chain_all_voices():
    """chosen model: every voice of v4 reaches 6020 alone, at its own pitch, silent before its gate"""
    for lv in range(4):
        p = [LO] * 4
        p[lv] = HI
        pk, f = heard(run('v4', chain.Model(), p, [lv]))
        assert pk > 0.9 and abs(cents(f, 880)) < 5, (lv, pk, f)


# ---------------------------------------------------------------- A: voice 1 (DSP2 voice B)
def test_A_v3_voice1_silent():
    """v3 MAIN4 multiplies the gate of voice 1 (0x39) by M 006, which nobody writes: voice 1 is silent [medido]"""
    pk, _ = heard(run('v3', chain.Model(), [LO, HI, LO, LO], [1]))
    assert pk == 0.0, pk


def test_A_v3_voice0_gate_plays_voice1_saw():
    """v3 MAIN4 multiplies the gate of voice 0 by M 005 = DSP2 Z 405 = the saw of voice B: the sound of voice 0
    has the pitch of voice 1. Prediction for the device (not measured yet): voice 0 at 880 Hz, voice 1 at 440 Hz,
    gate voice 0 -> 440 Hz is heard"""
    pk, f = heard(run('v3', chain.Model(), [HI, LO, LO, LO], [0]))
    assert pk > 0.9 and abs(cents(f, 440)) < 5, (pk, f)


def test_A_v3_same_with_and_without_ring():
    """the v3 result of A does not depend on the bank-1 ring: the saw also writes [221D] / [2225], which are what
    [2221] / [2229] return with the ring (4 samples later)"""
    for ring in (True, False):
        pk, f = heard(run('v3', chain.Model(ring1=ring), [HI, LO, LO, LO], [0]))
        assert pk > 0.9 and abs(cents(f, 440)) < 5, (ring, pk, f)


# ---------------------------------------------------------------- B: DSP3 / DSP5 voices
def test_B_v3_voices23_only_reach_60b0():
    """v3 sends voices 2, 3 to IOMD 60B0 (DSP5), which is not heard [medido WAVE_BUS 4b]; 6020 stays at 0"""
    out = run('v3', chain.Model(), [LO, LO, HI, LO], [2], words=(0x6020, 0x6021, 0x60B0))
    assert chain.peak(out[0x6020]) == 0.0 and chain.peak(out[0x60B0]) > 0.0


def test_B_listing_pair_window():
    """Roland: MAIN5 stores A400-A403 and MAIN4 loads them; MAIN4 stores A404-A407 and MAIN5 loads them; MAIN0
    stores A404.. and MAIN1 loads them [listado]: bank 5 0x400-0x7FF is a window shared by the pair"""
    need_listing()
    r = {m: listing.refs(m, 0xA400, 0xA800) for m in ('MAIN0', 'MAIN1', 'MAIN4', 'MAIN5')}
    assert r['MAIN5'][1] == r['MAIN4'][0] == {0xA400, 0xA401, 0xA402, 0xA403}
    assert r['MAIN4'][1] == r['MAIN5'][0] == {0xA404, 0xA405, 0xA406, 0xA407}
    assert r['MAIN1'][0] <= r['MAIN0'][1] and not r['MAIN1'][1] and not r['MAIN0'][0]


def test_B_v4_pair_window_decides():
    """v4 sends voices 2, 3 through A400 / A401: audible with the pair window, silent without it (the hardware test
    of B is C-V4 voices 2 and 3)"""
    pk, f = heard(run('v4', chain.Model(), [LO, LO, HI, LO], [2]))
    assert pk > 0.9 and abs(cents(f, 880)) < 5, (pk, f)
    pk, _ = heard(run('v4', chain.Model(dp_pairs=False), [LO, LO, HI, LO], [2]))
    assert pk == 0.0, pk


# ---------------------------------------------------------------- C: slave 1
LINK_C1 = {0x6060: 0x6000, 0x6061: 0x6001, 0x6062: 0x6002, 0x6063: 0x6003}


def run2(which, model, n=700):
    """two slaves, only slave 1's local voice 0 plays (880 Hz; its other voices hold the same pitch, because the v3
    gate of voice 0 plays the saw of voice 1, question A); returns slave 0 6020 and slave 1 6020"""
    b = chain.Bmc2(voices.programs(which), model)
    e1 = voices.Engine(b.s[1])
    voices.Engine(b.s[0])
    for lv in range(4):
        e1.pitch(lv, HI)
    a, c = [], []
    for t in range(n):
        if t == 100:
            e1.note_on(0, HI)
        b.step()
        a.append(b.s[0].iomd.get(0x6020, 0.0))
        c.append(b.s[1].iomd.get(0x6020, 0.0))
    return a[300:], c[300:]


def test_C_listing_main4_mixes_iomd_inputs():
    """Roland's MAIN4 copies IOMD 6000-6003 to bank 1 and adds them with its own voices and A400/A401 (eight
    sources), and writes 6060-6063 [listado]"""
    need_listing()
    fw = dict(listing.forwards('MAIN4'))
    assert all(a in fw for a in (0x6000, 0x6001, 0x6002, 0x6003))
    assert {0x6060, 0x6061, 0x6062, 0x6063} <= listing.refs('MAIN4', 0x6000, 0x6100)[1]


def test_C_v3_slave1_silent_needs_a_link():
    """v3 slave 1 does produce its voice at its own 6020. If that word were heard (no link), slave 1 would sound,
    against the measurement (C-V3L: only voice 0 of slave 0). With the link hypothesis C1 (slave 1 6060-6063 ->
    slave 0 6000-6003) v3 slave 1 is silent and v4 slave 1 is heard."""
    s0, s1 = run2('v3', chain.Model(link=LINK_C1))
    assert chain.peak(s1) > 0.9 and chain.peak(s0) == 0.0, (chain.peak(s1), chain.peak(s0))
    s0, s1 = run2('v4', chain.Model(link=LINK_C1))
    assert chain.peak(s0) > 0.9 and abs(cents(chain.freq(s0), 880)) < 5, (chain.peak(s0), chain.freq(s0))


# ---------------------------------------------------------------- D: who writes [2220]-[222A]
def test_D_listing_slot_outputs_read_plus_one():
    """the eight DCO outputs (odd [221B]-[2229]) are read by MAIN2 at +1 (even [221C]-[222A]), never as written,
    and no program writes the even words [listado]"""
    need_listing()
    out = set()
    for p in ('DCO1_SAW', 'DCO1_SQR', 'DCO2_SAW', 'DCO2_SQR', 'DCO2_NOISE'):
        out |= listing.refs(p, 0x2200, 0x2300)[1]
    assert out == {0x221B, 0x221D, 0x221F, 0x2221, 0x2223, 0x2225, 0x2227, 0x2229}
    ld2, st2 = listing.refs('MAIN2', 0x2200, 0x2300)
    plus = {a + 1 for a in out}
    assert plus <= ld2 and not (out & ld2) and not (plus & st2)


def test_D_ring_decides_v4_and_v4b():
    """v4 reads [2222] / [222A]: heard only if bank 1 is a ring. v4b reads [2221] / [2229]: heard either way.
    C-V4 silent and C-V4B heard on the device would reject the ring."""
    p = [HI, LO, LO, LO]
    pk, f = heard(run('v4', chain.Model(ring1=True), p, [0]))
    assert pk > 0.9 and abs(cents(f, 880)) < 5, (pk, f)
    pk, _ = heard(run('v4', chain.Model(ring1=False), p, [0]))
    assert pk == 0.0, pk
    for ring in (True, False):
        pk, f = heard(run('v4b', chain.Model(ring1=ring), p, [0], sources=voices.make_v4.sources(ring=False)))
        assert pk > 0.9 and abs(cents(f, 880)) < 5, (ring, pk, f)


# ---------------------------------------------------------------- E: bank 1 shared by DSP2 and DSP3?
def test_E_arm_table():
    """slave OS bank table (s_boot 0x0111C580): DSP0 and DSP1 have the same bank-1 base, DSP2-DSP5 one each [listado]"""
    t = listing.dt1_table()
    if t is None:
        raise Skip('work/os/s_boot/s_boot.bin missing')
    assert t[0] == t[1] and len(set(t[1:])) == 5, ['%08x' % x for x in t]


def test_E_shared_bank1_would_cross_voices():
    """if DSP2 and DSP3 shared bank 1, voice 0 (DSP2 voice A) would play DSP3's saw (voice 2): measurable"""
    p = [HI, LO, LO, LO]
    pk, f = heard(run('v4', chain.Model(), p, [0]))
    assert abs(cents(f, 880)) < 5, f
    pk, f = heard(run('v4', chain.Model(shared1=((2, 3),)), p, [0]))
    assert abs(cents(f, 440)) < 5, f


# ---------------------------------------------------------------- F: main before slot
def test_F_unison_does_not_decide():
    """C-V3L unison is in tune [medido], but that does not decide F: with the ring and slot-first, the saw's voice B
    reads [2214] = the [2207] of 13 samples before (voice A's pitch), and the v3 gate of voice 0 plays voice B"""
    p = [HI] * 4
    for mf in (True, False):
        pk, f = heard(run('v3', chain.Model(main_first=mf), p, [0, 1, 2, 3]))
        assert pk > 0.9 and abs(cents(f, 880)) < 5, (mf, pk, f)


def roland_layout_dsp2():
    """our own DSP2 main with Roland's MAIN2 input layout [listado]: the DCO2 pitch in [2207] and a non-pitch
    parameter (Roland: A02A) in the word below, [2206]; here 0.25. Output as v4 ([2222] -> Z 404)"""
    t = voices.make_v4.sources()[2].splitlines()
    return '\n'.join(t[:1] + ['mov r1, #0.25', 'mov r4, r4', 'st r1, [2206]'] + t[1:]) + '\n'


def test_F_csaw_layout_needs_main_first():
    """C-SAW (Roland's mains, our saw reading [2207]) is in tune from note 36 to 96 [documentado, CUSTOM_OSC.md].
    With the ring, a slot that ran first would read [2207] = the [2206] (A02A, not a pitch) of the sample before.
    So: ring => main first. Without the ring the order does not change the pitch."""
    src = voices.make_v4.sources()
    src[2] = roland_layout_dsp2()
    p = [HI, LO, LO, LO]
    pk, f = heard(run('v4', chain.Model(main_first=True), p, [0], sources=src))
    assert pk > 0.9 and abs(cents(f, 880)) < 5, (pk, f)
    _, f = heard(run('v4', chain.Model(main_first=False), p, [0], sources=src))
    assert abs(cents(f, 880)) > 100, f


# ---------------------------------------------------------------- G: MXM word number
def test_G_listing_forwards_keep_the_word():
    """MAIN2 copies M 0nn to Z 4nn with the same nn (004-00B, 00C-012, 013-015) [listado]"""
    need_listing()
    fw = [(a, b) for a, b in listing.forwards('MAIN2') if a >> 8 == 0x80 and b >> 8 == 0x84]
    same = [p for p in fw if p[1] - p[0] == 0x400]
    assert len(same) >= 18 and len(same) >= len(fw) - 1, fw


def test_G_shift_would_make_voice1_heard():
    """with Z 405 -> M 006 (shift 1) v3 voice 1 would be heard; it is not [medido]: the word number is kept"""
    pk, _ = heard(run('v3', chain.Model(mxm_shift=1), [LO, HI, LO, LO], [1]))
    assert pk > 0.9, pk
    pk, _ = heard(run('v3', chain.Model(mxm_shift=0), [LO, HI, LO, LO], [1]))
    assert pk == 0.0, pk


# ---------------------------------------------------------------- H: tuning
def test_H_chain_is_in_tune():
    """the chain does not detune: v4 voice 0 and v3 (voice 1 holding the same note, as in unison) within 1 cent
    for low, middle and high notes. The +-40 cents of C-V3L mono do not come from the DSP chain."""
    for note in (48, 64, 76):
        ref = chain.note_hz(note)
        _, f = heard(run('v4', chain.Model(), [note, LO, LO, LO], [0], n=3600))
        assert abs(cents(f, ref)) < 1, ('v4', note, f, ref)
        _, f = heard(run('v3', chain.Model(), [note, note, LO, LO], [0], n=3600))
        assert abs(cents(f, ref)) < 1, ('v3', note, f, ref)


def main(sel):
    tests = [(k, v) for k, v in globals().items() if k.startswith('test_') and callable(v)]
    if sel:
        tests = [(k, v) for k, v in tests if any(k.startswith(f'test_{s}') for s in sel)]
    bad = 0
    for k, fn in tests:
        try:
            fn()
            print('PASS', k, flush=True)
        except Skip as e:
            print('SKIP', k, '-', e, flush=True)
        except Exception:
            bad += 1
            print('FAIL', k, flush=True)
            traceback.print_exc()
    print(f'{len(tests) - bad}/{len(tests)} passed or skipped')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
