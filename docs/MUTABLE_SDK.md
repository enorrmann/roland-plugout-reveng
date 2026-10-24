# `sdk/mport`: portar código DSP de Mutable Instruments al microcódigo de los BMC de Roland

Especificación. Estado: 2026-10-24, versión 1.0.

`sdk/mport` es un conjunto de módulos Python para llevar osciladores, filtros y efectos del repositorio de Mutable
Instruments (`~/git/eurorack`, MIT) a nuestro microcódigo. El ISA es el mismo en el SYSTEM-1 y en el SYSTEM-8
([DSP_ISA.md](DSP_ISA.md)). Hace cuatro cosas:
1. Traduce el bucle por muestra a microcódigo (`kernel.py`).
2. Lleva a la build lo que el original calcula por bloque (`component.py`, `lut.py`).
3. Compila el original para tener una referencia (`reference.py`).
4. Mide el port contra esa referencia en el emulador (`bench.py`, `compare.py`, `cli.py check`).

Primer componente portado: **SAW_SWARM de Braids** (§7), que usa el ejemplo MUTABLE-SYSTEM
([examples_mutable_system_README.md](examples_mutable_system_README.md)). Qué más se puede portar:
[examples_mutable_system_CANDIDATES.md](examples_mutable_system_CANDIDATES.md).

Etiquetas: **[medido]** en el equipo; **[sim]** en el emulador (núcleo C); **[código]** leído en la fuente de
Mutable; **[abierto]**.

## 1. Archivos

| archivo | qué es |
|---|---|
| `sdk/mport/kernel.py` | `Kernel`: el bucle por muestra escrito con valores; asigna registros y genera líneas de una unidad (§3) |
| `sdk/mport/mi.py` | convenciones de punto fijo de Mutable → nuestro modelo numérico; `on_grid`; el LCG de stmlib (§2) |
| `sdk/mport/lut.py` | lee las tablas de `<módulo>/resources.cc`; `Interpolate88` / `Interpolate824`; ajuste de polinomios (§4) |
| `sdk/mport/component.py` | `Component` y `Param`: qué es un componente portado (§5) |
| `sdk/mport/components/` | componentes portados (`REGISTRY` en `__init__.py`); `braids_saw_swarm.py` (§7) |
| `sdk/mport/reference.py` | compila el C++ original con g++ y lo ejecuta (§6.1) |
| `sdk/mport/ref/braids_render.cc` | programa de referencia: `braids::DigitalOscillator` con forma, tono y parámetros por línea de comandos |
| `sdk/mport/ref/stmlib/` | sustitutos mínimos de stmlib (`stmlib.h`, `utils/dsp.h`, `utils/random.h`): el submódulo está vacío en el checkout de eurorack |
| `sdk/mport/bench.py` | un componente solo en un DSP del emulador (§6.3) |
| `sdk/mport/compare.py` | métricas port ↔ referencia: SNR, f0, RMS, pico, niveles por banda (§6.4) |
| `sdk/mport/cli.py` | línea de comandos: `list`, `check`, `lines`, `lut` (§8) |
| `sdk/mport/tests/test_mport.py` | pruebas (§9) |
| `sdk/mport/build/` | el ejecutable de referencia compilado (no está en git) |

Dependencias: Python 3 con numpy; g++ y el checkout de eurorack para la referencia (`EURORACK`, por defecto
`~/git/eurorack`); `asm/` y el emulador de `examples/from_scratch/emu/` del repositorio.

## 2. Modelo numérico (`mi.py`)

Nuestro DSP trabaja con números con signo de hasta ±512, mantisa de 29 bits y resultado truncado hacia cero
([DSP_HARDWARE.md](DSP_HARDWARE.md)). Corre a 96 kHz, la misma frecuencia que Braids. Cada magnitud de Mutable
tiene un equivalente fijo:

