@ JX-3P engine functions (region 1, slot-1 addresses), as Thumb symbols so
@ that calls to them are emitted as PC-relative BL (slot independent).
    .syntax unified
    .thumb
    .global prm_curve, dsp_send
    .thumb_set prm_curve, 0x60a12688   @ float prm_curve(void *, int curve, int value)
    .thumb_set dsp_send,  0x60a0b41e   @ void dsp_send(void *, int, int id, int, float)
