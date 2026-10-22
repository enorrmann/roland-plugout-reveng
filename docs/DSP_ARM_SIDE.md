# DSP "ports", banks and ARM-side configuration (static analysis, 2026-10-15)

Sources: `dumps/os_full.bin` → `work/os/{s_boot,m1_tcm,m1_sdram}/*.bin` (already extracted), Ghidra project
`work/ghidra/sboot` (decompiled with `re/ghidra/Decompile.java`), capstone (`re/tdis.py`), corpus `work/corpus/*.bin`
decoded with `re/dspisa.py`/`dspdis.py`. Nothing was run on the device.

> **Status (2026-10-15, after hardware tests):** §3.2 (`X9bY` = condition-group write) was **tested and rejected**
> on master DSP3 and on the slave DCO2 slot (see `DSP_ISA_SPEC.md` §5.3, `NOTES.md`): the condition groups never
> changed. The correlation in the corpus is kept below as data. Everything else here is static analysis, not yet
> verified on the hardware. Paths under `work/` are local (not in git).

> **Status note (2026-10-17)**: §3.2's hypothesis was right after all: `X9bY` writes the condition flags of group
> X−8 from unit Y (measured; the earlier "rejection" used an op with a constant result). Bus behaviour was measured
> on the master (MXM latched per sample, 1-sample latency, point-to-point; IOMD = owned I/O words). Current,
> consolidated reference: `DSP_HARDWARE.md`. This file stays as the static analysis of the ARM side.

## TL;DR

1. **The reference-word prefix is a 3-bit bank number** (bits 15–13) and the ARM driver's bank table uses the same
   numbering: 0 = DT0 (`0nnn`), 1 = DT1 (`2nnn`; one base for DSP0 and DSP1, one each for DSP2–DSP5 — corrected
   2026-10-22, §6), 2 = DT2 (`4nnn`, delay-line/tap window),
   3 = IOMD (`6nnn`, I/O mailbox, shared by all 6 DSPs), 4 = MXM (`8nnn`, inter-DSP mixer/bus window),
   5 = DPC (`Annn`), 6 = ENV/parameter block (`Cnnn`, handled by its own API), 7 = `Ennn` (read-only, unidentified).
2. **The audio buses between DSPs are ordinary `fe` loads/stores to banks 3 and 4**, not `X9bY`. In every synth the
   main programs show a fixed pipeline DSP0/1 → DSP2/3 → DSP4/5 → IOMD (out to the wave bus).
3. **`X9bY` is a "condition source" write**: in 831 of the 923 corpus instructions that contain an `X9bY`
   (90 %), a condition (C) word follows **exactly 2 instructions later**. The baseline rate is 7.4 %. Port Y selects
   the condition group that C words address with `cc>>2`: 456 of the 461 C words with cc 4–7 come after a port-1
   write, and 184 of 204 with cc 8–11 after a port-2 write. Port 3 goes with cc 12–15, more loosely.
4. **`Xa9Y` only appears as `Xa90`**, always 2–3 instructions after a special-function op `{8+X}f6c` on the
   same register. The pair is a delay-line tap request: the next loads read bank-2 words `4001..4006` (`4101..` on
   the master), which act as tap results (sample, next sample, fraction) feeding a linear interpolation. This is
   the chorus/delay mechanism.
5. ARM side: there is no ARM routing code for MXM, IOMD or DP. The routing is fixed by which bank-4/bank-3 addresses
   the microcode uses. The ARM sets up the following: system/DMA registers at 0x41100000; per-DSP control blocks
   0x4110{2,4..9}000 (page maps, +0x310 run/mode, +0x318 task, +0x400/+0x700 indirect ports); clearing IDM/EDM
   (delay memories); FIR tables at 0x41152000/0x41153000; generic bank access through tables in `s_boot`
   0x0111C2E0.

---

## 1. CEZUtilDSPDump table (`m1_sdram` 0x61014BB0, 129 entries × 16 B: {flags, name\*, address, bytes})

The same table and strings also exist in `s_data` (slave debug console). `flags` = (view type << 16) | page,
where the low half is the console page/group (0 = system + MXM0/1, 2..7 = DSP0..5, 8 = DSPM, 9 = DP/IOMD) and the
high half is the view (2 = SD31/Q31, 3 = RAW, 4 = IEEE float, 5 = program, 6/7 = IEEE first entry of a DPC/DP group,
0x1xx = start of a new column). The name strings are at 0x61078134..0x610787B0. Six IPGn/PRGn names point
into `m1_tcm` (0x0113FBD8…).

