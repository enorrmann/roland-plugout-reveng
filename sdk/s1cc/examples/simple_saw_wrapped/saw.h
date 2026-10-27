/* saw.h: a naive sawtooth oscillator, plain C (docs/examples_simple_saw_wrapped_README.md). Nothing of the DSP: it
 * could be any C library. The wrapper (simple_saw_wrapped.c) is the only file that knows about s1cc. */
#ifndef SAW_H_
#define SAW_H_

struct saw {
    float phase;    /* 0 .. 1 */
    float inc;      /* phase step per sample: frequency / sample rate */
};

void saw_init(struct saw *s);
void saw_set_frequency(struct saw *s, float hz, float sample_rate);
float saw_process(struct saw *s);     /* one sample, -1 .. 1 */
float square_process(struct saw *s);     /* one sample, -1 .. 1 */

#endif
