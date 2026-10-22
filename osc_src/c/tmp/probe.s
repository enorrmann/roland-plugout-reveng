	.cpu cortex-m7
	.arch armv7e-m
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 1
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"probe.cc"
	.text
	.align	1
	.global	_Z1fv
	.syntax unified
	.thumb
	.thumb_func
	.type	_Z1fv, %function
_Z1fv:
	.fnstart
.LFB0:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	mov	r3, #1610612736
	movs	r2, #5
	str	r2, [r3, #992]
	ldr	r2, [r3, #1000]
	adds	r2, r2, #2
	str	r2, [r3, #984]
	ldr	r3, [r3, #988]
	bx	lr
	.cantunwind
	.fnend
	.size	_Z1fv, .-_Z1fv
	.ident	"GCC: (15:14.2.rel1-1) 14.2.1 20241119"
