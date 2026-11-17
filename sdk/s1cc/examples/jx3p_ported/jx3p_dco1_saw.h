/* written by sdk/s1cc/tools/dsplift.py --kernel: jx3p_dco1_saw=/home/emilio/git/roland-plugout-reveng/work/jx3p/jx3p_dco1_saw.txt */
#include "dsplift_k.h"

/* jx3p_dco1_saw: written by sdk/s1cc/tools/dsplift.py --kernel. Every operation of the program, in order, as
 * expressions (dsplift.md §6): a name per value that is used more than once, stored in bank 0 or too long;
 * the number after each line = the instruction that makes it. SEL(w, a, b) = w ? a : b (w = 0 / 1), LERP(a, b, t) = a + t (b - a),
 * K_NZ / K_GE0 / K_LE0 = the condition bits, K_RECIP = the reciprocal (dsplift_k.h). Delay chains: X_zK =
 * what cell X held K samples ago. */
#define IN_DCO_PITCH 0xa0d0
#define IN_DCO_WORD1 0xa0d1
#define IN_DCO_WORD2 0xa0d2
#define IN_DCO_WORD3 0xa0d3
#define IN_DCO_WAVE 0xa0d4
static float cell_2_prev_z1;
static float cell_3_z1;
static float cell_4_z1;
static float cell_8_z1;
static float state_z1;
static float cell_9_z1, cell_9_z2, cell_9_z3, cell_9_z4, cell_9_z5, cell_9_z6, cell_9_z7;
static float cell_10_z1, cell_10_z2, cell_10_z3, cell_10_z4, cell_10_z5, cell_10_z6, cell_10_z7;
static float cell_11_z1, cell_11_z2, cell_11_z3, cell_11_z4, cell_11_z5, cell_11_z6, cell_11_z7;
static float cell_12_z1, cell_12_z2, cell_12_z3, cell_12_z4, cell_12_z5, cell_12_z6;
static float jx3p_dco1_saw_f0;
static inline __attribute__((always_inline)) void jx3p_dco1_saw(void)
{
    float dco_pitch_exp = K_EXP((KIN(IN_DCO_PITCH) - 4.75f) * 0.6931472f);                       /* 14 */
    float dco_word3 = KIN(IN_DCO_WORD3);                                                         /* 17 */
    float dco_word2 = KIN(IN_DCO_WORD2);                                                         /* 18 */
    float dco_wave = KIN(IN_DCO_WAVE);                                                           /* 18 */
    float t19 = dco_pitch_exp * 0.0022916666f;                                                   /* 19 */
    float t21 = SEL(K_Z(t19), 2.3283064e-10f, fminf(t19, 0.249f));                               /* 21 */
    float t24 = dco_word3 - 0.25f;                                                               /* 24 */
    float t27 = cell_3_z1 * KIN(IN_DCO_WORD1) * SEL(K_NZ(t24), 1.0f, dco_word2);                 /* 27 */
    float dco_word1_2 = s1_wrap(t27 + t21);                                                      /* 29 */
    float cell_2_prev = SEL(K_LT0(dco_word1_2 - t27), dco_word1_2, s1_wrap(SEL(K_Z(t24), -1.0f, cell_2_prev_z1) + t21)); /* 32 */
    float recip = K_RECIP(t21 * 256.0f);                                                         /* 34 */
    float t45 = K_CLAMP(K_TRI(0.5f * cell_2_prev + 0.5f) * 256.0f * recip * 0.25f) * 1.570796f;  /* 45 */
    float t46 = t45 * t45;                                                                       /* 46 */
    float cell_7 = (s1_wrap(cell_2_prev) + cell_2_prev) * 0.5f * K_SIN(t45);                     /* 51 */
    float t51 = dco_word3 - 0.5f;                                                                /* 51 */
    float t53 = cell_2_prev - cell_2_prev_z1;                                                    /* 53 */
    float t51_z = K_Z(t51);                                                                      /* 53 */
    float t53_le0 = K_LE0(t53);                                                                  /* 53 */
    float t54 = dco_word1_2 * SEL(t51_z, dco_word2, SEL(K_NZ(t51), 1.0f, t46 * t46));            /* 54 */
    float t53_gt0 = K_GT0(t53);                                                                  /* 55 */
    float cell = SEL(t53_gt0, 0.0f, SEL(t53_le0, 0.25f, cell_2_prev_z1));                        /* 55 */
    float cell_5 = fmaxf(SEL(t53_gt0, cell_4_z1, SEL(t53_le0, 1.6999998f, cell)) - 0.04f, 0.0f); /* 58 */
    float dco_word1_2_2 = s1_wrap(t54 + t21);                                                    /* 59 */
    float cell_9 = XFADE(cell_7, fminf(cell_5, 1.0f), dco_wave);                                 /* 63 */
    float cell_2_prev_2 = SEL(K_LT0(dco_word1_2_2 - t54), dco_word1_2_2, s1_wrap(SEL(t51_z, -1.0f, cell_2_prev + t21))); /* 64 */
    float t71 = K_CLAMP(K_TRI(0.5f * cell_2_prev_2 + 0.5f) * 256.0f * recip * 0.25f) * 1.570796f; /* 71 */
    float t72 = t71 * t71;                                                                       /* 72 */
    float cell_7_2 = (s1_wrap(cell_2_prev_2) + cell_2_prev_2) * 0.5f * K_SIN(t71);               /* 77 */
    float t77 = dco_word3 - 0.75f;                                                               /* 77 */
    float t79 = cell_2_prev_2 - cell_2_prev;                                                     /* 79 */
    float t77_z = K_Z(t77);                                                                      /* 79 */
    float t79_le0 = K_LE0(t79);                                                                  /* 79 */
    float t80 = dco_word1_2_2 * SEL(t77_z, dco_word2, SEL(K_NZ(t77), 1.0f, t72 * t72));          /* 80 */
    float t79_gt0 = K_GT0(t79);                                                                  /* 81 */
    float cell_2 = SEL(t79_gt0, cell, SEL(t79_le0, 0.5f, cell_2_prev));                          /* 81 */
    float cell_5_2 = fmaxf(SEL(t79_gt0, cell_5, SEL(t79_le0, 1.6999998f, cell_2)) - 0.04f, 0.0f); /* 84 */
    float dco_word1_2_3 = s1_wrap(t80 + t21);                                                    /* 85 */
    float cell_10 = XFADE(cell_7_2, fminf(cell_5_2, 1.0f), dco_wave);                            /* 89 */
    float cell_2_prev_3 = SEL(K_LT0(dco_word1_2_3 - t80), dco_word1_2_3, s1_wrap(SEL(t77_z, -1.0f, cell_2_prev_2 + t21))); /* 90 */
    float t97 = K_CLAMP(K_TRI(0.5f * cell_2_prev_3 + 0.5f) * 256.0f * recip * 0.25f) * 1.570796f; /* 97 */
    float t98 = t97 * t97;                                                                       /* 98 */
    float cell_7_3 = (s1_wrap(cell_2_prev_3) + cell_2_prev_3) * 0.5f * K_SIN(t97);               /* 103 */
    float t103 = dco_word3 - 1.0f;                                                               /* 103 */
    float t105 = cell_2_prev_3 - cell_2_prev_2;                                                  /* 105 */
    float t103_z = K_Z(t103);                                                                    /* 105 */
    float t105_le0 = K_LE0(t105);                                                                /* 105 */
    float t106 = dco_word1_2_3 * SEL(t103_z, dco_word2, SEL(K_NZ(t103), 1.0f, t98 * t98));       /* 106 */
    float t105_gt0 = K_GT0(t105);                                                                /* 107 */
    float cell_3 = SEL(t105_gt0, cell_2, SEL(t105_le0, 0.75f, cell_2_prev_2));                   /* 107 */
    float cell_5_3 = fmaxf(SEL(t105_gt0, cell_5_2, SEL(t105_le0, 1.6999998f, cell_3)) - 0.04f, 0.0f); /* 110 */
    float cell_3_2 = s1_wrap(t106 + t21);                                                        /* 111 */
    float cell_11 = XFADE(cell_7_3, fminf(cell_5_3, 1.0f), dco_wave);                            /* 115 */
    float cell_2_prev_4 = SEL(K_LT0(cell_3_2 - t106), cell_3_2, s1_wrap(SEL(t103_z, -1.0f, cell_2_prev_3 + t21))); /* 116 */
    float t123 = K_CLAMP(K_TRI(0.5f * cell_2_prev_4 + 0.5f) * 256.0f * recip * 0.25f) * 1.570796f; /* 123 */
    float t124 = t123 * t123;                                                                    /* 124 */
    float cell_6 = (s1_wrap(cell_2_prev_4) + cell_2_prev_4) * 0.5f * K_SIN(t123);                /* 129 */
    float t130 = cell_2_prev_4 - cell_2_prev_3;                                                  /* 130 */
    float cell_4 = fmaxf(SEL(K_GT0(t130), cell_5_3, SEL(K_LE0(t130), 1.6999998f, t124 * t124)) - 0.04f, 0.0f); /* 134 */
    float cell_12 = XFADE(cell_6, fminf(cell_4, 1.0f), dco_wave);                                /* 136 */
    float t139 = -(7.8084646e-05f * cell_10) - 7.8084646e-05f * cell_11_z7 - -9.239372e-05f * cell_11; /* 139 */
    float t140 = t139 - -9.239372e-05f * cell_10_z7 - -0.0011889678f * cell_12 - -0.0011889678f * cell_9_z7; /* 140 */
    float t142 = t140 - -0.0036064829f * cell_9_z1 - -0.0036064829f * cell_12_z6 - -0.0070102336f * cell_10_z1; /* 142 */
    float t143 = t142 - -0.0070102336f * cell_11_z6 - -0.010096775f * cell_11_z1 - -0.010096775f * cell_10_z6; /* 143 */
    float t145 = t143 - -0.01071596f * cell_12_z1 - -0.01071596f * cell_9_z6 - -0.006385348f * cell_9_z2; /* 145 */
    float t146 = t145 - -0.006385348f * cell_12_z5 - 0.0049214866f * cell_10_z2 - 0.0049214866f * cell_11_z5; /* 146 */
    float t148 = t146 - 0.023953114f * cell_11_z2 - 0.023953114f * cell_10_z5 - 0.04961065f * cell_12_z2; /* 148 */
    float t149 = t148 - 0.04961065f * cell_9_z5 - 0.07886231f * cell_9_z3 - 0.07886231f * cell_12_z4; /* 149 */
    float t151 = t149 - 0.107232735f * cell_10_z3 - 0.107232735f * cell_11_z4 - 0.12977375f * cell_11_z3; /* 151 */
    float cell_8 = t151 - 0.12977375f * cell_10_z4 - 0.14225915f * cell_12_z3 - 0.14225915f * cell_9_z4; /* 152 */
    float state = 0.8532445f * state_z1 + (0.44227272f * cell_8 - 0.29551718f * cell_8_z1);      /* 154 */
    float cell_13 = XFADE(state * -0.5f, state * 0.9f, dco_wave);                                /* 157 */
    KOUT(0x221f, cell_13);
    KOUT(0x221b, cell_3);

    jx3p_dco1_saw_f0 = jx3p_dco1_saw_f0;
    cell_2_prev_z1 = cell_2_prev_4;
    cell_3_z1 = cell_3_2;
    cell_4_z1 = cell_4;
    cell_8_z1 = cell_8;
    state_z1 = state;
    cell_9_z7 = cell_9_z6;
    cell_9_z6 = cell_9_z5;
    cell_9_z5 = cell_9_z4;
    cell_9_z4 = cell_9_z3;
    cell_9_z3 = cell_9_z2;
    cell_9_z2 = cell_9_z1;
    cell_9_z1 = cell_9;
    cell_10_z7 = cell_10_z6;
    cell_10_z6 = cell_10_z5;
    cell_10_z5 = cell_10_z4;
    cell_10_z4 = cell_10_z3;
    cell_10_z3 = cell_10_z2;
    cell_10_z2 = cell_10_z1;
    cell_10_z1 = cell_10;
    cell_11_z7 = cell_11_z6;
    cell_11_z6 = cell_11_z5;
    cell_11_z5 = cell_11_z4;
    cell_11_z4 = cell_11_z3;
    cell_11_z3 = cell_11_z2;
    cell_11_z2 = cell_11_z1;
    cell_11_z1 = cell_11;
    cell_12_z6 = cell_12_z5;
    cell_12_z5 = cell_12_z4;
    cell_12_z4 = cell_12_z3;
    cell_12_z3 = cell_12_z2;
    cell_12_z2 = cell_12_z1;
    cell_12_z1 = cell_12;
}

