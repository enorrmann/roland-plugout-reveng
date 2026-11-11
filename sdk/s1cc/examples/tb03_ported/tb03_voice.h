/* tb03_voice.h: the TB-03 voice program (BQ2:Appli 1.07, voice DSP 0x0022, 751 instructions) as C, in two parts
 * (tb03_voice_a: instructions 0..414 on DSP A, tb03_voice_b: 415..750 on DSP B), for the TB-03 setup: the
 * parameters no knob moves are the init's values (tb03_tables.h), names from tb03_names.txt. Written by
 * sdk/s1cc/tools/dsplift.py --bq2 --kernel from the firmware (tools/make_voice.sh); do not edit: README.md §2. */
#include "dsplift_k.h"

/* tb03_voice: written by sdk/s1cc/tools/dsplift.py --kernel --split 415. Every operation of the program, in order, as
 * expressions (dsplift.md §6): a name per value that is used more than once, stored in bank 0 or too long;
 * the number after each line = the instruction that makes it. SEL(w, a, b) = w ? a : b (w = 0 / 1), LERP(a, b, t) = a + t (b - a),
 * K_NZ / K_GE0 / K_LE0 = the condition bits, K_RECIP = the reciprocal (dsplift_k.h). Delay chains: X_zK =
 * what cell X held K samples ago. */
