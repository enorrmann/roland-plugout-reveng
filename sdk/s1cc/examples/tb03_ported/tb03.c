/* tb03.c: the Roland TB-03 (Boutique, BQ2_UPD.BIN 1.07) voice for SYSTEM-S1CC, ported from the firmware.
 *
 *   tb03_voice.h   the TB-03's voice DSP program (oscillators, filter, envelopes, accent, slide, VCA) translated to C
 *                  by dsplift.py --bq2 --kernel: every operation of Roland's microcode that reaches the output, in
 *                  order, as named expressions (tb03_names.txt; README.md §2)
 *   tb03_tables.h  the TB-03's ARM side as data: the parameter values after its init and the knob tables, recorded
 *                  by running its knob handlers in an ARM emulator (README.md §3)
 *   this file      the inputs the TB-03's ARM and second DSP give the voice, from the SYSTEM-1: parameters (knobs),
 *                  pitch, gate, accent, slide (README.md §4)
 *
 * Panel (TB-03 -> SYSTEM-1):  CUTOFF -> FILTER CUTOFF, RESONANCE -> FILTER RESONANCE, ENV MOD -> FILTER ENV AMOUNT
 * (0..255, not bipolar), DECAY -> FILTER KEY TRACK (the knob, 2026-11-11), ACCENT -> OSC1 COLOR (with its COLOR MOD), TUNE -> OSC2 FINE (128 = in tune, +-1
 * octave as on the TB-03), WAVEFORM -> OSC1 WAVE (0 saw, any other position square). The accent of a note: OSC2
 * COLOR with its COLOR MOD (0 none .. 1 full, taken when the note starts; the SYSTEM-1 keyboard has no velocity, so the TB-03's
 * velocity >= 100 rule cannot be used: README.md §4); slide: a key played while another is down (legato), as on the
 * TB-03 (MONO_LEGATO in S1_BUILD). */
#include <math.h>
#include "s1dsp.h"
#include "tb03_tables.h"

S1_PARAM(cutoff, S1_FILTER_CUTOFF, 1) { out[0] = tb03_cutoff[knob]; }
S1_PARAM(resonance, S1_FILTER_RESONANCE, 1) { out[0] = tb03_resonance[knob]; }
S1_PARAM(envmod, S1_FILTER_ENV_AMOUNT, 1) { out[0] = tb03_envmod[knob]; }
S1_PARAM(decay, S1_FILTER_KEY_TRACK, 1) { out[0] = tb03_decay[knob >> 1]; }
S1_PARAM(tune, S1_OSC2_FINE, 1) { out[0] = tb03_tune[knob]; }
/* ACCENT (C01C) is s1_color(0), OSC1 COLOR with its COLOR MOD: the TB-03's table is knob / 255 (tb03_accent, within
   3e-8), so the color is the value. The accent of the notes is s1_color(1) (S1_DSP_A). README.md §4 */
static float accstr;
/* WAVEFORM: the TB-03 writes C029 = 1, C02A = 0 (saw) or C029 = 0, C02A = 1 (square); row 128 (before the panel
   arrives) = saw */
S1_PARAM(wave, S1_OSC1_WAVE, 2) { int sq = knob >= 1 && knob <= 5; out[0] = !sq; out[1] = sq; }

/* the voice DSP's parameter words: knobs from the tables above, the others the TB-03 init's values (constants) */
static inline float tb03_param(int a)
{
    switch (a & 0x3F) {
    case 0x19: return s1_param(cutoff, 0);
    case 0x1a: return s1_param(resonance, 0);
    case 0x1b: return s1_param(envmod, 0);
    case 0x0d: return s1_param(decay, 0);
    case 0x1c: return accstr;                       /* ACCENT: OSC1 COLOR (s1_color(0)), read in part A only */
    case 0x10: return s1_param(tune, 0);
    case 0x29: return s1_param(wave, 0);
    case 0x2a: return s1_param(wave, 1);
    default: return tb03_init[a & 0x3F];
    }
}

/* the voice's bank-5 inputs: A003 = 1 (the TB-03 init), A004 pitch in octaves ((note - 12) / 12: the SYSTEM-1's
   s1_pitch, 4.75 = A 440), A005 gate, A006 accent, A007 slide; A104..A109 (from the second DSP) do not change the
   output with the TB-03's parameters (README.md §4). Gate, accent and slide are arguments of the voice's part A
   (KARGS_A): values of this sample, not state */
static inline float tb03_in(int a, float gate, float accent, float slide)
{
    switch (a) {
    case 0xa003: return 1.0f;
    case 0xa004: return s1_pitch();
    case 0xa005: return gate;
    case 0xa006: return accent;
    case 0xa007: return slide;
    default: return 0.0f;
    }
}
static float out_a100;

#define KARGS_A float in_gate, float in_accent, float in_slide
#define KP(a) tb03_param(a)
#define KIN(a) tb03_in(a, in_gate, in_accent, in_slide)
#define KOUT(a, v) do { if ((a) == 0xa100) out_a100 = (v); } while (0)
#define KE(a) 0.0f
/* the parameters no knob moves are constants in tb03_voice.h already (make_voice.sh --consts: the parts the TB-03
   switches off are not in it); fast floating point only for the voice (README.md §6) */
#pragma float_control(precise, off, push)
#include "tb03_voice.h"
#pragma float_control(pop)

/* the TB-03's second DSP (overdrive, delay, output stage) is not ported: a fixed gain instead (README.md §5) */
#ifndef TB03_GAIN
#define TB03_GAIN 12.0f
#endif

static float last_gate, last_pitch, slide, accent;

/* DSP A: the inputs, then the first part of the voice (pitch, glide, envelopes, accent, filter coefficients and the
   two oscillators); it hands TB03_VOICE_SLOTS values to DSP B (s1_export slots 0..TB03_VOICE_SLOTS-1) */
S1_DSP_A {
    float on = s1_range(s1_gate(), 0.0f, 1.0f), p = s1_pitch();
    /* TB-03 ARM (0x6001e376): slide = the note started while the gate was already on (legato), until the gate
       closes. Accent: OSC2 COLOR, latched when a new gate starts (README.md §4) */
    /* no compares (a compare the profiling never flips would be folded): 0 / 1 values from s1_sign, min, max */
    float changed = s1_abs(s1_sign(p - last_pitch));         /* 1: a new note's pitch */
    float legato = changed * on * last_gate;                  /* a new note while the gate stayed on */
    float rise = on * (1.0f - last_gate);                     /* a new gate */
    slide = on * fmaxf(slide, legato) * (1.0f - rise);
    accent = accent + rise * (s1_color(1) - accent);       /* OSC2 COLOR x its COLOR MOD, latched at the gate */
    accstr = s1_color(0);                                     /* ACCENT: OSC1 COLOR x its COLOR MOD */
    last_gate = on;
    last_pitch = p;
    tb03_voice_a(on, accent, slide);
    return 0.0f;
}

/* DSP B: the rest of the voice (oscillator mix, filter, VCA); x (DSP A's return) is not used */
S1_DSP_B {
    (void)x;
    tb03_voice_b();
    return out_a100 * TB03_GAIN;
}

S1_BUILD("NAME=TB-03-S1 MONO_LEGATO=1 MIX=0");
/* the voice in two parts (DSP A, DSP B: tb03_voice.h --split), all eight registers (no host in SYSTEM-S1CC) */
S1_COMPILE("--no-fused --regs 0,1,2,3,4,5,6,7");
