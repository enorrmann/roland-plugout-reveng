# Volume II — Chapter 10: Native Ops of the DSP — Status, Use and Plan

Goal (user's decision, 2026-10-28): **test all the native ops of the DSP on the device and use them all** in s1cc and
in the SYSTEM-S1CC host, to make the most of the DSP, starting with the ones that contribute most. This document
says, for each op, what state it is in and what is missing. The exact semantics are in
[Vol. II ch. 1](core_architecture.md) §5–§8; the use from C, in [Vol. V ch. 1](../vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md) §7.6.

## 1. States

| state | means |
|---|---|
| **S8 exact** | measured on the SYSTEM-8 master with the probe (`re/dspnative.py` or `re/dspexact.py`), bit-identical to the emulator |
| **S1 tested** | ran in a SYSTEM-1 PLUG-OUT without hanging it and sounded like the emulator |
| **emulator** | the C reference (`emu/dspdec.h`, `emu/dspcore.h`, `emu/dspjit.py`; `re/dspisa.py` archived on 2026-10-29) models it like the device |
| **s1cc** | intrinsic (`s1dsp.h`) and / or automatic use in `lower.cpp` / `kernel.cpp` |
| **host** | used by `examples/system_s1cc/make_s1cc.py` (LFO, ADSR, COLOR MOD, mixer) |

## 2. State per op (2026-10-29, late)

| op | what it does | S8 exact | emulator | s1cc | host | S1 tested |
|---|---|---|---|---|---|---|
| `shl` / `shr` / `shrn` | X·2^k, two's complement with 32 fraction bits (toward −∞ at 2^−32, wraps at ±512), then RAW truncates | yes (2026-10-29: 15 360 cases) | yes (corrected twice) | automatic (products by powers of 2 > 256), `s1_ldexp` | no | **no** |
| `floor` / `ceil` (SFU) | floor / ceiling | yes (`floor` was "negmask") | yes (corrected) | `floorf`, `s1_floor`, `s1_ceil` | no | **no** |
| `sign` (SFU) | −1 / 0 / +1 | yes | yes | `s1_sign` | no | **no** |
| `recipseed` + `newton` | Goldschmidt 1/x: 4 ops, 10⁻⁸; `recipseed` overwrites rY with m | yes | yes (corrected) | division `x / y`, `s1_recip` | no | **no** |
| `expsplit` (SFU) | e^⌊x⌋ from a ROM of its own + frac | yes (ROM read) | yes (corrected) | `exp2` with a range > ±8 octaves, `s1_exp2`, `s1_exp` | no | **no** |
| `log2split` (SFU) | x = m·2^e, m ∈ [0.75, 1.5); Y < 0 in two's complement (e from Y + 512) | yes (2026-10-29, Y < 0 over the whole range) | yes (corrected) | `s1_log2` | no | **no** |
| `add3` / `sum8` (A) | D + X + Y / X + Y + R | yes | yes | `s1_sum3` | no | **no** |
| MAC, coefficient in a register | X·Y + C·Z (+ W), /2 only with P2 | yes | yes | `s1_mac2` (needs 5–6 registers) | no (4 registers per voice) | **no** |
| `ph_wrap` / `ph_clamp` / `ph_tri` | phase, clipping, triangle | yes | yes | `s1_wrap`, `s1_clamp`, `s1_tri` | yes | **yes** |
| `min` / `max` | | yes | yes | `fminf`, `fmaxf`, comparisons | yes | **yes** |
| `absadd` / `subabs` | \|X\| + Y, Y − \|X\| | yes | yes | **no** (`fabs` comes out as `max r, r, -r`) | no | no |
| `minn` / `maxn` | min(−X, Y), max(−X, Y) | yes | yes | **no** | no | no |
| `frexp` | m ∈ [0.5, 1), exponent | yes | yes | **no** | no | no |
| `atanhsplit` (SFU) | split for log by atanh | yes | yes | no (covered by `log2split`) | no | no |
| `sqrtsplit` (SFU) | m = \|Y\|·4^k ∈ [1, 4) in rY, 1/√m seed (9 bits) in r(d+1), ±2^−k in r(d+3) | yes (2026-10-29) | yes | no | no | no |
| MAC with an immediate coefficient | X·Y + K·Z (+ W); K at byte C (+8 with bit 9) | yes (2026-10-29, 20 forms; D8 closed) | yes | no | — | no |
| two MACs in one instruction (class 0x080) | two one-product MACs, in parallel; with P2 none writes | yes (2026-10-29) | yes | no | no | no |
| flags + condition (`X9bY`, `cond`) | skip units according to the result of another | in progress 2026-10-29: Roland instructions with conditions on the master, exact except classes 0x590 / 0x190 (template) | yes | no | no | no |
| jumps and loops (`009Y`) | | yes (S8, slaves by audio) | yes | no | no | no |
| indexed loads (`X28Y` bank 0; `da`/`de`, `db`/`df` bank 5; `hset` h4–h7) | address = ⌊h·16384⌋ + Y | yes (master exact 2026-10-29, `dspverify --programs`) | yes | no (s1cc) | **yes**: COLOR MOD (`CMOD_IDX`); bank 5 in `LFO_LUT` / `ENV_CURVE` | **yes** (`hset` + `X28Y`, MP-CSAW2-8); bank 5 untested (MP-CSAW2-LUT) |
| delay line (`tapreq`, `f5c`, `f6Y`) | bank-2 taps | yes (master, DSP4 `f7dc`) | yes | no | no | no |
| parameter ramps (`FCE4`, ARM) | the ARM ramps a DSP parameter | — | no | no | no | **measured 2026-10-29**: linear, one step every 1/3 ms, 4–97 ms from 0 to 1; chainable through the end-of-ramp interrupt (`FE3C`; measured 2026-10-30: 6225 calls, one every 4.00 ms) ([examples/system_1_ramp/README.md](../../examples/system_1_ramp/README.md)) |

> Note: the "S1 tested" column is the state of the table's date; the log (§5, "2026-10-29, late: tested on the
> SYSTEM-1") records that NATIVE-CHECK / NATIVE-CHECK2 later ran every single op of the list on the SYSTEM-1, each
> alone in its instruction.

## 3. Plan, in order of contribution

Every step follows the same path: (1) measure on the SYSTEM-8 if it is not exact (`re/dspnative.py`: add the test to
`TESTS`); (2) emulator equal to the device; (3) intrinsic in s1cc and automatic use where it shortens; (4) use in the
host (`make_s1cc.py`); (5) test on the SYSTEM-1 (a PLUG-OUT like NATIVE-CHECK, or `TRY_FORMS` for groups of ops) and
only then mark it tested here and in `asm/proven_forms.txt`.

1. **Test on the SYSTEM-1 what is already done.**
   * `plugins/NATIVE-CHECK` (`sdk/s1cc/examples/native_ops/native_check.c`): clean saw = the kernel's ops are fine;
     OSC1 COLOR selects the op (0–27 all; 28 `s1_recip`, 56 `s1_log2` + `s1_exp`, 84 `s1_floor`, 112 `s1_sign`, 140
     `s1_mac2`, 168 `s1_sum3`, 196 `shl`, 224 `expsplit` + `s1_ceil`). If it hangs when selecting it: switch off and
     on, and bisect with builds of one op each.
   * `plugins/BRAIDS-CSAW` rebuilt with the corrected division (the clicks of 2026-10-27 came from reading after
     `recipseed`).
   * `MP-CSAW2` with 7 voices and `MOD_DSP=B`: installed on 2026-10-28, not confirmed by ear.
   * Any kernel rebuilt since 2026-10-28 already brings automatic `shl`: NATIVE-CHECK first.
2. **Indexed loads** (the biggest contribution).
   * COLOR MOD: instead of weighting 5 sources (10 chained `mad` per voice), the sources of each voice in blocks of 8
     words of bank 5 and one load `da` (color 0, h6) / `db` (color 1, h7) with h = base of the chosen source (`hset`
     mode 0 from a register loaded from a parameter; once per sample, outside the voices). Per voice: 2 loads + 2
     `mad` + 2 stores. It is what MP-CSAW2 is missing for 8 voices (DSP B uses 782 of 708). Careful: one instruction
     between `40TY` and the transfer; the envelopes live in bank 0 and would have to be copied to bank 5 (one access
     to bank 5 per instruction on the S1, rule R5).
   * Tables read by phase (wavetables, `const` tables with a computed index): today they go to islands
     (polynomials); with `da` / `X28Y` they would be one load. See [bank5-tables](core_architecture.md) §7.4 and
     `re/wt5.py` (wavetables on the S8).
   * Per-voice delay lines (chorus of CRUSHER, flanger): `X28Y` in bank 0, relative to the ring.
3. **Flags, conditions and jumps.** Selections (`a < b ? x : y`) without arithmetic; control-rate code (LFO, colors,
   envelopes every N samples with a counter and a jump). Tested on the S8 slaves by audio; on the S1, nothing. First a
   test PLUG-OUT with a jump that skips half of the voices.
4. **MAC with an immediate coefficient.** Measure on the S8 (D8: where it reads the immediate when Y is a register).
   With it the MAC works with 4 registers (the constant does not take one): colors, filters, mixes. Then the MAC /2
   and two MACs per instruction.
5. **Automatic `absadd` / `subabs` / `minn` / `maxn`.** The polyBLEP `u·(2 − |u|)` and `max(min(w, dt), −dt)` of
   `mp_saw_bl`, the triangles `1 − 2·|x|`: one op less per edge in all the oscillators.
6. **`frexp` (`s1_frexp`), `sqrtsplit` (model it: dspnative with other d / Y and the full Q table) → `s1_sqrt`,
   `s1_rsqrt`; `atanhsplit`.**
7. **Delay taps (`tapreq`) and `FCE4` ramps** (map the service in the S1 OS).

## 4. How to resume

```sh
# SYSTEM-8 (measure): the lab with the probe in slot 3, select PLG3
python3 examples/from_scratch/re/dsplive.py install l00.batch
python3 examples/from_scratch/re/dsppeek.py ping                 # pong 0x1234
python3 examples/from_scratch/re/dspnative.py -n 48              # all the tests; --list, --sim, names

# emulator and compiler
python3 examples/from_scratch/emu/test_core.py                   # 12/12
python3 -m unittest discover -s sdk/s1cc/tests                   # 6/6
for f in asm/tests/test_*.py; do python3 $f; done
sh sdk/s1cc/examples/mutable_ported/check_all.sh                 # voices per kernel (table of examples_mutable_ported_README.md §4)

# SYSTEM-1 (test)
python3 tools/plugout_tool.py install plugins/NATIVE-CHECK --slot 1 --force --no-patches -y
```

Only one program at a time on the device (MIDI): do not run two tools together.

## 5. Log

* **2026-10-30: ramp chain on the device.** C-S1RAMP with LFO SHAPE ≥ 3: `FE3C` registered our end-of-ramp function,
  which was called 6225 times, once every 4.00 ms, each time launching the next ramp. No hangs; `FE40` releases it.
  Envelopes and LFO on the ARM without a tick: possible ([Vol. III ch. 8](../vol3_voice_allocation_scheduling/s1_optimizations.md) §3).
* **2026-10-29, night (2): ramps and LUT.** `FCE4` measured on the SYSTEM-1 (C-S1RAMP): linear, one step every
  1/3 ms, 4–97 ms from 0 to 1 ([examples/system_1_ramp/README.md](../../examples/system_1_ramp/README.md)). In the OS:
  `FE3C(sel, id, fn)` registers an own end-of-ramp function and the interrupt calls it at every end, without
  releasing it: ramps can be chained without a tick (test ready in C-S1RAMP, not run). Bank-5 indexed loads
  (`ldb5`/`ldb5h7`) in the scheduler and in SYSTEM-S1CC (`LFO_LUT`, `ENV_CURVE`; MP-CSAW2-LUT built, untested). Group
  `min:imm + shl:reg` tested (MP-BELL-8). Catalogue of savings: [Vol. III ch. 8](../vol3_voice_allocation_scheduling/s1_optimizations.md).
* **2026-10-29, night: indexed loads on the SYSTEM-1.** `plugins/MP-CSAW2-8` (`CMOD_IDX=1`, `MOD_DSP=B`): COLOR MOD
  with `hset h6` (once per color and sample, the row of the chosen source written by the ARM) and `ldidx r, 2 v` (the
  source of voice v in a table of bank 0, `CMT0` 0x0500, where the envelopes store their level): 8 voices (DSP B 707
  of 708 instructions; 7 before), sound bit-identical to the weighted sum in the emulator with the 6 sources, and on
  the device "everything works". First time on the S1 (Roland's programs for the S1 do not use them).
  `asm/dspsched.py` packs `ldidx` as a bank-0 load (two instructions after the `hset`, after the bank-0 stores);
  `asm/s1rules.py` counts it as a bank-0 load. Op groups tested on the S1 the same day (bisection MP-CSAW2-T2…T6):
  `max + ph_clamp`, `min + ph_wrap`, `ph_wrap + subxy`, `mul:reg + subxy:imm`; they hang: two B ops of the same
  family and the MAC with immediate K together with another op (`asm/bad_forms.txt`, patterns; rule R9).
* **2026-10-29, late: tested on the SYSTEM-1.** `plugins/NATIVE-CHECK` (recompiled) and `plugins/NATIVE-CHECK2`
  (`sdk/s1cc/examples/native_ops/native_check2.c`) in slot 1: clean saw at full volume in all positions of OSC1 COLOR,
  that is, no error in any op: `recipseed` + `newton`, `log2split`, `expsplit`, `floor`, `ceil`, `sign`, register
  MAC, `sum8`, `shl`, `sqrtsplit` + Newton, MAC with an immediate coefficient (`mac_p1wc3`) and with an immediate Y
  (`mac_p1p2c3`), `absadd` with an immediate, the fusion into the MAC (`mac_p1w`). Each one alone in its instruction
  (the groups that share an instruction remain in `asm/proven_forms.txt`). Seen in passing: without DSP B in the
  kernel, the SUB / NOISE of the host's mixer does not go through any envelope (it keeps sounding).

* **2026-10-29.** The reference became the C code (`emu/dspref.py`; `re/dspisa.py` archived, [Vol. V ch. 5](../vol5_toolchain_compiler_sdk/emulator.md)
  §1.1). Measured on the S8 master and corrected in the reference: MAC with an immediate coefficient (20 forms, rule of
  byte C + 8·bit 9 in 19 classes: D8 closed; `asm/asm.py` and `asm/disasm.py` use it), two MACs per instruction, /2
  only with P2, rounding of `shl`/`shr` (two's complement with 32 fraction bits), `log2split` with Y < 0 over the
  whole range, complete `sqrtsplit` (9-bit Q table, output r(d+3)). Full battery of `dspnative.py`: all tests 0
  differences.

* **2026-10-28.** Exact measurements on the S8 master: `shl`/`shr`/`shrn`, `floor`, `ceil`, `sign`, `recipseed`,
  `expsplit`, `log2split`, `atanhsplit`, `frexp`, `newton`, `add3`, `sum8`, register-coefficient MAC (+, −, /2), 24–96
  cases per op, with and without an empty instruction before reading. Five differences of the emulator corrected
  ([Vol. V ch. 5](../vol5_toolchain_compiler_sdk/emulator.md) §3.7). Bug of s1cc found and corrected (the division read the input overwritten by
  `recipseed`). Intrinsics and automatic use in s1cc; `asm/isa.json` (SFU forms, `floor`, `shl` with a number, measured
  MACs), `asm/dspsched.py` (`_native`). `MOD_DSP=auto` in SYSTEM-S1CC: no kernel of mutable_ported lost voices, 8
  gained. NATIVE-CHECK and BRAIDS-CSAW built, not installed.

> **Note on paths:** in this chapter `emu/…`, `re/…` are relative to `examples/from_scratch/`.
