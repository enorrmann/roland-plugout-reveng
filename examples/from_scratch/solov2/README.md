# solov2: cuatro voces por slave, con el reparto de Roland

Ejemplo nuevo a partir de `dsp/make_solo.py` y `gate/engine_solo.c` (`solo`). `solo` ponía las ocho voces en el
programa principal de DSP4, sin osciladores en DSP2/DSP3. `solov2` respeta la asignación de Roland
(`docs/WAVE_BUS.md` §4g): cada componente queda en el DSP y la ranura que Roland le asigna.

| componente | DSP | ranura | voces del slave |
|---|---|---|---|
| SAW (oscilador) | DSP2 | DCO1 `0x6000` | voz 0 (`[A021]`) y voz 1 (`[A022]`) |
| SAW (oscilador) | DSP3 | DCO1 `0x6000` | voz 2 (`[A021]`) y voz 3 (`[A022]`) |
| puerta y mezcla | DSP4 | principal `0x0100` | las cuatro, `[A078]`..`[A07B]` |
| salida | DSP4 | IOMD `6020` / `6021` | suma / 4 |

Vacíos: DSP0/DSP1 (envolvente y LFO), DSP5, programas principales de DSP2/DSP3, ranuras DCO2 (programa nulo
`gate/dco2_null.hex`) y efecto de DSP4.

Caminos de MXM (`docs/DSP_ARM_SIDE.md`, `docs/DSP_HARDWARE.md` §8.2):
* DSP2 escribe Z `0404` / `0405` → DSP4 lee M `0005` / `0006`.
* DSP3 escribe Z `0404` / `0405` → DSP4 lee O `0204` / `0205`.

## Ficheros

| fichero | qué es |
|---|---|
| `make_solov2.py` | genera el microcódigo (asm de `asm/`): `solov2_osc.hex` (SAW de DSP2/DSP3) y `solov2_gate.hex` (DSP4) |
| `engine_solov2.c` | motor de región 1: copia de `engine_solo.c` con la distribución nueva |
| `build_solov2.sh` | construye el plug-out `plugins/C-SOLOV2` (usa `engine/build.sh`, no lo modifica) |

## Uso

```
python3 examples/from_scratch/solov2/make_solov2.py --sim   # simulación: voz 2 (DSP3) a 880 Hz con su puerta
examples/from_scratch/solov2/build_solov2.sh                 # plugins/C-SOLOV2
python3 tools/plugout_tool.py install plugins/C-SOLOV2 --slot 3 --yes --force
```

## Estado

* Compila: `solov2_osc` 201 palabras, `solov2_gate` 67 palabras; el motor enlaza en las dos direcciones de slot.
* Simulado (`re/dspisa.py`, con la memoria de MXM de una muestra de retraso): cada voz llega a la salida sólo
  con su puerta abierta.
* **Sin probar en el equipo.** Falta: instalar, seleccionar el PLUG-OUT y medir con `arecord -D hw:SYSTEM8,0` que
  suenan las cuatro voces (cada slave con dos voces de DSP2 o DSP3).

## Riesgos conocidos

* La ruta DSP3 → DSP4 (`O 0204`) viene de la topología estática de `DSP_ARM_SIDE.md` y de la medición en el
  maestro (`DSP_HARDWARE.md` §8.2). En los slaves no está medida. Si no suena la voz 2 o la 3, es lo primero que
  revisar.
* La causa por la que `solo` sólo oía un slave no se ha determinado. `solov2` no la diagnostica; sólo elimina
  la diferencia de diseño (todas las voces en un DSP4 y ids de puerta compartidos).
