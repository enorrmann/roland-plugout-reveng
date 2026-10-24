# s1cc: compilador de C / C++ al microcódigo de los BMC de Roland

Especificación. Estado: 2026-10-24, versión 1.0.

s1cc traduce código C o C++ de una voz de sinte al microcódigo de los DSP del SYSTEM-1 (el ISA es el mismo en el
SYSTEM-8). El código puede ser propio o copiado de otro proyecto: Mutable Instruments, stmlib, cualquier DSP portable.
No hace falta reescribir el algoritmo:
* clang compila el código tal cual a LLVM IR;
* s1cc (C++, API de LLVM 19) traduce ese IR;
* la misma fuente se compila además en nativo, como referencia, y `s1cc check` compara las dos versiones en el
  emulador del repositorio.

Reemplaza como herramienta de port a `sdk/mport` ([MUTABLE_SDK.md](MUTABLE_SDK.md)). Allí los componentes se
reescribían a mano en Python; mport queda como estudio y como banco de SAW_SWARM.

Etiquetas: **[medido]** en el equipo; **[sim]** en el emulador (núcleo C); **[abierto]**.

## 1. Uso

```sh
make -C sdk/s1cc                                                     # compila s1cc (necesita LLVM 19)
sdk/s1cc/s1cc build VOZ.cc [OTRA.cc ...] [-I DIR] [-o SALIDA]        # -> SALIDA/voice.s1k (+ informe)
sdk/s1cc/s1cc check SALIDA [--note 57] [nombre=perilla ...]          # emulador contra la referencia nativa
KERNEL=SALIDA sh examples/mutable_system/build.sh                    # la voz en el sinte completo (§12)
```

Ejemplo, SAW_SWARM de Braids a partir del C++ original:

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/braids_saw_swarm_s1.cc ~/git/eurorack/braids/resources.cc \
    -I sdk/mport/ref -I ~/git/eurorack
sdk/s1cc/s1cc check sdk/s1cc/build/braids_saw_swarm_s1 --note 45 ratio=200 hp_h=85
#   stage A: 74 units -> 49 instructions; SNR 57.6 dB (10 ms) ...; bands port - ref (dB): -0.00 +0.00 ...
#   stage B: 31 units -> 26 instructions; SNR 41.3 dB (10 ms) ...
```

`build` admite además `--tol X` (tolerancia de los ajustes, métrica de rango, por defecto 3·10⁻³),
`--tollog X` (métrica en octavas, por defecto 0,005), `--maxdeg N` (grado máximo de un polinomio, 11) y
`--regs LISTA` (registros que puede usar el kernel; por defecto 0,1,2,3,4,7, porque r5/r6 son del programa anfitrión
de C-S1SY).

Dependencias:
* clang / clang++ y LLVM 19 (`llvm-link`, `llvm-config`, `libLLVM`);
* `make`;
* `check` usa Python 3 con numpy, porque el ensamblador, el planificador y el emulador del repositorio están en Python.

## 2. Archivos

| archivo | lenguaje | qué es |
|---|---|---|
| `sdk/s1cc/include/s1dsp.h` | C / C++ | el SDK: modelo de programa e intrínsecos (§3) |
| `sdk/s1cc/src/s1cc.h` | C++ | estructuras comunes: grafo, perfil, ajustes, kernel |
| `sdk/s1cc/src/graph.cpp` | C++ | IR → grafo de datos (§4) |
| `sdk/s1cc/src/prof.cpp` | C++ | instrumentación para el perfilado y clones de las islas (§5) |
| `sdk/s1cc/src/fit.cpp` | C++ | ajuste de funciones (§6) |
| `sdk/s1cc/src/lower.cpp` | C++ | grafo → kernel: representación de los valores y lowering (§7) |
| `sdk/s1cc/src/kernel.cpp` | C++ | valores → registros (con derrame) → líneas de una unidad (§7.5) |
| `sdk/s1cc/src/main.cpp` | C++ | `s1cc prof` / `lower` / `init` / `graph` |
| `sdk/s1cc/runtime/s1rt.c` | C | runtime nativo: intrínsecos en el PC, tablas, referencia, escenarios de perfilado |
| `sdk/s1cc/s1cc` | sh | driver: `build`, `check` (§9) |
| `sdk/s1cc/Makefile` | make | compila `build/s1cc` |
| `sdk/s1cc/tools/s1kernel.py` | Python | lee un `.s1k` y lo coloca en direcciones reales (ejemplos, `check`) |
| `sdk/s1cc/tools/s1check.py` | Python | `s1cc check` (§10) |
| `sdk/s1cc/examples/` | C / C++ | `braids_saw_swarm.cc`, `braids_saw_swarm_s1.cc`, `va_saw_svf.c` (§11) |
| `sdk/s1cc/tests/test_s1cc.py` | Python | pruebas de punta a punta (§13) |
| `sdk/s1cc/build/` | — | compilados y salidas (no está en git) |

## 3. Modelo de programa (`s1dsp.h`)

```c
#include "s1dsp.h"

