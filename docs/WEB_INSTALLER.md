# Instalador web de PLUG-OUTs (`web/`)

Página que instala un `Code*.Dat` en un slot del SYSTEM-1 / SYSTEM-1m / SYSTEM-8 desde el navegador, por Web MIDI
con SysEx. Envía exactamente los mismos mensajes, en el mismo orden, que `tools/plugout_tool.py install … --force
--no-patches` ([PROTOCOL.md](PROTOCOL.md) §2 y §5), con un límite de bytes por segundo que reemplaza la escritura
bloqueante de la herramienta (§5). No hay que instalar nada: es HTML + JavaScript estático, sin dependencias.

Estado: 2026-10-23.
* Pruebas sin hardware: [sim], todas pasan (§6).
* **Equipo real: [abierto]**, todavía no se instaló nada desde la página.

Etiquetas: **[medido]** = probado en el equipo, **[sim]** = pruebas con sintetizador simulado, **[abierto]**.

## 1. Uso

```sh
python3 -m http.server 8000 --bind 127.0.0.1 --directory web     # desde la raíz del repositorio
# abrir http://localhost:8000/ en Chrome / Edge / Brave (Chromium)
```

1. **Web MIDI**: el navegador pide permiso al abrir la página. Hay que aceptar también SysEx.
   * Web MIDI solo funciona en un origen seguro (`http://localhost` o `https://`). Abriendo `index.html` como
     `file://` o desde una IP de la red no funciona.
2. **Equipo**: la página encuentra el puerto de control (§4) y pregunta tipo, firmware y contenido de los slots.
   * El usuario no elige puerto: el cuadro muestra solo *Find synth* y el puerto detectado, marcado
     "(control port)".
   * La lista de puertos está plegada en *Advanced: another MIDI port*. Ahí el puerto de control lleva
     "(control port — use this one)" y el otro del sintetizador "(not the control port)".
   * Si el puerto en uso no es el de control, el cuadro lo avisa y despliega *Advanced*.
3. **Archivo**: botón *Choose file…* o arrastrar el archivo sobre la tarjeta.
   * SYSTEM-1: `plugins/<NOMBRE>/Script/Code1.Dat`.
   * SYSTEM-8: `Script/Code8_<slot>.Dat`, el del slot destino.
   * El archivo no sale del navegador. **No hay archivo por defecto**: siempre se elige.
4. **Slot**: la página sugiere uno, en este orden:
   * el que ya tiene la misma versión;
   * el que tiene otra versión del mismo PLUG-OUT;
   * uno vacío;
   * el 1.
   En el SYSTEM-8 sugiere el slot que indica `Code8_<n>`.
5. **Confirmación**: muestra qué hay en el slot y qué se pierde. El botón *Install* se habilita solo con la casilla
   marcada y sin bloqueos (§3).
6. **Instalación**: START, los registros de datos con la consulta de flujo cada 32, y END. Al terminar indica qué
   PLG elegir en el panel.

Equivale en línea de comandos a `python3 tools/plugout_tool.py install plugins/<NOMBRE> --slot N --force
--no-patches -y`.

## 2. Archivos

| archivo | qué es |
|---|---|
| `web/index.html` | la página: 6 tarjetas (pasos), estilos, sin scripts externos |
| `web/plugout-protocol.js` | el protocolo, sin DOM: contenedor, mensajes, `Port` (preguntas, instalación), Web MIDI. Script plano (sin módulos, funciona con `file://` para leerlo) que expone `globalThis.Plugout`. También funciona en Node (`module.exports`) |
| `web/app.js` | la lógica de la página: pasos, bloqueos, avisos, progreso |
| `web/test/test-protocol.js` | prueba de la biblioteca en Node contra `tools/plugout.py` (§6.1) |
| `web/test/page-test.html`, `page-test.js`, `run-page-test.sh` | prueba de la página completa en Chrome headless con Web MIDI simulado (§6.2) |

`plugout-protocol.js` replica estas funciones de Python:

