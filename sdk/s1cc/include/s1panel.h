/* s1panel.h: the SYSTEM-1 panel controls as parameter ids, for S1_PARAM(name, panel_id, count) (docs/vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md §3).
 * Included by s1dsp.h. Ids measured with PARAMLOG on OS 1.30, 2026-10-23
 * (examples/system_1_synth/README.md §8: the table, the CCs and how they were captured).
 *
 * Every S1_PARAM table is computed for knob = 0..255. Knobs and faders send 0..255; selectors send 0..5 (only
 * the first entries of their table are used).
 *
 * In MUTABLE-SYSTEM the host program (C-S1SY) also uses some of these controls: a voice that takes one of them adds
 * its own use to the host's (examples/mutable_system/README.md, panel table); OSC1 / OSC2 COLOR are the free
 * ones. In SYSTEM-S1CC (examples/system_s1cc) there is no host: all of them are the kernel's.
 * Not here: VOLUME (0x100) goes to the master (region 0), not to the voice; HPF cutoff (0x180) was not seen on 2026-10-23
 * (C-S1V4, light off); with its light on it sends 0x180 (2026-11-11).
 *
 * Which lights are on: the kernel's panel map, VOICE.panel beside its source (lines `LIGHT on|off`, LIGHT = these
 * macros without S1_, plus OSC1_CROSS_MOD, HPF_CUTOFF, CRUSHER, PORTAMENTO), copied by `s1cc build`; without one
 * SYSTEM-S1CC drafts it from what the kernel reads (examples/system_s1cc/panel_map.py, README.md §1.7). The OS numbers
 * the lights apart from these ids (ctl 0xC0.., panel order, docs Vol. IV ch. 6 §9.5). */
#ifndef S1PANEL_H_
#define S1PANEL_H_

/* LFO */
#define S1_LFO_SHAPE          0x160   /* selector 0..5: sine, triangle, saw, square, sample & glide, sample & hold */
#define S1_LFO_FADE           0x161   /* delay / fade time */
#define S1_LFO_RATE           0x162
#define S1_LFO_TO_PITCH       0x163
#define S1_LFO_TO_FILTER      0x164
#define S1_LFO_TO_AMP         0x165

/* oscillators */
#define S1_OSC1_WAVE          0x167   /* selector 0..5 */
#define S1_OSC1_COLOR         0x168
#define S1_OSC1_COLOR_MOD     0x169   /* selector 0..5: manual, LFO, pitch env, filter env, amp env, sub osc */
#define S1_OSC1_RANGE         0x16A   /* selector 0..5: 64' .. 2' */
#define S1_OSC1_CROSS_MOD     0x16B   /* CROSS MOD knob 0..255 (also CC 52); PARAMLOG 2026-11-11, JP-08-S1 */
#define S1_OSC2_WAVE          0x16C
#define S1_OSC2_COLOR         0x16D
#define S1_OSC2_COLOR_MOD     0x16E
#define S1_OSC2_RANGE         0x16F
#define S1_OSC2_FINE          0x170   /* 128 = centre */

/* mixer */
#define S1_OSC1_MIX           0x174
#define S1_OSC2_MIX           0x175
#define S1_SUB_MIX            0x176
#define S1_NOISE_MIX          0x177

/* pitch envelope */
#define S1_PITCH_ENV_AMOUNT   0x17A
#define S1_PITCH_ENV_ATTACK   0x17B
#define S1_PITCH_ENV_DECAY    0x17C

/* filter */
#define S1_FILTER_CUTOFF      0x17D
#define S1_HPF_CUTOFF         0x180   /* HPF knob 0..255 (also CC 79); PARAMLOG 2026-11-11, JP-08-S1A, light on */
#define S1_FILTER_RESONANCE   0x17F
#define S1_FILTER_ENV_AMOUNT  0x181   /* bipolar: 128 = 0 */
#define S1_FILTER_ATTACK      0x182
#define S1_FILTER_DECAY       0x183
#define S1_FILTER_SUSTAIN     0x184
#define S1_FILTER_RELEASE     0x185
#define S1_FILTER_KEY_TRACK   0x186

/* amplifier */
#define S1_AMP_ATTACK         0x187
#define S1_AMP_DECAY          0x188
#define S1_AMP_SUSTAIN        0x189
#define S1_AMP_RELEASE        0x18A

#define S1_AMP_TONE           0x18B   /* (examples/system_1_synth/README.md §8: CC 69) */

/* effects */
#define S1_REVERB_AMOUNT      0x18D
#define S1_DELAY_AMOUNT       0x18E
#define S1_DELAY_TIME         0x18F


/* buttons (PARAMLOG 2026-11-11, examples/system_s1cc/README.md §1.6): each press reaches the PLUG-OUT's model (region
 * 0, the assigner's cc()) as this MIDI CC with value 42; no parameter carries the button's state, the PLUG-OUT keeps
 * it (toggle per press). Their LEDs follow group-3 patch words (writing 1 lights the LED, 0 turns it off), and the OS
 * hands a written word to the engine like a knob. Found 2026-11-11 by writing candidate words one at a time
 * (examples/system_s1cc model.c LED_PROBE_TEST): 0x16B, 0x173, 0x180, 0x18C light nothing; TEMPO SYNC, MONO and
 * LEGATO have no word among the gaps of the map (global settings?). Not usable from a kernel (S1_PARAM): the model (examples/system_s1cc/model.c)
 * handles them and hands the state to the engine. */
#define S1_CC_OSC2_RING       111
#define S1_CC_OSC2_SYNC       112
#define S1_CC_LPF_TYPE        115
#define S1_CC_LEGATO          116
#define S1_CC_LFO_KEY_TRIG    117
#define S1_CC_TEMPO_SYNC      118
#define S1_CC_MONO            119
#define S1_LPF_TYPE_WORD      0x17E   /* group 3: lights LPF TYPE (2026-11-11) */
#define S1_LFO_KEY_TRIG_WORD  0x166   /* group 3: lights LFO KEY TRIG (2026-11-11, LED_PROBE_TEST) */
#define S1_OSC2_RING_WORD     0x171   /* group 3: lights OSC2 RING */
#define S1_OSC2_SYNC_WORD     0x172   /* group 3: lights OSC2 SYNC */
#define S1_SUB_OSC_TYPE_WORD  0x178   /* group 3: lights SUB OSC TYPE (its button: CC not captured yet) */
#define S1_NOISE_TYPE_WORD    0x179   /* group 3: lights NOISE TYPE (its button: CC not captured yet) */

#endif
