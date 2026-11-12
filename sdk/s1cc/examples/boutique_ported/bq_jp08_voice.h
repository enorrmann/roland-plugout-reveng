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
#define P_LFO_RATE 0xc00a
#define P_LFO_DELAY 0xc00b
#define P_ENV1_A 0xc03c
#define P_ENV1_D 0xc03d
#define P_ENV1_S 0xc03e
#define P_ENV1_R 0xc03f
#define P_ENV2_A 0xc040
#define P_ENV2_D 0xc041
#define P_ENV2_S 0xc042
#define P_ENV2_R 0xc043
#define P_TUNE2 0xc046
#define P_LFO_PITCH 0xc04a
#define P_ENV_PITCH 0xc04b
#define P_XMOD 0xc04d
#define P_PWM_2 0xc054
#define P_MIX1 0xc05a
#define P_MIX2 0xc05b
#define P_CUTOFF 0xc05f
#define P_RESO 0xc060
#define P_LFO_FILTER 0xc064
#define P_ENVMOD 0xc065
#define P_KEYFOLLOW 0xc066
static float lfo_in_z1;   /* part A */
static float lfo_state_z1;   /* part A */
static float lfo_phase_z1;   /* part A */
static float gate_count_z1;   /* part A */
static float gate_on_prev2_z1;   /* part A */
static float gate_on_prev3_z1;   /* part A */
static float env1_attack_z1;   /* part A */
static float env1_z1, env1_z2;   /* part A */
static float gate_on_prev4_z1;   /* part A */
static float env2_attack_z1;   /* part A */
static float env2_z1, env2_z2;   /* part A */
static float hpf_lp_z1;   /* part B */
static float filter_in_z1;   /* part B */
static float filter_p1_z1;   /* part B */
static float filter_p2_z1;   /* part B */
static float filter_p3_z1;   /* part B */
static float filter_p4_z1;   /* part B */
static float filter_fb_z1, filter_fb_z2, filter_fb_z3;   /* part B */
static float vca_level_z1;   /* part B */
static float osc1_phase_z1;   /* part B */
static float osc1_z1;   /* part B */
static float osc1_3_z1;   /* part B */
static float osc1_4_z1;   /* part B */
static float osc1_5_z1;   /* part B */
static float osc1_state_z1;   /* part B */
static float osc1_state_prev2_z1, osc1_state_prev2_z2, osc1_state_prev2_z3, osc1_state_prev2_z4, osc1_state_prev2_z5, osc1_state_prev2_z6, osc1_state_prev2_z7;   /* part B */
static float osc1_state_prev3_z1, osc1_state_prev3_z2, osc1_state_prev3_z3, osc1_state_prev3_z4, osc1_state_prev3_z5, osc1_state_prev3_z6, osc1_state_prev3_z7;   /* part B */
static float osc1_state_prev4_z1, osc1_state_prev4_z2, osc1_state_prev4_z3, osc1_state_prev4_z4, osc1_state_prev4_z5, osc1_state_prev4_z6, osc1_state_prev4_z7;   /* part B */
static float osc1_5_prev_z1, osc1_5_prev_z2, osc1_5_prev_z3, osc1_5_prev_z4, osc1_5_prev_z5, osc1_5_prev_z6, osc1_5_prev_z7;   /* part B */
static float osc2_phase_z1;   /* part A */
static float osc2_z1;   /* part A */
static float osc2_3_z1;   /* part A */
static float osc2_4_z1;   /* part A */
static float osc2_5_z1;   /* part A */
static float osc2_state_z1;   /* part A */
static float osc2_state_prev2_z1, osc2_state_prev2_z2, osc2_state_prev2_z3, osc2_state_prev2_z4, osc2_state_prev2_z5, osc2_state_prev2_z6, osc2_state_prev2_z7;   /* part A */
static float osc2_state_prev3_z1, osc2_state_prev3_z2, osc2_state_prev3_z3, osc2_state_prev3_z4, osc2_state_prev3_z5, osc2_state_prev3_z6, osc2_state_prev3_z7;   /* part A */
static float osc2_state_prev4_z1, osc2_state_prev4_z2, osc2_state_prev4_z3, osc2_state_prev4_z4, osc2_state_prev4_z5, osc2_state_prev4_z6, osc2_state_prev4_z7;   /* part A */
static float osc2_5_prev_z1, osc2_5_prev_z2, osc2_5_prev_z3, osc2_5_prev_z4, osc2_5_prev_z5, osc2_5_prev_z6, osc2_5_prev_z7;   /* part A */
static float osc1_out_z1;   /* part B */
static float osc2_out_z1;   /* part A */
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
    /* ---- LFO (pre program): phase from LFO RATE, delay fade from LFO DELAY, shape, LFO x fade^2 ---- */
    float t14 = 0.1f * KP(P_LFO_DELAY) + 0.9f;                                                   /* 14 */
    float lfo_in = K_CLAMP(gate_on_prev2_z1);                                                    /* 14 */
    float lfo_4 = fminf(s1_floor(KP(P_LFO_RATE) * 65536.0f) * 2.3283064e-10f, 0.0071359314f);    /* 16 */
    float t17 = lfo_in_z1 - lfo_in;                                                              /* 17 */
    float t17_ge0 = K_GE0(t17);                                                                  /* 17 */
    float t17_lt0 = K_LT0(t17);                                                                  /* 19 */
    float lfo_state = SEL(t17_ge0, K_CLAMP(t14 * lfo_state_z1 + (1.0f - t14)), SEL(t17_lt0, 0.0f, lfo_phase_z1)); /* 19 */
    float t21 = fmaxf(5.0f * lfo_state - 4.0f, 0.0f);                                            /* 21 */
    float lfo_sq = t21 * t21;                                                                    /* 22 */
    float lfo_phase = s1_wrap((lfo_4 + lfo_phase_z1) * SEL(t17_ge0, 1.0f, SEL(t17_lt0, 1.0f, lfo_in_z1))); /* 26 */
    float t36 = s1_abs(s1_wrap(lfo_phase));                                                      /* 36 */
    float t37 = t36 * t36;                                                                       /* 37 */
    float lfo_2 = (3.125f * t37 - 0.04159999f * t36 - 2.0799866f * (t37 * t36) - 0.5039997f) * 2.0f; /* 42 */
    float lfo_3 = lfo_2 * lfo_sq;                                                                /* 43 */
    float lfo = lfo_3 * lfo_sq;                                                                  /* 49 */
    float pitch = KIN(IN_PITCH);                                                                 /* 60 */

    /* ---- gate: the key with Roland's retrigger gap (a short count before a new note's gate) ---- */
    float gate = KIN(IN_GATE);                                                                   /* 61 */
    float t67 = gate_count_z1 + 0.00074767857f;                                                  /* 67 */
    float gate_count = SEL(K_NZ(gate), fminf(t67, 0.0f), SEL(K_Z(gate), -1.0f, t67));            /* 68 */
    float gate_on = s1_sign(gate_count) + 1.0f;                                                  /* 70 */
    float b118_ge0 = K_GE0(SEL(K_LE0(env1_z1 - env1_z2), 0.0f, env1_z1) - 0.8925f);              /* 118 */

    /* ---- ENV1 (filter envelope, also pitch / PWM source): attack to 1.05 ends above 0.8925, decay to SUSTAIN x 0.87, release ---- */
    float env1_attack = SEL(b118_ge0, 0.0f, SEL(K_LT0(gate_on - gate_on_prev3_z1 - 1.0f), env1_attack_z1, 1.0f)); /* 121 */
    float t122 = gate_on - 1.0f;                                                                 /* 122 */
    float key_on = 1.0f - SEL(K_GE0(t122), 0.0f, SEL(K_LT0(t122), 1.0f, gate_on));               /* 125 */
    float key_on_le0 = K_LE0(key_on);                                                            /* 125 */
    float t129 = 1.05f - env1_z1;                                                                /* 129 */
    float t133 = fmaxf(1.0f - KP(P_ENV1_D), 0.00011998415f);                                     /* 133 */
    float env1_2 = KP(P_ENV1_S) * 0.87f;                                                         /* 134 */
    float t137 = 0.02f * fmaxf(t133 + 0.01f, 1.0f) - 0.02f + env1_2;                             /* 137 */
    float t138 = SEL(K_GT0(env1_attack * key_on), 1.05f - (t129 - fmaxf(KP(P_ENV1_A), 7.927418e-06f) * t129), env1_z1) - t137; /* 138 */
    float t142 = SEL(K_GT0(env1_attack * key_on), t138 + t137, fmaxf(0.01f * (t138 * t133) + t137 + t138 * 0.99f, 0.0f));                       /* 142  attack_fix */
    float t143 = fmaxf(1.0f - KP(P_ENV1_R), 0.00014305115f);                                     /* 143 */
    float t146 = 0.02f * fmaxf(t143 + 0.01f, 1.0f) - 0.0007080078f;                              /* 146 */
    float env1 = SEL(key_on_le0, fmaxf(LERP(t146, env1_z1, 0.99f) + 0.01f * ((env1_z1 - t146) * t143), 0.0f), t142); /* 153  release_fix */
    float env1_out = env1 * env1 * -0.09999484f * -10.0f;                                        /* 157 */
    float t165 = gate_on - gate_on_prev4_z1 - 1.0f;                                              /* 165 */
    float b165_ge0 = K_GE0(SEL(K_LE0(env2_z1 - env2_z2), 0.0f, env2_z1) - 0.8925f);              /* 165 */

    /* ---- ENV2 (VCA envelope) ---- */
    float env2_attack = SEL(b165_ge0, 0.0f, SEL(K_LT0(t165), env2_attack_z1, SEL(K_GE0(t165), 1.0f, key_on))); /* 168 */
    float t176 = 1.05f - env2_z1;                                                                /* 176 */
    float t180 = fmaxf(1.0f - KP(P_ENV2_D), 0.00011998415f);                                     /* 180 */
    float env2_2 = KP(P_ENV2_S) * 0.87f;                                                         /* 181 */
    float t184 = 0.02f * fmaxf(t180 + 0.01f, 1.0f) - 0.02f + env2_2;                             /* 184 */
    float t185 = SEL(K_GT0(env2_attack * key_on), 1.05f - (t176 - fmaxf(KP(P_ENV2_A), 7.927418e-06f) * t176), env2_z1) - t184; /* 185 */
    float t189 = SEL(K_GT0(env2_attack * key_on), t185 + t184, fmaxf(0.01f * (t185 * t180) + t184 + t185 * 0.99f, 0.0f));                       /* 189  attack_fix */
    float t190 = fmaxf(1.0f - KP(P_ENV2_R), 0.00014305115f);                                     /* 190 */
    float t193 = 0.02f * fmaxf(t190 + 0.01f, 1.0f) - 0.0007080078f;                              /* 193 */
    float env2 = SEL(key_on_le0, fmaxf(LERP(t193, env2_z1, 0.99f) + 0.01f * ((env2_z1 - t193) * t190), 0.0f), t189); /* 200  release_fix */
    float env2_out = env2 * env2 * -0.09999484f * -10.0f;                                        /* 204 */
    float t214 = lfo * 0.5f + 0.5f;                                                              /* 214 */

    /* ---- modulation: PWM source, LFO / ENV1 -> pitch, CROSS MOD (OSC2 output of the sample before -> OSC1 pitch) ---- */
    float pwm_src = KIN(0xa090) + (KIN(0xa093) * t214 + (KIN(0xa092) * env1_out + t214 * KIN(0xa091))); /* 218 */
    float pitch_mod = lfo * KP(P_LFO_PITCH) + env1_out * KP(P_ENV_PITCH);                        /* 222 */
    float osc1_mod = osc2_out_z1 * KP(P_XMOD) + pitch_mod;                                       /* 224 */
    float pwm = pwm_src * KP(P_PWM_2);                                                           /* 226 */
    float osc1_pitch = pitch + osc1_mod;                                                         /* 231 */
    float osc2_pitch = pitch + KP(P_TUNE2) + pitch_mod;                                          /* 231 */

    /* ---- OSC2 (the slot's program, make_voice.py OSC2): four sub-samples, shape, two-pole lowpass, 28-tap decimation FIR ---- */
    float osc2_freq = K_EXP((osc2_pitch - 4.75f) * 0.6931472f) * 0.459375f;                      /* 257 */
    float t261 = osc2_freq * 0.004988662f;                                                       /* 261 */
    float t263 = SEL(K_Z(t261), 2.3283064e-10f, fminf(t261, 0.249f));                            /* 263 */
    float osc2_7 = K_RECIP(t263 * 256.0f) * 0.19999981f;                                         /* 269 */
    float osc2_6 = -0.8999996f * pwm - 0.029999971f;                                             /* 270 */
    float osc2_2 = s1_wrap(osc2_z1 + t263);                                                      /* 272 */
    float t273 = s1_wrap(osc2_phase_z1 + t263);                                                  /* 273 */
    float osc2_phase = SEL(K_LT0(t273), osc2_2, t273);                                           /* 275 */
    float t276 = osc2_phase + osc2_6;                                                            /* 276 */
    float t288 = K_CLAMP(K_TRI(K_RECIP(osc2_6 + SEL(K_LT0(t276), -1.0f, 1.0f)) * t276) * 256.0f * osc2_7); /* 288 */
    float t291 = s1_wrap(osc2_phase + t263);                                                     /* 291 */
    float osc2_2_2 = s1_wrap(osc2_2 + t263);                                                     /* 293 */
    float t296 = s1_sign(t276) * t288;                                                           /* 296 */
    float osc2_phase_2 = SEL(K_LT0(osc2_2_2 - osc2_2), osc2_2_2, SEL(K_Z(t291), 0.0f, t291));    /* 296 */
    float t299 = 1.5991099f * osc2_5_z1 + (0.07904296f * osc2_3_z1 - t296 * 0.03952148f + 0.03952148f * osc2_4_z1); /* 299 */
    float osc2_state_prev2 = t299 - 0.75719583f * osc2_state_z1;                                 /* 299 */
    float t301 = osc2_phase_2 + osc2_6;                                                          /* 301 */
    float t311 = K_TRI(K_RECIP(osc2_6 + SEL(K_LT0(t301), -1.0f, SEL(K_GE0(t301), 1.0f, osc2_5_z1))) * t301) * 256.0f; /* 311 */
    float t317 = s1_wrap(osc2_phase_2 + t263);                                                   /* 317 */
    float osc2_2_3 = s1_wrap(osc2_2_2 + t263);                                                   /* 318 */
    float t321 = s1_sign(t301) * K_CLAMP(t311 * osc2_7);                                         /* 321 */
    float osc2_phase_3 = SEL(K_LT0(osc2_2_3 - osc2_2_2), osc2_2_3, SEL(K_Z(t317), 0.0f, t317));  /* 321 */
    float t324 = 1.5991099f * osc2_state_prev2 + (-(0.07904296f * t296) - t321 * 0.03952148f + 0.03952148f * osc2_3_z1); /* 324 */
    float osc2_state_prev3 = t324 - 0.75719583f * osc2_5_z1;                                     /* 324 */
    float t326 = osc2_phase_3 + osc2_6;                                                          /* 326 */
    float t336 = K_TRI(K_RECIP(osc2_6 + SEL(K_LT0(t326), -1.0f, SEL(K_GE0(t326), 1.0f, osc2_state_prev2))) * t326) * 256.0f; /* 336 */
    float t342 = s1_wrap(osc2_phase_3 + t263);                                                   /* 342 */
    float osc2 = s1_wrap(osc2_2_3 + t263);                                                       /* 343 */
    float t346 = s1_sign(t326) * K_CLAMP(t336 * osc2_7);                                         /* 346 */
    float osc2_4 = -t346;                                                                        /* 346 */
    float osc2_phase_4 = SEL(K_LT0(osc2 - osc2_2_3), osc2, SEL(K_Z(t342), 0.0f, t342));          /* 346 */
    float t349 = 1.5991099f * osc2_state_prev3 + (-(0.07904296f * t321) - t346 * 0.03952148f - 0.03952148f * t296); /* 349 */
    float osc2_state_prev4 = t349 - 0.75719583f * osc2_state_prev2;                              /* 349 */
    float t351 = osc2_phase_4 + osc2_6;                                                          /* 351 */
    float t361 = K_TRI(K_RECIP(osc2_6 + SEL(K_LT0(t351), -1.0f, SEL(K_GE0(t351), 1.0f, osc2_state_prev3))) * t351) * 256.0f; /* 361 */
    float t369 = s1_sign(t351) * K_CLAMP(t361 * osc2_7);                                         /* 369 */
    float osc2_3 = -t369;                                                                        /* 369 */
    float t372 = 1.5991099f * osc2_state_prev4 + (-(0.07904296f * t346) - t369 * 0.03952148f - 0.03952148f * t321); /* 372 */
    float osc2_5 = t372 - 0.75719583f * osc2_state_prev3;                                        /* 372 */
    float t375 = 0.0028439998f * osc2_state_prev3 + (-0.010209f * osc2_state_prev2 - 0.010209f * osc2_5_prev_z7) + 0.0028439998f * osc2_state_prev4_z7; /* 375 */
    float t377 = 0.025467f * osc2_5 + (0.016618f * osc2_state_prev4 + t375 + 0.016618f * osc2_state_prev3_z7) + 0.025467f * osc2_state_prev2_z7; /* 377 */
    float t379 = 0.013325f * osc2_state_prev3_z1 + (0.024861f * osc2_state_prev2_z1 + t377 + 0.024861f * osc2_5_prev_z6) + 0.013325f * osc2_state_prev4_z6; /* 379 */
    float t381 = t379 - 0.006489f * osc2_state_prev4_z1 - 0.006489f * osc2_state_prev3_z6 - 0.028085f * osc2_5_prev_z1 - 0.028085f * osc2_state_prev2_z6; /* 381 */
    float t383 = t381 - 0.042798f * osc2_state_prev2_z2 - 0.042798f * osc2_5_prev_z5 - 0.042425f * osc2_state_prev3_z2 - 0.042425f * osc2_state_prev4_z5; /* 383 */
    float t385 = 0.018176999f * osc2_5_prev_z2 + (t383 - 0.022002f * osc2_state_prev4_z2 - 0.022002f * osc2_state_prev3_z5); /* 385 */
    float t387 = 0.128923f * osc2_state_prev3_z3 + (0.072034f * osc2_state_prev2_z3 + (t385 + 0.018176999f * osc2_state_prev2_z5) + 0.072034f * osc2_5_prev_z4); /* 387 */
    float t389 = 0.203227f * osc2_5_prev_z3 + (0.176322f * osc2_state_prev4_z3 + (t387 + 0.128923f * osc2_state_prev4_z4) + 0.176322f * osc2_state_prev3_z4); /* 389 */
    float osc2_out = (t389 + 0.203227f * osc2_state_prev2_z4) * -0.5f;                           /* 390 */

    s1_export(0, osc1_pitch);
    s1_export(1, osc2_out_z1);
    s1_export(2, lfo);
    s1_export(3, env1_out);
    s1_export(4, env2_out);
    s1_export(5, pitch);
    lfo_in_z1 = lfo_in;
    lfo_state_z1 = lfo_state;
    lfo_phase_z1 = lfo_phase;
    gate_count_z1 = gate_count;
    gate_on_prev2_z1 = gate_on;
    gate_on_prev3_z1 = gate_on;
    env1_attack_z1 = env1_attack;
    env1_z2 = env1_z1;
    env1_z1 = env1;
    gate_on_prev4_z1 = gate_on;
    env2_attack_z1 = env2_attack;
    env2_z2 = env2_z1;
    env2_z1 = env2;
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
    osc2_out_z1 = osc2_out;
}
static inline __attribute__((always_inline)) void jp08_voice_b(KARGS_B)
{
    float osc1_pitch = s1_range(s1_import(0), -8.501f, 18.501f);                                 /* 400 */

    /* ---- OSC1 (the slot's program, make_voice.py OSC1): four sub-samples, shape, two-pole lowpass, 28-tap decimation FIR ---- */
    float osc1_freq = K_EXP((osc1_pitch - 4.75f) * 0.6931472f) * 0.459375f;                      /* 411 */
    float t415 = osc1_freq * 0.004988662f;                                                       /* 415 */
    float t417 = SEL(K_Z(t415), 2.3283064e-10f, fminf(t415, 0.249f));                            /* 417 */
    float osc1_2 = s1_wrap(osc1_z1 + t417);                                                      /* 423 */
    float osc1_phase = SEL(K_LT0(osc1_2 - osc1_z1), osc1_2, s1_wrap(osc1_phase_z1 + t417));      /* 426 */
    float osc1_recip = K_RECIP(t417 * 256.0f);                                                   /* 428 */
    float t444 = K_SIN(K_CLAMP(K_TRI(0.5f * osc1_phase + 0.5f) * 256.0f * osc1_recip * 0.25f) * 1.570796f); /* 444 */
    float osc1_4 = (s1_wrap(osc1_phase) + osc1_phase) * 0.5f * t444;                             /* 445 */
    float osc1_2_2 = s1_wrap(osc1_2 + t417);                                                     /* 447 */
    float t448 = s1_wrap(osc1_phase + t417);                                                     /* 448 */
    float t450 = 1.5688642f * osc1_5_z1 + (0.054386564f * osc1_4_z1 + (0.10877313f * osc1_3_z1 + osc1_4 * 0.054386564f)); /* 450 */
    float osc1_state_prev2 = t450 - 0.7864105f * osc1_state_z1;                                  /* 450 */
    float osc1_phase_2 = SEL(K_LT0(t448), osc1_2_2, t448);                                       /* 451 */
    float t463 = K_SIN(K_CLAMP(K_TRI(0.5f * osc1_phase_2 + 0.5f) * 256.0f * osc1_recip * 0.25f) * 1.570796f); /* 463 */
    float osc1_4_2 = (s1_wrap(osc1_phase_2) + osc1_phase_2) * 0.5f * t463;                       /* 464 */
    float osc1_2_3 = s1_wrap(osc1_2_2 + t417);                                                   /* 466 */
    float t467 = s1_wrap(osc1_phase_2 + t417);                                                   /* 467 */
    float t469 = 1.5688642f * osc1_state_prev2 + (0.054386564f * osc1_3_z1 + (0.10877313f * osc1_4 + osc1_4_2 * 0.054386564f)); /* 469 */
    float osc1_state_prev3 = t469 - 0.7864105f * osc1_5_z1;                                      /* 469 */
    float osc1_phase_3 = SEL(K_LT0(t467), osc1_2_3, t467);                                       /* 470 */
    float t482 = K_SIN(K_CLAMP(K_TRI(0.5f * osc1_phase_3 + 0.5f) * 256.0f * osc1_recip * 0.25f) * 1.570796f); /* 482 */
    float osc1_4_3 = (s1_wrap(osc1_phase_3) + osc1_phase_3) * 0.5f * t482;                       /* 483 */
    float osc1 = s1_wrap(osc1_2_3 + t417);                                                       /* 485 */
    float t486 = s1_wrap(osc1_phase_3 + t417);                                                   /* 486 */
    float t488 = 1.5688642f * osc1_state_prev3 + (0.054386564f * osc1_4 + (0.10877313f * osc1_4_2 + osc1_4_3 * 0.054386564f)); /* 488 */
    float osc1_state_prev4 = t488 - 0.7864105f * osc1_state_prev2;                               /* 488 */
    float osc1_phase_4 = SEL(K_LT0(t486), osc1, t486);                                           /* 489 */
    float t501 = K_SIN(K_CLAMP(K_TRI(0.5f * osc1_phase_4 + 0.5f) * 256.0f * osc1_recip * 0.25f) * 1.570796f); /* 501 */
    float osc1_3 = (s1_wrap(osc1_phase_4) + osc1_phase_4) * 0.5f * t501;                         /* 502 */
    float t505 = 1.5688642f * osc1_state_prev4 + (0.10877313f * osc1_4_3 + osc1_3 * 0.054386564f + 0.054386564f * osc1_4_2); /* 505 */
    float osc1_5_prev = t505 - 0.7864105f * osc1_state_prev3;                                    /* 505 */
    float t508 = -(-0.00062781386f * osc1_state_prev2) - -0.00062781386f * osc1_5_prev_z7 - -0.00181091f * osc1_state_prev3; /* 508 */
    float t509 = t508 - -0.00181091f * osc1_state_prev4_z7 - -0.0025511528f * osc1_state_prev4 - -0.0025511528f * osc1_state_prev3_z7; /* 509 */
    float t511 = t509 - -0.0015808088f * osc1_5_prev - -0.0015808088f * osc1_state_prev2_z7 - 0.002359534f * osc1_state_prev2_z1; /* 511 */
    float t512 = t511 - 0.002359534f * osc1_5_prev_z6 - 0.008297408f * osc1_state_prev3_z1 - 0.008297408f * osc1_state_prev4_z6; /* 512 */
    float t514 = t512 - 0.011753906f * osc1_state_prev4_z1 - 0.011753906f * osc1_state_prev3_z6 - 0.006730832f * osc1_5_prev_z1; /* 514 */
    float t515 = t514 - 0.006730832f * osc1_state_prev2_z6 - -0.009135877f * osc1_state_prev2_z2 - -0.009135877f * osc1_5_prev_z5; /* 515 */
    float t517 = t515 - -0.029605705f * osc1_state_prev3_z2 - -0.029605705f * osc1_state_prev4_z5 - -0.03964878f * osc1_state_prev4_z2; /* 517 */
    float t518 = t517 - -0.03964878f * osc1_state_prev3_z5 - -0.022207731f * osc1_5_prev_z2 - -0.022207731f * osc1_state_prev2_z5; /* 518 */
    float t520 = t518 - 0.030897303f * osc1_state_prev2_z3 - 0.030897303f * osc1_5_prev_z4 - 0.110675454f * osc1_state_prev3_z3; /* 520 */
    float t521 = t520 - 0.110675454f * osc1_state_prev4_z4 - 0.19164494f * osc1_state_prev4_z3 - 0.19164494f * osc1_state_prev3_z4; /* 521 */
    float osc1_6 = t521 - 0.24270383f * osc1_5_prev_z3 - 0.24270383f * osc1_state_prev2_z4;      /* 522 */
    float osc1_out = osc1_6 * -0.5f;                                                             /* 524 */
    float c_41e = s1_range(s1_import(1), -3.001f, 3.001f);                                       /* 530 */

    /* ---- mixer (MIX balance), HPF (one-pole lowpass subtracted) ---- */
    float mix = KP(P_MIX1) * osc1_out_z1 + KP(P_MIX2) * c_41e;                                   /* 540 */
    float reso = KP(P_RESO);                                                                     /* 544 */
    float lfo = s1_range(s1_import(2), -2.251f, 2.251f);                                         /* 546 */
    float env1_out = s1_range(s1_import(3), -0.376f, 0.376f);                                    /* 546 */
    float env2_out = s1_range(s1_import(4), -0.376f, 0.376f);                                    /* 547 */
    float pitch = s1_range(s1_import(5), -5.501f, 15.501f);                                      /* 547 */

    /* ---- filter: cutoff (ENV1, KEY FOLLOW, LFO, CUTOFF) -> octaves -> g = tan / (1 + tan); 4 one-pole stages, resonance feedback ---- */
    float cutoff_mod = env1_out * KP(P_ENVMOD) + ((pitch - 4.0f) * KP(P_KEYFOLLOW) + KP(P_LFO_FILTER) * lfo); /* 554 */
    float hpf_out = mix - hpf_lp_z1;                                                             /* 558 */
    float hpf_lp = LERP(hpf_lp_z1, mix, 7.387803e-05f);                                          /* 559 */
    float filter = 6.0f * reso;                                                                  /* 566 */
    float t568 = fminf((1.0f - reso) * 0.5f, 0.075f) + 0.16659999f + (10.625f * KP(P_CUTOFF) + cutoff_mod); /* 568 */
    float t569 = t568 * 0.25f;                                                                   /* 569 */
    float t570 = t569 * t569;                                                                    /* 570 */
    float t571 = t570 * t569;                                                                    /* 571 */
    float t572 = t568 - 11.0f;                                                                   /* 572 */
    float t573 = 2.3801785f * (t571 * t569) + (54.536343f * t570 - 48.64617f * t569 - 19.821596f * t571); /* 573 */
    float t574 = SEL(K_GE0(t572), t573 * 4.0f, SEL(K_LT0(t572), t568, t573));                    /* 574 */
    float cutoff_oct = fmaxf(SEL(K_GT0(t572), LERP(11.0f, t568, 0.0009999871f), t574), -3.0f);   /* 576 */
    float t586 = K_EXP(0.6931472f * cutoff_oct - 3.292449f) * 0.0035997033f;                     /* 586 */
    float t587 = t586 * t586;                                                                    /* 587 */
    float t589 = t587 * t587;                                                                    /* 589 */
    float t591 = K_SIN(t586);                                                                    /* 591 */
    float t594 = t589 * (0.041666627f - 0.0013888478f * t587 + t589 * (2.4795532e-05f - 2.3841858e-07f * t587)); /* 594 */
    float filter_tan = t591 * K_RECIP_POS(t591 + (1.0f - 0.5f * t587 + t594));                                /* 604  micro_fix */
    float filter_g = filter_tan * 2.0f;                                                          /* 612 */
    float t612 = filter * 0.8999996f;                                                            /* 612 */
    float t613 = filter_g * filter_g;                                                            /* 613 */
    float t616 = K_RECIP_POS(1.0f + t612 * (t613 * t613));                                           /* 616  micro_fix */
    float t624 = 1.0f - 2.0f * filter_g;                                                         /* 624 */
    float t625 = K_CLAMP(hpf_out * 0.119999945f * t616 - t616 * t612 * K_CLAMP((filter_fb_z1 + 0.1f * filter_fb_z3) * 0.6f)); /* 625 */
    float filter_in = t625 * -0.33333302f * (t625 * t625) + t625;                                /* 627 */
    float filter_p1 = filter_g * (filter_in + filter_in_z1) + t624 * filter_p1_z1;               /* 629 */
    float filter_p2 = filter_g * (filter_p1 + filter_p1_z1) + t624 * filter_p2_z1;               /* 631 */
    float filter_p3 = filter_g * (filter_p2 + filter_p2_z1) + t624 * filter_p3_z1;               /* 633 */
    float t634 = filter_g * (filter_g * (filter_g * filter_in + filter_p1 + t624 * filter_p1) + filter_p2 + t624 * filter_p2) + filter_p3; /* 634 */
    float filter_p4 = filter_g * (filter_p3 + filter_p3_z1) + t624 * filter_p4_z1;               /* 635 */
    float filter_fb = filter_g * (t634 + t624 * filter_p3) + t624 * filter_p4;                   /* 636 */

    /* ---- VCA: level ramp, ENV2 curve, output ---- */
    float vca_level = fmaxf(0.0075531006f + vca_level_z1 - 0.0075531006f * vca_level_z1, 0.0f);  /* 648 */
    float t666 = fmaxf(0.444274f * (env2_out * env2_out * env2_out) - 0.005f + 0.7f * (env2_out * 0.265983f), 0.0f); /* 666 */
    float vca_out = fminf((filter_p4 + reso * 0.5f * filter_p4) * t666, 5.5f) * 1.1999998f;      /* 671 */
    float vca = vca_out * vca_level;                                                             /* 677 */
    KOUT(OUT_VOICE, vca);

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
    osc1_out_z1 = osc1_out;
}

