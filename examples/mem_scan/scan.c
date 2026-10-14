/*
 * Region 0 init hook (C part): search OS memory for a masked byte pattern and
 * report the result over the names of the three PLUG-OUT slots (as
 * ../mem_names/hook.s does with raw bytes): 24 bytes = 6 big-endian u32
 *
 *     slot 1: count      | match 1
 *     slot 2: match 2    | match 3
 *     slot 3: match 4    | match 5
 *
 * count = all matches in LO..HI, match n = the n-th match after SKIP matches
 * (0 = none). Optional second pattern Q: a P match only counts if Q occurs
 * within -WIN..+WIN bytes of it (QSTEP-aligned), e.g. two Thumb immediates in
 * the same function.
 *
 * Patterns are passed as immediates (no .rodata, nothing slot-relative):
 * byte i of the pattern is byte i (little endian) of P0 (i < 4) / P1 (i >= 4),
 * MASK bits in M0 / M1 select the bits compared. Build-time -D (scan.py):
 * LO HI STEP SKIP P0 P1 M0 M1 PLEN [Q0 Q1 QM0 QM1 QLEN WIN QSTEP].
 */
typedef unsigned char u8;
typedef unsigned int u32;

#ifndef SKIP
#define SKIP 0
#endif
#ifndef STEP
#define STEP 1
#endif

static int match(const volatile u8 *p, u32 a0, u32 a1, u32 m0, u32 m1, u32 n)
{
    for (u32 i = 0; i < n; i++) {
        u32 b = i < 4 ? a0 >> (8 * i) : a1 >> (8 * (i - 4));
        u32 m = i < 4 ? m0 >> (8 * i) : m1 >> (8 * (i - 4));
        if ((p[i] ^ b) & m & 0xFF)
            return 0;
    }
    return 1;
}

static u8 hexd(u32 v) { v &= 15; return v < 10 ? '0' + v : 'A' - 10 + v; }

void scan_main(void)
{
    u32 r[6], count = 0, n = 0, i, k;

    for (i = 0; i < 6; i++)
        r[i] = 0;

    for (u32 a = LO; a + PLEN <= HI; a += STEP) {
        if (!match((const volatile u8 *)a, P0, P1, M0, M1, PLEN))
            continue;
#ifdef QLEN
        {
            u32 lo = a >= LO + WIN ? a - WIN : LO, hi = a + WIN, q, ok = 0;
            if (hi + QLEN > HI)
                hi = HI - QLEN;
            lo &= ~(u32)(QSTEP - 1);
            for (q = lo; q <= hi; q += QSTEP)
                if (match((const volatile u8 *)q, Q0, Q1, QM0, QM1, QLEN)) {
                    ok = 1;
                    break;
                }
            if (!ok)
                continue;
        }
#endif
        if (count >= SKIP && n < 5)
            r[1 + n++] = a;
        count++;
    }
    r[0] = count;

    for (k = 0; k < 3; k++) {
        volatile u8 *name = (volatile u8 *)(0x61800010u + k * 0x200000u);
        for (i = 0; i < 16; i++) {
            u32 w = r[2 * k + i / 8];
            name[i] = hexd(w >> (28 - 4 * (i % 8)));
        }
    }
}