| Mutable (Braids / Plaits) | aquí | función |
|---|---|---|
| muestra `int16` | float −1…1 (x = s / 32768) | — |
| fase `uint32`, sierra = `phase >> 19` | fase −1…1 envuelta con `ph_wrap`: p = phase / 2³¹ − 1 | `phase_from_u32` |
| incremento de fase `uint32` | 2·f / fs | `increment(hz)` |
| tono `int16` = nota MIDI << 7 (1/128 semitono) | palabra de tono = (nota − 12) / 12 octavas (4,75 = La 440) | `braids_pitch`, `pitch_word` |
| parámetro (TIMBRE / COLOR) 0…32767 | perilla del SYSTEM-1 0…255 → v · 32767 / 255 | `param`, `knob` |
| producto Q15 `a * b >> 15` | `a · b` en float | — |
| `CLIP` a int16 | `ph_clamp` (a −1…1) | — |
| tablas | polinomio (por muestra) o tabla del ARM (por bloque) | `lut.py` |
| `stmlib::Random` | `Lcg` (mismas constantes) | — |

Regla de la rejilla: un parámetro que se usa en la instrucción siguiente a su carga tiene que estar en la rejilla
del registro (múltiplos de 2⁻³² por debajo de 0,125). Si no, la versión empaquetada y la sin empaquetar difieren
en 1 LSB ([MICROCODE_OPTIMIZATION.md](MICROCODE_OPTIMIZATION.md), regla 6). `Param.values` aplica `on_grid` a
todo valor.

## 3. `Kernel`: el bucle por muestra

### 3.1 Uso

```python
from kernel import Kernel
k = Kernel('voz 0', regs=(0, 1, 2, 3, 4, 7))         # registros que puede usar; r5 / r6 son del programa anfitrión
p = k.state(0x0400)                                  # palabra de estado: carga 0x0401 (muestra anterior), guarda 0x0400
inc = k.increment(k.load(0xA030))                    # 2^(tono − 4,75) · 2 · 440 / 96000
q = k.wrap(k.add(p.value, inc))
p.write(q)
k.store(q, 0xA151)
lines = k.lines()                                    # 'ld r0, [a030]', 'mov r1, r1', 'addxy r1, r0, #-4.75', ...
```

Salen líneas de una unidad, con rellenos `mov rN, rN` tras las cargas. Es la misma forma que escriben a mano los
generadores del repositorio, así que se planifican y validan igual: `asm/dspsched.py`, `--pack-check`,
`tools/check_optimized.py`.

### 3.2 Operaciones

x es un valor. y es un valor, un valor negado (`-v`, bit 3 del campo de registro) o un número (inmediato).

| método | resultado | instrucción |
|---|---|---|
| `load(addr)`, `param(idx)` | palabra de memoria / parámetro `C0idx` | `ld rD, [addr]` |
| `store(v, addr)` | — | `st rV, [addr]` |
| `state(addr)` → `Ring` | `.value` carga `addr + 1`, `.write(v)` guarda `addr` | anillo del banco 0 |
| `mov(y)` | y | `mov rD, Y` |
| `mul(x, y)` | x·y | `mul rD, rX, Y` |
| `add(x, y)` | x + y | `addxy rD, rX, Y` |
| `sub(x, y)` | x − y | `subxy rD, rY, X` (la op calcula Y − X) o `addxy` con −y |
| `mad(acc, x, y)` | acc + x·y (el resultado toma el registro de acc) | `mad rD, rX, Y` |
| `ymac(acc, x, y)` | y + x·acc: un paso de Horner | `ymacd rD, rX, Y` |
| `min(x, y)`, `max(x, y)` | mínimo / máximo | `min` / `max` |
| `wrap(x)` | x envuelto a [−1, 1) | `ph_wrap rD, r3, rX` |
| `tri(x)` | 1 − 2·\|wrap(−x − 1,5)\| | `ph_tri rD, r3, -rX` |
| `clamp(x)` | x recortado a [−1, 1] | `ph_clamp rD, r3, -rX` |
| `horner(x, c)` | c₀ + x·(c₁ + x·(c₂ + …)) | `mul` + `addxy` + `ymacd`… |
| `poly_odd(x, a)` | x·(a₁ + a₃x² + a₅x⁴ + …) | ídem sobre x² |
| `exp2(x)` | 2ˣ para \|x\| ≤ 8: (e^(x·ln2/16))¹⁶, 4 términos de Taylor | la forma de `make_synth.py` |
| `increment(pitch)` | paso de fase de una palabra de tono | `exp2` + escala |
| `comment(text)` | línea `; text` | — |