| # | entry addr | flags | name | ARM address | bytes | words |
|---|---|---|---|---|---|---|
| 0 | 0x61014BB0 | 0x61014B48 | Sys Control | 0x41100000 | 0x80 | 0x20 |
| 1 | 0x61014BC0 | 0x01000000 | MXM0M | 0x411D0000 | 0x800 | 0x200 |
| 2 | 0x61014BD0 | 0x01000001 | MXM0O | 0x411D0800 | 0x800 | 0x200 |
| 3 | 0x61014BE0 | 0x00000001 | MXZ0 | 0x411D1000 | 0x800 | 0x200 |
| 4 | 0x61014BF0 | 0x00000001 | MXS0 | 0x411D1800 | 0x800 | 0x200 |
| 5 | 0x61014C00 | 0x00000001 | MXE0 | 0x411D2000 | 0x40 | 0x10 |
| 6 | 0x61014C10 | 0x00040001 | MXM1M | 0x411D8000 | 0x800 | 0x200 |
| 7 | 0x61014C20 | 0x00000001 | MXM1O | 0x411D8800 | 0x800 | 0x200 |
| 8 | 0x61014C30 | 0x00000001 | MXZ1 | 0x411D9000 | 0x800 | 0x200 |
| 9 | 0x61014C40 | 0x00000001 | MXS1 | 0x411D9800 | 0x800 | 0x200 |
| 10 | 0x61014C50 | 0x00000001 | MXE1 | 0x411DA000 | 0x40 | 0x10 |
| 11 | 0x61014C60 | 0x00040001 | DPC0 (IEEE) | 0x41114000 | 0x1000 | 0x400 |
| 12 | 0x61014C70 | 0x01020002 | DPC0 (SD31) | 0x41115000 | 0x1000 | 0x400 |
| 13 | 0x61014C80 | 0x00030002 | DPC0 (RAW) | 0x41116000 | 0x1000 | 0x400 |
| 14 | 0x61014C90 | 0x00040002 | DT00 (IEEE) | 0x41400000 | 0x2000 | 0x800 |
| 15 | 0x61014CA0 | 0x00020002 | DT00 (SD31) | 0x41402000 | 0x2000 | 0x800 |
| 16 | 0x61014CB0 | 0x00030002 | DT00 (RAW) | 0x41404000 | 0x2000 | 0x800 |
| 17 | 0x61014CC0 | 0x00040002 | DT01 (IEEE) | 0x41410000 | 0x2000 | 0x800 |
| 18 | 0x61014CD0 | 0x00020002 | DT01 (SD31) | 0x41412000 | 0x2000 | 0x800 |
| 19 | 0x61014CE0 | 0x00030002 | DT01 (RAW) | 0x41414000 | 0x2000 | 0x800 |
| 20 | 0x61014CF0 | 0x00040002 | MXM2M(->DSP2) | 0x41430000 | 0x800 | 0x200 |
| 21 | 0x61014D00 | 0x00000002 | MXM2O(->DSP2) | 0x41430800 | 0x800 | 0x200 |
| 22 | 0x61014D10 | 0x00000002 | MXZ2 (->DSP2) | 0x41431000 | 0x800 | 0x200 |
| 23 | 0x61014D20 | 0x00000002 | MXS2 (->DSP2) | 0x41431800 | 0x800 | 0x200 |
| 24 | 0x61014D30 | 0x00000002 | MXE2 (->DSP2) | 0x41432000 | 0x40 | 0x10 |
| 25 | 0x61014D40 | 0x00040002 | IPG0 (name in m1_tcm) | 0x41440000 | 0x3000 | 0xC00 |
| 26 | 0x61014D50 | 0x00050002 | PRG0 (name in m1_tcm) | 0x41460000 | 0x10000 | 0x4000 |
| 27 | 0x61014D60 | 0x00040002 | ENV0 | 0x414A0000 | 0x8000 | 0x2000 |
| 28 | 0x61014D70 | 0x00060002 | DPC1 (IEEE) | 0x41118000 | 0x1000 | 0x400 |
| 29 | 0x61014D80 | 0x01020003 | DPC1 (SD31) | 0x41119000 | 0x1000 | 0x400 |
| 30 | 0x61014D90 | 0x00030003 | DPC1 (RAW) | 0x4111A000 | 0x1000 | 0x400 |
| 31 | 0x61014DA0 | 0x00040003 | DT10 (IEEE) | 0x41500000 | 0x2000 | 0x800 |
| 32 | 0x61014DB0 | 0x00020003 | DT10 (SD31) | 0x41502000 | 0x2000 | 0x800 |
| 33 | 0x61014DC0 | 0x00030003 | DT10 (RAW) | 0x41504000 | 0x2000 | 0x800 |
| 34 | 0x61014DD0 | 0x00040003 | DT11 (IEEE) | 0x41510000 | 0x2000 | 0x800 |
| 35 | 0x61014DE0 | 0x00020003 | DT11 (SD31) | 0x41512000 | 0x2000 | 0x800 |
| 36 | 0x61014DF0 | 0x00030003 | DT11 (RAW) | 0x41514000 | 0x2000 | 0x800 |
| 37 | 0x61014E00 | 0x00040003 | MXM3M(->DSP3) | 0x41530000 | 0x800 | 0x200 |
| 38 | 0x61014E10 | 0x00000003 | MXM3O(->DSP3) | 0x41530800 | 0x800 | 0x200 |
| 39 | 0x61014E20 | 0x00000003 | MXZ3 (->DSP3) | 0x41531000 | 0x800 | 0x200 |
| 40 | 0x61014E30 | 0x00000003 | MXS3 (->DSP3) | 0x41531800 | 0x800 | 0x200 |
| 41 | 0x61014E40 | 0x00000003 | MXE3 (->DSP3) | 0x41532000 | 0x40 | 0x10 |
| 42 | 0x61014E50 | 0x00040003 | IPG1 (name in m1_tcm) | 0x41540000 | 0x3000 | 0xC00 |
| 43 | 0x61014E60 | 0x00050003 | PRG1 (name in m1_tcm) | 0x41560000 | 0x10000 | 0x4000 |
| 44 | 0x61014E70 | 0x00040003 | ENV1 | 0x415A0000 | 0x8000 | 0x2000 |
| 45 | 0x61014E80 | 0x00060003 | DPC2 (IEEE) | 0x4111C000 | 0x1000 | 0x400 |
| 46 | 0x61014E90 | 0x01020004 | DPC2 (SD31) | 0x4111D000 | 0x1000 | 0x400 |
| 47 | 0x61014EA0 | 0x00030004 | DPC2 (RAW) | 0x4111E000 | 0x1000 | 0x400 |
| 48 | 0x61014EB0 | 0x00040004 | DT20 (IEEE) | 0x41600000 | 0x2000 | 0x800 |
| 49 | 0x61014EC0 | 0x00020004 | DT20 (SD31) | 0x41602000 | 0x2000 | 0x800 |
| 50 | 0x61014ED0 | 0x00030004 | DT20 (RAW) | 0x41604000 | 0x2000 | 0x800 |
| 51 | 0x61014EE0 | 0x00040004 | DT21 (IEEE) | 0x41610000 | 0x2000 | 0x800 |
| 52 | 0x61014EF0 | 0x00020004 | DT21 (SD31) | 0x41612000 | 0x2000 | 0x800 |
| 53 | 0x61014F00 | 0x00030004 | DT21 (RAW) | 0x41614000 | 0x2000 | 0x800 |
| 54 | 0x61014F10 | 0x00040004 | MXM4M(->DSP4) | 0x41630000 | 0x800 | 0x200 |
| 55 | 0x61014F20 | 0x00000004 | MXM4O(->DSP4) | 0x41630800 | 0x800 | 0x200 |
| 56 | 0x61014F30 | 0x00000004 | MXZ4 (->DSP4) | 0x41631000 | 0x800 | 0x200 |
| 57 | 0x61014F40 | 0x00000004 | MXS4 (->DSP4) | 0x41631800 | 0x800 | 0x200 |
| 58 | 0x61014F50 | 0x00000004 | MXE4 (->DSP4) | 0x41632000 | 0x40 | 0x10 |
| 59 | 0x61014F60 | 0x00040004 | IPG2 (name in m1_tcm) | 0x41640000 | 0x3000 | 0xC00 |
| 60 | 0x61014F70 | 0x00050004 | PRG2 (name in m1_tcm) | 0x41660000 | 0x10000 | 0x4000 |
| 61 | 0x61014F80 | 0x00040004 | ENV2 | 0x416A0000 | 0x4000 | 0x1000 |
| 62 | 0x61014F90 | 0x00060004 | DPC3 (IEEE) | 0x41120000 | 0x1000 | 0x400 |
| 63 | 0x61014FA0 | 0x01020005 | DPC3 (SD31) | 0x41121000 | 0x1000 | 0x400 |
| 64 | 0x61014FB0 | 0x00030005 | DPC3 (RAW) | 0x41122000 | 0x1000 | 0x400 |
| 65 | 0x61014FC0 | 0x00040005 | DT30 (IEEE) | 0x41700000 | 0x2000 | 0x800 |
| 66 | 0x61014FD0 | 0x00020005 | DT30 (SD31) | 0x41702000 | 0x2000 | 0x800 |
| 67 | 0x61014FE0 | 0x00030005 | DT30 (RAW) | 0x41704000 | 0x2000 | 0x800 |
| 68 | 0x61014FF0 | 0x00040005 | DT31 (IEEE) | 0x41710000 | 0x2000 | 0x800 |
| 69 | 0x61015000 | 0x00020005 | DT31 (SD31) | 0x41712000 | 0x2000 | 0x800 |
| 70 | 0x61015010 | 0x00030005 | DT31 (RAW) | 0x41714000 | 0x2000 | 0x800 |
| 71 | 0x61015020 | 0x00040005 | MXM5M(->DSP5) | 0x41730000 | 0x800 | 0x200 |
| 72 | 0x61015030 | 0x00000005 | MXM5O(->DSP5) | 0x41730800 | 0x800 | 0x200 |
| 73 | 0x61015040 | 0x00000005 | MXZ5 (->DSP5) | 0x41731000 | 0x800 | 0x200 |
| 74 | 0x61015050 | 0x00000005 | MXS5 (->DSP5) | 0x41731800 | 0x800 | 0x200 |
| 75 | 0x61015060 | 0x00000005 | MXE5 (->DSP5) | 0x41732000 | 0x40 | 0x10 |
| 76 | 0x61015070 | 0x00040005 | IPG3 (name in m1_tcm) | 0x41740000 | 0x3000 | 0xC00 |
| 77 | 0x61015080 | 0x00050005 | PRG3 (name in m1_tcm) | 0x41760000 | 0x10000 | 0x4000 |
| 78 | 0x61015090 | 0x00040005 | ENV3 | 0x417A0000 | 0x4000 | 0x1000 |
| 79 | 0x610150A0 | 0x00060005 | DPC4 (IEEE) | 0x41124000 | 0x1000 | 0x400 |
| 80 | 0x610150B0 | 0x01020006 | DPC4 (SD31) | 0x41125000 | 0x1000 | 0x400 |
| 81 | 0x610150C0 | 0x00030006 | DPC4 (RAW) | 0x41126000 | 0x1000 | 0x400 |
| 82 | 0x610150D0 | 0x00040006 | DT40 (IEEE) | 0x41800000 | 0x2000 | 0x800 |
| 83 | 0x610150E0 | 0x00020006 | DT40 (SD31) | 0x41802000 | 0x2000 | 0x800 |
| 84 | 0x610150F0 | 0x00030006 | DT40 (RAW) | 0x41804000 | 0x2000 | 0x800 |
| 85 | 0x61015100 | 0x00040006 | DT41 (IEEE) | 0x41810000 | 0x2000 | 0x800 |
| 86 | 0x61015110 | 0x00020006 | DT41 (SD31) | 0x41812000 | 0x2000 | 0x800 |
| 87 | 0x61015120 | 0x00030006 | DT41 (RAW) | 0x41814000 | 0x2000 | 0x800 |
| 88 | 0x61015130 | 0x00040006 | IPG4 (name in m1_tcm) | 0x41840000 | 0x3000 | 0xC00 |
| 89 | 0x61015140 | 0x00050006 | PRG4 (name in m1_tcm) | 0x41860000 | 0x10000 | 0x4000 |
| 90 | 0x61015150 | 0x00040006 | ENV4 | 0x418A0000 | 0x4000 | 0x1000 |
| 91 | 0x61015160 | 0x00060006 | DPC5 (IEEE) | 0x41128000 | 0x1000 | 0x400 |
| 92 | 0x61015170 | 0x01020007 | DPC5 (SD31) | 0x41129000 | 0x1000 | 0x400 |
| 93 | 0x61015180 | 0x00030007 | DPC5 (RAW) | 0x4112A000 | 0x1000 | 0x400 |
| 94 | 0x61015190 | 0x00040007 | DT50 (IEEE) | 0x41900000 | 0x2000 | 0x800 |
| 95 | 0x610151A0 | 0x00020007 | DT50 (SD31) | 0x41902000 | 0x2000 | 0x800 |
| 96 | 0x610151B0 | 0x00030007 | DT50 (RAW) | 0x41904000 | 0x2000 | 0x800 |
| 97 | 0x610151C0 | 0x00040007 | DT51 (IEEE) | 0x41910000 | 0x2000 | 0x800 |
| 98 | 0x610151D0 | 0x00020007 | DT51 (SD31) | 0x41912000 | 0x2000 | 0x800 |
| 99 | 0x610151E0 | 0x00030007 | DT51 (RAW) | 0x41914000 | 0x2000 | 0x800 |
| 100 | 0x610151F0 | 0x00040007 | IPG5 (name in m1_tcm) | 0x41940000 | 0x3000 | 0xC00 |
| 101 | 0x61015200 | 0x00050007 | PRG5 (name in m1_tcm) | 0x41960000 | 0x10000 | 0x4000 |
| 102 | 0x61015210 | 0x00040007 | ENV5 | 0x419A0000 | 0x4000 | 0x1000 |
| 103 | 0x61015220 | 0x00060007 | DPCM (IEEE) | 0x41110000 | 0x1000 | 0x400 |
| 104 | 0x61015230 | 0x01020008 | DPCM (SD31) | 0x41111000 | 0x1000 | 0x400 |
| 105 | 0x61015240 | 0x00030008 | DPCM (RAW) | 0x41112000 | 0x1000 | 0x400 |
| 106 | 0x61015250 | 0x00040008 | DTM0 (IEEE) | 0x41200000 | 0x2000 | 0x800 |
| 107 | 0x61015260 | 0x00020008 | DTM0 (SD31) | 0x41202000 | 0x2000 | 0x800 |
| 108 | 0x61015270 | 0x00030008 | DTM0 (RAW) | 0x41204000 | 0x2000 | 0x800 |
| 109 | 0x61015280 | 0x00040008 | DTM1 (IEEE) | 0x41210000 | 0x2000 | 0x800 |
| 110 | 0x61015290 | 0x00020008 | DTM1 (SD31) | 0x41212000 | 0x2000 | 0x800 |
| 111 | 0x610152A0 | 0x00030008 | DTM1 (RAW) | 0x41214000 | 0x2000 | 0x800 |
| 112 | 0x610152B0 | 0x00040008 | PRGM | 0x41240000 | 0x10000 | 0x4000 |
| 113 | 0x610152C0 | 0x00040008 | PRGMPT | 0x41260000 | 0x10000 | 0x4000 |
| 114 | 0x610152D0 | 0x00040008 | MTCB | 0x41280000 | 0x40000 | 0x10000 |
| 115 | 0x610152E0 | 0x00000008 | WM2418 | 0x41300000 | 0x80000 | 0x20000 |
| 116 | 0x610152F0 | 0x00000008 | WM2418(RAW) | 0x41380000 | 0x80000 | 0x20000 |
| 117 | 0x61015300 | 0x00070008 | DP01 (IEEE) | 0x41130000 | 0x1000 | 0x400 |
| 118 | 0x61015310 | 0x01020009 | DP01 (SD31) | 0x41131000 | 0x1000 | 0x400 |
| 119 | 0x61015320 | 0x00030009 | DP01 (RAW) | 0x41132000 | 0x1000 | 0x400 |
| 120 | 0x61015330 | 0x00040009 | DP23 (IEEE) | 0x41134000 | 0x1000 | 0x400 |
| 121 | 0x61015340 | 0x00020009 | DP23 (SD31) | 0x41135000 | 0x1000 | 0x400 |
| 122 | 0x61015350 | 0x00030009 | DP23 (RAW) | 0x41136000 | 0x1000 | 0x400 |
| 123 | 0x61015360 | 0x00040009 | DP45 (IEEE) | 0x41138000 | 0x1000 | 0x400 |
| 124 | 0x61015370 | 0x00020009 | DP45 (SD31) | 0x41139000 | 0x1000 | 0x400 |
| 125 | 0x61015380 | 0x00030009 | DP45 (RAW) | 0x4113A000 | 0x1000 | 0x400 |
| 126 | 0x61015390 | 0x00040009 | IOMD0(DSP4/5) | 0x4113C000 | 0x300 | 0xC0 |
| 127 | 0x610153A0 | 0x00000009 | IOMD1(DSP2/3) | 0x4113C300 | 0x80 | 0x20 |
| 128 | 0x610153B0 | 0x00000009 | IOMD2(DSP0/1) | 0x4113C380 | 0x80 | 0x20 |

