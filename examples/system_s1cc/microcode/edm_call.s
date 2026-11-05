;; EDM_CALL (docs/MUTABLE_MEMORY.md §3.3.3): our own tapreq inside the OS's master B effect at 0x0, which is the only
;; program whose requests the delay memory serves (§3.3). Nothing is queued at 0x0 (system-1/WAVE_BUS.md §5.3): region
;; 0 (model.c edm_call) writes these words live into master B's program memory (0x40170000 + byte address):
;;   edm_call_block  at byte 0x0C00 (free: the effect ends at 0x552, our main starts at 0x1000), after the OS's own
;;                   two instructions that the call replaces (copied there at run time from the DSP, not shipped)
;;   edm_call_site   over those two instructions (the effect's second tapreq and the instruction after it, 5 halfwords
;;                   at byte 0x164 in OS 1.30): the call to 0x0C00 (word 0x600) and a filler, 5 halfwords too
;; At the site r4 = the effect's position counter p (its second tap computes its address the same way: f6 r2, -r4);
;; r2 is free until the effect reloads it. The results go to h5 = 402E (x0, x1, fraction; the effect uses
;; 4004..402D) and the main (edm.s master_edm, EDM_REQ 0) reads them one sample later.

.fragment edm_call_block EDM_RES EDM_AGE
mov r2, #{EDM_AGE}@0:24
hset h4, #256@0:24
hset h5, #{(EDM_RES[0] - 0x4000) / 16384}@0:16
f6 r2, -r4
tapreq r2, 0
cond 0, 15, 8

.fragment edm_call_site
.slot @0 0600
cond 0, 10, 0
mov r2, r2

;; EDM_CALL=write (probe): our own line. Left: the send 2000 (the main's, this sample) written to bank-2 word EDM_WR
;; with base h4 = EDM_BASE (beyond the OS line: base 256, 35.15625 units), then read at EDM_AGE with the same base
;; and the effect's position r4 (as its own taps). Right (control): the OS line (base 256) at EDM_AGE / 2. An echo on
;; the left = a store to an even bank-2 word writes the EDM at base - p, as the OS delay's [4001].
;; Probe 1 (EDM_RBASE = EDM_BASE = 296, 2026-11-03): left silent, right (OS line) echoed at 24006 samples.
;; Probe 2: left read with the OS base (EDM_RBASE 256), right with ours; region 0 turns the OS delay's write off
;; (C001 = 0, EDM_CALL_W in model.c), so the left hears only what our store wrote, at whatever age it landed.
;; Probe 2 result: silence on both (14 s recorded). Probe 3 (EDM_BASE 256 EDM_RBASE 296 EDM_OSOFF 0): the OS line
;; read with base 296 on the left, 256 on the right: is the base a plain address offset on one moving ring?
.fragment edm_call_block_w EDM_RES EDM_AGE EDM_BASE EDM_WR EDM_RBASE
ld r3, [2000]
hset h4, #{EDM_BASE}@0:24
mov r2, #{EDM_AGE}@0:24
st r3, [{EDM_WR:04x}]
hset h4, #{EDM_RBASE}@0:24
hset h5, #{(EDM_RES[0] - 0x4000) / 16384}@0:16
f6 r2, -r4
tapreq r2, 0
hset h4, #{EDM_BASE}@0:24
mov r2, #{EDM_AGE / 2}@0:24
hset h5, #{(EDM_RES[1] - 0x4000) / 16384}@0:16
f6 r2, -r4
tapreq r2, 0
cond 0, 15, 8

;; EDM_CALL=idx (probe, docs/MUTABLE_MEMORY.md §3.3.6): is a tapreq address absolute? With EDM_ECF our writer
;; [4034] -> 0x440000 (h4 units: 272) gets the left send (master_edm EDM_WR). Two requests on that line:
;;   left  (EDM_RES[0]): the OS's form, base 272 + age - p (r4 = the effect's position counter): an echo at EDM_AGE
;;                       if the OS's convention carries over to a line of ours
;;   right (EDM_RES[1]): base 272 alone (r2 = 0, f6 r2, -r2): one fixed address. Absolute addressing: a value that
;;                       changes once per lap of the ring (a step, near silence); relative to the write position: the
;;                       send itself one or two samples late (the dry signal doubled on the right)
;; h4 back to the OS's 256 before the return.
.fragment edm_call_block_idx EDM_RES EDM_AGE EDM_HB
mov r2, #{EDM_AGE}@0:24
hset h4, #{EDM_HB}@0:24
hset h5, #{(EDM_RES[0] - 0x4000) / 16384}@0:16
f6 r2, -r4
tapreq r2, 0
mov r2, #0@0:24
hset h5, #{(EDM_RES[1] - 0x4000) / 16384}@0:16
mov r7, r7
f6 r2, -r2
tapreq r2, 0
hset h4, #256@0:24
cond 0, 15, 8

