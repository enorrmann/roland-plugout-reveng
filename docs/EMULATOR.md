# Emulador de un PLUG-OUT completo: qué hay, qué falta y en qué orden

Estado: 2026-10-29 (la referencia es el C: §1, §1.1; velocidad: §3.6.1). Hechos el paso 1 (núcleo del DSP en C) y la velocidad (§3.6: un slave del JX-3P en tiempo real,
bit a bit igual a `chain.py`); el resto es plan. **Para retomar: §5.** Objetivo: ejecutar un PLUG-OUT de Roland (empezando por el JX-3P) en el PC
y obtener el mismo audio que el equipo, para estudiar el microcódigo y probar programas propios sin instalarlos.

Etiquetas: **[medido]** = en el equipo; **[código]** = está en nuestro código; **[listado]** = leído del microcódigo
desensamblado; **[documentado]** = en otro documento del repo; **[inferido]** = deducción sin medir.

**Regla de microcódigo** (`log/VOICE_ALLOCATION_LOG.md` §7): el emulador y sus pruebas **pueden** ejecutar el microcódigo de
Roland. Se carga en tiempo de ejecución desde la copia local del usuario (`engine/build/programs.h`,
`plugins/<NOMBRE>`), como hace `engine/programs.py`, y no se guardan bytes de Roland en el repositorio. La prohibición
vale sólo para nuestros plug-outs.

## 1. Lo que ya existe

**La referencia del DSP es el código C** (decisión del usuario, 2026-10-29): `emu/dspdec.h` (decodificador),
`emu/dspcore.h` (semántica de cada unidad) y `emu/dspcore.c` (intérprete, esclavo, máquina de un DSP). Desde Python se
usa sólo a través de `emu/dspref.py` (ctypes, sin semántica propia). El modelo en Python (`re/dspisa.py`, el `Slave`
de `chain.py`, el frente de `dspcomp.py`) quedó archivado en `archive/` (§1.1); `re/dspisa.py` es un aviso que falla
al importarlo.

| pieza | qué hace | límite |
|---|---|---|
| `examples/from_scratch/emu/dspdec.h` | plantillas de clase (`re/dsp_templates.txt`, `re/dsp_units_ab.txt`), área de slots, flujo de inmediatos, unidades, condiciones, saltos → `Op` / `Cond`; `dsp_check` (formas sin efecto medido), `dsp_hidden_use` | — |
| `examples/from_scratch/emu/dspref.py` | `Machine` (un DSP, memoria plana 0–FFFF, banco 5 plegado a 1024 palabras: el simulador de las herramientas `make_*.py`, `dspasm.py`, `dspnative.py`, `dspverify.py`, `sim_main.py`), `layout`, `template`, `check`, `quant`, `raw_value`, contextos de banderas | — |
| `examples/from_scratch/emu/chain.py` | `Model` (las hipótesis del esclavo), `Program`, `access_violations`, ayudas | el emulador es `cchain` |
| `examples/from_scratch/emu/test_chain.py` | 20 pruebas; el modelo coincide con la medida de C-V4 (`log/VOICE_ALLOCATION_LOG.md` §6.7), sobre el núcleo en C | — |
| `examples/from_scratch/emu/cchain.py` + `dspcore.c` + `dspcomp.py` | **núcleo en C**: `cchain.Slave` / `Bmc2`. `dspcomp.py` sólo empaqueta lo que decodifica `dspdec.h`. La `.so` se compila sola en `emu/build/` (`dspref.lib`) | huecos de §3.3 |
| `examples/from_scratch/emu/dspjit.py` + `dspcore.h` | traducción de cada programa a código nativo y grupos de hilos por muestra (§3.6 punto 2): un slave del JX-3P en tiempo real | GCC 14 `-O2` necesita `-fno-gcse -fno-peephole2` (§3.6) |
| `examples/from_scratch/emu/test_core.py` | las 15 756 instrucciones distintas del corpus local contra el archivo dorado `work/golden_insn.txt` (sólo hashes, local), JX-3P nativo contra intérprete, los 212 programas del corpus en nativo contra el intérprete, nativo con hilos contra serie, `quant` contra su definición, v3/v4, velocidad | el dorado se regenera (`--update-golden`) sólo tras un cambio medido de la semántica |
| `examples/from_scratch/re/emu1.py`, `tools/engine_emu.py` | región 1 (motor ARM) en Unicorn, servicios del OS `0x0113FC00` interceptados | sólo registran las llamadas; no las conectan a ningún DSP |
| `examples/from_scratch/re/emu0.py` | región 0 (modelo, asignador) en Unicorn | sin entrada MIDI ni enlace a los slaves |
| `examples/from_scratch/emu/listing.py` | lee programas de Roland de `programs.h` y los desensambla | sólo lectura estática |
| `voicelog/voicelog.py`, `re/dspaudio.py` | capturas del equipo para comparar | — |