#define IN_PITCH 0xa004
#define IN_GATE 0xa005
#define IN_ACCENT 0xa006
#define IN_SLIDE 0xa007
#define OUT_VOICE 0xa100
#define P_DECAY 0xc00d
#define P_TUNE 0xc010
#define P_CUTOFF 0xc019
#define P_RESONANCE 0xc01a
#define P_ENVMOD 0xc01b
#define P_ACCENT_LEVEL 0xc01c
#define P_SAW_LEVEL 0xc029
#define P_SQUARE_LEVEL 0xc02a
static float glide_err_z1;   /* part A */
static float glide_step_z1;   /* part A */
static float glide_pitch_z1;   /* part A */
static float amp_att_z1;   /* part A */
static float amp_att_x_z1;   /* part A */
static float amp_lvl_z1;   /* part A */
static float amp_dec_z1;   /* part A */
static float amp_env_z1;   /* part A */
static float gate_prev_z1;   /* part A */
static float fenv_att_z1;   /* part A */
static float fenv_att_x_z1;   /* part A */
static float fenv_lvl_z1;   /* part A */
static float fenv_dec_z1;   /* part A */
static float fenv_z1;   /* part A */
static float fenv_clip2_z1;   /* part A */
static float saw_phase_z1;   /* part A */
static float saw_s0_z1, saw_s0_z2, saw_s0_z3;   /* part A */
static float saw_s1_z1, saw_s1_z2, saw_s1_z3;   /* part A */
static float saw_s2_z1, saw_s2_z2, saw_s2_z3;   /* part A */
static float saw_s3_z1, saw_s3_z2, saw_s3_z3;   /* part A */
static float saw_lp_z1;   /* part B */
static float sq_phase_z1;   /* part B */
static float sq_s0_z1, sq_s0_z2, sq_s0_z3;   /* part B */
static float sq_s1_z1, sq_s1_z2, sq_s1_z3;   /* part B */
static float sq_s2_z1, sq_s2_z2, sq_s2_z3;   /* part B */
static float sq_s3_z1, sq_s3_z2, sq_s3_z3;   /* part B */
static float sq_leak_z1;   /* part B */
static float sq_slew_z1;   /* part B */
static float square_lp_z1;   /* part B */
static float svf_bp_z1;   /* part B */
static float svf_lp_z1;   /* part B */
static float filter_lp_z1;   /* part B */
static float filter_dc_z1;   /* part B */
static float post_s1_z1;   /* part B */
static float post_s2_z1;   /* part B */
static float amp_gain_z1;   /* part B */
static float accent_gain_z1;   /* part B */
/* the parts' arguments: what the including file's KIN / KP / KOUT use (default: none) */
#ifndef KARGS_A
#define KARGS_A void
#endif
#ifndef KARGS_B
#define KARGS_B void
#endif
/* handed from part A to part B: s1_export / s1_import slots 0.. (r0 r1 r2 c_033 c_067 c_068 c_041 c_04e c_055) */
#define TB03_VOICE_SLOTS 9
static inline __attribute__((always_inline)) void tb03_voice_a(KARGS_A)
{
    /* ---- pitch: glide (slide) and TUNE ---- */
    float pitch = KIN(IN_PITCH);                                                                 /* 66 */
    float gate = KIN(IN_GATE);                                                                   /* 67 */
    float note_accent = KIN(IN_ACCENT);                                                          /* 68 */
    float glide_err = glide_err_z1 - 256.0f * (glide_step_z1 * s1_recip(0.118935496f)) - glide_pitch_z1 + pitch; /* 81 */
    float glide_step = glide_err * 0.00046459178f;                                               /* 82 */
    float glide_pitch = glide_step + glide_pitch_z1;                                             /* 83 */
    float osc_pitch = KP(P_TUNE) + XFADE(pitch, glide_pitch, KIN(IN_SLIDE));                     /* 106 */

    /* ---- VCA envelope (from the gate) ---- */
    float t117 = XFADE(gate, XFADE(amp_att_z1, amp_env_z1, fmaxf(gate - gate_prev_z1, 0.0f)), 0.9869229f); /* 117 */
    float amp_att = SEL(K_GT0(gate - amp_att_z1), t117, gate);                                   /* 117 */
    float amp_att_x = amp_att * 1.0998535f;                                                      /* 118 */
    float t119 = amp_att_x - 1.0f;                                                               /* 119 */
    float t119_2 = amp_att_x - 0.099999964f;                                                     /* 119 */
    float t122 = amp_att_x - amp_att_x_z1;                                                       /* 122 */
    float t124 = SEL(K_GT0(t122), SEL(K_LE0(t119), amp_lvl_z1, SEL(K_GT0(t119), 1.0f, t119)), t122); /* 124 */
    float amp_lvl = SEL(K_LE0(t122), SEL(K_GE0(t119_2), amp_lvl_z1, SEL(K_LT0(t119_2), 0.0f, t119_2)), t124); /* 124 */
    float t125 = 1.0f - amp_lvl;                                                                 /* 125 */
    float t128 = SEL(K_LT0(amp_att * t125 - amp_dec_z1), 0.99999565f * amp_dec_z1 - 5.6880526e-07f, t125 * amp_att); /* 128 */
    float amp_dec = fmaxf(t128, 0.0f) * gate;                                                    /* 130 */
    float amp_env = fmaxf(SEL(K_Z(amp_att), 0.9979415f * amp_env_z1 - 1.2499513e-06f, amp_dec), 0.0f); /* 135 */

    /* ---- filter envelope; an accented note decays with 0.1865 instead of DECAY (README.md §4) ---- */
    float decay_eff = XFADE(KP(P_DECAY), 0.18653011f, note_accent);                              /* 143 */
    float t146 = 0.00023531099f * decay_eff + 0.999821f - 7.133279e-05f * (decay_eff * decay_eff); /* 146 */
    float fenv_coef = SEL(K_GE0(-decay_eff), 0.999821f, SEL(K_LE0(1.0f - decay_eff), 0.999985f, t146)); /* 151 */
    float fenv_att = SEL(K_GT0(gate - fenv_att_z1), XFADE(gate, fenv_att_z1, 0.981957f), gate);  /* 159 */
    float fenv_att_x = fenv_att * 1.0998535f;                                                    /* 160 */
    float t161 = fenv_att_x - 1.0f;                                                              /* 161 */
    float t161_2 = fenv_att_x - 0.099999964f;                                                    /* 161 */
    float t165 = fenv_att_x - fenv_att_x_z1;                                                     /* 165 */
    float t167 = SEL(K_GT0(t165), SEL(K_LE0(t161), fenv_lvl_z1, SEL(K_GT0(t161), 1.0998535f, t161)), t165); /* 167 */
    float fenv_lvl = SEL(K_LE0(t165), SEL(K_GE0(t161_2), fenv_lvl_z1, SEL(K_LT0(t161_2), 0.0f, t161_2)), t167); /* 167 */
    float t168 = 1.0998535f - fenv_lvl;                                                          /* 168 */
    float fenv_dec = SEL(K_LT0(fenv_att * t168 - fenv_dec_z1), fenv_dec_z1 * fenv_coef, t168 * fenv_att) * gate; /* 172 */
    float fenv = SEL(K_Z(fenv_att), fenv_z1 * 0.9998848f, fenv_dec);                             /* 176 */
    float accent_env = fenv * note_accent;                                                       /* 179 */

    /* ---- cutoff / resonance / env mod / accent -> the SVF's coefficients ---- */
    float cutoff = KP(P_CUTOFF);                                                                 /* 184 */
    float resonance = KP(P_RESONANCE);                                                           /* 184 */
    float envmod = KP(P_ENVMOD);                                                                 /* 185 */
    float accent_level = KP(P_ACCENT_LEVEL);                                                     /* 185 */
    float fenv_clip = SEL(K_LT0(fenv), -0.0f, SEL(K_GT0(fenv - 1.0f), 1.0f, fenv));              /* 195 */
    float t200 = cutoff * cutoff;                                                                /* 200 */
    float cutoff_curve = 0.7676888f * (t200 * cutoff) + (2.5798087f * cutoff - 2.3474975f * t200); /* 202 */
    float fenv_clip2 = fenv_clip + fenv_clip2_z1 - fenv_clip2_z1;                                /* 203 */
    float t204 = K_CLAMP(2.0f * accent_level - 2.0f) + 1.0f;                                     /* 204 */
    float t209 = accent_env * (1.0f - 0.79999924f * resonance) * (t204 * (0.023929149f * cutoff + 0.06f - 0.013929149f * t200)); /* 209 */
    float t210 = accent_env - 0.44f;                                                             /* 210 */
    float t211 = accent_env - 0.44055998f;                                                       /* 211 */
    float t213 = XFADE(accent_env, SEL(K_LE0(t210), accent_env, SEL(K_GT0(t210), 0.4400003f - t211 * t211, t211)), resonance); /* 213 */
    float t214 = accent_env * 8.0f;                                                              /* 214 */
    float t216 = 1.0f - 4.0f * fminf(envmod, 0.25f);                                             /* 216 */
    float c_064 = 1.0f + t216 * t216 * (t214 * t204);                                            /* 218 */
    float t219 = XFADE(t213 * (t204 * (0.017337462f * t200 + (0.06766254f * cutoff + 0.065f))), t209, envmod); /* 219 */
    float t221 = 0.6574785f * t200 + (0.11899884f * cutoff + 1.12f);                             /* 221 */
    float c_062 = K_CLAMP(t214);                                                                 /* 222 */
    float t227 = envmod * (0.08606291f * cutoff + 0.93f - 0.016062908f * t200);                  /* 227 */
    float c_064_2 = K_CLAMP(fenv_clip2 * c_064);                                                 /* 227 */
    float t228 = t227 * t227;                                                                    /* 228 */
    float t230 = 0.11192168f * t228 + (0.03161107f * t227 + 0.019f) - 0.022444464f * (t228 * t227); /* 230 */
    float t233 = envmod * (0.12862271f * t200 + (0.07137729f * cutoff + 0.8f));                  /* 233 */
    float t234 = t233 * t233;                                                                    /* 234 */
    float t237 = -0.015015462f * t234 - 0.029156359f * t233 - 0.00082817907f * (t234 * t233) + t219; /* 237 */
    float t237_2 = t237 + fenv_clip * XFADE(0.94751227f * cutoff + 0.8f - 0.5175123f * t200, t221, resonance) * t230; /* 237 */
    float t238 = c_062 * 2.0f;                                                                   /* 238 */
    float t243 = (XFADE(1.665f, XFADE(7.0f, 5.0f, cutoff_curve), t233 * resonance * resonance) + (t238 - cutoff_curve * t238)) * c_064_2 + 1.0f; /* 243 */
    float t244 = resonance * resonance;                                                          /* 244 */
    float svf_q_norm = K_RECIP(t243);                                                            /* 244 */
    float t248 = t244 * resonance;                                                               /* 248 */
    float t249 = t237_2 + (0.0605765f * cutoff + 3.730545f);                                     /* 249 */
    float t253 = SEL(K_GT0(t249 - 4.4f), 4.106f, 3.7854369f * t249 - 4.2083807f - 0.43086475f * (t249 * t249)); /* 253 */
    float t260 = 0.0013333333f * t248 + (0.0061666667f * resonance + 0.9935f - 0.0029999998f * t244); /* 260 */
    float t261 = XFADE(t260, 0.0023999999f * t248 + (0.0107f * resonance + 0.9935f - 0.0066f * t244), cutoff); /* 261 */
    float t263 = SEL(K_GE0(t249 - 4.0576f), t253, t249) * t261;                                  /* 263 */
    float t264 = 21.09999f * t244 + (8.0f - 21.583328f * resonance) - 7.0666656f * t248;         /* 264 */
    float t268 = (1.7405409f * (t263 * t263) + (24.007244f - 12.927981f * t263)) * 8.0f;         /* 268 */
    float t269 = t263 - 3.714f;                                                                  /* 269 */
    float svf_q = XFADE(t264, 27.110977f * t244 + (8.0f - 25.055511f * resonance) - 9.999908f * t248, cutoff_curve) * svf_q_norm; /* 270 */
    float t271 = SEL(K_LT0(t269), 0.021989455f * t263 - 0.0705624f, t263);                       /* 271 */
    float svf_f = SEL(K_GE0(t269), SEL(K_GT0(t263 - 4.0917f), 1.9997989f, t268), t271);          /* 271 */
    float t303 = SEL(K_LT0(svf_f - 1.0f), K_RECIP(svf_f + 4.656613e-10f), 2.0f - svf_f);         /* 303 */
    float t304 = SEL(K_GE0(svf_f - 2.0f), 0.0f, t303);                                           /* 304 */
    float svf_damp = SEL(K_GE0(svf_q - t304), t304, svf_q);                                      /* 307 */

    /* ---- saw oscillator: four sub-samples per sample (rounded edge), 20-tap FIR decimation ---- */
    float saw_pitch = SEL(K_LT0(osc_pitch + 2.0f), -2.0f, SEL(K_GT0(osc_pitch - 10.0f), 10.0f, osc_pitch)); /* 317 */
    float saw_freq = K_EXP((saw_pitch - 4.75f) * 0.6931472f);                                    /* 328 */
    float osc_inc_raw = saw_freq * 0.0022916666f;                                                /* 331 */
    float osc_inc = SEL(K_Z(osc_inc_raw), 2.3283064e-10f, osc_inc_raw);                          /* 333 */
    float saw_phase0 = s1_wrap(saw_phase_z1 + osc_inc);                                          /* 336 */
    float osc_edge = K_RECIP(osc_inc * 256.0f);                                                  /* 338 */
    float saw_s0 = saw_phase0 * K_SIN(K_CLAMP(K_TRI(0.5f * saw_phase0 + 0.5f) * 256.0f * osc_edge) * 1.570796f); /* 352 */
    float saw_phase1 = s1_wrap(saw_phase0 + osc_inc);                                            /* 353 */
    float saw_s1 = saw_phase1 * K_SIN(K_CLAMP(K_TRI(0.5f * saw_phase1 + 0.5f) * 256.0f * osc_edge) * 1.570796f); /* 369 */
    float saw_phase2 = s1_wrap(saw_phase1 + osc_inc);                                            /* 370 */
    float saw_s2 = saw_phase2 * K_SIN(K_CLAMP(K_TRI(0.5f * saw_phase2 + 0.5f) * 256.0f * osc_edge) * 1.570796f); /* 386 */
    float saw_phase = s1_wrap(saw_phase2 + osc_inc);                                             /* 387 */
    float saw_s3 = saw_phase * K_SIN(K_CLAMP(K_TRI(0.5f * saw_phase + 0.5f) * 256.0f * osc_edge) * 1.570796f); /* 403 */
    float t405 = -(0.009056807f * saw_s0) - 0.009056807f * saw_s3_z3 - 0.013877809f * saw_s1 - 0.013877809f * saw_s2_z3; /* 405 */
    float t407 = t405 - 0.02744174f * saw_s2 - 0.02744174f * saw_s1_z3 - 0.04779297f * saw_s3;   /* 407 */
    float t408 = t407 - 0.04779297f * saw_s0_z3 - 0.07156843f * saw_s0_z1 - 0.07156843f * saw_s3_z2; /* 408 */
    float t410 = t408 - 0.09462041f * saw_s1_z1 - 0.09462041f * saw_s2_z2 - 0.11281258f * saw_s2_z1; /* 410 */
    float saw = t410 - 0.11281258f * saw_s1_z2 - 0.12282902f * saw_s3_z1 - 0.12282902f * saw_s0_z2; /* 411 */

    s1_export(0, saw);
    s1_export(1, cutoff);
    s1_export(2, resonance);
    s1_export(3, osc_pitch);
    s1_export(4, svf_f);
    s1_export(5, svf_damp);
    s1_export(6, amp_env);
    s1_export(7, accent_env);
    s1_export(8, accent_level);
    glide_err_z1 = glide_err;
    glide_step_z1 = glide_step;
    glide_pitch_z1 = glide_pitch;
    amp_att_z1 = amp_att;
    amp_att_x_z1 = amp_att_x;
    amp_lvl_z1 = amp_lvl;
    amp_dec_z1 = amp_dec;
    amp_env_z1 = amp_env;
    gate_prev_z1 = gate;
    fenv_att_z1 = fenv_att;
    fenv_att_x_z1 = fenv_att_x;
    fenv_lvl_z1 = fenv_lvl;
    fenv_dec_z1 = fenv_dec;
    fenv_z1 = fenv;
    fenv_clip2_z1 = fenv_clip2;
    saw_phase_z1 = saw_phase;
    saw_s0_z3 = saw_s0_z2;
    saw_s0_z2 = saw_s0_z1;
    saw_s0_z1 = saw_s0;
    saw_s1_z3 = saw_s1_z2;
    saw_s1_z2 = saw_s1_z1;
    saw_s1_z1 = saw_s1;
    saw_s2_z3 = saw_s2_z2;
    saw_s2_z2 = saw_s2_z1;
    saw_s2_z1 = saw_s2;
    saw_s3_z3 = saw_s3_z2;
    saw_s3_z2 = saw_s3_z1;
    saw_s3_z1 = saw_s3;
}
static inline __attribute__((always_inline)) void tb03_voice_b(KARGS_B)
{
    /* ---- saw shaping: one-pole lowpass of the saw, resonance / cutoff dependent gains ---- */
    float saw = s1_range(s1_import(0), -1.499741f, 1.49971f);                                    /* 415 */
    float cutoff = s1_range(s1_import(1), -0.251f, 1.251f);                                      /* 415 */
    float resonance = s1_range(s1_import(2), -0.10394118f, 1.2215883f);                          /* 415 */
    float t415 = cutoff * cutoff;                                                                /* 415 */
    float t418 = resonance * resonance;                                                          /* 418 */
    float saw_lp = LERP(saw_lp_z1, saw, (0.73957944f * cutoff + 0.43f - 0.16957945f * t415) * 0.002814293f); /* 419 */
    float t419 = 3.25f * resonance - 1.7f * t418;                                                /* 419 */
    float t422 = t418 * resonance;                                                               /* 422 */
    float t423 = 1.61f * t418 + (0.425f - 1.3716667f * resonance) - 0.6333333f * t422;           /* 423 */
    float t425 = t415 * cutoff;                                                                  /* 425 */
    float t426 = 0.10442992f * t425 + (0.26471496f * cutoff + 0.9f - 0.2691449f * t415);         /* 426 */
    float saw_shaped = (t419 * saw_lp + (saw - saw_lp_z1)) * t423 * t426;                        /* 427 */

    /* ---- square oscillator: pulse width from the pitch, four sub-samples, the same FIR ---- */
    float osc_pitch = s1_range(s1_import(3), -1.69875f, 8.62775f);                               /* 429 */
    float sq_pitch = SEL(K_LT0(osc_pitch + 2.0f), -2.0f, SEL(K_GT0(osc_pitch - 10.0f), 10.0f, osc_pitch)); /* 436 */
    float sq_freq = K_EXP((sq_pitch - 4.75f) * 0.6931472f);                                      /* 447 */
    float t450 = sq_pitch * sq_pitch;                                                            /* 450 */
    float t453 = SEL(K_LE0(3.6666698f - sq_pitch), 0.56444f, 0.169f * sq_pitch + 0.254f - 0.023f * t450); /* 453 */
    float sq_pw = SEL(K_GE0(-sq_pitch), 0.254f, t453);                                           /* 456 */
    float t460 = sq_freq * 0.0022916666f;                                                        /* 460 */
    float sq_inc = SEL(K_Z(t460), 2.3283064e-10f, t460);                                         /* 462 */
    float sq_thr = 1.0f - 2.0f * sq_pw;                                                          /* 463 */
    float sq_ph = s1_wrap(sq_phase_z1 + sq_inc);                                                 /* 465 */
    float sq_lim = K_CLAMP(0.5007324f - 0.43432617f * sq_thr);                                   /* 466 */
    float t469 = sq_ph + sq_thr;                                                                 /* 469 */
    float t469_ge0 = K_GE0(t469);                                                                /* 469 */
    float t469_lt0 = K_LT0(t469);                                                                /* 471 */
    float t471 = SEL(t469_lt0, 0.0f, SEL(t469_ge0, 1.0f, 0.254f));                               /* 471 */
    float sq_leak = 0.00055629015f * SEL(t469_lt0, 1.0f, SEL(t469_ge0, -1.0f, t419)) + 0.99944305f * sq_leak_z1; /* 474 */
    float t476 = sq_inc * 65.0f;                                                                 /* 476 */
    float sq_slew = SEL(K_GT0(t471 - sq_slew_z1), XFADE(sq_slew_z1, t471, t476), t471);          /* 477 */
    float t480 = 3.0f * sq_lim;                                                                  /* 480 */
    float t480_2 = 1.0f - 3.0f * fmaxf(0.5f - 0.5f * sq_ph, sq_lim) + t480;                      /* 480 */
    float sq_edge = K_RECIP(sq_inc * 256.0f);                                                    /* 487 */
    float t492 = K_TRI(K_RECIP(sq_thr + SEL(t469_lt0, -1.0f, SEL(t469_ge0, 1.0f, t450))) * t469) * 256.0f * sq_edge; /* 492 */
    float sq_s0 = (-0.23799992f * sq_leak - 0.76199913f) * sq_slew * t480_2 * K_CLAMP(t492 * 0.0625f); /* 495 */
    float sq_ph_2 = s1_wrap(sq_ph + sq_inc);                                                     /* 496 */
    float t498 = sq_ph_2 + sq_thr;                                                               /* 498 */
    float t498_ge0 = K_GE0(t498);                                                                /* 498 */
    float t498_lt0 = K_LT0(t498);                                                                /* 500 */
    float t500 = SEL(t498_lt0, 0.0f, SEL(t498_ge0, 1.0f, sq_edge));                              /* 500 */
    float sq_leak_2 = 0.00055629015f * SEL(t498_lt0, 1.0f, SEL(t498_ge0, -1.0f, 1.0f)) + 0.99944305f * sq_leak; /* 503 */
    float sq_slew_2 = SEL(K_GT0(t500 - sq_slew), XFADE(sq_slew, t500, t476), t500);              /* 506 */
    float t509 = 1.0f - 3.0f * fmaxf(0.5f - 0.5f * sq_ph_2, sq_lim) + t480;                      /* 509 */
    float t523 = K_CLAMP(K_TRI(K_RECIP(sq_thr + SEL(t498_lt0, -1.0f, 1.0f)) * t498) * 256.0f * sq_edge * 0.0625f); /* 523 */
    float sq_s1 = (-0.23799992f * sq_leak_2 - 0.76199913f) * sq_slew_2 * t509 * t523;            /* 524 */
    float sq_ph_3 = s1_wrap(sq_ph_2 + sq_inc);                                                   /* 525 */
    float t527 = sq_ph_3 + sq_thr;                                                               /* 527 */
    float t527_ge0 = K_GE0(t527);                                                                /* 527 */
    float t527_lt0 = K_LT0(t527);                                                                /* 529 */
    float t529 = SEL(t527_lt0, 0.0f, SEL(t527_ge0, 1.0f, sq_edge));                              /* 529 */
    float sq_leak_3 = 0.00055629015f * SEL(t527_lt0, 1.0f, SEL(t527_ge0, -1.0f, 1.0f)) + 0.99944305f * sq_leak_2; /* 532 */
    float sq_slew_3 = SEL(K_GT0(t529 - sq_slew_2), XFADE(sq_slew_2, t529, t476), t529);          /* 535 */
    float t538 = 1.0f - 3.0f * fmaxf(0.5f - 0.5f * sq_ph_3, sq_lim) + t480;                      /* 538 */
    float t552 = K_CLAMP(K_TRI(K_RECIP(sq_thr + SEL(t527_lt0, -1.0f, 1.0f)) * t527) * 256.0f * sq_edge * 0.0625f); /* 552 */
    float sq_s2 = (-0.23799992f * sq_leak_3 - 0.76199913f) * sq_slew_3 * t538 * t552;            /* 553 */
    float sq_ph_4 = s1_wrap(sq_ph_3 + sq_inc);                                                   /* 554 */
    float t556 = sq_ph_4 + sq_thr;                                                               /* 556 */
    float t556_ge0 = K_GE0(t556);                                                                /* 556 */
    float t556_lt0 = K_LT0(t556);                                                                /* 558 */
    float t558 = SEL(t556_lt0, 0.0f, SEL(t556_ge0, 1.0f, sq_edge));                              /* 558 */
    float sq_leak_4 = 0.00055629015f * SEL(t556_lt0, 1.0f, SEL(t556_ge0, -1.0f, 1.0f)) + 0.99944305f * sq_leak_3; /* 561 */
    float sq_slew_4 = SEL(K_GT0(t558 - sq_slew_3), XFADE(sq_slew_3, t558, t476), t558);          /* 564 */
    float t567 = 1.0f - 3.0f * fmaxf(0.5f - 0.5f * sq_ph_4, sq_lim) + t480;                      /* 567 */
    float t581 = K_CLAMP(K_TRI(K_RECIP(sq_thr + SEL(t556_lt0, -1.0f, 1.0f)) * t556) * 256.0f * sq_edge * 0.0625f); /* 581 */
    float sq_s3 = (-0.23799992f * sq_leak_4 - 0.76199913f) * sq_slew_4 * t567 * t581;            /* 582 */
    float t584 = -(0.009056807f * sq_s0) - 0.009056807f * sq_s3_z3 - 0.013877809f * sq_s1 - 0.013877809f * sq_s2_z3; /* 584 */
    float t586 = t584 - 0.02744174f * sq_s2 - 0.02744174f * sq_s1_z3 - 0.04779297f * sq_s3 - 0.04779297f * sq_s0_z3; /* 586 */
    float t588 = t586 - 0.07156843f * sq_s0_z1 - 0.07156843f * sq_s3_z2 - 0.09462041f * sq_s1_z1; /* 588 */
    float t589 = t588 - 0.09462041f * sq_s2_z2 - 0.11281258f * sq_s2_z1 - 0.11281258f * sq_s1_z2; /* 589 */
    float square = t589 - 0.12282902f * sq_s3_z1 - 0.12282902f * sq_s0_z2;                       /* 590 */
    float square_lp = LERP(square_lp_z1, square, (0.9088975f * cutoff + 0.45f - 0.35889754f * t415) * 0.002814293f); /* 598 */
    float t602 = 3.3225f * t418 + (0.725f - 2.57f * resonance) - 1.425f * t422;                  /* 602 */
    float t605 = 0.038210943f * t425 + (0.34410548f * cutoff + 0.775f - 0.15731642f * t415);     /* 605 */
    float square_shaped = (t419 * square_lp + (square - square_lp_z1)) * t602 * t605;            /* 606 */

    /* ---- mixer: SAW / SQUARE levels (WAVEFORM) ---- */
    float saw_mix = saw_shaped * KP(P_SAW_LEVEL);                                                /* 673 */
    float square_mix = square_shaped * KP(P_SQUARE_LEVEL);                                       /* 673 */
    float osc_mix = saw_mix + square_mix;                                                        /* 678 */

    /* ---- filter: state-variable filter (svf_f, svf_damp from DSP A), one more pole, DC blocker ---- */
    float svf_f = s1_range(s1_import(4), -0.19455601f, 1.031209f);                               /* 694 */
    float svf_damp = s1_range(s1_import(5), -1.3512529f, 6.89599f);                              /* 695 */
    float svf_bp = svf_f * (osc_mix - svf_damp * svf_bp_z1 - svf_lp_z1) + svf_bp_z1;             /* 699 */
    float svf_lp = svf_f * svf_bp + svf_lp_z1;                                                   /* 700 */
    float filter_lp = LERP(filter_lp_z1, svf_lp, svf_f);                                         /* 702 */
    float filter_out = filter_lp - filter_dc_z1;                                                 /* 706 */
    float filter_dc = LERP(filter_dc_z1, filter_lp, 0.00030106306f);                             /* 707 */

    /* ---- output: fixed two-pole stage, accent and amp envelopes smoothed, VCA ---- */
    float amp_env = s1_range(s1_import(6), -0.22822341f, 1.137117f);                             /* 709 */
    float accent_env = s1_range(s1_import(7), -0.2506478f, 1.2492391f);                          /* 710 */
    float accent_level = s1_range(s1_import(8), -0.251f, 1.251f);                                /* 711 */
    float post_in = -post_s1_z1 - post_s2_z1 + (filter_out + resonance * 1.8097105f * filter_out); /* 717 */
    float post_s1 = 0.0046518445f * post_in + post_s1_z1;                                        /* 718 */
    float post_s2 = 0.0046518445f * post_s1 + post_s2_z1;                                        /* 719 */
    float post_out = 0.45151234f * post_s2 + (post_in * -0.8249998f - 0.11971277f * post_s1);    /* 720 */
    float accent_gain_tgt = 2.0416412f * fmaxf(-0.06372712f + accent_level * accent_env, 0.0f);  /* 721 */
    float t721_2 = accent_gain_tgt - accent_gain_z1;                                             /* 721 */
    float t721_3 = amp_env - amp_gain_z1;                                                        /* 721 */
    float t723 = SEL(K_GE0(t721_2), LERP(accent_gain_z1, accent_gain_tgt, 0.012500823f), t721_2); /* 723 */
    float accent_gain = SEL(K_LT0(t721_2), LERP(accent_gain_z1, accent_gain_tgt, 0.0025001764f), t723); /* 723 */
    float t724 = LERP(amp_gain_z1, amp_env, 0.39980412f);                                        /* 724 */
    float amp_gain = SEL(K_LT0(t721_3), t724, SEL(K_GE0(t721_3), t724, t721_3));                 /* 724 */
    float vca = accent_gain * post_out + amp_gain * post_out;                                    /* 725 */
    float voice_out = vca * 0.11948974f;                                                         /* 733 */
    KOUT(OUT_VOICE, voice_out);

    saw_lp_z1 = saw_lp;
    sq_phase_z1 = sq_ph_4;
    sq_s0_z3 = sq_s0_z2;
    sq_s0_z2 = sq_s0_z1;
    sq_s0_z1 = sq_s0;
    sq_s1_z3 = sq_s1_z2;
    sq_s1_z2 = sq_s1_z1;
    sq_s1_z1 = sq_s1;
    sq_s2_z3 = sq_s2_z2;
    sq_s2_z2 = sq_s2_z1;
    sq_s2_z1 = sq_s2;
    sq_s3_z3 = sq_s3_z2;
    sq_s3_z2 = sq_s3_z1;
    sq_s3_z1 = sq_s3;
    sq_leak_z1 = sq_leak_4;
    sq_slew_z1 = sq_slew_4;
    square_lp_z1 = square_lp;
    svf_bp_z1 = svf_bp;
    svf_lp_z1 = svf_lp;
    filter_lp_z1 = filter_lp;
    filter_dc_z1 = filter_dc;
    post_s1_z1 = post_s1;
    post_s2_z1 = post_s2;
    amp_gain_z1 = amp_gain;
    accent_gain_z1 = accent_gain;
}

