# Lectura de la memoria de los DSP esclavos (investigación, 2026-10-21)

Objetivo: poder leer, desde el host, la memoria de un DSP de un BMC esclavo (programa, banco 0, banco 5, banco 6)
para comparar con un simulador. Sin esto no se puede validar MAIN0, que corre en los voces (esclavos).

**Conclusión corta**: hoy **no hay ninguna vía de lectura de esclavo verificada**. El código del motor del esclavo sí
accede a esa memoria (escribe y podría leer), y el único canal de salida medido desde un esclavo hacia el host es el
**audio USB**. Falta un canal de datos de vuelta y medir la resolución de ese audio.

## 1. Qué está verificado

| hecho | fuente |
|---|---|
| La región 1 del plugout corre en el ARM de los BMC esclavos; la región 0 corre en el maestro. | `BASE_FIRMWARE.md` §8.13 («so the engine code is not present in the master's address space») y §7.10 |
| `dsppeek` sólo lee la memoria del maestro: «the slave's (the voices) is not reachable from region 0». | `CUSTOM_PLUGOUTS.md` (dump en vivo del maestro), `dsppeek.py` cabecera |
| Mapa de memoria de un DSP esclavo `n` (0–5): base `0x41400000 + n·0x100000`; banco 0 (float) en `+0x00000`; bancos 1, 2, 4 en `+0x10000`, `+0x20000`, `+0x30000`; programa en `+0x80000` (= `0x41480000 + n·0x100000`, la misma dirección que usa el maestro); bloque de parámetros en `+0xA0000`. | `CUSTOM_PLUGOUTS.md`, «Memory map of a slave DSP» (de `s_boot`) |
| Banco 5 del esclavo: `0x41114000 + n·0x4000` (0x400 floats). | `CUSTOM_PLUGOUTS.md`, misma sección |
| El motor del esclavo escribe programas y bancos en la memoria del DSP con punteros `volatile` (`live_prog`, `live_bank5`) y recibe órdenes por CC1 reenviadas por la región 0. | `engine/engine.c`, bloque `#ifdef DSPLIVE` (líneas ~352–360, ~506–560) |
| Las órdenes de `dsplive` (`ADDR`, `DATA`, `CTRL`) llegan a los esclavos (`cmd 0 ADDR`, `cmd 1 DATA`, `cmd 2 CTRL`, banco 5 con `ext & 4`). | `re/dsplive.py` cabecera |
| El maestro es host USB de los esclavos (`BMCBulk%d Attached`, `Detached`); la función que envía al esclavo es `0x010125C1` → `0x0100D548`. | `BASE_FIRMWARE.md` §8.13 (`USB host bulk to the BMCBulk slaves`) y §8.11 (driver USB host del kernel del maestro) |
| La salida de audio se captura por USB a 96 kHz, en float32 estéreo. | `re/dspaudio.py` cabecera |

## 2. Lo que no está verificado (bloqueos)

1. **Canal de vuelta esclavo → maestro.** Lo único documentado es el maestro como host USB que envía. No se ha
   encontrado código que reciba datos del esclavo en el maestro (ni bulk IN, ni mailbox, ni MIDI). Hay que medirlo.
2. **Resolución del audio.** La captura es float32, pero la cadena de salida del equipo (DAC, mezcla del maestro,
   USB) puede tener 16 o 24 bits. Sin esa medida, un valor RAW de 32 bits no se puede transmitir exacto por audio.
3. **Offset de lectura del banco 0 y del banco 5 en el esclavo.** El mapa de `CUSTOM_PLUGOUTS.md` da la vista float
   en `+0x00000` y dice que los alias u32 están en `+0x2000` y `+0x4000`. `engine.c` usa `+0x2000` para el banco 5
   («as the master's»), mientras que `CUSTOM_PLUGOUTS.md` da `0x41114000 + n·0x4000` sin desplazamiento. Hay que
   medir cuál es la vista RAW correcta antes de leer nada.
4. **Estado de DSP0 en el esclavo.** MAIN0 corre en un esclavo; hay que confirmar en cuál.

## 2b. Medidas de esta ronda (sin tocar el equipo)

- **Audio, ruido de fondo**: las grabaciones existentes de programas de salida cero o constante
  (`work/dspaudio/a01_zero.raw`, `a02_one_2203.raw`, `d00_zero.raw`, `pc_0.raw`) tienen desviación típica
  0.014–0.023 y no son DC. Ese fondo equivale a unos 6 bits sobre escala completa. Mientras el fondo no se elimine,
  el audio no puede transportar palabras RAW de 32 bits. No es una medida de la capacidad del DAC, sino de la mezcla
  actual del equipo; habría que medirlo con las demás voces silenciadas.
- **Enlace de los esclavos (`work/os/s_boot/s_boot.bin`)**: aparecen las cadenas `RXDT`, `TXDT`, `RXMIDI`, `TXMIDI`,
  `USB Access (DSP %c%c%c%c%c%c) %s` y `BMCInitWaveBus`. `s_data` tiene `RXDT Dmp` y `ASGN Dmp`, pero no hay
  referencias de código a esas cadenas (búsqueda de punteros literales en `s_data.bin` sin resultado): son
  etiquetas de la pantalla de depuración, no comandos. El formato del paquete `RXDT` y quién lo envía sigue sin
  identificarse.
- **Pantalla de depuración**: `CEZUtilDSPDump` (`DSP_ARM_SIDE.md` §1) lista la memoria de los DSP con páginas
  DSP0..5 también en el esclavo. Es el candidato más probable a usar un protocolo de lectura, pero no está
  decodificado.

## 2c. Protocolo de SysEx del esclavo (desensamblado de `s_boot`, 2026-10-21)

Desensamblado con `arm-none-eabi-objdump -d work/os/s_boot/s_boot.elf` (sección `.os`, base `0x01100000`). Las
cadenas se referencian con `adr` (relativo a PC), no con punteros literales; `objdump` anota el destino en el
comentario de cada `adr`.

| dirección | función | qué hace |
|---|---|---|
| `0x01102bd8` | bucle principal del esclavo | llama en cada vuelta a RX MIDI (`0x01107786`), a `0x011078e2` (depuración `TXMIDI`), a RXDT (`0x01107cee`) y a TXDT (`0x01107e9e`) |
| `0x01107d76` | reensamblado de SysEx de USB-MIDI | lee el CIN (`byte0 & 15`): 4 = bytes de SysEx, 5/6/7 = fin con 1/2/3 bytes; acumula en el contexto a partir de `+0x000` y guarda la longitud en `+0x200` |
| `0x01107d6c` | despachador | pasa a `0x011078fc` el contexto como buffer y `[ctx+0x200]` como longitud: **reenvía el mensaje completo** (eco) |
| `0x011078fc` | enviador de SysEx | empaqueta el buffer en paquetes USB-MIDI de 4 bytes (CIN 4, y 5/6/7 al final) y los mete en la cola de transmisión |
| `0x011079e6` | cola de transmisión | inserta una palabra de 32 bits en una cola circular de 1024 entradas (`cpsid`/`cpsie` alrededor) |

Conclusión provisional: el esclavo tiene un **canal SysEx bidireccional** por el enlace USB, con eco. Lo que no
está resuelto es si el maestro reenvía SysEx a los esclavos y si devuelve al host lo que recibe de ellos.

**Dato del usuario (2026-10-21)**: los SysEx arbitrarios no pasan del maestro a los esclavos, pero algunos CC
conocidos sí. Eso es consistente con lo documentado: el canal que funciona es el de CC que reenvía la región 0
(`dsplive.py`) y el de los parámetros del OS (`ARCHITECTURE.md` línea 68: `set(parte, grupo, id, valor)` del kernel
→ `vt+0x30(id, valor)` del engine). El eco de SysEx del esclavo sólo se activa con un SysEx que llegue al esclavo, y
con los SysEx genéricos no llega ninguno. Por tanto **el eco no se puede disparar desde el host hoy**. Según
`ARCHITECTURE.md` línea 72, los esclavos no tienen salida hacia el PC: el eco sólo sirve si el maestro lo recoge
del enlace USB, y eso no está medido.

**Prueba en el equipo**: un SysEx `F0 7D 77 01 02 03 04 F7` enviado al puerto `SYSTEM-8 MIDI 1` (`hw:3,0,0`) con el
laboratorio DSPLIVE instalado no produjo respuesta en 5 s. Es **no concluyente**: el laboratorio es el de DSPLIVE y no
sabemos si el maestro encamina SysEx genéricos a los esclavos.

## 2d. Prueba de retorno con el motor del esclavo (2026-10-21, hardware)

`engine/engine.c` (`-DDSPLIVE`): el CC de control `cmd 2, ext 2/3/4` llama a `live_sysex_test(cable)`, que mete en
la cola de transmisión del esclavo (`0x011079E6`) el SysEx `F0 7D 77 01 02 03 F7` como paquetes USB-MIDI de
cable 0, 8 y 9 (el enviador del OS siempre usa cable 0). El host escucha `SYSTEM-8 MIDI 1` con `amidi -d`.

| prueba | resultado |
|---|---|
| CC de control `cmd 2 ext 2`, `3`, `4` (cable 0, 8, 9) enviados con `dsplive.Link` | ningún SysEx de prueba llega al host |
| captura del host con un SysEx del maestro (`F0 7D 03 01 02 03 04 F7`, con `dsppeek ping`) | la captura **sí** recibe mensajes del maestro |

Qué queda **no verificado**: (a) que el motor reciba el CC de control (no hay efecto observable de `cmd 2`
con `ext 2–4`); (b) que la cola del esclavo se drene hacia el maestro; (c) que el maestro enrute al host los
paquetes que recibe del esclavo. Sin (a) no se puede separar de (b) y (c).

## 3. Ruta propuesta (en orden)

1. **Medir la resolución del audio** con un programa de prueba de valores conocidos (patrones de bits de 16 y 24 bits)
   en un DSP esclavo, con `dspaudio.py` y `dspexp/audio`. Es una medida de hardware que no cambia el código del motor.
2. **Confirmar la vista RAW del banco 5 del esclavo** con una escritura conocida (`dsplive poke` del banco 5) y
   una lectura por audio.
3. **Orden READ en el motor del esclavo** (`engine.c`, nueva rama de `live_cmd`): copia una palabra de la dirección
   pedida a una palabra de banco 5 de un DSP de prueba. Requiere compilar el laboratorio y reinstalarlo con
   `dsplive.py install`, y seleccionar PLG3 en el panel.
4. **Programa de salida en el DSP de prueba**: lee esa palabra del banco 5 y la emite por audio en dos fragmentos de
   16 bits. El host reconstruye la palabra RAW desde la grabación.
5. **Sólo si el paso 1 da resolución suficiente**: volcar el banco 0 de DSP0 del esclavo (ring de MAIN0) y comparar
   con el simulador.

Lo que el paso 3 no resuelve: la sincronía con la muestra. La lectura del anillo sigue dependiendo del momento
(ver `dspmaster.py`: lecturas repetidas y mayoría).

## 4. Estado

- Ningún código nuevo sobre el equipo en esta investigación. El DSPLIVE sigue instalado en PLG3 de la sesión
  anterior.
- Próximo paso que requiere el equipo: el paso 1 (medida de resolución de audio), con `dspaudio.py install`.