;; EDM_STR with STR_TAP (docs/MUTABLE_MEMORY.md §3.3.5): the strings' reads by tapreq instead of map-table reads (the
;; table cannot change at run time: the data queue drains only when the PLUG-OUT is selected, a live ARM store hung
;; the device, 2026-11-04). Per voice v two requests on its own line (base (STR_A0 + v STR_DA) / 16384, the OS's form
;; age - p, measured on our line by MP-EDMIDX: 48000 -> 48001): age d_v (C030 + v, written by region 0 at every note)
;; -> results STR_RES + 8v (y0, y1, fraction), age d_v + 2 (C038 + v) -> STR_RES + 8v + 3 (y2, y3); the main reads
;; them one sample later (microcode/edm_str.s). h4 back to the OS's 256 before the return; only r2 is used.
.fragment edm_str_block
.for v in range(str_nv())
    ld r2, [c{STR_PAGE0 + v:03x}]
    hset h4, #{(STR_A0 + STR_DA * v) // 16384}@0:24
    hset h5, #{(STR_RES + 8 * v - 0x4000) / 16384}@0:16
    f6 r2, -r4
    tapreq r2, 0
    ld r2, [c{STR_PAGE2 + v:03x}]
    hset h4, #{(STR_A0 + STR_DA * v) // 16384}@0:24
    hset h5, #{(STR_RES + 8 * v + 3 - 0x4000) / 16384}@0:16
    f6 r2, -r4
    tapreq r2, 0
.endfor
hset h4, #256@0:24
cond 0, 15, 8
;; EDM_STR_PAD=n (bisection 2026-11-04): n filler instructions after the return, never run: the block's size alone
.for _ in range(STR_PAD)
    mov r2, r2
.endfor

;; EDM_STR_ONE: one request per line (6 of ours per sample ran on the device, 8 hung: 2026-11-04); a line per voice,
;; two for BOWED / FLUTED (str_lines)
.fragment edm_str_block1
.for v in range(str_lines())
    ld r2, [c{STR_PAGE0 + v:03x}]
    hset h4, #{(STR_A0 + STR_DA * v) // 16384}@0:24
    hset h5, #{(STR_RES + 8 * v - 0x4000) / 16384}@0:16
    f6 r2, -r4
    tapreq r2, 0
.endfor
hset h4, #256@0:24
cond 0, 15, 8


.fragment edm_wt_block
;; EDM_WT (edm_wt.s): the requests of the wavetable voices (two per voice: frame f and f + 1) on their own line WT_A,
;; ages from bank 0 WT_AGEB + 4k + 1 (written by the main), results at WT_RES + 8k (x0, x1, fraction), read by the
;; main the next sample. 2 WT_NV <= 6 requests per sample (the measured limit, §3.3.5)
.for k in range(2 * WT_NV)
    ld r2, [{WT_AGEB + 4 * k + 1:04x}]
    hset h4, #{WT_A // 16384}@0:24
    hset h5, #{(WT_RES + 8 * k - 0x4000) / 16384}@0:16
    f6 r2, -r4
    tapreq r2, 0
.endfor
hset h4, #256@0:24
cond 0, 15, 8
