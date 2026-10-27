/* saw.c: a naive sawtooth (no band limiting), plain C. A state struct, a setter and a per-sample function, the usual
 * shape of an audio library. s1cc inlines these calls into the voice and keeps the struct's fields as the voice's
 * state (one ring word each). */
#include "saw.h"

void saw_init(struct saw *s) {
    s->phase = 0.0f;
    s->inc = 0.0f;
}

void saw_set_frequency(struct saw *s, float hz, float sample_rate) {
    s->inc = hz / sample_rate;
}

float saw_process(struct saw *s) {
    s->phase += s->inc;
    if (s->phase >= 1.0f)               /* the wrap: s1cc recognises it and makes it exact */
        s->phase -= 1.0f;
    return 2.0f * s->phase - 1.0f;
}
