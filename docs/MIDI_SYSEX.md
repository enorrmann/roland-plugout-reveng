# MIDI y SysEx en el SYSTEM-8: qué pasa con un mensaje (investigación, 2026-10-21)

Documento de trabajo sobre cómo viajan los mensajes MIDI y SysEx por el sistema: del host (PC) al maestro, del maestro
a los esclavos y de vuelta. Objetivo: un canal de lectura de la memoria de los DSP esclavos
(`SLAVE_DSP_READBACK.md`). Estado: **parcial**. Lo que sigue sin cerrar está al final.

Certeza: **[medido]** = visto en el equipo; **[código]** = leído en el desensamblado de la imagen, sin medir;
**[inferido]** = deducido, no confirmado.

---

## 1. Topología [código + medido]

| nodo | código | qué ejecuta | MIDI |
|---|---|---|---|
| PC (host) | — | `amidi`, `dsppeek.py`, `dsplive.py` | USB-MIDI con el SYSTEM-8 |
| Maestro, núcleo 0 | `m0_core0` (`0x60000000`) | OS (μT-Kernel), router MIDI `0x6000345C`, región 0 (hooks del plugout) | recibe del host y envía al host |
| Maestro, núcleo 1 | `m1_tcm`, `m1_sdram` | control de DSP y efectos globales | — |
| Esclavos (2) | `s_boot`, `s_data` (`0x01100000`, `0x60000000`) | OS del esclavo, región 1 (motor del plugout) | reciben del maestro por **USB bulk** (`BMCBulk`), envían al maestro por el mismo enlace |

Enlace maestro↔esclavo: USB interno. El maestro es host (`BMCBulk%d Attached/Detached`, `0x010125C1` → `0x0100D548`).
Los esclavos no tienen salida hacia el PC (`ARCHITECTURE.md`, «Los slaves no tienen salida»).

## 2. Mensajes que llegan del host [medido]

| mensaje | ¿llega al esclavo? | medido con |
|---|---|---|
| CC de control (`dsplive`: bend 0x465 + CC1 0x467) | **sí** (el motor los procesa) | `dsplive install/poke/recall` (pokes de DSP2/3 en vivo) |
| Parámetros del panel (`set(parte, grupo, id, valor)`) | sí, por el kernel | `ARCHITECTURE.md` |
| SysEx genérico `F0 7D 77 01 02 03 F7` | **no** | prueba del usuario (2026-10-21) |
| SysEx de instalación (`F0 41 10 00 00 ...`) | sí, al maestro (instalación) | `PROTOCOL.md` |

## 3. Recepción de SysEx en el maestro [código]

Desensamblado de `dumps/os_full.bin` (base `0x60000000`, Thumb), con `arm-none-eabi-objdump -D -b binary -m arm
-M force-thumb --adjust-vma=0x60000000`.

| dirección | función | qué hace |
|---|---|---|
| `0x60134692`–`0x601346cc` | manejador de paquetes (wraps) | `ldrb [sp+64] >> 4`: si es **3 (SysEx)** salta a `0x601346cc` → `0x6013459a`; los demás tipos van al despachador `0x6013a9dc` si `[r4] != 0` |
| `0x6013459a` | **reensamblador de SysEx de USB-MIDI** | CIN = `byte0 & 15`: 4 = bytes de SysEx; 5/6/7 = fin con 1/2/3 bytes. Acumula en el objeto `r4`: datos desde `+0x008`, longitud en `+0x208` (`uint16`), límite 508 bytes (`0x1FC`) |
| `0x60134640`–`0x6013464c` | **completar un mensaje** | llamada **virtual**: `r0 = [r4]` (vtable), `r6 = [vtable+4]`, `blx r6` con `r1 = obj+8` (buffer) |
| `0x6013a9dc` | despachador MIDI de mensajes de sistema común | `switch` sobre `0xF1`, `0xF2`, `0xF3` (tabla `tbb`); **no** trata `0xF0` |

**Punto de decisión**: el método `vtable+4` del objeto reensamblador. Ahí se decide qué hacer con un SysEx completo.

**Volcado en vivo (2026-10-21, `examples/midi_send/bulk_dump.py`, hardware)**:

