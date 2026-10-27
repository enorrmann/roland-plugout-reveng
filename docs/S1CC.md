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

### 9.1 Depuración (`--debug`)

`s1cc build ... --debug` (o `S1CC_DEBUG=1`) cambia tres cosas:

* **Información de depuración.** Las fuentes se compilan con `-g`. Cada error y aviso nombra el archivo, la línea y
  la función C / C++ de la instrucción, y la cadena de inlining hasta la voz:
  `digital_oscillator.cc:306 in braids::DigitalOscillator::RenderToy, inlined at braids_wrapper_digital.cc:28 in
  s1_voice`. Sin `--debug` el error muestra la instrucción del IR y la función de la voz.
* **Diagnóstico previo.** Antes del perfilado, `s1cc diag linked.ll --remarks inline.txt` recorre `s1_voice` y
  `s1_voice_b` y lista todas las funciones C que no puede compilar, cada una con su motivo y sus lugares. La lista
  queda en `SALIDA/diag.txt` y también sale por stderr con el prefijo `s1cc: debug:`. Así se ven todos los
  problemas aunque el build se detenga en el primero.
* **Motivos de LLVM.** `opt` corre con `-pass-remarks-missed=inline`. `SALIDA/inline.txt` guarda por qué el
  inliner dejó cada llamada, y `diag` agrega ese motivo a la función.

Qué detecta `diag` (son las mismas reglas que usa el constructor del grafo, `graph.cpp` `callProblem` /
`instProblem`):

| problema | motivo que informa |
|---|---|
| llamada a una función sin cuerpo | el archivo que la define no está entre las fuentes, o es una función de biblioteca que s1cc no conoce |
| llamada a una función definida pero no inlineada | `noinline`, `optnone`, variádica, recursiva, o el motivo del inliner |
| llamada indirecta | puntero a función o método virtual |
| intrínseco de LLVM sin operación | su nombre (`llvm.*`) |
| `switch`, `invoke` y otras instrucciones | la instrucción no está soportada (un `switch` se escribe con `if` / `?:`) |
| `bitcast`, `ptrtoint`, `inttoptr`, `addrspacecast` | puntero usado como número o reinterpretación de tipo |

Al final, `diag` también muestra las funciones C que sí quedaron inlineadas en la voz (`inlined into s1_voice:`).
El cuerpo de una función no inlineada también se revisa, porque sus problemas aparecerán cuando se inlinee.
`diag` no ve los errores que dependen del grafo o del perfil (rangos, islas con varias entradas, punteros
calculados). Esos siguen deteniendo el build en `lower`, pero con la ubicación C de arriba.

Ejemplo, la envoltura de Braids sin `digital_oscillator.cc` entre las fuentes:

```
s1cc: debug: S1_VOICE (s1_voice): 1 C function(s) s1cc cannot compile
s1cc: debug:   braids::DigitalOscillator::RenderToy
s1cc: debug:     E101: not inlined: it has no body in the module: the file that defines it is not among the build's sources (add it), or it is a library function s1cc does not know
s1cc: debug:       at sdk/s1cc/examples/braids_wrapper_digital.cc:28 in s1_voice
s1cc: debug:   inlined into s1_voice: braids::DigitalOscillator::set_parameters
```

Con `--debug`, cada error termina con una referencia a su código en §9.2.

`S1CC_DEBUG=2` además vuelca cada nodo del grafo y su representación tras el lowering (`n12 fmul: %3 s ... V [...]`).
`-g` no cambia el kernel: con y sin `--debug` sale el mismo `voice.s1k` (comprobado con `va_saw_svf.c` y
`braids_saw_swarm_s1.cc`, 2026-10-27).

### 9.2 Errores y cómo resolverlos

Cada error y cada aviso tiene un **código** fijo: `s1cc: error E501: ...`, `s1cc: warning W905: ...`. El
código identifica el caso exacto; el texto que sigue cambia con los nombres y valores. Para resolverlo, buscar el
código en las tablas de abajo (`grep E501 docs/S1CC.md`). Los errores detienen el build; los avisos no, y quedan
también en `report.txt` con su código. Con `--debug` (§9.1), cada mensaje lleva `archivo:línea in función` con la
cadena de inlining, y cada error termina con `s1cc: debug: cause and fix of E501: docs/S1CC.md §9.2`.

