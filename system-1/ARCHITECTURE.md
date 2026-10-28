# Arquitectura del SYSTEM-1: hardware, qué código corre dónde, y nuestras herramientas

Es el equivalente de `docs/ARCHITECTURE.md` (SYSTEM-8). Estado: 2026-10-22. Fuentes: OS 1.30 (`FIRMWARE.md`), los
PLUG-OUTs SH-2/SH-101/PROMARS para SYSTEM-1 (sólo leídos) y lo medido con C-S1V4 (`docs/examples_system_1_README.md`).
Detalle: voces en [VOICE_ALLOCATION.md](VOICE_ALLOCATION.md), audio en [WAVE_BUS.md](WAVE_BUS.md), memoria de
programa en [../docs/DSP_PROGRAM_MEMORY.md](../docs/DSP_PROGRAM_MEMORY.md).

Leyenda: **[medido]** · **[código]** · **[inferido]** · **[abierto]**.

## 1. Componentes de hardware

```
                 USB (al PC): MIDI 1 (hw:N,0,0) / MIDI 2 (hw:N,0,1), instalación de PLUG-OUTs
                                │
 ┌──────────────────────────────┴───────────────────────────────┐
 │ BMC "maestro"                                                 │
 │  ARM (Cortex-M, sin FPU) ── el OS: panel, MIDI, USB, patches, │
 │      gestión de PLUG-OUT + REGIÓN 0 del PLUG-OUT ("modelo")   │
 │  DSP A, DSP B ── modulación por voz, mezcla, salida (+efectos)│
 └──────┬────────────────────────────────────────────────────────┘
        │ imagen del esclavo, parámetros, notas; bus de onda (IOMD 6006–6021)
 ┌──────┴─────────────────────────────┐
 │ BMC "esclavo" (las 4 voces)        │
 │  ARM (sin FPU): OS del esclavo     │
 │   + REGIÓN 1 del PLUG-OUT ("motor")│
 │  DSP A, DSP B: VOCES               │
 └────────────────────────────────────┘
```

| | SYSTEM-8 | SYSTEM-1 |
|---|---|---|
| BMC | maestro + 2 slaves | **maestro + 1 esclavo** (una imagen de esclavo, una región 1) [código] |
| DSP por BMC | 6 + DSPM | **2 (A, B)** (`FC30`/`FC34`; accesos con `sel` 0/1) [código] |
| ARM | Cortex-M con VFP (core 0/1 en el maestro) | **Cortex-M sin FPU**: ningún binario usa VFP [código]; un núcleo en el maestro [inferido: un solo programa de aplicación] |
| voces | 8 | 4 |
| PLUG-OUT slots | 3 | 1 |
| muestreo | 96 kHz | 96 kHz [inferido: la sierra calibrada a 96 kHz suena afinada, a oído] |
| panel | STM32F103 por UART | [abierto]: el actualizador no trae imagen de otro micro (`FIRMWARE.md` §2.1) |

### 1.0 Capacidad de los DSP del esclavo (lo que entra en un PLUG-OUT)

| recurso | DSP A | DSP B | certeza / detalle |
|---|---|---|---|
| tiempo por muestra (96 kHz) | **703** instrucciones corrieron, 826 colgaron | **708** corrieron (sin límite buscado) | [medido], `docs/examples_system_1_synth_README.md` §4. 1 ciclo por instrucción, cualquiera sea su contenido (medido en el SYSTEM-8, `docs/DSP_HARDWARE.md` §1) |
| programa principal | **6 KiB** (`0x0100`–`0x18FF`); pasarse cuelga | igual | [medido 2026-10-28], `docs/DSP_PROGRAM_MEMORY.md` §3 |
| memoria de programa total | 32 KiB: principal + secciones 2–5 (~6 KiB c/u) + cierre. Una sección propia se coloca con `FC40` (con `FC30` se copia pero no corre) y se enciende con `FC44(i, 1)`; corre después del principal y suma a su tiempo | igual | [medido 2026-10-28], `docs/DSP_PROGRAM_MEMORY.md` §3.2 |
| tamaño de una instrucción | 2–62 bytes; ≈ 8–10 en los nuestros: 6 KiB son ≈ 600–750 instrucciones | | [medido] |
| ops por instrucción | hasta 3 (sólo los grupos de `asm/proven_forms.txt`) y 3 transferencias, con las reglas R2–R8 | | [medido], `asm/s1rules.py`, `docs/MICROCODE_OPTIMIZATION.md` |
| registros | r0–r7 compartidos por todos los programas del DSP; dos mitades r0–r3 / r4–r7 (dos voces de a pares, `docs/examples_system_s1cc_README.md` §4.2) | | [medido] |
| memoria de datos | banco 0 (anillo, 0x800 palabras), 1, 5 (0x400, el que escriben el ARM y los DSP: `A100`–`A1FF` para los DSP), 6 (parámetros, sólo lectura) | | [medido], `system-1/DSP_BANKS.md`, `docs/DSP_HARDWARE.md` §7 |

