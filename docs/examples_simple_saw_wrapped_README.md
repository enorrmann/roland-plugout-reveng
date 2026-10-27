# `sdk/s1cc/examples/simple_saw_wrapped`: una librería C y su envoltorio para el SYSTEM-1

El ejemplo mínimo de cómo llevar una librería de audio en C a una voz de s1cc ([S1CC.md](S1CC.md)). La librería
es un diente de sierra ingenuo, sin limitar banda, en C puro: no incluye nada del SDK. Un solo archivo, el
envoltorio, la adapta a la voz con **dos instancias** de la librería: dos sierras por voz, la segunda desafinada
con OSC2 COLOR.

## 1. Archivos

| archivo | qué es | incluye `s1dsp.h` |
|---|---|---|
| `saw.h` | interfaz de la librería: `struct saw`, `saw_init`, `saw_set_frequency`, `saw_process` | no |
| `saw.c` | la librería: fase 0…1, paso por muestra, salida −1…1 | no |
| `simple_saw_wrapped.c` | el envoltorio: dos tablas de perilla, dos instancias, una muestra por llamada | sí |

## 2. La librería (`saw.c`)

Tiene la forma habitual de una librería de audio: una estructura de estado, un *setter* y una función por muestra.

```c
struct saw { float phase; float inc; };

void saw_set_frequency(struct saw *s, float hz, float sample_rate) { s->inc = hz / sample_rate; }

float saw_process(struct saw *s) {
    s->phase += s->inc;
    if (s->phase >= 1.0f)
        s->phase -= 1.0f;
    return 2.0f * s->phase - 1.0f;
}
```

Qué hace s1cc con ella, sin ayuda:
* **Llamadas.** `saw_set_frequency` y `saw_process` están en otro archivo. El driver enlaza las fuentes y las
  inlinea en la voz (§9 de S1CC.md).
* **Estado.** `phase` e `inc` de cada instancia pasan a ser palabras del anillo de la voz, una por campo, 8 copias
  (una por voz).
* **El `if` del wrap.** Se convierte en un `select` y s1cc lo reconoce como envoltura de fase: queda exacto.
* **La división.** `hz / sample_rate` es por una constante: un producto.

## 3. El envoltorio (`simple_saw_wrapped.c`)

```c
#include "s1dsp.h"
#include "saw.h"

S1_PARAM(level, S1_OSC1_COLOR, 1) { out[0] = knob / 255.0f; }
S1_PARAM(detune, S1_OSC2_COLOR, 1) { out[0] = s1_exp2(knob / 255.0f * 0.5f / 12.0f); }

static struct saw osc1, osc2;

S1_INIT {
    saw_init(&osc1);
    saw_init(&osc2);
}

S1_DSP_A {
    float hz = 440.0f * s1_exp2(s1_pitch() - 4.75f);
    saw_set_frequency(&osc1, hz, 96000.0f);
    saw_set_frequency(&osc2, hz * s1_param(detune, 0), 96000.0f);
    float mix = 0.5f * (saw_process(&osc1) + saw_process(&osc2));
    return mix * s1_param(level, 0) * s1_gate();
}
```

### 3.1 Cuándo corre cada parte

Detalle general: [S1CC.md](S1CC.md) §3.1.