The table ends at 0x610153C0. Next to it, 0x610153E0 holds `{0x411D0800, 0x411D8800, 0x41430800, 0x41530800,
0x41630800, 0x41730800}` = MXMnO for n = 0..5, used by the MXM monitor (`m1_sdram` 0x61004810:
`&MXMnO[ch] = tbl[n] + 4·ch`; monitor 0x6100B84E prints `MXM%d Ch%3d (%08Xh) = %08Xh %f`).

**Per-DSP layout that follows** (DSP n, n = 0..5; DSPM in brackets):

| ARM address | name | size | notes |
|---|---|---|---|
| 0x41400000+n·0x100000 (+0x2000 SD31, +0x4000 RAW) [0x41200000] | DTn0 [DTM0] | 0x800 words | **bank 0** |
| +0x10000 [0x41210000] | DTn1 [DTM1] | 0x800 words | **bank 1**. DSP0's bank 1 is 0x41510000 (= DT11), the same as DSP1's. DSP2–DSP5 each have their own (0x41610000…0x41910000, s_boot 0x0111C580). **Corrected 2026-10-22:** only DSP0/DSP1 share it, not every pair (§6) |
| +0x20000 | (no debug entry) | 0x600 words, IEEE view only | **bank 2** (s_boot table 0x111C628) |
| +0x30000 (n ≥ 2); 0x411D0000/0x411D8000 for n = 0/1 | MXMnM / MXMnO (+0x800) / MXZn (+0x1000) / MXSn (+0x1800) / MXEn (+0x2000, 0x40 B) | 0x200 words each | **bank 4**. Labelled "(->DSPn)": DSP n's input mixer. Physically DSP2's lives in DSP0's window 0x41430000, and so on |
| +0x40000 | IPGn | 0x3000 B | page-mapped through ctrl +0x200 (12 × 1 KiB) |
| +0x60000 | PRGn | 0x10000 B | page-mapped through ctrl +0x100 (16 × 4 KiB) |
| +0x80000 | program memory (s_boot 0x111C190) | | slot programs |
| +0xA0000 | ENVn | 0x8000 B (DSP0/1) / 0x4000 B | **bank 6**, `+0xA0` API, 16 B per id |
| 0x41114000+n·0x4000 [0x41110000] (+0x1000 SD31, +0x2000 RAW) | DPCn [DPCM] | 0x400 words | **bank 5** (`A000`–`A3FF`). `A400`–`A7FF` behaves as a window shared by the pair (0,1)/(2,3)/(4,5) in Roland's mains; probably DP01/DP23/DP45 (§6, inferred) |
| 0x4113C000 (+0x1000 SD31, +0x2000 RAW) | IOMD0 (DSP4/5) 0x00–0xBF, IOMD1 (DSP2/3) 0xC0–0xDF, IOMD2 (DSP0/1) 0xE0–0xFF | 0x100 words | **bank 3**, same address for all 6 DSPs (s_boot 0x111C660) |
| 0x41130000 / 0x41134000 / 0x41138000 (+0x1000, +0x2000) | DP01 / DP23 / DP45 | 0x400 words each | members of the DSPM object (s_boot 0x01108B94: obj+0x70..+0x90). Not a DSP bank in the driver's table |
| 0x41240000 / 0x41260000 | PRGM / PRGMPT | 0x10000 B each | DSPM program (+ "PT" = page table?) |
| 0x41280000 | MTCB | 0x40000 B | "MTCB Monitor" / "VTCB Monitor" pages |
| 0x41300000 / 0x41380000 | WM2418 / WM2418(RAW) | 0x80000 B each | DSPM object fields obj+0x34/+0x38 (= DTM base + 0x100000/0x180000). The IDMEM-checksum list calls it "WMEM", so it is more likely **wave/delay memory** than the DAC |
| 0x41100000 | Sys Control | 0x80 B | system registers (§2.1) |