### 1.1 El port a C (2026-10-29)

`archive/dspisa.py`, `archive/chain.py` (con su `Slave` en Python) y `archive/dspcomp_py.py` son el modelo anterior,
guardado sólo para validar. `archive/validate_port.py` comparó el C con él sobre las 9791 instrucciones distintas de
`work/corpus_all`: plantilla, slots, unidades, decodificación y ejecución de una instrucción desde estados al azar
(registros, banderas, memoria, saltos). Resultado: idénticas, salvo 50 cuyo coeficiente de MAC cambió por la regla
medida ese día (`DSP_ISA_DISCREPANCIES.md` D8). `sim_main.py` dio las mismas negativas que antes sobre MAIN0–5.
Las herramientas pasaron a `dspref.Machine` y siguieron dando lo mismo: `make_synth.py` y `make_mutable.py
--sim-check` (el esclavo de `cchain` contra la máquina de memoria plana) idénticos, `make_*.py --sim` de
`examples/from_scratch` con las mismas frecuencias.

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
* **Lo que parecía un error de GCC 14** (`p0s_003661c`: r2 = −5 en vez de −3 tras la instrucción 86, en la tercera
  muestra; se iba quitando `-fgcse` o `-fpeephole2`) **era el signo de un NaN** [medido, 2026-10-23].
  * Los estados al azar del test traen inf, y aparecen NaN.
  * C no fija qué NaN devuelve `a + b` con operandos NaN, y el compilador puede conmutarlos. La op `sign` convertía
    luego ese signo en ±1.
  * Con clang 19 salía lo mismo, en `p0s_003661c` y en `p0s_003743c`.
  * Arreglo: `quant` devuelve un NaN canónico (+qNaN). Además, las ops exactas que niegan no se saltan la `quant`
    (§3.6.1). Con eso, gcc sin esas flags y clang dan 0 diferencias en los 212 programas, y las flags se quitaron.
    El DSP real no tiene NaN.
* Depuración: `cchain.Slave(..., trace=f)` llama `f(dsp, instrucción, regs, flags, pm, pv)` tras cada instrucción, en
  el intérprete y en el nativo (compilado aparte con las llamadas).

**Punto 3 no hecho, a propósito:** un DSP sin voces sigue cambiando su estado (envolventes, anillos de memoria), así
que saltearlo no puede dar los mismos bits; y ya no hace falta para el tiempo real de un slave. Para dos slaves
(paso 8) la opción exacta es correrlos en paralelo (12 hilos) en C.

### 3.6.1 Más velocidad (2026-10-23) y modelo del SYSTEM-1

Medido con C-S1SY paso 11 (SYSTEM-1: esclavo A + esclavo B + master B, 1264 instrucciones por muestra, nativo, un
hilo, 96 000 muestras = 1 s de audio). Todos los cambios dan los mismos bits: `test_core.py` completo (corpus de
instrucciones, cadena JX-3P, 212 programas nativos contra el intérprete) y `make_synth.py --sim-check`:

| cambio | 1 s de audio | M instr/s |
|---|---|---|
| antes | 1,207 s | 101 |
| `pymod` con caminos rápidos exactos para −m ≤ x < 2m (`ph_wrap` / `ph_tri` de una fase; `test_pymod` contra `fmod`) + `-fno-math-errno` | 0,872 s | 139 |
| `quant` como máscara de bits de la mantisa, en lugar de escalar y convertir a entero | 0,588 s | 206 |
| la máscara precalculada por exponente: `qmask.h` (`gen_qmask.py`), una carga y un AND | **0,341 s** | **356** |

