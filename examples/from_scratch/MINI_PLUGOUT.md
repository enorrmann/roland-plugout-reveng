# Plugout mínimo propio: diario, hallazgos y pasos (2026-10-21)

Objetivo acordado con el usuario: un plugout muy simple, **un SAW propio, un LFO y una envolvente**, con el
ARM en C y el DSP en el lenguaje de `asm/`, sin opciones del panel que no se usen, instalado en slot 3 con
`--force` (el usuario autorizó sobrescribir slot 3). Lo que sigue documenta lo hecho, lo que falta y de dónde
sale cada dato.

## 1. Estado

| parte | estado |
|---|---|
| SAW del DCO2 en `asm/` (`dsp/saw_dco2.s`) | **hecho**, verificado contra el SAW de `dspasm` en simulación |
| LFO en `asm/` | pendiente |
| Envolvente en `asm/` | pendiente |
| Programa principal propio (tono, nota, puerta) | pendiente; hoy lo escribe Roland |
| Salida de la voz al bus de onda | **estéreo en `6020` (canal 1) y `6021` (canal 0)**; mapa completo en `docs/WAVE_BUS.md` |
| Notas | **ocho notas suenan con afinación exacta** (`C-MINI2`, voz monofónica en el slave 0); el slave 1 no llega a la salida (`docs/WAVE_BUS.md` §4e) |
| Engine en C (región 1) sin Roland | pendiente |
| Región 0 / lista de parámetros reducida | pendiente |
| Instalación en slot 3 con `--force` | hecha con `C-IOMD` (prueba de salida, §4); **sustituye a C-SAW** en slot 3 |

Del equipo se ha tocado sólo slot 3 (instalación de `C-IOMD`, autorizada). El resto es simulación o análisis de audio.

## 2. Pasos seguidos

1. Revisar `from_scratch` (NOTES, `model/`, `engine/`) y el estado del SAW propio anterior (`re/dspasm.py`).
2. Mirar cómo el engine registra programas y escribe parámetros (`engine/engine.c`: `p_bank`, `note_pitch`,
   `note_gate`, `swap`, `dco1_prog`/`dco2_prog`).
3. Buscar el contrato de salida de la cadena DSP (`docs/DSP_ARM_SIDE.md`, `ARCHITECTURE.md`): DSP0/1 envolventes y
   LFO → DSP2/3 osciladores → DSP4/5 filtro y VCA → IOMD (bus de onda).
4. Leer la API y el conjunto de mnemónicos de `asm/asm.py` y `asm/isa.json` (campo `result` de cada variante).
5. Escribir `dsp/saw_dco2.s` en esa sintaxis; ensamblar: 62 instrucciones, 0 rechazadas.
6. Comparar en simulación contra `dspasm.saw('dco2')` (`dsp/check_saw.py`). Primero 1.45·10⁻⁶: la constante 1/24
   estaba en 16 bits. Con `#0.041666666666666664:32` la diferencia es **0**.
7. Generar el programa de prueba de salida (`dsp/make_iomd_test.py`), registrarlo con `engine.c -DIOMDTEST`
   (DSP5, slot DCO1), instalar con `--force` y medir (§4).

## 3. Parámetros: de dónde salen y cómo se usan

### 3.1 Parámetros de nota (ARM → DSP), de `engine/engine.c`

El engine escribe el DSP con `p_bank(dsp, id, valor)`, que llama a la tabla `+0xB0` del OS. El id de banco 5
(`0xA0nn` en el DSP) es la dirección de memoria del parámetro; el programa lo lee con `ld rR, [A0nn]`.

| función | id escrito (bank 5) | valor | uso |
|---|---|---|---|
| `note_pitch` | `0x21 + 0x12·k` en DSP `u = lv >> 1` | `(nota − 12) / 12` | tono de la voz `lv` (k = 0/1 dentro del DSP) |
| `note_gate` | `0x22 + 0x12·k` en DSP `u` | `1.0` al pulsar, `0.0` al soltar | puerta de la envolvente de la voz (DSP0/1) |
| `note_gate` | `0x2A + s`, `0x29 + s`, `0x22 + s` en DSP `d = 4 + u` | `1.0`, `vel_curve[vel]`, `vel/127` | puerta, curva de velocidad y velocidad del VCA (DSP4/5; `s = 0xF·k − u`, los ids de DSP5 son uno menos) |
| bend `0x465` (CC del OS) | `+0xA0` | `v/8191` | pitch bend (no usado en la versión mínima) |

Mapa de voces: `lv` = voz local del slave (slave 0 = voces 0–3, slave 1 = voces 4–7); `u = lv>>1`, `k = lv&1`.
Estos ids son los del JX-3P medidos en `docs/NOTES.md` ("Engine del JX-3P emulado"). Son parte de la interfaz
con el OS: **si los cambiamos, hay que cambiar también lo que lee el microcódigo**.

**Para el SAW mínimo**: el tono debería venir de `0x21` (`ld rR, [A021]`), pero hoy el SAW lee `[2207]`. Esa
conexión la tiene que hacer el programa principal propio y todavía no existe.

### 3.2 Parámetros del panel (lista de la región 0)

Fuente: `model/params.py`. Cada registro es `P(pid, name, lo, hi, init, flag, lst, kind, pos)`:

* `pid`: id del parámetro (el mismo que usa el engine; los nuestros conservan los ids del JX-3P).
* `name`: nombre que muestra el panel.
* `lo`, `hi`: rango (0/0 = rango completo).
* `init`, `flag`: sin verificar (en el propio código: "meaning not verified").
* `lst`: lista de etiquetas (`LISTS`). `kind`: tipo de control (0, 1, 2, 4). `pos`: posición en la página del grupo.