| dato | dirección | valor | archivo |
|---|---|---|---|
| objeto del reensamblador (RAM del maestro) | `0x20082008` | `[0]` = `0x0113EC30` (vtable), `[+0x0C]` = `0x10` | `work/ram/m0_ram_20082000.bin` |
| tabla virtual | `0x0113EC30` | `[0]` = `0x011061D5`, **`[+4]` = `0x01103907`** | `work/ram/m0_vt_0113ec00.bin` |
| método `vtable+4` | `0x01103906` (Thumb) | **`bx lr`: no hace nada** | `work/os/m1_tcm/m1_tcm.elf` |

El objeto y la tabla viven en RAM / en la zona de `m1_tcm` (`0x0113xxxx`), no en `os_full.bin`, por eso hacía falta
volcarlos. **Resultado**: el SysEx completo que recibe este objeto **se descarta**: el reensamblador lo guarda y la
llamada final va a una función vacía. Eso explica por qué un SysEx genérico del host no llega a los esclavos.

**[inferido]**: que este objeto sea el de MIDI 1 (`SYSTEM-8 MIDI 1`, el que usa el host en nuestras pruebas) y no el de
la instalación. El instalador usa MIDI 2 (`bulk_dump.py`), que puede tener otro receptor; no está comprobado. El
literal de `0x60134740` apunta a `0x20082008` solo en la rama de tipo 3 del manejador de `0x60134692`.

**Verificación de direcciones (2026-10-21, tarde)**: cada dirección citada se ha comprobado contra su fuente.

| dirección | fuente | resultado |
|---|---|---|
| `0x20082008` contiene `0x0113EC30` | volcado en vivo `work/ram/m0_ram_20082000.bin` | confirmado |
| `0x2008200C` contiene `0x10` | volcado en vivo | confirmado |
| `0x0113EC30` = `0x011061D5`; `0x0113EC34` = `0x01103907` | volcado en vivo `work/ram/m0_vt_0113ec00.bin` | confirmado |
| `0x01103906` = `bx lr` (`4770`) | **volcado en vivo** `work/ram/m1_code_01103800.bin` (y la imagen estática `m1_tcm.elf`) | confirmado en vivo |
| literal `0x60134740` = `0x20082008` | imagen `dumps/os_full.bin` | confirmado |
| `0x6000345C` = router (`push` de 14 registros) | `dumps/os_full.bin` | confirmado |
| `0x010125C1` → `0x0101257A` → `0x0100D548` | `dumps/kernel_01000000.bin` | confirmado (la cadena de saltos) |

**Corrección**: `0x010125C1` envía **imágenes binarias** (cabecera `BUDB`, dirección de carga, tipo `DATA`/`BOOT`)
al enlace USB de los esclavos (`BASE_FIRMWARE.md` §7). No sirve para reenviar SysEx: el receptor del esclavo trataría
los datos como una imagen a escribir en la dirección de carga. La vía segura para reenviar MIDI a un esclavo es el
router `0x6000345C` con paquetes USB-MIDI de cable 8 (la misma que usa el CC de `examples/midi_send/cc.s`).

**Parche posible**: sustituir `vtable[1]` de este objeto (o la entrada de la tabla) por una función que reenvíe el
mensaje a los esclavos con la rutina de envío por bulk del maestro (`0x010125C1`) y luego llame al método original
(vacío). Requiere escribir RAM/TCM con un hook de región 0 o al instalar. Sin verificar.

Las cadenas de depuración `SysEx Body %02X %02X %02X`, `SysEx End1/2/3` están en `0x6013adb4` y `0x601fcfa8`
(referenciadas con `adr` en `0x6013ab38` y `0x601fcd2c`, dentro del despachador). Son trazas, no la lógica de decisión.

## 4. Enrutado por puerto y cable [código]

Documentado en `BASE_FIRMWARE.md` (router `0x6000345C`):
* Tipo del wrap = `w[7] >> 1`: **3 = SysEx** (`Msg`, puerto en `Msg[6]`), **4 = paquete USB-MIDI** (`{cable<<4 | CIN, ...}`).
* El router trata cada bit de la máscara como un destino: bit 0 → `0x60007794`, bit 1 → `0x6003E32A`, bit 2 → `0x60057CB8`,
  bit 3 → `0x6003F37C`, bit 4 → `0x6003F526`, bit 5 → `0x60052B8A`, bit 6 → `0x6000EFC8`, bit 7/8 → cola de `queue2_post`.
* Los cables 8 y 9 se convierten en las etiquetas lógicas `0x800` y `0x1000`, y el router se llama a sí mismo con la
  máscara base `0x00081603` (bits 0, 1, 9, 10, 12, 19). **[inferido]**: los mensajes de los esclavos entran con cable 8/9.

