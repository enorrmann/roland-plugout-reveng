# Emulador de un PLUG-OUT completo: qué hay, qué falta y en qué orden

Estado: 2026-10-22. Plan, no implementado. Objetivo: ejecutar un PLUG-OUT de Roland (empezando por el JX-3P) en el PC
y obtener el mismo audio que el equipo, para estudiar el microcódigo y probar programas propios sin instalarlos.

Etiquetas: **[medido]** = en el equipo; **[código]** = está en nuestro código; **[listado]** = leído del microcódigo
desensamblado; **[documentado]** = en otro documento del repo; **[inferido]** = deducción sin medir.

**Regla de microcódigo** (`VOICE_ALLOCATION.md` §7): el emulador y sus pruebas **pueden** ejecutar el microcódigo de
Roland. Se carga en tiempo de ejecución desde la copia local del usuario (`engine/build/programs.h`,
`plugins/<NOMBRE>`), como hace `engine/programs.py`, y no se guardan bytes de Roland en el repositorio. La prohibición
vale sólo para nuestros plug-outs.

## 1. Lo que ya existe

| pieza | qué hace | límite |
|---|---|---|
| `examples/from_scratch/emu/chain.py` | seis DSP de un slave (y dos slaves con `Bmc2`): bancos 0/1 en anillo, MXM con cerrojo de 1 muestra, buzón de pareja `A400`…, IOMD, enlace entre slaves. Cada hipótesis es un campo de `Model` | sólo probado con nuestros programas (pequeños); banco 6 = 0; sin banco 2; Python lento |
| `examples/from_scratch/emu/test_chain.py` | 20 pruebas; el modelo coincide con la medida de C-V4 (`VOICE_ALLOCATION.md` §6.7) | — |
| `examples/from_scratch/re/dspisa.py` | simulador de una instrucción; 100/100 instrucciones de Roland al azar coinciden con el equipo [medido, `DSP_HARDWARE.md` §10] | huecos de §3.3; flotantes, no enteros |
| `examples/from_scratch/re/emu1.py`, `tools/engine_emu.py` | región 1 (motor ARM) en Unicorn, servicios del OS `0x0113FC00` interceptados | sólo registran las llamadas; no las conectan a ningún DSP |
| `examples/from_scratch/re/emu0.py` | región 0 (modelo, asignador) en Unicorn | sin entrada MIDI ni enlace a los slaves |
| `examples/from_scratch/emu/listing.py` | lee programas de Roland de `programs.h` y los desensambla | sólo lectura estática |
| `voicelog/voicelog.py`, `re/dspaudio.py` | capturas del equipo para comparar | — |

## 2. Arquitectura propuesta

```
MIDI ─► región 0 (Unicorn, emu0) ─ nota/velocidad 0x429+v / 0x43A+v ─► región 1 ×2 slaves (Unicorn, emu1)
                                                                         │ servicios del OS (§3.1)
                                                                         ▼
                              DSP0..DSP5 ×2 (núcleo en C, §3.6) ─► slave 0 6020/6021 ─► WAV
                              (+ más adelante: bus de onda → DSP del maestro, §3.5)
```

## 3. Lo que falta

### 3.1 Unir el ARM con los DSP (complejidad media, conocido)

Los servicios del OS que llama el motor ARM tienen que actuar sobre el emulador de DSP [código, `engine.c` /
`engine_v3.c`; `DSP_ARM_SIDE.md` §2]:

| servicio | acción en el emulador |
|---|---|
| `+0x28/+0x40/+0x58 + 4·dsp` | registrar ranuras, programas y datos: copiar el microcódigo a la ranura del DSP |
| `+0x70` `swap` | cambiar el programa de una ranura y activar su tarea (DCO1 = 2, DCO2 = 3, efecto = 3) |
| `+0xB0` `p_bank` | escribir `A000 + id` del banco 5 del DSP |
| `+0xA0` `p_ramp` | escribir una entrada del banco 6 con rampa (§3.2) |
| `+0x00` `get` | leer el parámetro del patch (modelo de patch en Python) |
| `+0xC0` `sync` | límite de trama: aplicar el cambio entre muestras |
| `+0xF0` | índice del slave |

### 3.2 Banco 6 (`Cnnn`): el bloque de parámetros con rampas (lo más importante y lo menos conocido)

* Los principales de Roland leen mucho de ahí: ≈ 2000 cargas en el corpus (`DSP_ARM_SIDE.md` §3.1); p. ej. los
  coeficientes `C044`–`C048` de la mezcla de MAIN4 [listado]. Hoy `chain.py` devuelve 0.
* Lo que escribe el ARM es conocido: entrada de 16 bytes por id, `[+8] = modo<<28 | flag<<24 | rampa<<20 |
  0x08000000`, `[+0xC] = objetivo`; modos "00 disable, 10 auto ini, 01 timer, 11 env" [documentado, `DSP_ARM_SIDE.md`
  §1 y §2.2].
* **No está medido cómo evoluciona el valor muestra a muestra** (forma y duración de la rampa según `ramp`, qué hacen
  `flag` y los modos). Sin eso no salen envolventes ni niveles.
* Medida propuesta: con el laboratorio del maestro (`re/dspmaster.py` / `dsppeek`), un programa en el DSP3 que copie
  cada muestra una palabra `Cnnn` a DT0, y el ARM que escriba con `+0xA0` saltos de 0 a 1 con distintos `ramp`, `flag`
  y modos; leer la serie y ajustar el modelo.

### 3.3 Fidelidad del simulador de instrucciones con programas largos