Other debug strings that place these: the resource column header `PRG DP0 DP1 DT0 DT1 ECF EDM STEP` (per DSP,
used/total ‰; `m1_sdram` 0x6100123C), the IDMEM checksum list `IOMD0 DSP0 DT0 DPC01 IOMD1 DSP1 DT1 IOMD2 DSP2
DPC23 DSP3 DSP4 DPC45 DSP5 ECF WMEM DPM DSPM`, the DSP interrupt units `DSP_MXM0..5 DSP_ENV0..5 DSP_AC DSP_DMC DSP_TEE
DSP_REF DSP_CUF DSP_PDH DSP_TCSH DSP_SN DSP_LOOP DSP_SEQ DSP_MDT DSP_WDT`, and the ENV modes
`00:disable 10:auto ini 01:timer 11:env`.

## 2. ARM code that configures the DSP blocks (s_boot = slave; m1_tcm has the same driver, `bmc_dsp_drv.cpp`)

### 2.1 System block 0x41100000 (s_boot 0x0110D982 `BMC DSP init`, 0x0110DB70, 0x01111D34, 0x01112670; m1_sdram 0x61003794)
* `+0x04` enable mask (`|= param`, shadow at 0x20081050+0xC): 0x01000000 at init, 0x04000000 in 0x0110DB70, and
  **0x02007FFF in CORE1_DSPEnable** (s_boot 0x01102504 / m1_tcm 0x011022B4). `+0x08` bit 31 = busy (polled by the
  indirect accessors). `+0x0C` written `pc<<18` and then 0x80000000 by BMC11 init, then polled.