## 5. Lado del esclavo [código]

Desensamblado de `work/os/s_boot/s_boot.elf` (`arm-none-eabi-objdump -d`, sección `.os` base `0x01100000`). Las cadenas
se referencian con `adr`; `objdump` anota el destino.

| dirección | función | qué hace |
|---|---|---|
| `0x01102bd8` | bucle principal | llama en cada vuelta a RX MIDI (`0x01107786`), depuración `TXMIDI` (`0x011078e2`), RXDT (`0x01107cee`), TXDT (`0x01107e9e`) |
| `0x01107d76` | reensamblador de SysEx | igual que el del maestro (CIN 4–7, buffer en `+0x000`, longitud en `+0x200`) |
| `0x01107d6c` | despachador de eco | pasa el mensaje completo a `0x011078fc` (**eco** del SysEx recibido) |
| `0x011078fc` | enviador de SysEx | empaqueta en paquetes USB-MIDI de 4 bytes; **cable siempre 0** (el paquete lleva CIN 4 en el nibble bajo) |
| `0x011079e6` | cola de transmisión | inserta una palabra de 32 bits en una cola circular de 1024 entradas (`cpsid`/`cpsie`) |

Conclusión: el esclavo tiene un canal SysEx bidireccional con eco. El eco solo se dispara con un SysEx que le llegue, y
los SysEx genéricos del host no le llegan (sección 2).

## 6. Pruebas en el equipo

| prueba | resultado | nota |
|---|---|---|
| SysEx `F0 7D 77 01 02 03 F7` al puerto `SYSTEM-8 MIDI 1` (`hw:3,0,0`) | no llega al esclavo (no hay eco) | sección 2 |
| captura del host (`amidi -d`) con un SysEx del maestro (`dsppeek ping`) | **sí** recibe mensajes del maestro (`F0 7D 03 01 02 03 04 F7`) | la escucha funciona |
| motor con comando de prueba `cmd 2, ext 2/3/4` → SysEx por cable 0/8/9 (`engine/engine.c`, `live_sysex_test`) | **ningún SysEx llega al host** | no se sabe si el motor recibe el CC, si la cola se drena, o si el maestro reenvía al host |

El comando de prueba está en `engine/engine.c` (bloque `#ifdef DSPLIVE`, funciones `live_tx_packet` y `live_sysex_test`).
Está instalado en PLG3 desde esa prueba.

## 7. Lo que no está cerrado

1. **Vtable del reensamblador del maestro** (sección 3): leer la RAM viva del maestro o seguir la inicialización del objeto.
   Con eso se sabe qué función decide el reenvío.
2. **Filtro de SysEx del maestro**: si la función de `vtable+4` descarta los SysEx de fabricante desconocido, el parche
   debe sustituir o envolver esa función.
3. **Si el motor recibe el CC de control**: no hay efecto observable de `cmd 2`. Hace falta una prueba audible o una
   señal en la cola del esclavo antes de culpar al maestro.
4. **Si el maestro acepta los paquetes de los esclavos** (cable 8/9): la prueba de la sección 6 no lo separa de las
   causas 3 y 1.

## 8. Riesgos de parchear el maestro

* Un parche en la región 0 del maestro puede dejar el equipo inutilizable hasta reinstalar el firmware.
* El parche en el esclavo (motor) ya se ha usado varias veces sin dañar el equipo; el parche del maestro no.
* Cualquier cambio requiere reinstalar el laboratorio y seleccionar PLG3 en el panel.

## 9. Herramientas y rutas

| qué | dónde |
|---|---|
| Imagen del OS completa | `dumps/os_full.bin` (2939504 bytes, base `0x60000000`) |
| Imagen del maestro (core 0) | `work/os/m0_core0/m0_core0.elf` |
| Imagen del esclavo | `work/os/s_boot/s_boot.elf`, `work/os/s_data/s_data.elf` |
| Desensamblado completo del OS | `arm-none-eabi-objdump -D -b binary -m arm -M force-thumb --adjust-vma=0x60000000 dumps/os_full.bin` |
| Eco de SysEx en el motor | `examples/from_scratch/engine/engine.c` (`live_sysex_test`) |
| Envío de CC de control | `examples/from_scratch/re/dsplive.py` (`Link.cmd`) |
| Captura del host | `amidi -p hw:3,0,0 -d` (SYSTEM-8 MIDI 1) |