| parte | cuándo corre | qué hace aquí |
|---|---|---|
| `S1_PARAM(level, S1_OSC1_COLOR, 1)` | **PC**, al compilar: 256 veces (knob = 0…255). **Sinte**: el ARM escribe la fila de la perilla en un parámetro del DSP A, la 128 al cargar y después cada vez que se mueve OSC1 COLOR | nivel 0…1 |
| `S1_PARAM(detune, S1_OSC2_COLOR, 1)` | igual, con OSC2 COLOR | razón de frecuencia de la segunda sierra, 1 … 2^(0,5/12) (0…+50 cents). El `s1_exp2` corre en el PC: en el DSP es una tabla |
| `S1_INIT` | **PC** solamente: antes de la referencia y de cada escenario del perfilado. **Nunca en el sinte** | `saw_init` de las dos instancias: fases y pasos en 0. s1cc hace que el DSP arranque igual (S1CC.md §11.1; aquí no hace falta corrección: todo empieza en 0) |
| `S1_DSP_A` | **Sinte**: DSP A del esclavo, cada muestra (96 kHz), las 8 voces, suene o no una nota. **PC**: en la referencia y el perfilado | tono → frecuencia; las dos sierras; promedio × nivel × puerta |
| `saw_set_frequency`, `saw_process` | donde corre `S1_DSP_A`: s1cc las inlinea | — |
| `static struct saw osc1, osc2` | — | las dos instancias: sus 4 campos son el estado de la voz, 4 palabras × 8 voces |

| intrínseco | qué da |
|---|---|
| `s1_pitch()` | tono de la voz en octavas (4,75 = La 440), con el LFO y el RANGE de OSC1 del anfitrión |
| `s1_exp2(x)` | 2ˣ con el `exp2` del DSP (en el PC, la función de C) |
| `s1_param(nombre, i)` | el valor `i` de la tabla de esa perilla, en la posición actual |
| `s1_gate()` | puerta de la voz, 0 / 1 |

* **Varias sierras van en `S1_DSP_A`.** `S1_DSP_B` no es "el oscilador 2": es el DSP B, que sigue al A. Aquí no
  se usa: el DSP B queda para el filtro y las envolventes del anfitrión (C-S1SY).
* **Después de `S1_DSP_A`**, el anfitrión multiplica la salida por OSC1 MIX y el trémolo, y en el DSP B aplica el
  VCA, la envolvente del filtro y el paso bajo.
* **Costo.** El DSP no salta: las dos sierras, con sus `if`, cuestan lo mismo en cada muestra (30 instrucciones por
  voz).

Lo que un envoltorio tiene que resolver con una librería más grande:
* **Notas.** Si la librería tiene un `note_on` o reinicia la fase, detectar el comienzo de nota con `s1_trig()`, que
  alterna 0 / 1 en cada nota (ejemplo: `saw_wrapper.c`).
* **Rangos ocultos.** Si una rama esconde el rango de un valor, declararlo con `s1_range(x, lo, hi)` (S1CC.md §3.2).
* **Bloques.** Si la librería procesa bloques (`Render(out, size)`), llamarla con tamaño 1.

Para SYSTEM-S1CC, que no tiene VCA propio, `simple_saw_env.c` es la misma voz con una envolvente ADSR y el panel
mapeado como en C-S1SY ([examples_system_s1cc_README.md](examples_system_s1cc_README.md) §3). Todo en C, con dos
librerías puras:

| archivo | qué es | incluye `s1dsp.h` |
|---|---|---|
| `env.h` / `env.c` | ADSR de segmentos de un polo: `env_init`, `env_coefficient(segundos, fs)` (1 − e^(−1/(t·fs))), `env_process(e, puerta, inicio, params)`. Un flag `attacking` (0 / 1 como float) y un paso por muestra hacia un objetivo: 1,2 en el ataque (hasta llegar a 1), el sustain en el decay, 0 en el release | no |
| `simple_saw_env.c` | el envoltorio: OSC1 / OSC2 MIX → nivel de cada sierra, OSC2 FINE → ±1 semitono en la segunda, AMP ATTACK / DECAY / SUSTAIN / RELEASE → la ADSR; un inicio de nota es un cambio de `s1_trig()` o la puerta que sube | sí |