| rango | etapa | § |
|---|---|---|
| `E0xx` | archivos, estructura del programa, clang y enlazador (driver) | 9.2.0, 9.2.1 |
| `E1xx` | llamadas, inlining, intrínsecos `s1_*` | 9.2.2 |
| `E2xx` | instrucciones | 9.2.3 |
| `E3xx` | memoria y estado | 9.2.4 |
| `E4xx` | bucles | 9.2.5 |
| `E5xx` | islas y funciones | 9.2.6 |
| `E6xx` | operaciones | 9.2.7 |
| `E7xx` | kernel (inmediatos, registros) | 9.2.8 |
| `W9xx` | avisos | 9.2.9 |

Los códigos son estables: un código no cambia de significado; un caso nuevo recibe un número nuevo. Los de
`E1xx` y `E2xx` también salen en la lista de `--debug` (`E101: not inlined: ...`). En el código fuente, cada
`fail("Exxx", ...)` / `warn("Wxxx", ...)` lleva el suyo.

#### 9.2.0 Archivos y estructura (`main.cpp`, `graph.cpp`)

| código | mensaje | causa | solución |
|---|---|---|---|
| `E001` | `cannot read IN.ll` | el IR no existe o no es válido (LLVM imprime el detalle antes) | correr `s1cc build`, no los subcomandos a mano; comprobar LLVM 19 |
| `E002` | `cannot read the tables` / `bad tables file` | `tables.txt` falta o está truncado | rehacer el build completo |
| `E003` | `cannot write F` | directorio de salida sin permisos o sin espacio | otro `-o` |
| `E004` | `init: state 'v' not in ORIG.ll` | `voice.ll` y `linked.ll` no coinciden (archivos de builds distintos) | rehacer el build completo |
| `E005` | `cannot read the profile` | el binario `prof` no corrió o falló | rehacer el build; si falla, correr `SALIDA/prof prof x.txt` y ver su error |
| `E007` | `pass pipeline: ...` | error interno de LLVM al preparar los bucles | informar |
| `E008` | `no S1_VOICE (s1_voice) in the module` | falta `S1_VOICE`, o se definió dentro de un `namespace` / sin `#include "s1dsp.h"` | definir `S1_VOICE { ... }` a nivel de archivo |
| `E009` | `s1_voice returns nothing` | la voz nunca llega a un `return` con valor | devolver la muestra en todas las ramas |

#### 9.2.1 Antes de s1cc: clang y el enlazador

| código | mensaje | causa | solución |
|---|---|---|---|
| `E010` | errores de `clang` / `clang++` (`error: unknown type name`, `file not found`) | el C no compila como C nativo | corregirlo como cualquier error de compilación. Falta un `-I` o falta definir una macro (`-DTEST` lo pone el driver si existe `build/deps/stmlib`) |
| `E011` | `undefined reference to 'f'` (enlace de `trips`, `ref` o `prof`) | `f` se llama y no está definida en ninguna fuente | agregar al build el `.c` / `.cc` que la define. `--debug` ya la lista antes como `it has no body in the module` |

#### 9.2.2 Llamadas (`graph.cpp`, también en `diag`)

La voz se convierte en una sola función sin llamadas: todo lo que llama tiene que quedar inlineado. Sólo pueden
quedar llamadas a los intrínsecos `s1_*`, a libm (§7.3) y a intrínsecos de LLVM con operación propia.

