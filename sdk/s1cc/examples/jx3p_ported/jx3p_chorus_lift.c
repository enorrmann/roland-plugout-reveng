/* written by sdk/s1cc/tools/dsplift.py: jx3p_chorus=/home/emilio/git/roland-plugout-reveng/work/jx3p/chorus/fx_ch.hex */
#include "dsplift_rt.h"

static void jx3p_chorus(LiftM *M)
{
    double *r = M->r, n[8], rv[64];
    int rz[64], jto = -1;
    uint32_t um, uv;
    uint64_t sk, dn;
    (void)rv, (void)rz, (void)jto, (void)sk, (void)dn, (void)um, (void)uv;
    /* 0: f7 r7, r0   [0a02 ff70] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); QUANT(n); DONE(1, 7); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 1: ld r0, [2200]@0 | ld r1, [2200]@1   [1e05 0fe0 1fe2 2200 2200] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[0] = RD(M, 0x2200); }
        { n[1] = RD(M, 0x2200); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 2: mov r2, r0 | mov r3, r1   [1503 2040 3041] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[0]; QUANT(n); DONE(1, 2); }
        { RESET(); n[3] = r[1]; QUANT(n); DONE(2, 3); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 3: ld r5, [064a]@0 | ld r4, [e013]@1   [1e05 5fe0 4fe2 064a e013] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[5] = RD(M, 0x064a); }
        { n[4] = RD(M, 0xe013); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 4: ph_add r5, r3, r4   [0a02 dec4] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = r[5] + r[4]; QUANT(n); DONE(1, 5); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 5: cmp r5, r0, #-8@0:32 | setflags 3, 1   [1b05 de30 b9b1 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[5] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 5); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 6: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 7: addxy r5, r5, #-8@0:32 | cond !13, 1, 0   [1705 5d70 1b10 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 1 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[5] = AWRAP(r[5] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 5); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 8: hset h7, r5   [0b02 40c5] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 0) & 1) != 0 ? 0 : 0;
        if (!(sk >> 0 & 1)) { M->H[7] = pymod(r[5], 512); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 9: st r5, [0649]@0   [0c03 dfe0 0649] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0649, r[5]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 10: ld r4, [0647]@0 | ld r6, [e013]@1   [1e05 4fe0 6fe2 0647 e013] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[4] = RD(M, 0x0647); }
        { n[6] = RD(M, 0xe013); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 11: addxy r4, r4, r6 | mov r0, #0.585938@0:24   [1505 4d06 0060 2960 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = AWRAP(r[4] + r[6]); QUANT(n); DONE(1, 4); }
        { RESET(); n[0] = 0x1.2c00000000000p-1; QUANT(n); DONE(2, 0); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 12: min r4, r4, r0   [0a02 c100] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = pymin(r[4], r[0]); RESZ(n[4], (r[4]) == r[0]); QUANT(n); DONE(1, 4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 13: cmp r4, r0, -r0 | st r4, [0646]@0 | setflags 3, 1   [3a05 ce08 cfe0 b9b1 0646] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[4] + -r[0]); QUANT(n); DONE(1, 4); }
        { WR(M, 0x0646, r[4]); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 14: ld r7, [0648]@0   [0c03 7fe0 0648] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[7] = RD(M, 0x0648); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 15: mov r4, #0@0:16 | addxy r4, r7, #0.0026@2:24 | cond 13, 1, 2   [2d07 4050 4de2 1a12 0000 00aa 6400] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        if (!(sk >> 0 & 1)) { RESET(); n[4] = 0x0.0p+0; QUANT(n); DONE(1, 4); }
        if (!(sk >> 1 & 1)) { RESET(); n[4] = AWRAP(r[7] + 0x1.54c8000000000p-9); QUANT(n); DONE(2, 4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 16: min r4, r4, #1@0:16   [0a03 c110 3000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = pymin(r[4], 0x1.0000000000000p+0); RESZ(n[4], (r[4]) == 0x1.0000000000000p+0); QUANT(n); DONE(1, 4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 17: st r4, [0647]@0   [0c03 cfe0 0647] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0647, r[4]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 18: ld r4, [c1f2]@0 | ld r6, [c1f1]@1   [1e05 4fe0 6fe2 c1f2 c1f1] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[4] = RD(M, 0xc1f2); }
        { n[6] = RD(M, 0xc1f1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 19: mul r4, r4, #1.1@6:24 | subxy r7, r6, #1@0:16 | ld r5, [c1f3]@1 | ld r1, [060b]@2   [5f0a 4526 7f90 5fe2 1fe4 3000 c1f3 060b 3199 9900] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = AWRAP(r[4] * 0x1.1999900000000p+0); QUANT(n); DONE(1, 4); }
        { RESET(); n[7] = AWRAP(0x1.0000000000000p+0 - r[6]); QUANT(n); DONE(2, 7); }
        { n[5] = RD(M, 0xc1f3); }
        { n[1] = RD(M, 0x060b); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 20: mul r0, r2, r5 | mul r1, r1, r4 | ld r5, [0648]@0   [2e05 0485 1444 5fe0 0648] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = AWRAP(r[2] * r[5]); QUANT(n); DONE(1, 0); }
        { RESET(); n[1] = AWRAP(r[1] * r[4]); QUANT(n); DONE(2, 1); }
        { n[5] = RD(M, 0x0648); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 21: mul r1, r1, r5   [0902 1445] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[1] = AWRAP(r[1] * r[5]); QUANT(n); DONE(1, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 22: mac_p1p2 r0, r0, r6, r2, r7, r1 | ld r1, [060d]@1 | ld r5, [c1f3]@2   [2b07 0c0e 1fe2 5fe4 23c1 060d c1f3] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = (r[6] * r[0]) + (r[1]) + (r[7] * r[2]); QUANT(n); DONE(1, 0); }
        { n[1] = RD(M, 0x060d); }
        { n[5] = RD(M, 0xc1f3); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 23: mul r0, r3, r5 | mul r1, r1, r4 | ld r5, [0648]@0 | st r0, [0600]@1   [5f07 04c5 1444 5fe0 8fe2 0648 0600] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = AWRAP(r[3] * r[5]); QUANT(n); DONE(1, 0); }
        { RESET(); n[1] = AWRAP(r[1] * r[4]); QUANT(n); DONE(2, 1); }
        { n[5] = RD(M, 0x0648); }
        { WR(M, 0x0600, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 24: mul r1, r1, r5   [0902 1445] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[1] = AWRAP(r[1] * r[5]); QUANT(n); DONE(1, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 25: mac_p1p2 r0, r0, r6, r3, r7, r1   [0803 0c0e 33c1] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = (r[6] * r[0]) + (r[1]) + (r[7] * r[3]); QUANT(n); DONE(1, 0); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 26: mac_p1wc1 r5, r2, #0.5@2:16, #0.5@4, r3 | ld r4, [0610]@3 | st r0, [0601]@4 | ld r7, [c1f0]@5   [580b 5892 4fe6 8fe8 7fea 3500 2800 2800 0610 0601 c1f0] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = (0x1.0000000000000p-1 * r[2]) + (0x1.0000000000000p-1 * r[3]); QUANT(n); DONE(1, 5); }
        { n[4] = RD(M, 0x0610); }
        { WR(M, 0x0601, r[0]); }
        { n[7] = RD(M, 0xc1f0); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 27: mul r5, r5, r7 | mul r4, r4, #0.07921255@2:24 | ld r6, [060f]@0   [2e07 5547 4522 6fe0 060f 1447 4600] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = AWRAP(r[5] * r[7]); QUANT(n); DONE(1, 5); }
        { RESET(); n[4] = AWRAP(r[4] * 0x1.4474600000000p-4); QUANT(n); DONE(2, 4); }
        { n[6] = RD(M, 0x060f); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 28: mac_p1p2c2r r5, r5, #0.07921255@8:24, #0.1584244@3, r6, r4 | ld r4, [0611]@1 | ld r6, [0612]@2 | st r5, [060e]@3   [580c 5d68 4fe2 6fe4 dfe6 6ac4 0611 0612 060e 1447 4622 88e8] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = (0x1.4474600000000p-4 * r[5]) + (r[4]) + (0x1.4474000000000p-3 * r[6]); QUANT(n); DONE(1, 5); }
        { n[4] = RD(M, 0x0611); }
        { n[6] = RD(M, 0x0612); }
        { WR(M, 0x060e, r[5]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 29: mac_p1p2c2 r5, r4, #1.21644402@4:24, #-0.533294@7, r6, r5 | ld r1, [0613]@1   [1408 5d24 1fe2 69c5 0613 3376 8ea8 885f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = (0x1.3768e00000000p+0 * r[4]) + (r[5]) + (-0x1.110be00000000p-1 * r[6]); QUANT(n); DONE(1, 5); }
        { n[1] = RD(M, 0x0613); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 30: mul r1, r1, #-0.999019@2:24 | mul r6, r6, #0.07921255@5:24 | st r5, [0610]@0   [2e08 1462 65a5 dfe0 0610 affb fb14 4746] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[1] = AWRAP(r[1] * -0x1.ff7f600000000p-1); QUANT(n); DONE(1, 1); }
        { RESET(); n[6] = AWRAP(r[6] * 0x1.4474600000000p-4); QUANT(n); DONE(2, 6); }
        { WR(M, 0x0610, r[5]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 31: mac_p1p2c2r r5, r5, #0.07921255@6:24, #0.1584244@1, r4, r6 | ld r4, [0613]@1 | ld r6, [0614]@2   [2b0a 5d66 4fe2 6fe4 4a46 0613 0614 1447 4622 88e8] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = (0x1.4474600000000p-4 * r[5]) + (r[6]) + (0x1.4474000000000p-3 * r[4]); QUANT(n); DONE(1, 5); }
        { n[4] = RD(M, 0x0613); }
        { n[6] = RD(M, 0x0614); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 32: mac_p1p2c2 r5, r4, #1.21644402@4:24, #-0.533294@7, r6, r5 | ld r2, [0615]@1   [1408 5d24 2fe2 69c5 0615 3376 8ea8 885f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = (0x1.3768e00000000p+0 * r[4]) + (r[5]) + (-0x1.110be00000000p-1 * r[6]); QUANT(n); DONE(1, 5); }
        { n[2] = RD(M, 0x0615); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 33: mac_p1p2c2 r5, r5, #0.999019@4:24, #0.9980383@7, r2, r1 | st r5, [0612]@1   [1408 5d64 dfe2 29c1 0612 2ffb fb2f f7f7] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = (0x1.ff7f600000000p-1 * r[5]) + (r[1]) + (0x1.fefee00000000p-1 * r[2]); QUANT(n); DONE(1, 5); }
        { WR(M, 0x0612, r[5]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 34: st r5, [0614]@0   [0c03 dfe0 0614] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0614, r[5]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 35: ld r1, [c1f5]@0 | ld r0, [061b]@1   [1e05 1fe0 0fe2 c1f5 061b] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[1] = RD(M, 0xc1f5); }
        { n[0] = RD(M, 0x061b); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 36: addxy r0, r0, r1 | ld r1, [061f]@0   [1804 0c01 1fe0 061f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = AWRAP(r[0] + r[1]); QUANT(n); DONE(1, 0); }
        { n[1] = RD(M, 0x061f); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 37: addxy r0, r0, r1   [0902 0c01] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = AWRAP(r[0] + r[1]); QUANT(n); DONE(1, 0); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 38: ph_wrap r0, r3, r0   [0a02 8ee0] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = wrap1(r[0]); QUANT(n); DONE(1, 0); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 39: cmp r0, r0, #0@0:16 | setflags 3, 1   [1b04 8e10 b9b1 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[0] + 0x0.0p+0); QUANT(n); DONE(1, 0); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 40: mov r1, #5e-07@2:24 | st r0, [061a]@0   [1806 1062 8fe0 061a 0000 0800] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[1] = 0x1.0000000000000p-21; QUANT(n); DONE(1, 1); }
        { WR(M, 0x061a, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 41: mov r1, -r1 | cond 13, 1, 0   [1703 1049 1a10] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[1] = -r[1]; QUANT(n); DONE(1, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 42: ph_abs r0, r3, r0 | st r1, [061e]@0 | ld r2, [0607]@1 | ld r3, [c1f6]@2   [7308 8ed0 9fe0 2fe2 3fe4 061e 0607 c1f6] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = fabs(r[0]); QUANT(n); DONE(1, 0); }
        { WR(M, 0x061e, r[1]); }
        { n[2] = RD(M, 0x0607); }
        { n[3] = RD(M, 0xc1f6); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 43: subxy r0, r0, #1@0:16 | mul r3, r2, r3 | bic r1, r1, r0 | ld r4, [c1f1]@1   [5c07 0e10 3483 9e40 4fe2 3000 c1f1] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = AWRAP(0x1.0000000000000p+0 - r[0]); QUANT(n); DONE(1, 0); }
        { RESET(); n[3] = AWRAP(r[2] * r[3]); QUANT(n); DONE(2, 3); }
        { RESET(); n[1] = r[0]; QUANT(n); DONE(3, 1); }
        { n[4] = RD(M, 0xc1f1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 44: addxy r6, r0, #0.3@0:24 | addxy r7, r1, #0.3@3:24   [1506 6c20 7c63 24cc cc24 cccc] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[6] = AWRAP(r[0] + 0x1.3333000000000p-2); QUANT(n); DONE(1, 6); }
        { RESET(); n[7] = AWRAP(r[1] + 0x1.3333000000000p-2); QUANT(n); DONE(2, 7); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 45: mul r6, r6, r3 | mul r7, r7, r3   [1503 6583 75c3] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[6] = AWRAP(r[6] * r[3]); QUANT(n); DONE(1, 6); }
        { RESET(); n[7] = AWRAP(r[7] * r[3]); QUANT(n); DONE(2, 7); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 46: st r6, [0608]@0 | st r7, [0609]@1 | ld r6, [c1f4]@2   [3e07 efe0 ffe2 6fe4 0608 0609 c1f4] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0608, r[6]); }
        { WR(M, 0x0609, r[7]); }
        { n[6] = RD(M, 0xc1f4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 47: mul r2, r5, r4   [0902 2544] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = AWRAP(r[5] * r[4]); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 48: mul r6, r6, #1.30823231@4:24 | mov r7, #0.449236@7:24 | st r2, [0616]@0 | st r2, [0618]@1   [5f0a 65a4 7067 afe0 afe2 0616 0618 34ee 8527 3012] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[6] = AWRAP(r[6] * 0x1.4ee8500000000p+0); QUANT(n); DONE(1, 6); }
        { RESET(); n[7] = 0x1.cc04800000000p-2; QUANT(n); DONE(2, 7); }
        { WR(M, 0x0616, r[2]); }
        { WR(M, 0x0618, r[2]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 49: mac_p1 r4, r0, r6, r7   [0803 440e 0007] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = (r[6] * r[0]) + (r[7]); QUANT(n); DONE(1, 4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 50: recipseed r2, r4   [0a02 af04] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); SFU(0xaf04); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 51: newton r2, r4   [0902 2084] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[2] * r[3]; n[3] = 2 - r[3] * r[4]; n[4] = r[3] * r[4]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 52: newton r2, r4   [0902 2084] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[2] * r[3]; n[3] = 2 - r[3] * r[4]; n[4] = r[3] * r[4]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 53: newton r2, r4   [0902 2084] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[2] * r[3]; n[3] = 2 - r[3] * r[4]; n[4] = r[3] * r[4]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 54: mov r0, r2 | ld r3, [a301]@0   [1804 0042 3fe0 a301] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = r[2]; QUANT(n); DONE(1, 0); }
        { n[3] = RD(M, 0xa301); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 55: addxy r3, r3, #1@0:16 | ld r4, [a300]@1   [1805 3cd0 4fe2 3000 a300] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[3] = AWRAP(r[3] + 0x1.0000000000000p+0); QUANT(n); DONE(1, 3); }
        { n[4] = RD(M, 0xa300); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 56: mul r6, r6, r3 | addxy r7, r7, r4   [1503 6583 7dc4] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[6] = AWRAP(r[6] * r[3]); QUANT(n); DONE(1, 6); }
        { RESET(); n[7] = AWRAP(r[7] + r[4]); QUANT(n); DONE(2, 7); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 57: mac_p1 r4, r1, r6, r7   [0803 444e 0007] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = (r[6] * r[1]) + (r[7]); QUANT(n); DONE(1, 4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 58: recipseed r2, r4   [0a02 af04] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); SFU(0xaf04); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 59: newton r2, r4   [0902 2084] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[2] * r[3]; n[3] = 2 - r[3] * r[4]; n[4] = r[3] * r[4]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 60: newton r2, r4   [0902 2084] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[2] * r[3]; n[3] = 2 - r[3] * r[4]; n[4] = r[3] * r[4]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 61: newton r2, r4   [0902 2084] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[2] * r[3]; n[3] = 2 - r[3] * r[4]; n[4] = r[3] * r[4]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 62: mov r1, r2   [0902 1042] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[1] = r[2]; QUANT(n); DONE(1, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 63: mov r4, r0   [0902 4040] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = r[0]; QUANT(n); DONE(1, 4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 64: recipseed r2, r4   [0a02 af04] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); SFU(0xaf04); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 65: newton r2, r4   [0902 2084] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[2] * r[3]; n[3] = 2 - r[3] * r[4]; n[4] = r[3] * r[4]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 66: newton r2, r4   [0902 2084] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[2] * r[3]; n[3] = 2 - r[3] * r[4]; n[4] = r[3] * r[4]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 67: newton r2, r4   [0902 2084] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[2] * r[3]; n[3] = 2 - r[3] * r[4]; n[4] = r[3] * r[4]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 68: mov r4, r1 | st r2, [0620]@0 | ld r5, [e013]@1   [3406 4041 afe0 5fe2 0620 e013] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = r[1]; QUANT(n); DONE(1, 4); }
        { WR(M, 0x0620, r[2]); }
        { n[5] = RD(M, 0xe013); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 69: recipseed r2, r4   [0a02 af04] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); SFU(0xaf04); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 70: newton r2, r4   [0902 2084] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[2] * r[3]; n[3] = 2 - r[3] * r[4]; n[4] = r[3] * r[4]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 71: newton r2, r4   [0902 2084] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[2] * r[3]; n[3] = 2 - r[3] * r[4]; n[4] = r[3] * r[4]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 72: newton r2, r4   [0902 2084] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[2] * r[3]; n[3] = 2 - r[3] * r[4]; n[4] = r[3] * r[4]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 73: mul r0, r0, r5 | mul r1, r1, r5 | st r2, [0621]@0 | ld r6, [0603]@1   [5f07 0405 1445 afe0 6fe2 0621 0603] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = AWRAP(r[0] * r[5]); QUANT(n); DONE(1, 0); }
        { RESET(); n[1] = AWRAP(r[1] * r[5]); QUANT(n); DONE(2, 1); }
        { WR(M, 0x0621, r[2]); }
        { n[6] = RD(M, 0x0603); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 74: raw 6046 0034 3fe0 7fe2 0623 0604 5fff ffff   [6509 6046 0034 3fe0 7fe2 0623 0604 5fff ffff] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[6] = r[6]; QUANT(n); DONE(1, 6); }
        { RESET(); n[0] = BITOP(0, r[0], 0x1.fffffff000000p+4); QUANT(n); DONE(2, 0); }
        { n[3] = RD(M, 0x0623); }
        { n[7] = RD(M, 0x0604); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 75: addxy r2, r0, r3 | mov r4, #0.0234375@2:24 | mov r1, #31.99999994@5:32 | sum8 r6, r3, #-511.999756@6:32, r7 | ld r3, [0625]@0   [b10b 2c03 4062 1075 eef6 3fe0 0625 0600 005f ffff ff00] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = AWRAP(r[0] + r[3]); QUANT(n); DONE(1, 2); }
        { RESET(); n[4] = 0x1.8000000000000p-6; QUANT(n); DONE(2, 4); }
        { RESET(); n[1] = BITOP(0, r[1], 0x1.fffffff000000p+4); QUANT(n); DONE(3, 1); }
        { RESET(); n[6] = pymod(r[6], 2) / 2; QUANT(n); DONE(4, 6); }
        { n[3] = RD(M, 0x0625); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 76: addxy r5, r1, r3 | mul r7, r7, #0.716599@4:24 | cmp r2, r0, -r4 | setflags 3, 3 | ld r3, [0603]@0 | st r6, [0602]@1   [b44b 5c43 75e4 ae0c b9b3 3fe0 efe2 0603 0602 2b77 3000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = AWRAP(r[1] + r[3]); QUANT(n); DONE(1, 5); }
        { RESET(); n[7] = AWRAP(r[7] * 0x1.6ee6000000000p-1); QUANT(n); DONE(2, 7); }
        { RESET(); RESV(r[2] + -r[4]); QUANT(n); DONE(3, 2); }
        { SETFLAGS(3, 3); }
        { n[3] = RD(M, 0x0603); }
        { WR(M, 0x0602, r[6]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 77: mac_p1p2c2r r6, r6, #0.8582945@6:24, #-0.8582945@1, r3, r7 | cmp r5, r0, -r4 | setflags 3, 2 | ld r3, [a302]@1 | ld r7, [0604]@2   [a60c 6da6 de0c b9b2 3fe2 7fe4 3a47 a302 0604 2dbb 93ad bb93] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[6] = (0x1.b772600000000p-1 * r[6]) + (r[7]) + (-0x1.b772600000000p-1 * r[3]); QUANT(n); DONE(1, 6); }
        { RESET(); RESV(r[5] + -r[4]); QUANT(n); DONE(2, 5); }
        { SETFLAGS(3, 2); }
        { n[3] = RD(M, 0xa302); }
        { n[7] = RD(M, 0x0604); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 78: mul r7, r7, #0.0489201@6:24 | mov r0, r2 | addxy r0, r2, -r4 | cmp r3, r0, #0@0:16 | cond 13, 2, 3 | setflags 3, 4 | st r6, [0603]@1 | ld r3, [0605]@2   [acce 75e6 0042 0c8c be10 1a23 b9b4 efe2 3fe4 0000 0603 0605 0c86 0700] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 1 : 1ull << 2;
        if (!(sk >> 0 & 1)) { RESET(); n[7] = AWRAP(r[7] * 0x1.90c0e00000000p-5); QUANT(n); DONE(1, 7); }
        if (!(sk >> 1 & 1)) { RESET(); n[0] = r[2]; QUANT(n); DONE(2, 0); }
        if (!(sk >> 2 & 1)) { RESET(); n[0] = AWRAP(r[2] + -r[4]); QUANT(n); DONE(3, 0); }
        if (!(sk >> 3 & 1)) { RESET(); RESV(r[3] + 0x0.0p+0); QUANT(n); DONE(4, 3); }
        if (!(sk >> 4 & 1)) { SETFLAGS(3, 4); }
        if (!(sk >> 5 & 1)) { WR(M, 0x0603, r[6]); }
        if (!(sk >> 6 & 1)) { n[3] = RD(M, 0x0605); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 79: mov r1, r5 | addxy r1, r5, -r4 | mul r3, r3, #0.0489201@2:24 | cond 13, 1, 2 | ld r5, [e013]@0   [ae09 1045 1d4c 34e2 1a12 5fe0 e013 0c86 0700] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        if (!(sk >> 0 & 1)) { RESET(); n[1] = r[5]; QUANT(n); DONE(1, 1); }
        if (!(sk >> 1 & 1)) { RESET(); n[1] = AWRAP(r[5] + -r[4]); QUANT(n); DONE(2, 1); }
        if (!(sk >> 2 & 1)) { RESET(); n[3] = AWRAP(r[3] * 0x1.90c0e00000000p-5); QUANT(n); DONE(3, 3); }
        if (!(sk >> 3 & 1)) { n[5] = RD(M, 0xe013); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 80: mov r2, #0.0944827@0:24 | mov r0, r5 | bic r1, r1, r5 | cond 12, 2, 0   [b208 2060 0045 9e45 1820 1830 04cc cc00] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 1 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[2] = 0x1.8300400000000p-4; QUANT(n); DONE(1, 2); }
        if (!(sk >> 1 & 1)) { RESET(); n[0] = r[5]; QUANT(n); DONE(2, 0); }
        if (!(sk >> 2 & 1)) { RESET(); n[1] = r[5]; QUANT(n); DONE(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 81: mac_p1p2c2r r5, r6, #0.0489201@6:24, #0.0978402@1, r7, r3 | ld r6, [0606]@1 | ld r7, [0607]@2   [2b0a 5da6 6fe2 7fe4 7a43 0606 0607 0c86 0719 0c0e] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = (0x1.90c0e00000000p-5 * r[6]) + (r[3]) + (0x1.90c0e00000000p-4 * r[7]); QUANT(n); DONE(1, 5); }
        { n[6] = RD(M, 0x0606); }
        { n[7] = RD(M, 0x0607); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 82: mac_p1p2c2 r5, r6, #1.4190722@2:24, #-0.614753@5, r7, r5   [0806 5da2 7945 36b4 85a9 d607] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = (0x1.6b48500000000p+0 * r[6]) + (r[5]) + (-0x1.3ac0e00000000p-1 * r[7]); QUANT(n); DONE(1, 5); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 83: st r5, [0605]@0   [0c03 dfe0 0605] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0605, r[5]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 84: addxy r2, r2, r0 | addxy r5, r2, r1 | hset h5, #1@4:24 | st r0, [0622]@0 | st r1, [0624]@1   [b70a 2c80 5c81 4064 8fe0 9fe2 0622 0624 0040 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 0) & 1) != 0 ? 0 : 1ull << 4;
        if (!(sk >> 0 & 1)) { RESET(); n[2] = AWRAP(r[2] + r[0]); QUANT(n); DONE(1, 2); }
        if (!(sk >> 1 & 1)) { RESET(); n[5] = AWRAP(r[2] + r[1]); QUANT(n); DONE(2, 5); }
        if (!(sk >> 2 & 1)) { M->H[5] = 0x1.0000000000000p+0; }
        if (!(sk >> 3 & 1)) { WR(M, 0x0622, r[0]); }
        if (!(sk >> 4 & 1)) { WR(M, 0x0624, r[1]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 85: cmp r2, r0, -r4 | setflags 3, 1   [1b03 ae0c b9b1] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[2] + -r[4]); QUANT(n); DONE(1, 2); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 86: cmp r5, r0, -r4 | setflags 3, 1   [1b03 de0c b9b1] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[5] + -r[4]); QUANT(n); DONE(1, 5); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 87: mov r0, r2 | addxy r0, r2, -r4 | cond 13, 1, 2 | hset h4, #0@0:24   [5d07 0042 0c8c 1a12 4020 0000 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        sk |= ((M->F >> 0) & 1) != 0 ? 1ull << 1 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[0] = r[2]; QUANT(n); DONE(1, 0); }
        if (!(sk >> 1 & 1)) { RESET(); n[0] = AWRAP(r[2] + -r[4]); QUANT(n); DONE(2, 0); }
        if (!(sk >> 2 & 1)) { M->H[4] = 0x0.0p+0; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 88: mov r1, r5 | addxy r1, r5, -r4 | f6 r0, -r4   [5b05 1045 1d4c 8f6c 1a12] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[1] = r[5]; QUANT(n); DONE(1, 1); }
        { RESET(); n[1] = AWRAP(r[5] + -r[4]); QUANT(n); DONE(2, 1); }
        { RESET(); n[0] = pymod(r[0] + M->H[4], 512); QUANT(n); DONE(3, 0); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 89: f4 r3, -r0 | f6 r1, -r5 | ld r2, [0627]@0 | st r0, [062f]@1   [6f07 bf48 9f6d 2fe0 8fe2 0627 062f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[3] = r[0]; QUANT(n); DONE(1, 3); }
        { RESET(); n[1] = pymod(r[1] + M->H[5], 512); QUANT(n); DONE(2, 1); }
        { n[2] = RD(M, 0x0627); }
        { WR(M, 0x062f, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 90: addxy r0, r3, -r2 | cmp r3, r0, -r2 | f4 r4, -r1 | setflags 3, 2 | st r1, [0639]@0 | st r3, [0626]@1   [be49 0cca be0a cf49 b9b2 9fe0 bfe2 0639 0626] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = AWRAP(r[3] + -r[2]); QUANT(n); DONE(1, 0); }
        { RESET(); RESV(r[3] + -r[2]); QUANT(n); DONE(2, 3); }
        { RESET(); n[4] = r[1]; QUANT(n); DONE(3, 4); }
        { SETFLAGS(3, 2); }
        { WR(M, 0x0639, r[1]); }
        { WR(M, 0x0626, r[3]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 91: st r0, [0633]@0 | st r4, [0628]@1   [1e05 8fe0 cfe2 0633 0628] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0633, r[0]); }
        { WR(M, 0x0628, r[4]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 92: jmp .+297@0 | cond 12, 1, 0   [1c04 0090 1810 0129] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { jto = 146; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 93: ld r4, [062f]@0 | ld r5, [0620]@1   [1e05 4fe0 5fe2 062f 0620] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[4] = RD(M, 0x062f); }
        { n[5] = RD(M, 0x0620); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 94: f5 r4, r4   [0a02 cf54] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = 0.0; QUANT(n); DONE(1, 4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 95: mul r4, r4, r5 | ld r0, [0616]@0   [1804 4505 0fe0 0616] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = AWRAP(r[4] * r[5]); QUANT(n); DONE(1, 4); }
        { n[0] = RD(M, 0x0616); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 96: subxy r4, r4, #1@0:16 | ld r1, [0617]@1 | ld r2, [0633]@2   [3407 4f10 1fe2 2fe4 3000 0617 0633] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = AWRAP(0x1.0000000000000p+0 - r[4]); QUANT(n); DONE(1, 4); }
        { n[1] = RD(M, 0x0617); }
        { n[2] = RD(M, 0x0633); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 97: subxy r5, r4, #1@0:16 | cmp r2, r0, #0@2:16 | setflags 3, 2   [3106 5f10 ae12 b9b2 3000 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = AWRAP(0x1.0000000000000p+0 - r[4]); QUANT(n); DONE(1, 5); }
        { RESET(); RESV(r[2] + 0x0.0p+0); QUANT(n); DONE(2, 2); }
        { SETFLAGS(3, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 98: mac_p1w r0, r0, r4, r1, r5 | mov r1, #0.0234375@4:24 | ld r4, [e013]@1   [2508 080c 1064 4fe2 1340 e013 0600 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = (r[4] * r[0]) + (r[5] * r[1]); QUANT(n); DONE(1, 0); }
        { RESET(); n[1] = 0x1.8000000000000p-6; QUANT(n); DONE(2, 1); }
        { n[4] = RD(M, 0xe013); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 99: mul r5, r4, #4@0:16 | addxy r0, r1, r2 | mov r0, r2 | cond 13, 2, 3 | st r0, [062a]@1   [ae08 5510 0c42 0042 1a23 8fe2 4400 062a] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 1 : 1ull << 2;
        if (!(sk >> 0 & 1)) { RESET(); n[5] = AWRAP(r[4] * 0x1.0000000000000p+2); QUANT(n); DONE(1, 5); }
        if (!(sk >> 1 & 1)) { RESET(); n[0] = AWRAP(r[1] + r[2]); QUANT(n); DONE(2, 0); }
        if (!(sk >> 2 & 1)) { RESET(); n[0] = r[2]; QUANT(n); DONE(3, 0); }
        if (!(sk >> 3 & 1)) { WR(M, 0x062a, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 100: mul r6, r4, #3@0:16 | cmp r5, r0, -r0 | setflags 3, 2 | st r0, [0633]@1   [6507 6510 de08 b9b2 8fe2 4300 0633] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[6] = AWRAP(r[4] * 0x1.8000000000000p+1); QUANT(n); DONE(1, 6); }
        { RESET(); RESV(r[5] + -r[0]); QUANT(n); DONE(2, 5); }
        { SETFLAGS(3, 2); }
        { WR(M, 0x0633, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 101: mul r7, r4, #2@0:16 | cmp r6, r0, -r0 | setflags 3, 2   [3105 7510 ee08 b9b2 4200] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = AWRAP(r[4] * 0x1.0000000000000p+1); QUANT(n); DONE(1, 7); }
        { RESET(); RESV(r[6] + -r[0]); QUANT(n); DONE(2, 6); }
        { SETFLAGS(3, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 102: cmp r7, r0, -r0 | jmp .+128@0 | cond 12, 2, 0 | setflags 3, 1   [7106 fe08 0090 1820 b9b1 0080] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 1 : 0;
        if (!(sk >> 0 & 1)) { RESET(); RESV(r[7] + -r[0]); QUANT(n); DONE(1, 7); }
        if (!(sk >> 1 & 1)) { jto = 127; }
        if (!(sk >> 2 & 1)) { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 103: jmp .+56@0 | cond 12, 1, 0   [1c04 0090 1810 0038] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { jto = 116; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 104: raw 0090 0092 1812 0001 00cd   [3b06 0090 0092 1812 0001 00cd] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        if (!(sk >> 0 & 1)) { jto = 106; }
        if (!(sk >> 1 & 1)) { jto = 140; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 105: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
L106:;
    /* 106: ld r0, [062a]@0 | ld r1, [062b]@1   [1e05 0fe0 1fe2 062a 062b] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[0] = RD(M, 0x062a); }
        { n[1] = RD(M, 0x062b); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 107: mac_p1wc1 r0, r0, #0.5@2:16, #0.5@4, r1 | mov r2, #6.104e-05@8:24 | ld r3, [062f]@3   [250a 0812 2068 3fe6 1500 2800 2800 062f 0004 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = (0x1.0000000000000p-1 * r[0]) + (0x1.0000000000000p-1 * r[1]); QUANT(n); DONE(1, 0); }
        { RESET(); n[2] = 0x1.0000000000000p-14; QUANT(n); DONE(2, 2); }
        { n[3] = RD(M, 0x062f); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 108: f6 r2, -r4 | st r0, [062c]@0 | st r0, [062d]@1   [3a06 af6c 8fe0 8fe2 062c 062d] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = pymod(r[2] + M->H[4], 512); QUANT(n); DONE(1, 2); }
        { WR(M, 0x062c, r[0]); }
        { WR(M, 0x062d, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 109: cmp r3, r0, -r2 | setflags 3, 1   [1b03 be0a b9b1] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[3] + -r[2]); QUANT(n); DONE(1, 3); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 110: mov r7, #0.0234375@2:24 | mov r2, #6.104e-05@5:24 | st r0, [062e]@0   [2e08 7062 2065 8fe0 062e 0600 0000 0400] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = 0x1.8000000000000p-6; QUANT(n); DONE(1, 7); }
        { RESET(); n[2] = 0x1.0000000000000p-14; QUANT(n); DONE(2, 2); }
        { WR(M, 0x062e, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 111: raw 0cc0 0cca 1a12 a007   [2405 0cc0 0cca 1a12 a007] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = (r[0] * r[3]) + (-0x1.c000000000000p-10) + (0x0.0p+0 * r[1]); QUANT(n); DONE(1, 0); }
        { RESET(); n[0] = AWRAP(r[3] + -r[2]); QUANT(n); DONE(2, 0); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 112: st r0, [0630]@0 | st r0, [0631]@1   [1e05 8fe0 8fe2 0630 0631] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0630, r[0]); }
        { WR(M, 0x0631, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 113: st r0, [0632]@0   [0c03 8fe0 0632] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0632, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 114: jmp .+203@0   [0b03 0090 00cb] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { jto = 151; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 115: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
L116:;
    /* 116: ld r0, [062a]@0 | ld r1, [062b]@1   [1e05 0fe0 1fe2 062a 062b] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[0] = RD(M, 0x062a); }
        { n[1] = RD(M, 0x062b); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 117: mac_p1wc1 r4, r0, #0.67@2:16, #0.33@4, r1 | mov r2, #6.104e-05@8:24 | ld r3, [062f]@3   [250a 4812 2068 3fe6 1500 2ab8 2547 062f 0004 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = (0x1.5700000000000p-1 * r[0]) + (0x1.51c0000000000p-2 * r[1]); QUANT(n); DONE(1, 4); }
        { RESET(); n[2] = 0x1.0000000000000p-14; QUANT(n); DONE(2, 2); }
        { n[3] = RD(M, 0x062f); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 118: mac_p1wc1 r5, r0, #0.33@2:16, #0.67@4, r1 | mov r6, #0.0001221@8:24 | f6 r2, -r4 | st r4, [062c]@3   [4b0b 5812 6068 af6c cfe6 1500 2547 2ab8 062c 0008 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = (0x1.51c0000000000p-2 * r[0]) + (0x1.5700000000000p-1 * r[1]); QUANT(n); DONE(1, 5); }
        { RESET(); n[6] = 0x1.0000000000000p-13; QUANT(n); DONE(2, 6); }
        { RESET(); n[2] = pymod(r[2] + M->H[4], 512); QUANT(n); DONE(3, 2); }
        { WR(M, 0x062c, r[4]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 119: mov r7, #0.0234375@4:24 | f6 r6, -r4 | st r5, [062d]@0 | st r5, [062e]@1   [6509 7064 ef6c dfe0 dfe2 062d 062e 0600 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = 0x1.8000000000000p-6; QUANT(n); DONE(1, 7); }
        { RESET(); n[6] = pymod(r[6] + M->H[4], 512); QUANT(n); DONE(2, 6); }
        { WR(M, 0x062d, r[5]); }
        { WR(M, 0x062e, r[5]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 120: cmp r3, r0, -r2 | setflags 3, 1   [1b03 be0a b9b1] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[3] + -r[2]); QUANT(n); DONE(1, 3); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 121: mov r2, #6.104e-05@0:24 | cmp r3, r0, -r6 | setflags 3, 2   [3106 2060 be0e b9b2 0004 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = 0x1.0000000000000p-14; QUANT(n); DONE(1, 2); }
        { RESET(); RESV(r[3] + -r[6]); QUANT(n); DONE(2, 3); }
        { SETFLAGS(3, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 122: mac_p1p2c0 r0, r3, -r2, r7 | addxy r0, r3, -r2 | mov r6, #0.0001221@2:24 | cond 13, 1, 2   [4808 0cc0 0cca 6062 1a12 a007 0008 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        if (!(sk >> 0 & 1)) { RESET(); n[0] = (r[3]) + (r[7]) + (-r[2]); QUANT(n); DONE(1, 0); }
        if (!(sk >> 1 & 1)) { RESET(); n[0] = AWRAP(r[3] + -r[2]); QUANT(n); DONE(2, 0); }
        if (!(sk >> 2 & 1)) { RESET(); n[6] = 0x1.0000000000000p-13; QUANT(n); DONE(3, 6); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 123: mac_p1p2c0 r4, r3, -r6, r7 | addxy r4, r3, -r6 | cond 13, 1, 2 | st r0, [0630]@1   [4d07 4cc0 4cce 1a12 8fe2 e007 0630] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        if (!(sk >> 0 & 1)) { RESET(); n[4] = (r[3]) + (r[7]) + (-r[6]); QUANT(n); DONE(1, 4); }
        if (!(sk >> 1 & 1)) { RESET(); n[4] = AWRAP(r[3] + -r[6]); QUANT(n); DONE(2, 4); }
        if (!(sk >> 2 & 1)) { WR(M, 0x0630, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 124: st r4, [0631]@0 | st r4, [0632]@1   [1e05 cfe0 cfe2 0631 0632] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0631, r[4]); }
        { WR(M, 0x0632, r[4]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 125: jmp .+135@0   [0b03 0090 0087] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { jto = 151; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 126: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
L127:;
    /* 127: ld r0, [062a]@0 | ld r1, [062b]@1   [1e05 0fe0 1fe2 062a 062b] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[0] = RD(M, 0x062a); }
        { n[1] = RD(M, 0x062b); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 128: mac_p1wc1 r4, r0, #0.75@2:16, #0.25@4, r1 | mov r2, #6.104e-05@8:24 | ld r3, [062f]@3   [250a 4812 2068 3fe6 1500 2c00 2400 062f 0004 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = (0x1.8000000000000p-1 * r[0]) + (0x1.0000000000000p-2 * r[1]); QUANT(n); DONE(1, 4); }
        { RESET(); n[2] = 0x1.0000000000000p-14; QUANT(n); DONE(2, 2); }
        { n[3] = RD(M, 0x062f); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 129: mac_p1wc1 r5, r0, #0.5@2:16, #0.5@4, r1 | mov r6, #0.0001221@8:24 | f6 r2, -r4 | st r4, [062c]@3   [4b0b 5812 6068 af6c cfe6 1500 2800 2800 062c 0008 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = (0x1.0000000000000p-1 * r[0]) + (0x1.0000000000000p-1 * r[1]); QUANT(n); DONE(1, 5); }
        { RESET(); n[6] = 0x1.0000000000000p-13; QUANT(n); DONE(2, 6); }
        { RESET(); n[2] = pymod(r[2] + M->H[4], 512); QUANT(n); DONE(3, 2); }
        { WR(M, 0x062c, r[4]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 130: mac_p1wc1 r4, r0, #0.25@2:16, #0.75@4, r1 | mov r7, #0.00018311@8:24 | f6 r6, -r4 | st r5, [062d]@3   [4b0b 4812 7068 ef6c dfe6 1500 2400 2c00 062d 000c 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = (0x1.0000000000000p-2 * r[0]) + (0x1.8000000000000p-1 * r[1]); QUANT(n); DONE(1, 4); }
        { RESET(); n[7] = 0x1.8000000000000p-13; QUANT(n); DONE(2, 7); }
        { RESET(); n[6] = pymod(r[6] + M->H[4], 512); QUANT(n); DONE(3, 6); }
        { WR(M, 0x062d, r[5]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 131: mov r5, #0.0234375@2:24 | f6 r7, -r4 | st r4, [062e]@0   [3107 5062 ff6c cfe0 062e 0600 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = 0x1.8000000000000p-6; QUANT(n); DONE(1, 5); }
        { RESET(); n[7] = pymod(r[7] + M->H[4], 512); QUANT(n); DONE(2, 7); }
        { WR(M, 0x062e, r[4]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 132: cmp r3, r0, -r2 | setflags 3, 1   [1b03 be0a b9b1] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[3] + -r[2]); QUANT(n); DONE(1, 3); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 133: mov r2, #6.104e-05@0:24 | cmp r3, r0, -r6 | setflags 3, 2   [3106 2060 be0e b9b2 0004 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = 0x1.0000000000000p-14; QUANT(n); DONE(1, 2); }
        { RESET(); RESV(r[3] + -r[6]); QUANT(n); DONE(2, 3); }
        { SETFLAGS(3, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 134: mac_p1p2c0 r0, r3, -r2, r5 | addxy r0, r3, -r2 | mov r6, #0.0001221@2:24 | sum8 r3, r0, -r7, r7 | cond 13, 1, 2 | setflags 3, 4   [904a 0cc0 0cca 6062 be0f 1a12 b9b4 a005 0008 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        if (!(sk >> 0 & 1)) { RESET(); n[0] = (r[3]) + (r[5]) + (-r[2]); QUANT(n); DONE(1, 0); }
        if (!(sk >> 1 & 1)) { RESET(); n[0] = AWRAP(r[3] + -r[2]); QUANT(n); DONE(2, 0); }
        if (!(sk >> 2 & 1)) { RESET(); n[6] = 0x1.0000000000000p-13; QUANT(n); DONE(3, 6); }
        if (!(sk >> 3 & 1)) { RESET(); RESV(r[3] + -r[7]); QUANT(n); DONE(4, 3); }
        if (!(sk >> 4 & 1)) { SETFLAGS(3, 4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 135: mac_p1p2c0 r4, r3, -r6, r5 | addxy r4, r3, -r6 | mov r7, #0.00018311@4:24 | cond 13, 1, 2 | st r0, [0630]@1   [920a 4cc0 4cce 7064 1a12 8fe2 e005 0630 000c 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        if (!(sk >> 0 & 1)) { RESET(); n[4] = (r[3]) + (r[5]) + (-r[6]); QUANT(n); DONE(1, 4); }
        if (!(sk >> 1 & 1)) { RESET(); n[4] = AWRAP(r[3] + -r[6]); QUANT(n); DONE(2, 4); }
        if (!(sk >> 2 & 1)) { RESET(); n[7] = 0x1.8000000000000p-13; QUANT(n); DONE(3, 7); }
        if (!(sk >> 3 & 1)) { WR(M, 0x0630, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 136: mac_p1p2c0 r2, r3, -r7, r5 | addxy r2, r3, -r7 | cond 13, 1, 2 | st r4, [0631]@1   [4d07 2cc0 2ccf 1a12 cfe2 f005 0631] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        if (!(sk >> 0 & 1)) { RESET(); n[2] = (r[3]) + (r[5]) + (-r[7]); QUANT(n); DONE(1, 2); }
        if (!(sk >> 1 & 1)) { RESET(); n[2] = AWRAP(r[3] + -r[7]); QUANT(n); DONE(2, 2); }
        if (!(sk >> 2 & 1)) { WR(M, 0x0631, r[4]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 137: st r2, [0632]@0   [0c03 afe0 0632] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0632, r[2]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 138: jmp .+48@0   [0b03 0090 0030] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { jto = 151; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 139: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
L140:;
    /* 140: ld r1, [062a]@0 | ld r2, [062f]@1   [1e05 1fe0 2fe2 062a 062f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[1] = RD(M, 0x062a); }
        { n[2] = RD(M, 0x062f); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 141: st r1, [062c]@0 | st r2, [0630]@1   [1e05 9fe0 afe2 062c 0630] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x062c, r[1]); }
        { WR(M, 0x0630, r[2]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 142: st r1, [062d]@0 | st r2, [0631]@1   [1e05 9fe0 afe2 062d 0631] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x062d, r[1]); }
        { WR(M, 0x0631, r[2]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 143: st r1, [062e]@0 | st r2, [0632]@1   [1e05 9fe0 afe2 062e 0632] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x062e, r[1]); }
        { WR(M, 0x0632, r[2]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 144: jmp .+24@0   [0b03 0090 0018] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { jto = 151; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 145: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
L146:;
    /* 146: ld r0, [062b]@0   [0c03 0fe0 062b] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[0] = RD(M, 0x062b); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 147: st r0, [062a]@0 | st r0, [062c]@1   [1e05 8fe0 8fe2 062a 062c] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x062a, r[0]); }
        { WR(M, 0x062c, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 148: ld r1, [062f]@0 | st r0, [062d]@1 | st r0, [062e]@2   [3e07 1fe0 8fe2 8fe4 062f 062d 062e] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[1] = RD(M, 0x062f); }
        { WR(M, 0x062d, r[0]); }
        { WR(M, 0x062e, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 149: st r1, [0630]@0 | st r1, [0631]@1   [1e05 9fe0 9fe2 0630 0631] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0630, r[1]); }
        { WR(M, 0x0631, r[1]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 150: st r1, [0632]@0   [0c03 9fe0 0632] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0632, r[1]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
L151:;
    /* 151: mov r4, #0.0234375@2:24 | ld r1, [062f]@0   [1806 4062 1fe0 062f 0600 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = 0x1.8000000000000p-6; QUANT(n); DONE(1, 4); }
        { n[1] = RD(M, 0x062f); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 152: mov r6, #0@4:32 | f4 r1, -r1 | f6 r4, -r4 | ld r0, [062a]@0 | ld r3, [0630]@1   [be0a 6074 9f49 cf6c 0fe0 3fe2 062a 0630 0000 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[6] = 0x0.0p+0; QUANT(n); DONE(1, 6); }
        { RESET(); n[1] = r[1]; QUANT(n); DONE(2, 1); }
        { RESET(); n[4] = pymod(r[4] + M->H[4], 512); QUANT(n); DONE(3, 4); }
        { n[0] = RD(M, 0x062a); }
        { n[3] = RD(M, 0x0630); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 153: mov r2, r1 | f6 r4, -r7   [1603 2041 cf6f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[1]; QUANT(n); DONE(1, 2); }
        { RESET(); n[4] = pymod(r[4] + M->H[7], 512); QUANT(n); DONE(2, 4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 154: f6 r2, -r7   [0a02 af6f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = pymod(r[2] + M->H[7], 512); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 155: cmp r2, r0, #-8@0:32 | setflags 3, 1   [1b05 ae30 b9b1 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 156: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 157: addxy r2, r2, #-8@0:32 | cond !13, 1, 0   [1705 2cb0 1b10 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 1 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[2] = AWRAP(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 158: f4 r3, -r3 | ld r0, [062c]@0 | ld r5, [0631]@1 | setflags 0, 10   [7307 bf4b 0fe0 5fe2 89ba 062c 0631] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[3] = r[3]; QUANT(n); DONE(1, 3); }
        { n[0] = RD(M, 0x062c); }
        { n[5] = RD(M, 0x0631); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 159: mov r2, r3   [0902 2043] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[3]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 160: f6 r2, -r7   [0a02 af6f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = pymod(r[2] + M->H[7], 512); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 161: cmp r2, r0, #-8@0:32 | setflags 3, 1   [1b05 ae30 b9b1 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 162: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 163: addxy r2, r2, #-8@0:32 | cond !13, 1, 0   [1705 2cb0 1b10 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 1 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[2] = AWRAP(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 164: f4 r5, -r5 | ld r0, [062d]@0 | ld r7, [0632]@1 | setflags 0, 10   [7307 df4d 0fe0 7fe2 89ba 062d 0632] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = r[5]; QUANT(n); DONE(1, 5); }
        { n[0] = RD(M, 0x062d); }
        { n[7] = RD(M, 0x0632); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 165: mov r2, r5   [0902 2045] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[5]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 166: f6 r2, -r7   [0a02 af6f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = pymod(r[2] + M->H[7], 512); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 167: cmp r2, r0, #-8@0:32 | setflags 3, 1   [1b05 ae30 b9b1 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 168: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 169: addxy r2, r2, #-8@0:32 | cond !13, 1, 0   [1705 2cb0 1b10 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 1 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[2] = AWRAP(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 170: f4 r7, -r7 | ld r0, [062e]@0 | setflags 0, 10   [3a05 ff4f 0fe0 89ba 062e] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = r[7]; QUANT(n); DONE(1, 7); }
        { n[0] = RD(M, 0x062e); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 171: mov r2, r7   [0902 2047] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[7]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 172: f6 r2, -r7   [0a02 af6f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = pymod(r[2] + M->H[7], 512); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 173: cmp r2, r0, #-8@0:32 | setflags 3, 1   [1b05 ae30 b9b1 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 174: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 175: addxy r2, r2, #-8@0:32 | cond !13, 1, 0   [1705 2cb0 1b10 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 1 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[2] = AWRAP(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 176: cmp r4, r0, #-8@0:32 | setflags 3, 1 | setflags 0, 10   [3a06 ce30 b9b1 89ba c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[4] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 4); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 177: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 178: addxy r4, r4, #-8@0:32 | cond !13, 1, 0   [1705 4d30 1b10 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 1 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[4] = AWRAP(r[4] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 179: cmp r1, r0, -r6 | setflags 3, 1   [1b03 9e0e b9b1] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[1] + -r[6]); QUANT(n); DONE(1, 1); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 180: cmp r3, r0, -r6 | setflags 3, 1 | ld r1, [062a]@0   [3a05 be0e b9b1 1fe0 062a] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[3] + -r[6]); QUANT(n); DONE(1, 3); }
        { SETFLAGS(3, 1); }
        { n[1] = RD(M, 0x062a); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 181: sum8 r5, r0, -r6, r7 | cond 12, 3, 0 | setflags 3, 1 | setflags 1, 12 | ld r1, [062c]@0   [dd07 de0e 1830 b9b1 99bc 1fe0 062c] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 2 : 0;
        if (!(sk >> 0 & 1)) { RESET(); RESV(r[5] + -r[6]); QUANT(n); DONE(1, 5); }
        if (!(sk >> 1 & 1)) { SETFLAGS(3, 1); }
        if (!(sk >> 2 & 1)) { SETFLAGS(1, 12); }
        if (!(sk >> 3 & 1)) { n[1] = RD(M, 0x062c); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 182: sum8 r7, r0, -r6, r7 | cond 12, 3, 0 | setflags 3, 1 | setflags 1, 12 | ld r1, [062d]@0   [dd07 fe0e 1830 b9b1 99bc 1fe0 062d] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 2 : 0;
        if (!(sk >> 0 & 1)) { RESET(); RESV(r[7] + -r[6]); QUANT(n); DONE(1, 7); }
        if (!(sk >> 1 & 1)) { SETFLAGS(3, 1); }
        if (!(sk >> 2 & 1)) { SETFLAGS(1, 12); }
        if (!(sk >> 3 & 1)) { n[1] = RD(M, 0x062d); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 183: cond 12, 1, 0 | setflags 1, 12 | ld r1, [062e]@0   [3d05 1810 99bc 1fe0 062e] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { SETFLAGS(1, 12); }
        if (!(sk >> 1 & 1)) { n[1] = RD(M, 0x062e); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 184: cond 12, 1, 0 | setflags 1, 12   [1d03 1810 99bc] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { SETFLAGS(1, 12); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 185: ld r2, [0629]@0 | ld r3, [0628]@1   [1e05 2fe0 3fe2 0629 0628] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[2] = RD(M, 0x0629); }
        { n[3] = RD(M, 0x0628); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 186: addxy r0, r3, -r2 | cmp r3, r0, -r2 | setflags 3, 2   [3104 0cca be0a b9b2] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = AWRAP(r[3] + -r[2]); QUANT(n); DONE(1, 0); }
        { RESET(); RESV(r[3] + -r[2]); QUANT(n); DONE(2, 3); }
        { SETFLAGS(3, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 187: hset h4, #1@2:24 | st r0, [063d]@0   [1d06 4022 8fe0 063d 0040 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 0) & 1) != 0 ? 0 : 0;
        if (!(sk >> 0 & 1)) { M->H[4] = 0x1.0000000000000p+0; }
        if (!(sk >> 1 & 1)) { WR(M, 0x063d, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 188: jmp .+297@0 | cond 12, 1, 0   [1c04 0090 1810 0129] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { jto = 242; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 189: ld r4, [0639]@0 | ld r5, [0621]@1   [1e05 4fe0 5fe2 0639 0621] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[4] = RD(M, 0x0639); }
        { n[5] = RD(M, 0x0621); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 190: f5 r4, r4   [0a02 cf54] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = 0.0; QUANT(n); DONE(1, 4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 191: mul r4, r4, r5 | ld r0, [0618]@0   [1804 4505 0fe0 0618] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = AWRAP(r[4] * r[5]); QUANT(n); DONE(1, 4); }
        { n[0] = RD(M, 0x0618); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 192: subxy r4, r4, #1@0:16 | ld r1, [0619]@1 | ld r2, [063d]@2   [3407 4f10 1fe2 2fe4 3000 0619 063d] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = AWRAP(0x1.0000000000000p+0 - r[4]); QUANT(n); DONE(1, 4); }
        { n[1] = RD(M, 0x0619); }
        { n[2] = RD(M, 0x063d); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 193: subxy r5, r4, #1@0:16 | cmp r2, r0, #0@2:16 | setflags 3, 2   [3106 5f10 ae12 b9b2 3000 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = AWRAP(0x1.0000000000000p+0 - r[4]); QUANT(n); DONE(1, 5); }
        { RESET(); RESV(r[2] + 0x0.0p+0); QUANT(n); DONE(2, 2); }
        { SETFLAGS(3, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 194: mac_p1w r0, r0, r4, r1, r5 | mov r1, #0.0234375@4:24 | ld r4, [e013]@1   [2508 080c 1064 4fe2 1340 e013 0600 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = (r[4] * r[0]) + (r[5] * r[1]); QUANT(n); DONE(1, 0); }
        { RESET(); n[1] = 0x1.8000000000000p-6; QUANT(n); DONE(2, 1); }
        { n[4] = RD(M, 0xe013); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 195: mul r5, r4, #4@0:16 | addxy r0, r1, r2 | mov r0, r2 | cond 13, 2, 3 | st r0, [0634]@1   [ae08 5510 0c42 0042 1a23 8fe2 4400 0634] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 1 : 1ull << 2;
        if (!(sk >> 0 & 1)) { RESET(); n[5] = AWRAP(r[4] * 0x1.0000000000000p+2); QUANT(n); DONE(1, 5); }
        if (!(sk >> 1 & 1)) { RESET(); n[0] = AWRAP(r[1] + r[2]); QUANT(n); DONE(2, 0); }
        if (!(sk >> 2 & 1)) { RESET(); n[0] = r[2]; QUANT(n); DONE(3, 0); }
        if (!(sk >> 3 & 1)) { WR(M, 0x0634, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 196: mul r6, r4, #3@0:16 | cmp r5, r0, -r0 | setflags 3, 2 | st r0, [063d]@1   [6507 6510 de08 b9b2 8fe2 4300 063d] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[6] = AWRAP(r[4] * 0x1.8000000000000p+1); QUANT(n); DONE(1, 6); }
        { RESET(); RESV(r[5] + -r[0]); QUANT(n); DONE(2, 5); }
        { SETFLAGS(3, 2); }
        { WR(M, 0x063d, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 197: mul r7, r4, #2@0:16 | cmp r6, r0, -r0 | setflags 3, 2   [3105 7510 ee08 b9b2 4200] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = AWRAP(r[4] * 0x1.0000000000000p+1); QUANT(n); DONE(1, 7); }
        { RESET(); RESV(r[6] + -r[0]); QUANT(n); DONE(2, 6); }
        { SETFLAGS(3, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 198: cmp r7, r0, -r0 | jmp .+128@0 | cond 12, 2, 0 | setflags 3, 1   [7106 fe08 0090 1820 b9b1 0080] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 1 : 0;
        if (!(sk >> 0 & 1)) { RESET(); RESV(r[7] + -r[0]); QUANT(n); DONE(1, 7); }
        if (!(sk >> 1 & 1)) { jto = 223; }
        if (!(sk >> 2 & 1)) { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 199: jmp .+56@0 | cond 12, 1, 0   [1c04 0090 1810 0038] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { jto = 212; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 200: raw 0090 0092 1812 0001 00cd   [3b06 0090 0092 1812 0001 00cd] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        if (!(sk >> 0 & 1)) { jto = 202; }
        if (!(sk >> 1 & 1)) { jto = 236; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 201: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
L202:;
    /* 202: ld r0, [0634]@0 | ld r1, [0635]@1   [1e05 0fe0 1fe2 0634 0635] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[0] = RD(M, 0x0634); }
        { n[1] = RD(M, 0x0635); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 203: mac_p1wc1 r0, r0, #0.5@2:16, #0.5@4, r1 | mov r2, #6.104e-05@8:24 | ld r3, [0639]@3   [250a 0812 2068 3fe6 1500 2800 2800 0639 0004 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = (0x1.0000000000000p-1 * r[0]) + (0x1.0000000000000p-1 * r[1]); QUANT(n); DONE(1, 0); }
        { RESET(); n[2] = 0x1.0000000000000p-14; QUANT(n); DONE(2, 2); }
        { n[3] = RD(M, 0x0639); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 204: f6 r2, -r4 | st r0, [0636]@0 | st r0, [0637]@1   [3a06 af6c 8fe0 8fe2 0636 0637] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = pymod(r[2] + M->H[4], 512); QUANT(n); DONE(1, 2); }
        { WR(M, 0x0636, r[0]); }
        { WR(M, 0x0637, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 205: cmp r3, r0, -r2 | setflags 3, 1   [1b03 be0a b9b1] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[3] + -r[2]); QUANT(n); DONE(1, 3); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 206: mov r7, #0.0234375@2:24 | mov r2, #6.104e-05@5:24 | st r0, [0638]@0   [2e08 7062 2065 8fe0 0638 0600 0000 0400] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = 0x1.8000000000000p-6; QUANT(n); DONE(1, 7); }
        { RESET(); n[2] = 0x1.0000000000000p-14; QUANT(n); DONE(2, 2); }
        { WR(M, 0x0638, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 207: raw 0cc0 0cca 1a12 a007   [2405 0cc0 0cca 1a12 a007] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[0] = (r[0] * r[3]) + (-0x1.c000000000000p-10) + (0x0.0p+0 * r[1]); QUANT(n); DONE(1, 0); }
        { RESET(); n[0] = AWRAP(r[3] + -r[2]); QUANT(n); DONE(2, 0); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 208: st r0, [063a]@0 | st r0, [063b]@1   [1e05 8fe0 8fe2 063a 063b] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x063a, r[0]); }
        { WR(M, 0x063b, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 209: st r0, [063c]@0   [0c03 8fe0 063c] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x063c, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 210: jmp .+203@0   [0b03 0090 00cb] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { jto = 247; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 211: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
L212:;
    /* 212: ld r0, [0634]@0 | ld r1, [0635]@1   [1e05 0fe0 1fe2 0634 0635] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[0] = RD(M, 0x0634); }
        { n[1] = RD(M, 0x0635); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 213: mac_p1wc1 r4, r0, #0.67@2:16, #0.33@4, r1 | mov r2, #6.104e-05@8:24 | ld r3, [0639]@3   [250a 4812 2068 3fe6 1500 2ab8 2547 0639 0004 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = (0x1.5700000000000p-1 * r[0]) + (0x1.51c0000000000p-2 * r[1]); QUANT(n); DONE(1, 4); }
        { RESET(); n[2] = 0x1.0000000000000p-14; QUANT(n); DONE(2, 2); }
        { n[3] = RD(M, 0x0639); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 214: mac_p1wc1 r5, r0, #0.33@2:16, #0.67@4, r1 | mov r6, #0.0001221@8:24 | f6 r2, -r4 | st r4, [0636]@3   [4b0b 5812 6068 af6c cfe6 1500 2547 2ab8 0636 0008 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = (0x1.51c0000000000p-2 * r[0]) + (0x1.5700000000000p-1 * r[1]); QUANT(n); DONE(1, 5); }
        { RESET(); n[6] = 0x1.0000000000000p-13; QUANT(n); DONE(2, 6); }
        { RESET(); n[2] = pymod(r[2] + M->H[4], 512); QUANT(n); DONE(3, 2); }
        { WR(M, 0x0636, r[4]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 215: mov r7, #0.0234375@4:24 | f6 r6, -r4 | st r5, [0637]@0 | st r5, [0638]@1   [6509 7064 ef6c dfe0 dfe2 0637 0638 0600 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = 0x1.8000000000000p-6; QUANT(n); DONE(1, 7); }
        { RESET(); n[6] = pymod(r[6] + M->H[4], 512); QUANT(n); DONE(2, 6); }
        { WR(M, 0x0637, r[5]); }
        { WR(M, 0x0638, r[5]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 216: cmp r3, r0, -r2 | setflags 3, 1   [1b03 be0a b9b1] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[3] + -r[2]); QUANT(n); DONE(1, 3); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 217: mov r2, #6.104e-05@0:24 | cmp r3, r0, -r6 | setflags 3, 2   [3106 2060 be0e b9b2 0004 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = 0x1.0000000000000p-14; QUANT(n); DONE(1, 2); }
        { RESET(); RESV(r[3] + -r[6]); QUANT(n); DONE(2, 3); }
        { SETFLAGS(3, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 218: mac_p1p2c0 r0, r3, -r2, r7 | addxy r0, r3, -r2 | mov r6, #0.0001221@2:24 | cond 13, 1, 2   [4808 0cc0 0cca 6062 1a12 a007 0008 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        if (!(sk >> 0 & 1)) { RESET(); n[0] = (r[3]) + (r[7]) + (-r[2]); QUANT(n); DONE(1, 0); }
        if (!(sk >> 1 & 1)) { RESET(); n[0] = AWRAP(r[3] + -r[2]); QUANT(n); DONE(2, 0); }
        if (!(sk >> 2 & 1)) { RESET(); n[6] = 0x1.0000000000000p-13; QUANT(n); DONE(3, 6); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 219: mac_p1p2c0 r4, r3, -r6, r7 | addxy r4, r3, -r6 | cond 13, 1, 2 | st r0, [063a]@1   [4d07 4cc0 4cce 1a12 8fe2 e007 063a] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        if (!(sk >> 0 & 1)) { RESET(); n[4] = (r[3]) + (r[7]) + (-r[6]); QUANT(n); DONE(1, 4); }
        if (!(sk >> 1 & 1)) { RESET(); n[4] = AWRAP(r[3] + -r[6]); QUANT(n); DONE(2, 4); }
        if (!(sk >> 2 & 1)) { WR(M, 0x063a, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 220: st r4, [063b]@0 | st r4, [063c]@1   [1e05 cfe0 cfe2 063b 063c] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x063b, r[4]); }
        { WR(M, 0x063c, r[4]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 221: jmp .+135@0   [0b03 0090 0087] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { jto = 247; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 222: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
L223:;
    /* 223: ld r0, [0634]@0 | ld r1, [0635]@1   [1e05 0fe0 1fe2 0634 0635] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[0] = RD(M, 0x0634); }
        { n[1] = RD(M, 0x0635); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 224: mac_p1wc1 r4, r0, #0.75@2:16, #0.25@4, r1 | mov r2, #6.104e-05@8:24 | ld r3, [0639]@3   [250a 4812 2068 3fe6 1500 2c00 2400 0639 0004 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = (0x1.8000000000000p-1 * r[0]) + (0x1.0000000000000p-2 * r[1]); QUANT(n); DONE(1, 4); }
        { RESET(); n[2] = 0x1.0000000000000p-14; QUANT(n); DONE(2, 2); }
        { n[3] = RD(M, 0x0639); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 225: mac_p1wc1 r5, r0, #0.5@2:16, #0.5@4, r1 | mov r6, #0.0001221@8:24 | f6 r2, -r4 | st r4, [0636]@3   [4b0b 5812 6068 af6c cfe6 1500 2800 2800 0636 0008 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = (0x1.0000000000000p-1 * r[0]) + (0x1.0000000000000p-1 * r[1]); QUANT(n); DONE(1, 5); }
        { RESET(); n[6] = 0x1.0000000000000p-13; QUANT(n); DONE(2, 6); }
        { RESET(); n[2] = pymod(r[2] + M->H[4], 512); QUANT(n); DONE(3, 2); }
        { WR(M, 0x0636, r[4]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 226: mac_p1wc1 r4, r0, #0.25@2:16, #0.75@4, r1 | mov r7, #0.00018311@8:24 | f6 r6, -r4 | st r5, [0637]@3   [4b0b 4812 7068 ef6c dfe6 1500 2400 2c00 0637 000c 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = (0x1.0000000000000p-2 * r[0]) + (0x1.8000000000000p-1 * r[1]); QUANT(n); DONE(1, 4); }
        { RESET(); n[7] = 0x1.8000000000000p-13; QUANT(n); DONE(2, 7); }
        { RESET(); n[6] = pymod(r[6] + M->H[4], 512); QUANT(n); DONE(3, 6); }
        { WR(M, 0x0637, r[5]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 227: mov r5, #0.0234375@2:24 | f6 r7, -r4 | st r4, [0638]@0   [3107 5062 ff6c cfe0 0638 0600 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = 0x1.8000000000000p-6; QUANT(n); DONE(1, 5); }
        { RESET(); n[7] = pymod(r[7] + M->H[4], 512); QUANT(n); DONE(2, 7); }
        { WR(M, 0x0638, r[4]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 228: cmp r3, r0, -r2 | setflags 3, 1   [1b03 be0a b9b1] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[3] + -r[2]); QUANT(n); DONE(1, 3); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 229: mov r2, #6.104e-05@0:24 | cmp r3, r0, -r6 | setflags 3, 2   [3106 2060 be0e b9b2 0004 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = 0x1.0000000000000p-14; QUANT(n); DONE(1, 2); }
        { RESET(); RESV(r[3] + -r[6]); QUANT(n); DONE(2, 3); }
        { SETFLAGS(3, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 230: mac_p1p2c0 r0, r3, -r2, r5 | addxy r0, r3, -r2 | mov r6, #0.0001221@2:24 | sum8 r3, r0, -r7, r7 | cond 13, 1, 2 | setflags 3, 4   [904a 0cc0 0cca 6062 be0f 1a12 b9b4 a005 0008 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        if (!(sk >> 0 & 1)) { RESET(); n[0] = (r[3]) + (r[5]) + (-r[2]); QUANT(n); DONE(1, 0); }
        if (!(sk >> 1 & 1)) { RESET(); n[0] = AWRAP(r[3] + -r[2]); QUANT(n); DONE(2, 0); }
        if (!(sk >> 2 & 1)) { RESET(); n[6] = 0x1.0000000000000p-13; QUANT(n); DONE(3, 6); }
        if (!(sk >> 3 & 1)) { RESET(); RESV(r[3] + -r[7]); QUANT(n); DONE(4, 3); }
        if (!(sk >> 4 & 1)) { SETFLAGS(3, 4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 231: mac_p1p2c0 r4, r3, -r6, r5 | addxy r4, r3, -r6 | mov r7, #0.00018311@4:24 | cond 13, 1, 2 | st r0, [063a]@1   [920a 4cc0 4cce 7064 1a12 8fe2 e005 063a 000c 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        if (!(sk >> 0 & 1)) { RESET(); n[4] = (r[3]) + (r[5]) + (-r[6]); QUANT(n); DONE(1, 4); }
        if (!(sk >> 1 & 1)) { RESET(); n[4] = AWRAP(r[3] + -r[6]); QUANT(n); DONE(2, 4); }
        if (!(sk >> 2 & 1)) { RESET(); n[7] = 0x1.8000000000000p-13; QUANT(n); DONE(3, 7); }
        if (!(sk >> 3 & 1)) { WR(M, 0x063a, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 232: mac_p1p2c0 r2, r3, -r7, r5 | addxy r2, r3, -r7 | cond 13, 1, 2 | st r4, [063b]@1   [4d07 2cc0 2ccf 1a12 cfe2 f005 063b] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 0 ? 1ull << 0 : 1ull << 1;
        if (!(sk >> 0 & 1)) { RESET(); n[2] = (r[3]) + (r[5]) + (-r[7]); QUANT(n); DONE(1, 2); }
        if (!(sk >> 1 & 1)) { RESET(); n[2] = AWRAP(r[3] + -r[7]); QUANT(n); DONE(2, 2); }
        if (!(sk >> 2 & 1)) { WR(M, 0x063b, r[4]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 233: st r2, [063c]@0   [0c03 afe0 063c] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x063c, r[2]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 234: jmp .+48@0   [0b03 0090 0030] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { jto = 247; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 235: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
L236:;
    /* 236: ld r1, [0634]@0 | ld r2, [0639]@1   [1e05 1fe0 2fe2 0634 0639] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[1] = RD(M, 0x0634); }
        { n[2] = RD(M, 0x0639); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 237: st r1, [0636]@0 | st r2, [063a]@1   [1e05 9fe0 afe2 0636 063a] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0636, r[1]); }
        { WR(M, 0x063a, r[2]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 238: st r1, [0637]@0 | st r2, [063b]@1   [1e05 9fe0 afe2 0637 063b] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0637, r[1]); }
        { WR(M, 0x063b, r[2]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 239: st r1, [0638]@0 | st r2, [063c]@1   [1e05 9fe0 afe2 0638 063c] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0638, r[1]); }
        { WR(M, 0x063c, r[2]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 240: jmp .+24@0   [0b03 0090 0018] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { jto = 247; }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
        if (jto >= 0) switch (jto) { case 106: goto L106; case 116: goto L116; case 127: goto L127; case 140: goto L140; case 146: goto L146; case 151: goto L151; case 202: goto L202; case 212: goto L212; case 223: goto L223; case 236: goto L236; case 242: goto L242; case 247: goto L247; default: ABORT("jump"); }
    }
    /* 241: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
L242:;
    /* 242: ld r0, [0635]@0   [0c03 0fe0 0635] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[0] = RD(M, 0x0635); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 243: st r0, [0634]@0 | st r0, [0636]@1   [1e05 8fe0 8fe2 0634 0636] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x0634, r[0]); }
        { WR(M, 0x0636, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 244: ld r1, [0639]@0 | st r0, [0637]@1 | st r0, [0638]@2   [3e07 1fe0 8fe2 8fe4 0639 0637 0638] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[1] = RD(M, 0x0639); }
        { WR(M, 0x0637, r[0]); }
        { WR(M, 0x0638, r[0]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 245: st r1, [063a]@0 | st r1, [063b]@1   [1e05 9fe0 9fe2 063a 063b] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x063a, r[1]); }
        { WR(M, 0x063b, r[1]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 246: st r1, [063c]@0   [0c03 9fe0 063c] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x063c, r[1]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
L247:;
    /* 247: mov r4, #0.0234375@2:24 | ld r1, [0639]@0   [1806 4062 1fe0 0639 0600 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[4] = 0x1.8000000000000p-6; QUANT(n); DONE(1, 4); }
        { n[1] = RD(M, 0x0639); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 248: mov r6, #1@4:32 | f4 r1, -r1 | f6 r4, -r4 | ld r0, [0634]@0 | ld r3, [063a]@1   [be0a 6074 9f49 cf6c 0fe0 3fe2 0634 063a 3000 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[6] = 0x1.0000000000000p+0; QUANT(n); DONE(1, 6); }
        { RESET(); n[1] = r[1]; QUANT(n); DONE(2, 1); }
        { RESET(); n[4] = pymod(r[4] + M->H[4], 512); QUANT(n); DONE(3, 4); }
        { n[0] = RD(M, 0x0634); }
        { n[3] = RD(M, 0x063a); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 249: mov r2, r1 | f6 r4, -r7   [1603 2041 cf6f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[1]; QUANT(n); DONE(1, 2); }
        { RESET(); n[4] = pymod(r[4] + M->H[7], 512); QUANT(n); DONE(2, 4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 250: f6 r2, -r7   [0a02 af6f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = pymod(r[2] + M->H[7], 512); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 251: cmp r2, r0, #-8@0:32 | setflags 3, 1   [1b05 ae30 b9b1 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 252: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 253: addxy r2, r2, #-8@0:32 | cond !13, 1, 0   [1705 2cb0 1b10 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 1 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[2] = AWRAP(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 254: f4 r3, -r3 | ld r0, [0636]@0 | ld r5, [063b]@1 | setflags 0, 10   [7307 bf4b 0fe0 5fe2 89ba 0636 063b] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[3] = r[3]; QUANT(n); DONE(1, 3); }
        { n[0] = RD(M, 0x0636); }
        { n[5] = RD(M, 0x063b); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 255: mov r2, r3   [0902 2043] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[3]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 256: f6 r2, -r7   [0a02 af6f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = pymod(r[2] + M->H[7], 512); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 257: cmp r2, r0, #-8@0:32 | setflags 3, 1   [1b05 ae30 b9b1 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 258: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 259: addxy r2, r2, #-8@0:32 | cond !13, 1, 0   [1705 2cb0 1b10 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 1 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[2] = AWRAP(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 260: f4 r5, -r5 | ld r0, [0637]@0 | ld r7, [063c]@1 | setflags 0, 10   [7307 df4d 0fe0 7fe2 89ba 0637 063c] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = r[5]; QUANT(n); DONE(1, 5); }
        { n[0] = RD(M, 0x0637); }
        { n[7] = RD(M, 0x063c); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 261: mov r2, r5   [0902 2045] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[5]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 262: f6 r2, -r7   [0a02 af6f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = pymod(r[2] + M->H[7], 512); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 263: cmp r2, r0, #-8@0:32 | setflags 3, 1   [1b05 ae30 b9b1 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 264: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 265: addxy r2, r2, #-8@0:32 | cond !13, 1, 0   [1705 2cb0 1b10 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 1 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[2] = AWRAP(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 266: f4 r7, -r7 | ld r0, [0638]@0 | setflags 0, 10   [3a05 ff4f 0fe0 89ba 0638] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = r[7]; QUANT(n); DONE(1, 7); }
        { n[0] = RD(M, 0x0638); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 267: mov r2, r7   [0902 2047] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = r[7]; QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 268: f6 r2, -r7   [0a02 af6f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[2] = pymod(r[2] + M->H[7], 512); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 269: cmp r2, r0, #-8@0:32 | setflags 3, 1   [1b05 ae30 b9b1 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 270: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 271: addxy r2, r2, #-8@0:32 | cond !13, 1, 0   [1705 2cb0 1b10 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 1 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[2] = AWRAP(r[2] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 2); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 272: cmp r4, r0, #-8@0:32 | setflags 3, 1 | setflags 0, 10   [3a06 ce30 b9b1 89ba c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[4] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 4); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 273: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 274: addxy r4, r4, #-8@0:32 | cond !13, 1, 0   [1705 4d30 1b10 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 1 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[4] = AWRAP(r[4] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 4); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 275: cmp r1, r0, -r6 | setflags 3, 1   [1b03 9e0e b9b1] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[1] + -r[6]); QUANT(n); DONE(1, 1); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 276: cmp r3, r0, -r6 | setflags 3, 1 | ld r1, [0634]@0   [3a05 be0e b9b1 1fe0 0634] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[3] + -r[6]); QUANT(n); DONE(1, 3); }
        { SETFLAGS(3, 1); }
        { n[1] = RD(M, 0x0634); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 277: sum8 r5, r0, -r6, r7 | cond 12, 3, 0 | setflags 3, 1 | setflags 1, 12 | ld r1, [0636]@0   [dd07 de0e 1830 b9b1 99bc 1fe0 0636] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 2 : 0;
        if (!(sk >> 0 & 1)) { RESET(); RESV(r[5] + -r[6]); QUANT(n); DONE(1, 5); }
        if (!(sk >> 1 & 1)) { SETFLAGS(3, 1); }
        if (!(sk >> 2 & 1)) { SETFLAGS(1, 12); }
        if (!(sk >> 3 & 1)) { n[1] = RD(M, 0x0636); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 278: sum8 r7, r0, -r6, r7 | cond 12, 3, 0 | setflags 3, 1 | setflags 1, 12 | ld r1, [0637]@0   [dd07 fe0e 1830 b9b1 99bc 1fe0 0637] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 2 : 0;
        if (!(sk >> 0 & 1)) { RESET(); RESV(r[7] + -r[6]); QUANT(n); DONE(1, 7); }
        if (!(sk >> 1 & 1)) { SETFLAGS(3, 1); }
        if (!(sk >> 2 & 1)) { SETFLAGS(1, 12); }
        if (!(sk >> 3 & 1)) { n[1] = RD(M, 0x0637); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 279: cond 12, 1, 0 | setflags 1, 12 | ld r1, [0638]@0   [3d05 1810 99bc 1fe0 0638] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { SETFLAGS(1, 12); }
        if (!(sk >> 1 & 1)) { n[1] = RD(M, 0x0638); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 280: cond 12, 1, 0 | setflags 1, 12   [1d03 1810 99bc] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 12) & 1) != 0 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { SETFLAGS(1, 12); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 281: hset h6, #0@6:24 | ld r1, [0622]@0 | ld r3, [4001]@1 | ld r4, [4002]@2   [770a 40a6 1fe0 3fe2 4fe4 0622 4001 4002 0000 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 0) & 1) != 0 ? 0 : 0;
        if (!(sk >> 0 & 1)) { M->H[6] = 0x0.0p+0; }
        if (!(sk >> 1 & 1)) { n[1] = RD(M, 0x0622); }
        if (!(sk >> 2 & 1)) { n[3] = RD(M, 0x4001); }
        if (!(sk >> 3 & 1)) { n[4] = RD(M, 0x4002); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 282: subxy r5, r4, #1@0:16 | f6 r1, -r6 | ld r2, [4000]@1   [3106 5f10 9f6e 2fe2 3000 4000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = AWRAP(0x1.0000000000000p+0 - r[4]); QUANT(n); DONE(1, 5); }
        { RESET(); n[1] = pymod(r[1] + M->H[6], 512); QUANT(n); DONE(2, 1); }
        { n[2] = RD(M, 0x4000); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 283: mac_p1w r7, r2, r5, r3, r4 | hset h4, #0@2:16 | ld r0, [063f]@2 | ld r2, [0640]@3   [5709 788d 4012 0fe4 2fe6 3300 0000 063f 0640] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 0) & 1) != 0 ? 1ull << 0 : 1ull << 2;
        if (!(sk >> 0 & 1)) { RESET(); n[7] = (r[5] * r[2]) + (r[4] * r[3]); QUANT(n); DONE(1, 7); }
        if (!(sk >> 1 & 1)) { M->H[4] = 0x1.9800000000000p-1; }
        if (!(sk >> 2 & 1)) { n[0] = RD(M, 0x063f); }
        if (!(sk >> 3 & 1)) { n[2] = RD(M, 0x0640); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 284: f6 r1, -r7   [0a02 9f6f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[1] = pymod(r[1] + M->H[7], 512); QUANT(n); DONE(1, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 285: cmp r1, r0, #-8@0:32 | setflags 3, 1   [1b05 9e30 b9b1 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[1] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 1); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 286: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 287: addxy r1, r1, #-8@0:32 | cond !13, 1, 0   [1705 1c70 1b10 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 1 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[1] = AWRAP(r[1] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 288: raw 7dd2 1a80 0fe6 ffe8 050a 2800 2800 0641 063e   [580a 7dd2 1a80 0fe6 ffe8 050a 2800 2800 0641 063e] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = (0x1.0000000000000p-1 * r[7]) + (-r[2]) + (0x1.0000000000000p-1 * r[0]); QUANT(n); DONE(1, 7); }
        { n[0] = RD(M, 0x0641); }
        { WR(M, 0x063e, r[7]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 289: mac_p1 r7, r7, #1.4@4:24, -r0 | ld r3, [c1f7]@1   [1407 75e4 3fe2 0008 c1f7 3666 6600] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = (0x1.6666600000000p+0 * r[7]) + (-r[0]); QUANT(n); DONE(1, 7); }
        { n[3] = RD(M, 0xc1f7); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 290: ymacd r7, r3, r2 | ld r6, [0608]@0   [1804 78c2 6fe0 0608] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = AWRAP(r[2] + r[3] * r[7]); QUANT(n); DONE(1, 7); }
        { n[6] = RD(M, 0x0608); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 291: ymacd r7, r3, r0 | st r7, [063f]@0   [1804 78c0 ffe0 063f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = AWRAP(r[0] + r[3] * r[7]); QUANT(n); DONE(1, 7); }
        { WR(M, 0x063f, r[7]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 292: addxy r7, r7, r6 | st r7, [0640]@0   [1804 7dc6 ffe0 0640] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = AWRAP(r[7] + r[6]); QUANT(n); DONE(1, 7); }
        { WR(M, 0x0640, r[7]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 293: st r7, [060a]@0   [0c03 ffe0 060a] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x060a, r[7]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 294: hset h6, #1@6:24 | ld r1, [0624]@0 | ld r3, [4004]@1 | ld r4, [4005]@2   [770a 40a6 1fe0 3fe2 4fe4 0624 4004 4005 0040 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 0) & 1) != 0 ? 0 : 0;
        if (!(sk >> 0 & 1)) { M->H[6] = 0x1.0000000000000p+0; }
        if (!(sk >> 1 & 1)) { n[1] = RD(M, 0x0624); }
        if (!(sk >> 2 & 1)) { n[3] = RD(M, 0x4004); }
        if (!(sk >> 3 & 1)) { n[4] = RD(M, 0x4005); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 295: subxy r5, r4, #1@0:16 | f6 r1, -r6 | ld r2, [4003]@1   [3106 5f10 9f6e 2fe2 3000 4003] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[5] = AWRAP(0x1.0000000000000p+0 - r[4]); QUANT(n); DONE(1, 5); }
        { RESET(); n[1] = pymod(r[1] + M->H[6], 512); QUANT(n); DONE(2, 1); }
        { n[2] = RD(M, 0x4003); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 296: mac_p1w r7, r2, r5, r3, r4 | hset h4, #0.00018310546875@2:16 | ld r0, [0643]@2 | ld r2, [0644]@3   [5709 788d 4012 0fe4 2fe6 3300 0003 0643 0644] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 0) & 1) != 0 ? 1ull << 0 : 1ull << 2;
        if (!(sk >> 0 & 1)) { RESET(); n[7] = (r[5] * r[2]) + (r[4] * r[3]); QUANT(n); DONE(1, 7); }
        if (!(sk >> 1 & 1)) { M->H[4] = 0x1.9800000000000p-1; }
        if (!(sk >> 2 & 1)) { n[0] = RD(M, 0x0643); }
        if (!(sk >> 3 & 1)) { n[2] = RD(M, 0x0644); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 297: f6 r1, -r7   [0a02 9f6f] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[1] = pymod(r[1] + M->H[7], 512); QUANT(n); DONE(1, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 298: cmp r1, r0, #-8@0:32 | setflags 3, 1   [1b05 9e30 b9b1 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); RESV(r[1] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 1); }
        { SETFLAGS(3, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 299: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 300: addxy r1, r1, #-8@0:32 | cond !13, 1, 0   [1705 1c70 1b10 c800 0000] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 13) & 1) != 1 ? 1ull << 0 : 0;
        if (!(sk >> 0 & 1)) { RESET(); n[1] = AWRAP(r[1] + -0x1.0000000000000p+3); QUANT(n); DONE(1, 1); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 301: raw 7dd2 1a80 0fe6 ffe8 050a 2800 2800 0645 0642   [580a 7dd2 1a80 0fe6 ffe8 050a 2800 2800 0645 0642] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = (0x1.0000000000000p-1 * r[7]) + (-r[2]) + (0x1.0000000000000p-1 * r[0]); QUANT(n); DONE(1, 7); }
        { n[0] = RD(M, 0x0645); }
        { WR(M, 0x0642, r[7]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 302: mac_p1 r7, r7, #1.4@4:24, -r0 | ld r3, [c1f7]@1   [1407 75e4 3fe2 0008 c1f7 3666 6600] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = (0x1.6666600000000p+0 * r[7]) + (-r[0]); QUANT(n); DONE(1, 7); }
        { n[3] = RD(M, 0xc1f7); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 303: ymacd r7, r3, r2 | ld r6, [0609]@0   [1804 78c2 6fe0 0609] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = AWRAP(r[2] + r[3] * r[7]); QUANT(n); DONE(1, 7); }
        { n[6] = RD(M, 0x0609); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 304: ymacd r7, r3, r0 | st r7, [0643]@0   [1804 78c0 ffe0 0643] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = AWRAP(r[0] + r[3] * r[7]); QUANT(n); DONE(1, 7); }
        { WR(M, 0x0643, r[7]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 305: addxy r7, r7, r6 | st r7, [0644]@0   [1804 7dc6 ffe0 0644] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); n[7] = AWRAP(r[7] + r[6]); QUANT(n); DONE(1, 7); }
        { WR(M, 0x0644, r[7]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 306: st r7, [060c]@0 | ld r0, [0600]@1 | ld r1, [0601]@2   [3e07 ffe0 0fe2 1fe4 060c 0600 0601] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x060c, r[7]); }
        { n[0] = RD(M, 0x0600); }
        { n[1] = RD(M, 0x0601); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 307: ld r0, [0600]@0 | ld r1, [0601]@1   [1e05 0fe0 1fe2 0600 0601] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { n[0] = RD(M, 0x0600); }
        { n[1] = RD(M, 0x0601); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 308: st r0, [2203]@0 | st r1, [2205]@1   [1e05 8fe0 9fe2 2203 2205] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { WR(M, 0x2203, r[0]); }
        { WR(M, 0x2205, r[1]); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 309: f7 r7, r0   [0a02 ff70] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        { RESET(); QUANT(n); DONE(1, 7); }
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 310: cond 0, 15, 8   [0b02 00f8] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        sk |= ((M->F >> 0) & 1) != 0 ? 0 : 0;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
    /* 311: raw   [0001] */
    {
        memcpy(n, r, sizeof n);
        um = uv = 0, sk = dn = 0, jto = -1;
        M->F = (M->F & ~M->PM) | M->PV;
        M->PM = um, M->PV = uv;
        memcpy(r, n, sizeof n);
    }
}

