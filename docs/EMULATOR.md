# Emulador de un PLUG-OUT completo: qué hay, qué falta y en qué orden

Estado: 2026-10-22. Hechos el paso 1 (núcleo del DSP en C) y la velocidad (§3.6: un slave del JX-3P en tiempo real,
bit a bit igual a `chain.py`); el resto es plan. **Para retomar: §5.** Objetivo: ejecutar un PLUG-OUT de Roland (empezando por el JX-3P) en el PC
y obtener el mismo audio que el equipo, para estudiar el microcódigo y probar programas propios sin instalarlos.

Etiquetas: **[medido]** = en el equipo; **[código]** = está en nuestro código; **[listado]** = leído del microcódigo
desensamblado; **[documentado]** = en otro documento del repo; **[inferido]** = deducción sin medir.

**Regla de microcódigo** (`log/VOICE_ALLOCATION_LOG.md` §7): el emulador y sus pruebas **pueden** ejecutar el microcódigo de
Roland. Se carga en tiempo de ejecución desde la copia local del usuario (`engine/build/programs.h`,
`plugins/<NOMBRE>`), como hace `engine/programs.py`, y no se guardan bytes de Roland en el repositorio. La prohibición
vale sólo para nuestros plug-outs.

## 1. Lo que ya existe

| pieza | qué hace | límite |
|---|---|---|
| `examples/from_scratch/emu/chain.py` | seis DSP de un slave (y dos slaves con `Bmc2`): bancos 0/1 en anillo, MXM con cerrojo de 1 muestra, buzón de pareja `A400`…, IOMD, enlace entre slaves. Cada hipótesis es un campo de `Model` | sólo probado con nuestros programas (pequeños); banco 6 = 0; sin banco 2; Python lento |
| `examples/from_scratch/emu/test_chain.py` | 20 pruebas; el modelo coincide con la medida de C-V4 (`log/VOICE_ALLOCATION_LOG.md` §6.7). Corre sobre el núcleo en C (4,7 s; antes 9 min 36 s); `--py` = `chain.py` | — |
| `examples/from_scratch/emu/cchain.py` + `dspcore.c` + `dspcomp.py` | **núcleo en C** (paso 1): `cchain.Slave` / `Bmc2` con la interfaz de `chain.py`. `dspcomp.py` decodifica cada instrucción una vez con `dspisa.Layout` (el mismo decodificador que `dspisa.run`, no una copia); `dspcore.c` ejecuta. La `.so` se compila sola en `emu/build/` | misma semántica que `dspisa` (con sus huecos de §3.3) |
| `examples/from_scratch/emu/dspjit.py` + `dspcore.h` | traducción de cada programa a código nativo y grupos de hilos por muestra (§3.6 punto 2): un slave del JX-3P en tiempo real | GCC 14 `-O2` necesita `-fno-gcse -fno-peephole2` (§3.6) |
| `examples/from_scratch/emu/test_core.py` | C contra `chain.py`, bit a bit: las 15 756 instrucciones distintas del corpus local desde estados al azar, la cadena JX-3P (MAIN0–5 + DCO) muestra a muestra, los 212 programas del corpus en nativo contra el intérprete, nativo con hilos contra serie (4000 muestras), `quant`, v3/v4, velocidad (tiempo real) | — |
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
  palabras del slave 1 entran en `6000`–`6003` del slave 0 (`log/VOICE_ALLOCATION_LOG.md` §6.6). Para sonar no importan; para
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

**Hecho el punto 1 (2026-10-22)** [código, `emu/test_core.py`]: el JX-3P en un slave (MAIN0–5 + DCO1/DCO2 SAW en DSP2 y
DSP3, banco 5 al azar) ejecuta **3 621 instrucciones por muestra** (menos que la estimación de arriba: hay saltos), y el
núcleo en C hace ≈ 18 M instrucciones/s (un núcleo, `-O2`): **1 s de audio de un slave ≈ 19 s**. Tiempo real pide
≈ 0,35 G instr/s por slave, unas 20 veces más. El
núcleo se compila con `-ffp-contract=off`: una multiplicación-suma fusionada (FMA) cambiaría los bits respecto de
`dspisa`, que redondea cada producto.