S1_PARAM(cutoff, 0x168, 1) {               /* perilla OSC1 COLOR -> tabla: corre en el PC, knob = 0..255 */
    out[0] = tanf(3.14159265f * 50.0f * powf(240.0f, knob / 255.0f) / 96000.0f);
}

static float phase, ic1, ic2;              /* estado de la voz: una palabra de anillo cada una, 8 copias */

S1_VOICE {                                 /* DSP A del esclavo, cada muestra */
    phase += s1_increment() * 0.5f;
    if (phase >= 1.0f) phase -= 1.0f;
    return 2.0f * phase - 1.0f;
}

S1_VOICE_B {                               /* opcional: DSP B; x = la señal de la voz del anfitrión */
    float g = s1_param(cutoff, 0);
    ...
}
```

| elemento | regla |
|---|---|
| `S1_VOICE` | obligatorio; corre en el DSP A del esclavo; devuelve la señal de la voz (≈ −1…1) |
| `S1_VOICE_B` | opcional; corre en el DSP B; `x` = la señal de la voz después del nivel y del trémolo del anfitrión |
| variables de archivo no `const` | estado de la voz (int o float; los arrays con índice constante sirven, clang los separa). Una variable sólo puede usarla una de las dos funciones |
| arrays `const` | tablas: con índice constante se pliegan; con índice calculado pasan a una isla (§4.3) |
| `S1_PARAM(nombre, panel_id, n)` | tabla de perilla: la función corre en el PC para knob = 0…255 y escribe `out[0..n-1]`; el ARM la escribe como parámetros `C0xx` del DSP cuando se mueve el control `panel_id` |
| bucles | sólo con número de iteraciones constante (clang los desenrolla) |
| llamadas | sólo código que se inline-a (en el mismo archivo o en headers) y los intrínsecos; libm: §7.3 |

Intrínsecos:

| función | qué es |
|---|---|
| `s1_pitch()` | tono de la voz en octavas (4,75 = La 440), con el LFO y el range del anfitrión |
| `s1_increment()` | paso de fase de ese tono para una fase −1…1: 2·f/96000 |
| `s1_gate()`, `s1_trig()` | puerta 0 / 1; disparo: alterna 0 / 1 en cada comienzo de nota |
| `s1_param(nombre, i)` | valor i de una tabla `S1_PARAM` |
| `s1_export(i, v)` / `s1_import(i)` | un valor de `S1_VOICE` (DSP A) a `S1_VOICE_B` (DSP B), en la misma muestra |
| `s1_wrap(x)`, `s1_clamp(x)`, `s1_tri(x)` | x envuelto a [−1, 1) / recortado a [−1, 1] / 1 − 2·\|wrap(−x − 1,5)\|: una op cada uno (`ph_wrap` / `ph_clamp` / `ph_tri`) |
| `s1_exp2(x)` | 2ˣ (`exp2` del DSP: 10 ops más las cuadraturas que pida el rango) |
| `s1_range(x, lo, hi)` | x, declarando su rango (cuando el perfilado no lo ve) |

## 4. Del IR al grafo (`graph.cpp`)

### 4.1 Funciones, bloques y memoria

* **Módulo.** Se lee el módulo enlazado (`llvm-link` de todas las fuentes, para tener los inicializadores de las
  tablas), compilado con `clang -O2`.
* **Bloques.** Los bloques de `s1_voice` / `s1_voice_b` se recorren en orden topológico. Un bucle (arista hacia
  atrás) es un error.
* **If-conversion.** Cada bloque tiene un predicado: la suma de los predicados de sus aristas de entrada, que son
  excluyentes. Cada arista toma el predicado de su bloque por la condición del salto (o su negación). Un `phi` se
  convierte en una cadena de `select`. Un `store` en un bloque condicional pasa a ser `estado = select(predicado,
  nuevo, anterior)`.
* **Estado.** Una palabra de estado es (variable global, desplazamiento). La primera carga lee el valor de la muestra
  anterior; las siguientes leen el último valor guardado en la muestra (reenvío en el grafo). El valor final se guarda
  en el anillo, siempre: un anillo que no se reescribe pierde el valor.
* **Tablas.** Una carga de una global `const` con desplazamiento constante se pliega (`ConstantFoldLoadFromConst`).
  Con un índice calculado se marca para una isla.

### 4.2 Signo de los enteros

Un entero de LLVM no tiene signo: su vista depende del uso.
* **Regla estructural.** Es sin signo si lo lee o lo produce una operación sin signo (`lshr`, `zext`, `uitofp`,
  `udiv`, `urem`, `umin`/`umax`, `icmp u*`, `fptoui`). La marca se propaga hacia los operandos a través de `select`,
  `shl`, `add`, `sub`, `and`, `or`, `xor`, `trunc` y `mul`, y entre una palabra de estado y su valor final.
* **Perfil.** En `lower.cpp` (`unsignedView`), para `add` / `sub` / `mul` / `shl` decide la vista en la que la
  operación de C no desbordó: `p>>19 − 28672` desborda sin signo y no con signo, así que es con signo. Si desbordó en
  las dos vistas (la fase de un oscilador) o en ninguna, decide la regla estructural. Para el resto, la vista con
  signo sólo gana si su rango es mucho más estrecho.

### 4.3 Islas

Algunos nodos no se pueden ejecutar tal cual en el DSP: lecturas de tabla con índice calculado, punteros
calculados, funciones desconocidas, divisiones o restos enteros por variables, `or`, máscaras no contiguas. Para cada
uno se buscan raíces: los valores que cortan todos sus caminos hasta las fuentes (dominadores en el DAG de
dependencias; las fuentes son entradas, parámetros y estado), primero el más alto.

* **Isla.** Una isla con raíz R son los nodos que dependen sólo de R (y de constantes). Su salida es la función
  y = F(R).
* **Clones.** `s1cc prof` copia la isla como una función nativa `double __s1p_isl_<k>(double x)`. El runtime la
  muestrea en el rango observado de R, con 2049 puntos.
* **Elección.** El lowering ajusta F (§6) para cada raíz candidata y se queda con la primera que cumple la
  tolerancia, o con la de menor error (con un aviso).

Ejemplo [sim, 2026-10-24]: en SAW_SWARM la isla de `Interpolate88(ws_moderate_overdrive, sample + 32768)`, que clang
deja en `lshr`/`and`/`xor`/dos lecturas/interpolación, tiene raíz `sample`. Sale un polinomio de grado 7, con un
error de 1,2·10⁻³ del rango, en 7 ops.

### 4.4 Valores especulados

* **El problema.** Clang convierte en `select` los `if` pequeños y calcula las dos ramas en todas las muestras. Un
  valor que sólo alimenta una rama (por ejemplo el `t/dt` del polyBLEP) toma entonces, en las muestras en que no se
  usa, valores enormes: hasta 10⁸.
* **Detección (`speculation`).** Se marcan esos conos: nodos cuyos usuarios están todos dentro del cono, o son el
  `select` por esa rama.
* **Perfilado.** El perfil registra además su rango sólo cuando se toma la rama (`__s1p_c`).
* **Lowering.** Usa ese rango y limita a él los nodos de entrada del cono (min/max). Sin esto el polyBLEP salía con
  ~20 dB de SNR; con esto, con 68–81 dB [sim].

## 5. Perfilado (`prof.cpp`, `runtime/s1rt.c`)

`s1cc prof` inserta, después de cada valor de las funciones de voz:
* `__s1p_v(id, vista con signo, vista sin signo)`: rangos;
* en `add` / `sub` / `mul` / `shl` enteros, `__s1p_o(id, desbordó con signo, desbordó sin signo)`: el resultado
  exacto en 64 bits comparado con el de la operación;
* en los `select`, `__s1p_c(id, rama tomada, valor)` para los valores especulados.

Agrega también los clones de las islas (`__s1p_isl_tab`) y `__s1p_reset()`, que devuelve el estado a sus valores
iniciales.

Escenarios (`s1rt.c prof`):
* cada tabla `S1_PARAM` recorrida de 0 a 255 en pasos de 17, con las otras en 128;
* 24 juegos de perillas al azar;
* todas a 0 y todas a 255.

Cada uno sobre 14 notas de 0 a 127, 2400 muestras por nota, con la puerta cerrándose a mitad.

## 6. Ajuste de funciones (`fit.cpp`)

El dominio se lleva a t = ta·V + tb ∈ [−1, 1], con V el valor de la raíz. Se prueban, del más barato al más caro,
los modelos que cumplen la tolerancia:

| modelo | coste (ops) | cuándo |
|---|---|---|
| P(t), grado 1…`maxdeg` | grado + 1, más 0–2 de la transformación de t | siempre |
| P impar (t·Q(t²)) | (grado − 1)/2 + 3 | dominio y función simétricos |
| sign·2^Q(t), Q de grado 1…6 | grado + 11 | salida de un solo signo (frecuencias, coeficientes) |
| + recorte a las mesetas | + 1 por extremo | ≥ 2 % de las muestras en el mínimo o el máximo en un extremo (CLIP, fin de tabla) |

* **Mínimos cuadrados.** En la base de Chebyshev, convertidos a potencias de t.
* **Métrica.** Percentil 99,5 del error, para que una esquina de tabla no decida el ajuste; también se informa el
  máximo.
* **Métrica de rango.** |error| / rango de la salida, con tolerancia `--tol`.
* **Métrica en octavas.** Si la salida es positiva y cubre más de 3 octavas (un coeficiente, una frecuencia):
  |log2(ajuste / y)| sobre las muestras ≥ máx/64, con tolerancia `--tollog`.

## 7. Lowering (`lower.cpp`)

### 7.1 Representación de los valores

Cada valor V del programa vive en un registro r con una escala s y un desplazamiento o: **V = (r − o)·s**.
* **Lo que cuesta cero.** Un desplazamiento de bits o un factor constante cambia s; sumar una constante cambia o.
  Con eso el punto fijo de los enteros (productos Q15, `>> 15`, `+ 32768`, la fase `uint32` de un oscilador) no gasta
  instrucciones.
* **Normalización.** Se mantiene |r| ≤ 16, y se lleva a ≤ 1 cuando se pasa (1 op). También se reescala hacia arriba
  si |r| < 2⁻⁸: por debajo de 0,125 la rejilla del DSP es 2⁻³² absoluta y un registro diminuto pierde precisión.
* **Escala de un parámetro.** Es la potencia de 2 que cubre su tabla; el ARM escribe valor / escala.
* **Escala de una palabra de estado.** También una potencia de 2, salvo:
  * entero sin signo que recorre todo su rango (una fase): s = 2ⁿ⁻¹, o = −1, es decir r ∈ [−1, 1), listo para
    `ph_wrap`;
  * con signo en todo su rango: s = 2ⁿ⁻¹, o = 0.

### 7.2 Operaciones

| C / IR | microcódigo |
|---|---|
| `a + b`, `a - b` | r = rₐ + q·r_b, con q = s_b / sₐ (`addxy` / `subxy` / `mad`); o = oₐ + q·o_b. Si va a un wrap, la base es el operando con la escala del wrap |
| `a + c`, `a * c`, `a << n`, `a >> n`, `a / c` | sólo cambian s y o |
| `a * b` | materializa o = 0 en los dos (0–2 ops) y un `mul` |
| desborde de `add`/`sub`/`shl` en C (visto en el perfil) | V mod 2ⁿ = 2ⁿ⁻¹·(wrap(V/2ⁿ⁻¹ − 1) + 1): `ph_wrap` más 0–2 ops; con signo, wrap(V/2ⁿ⁻¹) |
| desborde de `mul` en C | aviso; el port calcula sin envolver (en el código real casi siempre es un fallo, como el de Braids, [MUTABLE_SDK.md](MUTABLE_SDK.md) §7.3) |
| `x & (2ᵐ − 1)`, máscara contigua, `x ^ 2ᵇ`, `~x` | mod 2ᵐ; resta de dos mod; + 2ᵇ mod 2ᵇ⁺¹; −x − 1 |
| `a < b` y demás comparaciones | `ph_clamp(K·(a − b − θ))` en etapas de ganancia ≤ 16. Enteros: θ = ±0,5, exacto. Floats: hasta 2¹⁷ / rango |
| `c ? a : b` | b + c·(a − b); con brazos constantes, afín en c (0 ops) |
| `x < T ? x : x - K` (y la forma `>=`) | detectado: (T − K) + mod(x − (T − K), K), exacto, si el perfil deja x en [T − K, T + K) |
| `min`, `max`, `CLIP`, `smin`/`smax` | 1 op por extremo |
| `fabs`, `abs` | `max r, r, -r` |
| `floor` | V − mod(V, 1) |
| casts | sin coste. Un `trunc` que envuelve pasa a mod; una constante leída con signo se corrige en `zext` |

### 7.3 Funciones y divisiones

| función | cómo |
|---|---|
| `exp`, `exp2`, `pow(c, x)` | `exp2` del DSP (exacto salvo el polinomio de Taylor: 1,2 cents a \|x\| = 8), centrado en el rango del argumento |
| `x / y`, `1/y` | si y viene de un `exp2` (`s1_increment`, frecuencias): 2^(−argumento), exacto. Si no: semilla ajustada (grado ≤ 4) y pasos de Newton b ← b·(2 − y·b) hasta un error relativo < 10⁻⁷ |
| `sin`, `cos`, `tan`, `tanh`, `sinh`, `cosh`, `log`, `log2`, `log10`, `sqrt`, `atan`, `asin`, `acos`, `cbrt`, `pow(x, c)` | ajuste analítico en el rango perfilado del argumento (§6). El argumento puede depender de cualquier número de valores |
| otras | isla (§4.3) |

### 7.4 Kernel (`.s1k`)

```text
.state <i> <A|B> <s> <o> <nombre>    palabra de estado i: V = (r - o) s
.param <nombre> <n> <escala>         tabla S1_PARAM (el ARM escribe valor / escala)
.export <i> <s> <o>                  s1_export i y su representación
.inputb <entrada>                    entrada que B necesita de A (A la copia)
.stage A <unidades>
.scratch <n>                         palabras temporales del derrame
<líneas de una unidad con marcadores {IN:pitch} {IN:gate} {IN:trig} {S<i>} {S<i>+1} {P:<nombre>:<i>}
 {X:<i>} {INB:<entrada>} {T<k>} {XIN} {OUT}>