En las ops de fase, la X = r3 es la codificación de Roland: `ph_*` no lee el registro r3. Sólo se emiten tipos de
op que ya corrieron en el SYSTEM-1, el conjunto `OPS` de `dspsched`.

### 3.3 Asignación de registros

* **Barrido lineal en orden de programa.** Un valor vive desde su línea hasta su última lectura.
* **Reutilización.** Todas las unidades de una instrucción leen antes de escribir. Por eso un resultado puede
  ocupar el registro de un operando que muere en esa misma línea.
* **Orden de los registros libres.** Se toma el que lleva más tiempo libre. Así hay menos dependencias falsas
  (WAR/WAW) y el planificador tiene más margen.
* **Acumuladores.** `mad` / `ymacd` escriben en el registro del acumulador. Si ese valor se vuelve a leer más
  adelante, el kernel hace antes una copia con `mov`.
* **Errores.** Si no queda ningún registro libre: `KernelError` con la lista de valores vivos. No hay
  derrame a memoria (§10).
* **Rellenos.** `lines(fillers=True)` pone `mov rS, rS` tras una carga cuando la línea siguiente usa ese registro
  (latencia de carga 1). El planificador los quita.

### 3.4 Ejemplo verificado

Programa de banco de SAW_SWARM, sin planificar: `python3 sdk/mport/cli.py lines saw_swarm --plain`. Se muestran
el principio (incremento) y la primera sierra:

```dsp
ld r0, [a030]
mov r1, r1
addxy r1, r0, #-4.75
mul r2, r1, #0.04332169878499658
mov r3, #0.041666666666666664
ymacd r3, r2, #0.16666666666666666
ymacd r3, r2, #0.5
ymacd r3, r2, #1.0
ymacd r3, r2, #1.0
mul r4, r3, r3
mul r5, r4, r4
mul r6, r5, r5
mul r7, r6, r6
mul r0, r7, #0.009166666666666667
ld r1, [c040]
mov r0, r0
mul r2, r0, r1
ld r3, [0101]
mov r0, r0
addxy r4, r3, r2
ph_wrap r5, r3, r4
st r5, [0100]
mul r6, r5, #0.125
```

## 4. Tablas (`lut.py`)

Los módulos de Mutable leen sus curvas de tablas `int16` / `uint16` de `resources.cc`, que generan
`resources/*.py`. Aquí se tratan según cuándo se leen:

| uso de la tabla | qué se hace | ejemplo |
|---|---|---|
| **por muestra** (waveshaper, seno, ventana) | polinomio ajustado por mínimos cuadrados (`fit_odd`, `fit`); el error se da en LSB de la salida int16 | `ws_moderate_overdrive` → polinomio impar de grado 9 |
| **por bloque** (coeficiente que depende sólo de una perilla o de la nota) | se evalúa en Python en la build, para los 256 valores de la perilla, y va a una tabla del ARM | `lut_svf_cutoff` → fórmula exacta sobre el incremento (§7.2) |

Una lectura por fase de una tabla en el DSP necesita un índice calculado. En el SYSTEM-1 eso está [abierto]: es el
paso 15 de C-S1SY, con las tomas en silencio.

API:

| función | qué hace |
|---|---|
| `tables(module)` / `table(module, name)` | arrays de `<EURORACK>/<module>/resources.cc` |
| `interp88(t)` | `Interpolate88(t, x·32768 + 32768) / 32768` como función de x ∈ [−1, 1) |
| `interp824(t)` | `Interpolate824(t, phase) / 32768` como función de la fase ∈ [0, 1) |
| `fit_odd(f, degree, x_max)` | coeficientes (a₁, a₃, …) y error máximo en LSB |
| `fit(f, degree, x_min, x_max)` | coeficientes c₀… (orden de `Kernel.horner`) y error máximo |
| `scaled_odd(c, s)` | el mismo polinomio para una entrada escalada |

`ws_moderate_overdrive` (tanh(2x), Braids) en x ∈ [−0,875; 0,875], que es el rango de SAW_SWARM [sim]:

| grado | error máximo (LSB) | ops |
|---|---|---|
| 3 | 1858 | 3 |
| 5 | 434 | 4 |
| 7 | 97 | 5 |
| 9 | 21,6 | 6 |
| 11 | 4,8 | 7 |

## 5. Componente (`component.py`)

Una función `Render()` de Mutable se divide en dos partes:

| parte | en Mutable | aquí |
|---|---|---|
| **control** | lo que `Render()` calcula una vez por bloque de 24 muestras a partir de `pitch_` y `parameter_[]` | `Param(name, fn, count)`: fn(perilla 0…255) → float o tupla; `Param.table()` = 256 × count floats para el ARM, que los escribe con `FCEC` en `C0xx` |
| **audio** | el bucle por muestra | `Component.kernel(k, ins, prm, st)` → `{salida: Value}`; `ins`: entradas (por ejemplo `inc`); `prm[name](i)`: el valor `i` del parámetro; `st[name]`: `Ring` |

Atributos de un componente: `name`, `source` (archivo y función de origen), `params`, `state` (nombres de las
palabras de estado), `inputs`, `outputs` y `reference(note, knobs, n)`.

Un componente puede dar **partes**: métodos con los mismos argumentos que el anfitrión coloca en DSP distintos.
SAW_SWARM tiene `osc()` (DSP A), `hp_f()` y `hp()` (DSP B).

## 6. Verificación

Tres comparaciones independientes:

```
C++ original  ==(bit a bit)==  modelo entero en Python  --(float, sin los fallos del original)-->  port en el emulador
```

### 6.1 Referencia C++ (`reference.py`, `ref/`)

* **Compilación.** `braids_render.cc` se compila con `braids/digital_oscillator.cc` y `braids/resources.cc` del
  checkout, sin cambios. Comando: `g++ -O2 -I sdk/mport/ref -I $EURORACK`. El ejecutable se rehace solo si una
  fuente es más reciente.
* **Uso.** `braids_render SHAPE PITCH P1 P2 N [SEED] [BLOCK] [STRIKE]` escribe N muestras int16 en la salida
  estándar. `BLOCK` vale 24 por defecto, como Braids.
* **`STRIKE=0`.** El estado empieza a 0, con las fases de las sierras en 2³¹ (nuestra fase 0), en vez de con las
  fases al azar del primer render. Hace falta para comparar muestra a muestra. Para llegar al estado privado, sólo
  el programa de referencia usa `#define private public`.
* **Sustitutos de stmlib.** Implementan `CLIP`, `CONSTRAIN`, `Interpolate824/88/1022/115`, `Crossfade`, `Mix` y
  `Random` con la semántica de stmlib: diferencia por fracción y desplazamientos que truncan. Están verificados
  sólo en los caminos de SAW_SWARM, porque el modelo entero coincide bit a bit con el C++ (§6.2). Plaits, Rings y
  Elements necesitan además `stmlib/dsp/*` [abierto].

### 6.2 Modelo entero (`braids_saw_swarm.int_model`)