* `+0x20` = 0x31, `+0x3C` = 0, `+0x2C` = 0x1422220B, `+0xEC` = 0, `+0x80` = 0x80200005, `+0x90` = 0x80000005
  (memory-controller set-up, probably for the delay memories).
* **Two fill/DMA channels**: `+0x50/+0x54/+0x58` = **IDM** (address, ctl = len \| flag<<28 \| 0xC0000000, monitor),
  `+0x40/+0x44/+0x48` = **EDM** (external delay memory). At init: IDM cleared with `+0x54 = 0xC0020000`
  (0x20000 words), EDM cleared with `+0x44 = 0xC0200000` (0x200000 words). API at 0x01111D34(ch, addr, len, flag).
  The debug page (m1_sdram 0x61003794) prints `IDM_CTRL` = [+0x54], `IDM_MON` = [+0x58] (max), `EDM_CTRL`,
  `EDM_MON` = [+0x48] (max), `WVM_MON` = [+0xF0] & 0x1FFF (wave-bus memory fill, ‰ of 4096) and `WVM_THRE`.
* `+0x100..+0x17C`: 16 pairs (descriptor, value) = {0xCCC41200, 0x38008200}, {+0x10, 0x38008000}, 0xDC441220…,
  0x9C003030…, the rest 0. These are probably the MEVF event slots (`StartTrig(MEVF ID=%d)`: a ring of 4 event
  words, command 0x13, s_boot 0x01108C16).
