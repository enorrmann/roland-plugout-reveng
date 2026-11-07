/* saw_core.c: a sawtooth in plain C, as a library sees it (docs/examples_s1cc_saw_README.md §2). A state struct, a
 * start function (one note) and a render function (one sample). It knows nothing of the DSP: the wrapper
 * (saw_wrapper.c) is what adapts it to a voice, and s1cc compiles both as one program.
 *
 * What s1cc handles without help is everything else here: a function called from the loop (inlined), the branches of
 * the polyBLEP (each run as a select, its value profiled only where its branch is taken), divisions, and the
 * struct's fields, which become the voice's state. The one thing a branch hides from the compiler is the range of the
 * frequency, so the library declares it with s1_range: one call, in saw_start. */
#include <math.h>
#include "s1dsp.h"              /* for s1_range: the only DSP-specific line of the library */
#include "saw_core.h"

float saw_note_hz = 440.0f;     /* the wrapper writes the note here (a library would own the keyboard) */

/* Band-limited step of the saw at the wrap (polyBLEP, as stmlib / Plaits write it). One `if` per corner: this is the
 * form s1cc profiles best (the two corners differ by tens of dB, so they are fitted apart). */
static inline float poly_blep(float t, float dt) {
    float a = 0.0f, b = 0.0f;
    if (t < dt) {                       /* the step at the wrap: 0 -> 1 */
        float u = t / dt;
        a = u + u - u * u - 1.0f;
    }
    if (t > 1.0f - dt) {                /* the step at the jump back to 0 */
        float u = (t - 1.0f) / dt;
        b = u * u + u + u + 1.0f;
    }
    return a + b;
}

void saw_start(struct saw *s) {
    s->hz = s1_range(saw_note_hz, 55.0f, 7040.0f);   /* the note stays there: A 440 +-7 octaves */
    s->phase = 0.0f;
    s->x1 = 0.0f;
}

float saw_tick(struct saw *s, float level) {
    float dt = s->hz / 96000.0f;
    float ph = s1_wrap(s->phase + dt);                 /* phase 0..1 (s1_wrap: one op on the DSP) */
    s->phase = ph;
    float x = (2.0f * ph - 1.0f - poly_blep(ph, dt)) * level;
    float y = x - s->x1;                               /* a differencer: no DC offset after a note start */
    s->x1 = x;
    return 0.5f * y;
}
