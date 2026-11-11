/* bq_jp08_arm_voice.h: the JP08 voice (Boutique 1.21 PX-11:Appli, DSP chip 0: voice A 0x1900, OSC1 variant 2, OSC2 variant 3) as C, the model's 59 immediate patches and 3 rate conversions (44.1 -> 96 kHz) applied; envelopes and LFO from the host (--host, README.md §8). Written by tools/make_voice.py with sdk/s1cc/tools/dsplift.py; do not edit: README.md §2. */
#include "dsplift_k.h"

/* jp08_arm_voice: written by sdk/s1cc/tools/dsplift.py --kernel. Every operation of the program, in order, as
 * expressions (dsplift.md §6): a name per value that is used more than once, stored in bank 0 or too long;
 * the number after each line = the instruction that makes it. SEL(w, a, b) = w ? a : b (w = 0 / 1), LERP(a, b, t) = a + t (b - a),
 * K_NZ / K_GE0 / K_LE0 = the condition bits, K_RECIP = the reciprocal (dsplift_k.h). Delay chains: X_zK =
 * what cell X held K samples ago. */
#define OUT_OSC1_PITCH 0x200a
#define OUT_OSC2_PW 0x200b
#define OUT_OSC2_PITCH 0x200f
#define OUT_VOICE 0x203a
#define IN_PITCH 0xa080
#define IN_PWM 0xa090
#define IN_ENV_FILTER 0xa0f0
#define IN_ENV_AMP 0xa0f1
#define IN_LFO_FILTER 0xa0f2
#define IN_OSC1_OUT 0xa0f3
#define IN_OSC2_OUT 0xa0f4
static float D_0b0_1;
static float D_0b7_1;
static float D_0b8_1;
static float D_0b9_1;
static float D_0ba_1;
static float D_0bb_1;
static float D_0bc_1, D_0bc_2, D_0bc_3;
static float D_0c8_1;
static inline __attribute__((always_inline)) void jp08_arm_voice(void)
{
    float pitch = KIN(IN_PITCH);                                                                 /* 1 */
    float env_filter = KIN(IN_ENV_FILTER);                                                       /* 95 */
    float c_081 = env_filter * env_filter * -0.09999484f * -10.0f;                               /* 98 */
    float env_amp = KIN(IN_ENV_AMP);                                                             /* 142 */
    float c_08e = env_amp * env_amp * -0.09999484f * -10.0f;                                     /* 145 */
    float osc2_out = KIN(IN_OSC2_OUT);                                                           /* 149 */
    float c_093 = osc2_out * KP(0xc04d);                                                         /* 161 */
    float c_40b = KIN(IN_PWM) * 1.2f;                                                            /* 166 */
    KOUT(OUT_OSC2_PW, c_40b);
    float c_40f = pitch + KP(0xc046);                                                            /* 171 */
    float c_40a = pitch + c_093;                                                                 /* 172 */
    KOUT(OUT_OSC1_PITCH, c_40a);
    KOUT(OUT_OSC2_PITCH, c_40f);
    float c_0a9 = KP(0xc05a) * KIN(IN_OSC1_OUT) + KP(0xc05b) * osc2_out;                         /* 195 */
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
    float c_0b2 = t246 * K_RECIP_POS(t246 + (1.0f - 0.5f * t242 + t249));                                     /* 259  micro_fix */
    float c_0b6 = KIN(0xa0f5);                                                                  /* 267  arm_fix: g from the ARM */
    float t267 = c_0b3 * 0.8999996f;                                                             /* 267 */
    float t268 = c_0b6 * c_0b6;                                                                  /* 268 */
    float t271 = K_RECIP_POS(1.0f + t267 * (t268 * t268));                                           /* 271  micro_fix */
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
    float t321 = KIN(0xa0f6); /* 321  arm_fix: the VCA gain from the ARM */
    float c_0cf = fminf((c_0bb + c_0ad * 0.5f * c_0bb) * t321, 5.5f) * 1.1999998f;               /* 326 */
    float c_43a = c_0cf * c_0c8;                                                                 /* 332 */
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
}

