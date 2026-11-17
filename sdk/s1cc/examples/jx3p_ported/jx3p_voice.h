/* written by sdk/s1cc/tools/dsplift.py --kernel: jx3p_voice=/home/emilio/git/roland-plugout-reveng/work/jx3p/jx3p_voice.txt */
#include "dsplift_k.h"

/* jx3p_voice: written by sdk/s1cc/tools/dsplift.py --kernel. Every operation of the program, in order, as
 * expressions (dsplift.md §6): a name per value that is used more than once, stored in bank 0 or too long;
 * the number after each line = the instruction that makes it. SEL(w, a, b) = w ? a : b (w = 0 / 1), LERP(a, b, t) = a + t (b - a),
 * K_NZ / K_GE0 / K_LE0 = the condition bits, K_RECIP = the reciprocal (dsplift_k.h). Delay chains: X_zK =
 * what cell X held K samples ago. */
#define IN_DCO2_CROSS_MOD 0xa022
#define IN_DCO1_RANGE 0xa026
#define IN_DCO2_RANGE 0xa027
#define IN_DCO2_CROSS_MOD_1 0xa028
#define IN_DCO2_CROSS_MOD_2 0xa029
#define IN_DCO1_WAVEFORM 0xa02a
#define IN_DCO2_WAVEFORM 0xa02b
#define IN_DCO_ENV1_MOD 0xa02d
#define IN_VCF_ENV1_MOD 0xa066
#define IN_LFO 0xa0a5
#define IN_LFO_HALF 0xa0a6
#define IN_NOTE_PITCH 0xa0a7
#define IN_PITCH_OFFSET 0xa0a8
#define IN_GATE_WORD 0xa0a9
#define IN_ENV1 0xa0aa
#define IN_ENV2 0xa0ab
#define IN_IOMD_60E0 0xa0b3
#define IN_IOMD_60E2 0xa0b4
#define P_DCO2_TUNE 0xc020
#define P_DCO2_FINE_TUNE 0xc021
#define P_DCO_LFO_MOD 0xc026
#define P_DCO_ENV1_MOD_2 0xc027
#define P_DCO1_FREQ_MOD 0xc028
#define P_DCO2_FREQ_MOD 0xc029
#define P_DCO1_FREQ_MOD_1 0xc02a
#define P_DCO2_FREQ_MOD_1 0xc02b
#define P_DCO1_LEVEL 0xc034
#define P_DCO2_LEVEL 0xc035
#define P_VCF_CUTOFF_FREQ 0xc120
#define P_VCF_RESONANCE 0xc121
#define P_VCF_LFO_MOD 0xc126
#define P_VCF_ENV1_MOD_2 0xc127
#define P_VCF_PITCH_FOLLOW 0xc128
#define P_BRILLIANCE 0xc12d
static float D_01a_1;
static float D_01c_1;
static float D_01e_1;
static float D_020_1;
static float D_022_1;
static float D_024_1;
static float D_026_1;
static float D_036_1;
static float D_038_1;
static float dco1_z1;
static float D_08d_1, D_08d_2, D_08d_3;
static float dco_8_prev_z1;
static float dco_11_z1;
static float dco_15_z1;
static float dco_state_z1;
static float dco_16_z1, dco_16_z2, dco_16_z3, dco_16_z4, dco_16_z5, dco_16_z6, dco_16_z7;
static float dco_17_z1, dco_17_z2, dco_17_z3, dco_17_z4, dco_17_z5, dco_17_z6, dco_17_z7;
static float dco_18_z1, dco_18_z2, dco_18_z3, dco_18_z4, dco_18_z5, dco_18_z6, dco_18_z7;
static float dco_19_z1, dco_19_z2, dco_19_z3, dco_19_z4, dco_19_z5, dco_19_z6;
static float dco2_4_prev_z1;
static float dco2_5_z1;
static float dco2_7_z1;
static float dco2_11_z1;
static float dco2_state_z1;
static float dco2_12_z1, dco2_12_z2, dco2_12_z3, dco2_12_z4, dco2_12_z5, dco2_12_z6, dco2_12_z7;
static float dco2_13_z1, dco2_13_z2, dco2_13_z3, dco2_13_z4, dco2_13_z5, dco2_13_z6, dco2_13_z7;
static float dco2_14_z1, dco2_14_z2, dco2_14_z3, dco2_14_z4, dco2_14_z5, dco2_14_z6, dco2_14_z7;
static float dco2_15_z1, dco2_15_z2, dco2_15_z3, dco2_15_z4, dco2_15_z5, dco2_15_z6;
static float in_2_z1, in_2_z2, in_2_z3, in_2_z4;
static float lp_z1;
static float vcf_state_z1;
static float brilliance_3_z1;
static float brilliance_state_z1;
static float brilliance_state_2_z1;
static float brilliance_state_3_z1;
static float brilliance_state_4_z1;
static float brilliance_4_z1;
static float brilliance_5_z1;
static float cell_6_prev_z1;
static float state_z1;
static float state_2_z1;
static float state_3_z1;
static float brilliance_state_5_z1;
static float brilliance_5_prev_z1;
static float brilliance_state_6_z1;
static float brilliance_7_prev_z1;
static float brilliance_state_7_z1;
static float brilliance_state_8_z1;
static float dco_7_prev_z1;
static float dco_20_prev_z1;
static float dco2_16_prev_z1;
static float jx3p_voice_f0;
static inline __attribute__((always_inline)) void jx3p_voice(void)
{
    float lfo = KIN(IN_LFO);                                                                     /* 2 */
    float lfo_half = KIN(IN_LFO_HALF);                                                           /* 3 */
    float note_pitch = KIN(IN_NOTE_PITCH);                                                       /* 4 */
    float pitch_offset = KIN(IN_PITCH_OFFSET);                                                   /* 5 */
    float gate_word = KIN(IN_GATE_WORD);                                                         /* 6 */
    float env1 = KIN(IN_ENV1);                                                                   /* 7 */
    float env2 = KIN(IN_ENV2);                                                                   /* 8 */
    float iomd_60e0 = KIN(IN_IOMD_60E0);                                                         /* 9 */
    float iomd_60e2 = KIN(IN_IOMD_60E2);                                                         /* 10 */
    float dco = XFADE(dco_7_prev_z1, D_08d_3, KIN(0xa020));                                      /* 30 */
    float dco_2 = dco * KIN(IN_DCO2_CROSS_MOD);                                                  /* 34 */
    float cell = note_pitch + pitch_offset;                                                      /* 38 */
    float cell_3 = KP(0xc025) * lfo_half * KP(0xc024);                                           /* 47 */
    float dco_env1_mod_2 = KP(P_DCO_ENV1_MOD_2);                                                 /* 47 */
    float dco_env1_mod = KIN(IN_DCO_ENV1_MOD);                                                   /* 47 */
    float t50 = lfo * KP(P_DCO_LFO_MOD);                                                         /* 50 */
    float t50_2 = env1 * dco_env1_mod_2 * (1.0f - 2.0f * dco_env1_mod) + dco_env1_mod * dco_env1_mod_2; /* 50 */
    float cell_2 = XFADE(KP(0xc022), iomd_60e2, KIN(0xa02c)) * KP(0xc023);                       /* 56 */
    float dco_3 = cell_2 + (KP(P_DCO1_FREQ_MOD) * t50 + cell_3 + KP(P_DCO1_FREQ_MOD_1) * t50_2); /* 57 */
    float dco_4 = cell_2 + (KP(P_DCO2_FREQ_MOD) * t50 + cell_3 + KP(P_DCO2_FREQ_MOD_1) * t50_2); /* 58 */
    float dco2_cross_mod_2 = KIN(IN_DCO2_CROSS_MOD_2);                                           /* 58 */
    float t64 = cell + KIN(0xa024) + dco_3 + KIN(IN_DCO1_RANGE);                                 /* 64 */
    float t65 = cell + KIN(0xa025) + KP(P_DCO2_TUNE) + KP(P_DCO2_FINE_TUNE) + dco_4 + KIN(IN_DCO2_RANGE); /* 65 */
    float t69 = t64 + fmaxf(fminf(0.0003999998f * s1_floor(t64) - 0.0015999991f, 0.0009999999f), -9.999983e-05f); /* 69 */
    float dco1_waveform = KIN(IN_DCO1_WAVEFORM);                                                 /* 71 */
    float t72 = t65 + fmaxf(fminf(0.0003999998f * s1_floor(t65) - 0.0015999991f, 0.0009999999f), -9.999983e-05f); /* 72 */
    float dco_5 = fmaxf(fminf(t69, 10.0f), -4.0f);                                               /* 72 */
    float dco2_waveform = KIN(IN_DCO2_WAVEFORM);                                                 /* 72 */
    float dco2 = fmaxf(fminf(t72, 10.0f), -4.0f);                                                /* 74 */
    float dco_6 = dco_20_prev_z1 * KP(0xc032);                                                   /* 89 */
    float dco2_2 = dco2_16_prev_z1 * KP(0xc033);                                                 /* 89 */
    float dco1_prev = KP(P_DCO1_LEVEL) * dco_6 + KP(P_DCO2_LEVEL) * dco2_2;                      /* 92 */
    float dco_exp = K_EXP((dco_5 - 4.75f) * 0.6931472f);                                         /* 201 */
    float t206 = dco_exp * 0.0022916666f;                                                        /* 206 */
    float t208 = SEL(K_Z(t206), 2.3283064e-10f, fminf(t206, 0.249f));                            /* 208 */
    float dco_10 = s1_wrap(t208);                                                                /* 216 */
    float dco_8_prev = SEL(K_LT0(dco_10), dco_10, s1_wrap(dco_8_prev_z1 + t208));                /* 219 */
    float dco_recip = K_RECIP(t208 * 256.0f);                                                    /* 221 */
    float t237 = K_SIN(K_CLAMP(K_TRI(0.5f * dco_8_prev + 0.5f) * 256.0f * dco_recip * 0.25f) * 1.570796f); /* 237 */
    float dco_14 = (s1_wrap(dco_8_prev) + dco_8_prev) * 0.5f * t237;                             /* 238 */
    float t240 = dco_8_prev - dco_8_prev_z1;                                                     /* 240 */
    float t240_le0 = K_LE0(t240);                                                                /* 240 */
    float t240_gt0 = K_GT0(t240);                                                                /* 242 */
    float dco_7 = SEL(t240_gt0, 0.0f, SEL(t240_le0, 0.25f, dco_8_prev_z1));                      /* 242 */
    float dco_12 = fmaxf(SEL(t240_gt0, dco_11_z1, SEL(t240_le0, 1.6999998f, dco_7)) - 0.04f, 0.0f); /* 245 */
    float dco_10_2 = s1_wrap(dco_10 + t208);                                                     /* 246 */
    float dco_16 = XFADE(dco_14, fminf(dco_12, 1.0f), dco1_waveform);                            /* 250 */
    float dco_8_prev_2 = SEL(K_LT0(dco_10_2 - dco_10), dco_10_2, s1_wrap(dco_8_prev + t208));    /* 251 */
    float t263 = K_SIN(K_CLAMP(K_TRI(0.5f * dco_8_prev_2 + 0.5f) * 256.0f * dco_recip * 0.25f) * 1.570796f); /* 263 */
    float dco_14_2 = (s1_wrap(dco_8_prev_2) + dco_8_prev_2) * 0.5f * t263;                       /* 264 */
    float t266 = dco_8_prev_2 - dco_8_prev;                                                      /* 266 */
    float t266_le0 = K_LE0(t266);                                                                /* 266 */
    float t266_gt0 = K_GT0(t266);                                                                /* 268 */
    float dco_7_2 = SEL(t266_gt0, dco_7, SEL(t266_le0, 0.5f, dco_8_prev));                       /* 268 */
    float dco_12_2 = fmaxf(SEL(t266_gt0, dco_12, SEL(t266_le0, 1.6999998f, dco_7_2)) - 0.04f, 0.0f); /* 271 */
    float dco_10_3 = s1_wrap(dco_10_2 + t208);                                                   /* 272 */
    float dco_17 = XFADE(dco_14_2, fminf(dco_12_2, 1.0f), dco1_waveform);                        /* 276 */
    float dco_8_prev_3 = SEL(K_LT0(dco_10_3 - dco_10_2), dco_10_3, s1_wrap(dco_8_prev_2 + t208)); /* 277 */
    float t289 = K_SIN(K_CLAMP(K_TRI(0.5f * dco_8_prev_3 + 0.5f) * 256.0f * dco_recip * 0.25f) * 1.570796f); /* 289 */
    float dco_14_3 = (s1_wrap(dco_8_prev_3) + dco_8_prev_3) * 0.5f * t289;                       /* 290 */
    float t292 = dco_8_prev_3 - dco_8_prev_2;                                                    /* 292 */
    float t292_le0 = K_LE0(t292);                                                                /* 292 */
    float t292_gt0 = K_GT0(t292);                                                                /* 294 */
    float dco_7_3 = SEL(t292_gt0, dco_7_2, SEL(t292_le0, 0.75f, dco_8_prev_2));                  /* 294 */
    float dco_12_3 = fmaxf(SEL(t292_gt0, dco_12_2, SEL(t292_le0, 1.6999998f, dco_7_3)) - 0.04f, 0.0f); /* 297 */
    float dco_9 = s1_wrap(dco_10_3 + t208);                                                      /* 298 */
    float dco_18 = XFADE(dco_14_3, fminf(dco_12_3, 1.0f), dco1_waveform);                        /* 302 */
    float dco_8_prev_4 = SEL(K_LT0(dco_9 - dco_10_3), dco_9, s1_wrap(dco_8_prev_3 + t208));      /* 303 */
    float t310 = K_CLAMP(K_TRI(0.5f * dco_8_prev_4 + 0.5f) * 256.0f * dco_recip * 0.25f) * 1.570796f; /* 310 */
    float t311 = t310 * t310;                                                                    /* 311 */
    float dco_13 = (s1_wrap(dco_8_prev_4) + dco_8_prev_4) * 0.5f * K_SIN(t310);                  /* 316 */
    float t317 = dco_8_prev_4 - dco_8_prev_3;                                                    /* 317 */
    float dco_11 = fmaxf(SEL(K_GT0(t317), dco_12_3, SEL(K_LE0(t317), 1.6999998f, t311 * t311)) - 0.04f, 0.0f); /* 321 */
    float dco_19 = XFADE(dco_13, fminf(dco_11, 1.0f), dco1_waveform);                            /* 323 */
    float t326 = -(7.8084646e-05f * dco_17) - 7.8084646e-05f * dco_18_z7 - -9.239372e-05f * dco_18; /* 326 */
    float t327 = t326 - -9.239372e-05f * dco_17_z7 - -0.0011889678f * dco_19 - -0.0011889678f * dco_16_z7; /* 327 */
    float t329 = t327 - -0.0036064829f * dco_16_z1 - -0.0036064829f * dco_19_z6 - -0.0070102336f * dco_17_z1; /* 329 */
    float t330 = t329 - -0.0070102336f * dco_18_z6 - -0.010096775f * dco_18_z1 - -0.010096775f * dco_17_z6; /* 330 */
    float t332 = t330 - -0.01071596f * dco_19_z1 - -0.01071596f * dco_16_z6 - -0.006385348f * dco_16_z2; /* 332 */
    float t333 = t332 - -0.006385348f * dco_19_z5 - 0.0049214866f * dco_17_z2 - 0.0049214866f * dco_18_z5; /* 333 */
    float t335 = t333 - 0.023953114f * dco_18_z2 - 0.023953114f * dco_17_z5 - 0.04961065f * dco_19_z2; /* 335 */
    float t336 = t335 - 0.04961065f * dco_16_z5 - 0.07886231f * dco_16_z3 - 0.07886231f * dco_19_z4; /* 336 */
    float t338 = t336 - 0.107232735f * dco_17_z3 - 0.107232735f * dco_18_z4 - 0.12977375f * dco_18_z3; /* 338 */
    float dco_15 = t338 - 0.12977375f * dco_17_z4 - 0.14225915f * dco_19_z3 - 0.14225915f * dco_16_z4; /* 339 */
    float dco_state = 0.8532445f * dco_state_z1 + (0.44227272f * dco_15 - 0.29551718f * dco_15_z1); /* 341 */
    float dco_20 = XFADE(dco_state * -0.5f, dco_state * 0.9f, dco1_waveform);                    /* 344 */
    float dco2_exp = K_EXP((dco2 - 4.75f) * 0.6931472f);                                         /* 524 */
    float t529 = dco2_exp * 0.0022916666f;                                                       /* 529 */
    float t531 = SEL(K_Z(t529), 2.3283064e-10f, fminf(t529, 0.249f));                            /* 531 */
    float t534 = dco_2 - 0.25f;                                                                  /* 534 */
    float t537 = dco2_5_z1 * KIN(IN_DCO2_CROSS_MOD_1) * SEL(K_NZ(t534), 1.0f, dco2_cross_mod_2); /* 537 */
    float dco2_6 = s1_wrap(t537 + t531);                                                         /* 539 */
    float dco2_4_prev = SEL(K_LT0(dco2_6 - t537), dco2_6, s1_wrap(SEL(K_Z(t534), -1.0f, dco2_4_prev_z1) + t531)); /* 542 */
    float dco2_recip = K_RECIP(t531 * 256.0f);                                                   /* 544 */
    float t555 = K_CLAMP(K_TRI(0.5f * dco2_4_prev + 0.5f) * 256.0f * dco2_recip * 0.25f) * 1.570796f; /* 555 */
    float t556 = t555 * t555;                                                                    /* 556 */
    float dco2_10 = (s1_wrap(dco2_4_prev) + dco2_4_prev) * 0.5f * K_SIN(t555);                   /* 561 */
    float t561 = dco_2 - 0.5f;                                                                   /* 561 */
    float t563 = dco2_4_prev - dco2_4_prev_z1;                                                   /* 563 */
    float t561_z = K_Z(t561);                                                                    /* 563 */
    float t563_le0 = K_LE0(t563);                                                                /* 563 */
    float t564 = dco2_6 * SEL(t561_z, dco2_cross_mod_2, SEL(K_NZ(t561), 1.0f, t556 * t556));     /* 564 */
    float t563_gt0 = K_GT0(t563);                                                                /* 565 */
    float dco2_3 = SEL(t563_gt0, 0.0f, SEL(t563_le0, 0.25f, dco2_4_prev_z1));                    /* 565 */
    float dco2_8 = fmaxf(SEL(t563_gt0, dco2_7_z1, SEL(t563_le0, 1.6999998f, dco2_3)) - 0.04f, 0.0f); /* 568 */
    float dco2_6_2 = s1_wrap(t564 + t531);                                                       /* 569 */
    float dco2_12 = XFADE(dco2_10, fminf(dco2_8, 1.0f), dco2_waveform);                          /* 573 */
    float dco2_4_prev_2 = SEL(K_LT0(dco2_6_2 - t564), dco2_6_2, s1_wrap(SEL(t561_z, -1.0f, dco2_4_prev + t531))); /* 574 */
    float t581 = K_CLAMP(K_TRI(0.5f * dco2_4_prev_2 + 0.5f) * 256.0f * dco2_recip * 0.25f) * 1.570796f; /* 581 */
    float t582 = t581 * t581;                                                                    /* 582 */
    float dco2_10_2 = (s1_wrap(dco2_4_prev_2) + dco2_4_prev_2) * 0.5f * K_SIN(t581);             /* 587 */
    float t587 = dco_2 - 0.75f;                                                                  /* 587 */
    float t589 = dco2_4_prev_2 - dco2_4_prev;                                                    /* 589 */
    float t587_z = K_Z(t587);                                                                    /* 589 */
    float t589_le0 = K_LE0(t589);                                                                /* 589 */
    float t590 = dco2_6_2 * SEL(t587_z, dco2_cross_mod_2, SEL(K_NZ(t587), 1.0f, t582 * t582));   /* 590 */
    float t589_gt0 = K_GT0(t589);                                                                /* 591 */
    float dco2_3_2 = SEL(t589_gt0, dco2_3, SEL(t589_le0, 0.5f, dco2_4_prev));                    /* 591 */
    float dco2_8_2 = fmaxf(SEL(t589_gt0, dco2_8, SEL(t589_le0, 1.6999998f, dco2_3_2)) - 0.04f, 0.0f); /* 594 */
    float dco2_6_3 = s1_wrap(t590 + t531);                                                       /* 595 */
    float dco2_13 = XFADE(dco2_10_2, fminf(dco2_8_2, 1.0f), dco2_waveform);                      /* 599 */
    float dco2_4_prev_3 = SEL(K_LT0(dco2_6_3 - t590), dco2_6_3, s1_wrap(SEL(t587_z, -1.0f, dco2_4_prev_2 + t531))); /* 600 */
    float t607 = K_CLAMP(K_TRI(0.5f * dco2_4_prev_3 + 0.5f) * 256.0f * dco2_recip * 0.25f) * 1.570796f; /* 607 */
    float t608 = t607 * t607;                                                                    /* 608 */
    float dco2_10_3 = (s1_wrap(dco2_4_prev_3) + dco2_4_prev_3) * 0.5f * K_SIN(t607);             /* 613 */
    float t613 = dco_2 - 1.0f;                                                                   /* 613 */
    float t615 = dco2_4_prev_3 - dco2_4_prev_2;                                                  /* 615 */
    float t613_z = K_Z(t613);                                                                    /* 615 */
    float t615_le0 = K_LE0(t615);                                                                /* 615 */
    float t616 = dco2_6_3 * SEL(t613_z, dco2_cross_mod_2, SEL(K_NZ(t613), 1.0f, t608 * t608));   /* 616 */
    float t615_gt0 = K_GT0(t615);                                                                /* 617 */
    float dco2_3_3 = SEL(t615_gt0, dco2_3_2, SEL(t615_le0, 0.75f, dco2_4_prev_2));               /* 617 */
    float dco2_8_3 = fmaxf(SEL(t615_gt0, dco2_8_2, SEL(t615_le0, 1.6999998f, dco2_3_3)) - 0.04f, 0.0f); /* 620 */
    float dco2_5 = s1_wrap(t616 + t531);                                                         /* 621 */
    float dco2_14 = XFADE(dco2_10_3, fminf(dco2_8_3, 1.0f), dco2_waveform);                      /* 625 */
    float dco2_4_prev_4 = SEL(K_LT0(dco2_5 - t616), dco2_5, s1_wrap(SEL(t613_z, -1.0f, dco2_4_prev_3 + t531))); /* 626 */
    float t633 = K_CLAMP(K_TRI(0.5f * dco2_4_prev_4 + 0.5f) * 256.0f * dco2_recip * 0.25f) * 1.570796f; /* 633 */
    float t634 = t633 * t633;                                                                    /* 634 */
    float dco2_9 = (s1_wrap(dco2_4_prev_4) + dco2_4_prev_4) * 0.5f * K_SIN(t633);                /* 639 */
    float t640 = dco2_4_prev_4 - dco2_4_prev_3;                                                  /* 640 */
    float dco2_7 = fmaxf(SEL(K_GT0(t640), dco2_8_3, SEL(K_LE0(t640), 1.6999998f, t634 * t634)) - 0.04f, 0.0f); /* 644 */
    float dco2_15 = XFADE(dco2_9, fminf(dco2_7, 1.0f), dco2_waveform);                           /* 646 */
    float t649 = -(7.8084646e-05f * dco2_13) - 7.8084646e-05f * dco2_14_z7 - -9.239372e-05f * dco2_14; /* 649 */
    float t650 = t649 - -9.239372e-05f * dco2_13_z7 - -0.0011889678f * dco2_15 - -0.0011889678f * dco2_12_z7; /* 650 */
    float t652 = t650 - -0.0036064829f * dco2_12_z1 - -0.0036064829f * dco2_15_z6 - -0.0070102336f * dco2_13_z1; /* 652 */
    float t653 = t652 - -0.0070102336f * dco2_14_z6 - -0.010096775f * dco2_14_z1 - -0.010096775f * dco2_13_z6; /* 653 */
    float t655 = t653 - -0.01071596f * dco2_15_z1 - -0.01071596f * dco2_12_z6 - -0.006385348f * dco2_12_z2; /* 655 */
    float t656 = t655 - -0.006385348f * dco2_15_z5 - 0.0049214866f * dco2_13_z2 - 0.0049214866f * dco2_14_z5; /* 656 */
    float t658 = t656 - 0.023953114f * dco2_14_z2 - 0.023953114f * dco2_13_z5 - 0.04961065f * dco2_15_z2; /* 658 */
    float t659 = t658 - 0.04961065f * dco2_12_z5 - 0.07886231f * dco2_12_z3 - 0.07886231f * dco2_15_z4; /* 659 */
    float t661 = t659 - 0.107232735f * dco2_13_z3 - 0.107232735f * dco2_14_z4 - 0.12977375f * dco2_14_z3; /* 661 */
    float dco2_11 = t661 - 0.12977375f * dco2_13_z4 - 0.14225915f * dco2_15_z3 - 0.14225915f * dco2_12_z4; /* 662 */
    float dco2_state = 0.8532445f * dco2_state_z1 + (0.44227272f * dco2_11 - 0.29551718f * dco2_11_z1); /* 664 */
    float dco2_16 = XFADE(dco2_state * -0.5f, dco2_state * 0.9f, dco2_waveform);                 /* 667 */
    float t881 = XFADE(KP(P_VCF_CUTOFF_FREQ), D_036_1, KIN(0xa061));                             /* 881 */
    float t882 = t881 * t881;                                                                    /* 882 */
    float vcf = fminf(fmaxf(0.65628016f * t881 + t882 * 0.6643478f + t882 * t881 * -0.32077295f, 0.0f), 1.0f); /* 887 */
    float vcf_resonance = KP(P_VCF_RESONANCE);                                                   /* 889 */
    float lp = LERP(lp_z1, KIN(0xa062), 0.64287853f);                                            /* 895 */
    float cell_4 = D_01e_1 + D_020_1;                                                            /* 898 */
    float cell_5 = XFADE(D_024_1, in_2_z2, KIN(0xa063));                                         /* 904 */
    float in_a065 = KIN(0xa065);                                                                 /* 910 */
    float vcf_env1_mod_2 = KP(P_VCF_ENV1_MOD_2);                                                 /* 913 */
    float vcf_env1_mod = KIN(IN_VCF_ENV1_MOD);                                                   /* 913 */
    float t916 = XFADE(D_01c_1, in_2_z4, in_a065) * (KP(0xc125) * KP(0xc124));                   /* 916 */
    float t916_2 = cell_5 * vcf_env1_mod_2 * (1.0f - 2.0f * vcf_env1_mod) + vcf_env1_mod_2 * vcf_env1_mod; /* 916 */
    float t918 = XFADE(D_01a_1, in_2_z4, in_a065) * KP(P_VCF_LFO_MOD) + t916 + t916_2 + (cell_4 - 4.0f) * KP(P_VCF_PITCH_FOLLOW); /* 918 */
    float t920 = KP(0xc123) * XFADE(KP(0xc122), D_038_1, KIN(0xa064));                           /* 920 */
    float vcf_2 = 8.0f * ((lp + KP(0xc12a)) * KP(0xc129)) + t918 + t920;                         /* 920 */
    float vcf_state_prev = vcf_2 * 0.0065235896f + 0.9934764f * vcf_state_z1;                    /* 922 */
    float vcf_3 = 6.0f * vcf_resonance + KP(0xc12c);                                             /* 932 */
    float t934 = fminf((1.0f - vcf_resonance) * 0.5f, 0.075f) - 0.7f + (11.6f * vcf + (KP(P_BRILLIANCE) + KP(0xc12b)) + vcf_state_prev); /* 934 */
    float t935 = 11.5f - t934;                                                                   /* 935 */
    float t941 = 0.6931472f * fminf(fmaxf(SEL(K_LT0(t935), 11.5f - 0.19999981f * t935, t934), -3.0f), 11.5f); /* 941 */
    float t950 = K_EXP(t941 - 3.292449f) * 0.014398932f;                                         /* 950 */
    float t951 = t950 * t950;                                                                    /* 951 */
    float t953 = t951 * t951;                                                                    /* 953 */
    float t955 = K_SIN(t950);                                                                    /* 955 */
    float t958 = t953 * (0.041666627f - 0.0013888478f * t951 + t953 * (2.4795532e-05f - 2.3841858e-07f * t951)); /* 958 */
    float t959 = K_RECIP(1.0f - 0.5f * t951 + t958);                                             /* 959 */
    float brilliance_2 = fminf(t955 * t959 * K_RECIP(t959 * t955 + 1.0f), 1.0f);                 /* 969 */
    float t976 = vcf_3 * 0.675f;                                                                 /* 976 */
    float t977 = brilliance_2 * brilliance_2;                                                    /* 977 */
    float t980 = K_RECIP(1.0f + t976 * (t977 * t977));                                           /* 980 */
    float t988 = 1.0f - 2.0f * brilliance_2;                                                     /* 988 */
    float brilliance_3 = K_CLAMP(dco1_z1 * 0.29999924f * t980 - t980 * t976 * (0.071428575f * K_CLAMP(brilliance_4_z1 * 14.0f))); /* 989 */
    float brilliance_state = brilliance_2 * (brilliance_3 + brilliance_3_z1) + t988 * brilliance_state_z1; /* 993 */
    float brilliance_state_2 = brilliance_2 * (brilliance_state + brilliance_state_z1) + t988 * brilliance_state_2_z1; /* 995 */
    float brilliance_state_3 = brilliance_2 * (brilliance_state_2 + brilliance_state_2_z1) + t988 * brilliance_state_3_z1; /* 997 */
    float t998 = brilliance_2 * (brilliance_2 * (brilliance_2 * brilliance_3 + brilliance_state + t988 * brilliance_state) + brilliance_state_2 + t988 * brilliance_state_2) + brilliance_state_3; /* 998 */
    float brilliance_state_4 = brilliance_2 * (brilliance_state_3 + brilliance_state_3_z1) + t988 * brilliance_state_4_z1; /* 999 */
    float brilliance_4 = brilliance_2 * (t998 + t988 * brilliance_state_3) + t988 * brilliance_state_4; /* 1000 */
    float brilliance_5 = brilliance_state_4 * 0.32999992f;                                       /* 1004 */
    float c_27b = KP(0xc12e);                                                                    /* 1008 */
    float cell_6 = XFADE(KP(0xc130), KIN(0xa069), KP(0xc12f));                                   /* 1015 */
    float t1023 = cell_6 - state_2_z1;                                                           /* 1023 */
    float state = SEL(K_NZ(cell_6 - cell_6_prev_z1), t1023, state_z1);                           /* 1023 */
    float t1025 = s1_abs(state) * 0.0035899878f;                                                 /* 1025 */
    float state_2 = SEL(K_GE0(t1023), fminf(state_2_z1 + t1025, cell_6), fmaxf(state_2_z1 - t1025, cell_6)); /* 1029 */
    float state_3 = fmaxf(XFADE(state_3_z1, KIN(0xa06a), 0.0034637451f), 0.0f);                  /* 1035 */
    float brilliance_state_6 = brilliance_5 * 0.9998364f - 0.9998364f * brilliance_5_prev_z1 + 0.99967283f * brilliance_state_6_z1; /* 1041 */
    float brilliance_6 = XFADE(KIN(0xa06d) * D_022_1 + (KIN(0xa06c) * D_026_1 + D_024_1 * KIN(0xa06b)), brilliance_5_z1, KIN(0xa06e)); /* 1047 */
    float brilliance_state_5 = 0.9869955f * brilliance_state_5_z1 + brilliance_6 * 0.013004484f; /* 1050 */
    float t1067 = fmaxf(SEL(K_GE0(brilliance_state_5), brilliance_state_5, SEL(K_LT0(brilliance_state_5), brilliance_state_5, 0.0f)), 0.0f) * (1.0f + KP(0xc131)); /* 1067 */
    float brilliance_7 = fminf((brilliance_state_6 + vcf_resonance * 0.19999981f * brilliance_state_6) * t1067, 5.5f) * 0.5999994f; /* 1070 */
    float brilliance_state_7 = 0.76939964f * brilliance_state_7_z1 + (brilliance_7 * 3.6373444f - 3.4067383f * brilliance_7_prev_z1); /* 1075 */
    float brilliance_state_8 = 0.9836874f * brilliance_state_8_z1 + (brilliance_7 * 1.024313f - 0.9593744f * brilliance_7_prev_z1); /* 1075 */
    float brilliance_8 = SEL(K_GE0(c_27b), XFADE(brilliance_7, brilliance_state_7, fmaxf(c_27b, 0.0f)), XFADE(brilliance_7, brilliance_state_8, fmaxf(-c_27b, 0.0f))); /* 1079 */
    float brilliance_9 = brilliance_8 * state_2;                                                 /* 1082 */
    float brilliance_10 = brilliance_9 * state_3;                                                /* 1085 */
    KOUT(0x231e, brilliance_10);

    jx3p_voice_f0 = jx3p_voice_f0;
    D_01a_1 = lfo;
    D_01c_1 = lfo_half;
    D_01e_1 = note_pitch;
    D_020_1 = pitch_offset;
    D_022_1 = gate_word;
    D_024_1 = env1;
    D_026_1 = env2;
    D_036_1 = iomd_60e0;
    dco1_z1 = dco1_prev;
    D_08d_3 = D_08d_2;
    D_08d_2 = D_08d_1;
    D_08d_1 = iomd_60e2;
    dco_8_prev_z1 = dco_8_prev_4;
    dco_11_z1 = dco_11;
    dco_15_z1 = dco_15;
    dco_state_z1 = dco_state;
    dco_16_z7 = dco_16_z6;
    dco_16_z6 = dco_16_z5;
    dco_16_z5 = dco_16_z4;
    dco_16_z4 = dco_16_z3;
    dco_16_z3 = dco_16_z2;
    dco_16_z2 = dco_16_z1;
    dco_16_z1 = dco_16;
    dco_17_z7 = dco_17_z6;
    dco_17_z6 = dco_17_z5;
    dco_17_z5 = dco_17_z4;
    dco_17_z4 = dco_17_z3;
    dco_17_z3 = dco_17_z2;
    dco_17_z2 = dco_17_z1;
    dco_17_z1 = dco_17;
    dco_18_z7 = dco_18_z6;
    dco_18_z6 = dco_18_z5;
    dco_18_z5 = dco_18_z4;
    dco_18_z4 = dco_18_z3;
    dco_18_z3 = dco_18_z2;
    dco_18_z2 = dco_18_z1;
    dco_18_z1 = dco_18;
    dco_19_z6 = dco_19_z5;
    dco_19_z5 = dco_19_z4;
    dco_19_z4 = dco_19_z3;
    dco_19_z3 = dco_19_z2;
    dco_19_z2 = dco_19_z1;
    dco_19_z1 = dco_19;
    dco2_4_prev_z1 = dco2_4_prev_4;
    dco2_5_z1 = dco2_5;
    dco2_7_z1 = dco2_7;
    dco2_11_z1 = dco2_11;
    dco2_state_z1 = dco2_state;
    dco2_12_z7 = dco2_12_z6;
    dco2_12_z6 = dco2_12_z5;
    dco2_12_z5 = dco2_12_z4;
    dco2_12_z4 = dco2_12_z3;
    dco2_12_z3 = dco2_12_z2;
    dco2_12_z2 = dco2_12_z1;
    dco2_12_z1 = dco2_12;
    dco2_13_z7 = dco2_13_z6;
    dco2_13_z6 = dco2_13_z5;
    dco2_13_z5 = dco2_13_z4;
    dco2_13_z4 = dco2_13_z3;
    dco2_13_z3 = dco2_13_z2;
    dco2_13_z2 = dco2_13_z1;
    dco2_13_z1 = dco2_13;
    dco2_14_z7 = dco2_14_z6;
    dco2_14_z6 = dco2_14_z5;
    dco2_14_z5 = dco2_14_z4;
    dco2_14_z4 = dco2_14_z3;
    dco2_14_z3 = dco2_14_z2;
    dco2_14_z2 = dco2_14_z1;
    dco2_14_z1 = dco2_14;
    dco2_15_z6 = dco2_15_z5;
    dco2_15_z5 = dco2_15_z4;
    dco2_15_z4 = dco2_15_z3;
    dco2_15_z3 = dco2_15_z2;
    dco2_15_z2 = dco2_15_z1;
    dco2_15_z1 = dco2_15;
    in_2_z4 = in_2_z3;
    in_2_z3 = in_2_z2;
    in_2_z2 = in_2_z1;
    in_2_z1 = D_038_1;
    D_038_1 = iomd_60e2;
    lp_z1 = lp;
    vcf_state_z1 = vcf_state_prev;
    brilliance_3_z1 = brilliance_3;
    brilliance_state_z1 = brilliance_state;
    brilliance_state_2_z1 = brilliance_state_2;
    brilliance_state_3_z1 = brilliance_state_3;
    brilliance_state_4_z1 = brilliance_state_4;
    brilliance_4_z1 = brilliance_4;
    brilliance_5_z1 = brilliance_5;
    cell_6_prev_z1 = cell_6;
    state_z1 = state;
    state_2_z1 = state_2;
    state_3_z1 = state_3;
    brilliance_state_5_z1 = brilliance_state_5;
    brilliance_5_prev_z1 = brilliance_5;
    brilliance_state_6_z1 = brilliance_state_6;
    brilliance_7_prev_z1 = brilliance_7;
    brilliance_state_7_z1 = brilliance_state_7;
    brilliance_state_8_z1 = brilliance_state_8;
    dco_7_prev_z1 = dco_7_3;
    dco_20_prev_z1 = dco_20;
    dco2_16_prev_z1 = dco2_16;
}

