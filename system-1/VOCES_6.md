# SYSTEM-1 con 6 voces: análisis y plan

> **2026-10-22:** por el camino de PLUG-OUT ya hay **8 voces funcionando** (C-S1V8, `examples/system_1/`,
> `../docs/examples_system_1_README.md` §7): el OS acepta 8 voces de un asignador de PLUG-OUT y reenvía los ids
> `0x114`–`0x117`/`0x124`–`0x127` al esclavo. Esto resuelve la pregunta "¿el esclavo acepta esos ids?" de §3 y
> es la vía 2 de §5 (un PLUG-OUT propio) sin portar el sinte de serie. Lo que sigue es el análisis para parchear el
> sinte de serie, que sigue abierto.

Objetivo (pedido del 2026-10-22): una versión parcheada del SYSTEM-1 con 6 voces de polifonía en vez de 4. Este
documento reúne lo medido en las imágenes 1.30 (`FIRMWARE.md`) y lo que falta. **No se ha parcheado ni instalado
nada.**

Leyenda: **[código]** visto en el binario · **[estimado]** calculado, sin medir · **[abierto]** sin resolver.

## 1. Resumen

| Capa | Qué fija 4 voces | ¿Cabe 6? |
|---|---|---|
| Maestro: asignador (`0x604A26B0`) | bucles `cmp #4`, índices `& 3`, tablas de 4 entradas | sí, es un parche ARM (unos 30 sitios) [código] |
| Maestro → slave: ids de parámetro | `0x110+v`, `0x120+v`, `0x128+v` (grupo 2) | sí: los bloques tienen 16, 8 y 8 ids [código] |
| Slave: programas DSP de voz | 4 ranuras de voz por memoria de programa | tiempo: **sí** (peor caso unas 740 de unas 925 instrucciones) [estimado]; espacio y datos: [abierto] |
| Slave: motor ARM | escribe tono, puerta, etc. de 4 voces | [abierto] |
| Instalación | el OS no va en el slot 1 (`FIRMWARE.md` §2.2) | o se flashea el OS parcheado o se porta a PLUG-OUT |

## 2. Slave: programas de voz y presupuesto

`python3 system-1/voice_sets.py` lista los "sets" que el slave carga. Un set es una serie de entradas de 12 bytes
`{dirección del programa (bytes), longitud (bytes), puntero}` [código]:

| Dirección | Contenido |
|---|---|
| `0x0000` | cabecera (datos `0000 0001`, luego código) |
| `0x0100` | parte principal (vacía en los sets de voz) |
| `0x1900` | voz 0 |
| `0x3100` | voz 1 |
| `0x4900` | voz 2 |
| `0x6100` | voz 3 |
| `0x7FF2` | cierre de 7 medias palabras |

Las ranuras de voz están separadas `0x1800` bytes. El servicio del slave `0x0100FC40` coloca cada programa de voz
según un índice 0–7 → (A/B, ranura, sección 2–5): son **dos DSP de oscilador (A/B) con 4 ranuras cada uno**
([SH101_POLY.md](SH101_POLY.md) §2) [inferido]. Hay 50 sets: 2 de arranque (voces vacías de 4 instrucciones) y
48 de voz, en parejas idénticas, 24 variantes (probablemente combinaciones de forma de onda). Cada programa de voz
tiene una sección `ef75`/`ef74`. Los cuatro programas de un set solo se distinguen en las direcciones de datos de
cada voz [código]:

| Voz | Palabras del banco 1 | Otros campos |
|---|---|---|
| 0 | `2004`, `2006` | `003c`, `0019` |
| 1 | `200e`, `2010` | `003e`, `001a` |
| 2 | `2014`, `2016` | `005a`, `001c` |
| 3 | `201e`, `2020` | `005c`, `001d` |

**Coste** (1 instrucción = 1 ciclo; límite ≈ 925–945 por muestra, `docs/DSP_HARDWARE.md` §1):

| | instrucciones por muestra |
|---|---|
| peor set, 4 voces | 494 (voces 123/121/123/121 + 6) |
| peor set, 6 voces (4 + 2 copias de la voz más larga) | **≈ 740** [estimado] |
| set más ligero, 4 → 6 voces | 302 → 454 |

El tiempo de DSP no impide las 6 voces en estos sets. No está estudiado lo que corre en los otros DSP del slave (los
dos programas grandes de unas 240 instrucciones, el filtro, las envolventes, la mezcla). Si alguno itera sobre 4
voces dentro del programa, también hay que ampliarlo y medir su coste [abierto].

