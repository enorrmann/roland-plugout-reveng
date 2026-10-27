/* s1panel.h: the SYSTEM-1 panel controls as parameter ids, for S1_PARAM(name, panel_id, count) (docs/S1CC.md §3).
 * Included by s1dsp.h. Ids measured with PARAMLOG on OS 1.30, 2026-10-23
 * (docs/examples_system_1_synth_README.md §8: the table, the CCs and how they were captured).
 *
 * Every S1_PARAM table is computed for knob = 0..255. Knobs and faders send 0..255; selectors send 0..5 (only
 * the first entries of their table are used).
 *
 * The host program (C-S1SY) also uses some of these controls: a voice that takes one of them adds its own use to
 * the host's (docs/examples_mutable_system_README.md, panel table). OSC1 / OSC2 COLOR are the free ones.
 * Not here: VOLUME (0x100) goes to the master (region 0), not to the voice; HPF cutoff sends nothing. */
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

/* effects */
#define S1_REVERB_AMOUNT      0x18D
#define S1_DELAY_AMOUNT       0x18E
#define S1_DELAY_TIME         0x18F

#endif
