# Arquitectura del SYSTEM-8: hardware, qué código corre dónde, y nuestras herramientas

Resumen para orientarse. El detalle y las pruebas están en `BASE_FIRMWARE.md` §8 (anatomía del OS),
`CUSTOM_PLUGOUTS.md` (interfaces de PLUG-OUT, microcódigo) y `examples/from_scratch/NOTES.md`
(PLUG-OUT propio, ISA del DSP). Estado: 2026-10-15.

## 1. Componentes de hardware

```
                      USB (al PC): MIDI 1 / MIDI 2, audio USB 96 kHz, instalación de PLUG-OUTs
                                │
 ┌──────────────────────────────┴───────────────────────────────┐
 │ BMC #0 "Master"  (chip propio de Roland)                      │
 │  ARM core 0 ── el OS: μT-Kernel, panel, menús, MIDI, USB,    │
 │                SD, patches, secuenciador, gestión de PLUG-OUT │
 │                + REGIÓN 0 del PLUG-OUT (el "modelo")          │
 │  ARM core 1 ── control de DSP + efectos globales del SYSTEM-8 │
 │  DSP0..DSP5 + DSPM ── reverb, delays, chorus, MFX, master     │
 │  SDRAM 0x60000000 (OS en ejecución), QSPI flash, SD, DAC/amp  │
 └──────┬──────────────────────────────┬────────────────────────┘
        │ USB host interno "BMCBulk"    │ UART
        │ (imágenes de arranque,        │
        │  parámetros, notas)           │
 ┌──────┴─────────────────────┐   ┌─────┴──────────────────────┐
 │ BMC #1 "Slave0" (voces 0-3) │   │ STM32F103 "Kbd" (panel)    │
 │ BMC #2 "Slave1" (voces 4-7) │   │  teclas, perillas, sliders,│
 │  ARM: s_boot / s_data       │   │  LEDs, teclado             │
 │   + REGIÓN 1 del PLUG-OUT   │   └────────────────────────────┘
 │     (el "engine")           │
 │  DSP0..DSP5 + DSPM: VOCES   │
 │  RAM 0x60000000-0x61000000  │
 └─────────────────────────────┘
```

* **BMC**: chip de Roland con núcleos ARM Cortex-M, varios DSP propios (6 + DSPM), DMA, USB host y
  controlador de memoria. Hay **tres**: un maestro y dos slaves, iguales en hardware, con programas distintos.
* **DSP**: procesador de audio propio de Roland, con su propio ISA (microcódigo de 16 bits, VLIW, números en
  formato RAW de signo y magnitud con exponente). Ejecuta su programa entero una vez por muestra. **No es ARM.**
  El ARM de su BMC le carga programas y parámetros escribiendo en su memoria (§4).
* **STM32F1**: microcontrolador del panel. El OS del maestro lo reprograma por UART si hace falta.
* Audio: las voces salen de los DSP de los slaves. Los efectos y la sección master están en los DSP del
  maestro, y de ahí al DAC (WM2418) y al audio USB. Por qué enlace pasa el audio de los slaves al maestro
  no lo investigamos.

## 2. Qué código corre en cada procesador

| Procesador | Código | Dónde vive / de dónde viene |
|---|---|---|
| Maestro, ARM core 0 | **OS** (`m0_core0`): μT-Kernel, UI, MIDI (router `0x6000345C`), USB, SD, instalación y carga de PLUG-OUTs, kernel de parámetros `0x01000400` | en sitio en `0x60000000`; kernel en `0x01000000` |
| Maestro, ARM core 0 | **Región 0 del PLUG-OUT** ("modelo": lista de parámetros, asignador de notas, memoria del slot). Nuestro: `examples/from_scratch/model/model.c` | `0x61600000 + 0x200000·slot` (slot 3 = `0x61C00000`) |
| Maestro, ARM core 1 | `m1_tcm` + `m1_sdram`: control de los DSP del maestro, efectos globales (`CDSPSystem8Rev`, `…Dly…`, `…Mst`), DAC/amp | `0x01100000` (TCM del core 1) y `0x61000000` |
| Maestro, DSP0..5 | microcódigo de los efectos globales (p. ej. DSP4 ranura `0xEFDC`) | lo carga el core 1 |
| Slave0 / Slave1, ARM | `s_boot` + `s_data`: control de DSP, recepción de parámetros (`UpdateSynthParam` `0x01103F2C`), servicios `0x0113FC00` para el engine | enviados por el maestro al arrancar: `s_boot` → `0x01100000`, `s_data` → `0x60000000` **del slave** |
| Slave0 / Slave1, ARM | **Región 1 del PLUG-OUT** ("engine": del parámetro a los valores del DSP, carga de microcódigo). Nuestro: `examples/from_scratch/engine/engine.c` | `0x60800000 + 0x200000·slot` **del slave** (slot 3 = `0x60E00000`) |
| Slave, DSP0..5 | microcódigo de las voces que trae el engine: DSP0/1 envolventes y LFO, DSP2/3 osciladores (ranuras DCO1 `0x6000`, DCO2 `0xB000`), DSP4/5 filtro y VCA (DSP4 ranura `0xB000` = efecto del PLUG-OUT) | lo copia el ARM del slave a la memoria de programa |
| STM32F1 | `subcpu` (panel) | `0x08000000` |