**Hecho el punto 2 (2026-10-22)** [código, `emu/test_core.py`]: **un slave del JX-3P corre en tiempo real**
(1 s de audio en ≈ 0,8 s; ≈ 280 M instr/s), con los mismos bits que el intérprete y que `chain.py`.

* `emu/dspjit.py` traduce cada programa a una función C: palabras como constantes, saltos como `goto`, y las unidades
  llaman a las mismas funciones inline del intérprete (`emu/dspcore.h`), así que la semántica está en un solo sitio.
  Cada programa es un objeto propio, compilado en paralelo y guardado en `emu/build/jit/` por el hash de su fuente
  (JX-3P: ≈ 7 s la primera vez). `cchain.Slave(..., native=True, threads=True)` es el valor por defecto.
* `quant` sin ramas (escala elegida por el exponente IEEE): la versión con ramas era ≈ 90 % del tiempo por saltos mal
  predichos. El traductor sólo cuantiza los registros que pueden estar sin cuantizar (`quant` es idempotente).
* Hilos: un grupo de DSP por hilo dentro de cada muestra (`dspjit.groups`). Van juntos los DSP que comparten una
  palabra IOMD o el banco 1 (`Model.shared1`); con MXM en la misma muestra o la ventana de pareja inmediata, todo va
  en un hilo. La ventana de pareja del banco 5 se fusiona al final de la muestra en el orden del modelo, así el
  resultado no depende de los hilos. El JX-3P da seis grupos; el techo es el DSP más cargado (DSP2/3, ≈ 840 instr.).
  Los hilos se usan en `run()` / `step(n)` con n ≥ 64.
* **GCC 14 con `-O2` compiló mal un programa del corpus** (`p0s_003661c`: r2 = −5 en vez de −3 tras la instrucción 86,
  en la tercera muestra). Se corrige quitando `-fgcse` o `-fpeephole2`; con `-O0`/`-O1`, con la traza o con el
  sanitizer no aparece, y el sanitizer no encuentra comportamiento indefinido. Causa de fondo sin encontrar. Se
  compila con `-O2 -fno-gcse -fno-peephole2`; `test_core.py test_native_corpus` (los 212 programas del corpus,
  nativo contra intérprete) lo vigila.
* Depuración: `cchain.Slave(..., trace=f)` llama `f(dsp, instrucción, regs, flags, pm, pv)` tras cada instrucción, en
  el intérprete y en el nativo (compilado aparte con las llamadas).

**Punto 3 no hecho, a propósito:** un DSP sin voces sigue cambiando su estado (envolventes, anillos de memoria), así
que saltearlo no puede dar los mismos bits; y ya no hace falta para el tiempo real de un slave. Para dos slaves
(paso 8) la opción exacta es correrlos en paralelo (12 hilos) en C.

### 3.7 Validación

Comparar el render con capturas del equipo, nota por nota: frecuencia, nivel, envolvente. Referencias: C-SAWPOLY
(cadena de Roland con nuestras sierras), el JX-3P de serie en un slot, C-V4. Herramientas: `voicelog.py`
(asignación + audio), `re/dspaudio.py`.

## 4. Orden propuesto

| paso | qué | resultado esperado |
|---|---|---|
| 1 | **hecho**: núcleo del DSP en C con la interfaz de `chain.py`; pasar `test_chain.py` con él; además código nativo + hilos (§3.6) | 20/20 pruebas en ≈ 5 s (antes 9 min 36 s); `test_core.py`: 15 756 instrucciones y la cadena JX-3P iguales bit a bit a `chain.py`; un slave del JX-3P a 1,23× tiempo real |
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

## 5. Estado al 2026-10-22 y lo que falta

### 5.1 Hecho (sin commit; el usuario hace los commits)

| qué | dónde | prueba |
|---|---|---|
| `dspisa.run()` separado en `dspisa.Layout` (parte estática) + ejecución; mismo comportamiento | `re/dspisa.py` | 148 722 ejecuciones del corpus iguales a la versión anterior (script de la sesión, no guardado) |
| núcleo en C: intérprete + memoria de `chain.py` | `emu/dspcore.h`, `emu/dspcore.c`, `emu/dspcomp.py`, `emu/cchain.py` | `test_core.py insn` (≈ 5 min), `jx3p`, `ours`, `quant` |
| traducción a nativo + hilos por grupo de DSP | `emu/dspjit.py` | `test_core.py native` (≈ 2 min), `speed` |
| `test_chain.py` sobre el núcleo en C (`--py` = `chain.py`) | `emu/test_chain.py` | 20/20 en ≈ 5 s |
| traza de depuración por instrucción | `cchain.Slave(..., trace=f)` | usada para aislar el fallo de GCC |