| JavaScript (`Plugout.*`) | Python | qué hace |
|---|---|---|
| `decodeContainer`, `Container` | `plugout.load`, `Container` | descifrado (XOR encadenado, IV `0xBACADABE`), cabecera, START / END / registros |
| `Container.regions`, `.target` | `Container.image_regions()` | regiones del firmware y equipo destino (§3) |
| `unpack7in8` | `unpack_7in8` | datos 7-en-8 → bytes |
| `buildMessage`, `withData`, `withDeviceId`, `checksum` | `build`, `with_data`, `with_device_id` | mensajes DT1 de Roland |
| `Port.request`, `Port.write` | `Session.request`, `Session.write` | enviar y esperar respuesta en `dirección \| bit 21`. Hasta 3 intentos de 5 s (END 20 s); tira las respuestas viejas antes de cada pregunta |
| `Port.deviceInfo` | `Session.device_info` | tipo, firmware, patches por slot, número de slots, cada slot |
| `Port.install` | `Session.install` | START (`slot \| flags`), registros con `70 30 03 00` antes de cada registro `i % 32 == 31`, `71 30 00 00` = rechazo → aborta, END |
| `planInstall`, `slotState` | `SlotInfo.state_for` + chequeos de `cmd_install` | estado del slot destino y bloqueos |
| `findDevicePorts` | `midi_io.pick_port` | elección del puerto (§4) |

## 3. Comprobaciones

| caso | la página | `plugout_tool.py` | plug-in de Roland |
|---|---|---|---|
| firmware del equipo < 1.30 (SYSTEM-1/1m) / 1.31 (SYSTEM-8) | **bloquea** | sale con error | rechaza |
| el archivo es para otro equipo (§3.1) | **bloquea** | no lo comprueba | elige el archivo por tipo ([PROTOCOL.md](PROTOCOL.md) §3) |
| SYSTEM-8: `Code8_<n>.Dat` en un slot ≠ n | **bloquea** | no lo comprueba | elige `Code8_<slot>` |
| regiones sin cabecera `SYSTEM-x:Plugout` | avisa | — | — |
| el mismo PLUG-OUT en otro slot | avisa | se niega sin `--force` | se niega |
| misma versión en el slot destino | avisa y la reenvía | la salta sin `--force` | la salta |
| otro PLUG-OUT en el slot destino | avisa: se sobrescribe | avisa | pide confirmación |
| banco de patches | **no se envía** | se envía salvo `--no-patches` | se envía |

Para nuestros builds conviene reenviar siempre: un build cambiado conserva el número de versión. Por eso la página
se comporta como `--force`.

El banco de patches queda fuera: necesita `Script.xml` y los `Patch/*.bin` del plug-in ([PROTOCOL.md](PROTOCOL.md)
§6), y los PLUG-OUT propios no lo usan. Para Roland: `plugout_tool.py install <plug-in>` con patches.

### 3.1 Equipo destino de un `Code*.Dat`

Cada región del firmware (cada tramo de registros `70 30 00 09`) empieza con 16 bytes ASCII
`SYSTEM-1:Plugout` o `SYSTEM-8:Plugout`, seguidos de la fecha y la versión de la herramienta de Roland
([PROTOCOL.md](PROTOCOL.md) §4). La página lee el primer registro de cada región (`unpack7in8`, primeros 16 bytes):
* si todas las regiones nombran el mismo equipo, ese es `Container.target`;
* si no, `null`.

Un SYSTEM-1m (tipo 1) acepta imágenes `SYSTEM-1`. Visto en:

| archivo | modelo (cabecera) | regiones |
|---|---|---|
| `plugins/C-S1SY13F/Script/Code1.Dat` (nuestro) | `C-S1SY13F` | 2 × `SYSTEM-1:Plugout` |
| `plugins/JX-3P/Script/Code8_1.Dat` | `JX-3P` | 2 × `SYSTEM-8:Plugout` |
| `plugins/JX-3P/Script/Code1.Dat` | `SYSTEM-8` (sic) | 2 × `SYSTEM-1:Plugout` |
| `plugins/C-BMAP/Script/Code8_1.Dat` (nuestro) | `C-BMAP` | `SYSTEM-8:Plugout` |

El nombre de modelo de la cabecera no sirve para saber el equipo: es el nombre del PLUG-OUT, y en
`JX-3P/Code1.Dat` dice `SYSTEM-8` aunque las regiones son de SYSTEM-1.

## 4. Puerto MIDI

* **Elección** (como `midi_io.pick_port`, pero por nombre y no por orden):
  * entre los puertos cuyo nombre contiene `SYSTEM-8` / `SYSTEM-1`, gana el que tiene `CTRL`;
  * si ninguno, el de nombre de control: `MIDI 2` (Linux, = `hw:N,0,1`), `MIDIIN2` / `MIDIOUT2` (Windows);
  * si ninguno, el segundo en orden alfabético;
  * si hay uno solo, ese.
  * La entrada es la del mismo nombre.
  * Se puede elegir otro puerto en la lista; un puerto que no nombra al sintetizador solo se consulta si se elige a mano.