**Cómo deshabilitar una opción del panel**: quitar su registro `P(...)` de la lista que `params.py` escribe en
la región 0 (SYNINFO +0x28). Eso es lo que el OS lee para construir el panel. **No está probado en el equipo**;
es el mecanismo que indica la documentación del propio `params.py`, pero no lo he verificado. Antes de usarlo
hay que comprobar que el panel oculta la opción y que el engine ya no recibe el id (el id no enviado queda con
su último valor).

**Qué parámetros usa la versión mínima** (los demás se quitan de la lista):

| función | id | fuente del id | uso previsto |
|---|---|---|---|
| forma de onda DCO2 | `0x6D9` | `docs/NOTES.md` ("Ondas (0x6D4/0x6D9)") | sólo SAW: se deja una sola opción |
| nivel / volumen | por confirmar | `engine.c`: `dco_levels` (`0x32`/`0x33`) | nivel de la voz |
| tiempos de envolvente A/D/S/R | por confirmar | `docs/NOTES.md`: `C02F`..`C036` en MAIN0/1 | envolvente mínima |
| velocidad y profundidad del LFO | por confirmar | `docs/NOTES.md`: `A028 rate ← 0x3EF/0x6CF`, `A02C forma ← 0x6CD`, `A02D delay ← 0x6CE` | LFO mínimo |
| filtro, efectos, unisón, portamento, etc. | `0x748`, `0x7E4`, ... | `engine/model_spec.py` | **se quitan del panel** |

Los ids "por confirmar" no los he leído aún de `model/params.py` en esta sesión: hay que ligar cada uno con su
registro antes de tocar la lista. Este es el paso siguiente del diseño de la región 0.

## 4. Salida de la voz: resuelta en parte (prueba en el equipo, 2026-10-21)

Un slave saca la voz por el bus de onda hacia el maestro. `docs/DSP_ARM_SIDE.md` (líneas 283–284) dice que el
DSP5 escribe `B0–B7` (IOMD0), pero qué palabra es cada canal estaba marcado como desconocido en
`docs/DSP_HARDWARE.md`.

**Prueba** (`dsp/make_iomd_test.py`, `engine.c -DIOMDTEST`, plugin `C-IOMD` en slot 3, sin notas, PLG3):
tres SAW de tono fijo escritos cada muestra desde el slot DCO1 del DSP5: `60B0` = 440 Hz, `60B1` = 660 Hz,
`6020` = 880 Hz. Captura de 6 s con `arecord -D hw:SYSTEM8,0 -f FLOAT_LE -r 96000 -c 2`.

| palabra | resultado en la captura |
|---|---|
| `6020` (IOMD0 `0x20`, la misma zona que guarda DSP4) | **sale**: 880.0 Hz, armónicos 1, ½, ⅓, ¼ (SAW ideal), **canal derecho**, rms 0.0127, pico 0.022 |
| `60B0` (IOMD0 `B0`) | **no sale**: 440 Hz ausente en ambos canales |
| `60B1` (IOMD0 `B1`) | **no sale**: 660 Hz ausente en ambos canales |

Observaciones:

* El canal izquierdo queda en silencio exacto (0.000000).
* Durante el primer segundo aparece un transitorio enorme (valores de 10^17 a 10^20): el arranque del equipo o un
  efecto del cambio de programa. No lo entiendo; el análisis lo descarta (empieza en 1.5 s).
* El "sonido trabado" es esperable en esta prueba: el tono es constante, sin puerta ni envolvente.
* Hipótesis, no medida: la palabra `6020` es la salida de voz que usa DSP4 (`20–24`). Para ser voz (no sólo una
  palabra que llega) hay que confirmar que varía con la nota.

Consecuencia: el camino de salida existe. El siguiente paso es **escribir la voz (SAW con puerta) en `6020`** y
comprobar que sigue la nota y la puerta desde el ARM.

## 5. Pasos siguientes (en orden)

1. ~~Prueba de salida~~ hecha (§4): sólo `6020` llega a la salida.
   Siguiente: voz con nota y puerta escrita en `6020`.
2. LFO en `asm/` (fase en anillo + triángulo sin ramas, `1 − 2|wrap(·)|`); simulación contra referencia float.
3. Envolvente en `asm/` (ataque y caída lineales hacia el sustain, liberación; sin ramas con `min`/`max`).
4. Programa principal propio: lee el tono `[A021]` y la puerta `[A022]`, escribe `[2207]`, envolvente y LFO.
5. Lista de parámetros reducida en `model/params.py` (quitar registros; verificar en el panel).
6. Engine en C que registre sólo nuestros programas (`engine/engine.c`: sin `programs.py`).
7. `build.sh` nuevo para este plugout; instalar en slot 3 con `plugout_tool.py install ... --slot 3 --force`.

## 6. Riesgos y decisiones abiertas

* El programa principal de Roland escribe el tono, la puerta y la salida; retirarlo afecta a todo lo que sigue.
* La mezcla de la voz en DSP4/5 (VCA) y el filtro también son de Roland hoy; la versión mínima los sustituye
  o los omite, lo que hay que decidir con la prueba de salida.
* Ninguno de los nuevos mnemónicos (`mov` de 32 bits) está en la cabecera de `asm/asm.py`: la sintaxis `#v:32`
  funciona pero no está documentada en el módulo (ver `docs/ASM_PENDIENTES.md`).
