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
#define P_TUNE2 0xc046
#define P_XMOD 0xc04d
#define P_MIX1 0xc05a
#define P_MIX2 0xc05b
#define P_HPF 0xc05e
#define P_CUTOFF 0xc05f
#define P_RESO 0xc060
#define P_ENVMOD 0xc065
#define P_KEYFOLLOW 0xc066
static float hpf_lp_z1;
static float filter_in_z1;
static float filter_p1_z1;
static float filter_p2_z1;
static float filter_p3_z1;
static float filter_p4_z1;
static float filter_fb_z1, filter_fb_z2, filter_fb_z3;
static float vca_level_z1;
static float osc1_phase_z1;
static float osc1_z1;
static float osc1_3_z1;
static float osc1_4_z1;
static float osc1_5_z1;
static float osc1_state_z1;
static float osc1_state_prev2_z1, osc1_state_prev2_z2, osc1_state_prev2_z3, osc1_state_prev2_z4, osc1_state_prev2_z5, osc1_state_prev2_z6, osc1_state_prev2_z7;
static float osc1_state_prev3_z1, osc1_state_prev3_z2, osc1_state_prev3_z3, osc1_state_prev3_z4, osc1_state_prev3_z5, osc1_state_prev3_z6, osc1_state_prev3_z7;
static float osc1_state_prev4_z1, osc1_state_prev4_z2, osc1_state_prev4_z3, osc1_state_prev4_z4, osc1_state_prev4_z5, osc1_state_prev4_z6, osc1_state_prev4_z7;
static float osc1_5_prev_z1, osc1_5_prev_z2, osc1_5_prev_z3, osc1_5_prev_z4, osc1_5_prev_z5, osc1_5_prev_z6, osc1_5_prev_z7;
static float osc2_phase_z1;
static float osc2_z1;
static float osc2_3_z1;
static float osc2_4_z1;
static float osc2_5_z1;
static float osc2_state_z1;
static float osc2_state_prev2_z1, osc2_state_prev2_z2, osc2_state_prev2_z3, osc2_state_prev2_z4, osc2_state_prev2_z5, osc2_state_prev2_z6, osc2_state_prev2_z7;
static float osc2_state_prev3_z1, osc2_state_prev3_z2, osc2_state_prev3_z3, osc2_state_prev3_z4, osc2_state_prev3_z5, osc2_state_prev3_z6, osc2_state_prev3_z7;
static float osc2_state_prev4_z1, osc2_state_prev4_z2, osc2_state_prev4_z3, osc2_state_prev4_z4, osc2_state_prev4_z5, osc2_state_prev4_z6, osc2_state_prev4_z7;
static float osc2_5_prev_z1, osc2_5_prev_z2, osc2_5_prev_z3, osc2_5_prev_z4, osc2_5_prev_z5, osc2_5_prev_z6, osc2_5_prev_z7;
static float osc1_out_z1;
static float osc2_out_z1;
static inline __attribute__((always_inline)) void jp08_host_voice(void)
{
    float pitch = KIN(IN_PITCH);                                                                 /* 1 */
    float env_filter = KIN(IN_ENV_FILTER);                                                       /* 95 */

    /* ---- ENV1 (filter envelope, also pitch / PWM source): attack to 1.05 ends above 0.8925, decay to SUSTAIN x 0.87, release ---- */
    float env1_out = env_filter * env_filter * -0.09999484f * -10.0f;                            /* 98 */
    float env_amp = KIN(IN_ENV_AMP);                                                             /* 142 */

    /* ---- ENV2 (VCA envelope) ---- */
    float env2_out = env_amp * env_amp * -0.09999484f * -10.0f;                                  /* 145 */
    float osc1_mod = osc2_out_z1 * KP(P_XMOD);                                                   /* 161 */
    float osc2_8 = KIN(IN_PWM) * 1.2f;                                                           /* 166 */
    float osc2_pitch = pitch + KP(P_TUNE2);                                                      /* 171 */
    float osc1_pitch = pitch + osc1_mod;                                                         /* 172 */

    /* ---- mixer (MIX balance), HPF (one-pole lowpass subtracted) ---- */
    float mix = KP(P_MIX1) * osc1_out_z1 + KP(P_MIX2) * osc2_out_z1;                             /* 195 */
    float reso = KP(P_RESO);                                                                     /* 199 */

    /* ---- filter: cutoff (ENV1, KEY FOLLOW, LFO, CUTOFF) -> octaves -> g = tan / (1 + tan); 4 one-pole stages, resonance feedback ---- */
    float cutoff_mod = env1_out * KP(P_ENVMOD) + ((pitch - 4.0f) * KP(P_KEYFOLLOW) + KIN(IN_LFO_FILTER)); /* 209 */
    float hpf_out = mix - hpf_lp_z1;                                                             /* 213 */
    float hpf_lp = LERP(hpf_lp_z1, mix, KP(P_HPF));                                              /* 214 */
    float filter = 6.0f * reso;                                                                  /* 221 */
    float t223 = fminf((1.0f - reso) * 0.5f, 0.075f) + 0.16659999f + (10.625f * KP(P_CUTOFF) + cutoff_mod); /* 223 */
    float t224 = t223 * 0.25f;                                                                   /* 224 */
    float t225 = t224 * t224;                                                                    /* 225 */
    float t226 = t225 * t224;                                                                    /* 226 */
    float t227 = t223 - 11.0f;                                                                   /* 227 */
    float t228 = 2.3801785f * (t226 * t224) + (54.536343f * t225 - 48.64617f * t224 - 19.821596f * t226); /* 228 */
    float t229 = SEL(K_GE0(t227), t228 * 4.0f, SEL(K_LT0(t227), t223, t228));                    /* 229 */
    float cutoff_oct = fmaxf(SEL(K_GT0(t227), LERP(11.0f, t223, 0.0009999871f), t229), -3.0f);   /* 231 */
    float t241 = K_EXP(0.6931472f * cutoff_oct - 3.292449f) * 0.0035997033f;                     /* 241 */
    float t242 = t241 * t241;                                                                    /* 242 */
    float t244 = t242 * t242;                                                                    /* 244 */
    float t246 = K_SIN(t241);                                                                    /* 246 */
    float t249 = t244 * (0.041666627f - 0.0013888478f * t242 + t244 * (2.4795532e-05f - 2.3841858e-07f * t242)); /* 249 */
    float filter_tan = t246 * K_RECIP_POS(t246 + (1.0f - 0.5f * t242 + t249));                                /* 259  micro_fix */
    float filter_g = filter_tan * 2.0f;                                                          /* 267 */
    float t267 = filter * 0.8999996f;                                                            /* 267 */
    float t268 = filter_g * filter_g;                                                            /* 268 */
    float t271 = K_RECIP_POS(1.0f + t267 * (t268 * t268));                                           /* 271  micro_fix */
    float t279 = 1.0f - 2.0f * filter_g;                                                         /* 279 */
    float t280 = K_CLAMP(hpf_out * 0.119999945f * t271 - t271 * t267 * K_CLAMP((filter_fb_z1 + 0.1f * filter_fb_z3) * 0.6f)); /* 280 */
    float filter_in = t280 * -0.33333302f * (t280 * t280) + t280;                                /* 282 */
    float filter_p1 = filter_g * (filter_in + filter_in_z1) + t279 * filter_p1_z1;               /* 284 */
    float filter_p2 = filter_g * (filter_p1 + filter_p1_z1) + t279 * filter_p2_z1;               /* 286 */
    float filter_p3 = filter_g * (filter_p2 + filter_p2_z1) + t279 * filter_p3_z1;               /* 288 */
    float t289 = filter_g * (filter_g * (filter_g * filter_in + filter_p1 + t279 * filter_p1) + filter_p2 + t279 * filter_p2) + filter_p3 + t279 * filter_p3; /* 289 */
    float filter_p4 = filter_g * (filter_p3 + filter_p3_z1) + t279 * filter_p4_z1;               /* 290 */
    float filter_fb = filter_g * t289 + t279 * filter_p4;                                        /* 291 */

    /* ---- VCA: level ramp, ENV2 curve, output ---- */
    float vca_level = fmaxf(0.0075531006f + vca_level_z1 - 0.0075531006f * vca_level_z1, 0.0f);  /* 303 */
    float t321 = fmaxf(0.444274f * (env2_out * env2_out * env2_out) - 0.005f + 0.7f * (env2_out * 0.265983f), 0.0f); /* 321 */
    float vca_out = fminf((filter_p4 + reso * 0.5f * filter_p4) * t321, 5.5f) * 1.1999998f;      /* 326 */
    float vca = vca_out * vca_level;                                                             /* 332 */
    KOUT(OUT_VOICE, vca);

    /* ---- OSC1 (the slot's program, make_voice.py OSC1): four sub-samples, shape, two-pole lowpass, 28-tap decimation FIR ---- */
    float osc1_freq = K_EXP((osc1_pitch - 4.75f) * 0.6931472f) * 0.459375f;                      /* 354 */
    float t358 = osc1_freq * 0.004988662f;                                                       /* 358 */
    float t360 = SEL(K_Z(t358), 2.3283064e-10f, fminf(t358, 0.249f));                            /* 360 */
    float osc1_2 = s1_wrap(osc1_z1 + t360);                                                      /* 366 */
    float osc1_phase = SEL(K_LT0(osc1_2 - osc1_z1), osc1_2, s1_wrap(osc1_phase_z1 + t360));      /* 369 */
    float osc1_recip = K_RECIP(t360 * 256.0f);                                                   /* 371 */
    float t387 = K_SIN(K_CLAMP(K_TRI(0.5f * osc1_phase + 0.5f) * 256.0f * osc1_recip * 0.25f) * 1.570796f); /* 387 */
    float osc1_4 = (s1_wrap(osc1_phase) + osc1_phase) * 0.5f * t387;                             /* 388 */
    float osc1_2_2 = s1_wrap(osc1_2 + t360);                                                     /* 390 */
    float t391 = s1_wrap(osc1_phase + t360);                                                     /* 391 */
    float t393 = 1.5688642f * osc1_5_z1 + (0.054386564f * osc1_4_z1 + (0.10877313f * osc1_3_z1 + osc1_4 * 0.054386564f)); /* 393 */
    float osc1_state_prev2 = t393 - 0.7864105f * osc1_state_z1;                                  /* 393 */
    float osc1_phase_2 = SEL(K_LT0(t391), osc1_2_2, t391);                                       /* 394 */
    float t406 = K_SIN(K_CLAMP(K_TRI(0.5f * osc1_phase_2 + 0.5f) * 256.0f * osc1_recip * 0.25f) * 1.570796f); /* 406 */
    float osc1_4_2 = (s1_wrap(osc1_phase_2) + osc1_phase_2) * 0.5f * t406;                       /* 407 */
    float osc1_2_3 = s1_wrap(osc1_2_2 + t360);                                                   /* 409 */
    float t410 = s1_wrap(osc1_phase_2 + t360);                                                   /* 410 */
    float t412 = 1.5688642f * osc1_state_prev2 + (0.054386564f * osc1_3_z1 + (0.10877313f * osc1_4 + osc1_4_2 * 0.054386564f)); /* 412 */
    float osc1_state_prev3 = t412 - 0.7864105f * osc1_5_z1;                                      /* 412 */
    float osc1_phase_3 = SEL(K_LT0(t410), osc1_2_3, t410);                                       /* 413 */
    float t425 = K_SIN(K_CLAMP(K_TRI(0.5f * osc1_phase_3 + 0.5f) * 256.0f * osc1_recip * 0.25f) * 1.570796f); /* 425 */
    float osc1_4_3 = (s1_wrap(osc1_phase_3) + osc1_phase_3) * 0.5f * t425;                       /* 426 */
    float osc1 = s1_wrap(osc1_2_3 + t360);                                                       /* 428 */
    float t429 = s1_wrap(osc1_phase_3 + t360);                                                   /* 429 */
    float t431 = 1.5688642f * osc1_state_prev3 + (0.054386564f * osc1_4 + (0.10877313f * osc1_4_2 + osc1_4_3 * 0.054386564f)); /* 431 */
    float osc1_state_prev4 = t431 - 0.7864105f * osc1_state_prev2;                               /* 431 */
    float osc1_phase_4 = SEL(K_LT0(t429), osc1, t429);                                           /* 432 */
    float t444 = K_SIN(K_CLAMP(K_TRI(0.5f * osc1_phase_4 + 0.5f) * 256.0f * osc1_recip * 0.25f) * 1.570796f); /* 444 */
    float osc1_3 = (s1_wrap(osc1_phase_4) + osc1_phase_4) * 0.5f * t444;                         /* 445 */
    float t448 = 1.5688642f * osc1_state_prev4 + (0.10877313f * osc1_4_3 + osc1_3 * 0.054386564f + 0.054386564f * osc1_4_2); /* 448 */
    float osc1_5_prev = t448 - 0.7864105f * osc1_state_prev3;                                    /* 448 */
    float t451 = -(-0.00062781386f * osc1_state_prev2) - -0.00062781386f * osc1_5_prev_z7 - -0.00181091f * osc1_state_prev3; /* 451 */
    float t452 = t451 - -0.00181091f * osc1_state_prev4_z7 - -0.0025511528f * osc1_state_prev4 - -0.0025511528f * osc1_state_prev3_z7; /* 452 */
    float t454 = t452 - -0.0015808088f * osc1_5_prev - -0.0015808088f * osc1_state_prev2_z7 - 0.002359534f * osc1_state_prev2_z1; /* 454 */
    float t455 = t454 - 0.002359534f * osc1_5_prev_z6 - 0.008297408f * osc1_state_prev3_z1 - 0.008297408f * osc1_state_prev4_z6; /* 455 */
    float t457 = t455 - 0.011753906f * osc1_state_prev4_z1 - 0.011753906f * osc1_state_prev3_z6 - 0.006730832f * osc1_5_prev_z1; /* 457 */
    float t458 = t457 - 0.006730832f * osc1_state_prev2_z6 - -0.009135877f * osc1_state_prev2_z2 - -0.009135877f * osc1_5_prev_z5; /* 458 */
    float t460 = t458 - -0.029605705f * osc1_state_prev3_z2 - -0.029605705f * osc1_state_prev4_z5 - -0.03964878f * osc1_state_prev4_z2; /* 460 */
    float t461 = t460 - -0.03964878f * osc1_state_prev3_z5 - -0.022207731f * osc1_5_prev_z2 - -0.022207731f * osc1_state_prev2_z5; /* 461 */
    float t463 = t461 - 0.030897303f * osc1_state_prev2_z3 - 0.030897303f * osc1_5_prev_z4 - 0.110675454f * osc1_state_prev3_z3; /* 463 */
    float t464 = t463 - 0.110675454f * osc1_state_prev4_z4 - 0.19164494f * osc1_state_prev4_z3 - 0.19164494f * osc1_state_prev3_z4; /* 464 */
    float osc1_6 = t464 - 0.24270383f * osc1_5_prev_z3 - 0.24270383f * osc1_state_prev2_z4;      /* 465 */
    float osc1_out = osc1_6 * -0.5f;                                                             /* 467 */

    /* ---- OSC2 (the slot's program, make_voice.py OSC2): four sub-samples, shape, two-pole lowpass, 28-tap decimation FIR ---- */
    float osc2_freq = K_EXP((osc2_pitch - 4.75f) * 0.6931472f) * 0.459375f;                      /* 486 */
    float t490 = osc2_freq * 0.004988662f;                                                       /* 490 */
    float t492 = SEL(K_Z(t490), 2.3283064e-10f, fminf(t490, 0.249f));                            /* 492 */
    float osc2_7 = K_RECIP(t492 * 256.0f) * 0.19999981f;                                         /* 498 */
    float osc2_6 = -0.8999996f * osc2_8 - 0.029999971f;                                          /* 499 */
    float osc2_2 = s1_wrap(osc2_z1 + t492);                                                      /* 501 */
    float t502 = s1_wrap(osc2_phase_z1 + t492);                                                  /* 502 */
    float osc2_phase = SEL(K_LT0(t502), osc2_2, t502);                                           /* 504 */
    float t505 = osc2_phase + osc2_6;                                                            /* 505 */
    float t517 = K_CLAMP(K_TRI(K_RECIP(osc2_6 + SEL(K_LT0(t505), -1.0f, 1.0f)) * t505) * 256.0f * osc2_7); /* 517 */
    float t520 = s1_wrap(osc2_phase + t492);                                                     /* 520 */
    float osc2_2_2 = s1_wrap(osc2_2 + t492);                                                     /* 522 */
    float t525 = s1_sign(t505) * t517;                                                           /* 525 */
    float osc2_phase_2 = SEL(K_LT0(osc2_2_2 - osc2_2), osc2_2_2, SEL(K_Z(t520), 0.0f, t520));    /* 525 */
    float t528 = 1.5991099f * osc2_5_z1 + (0.07904296f * osc2_3_z1 - t525 * 0.03952148f + 0.03952148f * osc2_4_z1); /* 528 */
    float osc2_state_prev2 = t528 - 0.75719583f * osc2_state_z1;                                 /* 528 */
    float t530 = osc2_phase_2 + osc2_6;                                                          /* 530 */
    float t540 = K_TRI(K_RECIP(osc2_6 + SEL(K_LT0(t530), -1.0f, SEL(K_GE0(t530), 1.0f, osc2_5_z1))) * t530) * 256.0f; /* 540 */
    float t546 = s1_wrap(osc2_phase_2 + t492);                                                   /* 546 */
    float osc2_2_3 = s1_wrap(osc2_2_2 + t492);                                                   /* 547 */
    float t550 = s1_sign(t530) * K_CLAMP(t540 * osc2_7);                                         /* 550 */
    float osc2_phase_3 = SEL(K_LT0(osc2_2_3 - osc2_2_2), osc2_2_3, SEL(K_Z(t546), 0.0f, t546));  /* 550 */
    float t553 = 1.5991099f * osc2_state_prev2 + (-(0.07904296f * t525) - t550 * 0.03952148f + 0.03952148f * osc2_3_z1); /* 553 */
    float osc2_state_prev3 = t553 - 0.75719583f * osc2_5_z1;                                     /* 553 */
    float t555 = osc2_phase_3 + osc2_6;                                                          /* 555 */
    float t565 = K_TRI(K_RECIP(osc2_6 + SEL(K_LT0(t555), -1.0f, SEL(K_GE0(t555), 1.0f, osc2_state_prev2))) * t555) * 256.0f; /* 565 */
    float t571 = s1_wrap(osc2_phase_3 + t492);                                                   /* 571 */
    float osc2 = s1_wrap(osc2_2_3 + t492);                                                       /* 572 */
    float t575 = s1_sign(t555) * K_CLAMP(t565 * osc2_7);                                         /* 575 */
    float osc2_4 = -t575;                                                                        /* 575 */
    float osc2_phase_4 = SEL(K_LT0(osc2 - osc2_2_3), osc2, SEL(K_Z(t571), 0.0f, t571));          /* 575 */
    float t578 = 1.5991099f * osc2_state_prev3 + (-(0.07904296f * t550) - t575 * 0.03952148f - 0.03952148f * t525); /* 578 */
    float osc2_state_prev4 = t578 - 0.75719583f * osc2_state_prev2;                              /* 578 */
    float t580 = osc2_phase_4 + osc2_6;                                                          /* 580 */
    float t590 = K_TRI(K_RECIP(osc2_6 + SEL(K_LT0(t580), -1.0f, SEL(K_GE0(t580), 1.0f, osc2_state_prev3))) * t580) * 256.0f; /* 590 */
    float t598 = s1_sign(t580) * K_CLAMP(t590 * osc2_7);                                         /* 598 */
    float osc2_3 = -t598;                                                                        /* 598 */
    float t601 = 1.5991099f * osc2_state_prev4 + (-(0.07904296f * t575) - t598 * 0.03952148f - 0.03952148f * t550); /* 601 */
    float osc2_5 = t601 - 0.75719583f * osc2_state_prev3;                                        /* 601 */
    float t604 = 0.0028439998f * osc2_state_prev3 + (-0.010209f * osc2_state_prev2 - 0.010209f * osc2_5_prev_z7) + 0.0028439998f * osc2_state_prev4_z7; /* 604 */
    float t606 = 0.025467f * osc2_5 + (0.016618f * osc2_state_prev4 + t604 + 0.016618f * osc2_state_prev3_z7) + 0.025467f * osc2_state_prev2_z7; /* 606 */
    float t608 = 0.013325f * osc2_state_prev3_z1 + (0.024861f * osc2_state_prev2_z1 + t606 + 0.024861f * osc2_5_prev_z6) + 0.013325f * osc2_state_prev4_z6; /* 608 */
    float t610 = t608 - 0.006489f * osc2_state_prev4_z1 - 0.006489f * osc2_state_prev3_z6 - 0.028085f * osc2_5_prev_z1 - 0.028085f * osc2_state_prev2_z6; /* 610 */
    float t612 = t610 - 0.042798f * osc2_state_prev2_z2 - 0.042798f * osc2_5_prev_z5 - 0.042425f * osc2_state_prev3_z2 - 0.042425f * osc2_state_prev4_z5; /* 612 */
    float t614 = 0.018176999f * osc2_5_prev_z2 + (t612 - 0.022002f * osc2_state_prev4_z2 - 0.022002f * osc2_state_prev3_z5); /* 614 */
    float t616 = 0.128923f * osc2_state_prev3_z3 + (0.072034f * osc2_state_prev2_z3 + (t614 + 0.018176999f * osc2_state_prev2_z5) + 0.072034f * osc2_5_prev_z4); /* 616 */
    float t618 = 0.203227f * osc2_5_prev_z3 + (0.176322f * osc2_state_prev4_z3 + (t616 + 0.128923f * osc2_state_prev4_z4) + 0.176322f * osc2_state_prev3_z4); /* 618 */
    float osc2_out = (t618 + 0.203227f * osc2_state_prev2_z4) * -0.5f;                           /* 619 */

    hpf_lp_z1 = hpf_lp;
    filter_in_z1 = filter_in;
    filter_p1_z1 = filter_p1;
    filter_p2_z1 = filter_p2;
    filter_p3_z1 = filter_p3;
    filter_p4_z1 = filter_p4;
    filter_fb_z3 = filter_fb_z2;
    filter_fb_z2 = filter_fb_z1;
    filter_fb_z1 = filter_fb;
    vca_level_z1 = vca_level;
    osc1_phase_z1 = osc1_phase_4;
    osc1_z1 = osc1;
    osc1_3_z1 = osc1_3;
    osc1_4_z1 = osc1_4_3;
    osc1_5_z1 = osc1_5_prev;
    osc1_state_z1 = osc1_state_prev4;
    osc1_state_prev2_z7 = osc1_state_prev2_z6;
    osc1_state_prev2_z6 = osc1_state_prev2_z5;
    osc1_state_prev2_z5 = osc1_state_prev2_z4;
    osc1_state_prev2_z4 = osc1_state_prev2_z3;
    osc1_state_prev2_z3 = osc1_state_prev2_z2;
    osc1_state_prev2_z2 = osc1_state_prev2_z1;
    osc1_state_prev2_z1 = osc1_state_prev2;
    osc1_state_prev3_z7 = osc1_state_prev3_z6;
    osc1_state_prev3_z6 = osc1_state_prev3_z5;
    osc1_state_prev3_z5 = osc1_state_prev3_z4;
    osc1_state_prev3_z4 = osc1_state_prev3_z3;
    osc1_state_prev3_z3 = osc1_state_prev3_z2;
    osc1_state_prev3_z2 = osc1_state_prev3_z1;
    osc1_state_prev3_z1 = osc1_state_prev3;
    osc1_state_prev4_z7 = osc1_state_prev4_z6;
    osc1_state_prev4_z6 = osc1_state_prev4_z5;
    osc1_state_prev4_z5 = osc1_state_prev4_z4;
    osc1_state_prev4_z4 = osc1_state_prev4_z3;
    osc1_state_prev4_z3 = osc1_state_prev4_z2;
    osc1_state_prev4_z2 = osc1_state_prev4_z1;
    osc1_state_prev4_z1 = osc1_state_prev4;
    osc1_5_prev_z7 = osc1_5_prev_z6;
    osc1_5_prev_z6 = osc1_5_prev_z5;
    osc1_5_prev_z5 = osc1_5_prev_z4;
    osc1_5_prev_z4 = osc1_5_prev_z3;
    osc1_5_prev_z3 = osc1_5_prev_z2;
    osc1_5_prev_z2 = osc1_5_prev_z1;
    osc1_5_prev_z1 = osc1_5_prev;
    osc2_phase_z1 = osc2_phase_4;
    osc2_z1 = osc2;
    osc2_3_z1 = osc2_3;
    osc2_4_z1 = osc2_4;
    osc2_5_z1 = osc2_5;
    osc2_state_z1 = osc2_state_prev4;
    osc2_state_prev2_z7 = osc2_state_prev2_z6;
    osc2_state_prev2_z6 = osc2_state_prev2_z5;
    osc2_state_prev2_z5 = osc2_state_prev2_z4;
    osc2_state_prev2_z4 = osc2_state_prev2_z3;
    osc2_state_prev2_z3 = osc2_state_prev2_z2;
    osc2_state_prev2_z2 = osc2_state_prev2_z1;
    osc2_state_prev2_z1 = osc2_state_prev2;
    osc2_state_prev3_z7 = osc2_state_prev3_z6;
    osc2_state_prev3_z6 = osc2_state_prev3_z5;
    osc2_state_prev3_z5 = osc2_state_prev3_z4;
    osc2_state_prev3_z4 = osc2_state_prev3_z3;
    osc2_state_prev3_z3 = osc2_state_prev3_z2;
    osc2_state_prev3_z2 = osc2_state_prev3_z1;
    osc2_state_prev3_z1 = osc2_state_prev3;
    osc2_state_prev4_z7 = osc2_state_prev4_z6;
    osc2_state_prev4_z6 = osc2_state_prev4_z5;
    osc2_state_prev4_z5 = osc2_state_prev4_z4;
    osc2_state_prev4_z4 = osc2_state_prev4_z3;
    osc2_state_prev4_z3 = osc2_state_prev4_z2;
    osc2_state_prev4_z2 = osc2_state_prev4_z1;
    osc2_state_prev4_z1 = osc2_state_prev4;
    osc2_5_prev_z7 = osc2_5_prev_z6;
    osc2_5_prev_z6 = osc2_5_prev_z5;
    osc2_5_prev_z5 = osc2_5_prev_z4;
    osc2_5_prev_z4 = osc2_5_prev_z3;
    osc2_5_prev_z3 = osc2_5_prev_z2;
    osc2_5_prev_z2 = osc2_5_prev_z1;
    osc2_5_prev_z1 = osc2_5;
    osc1_out_z1 = osc1_out;
    osc2_out_z1 = osc2_out;
}