.stage B <unidades>
...
.end
```

El anfitrión cambia cada marcador por una dirección (`tools/s1kernel.py place`) y planifica con `asm/dspsched.py`.

### 7.5 Registros (`kernel.cpp`)

* **Asignación.** Barrido lineal; el resultado puede tomar el registro de un operando que muere en la misma
  instrucción. Un acumulador de `mad`/`ymacd` que se vuelve a leer se copia antes.
* **Derrame.** Si no alcanzan los registros, el valor cuyo próximo uso está más lejos se guarda en `{T<k>}` y se
  vuelve a cargar antes de usarlo. Es una palabra del banco 0 que se escribe y se lee en la misma muestra.
* **Plegado de constantes.** Dos multiplicaciones por constante seguidas se pliegan en una. Un inmediato fuera de
  [2⁻¹², 256] que no es potencia de 2 se parte en pasos.
* **Limpieza.** Se eliminan los resultados que nadie usa, y se pone un relleno `mov rS, rS` tras una carga cuyo
  registro usa la línea siguiente.

## 8. Diferencias con el C, por diseño

| tema | comportamiento |
|---|---|
| estado inicial | el anillo empieza en 0, es decir V = −o·s. Para una fase con o = −1 eso es medio ciclo (2³¹), no 0. Aviso por palabra. `s1cc init` hace la referencia `refdsp` que arranca igual, para `check` |
| redondeos | los `>> n` y las conversiones a entero no redondean: el DSP calcula en reales, con un error ≤ 1 LSB del entero original |
| comparaciones de floats | exactas salvo en una franja de 2⁻¹⁷ del rango alrededor del umbral. El wrap de fase se reconoce y es exacto |
| desbordes de productos en C | no se reproducen (aviso) |
| funciones ajustadas | error ≤ tolerancia en el 99,5 % del rango perfilado; fuera del rango perfilado, sin garantía (`s1_range`) |

## 9. Driver (`s1cc build`)

1. `clang(++) -O2 -S -emit-llvm -fno-math-errno -I include ...` de cada fuente, y `llvm-link` → `voice.ll`.
2. `s1cc prof voice.ll -o prof.ll`.
3. `clang voice.ll runtime/s1rt.c -o ref` y `clang prof.ll runtime/s1rt.c -o prof`, nativos.
4. `ref tables tables.txt` (tablas `S1_PARAM`) y `prof prof profile.txt` (escenarios, rangos, desbordes, islas).
5. `s1cc lower voice.ll --profile profile.txt --tables tables.txt -o voice.s1k --report report.txt`.
6. `s1cc init voice.ll --kernel voice.s1k -o refdsp.ll` y `clang ... -o refdsp`: la referencia que arranca del
   estado del DSP.

`report.txt` incluye las unidades por etapa, los derrames, cada isla y ajuste (modelo, grado, error, coste), las
candidatas descartadas y los avisos.

## 10. Verificación (`s1cc check`)

* **Banco.** DSP 0 del emulador = etapa A, DSP 1 = etapa B, un banco 5 compartido como en el par de esclavos.
* **Entradas.** Tono en `A030`, puerta `A038` = 1, disparo `A060` = 1.
* **Conexiones.** La salida de A va a `A151`, que es la entrada de B.
* **Memoria.** Estado desde `0x0100` y temporales desde `0x0300` en el banco 0 de cada DSP; parámetros desde
  `C040`.
* **Planificación.** Con las formas probadas (`--all-forms` para todas).
* **Comparación.** Contra `refdsp`: SNR en los primeros 10 ms (muestra a muestra) y en todo, niveles por banda e
  instrucciones.

## 11. Ejemplos [sim, 2026-10-24]

| ejemplo | qué prueba | A (instr.) | B (instr.) | SNR 10 ms A / B |
|---|---|---|---|---|
| `braids_saw_swarm.cc` | el bucle original de RenderSawSwarm (enteros, fases `uint32`, `Interpolate88`, SVF Q15, `lut_svf_cutoff` por isla) | 49 | 50 | 55–58 / 42–49 dB |
| `braids_saw_swarm_s1.cc` | lo mismo, con el coeficiente del paso alto desde el incremento exportado (cabe en 8 voces) | 49 | 26 | 55–58 / 41–49 dB |
| `va_saw_svf.c` | float: polyBLEP (`if`, divisiones por `dt`), SVF de Simper con `tanhf`, tablas con `powf`/`tanf` | 92 | 38 | 68–81 / 53–70 dB |

El SNR total (100 ms) es menor: las fases del original en float32 y las del DSP se separan poco a poco por el
redondeo. Los niveles por banda coinciden a ±0,05 dB de 200 Hz a 12 kHz. Excepción: con el paso alto de Braids muy
abierto, los graves por debajo de 200 Hz difieren ~5 dB, por los truncamientos enteros del original.

## 12. En el sinte (`examples/mutable_system`, `KERNEL=`)

* **Build.** `KERNEL=<salida de s1cc> sh examples/mutable_system/build.sh` cambia el oscilador de C-S1SY13 por la
  etapa A del kernel y pone la etapa B al principio de la voz del DSP B.
* **Perillas.** Cada `S1_PARAM` queda en la perilla de su `panel_id` (tablas `kparam_*`, `engine.c kparam_set`).
* **Direcciones.** [examples_mutable_system_README.md](examples_mutable_system_README.md) §8.

Con `braids_saw_swarm_s1.cc` [sim]:
* DSP A 626 y DSP B 620 instrucciones. La versión a mano en Python usaba 638 / 564; los máximos probados son
  682 / 708.
* `--pack-check` y `--sim-check` idénticos en 7 paneles.
* La salida del sinte coincide con la de la versión a mano a ±0,03 dB por banda.

## 13. Pruebas

```sh
python3 -m unittest discover -s sdk/s1cc/tests      # ≈ 8 s: build + check de los tres ejemplos, umbrales de §11
```

## 14. Abierto

* **Ningún kernel probado en el equipo.** El de SAW_SWARM está listo para instalar (§12).
* **Bucles de trip variable.** Daisy-chains, búsquedas: hoy son un error.
* **Retardos y tablas leídas por fase en memoria.** Esperan al índice calculado del SYSTEM-1 (paso 15Y de C-S1SY);
  hoy las tablas pasan por islas, y las islas sólo sirven para funciones de un valor.
* **Estado inicial exacto.** Opción para generar el código que pone el estado inicial del C (una bandera y un `mad`
  por palabra).
* **Costes.** Compares de float más baratos con los flags del DSP (`X9bY`, probados en el SYSTEM-8, no en el
  SYSTEM-1) y fusión de más formas (lista de formas probadas).
* **stmlib.** Sustitutos de `stmlib/dsp/*` (filter, polyblep, parameter_interpolator) para compilar Plaits, Rings o
  Elements tal cual.