* `+0x180/+0x188` = timestamps (0x011147E0), `+0x184` = 0x80134000. `+0x200` interrupt enable `|= 0x3FFFFFFF`
  after the 0x39–0x56, 0x81, 0x82 IRQs are registered (0x01112670). `+0x280` (0xE000E008 alias) = 0x400000.
* 0x41152000 ← 225 halfword coefficients (<<16) from s_boot 0x0111901C, and 0x41153000 ← 449 from 0x011191DE. Both
  are symmetric low-pass FIR kernels (sample-rate converter or decimator, "ECF"?).
* 0x41101820 ← a float table (DSPM test/converter coefficients, 0x01102370), and 0x41200000+0xEC = 0x30.

### 2.2 Per-DSP control block 0x41104000 + n·0x1000 (n = 0..5), DSPM 0x41102000 (pointer tables at s_boot 0x0111C190..0x0111C2B0)
| reg | writer | value / meaning |
|---|---|---|
| +0x20..+0x3C | 0x0110D982 | {0x84000005, 0x04000000 ×3} ×2 (DSPM: 0x04000005…) |
| +0x100..+0x13C, +0x140 | 0x0110D93C | page map, 16 × (`i<<12 \| 0x80000000`), then +0x140 = 0x20000000 (PRG window) |
| +0x200..+0x22C, +0x240 | 0x0110D93C, remap 0x01110CAE | 12 × (`i<<10 \| 0x80000000`), then +0x240 = 0x20000000 (IPG window, 1 KiB pages < 0x3000) |
| +0x310 | init 0x20000000; 0x011123A2(dsp, bank, addr, mode) | `mode<<28 \| bank<<24 \| addr` (bank < 3, addr even; DSP4/5 may not use bank 2). DSPEnable: slave DSP0..3 = 0x30000000, DSPM = 0x32000000, slave DSP4/5 left at 0x20000000; master DSP0..5 and DSPM = 0x3000000x. Meaning unknown: run/sync mode and a start pointer? |
| +0x318 | 0x01110054 | task mask \| prio<<28 (already known) |
| +0x400/+0x404 | 0x01110D98 | indirect data access: `addr&0x7FFFF \| size<<28`, wait sys+8, data at +4 |
| +0x700/+0x704 | 0x01110098 / 0x011101DE | the same for program memory (halfword program port) |
| +0x600 | table only | not seen written |

Data-memory helpers: 0x0111085A (zero +0x40000/+0x44000 windows, DMA if ≥ 64 B), 0x01110660 / 0x011106F4
(copy into or out of +0x40000/+0x44000), 0x01111C9E (+0x48000/+0x4C000: writes a 20-bit value three times),
0x0111178A (ENV entry: `[+8] = mode<<28 | flag<<24 | ramp<<20 | 0x08000000`, `[+0xC] = target`).

### 2.3 Generic bank API (the `+0xB0`-style services)
s_boot 0x0111C2E0 = 12 bank pointers per view (IEEE; SD31 at +0x30; RAW at +0x60; sizes at +0x30/+0x90 of each
group): bank 0 0x111C4D8 (DT0), 1 0x111C580 (DT1), 2 0x111C628 (DT2), 3 0x111C660 (IOMD), 4 0x111C400 (MXM M),
5 0x111C430 (DPC), 6..11 = none. The writer at 0x011111FC (and the readers after it) index `tbl[bank][dsp] + 4·idx`
with a bound check. **This is the same numbering as the microcode reference prefix.**

### 2.4 What was not found
* No ARM code writes the MXM, MXZ, MXS or MXE areas, IOMD, or DP01/23/45 at start-up or at program load. The only
  literals are the debug table, the bank table and the DSPM object fields (DP accessors at s_boot 0x01108A3A..0x01108B10,
  not referenced by any vtable, so possibly dead). Routing is therefore fixed by the microcode addresses.
* `BMCInitWaveBus()` (s_boot 0x01100B90) only programs pin-mux registers 0x400265xx–0x400267F0 (0x801/0xC01): the
  wave bus is an external pin bus between the BMCs.
* The slot-swap path (0x01105F94 → 0x011100DE) writes program memory only.

## 3. What the microcode banks and `X9bY` / `Xa9Y` are

