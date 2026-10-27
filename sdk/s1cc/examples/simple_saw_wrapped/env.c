/* env.c: ADSR envelope with one-pole segments. */
#include <math.h>
#include "env.h"

#define ENV_ATTACK_TARGET 1.2f

void env_init(struct env *e) {
    e->level = 0.0f;
    e->attacking = 0.0f;
    e->pending = 0.0f;
}

float env_coefficient(float seconds, float sample_rate) {
    if (seconds <= 0.0f || sample_rate <= 0.0f) {
        return 1.0f;
    }
    return 1.0f - expf(-1.0f / (seconds * sample_rate));
}

float env_process(struct env *e, float gate, float start, const struct env_params *p) {
    /* a note start waits for its gate: on the device the trigger and the gate are separate writes (engine.c) and can
       reach the DSP samples apart; a start seen without the gate was lost and the attack never began (2026-10-27) */
    float pending = fmaxf(e->pending, start);
    float go = pending * gate;
    e->pending = pending * (1.0f - gate);                       /* kept until the gate is up */
    float attacking = gate * fmaxf(e->attacking, go);
    float target = attacking > 0.5f ? ENV_ATTACK_TARGET : gate * p->sustain;
    float k = attacking > 0.5f ? p->k_attack : gate > 0.5f ? p->k_decay : p->k_release;

    float level = e->level + (target - e->level) * k;

    if (attacking > 0.5f && level >= 1.0f) {
        level = 1.0f;
        attacking = 0.0f;
    }

    e->level = level;
    e->attacking = attacking;

    return level;
}