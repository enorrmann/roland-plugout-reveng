/* saw_core.h: the interface of the library (saw_core.c). Plain C: no s1dsp.h, nothing of the DSP. */
#ifndef SAW_CORE_H_
#define SAW_CORE_H_

struct saw {
    float hz;       /* frequency of the note, Hz (set by saw_start) */
    float phase;    /* 0 .. 1 */
    float x1;       /* last input (the differencer) */
};

extern float saw_note_hz;               /* the note the wrapper asked for, Hz (range declared in saw_start) */

void saw_start(struct saw *s);                /* call it when a note starts */
float saw_tick(struct saw *s, float level);   /* one sample; level 0..1 */

#endif