* **Consultas automáticas**: solo en un puerto que nombra al sintetizador, al abrir la página o al conectarse un
  puerto. Hasta 5 intentos, uno cada 2 s; después hay que usar *Find synth*.
* **`statechange`**: Chrome lo dispara también cuando un puerto solo se abre o se cierra (cambia `connection`), y
  eso pasa en cada ronda de preguntas. La página compara la lista de puertos (`id` + `state`) y solo reacciona si
  cambió: un puerto nuevo, o uno que pasó a `disconnected`. Si desaparece el puerto del equipo, el paso 2 pasa a
  *Disconnected*.
* **Liberación**: después de cada ronda de preguntas y después de la instalación, `Port.close()` cierra la entrada
  y la salida (`MIDIPort.close()`), para que otro programa pueda abrir el equipo.
  * [medido, 2026-10-23] Con la versión anterior de la página abierta en Brave, `plugout_tool.py install` falló
    con `cannot open hw:3,0,1: Device or resource busy`: los clientes ALSA `Chrome (input)` / `Chrome (output)`
    estaban suscritos a los dos puertos del SYSTEM-1 (`/proc/asound/seq/clients`).
  * [abierto] Si `close()` alcanza para que Chrome suelte el puerto ALSA. Hasta comprobarlo: **cerrar la pestaña
    antes de usar las herramientas de línea de comandos** (regla de un solo cliente MIDI).
* **Escucha en todas las entradas del sintetizador**: `Port(output, [inputs])` envía por la salida elegida y escucha
  la entrada del mismo nombre y además todas las demás entradas cuyo nombre nombra al sintetizador. Las respuestas
  se reconocen por dirección y tamaño, así que da igual el puerto por el que vuelvan.
* **[medido por el usuario, 2026-10-23] puerto 1**: instalando por `SYSTEM-1 MIDI 1` desde Brave (Linux):
  * las consultas (§2 de [PROTOCOL.md](PROTOCOL.md)) y START tienen respuesta, START duplicada;
  * la consulta de flujo `70 30 03 00` solo tiene respuesta después del tercer envío (2 reenvíos de 5 s, ≈ 10 s
    cada 32 registros).
  * La página había elegido el puerto 1 porque Brave lista `MIDI 2` antes que `MIDI 1` y la regla tomaba "el
    segundo" de la lista. Ahora elige por nombre (`isControlName`: `CTRL`, `MIDI 2`, `MIDIIN2` / `MIDIOUT2`), y avisa
    en el paso 2 si el puerto elegido no es el de control.
  * [abierto] Si el equipo responde por el otro puerto (ida por 1, vuelta por 2) o si por el puerto 1 no responde
    a tiempo. Con la escucha en todas las entradas, cualquiera de los dos casos queda cubierto.
* **`Port` recibe los `MIDIPort`**: `listOutputs` / `listInputs` devuelven descriptores
  `{id, name, manufacturer, state, port}`, donde `port` es el `MIDIPort` del navegador.

## 5. Ritmo de envío

**Misma secuencia que `plugout_tool.py`**: START, una consulta de flujo `70 30 03 00` antes de cada registro
`i % 32 == 31` (5 s, 3 intentos), END. Lo único distinto es el **ritmo**: la página limita los bytes por segundo.

* **Por qué hace falta**:
  * `plugout_tool.py` envía sin pausas porque su escritura en `rawmidi` es bloqueante. Mientras el buffer USB está
    lleno, el kernel la retiene, y así el envío va al ritmo del equipo.
  * `MIDIOutput.send` nunca se bloquea y Web MIDI no informa el estado del buffer. Sin límite, los registros entran
    de golpe en la cola del navegador / ALSA.
* **Límite**: `WEB_BYTES_PER_SECOND` = 32 000 B/s por defecto; se cambia con `?rate=<bytes por segundo>` en la URL
  de la página.
  * `Port({bytesPerSecond})` envía enseguida mientras va menos de `RATE_SLACK_MS` (20 ms) por delante de ese ritmo,
    y espera después. La media se mantiene aunque los timers del navegador sean gruesos.
  * Duración aproximada: C-S1SY13F (60 KB en el cable) ≈ 2 s; SH-101 (≈ 355 KB) ≈ 11 s.