Es el algoritmo original escrito en Python con enteros: `ComputePhaseIncrement`, la interpolación de los
incrementos, `Interpolate88` / `Interpolate824` y el SVF con `>> 15`. Sirve para dos cosas:
* demostrar que entendimos el original: es **idéntico bit a bit** al C++ en todos los casos probados [sim];
* aislar los fallos del original. `wrap=False` quita el desbordamiento int32 de §7.3, y el port se compara con
  esa versión.

### 6.3 Banco en el emulador (`bench.py`)

Un componente solo en el DSP 0 del núcleo C (`cchain.Slave`, `Model(order=(0,), own_params=(0,))`). Disposición:

| qué | dónde |
|---|---|
| tono | banco 5 `A030`, float32 como lo escribe el ARM |
| parámetros | `C040`… en el orden de `Component.params` |
| estado | banco 0 `0x0100`…, 2 palabras por nombre |
| salidas | banco 5 `A1F0`… |
| `inc` | `Kernel.increment(tono)` |

`bench.run(comp, note, knobs, n, sched=True)` devuelve (muestras, `{'instructions', 'units', 'program'}`). El banco
planifica con `dspsched` **sin** la lista de formas probadas. Por eso su número de instrucciones es una cota
inferior de lo que cuesta en un ejemplo (§7.4).

### 6.4 Métricas (`compare.py`)

| métrica | qué mide | cuándo vale |
|---|---|---|
| `snr_db(port, ref)` | potencia de la referencia sobre la del error, muestra a muestra | en los primeros 10 ms; después las pequeñas diferencias de fase se acumulan |
| `band_levels(x)` | potencia en bandas 20–200–1k–4k–12k–40k Hz | siempre; no depende de las fases |
| `f0`, `rms`, `peak` | fundamental por cruces por cero, nivel | orientativas (los cruces fallan con ondas ricas en armónicos) |

## 7. Componente portado: SAW_SWARM de Braids

Origen: `braids/digital_oscillator.cc`, `DigitalOscillator::RenderSawSwarm` [código]. Port:
`sdk/mport/components/braids_saw_swarm.py`.

### 7.1 Original

* **Por bloque:**
  * `detune = (p1 + 1024)² >> 9`;
  * incremento de la sierra i = `inc(pitch_ + detune·(i − 3)/65536)`, para i = 0…6, interpolado entre pasos de
    1/128 de semitono;
  * `hp_cutoff = pitch_ + ((p2 − 10922)·24 >> 5)` si p2 < 10922, si no `(p2 − 10922)·12 >> 5`, recortado a
    0…32767;
  * `f = lut_svf_cutoff[hp_cutoff]` (2·sin(π·min(fc/fs, 1/8))·32767) y `damp = lut_svf_damp[0] = 65534`.
* **Por muestra:**
  * `phase_i += inc_i`;
  * `sample = −28672 + Σ(phase_i >> 19)`;
  * `sample = Interpolate88(ws_moderate_overdrive, sample + 32768)`;
  * `notch = sample − bp·damp >> 15`; `lp += f·bp >> 15` con `CLIP`; `hp = notch − lp`; `bp += f·hp >> 15`;
  * `out = CLIP(hp)`.
* **Strike:** al empezar una nota, 6 de las 7 fases toman valores al azar.

### 7.2 Port

| parte del original | port |
|---|---|
| incrementos de las 7 sierras | `inc` del centro (del anfitrión) × razón `R_k = 2^(k·detune/(65536·128·12))`, k = −3…−1, 1…3: `Param('ratio', count=6)` |
| suma de sierras | x = Σ p_k / 8, acumulada con `mad x, p, #0.125`; es exactamente `sample / 32768` |
| `ws_moderate_overdrive` | polinomio impar de grado 9 (`WS_DEGREE`), 21,6 LSB de error máximo |
| `lut_svf_cutoff` | u = clamp(inc·H, π·8,18 Hz/fs, π/8) con `H = π·2^(offset/12)/2` (`Param('hp')`); f = (2u − u³/3)·32767/32768 |
| SVF | `notch = mad(x, bp, −65534/32768)`, `lp = clamp(mad(lp, f, bp))`, `hp = notch − lp`, `bp = mad(bp, f, hp)`, `out = clamp(hp)` |
| estado | 7 fases + `lp` + `bp` en anillos del banco 0 |