* `quant` era ≈ 80 % del tiempo: está en la cadena de dependencias de cada instrucción. La máscara deja a 0 los
  k = 20 + 4s − E bits bajos de la mantisa (k > 52 → un cero con signo; |v| ≥ 512, inf, nan: nada). Es la misma
  regla que el escalado (la conversión a entero de m·2^p es su suelo). `test_quant` la compara con `quant_ref` y con
  `dspisa.quant`.
* **Modelo del SYSTEM-1 en un solo `Slave`.**
  * `Model.shared5 = ((0, 1),)`: un banco 5 (`A000`–`A3FF`) para dos DSP, como el `A1xx` que el esclavo A escribe
    y el B lee.
  * `Model.own_params = (0, 1, 2)`: un banco 6 por DSP (`FCEC(sel, …)`). `cchain.DspView.params` es el banco 6 de
    ese DSP.
  * `dspjit.groups` pone en el mismo hilo los DSP que comparten banco 5. `chain.py` implementa las dos opciones
    igual; `test_s1_shared5` compara el C con `chain.py` muestra a muestra.
  * `Slave.run(n, [(dsp, dirección)])` registra palabras de cualquier DSP (por ejemplo `A118` del master), no sólo
    del IOMD.
* `make_synth.py simulate()` usa ese modelo: una llamada a C por tramo entre eventos, en vez de un bucle de Python
  por muestra. Antes eran dos `Slave` y copias del banco 5 en cada muestra. 1 s de audio: de 1,87 s a **0,43 s**
  (tiempo real × 2,3), con 0,08 s de preparación.
* SYSTEM-8: el slave del JX-3P (6 hilos) pasa de 1,23× a **2,05× tiempo real** (170 M instr/s; `test_speed`).
* Pruebas nuevas: `test_pymod` (caminos rápidos contra `fmod`) y `test_s1_shared5` (C contra `chain.py` con
  `shared5` / `own_params` y C-S1SY paso 11). `test_core.py` pasa 9/9 y `test_chain.py` 20/20 (2026-10-23).
* `dspcomp.decode_program` guarda la decodificación en el `Program`: antes se decodificaba tres veces por `Slave`
  (`compile_program`, `translate` y `groups`).

**Segunda tanda (2026-10-23, más tarde)**, mismos bits (`test_core.py` 10/10, `--sim-check`). La simulación completa del SYSTEM-1
(`make_synth.simulate`, 1 s de audio):

| cambio | 1 s de audio | qué hace |
|---|---|---|
| antes de la segunda tanda (un hilo, modelo del SYSTEM-1) | 0,57 s | — |
| **pipeline A ∥ B** (`cchain.pipeline_plan`, `sl_set_pipeline`) | 0,29 s | el esclavo A calcula la muestra t + 1 en su propio hilo mientras B y el master calculan la t |
| **sin `quant` tras las ops exactas** (`dspjit.exact_dest`) | **0,24 s** (4× tiempo real) | `min`/`max`/`abs`/`clamp`/`neg`/copias de la unidad B con entradas en la rejilla: el resultado ya está en ella |

* **Pipeline.** Cada DSP lleva su propio reloj de anillo (`Dsp.t`): el de la etapa A va una muestra por delante.
  * La etapa B lee un búfer del banco 5 compartido. En cada barrera se copian a él sólo las palabras que escribe A
    (40 en C-S1SY). Al terminar `sl_step`, las que escriben los otros DSP (6) vuelven al banco compartido.
  * `pipeline_plan` sólo lo activa si da los mismos bits:
    * un único par `shared5`, con A antes que B;
    * MXM latcheado;
    * A no toca IOMD, MXM, la ventana de pareja ni un banco 1 compartido;
    * no hay transferencias indexadas al banco 5;
    * A no lee ni escribe palabras que escriben los demás.
  * `test_s1_pipeline` compara pipeline y serie, palabra a palabra, durante 6000 muestras con notas y LFO.
