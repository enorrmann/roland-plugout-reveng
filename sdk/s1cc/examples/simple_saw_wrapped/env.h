/* env.h: an ADSR envelope with one-pole segments, plain C (docs/examples_simple_saw_wrapped_README.md). Nothing of the
 * DSP: the wrapper (simple_saw_env.c) gives it the gate, the note starts and the coefficients. */
#ifndef ENV_H_
#define ENV_H_

struct env {
    float level;    /* 0 .. 1 */
    float attacking; /* 1 from a note start until the level reaches 1, else 0 (decay / sustain, release) */
    float pending;   /* 1 from a note start until the gate is up: the two may arrive apart (2026-10-27) */
};

struct env_params {
    float k_attack, k_decay, k_release;   /* one-pole coefficients per sample (env_coefficient) */
    float sustain;                        /* 0 .. 1 */
};

void env_init(struct env *e);
float env_coefficient(float seconds, float sample_rate);   /* k of a time constant: 1 - e^(-1 / (t fs)) */
/* one sample. gate: 1 while the key is held, else 0; start: 1 when a note starts this sample (attack from the current
   level, no click), else 0. Returns the level. */
float env_process(struct env *e, float gate, float start, const struct env_params *p);

#endif
