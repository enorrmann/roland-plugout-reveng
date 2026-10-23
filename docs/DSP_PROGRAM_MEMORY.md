# Memoria de programa de los DSP: principal, ranuras y secciones (SYSTEM-8 y SYSTEM-1)

Estado: 2026-10-22. Reúne lo que se sabe de cómo está dividida la memoria de programa de un DSP, quién ejecuta qué
parte, cuánto cabe en cada una y qué variantes de uso son posibles. Sirve para decidir dónde poner un programa
propio. Detalle del ISA y del tiempo: [DSP_HARDWARE.md](DSP_HARDWARE.md); carga desde el ARM:
[DSP_ARM_SIDE.md](DSP_ARM_SIDE.md); PLUG-OUTs propios: [FROM_SCRATCH.md](FROM_SCRATCH.md) (SYSTEM-8),
[examples_system_1_README.md](examples_system_1_README.md) (SYSTEM-1).

Leyenda: **[medido]** en el equipo · **[código]** visto en los binarios · **[inferido]** · **[abierto]** (sin probar).

## 1. Lo común a los dos equipos

* La memoria de programa es un flujo de palabras de 16 bits; cada palabra de 32 bits lleva dos, la mitad alta
  primero. Las direcciones de ranura/sección se dan **en bytes** [código].
* En cada muestra el DSP ejecuta **varios programas**: el **principal** y después los **programas de ranura**
  (SYSTEM-8) o **secciones** (SYSTEM-1) que estén habilitados, en orden. Cada uno termina con
  `0a02 ff70 0b02 00f8 0001`; `0b02 00f8` es el retorno al planificador [medido en el SYSTEM-8; mismo final en el
  SYSTEM-1, que funciona: C-S1V4].
* Todos los programas de un DSP **comparten r0–r7, banderas y registros ocultos**; nada se reinicia entre programas
  ni entre muestras [medido, SYSTEM-8].
* El principal corre **antes** que las ranuras: escribe una entrada en el banco 1 y la ranura la lee en la misma
  muestra; la salida de la ranura vuelve al principal una muestra después (anillo) [medido, C-V4].
* **El límite real es el tiempo, no el espacio**: unas 925–945 instrucciones por muestra y por DSP en el SYSTEM-8
  (en el SYSTEM-1, DSP A del esclavo: **entre 682 y 826** [medido 2026-10-23]) **sumando todos sus programas**; pasarse pierde el final y en los slaves deja el equipo inestable hasta apagarlo [medido]. Lo que
  un salto se saltea no cuesta [medido]. Una instrucción ocupa de 2 a 12 bytes (media ≈ 6–8 en los programas de
  Roland: 575 instrucciones del SH-101 = 5222 bytes), así que el presupuesto de una muestra son ≈ 6–8 KiB de código
  ejecutado.
* Inicio: SYSTEM-8 `0a02 ff70`, SYSTEM-1 `0a02 ff71`; son marcadores sin efecto [medido], se ponen por fidelidad.

## 2. SYSTEM-8

### 2.1 Mapa [código]

Ventana de programa de 64 KiB por DSP (`PRG0` = `0x41460000`, 0x10000 bytes, en la tabla de volcado del OS; el ARM
del slave escribe en `0x41480000 + n·0x100000` + dirección). Ranuras que usan los PLUG-OUTs de Roland (las mismas
que registra `engine_v3.c`):

| DSP (slave) | principal | ranuras | espacio del principal |
|---|---|---|---|
| DSP0, DSP1 | `0x0100` | `0x8000` (LFO/envolventes) | `0x0100`–`0x7FFF` (≈ 32 KiB) |
| DSP2, DSP3 | `0x0100` | DCO1 `0x6000`, DCO2 `0xB000` | `0x0100`–`0x5FFF` (≈ 24 KiB); cada DCO `0x5000` (20 KiB) |
| DSP4 | `0x0100` | `0x6000`, efecto `0xB000` | `0x0100`–`0x5FFF` |
| DSP5 | `0x0100` | `0x6000` | `0x0100`–`0x5FFF` |
| maestro DSP4 | (OS) | efectos `0xEFDC`, `0xF7DC` | — |
| maestro DSP3 | (OS) | — (ocioso; el principal termina `0b03 0080 7fee`) | — |

### 2.2 Quién decide qué corre [código, medido]

* El motor (región 1) **registra** en `e_load` las direcciones de ranura de cada DSP (`+0x28 + 4·dsp`), los
  programas (`+0x40 + 4·dsp`) y los bloques de datos del banco 0 (`+0x58 + 4·dsp`); el OS los copia.
* Una ranura **sólo corre cuando su bit de tarea está puesto**: lo pone `+0x70` (`swap`: cambia el programa de una
  ranura en marcha y activa su tarea; DCO1 = tarea 2, DCO2 = 3; efecto de DSP4 = 3). Registro de tareas del DSP:
  `0x41104318 + n·0x1000` (`máscara | prio << 28`) [código].
* Descriptores: `{dirección, bytes, bytes, puntero, 2 palabras del OS}`; **uno por DSP**, no se comparten (con uno
  compartido se silenciaba un swap de cada dos) [medido].
* Programas vacíos: DSP0, 1, 2, 3 y 5 pueden tener el principal vacío; **un principal vacío en DSP4 silencia todo**
  [medido, `log/WAVE_BUS_LOG.md` §4l]. A las ranuras DCO registradas se les pone un programa nulo propio
  (`gate/dco2_null.hex`) en lugar de dejarlas sin programa [código].