**Espacio** [abierto]: 6 ranuras con el paso actual (`0x100 + 6 × 0x1800 = 0x9100`) no caben antes de `0x7FF2`. El
programa de voz más largo ocupa `0x44A` bytes, así que con un paso de `0x1000` sí cabrían. Falta saber cómo pasa el
control de una ranura a la siguiente (relleno entre ranuras, saltos absolutos) y si el driver ARM impone el paso.

**Datos** [abierto]: las voces 4 y 5 necesitan palabras propias en el banco 1 y en los otros campos, sin pisar
nada. La separación actual (4, 0xA, 6, 0xA) no es regular: hay que mapear qué ocupa cada hueco.

## 3. Maestro → slave

El asignador envía los datos de cada voz con `set(obj, 2, id, valor)` (`0x600094FC` → `0x60008BD4`), eligiendo las
voces con una máscara de bits de un byte (bits 0–3) [código]:

| Ids | Uso (inferido por el valor) | Hueco del bloque |
|---|---|---|
| `0x110 + v` | nota / tono | `0x110`–`0x11F` |
| `0x120 + v` | velocidad / nivel; a 0 al soltar el sustain | `0x120`–`0x127` |
| `0x128 + v` | disparo (1/0) | `0x128`–`0x12F` |
| `0x130` | pitch bend | — |

Las voces 4 y 5 usarían `0x114/0x115`, `0x124/0x125` y `0x12C/0x12D`. **El esclavo recibe `0x114`–`0x117` y
`0x124`–`0x127`** [medido con C-S1V8, 2026-10-22]; los de disparo `0x12C+` no se probaron.

## 4. Maestro: asignador

Asignador de voces en `0x604A26B0`, código en `0x60009500`–`0x6000A400` (1.30). La inicialización está en
`0x600096E4`: `+0x08` = nota de cada voz (4 entradas), `+0x218` = otra tabla por voz (4 entradas), `+0x18` = 128
entradas por tecla. Modos (`obj[+4]`, elegidos por el parámetro `0x192` del grupo 3): `0x60009608`, `0x600096B8` y
`0x600096B6`. Notas: `0x60009732`, `0x6000994C`, `0x6000A094`, `0x60009D60` y `0x60009E8A` [código].

`python3 system-1/asgn_sites.py` lista 58 instrucciones candidatas (`#4`, `#3`, `0x120`). No todas son de voces:
los `movs r0, #3` de después de `0x6000A2F8` son el grupo 3 de `set()`. Hay que revisar cada sitio a mano.

El objeto ocupa al menos `0x218 + 4×4` bytes. Con 6 voces las tablas crecen y hay que comprobar que caben (lo que
venga después de `0x604A26B0` + tamaño) o mover el objeto [abierto].

## 5. Cómo se instalaría

1. **Flashear el OS parcheado** con el mismo mecanismo que usa el plug-in (`70 20 00 00`, contenedor
   `UPD_POUT_INFO`, `FIRMWARE.md` §2.2) o con el procedimiento de actualización de Roland. Se recupera reinstalando
   `SYSTEM1_UPD.BIN` 1.30 por el procedimiento oficial. `tools/plugout_tool.py` aún no envía contenedores
   `70 20 …`.
2. **Portarlo a un PLUG-OUT del slot 1**: extraer del OS el modelo (maestro) y el motor (slave) del sinte de serie y
   adaptarlos a la interfaz de PLUG-OUT del SYSTEM-1 (regiones en `0x60800000`/`0x60200000`, `SYSTEM1_SYNINFO`,
   tabla `0x0100FC00`). Es un port mucho mayor que un parche.

## 6. Próximos pasos

1. Slave: encontrar el código ARM que usa los sets (tablas de punteros en `0x6004A86C`…: 23 variantes × 4
   columnas) y el que escribe los ids `0x110/0x120/0x128` en los DSP. Así se sabe en qué DSP va cada set y qué
   corre en los demás.
2. Slave: mapear las palabras de datos por voz (banco 1 y los otros campos) para elegir sitio para las voces 4 y 5.
3. Slave: medir el coste de los programas de los otros DSP (`voice_sets.py` solo cuenta los sets).
4. Maestro: revisar los 58 sitios y redactar el parche del asignador.
5. Construir la imagen y comprobarla en el emulador (`examples/from_scratch/emu/`) antes de tocar el equipo.