### 3.1 Banks: corpus statistics (`fe` transfers, reference = bank<<13 | addr)
| bank | loads / stores | address range used | ARM |
|---|---|---|---|
| 0 | 7423 / 5412 | 0000–0681 | DT0 |
| 1 | 684 / 575 | 0200–024D | DT1 (`22nn` mailbox) |
| 2 | 120 / 32 | 0000–030F, **only in effect programs** (m1_tcm DSP4 slots, J60/JX `b000`) | DT2: delay/tap window |
| 3 | 82 / 106 | 0000–00E3 (≤ 0x100) | IOMD |
| 4 | 552 / 596 | 0000–062B | MXM: M 0x000–1FF, O 0x200–3FF, Z 0x400–5FF, S 0x600–7FF |
| 5 | 767 / 132 | 0020–040F | DPC |
| 6 | 1985 / 0 | 0000–021C | ENV/params (read-only) |
| 7 | 7 / 0 | 0013–0014 | ? (e.g. `e013` read repeatedly in the JX effect; maybe a hardware source such as noise or a counter) |

**Routing (identical in JX, J106, SH, S100 and the s_data defaults; main programs in DSP0..5 order):**
* DSP0 stores Z `404–416`, and DSP2 loads M `004–016`. DSP1 stores Z `400–410`, and DSP3 loads O `200–210`.
* DSP2 stores Z `404–41A`, and DSP4 loads M `005–01A`. DSP3 stores Z `400–414`, and DSP5 loads O `201–214`.
  So MXM of DSP n: M ← Z of the even DSP of the previous pair, O ← Z of the odd one. The word number is kept
  (`Z 4nn → M/O nn`): MAIN2 copies `M 0nn → Z 4nn` with the same nn, and the ranges above are ranges of use
  (MAIN4 does not read M 004) (§6).
* Feedback: DSP2 stores M `017–019`, and DSP0 loads S `617–619`. DSP4 stores M `01B–01D`, and DSP2 loads
  S `61B–61D`. A store into one's own M area appears in the S area of the DSP two positions back.
* Pair window (bank 5 `A4nn`, §6): DSP0 stores `A404–A40E`, DSP1 loads them; DSP4 stores `A404–A407`, DSP5 loads
  them; DSP5 stores `A400–A403`, DSP4 loads them. No main loads what it stores there itself.
* IOMD: DSP0 loads `E0–E2` (IOMD2 area), DSP4 loads `00–06` and stores `20–24`, `60–66`, DSP5 loads `10–17` and
  stores `B0–B7` (IOMD0 area). These are the slave's outputs to the wave bus and its inputs from it.

### 3.2 `X9bY` = condition flags (CONFIRMED 2026-10-17: group X−8 ← flags of unit Y; see DSP_ISA_SPEC.md §8)
* Occurrences: `99b1` 377, `b9b2` 221, `b9b1` 191, `a9b2` 134, `b9b3` 109, `c9b1/2/3` 35, `99b2` 9, `d9b1` 1, plus
  `b9b4`, `89ba`, `99bc` in a single JX effect. X is always ≥ 8 (store form), Y = 1..3 (4, A, C rare), and no slot
  is used.
* **Latency 2:** of 923 instructions with an `X9bY`, 831 have a C word at distance 2 (110 at d = 1, 101 at d = 0,
  261 at d = 3). Roland pads with `0001` to get there (JX MAIN0 818 → 825 `0001` → 826 C).
* **Group = `cc>>2`** (cc = bits 12–9 of the C word): cc 4–7 → port 1 (456/461 preceded by a port-1 write, 174 of
  the 197 cc 5 at exactly d = 2), cc 8–11 → port 2 (184/204), cc 12–15 → port 3 (cc 15 135/137; cc 12/13 are also
  used without any write, so they may be partly global), cc 0–3 → never preceded by a write (group 0 = global/context
  flags such as program start).
* This explains the "context condition bits": groups 1–3 hold whatever was last written. On idle master DSP3 the three
  rows are identical (`1110 1110 1110`), as expected with no writes. In the slave DCO slot they are `0110 0110 0110`
  (stale values from the main program). The earlier measurement "X9bY does not change the condition bits" probably
  used a C in a different group, or a distance below 2.
* `cc&3` selects the test on the written value (cc 4/5/7 are common, 6 rare). The exact predicates (sign, zero,
  ≥ 1?) are still unknown.

### 3.3 `Xa9Y` = fetch a special-function / delay-tap result
* Seen only as `3a90`, `4a90`, `6a90` (effects: master DSP4 slots `efdc`/`f7dc`, J60 chorus `b000`). Each comes
  2–3 instructions after `bf6c`/`cf6c`/`ef6c` on the same register (100 % pairing, 24 cases).
* Example (J60 chorus, `work/corpus/J60_b000_60a1f8e8.bin`): 264 `[4000] <- r0` (write the delay input), r3/r6 =
  clamped LFO delays, 317 `bf6c` (r3), 332 `ef6c` (r6) + `3a90`, 335 `r2 <- [4003]; r1 <- [4002]`, 341
  `r0 <- [4001]`, 346 `r0 = r0 + r2·(r1 − r0)` + `6a90`, 350–358 `[4004..4006]` with the same interpolation. Master:
  the same with `[4100..410F]`.
  The `Xa90` destination register is overwritten before use (r4 at 204 in `efdc_01133828`), which matches "no effect
  on r0–r7". **Model:** `{8+d}f6c` takes rd as a tap delay. `da90` commits tap 0/1. The hardware then exposes
  `[base+1]` = x[n−⌊d⌋], `[base+2]` = x[n−⌊d⌋−1], `[base+3]` = frac(d) (tap 1) and `[base+4..6]` (tap 2) in bank 2.
  `[base+0]` is the write port.

