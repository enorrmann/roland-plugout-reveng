/* bq.c: the Roland Boutique JP-08 / JU-06 / JX-03 voice (firmware BQ_UPD.BIN 1.21, one firmware for the three) for
 * SYSTEM-S1CC, ported from the firmware. README.md.
 *
 *   bq_MODEL_voice.h   the model's voice DSP program (LFO, glide, two ADSRs, oscillators, mixer, filter, VCA) as C,
 *                      lifted by dsplift.py --bq2 from Roland's microcode with the model's own patches applied and
 *                      its per-sample constants converted from 44.1 to 96 kHz (tools/make_voice.py, README.md §2)
 *   bq_MODEL_tables.h  the model's ARM side as data: the parameter words after its set-up and what each panel
 *                      control writes for every knob value, recorded by running the firmware's parameter handlers in
 *                      an emulator (tools/bq_arm.py, README.md §3)
 *   this file          the per-voice inputs from the SYSTEM-1 (pitch, gate) and the build
 *
 * One model per build: BQ_MODEL = jp08 (default), ju06, jx03 (the voice programs differ per model: the oscillator
 * programs and the immediates the ARM patches; README.md §1). The voice is cut in two (tools/make_voice.py
 * VOICE_SPLIT): DSP A = LFO, glide, envelopes, modulation, OSC2; DSP B = OSC1, mixer, filter, VCA (README.md §5). */
#include <math.h>
#include "s1dsp.h"

#define BQ_PANEL
#include "bq_jp08_tables.h"

static float out_voice;

/* LFO -> AMP (C06B) fixed at 0 in this build: with the knob low its values span 1e-25 .. 1e-8 and s1cc scales them
   wrongly (README.md §7, open) */
#define KP(a) (((a) & 0xff) == 0x6b ? 0.0f : jp08_kp((a) & 0xff))
/* PWM SOURCE (the JP-08's switch) from OSC2 COLOR MOD: the pulse width is C054 x (A090 + (A091 + A093) LFO - A092
   ENV1); MANUAL -> A090 = 1, LFO -> A091 = 1, PITCH / FILTER / AMP ENV -> A092 = 1 (ENV1), SUB -> MANUAL */
S1_PARAM(pwmsrc, S1_OSC2_COLOR_MOD, 3)
{
    int src = knob > 5 ? 0 : knob;
    out[0] = src == 0 || src == 5;
    out[1] = src == 1;
    out[2] = src >= 2 && src <= 4;
}

/* the per-voice inputs: pitch, gate, the PWM source; every other A0xx input is a constant of the lift (bq_names.txt) */
static inline float bq_in(int a)
{
    switch (a) {
    case 0xa080: return s1_pitch();                       /* pitch (bq_names.txt: in A080) */
    case 0xa081: return s1_range(s1_gate(), 0.0f, 1.0f);  /* gate (in A081) */
    case 0xa090: return s1_param(pwmsrc, 0);
    case 0xa091: return s1_param(pwmsrc, 1);
    case 0xa092: return s1_param(pwmsrc, 2);
    default: return 0.0f;
    }
}
#define KIN(a) bq_in(a)
#define KOUT(a, v) do { out_voice = (v); } while (0)
#define KE(a) 0.0f
#pragma float_control(precise, off, push)
#include "bq_jp08_voice.h"
#pragma float_control(pop)

#ifndef BQ_GAIN
#define BQ_GAIN 30.0f
#endif

/* DSP A: the first part of the voice; it hands JP08_VOICE_SLOTS values to DSP B (s1_export) */
S1_DSP_A {
    jp08_voice_a();
    return 0.0f;
}

/* DSP B: the rest of the voice. The JP-08's output stage (post program: the four voices to stereo, level, soft
   clip) is not ported: a fixed gain instead (README.md §6) */
S1_DSP_B {
    (void)x;
    jp08_voice_b();
    return out_voice * BQ_GAIN;
}

S1_BUILD("NAME=JP-08-S1 MIX=0");
S1_COMPILE("--no-fused --regs 0,1,2,3,4,5,6,7");