| código | mensaje | causa | solución |
|---|---|---|---|
| `E101` | `call to 'f' not inlined: it has no body in the module` | `f` está declarada (header) pero su archivo no está entre las fuentes. O es una función de biblioteca que s1cc no conoce (`rand`, `printf`, `memcpy` con tamaño variable) | agregar la fuente. Si es de biblioteca, reemplazarla: ruido con un LCG en una variable de estado, sin `printf` |
| `E102`, `E103` | `... it is marked noinline` / `optnone` | atributo en la fuente (`__attribute__((noinline))`, `__attribute__((optnone))`, o una macro que los pone) | quitar el atributo en la copia que se compila, o definir la macro vacía con `-D` |
| `E104` | `... it is variadic (...)` | función con `...` | una versión con argumentos fijos |
| `E105` | `... it is recursive` | la función se llama a sí misma (y no es recursión de cola, que LLVM convierte en bucle) | escribirla como bucle de iteraciones acotadas (§9.2.5) |
| `E106` | `... LLVM did not inline it (LLVM: ...)` | el inliner la rechazó por otro motivo; el texto entre paréntesis es el de `-pass-remarks-missed=inline` | según ese texto. Lo habitual: `static inline` o moverla a un header |
| `E107` | `indirect call (function pointer or virtual method)` | puntero a función no constante, o método `virtual` que clang no pudo devirtualizar | llamar a la función directamente. Con un `switch` sobre un modo: `if` / `else` llamando a cada función. Para virtuales: llamar sobre el tipo concreto (`obj.Clase::metodo()`) o marcar la clase `final` |
| `E108` | `intrinsic llvm.X is not supported` | un intrínseco entero sin operación en s1cc (`llvm.ctlz`, `llvm.bswap`, `llvm.memcpy` grande) | evitar la construcción que lo genera: `__builtin_clz` → comparación en escalera; copiar structs campo a campo |
| `E110` | `s1_*: argument N must be a constant` | índice de `s1_param`, `s1_export`, `s1_import` calculado | un literal o una constante de compilación |
| `E111` | `s1_param(nombre, i): index out of the table` | `i` ≥ `n` de `S1_PARAM(nombre, panel, n)` | agrandar `n` o corregir el índice |
| `E112`, `E113` | `s1_param: unknown table` / `not an S1_PARAM descriptor` | el primer argumento no es un nombre declarado con `S1_PARAM` | usar el nombre de la macro, no una variable |
| `E114`, `E115` | `s1_export in S1_VOICE_B` / `s1_import in S1_VOICE` | el sentido es sólo A → B | exportar en `S1_VOICE`, importar en `S1_VOICE_B` |

#### 9.2.3 Instrucciones (`graph.cpp`, también en `diag`)

| código | mensaje | causa | solución |
|---|---|---|---|
| `E202` | `instruction 'switch' is not supported` | un `switch` con valor calculado (clang no siempre lo baja a `select`) | `if` / `else if` o `?:`. Clang a veces convierte un `switch` chico en `select`, pero no siempre (un `switch` de 5 casos de una línea queda como `switch`) |
| `E203` | `instruction 'invoke' / 'landingpad' ...` | excepciones C++ | el driver compila con `-fno-exceptions`. Quitar `try` / `throw` del código de la voz |
| `E204` | `cast bitcast / ptrtoint / inttoptr is not supported` | un puntero usado como número, o reinterpretación de tipo por puntero (`*(int *)&f`) | trabajar con el valor: para el exponente de un float, `log2f`; para el resto, aritmética o una tabla |
| `E205` | `unsupported constant` | constante vectorial o de un tipo raro (`long double`, `__int128`) | `float`, `double` o enteros ≤ 64 bits |
| `E201` | `instruction 'X' is not supported` (otras) | una instrucción del IR que el grafo no modela (`atomicrmw`, `cmpxchg`, `fence`, `va_arg`, vectores) | quitar atómicos, `va_arg` y vectores explícitos del código de la voz |
| `E206` | `value not available (pointer used as a number, or an unsupported instruction)` | un valor que viene de algo que el grafo no modela | ver la instrucción del mensaje; casi siempre un puntero que sale de la voz o un vector |

#### 9.2.4 Memoria y estado (`graph.cpp`)

El estado son las variables de archivo no `const`. Las tablas son las `const`. No hay más memoria (§3).

| código | mensaje | causa | solución |
|---|---|---|---|
| `E301` | `state 'v' is not an int or a float` | variable de archivo de tipo puntero, struct no separable o `long double` | guardar índices en vez de punteros; separar el struct en campos escalares |
| `E302` | `state 'v' used by both S1_VOICE and S1_VOICE_B` | cada palabra de estado vive en un solo DSP | duplicarla, o pasar el valor con `s1_export` / `s1_import` |
| `E303` | `computed index into a state array: state arrays need constant indices` | `buf[i]` con `i` variable en un array de estado (líneas de retardo, buffers circulares) | índices constantes (desplazar el array a mano), o un bucle desenrollado. Las líneas de retardo largas no caben en el modelo |
| `E304` | `computed index into a local array: use constant indices` | array local con índice variable | lo mismo, o convertirlo en `const` si es una tabla |
| `E305`, `E306` | `load from an unknown address` / `unsupported pointer (only file-scope variables and tables)` | puntero que no es una global (argumento, `malloc`, puntero guardado) | acceder a la global directamente. Un objeto por referencia sirve si después del inlining la referencia apunta a una global |
| `E307` | `'t' has no definition in the module` | tabla `extern const` sin su archivo (p. ej. `resources.cc` de Braids) | agregar la fuente de la tabla |
| `E308` | `cannot fold the table read` | lectura de tabla con desplazamiento constante que LLVM no puede plegar (tabla sin inicializador visible) | lo mismo: la definición con sus valores tiene que estar en el build |
| `E309` | `store to something that is not a state word` | escritura a una tabla `const` o a memoria calculada | escribir sólo en variables de estado |