## 4. Proposed experiments (master DSP3 through the peek plug-out, or lab slot + USB audio)
1. **Condition groups.** Program: `r1 = v` (imm) ; `99b1` ; `0001` ; `2d06 1050 1052 Cw 3000 0000` with
   Cw = `(2cc)<<8 | 0x12` for cc = 4..15 ; store r1 to bank 0 → peek. Sweep v ∈ {−1, −2⁻²⁰, 0, +2⁻²⁰, 0.5, 1, 2} and
   d = 0..3 (`0001` padding). Expected: only cc 4–7 react, only from d = 2, and the 4×7 table gives the predicates.
   Repeat with `b9b2` (cc 8–11) and `b9b3` (cc 12–15). On audio: the select chooses between two constant levels
   on the DCO2 output.
2. **Bank decode check (ARM read/write only):** in DSP3 store constants to `6000+k` and `8400+k` and read
   0x4113C000+4k (IOMD, IEEE) and 0x41531000+4k (MXZ3 in DSP1's window; check 0x41530000..0x41532000). Write
   0x41530000+4k (MXM3M) from the ARM and load `80kk` in the DSP (copy to bank 0). Also store to `8000+k` and look
   for it in another DSP's S area.
3. **Delay taps:** each sample, store a ramp (sample counter) to `[4000]`. Set r3 = D (try 1.0, 10.0, 0.5, D/2¹⁶
   scalings), `bf6c` r3, `0001`, `3a90`, then load `[4001] [4002] [4003]` and store them to bank 0. The difference
   between the ramp and `[4001]` gives the delay unit, and `[4003]` the fraction encoding. Read DT2 at 0x41620000
   (IEEE) in DSP2/… windows to see the ring. Try it on master DSP4 if DSP3's DT2 is not active (+0x310 bank field).
4. Optional: read sys+0x48/+0x58/+0xF0 (EDM/IDM/WVM monitors) and the per-DSP +0x310 values on the running master
   to see the delay memories in use.

## 5. Useful addresses (s_boot unless noted)
0x0110D982 BMC DSP init · 0x0110DB70 memory-controller/IDM/EDM clear · 0x0110D93C page maps · 0x01102504
CORE1_DSPEnable (m1_tcm 0x011022B4) · 0x011123A2 +0x310 writer · 0x01110054 task · 0x01110098/0x011101DE program
port · 0x01110D98 data port · 0x01110CAE IPG remap · 0x01111D34 IDM/EDM DMA · 0x011111FC bank write · 0x0111C190..
0x0111C700 pointer tables · 0x01108B18 DSP-object constructor (descriptor table 0x01117A94, DSPM gets DP01/23/45) ·
m1_sdram 0x61003794 IDM/EDM/WVM monitor, 0x6100123C resource usage, 0x6100B84E MXM monitor.

## 6. Verificación y correcciones (2026-10-22, emulador y listados)

Estudio completo en `VOICE_ALLOCATION.md` §6; aquí sólo lo que corrige o amplía este documento. Etiquetas:
[listado] = microcódigo o binario ARM desensamblado; [inferido] = deducción con la medida que la confirmaría.

* **Banco 1 (DT1) no se comparte por pares** [listado]. La tabla de bancos del OS del slave (`s_boot` `0x0111C580`,
  vista IEEE) da a DSP0 y DSP1 la misma base `0x41510000`; DSP2–DSP5 tienen `0x41610000`, `0x41710000`,
  `0x41810000`, `0x41910000`. El TL;DR y la tabla del §1 decían "compartido por el par" en general. Si DSP0/DSP1
  comparten de verdad la memoria o sólo la ventana del ARM sigue **abierto** (medida: escribir valores distintos en
  `[2210]` desde DSP0 y DSP1 y leer `0x41410000`/`0x41510000`).
* **Banco 1 se comporta como anillo** [listado, inferido]: los programas de ranura DCO escriben sus salidas en
  `[221B]`–`[2229]` (impares) y MAIN2/MAIN3 las leen en `[221C]`–`[222A]` (+1), nunca en la dirección escrita; nadie
  escribe las pares. MAIN0 guarda `[2200]` y lee `[2201]`. Es el mismo idioma "guardar en S, leer S+1" del banco 0
  (`DSP_HARDWARE.md` §7.2). Igual en JX-3P, JUNO-60, JUNO-106, JUPITER-4 y JUPITER-8 (`emu/allplugouts.py`). **Medido
  2026-10-22:** C-V4, que sólo lee las salidas en `+1`, suena con las ocho voces (`VOICE_ALLOCATION.md` §6.7).
* **Banco 5 `A400`–`A7FF` = buzón de la pareja** [listado; medido con C-V4 para DSP5 → DSP4]: ver la viñeta nueva del §3.1. Explica cómo las
  voces 2 y 3 (DSP5) llegan a la salida `6020`/`6021` que escribe DSP4, y corrige la idea de que DSP5 sólo reenvía a
  `60B0` (que no se oye).
* **MXM conserva el número de palabra** [listado + medido en el maestro]: añadido al §3.1.
* **Ningún código ARM escribe MXM/IOMD** (§2.4) sigue en pie: el enlace entre slaves por IOMD (`6000`–`6003` que
  suma MAIN4) lo pone el hardware o el maestro, no el slave. Qué palabra del slave 1 llega a qué entrada del slave 0
  está **abierto**.
