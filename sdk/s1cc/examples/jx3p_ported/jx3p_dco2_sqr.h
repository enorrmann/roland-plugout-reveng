/* written by sdk/s1cc/tools/dsplift.py --kernel: jx3p_dco2_sqr=/home/emilio/git/roland-plugout-reveng/work/jx3p/jx3p_dco2_sqr.txt */
#include "dsplift_k.h"

/* jx3p_dco2_sqr: written by sdk/s1cc/tools/dsplift.py --kernel. Every operation of the program, in order, as
 * expressions (dsplift.md §6): a name per value that is used more than once, stored in bank 0 or too long;
 * the number after each line = the instruction that makes it. SEL(w, a, b) = w ? a : b (w = 0 / 1), LERP(a, b, t) = a + t (b - a),
 * K_NZ / K_GE0 / K_LE0 = the condition bits, K_RECIP = the reciprocal (dsplift_k.h). Delay chains: X_zK =
 * what cell X held K samples ago. */
#define IN_DCO_PITCH 0xa0d0
#define IN_DCO_WORD2 0xa0d2
#define IN_DCO_WORD3 0xa0d3
static float cell_2_prev_z1;
static float cell_3_z1;
static float cell_4_z1;
static float cell_6_z1;
static float cell_7_prev2_z1, cell_7_prev2_z2, cell_7_prev2_z3, cell_7_prev2_z4, cell_7_prev2_z5, cell_7_prev2_z6, cell_7_prev2_z7;
static float cell_7_prev3_z1, cell_7_prev3_z2, cell_7_prev3_z3, cell_7_prev3_z4, cell_7_prev3_z5, cell_7_prev3_z6, cell_7_prev3_z7;
static float cell_7_prev4_z1, cell_7_prev4_z2, cell_7_prev4_z3, cell_7_prev4_z4, cell_7_prev4_z5, cell_7_prev4_z6, cell_7_prev4_z7;
static float cell_6_prev_z1, cell_6_prev_z2, cell_6_prev_z3, cell_6_prev_z4, cell_6_prev_z5, cell_6_prev_z6;
static float cell_9_z1;
static float cell_11_z1;
static float state_z1;
static float jx3p_dco2_sqr_f0;
static inline __attribute__((always_inline)) void jx3p_dco2_sqr(void)
{
    float dco_pitch_exp = K_EXP((KIN(IN_DCO_PITCH) - 4.75f) * 0.6931472f);                       /* 14 */
    float dco_word3 = KIN(IN_DCO_WORD3);                                                         /* 17 */
    float t20 = fminf(fmaxf(dco_pitch_exp * 0.0022916666f, 2.3283064e-10f), 0.249f);             /* 20 */
    float cell_10 = K_RECIP(t20 * 256.0f) * 0.19999981f;                                         /* 26 */
    float t27 = cell_3_z1 * KIN(IN_DCO_WORD2);                                                   /* 27 */
    float dco_word2_2 = s1_wrap(t27 + t20);                                                      /* 31 */
    float cell_2_prev = SEL(K_LT0(dco_word2_2 - t27), dco_word2_2, SEL(K_Z(dco_word3 - 0.25f), -1.0f, s1_wrap(cell_2_prev_z1 + t20))); /* 34 */
    float t38 = s1_sign(cell_2_prev);                                                            /* 38 */
    float t47 = K_CLAMP(K_TRI(K_RECIP(SEL(K_LT0(cell_2_prev), -1.0f, SEL(K_GE0(cell_2_prev), 1.0f, dco_word2_2))) * cell_2_prev) * 256.0f * cell_10); /* 47 */
    float t49 = cell_2_prev_z1 - cell_2_prev;                                                    /* 49 */
    float cell_12 = SEL(K_NZ(cell_11_z1 + t38), 1.0f, cell_9_z1);                                /* 51 */
    float cell = SEL(K_LE0(t49), 0.0f, SEL(K_GT0(t49), 0.25f, dco_word3));                       /* 51 */
    float dco_word2_2_2 = s1_wrap(dco_word2_2 + t20);                                            /* 52 */
    float cell_9 = cell_12 * 0.98f;                                                              /* 53 */
    float cell_2_prev_2 = SEL(K_LT0(dco_word2_2_2 - dco_word2_2), dco_word2_2_2, SEL(K_Z(dco_word3 - 0.5f), -1.0f, s1_wrap(cell_2_prev + t20))); /* 55 */
    float t56 = 0.5f * t20;                                                                      /* 56 */
    float t56_2 = s1_abs(cell_12);                                                               /* 56 */
    float state = fmaxf(state_z1 * (t56_2 * -0.5f) + (state_z1 - t56 + t56_2), 0.0f);            /* 59 */
    float cell_5 = (0.5f * cell_9 - t38) * (t47 + 1.0f - fminf(state, 1.0f));                    /* 62 */
    float cell_7 = 0.79069895f * cell_6_z1 + (cell_5 * 0.7385094f - 0.52920836f * cell_4_z1);    /* 65 */
    float t69 = s1_sign(cell_2_prev_2);                                                          /* 69 */
    float t77 = K_TRI(K_RECIP(SEL(K_LT0(cell_2_prev_2), -1.0f, SEL(K_GE0(cell_2_prev_2), 1.0f, cell_6_z1))) * cell_2_prev_2) * 256.0f * cell_10; /* 77 */
    float cell_12_2 = SEL(K_NZ(t69 - t38), 1.0f, cell_9);                                        /* 82 */
    float dco_word2_2_3 = s1_wrap(dco_word2_2_2 + t20);                                          /* 83 */
    float cell_9_2 = cell_12_2 * 0.98f;                                                          /* 84 */
    float cell_2_prev_3 = SEL(K_LT0(dco_word2_2_3 - dco_word2_2_2), dco_word2_2_3, SEL(K_Z(dco_word3 - 0.75f), -1.0f, s1_wrap(cell_2_prev_2 + t20))); /* 86 */
    float t87 = s1_abs(cell_12_2);                                                               /* 87 */
    float state_2 = fmaxf(state * (t87 * -0.5f) + (state - t56 + t87), 0.0f);                    /* 90 */
    float cell_5_2 = (0.5f * cell_9_2 - t69) * (K_CLAMP(t77) + 1.0f - fminf(state_2, 1.0f));     /* 93 */
    float cell_7_2 = 0.79069895f * cell_7 + (cell_5_2 * 0.7385094f - 0.52920836f * cell_5);      /* 96 */
    float t100 = s1_sign(cell_2_prev_3);                                                         /* 100 */
    float t109 = K_CLAMP(K_TRI(K_RECIP(SEL(K_LT0(cell_2_prev_3), -1.0f, SEL(K_GE0(cell_2_prev_3), 1.0f, cell_7))) * cell_2_prev_3) * 256.0f * cell_10); /* 109 */
    float cell_12_3 = SEL(K_NZ(t100 - t69), 1.0f, cell_9_2);                                     /* 113 */
    float cell_3 = s1_wrap(dco_word2_2_3 + t20);                                                 /* 114 */
    float cell_9_3 = cell_12_3 * 0.98f;                                                          /* 115 */
    float cell_2_prev_4 = SEL(K_LT0(cell_3 - dco_word2_2_3), cell_3, SEL(K_Z(dco_word3 - 1.0f), -1.0f, s1_wrap(cell_2_prev_3 + t20))); /* 117 */
    float t118 = s1_abs(cell_12_3);                                                              /* 118 */
    float state_3 = fmaxf(state_2 * (t118 * -0.5f) + (state_2 - t56 + t118), 0.0f);              /* 121 */
    float cell_5_3 = (0.5f * cell_9_3 - t100) * (t109 + 1.0f - fminf(state_3, 1.0f));            /* 124 */
    float cell_7_3 = 0.79069895f * cell_7_2 + (cell_5_3 * 0.7385094f - 0.52920836f * cell_5_2);  /* 127 */
    float t131 = s1_sign(cell_2_prev_4);                                                         /* 131 */
    float cell_11 = -t131;                                                                       /* 136 */
    float t140 = K_CLAMP(K_TRI(K_RECIP(SEL(K_LT0(cell_2_prev_4), -1.0f, SEL(K_GE0(cell_2_prev_4), 1.0f, cell_7_2))) * cell_2_prev_4) * 256.0f * cell_10); /* 140 */
    float cell_12_4 = SEL(K_NZ(t131 - t100), 1.0f, cell_9_3);                                    /* 143 */
    float cell_9_4 = cell_12_4 * 0.98f;                                                          /* 144 */
    float t146 = s1_abs(cell_12_4);                                                              /* 146 */
    float state_4 = fmaxf(state_3 * (t146 * -0.5f) + (state_3 - t56 + t146), 0.0f);              /* 149 */
    float cell_4 = (0.5f * cell_9_4 - t131) * (t140 + 1.0f - fminf(state_4, 1.0f));              /* 152 */
    float cell_6_prev = 0.79069895f * cell_7_3 + (cell_4 * 0.7385094f - 0.52920836f * cell_5_3); /* 155 */
    float t159 = 0.00078085996f * cell_7_3 + (7.014e-05f * cell_7_2 + 7.014e-05f * cell_7_prev4_z7) + 0.00078085996f * cell_7_prev3_z7; /* 159 */
    float t161 = 0.00351809f * cell_7_prev2_z1 + (0.00230546f * cell_6_prev + t159 + 0.00230546f * cell_7_prev2_z7) + 0.00351809f * cell_6_prev_z6; /* 161 */
    float t163 = 0.0022288798f * cell_7_prev3_z1 + t161 + 0.0022288798f * cell_7_prev4_z6 - 0.00341358f * cell_7_prev4_z1; /* 163 */
    float t164 = t163 - 0.00341358f * cell_7_prev3_z6 - 0.012985799f * cell_6_prev_z1 - 0.012985799f * cell_7_prev2_z6; /* 164 */
    float t166 = t164 - 0.02252359f * cell_7_prev2_z2 - 0.02252359f * cell_6_prev_z5 - 0.02502777f * cell_7_prev3_z2 - 0.02502777f * cell_7_prev4_z5; /* 166 */
    float t168 = 0.01779937f * cell_6_prev_z2 + (t166 - 0.01301194f * cell_7_prev4_z2 - 0.01301194f * cell_7_prev3_z5); /* 168 */
    float t169 = 0.06536375f * cell_7_prev2_z3 + (t168 + 0.01779937f * cell_7_prev2_z5) + 0.06536375f * cell_6_prev_z4; /* 169 */
    float t171 = 0.16945842f * cell_7_prev4_z3 + (0.12054555f * cell_7_prev3_z3 + t169 + 0.12054555f * cell_7_prev4_z4); /* 171 */
    float cell_8 = (0.1982294f * cell_6_prev_z3 + (t171 + 0.16945842f * cell_7_prev3_z4) + 0.1982294f * cell_7_prev2_z4) * 0.32999998f; /* 173 */
    KOUT(0x2221, cell_8);
    KOUT(0x221d, cell);

    jx3p_dco2_sqr_f0 = jx3p_dco2_sqr_f0;
    cell_2_prev_z1 = cell_2_prev_4;
    cell_3_z1 = cell_3;
    cell_4_z1 = cell_4;
    cell_6_z1 = cell_6_prev;
    cell_7_prev2_z7 = cell_7_prev2_z6;
    cell_7_prev2_z6 = cell_7_prev2_z5;
    cell_7_prev2_z5 = cell_7_prev2_z4;
    cell_7_prev2_z4 = cell_7_prev2_z3;
    cell_7_prev2_z3 = cell_7_prev2_z2;
    cell_7_prev2_z2 = cell_7_prev2_z1;
    cell_7_prev2_z1 = cell_7;
    cell_7_prev3_z7 = cell_7_prev3_z6;
    cell_7_prev3_z6 = cell_7_prev3_z5;
    cell_7_prev3_z5 = cell_7_prev3_z4;
    cell_7_prev3_z4 = cell_7_prev3_z3;
    cell_7_prev3_z3 = cell_7_prev3_z2;
    cell_7_prev3_z2 = cell_7_prev3_z1;
    cell_7_prev3_z1 = cell_7_2;
    cell_7_prev4_z7 = cell_7_prev4_z6;
    cell_7_prev4_z6 = cell_7_prev4_z5;
    cell_7_prev4_z5 = cell_7_prev4_z4;
    cell_7_prev4_z4 = cell_7_prev4_z3;
    cell_7_prev4_z3 = cell_7_prev4_z2;
    cell_7_prev4_z2 = cell_7_prev4_z1;
    cell_7_prev4_z1 = cell_7_3;
    cell_6_prev_z6 = cell_6_prev_z5;
    cell_6_prev_z5 = cell_6_prev_z4;
    cell_6_prev_z4 = cell_6_prev_z3;
    cell_6_prev_z3 = cell_6_prev_z2;
    cell_6_prev_z2 = cell_6_prev_z1;
    cell_6_prev_z1 = cell_6_prev;
    cell_9_z1 = cell_9_4;
    cell_11_z1 = cell_11;
    state_z1 = state_4;
}

