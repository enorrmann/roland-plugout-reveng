/*
 * Code-cave test: a C replacement for the EFFECT TONE method shared by the
 * JX-3P OVERDRIVE / DISTORTION / FUZZ classes (0x60a0ac54), with the tone
 * inverted. The original method entry is patched with a B.W to tone_hook.
 */

struct efx {
    void *vtable;
    int handle;         /* +0x04, passed to the sender */
    int id[5];          /* +0x08.. DSP parameter ids */
};

float prm_curve(void *unused, int curve, int value);
void dsp_send(void *unused, int handle, int id, int arg, float value);

#define CURVE_TONE 0x10     /* -1.0 .. +1.0 */

__attribute__((section(".text.hook")))
void tone_hook(struct efx *self, int arg, int value)
{
    float tone = prm_curve(0, CURVE_TONE, value);
    dsp_send(0, self->handle, self->id[3] /* +0x14 */, arg, -tone);
}