* **Ops exactas.** De las cuantizaciones que quedan, casi todas son de registros recién cargados, y son obligatorias:
  una op que usa una carga lee el valor crudo y el registro se cuantiza después (§3.6.1 arriba; regla 6 de
  [MICROCODE_OPTIMIZATION.md](MICROCODE_OPTIMIZATION.md)). Sin ninguna `quant` serían 0,13 s, pero no sería exacto.
* **Compilador:** `DSP_CC=gcc|clang` (por defecto clang si está instalado). Mismo rendimiento dentro del ruido
  (S1: 0,24 s con los dos; JX-3P: 2,3× / 2,45× tiempo real). Ya ninguno necesita `-fno-gcse -fno-peephole2` (era un NaN, §3.6).
  El nombre del compilador entra en el hash de la caché y hay una `dspcore-<cc>.so` por compilador.
* **Referencia Python más rápida, sin cambiar su semántica:** `dspisa.layout()` guarda el `Layout` por palabras,
  `Imm` guarda la cadena de bits y `quant` está memoizada (sin los ceros: 0,0 y −0,0 son la misma clave). La
  referencia pasa de ≈ 83 a ≈ 30 ms por muestra.
* **Simulaciones en paralelo:** `make_synth.run_sims` corre cada simulación independiente en su propio proceso
  (`SIM_JOBS`, por defecto todos los núcleos). La caché del JIT admite varios procesos a la vez (temporales por PID)
  y el código nativo se compila en el padre antes del fork. `--sim-check`: de ≈ 2,5–3 min a **27 s**; `--sim`: 2,8 s.
* **Preparación:** `dspcomp.decode_program` guarda la decodificación por palabras para todo el proceso: 0,084 →
  0,037 s por simulación.

Lo que queda en Python:
* (hasta el 2026-10-29: la referencia `dspisa`; archivada, la referencia es el C, §1.1);
* la preparación (≈ 0,04 s);
* `Bmc2` (los dos slaves del SYSTEM-8, un bucle de Python por muestra; sólo lo usa `test_chain.py`). Pasarlo a C, con
  los dos slaves en paralelo, es lo siguiente para el SYSTEM-8.

Siguientes mejoras posibles, en orden de rentabilidad [inferido, sin medir]:

1. ~~**Sin `quant` tras las ops exactas.**~~ Hecho (arriba). Lo que sigue es la versión original de la idea: `min`, `max`, `mov rX`, `ph_clamp`, `ph_abs` y `ph_neg` devuelven una de
   sus entradas, o ±1, o −x. Si las entradas están en la rejilla, el resultado también. Hace falta un análisis en
   `dspjit.translate`. Sin `quant` el tiempo es 0,13 s en vez de 0,34, aunque sólo una parte de las ops es exacta.
2. **Audio en vivo.** Un reproductor que corra `Slave.run()` en bloques de, por ejemplo, 256 muestras y los mande a
   ALSA, con las notas por MIDI desde el PC. Así se puede escuchar un PLUG-OUT propio sin instalarlo.
3. **Los dos slaves del SYSTEM-8 en paralelo** (12 hilos, §3.6 punto 3).
4. **El lado ARM** (`engine.c` compilado para el PC, con los servicios del OS simulados) para que los mensajes MIDI
   lleguen a los DSP como en el equipo (§3.1).

### 3.7 Validación

Comparar el render con capturas del equipo, nota por nota: frecuencia, nivel, envolvente. Referencias: C-SAWPOLY
(cadena de Roland con nuestras sierras), el JX-3P de serie en un slot, C-V4. Herramientas: `voicelog.py`
(asignación + audio), `re/dspaudio.py`.

**Ops nativas contra el equipo** (`re/dspnative.py`, maestro del SYSTEM-8 con la sonda PEEK, 2026-10-28): cada op
con registros al azar, con y sin instrucción vacía antes de guardar, comparada palabra por palabra con
la referencia (hasta el 2026-10-28 `re/dspisa.py run`, desde el 2026-10-29 el C a través de `emu/dspref.py`). Lo que
se corrigió en `emu/dspcore.h` (y, antes del port, en `dspisa.py`) y `emu/dspjit.py` (`writes`):