| hueco | por qué importa | cómo cerrarlo |
|---|---|---|
| **Latencia de carga para las ops**: `dspisa` hace visible una carga a la instrucción siguiente; `DSP_HARDWARE.md` §7.3 dice que la op siguiente aún ve el valor viejo. Roland tiene `ld r0,[02b2] \| ld r1,[029a]` seguido de `mul r0,r0,r1` | cambia resultados en cadena | prueba en el maestro (`re/dspverify.py` con dos instrucciones) |
| Instrucciones "inferred"/"fitted" del desensamblado (MAIN4: 56 inferidas, 10 ajustadas) | resultados posiblemente distintos | barrido con `re/dspverify.py` de las clases afectadas |
| Modos de transferencia sin decodificar: `9e`, `e0`/`e1`, `d8`/`d9`/`dc`/`dd` | accesos a memoria que hoy no hacen nada | `re/dspmodescan.py`, pruebas en el maestro |
| Banco 7 (`e013`, el JX lo lee como fuente) | ¿ruido, contador? | leer la palabra en el maestro varias muestras seguidas |
| Aritmética en flotante con regla de truncado | errores de redondeo acumulados | núcleo en enteros de 29 bits de mantisa (§3.6) |
| Banderas y saltos por DSP | ya se guardan por DSP en `chain.py` | — |

### 3.4 Banco 2: línea de retardo y EDM

Chorus, delay y phaser la usan. Medido en parte (`DSP_HARDWARE.md` §8.4: escritura `[4300]`, pedido `Xa9Y`, fracción
de 6 bits, unidad 16384 muestras, línea de `f7dc` en EDM). Falta en el emulador: la memoria EDM/IDM, el DMA por muestra
y la ventana de cada DSP/ranura (el ARM la conecta: `+0x310`).

### 3.5 Detalles del slave aún abiertos y el maestro

* Slave: si el buzón `A400` llega en la misma muestra o en la siguiente; orden de los DSP dentro de la muestra; qué
  palabras del slave 1 entran en `6000`–`6003` del slave 0 (`VOICE_ALLOCATION.md` §6.6). Para sonar no importan; para
  coincidir bit a bit, sí.
* Maestro (sólo si se quiere la salida final): DSP del maestro con su microcódigo (efectos globales, master, reverb;
  está en el OS, `m1_tcm`), la región 0 con entrada MIDI y panel, y el bus de onda slaves → maestro (correspondencia
  desconocida). Mientras tanto, la salida de referencia es `6020`/`6021` del slave 0.

### 3.6 Velocidad

`dspisa` en Python hace ≈ 20 000 instrucciones/s (66 instrucciones de la sierra en ≈ 3 ms). Un plug-out de Roland son
≈ 11 000 instrucciones por muestra (6 DSP × ≈ 930 × 2 slaves): **más de 10 horas por segundo de audio**. Hace falta:
1. Núcleo del DSP en C (intérprete), con la misma interfaz que `chain.py` (bancos, MXM, buzón, IOMD), llamable desde
   Python (ctypes/cffi). Validarlo contra `dspisa` instrucción a instrucción y contra `test_chain.py`.
2. Después, traducir cada programa a código nativo una vez (casi no hay saltos) y paralelizar por DSP: el MXM se
   congela por muestra, así que los DSP de una muestra son independientes.
3. Saltear DSP sin voces activas.

### 3.7 Validación

Comparar el render con capturas del equipo, nota por nota: frecuencia, nivel, envolvente. Referencias: C-SAWPOLY
(cadena de Roland con nuestras sierras), el JX-3P de serie en un slot, C-V4. Herramientas: `voicelog.py`
(asignación + audio), `re/dspaudio.py`.

## 4. Orden propuesto

| paso | qué | resultado esperado |
|---|---|---|
| 1 | Núcleo del DSP en C con la interfaz de `chain.py`; pasar `test_chain.py` con él | mismas 20 pruebas, ≥ 100× más rápido |
| 2 | Cargador de programas de Roland desde `programs.h` / `region1.bin` (reutilizar `emu/listing.py`, `re/alloc_scan.py`) | MAIN0–5 y DCO del JX-3P cargados en el emulador |
| 3 | Medir y modelar el banco 6 (§3.2) | rampas y envolventes en el emulador |
| 4 | Resolver la latencia de carga de las ops (§3.3) | `dspisa` y el núcleo en C corregidos |
| 5 | Conectar `re/emu1.py` a los DSP (§3.1): el motor ARM de Roland controla el emulador | una nota MIDI → voz en `6020` |
| 6 | JX-3P sin efectos: render de las ocho notas de `voicelog` y comparación con una captura | afinación y envolvente iguales a la captura |
| 7 | Banco 2 / línea de retardo; efectos del JX-3P | chorus y delay |
| 8 | Región 0 en el lazo (asignador real), varios slaves | polifonía como en el equipo |
| 9 | (opcional) Maestro: DSP de efectos globales y bus de onda | salida final |

Cada paso debe tener una prueba que pueda fallar (como `test_chain.py`). Antes de cada medida en el equipo: una
instalación cada vez, el usuario vuelve a elegir PLG3, nunca dos herramientas de MIDI a la vez.

## 5. Referencias

`VOICE_ALLOCATION.md` §6 (modelo del slave, emulador actual), `DSP_HARDWARE.md` §7–§10 (memoria, comunicación, modelo
de ejecución), `DSP_ARM_SIDE.md` (servicios y bancos del ARM), `FROM_SCRATCH.md` (C-V4), `NOTES.md` ("Posibles
próximos pasos: emulador", plan original con Unicorn y JIT).