Las sierras corren en `S1_DSP_A` y la ADSR en `S1_DSP_B` (sobre `x`, la salida de A); `env_coefficient` corre en
`S1_PARAM`, en el PC. `s1cc build` agrega solo `saw.c` y `env.c`. Kernel [sim, 2026-10-27], con `--regs
0,1,2,3,4,5,6,7`: A 29 instrucciones por voz, B 46; SNR 10 ms 84–110 dB en cinco combinaciones de notas y perillas
(decay rápido, ataque lento, sustain 0 con release).

**Escribir para un DSP que ejecuta todos los caminos.** La primera ADSR (un `switch` sobre un estado entero con tres
etapas) daba 174 instrucciones por voz: cada store condicional del estado es una cadena de `select`, y cada
comparación de float, varias etapas. Con un flag float, un solo paso y los `?:`, quedan 100 (y s1cc resuelve gratis
las comparaciones de valores 0 / 1, S1CC.md §7.2). Detectar la nota solo por `s1_trig()` (el ARM lo cambia en cada
nota), los 8 registros y la ADSR en el DSP B dejan A en 29 y B en 46: 8 voces entran con la mitad del presupuesto
libre. Esa primera versión destapó cuatro errores de s1cc, corregidos:
`switch`, igualdad entre enteros, booleanos especulados y el rango de un estado que se escribe antes de leerse
(S1CC.md §4.1, §4.4, §5, §7.2).

## 4. Build y verificación

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_wrapped.c --check   # -> sdk/s1cc/build/simple_saw_wrapped
sdk/s1cc/s1cc check sdk/s1cc/build/simple_saw_wrapped --note 33 level=255 detune=255
KERNEL=sdk/s1cc/build/simple_saw_wrapped sh examples/mutable_system/build.sh   # en el sinte (S1CC.md §12)
```

`saw.c` no se nombra: el driver agrega solo la fuente vecina de cada header incluido (`saw.h` → `saw.c`, S1CC.md
§9, paso 0) e informa `source: …/saw.c (for saw.h; …)`. El primer archivo de `build` da el nombre de la carpeta de
salida. Con `--debug`, s1cc lista las funciones C que no
puede compilar y por qué (S1CC.md §9.1).

## 5. Resultados [sim, 2026-10-27]

Etapa A: 40 unidades → **30 instrucciones** (una sola sierra: 19). `check`:

| nota | perillas | SNR 10 ms | SNR total | niveles por banda port − ref |
|---|---|---|---|---|
| 33 | `level` 255, `detune` 255 | 107,6 dB | 34,0 dB | 0,00 dB |
| 57 | `level` 128, `detune` 0 | 94,7 dB | 26,0 dB | 0,00 dB |
| 57 | `level` 255, `detune` 128 | 111,7 dB | 30,0 dB | −1,25 dB en la banda grave, 0,00–0,01 dB en las demás |
| 81 | `level` 255, `detune` 255 | 99,3 dB | 32,0 dB | 0,00–0,02 dB |

* **SNR total.** Las fases del original (float32) y las del DSP se separan poco a poco por el redondeo (S1CC.md
  §11); con dos sierras que laten entre sí se nota antes.
* **Banda grave con `detune` 128.** El batido de las dos sierras pone energía en la banda grave; con la deriva de
  fase, esa banda difiere 1,25 dB.
* **Notas muy agudas** (medido con una sola sierra, nota 105 ≈ 3,5 kHz: SNR 14 dB, banda grave +24,6 dB). Es el
  aliasing del diente de sierra ingenuo, no un error del port: sus armónicos se pliegan, y una diferencia mínima de
  afinación los deja en otro lugar. La banda principal coincide (79,4 dB en los dos). Para una librería que limite
  banda, ver `va_saw_svf.c` (polyBLEP).
* **Sinte completo.** `make_mutable.py --sim --pack-check` con `KERNEL=`: los siete casos dan resultados idénticos.
* **Test.** `tests/test_s1cc.py`: SNR A > 90 dB en las notas 33, 57 y 81; A ≤ 33 instrucciones.

Sin probar en el equipo.

## 6. ADSR en C: el inicio de nota que se perdía [medido en el equipo, 2026-10-27]

**Síntoma** (SYSTEM-S1CC, `simple_saw_env.c`): AMP ATTACK no hacía nada, AMP DECAY se comportaba como un ataque y,
con SUSTAIN en 0, no había ni ataque ni decay. SUSTAIN y RELEASE funcionaban.

**Causa.** El inicio de nota era "cambió `s1_trig()`", y dura una muestra. El ataque solo empezaba si en esa muestra la
puerta ya estaba arriba (`attacking = gate · max(attacking, start)`). En el equipo, `engine.c` escribe el disparo y la
puerta con dos llamadas, que pueden llegar al DSP con muestras de diferencia: el inicio se perdía, y la envolvente iba
con el coeficiente del decay hacia el sustain. En la referencia nativa y en el emulador llegaban juntos.

**Arreglo** (`env.c`): el inicio queda pendiente hasta que la puerta sube (`pending = max(pending, start)`,
`go = pending · gate`, `pending = pending · (1 − gate)`). DSP B: 46 → 50 instrucciones por voz.

**Control.** `make_s1cc.py --sim` simula también la puerta 3 muestras después del disparo y avisa si cambia la energía
de los primeros 50 ms más de un 10 % (el build viejo: RMS 0,0106 contra 0,0254; el nuevo: 0,0253 contra 0,0254).

## 7. Envolventes del anfitrión (`simple_saw_hostenv.c`)

Dos sierras, un paso bajo en C (`filter.c`) y las dos envolventes del anfitrión, como las usa un sinte: la 0 es el
VCA y la 1 mueve el corte del filtro. Las envolventes las pone SYSTEM-S1CC, en ensamblador, en el DSP B; el kernel
solo las lee con `s1_env(i)`.

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_hostenv.c --regs 0,1,2,3,4,5,6,7 --check
KERNEL=sdk/s1cc/build/simple_saw_hostenv NAME=S1CC-HOSTENV sh examples/system_s1cc/build.sh
```

