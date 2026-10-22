# SYSTEM-1: imágenes de firmware

Qué hay en cada imagen del sistema operativo del SYSTEM-1, de dónde sale y qué direcciones conocemos. Todo es
análisis estático: en el equipo solo se leyó la versión (`plugout_tool.py info`, 2026-10-22).

## 1. El equipo

`python3 tools/plugout_tool.py info` (2026-10-22, puertos `SYSTEM-1 MIDI 1` = `hw:3,0,0`, `MIDI 2` = `hw:3,0,1`;
la herramienta usa MIDI 2):

```
device   : SYSTEM-1
firmware : 1.30   patches per slot: 64
  (base)  slot 0: SH-2 id 0x0002 version 14.FA
  PLUG-OUT slot 1: SH-2 id 0x0002 version 14.FA
```

El slot 1 tiene el SH-2. Que el slot 0 diga también SH-2 no está explicado (puede que el equipo informe del modo
activo; sin comprobar).

## 2. Fuentes

| Fichero | Qué es | Versión |
|---|---|---|
| `firmware/sys1_sys_v130.zip` → `sys1_sys_v130/SYSTEM1_UPD.BIN` | actualizador oficial de Roland: **imagen completa de la flash, 4 MiB + 16 bytes, sin cifrar** | 1.30 (2016-10-26) = la del equipo |
| `plugins/SYSTEM-1/Script/Code1.Dat` (= `Code.Dat`) | actualización del OS que trae el plug-in SYSTEM-1. **No es un PLUG-OUT** | 1.20 (2015-09-24) |

`python3 system-1/extract_fw.py` descomprime las dos en `work/` (fuera de git):

| Salida | Origen | Tamaño |
|---|---|---|
| `work/sys1_v130/updater.bin` | `SYSTEM1_UPD.BIN` + `0x008000`, `"SYSTEM-1:Updater"` 14-0303 | 174 428 |
| `work/sys1_v130/app_master.bin` | + `0x030000`, `"SYSTEM-1:Appli"` 16-1026_16:53:27 | 703 420 |
| `work/sys1_v130/app_slave.bin` | + `0x130000`, `"SYSTEM-1:Appli"` 16-1026_16:53:27 | 372 760 |
| `work/sys1_v120/app_master.bin` | `Code1.Dat` bloques 0–839 | 702 520 |
| `work/sys1_v120/app_slave.bin` | `Code1.Dat` bloques 2048–2312 | 373 604 |

En el SYSTEM-8 el OS venía cifrado y hubo que volcarlo del equipo (bulk dump, `docs/BASE_FIRMWARE.md` §7.13). En el
SYSTEM-1 **no hace falta volcar nada**: el actualizador trae el código completo. Un volcado solo añadiría el estado
vivo de la RAM.

### 2.1 `SYSTEM1_UPD.BIN`

| Offset | Contenido |
|---|---|
| `0x000000` | cargador `"ESC91.000"`, en claro (desde `+0x30` parece una tabla de vectores Cortex-M: `0x01011698`, `0x01000BDD`, …; sin estudiar) |
| `0x004000` | otra copia de la cabecera `"ESC91.000"` |
| `0x008000` | región `"SYSTEM-1:Updater"` |
| `0x030000` | región `"SYSTEM-1:Appli"`: aplicación del maestro |
| `0x130000` | región `"SYSTEM-1:Appli"`: aplicación del slave |
| resto | `0xFF` hasta 4 MiB; 16 bytes finales `30 00 00 00 2f 02 00 00 00 10 00 00 00 00 40 00` (sin estudiar) |

Formato de región: el de los PLUG-OUTs (`docs/CUSTOM_PLUGOUTS.md` §2–3): cabecera de 0x40 bytes, carga y entrada
`0x60000000`, LZSS con el anillo a cero, suma de 32 bits del flujo comprimido. Las tres sumas coinciden.

### 2.2 `Code1.Dat` del plug-in SYSTEM-1

Contenedor `PLUGOUT_SYS_INF` con nombre `UPD_POUT_INFO`, versión `0x0120`, 4096 registros. START `70 20 00 00`
y END `70 20 00 01` (datos `01 05 03 02`); los PLUG-OUTs usan `70 10 00 00`/`70 10 00 01` y 2048 registros. Es la
actualización del sistema que el plug-in puede enviar. **No se puede instalar en el slot de PLUG-OUT.**

## 3. Aplicación del maestro (1.30)

* CPU **Cortex-M** (Thumb-2; `msr basepri` en la entrada). Sin RTTI. Las rutas de los fuentes quedan en los asserts
  (`midi\task_asgn.cpp`, `dsp_firm\dsp_mgr.cpp`, …).
* Arranque: tabla de ternas {destino, tamaño, origen} en `+0x100`, emulada con Unicorn hasta el salto a la entrada:

  | | 1.20 | 1.30 |
  |---|---|---|
  | código rápido → `0x01000300` | `0x6009BFF0`, 0xF26C bytes | `0x6009BF0C`, 0xF6D0 bytes |
  | datos → `0x01010000` | `0x600AB4E0`, 0xA0 | `0x600AB864`, 0xA0 |
  | **tabla de servicios de los PLUG-OUTs → `0x0100FC00`** | `0x600AB25C`, 0x284 | `0x600AB5DC`, 0x288 |
  | ceros | `0x0100F56C`, `0x010100A0`, `0x20000000`… | `0x0100F9D0`, `0x010100A0`, `0x20000000`… |
  | entrada | `0x01007501` | `0x01007555` |
