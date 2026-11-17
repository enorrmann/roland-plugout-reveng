/* written by sdk/s1cc/tools/dsplift.py --kernel: jx3p_core=/home/emilio/git/roland-plugout-reveng/work/jx3p/jx3p_core.txt */
#include "dsplift_k.h"

/* jx3p_core: written by sdk/s1cc/tools/dsplift.py --kernel. Every operation of the program, in order, as
 * expressions (dsplift.md §6): a name per value that is used more than once, stored in bank 0 or too long;
 * the number after each line = the instruction that makes it. SEL(w, a, b) = w ? a : b (w = 0 / 1), LERP(a, b, t) = a + t (b - a),
 * K_NZ / K_GE0 / K_LE0 = the condition bits, K_RECIP = the reciprocal (dsplift_k.h). Delay chains: X_zK =
 * what cell X held K samples ago. */
#define IN_DCO1_OUT 0xa0c0
#define IN_DCO2_OUT 0xa0c2
#define IN_FILTER_G 0xa0c4
#define IN_VCA_IN 0xa0c5
#define P_DCO1_LEVEL 0xc034
#define P_DCO2_LEVEL 0xc035
#define P_VCF_RESONANCE 0xc121
static float dco1_z1;
static float vcf_2_z1;
static float vcf_state_z1;
static float vcf_state_2_z1;
static float vcf_state_3_z1;
static float vcf_state_4_z1;
static float vcf_3_z1;
static float vca_in_state_z1;
static float vcf_4_prev_z1;
static float vcf_state_5_z1;
static float vcf_5_prev_z1;
static float vcf_state_6_z1;
static float vcf_state_7_z1;
static float jx3p_core_f0;
static inline __attribute__((always_inline)) void jx3p_core(void)
{
    float dco1_out_2 = KIN(IN_DCO1_OUT) * KP(0xc032);                                            /* 89 */
    float dco2_out_2 = KIN(IN_DCO2_OUT) * KP(0xc033);                                            /* 89 */
    float dco1_prev = KP(P_DCO1_LEVEL) * dco1_out_2 + KP(P_DCO2_LEVEL) * dco2_out_2;             /* 92 */
    float vcf_resonance = KP(P_VCF_RESONANCE);                                                   /* 889 */
    float vcf = 6.0f * vcf_resonance + KP(0xc12c);                                               /* 932 */
    float filter_g = KIN(IN_FILTER_G);                                                           /* 970 */
    float t976 = vcf * 0.675f;                                                                   /* 976 */
    float t977 = filter_g * filter_g;                                                            /* 977 */
    float t980 = K_RECIP(1.0f + t976 * (t977 * t977));                                           /* 980 */
    float t988 = 1.0f - 2.0f * filter_g;                                                         /* 988 */
    float vcf_2 = K_CLAMP(dco1_z1 * 0.29999924f * t980 - t980 * t976 * (0.071428575f * K_CLAMP(vcf_3_z1 * 14.0f))); /* 989 */
    float vcf_state = filter_g * (vcf_2 + vcf_2_z1) + t988 * vcf_state_z1;                       /* 993 */
    float vcf_state_2 = filter_g * (vcf_state + vcf_state_z1) + t988 * vcf_state_2_z1;           /* 995 */
    float vcf_state_3 = filter_g * (vcf_state_2 + vcf_state_2_z1) + t988 * vcf_state_3_z1;       /* 997 */
    float t998 = filter_g * (filter_g * (filter_g * vcf_2 + vcf_state + t988 * vcf_state) + vcf_state_2 + t988 * vcf_state_2) + vcf_state_3; /* 998 */
    float vcf_state_4 = filter_g * (vcf_state_3 + vcf_state_3_z1) + t988 * vcf_state_4_z1;       /* 999 */
    float vcf_3 = filter_g * (t998 + t988 * vcf_state_3) + t988 * vcf_state_4;                   /* 1000 */
    float vcf_4 = vcf_state_4 * 0.32999992f;                                                     /* 1004 */
    float c_27b = KP(0xc12e);                                                                    /* 1008 */
    float vcf_state_5 = vcf_4 * 0.9998364f - 0.9998364f * vcf_4_prev_z1 + 0.99967283f * vcf_state_5_z1; /* 1041 */
    float vca_in_state = 0.9869955f * vca_in_state_z1 + KIN(IN_VCA_IN);                          /* 1050 */
    float t1067 = fmaxf(SEL(K_GE0(vca_in_state), vca_in_state, SEL(K_LT0(vca_in_state), vca_in_state, 0.0f)), 0.0f) * (1.0f + KP(0xc131)); /* 1067 */
    float vcf_5 = fminf((vcf_state_5 + vcf_resonance * 0.19999981f * vcf_state_5) * t1067, 5.5f) * 0.5999994f; /* 1070 */
    float vcf_state_6 = 0.76939964f * vcf_state_6_z1 + (vcf_5 * 3.6373444f - 3.4067383f * vcf_5_prev_z1); /* 1075 */
    float vcf_state_7 = 0.9836874f * vcf_state_7_z1 + (vcf_5 * 1.024313f - 0.9593744f * vcf_5_prev_z1); /* 1075 */
    float vcf_6 = SEL(K_GE0(c_27b), XFADE(vcf_5, vcf_state_6, fmaxf(c_27b, 0.0f)), XFADE(vcf_5, vcf_state_7, fmaxf(-c_27b, 0.0f))); /* 1079 */
    float vcf_7 = vcf_6 * KIN(0xa0c6);                                                           /* 1082 */
    float vcf_8 = vcf_7 * KIN(0xa0c7);                                                           /* 1085 */
    KOUT(0x231e, vcf_8);

    jx3p_core_f0 = jx3p_core_f0;
    dco1_z1 = dco1_prev;
    vcf_2_z1 = vcf_2;
    vcf_state_z1 = vcf_state;
    vcf_state_2_z1 = vcf_state_2;
    vcf_state_3_z1 = vcf_state_3;
    vcf_state_4_z1 = vcf_state_4;
    vcf_3_z1 = vcf_3;
    vca_in_state_z1 = vca_in_state;
    vcf_4_prev_z1 = vcf_4;
    vcf_state_5_z1 = vcf_state_5;
    vcf_5_prev_z1 = vcf_5;
    vcf_state_6_z1 = vcf_state_6;
    vcf_state_7_z1 = vcf_state_7;
}