* **Ritmo de la herramienta** [medido, log de la instalación de C-S1SY13F del 2026-10-23, resolución de 1 s]:
  * 2048 registros (60 KB) en ≈ 3 s;
  * los 40 bloques de la región 2 (24 KB) entraron en menos de 1 s, es decir ≥ 25–40 KB/s.
  * El techo del equipo no está medido [abierto].
* **Espera de respuestas**: por evento (`_wait`): la promesa se resuelve con el propio mensaje MIDI. La primera
  versión sondeaba cada 5 ms con `setTimeout`, que el navegador frena (≥ 1 s en una pestaña en segundo plano).
* **Log de diagnóstico** (*Show the MIDI traffic*), en el registro 32, cada 512 y al final:
  * tiempo acumulado;
  * espera media y máxima de las respuestas de flujo;
  * reenvíos;
  * ritmo usado.
* `new Port(out, in, {gap: 10, wirePacing: true})` reproduce el ritmo del plug-in (10 ms + tiempo de línea a
  31 250 baudios, [PROTOCOL.md](PROTOCOL.md) §5). Son ≈ 7 min para el SH-101; la página no lo usa.

### 5.1 Intentos (2026-10-23, Brave en Linux, SYSTEM-1, firmware SH-101, medido por el usuario)

| versión | resultado |
|---|---|
| sin límite, espera por sondeo, puerto `MIDI 1` | cada consulta de flujo con respuesta recién al 3.er envío (2 × 5 s): ≈ 10 s cada 32 registros |
| sin límite, espera por evento, puerto `MIDI 2` | la 1.ª consulta de flujo sin respuesta y el reenvío respondido en 6 ms: 5 s cada 32 registros |
| sin límite, reenvío de la consulta de flujo a los 250 ms | transferencia en 9,9 s, 20 reenvíos; **END respondió 3**: se perdieron datos. Por lo tanto, la cola no solo perdía la consulta |
| consulta de flujo cada 8 registros | escrita, nunca instalada: la reemplazó la decisión de mantener la secuencia de la herramienta |
| **secuencia de la herramienta + 32 KB/s** | actual; **sin probar** |

## 6. Pruebas

```sh
node web/test/test-protocol.js [Code*.Dat ...]        # biblioteca, ≈ 3 s
sh web/test/run-page-test.sh [/ruta/desde/la/raíz/Code1.Dat]   # página completa, ≈ 2 s (Chrome headless)
```

### 6.1 `test-protocol.js` (Node)

Usa un sintetizador simulado que responde como el equipo: tipo, slots de 48 bytes, 1 byte, flujo de 4 bytes,
START / END, rechazo de registro. Comprueba:
* mensajes, checksum, `withDeviceId`, `unpack7in8`, decodificación de slots, versiones;
* elección del puerto: nombres de Linux, `CTRL`, ningún puerto ajeno sin `allowAny`; las listas llevan el `MIDIPort`;
* motor de preguntas:
  * una respuesta vieja se descarta;
  * SysEx ajeno no provoca reenvíos;
  * sin respuesta: 3 intentos y error `timeout`;
  * `deviceInfo` completo;
  * `close()` cierra los dos puertos;
* instalación:
  * un rechazo de START no envía nada más;
  * un registro rechazado corta ahí, sin END;
  * el device id llega a START, a cada registro y a END;
* `planInstall`: sobrescritura, archivo de otro equipo, SYSTEM-1m, mismo modelo en otro slot, firmware viejo;
* por cada `Code*.Dat` (por defecto, un `C-S1SY*/Code1.Dat` y un `Code8_1.Dat` de `plugins/`):
  * cabecera, los 2048 / 4096 registros, START / END y regiones idénticos a `tools/plugout.py --json`;
  * la secuencia completa de la instalación, mensaje por mensaje.

`tools/plugout.py --json FILE` imprime lo que lee el lado Python: `model`, `modelId`, `version`, `dataCount`,
`start`, `end`, `records` (lista de hex) y `regions` (16 bytes de cabecera de cada región).

### 6.2 `run-page-test.sh` (Chrome headless)

`run-page-test.sh` hace esto:
1. Levanta `python3 -m http.server` en `127.0.0.1:8765` (variable `PORT`) sobre la raíz del repositorio.
2. Abre `web/test/page-test.html` en Chrome headless con un perfil temporal. El navegador se elige con `CHROME=`;
   por defecto `google-chrome` o `chromium`.
3. Lee el veredicto del DOM. Código de salida 0 = `PASS`.