### 2.3 Usos

| uso | cómo | estado |
|---|---|---|
| todo en principales (seis programas) | C-V3/C-V4: principales propios, ranuras DCO con nuestra sierra | [medido] C-V4 |
| ranura como banco de programas | `+0x70` cambia el programa de una ranura sin parar el resto (el JX-3P lo hace al cambiar la forma de onda o el efecto) | [código], usado por C-V4 al arrancar |
| pruebas en vivo | `re/dsplive.py` escribe palabras sueltas en la ranura DCO2 de DSP2/3 (sombra de 8 KiB) | [medido] |
| principal más largo que su espacio (pisar la ranura siguiente) | posible sólo si esa ranura no tiene tarea ni programa registrado; el OS no lo comprueba en lo visto | [abierto] |
| direcciones de ranura propias (otras que las de Roland) | `+0x28` las recibe del motor: podrían elegirse | [abierto] |

## 3. SYSTEM-1

### 3.1 Mapa [código]

Dos DSP por BMC (A y B). Memoria de programa de 32 KiB (lo más alto que se escribe es el cierre en `0x7FF2`–`0x7FFF`).

```
0x0000 ─ cabecera (la escribe el OS: sets de serie {0x0000, 0x1E bytes, …}). Los PLUG-OUTs NO la encolan: hacerlo
         cuelga el equipo [medido, C-S1JX7, 2026-10-22; ../system-1/WAVE_BUS.md §5.3]
0x0100 ─ PRINCIPAL ─────────────── hasta 0x18FF (6 KiB)
0x1900 ─ sección 2 ──────────────── hasta 0x30FF (6 KiB)
0x3100 ─ sección 3                  hasta 0x48FF
0x4900 ─ sección 4                  hasta 0x60FF
0x6100 ─ sección 5                  hasta 0x7FF1
0x7FF2 ─ CIERRE: 7 medias palabras (0000, ld r0,[e000], st r0,[a0xx]): contador de trama al banco 5
```

En el **maestro** se usan además: DSP A `0x0082` (programa de salida que el OS pone en modo PLUG-OUT, `0x82`–`0xFF`)
y DSP B `0x1000` (programa del PLUG-OUT); y cierres de sección vacíos en `0x5FF2`, `0x6DF2`, `0x6FF2` [código].

### 3.2 Quién decide qué corre [código, medido]

* Programas: **sets** `{dirección, bytes, puntero}` terminados en `{0, 0, 0}` en las colas `FC30` (DSP A) y `FC34`
  (DSP B); `FC38`/`FC3C` son colas de datos. El OS vacía las colas y luego llama a `load` del motor (esclavo) o del
  modelo (maestro), que encola lo suyo; el maestro encola antes sus propios sets de modo PLUG-OUT.
* Secciones de voz: `FC44(índice 0–7, on)` pone o quita el bit `1 << sección` de la máscara del DSP (A/B); `FC40`
  coloca un programa de voz en una sección. Índice → (DSP, sección): 0 → A/2, 1 → A/4, 2 → B/2, 3 → B/4,
  4 → A/3, 5 → A/5, 6 → B/3, 7 → B/5 ([../system-1/SH101_POLY.md](../system-1/SH101_POLY.md) §2). Al recargar, el
  OS limpia los bits 1–5 de las dos máscaras (`bic 0x3E`) [código].
* El principal y el cierre corren siempre (C-S1V4 suena con todas las secciones apagadas) [medido].

### 3.3 Usos

| uso | quién | estado |
|---|---|---|
| todo en el principal, secciones apagadas | SH-101 (575 instrucciones), **C-S1V4** (137) | [medido] C-S1V4 |
| principal + secciones de voz (un programa por voz) | sinte de serie (4 por DSP), SH-2 (2 en el DSP A) | [código] |
| encender/apagar voces sin recargar | `FC44(i, on)`: también ahorra ciclos | [código] |
| cambiar el programa de una voz | `FC40` sobre una sección | [código] |
| principal más largo que 6 KiB (pisando `0x1900`…) | con las secciones apagadas nada ejecuta ni escribe esa zona después de `load` (sólo `FC40` o un set la escribirían). El programa seguiría siendo uno solo, lineal, hasta su `0b02 00f8`. Límite: no llegar a `0x7FF2` (el cierre) | [abierto] |
| código que no corre en cada muestra | saltos condicionales: programas alternativos en la misma memoria, elegidos por una bandera; lo salteado no cuesta | [medido en el SYSTEM-8; ISA igual] |
| usar una sección como "segundo principal" | poner un programa propio en `0x1900` y encenderlo con `FC44(0, 1)`: corre después del principal en el mismo DSP | [abierto] (el SH-2 lo hace con programas de voz) |

## 4. Elegir

* **Un solo programa en el principal** (C-V4/C-S1V4, SH-101): lo más simple; todos los datos se eligen libres.
  Cabe todo lo que cabe en el tiempo de una muestra (6 KiB en el SYSTEM-1, ≥ 24 KiB en el SYSTEM-8).
* **Ranuras / secciones**: cuando una parte del sonido se cambia sola (forma de onda, efecto), cuando hay copias por
  voz que conviene apagar, o para cambiar programas en marcha sin tocar el principal.
* **Pasar del espacio de una zona a la siguiente**: sólo con esa zona sin uso y sin tarea; está sin probar en los dos
  equipos. Antes de intentarlo, hacer una prueba con un programa largo que salte sobre lo que no cabe.