### 7.1 Qué pone el C y qué pone el anfitrión, en una voz y una muestra

| dónde | quién | qué | palabra |
|---|---|---|---|
| DSP A | kernel (`S1_DSP_A`) | las dos sierras × OSC1 / OSC2 MIX, mezcladas: el valor de retorno | banco 5 `A151+v` = `x` del DSP B |
| DSP A | anfitrión (`make_s1cc.py`, porque el kernel usa `s1_env`) | copia la puerta `A038+v` y el disparo `A060+v` | banco 5 `A160+v`, `A168+v` |
| DSP B | anfitrión | envolvente 0: ADSR de C-S1SY con `C040`–`C043` (perillas AMP) | banco 0, bloque de la voz, palabra 0 |
| DSP B | anfitrión | envolvente 1: la misma ADSR con `C044`–`C047` (perillas FILTER) | banco 0, bloque de la voz, palabra 3 |
| DSP B | kernel (`S1_DSP_B`) | `g = min(corte · 2^(env1 · cantidad), 0,9)`; paso bajo de `x` con `g`; × `env0` | la salida de la voz, que la mezcla suma |
| ARM | `engine.c` | perillas AMP / FILTER A, D, S, R → `C040`…`C047` del DSP B | banco 6 |
| ARM | `engine.c` | FILTER CUTOFF, FILTER ENV AMOUNT → tablas `S1_PARAM` del kernel (`C060`…) | banco 6 |

* **Orden en el DSP B, por voz:** primero las envolventes, después el kernel. El kernel lee el nivel que la
  envolvente acaba de guardar en esa misma muestra.
* **Sin `s1_export`.** Las envolventes y el filtro actúan sobre la voz entera, y `x` la trae de A. Cuándo sí hace
  falta un export: §8.
