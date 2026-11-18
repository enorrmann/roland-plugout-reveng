/* jx3p.c: the JX-3P voice for SYSTEM-S1CC-BOUTIQUE (README.md): Roland's filter, VCA and output stage (the S8 JX-3P
 * PLUG-OUT's MAIN4, lifted: jx3p_core_voice.h) with our own oscillators (jx3p_osc.h) and the per-voice values on the
 * ARM (jx3p_arm.h, ARM_CUSTOM, from tools/make_arm.py):
 *   s1_dt() / s1_rdt()   DCO1's step and its inverse (note, RANGE, FREQ MOD: LFO and ENV1, bend)
 *   s1_pitch()           DCO2's step (RANGE, TUNE, FINE TUNE, its FREQ MOD): the ARM writes a step, not octaves
 *   s1_arm(0)            dt1 / dt2 / 256: DCO2's 1 / dt from DCO1's (the width of its BLEP)
 * s1_ref_pitch / s1_ref_arm (S1_REF_INPUTS) give s1cc's native reference the same meanings, so the profile, and the
 * scaling s1cc derives from it, see the values the device has (README.md §3.1)
 *   s1_env(1)            the filter's g (cutoff, resonance term, brilliance, LFO, ENV1, key follow; tan(w) / (1 + tan w))
 *   s1_env(0)            the VCA's input term: ENV2 (the AMP envelope) x 0.013004484
 * The ADSRs and the LFO are the host's (ARM_ENV / ARM_LFO): AMP A D S R = ENV2, FILTER A D S R = ENV1.
 * The whole voice is one function (s1cc --fused): SYSTEM-S1CC places whole voices on DSP A and DSP B.
 * Build: examples/system_s1cc_boutique/build.sh (README.md §1). */
#include <math.h>
#include "s1dsp.h"
#include "s1panel.h"
#include "dsplift_k.h"

/* the output gains of Roland's DCO programs (tools/osc_check.c: the FIR of the lifts inverts, so these signs) */
#define JX_SAW_GAIN 0.5f
#define JX_PULSE_GAIN (-0.9f)
#define JX_SQ_GAIN 0.33f
#define JX_SQ_SPIKE 0.5f
#include "jx3p_osc.h"
#include "jx3p_tables.h"

/* the DCO waveforms of this program: JX_DCO1 0 SAW, 1 SQUARE, 2 PULSE; JX_DCO2 the same, 3 NOISE (README.md §4) */
#ifndef JX_DCO1
#define JX_DCO1 0
#endif
#ifndef JX_DCO2
#define JX_DCO2 0
#endif

static float sm_g, out_voice, dco1_out, dco2_out, mark, hpf_lp;
static jx_osc_t dco1 __attribute__((used)), dco2 __attribute__((used));
S1_FIXED_STATES;

/* the output: C14B (VCA LEVEL of the default patch, the pair's output on the S8) x the inverse of SYSTEM-S1CC's
   mix of the voices (2 / (1.5 NV), make_s1cc.py GAIN; NV = 8) x the level measured against the SYSTEM-1's own synth
   (README.md §5.1, 2026-11-17: saw, one OSC MIX at the middle, VOLUME at the middle, filter open, note 60: SYSTEM-1
   rms 0.0516, JX-3P-S1 with x 6 only 0.00199: x 25.89) */
#define JX_MIX_GAIN (6.0f * 25.89f)

/* MAIN4 instruction 922: Roland smooths the cutoff's modulation sum with this one-pole (per sample); here it smooths
   g, which the ARM changes every 1 ms */
#define JX_G_SMOOTH 0.0065235896f

static inline __attribute__((always_inline)) float jx_in(int a)
{
    switch (a) {
    case 0xa0c0: return dco1_out;                   /* DCO1 (this sample's) */
    case 0xa0c2: return dco2_out;                   /* DCO2 */
    case 0xa0c4: return sm_g;                       /* the filter's g (ARM) */
    case 0xa0c5: return s1_env(0);                  /* the VCA's input term (ARM): ENV2 x 0.013004484 */
    case 0xa0c6: return jx_kp(0xc130);              /* the VCA level slew's target C130 = 0.93 (README.md §2.2) */
    case 0xa0c7: return 1.0f;                       /* the gate slew's target: A02A = 1 from the first note on (the
                                                       S8 engine never clears it: s1jx.note_writes) */
    case 0xa06a: return 1.0f;
    default: return 0.0f;                           /* A029 velocity curve: unused (C12F = 0) */
    }
}

/* DCO2 CROSS MOD (README.md §4.3): Roland's patch word 0x172 (0 off, 1 SYNC, 2 METAL; Roland's rows 0x6df: A028 = SYNC
   or METAL, A029 = METAL; 3 writes nothing there: here as off), set by the OSC2 SYNC / RING buttons (model.c
   ARM_CUSTOM_XMOD_WORD) */