* El sinte de serie está organizado como un "PLUG-OUT del slot 0" **compilado dentro del OS**: `SYSTEM1_SYNINFO`
  `"SYSTEM-1"` en el maestro y en el slave. No hay regiones separadas como `p1y01_p0m/p0s` del SYSTEM-8, así que
  no se puede reenlazar al slot 1 como S8LAB.

### 3.0 ¿Se puede poner el SYSTEM-1 en el slot 1, como S8LAB? (revisado 2026-10-22)

S8LAB (`examples/sys8_lab/build_s8lab.sh`) funcionó porque `SYSTEM8_UP.BIN` trae el sinte del slot 0 como dos
regiones `SYSTEM-8:Plugout` (`p1y01_p0m.bin` → región 0 en `0x60800000` y `p1y01_p0s.bin` → región 1 en
`0x60200000`). Tienen SYNINFO con `create`/`destroy`, así que basta con cambiar la identidad y empaquetar para el
slot 1.

En el SYSTEM-1 no existe nada equivalente [código]:

* `SYSTEM1_UPD.BIN` solo contiene datos en `0x0000` (cargador), `0x4000` (cabecera), `0x8000` (Updater),
  `0x30000` y `0x130000` (las dos aplicaciones); el resto es `0xFF`. No hay ninguna región `SYSTEM-1:Plugout`.
* El `SYSTEM1_SYNINFO` "SYSTEM-1" del OS (maestro 1.30 `0x6007BA48`, slave `0x60007784`) tiene id `0x0000`,
  versión `0x0100` y **los cuatro punteros de función a cero**. En el SH-2 para SYSTEM-1, en cambio, son
  `create`/`destroy`/getters (`0x608090D9`… en la región 0 y `0x602010D9`… en la región 1). El sinte de serie no es
  un módulo con puntos de entrada: está compilado dentro del OS.

Para tenerlo en el slot 1 hay que **construir** las dos regiones: sacar del OS el código del modelo (maestro) y del
motor (slave), reenlazarlo en `0x60800000`/`0x60200000`, darle `create`/`destroy` y conectarlo a la interfaz que usan
los PLUG-OUTs (tabla `0x0100FC00`, descriptor). Los `BL` al OS se pueden recodificar, porque `0x60800000` está a
menos de 16 MiB de `0x60000000`. Es un port, no un cambio de identidad.

### 3.1 Direcciones (1.30, salvo indicación)

| Dirección | Qué | Cómo se encontró |
|---|---|---|
| `0x0100FC00` | tabla de servicios de los PLUG-OUTs (162 entradas en 1.30, 161 en 1.20; revisada en la 1.20: casi todas son envoltorios de DSP/parámetros y **ninguna envía MIDI**) | tabla de arranque, uso desde la región 0 del SH-2 |
| `0x600A195C` | objeto de entrada de notas (equivalente a `CAssign`), vtable `0x60081008`: `+0x08` note off, `+0x0C` note on, `+0x10` CC, `+0x18` bend (id `0x130`) | constructor `0x60005B2E` |
| `0x604A26B0` | **asignador de voces** (puntero en `0x600094F4`); código `0x60009500`–`0x6000A400` | §4 de `VOCES_6.md` |
| `0x60008BD4` | `set(obj, grupo, id, valor)` → servicio en `[0x60008FA8]` | |
| `0x60006682` | **envío MIDI de bytes**: `send(buf, len, cable, máscara)` → `0x600065B0`; singleton `0x600409C8` | `fnmatch.py` desde 1.20 `0x60006606` |

`fnmatch.py` traslada direcciones de la 1.20 a la 1.30. Ejemplos: `0x60006606` → `0x60006682`, `0x60006534` →
`0x600065B0`, `0x60005AB2` → `0x60005B2E`. Las funciones pequeñas del núcleo tienen otras formas y no coinciden con
las del SYSTEM-8.

### 3.2 Envío MIDI (`0x600065B0`, 1.30)

`send(obj, buf, len, cable, [sp] = máscara)` convierte los bytes en paquetes USB-MIDI (`0x60005D20`, como mucho 86
paquetes) y los envía según la máscara: bit 0 cable 0 y bit 1 cable 1 por `0x60008860` (objeto `0x6009BDE0`; en
la 1.20 `0x600087DE`/`0x6009BEC4`, que comprueba antes si hay conexión); bit 2 por la cola de `midi_drv`; bit 3 al
motor interno. En la 1.20, las respuestas DT1 del propio OS usan la máscara 1. **Sin probar en el equipo.** Con esto se podría hacer un hook de volcado como el del
SYSTEM-8, pero ya no hace falta (§2).

## 4. Aplicación del slave (1.30)

Driver de los DSP (`esc2_dsp_drv`), teclado, MIDI, `SYSTEM1_SYNINFO` "SYSTEM-1" en `0x7783` y presets
`TSYS1-METAP 10HC`…`80HC`. Los programas DSP y su reparto están en `VOCES_6.md` §2.
