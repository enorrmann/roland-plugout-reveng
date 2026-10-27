# `sdk/s1cc/examples/simple_saw_wrapped`: una librería C y su envoltorio para el SYSTEM-1

El ejemplo mínimo de cómo llevar una librería de audio en C a una voz de s1cc ([S1CC.md](S1CC.md)). La librería
es un diente de sierra ingenuo, sin limitar banda, en C puro: no incluye nada del SDK. Un solo archivo, el
envoltorio, la adapta a la voz.

## 1. Archivos

| archivo | qué es | incluye `s1dsp.h` |
|---|---|---|
| `saw.h` | interfaz de la librería: `struct saw`, `saw_init`, `saw_set_frequency`, `saw_process` | no |
| `saw.c` | la librería: fase 0…1, paso por muestra, salida −1…1 | no |
| `simple_saw_wrapped.c` | el envoltorio: tablas de perilla, inicio, una muestra por llamada | sí |

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
* **Estado.** `phase` e `inc` pasan a ser palabras del anillo de la voz, una por campo, 8 copias (una por voz).
* **El `if` del wrap.** Se convierte en un `select` y s1cc lo reconoce como envoltura de fase: queda exacto.
* **La división.** `hz / sample_rate` es por una constante: un producto.

## 3. El envoltorio (`simple_saw_wrapped.c`)

```c
#include "s1dsp.h"
#include "saw.h"

S1_PARAM(level, S1_OSC1_COLOR, 1) { out[0] = knob / 255.0f; }

static struct saw osc;

S1_INIT {
    saw_init(&osc);
}

S1_VOICE {
    float hz = 440.0f * s1_exp2(s1_pitch() - 4.75f);
    saw_set_frequency(&osc, hz, 96000.0f);
    return saw_process(&osc) * s1_param(level, 0) * s1_gate();
}
```

| elemento | para qué |
|---|---|
| `S1_PARAM(level, S1_OSC1_COLOR, 1)` | OSC1 COLOR (`0x168`, de `s1panel.h`) → nivel. La función corre en el PC para knob = 0…255; el ARM escribe el valor en el DSP cuando se mueve la perilla |
| `static struct saw osc` | la instancia de la librería; sus campos son el estado de la voz |
| `S1_INIT` | la inicialización de la librería. Corre en el PC antes de la referencia y del perfilado; lo que deja es el estado inicial también en el DSP (S1CC.md §11.1) |
| `S1_VOICE` | una muestra del DSP A del esclavo: tono → frecuencia, una muestra de la librería, × nivel × puerta |
| `s1_pitch()` | tono de la voz en octavas (4,75 = La 440), con el LFO y el range del anfitrión |
| `s1_exp2(x)` | 2ˣ con el `exp2` del DSP |
| `s1_gate()` | puerta, 0 / 1 |
| 96000 | la frecuencia de muestreo del DSP |

No hay `S1_VOICE_B`: el DSP B queda para el filtro y las envolventes del programa anfitrión (C-S1SY).

Lo que un envoltorio tiene que resolver con una librería más grande:
* **Notas.** Si la librería tiene un `note_on` o reinicia la fase, detectar el comienzo de nota con `s1_trig()`, que
  alterna 0 / 1 en cada nota (ejemplo: `saw_wrapper.c`).
* **Rangos ocultos.** Si una rama esconde el rango de un valor, declararlo con `s1_range(x, lo, hi)`.
* **Bloques.** Si la librería procesa bloques (`Render(out, size)`), llamarla con tamaño 1.

## 4. Build y verificación

```sh
D=sdk/s1cc/examples/simple_saw_wrapped
sdk/s1cc/s1cc build $D/simple_saw_wrapped.c $D/saw.c -I $D --check     # -> sdk/s1cc/build/simple_saw_wrapped
sdk/s1cc/s1cc check sdk/s1cc/build/simple_saw_wrapped --note 33 level=255
KERNEL=sdk/s1cc/build/simple_saw_wrapped sh examples/mutable_system/build.sh   # en el sinte (S1CC.md §12)
```

El primer archivo de `build` da el nombre de la carpeta de salida. Con `--debug`, s1cc lista las funciones C que no
puede compilar y por qué (S1CC.md §9.1).

## 5. Resultados [sim, 2026-10-27]

Etapa A: 26 unidades → **19 instrucciones**. `check`, perilla `level` = 255:

| nota | SNR 10 ms | SNR total | niveles por banda port − ref |
|---|---|---|---|
| 33 | 107,8 dB | 85,1 dB | 0,00 dB |
| 57 (`level` = 128) | 94,7 dB | 26,0 dB | 0,00 dB |
| 81 | 100,4 dB | 80,5 dB | 0,00 dB |
| 105 | 14,3 dB | 14,0 dB | +24,6 dB en la banda grave, 0,00 dB en las demás |

* **Nota 105 (≈ 3,5 kHz).** La diferencia es el aliasing del diente de sierra ingenuo, no un error del port. Sus
  armónicos se pliegan, y una diferencia mínima de afinación los deja en otro lugar. En el original la banda grave
  casi no tiene energía (−44,6 dB), así que esa diferencia mínima se ve grande: en el port queda en 24,4 dB, con la
  banda principal en 79,4 dB en los dos. El SNR bajo es una deriva de fase por la misma diferencia de frecuencia.
  Para una librería que limite banda, ver `va_saw_svf.c` (polyBLEP).
* **SNR total en la nota 57.** Las fases del original (float32) y las del DSP se separan poco a poco por el
  redondeo (S1CC.md §11).
* **Sinte completo.** `make_mutable.py --sim --pack-check` con `KERNEL=`: los siete casos dan resultados idénticos.
* **Test.** `tests/test_s1cc.py`: SNR A > 90 dB en las notas 33, 57 y 81; A ≤ 22 instrucciones.

Sin probar en el equipo.
