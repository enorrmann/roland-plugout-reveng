;; SYSTEM-S1CC host microcode: SL_WT, a wavetable oscillator on the slave's DSP A (docs/MUTABLE_MEMORY.md §4.1).
;; Template syntax: make_s1cc.py frag(). Scheduled like the rest of the slave programs (asm/dspsched.py).
;;
;; The frame being played lives in a loop of SL_L words in DSP A's bank 0 (the ring: stored at S, read at S + k k
;; samples later): each sample the word read at S + SL_L is stored again at S, so the loop keeps its contents. Slot j
;; = the value stored when the loop counter c was j; the frame's point k is in slot SL_L - 1 - k (reversed, so that
;; point k + 1 is the next ring word). Region 1 (engine.c sl_wt_tick, every 1 ms) puts SL_B points of the frame into
;; bank 5 SL_STG.. (FD6C sel 0, A0xx: DSP A reads them) and the first slot they belong to into SL_BLK; while c is in
;; that block the loop takes the staged point instead of the re-circulated one. Each voice reads its two neighbour
;; points with ldidx and interpolates; the result replaces the kernel's output word (XV + v).
;; Units of c, d, a: samples / 16384 (the hset unit). subxy rD, rX, Y = Y - rX.

.fragment sl_wt_loop
; SL_WT: the loop counter c (0 .. SL_L - 1, units), the frame loop at bank 0 {SL_S:04x} .. {SL_S + SL_L:04x}
ld r0, [{SL_C + 1:04x}]
addxy r0, r0, #{2.0 ** -14!r}
mov r1, #{SL_L / 16384 - 2.0 ** -15!r}
subxy r1, r0, r1
sign r1, r1
addxy r1, r1, #1.0
mul r1, r1, #0.5
mul r0, r0, r1
st r0, [{SL_C:04x}]
; the staged block: d = c - its first slot; m = 1 when 0 <= d < SL_B
ld r1, [{SL_BLK:04x}]
subxy r1, r1, r0
mov r2, r1
addxy r2, r2, #{2.0 ** -15!r}
sign r2, r2
addxy r2, r2, #1.0
mul r2, r2, #0.5
mov r3, #{SL_B / 16384 - 2.0 ** -15!r}
subxy r3, r1, r3
sign r3, r3
addxy r3, r3, #1.0
mul r3, r3, #0.5
mul r2, r2, r3
; the staged point of slot c (index kept inside the block), the re-circulated one, w = x + m (s - x)
max r1, r1, #0.0
min r1, r1, #{(SL_B - 1) / 16384!r}
addxy r1, r1, #{((SL_STG & 0x3FF) + 0.5) / 16384!r}
hset h6, r1
ldb5 r3, 0
ld r1, [{SL_S + SL_L:04x}]
subxy r3, r1, r3
mul r3, r3, r2
addxy r3, r3, r1
st r3, [{SL_S:04x}]
.if SL_ALIAS
mov r1, #1.0
st r1, [0770]
.endif


