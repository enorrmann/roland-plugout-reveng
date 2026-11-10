# Volume I — Chapter 6: SYSTEM-1 Slave ARM Clock (1 ms tick) and Writing DSP Parameters

> **Scope:** SYSTEM-1.

A SYSTEM-1 PLUG-OUT can run its own code on the slave's ARM **every 1 ms**, without its own task or its own interrupt,
and from there write DSP parameters that the DSP program reads in the next sample. It is the basis of the envelopes and
the LFO on the ARM of SYSTEM-S1CC (`ARM_ENV`, `ARM_LFO`, [examples/system_s1cc/README.md](../../examples/system_s1cc/README.md)
§3.7). This document describes the mechanism so that it can be used for other things (sequencers, modulations,
streaming of tables, measurement).

## 1. The tick [code; measured 2026-11-03]

| element | value |
|---|---|
| origin | the "cyclic handler" of the slave's RTOS (`driver\cyclic_handle.c`, handler `0x01008983`, period "1") |
| client | a function pointer in RAM, `0x20002BFC`: the handler calls `(*0x20002BFC)(i)` every period |
| measured period | **1.00 ms** (telemetry: one counter per call, a sawtooth of 64 calls = 64.0 ms, autocorrelation 0.99) |
| context | that of the OS handler (interrupt / kernel task): short, non-blocking, no slow SVCs |
| wall clock | DWT `CYCCNT` (enabled with `DEMCR` bit 24 and `DWT_CTRL` bit 0); cycles per ms = `SysTick LOAD + 1` (`0xE000E014`) |

### 1.1 Hooking in

```c
typedef void (*cyc_fn)(int i);
#define CYC_CLIENT (*(volatile cyc_fn *)0x20002BFCu)
static cyc_fn cyc_os;                       /* the OS's function: it is still called first */

static void tick(int i)
{
    if (cyc_os)
        cyc_os(i);
    if (CYC_CLIENT != tick) {               /* the OS replaced itself: keep the new one and hook in again */
        cyc_os = CYC_CLIENT;
        CYC_CLIENT = tick;
    }
    /* own work, every 1 ms */
}

/* load (vt+0x08): */  if (CYC_CLIENT != tick) { cyc_os = CYC_CLIENT; CYC_CLIENT = tick; }
/* deinit (vt+0x04): */ if (CYC_CLIENT == tick) CYC_CLIENT = cyc_os;
```

* **Always call the OS's function first** and release the hook in `deinit`: when leaving the PLUG-OUT the code of
  region 1 stops being valid.
* **Measure time, do not count calls.** `engine.c tick` accumulates `CYCCNT` and advances the state in sixteenths of a
  step (`seg_cycles >> 4`); a late call is compensated in the next one. A long pause (loading programs) is discarded.

## 2. Writing a DSP parameter from the tick [measured 2026-11-03]

| service | what it does | result on the slave's DSP B |
|---|---|---|
| `FCEC(sel, id, bits)` (`SVC 0xEC`), immediate | `[+0] = 0xE0000000, [+4] = [+8] = value, [+0xC] = 0` in the block `0x4016C000 + 16·id` | **`C060`–`C0FF`: arrives exactly**, 1000 writes per second without losing any |
| `FCE4(sel, id, target, y, z)` (`SVC 0xE4`), hardware ramp | linear ramp of 4–97 ms, steps of 1/3 ms ([examples/system_1_ramp/README.md](../../examples/system_1_ramp/README.md)) | **only ids < `0x80`**: `C061`–`C07F` ramps up to the target; `C080`–`C0FF` does not move |
| any write to `C100`… | — | **they are not parameters**: `FCEC` to `C107` is barely seen, and writing `C100`–`C13F` (224 values of a table at load time) left the slave silent until the PLUG-OUT was changed |

Sweep of ids (`ramp_probe.c`, `ARM_RAMPTEST=1`, capture `work/captures/ramp_probe4_*.wav`): a heartbeat on `C060` every
128 ms proves that the ARM is still alive; each slot writes 0.5 with `FCEC`, launches `FCE4` to 1.0 (`y = 5`) and goes
back to 0. The heartbeat did not stop in 22 s, and the ramp arrived exactly in `C061`–`C07F`.

Two ways of using the tick:

| | `FCEC` every 1 ms (`ARM_RAMP=0`) | `FCE4` every 4 ms (`ARM_RAMP=1`) |
|---|---|---|
| resolution | 1 ms, steps | 4 ms lines (1/3 ms hardware steps) |
| delay on a change | ≤ 1 ms | ~4 ms |
| ids | `C060`–`C0FF` | only `< 0x80` |
| ARM writes | 1 per parameter per ms | 1 per parameter every 4 ms |

## 3. How it was measured: telemetry through audio

The slave's ARM has no output to the PC (it does not send MIDI). The telemetry uses the audio:

1. A probe kernel (`sdk/s1cc/examples/arm_probe/arm_probe.c`) gives three fixed tones on DSP B: 1000 Hz × `s1_env(0)`,
   2200 Hz × `s1_env(1)`, 4100 Hz × (0.5 + `s1_lfo_filter()`/8).
2. With `ARM_PROBE=1` (`make_s1cc.py`), the ARM writes into the envelope parameters of voice 7: `(tick calls mod 64)/64`
   and `(envelope steps mod 64)/64`. With voice 7 untouched, those tones carry the telemetry.
3. USB capture (`arecord -D hw:SYSTEM1,0 -f S32_LE -r 96000 -c 2`) and demodulation of each tone (mix with the carrier
   + low-pass of 250–500 Hz).

| capture (`work/captures/`) | parameters | result |
|---|---|---|
| `arm_probe_tele_*.wav` | `C107` / `C10F`, `FCEC` | tick 64.0 ms, steps 256 ms; the DSP sees an almost fixed value |
| `arm_probe_c080_*.wav` | `C087` / `C08F`, `FCEC` | clean sawtooth 0…1 in both |
| `arm_probe_fcec_*.wav` | `C080`…, envelopes and LFO with `FCEC` every 1 ms | complete and smooth ADSR and LFO, release down to 0 |
| `ramp_probe4_*.wav` | sweep `C061`–`C0FF`, `FCEC` + `FCE4` | ramp only in `C061`–`C07F` |
| `arm_probe_ramp_*.wav` | `C060`…, envelopes and LFO with `FCE4` every 4 ms | complete ADSR and LFO, release down to 0 |

## 4. Open

* What is at `0x4016C000 + 16·id` for `id ≥ 0x100` and why the ramp only exists for `id < 0x80`.
* Whether the ramps work in `C000`–`C05F` (not measured: the OS and the host use part of that zone).
* The time the tick leaves: how many `FCEC` writes fit per ms before disturbing the OS (tested: ~33 per ms, 8 voices ×
  2 envelopes + LFO + fade).
* The same hook on the master.