Comandos: `cd examples/from_scratch/emu; python3 test_chain.py; python3 test_core.py` (todo, ≈ 8 min; necesita
`work/corpus*` y `engine/build/programs.h` locales, si faltan esas pruebas se saltean).

### 5.2 Abierto en lo hecho

* **Fallo de GCC 14 `-O2`** (§3.6): sin causa encontrada; se evita con `-fno-gcse -fno-peephole2`. Si se cambia de
  compilador o de flags, correr `test_core.py native`. Para seguirlo: el caso es `p0s_003661c` (corpus local), estado
  al azar de `test_native_corpus` (semilla 11), instrucción 86 en la tercera muestra; con la traza activada no aparece.
* Velocidad: el techo con hilos es el DSP más cargado (DSP2/3, ≈ 840 instr./muestra); `quant` y los accesos a
  memoria (`rd`/`wr`, con comprobaciones de sello) son lo que queda en el perfil.
* Punto 3 de §3.6 (saltear DSP sin voces) **no se hace**: no puede ser bit a bit igual.
* Dos slaves: `cchain.Bmc2` avanza muestra a muestra desde Python (sin hilos). Para el paso 8 hace falta un `run` en C
  de varios slaves con el enlace `Model.link`, en paralelo (12 hilos).
* `asm/` (capa de formato del ensamblador) y `dspisa.Layout` siguen siendo dos decodificadores; unificarlos es una
  tarea aparte (comparar los dos sobre el corpus primero; `DSP_ISA_DISCREPANCIES.md` tiene 4 puntos abiertos).

* **La cadena completa del JX-3P no da audio válido** (2026-10-22, `examples/system_1_jx`, `docs/examples_system_1_jx_README.md`
  §7): con el modelo completo y el INIT PATCH, la salida `6020` de MAIN4 es 0, enorme (511) o NaN según el corte
  (barrido 0–255). El coeficiente del filtro sale de `expsplit` + `newton r2, #…` + un polinomio (instrucciones
  ≈ 302–347 de MAIN4) y da valores fuera de rango (−9,6). El DCO1 empieza con la misma secuencia y, con la semántica
  actual de `newton` (`DSP_HARDWARE.md` §5.1: lee `r2`·`r3`), depende de los registros que deja el programa
  anterior. Una de las dos cosas: `newton` con inmediato tiene otra semántica, o Roland encadena registros entre
  programas. Sin resolver; las pruebas de C-S1JX comparan programas entre sí (bit a bit) y no dependen de ello.

### 5.3 Siguiente: paso 2 de §4 (cargador de programas de Roland)

Hoy `test_core.jx_programs()` arma el JX-3P a mano: MAIN0–5 en DSP0–5 y `DCO1_SAW` + `DCO2_SAW` en las ranuras de
DSP2 y DSP3. Falta:

1. leer de `programs.h` / `region1.bin` qué programas hay y su tipo (principal, DCO1, DCO2, efecto) con
   `re/alloc_scan.py` / `emu/allplugouts.py`;
2. modelar las ranuras como las registra el motor ARM (`+0x28/+0x40/+0x58`, `+0x70 swap`: DCO1 = tarea 2, DCO2 = 3,
   efecto = 3; §3.1), con el cambio de programa entre muestras;
3. una prueba que pueda fallar: por ejemplo, que el JX-3P cargado así dé lo mismo que `jx_programs()`, y que cambiar
   la forma de onda (SAW → SQR) cambie la salida.

Después: paso 3 (banco 6, medida en el equipo con el usuario), paso 4 (latencia de carga, medida), paso 5 (`emu1.py`
conectado al núcleo).

## 6. Referencias

`log/VOICE_ALLOCATION_LOG.md` §6 (modelo del slave, emulador actual), `DSP_HARDWARE.md` §7–§10 (memoria, comunicación, modelo
de ejecución), `DSP_ARM_SIDE.md` (servicios y bancos del ARM), `FROM_SCRATCH.md` (C-V4), `NOTES.md` ("Posibles
próximos pasos: emulador", plan original con Unicorn y JIT).