#### 9.2.5 Bucles (`loops.cpp`)

El driver perfila cuántas veces gira cada bucle y lo desenrolla hasta ese máximo (`s1cc trips` / `unroll`, §9).

| código | mensaje | causa | solución |
|---|---|---|---|
| `E401` | `a loop at ... never ran in the profiling scenarios` | el bucle está en una rama que ningún escenario de §5 tomó, así que su máximo es desconocido | hacer que el escenario lo alcance (rango del parámetro, nota), o darle un número de iteraciones constante |
| `E402` | `a loop at ... turns up to N times per sample: too long` | más de 64 vueltas por muestra | reducirlo (una vuelta por muestra con el contador en el estado), o reescribirlo como fórmula cerrada |
| `E403` | `cannot peel the loop at ...` | LLVM no pudo desenrollarlo (salidas múltiples complicadas) | simplificar: una sola condición de salida, contador entero |
| `E404` | `loop in voice code (...)` | quedó un bucle después del desenrollado (no debería pasar) | informar; mientras tanto, desenrollar a mano |
| `E405` | `a loop of f without a preheader` | error interno de la forma del bucle | informar con el fuente y `--debug` |

#### 9.2.6 Islas y funciones (`graph.cpp`, `lower.cpp`, `fit.cpp`)

Lo que el DSP no ejecuta directamente (lecturas de tabla con índice calculado, `or`, máscaras no contiguas,
divisiones enteras por variables, funciones desconocidas) se convierte en una función de **un** valor y se ajusta
(§4.3, §6).

| código | mensaje | causa | solución |
|---|---|---|---|
| `E501` | `a table read / function / division that depends on several values ... only functions of one value can be tabulated` | el nodo depende de dos o más fuentes independientes (entradas, parámetros, estado) que no pasan por un único valor intermedio | ver abajo |
| `E502` | `cannot tabulate ...: no single-value island could be sampled` | ninguna raíz candidata tuvo muestras utilizables | igual que `island without samples` |
| `E503` | `island without samples (the profiling run never reached it)` | el código está en una rama que ningún escenario tomó | que algún escenario la alcance, o `s1_range` en la raíz |
| `E504` | `not translatable and not tabulated` | nodo marcado como isla al que no se le eligió isla | informar con `--debug`; reescribir la expresión |
| `E505` | `F: the argument's range is unknown (use s1_range)` | `sin`, `tanh`, `log`... sobre un valor que el perfil nunca vio (rama no tomada) | `s1_range(x, lo, hi)` sobre el argumento |
| `E506` | `pow with two variables` | `pow(x, y)` con ambos variables | `exp2(y * log2(x))`, que s1cc sí baja |

**Varios valores.** Ejemplo real, `RenderToy` de Braids (`digital_oscillator.cc:308`):
`held_sample = (((phase >> 24) ^ (x << 1)) & (~x)) + (x >> 1)`, con `x` de un parámetro y `phase` del estado. El
`xor` / `and` con una máscara **variable** no es aritmética, y depende de dos fuentes. Salidas, en orden de
preferencia:
1. Que un operando sea constante: los `and` / `xor` con una constante se bajan como aritmética (§7.2). Si el
   parámetro puede fijarse (un modo, no una perilla continua), fijarlo.
2. Escribir la operación como aritmética de los dos valores (sumas, productos, `min` / `max`, comparaciones).
3. Que dependa de un solo valor intermedio: si `x` toma pocos valores, un `if` por valor deja en cada rama una
   función sólo de `phase`, con su propia isla.
4. Si nada de eso sirve, ese algoritmo no cabe en el modelo de s1cc.