| op | antes en el emulador | en el equipo |
|---|---|---|
| `shl` / `shr` / `shrn` | X·2^k sin límite, truncado hacia 0 | complemento a 2 con 32 bits de fracción: redondea hacia −∞ en 2^−32, envuelve en [−512, 512); después la codificación RAW trunca hacia 0 (2026-10-29: 672/672 casos crudos; el «hacia −∞ en la grilla RAW» del 2026-10-28 sólo acertaba con el resultado en la grilla 2^−32) |
| `recipseed` | escribía r(d), r(d+1) | también rY ← m; r(d) = 0 si 2^−e ≥ 512 |
| `expsplit` | e^⌊Y⌋ exacto | ROM medida (`EXP_ROM`, n = −19…6), 0 debajo, satura arriba |
| SFU modo 3, bit 3 | −(Y < 0) | ⌊Y⌋ (`floor`) |
| `log2split`, Y < 0 | Y/512 truncado | Y leída en complemento a 2: e = ⌊log2((Y + 512)/0,75)⌋, m = Y/2^e redondeado hacia −∞ en 2^−32 y truncado por la codificación (2026-10-29: 120/120 crudos; e = 9 sólo para \|Y\| < 115) |
| `sqrtsplit` | escribía r7 ← ±2^−k, semilla de 8 bits | rY ← m = \|Y\|·4^k ∈ [1, 4), r(d+1) ← Q[⌊(m−1)·16⌋]/512 con Q = round(512/√(medio del intervalo)) (9 bits), **r(d+3)** ← signo·2^−k (2026-10-29: 384 crudos + 3072 en `dspnative`). Con Y en r(d+1) o r(d+3) las escrituras mezclan bits: `dsp_check` la marca no medida |
| MAC con D ≥ 8 | /2 siempre | /2 sólo con término P2 (op1 ≥ 4); sin P2 no escribe nada (2026-10-29) |
| dos MAC (clase 0x080) | ambas escriben | un término P2 en cualquiera de las dos: ninguna escribe (2026-10-29) |
| coeficiente inmediato de la MAC | después del inmediato de Y (Y inmediato) | en el byte C del arreglo de slots, +8 con el bit 9 de N; Y registro o inmediato, con o sin transferencias (19 clases, 2026-10-29) |

Después de los cambios: 0 diferencias en 96 casos por op (`dspnative.py -n 96`), `test_core.py` 11/11 (incluidos
los 212 programas de Roland en la traducción nativa).

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
* `asm/` (capa de formato del ensamblador) y `emu/dspdec.h` siguen siendo dos decodificadores; unificarlos es una
  tarea aparte (comparar los dos sobre el corpus primero).

* **La cadena completa del JX-3P no da audio válido** (2026-10-22, `examples/system_1_jx`, `docs/examples_system_1_jx_README.md`
  §7): con el modelo completo y el INIT PATCH, la salida `6020` de MAIN4 es 0, enorme (511) o NaN según el corte
  (barrido 0–255). El coeficiente del filtro sale de `expsplit` + `newton r2, #…` + un polinomio (instrucciones
  ≈ 302–347 de MAIN4) y da valores fuera de rango (−9,6). El DCO1 empieza con la misma secuencia y, con la semántica
  actual de `newton` (`DSP_HARDWARE.md` §5.1: lee `r2`·`r3`), depende de los registros que deja el programa
  anterior. Una de las dos cosas: `newton` con inmediato tiene otra semántica, o Roland encadena registros entre
  programas. Sin resolver; las pruebas de C-S1JX comparan programas entre sí (bit a bit) y no dependen de ello.

* **Límites de acceso por instrucción** (2026-10-23): el intérprete hace todos los accesos de una instrucción, pero el
  hardware no (banco 5 ≤ 1 carga y ≤ 1 store, banco 0 ≤ 2 cargas y ≤ 2 stores, banco 6 ≤ 2 cargas). Programas que
  pasaban el emulador colgaron el SYSTEM-1. Desde ahora `chain.Program` (y por tanto `cchain`) **rechaza** un programa
  que incumpla esos límites (`chain.access_violations`, `STRICT`; `strict=False` sólo para arneses de prueba que no
  van al equipo). Comprobado contra las builds instaladas: rechaza exactamente las que colgaron por esa causa
  (`examples/system_1_jx/test_s1jx.py` `device_history`).

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