### 7.3 Diferencias con el original

| diferencia | causa | efecto |
|---|---|---|
| sin strike | las fases siguen corriendo entre notas | sólo la primera nota de cada voz tras el encendido empieza con las fases alineadas |
| sin entrada de sync | el SYSTEM-1 no tiene ninguna fuente que la use | — |
| **no reproduce el desbordamiento int32 de Braids** | en el original `(increment_b − increment_a) · detune_fractional` desborda desde la nota MIDI ≈ 92, según el detune; las sierras graves quedan con incrementos erróneos [código + modelo entero] | en notas agudas el port difiere del C++ (SNR 7,6 dB en la nota 96) y coincide con el modelo sin el fallo (58,3 dB) |
| SVF en float | Braids trunca `f·bp >> 15` | con el paso alto muy abierto (f ≈ 7·10⁻⁴) los graves por debajo de 200 Hz difieren hasta 5 dB |
| `lut_svf_cutoff` exacta | el original interpola linealmente entre semitonos | menos de 0,1 % en f |
| régimen casi inestable | con COLOR alto y notas agudas, f ≈ 0,765 y damp = 2 dejan los polos cerca de −1 | las dos versiones divergen de forma caótica; sólo valen las comparaciones por banda |

### 7.4 Resultados

`python3 sdk/mport/cli.py check saw_swarm` (2026-10-24) [sim]. El SNR se mide en los primeros 10 ms; las bandas
son port − C++ en 20–200–1k–4k–12k–40k Hz:

| nota | ratio / hp (perillas) | C++ = modelo | SNR vs C++ | SNR vs modelo sin desbordar | bandas (dB) |
|---|---|---|---|---|---|
| 45 | 200 / 85 | sí | 41,3 | 41,3 | −0,03 −0,01 0,00 0,00 0,00 |
| 57 | 30 / 170 | sí | 55,9 | 55,9 | −0,69 0,00 −0,01 −0,01 +0,17 |
| 69 | 128 / 128 | sí | 57,4 | 57,4 | −1,43 0,00 −0,03 +0,01 0,00 |
| 69 | 128 / 0 | sí | 31,9 | 31,9 | −5,25 0,00 −0,02 +0,01 0,00 |
| 81 | 255 / 100 | sí | 58,5 | 58,5 | +0,26 0,00 −0,01 0,00 −0,07 |
| 33 | 255 / 255 | sí | 49,0 | 49,0 | −0,02 −0,01 0,00 0,00 0,00 |
| 96 | 128 / 128 | sí | 7,6 | 58,3 | +9,38 +8,30 −1,07 −0,02 −0,02 |

Coste por voz:

| contexto | unidades | instrucciones |
|---|---|---|
| banco, todo en un DSP, fusión sin restricciones | 86 | 48 |
| MUTABLE-SYSTEM, DSP A (incremento + sierras + waveshaper + nivel + trémolo), sólo formas probadas | — | ≈ 46 |
| MUTABLE-SYSTEM, DSP B (coeficiente + SVF) | — | ≈ 9 |

## 8. Línea de comandos (`cli.py`)

```sh
python3 sdk/mport/cli.py list                                        # componentes portados
python3 sdk/mport/cli.py check saw_swarm [-n 9600] [--window 960]    # tabla de §7.4 (≈ 1,4 s)
python3 sdk/mport/cli.py lines saw_swarm [--plain]                   # programa del banco, planificado o no
python3 sdk/mport/cli.py lut braids ws_moderate_overdrive --odd 9 --xmax 0.875
python3 sdk/mport/cli.py lut braids lut_svf_cutoff --kind 824 --deg 5 --xmin 0 --xmax 1
```

