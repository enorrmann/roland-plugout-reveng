/* bq_jp08_host2x_voice.h: the JP08 voice (Boutique 1.21 PX-11:Appli, DSP chip 0: voice A 0x1900, OSC1 variant 2, OSC2 variant 3) as C, the model's 59 immediate patches and 3 rate conversions (44.1 -> 96 kHz) applied; envelopes and LFO from the host (--host, README.md §8). Written by tools/make_voice.py with sdk/s1cc/tools/dsplift.py; do not edit: README.md §2. */
#include "dsplift_k.h"

/* jp08_host2x_voice: written by sdk/s1cc/tools/dsplift.py --kernel. Every operation of the program, in order, as
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
static inline __attribute__((always_inline)) void jp08_host2x_voice(void)
{
    float pitch = KIN(IN_PITCH);                                                                 /* 1 */
    float env_filter = KIN(IN_ENV_FILTER);                                                       /* 95 */

    /* ---- ENV1 (filter envelope, also pitch / PWM source): attack to 1.05 ends above 0.8925, decay to SUSTAIN x 0.87, release ---- */
    float env1_out = env_filter * env_filter * -0.09999484f * -10.0f;                            /* 98 */
    float env_amp = KIN(IN_ENV_AMP);                                                             /* 142 */

    /* ---- ENV2 (VCA envelope) ---- */
    float env2_out = env_amp * env_amp * -0.09999484f * -10.0f;                                  /* 145 */
    float osc2_out = KIN(IN_OSC2_OUT);                                                           /* 149 */
    float osc1_mod = osc2_out * KP(P_XMOD);                                                      /* 161 */
    float pwm_2 = KIN(IN_PWM) * 1.2f;                                                            /* 166 */
    KOUT(OUT_OSC2_PW, pwm_2);
    float osc2_pitch = pitch + KP(P_TUNE2);                                                      /* 171 */
    float osc1_pitch = pitch + osc1_mod;                                                         /* 172 */
    KOUT(OUT_OSC1_PITCH, osc1_pitch);
    KOUT(OUT_OSC2_PITCH, osc2_pitch);

    /* ---- mixer (MIX balance), HPF (one-pole lowpass subtracted) ---- */
    float mix = KP(P_MIX1) * KIN(IN_OSC1_OUT) + KP(P_MIX2) * osc2_out;                           /* 195 */
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
}