* **`s1_env(i)`.** Solo en `S1_DSP_B` (E117); `i` es 0 (AMP) o 1 (FILTER) (E118). `make_s1cc.py` arma solo las
  envolventes que el kernel lee. Valores por defecto hasta que se mueve una perilla: AMP S 255 / A 0 / D 128 / R 60,
  FILTER S 0 / A 0 / D 128 / R 60.
* **Tiempos.** De 1 ms (perilla en 0) a 10 s (255), exponencial, como C-S1SY. Se cambian al generar el PLUG-OUT:
  `ENV_TMIN=0.002 ENV_TMAX=4 KERNEL=… sh examples/system_s1cc/build.sh`.
* **La ADSR del anfitrión sigue a la puerta misma** (ataque = puerta − latch, el latch se borra con cada disparo): no
  le pasa lo de §6.
* **Filtro.** FILTER CUTOFF: 20 Hz…20 kHz (exponencial); FILTER ENV AMOUNT: −8…+8 octavas en el pico de la
  envolvente (128 = nada). `lowpass_coefficient` corre en `S1_PARAM`, en el PC; en el DSP, `s1_exp2` y un `min`.
* **Verificación.** En la referencia nativa y en `s1cc check`, `s1_env()` es la puerta (`s1rt.c`): el kernel se
  verifica con envolventes cuadradas (A 29, B 23 instrucciones por voz; SNR 10 ms 62–95 dB, el más bajo con el corte
  en 17 Hz, donde casi no pasa señal). Las envolventes reales se verifican en la cadena entera (`make_s1cc.py --sim`):
  ataque, decay al sustain, release; y en `tests/test_s1cc.py TestSystemS1ccHostEnvelopes`.

### 7.2 Costo [sim, 2026-10-27], 8 voces

| kernel | DSP A | DSP B |
|---|---|---|
| `simple_saw_env` (ADSR en C, en `S1_DSP_B`) | 225 | 404 |
| una envolvente del anfitrión para toda la voz (`return x * s1_env(0);`) | 226 | 175 |
| `simple_saw_hostenv`: dos envolventes del anfitrión + paso bajo en C | 226 | 471 |

## 8. `s1_export` / `s1_import` (`simple_saw_export.c`)

El ejemplo que muestra solo el export: dos sierras en el DSP A, y en el DSP B un paso bajo cuyo corte sigue la nota
(*key tracking*). B necesita la frecuencia de la nota, que A ya calculó para las sierras: A la exporta y B la importa.
El código C explica en detalle qué cruza de A a B y cómo se compila.

```sh
sdk/s1cc/s1cc build sdk/s1cc/examples/simple_saw_wrapped/simple_saw_export.c --regs 0,1,2,3,4,5,6,7 --check
KERNEL=sdk/s1cc/build/simple_saw_export NAME=S1CC-EXPORT sh examples/system_s1cc/build.sh
```

**Qué cruza del DSP A al DSP B.** Los dos DSP solo comparten palabras del banco 5. Una variable del C pertenece al DSP
cuya función la usa (E302 si la usan los dos).

| a `S1_DSP_B` llega | qué es | palabra |
|---|---|---|
| `x` | lo que devolvió `S1_DSP_A`: la señal | `A151+v` |
| `s1_pitch()`, `s1_gate()`, `s1_trig()` | las entradas de la voz: s1cc hace que A se las copie | `A1A0 + 8j + v` |
| `s1_import(i)` | lo que `S1_DSP_A` pasó a `s1_export(i, v)` en la misma muestra: cualquier otro valor | `A188 + 8i + v` |

**Cuándo conviene un export:**
* cuando B necesita un valor que A ya tiene (una frecuencia, un nivel, una modulación), y calcularlo de nuevo cuesta
  más que un store y una carga;
* cuando B tiene que tratar por separado partes que A mezcla en `x`.

No hace falta cuando B solo trabaja sobre la voz entera (una envolvente, un filtro con corte fijo): `x` la trae (§7).

