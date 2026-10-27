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

## 4. Build y verificación

```sh
D=sdk/s1cc/examples/simple_saw_wrapped
sdk/s1cc/s1cc build $D/simple_saw_wrapped.c $D/saw.c -I $D --check     # -> sdk/s1cc/build/simple_saw_wrapped
sdk/s1cc/s1cc check sdk/s1cc/build/simple_saw_wrapped --note 33 level=255 detune=255
KERNEL=sdk/s1cc/build/simple_saw_wrapped sh examples/mutable_system/build.sh   # en el sinte (S1CC.md §12)
```

El primer archivo de `build` da el nombre de la carpeta de salida. Con `--debug`, s1cc lista las funciones C que no
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
