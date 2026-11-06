# Capturar audio y enviar notas al SYSTEM-1

Cómo tocar notas en el SYSTEM-1 por USB y grabar su salida, para verificar un PLUG-OUT en el equipo
(el rig que usamos, p. ej., para comprobar la wavetable: mandar un frame y clasificar la onda capturada).

Herramienta lista: **`tools/s1_capture.py`** (hace todo esto en un comando). Este doc explica el método por si hay que
hacerlo a mano o entenderlo.

## Dispositivos (este equipo)

| Qué | Valor | Cómo se averigua |
|---|---|---|
| Puerto MIDI | `hw:3,0,0` (SYSTEM-1 MIDI 1) | `amidi -l` → línea con `SYSTEM-1 MIDI 1` |
| Device de audio | `hw:3,0` (tarjeta `SYSTEM1`) | `/proc/asound/cards`, o `arecord -l` |
| Formato de audio | **S32_LE, 96000 Hz, 2 canales** | `cat /proc/asound/card3/stream0` |
| **Canal MIDI** | **5** (nibble 4: note-on `0x94`) | ⚠️ se configura en el equipo; si cambia, nada suena |

El número de tarjeta (3) puede cambiar entre arranques; sacarlo de `/proc/asound/cards` en vez de fijarlo.
**La salida del sintetizador aparece en la entrada de audio USB sólo mientras el equipo suena.**

## Reglas

- **Un solo cliente MIDI a la vez** ([[one-device-client]]): no correr otro tool del SYSTEM-1 (p. ej. `wt_load.py`)
  mientras se captura. `amidi` abre y cierra en cada llamada, así que se puede intercalar con otros tools, pero no
  simultáneo.
- **No tocar el teclado mientras graba**: tus notas se suman a la captura y ensucian el espectro (nos pasó: salía
  "saw" donde había un square). El tool manda la nota solo.
- El canal MIDI es del equipo, no del tool: si no suena nada y el audio está en silencio, casi siempre es el canal.

## El método a mano

Nota (canal 5 = nibble 4): note-on `0x94 nota vel`, note-off `0x84 nota 00`. CC = `0xB4 cc val`, pitch bend =
`0xE4 lsb msb`.

```sh
# 1. tocar una nota (La = 0x45 no; 0x3c = do central), y en paralelo grabar 2 s
amidi -p hw:3,0,0 -S '94 3c 64' &            # note-on, nota 60, vel 100
arecord -q -D hw:3,0 -d 2 -f S32_LE -r 96000 -c 2 -t raw out.raw
amidi -p hw:3,0,0 -S '84 3c 00'              # note-off
```

Decodificar el raw (S32_LE estéreo entrelazado) en Python:

```python
import numpy as np
x = np.fromfile('out.raw', '<i4').reshape(-1, 2).astype(np.float64) / 2147483648.0
mono = x[:, 0] + x[:, 1]
print('pico', np.abs(mono).max())            # 0.0 = silencio (¿canal?, ¿PLUG-OUT seleccionado?)
```

### Clasificar la onda (saw / square / triangle) por armónicos

Tomar el **tramo sostenido** (saltar ataque y release), ventana de Hann, FFT, medir h2..h6 respecto de h1:

```python
nz = np.nonzero(np.abs(mono) > mono.std() * 0.1)[0]
a, b = nz[0], nz[-1]
mid = mono[a + (b - a)//4 : b - (b - a)//4]; mid -= mid.mean()
X = np.abs(np.fft.rfft(mid * np.hanning(len(mid))))
f = np.fft.rfftfreq(len(mid), 1/96000.)
f0 = f[1 + np.argmax(X[1:])]
h = lambda k: X[(f >= f0*k*0.97) & (f <= f0*k*1.03)].max()
h1 = h(1)
# saw: pares fuertes (h2/h1 ~0.5). square: pares ~0, h3/h1 ~0.33. triangle: h3/h1 ~0.11 (1/9).
```

Reglas: `(h2+h4)/2 > 0.25` → **saw**; si no y `h3/h1 > 0.22` → **square**; si no y `h3/h1 > 0.03` → **triangle**;
si no → seno. (Nota 60 debería dar f0 ≈ 261 Hz; si sale ~1 Hz, la captura está en silencio o la detección agarró un
subarmónico de un transitorio.)

## La herramienta

```sh
python3 tools/s1_capture.py                         # nota 60, canal 5, 1.5 s -> resumen de armónicos
python3 tools/s1_capture.py --note 67 --secs 3
python3 tools/s1_capture.py --cmd "python3 tools/wt_load.py --square"   # corre eso, después captura
python3 tools/s1_capture.py --raw cap.raw           # guarda el raw (S32_LE 96k 2ch)
python3 tools/s1_capture.py --list                  # muestra los devices y sale
python3 tools/s1_capture.py --channel 1             # otro canal MIDI (1..16)
```

Para mandar un frame de wavetable y verlo cambiar: `tools/wt_load.py` (docs/MUTABLE_MEMORY.md §4.5); también manda en
canal 5 por defecto (`--channel`).
