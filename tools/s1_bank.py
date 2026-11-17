#!/usr/bin/env python3
"""s1_bank.py: the SYSTEM-1's patch values and its patch files, for any PLUG-OUT's patch converter
(sdk/s1cc/examples/boutique_ported/tools/jp08_prm.py, sdk/s1cc/examples/jx3p_ported/tools/jx3p_bank.py).

    S1_NAMES, S1_DEF   the 112 values in file order (64 panel / patch values, ids 0x160..0x19F, then PLUGOUT_EXTEND01..40)
                       and their defaults
    write_prm(path, s1, name)        a PLUGOUT_PATCH<n>.PRM text file (the SYSTEM-1's RESTORE)
    bank_record(s1, name)            one KoaBankFile00002 record (tools/plugout_tool.py install --bank)
    write_bank(path, pats, tag, empty)   a 64-record bank: pats = [(s1, name)], the rest `empty` named "----------------"

Moved from jp08_prm.py (2026-11-17, unchanged: the JP-08 bank of jupiter8_bank.py is byte-identical before and after);
boutique_ported README.md §13.5 has the measurements behind it.
"""

# the SYSTEM-1's patch values in file order (OS 1.30 table 0x5b15c, ids 0x160..0x19F, 0x1B0..0x1DF) and defaults
# (SYSTEM-1 Script.xml struct "patch"; OSC2_COARSE as the .PRM writes it: bank byte - 11, README.md §13.5)
S1 = ('LFO_WAVE 0 LFO_FADE 0 LFO_RATE 0 LFO_PITCH 128 LFO_FLT 128 LFO_AMP 128 LFO_TRIG 0 OSC1_WAVE 0 OSC1_COLOR 0 '
      'OSC1_MOD 0 OSC1_RANGE 0 OSC1_CRS_MOD 0 OSC2_WAVE 0 OSC2_COLOR 0 OSC2_MOD 0 OSC2_RANGE 0 OSC2_TUNE 128 '
      'OSC2_RING 0 OSC2_SYNC 0 OSC2_COARSE 0 MIX_OSC1 128 MIX_OSC2 0 MIX_SUB_OSC 0 MIX_NOISE 0 MIX_OSC_TYPE 1 '
      'MIX_NOISE_TYPE 1 PIT_ENV 128 PIT_ATK 0 PIT_DCY 0 FLT_LPF 255 FLT_TYPE 1 FLT_RESO 0 FLT_HPF 0 FLT_ENV 128 '
      'FLT_ATK 0 FLT_DCY 0 FLT_SUS 0 FLT_REL 0 FLT_KEY 128 AMP_ATK 0 AMP_DCY 0 AMP_SUS 255 AMP_REL 0 AMP_TONE 128 '
      'CRUSHER 0 REVERB 0 DELAY 0 DELAY_TM 0 PORTAMENTO 0 LEGATO 0 MONO 0 BEND_RANGE 1 MOD_ASGN 0 TEMPO_SYNC 0 '
      'MOD_ID1 0 MOD_ID2 0 MOD_ID3 0 MOD_PRM1 0 MOD_PRM2 0 MOD_PRM3 0 PAT_LEV 128 PAT_LEV_SW 1 OSC1_EX 0 OSC2_EX 0 '
      'INPUT_JACK_SW 0 ARP_SW 0 ARP_TYPE 0 ARP_STEP 0 SCAT_TYPE 0 SCAT_DEP 0 CTRL_OCT 0 CTRL_HOLD 0').split()
S1_DEF = {S1[i]: int(S1[i + 1]) for i in range(0, len(S1), 2)}
for _i in range(40):
    S1_DEF['PLUGOUT_EXTEND%02d' % (_i + 1)] = 0
S1_NAMES = list(S1_DEF)                                  # 112, in file order


def write_prm(path, s1, name):
    out = ''.join('%s(%d);\r\n' % (n, s1[n]) for n in S1_NAMES) + 'PATCH_NAME(%s);' % name
    open(path, 'wb').write(out.encode('latin-1'))


# KoaBankFile00002 (README.md §13.5, docs Vol. IV ch. 4 "Bank files"): name 16 + header 27 + struct "patch" 640:
# 64 int2x4 (a byte as two nibbles), the name as 16 x 2 nibbles in 128 bytes (NAME0), 48 int8x4 (32 bits, 8 nibbles)
HEADER = bytes.fromhex('010a00020100070604060406040604060406040604080000030200')   # a SH-2 bank record's (unread)


def bank_record(s1, name):
    m = bytearray()
    for n in S1_NAMES[:64]:
        v = s1[n] + (11 if n == 'OSC2_COARSE' else 0)    # the bank stores the unsigned value (measured, §13.5)
        m += bytes((v >> 4 & 15, v & 15))
    nm = bytearray()
    for ch in name.encode('latin-1')[:16].ljust(16):
        nm += bytes((ch >> 4, ch & 15))
    m += nm.ljust(128, b'\0')
    for n in S1_NAMES[64:]:
        v = s1[n] & 0xFFFFFFFF
        m += bytes((v >> (28 - 4 * j)) & 15 for j in range(8))
    assert len(m) == 640
    return name.encode('latin-1')[:16].ljust(16) + HEADER + bytes(m)



def write_bank(path, pats, tag, empty):
    recs = [bank_record(s1, name) for s1, name in pats[:64]]
    recs += [bank_record(empty, '-' * 16)] * (64 - len(recs))
    open(path, 'wb').write(b'KoaBankFile00002' + tag + b''.join(recs))