#### 9.2.7 Operaciones (`lower.cpp`)

| código | mensaje | causa | solución |
|---|---|---|---|
| `E601` | `OP: operand N must be a constant` | desplazamiento (`<<`, `>>`) por una cantidad variable; división o resto entero por una variable que no quedó en una isla | desplazar por constante; para `x << n` variable, `x * s1_exp2(n)` en float |
| `E602` | `xor with a bit outside the value's range` | `x ^ c` con `x` negativo o mayor que `2c` | asegurar el rango (máscara antes) o reescribir |
| `E603` | `'or' with a constant is not supported outside a tabulated function` | `x \| c` con `x` de varias fuentes | `x + c` si los bits no se pisan; si no, que dependa de un valor (isla) |
| `E604` | `compare 'P' not supported` | predicado de comparación raro (`ord`, `uno`: chequeos de NaN) | quitar los chequeos de NaN: el DSP no tiene NaN |
| `E605` | `operation 'OP' not supported` | operación del IR sin bajada en `lower.cpp` (enteros de más de 64 bits, operaciones de bits raras) | reescribirla con operaciones de §7.2 |
| `E116` | `s1_import(i) without an s1_export(i)` | `S1_VOICE_B` importa un índice que `S1_VOICE` nunca exporta | agregar el `s1_export(i, v)` |
| `E006` | `no table for S1_PARAM 'p' (tables file)` | `tables.txt` viejo o la función de la tabla no se ejecutó | rehacer el build completo |

#### 9.2.8 Kernel (`kernel.cpp`)

| código | mensaje | causa | solución |
|---|---|---|---|
| `E701` | `kernel: constant K outside the DSP range (+-512)` | una escala u offset fuera del rango de los inmediatos; suele venir de un rango perfilado enorme | `s1_range` en el valor que explota, o limitarlo en el C (`s1_clamp`) |
| `E702` | `kernel: a multiply by a non-finite constant` / `a non-finite constant` | un rango sin límite (división por un valor que llega a 0, `log(0)`) | `s1_range`, o proteger la división en el C (`max(y, eps)`) |
| `E703` | `kernel: exp2 of a value spanning more than 2^11` | el argumento de `exp2` / `exp` / `pow(c, x)` cubre más de 2048 octavas | limitar el argumento (`s1_range` o `s1_clamp`) |
| `E704` | `kernel: out of registers (N usable) even with spilling` | demasiados valores vivos a la vez | más registros con `--regs` si el programa anfitrión lo permite, o partir el cálculo entre `S1_VOICE` y `S1_VOICE_B` |
| `E705`, `E706` | `kernel: value read before it is defined` / `no free register pair for recipseed` | error interno | informar con el fuente y `--debug` |

#### 9.2.9 Avisos

El build termina, pero el resultado puede diferir del C. `s1cc check` mide cuánto.

| código | aviso | qué significa | qué hacer |
|---|---|---|---|
| `W901` | `the original overflows at ... (integer product); the port computes it without wrapping` | el producto entero desborda en C y el DSP no reproduce el desborde | nada, si el desborde era accidental. Si el algoritmo depende de él (hash, ruido), reescribirlo con `&` / `%` por constante |
| `W902` | `truncation wraps at ...` | un `trunc` a un entero más chico desborda en el perfil; el port reproduce el módulo | normalmente correcto (fases). Revisar si no era intencional |
| `W903` | `division by a value in [lo, hi]: the port divides by max(y, c)` | el divisor llega a 0 o cruza el signo | proteger el divisor en el C, o `s1_range` |
| `W904` | `division by a value over [...]: more than 16 octaves, the ends saturate` | divisor de rango demasiado amplio | acotar el divisor |
| `W905` | `F at ...: the fit misses the tolerance (error E)` / `island at ...: the fit misses the tolerance` | el ajuste no llega a `--tol` / `--tollog` | `--maxdeg` más alto, `--tol` más grande, o un rango más chico (`s1_range`) |
| `W906` | `stage S: N values limited to the range of 99.99 % of their profiled samples` | valores con picos raros; se recortan para no arruinar la escala | nada si `check` da buen SNR; si no, `s1_range` en esos valores |
| `W907` | `state 'v' starts at 0 in the ring (...)` | con `--no-exact-init`, la palabra de estado no arranca en su valor inicial | quitar `--no-exact-init` |

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