**Trampa frecuente: la misma dirección significa cosas distintas en cada chip.** `0x60000000` es el OS en el
maestro y `s_data` en un slave; `0x01100000` es `m1_tcm` en el core 1 del maestro y `s_boot` en un slave.
Por eso el engine (región 1) **no puede llamar al OS del maestro**: salta a otro código y se cuelga. En
cambio sí puede llamar a los servicios del slave (`0x0113FC00`). Cada slave corre su propia copia del engine.

## 3. Cómo se conectan (flujos)

| Flujo | Camino |
|---|---|
| Instalación de un PLUG-OUT | PC → USB MIDI 2 → OS del maestro → QSPI. Al cargar: región 1 → USB interno → RAM de cada slave; región 0 → SDRAM del maestro; luego `init` (SYNINFO `+0x24`) |
| Nota MIDI o tecla | OS del maestro → asignador de la región 0 (`note_on`) → `set(parte, grupo, id, valor)` del kernel (ids de voz `0x429+v`, `0x43A+v`) → el maestro manda rangos de ids modificados → el slave llama a `vt+0x30(id, valor)` del engine → el engine escribe parámetros del DSP (`+0xA0`/`+0xA4`/`+0xB0`) |
| Perilla del panel | STM32 → OS → kernel de parámetros (ids de patch `0x6CD…`) → slave → engine `vt+0x30` |
| Microcódigo de las voces | engine `vt+8`: registra programas (`+0x28/+0x40/+0x58`); el slave los copia a la memoria de programa de sus DSP; `+0x70` cambia el programa de una ranura en marcha |
| Audio | DSP de los slaves (voces) → maestro (efectos, master) → DAC / audio USB (`arecord -D hw:SYSTEM8,0`) |
| MIDI hacia el PC | sólo desde el maestro: región 0 u hooks llamando al router del OS. Los slaves **no** tienen salida |

## 4. Mapa de memoria de un DSP (igual en maestro y slaves, cada uno ve los suyos)

| Dirección (DSP n = 0..5) | Contenido | ARM puede |
|---|---|---|
| `0x41400000 + n·0x100000` (+`0x2000` SD31, +`0x4000` RAW) | banco 0 (DT0): RAM de datos del programa (refs `02nn`/`04nn`/`06nn`) | leer (escribir no queda) |
| `0x41480000 + n·0x100000` | **memoria de programa** (+ dirección de ranura en bytes) | leer y escribir (stores de 32 bits) |
| `0x41104318 + n·0x1000` | control y bit de tarea de cada ranura | escribir |
| `0x41114000 + n·0x4000` | banco 5 / DPC (refs `A0nn`, `+0xB0`) | leer y escribir |
| `0x41400000 + n·0x100000 + 0xA0000` | bloque de parámetros con rampa (refs `C0nn`, `+0xA0`) | escribir vía OS |

## 5. Nuestras herramientas y qué tocan

| Herramienta | Corre en | Qué toca | Para qué |
|---|---|---|---|
| `tools/plugout_tool.py install` | PC | OS del maestro (USB MIDI 2) | instalar PLUG-OUTs |
| `examples/midi_send/bulk_dump.py` / `bulk_loop.py` | hook en el `init` de la región 0 (maestro core 0) | lee memoria **del maestro** y la manda por SysEx (una instalación por volcado) | volcados del OS, flash, DSP del maestro |
| `re/dspaudio.py` (`examples/from_scratch`) | PC + engine `-DDSPTEST` en los **slaves** | programas de prueba en la ranura DCO2 de DSP2/DSP3 de los slaves; medición por audio USB | ISA del DSP por audio (una instalación por lote) |
| `re/dsplive.py` | PC + engine `-DDSPLIVE` en los **slaves** | escribe en vivo la memoria de programa de DSP2/DSP3 de los slaves (pitch bend + CC1 → ids `0x465`/`0x467` → engine) | ISA por audio sin reinstalar |
| `re/dsppeek.py` | PC + región 0 `-DPEEK` en el **maestro** (core 0) | lee y escribe memoria **del maestro** en vivo, incluidos sus DSP; responde por SysEx | valores exactos de registros (DSP del maestro) |

| `re/dspmaster.py`, `re/dspprobe.py`, `re/dspfit.py`, `re/dspclass.py`, `re/dspslots.py`, `re/dspcount.py` | PC + región 0 `-DPEEK` | programas de prueba en la ranura `0xEFDC` del **DSP4 del maestro** (una tarea de control, ~4 000 ejecuciones/s, comparte el DSP con otras), resultados leídos de DT0 | ISA exacto (`examples/from_scratch/DSP_ISA.md`) |
| `re/dspisa.py` | PC | nada (decodificador / desensamblador / simulador con `re/dsp_templates.txt`) | leer el microcódigo de Roland |

La distinción importante: **dsplive** actúa en los DSP de los **slaves** (voces, se observan por audio) y
**dsppeek** en los del **maestro** (efectos, se leen directamente). No hay forma conocida de leer la
memoria de un slave desde el PC.