## 10. Historial

* 2026-10-21: mapa de recepción de SysEx del maestro (sección 3) y del esclavo (sección 5); pruebas de eco
  (sección 6).
* 2026-10-21 (tarde): volcado de RAM del maestro. El objeto reensamblador (`0x20082008`) tiene `vtable[1]` =
  `bx lr`: el SysEx completo se descarta. Verificado en el volcado; el puerto al que corresponde, **[inferido]**.

## 11. Parche de reenvío de SysEx (2026-10-21, tarde): aplicado, no se activa

Hook: `examples/midi_send/init_sxfwd.s`, construido con `make_init_sxfwd.sh`, instalado en la ranura 3 como
`JX-3P-INIT-SXFWD` (nombre `SX FWD OK`, versión 1.21).

Qué hace el hook al instalarse (región 0, núcleo 0):
1. Copia las 16 entradas de la tabla virtual `0x0113EC30` en una tabla nueva dentro de la cueva del hook, con
   `[1]` = `fwd`.
2. Pone el puntero del objeto `0x20082008` en la tabla nueva.
3. Escribe la marca `SX FWD OK` en su nombre.

**Verificado**:
* La marca se lee con `tools/plugout_tool.py info`: el hook se ejecuta al instalar.
* Volcado de `0x20082000` (`work/ram/m0_obj_after_patch.bin`): `[0x20082008] = 0x61C001E0`, que es la tabla nueva
  (la cueva está en `0x61C00000` para la ranura 3; el hook se compila para la 1 y la herramienta lo reubica).

**No verificado / resultado negativo**: con `fwd` (que primero envía un CC de marca `4B BF 66 01` por el router y
después los paquetes USB-MIDI del SysEx por el cable 8) ningún mensaje aparece en el host ni en el puerto 1 ni en el
2, con `F0 7D 77 01 02 03 04 F7` enviado por `amidi`. Ni la marca CC ni el eco. Eso indica que **el SysEx enviado
desde el host no llega a `fwd`**: el objeto parcheado no es el que procesa esa entrada MIDI. El SysEx también podría
no pasar del reensamblador; no está separado.

**Riesgo registrado**: el objeto `0x20082008` apunta a una tabla que vive en la cueva del hook. Si se instala otro
plugout, la cueva se reescribe y la tabla queda apuntando a código ajeno. Un SysEx que llegue a ese objeto después
podría colgar el OS. Hasta ahora no se ha producido ningún cuelgue con los volcados posteriores.

**Siguiente paso**: encontrar qué objeto procesa la entrada MIDI del host. Hay que seguir hacia atrás desde el
manejador de `0x60134692` (el que llama a `0x6013459a` con el objeto): quién crea el objeto, qué puerto lo alimenta
y por qué la llamada de `vtable+4` no se produce para `amidi`.

### 11b. Búsqueda del receptor de la entrada del host (2026-10-21, noche)

* El constructor `0x60134708` crea **dos** receptores de SysEx con el mismo código: `A` = `0x20081DFC` (tabla
  `0x0113E49C`) y `B` = `0x20082008` (tabla `0x0113EC30`). Los literales de la función son los que lo muestran
  (`0x60134728` = `0x20081DFC`, `0x60134740` = `0x20082008`).
* **Los dos tienen `vtable[1]` = `bx lr`**: el SysEx completo se descarta en ambos. Verificado en la imagen de
  `m1_tcm` (`0x0111E6B2` y `0x01103906`).
* El hook parchea ahora los dos objetos (versión 1.22). Con un SysEx `F0 7D 77 01 02 03 04 F7` enviado por `amidi`
  a MIDI 1 y a MIDI 2, **no aparece ni la marca CC ni el eco**. Ninguno de los dos receptores es el que procesa la
  entrada del host, o el SysEx no llega a la fase de completado.
* El puntero a la tabla del objeto `B` se mantiene apuntando a la cueva del hook (riesgo de la sección 11).
* Candidatos no probados: (1) la gestión de Rx/Tx de MIDI del sistema (parámetros `0x22C`, `0x22`, `0x1E`, `0x2A`,
  `0x2B`, `0x0C`, leídos en `0x600753A4`: `BASE_FIRMWARE.md`, router), que podría descartar SysEx antes del
  reensamblador; (2) otro constructor de receptor no encontrado con los literales.

