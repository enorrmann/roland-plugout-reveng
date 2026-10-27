/* simple_saw_export.c: s1_export / s1_import, and nothing else new (docs/examples_simple_saw_wrapped_README.md §8).
 * Two saws on DSP A; on DSP B a low-pass whose cutoff follows the note (key tracking). DSP B needs the note's
 * frequency, and DSP A has already computed it: DSP A exports it, DSP B imports it.
 *
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_export.c --regs 0,1,2,3,4,5,6,7 --check
 *         (saw.c and filter.c by themselves)
 * Synth:  KERNEL=sdk/s1cc/build/simple_saw_export NAME=S1CC-EXPORT sh examples/system_s1cc/build.sh
 *         (no envelope, to keep to the point: the voice is the gate; an envelope: simple_saw_env.c / _hostenv.c)
 *
 * WHAT CROSSES FROM DSP A TO DSP B. Each voice runs through the slave's two DSPs in series, A then B, and they share
 * nothing but bank-5 words (a variable of the C belongs to the DSP whose function uses it: E302 if both do). Three
 * kinds of value reach S1_DSP_B:
 *   x                 what S1_DSP_A returned: the signal (bank 5 A151+v)
 *   s1_pitch(), s1_gate(), s1_trig()   the voice's inputs: s1cc has DSP A copy them for B (bank 5 A1A0 + 8 j + v)
 *   s1_import(i)      what S1_DSP_A gave to s1_export(i, v) in the same sample: any other value (bank 5 A188 + 8 i + v)
 *
 * WHY AN EXPORT HERE. The cutoff is "ratio x the note's frequency". DSP B could compute the frequency itself from
 * s1_pitch(): 440 x s1_exp2(pitch - 4.75) / 96000 (s1_exp2: the DSP's exp2, a polynomial and squarings). DSP A
 * computes exactly that value already for the saws. Exporting it costs one store on A and one load on B per voice.
 * Measured [sim, 2026-10-27], per voice: DSP B 15 instructions with the export, 25 computing it again; DSP A 29 in
 * both (the store packs into an instruction it already has). 8 voices: DSP B 110 against 195.
 * Use an export when DSP B needs a value that DSP A has (a frequency, a level, a modulation) and computing it again
 * would cost more than a store and a load; or when B must treat apart what A mixes into x. Not needed when B only
 * works on the whole voice (an envelope, a filter with a fixed cutoff): x brings it.
 *
 * THE SLOT NUMBER. The 0 of s1_export(0, v) / s1_import(0) is the slot: the channel the value goes through. The
 * export and the import with the same number are the same word; two values take two slots (0 and 1). It must be a
 * constant (E110); SYSTEM-S1CC has slots 0..11 (more stops the build).
 *
 * THE MECHANISM, step by step:
 *   1. s1cc (graph.cpp): s1_export(i, v) in S1_DSP_A records "v leaves through slot i" (E114 in S1_DSP_B);
 *      s1_import(i) in S1_DSP_B is an input node "slot i" (E115 in S1_DSP_A; E116 if no export i exists).
 *   2. s1cc (lower.cpp): on DSP A, as soon as v is computed, `st r, [{X:i}]`, with v in the representation it has
 *      there (V = (r - o) s); the kernel file records it (".export i s o" in voice.s1k: here 0.1467 0). On DSP B,
 *      `ld r, [{X:i}]` read with that same s and o: no conversion instruction on either side.
 *   3. The PLUG-OUT generator (examples/system_s1cc make_s1cc.py export_addr) puts {X:i} of voice v on a bank-5 word
 *      a DSP may store: A188 + 8 i + v for slots 0..2, A1B8 + 8 (i - 3) + v for 3..11 (bank 5 is the only memory the
 *      chip's two DSPs share; each DSP has its own banks 0 and 6).
 *   4. On the device: per voice, one store in DSP A's program and one load in DSP B's.
 *
 * WHEN THE VALUE ARRIVES. In the native reference (s1rt.c: the slot is an array) and in the emulator (it runs DSP A,
 * then DSP B), DSP B reads the value of the same sample. On the device the two DSPs of the chip run at the same time
 * (docs/examples_system_1_jx_README.md §13): B gets this sample's value only if A's store comes before B's load in
 * cycles, otherwise the value of the sample before. make_s1cc.py prints it for every crossing ("A -> B export 0: ..."):
 * here voice 0 a race (store and load within 8 cycles: one or the other, always the same), voices 1..7 one sample
 * late. For a frequency that is 10 us late: inaudible. x itself crosses the same way (here one sample late, a fixed
 * latency). A value that must not lag (a sample-exact pairing of two signals) has to be computed on the DSP that
 * uses it. */
#include <math.h>
#include "s1dsp.h"
#include "saw.h"
#include "filter.h"

S1_PARAM(mix1, S1_OSC1_MIX, 1) { out[0] = knob / 255.0f; }
S1_PARAM(mix2, S1_OSC2_MIX, 1) { out[0] = knob / 255.0f; }
S1_PARAM(fine2, S1_OSC2_FINE, 1) { out[0] = s1_exp2((knob - 128) / 128.0f / 12.0f); }
/* FILTER CUTOFF -> cutoff / note frequency: 1 .. 32 (exponential), times 2 pi: g ~ 2 pi fc / fs for a low cutoff */
S1_PARAM(track, S1_FILTER_CUTOFF, 1) { out[0] = 6.2831853f * powf(32.0f, knob / 255.0f); }

static struct saw osc1, osc2;            /* DSP A's state */
static struct lowpass lp;                /* DSP B's state */

S1_INIT {
    saw_init(&osc1);
    saw_init(&osc2);
    lowpass_init(&lp);
}

S1_DSP_A {
    float hz = 440.0f * s1_exp2(s1_pitch() - 4.75f);
    float inc = hz / 96000.0f;           /* the note's frequency in cycles per sample: computed once, here */
    s1_export(0, inc);                   /* -> slot 0: DSP B's filter follows the note with it */
    saw_set_frequency(&osc1, hz, 96000.0f);
    saw_set_frequency(&osc2, hz * s1_param(fine2, 0), 96000.0f);
    return 0.5f * (saw_process(&osc1) * s1_param(mix1, 0) + saw_process(&osc2) * s1_param(mix2, 0));
}

S1_DSP_B {                               /* x = the saws; the note's frequency from slot 0 */
    float inc = s1_import(0);
    float g = fminf(inc * s1_param(track, 0), 0.9f);     /* cutoff = track / 2 pi x the note (g <= 0.9: stable) */
    return lowpass_process(&lp, x, g) * s1_gate();
}