El build de SYSTEM-S1CC aplica estos límites (`make_s1cc.py`: `BUDGET_A` / `BUDGET_B` / `MAIN_BYTES`) y elige solo
cuántas voces entran.

### 1.1 Coma flotante: el SYSTEM-8 tiene FPU, el SYSTEM-1 no [código]

Los ARM de los BMC del SYSTEM-8 tienen unidad de coma flotante (VFP, simple precisión) y todo su código la usa; los
del SYSTEM-1 no la usan en ningún binario. Recuento de instrucciones VFP (`python3 system-1/vfp_count.py`; los datos
dan por azar ≈ 1 por 10 KiB, que es el ruido):

| imagen | instrucciones VFP | por 10 KiB |
|---|---|---|
| SYSTEM-8, OS (maestro + slaves, `dumps/os_full.bin`) | 3054 | 10,6 |
| SYSTEM-8, JX-3P región 1 (motor de los slaves) | 679 | 24,6 |
| SYSTEM-8, SH-2 región 1 | 992 | 47,5 |
| SYSTEM-8, JX-3P región 0 (maestro) | 0 | 0 (no hace cuentas en coma flotante, pero el chip tiene FPU) |
| SYSTEM-1, OS 1.30 maestro | 3 | 0,0 |
| SYSTEM-1, OS 1.30 esclavo | 24 | 0,7 |
| SYSTEM-1, SH-2 / PROMARS regiones 0 y 1 | 5–8 | 0,2–0,4 |

El chip exacto de los BMC no se conoce (son de Roland); por eso se habla de "con VFP" y "sin VFP", y no de Cortex-M4
frente a Cortex-M3. Que el SYSTEM-1 no tenga FPU es **[inferido]**: ningún código la usa, pero no se ha probado a
ejecutar una instrucción VFP en el equipo (si no hay FPU, provocaría un fallo de uso y probablemente colgaría el chip).

Consecuencias para nuestros PLUG-OUTs:

* Compilar para el SYSTEM-1 con `-mcpu=cortex-m3 -mfloat-abi=soft` (`examples/system_1/build.sh`); para el SYSTEM-8
  con `-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16` (`examples/from_scratch/engine/build.sh`).
* En el SYSTEM-1 los valores que van a los DSP siguen siendo `float` IEEE (los servicios `FD6C`, `FCEC` los reciben
  como patrones de bits en registros enteros); C-S1V4 los precalcula en el PC (`make_s1.py`). Hacer cuentas `float`
  en el ARM del SYSTEM-1 es posible con la emulación por software de gcc (`libgcc`: `__aeabi_fmul`, …), más lenta.
* El código ARM de un PLUG-OUT del SYSTEM-8 (p. ej. el motor del JX-3P) **no corre en el SYSTEM-1** tal cual:
  además de otra interfaz y otros servicios, usa VFP. Hay que portarlo o reescribirlo (`DSP_FIT.md`).
* Los DSP no se ven afectados: hacen su propia aritmética (`docs/DSP_HARDWARE.md`), igual en los dos equipos.

## 2. Qué código corre en cada procesador

| Procesador | Código | Dónde vive |
|---|---|---|
| maestro, ARM | **OS** (`"SYSTEM-1:Appli"`, `0x60000000`), código rápido en `0x01000300`, **servicios para PLUG-OUTs en `0x0100FC00`** (162 entradas) | flash `SYSTEM1_UPD.BIN` + `0x30000` |
| maestro, ARM | **región 0 del PLUG-OUT**: SYNINFO, asignador (`+0x2C`), objeto modelo (`+0x30`: carga los DSP del maestro, recibe los parámetros), 64 patches en `+0x1000` | **`0x60800000`** |
| maestro, DSP A/B | modulación por voz, mezclador, salida (`0x82` en modo PLUG-OUT); en modo PLUG-OUT el DSP B lo pone el PLUG-OUT (`0x1000`) | lo carga el OS / la región 0 |
| esclavo, ARM | OS del esclavo (`0x60000000` del esclavo), código rápido `0x01000300`, **servicios `0x0100FC00`** (las mismas direcciones que en el maestro, otro código) | flash + `0x130000`, lo manda el maestro |
| esclavo, ARM | **región 1 del PLUG-OUT**: SYNINFO, motor (`+0x30`; vtable de 22: `load`, `refresh`, `param`) | **`0x60200000`** del esclavo |
| esclavo, DSP A/B | voces: de serie principal + 4 secciones por DSP; PLUG-OUTs lo suyo (C-S1V4: todo en el principal de A, salida en B) | lo carga el ARM del esclavo |