S1_PARAM(xsync, S1_OSC2_SYNC_WORD, 1) { out[0] = (knob == 1 || knob == 2) ? 1.0f : 0.0f; }
S1_PARAM(xmetal, S1_OSC2_SYNC_WORD, 1) { out[0] = knob == 2 ? 1.0f : 0.0f; }
/* HPF CUTOFF (C148, MAIN4 instructions 1308..1311, on the pair's sum: here per voice, the same, it is linear):
   lp += C148 (x - lp), out = x - lp (Roland: x 11, in the level calibration, README.md §5.1) */
S1_PARAM(hpf, S1_HPF_CUTOFF, 1) { out[0] = jx_hpf[knob]; }

#define KP(a) jx_kp(a)
#define KIN(a) jx_in(a)
#define KOUT(a, v) do { if ((a) == 0x231e) out_voice = (v); } while (0)
#define KE(a) 0.0f
#pragma float_control(precise, off, push)
#include "jx3p_core_voice.h"
#pragma float_control(pop)

S1_DSP_A {
    sm_g += JX_G_SMOOTH * (s1_env(1) - sm_g);
    float dt1 = s1_dt(), rd1 = s1_rdt();
    /* the ARM: dt2 <= 0.18 (pitch <= 10 octaves), s1_arm(0) = 2^-16 .. 1 (DCO2 within 8 octaves of DCO1) */
    float dt2 = s1_range(s1_pitch(), 0.0f, 0.25f), rd2 = rd1 * 256.0f * s1_range(s1_arm(0), 0.0f, 1.0f);
    float xm = fmaxf(s1_param(xsync, 0), s1_param(xmetal, 0)), mt = s1_param(xmetal, 0);
    float k = mark * xm;                               /* DCO1's mark of the sample before (MAIN2: A022 x) */
#if JX_DCO1 == 0
    dco1_out = jx_saw(&dco1, dt1, rd1);
#elif JX_DCO1 == 1
    dco1_out = jx_square(&dco1, dt1, rd1);
#elif JX_DCO1 == 2
    dco1_out = jx_pulse(&dco1, dt1, rd1);
#else
#error "JX_DCO1: 0..2"
#endif
    mark = jx_mark(dco1.p, dt1, rd1);
#if JX_DCO2 == 0 && defined(JX_NO_XMOD_TEST)
    (void)k; (void)xm; (void)mt;
    dco2_out = jx_saw(&dco2, dt2, rd2);
#elif JX_DCO2 == 0
    dco2_out = jx_saw_xl(&dco2, dt2, rd2, k, xm, mt);
#elif JX_DCO2 == 1
    dco2_out = jx_square_xl(&dco2, dt2, rd2, k, xm, mt);
#elif JX_DCO2 == 2
    dco2_out = jx_pulse_xl(&dco2, dt2, rd2, k, xm, mt);
#elif JX_DCO2 == 3
    dco2_out = jx_noise(&dco2);
#else
#error "JX_DCO2: 0..3"
#endif
    jx3p_core();
#ifdef JX_NO_HPF_TEST
    float hp = out_voice;
#else
    float hp = out_voice - hpf_lp;
    hpf_lp += s1_param(hpf, 0) * hp;
#endif
    return hp * (jx_kp(0xc14b) * JX_MIX_GAIN);
}

/* S1_REF_INPUTS (s1dsp.h): the native reference's s1_pitch / s1_arm(0) as the ARM writes them for the default patch
   (DCO2 = DCO1 + A025 = 8.3e-5 octave) */
float s1_ref_pitch(float pitch) { return 440.0f * exp2f(pitch + 8.3e-5f - 4.75f) / 96000.0f; }
float s1_ref_arm(int i, float pitch) { (void)pitch; return i == 0 ? exp2f(-8.3e-5f) / 256.0f : 0.0f; }

S1_DSP_B { return x; }

S1_BUILD("NAME=JX-3P-S1 MIX=0 LFO_PITCH=0 ARM_CUSTOM=sdk/s1cc/examples/jx3p_ported/jx3p_arm.h PANEL=sdk/s1cc/examples/jx3p_ported/jx3p.panel");
/* --no-fold-branches: s1cc folds a compare that gave one answer in every profiling scenario (W909); its profiling
   runs with CROSS MOD off, where the wraps of DCO2's cross mod phase never happen (jx3p_osc.h jx_phase_xl): folded,
   SYNC / METAL ran wrong on the DSP (2026-11-17: METAL on the device; s1cc check xsync=2: SNR -10 dB against the C,
   bands +70 dB; kept: the bands within 1 dB) */
S1_COMPILE("--regs 0,1,2,3,4,5,6,7 --no-fold-branches");