### 11c. Segunda cola de entrada y tercer receptor (2026-10-21, noche)

* El productor de la cola de entrada es `0x601F4A6C` (cola en `0x20080370`), con un único llamador
  (`0x601F4D6A`, envoltorio `0x601F4D66`).
* Hay una **segunda copia del bucle de consumo** en `0x601F4CFC`, con su propio receptor de SysEx de tipo 3:
  objeto `C` = `0x20081D88` (literal `0x601F4DB8`), completado por `0x601F4C12`. Objetos `A` y `B` del bucle
  `0x60134684` como en 11b.
* Volcado de `C` (`work/ram/m0_objC_20081d00.bin`): `[C] = 0x01F75516` (tabla virtual); `C+0x208` = 0 en reposo.
  **La tabla virtual de `C` no se ha leído**: `0x01F75516` no está en los volcados ni en la imagen del OS.
* **Incidente**: leer `0x01F75500` con `bulk_dump.py` no devolvió datos (0 mensajes) y el `info` posterior no
  respondió: el equipo se quedó sin USB hasta reiniciarlo. Causa probable: dirección no mapeada en la región 0.
  **No volver a leer direcciones sin confirmar que existen.**
* Tras el reinicio el parche de RAM se pierde (la RAM se reinicia), y la ranura 3 quedó con `BULK` (el último volcado).
* Completado de `C`: en `0x601F4CBC`-`0x601F4CC4`, `[C+0]` → `[vt+4]` → `blx` (virtual, igual que en `B`).
* **Pero** `[C+0] = 0x01F75516` no es un puntero válido de tabla virtual: no está alineado a 4 bytes y no es código
  Thumb. Además, `C` en reposo no tiene la forma de un reensamblador (`+0x208` = 0, resto de campos sin sentido).
  Conclusión provisional: `C` no es un receptor activo, o el puntero que se lee es de otro objeto. **No se puede
  decir qué hace su completado.**
* Próximo paso propuesto: comprobar si el SysEx del host llega a la cola de entrada, comparando con un CC (que sí
  llega al esclavo) enviado por el mismo puerto; la diferencia dirá si el filtro está antes o después de la cola.

### 11d. Sonda de la cola de entrada (preparada, no instalada, 2026-10-21)

Hook: `examples/midi_send/init_probe_q.s`, construido con `make_init_probe_q.sh` (`plugins/JX-3P-INIT-PROBEQ`,
nombre `PROBE Q`, versión 1.30).

* Parchea el envoltorio productor de la cola de entrada, `0x601F4D66` (10 bytes: `nop`, `ldr.w pc,[pc,#0]`,
  dirección del hook). El envoltorio es el único productor de la cola `0x20080370`.
* Cada mensaje que entra cuenta por tipo (`wrap[7] >> 1`: 3 = SysEx, 4 = paquete USB-MIDI) y escribe el resultado en
  el nombre del plugout: `SX<n> P<m>` (n y m, módulo 10).
* Prueba: instalar, luego enviar por `SYSTEM-8 MIDI 1` un CC y un SysEx, y leer el nombre con
  `tools/plugout_tool.py info` tras cada envío.
  * Si sólo sube `P` con el CC y `SX` no sube con el SysEx: el SysEx se descarta **antes** de la cola.
  * Si sube `SX` con el SysEx: el SysEx entra en la cola, y el descarte está en el consumidor (`B` o `C`).
  * Si no sube nada: el puerto no entra en esta cola, o el parche no se ha aplicado (coherencia de caché de código
    en el otro núcleo, no comprobado).
* Riesgo: parchea código en SDRAM desde el núcleo 0. Si el envoltorio está en uso por el otro núcleo, puede haber
  coherencia de caché. Requiere reinicio para retirarlo.

* **Resultados de la sonda (2026-10-21, con PLG3 seleccionado)**. Versión 1.30 (testigo en el nombre): el nombre
  no cambia con CC, SysEx ni con las consultas de `info`; el nombre sólo se fija al instalar, así que no sirve como
  testigo en vivo. Versión 1.31 (testigo en el área NAME0 de `read_diag.py`): tras instalar y tras un SysEx, los
  bytes decodificados siguen siendo el nombre del preset (`String 1 [Str1]`); ni la marca `DEADBEEF` ni los
  contadores aparecen. Sin resultado: no se sabe si el hook corre o si la escritura no llega a la zona que lee RQ1.
