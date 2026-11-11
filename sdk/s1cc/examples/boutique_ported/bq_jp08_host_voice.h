/* bq_jp08_host_voice.h: the JP08 voice (Boutique 1.21 PX-11:Appli, DSP chip 0: voice A 0x1900, OSC1 variant 2, OSC2 variant 3) as C, the model's 59 immediate patches and 3 rate conversions (44.1 -> 96 kHz) applied; envelopes and LFO from the host (--host, README.md §8). Written by tools/make_voice.py with sdk/s1cc/tools/dsplift.py; do not edit: README.md §2. */
#include "dsplift_k.h"

/* jp08_host_voice: written by sdk/s1cc/tools/dsplift.py --kernel. Every operation of the program, in order, as
 * expressions (dsplift.md §6): a name per value that is used more than once, stored in bank 0 or too long;
 * the number after each line = the instruction that makes it. SEL(w, a, b) = w ? a : b (w = 0 / 1), LERP(a, b, t) = a + t (b - a),
 * K_NZ / K_GE0 / K_LE0 = the condition bits, K_RECIP = the reciprocal (dsplift_k.h). Delay chains: X_zK =
 * what cell X held K samples ago. */
#define OUT_VOICE 0x203a
#define IN_PITCH 0xa080
#define IN_PWM 0xa090
#define IN_ENV_FILTER 0xa0f0
#define IN_ENV_AMP 0xa0f1
#define IN_LFO_FILTER 0xa0f2
static float D_0b0_1;
static float D_0b7_1;
static float D_0b8_1;
static float D_0b9_1;
static float D_0ba_1;
static float D_0bb_1;
static float D_0bc_1, D_0bc_2, D_0bc_3;
static float D_0c8_1;
static float D_1f5_1;
static float D_1f7_1;
static float D_1f9_1;
static float D_1fa_1;
static float D_1fc_1;
static float D_1fd_1;
static float D_201_1, D_201_2, D_201_3, D_201_4, D_201_5, D_201_6, D_201_7;
static float D_209_1, D_209_2, D_209_3, D_209_4, D_209_5, D_209_6, D_209_7;
static float D_211_1, D_211_2, D_211_3, D_211_4, D_211_5, D_211_6, D_211_7;
static float D_219_1, D_219_2, D_219_3, D_219_4, D_219_5, D_219_6, D_219_7;
static float D_257_1;
static float D_259_1;
static float D_25b_1;
static float D_25c_1;
static float D_25e_1;
static float D_25f_1;
static float D_261_1, D_261_2, D_261_3, D_261_4, D_261_5, D_261_6, D_261_7;
static float D_269_1, D_269_2, D_269_3, D_269_4, D_269_5, D_269_6, D_269_7;
static float D_271_1, D_271_2, D_271_3, D_271_4, D_271_5, D_271_6, D_271_7;
static float D_279_1, D_279_2, D_279_3, D_279_4, D_279_5, D_279_6, D_279_7;
static float D_418_1;
static float D_41d_1;
static inline __attribute__((always_inline)) void jp08_host_voice(void)
{
    float pitch = KIN(IN_PITCH);                                                                 /* 1 */
    float env_filter = KIN(IN_ENV_FILTER);                                                       /* 95 */
    float c_081 = env_filter * env_filter * -0.09999484f * -10.0f;                               /* 98 */
    float env_amp = KIN(IN_ENV_AMP);                                                             /* 142 */
    float c_08e = env_amp * env_amp * -0.09999484f * -10.0f;                                     /* 145 */
    float c_093 = D_41d_1 * KP(0xc04d);                                                          /* 161 */
    float c_40b = KIN(IN_PWM) * 1.2f;                                                            /* 166 */
    float c_40f = pitch + KP(0xc046);                                                            /* 171 */
    float c_40a = pitch + c_093;                                                                 /* 172 */
    float c_0a9 = KP(0xc05a) * D_418_1 + KP(0xc05b) * D_41d_1;                                   /* 195 */
    float c_0ad = KP(0xc060);                                                                    /* 199 */
    float c_0af = c_081 * KP(0xc065) + ((pitch - 4.0f) * KP(0xc066) + KIN(IN_LFO_FILTER));       /* 209 */
    float c_0b1 = c_0a9 - D_0b0_1;                                                               /* 213 */
    float c_0b0 = LERP(D_0b0_1, c_0a9, 7.387803e-05f);                                           /* 214 */
    float c_0b3 = 6.0f * c_0ad;                                                                  /* 221 */
    float t223 = fminf((1.0f - c_0ad) * 0.5f, 0.075f) + 0.16659999f + (10.625f * KP(0xc05f) + c_0af); /* 223 */
    float t224 = t223 * 0.25f;                                                                   /* 224 */
    float t225 = t224 * t224;                                                                    /* 225 */
    float t226 = t225 * t224;                                                                    /* 226 */
    float t227 = t223 - 11.0f;                                                                   /* 227 */
    float t228 = 2.3801785f * (t226 * t224) + (54.536343f * t225 - 48.64617f * t224 - 19.821596f * t226); /* 228 */
    float t229 = SEL(K_GE0(t227), t228 * 4.0f, SEL(K_LT0(t227), t223, t228));                    /* 229 */
    float c_0b5 = fmaxf(SEL(K_GT0(t227), LERP(11.0f, t223, 0.0009999871f), t229), -3.0f);        /* 231 */
    float t241 = K_EXP(0.6931472f * c_0b5 - 3.292449f) * 0.0035997033f;                          /* 241 */
    float t242 = t241 * t241;                                                                    /* 242 */
    float t244 = t242 * t242;                                                                    /* 244 */
    float t246 = K_SIN(t241);                                                                    /* 246 */
    float t249 = t244 * (0.041666627f - 0.0013888478f * t242 + t244 * (2.4795532e-05f - 2.3841858e-07f * t242)); /* 249 */
    float t250 = K_RECIP(1.0f - 0.5f * t242 + t249);                                             /* 250 */
    float c_0b2 = t246 * t250 * K_RECIP(t250 * t246 + 1.0f);                                     /* 259 */
    float c_0b6 = c_0b2 * 2.0f;                                                                  /* 267 */
    float t267 = c_0b3 * 0.8999996f;                                                             /* 267 */
    float t268 = c_0b6 * c_0b6;                                                                  /* 268 */
    float t271 = K_RECIP(1.0f + t267 * (t268 * t268));                                           /* 271 */
    float t279 = 1.0f - 2.0f * c_0b6;                                                            /* 279 */
    float t280 = K_CLAMP(c_0b1 * 0.119999945f * t271 - t271 * t267 * K_CLAMP((D_0bc_1 + 0.1f * D_0bc_3) * 0.6f)); /* 280 */
    float c_0b7 = t280 * -0.33333302f * (t280 * t280) + t280;                                    /* 282 */
    float c_0b8 = c_0b6 * (c_0b7 + D_0b7_1) + t279 * D_0b8_1;                                    /* 284 */
    float c_0b9 = c_0b6 * (c_0b8 + D_0b8_1) + t279 * D_0b9_1;                                    /* 286 */
    float c_0ba = c_0b6 * (c_0b9 + D_0b9_1) + t279 * D_0ba_1;                                    /* 288 */
    float t289 = c_0b6 * (c_0b6 * (c_0b6 * c_0b7 + c_0b8 + t279 * c_0b8) + c_0b9 + t279 * c_0b9) + c_0ba + t279 * c_0ba; /* 289 */
    float c_0bb = c_0b6 * (c_0ba + D_0ba_1) + t279 * D_0bb_1;                                    /* 290 */
    float c_0bc = c_0b6 * t289 + t279 * c_0bb;                                                   /* 291 */
    float c_0c8 = fmaxf(0.0075531006f + D_0c8_1 - 0.0075531006f * D_0c8_1, 0.0f);                /* 303 */
    float t321 = fmaxf(0.444274f * (c_08e * c_08e * c_08e) - 0.005f + 0.7f * (c_08e * 0.265983f), 0.0f); /* 321 */
    float c_0cf = fminf((c_0bb + c_0ad * 0.5f * c_0bb) * t321, 5.5f) * 1.1999998f;               /* 326 */
    float c_43a = c_0cf * c_0c8;                                                                 /* 332 */
    KOUT(OUT_VOICE, c_43a);
    float c_1ef = K_EXP((c_40a - 4.75f) * 0.6931472f) * 0.459375f;                               /* 354 */
    float t358 = c_1ef * 0.004988662f;                                                           /* 358 */
    float t360 = SEL(K_Z(t358), 2.3283064e-10f, fminf(t358, 0.249f));                            /* 360 */
    float c_1f8 = s1_wrap(D_1f7_1 + t360);                                                       /* 366 */
    float c_1f5 = SEL(K_LT0(c_1f8 - D_1f7_1), c_1f8, s1_wrap(D_1f5_1 + t360));                   /* 369 */
    float c_1f1 = K_RECIP(t360 * 256.0f);                                                        /* 371 */
    float t387 = K_SIN(K_CLAMP(K_TRI(0.5f * c_1f5 + 0.5f) * 256.0f * c_1f1 * 0.25f) * 1.570796f); /* 387 */
    float c_1fa = (s1_wrap(c_1f5) + c_1f5) * 0.5f * t387;                                        /* 388 */
    float c_1f8_2 = s1_wrap(c_1f8 + t360);                                                       /* 390 */
    float t391 = s1_wrap(c_1f5 + t360);                                                          /* 391 */
    float t393 = 1.5688642f * D_1fc_1 + (0.054386564f * D_1fa_1 + (0.10877313f * D_1f9_1 + c_1fa * 0.054386564f)); /* 393 */
    float c_201 = t393 - 0.7864105f * D_1fd_1;                                                   /* 393 */
    float c_1f5_2 = SEL(K_LT0(t391), c_1f8_2, t391);                                             /* 394 */
    float t406 = K_SIN(K_CLAMP(K_TRI(0.5f * c_1f5_2 + 0.5f) * 256.0f * c_1f1 * 0.25f) * 1.570796f); /* 406 */
    float c_1fa_2 = (s1_wrap(c_1f5_2) + c_1f5_2) * 0.5f * t406;                                  /* 407 */
    float c_1f8_3 = s1_wrap(c_1f8_2 + t360);                                                     /* 409 */
    float t410 = s1_wrap(c_1f5_2 + t360);                                                        /* 410 */
    float t412 = 1.5688642f * c_201 + (0.054386564f * D_1f9_1 + (0.10877313f * c_1fa + c_1fa_2 * 0.054386564f)); /* 412 */
    float c_209 = t412 - 0.7864105f * D_1fc_1;                                                   /* 412 */
    float c_1f5_3 = SEL(K_LT0(t410), c_1f8_3, t410);                                             /* 413 */
    float t425 = K_SIN(K_CLAMP(K_TRI(0.5f * c_1f5_3 + 0.5f) * 256.0f * c_1f1 * 0.25f) * 1.570796f); /* 425 */
    float c_1fa_3 = (s1_wrap(c_1f5_3) + c_1f5_3) * 0.5f * t425;                                  /* 426 */
    float c_1f7 = s1_wrap(c_1f8_3 + t360);                                                       /* 428 */
    float t429 = s1_wrap(c_1f5_3 + t360);                                                        /* 429 */
    float t431 = 1.5688642f * c_209 + (0.054386564f * c_1fa + (0.10877313f * c_1fa_2 + c_1fa_3 * 0.054386564f)); /* 431 */
    float c_211 = t431 - 0.7864105f * c_201;                                                     /* 431 */
    float c_1f5_4 = SEL(K_LT0(t429), c_1f7, t429);                                               /* 432 */
    float t444 = K_SIN(K_CLAMP(K_TRI(0.5f * c_1f5_4 + 0.5f) * 256.0f * c_1f1 * 0.25f) * 1.570796f); /* 444 */
    float c_1f9 = (s1_wrap(c_1f5_4) + c_1f5_4) * 0.5f * t444;                                    /* 445 */
    float t448 = 1.5688642f * c_211 + (0.10877313f * c_1fa_3 + c_1f9 * 0.054386564f + 0.054386564f * c_1fa_2); /* 448 */
    float c_219 = t448 - 0.7864105f * c_209;                                                     /* 448 */
    float t451 = -(-0.00062781386f * c_201) - -0.00062781386f * D_219_7 - -0.00181091f * c_209;  /* 451 */
    float t452 = t451 - -0.00181091f * D_211_7 - -0.0025511528f * c_211 - -0.0025511528f * D_209_7; /* 452 */
    float t454 = t452 - -0.0015808088f * c_219 - -0.0015808088f * D_201_7 - 0.002359534f * D_201_1; /* 454 */
    float t455 = t454 - 0.002359534f * D_219_6 - 0.008297408f * D_209_1 - 0.008297408f * D_211_6; /* 455 */
    float t457 = t455 - 0.011753906f * D_211_1 - 0.011753906f * D_209_6 - 0.006730832f * D_219_1; /* 457 */
    float t458 = t457 - 0.006730832f * D_201_6 - -0.009135877f * D_201_2 - -0.009135877f * D_219_5; /* 458 */
    float t460 = t458 - -0.029605705f * D_209_2 - -0.029605705f * D_211_5 - -0.03964878f * D_211_2; /* 460 */
    float t461 = t460 - -0.03964878f * D_209_5 - -0.022207731f * D_219_2 - -0.022207731f * D_201_5; /* 461 */
    float t463 = t461 - 0.030897303f * D_201_3 - 0.030897303f * D_219_4 - 0.110675454f * D_209_3; /* 463 */
    float t464 = t463 - 0.110675454f * D_211_4 - 0.19164494f * D_211_3 - 0.19164494f * D_209_4;  /* 464 */
    float c_1ff = t464 - 0.24270383f * D_219_3 - 0.24270383f * D_201_4;                          /* 465 */
    float c_418 = c_1ff * -0.5f;                                                                 /* 467 */
    float c_252 = K_EXP((c_40f - 4.75f) * 0.6931472f) * 0.459375f;                               /* 486 */
    float t490 = c_252 * 0.004988662f;                                                           /* 490 */
    float t492 = SEL(K_Z(t490), 2.3283064e-10f, fminf(t490, 0.249f));                            /* 492 */
    float c_288 = K_RECIP(t492 * 256.0f) * 0.19999981f;                                          /* 498 */
    float c_281 = -0.8999996f * c_40b - 0.029999971f;                                            /* 499 */
    float c_25a = s1_wrap(D_259_1 + t492);                                                       /* 501 */
    float t502 = s1_wrap(D_257_1 + t492);                                                        /* 502 */
    float c_257 = SEL(K_LT0(t502), c_25a, t502);                                                 /* 504 */
    float t505 = c_257 + c_281;                                                                  /* 505 */
    float t517 = K_CLAMP(K_TRI(K_RECIP(c_281 + SEL(K_LT0(t505), -1.0f, 1.0f)) * t505) * 256.0f * c_288); /* 517 */
    float t520 = s1_wrap(c_257 + t492);                                                          /* 520 */
    float c_25a_2 = s1_wrap(c_25a + t492);                                                       /* 522 */
    float t525 = s1_sign(t505) * t517;                                                           /* 525 */
    float c_257_2 = SEL(K_LT0(c_25a_2 - c_25a), c_25a_2, SEL(K_Z(t520), 0.0f, t520));            /* 525 */
    float t528 = 1.5991099f * D_25e_1 + (0.07904296f * D_25b_1 - t525 * 0.03952148f + 0.03952148f * D_25c_1); /* 528 */
    float c_261 = t528 - 0.75719583f * D_25f_1;                                                  /* 528 */
    float t530 = c_257_2 + c_281;                                                                /* 530 */
    float t540 = K_TRI(K_RECIP(c_281 + SEL(K_LT0(t530), -1.0f, SEL(K_GE0(t530), 1.0f, D_25e_1))) * t530) * 256.0f; /* 540 */
    float t546 = s1_wrap(c_257_2 + t492);                                                        /* 546 */
    float c_25a_3 = s1_wrap(c_25a_2 + t492);                                                     /* 547 */
    float t550 = s1_sign(t530) * K_CLAMP(t540 * c_288);                                          /* 550 */
    float c_257_3 = SEL(K_LT0(c_25a_3 - c_25a_2), c_25a_3, SEL(K_Z(t546), 0.0f, t546));          /* 550 */
    float t553 = 1.5991099f * c_261 + (-(0.07904296f * t525) - t550 * 0.03952148f + 0.03952148f * D_25b_1); /* 553 */
    float c_269 = t553 - 0.75719583f * D_25e_1;                                                  /* 553 */
    float t555 = c_257_3 + c_281;                                                                /* 555 */
    float t565 = K_TRI(K_RECIP(c_281 + SEL(K_LT0(t555), -1.0f, SEL(K_GE0(t555), 1.0f, c_261))) * t555) * 256.0f; /* 565 */
    float t571 = s1_wrap(c_257_3 + t492);                                                        /* 571 */
    float c_259 = s1_wrap(c_25a_3 + t492);                                                       /* 572 */
    float t575 = s1_sign(t555) * K_CLAMP(t565 * c_288);                                          /* 575 */
    float c_25c = -t575;                                                                         /* 575 */
    float c_257_4 = SEL(K_LT0(c_259 - c_25a_3), c_259, SEL(K_Z(t571), 0.0f, t571));              /* 575 */
    float t578 = 1.5991099f * c_269 + (-(0.07904296f * t550) - t575 * 0.03952148f - 0.03952148f * t525); /* 578 */
    float c_271 = t578 - 0.75719583f * c_261;                                                    /* 578 */
    float t580 = c_257_4 + c_281;                                                                /* 580 */
    float t590 = K_TRI(K_RECIP(c_281 + SEL(K_LT0(t580), -1.0f, SEL(K_GE0(t580), 1.0f, c_269))) * t580) * 256.0f; /* 590 */
    float t598 = s1_sign(t580) * K_CLAMP(t590 * c_288);                                          /* 598 */
    float c_25b = -t598;                                                                         /* 598 */
    float t601 = 1.5991099f * c_271 + (-(0.07904296f * t575) - t598 * 0.03952148f - 0.03952148f * t550); /* 601 */
    float c_25e = t601 - 0.75719583f * c_269;                                                    /* 601 */
    float t604 = 0.0028439998f * c_269 + (-0.010209f * c_261 - 0.010209f * D_279_7) + 0.0028439998f * D_271_7; /* 604 */
    float t606 = 0.025467f * c_25e + (0.016618f * c_271 + t604 + 0.016618f * D_269_7) + 0.025467f * D_261_7; /* 606 */
    float t608 = 0.013325f * D_269_1 + (0.024861f * D_261_1 + t606 + 0.024861f * D_279_6) + 0.013325f * D_271_6; /* 608 */
    float t610 = t608 - 0.006489f * D_271_1 - 0.006489f * D_269_6 - 0.028085f * D_279_1 - 0.028085f * D_261_6; /* 610 */
    float t612 = t610 - 0.042798f * D_261_2 - 0.042798f * D_279_5 - 0.042425f * D_269_2 - 0.042425f * D_271_5; /* 612 */
    float t614 = 0.018176999f * D_279_2 + (t612 - 0.022002f * D_271_2 - 0.022002f * D_269_5);    /* 614 */
    float t616 = 0.128923f * D_269_3 + (0.072034f * D_261_3 + (t614 + 0.018176999f * D_261_5) + 0.072034f * D_279_4); /* 616 */
    float t618 = 0.203227f * D_279_3 + (0.176322f * D_271_3 + (t616 + 0.128923f * D_271_4) + 0.176322f * D_269_4); /* 618 */
    float c_41d = (t618 + 0.203227f * D_261_4) * -0.5f;                                          /* 619 */

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
    D_1f7_1 = c_1f7;
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
    D_257_1 = c_257_4;
    D_259_1 = c_259;
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
    D_418_1 = c_418;
    D_41d_1 = c_41d;
}