.fragment sl_wt_voice v
;; SL_WT=b0 voice {v}: the loop holds two frames interleaved (data d = 2k: frame E point k, 2k + 1: frame O point k;
;; slot j = data SL_L - 1 - j, so data d + 1 is the next ring word): phase p (0 .. 1) += s1_dt, x = p SL_N, one hset
;; at data 2 floor(x), ldidx Y 0..3 = E k, O k, E k+1, O k+1; y = yE + w (yO - yE), w = bank 5 SL_W5 (region 1:
;; COLOR between the two frames, 0 = E) -> its output word
; SL_WT=b0 voice {v}
ld r4, [{DT[v]:04x}]
ld r5, [{SL_P + 2 * v + 1:04x}]
addxy r5, r5, r4
mov r6, #1.0
subxy r6, r6, r5
sign r6, r6
addxy r6, r6, #1.0
mul r6, r6, #0.5
subxy r5, r6, r5
st r5, [{SL_P + 2 * v:04x}]
mul r4, r5, #{float(SL_N)!r}
floor r6, r4
subxy r4, r6, r4
mul r6, r6, #{2 / 16384!r}
ld r0, [{SL_C + 1:04x}]
addxy r6, r6, r0
mov r1, #{(SL_L - 0.5) / 16384!r}
subxy r1, r1, r6
sign r1, r1
addxy r1, r1, #1.0
mul r1, r1, #{SL_L / 32768!r}
subxy r6, r1, r6
addxy r6, r6, #{(SL_S + 1 + 0.5) / 16384!r}
hset h6, r6
ldidx r0, 0
ldidx r1, 1
ldidx r2, 2
ldidx r3, 3
subxy r2, r0, r2
mul r2, r2, r4
addxy r0, r0, r2
subxy r3, r1, r3
mul r3, r3, r4
addxy r1, r1, r3
ld r2, [{SL_W5:04x}]
subxy r1, r0, r1
mul r1, r1, r2
addxy r0, r0, r1
.if SL_VTAG
mul r0, r0, #{(v + 1) / 8!r}
.endif
st r0, [{XV[v]:04x}]


;; SL_WT=b0 SL_MORPH=0 voice {v} (as WT-SLAVE0L): the loop holds one frame of SL_N = SL_L points (data k = point k,
;; slot j = data SL_L - 1 - j); phase p += s1_dt, x = p SL_N, one hset at data floor(x), ldidx Y 0 / 1 = points k and
;; k + 1 (k + 1 = SL_L reads the word past the loop, which is slot c - 1 again: the frame wraps), linear interpolation
;; between the two points (no mix of frames: region 1 writes the frame COLOR picks) -> its output word
.fragment sl_wt1_voice v
; SL_WT=b0 voice {v}, one frame
ld r4, [{DT[v]:04x}]
ld r5, [{SL_P + 2 * v + 1:04x}]
addxy r5, r5, r4
mov r6, #1.0
subxy r6, r6, r5
sign r6, r6
addxy r6, r6, #1.0
mul r6, r6, #0.5
subxy r5, r6, r5
st r5, [{SL_P + 2 * v:04x}]
.if SL_N > 256
; x = p SL_N as (p 256) (SL_N / 256): immediates above 256.0 have no encoding
mul r4, r5, #256.0
mul r4, r4, #{SL_N / 256!r}
.else
mul r4, r5, #{float(SL_N)!r}
.endif
floor r6, r4
subxy r4, r6, r4
mul r6, r6, #{1 / 16384!r}
ld r0, [{SL_C + 1:04x}]
addxy r6, r6, r0
mov r1, #{(SL_L - 0.5) / 16384!r}
subxy r1, r1, r6
sign r1, r1
addxy r1, r1, #1.0
mul r1, r1, #{SL_L / 32768!r}
subxy r6, r1, r6
addxy r6, r6, #{(SL_S + 1 + 0.5) / 16384!r}
hset h6, r6
ldidx r0, 0
ldidx r1, 1
subxy r1, r0, r1
mul r1, r1, r4
addxy r0, r0, r1
.if SL_VTAG
mul r0, r0, #{(v + 1) / 8!r}
.endif
.if SL_ALIAS
; SL_ALIAS probe: x the word sl_wt_loop stores 1.0 into at 0x0770, read at 0x0371 (voices 0-3: 1.0 only if bank 0
; wraps at 0x0400) or 0x0771 (voices 4-7: the control, 1.0 in any model)
ld r1, [{(0x0371 if v < 4 else 0x0771):04x}]
mul r0, r0, r1
.endif
.if M2S_READ
; M2S_PROBE: x the master B -> slave word 600E + v (1.0 / 0.5 if it arrives)
ld r1, [{0x600E + v:04x}]
mul r0, r0, r1
.endif
st r0, [{XV[v]:04x}]