La página de prueba carga `index.html` en un iframe, con `navigator.requestMIDIAccess` reemplazado por un
`MIDIAccess` simulado. **No abre ningún puerto MIDI real.** Escenarios:

1. **SYSTEM-1** (`SYSTEM-1 MIDI 1` / `MIDI 2`, slot 1 con `C-S1SY13`):
   * encuentra el equipo sola en `MIDI 2` y cierra los puertos tras preguntar;
   * sin archivo no se puede instalar;
   * el paso 3 muestra modelo y equipo destino;
   * sugiere el slot vacío;
   * en el slot 1 avisa de la sobrescritura;
   * la casilla habilita el botón;
   * la instalación termina, y lo enviado coincide mensaje por mensaje con START (slot 1) + registros + flujo + END;
   * cierra los puertos y dice "PLG1".
2. **SYSTEM-8 con el mismo archivo de SYSTEM-1**: queda bloqueado, el botón deshabilitado, y no se envía START.
3. **Equipo que no responde** (el simulado dispara `statechange` al abrir y cerrar puertos, como Chrome): como
   máximo 5 rondas × 3 intentos, y después la página espera el botón. Escenario escrito el 2026-10-23, **todavía no
   ejecutado**.

Las esperas se miden en el tiempo virtual de Chrome (`--virtual-time-budget`); por eso los límites son largos
aunque la prueba tarda 1–2 s reales.

## 7. Registro

2026-10-23. Errores de la primera versión de `web/`, encontrados al completarla y corregidos:

| error | efecto | corrección |
|---|---|---|
| `listOutputs` / `listInputs` devolvían copias `{id, name}` y la página las pasaba a `Port` | `this.output.send is not a function`: **no podía hablar con ningún equipo** (lo encontró la prueba de página) | campo `port` con el `MIDIPort` |
| `request()` vaciaba la bandeja y se llamaba a sí misma si llegaba cualquier mensaje | con SysEx ajeno en el puerto, reenvíos sin fin | bucle simple: en JavaScript la respuesta no puede perderse entre el filtro y `take()` |
| sin `withDeviceId` en START / registros / END | con un device id distinto de `0x10`, el equipo no los acepta | igual que Python |
| un `Code1.Dat` fijo junto a la página (el del SH-101 de Roland), sin selector | solo instalaba ese archivo, y lo tomaba solo | selector y arrastrar; sin archivo por defecto |
| `Patch/Patch.Dat` (formato inventado `<u32 largo><SysEx>`) | — | quitado: no hay banco de patches (§3) |
| sin chequeo de firmware, de equipo destino ni de `Code8_<n>` | podía enviar una imagen de SYSTEM-8 a un SYSTEM-1 | bloqueos (§3) |
| consultas automáticas cada 1,2 s a cualquier puerto, que nunca se liberaba | SysEx a otros equipos; puerto ocupado para la herramienta | solo puertos SYSTEM, 5 intentos, `close()` (§4) |
| cada `statechange` reiniciaba los intentos y volvía a preguntar, y la propia pregunta (abrir / cerrar puerto) disparaba otro | con un equipo que no responde, bucle sin fin: el recuadro rojo del paso 2 aparecía y desaparecía [medido por el usuario, 2026-10-23] | solo reacciona si cambia la lista de puertos (§4); corrección sin probar en el navegador todavía |
| la espera de cada respuesta sondeaba con `setTimeout` (5 ms) | instalación lenta, en saltos de 32 registros [medido por el usuario] | espera por evento (§5) |
| envío sin límite de ritmo (`MIDIOutput.send` no bloquea) | consultas de flujo sin respuesta y datos perdidos: END = 3 [medido por el usuario] | límite de 32 KB/s con la secuencia de la herramienta (§5, §5.1); sin probar |
| `test-protocol.js`: el simulado solo respondía mensajes de 13 bytes (una consulta tiene 14) y nunca daba 48 / 4 bytes; la paridad de registros cortaba mal el hex | la prueba fallaba (3 FAIL) | simulado completo; `plugout.py --json` da `records` como lista |

## 8. Abierto

* Primera instalación real desde la página (SYSTEM-1, `C-S1SY13F`), y luego en el SYSTEM-8.
* Si `MIDIPort.close()` libera el puerto ALSA mientras la pestaña sigue abierta (§4).
* `web/Code1.Dat` (imagen del SH-101 de Roland, copiada antes) ya no se usa; no debe distribuirse con la página.
