# Reloj del ARM del esclavo del SYSTEM-1 (tick de 1 ms) y escritura de parámetros del DSP

Un PLUG-OUT del SYSTEM-1 puede correr código propio en el ARM del esclavo **cada 1 ms**, sin tarea propia ni
interrupción propia, y desde ahí escribir parámetros del DSP que el programa del DSP lee en la muestra siguiente.
Es la base de las envolventes y el LFO en el ARM de SYSTEM-S1CC (`ARM_ENV`, `ARM_LFO`,
[examples_system_s1cc_README.md](examples_system_s1cc_README.md) §3.7). Este documento describe el mecanismo para
usarlo en otras cosas (secuenciadores, modulaciones, streaming de tablas, medición).

## 1. El tick [código; medido 2026-11-03]

| elemento | valor |
|---|---|
| origen | el "cyclic handler" del RTOS del esclavo (`driver\cyclic_handle.c`, handler `0x01008983`, período "1") |
| cliente | un puntero a función en RAM, `0x20002BFC`: el handler llama `(*0x20002BFC)(i)` en cada período |
| período medido | **1,00 ms** (telemetría: un contador por llamada, diente de sierra de 64 llamadas = 64,0 ms, autocorrelación 0,99) |
| contexto | el del handler del OS (interrupción / tarea del kernel): corto, sin bloquear, sin SVC lentos |
| reloj de pared | DWT `CYCCNT` (se habilita con `DEMCR` bit 24 y `DWT_CTRL` bit 0); ciclos por ms = `SysTick LOAD + 1` (`0xE000E014`) |

### 1.1 Engancharse

```c
typedef void (*cyc_fn)(int i);
#define CYC_CLIENT (*(volatile cyc_fn *)0x20002BFCu)
static cyc_fn cyc_os;                       /* la función del OS: se sigue llamando primero */

static void tick(int i)
{
    if (cyc_os)
        cyc_os(i);
    if (CYC_CLIENT != tick) {               /* el OS se reemplazó a sí mismo: guardar la nueva y volver a entrar */
        cyc_os = CYC_CLIENT;
        CYC_CLIENT = tick;
    }
    /* trabajo propio, cada 1 ms */
}

/* load (vt+0x08): */  if (CYC_CLIENT != tick) { cyc_os = CYC_CLIENT; CYC_CLIENT = tick; }
/* deinit (vt+0x04): */ if (CYC_CLIENT == tick) CYC_CLIENT = cyc_os;
```

* **Siempre llamar primero a la función del OS** y soltar el gancho en `deinit`: al salir del PLUG-OUT el código de
  la región 1 deja de ser válido.
* **Medir el tiempo, no contar llamadas.** `engine.c tick` acumula `CYCCNT` y avanza el estado en dieciseisavos de
  un paso (`seg_cycles >> 4`); una llamada atrasada se compensa en la siguiente. Una pausa larga (carga de programas)
  se descarta.

## 2. Escribir un parámetro del DSP desde el tick [medido 2026-11-03]

| servicio | qué hace | resultado en el DSP B del esclavo |
|---|---|---|
| `FCEC(sel, id, bits)` (`SVC 0xEC`), inmediato | `[+0] = 0xE0000000, [+4] = [+8] = valor, [+0xC] = 0` en el bloque `0x4016C000 + 16·id` | **`C080`–`C09F`: llega exacto**, 1000 escrituras por segundo sin perder ninguna (diente de sierra de 64 pasos, lineal de punta a punta) |
| `FCEC` a `C100`… | lo mismo | **no sirve**: el DSP lee un valor casi fijo y el nuestro solo aparece unos ms después de un cambio grande |
| `FCE4(sel, id, destino, y, z)` (`SVC 0xE4`), rampa del hardware | rampa lineal de 4–97 ms ([examples_system_1_ramp_README.md](examples_system_1_ramp_README.md)) | **a `C080`…: no llega** (el parámetro no se mueve); a `C0C0`… tampoco (2026-10-30). Medida solo en el maestro (`C030`) |

Uso recomendado: **un `FCEC` por parámetro por tick, en `C080`–`C09F`**. La curva queda en escalones de 1 ms
(1000 puntos por segundo). Los parámetros `C040`–`C07F` del DSP B también los escribe el ARM sin problema (las
perillas del anfitrión y del kernel, `b6`), pero solo cuando cambia una perilla.

## 3. Cómo se midió: telemetría por audio

El ARM del esclavo no tiene salida al PC (no manda MIDI). La telemetría usa el audio:

1. Un kernel sonda (`sdk/s1cc/examples/arm_probe/arm_probe.c`) da tres tonos fijos en el DSP B: 1000 Hz × `s1_env(0)`,
   2200 Hz × `s1_env(1)`, 4100 Hz × (0,5 + `s1_lfo_filter()`/8).
2. Con `ARM_PROBE=1` (`make_s1cc.py`), el ARM escribe en los parámetros de envolvente de la voz 7: `(llamadas de
   tick mod 64)/64` y `(pasos de envolvente mod 64)/64`. Con la voz 7 sin tocar, esos tonos llevan la telemetría.
3. Captura USB (`arecord -D hw:SYSTEM1,0 -f S32_LE -r 96000 -c 2`) y demodulación de cada tono (mezcla con la
   portadora + paso bajo de 250–500 Hz).

| captura (`work/captures/`) | parámetros | resultado |
|---|---|---|
| `arm_probe_tele_*.wav` | `C107` / `C10F`, `FCEC` | tick 64,0 ms, pasos 256 ms; el DSP ve casi un valor fijo |
| `arm_probe_c080_*.wav` | `C087` / `C08F`, `FCEC` | diente de sierra limpio 0…1 en los dos |
| `arm_probe_fcec_*.wav` | `C080`…, envolventes y LFO con `FCEC` cada 1 ms | ADSR y LFO completos y suaves, release hasta 0 |

## 4. Abierto

* Qué escribe en `C100`… (o a qué alias apunta ese bloque) y por qué `FCE4` no mueve `C080`… en el esclavo.
* El tiempo que deja el tick: cuántas escrituras `FCEC` caben por ms antes de molestar al OS (probadas: ~33 por ms,
  8 voces × 2 envolventes + LFO + fade).
* El mismo gancho en el maestro.
