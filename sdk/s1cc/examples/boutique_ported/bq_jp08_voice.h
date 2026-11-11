/* bq_jp08_voice.h: the JP08 voice (Boutique 1.21 PX-11:Appli, DSP chip 0: pre 0x1000, voice A 0x1900, OSC1 variant 2, OSC2 variant 3) as C, the model's 59 immediate patches and 3 rate conversions (44.1 -> 96 kHz) applied. Written by tools/make_voice.py with sdk/s1cc/tools/dsplift.py; do not edit: README.md §2. */
#include "dsplift_k.h"

/* jp08_voice: written by sdk/s1cc/tools/dsplift.py --kernel --split 396. Every operation of the program, in order, as
 * expressions (dsplift.md §6): a name per value that is used more than once, stored in bank 0 or too long;
 * the number after each line = the instruction that makes it. SEL(w, a, b) = w ? a : b (w = 0 / 1), LERP(a, b, t) = a + t (b - a),
 * K_NZ / K_GE0 / K_LE0 = the condition bits, K_RECIP = the reciprocal (dsplift_k.h). Delay chains: X_zK =
 * what cell X held K samples ago. */
#define OUT_VOICE 0x203a
#define IN_PITCH 0xa080
#define IN_GATE 0xa081
static float D_01f_1;   /* part A */
static float D_020_1;   /* part A */
static float D_024_1;   /* part A */
static float D_062_1;   /* part A */
static float D_067_1;   /* part A */
static float D_075_1;   /* part A */
static float D_077_1;   /* part A */
static float D_07b_1, D_07b_2;   /* part A */
static float D_082_1;   /* part A */
static float D_084_1;   /* part A */
static float D_088_1, D_088_2;   /* part A */
static float D_0b0_1;   /* part B */
static float D_0b7_1;   /* part B */
static float D_0b8_1;   /* part B */
static float D_0b9_1;   /* part B */
static float D_0ba_1;   /* part B */
static float D_0bb_1;   /* part B */
static float D_0bc_1, D_0bc_2, D_0bc_3;   /* part B */
static float D_0c8_1;   /* part B */
static float D_1f5_1;   /* part B */
static float D_1f9_1;   /* part B */
static float D_1fa_1;   /* part B */
static float D_1fc_1;   /* part B */
static float D_1fd_1;   /* part B */
static float D_201_1, D_201_2, D_201_3, D_201_4, D_201_5, D_201_6, D_201_7;   /* part B */
static float D_209_1, D_209_2, D_209_3, D_209_4, D_209_5, D_209_6, D_209_7;   /* part B */
static float D_211_1, D_211_2, D_211_3, D_211_4, D_211_5, D_211_6, D_211_7;   /* part B */
static float D_219_1, D_219_2, D_219_3, D_219_4, D_219_5, D_219_6, D_219_7;   /* part B */
static float D_257_1;   /* part A */
static float D_25b_1;   /* part A */
static float D_25c_1;   /* part A */
static float D_25e_1;   /* part A */
static float D_25f_1;   /* part A */
static float D_261_1, D_261_2, D_261_3, D_261_4, D_261_5, D_261_6, D_261_7;   /* part A */
static float D_269_1, D_269_2, D_269_3, D_269_4, D_269_5, D_269_6, D_269_7;   /* part A */
static float D_271_1, D_271_2, D_271_3, D_271_4, D_271_5, D_271_6, D_271_7;   /* part A */
static float D_279_1, D_279_2, D_279_3, D_279_4, D_279_5, D_279_6, D_279_7;   /* part A */
static float D_418_1;   /* part B */
static float D_41d_1;   /* part A */
/* the parts' arguments: what the including file's KIN / KP / KOUT use (default: none) */
#ifndef KARGS_A
#define KARGS_A void
#endif
#ifndef KARGS_B
#define KARGS_B void
#endif
/* handed from part A to part B: s1_export / s1_import slots 0.. (c_40a c_41e c_074 c_081 c_08e c_071) */
#define JP08_VOICE_SLOTS 6
static inline __attribute__((always_inline)) void jp08_voice_a(KARGS_A)
{
    float t14 = 0.1f * KP(0xc00b) + 0.9f;                                                        /* 14 */
    float c_01f = K_CLAMP(D_067_1);                                                              /* 14 */
    float c_037 = fminf(s1_floor(KP(0xc00a) * 65536.0f) * 2.3283064e-10f, 0.0071359314f);        /* 16 */
    float t17 = D_01f_1 - c_01f;                                                                 /* 17 */
    float t17_ge0 = K_GE0(t17);                                                                  /* 17 */
    float t17_lt0 = K_LT0(t17);                                                                  /* 19 */
    float c_020 = SEL(t17_ge0, K_CLAMP(t14 * D_020_1 + (1.0f - t14)), SEL(t17_lt0, 0.0f, D_024_1)); /* 19 */
    float t21 = fmaxf(5.0f * c_020 - 4.0f, 0.0f);                                                /* 21 */
    float c_036 = t21 * t21;                                                                     /* 22 */
    float c_024 = s1_wrap((c_037 + D_024_1) * SEL(t17_ge0, 1.0f, SEL(t17_lt0, 1.0f, D_01f_1)));  /* 26 */
    float t36 = s1_abs(s1_wrap(c_024));                                                          /* 36 */
    float t37 = t36 * t36;                                                                       /* 37 */
    float c_02e = (3.125f * t37 - 0.04159999f * t36 - 2.0799866f * (t37 * t36) - 0.5039997f) * 2.0f; /* 42 */
    float c_034 = c_02e * c_036;                                                                 /* 43 */
    float c_039 = c_034 * c_036;                                                                 /* 49 */
    float pitch = KIN(IN_PITCH);                                                                 /* 60 */
    float gate = KIN(IN_GATE);                                                                   /* 61 */
    float t67 = D_062_1 + 0.00074767857f;                                                        /* 67 */
    float c_062 = SEL(K_NZ(gate), fminf(t67, 0.0f), SEL(K_Z(gate), -1.0f, t67));                 /* 68 */
    float c_064 = s1_sign(c_062) + 1.0f;                                                         /* 70 */
    float b118_ge0 = K_GE0(SEL(K_LE0(D_07b_1 - D_07b_2), 0.0f, D_07b_1) - 0.8925f);              /* 118 */
    float c_077 = SEL(b118_ge0, 0.0f, SEL(K_LT0(c_064 - D_075_1 - 1.0f), D_077_1, 1.0f));        /* 121 */
    float t122 = c_064 - 1.0f;                                                                   /* 122 */
    float c_07e = 1.0f - SEL(K_GE0(t122), 0.0f, SEL(K_LT0(t122), 1.0f, c_064));                  /* 125 */
    float c_07e_le0 = K_LE0(c_07e);                                                              /* 125 */
    float t129 = 1.05f - D_07b_1;                                                                /* 129 */
    float t133 = fmaxf(1.0f - KP(0xc03d), 0.00011998415f);                                       /* 133 */
    float c_07f = KP(0xc03e) * 0.87f;                                                            /* 134 */
    float t137 = 0.02f * fmaxf(t133 + 0.01f, 1.0f) - 0.02f + c_07f;                              /* 137 */
    float t138 = SEL(K_GT0(c_077 * c_07e), 1.05f - (t129 - fmaxf(KP(0xc03c), 7.927418e-06f) * t129), D_07b_1) - t137; /* 138 */
    float t142 = fmaxf(0.01f * (t138 * t133) + t137 + t138 * 0.99f, 0.0f);                       /* 142 */
    float t143 = fmaxf(1.0f - KP(0xc03f), 0.00014305115f);                                       /* 143 */
    float t146 = 0.02f * fmaxf(t143 + 0.01f, 1.0f) - 0.0007080078f;                              /* 146 */
    float c_07b = SEL(c_07e_le0, fmaxf(LERP(t146, t142, 0.99f) + 0.01f * ((t142 - t146) * t143), 0.0f), t142); /* 153 */
    float c_081 = c_07b * c_07b * -0.09999484f * -10.0f;                                         /* 157 */
    float t165 = c_064 - D_082_1 - 1.0f;                                                         /* 165 */
    float b165_ge0 = K_GE0(SEL(K_LE0(D_088_1 - D_088_2), 0.0f, D_088_1) - 0.8925f);              /* 165 */
    float c_084 = SEL(b165_ge0, 0.0f, SEL(K_LT0(t165), D_084_1, SEL(K_GE0(t165), 1.0f, c_07e))); /* 168 */
    float t176 = 1.05f - D_088_1;                                                                /* 176 */
    float t180 = fmaxf(1.0f - KP(0xc041), 0.00011998415f);                                       /* 180 */
    float c_08c = KP(0xc042) * 0.87f;                                                            /* 181 */
    float t184 = 0.02f * fmaxf(t180 + 0.01f, 1.0f) - 0.02f + c_08c;                              /* 184 */
    float t185 = SEL(K_GT0(c_084 * c_07e), 1.05f - (t176 - fmaxf(KP(0xc040), 7.927418e-06f) * t176), D_088_1) - t184; /* 185 */
    float t189 = fmaxf(0.01f * (t185 * t180) + t184 + t185 * 0.99f, 0.0f);                       /* 189 */
    float t190 = fmaxf(1.0f - KP(0xc043), 0.00014305115f);                                       /* 190 */
    float t193 = 0.02f * fmaxf(t190 + 0.01f, 1.0f) - 0.0007080078f;                              /* 193 */
    float c_088 = SEL(c_07e_le0, fmaxf(LERP(t193, t189, 0.99f) + 0.01f * ((t189 - t193) * t190), 0.0f), t189); /* 200 */
    float c_08e = c_088 * c_088 * -0.09999484f * -10.0f;                                         /* 204 */
    float t214 = c_039 * 0.5f + 0.5f;                                                            /* 214 */
    float c_095 = KIN(0xa090) + (KIN(0xa093) * t214 + (KIN(0xa092) * c_081 + t214 * KIN(0xa091))); /* 218 */
    float c_094 = c_039 * KP(0xc04a) + c_081 * KP(0xc04b);                                       /* 222 */
    float c_093 = D_41d_1 * KP(0xc04d) + c_094;                                                  /* 224 */
    float c_410 = c_095 * KP(0xc054);                                                            /* 226 */
    float c_40a = pitch + c_093;                                                                 /* 231 */
    float c_40f = pitch + KP(0xc046) + c_094;                                                    /* 231 */
    float c_252 = K_EXP((c_40f - 4.75f) * 0.6931472f) * 0.2296875f;                              /* 257 */
    float t261 = c_252 * 0.004988662f;                                                           /* 261 */
    float t263 = SEL(K_Z(t261), 2.3283064e-10f, fminf(t261, 0.249f));                            /* 263 */
    float c_288 = K_RECIP(t263 * 256.0f) * 0.19999981f;                                          /* 269 */
    float c_281 = -0.8999996f * c_410 - 0.029999971f;                                            /* 270 */
    float c_25a = s1_wrap(t263);                                                                 /* 272 */
    float t273 = s1_wrap(D_257_1 + t263);                                                        /* 273 */
    float c_257 = SEL(K_LT0(t273), c_25a, t273);                                                 /* 275 */
    float t276 = c_257 + c_281;                                                                  /* 276 */
    float t288 = K_CLAMP(K_TRI(K_RECIP(c_281 + SEL(K_LT0(t276), -1.0f, 1.0f)) * t276) * 256.0f * c_288); /* 288 */
    float t291 = s1_wrap(c_257 + t263);                                                          /* 291 */
    float c_25a_2 = s1_wrap(c_25a + t263);                                                       /* 293 */
    float t296 = s1_sign(t276) * t288;                                                           /* 296 */
    float c_257_2 = SEL(K_LT0(c_25a_2 - c_25a), c_25a_2, SEL(K_Z(t291), 0.0f, t291));            /* 296 */
    float t299 = 1.5991099f * D_25e_1 + (0.07904296f * D_25b_1 - t296 * 0.03952148f + 0.03952148f * D_25c_1); /* 299 */
    float c_261 = t299 - 0.75719583f * D_25f_1;                                                  /* 299 */
    float t301 = c_257_2 + c_281;                                                                /* 301 */
    float t311 = K_TRI(K_RECIP(c_281 + SEL(K_LT0(t301), -1.0f, SEL(K_GE0(t301), 1.0f, D_25e_1))) * t301) * 256.0f; /* 311 */
    float t317 = s1_wrap(c_257_2 + t263);                                                        /* 317 */
    float c_25a_3 = s1_wrap(c_25a_2 + t263);                                                     /* 318 */
    float t321 = s1_sign(t301) * K_CLAMP(t311 * c_288);                                          /* 321 */
    float c_257_3 = SEL(K_LT0(c_25a_3 - c_25a_2), c_25a_3, SEL(K_Z(t317), 0.0f, t317));          /* 321 */
    float t324 = 1.5991099f * c_261 + (-(0.07904296f * t296) - t321 * 0.03952148f + 0.03952148f * D_25b_1); /* 324 */
    float c_269 = t324 - 0.75719583f * D_25e_1;                                                  /* 324 */
    float t326 = c_257_3 + c_281;                                                                /* 326 */
    float t336 = K_TRI(K_RECIP(c_281 + SEL(K_LT0(t326), -1.0f, SEL(K_GE0(t326), 1.0f, c_261))) * t326) * 256.0f; /* 336 */
    float t342 = s1_wrap(c_257_3 + t263);                                                        /* 342 */
    float c_259 = s1_wrap(c_25a_3 + t263);                                                       /* 343 */
    float t346 = s1_sign(t326) * K_CLAMP(t336 * c_288);                                          /* 346 */
    float c_25c = -t346;                                                                         /* 346 */
    float c_257_4 = SEL(K_LT0(c_259 - c_25a_3), c_259, SEL(K_Z(t342), 0.0f, t342));              /* 346 */
    float t349 = 1.5991099f * c_269 + (-(0.07904296f * t321) - t346 * 0.03952148f - 0.03952148f * t296); /* 349 */
    float c_271 = t349 - 0.75719583f * c_261;                                                    /* 349 */
    float t351 = c_257_4 + c_281;                                                                /* 351 */
    float t361 = K_TRI(K_RECIP(c_281 + SEL(K_LT0(t351), -1.0f, SEL(K_GE0(t351), 1.0f, c_269))) * t351) * 256.0f; /* 361 */
    float t369 = s1_sign(t351) * K_CLAMP(t361 * c_288);                                          /* 369 */
    float c_25b = -t369;                                                                         /* 369 */
    float t372 = 1.5991099f * c_271 + (-(0.07904296f * t346) - t369 * 0.03952148f - 0.03952148f * t321); /* 372 */
    float c_25e = t372 - 0.75719583f * c_269;                                                    /* 372 */
    float t375 = 0.0028439998f * c_269 + (-0.010209f * c_261 - 0.010209f * D_279_7) + 0.0028439998f * D_271_7; /* 375 */
    float t377 = 0.025467f * c_25e + (0.016618f * c_271 + t375 + 0.016618f * D_269_7) + 0.025467f * D_261_7; /* 377 */
    float t379 = 0.013325f * D_269_1 + (0.024861f * D_261_1 + t377 + 0.024861f * D_279_6) + 0.013325f * D_271_6; /* 379 */
    float t381 = t379 - 0.006489f * D_271_1 - 0.006489f * D_269_6 - 0.028085f * D_279_1 - 0.028085f * D_261_6; /* 381 */
    float t383 = t381 - 0.042798f * D_261_2 - 0.042798f * D_279_5 - 0.042425f * D_269_2 - 0.042425f * D_271_5; /* 383 */
    float t385 = 0.018176999f * D_279_2 + (t383 - 0.022002f * D_271_2 - 0.022002f * D_269_5);    /* 385 */
    float t387 = 0.128923f * D_269_3 + (0.072034f * D_261_3 + (t385 + 0.018176999f * D_261_5) + 0.072034f * D_279_4); /* 387 */
    float t389 = 0.203227f * D_279_3 + (0.176322f * D_271_3 + (t387 + 0.128923f * D_271_4) + 0.176322f * D_269_4); /* 389 */
    float c_41d = (t389 + 0.203227f * D_261_4) * -0.5f;                                          /* 390 */

    s1_export(0, c_40a);
    s1_export(1, D_41d_1);
    s1_export(2, c_039);
    s1_export(3, c_081);
    s1_export(4, c_08e);
    s1_export(5, pitch);
    D_01f_1 = c_01f;
    D_020_1 = c_020;
    D_024_1 = c_024;
    D_062_1 = c_062;
    D_067_1 = c_064;
    D_075_1 = c_064;
    D_077_1 = c_077;
    D_07b_2 = D_07b_1;
    D_07b_1 = c_07b;
    D_082_1 = c_064;
    D_084_1 = c_084;
    D_088_2 = D_088_1;
    D_088_1 = c_088;
    D_257_1 = c_257_4;
    D_25b_1 = c_25b;
    D_25c_1 = c_25c;
    D_25e_1 = c_25e;
    D_25f_1 = c_271;
    D_261_7 = D_261_6;
    D_261_6 = D_261_5;
    D_261_5 = D_261_4;
    D_261_4 = D_261_3;
    D_261_3 = D_261_2;
    D_261_2 = D_261_1;
    D_261_1 = c_261;
    D_269_7 = D_269_6;
    D_269_6 = D_269_5;
    D_269_5 = D_269_4;
    D_269_4 = D_269_3;
    D_269_3 = D_269_2;
    D_269_2 = D_269_1;
    D_269_1 = c_269;
    D_271_7 = D_271_6;
    D_271_6 = D_271_5;
    D_271_5 = D_271_4;
    D_271_4 = D_271_3;
    D_271_3 = D_271_2;
    D_271_2 = D_271_1;
    D_271_1 = c_271;
    D_279_7 = D_279_6;
    D_279_6 = D_279_5;
    D_279_5 = D_279_4;
    D_279_4 = D_279_3;
    D_279_3 = D_279_2;
    D_279_2 = D_279_1;
    D_279_1 = c_25e;
    D_41d_1 = c_41d;
}
static inline __attribute__((always_inline)) void jp08_voice_b(KARGS_B)
{
    float c_40a = s1_range(s1_import(0), -8.501f, 18.501f);                                      /* 400 */
    float c_1ef = K_EXP((c_40a - 4.75f) * 0.6931472f) * 0.2296875f;                              /* 411 */
    float t415 = c_1ef * 0.004988662f;                                                           /* 415 */
    float t417 = SEL(K_Z(t415), 2.3283064e-10f, fminf(t415, 0.249f));                            /* 417 */
    float c_1f8 = s1_wrap(t417);                                                                 /* 423 */
    float c_1f5 = SEL(K_LT0(c_1f8), c_1f8, s1_wrap(D_1f5_1 + t417));                             /* 426 */
    float c_1f1 = K_RECIP(t417 * 256.0f);                                                        /* 428 */
    float t444 = K_SIN(K_CLAMP(K_TRI(0.5f * c_1f5 + 0.5f) * 256.0f * c_1f1 * 0.25f) * 1.570796f); /* 444 */
    float c_1fa = (s1_wrap(c_1f5) + c_1f5) * 0.5f * t444;                                        /* 445 */
    float c_1f8_2 = s1_wrap(c_1f8 + t417);                                                       /* 447 */
    float t448 = s1_wrap(c_1f5 + t417);                                                          /* 448 */
    float t450 = 1.5688642f * D_1fc_1 + (0.054386564f * D_1fa_1 + (0.10877313f * D_1f9_1 + c_1fa * 0.054386564f)); /* 450 */
    float c_201 = t450 - 0.7864105f * D_1fd_1;                                                   /* 450 */
    float c_1f5_2 = SEL(K_LT0(t448), c_1f8_2, t448);                                             /* 451 */
    float t463 = K_SIN(K_CLAMP(K_TRI(0.5f * c_1f5_2 + 0.5f) * 256.0f * c_1f1 * 0.25f) * 1.570796f); /* 463 */
    float c_1fa_2 = (s1_wrap(c_1f5_2) + c_1f5_2) * 0.5f * t463;                                  /* 464 */
    float c_1f8_3 = s1_wrap(c_1f8_2 + t417);                                                     /* 466 */
    float t467 = s1_wrap(c_1f5_2 + t417);                                                        /* 467 */
    float t469 = 1.5688642f * c_201 + (0.054386564f * D_1f9_1 + (0.10877313f * c_1fa + c_1fa_2 * 0.054386564f)); /* 469 */
    float c_209 = t469 - 0.7864105f * D_1fc_1;                                                   /* 469 */
    float c_1f5_3 = SEL(K_LT0(t467), c_1f8_3, t467);                                             /* 470 */
    float t482 = K_SIN(K_CLAMP(K_TRI(0.5f * c_1f5_3 + 0.5f) * 256.0f * c_1f1 * 0.25f) * 1.570796f); /* 482 */
    float c_1fa_3 = (s1_wrap(c_1f5_3) + c_1f5_3) * 0.5f * t482;                                  /* 483 */
    float c_1f7 = s1_wrap(c_1f8_3 + t417);                                                       /* 485 */
    float t486 = s1_wrap(c_1f5_3 + t417);                                                        /* 486 */
    float t488 = 1.5688642f * c_209 + (0.054386564f * c_1fa + (0.10877313f * c_1fa_2 + c_1fa_3 * 0.054386564f)); /* 488 */
    float c_211 = t488 - 0.7864105f * c_201;                                                     /* 488 */
    float c_1f5_4 = SEL(K_LT0(t486), c_1f7, t486);                                               /* 489 */
    float t501 = K_SIN(K_CLAMP(K_TRI(0.5f * c_1f5_4 + 0.5f) * 256.0f * c_1f1 * 0.25f) * 1.570796f); /* 501 */
    float c_1f9 = (s1_wrap(c_1f5_4) + c_1f5_4) * 0.5f * t501;                                    /* 502 */
    float t505 = 1.5688642f * c_211 + (0.10877313f * c_1fa_3 + c_1f9 * 0.054386564f + 0.054386564f * c_1fa_2); /* 505 */
    float c_219 = t505 - 0.7864105f * c_209;                                                     /* 505 */
    float t508 = -(-0.00062781386f * c_201) - -0.00062781386f * D_219_7 - -0.00181091f * c_209;  /* 508 */
    float t509 = t508 - -0.00181091f * D_211_7 - -0.0025511528f * c_211 - -0.0025511528f * D_209_7; /* 509 */
    float t511 = t509 - -0.0015808088f * c_219 - -0.0015808088f * D_201_7 - 0.002359534f * D_201_1; /* 511 */
    float t512 = t511 - 0.002359534f * D_219_6 - 0.008297408f * D_209_1 - 0.008297408f * D_211_6; /* 512 */
    float t514 = t512 - 0.011753906f * D_211_1 - 0.011753906f * D_209_6 - 0.006730832f * D_219_1; /* 514 */
    float t515 = t514 - 0.006730832f * D_201_6 - -0.009135877f * D_201_2 - -0.009135877f * D_219_5; /* 515 */
    float t517 = t515 - -0.029605705f * D_209_2 - -0.029605705f * D_211_5 - -0.03964878f * D_211_2; /* 517 */
    float t518 = t517 - -0.03964878f * D_209_5 - -0.022207731f * D_219_2 - -0.022207731f * D_201_5; /* 518 */
    float t520 = t518 - 0.030897303f * D_201_3 - 0.030897303f * D_219_4 - 0.110675454f * D_209_3; /* 520 */
    float t521 = t520 - 0.110675454f * D_211_4 - 0.19164494f * D_211_3 - 0.19164494f * D_209_4;  /* 521 */
    float c_1ff = t521 - 0.24270383f * D_219_3 - 0.24270383f * D_201_4;                          /* 522 */
    float c_418 = c_1ff * -0.5f;                                                                 /* 524 */
    float c_41e = s1_range(s1_import(1), -3.001f, 3.001f);                                       /* 530 */
    float c_0a9 = KP(0xc05a) * D_418_1 + KP(0xc05b) * c_41e;                                     /* 540 */
    float c_0ad = KP(0xc060);                                                                    /* 544 */
    float c_039 = s1_range(s1_import(2), -2.251f, 2.251f);                                       /* 546 */
    float c_081 = s1_range(s1_import(3), -0.376f, 0.376f);                                       /* 546 */
    float c_08e = s1_range(s1_import(4), -0.376f, 0.376f);                                       /* 547 */
    float pitch = s1_range(s1_import(5), -5.501f, 15.501f);                                      /* 547 */
    float c_0af = c_081 * KP(0xc065) + ((pitch - 4.0f) * KP(0xc066) + KP(0xc064) * c_039);       /* 554 */
    float c_0b1 = c_0a9 - D_0b0_1;                                                               /* 558 */
    float c_0b0 = LERP(D_0b0_1, c_0a9, 7.387803e-05f);                                           /* 559 */
    float c_0b3 = 6.0f * c_0ad;                                                                  /* 566 */
    float t568 = fminf((1.0f - c_0ad) * 0.5f, 0.075f) + 0.16659999f + (10.625f * KP(0xc05f) + c_0af); /* 568 */
    float t569 = t568 * 0.25f;                                                                   /* 569 */
    float t570 = t569 * t569;                                                                    /* 570 */
    float t571 = t570 * t569;                                                                    /* 571 */
    float t572 = t568 - 11.0f;                                                                   /* 572 */
    float t573 = 2.3801785f * (t571 * t569) + (54.536343f * t570 - 48.64617f * t569 - 19.821596f * t571); /* 573 */
    float t574 = SEL(K_GE0(t572), t573 * 4.0f, SEL(K_LT0(t572), t568, t573));                    /* 574 */
    float c_0b5 = fmaxf(SEL(K_GT0(t572), LERP(11.0f, t568, 0.0009999871f), t574), -3.0f);        /* 576 */
    float t586 = K_EXP(0.6931472f * c_0b5 - 3.292449f) * 0.0035997033f;                          /* 586 */
    float t587 = t586 * t586;                                                                    /* 587 */
    float t589 = t587 * t587;                                                                    /* 589 */
    float t591 = K_SIN(t586);                                                                    /* 591 */
    float t594 = t589 * (0.041666627f - 0.0013888478f * t587 + t589 * (2.4795532e-05f - 2.3841858e-07f * t587)); /* 594 */
    float t595 = K_RECIP(1.0f - 0.5f * t587 + t594);                                             /* 595 */
    float c_0b2 = t591 * t595 * K_RECIP(t595 * t591 + 1.0f);                                     /* 604 */
    float c_0b6 = c_0b2 * 2.0f;                                                                  /* 612 */
    float t612 = c_0b3 * 0.8999996f;                                                             /* 612 */
    float t613 = c_0b6 * c_0b6;                                                                  /* 613 */
    float t616 = K_RECIP(1.0f + t612 * (t613 * t613));                                           /* 616 */
    float t624 = 1.0f - 2.0f * c_0b6;                                                            /* 624 */
    float t625 = K_CLAMP(c_0b1 * 0.119999945f * t616 - t616 * t612 * K_CLAMP((D_0bc_1 + 0.1f * D_0bc_3) * 0.6f)); /* 625 */
    float c_0b7 = t625 * -0.33333302f * (t625 * t625) + t625;                                    /* 627 */
    float c_0b8 = c_0b6 * (c_0b7 + D_0b7_1) + t624 * D_0b8_1;                                    /* 629 */
    float c_0b9 = c_0b6 * (c_0b8 + D_0b8_1) + t624 * D_0b9_1;                                    /* 631 */
    float c_0ba = c_0b6 * (c_0b9 + D_0b9_1) + t624 * D_0ba_1;                                    /* 633 */
    float t634 = c_0b6 * (c_0b6 * (c_0b6 * c_0b7 + c_0b8 + t624 * c_0b8) + c_0b9 + t624 * c_0b9) + c_0ba; /* 634 */
    float c_0bb = c_0b6 * (c_0ba + D_0ba_1) + t624 * D_0bb_1;                                    /* 635 */
    float c_0bc = c_0b6 * (t634 + t624 * c_0ba) + t624 * c_0bb;                                  /* 636 */
    float c_0c8 = fmaxf(0.0075531006f + D_0c8_1 - 0.0075531006f * D_0c8_1, 0.0f);                /* 648 */
    float t666 = fmaxf(0.444274f * (c_08e * c_08e * c_08e) - 0.005f + 0.7f * (c_08e * 0.265983f), 0.0f); /* 666 */
    float c_0cf = fminf((c_0bb + c_0ad * 0.5f * c_0bb) * t666, 5.5f) * 1.1999998f;               /* 671 */
    float c_43a = c_0cf * c_0c8;                                                                 /* 677 */
    KOUT(OUT_VOICE, c_43a);

    D_0b0_1 = c_0b0;
    D_0b7_1 = c_0b7;
    D_0b8_1 = c_0b8;
    D_0b9_1 = c_0b9;
    D_0ba_1 = c_0ba;
    D_0bb_1 = c_0bb;
    D_0bc_3 = D_0bc_2;
    D_0bc_2 = D_0bc_1;
    D_0bc_1 = c_0bc;
    D_0c8_1 = c_0c8;
    D_1f5_1 = c_1f5_4;
    D_1f9_1 = c_1f9;
    D_1fa_1 = c_1fa_3;
    D_1fc_1 = c_219;
    D_1fd_1 = c_211;
    D_201_7 = D_201_6;
    D_201_6 = D_201_5;
    D_201_5 = D_201_4;
    D_201_4 = D_201_3;
    D_201_3 = D_201_2;
    D_201_2 = D_201_1;
    D_201_1 = c_201;
    D_209_7 = D_209_6;
    D_209_6 = D_209_5;
    D_209_5 = D_209_4;
    D_209_4 = D_209_3;
    D_209_3 = D_209_2;
    D_209_2 = D_209_1;
    D_209_1 = c_209;
    D_211_7 = D_211_6;
    D_211_6 = D_211_5;
    D_211_5 = D_211_4;
    D_211_4 = D_211_3;
    D_211_3 = D_211_2;
    D_211_2 = D_211_1;
    D_211_1 = c_211;
    D_219_7 = D_219_6;
    D_219_6 = D_219_5;
    D_219_5 = D_219_4;
    D_219_4 = D_219_3;
    D_219_3 = D_219_2;
    D_219_2 = D_219_1;
    D_219_1 = c_219;
    D_418_1 = c_418;
}