`check` recorre los casos de `CASES` en `cli.py`, uno por fila. Para cada caso compila la referencia (la primera
vez), corre el banco y, si el componente tiene `model()`, comprueba que C++ = modelo.

## 9. Pruebas

```sh
python3 -m unittest discover -s sdk/mport/tests        # 7 pruebas, ≈ 1,2 s
```

| prueba | qué comprueba |
|---|---|
| `test_expressions` | un componente de expresiones arbitrarias da en el emulador lo mismo que en Python (asignación de registros, `sub`, `mad`, `ymac`, `min`/`max`, `wrap`, anillo) |
| `test_out_of_registers` | `KernelError` cuando no quedan registros |
| `test_accumulator_copy` | `mad` sobre un valor que se vuelve a leer trabaja sobre una copia |
| `test_sched_equals_plain` | el programa planificado da las mismas muestras que el sin planificar, en menos instrucciones |
| `test_overdrive_fit` | el ajuste de grado 9 de `ws_moderate_overdrive` tiene menos de 25 LSB de error |
| `test_int_model_is_the_cpp` | modelo entero = C++ bit a bit (4 casos; se salta sin g++ o sin el checkout) |
| `test_port_close_to_original` | SNR > 40 dB en 10 ms frente al modelo sin desbordamiento, bandas 200 Hz–4 kHz a ±0,5 dB, ≤ 50 instrucciones |

## 10. Cómo portar un componente nuevo

1. **Leer el original.** Separar lo que se calcula por bloque de lo que se calcula por muestra, y anotar las
   tablas y el estado. Antes de inventar un mecanismo, mirar cómo lo hace Roland (regla del proyecto).
2. **Referencia.** Añadir el componente a `ref/` y, si hace falta, a los sustitutos de stmlib. Escribir
   `reference()`.
3. **Modelo entero o float del original** (recomendado). Confirmar que coincide bit a bit con el C++ y anotar en
   §7.3 los fallos del original que no se van a copiar.
4. **`Param` para cada valor de control.** Calcularlo como lo hace el original, en Python.
5. **Tablas por muestra a polinomio** con `lut.fit_odd` / `fit`. Elegir el grado según el error en LSB.
6. **`kernel()` con `Kernel`**, separado en partes si va a ir en dos DSP.
7. **Registrarlo** en `components/__init__.py` y añadir sus casos a `CASES` de `cli.py`.
8. **Medir.** `cli.py check`, luego `cli.py lines` (coste) y las pruebas.
9. **Ejemplo.** Integrarlo en una copia de un ejemplo, como `examples/mutable_system`: `--sim`, `--pack-check`,
   `--sim-check` si aparece una forma de instrucción nueva, `check_optimized` y build. Una sola cosa nueva por
   instalación.

## 11. Abierto

* **Derrame de registros.** Hoy un kernel con más de 6–8 valores vivos falla (`KernelError`). Haría falta guardar
  en palabras de memoria de la misma muestra.
* **Índice calculado** (tablas leídas por fase, retardos variables) en el SYSTEM-1: paso 15Y de C-S1SY. Bloquea
  wavetables, Karplus, comb y chorus.
* **Ops de la SFU** (recíproco, raíz, log, exp por semilla): no están en `Kernel`, porque no se han probado en el
  SYSTEM-1.
* **Formas fusionadas.** `ph_wrap` no aparece en `asm/proven_forms.txt`. Si una build con `SCHED` sin restricciones
  funciona en el equipo, el DSP A de MUTABLE-SYSTEM bajaría de 638 a 598 instrucciones.
* **stmlib.** Sustitutos de `stmlib/dsp/*` para referencias de Plaits, Rings y Elements, o clonar el submódulo.