**La misma trampa que en el SYSTEM-8**: `0x60000000` y `0x0100FC00` existen en los dos chips con contenido distinto;
la región 1 sólo puede llamar a los servicios del esclavo, y sólo la región 0 (maestro) puede mandar MIDI.

## 3. Cómo se conectan (flujos)

| flujo | camino |
|---|---|
| instalación | PC → USB MIDI 2 → OS del maestro → flash (`Code1.Dat`, 2048 registros, `70 10 00 00`). Al cargar: región 1 → esclavo, región 0 → `0x60800000`; el OS comprueba la SYNINFO (`+0x000`, `+0x1B4`) y llama a `create` (`+0x24`) en los dos [medido: C-S1V4 aceptado] |
| pasar al PLUG-OUT | maestro: `get_b()` → `vt[0]`, `get_a()` = asignador; colas del maestro vaciadas, sets de modo PLUG-OUT del OS, `model->vt[2]` (load). Esclavo: `get_b()` → `vt[0]`, colas vaciadas, secciones apagadas, `engine->vt[2]` [código, medido] |
| nota | OS → asignador `note_on` → `set(2, 0x110+v / 0x120+v)` → maestro (su DSP) y esclavo (`engine vt+0x30(id, 0, valor)`) → el motor escribe el banco 5 (`FD6C`) [medido] |
| control del panel | OS → `model vt+0x30(id, flag, valor)` en el maestro (VOLUME = `0x100`) y, para los grupos 0–3, `engine vt+0x30` en el esclavo [medido con PARAMLOG] |
| valores del panel al entrar al PLUG-OUT | el OS no los reenvía; el modelo hace `touch(3, id)` de `0x160`–`0x18F` y `flush(3)` en `load` y el OS los manda al engine (`examples/system_s1cc/model.c resend_panel`) [medido 2026-10-28: los controles del sinte son del grupo 3] |
| audio | esclavo A → B (banco 5) → IOMD `6006`–`600D` → maestro B → `A118`/`A119` → maestro A (`0x82`) → `6000`/`6001` → salida [medido] |
| modulación (sinte de serie) | maestro (programa de 696 instrucciones) → IOMD `600E`–`6021` → esclavo A [código] |
| MIDI hacia el PC | sólo desde el maestro: la región 0 puede usar el envío del OS (`0x600065B0`, objeto `0x605F20D4`, cable 3, máscara 1) → sale por MIDI 1 [medido] |

## 4. Memoria de los DSP vista desde el ARM del esclavo [código, parcial]

| dirección | qué | estado |
|---|---|---|
| `0x40101000 + 4·i` | banco 5 (`A000+i`), vista IEEE; `+0x400` = `A100+i` (`FD6C`, `sel` 1) | `A0xx` [medido con C-S1V4]; `A1xx` [inferido] |
| `0x4014C000 + 16·id` / `0x4016C000 + 16·id` | parámetros (banco 6, `C0id`) del DSP A / B (`FCEC` inmediato, `FCE4` con rampa: `mode<<28 \| …`) | [código]; `FCEC` en el maestro [medido: VOLUME] |
| `0x40140000`, `0x40160000` (+`0xE000`, +`0x10000`) | bloques de cada DSP; `+0x10000` es destino de copias por DMA en bloques de `0x200` (memoria de programa o de datos) | [abierto] |
| `0x40100000` | control común | [abierto] |

La memoria de programa (secciones, cierres) se describe en [../docs/DSP_PROGRAM_MEMORY.md](../docs/DSP_PROGRAM_MEMORY.md) §3.

## 5. Nuestras herramientas

| herramienta | corre en | qué toca | para qué |
|---|---|---|---|
| `tools/plugout_tool.py info / install` | PC | OS del maestro (MIDI 2) | instalar en el slot 1 (`--slot 1 --force --no-patches`) |
| `tools/plugout_build.py unpack / pack / show` | PC | `Code1.Dat` | desempaquetar y construir (ida y vuelta exacta con el SH-2) |
| `examples/system_1/build.sh` (`TONE=1`, `PARAMLOG=1`) | PC | genera C-S1V4 / C-S1TONE / C-S1LOG | PLUG-OUT propio y pruebas |
| `examples/system_1/make_s1.py --sim` | PC | nada (simula esclavo A, B y maestro B con `re/dspisa.py`) | comprobar los programas antes de instalar |
| `examples/system_1/paramlog.py` | PC + build `PARAMLOG` en el maestro | escucha MIDI 1/2 | qué id manda cada control (`docs/examples_system_1_README.md` §6) |
| `system-1/extract_fw.py`, `voice_sets.py`, `fnmatch.py`, `asgn_sites.py` | PC | imágenes del OS | estudio estático |

Lo que **no** hay todavía en el SYSTEM-1: lectura de la memoria de los DSP desde el PC (como `dsppeek` del SYSTEM-8),
escritura en vivo (`dsplive`) y medida por audio USB (`dspaudio`, `voicelog`).
