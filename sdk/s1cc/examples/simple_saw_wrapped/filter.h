/* filter.h: a low-pass of two one-pole stages in cascade (12 dB / octave), plain C
 * (docs/examples_simple_saw_wrapped_README.md §7). Nothing of the DSP: the wrapper gives it the coefficient. */
#ifndef FILTER_H_
#define FILTER_H_

struct lowpass {
    float y1, y2;   /* the two stages' outputs */
};

void lowpass_init(struct lowpass *f);
float lowpass_coefficient(float hz, float sample_rate);   /* g of a cutoff: 1 - e^(-2 pi fc / fs) */
float lowpass_process(struct lowpass *f, float x, float g);   /* one sample; g 0 .. 1 */

#endif
