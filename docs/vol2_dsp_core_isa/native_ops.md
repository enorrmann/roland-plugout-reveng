# Volume II — Chapter 10: Native Ops of the DSP — Status, Use and Plan

> **Scope:** both: each op has a column for the SYSTEM-8 measurement and one for the SYSTEM-1 test.

Goal (user's decision, 2026-10-28): **test all the native ops of the DSP on the device and use them all** in s1cc and
in the SYSTEM-S1CC host, to make the most of the DSP, starting with the ones that contribute most. This document
says, for each op, what state it is in and what is missing. The exact semantics are in
[Vol. II ch. 1](core_architecture.md) §5–§8; the use from C, in [Vol. V ch. 1](../vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md) §7.6.

## 1. States

| state | means |
|---|---|
| **S8 exact** | measured on the SYSTEM-8 master with the probe (`re/dspnative.py` or `re/dspexact.py`), bit-identical to the emulator |
| **S1 tested** | ran in a SYSTEM-1 PLUG-OUT without hanging it and sounded like the emulator |
| **emulator** | the C reference (`emu/dspdec.h`, `emu/dspcore.h`, `emu/dspjit.py`) models it like the device |
| **s1cc** | intrinsic (`s1dsp.h`) and / or automatic use in `lower.cpp` / `kernel.cpp` |
| **host** | used by `examples/system_s1cc/make_s1cc.py` (LFO, ADSR, COLOR MOD, mixer) |

## 2. State per op

| op | what it does | S8 exact | emulator | s1cc | host | S1 tested |
|---|---|---|---|---|---|---|
| `shl` / `shr` / `shrn` | X·2^k, two's complement with 32 fraction bits (toward −∞ at 2^−32, wraps at ±512), then RAW truncates | yes (2026-10-29: 15 360 cases) | yes | automatic (products by powers of 2 > 256), `s1_ldexp` | no | **yes** |
| `floor` / `ceil` (SFU) | floor / ceiling | yes | yes | `floorf`, `s1_floor`, `s1_ceil` | no | **yes** |
| `sign` (SFU) | −1 / 0 / +1 | yes | yes | `s1_sign` | no | **yes** |
| `recipseed` + `newton` | Goldschmidt 1/x: 4 ops, 10⁻⁸; `recipseed` overwrites rY with m | yes | yes | division `x / y`, `s1_recip` | no | **yes** |
| `expsplit` (SFU) | e^⌊x⌋ from a ROM of its own + frac | yes (ROM read) | yes | `exp2` with a range > ±8 octaves, `s1_exp2`, `s1_exp` | no | **yes** |
| `log2split` (SFU) | x = m·2^e, m ∈ [0.75, 1.5); Y < 0 in two's complement (e from Y + 512) | yes (2026-10-29, Y < 0 over the whole range) | yes | `s1_log2` | no | **yes** |
| `add3` / `sum8` (A) | D + X + Y / X + Y + R | yes | yes | `s1_sum3` | no | **yes** (`sum8`) |
| MAC, coefficient in a register | X·Y + C·Z (+ W), /2 only with P2 | yes | yes | `s1_mac2` (needs 5–6 registers) | no (4 registers per voice) | **yes** |
| `ph_wrap` / `ph_clamp` / `ph_tri` | phase, clipping, triangle | yes | yes | `s1_wrap`, `s1_clamp`, `s1_tri` | yes | **yes** |
| `min` / `max` | | yes | yes | `fminf`, `fmaxf`, comparisons | yes | **yes** |
| `absadd` / `subabs` | \|X\| + Y, Y − \|X\| | yes | yes | automatic `absadd` with an immediate (the polyBLEP's \|u\| − 2) | no | **yes** (`absadd` with an immediate) |
| `minn` / `maxn` | min(−X, Y), max(−X, Y) | yes | yes | **no** | no | no |
| `frexp` | m ∈ [0.5, 1), exponent | yes | yes | **no** | no | no |
| `atanhsplit` (SFU) | split for log by atanh | yes | yes | no (covered by `log2split`) | no | no |
| `sqrtsplit` (SFU) | m = \|Y\|·4^k ∈ [1, 4) in rY, 1/√m seed (9 bits) in r(d+1), ±2^−k in r(d+3) | yes (2026-10-29) | yes | `s1_sqrt`, `s1_rsqrt` (+ 2 Newton steps) | no | **yes** (+ Newton) |
| MAC with an immediate coefficient | X·Y + K·Z (+ W); K at byte C (+8 with bit 9) | yes (2026-10-29, 20 forms; D8 closed) | yes | automatic (`mac_p1wc3` / `mac_p1p2c3`) | — | **yes** |
| two MACs in one instruction (class 0x080) | two one-product MACs, in parallel; with P2 none writes | yes (2026-10-29) | yes | no | no | no |
| flags + condition (`X9bY`, `cond`) | skip units according to the result of another | in progress 2026-10-29: Roland instructions with conditions on the master, exact except classes 0x590 / 0x190 (template) | yes | no | no | no |
| jumps and loops (`009Y`) | | yes (S8, slaves by audio) | yes | no | no | no |
| indexed loads (`X28Y` bank 0; `da`/`de`, `db`/`df` bank 5; `hset` h4–h7) | address = ⌊h·16384⌋ + Y | yes (master exact 2026-10-29, `dspverify --programs`) | yes | `s1_ldb5` (bank 5) | **yes**: COLOR MOD (`CMOD_IDX`); bank 5 in `LFO_LUT` / `ENV_CURVE` | **yes** (`hset` + `X28Y`, MP-CSAW2-8); bank 5 untested (MP-CSAW2-LUT) |
| delay line (`tapreq`, `f5c`, `f6Y`) | bank-2 taps | yes (master, DSP4 `f7dc`) | yes | no | no | no |
| parameter ramps (`FCE4`, ARM) | the ARM ramps a DSP parameter | — | no | no | no | **measured 2026-10-29**: linear, one step every 1/3 ms, 4–97 ms from 0 to 1; chainable through the end-of-ramp interrupt (`FE3C`; measured 2026-10-30: 6225 calls, one every 4.00 ms) ([examples/system_1_ramp/README.md](../../examples/system_1_ramp/README.md)) |

The ops marked tested ran on the SYSTEM-1 each alone in its instruction (NATIVE-CHECK / NATIVE-CHECK2, §5); the
groups of ops that may share an instruction are those of `asm/proven_forms.txt` ([Vol. III ch. 6](../vol3_voice_allocation_scheduling/microcode_optimization.md)).

## 3. Open, in order of contribution

Every op follows the same path: (1) measure on the SYSTEM-8 if it is not exact (`re/dspnative.py`: add the test to
`TESTS`); (2) emulator equal to the device; (3) intrinsic in s1cc and automatic use where it shortens; (4) use in the
host (`make_s1cc.py`); (5) test on the SYSTEM-1 (a PLUG-OUT like NATIVE-CHECK, or `TRY_FORMS` for groups of ops) and
only then mark it tested here and in `asm/proven_forms.txt`.

1. **Indexed loads from s1cc.** Bank 0 (`X28Y`, `ldidx`) is used by the host (COLOR MOD of MP-CSAW2-8, tested) but has
   no intrinsic; bank 5 has `s1_ldb5`. Uses: tables read by phase (wavetables, `const` tables with a computed index,
   today islands of polynomials), per-voice delay lines (chorus of CRUSHER, flanger) with `X28Y` relative to the ring.
   The bank-5 LUTs of the host (`LFO_LUT`, `ENV_CURVE`, MP-CSAW2-LUT) are built and not tested on the device.
2. **Flags, conditions and jumps on the SYSTEM-1.** Selections (`a < b ? x : y`) without arithmetic; control-rate
   code (LFO, colours, envelopes every N samples with a counter and a jump). Tested on the S8 slaves by audio; on the
   S1 not yet. First a test PLUG-OUT with a jump that skips half of the voices.
3. **Two MACs per instruction and the MAC /2** (measured on the S8) in s1cc and on the S1.
4. **`minn` / `maxn` automatic** (the clamps `max(min(w, dt), −dt)` of `mp_saw_bl`, the triangles `1 − 2·|x|`), and
   `subabs`.
5. **`frexp` (`s1_frexp`) and `atanhsplit`** in s1cc.
6. **Delay taps (`tapreq`) from s1cc** (the EDM lines run from the host on master B,
   [Vol. VI ch. 7](../vol6_reference_designs_applications/mutable_memory.md)).
7. Confirm by ear `MP-CSAW2` with 7 voices and `MOD_DSP=B` (installed 2026-10-28).

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

## 5. Measurements and results (dated)

* **2026-10-30: ramp chain on the device.** C-S1RAMP with LFO SHAPE ≥ 3: `FE3C` registered our end-of-ramp function,
  which was called 6225 times, once every 4.00 ms, each time launching the next ramp. No hangs; `FE40` releases it.
  Envelopes and LFO on the ARM without a tick: possible ([Vol. III ch. 8](../vol3_voice_allocation_scheduling/s1_optimizations.md) §3).
* **2026-10-29, night (2): ramps and LUT.** `FCE4` measured on the SYSTEM-1 (C-S1RAMP): linear, one step every
  1/3 ms, 4–97 ms from 0 to 1 ([examples/system_1_ramp/README.md](../../examples/system_1_ramp/README.md)). In the OS:
  `FE3C(sel, id, fn)` registers an own end-of-ramp function and the interrupt calls it at every end, without
  releasing it: ramps can be chained without a tick (run on 2026-10-30, above). Bank-5 indexed loads
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

* **2026-10-29.** Measured on the S8 master and modelled in the C reference (`emu/dspref.py`, [Vol. V ch. 5](../vol5_toolchain_compiler_sdk/emulator.md)
  §1.1): MAC with an immediate coefficient (20 forms, rule of
  byte C + 8·bit 9 in 19 classes: D8 closed; `asm/asm.py` and `asm/disasm.py` use it), two MACs per instruction, /2
  only with P2, rounding of `shl`/`shr` (two's complement with 32 fraction bits), `log2split` with Y < 0 over the
  whole range, complete `sqrtsplit` (9-bit Q table, output r(d+3)). Full battery of `dspnative.py`: all tests 0
  differences.

* **2026-10-28.** Exact measurements on the S8 master: `shl`/`shr`/`shrn`, `floor`, `ceil`, `sign`, `recipseed`,
  `expsplit`, `log2split`, `atanhsplit`, `frexp`, `newton`, `add3`, `sum8`, register-coefficient MAC (+, −, /2), 24–96
  cases per op, with and without an empty instruction before reading; the emulator follows them
  ([Vol. V ch. 5](../vol5_toolchain_compiler_sdk/emulator.md) §3.7). The s1cc division does not read the input after
  `recipseed`, which overwrites it. Intrinsics and automatic use in s1cc; `asm/isa.json` (SFU forms, `floor`, `shl` with a number, measured
  MACs), `asm/dspsched.py` (`_native`). `MOD_DSP=auto` in SYSTEM-S1CC: no kernel of mutable_ported lost voices, 8
  gained. NATIVE-CHECK and BRAIDS-CSAW built (NATIVE-CHECK ran on 2026-10-29, above).

> **Note on paths:** in this chapter `emu/…`, `re/…` are relative to `examples/from_scratch/`.
