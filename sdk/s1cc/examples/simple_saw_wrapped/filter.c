/* filter.c: a low-pass of two one-pole stages in cascade (12 dB / octave), plain C. s1cc inlines lowpass_process into
 * the voice and keeps the struct's fields as the voice's state; lowpass_coefficient runs where the wrapper calls it
 * (in S1_PARAM: on the PC). */
#include <math.h>
#include "filter.h"

void lowpass_init(struct lowpass *f) {
    f->y1 = 0.0f;
    f->y2 = 0.0f;
}

float lowpass_coefficient(float hz, float sample_rate) {
    return 1.0f - expf(-6.2831853f * hz / sample_rate);
}

float lowpass_process(struct lowpass *f, float x, float g) {
    f->y1 += (x - f->y1) * g;
    f->y2 += (f->y1 - f->y2) * g;
    return f->y2;
}
