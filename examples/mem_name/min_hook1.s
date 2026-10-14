@ Minimal region 1 ("engine") for a dump-only PLUG-OUT: no DSP code at all.
@ The init function fills a descriptor with a stub returning 0 and returns
@ descriptor + 4, like the stock engine init does (struct at r0 - 4).
@ Position independent.
    .syntax unified
    .thumb
    .text
    .global init
    .thumb_func
init:
    adr     r1, stub
    orr     r1, r1, #1
    adr     r0, desc
    str     r1, [r0, #8]
    str     r1, [r0, #12]
    adds    r0, r0, #4
    bx      lr

    .thumb_func
stub:
    movs    r0, #0
    bx      lr
    .align  2
desc:
    .space  32