**La ranura.** El `0` de `s1_export(0, v)` / `s1_import(0)` es la ranura: el canal por el que pasa el valor. El export
y el import con el mismo número son la misma palabra; dos valores usan dos ranuras (0 y 1). Es una constante (E110);
SYSTEM-S1CC tiene las ranuras 0…11.

**El mecanismo, paso a paso:**
1. **s1cc, grafo** (`graph.cpp`): `s1_export(i, v)` en `S1_DSP_A` anota "v sale por la ranura i" (E114 si está en
   `S1_DSP_B`); `s1_import(i)` en `S1_DSP_B` es un nodo de entrada "ranura i" (E115 en `S1_DSP_A`; E116 si no hay
   export i).
2. **s1cc, lowering** (`lower.cpp`): en el DSP A, apenas se calcula v, `st r, [{X:i}]`, con v en la representación que
   tiene ahí (V = (r − o)·s). El kernel la anota (`.export i s o` en `voice.s1k`; aquí `0,1467 0`). En el DSP B,
   `ld r, [{X:i}]`, leída con la misma s y o: ninguna instrucción de conversión en ningún lado.
3. **El generador del PLUG-OUT** (`examples/system_s1cc/make_s1cc.py`, `export_addr`): pone `{X:i}` de la voz v en una
   palabra del banco 5 en la que un DSP puede guardar: `A188 + 8i + v` para las ranuras 0…2 y `A1B8 + 8(i − 3) + v`
   para 3…11. El banco 5 es la única memoria que comparten los dos DSP del chip; cada uno tiene sus bancos 0 y 6.
4. **En el equipo:** por voz, un store en el programa del DSP A y una carga en el del DSP B.

**Cuándo llega el valor.**
* **En la referencia nativa y en el emulador**, el DSP B lee el valor de la misma muestra. En `s1rt.c` la ranura es un
  arreglo, y el emulador ejecuta primero el DSP A y después el B.
* **En el equipo, los dos DSP del chip corren a la vez** ([examples_system_1_jx_README.md](examples_system_1_jx_README.md)
  §13). B recibe el valor de esta muestra solo si el store de A ocurre antes que la carga de B, en ciclos; si no,
  recibe el de la muestra anterior.
* **`make_s1cc.py` lo informa** para cada valor que cruza (`x`, las entradas, los exports, las copias de puerta y
  disparo), contando la posición de cada instrucción en los dos programas:

```
A -> B x: one sample late (voices 0,1,2,3,4,5,6,7)
A -> B export 0: race (voices 0); one sample late (voices 1,2,3,4,5,6,7)
A -> B input gate: race (voices 0); one sample late (voices 1,2,3,4,5,6,7)
```

  "race" quiere decir que el store y la carga están a menos de 8 ciclos: es una de las dos muestras, siempre la misma,
  pero no se sabe cuál. Supone que los dos programas empiezan la muestra juntos [inferido, como `timing.py`].
* **Efecto:** una muestra de retraso son 10 µs, inaudible para una frecuencia o un nivel. La propia `x` cruza igual
  (aquí, una muestra tarde: una latencia fija). Un valor que no puede atrasarse (dos señales que deben coincidir
  muestra a muestra) tiene que calcularse en el DSP que lo usa.

**Medido [sim, 2026-10-27], por voz:**

| | DSP A | DSP B | 8 voces: DSP B | SNR 10 ms A / B |
|---|---|---|---|---|
| con export | 29 | 15 | 110 | 94,7 / 90,4 dB |
| sin export (B calcula `440·2^(pitch − 4,75)/96000` de nuevo) | 29 | 25 | 195 | 94,7 / 90,4 dB |

El store del export no suma instrucciones en A: se empaqueta en una que ya existe.

Sin envolvente, para no mezclar temas: la voz es la puerta. Con envolvente: `simple_saw_env.c` (§6) o
`simple_saw_hostenv.c` (§7).