.fragment sl_wt5_voice v
;; SL_WT=b5 (first test, docs/MUTABLE_MEMORY.md §4.1): the frame in bank 5 SL_STG .. + SL_N (SL_N points and a guard
;; copy of point 0), written by region 1 (FD6C sel 0) whenever COLOR moves; voice {v}: phase p (0 .. 1) += s1_dt,
;; x = p SL_N, two indexed bank-5 loads (ldb5, h6 = the point's word), linear interpolation -> its output word
; SL_WT=b5 voice {v}
ld r4, [{DT[v]:04x}]
ld r5, [{SL_P + 2 * v + 1:04x}]
addxy r5, r5, r4
mov r6, #1.0
subxy r6, r6, r5
sign r6, r6
addxy r6, r6, #1.0
mul r6, r6, #0.5
subxy r5, r6, r5
st r5, [{SL_P + 2 * v:04x}]
mul r4, r5, #{float(SL_N)!r}
floor r6, r4
subxy r4, r6, r4
mul r6, r6, #{1 / 16384!r}
addxy r6, r6, #{((SL_STG & 0x3FF) + 0.5) / 16384!r}
hset h{SL_H[v]}, r6
{"ldb5" if SL_H[v] == 6 else "ldb5h7"} r1, 0
{"ldb5" if SL_H[v] == 6 else "ldb5h7"} r2, 1
subxy r2, r1, r2
mul r2, r2, r4
addxy r1, r1, r2
.if SL_VTAG
mul r1, r1, #{(v + 1) / 8!r}
.endif
st r1, [{XV[v]:04x}]


;; SL_WT=m voice {v} (EDM_WT WT_SLAVE, docs/MUTABLE_MEMORY.md §4.4): master B reads the voice's sample from the table in
;; the delay memory and sends it by the master -> slave word 600E + v (one sample late); here it only becomes the voice's
;; output, which DSP B filters and amplifies as any oscillator
.fragment sl_m2s_voice v col nfm1
; SL_WT=m voice {v}
ld r0, [{WT_M2S + v:04x}]
st r0, [{XV[v]:04x}]
.if WT_CMOD and v < WT_NV
; WT_CMOD: the host's COLOR of voice {v} (0..1) -> the table position (units, frame x 2048 / 16384), in steps of
; 1 / WT_CQ frame (WT_CQ = 0: no step) -> master B by 6008 + v
ld r1, [{col:04x}]
;; COLOR + COLOR MOD can leave 0..1: a position outside the table reads the loop's unwritten part (silence with bursts,
;; WT-CONT with COLOR MOD, 2026-11-06): clamp to 0..1
max r1, r1, #0.0
min r1, r1, #1.0
.if WT_LINES
;; WT_LINES: q = the nearest quarter frame, frame k = floor(q/4), line j = q - 4k: u = k/8 - j WT_LSU - WT_LOFFU (units)
;; -> 6008 + v. Master B's age then never wraps (always >= 254) and lands in line j one lap older (the lines keep two
;; laps; they are rebuilt identically every lap); no split on master B (its budget, WT-LINES 640 instructions hung)
mul r1, r1, #{4.0 * nfm1!r}
addxy r1, r1, #0.5
floor r1, r1
mul r0, r1, #0.25
floor r0, r0
mul r0, r0, #-4.0
addxy r1, r1, r0
mul r1, r1, #{-WT_LSU!r}
addxy r1, r1, #{-WT_LOFFU!r}
mul r0, r0, #-0.03125
addxy r1, r1, r0
.else
mul r1, r1, #{nfm1!r}
.endif
.if WT_LINES
.elif WT_CQ
.if WT_CQ > 1
mul r1, r1, #{float(WT_CQ)!r}
.endif
addxy r1, r1, #0.5
floor r1, r1
mul r1, r1, #{0.125 / WT_CQ!r}
.else
mul r1, r1, #0.125
.endif
st r1, [{wt_s2m(v):04x}]
.endif
