# PLUG-OUT desde cero: interfaces descubiertas (2026-10-14, sin terminar)

Objetivo: PLUG-OUT propio en C/C++ (región 0 + región 1), un solo oscilador cuadrado.
Herramientas: `re/emu0.py` (emula región 0, atrapa callbacks 0x01000400),
`re/emu1.py` (emula región 1, atrapa tabla 0x0113FC00), `re/tdis.py` (capstone con literales).
Direcciones = JX-3P slot 1 (`plugout_build.py unpack ... --relocs-from Code8_2`).

## Región 0 (CPU del OS, maestro)
* SYNINFO +0x24 init: el OS la llama MUCHAS veces (LoadPlugout 0x60051D32 y 0x60052464
  por parte, params 0x235/0x236/0x237): debe ser idempotente y devolver el descriptor.
  Si devuelve 0 -> 0x60073550 (falla). Luego el OS llama desc[1]() y pone desc[0]=slot.
* Descriptor: [0]=slot, [1]=create, [2]=destroy, [3]=obj parte A, [4]=obj parte B,
  [5]=0, [6]=select(sel, mode) -> setMode del obj.
* SYNINFO +0x28 -> lista de registros {i32 tipo, u32 tamaño, ...} terminada en -1
  (walker 0x600525B4). Tipos: 0 grupo(nombre), 1 parámetro (0x4C: id, 1, min, max, ?, 1,
  lista, display, índice, ..., nombre@+0x3C), 2 cabecera de lista (id, count), 3 etiqueta
  (char[20]), 4, 7. Tabla en CERO = tamaño 0 = bucle infinito: causa probable de los
  cuelgues "receiving plg3" de los PLUG-OUTs mínimos.
* Objeto asignador (CAssignPlugout, vtable 0x619197C8, 17 slots):
  0 reset, 1 (r1==4) refresco, 2 noteOff(n), 3 noteOn(n,vel), 4 CC(cc,v), 5 param 0x22D,
  6 bend(v), 7/8 nop, 9 set, 10 get, 11 set2, 12 flush, 13 nop, 14 setVoices(n),
  15 setMode(n), 16 voces=[this+4]. Parte = {0,1,2,1,2}[this+0xA0].
* 0x01000400 = tabla REAL del kernel (volcado `dumps/kernel_01000000.bin`): [0] get(parte,grupo,id)
  -> valor en r0 (0x01012847), [1] set(parte,grupo,id,v), [2] flush (nop en este OS), [3] touch(parte,a,b).
  get/set ignoran el grupo. Los métodos 9-12 del objeto (`obj->vt[0x24/0x28/0x2C/0x30]`) son sólo
  envoltorios que calculan la parte y llaman a esta tabla (emu0.py la emulaba mal: escribía en r3).
* Protocolo de notas (emulado): noteOn -> set(p,2,0x429+voz,nota), set(p,2,0x43A+voz,vel),
  set(p,2,0x45C,voz); noteOff -> set(p,2,0x43A+voz,0); bend -> set(p,2,0x465,v-0x2000);
  CC1 -> set(p,2,0x467,v); CC11 -> set(p,2,0x46A,v).

## Región 1 (slaves; slave 0 voces 0-3, slave 1 voces 4-7)
* init 0x60A0FCCC devuelve D=desc+4: D[0]=slot, D[1]=create, D[2]=destroy, D[5]=objeto
  principal (CPrmDSPJx3pSys8, vtable 0x60A445CC). SYNINFO+0x28 apunta a 0xFFFFFFFF (lista vacía).
* Slave (s_boot, UpdateSynthPlugout 0x01104226): D=init(); D[1](); D[5]->vt[0](obj); D[0]=slot.
  Otros: vt+8 / vt+0xC (0x011041EE), vt+0x24/vt+0x28 (0x01105B6E).
* Parámetros (UpdateSynthParam 0x01103F2C): obj->vt[0x30](obj, id, 0, valor) para cada id
  (patch 0x6CD.., voces 0x429.., 0x43A..0x44A con valor de 0x0110666A).
* Tabla de servicios 0x0113FC00 (copiada de s_boot 0x0111C95C, 0xF8 B). El JX-3P usa solo
  +0x00 get, +0xA0 setDSP(unidad,id,rampa,flag,[sp]), +0xA4 setDSP(unidad,id,s0),
  +0xB0, +0xF0 (¿slave 1?). ~~El engine NO carga programa DSP~~: FALSO, ver abajo (microcódigo DSP).

## Falta
1. Emular el engine JX-3P: vt[0], luego vt+0x30 con defaults del patch + nota (0x429/0x43A)
   y registrar las llamadas +0xA0/+0xA4 -> ids DSP de pitch, onda (SQR=-2.0), VCA, gate.
2. Ver vt+8/0xC/0x24/0x28 del objeto principal (qué espera el slave).
3. Escribir región 0 y región 1 en C++ (freestanding, arm-none-eabi, Thumb-2 + VFP),
   enlazar en 0x61800000 / 0x60A00000, empaquetar con plugout_build.py, probar en emu0/emu1
   contra el JX-3P de serie y recién después en hardware (slot 3).

## Engine del JX-3P emulado (2026-10-14, `re/run_jx.py`)
* Ids de patch: `0x6CD + offset_nibble/2` del struct `patch` (`re/jxpatch.py`), coincide con la tabla de la región 0.
* Unidades DSP: 0/1 = envolventes/LFO/voz común, 2/3 = osciladores, 4/5 = filtro/VCA; 2 voces por unidad.
* Nota (voz local lv = voz − primera voz del slave; slave 0 = voces 0-3, slave 1 = 4-7):
  `+0xB0(unidad lv>>1, &id 0x21 + 0x12·(lv&1), (nota−12)/12)`.
* Gate: unidad `4+(lv>>1)` ids `0x2A/0x29/0x22` (`0x39/0x38/0x31`; unidad 5 corrida −1) = 1, curva de vel, vel/127;
  envolvente unidad `lv>>1` id `0x22`/`0x34` = 1/0; 4 flags (u0/u1 ids 0x29/0x3B) = alguna voz activa.
* Bend `0x465`: `+0xA0` a u2..u5 (ids 0x22/0x38, u4 0x22/0x34, u5 0xA2/0xB4) con s0 = v/8191.
* `+0xB0(unidad, &desc, s0)`: el slave solo lee el primer word (id).
* vtable del objeto principal (28 slots); el slave usa +0, +8 (cargar DSP), +0xC (init), +0x10, +0x18, +0x24 (refresco), +0x28, +0x30 (setter).
* `+0x28..+0x3C`: dirección DSP por DSP; `+0x40..+0x6C`: descriptores `{dir DSP, bytes, bytes, src}`.
  **El sonido (incluida la onda cuadrada) es microcódigo DSP que trae el engine**, distinto por forma de onda;
  el ISA del DSP es desconocido.

## Microcódigo DSP (2026-10-14, `re/dsp_progs.py`; detalle en CUSTOM_PLUGOUTS.md "DSP microcode in the engine")
* El sonido lo hacen programas DSP que vienen en la región 1 y el slave los baja a sus 6 DSPs.
  `vt+8` (0x60A0FA98) los registra: OS `+0x28+4n` ranura, `+0x40+4n` programa, `+0x58+4n` datos
  (n = DSP 0–5; descriptor `{dir DSP, bytes, bytes, src}`; los tres = `0x01105808`, agregar a lista).
* DSP2/3 (osciladores): ranura `0x6000` = DCO1, `0xB000` = DCO2; programa elegido por 0x6D4/0x6D9.
  SAW y PULSE = mismo programa (0x1008 B) + flag `+0xB0` 0.0/1.0; **SQR = programa propio (0xF88 B,
  DCO1 src 0x60A1D5F4, DCO2 0x60A1E57C)**; NOISE = 0x180 B. DSP4 ranura `0xB000` = efecto (0x748, 6 programas).
* Cambio en vivo: OS `+0x70(dsp, &prog, &datos, 0x60A44E18, ranura, 2, 0, 0)` con nivel en rampa a 0.
* Palabras de 16 bits LE; DCO1 vs DCO2 = mismo código con operandos +0x200 (banco de memoria). ISA desconocido.
* Para el PLUG-OUT propio: copiar los blobs de Roland (principal de cada DSP + SQR) y registrarlos igual.
* Formato (detalle en CUSTOM_PLUGOUTS.md "Microcode format"): NO son tablas de ondas, es código
  (383 valores distintos en 1988 palabras, sin suavidad; va a memoria de programa 0x41480000+n·0x100000,
  copiado tal cual). Palabras de 16 bits LE, instrucción + palabras operando (direcciones 0x04nn/0x06nn).
  Corpus: `re/dsp_corpus.py` -> 199 programas únicos en todas las imágenes. Mapa de opcodes: pendiente.
* Espacios de operandos (CUSTOM_PLUGOUTS.md "Memory map of a slave DSP"): `C0nn` = parámetro `+0xA0` id nn,
  `A0nn` = banco 5 `+0xB0` id nn (flags SAW/PULSE A02A/A02B), `02nn`/`04nn`/`06nn` = RAM local, `22nn` = buzón
  entre programa principal y ranura. Los programas de oscilador NO leen parámetros; el principal se los pasa.
  Herramienta: `re/dsp_wdiff.py` (alinea copias y clasifica operandos).
* **Orden de ejecución**: cada palabra de 32 bits LE lleva 2 instrucciones de 16 bits, **mitad alta primero**
  (verificado en hardware: volcado de la memoria de programa de los DSP del maestro, DSP4/DSP5 tienen los
  efectos de m1_tcm byte a byte). Un blob de 4n+2 bytes: su último `0000` no se carga, la instrucción final
  real es el `0001` que sigue en la imagen. En este orden los 199 programas empiezan `0a02 ff70` y terminan
  `0b02 00f8 0001`. `re/dspmc.py` (palabras en orden de ejecución). Bucle de parámetros: 7 palabras/elemento,
  `80k 3e07 8fe0 8fe2 0fe4 22(k-1) 84k` (pipeline con latencia fija).
* Próximo experimento posible: parchear palabras de un programa de efecto del maestro en vivo (región 0) y escuchar.

## C-SAW (2026-10-14): primer engine propio en C — `engine/`
Región 1 en C (`engine/engine.c`), microcódigo de Roland (programas principales + SAW) sacado del JX-3P en
build (`gen_tables.py`), región 0 todavía la del JX-3P. `test_emu.py`: mismas llamadas al OS que el JX-3P
(registro, estado, notas en las 8 voces). Instalado en slot 3: el equipo lo reporta (`C-SAW id 0x0008 0.01`).
1ª prueba: no se cuelga, no suena (el resto de los PLUG-OUTs sigue bien). Causa probable: 4 ids fuera del
patch (`0x4A1-0x4A3`, `0x73E`) son interruptores que el JX-3P lee del OS; en el emulador valían 0 → tabla con
todo apagado. Ahora el engine los lee en vivo con `+0x00 get`. 2ª prueba: tampoco suena. Causa más probable: los programas de
ranura (DCO1/DCO2/efecto) sólo corren como tareas tras `+0x70` (que habilita el bit de tarea), y el JX-3P lo llama al
recibir onda/tipo de efecto; nuestro engine nunca lo llamaba. Ahora `vt+0x0C` hace la misma secuencia
(`+0xC0` sync, `+0x70` por DSP, rampas). `test_emu.py` compara también estos arranques. 3ª prueba: tampoco suena.
* Diagnóstico híbrido (`engine/hybrid.c`, `hybrid.sh`, `plugins/C-HYB`): engine del JX-3P + el nuestro en la misma
  imagen; slave 0 = JX-3P (control), slave 1 = partes nuestras. Ronda 1 (notas nuestras): suenan las 8 voces.
  Ronda 2 (todo nuestro salvo parámetros → JX-3P): suenan las 8. Conclusión: falla lo que el JX-3P hace con los
  parámetros que manda el OS; nuestras tablas tenían los ids fuera del patch en 0 (valor del emulador).
* `examples/midi_send/params_dump.py` + `init_params.s`: intento de leer esos valores desde la región 0 — falló
  (todos sin escribir): el `get` del maestro no es una tabla en 0x01000400 (eso era el stub de emu0); la región 0
  recibe un objeto del OS y llama `obj->vt[0x28](obj, parte, id, &v)`. Corregir la nota de arriba.
* Ahora: respuestas aprendidas (`gen_tables.responses`): para 30 ids fuera del patch, la respuesta del JX-3P a
  valores 0..255 (recta o tabla); el engine lee el valor real con `+0x00 get` y la aplica (y en `vt+0x30`).
  Dependencias entre ids (26 casos, p. ej. 0x2DF, 0x744) no se modelan.
* **4ª prueba (2026-10-14): SUENA.** Arranca filtrado; al subir CUTOFF del panel suena; las 8 voces y el tracking
  correctos. Ningún otro control responde (esperable: el engine ignora los parámetros del patch). El CUTOFF del
  panel llega como id fuera del patch (grupo 1, 0x2DB-0x2E0, probablemente 0x2DD -> DSP4), que sí tiene curva aprendida.
  Primer sonido de un engine propio en C con microcódigo SAW de Roland.

## Próximos pasos (pendientes, 2026-10-14)
1. **Panel**: respuestas aprendidas también para los ids del patch (`gen_tables.responses` hoy sólo cubre los de
   fuera del patch); el engine aplica los valores reales del patch (`vt+0x30` y `+0x00 get` al enviar el estado).
   Arregla también el arranque filtrado. Ondas (`0x6D4`/`0x6D9`) y tipo de efecto (`0x748`): grabar la secuencia
   de `+0xC0`/`+0x70` por valor (como `swap_ops`) para cargar SQR/PULSE/NOISE y los 6 efectos. Las dependencias entre
   ids (26 medidas en emulación: 0x22C/0x519/0x544/0x56E <- 0x739-0x73C; 0x2DC <- 0x6ED; 0x2DD <- 0x6EC;
   0x2DF <- 0x6FD; 0x2E0 <- 0x748, 0x6F9; 0x3EF <- 0x6CF; 0x736 <- 0x744, ENV2; 0x737 <-> 0x744; 0x738 <- 0x739/A;
   0x739/A <-> 0x700; ENV1/ENV2 <-> 0x744; 0x749 <- 0x748; 0x6F9 <- 0x748; 0x6FE/0x6FF <- 0x6FD) quedan aproximadas;
   después modelarlas o recalcular las curvas con los valores reales.
2. ~~Región 0 en C~~: hecho en emulación (`model/`, ver abajo). Falta la prueba en hardware.
3. Opcional: leer los valores reales de los ids fuera del patch desde la región 0 (arreglar `params_dump.py` /
   `init_params.s` con el objeto del OS) para validar las curvas.

## Región 0 en C (2026-10-14) — `model/` (detalle en `model/README.md`)
* `model.c`: SYNINFO, init/descriptor, create/destroy/select y el asignador (POLY/MONO/UNISON, hold, CC1/11/64/
  120-127, bend, modos de split 0-4) escritos por nosotros; sólo llama a la tabla del kernel 0x01000400.
  `params.py`: lista de parámetros propia (codifica igual que la de serie salvo `JX-3P CH` -> `CHORUS`).
* **Memoria del slot dentro de la región 0**: el OS guarda los patches del slot en `base + 0x8000` (tabla
  0x602B9194, accesores 0x6000870E..0x600087A0): 64 patches × 0x200 (u32 por id, índice id − 0x6CD, nombre en
  palabras 64-67), 64 × 0x898 (patch2), 64 × 0x3710 (patch3 = secuenciador) y buffers de edición en
  +0x10EA00/+0x10EC00/+0x10F498. Por eso la región 0 mide ~1,1 MiB; un stub chico deja al OS sin esa memoria.
  Nosotros llevamos 64 `INIT PATCH`; el banco del plugin los sobrescribe al instalar.
* `model/test_emu.py`: 1000 secuencias aleatorias × 120 eventos dan las mismas llamadas al kernel que la región 0
  del JX-3P; una mutación deliberada se detecta. `engine/build.sh` ahora arma `plugins/C-SAW` con las dos
  regiones nuestras (`MODEL=jx` vuelve a la región 0 de serie). **Sin probar en hardware.**

## Qué sigue dependiendo de Roland en C-SAW (2026-10-14, región 0 propia verificada en hardware)
| Parte | Origen | Estado |
|---|---|---|
| Región 0 (modelo, lista de parámetros, memoria del slot) | `model/` | **propia**, suena en hardware |
| Código del engine (región 1) | `engine/engine.c` | propio |
| Metadatos del contenedor (`plugout.json`) | `engine/build.sh` | **propios** (antes: `unpack` del JX-3P) |
| `Script.xml` + `Patch/` en `plugins/C-SAW` | plantilla JX-3P | **eliminados**: sin Script.xml `plugout_tool.py install` manda sólo el firmware; los 64 INIT están en la región 0 (`MODEL=jx` los sigue copiando) |
| Microcódigo DSP (6 principales, SAW/SQR/NOISE, 6 efectos; 48 KB) | copiado del JX-3P al compilar, **estático** (`engine/programs.py`, offsets + SHA-256, sin ejecutar nada) | **pendiente**: requiere el ISA del DSP (assembler propio); el programa vacío y los bloques de datos ya son nuestros |
| Tablas del engine (estado, curvas, rampas, velocidad, registro, cambios de programa) | antes: `gen_tables.py` ejecutaba el engine del JX-3P | **propio** (2026-10-14): modelo `engine/model_spec.py` + intérprete en `engine.c`; sin probar en hardware |
| Patch por defecto de esas tablas | `Script.xml` del JX-3P | ya no hace falta: el engine lee los valores reales con `+0x00 get` |

Orden propuesto: (1) modelo propio de parámetros en el engine: fórmulas por id (recta / tabla) escritas por nosotros
a partir de lo medido, y el estado inicial calculado con ellas desde `INIT`; `gen_tables.py` queda sólo para copiar
microcódigo y como referencia de test. (2) ISA del DSP (`re/dspmc.py`, `dsp_wdiff.py`): sin eso el microcódigo no se
puede reemplazar.

## Modelo de parámetros propio (2026-10-14) — `engine/model_spec.py` (detalle en `engine/README.md`)
* Medido como caja negra (JX-3P en emu1, sólo referencia): barridos 0..255 por id (`re/measure.py`, a precisión
  float32 completa: `Emu1.PREC = None`), dependencias cruzadas (`re/deps.py`: 29 ids cuya escritura depende de
  otros), sondas y barridos en contexto (`re/probe.py`, `sweepfit.py`, `spec_blocks.py`), nodos (`re/knotfit.py`).
* Semántica: cada salida pertenece al parámetro que la dispara; varias fuentes escriben el mismo id DSP y gana la
  última (p. ej. cutoff `P4:0x20` <- 0x6EA, 0x2DB, 0x7E4). Al refrescar: orden del JX-3P.
* Hallazgos: ids de parte 0x2DB-0x2E0 = ajuste en % que lleva el valor del patch hacia 255 (`floor(p+(255-p)q/100)`);
  0x3EF multiplica la velocidad del LFO; Circuit Mod (0x744) = dispersión por voz de tiempos de envolvente, detune
  y filtro; UNISON añade offsets de detune por voz; 0x22C/0x519/0x544/0x56E = estados de ruteo de parte (5, por
  slave); el JX-3P cachea un flag de sens en su handler de 0x22C (el modelo sigue los valores actuales).
* Parámetros float (grupo 5): 0x74D y 0x7E4 llegan del OS como bits float32 y se escriben tal cual. 0x7E4 es el
  cutoff que el JX-3P escribe último al refrescar; tratarlo como entero 0 dejó el filtro cerrado al arrancar en el
  equipo (el emulador devolvía 0 = 0.0 y no lo mostró). 0x2DB lleva ese valor hacia 1: `F + (1-F)·q/100`.
* Tests: `engine/test_model.py` (modelo vs JX-3P: 0 diferencias en cambios con tolerancia 3 %, 0 en refresco;
  error máximo 2 %) y `engine/test_emu.py` (engine compilado vs JX-3P: registro, estado de arranque, cambios de
  los 72 parámetros, swaps de onda/efecto, mute/unmute, notas: todo igual). **Sin probar en hardware.**

## Motor DSP en el VST3 (2026-10-14) — `re/vst_rtti.py`, `re/vst_sim.py` (sólo análisis estático)
`plugins/JX-3P/JX-3P.vst3` (1.0.7, x86-64) **no** trae el microcódigo (ningún fragmento de los 17 programas, en
ningún orden de bytes). Comparte con la región 1 las tablas float del engine (>100 KB de 0x60A258A0… = `.rdata`
0x9C8744…) y las clases de control `CDSPJx3p{Env,Lfo,Osc,Flt,Amp}Voice`, `VoiceCmn`, `VoiceMix`, `Mst`, `Noise`,
`PatchLev`, `Efx{Od,Ds,Fz,Cr,Ch,Ph,Cmn}` (= el código ARM del PLUG-OUT; métodos = lookup en tabla + escritura de
parámetro). Las escrituras van a `CJx3pSim`, un **modelo DSP nativo en float**:
* 1138 parámetros con nombre registrados en 0x180382BF0 (`vst_sim.py params`): por voz 110 (stride de índice
  0x6E, de campo 0x3F80): LFO (Rate, Delay, Sin/Sqr/S&H Sw, Trig…), ENV1/ENV2 (Attack/Decay/Sustain/Release,
  "LFO trigger env sw"), OSC (Feet, Detune, Sync, Pulse Sw, Bend, LFO/Env mod, PWM), LPF (Cutoff, Resonance, Env/LFO/
  Key/Velocity mod), AMP; luego común: HPF, efectos (OD/DS/FZ/Crusher/Phaser/Chorus/Flanger), delay, reverb, master.
* `CDSPJx3pEnvVoice` ctor (0x180375FC0, voz 0..7) fija los índices: voz 0 = 0x1F (LFO trigger env sw), 0x20-0x23
  (ENV1 A/D/S/R), 0x24, 0x25-0x28 (ENV2); en el ARM los mismos métodos escriben P{u}:0x2F+etapa(+0x17) / 0x33+etapa.
* Render: 8 funciones de ~33 KB, una por voz (0x1803B2530, 0x1803BA830, … 0x1803EB820; ~5900 instrucciones SSE,
  cada una usa los 110 campos de su voz) = LFO + envolventes + osciladores + LPF + AMP; 0x1803AA580 = parte común
  (HPF, efectos, delay, reverb, master). Helpers: 0x1803B23F0/0x1803B2480 (curvas con pow), 0x1803B2270
  (polinomio por tramos, tabla de 0xD0 bytes por tramo).
* Uso: especificación legible del algoritmo (qué calcula cada programa de microcódigo y con qué parámetros) y
  nombres para los ids DSP; el equipo sigue necesitando microcódigo (el DSP es el que genera audio).

### LFO y envolventes según el render del VST (Ghidra, `re/ghidra/`, 2026-10-14)
Descripción propia del algoritmo (voz 0, 0x1803B2530; constantes de 0x180391630; dos juegos por frecuencia de
muestreo). Los valores que escribe el lado de control (= nuestros P0/P1 del modelo) son los parámetros de esto.
* **LFO** (por muestra): rate 0..1 (o el de la entrada externa / tempo) -> `f = 0.46624·e^(5.28342·rate) − 0.41338`,
  incremento de fase `f/65536` (máx. 0.015534) × Rate Ratio × factor de frecuencia de muestreo. Fase en [−1, 1)
  con wrap; LFO Trig / Reset Sw intervienen en el reinicio de fase (detalle sin verificar). Formas: **Sin** = polinomio sobre |fase|
  `−1.181x³ + 1.8136x² + 1.3674x − 1` (de −1 a 1, aproximación de seno a partir de un triángulo); **Sqr** = signo
  de la fase; **S&H** = muestra del ruido LFSR de 24 bits en cada vuelta de fase. Salida = suma de cada forma × su
  switch. **Delay**: rampa 0..1 desde la nota (LFO Delay) que multiplica la salida si LFO Delay OnOff.
* **ENV1/ENV2** (a 1 kHz: contador que suma 1/48 o 1/88.2 por muestra y dispara la etapa al volver a −1). Gate =
  gate de la voz (o el cuadrado del LFO si "LFO trigger env sw"); flanco ascendente -> fase de ataque.
  **Attack** lineal: `env += Attack` por tick hasta el pico 1.0 (fin del ataque). **Decay**: un polo hacia
  `S = Sustain`: `env = S + (1 − Decay)(env − S)`, sin bajar de S mientras dura el gate. **Release** (gate off):
  `env = (1 − Release)·env`, piso 0. En ambos se suma una deriva pequeña `−1.5e−5·(0.5 − env)` si env < 0.5
  (imitación del circuito analógico). O sea, Attack/Decay/Release son incremento/coeficiente por tick, no tiempos.
* Modulaciones (siguiente bloque): pitch = Detune + Feet + bend×range + ENV1×Env1 Mod Level (con inversión) ×
  Osc Env Mod Sw + LFO×Lfo Mod Level × Osc LFO Mod Sw (+ mod wheel); PWM = nivel PWMod; cutoff = LPF Cutoff
  pasado por una cúbica y saturado a 0..1, más las modulaciones de ENV/LFO/Key/Velocity.

## ISA del DSP: primeros hallazgos (2026-10-14, estático) — `re/dspdis.py`
* **Longitud de instrucción = 5 bits bajos de su primera palabra** (en palabras de 16 bits, orden de ejecución).
  Los 84 programas únicos del corpus (`work/corpus`: los 6 PLUG-OUTs + OS: s_data, m1_tcm…) se dividen exactos
  con esta regla; con 4 bits fallaba sólo SAW (`9692` = 18 palabras). Longitudes 1–18; `0001` = instrucción de 1
  palabra (frecuente: ¿nop / burbuja?). 119 valores distintos de los 11 bits altos.
* Forma de una instrucción (VLIW): cabecera, palabras de control (`XfeY` = registro X en la posición Y/2, y otras
  como `99b1`, `b9b2`, `be0c`), referencias de memoria (`02nn` local, `C0nn` parámetro, `A0nn` banco 5, `22nn`) y a
  veces **inmediatos de 32 bits Q31** al final: `0155 5555` = 1/96. Ejemplos: `1e05 Afe0 Bfe2 m1 m2` (dos refs, dos
  registros), `0c03 Afe0 m` (una ref; al inicio de MAIN0 se usa con r8 sobre todo el estado = borrado), `0903 hi lo`.
* **Ancla con el VST**: `6507 0c70 be0a 99b1 b9b2 0155 5555` = contador de tick de la envolvente (+1/192 por
  muestra en formato RAW, ver abajo; en el VST +1/48 a 48 kHz), 4 veces en MAIN0/MAIN1 = 2 voces × 2 envolventes.
* **Mapa de MAIN0** (DSP0, voces A/B; MAIN1 igual con otras direcciones). Voz A palabras 31–1212, voz B 1213–
  (mismos bloques, ids +0x17): parámetros de voz/portamento (C020–C025, A020–A025), LFO (A026 ← 0x4A1, A027 ← 0x702,
  **A028 = rate** ← 0x3EF/0x6CF en `1409 … a028 2775b5ea a69d3813` con dos inmediatos, A02D delay ← 0x6CE, A02C
  forma ← 0x6CD, A02A ← 0x6D3, C026–C02E), **ENV1** (tick en 818, **Attack C02F en 859** `6907 … 024e c02f`, Decay/
  Sustain C030/C031 en 874, Release C032 en 913), **ENV2** igual en 1068–1163 (C033–C036). Estado de la ENV1 de la
  voz A en 0x0246–0x0253 (0x024E = valor de la envolvente, candidato).

### Formato numérico del DSP (medido en el equipo, 2026-10-14) — `re/dspnum.py`
* Mapa de memoria de los DSP (tabla de la pantalla de debug `CEZUtilDSPDump`, `m1_sdram` 0x61014BB0, entradas
  {flags, nombre, dirección, bytes}): DSP n = `0x41400000 + n·0x100000`: DTn0 (banco 0, 0x800 palabras) IEEE `+0`,
  SD31 `+0x2000`, RAW `+0x4000`; DTn1 igual en `+0x10000` (en el DSP4 del maestro lee 0 y no guarda lo escrito);
  `+0x40000` (0x3000 B), `+0x60000` (0x10000 B), ENVn `+0xA0000` (= bloque de parámetros con rampa);
  DPCn (banco 5) `0x41114000 + n·0x4000`, vistas IEEE/SD31/RAW cada 0x1000; DPCM, DTM, PRGM (`0x41240000`), MTCB,
  WM2418 (el DAC/códec), IOMD, MXM (mezclador), DP01/23/45.
* **RAW** = la palabra interna del DSP: bit 31 signo (signo-magnitud), bits 30-29 exponente s (×16 por paso), bits
  28-0 mantisa M: **valor = ±M / 2^(32−4s)**. 1.0 = `30000000`, 0.5 = `28000000`, −1 = `B0000000`, 100 = `66400000`.
  Verificado escribiendo 74 floats por la vista IEEE de DPC4 (0x41124E00) y leyendo SD31/RAW (`bulk_dump.py --tri`,
  `CONV=conv.bin`): 74/74. SD31 = Q31 saturado.
* Los inmediatos del microcódigo están en RAW. MAIN0/MAIN1 contienen las constantes del render del VST: LFO
  `0.466238·e^(5.283422·rate) − 0.413384` (`2775B5EA`, `45488E4F`, `A69D3813`), tope de incremento 0.015534
  (`03FA0945`), deriva de la envolvente −1.5e−5 (`8000FBA8`), tick 1/192 (`01555555`); cerca del LFO, `0807 3cca …`
  con 2/315, 4/45, 2/3, −2/3, 1 (Horner de un polinomio). Las del seno del VST (1.181, 1.8136, 1.3674) no aparecen.
* Herramientas en el equipo: `examples/midi_send/bulk_dump.py --tri` (por palabra, las tres vistas leídas seguidas;
  `VSTRIDE=0x1000` para DPC) y `CONV=<dir>/conv.bin` (escribe esa tabla en SRC antes de leer). `bulk_dump.py`
  busca el puerto "SYSTEM-8 MIDI 1" solo (antes fijo en hw:3).
* Volcados: `dumps/dsp{4,5}_dt0{,_tri}.bin` (efectos del maestro en marcha), `dumps/conv_dpc.bin`.

### Pruebas en el equipo sobre los DSP del maestro: abandonadas (2026-10-14)
`re/dsprun.py` + hook (SAVE/POKE/SNAP/RESTORE en `init_bulk.s`): reemplazar el programa de DSP4 0xEFDC un instante
funciona y se restaura, pero el ARM **no puede escribir el banco DT0** (ni por la vista RAW ni por la IEEE; sí el
banco 5 / DPC y la RAM de la región 0) y un programa de prueba con E/S en el banco 5 no mostró cambios. Sin un
canal de observación fiable el usuario pidió otro camino: análisis estático con el VST como referencia.

### Osciladores: VST vs microcódigo (estático, 2026-10-14) — `re/vst_anchor.py`
* **VST** (render de voz, OSC1 ~línea 1050 de `voice0.c`, OSC2 ~2380; salida `st_d30`/`st_d90`, usada una muestra
  después): pitch + `st_1310` (−4.75) -> `poly_table` (doble precisión) -> incremento, tope ±512 y `st_17d0`
  (0.249); fase en [−1, 1) con wrap; **4 sub-muestras por muestra** (posiciones 0.25/0.5 para el sync);
  transición suavizada `tri((fase+1)/2)·256·(1/256)/inc`, saturada a ±1, polinomio impar (`st_1a40…1a80`) y
  cadena de retardos (`st_14d0…17a0`, FIR de diezmado). Dos ramas según un modo (`*(sim+0x88)+0x28`), una por
  frecuencia de muestreo.
* **Anclas** (`vst_anchor.py init.c work/corpus/JX_*.bin`: cada constante de la inicialización del VST en RAW,
  buscada como inmediato): en SAW/SQR (DCO1 `6000_60a1b594`/`…d5f4`, DCO2 `b000_…`): −4.75 en la instr. 13 (voz B
  1026), ln2 en 17, la serie de 23–59, 0.249 en 94 (1107), **π/2 en 201/373/545/717 = las 4 sub-muestras**. El mapa
  completo une también LFO, envolventes (MAIN0/1), filtro y VCA (MAIN4/5: −3.29, 11.5, 0.675, 14, 5.5…).
* **Exponencial del pitch en el microcódigo**: x = (pitch − 4.75)·ln2 y luego e^(2y) con la serie 2ⁿ/n!
  repartida en dos cadenas de Horner en paralelo: pares 1, 2, 2/3, 4/45, 2/315, 4/14175 y impares (÷2) 1, 2/15,
  4/315, 2/2835, 2.565e−5 (= (2¹¹/11!)/2). El VST usa una tabla en su lugar (las mismas constantes están en su
  inicialización, `st_1360…13c0`, sin uso en el render).
* **Primer opcode con semántica (hipótesis fuerte, por el patrón de coeficientes)**: `0807 3cca 1cb6 A B` (y `0806 … A`) = paso de Horner doble: dos
  multiplicar-sumar en paralelo con dos inmediatos RAW de 32 bits; `1108 3472 20a1 0036 0001 A B` arranca las dos
  cadenas; `0904 0c20 imm` / `0904 1430 imm` = operación con un inmediato (probablemente suma −4.75 y producto ×ln2; sin verificar).

### Formato de instrucción: transferencias y referencias (estático, 2026-10-14) — `re/dspvar.py`
* **Transferencias `XfeY`**: X = 0–7 **lee** memoria al registro X, X = 8–15 **escribe** en memoria el registro X;
  Y/2 = índice de la referencia. Evidencia: los parámetros `C0nn` (sólo lectura para el DSP) aparecen siempre con
  X = 0–7 (136 de 136 en `1e05`); el gate de la voz `0207` (MAIN0) se escribe una vez desde `A022` —el gate que manda
  el ARM (`note_gate`, banco 5 id 0x22)— en la instr. 40 `1e05 8fe0 1fe2 0207 a022` (`[0207] ← w8`, `r1 ← [A022]`)
  y se lee tres veces (`1fe2` en 78, 735, 985); al inicio de MAIN0 `0903 0050 0000` + `0c03 8fe0 0200/0202/0204`
  (y 0262/0264/0266 voz B) = las tres entradas del jack externo del VST (`st_b0/d0/f0`) puestas a 0.
* **Referencias = una por transferencia**, contiguas, en el mismo orden, después de la última transferencia y de
  0–4 palabras "puente"; luego, inmediatos. Medido alineando copias del mismo código (mitades de voz A/B,
  DCO1/DCO2, instancias de efectos): las palabras que cambian entre copias están exactamente ahí (1025× 1-1-0,
  955× 2-2-0, 545× 2-2 con 1 puente, 484× 1-1 con 1 puente, 233× 3-3-0…). Sólo transferencias: `0c03` (1),
  `1e05` (2), `3e07` (3), `7809` (4), `e40b` (5).
* **Palabras de operación** (antes de la primera transferencia): casi todas `X op Y` (nibble, byte central,
  nibble): `X fe Y` (transferencia), `X 9b Y` (`99b1`, `b9b2`…), `X 50 Y`, `X 44 Y`, `X cc Y`… ~200 bytes centrales.
  Hipótesis: byte central = operación, nibbles = registros (16).
* **Inmediatos**: RAW de 32 bits, pero no siempre alineados a palabra: en el SAW `0807 3cca 1d62 4200 002a aaaa aa00`
  lleva 2.0 (`42000000`) y 2/3 (`2aaaaaaa`) desplazados un byte; el empaquetado depende de la operación.

## ISA del DSP medido en el equipo por audio USB (2026-10-14) — `re/dspaudio.py`, `re/dspexp/audio/`
**Canal de observación**: el audio USB del SYSTEM-8 (`arecord -D hw:SYSTEM8,0 -f FLOAT_LE -r 96000 -c 2`, sólo
FLOAT_LE). Hay un filtro de continua en la cadena: una constante sólo se ve como transitorio. Estímulo: el ruido que
el principal escribe en `[2200]` en cada muestra (lo lee el programa NOISE). Medida: rms y 4 bandas de espectro
(0–300, 300–1k, 1k–3k, 3k–8k Hz) contra la calibración, con la misma nota. Con ruido blanco, x² también es plano.

**Banco de pruebas** (`DSPTEST=1 MC_TEST=… engine/build.sh`, plugin `plugins/C-DSPT`, slot 3):
* `programs.py`: `MC_TEST=prefijo=archivo,…` reemplaza programas (`dco1_=z_dco1.txt` silencia el DCO1;
  `t0=`/`t1=` = programas de prueba). Archivos = palabras hex en orden de ejecución, `#` comentarios.
* `engine.c -DDSPTEST`: el slot DCO2 del **DSP2 corre t0 y el del DSP3 t1**, cargados al arrancar (sin swaps en
  vivo). Cada programa tiene camino de voz A (salidas `[2221] [221d]`) y B (`[2229] [2225]`) → **4 pruebas por
  instalación** (`d2a= d2b= d3a= d3b=` en el `.batch`; `dspaudio.py` arma los dos programas, `work/dspaudio/prog_d*.txt`).
  `model_spec.py` con `DSPTEST`: nivel de efecto P4:0x49 fijo en 1 (el slot de efecto con el patch del equipo no
  se oyó; no usado).
* Flujo: `dspaudio.py install bNN.batch` (con `--force`: sin él `plugout_tool` se saltea la instalación si nombre y
  versión coinciden, "already in slot 3") → **el usuario selecciona PLG3 a mano** (tras instalar el equipo vuelve a
  SYSTEM-8) sin tocar notas → `dspaudio.py recall bNN` (8 notas distintas, una por voz) → `dspaudio.py report bNN`.
* **Mapa de voces** (calibración `b11`, ganancias 1/8 1/4 1/2 1): notas 60 62 64 65 / 67 69 71 72 (recién
  seleccionado, poly) = DSP3-B, DSP3-A, DSP2-B, DSP2-A en cada slave; cada prueba sale replicada en los dos slaves.
  El nivel sube un poco con la nota (seguimiento del filtro): `report` normaliza contra `b11` nota por nota.
* Swaps en vivo (`+0x70`) desde eventos de nota: **no fiables** (programa mudo o se queda el anterior). Cada slave
  sólo hace el swap de sus voces y el asignador alterna slaves; tampoco con nota selectora separada. Causa sin aclarar
  (el swap del OS, `0x01105F94`, no usa doble buffer: para la tarea, escribe programa y datos, `+0x1105f2c` pone a
  un valor el rango de parámetros `x0..x0+x1`, `ctx` = pares {dir, tamaño} a limpiar, rehabilita la tarea).
* De paso, **bug del motor corregido**: los descriptores de programa/datos del JX-3P son de 6 palabras
  `{dir, bytes, bytes, src, 0, 0}` (las dos últimas las usa el OS) y uno por DSP y por programa (el JX-3P tiene
  tablas separadas para DSP2 y DSP3, `0x60A0B878`/`0x60A0B87C`, y un descriptor de datos tras cada programa). Los
  nuestros eran de 4 palabras y compartidos entre DSP2/DSP3; ahora `dspdesc_t` tiene `os[2]` y todos son únicos.
  `test_emu.py` sigue OK (el emulador no modela esas palabras). **Falta probar el C-SAW normal en el equipo** con
  cambios de onda (SQR, NOISE) y de efecto.

**Confirmado** (todo replicado en dos slaves salvo donde se indica; x = ruido de `[2200]`):
| Instrucción | Efecto | Medida |
|---|---|---|
| `0c03 Xfe0 ref` / `1e05 …` / `7809 …` | transferencias: X 0–7 carga rX ← [ref], X 8–f guarda [ref] ← r(X−8); Y/2 = índice de la referencia | copia de `[2200]` a la salida = referencia |
| `0904 d43s imm32` | rd = rs × imm (inmediato RAW de 32 bits) | ×0.5 → 0.50, ×2.0 → 2.15 (lote viejo), ×1.0 → 1.02 |
| `0903 d41s imm16` | rd = rs × imm (16 bits = mitad alta del RAW) | ×0.5 → 0.50 |
| `0903 d050 imm16` | rd = imm | r1 = 0.5 → 0.50 (vía `0401`) |
| `0902 0401` | r0 = r0 × r1 | 0.5x·0.5x = x·0.25x → 0.100 = 0.100 |
| `0902 0081` | r0 = r0 × r1 (otra multiplicación, op 08) | 0.20 (= 0.5x²) |
| `0902 0c01` | r0 = r0 + r1 | 1.51 |
| `0902 04c1` | r0 = r1 (copia) | r1 = 0.5x → 0.50, r1 = 2x → 2.00 |
| `0001` | nop de 1 palabra | no cambia nada |
| encadenado | una instrucción ve el resultado de la anterior (sin latencia visible) | `0430` ×0.5 → `1430` ×1 sobre r0 → 0.50 |
* Constante −1 (`0903 0050 b000`): transitorio negativo al cambiar programa; +1 (`3000`) residuo DC opuesto.
  `0904 0c20 imm` sobre el ruido ≈ 1× (compatible con suma de inmediato; el DC no se ve).
* Palabra de operación de una instrucción `09xx` = `d op s` (nibble destino, byte operación, nibble fuente).

**Abierto**:
* `0902 1080` (op 08, destino 1, fuente 0) dejó **r1 sin cambiar** (0.50 con r1 = 0.5x): `08` no es
  `rd = rd × rs`; ¿escribe siempre r0? Probar sacando r0.
* `0902 0541` (op 54): con r1 = 0.5x da 2.0 / 2.3, con 0.25x 1.10 / 1.22 (distinto por slave → no lineal: ¿saturación,
  división, valor absoluto?). Probar con r1 constante (`0903 1050 imm`) y amplitudes distintas.
* `0904 0060 3000 0000` sobre el ruido: silencio (b07); sin ruido, ruido con pasabajos de registros viejos: ¿acumulador?
* Ops frecuentes sin probar: `4c` con otros nibbles (`34c4`/`44c3` en pares), `50` (`4507`), `88` (`0881`), `c8`,
  `bc`, `44`, `cc`; las de dos palabras de operación (`0803 0c0a 9281`, 705×; `0803 0809 5280`); los formatos con
  inmediatos desalineados (`0807 3cca 1d62 …`).
* Fuentes > 7 (`5549`, `5546`): ¿registros 8–15 o modificador?

## Pokes de microcódigo en vivo por MIDI (2026-10-15) — `re/dsplive.py`, `engine.c -DDSPLIVE`
* **Escritura en el slave**: el OS del slave escribe la memoria de programa con `str` de 32 bits comunes
  (`s_boot` `0x011100DE`, llamado desde el swap `0x01105F94`): DSP n en `0x41480000 + n·0x100000` (tabla
  `0x0111C20C−0x7C`) + dirección de la ranura en bytes (DCO2 `0xB000` → DSP2 `0x4168B000`, DSP3 `0x4178B000`).
  Registro de control/tarea `0x41104318 + n·0x1000` (`[ctl] = máscara | 2<<28`), puerto de medias palabras
  `0x41104700 + n·0x1000` (el OS lo usa para trozos de 2 bytes: escribimos palabras enteras). Bloques ≥ 0x40 B por DMA.
* **Transporte sin tocar la región 0 ni el OS**: pitch bend → id de voz 0x465 (14 bits de datos), luego CC1 →
  0x467 (comando: `seq<<6 | cmd<<4 | ext`; cmd 0 dirección, 1 dato y avanza, 2 control: ext 0 restaurar). El
  engine actúa cuando cambia 0x467 y lee 0x465 con `+0x00 get`. Los dos slaves reciben lo mismo → cada prueba
  sigue replicada. Bend y modulación quedan neutralizados en el modelo del engine en este build. El maestro manda
  rangos de ids "sucios" (kernel `0x01011B7E` guarda min/max de id) y el slave notifica cada id del rango
  (`UpdateSynthParam`): por eso un comando = valores en ids distintos + cambio del id de comando, y 15 ms entre comandos.
  El kernel recorta cada id a su min/max (`0x01010C24`): por eso no usamos ids de 32 bits arbitrarios.
* Engine: sombra de la ranura DCO2 de DSP2/DSP3 en RAM (8 KiB c/u), escribe la palabra de 32 bits completa (mitad
  alta = instrucción de 16 bits par, primero en ejecución). `engine/test_live.py` lo verifica en emu1.
* Flujo: `dsplive.py install l00.batch` (una vez) → seleccionar PLG3 → `dsplive.py poke d2a=x.txt d3b=y.txt` (sólo
  las palabras que cambian, cola primero) → `dsplive.py recall NAME` (graba + `report` contra b11) → `restore`.
  Unos 30 palabras = 0.5 s; sin reinstalar ni tocar el panel.
* **Verificado en el equipo**: L00 (4 caminos directos) = 1.00 ±2 %; poke de 3 palabras d2a = cero → sólo las notas
  65/72 a 0.000 (ambos slaves); restore → 1.00; las 4 pruebas de b14 por poke (L03) = b14 instalado dentro de 1 %
  (incluida la diferencia entre slaves del op `54`: 1.09 / 1.23).

### ALU `0902 w` decodificada en vivo (2026-10-15) — `re/dspexp/audio/live/`, lotes L10–L25, Q*, R*, V0, O0/O1, W0
Método: `dsplive.py poke` + sondas de constantes (`live/probe.py`, `probe3.py`): r3 = ruido, constantes exactas
(`0903 d050 imm16`) en D (destino viejo), X e Y, la operación, +1.0 (conserva el signo) y `r0 *= r3`
(`0902 0403`) → ganancia − 1 = resultado exacto (±1–2 %). Con ruido solo (r0 = x, r1 = x/2, r2 = x/4, r3 = x/8)
varias lecturas eran ambiguas (las de la tabla "Confirmado" de arriba con `04c1`, `0081`, `0541` usaban
registros sin inicializar).

**Palabra de operación** (la cabecera `0902` = clase 0x48 + largo 2):
| bits | campo |
|---|---|
| 15–12 | destino D (r0–r15) |
| 11–9 | operación (8) |
| 8–6 | X (r0–r7) |
| 5–0 | Y (con 5–4 = 00: r0–r15; otros valores: registros que leímos sin inicializar → probablemente r16–r63) |

| op (bits 11–9) | B = bits 11–8 | resultado | sondas (D, X, Y) |
|---|---|---|---|
| 0 | 0/1 | **selección** (devuelve exactamente uno de D/X/Y; regla abierta, ver abajo) | |
| 1 | 2/3 | D + X + Y | (.5,.25,.125) → .87; (.25,.5,−.125) → .62 |
| 2 | 4/5 | X · Y | .03, −.06 |
| 3 | 6/7 | D + X · Y | .54, .19 |
| 4 | 8/9 | Y + X · D | .25, 0 |
| 5 | a/b | Y − X · D | 0, −.25 |
| 6 | c/d | X + Y | .375, .375 |
| 7 | e/f | Y − X | −.125, −.625 |
Con B impar X es r4–r7: verificado cargando r5 (V0: .88, .53, .25, .375 = lo predicho). Todo coincide con las
mediciones con ruido (p. ej. `0c81` = r2 + r1, `0cc2` = r3 + r2, `1c01` = r1 = r0 + r1, `3c01` = r3 = r0 + r1).
* Op 0, datos: `0001`/`0002` (X = r0 = D = .5) → .5; `0041` (X = Y = r1 = .25) → .25; `0081` (X = r2 = .125,
  Y = r1 = .25) → .125; `0042` (X = r1, Y = r2) → Y en 10 de 10 combinaciones (D, X ∈ {0, ±.25, ±.5}, Y = ±.125).
  Candidatos: mín/máx por magnitud o comparación con D; falta una sonda con Y > X para X = r1.
* **Modo de Y (bits 5–4)**: 00 = registro; 01 = inmediato de 16 bits (`0903 d050 imm16` = op 0 sub 1 "Y" → rd = imm;
  `0903 d41s` = X·imm16); 11 = inmediato de 32 bits (`0904 1430 imm32` = op 2, r1 = r0·imm). Sin inmediato (`0902`)
  Y vale 0 (de ahí las "sumas que no suman" de L12–L19).
* **Op 0**: X es un sub-código, no un registro (cargar r2/r4/r5 no cambia nada): 0 → D, 1 → Y (copia), 3 → D,
  2 → ≈ D·0.27, 5/6/7 → ≈ D + Y, 4 → ≈ D + Y + 0.92. Poco usada en el corpus (8× `0040`).
* **Clase 0x40 (`08xx`)**: NO son dos palabras `0902`. `0803 0c01 0000` (r0=.5, r1=.25, r2=.125) → r0 = 3·r0
  (5 conjuntos de valores). Barrido de bits (F0–F7): en w2 los bits 3–0 y 15–12 son dos registros-operando más
  (1 → r1, 2 → r2: 1.25, 1.125), bits 5–4 anulan un operando (→ 1.0), bits 11–9 cambian la operación
  (.89/.64/.66), bits 8–6 sin efecto aquí; w1 se parece a la palabra ALU (D en 15–12: 1c01/2c01/4c01 dejan r0;
  X en 8–6: 0c41 → 1.25, 0c81 → 1.125) pero su Y casi no influye (sólo `0c09` → 1.12). Hipótesis: operación de
  3–4 entradas (suma/MAC) con el resto de operandos en w2. Sigue.
* r2 y r3 parecen valer 1.0 al empezar la ranura cuando nadie los escribe (explica `04c1` = r1 y `0881` = 1.5).

### Peek exacto en los DSP del maestro (2026-10-15) — `re/dsppeek.py`, `re/dspmaster.py`, `re/dspprobe.py`, `re/dspfit.py`
Ver `ARCHITECTURE.md` (raíz) para qué corre dónde. La región 0 (`model.c -DPEEK`, se instala con `dsplive.py
install`) recibe comandos por CC1 (`seq<<6 | 3<<4 | cmd`, el dato de 14 bits en el pitch bend, que la región 0
relee del id de voz 0x465: el OS escribe el bend ahí y **no** llama al método bend del asignador) y contesta con
SysEx `F0 7D 02/03` desde el manejador de CC (funciona, no cuelga). Lectura de 8 palabras ≈ 40 ms.
* `dspmaster.py`: escribe un programa de prueba en la ranura `0xEFDC` del **DSP4 del maestro** (efecto global; se
  guarda y se restaura), espera y lee el banco 0 (vista RAW `0x41804000`). **El programa corre**: los `0c03` que
  guardan registros aparecen exactos en DT0.
* Lecturas de DT0 con el DSP en marcha: ~1 palabra de cada 8 sale mal (0 o `4ea5f810`): se lee 5 veces y se toma
  la mayoría por palabra. Zona usada: `[0780..0787]` (libre en DT40; `0x400` la tocan otros programas).
* `7809` con 4 registros distintos guardó sólo el último (en las pruebas por audio siempre era el mismo registro).
  Para varios registros: un `0c03` por registro.
* Sólo r0–r7 se pueden guardar (XfeY: X 8–15 = guardar r0–r7) y `0903` con D ≥ 8 no carga r8+.
* `dspprobe.py`: r0..r7 = constantes, instrucción, r0..r7 → DT0; `dspfit.py`: N juegos aleatorios y búsqueda de
  la fórmula exacta más chica (hasta 3 monomios). Con esto la tabla de `0902` queda **exacta** (error 0):
  op0 sub1 = Y, 1 = D+X+Y, 2 = X·Y, 3 = D+X·Y, 4 = Y+D·X, 5 = Y−D·X, 6 = X+Y, 7 = Y−X.
* **Y bit 3 = negado**: Y = 8+n → −rn (`3c08` → 0, `3c0f` → X + r7·(−1)). X: 3 bits (r0–r7). D: 4 bits; D ≥ 8
  escribe r(D−8) con otro resultado (`8c01` → r0 = 1.625 en vez de .75): sin aclarar.
* Op 0, sub-códigos (exacto): 0/3 = nada, 1 = Y, 4 = D+Y+h4, 5 = D+Y+h5, 6/7 = D+Y, 2 = cambia r1, r3, r4 a la
  vez. h4 = 0.0313, h5 = 320 en el maestro (≈ 0.92 en el slave): valores ocultos (¿otro banco de registros?).
* **Clase 0x40 (`0803 w1 w2`)**, exacto con `dspfit.py --all` (plantilla `0803 3{op1}40 2{op2}04`: D=r3, X=r1
  (w1 bits 8–6), Y=r0 (w1 bits 3–0), Z=r2 (w2 bits 15–12), W=r4 (w2 bits 3–0)):
  | op1 (w1 bits 11–9) | resultado |
  |---|---|
  | 0, 1 | no escribe |
  | 2 / 3 | W ± Y·X |
  | 4 / 5 | ±Y·X + c·Z |
  | 6 / 7 | W ± Y·X + c·Z (con op2 = 0: W ± X + Z, sin productos) |
  op2 (w2 bits 11–9) elige c: 1 → Y; 0 → sin multiplicador; 2–7 → constante que sale de los bits de la propia
  instrucción (0.250977 = RAW `2404…` = w2) → es un **inmediato**; en `0803` (sin inmediatos) lee basura. Encaja
  con `0807 3cca 1cb6 imm32 imm32` (Horner) del corpus. Bits 15 de Z y 3 de W = negado (barrido F0–F7).

### Formato de instrucción (2026-10-15, medido en el maestro) — `re/dspslots.py`, `re/dspclass.py`
**Instrucción** = cabecera `clase<<5 | largo` + palabras. La **clase (11 bits) es una plantilla**: fija qué
es cada posición (no hay marcas de tipo en las palabras: un `0000` en una posición de transferencia ES una
transferencia; el `fe` de `XfeY` son bits de modo). No parece un campo de bits simple: se cataloga por medición
(`dspslots.py`: sondas `3c01`/`5c01` en cada posición, resto `0000`). Orden general observado:
`[ops A y w1 de MAC] [transferencias] [ranuras: w2 de MAC, luego referencias] [inmediatos]`.

| clase | ejemplo | plantilla |
|---|---|---|
| 0x048 | `0902 w` / `0903 w imm16` / `0904 w imm32` | op A (+ inmediato) |
| 0x0A8 | `1503 2987 7505` | op A, op A (en paralelo; un solo inmediato en total) |
| 0x040 | `0803 w1 w2` (+ inmediatos en 0806/0807) | MAC |
| 0x060 | `0c03 T r` | transferencia, ranura |
| 0x0F0 / 0x1F0 / 0x3C0 / 0x720 / 0x722 | `1e05`, `3e07`, `7809`, `e40b`, `e44d` | 2 / 3 / 4 / 5 / 6 transferencias + sus ranuras |
| 0x0A0 | `1405 w1 T w2 r` | MAC con una transferencia (w2 = ranura 0) |
| 0x088 | `1107 w1 A w2 imm…` | MAC + op A |
| 0x158 | `2b09 w1 T T w2 r r imm` | MAC + 2 transferencias |

* **Transferencia** `XfeY` (posición de transferencia): X 0–7 carga rX ← [ranura Y/2], X 8–15 guarda
  [ranura Y/2] ← r(X−8). Las ranuras se cuentan desde la primera palabra después de las transferencias, **incluidas
  las w2 de MAC** (por eso hay `1fe2` = ranura 1 cuando la ranura 0 es la w2). El byte del medio (`fe`, `9b`, `a9`,
  `e0`…) es modo (sin decodificar; `X9bY` no cambia r0–r7 ni los valores ocultos de la op 0).
* **Op A** (16 bits): D[15:12] op[11:9] X[8:6] modoY[5:4] Y[3:0] — tabla exacta más arriba. Y: bit 3 = negado.
* **Modo de un operando** (Y de op A, Y de w1, W de w2, c de w2): 00 registro, 01 inmediato de 16 bits, **10 de 24
  bits**, 11 de 32 bits. Los inmediatos salen de un **flujo de bits** al final de la instrucción, en orden, sin
  alinear (24 bits = 1,5 palabras; `0903 3060 2800` lee 8 bits de la instrucción siguiente). Ej.: `1107 7522 6504
  0025 800d 0002 2222` → MAC r7 = W + Y·X con Y = `800d00` = −1/5040 y W = `022222` = 1/120 (serie del seno).
  Dos ops A con inmediato en la misma instrucción la vuelven ilegal (el programa deja de guardar).
* **MAC** (w1 + w2): w1 = D[15:12] op1[11:9] X[8:6] modoY[5:4] Y[3:0]; w2 = Z[15:12] (bit 15 negado) op2[11:9]
  C[8:6] modoW[5:4] W[3:0] (bit 3 negado). Resultado: op1 bit0 = signo de P1 = Y·X, bit1 = + W, bit2 = + P2 = c·Z;
  op1 0/1 no escribe. op2: bits 11–10 = modo de c (00 → c = rC, 10 → inmediato de 24 bits, 11 → de 32), bit 9 = 1
  con c registro; con op2 = 000 y op1 = 6/7 no hay productos (W ± X + Z). Ejemplos exactos: `0803 0c0a 9281` =
  interpolación lineal r1 + r2·(r0 − r1); `2b09 2c8f … 5983 … a2aa aa00` = r3 + r7·r2 − (1/6)·r5 (serie del seno);
  `0807 3cca 1cb6 002e3bc7 01a01a01` = 2/315 + r2·r3 + (2/2835)·r1 (Horner de la exponencial).
* **Op 0 sub 2** (`0902 4086`): rD = rD·rD+1; rD+1 = 2 − rD+1·rY; rY = rD+1·rY = paso de Newton del recíproco.
* Peligro de las sondas: un programa cortado con una señal dejaba el efecto del maestro reemplazado. Ahora
  `dspmaster.Session` guarda el original en `work/dspmaster_saved.json` y `dspmaster.py --restore` lo repone; el
  programa original de `0xEFDC` también está en `dumps/os_full.bin` `0x163868` (183 palabras).

### Unidades, plantillas y predicación (2026-10-15, maestro) — `re/dspslots.py`, `re/dsptemplates.py`, `re/dspisa.py`, `re/dspnlfit.py`
* **Plantillas de las 118 clases del corpus** medidas (`dspslots.py all` + resto; `dsptemplates.py` →
  `work/dspclass/templates.txt`; `dspslots.py ab` → `work/dspclass/ab.txt` separa unidad A de B). Cubren el 99.9 %
  de las instrucciones del corpus. Letras: A (op ALU unidad A), B (op unidad B, mismo formato de palabra), M/N
  (MAC w1/w2), C (palabra de condición), T (transferencia), R (referencia), I (inmediatos). Ej.: `0x0050` (`0a02`) = B;
  `0x00b0` = A B; `0x0188` = A B T R; `0x0160` = A A B; `0x06c8` = B B T T T R R.
* **Op A con D ≥ 8** (`0902 b{op}42`, exacto): r(D−8) = X + Y + r[op] (el campo op es el índice del tercer sumando).
* **Unidad B** (`0a02 w`, exacto en 7 sondas): `3c42` = |X| + Y, `3e42` = Y − |X|, `b042` = **min(X, Y)**, `b442` =
  **max(X, Y)**, `b842` = |X|, `ba42` = |Y|, `be42` = Y. Las demás dan valores con el patrón de bits saturado
  (`7ffxxxxx`, ≈ 511.x en RAW): comparaciones/conversiones a entero sin decodificar (`work/dspclass/bunit_raw.jsonl`).
* **Unidad C = predicación**: solas no cambian registros; en `2d06 1050 1052 Cw 3000 0000` (dos cargas de r1: 1.0 y 0)
  `1b10` deja siempre la primera, `1a10` siempre la segunda, y `0e12`/`0e10`/`1312`/`0b12` eligen según una
  condición que depende de la op B anterior (con min/max/|X|+Y cambian los patrones) pero no es el signo de su
  resultado: probablemente varios bits de bandera; sin resolver (pocas sondas: hace falta n ≥ 16).
* `dspisa.py dis` ya desensambla los programas de Roland con estas plantillas (p. ej. el SAW del JX-3P: pitch
  [2201] − 4.75, × ln2, Horner de la exponencial, Newton del recíproco, π/2 y serie del seno).
* `dspclass.py ins "a;b"` acepta secuencias; `PAD=n` mete no-ops; `dspnlfit.py` ajusta con abs/min/max/signo.
* Sigue abierto: valores ocultos de la op 0 sub 4/5; ops B "511"; bits de condición de C; modos de transferencia
  distintos de `fe` (`9b`, `a9`, `e0`, `e1`, `db`…: no cambian r0–r7, probablemente registros de dirección /
  líneas de retardo); segunda palabra de C en el arreglo de ranuras.

### Predicación por tiempo y memoria entre ejecuciones (2026-10-15, abierto) — `re/dspcount.py`
* La condición de las palabras C **depende del tiempo**, no sólo de los datos: la misma sonda con los mismos valores
  da 1 en 8 de 10 repeticiones y 0 en 2 (`0a02 b042; 2d06 1050 1052 0e12 3000 0000`). Hipótesis: contador de
  muestras / sub-muestras o banderas de otra tarea del DSP4.
* **Hipótesis (no confirmada)**: una carga en la instrucción siguiente a una escritura en memoria devolvería el dato
  recién escrito. Salió de una sola prueba; el programa "bueno" después tampoco acumuló, así que puede ser otra cosa.
* Acumular en memoria entre ejecuciones (`r2 <- [07a1]; r2 += 2^-16; [07a1] <- r2`) **no funcionó de forma
  reproducible**: una vez la dirección `07a1` crecía (~4 000 incrementos/s), pero el mismo programa después quedó fijo.
  **Explicado después**: la memoria es un anillo (lo guardado en S se lee en S + 1 la muestra siguiente) y las
  cargas tienen una instrucción de latencia (sección "SAW propio").
  Probables causas: otras tareas del DSP4 (la ranura `0xEFDC` parece una tarea de control) usan los mismos
  registros/memoria entre ejecuciones, o la memoria se direcciona con un puntero que avanza (líneas de retardo).
  Para estudiarlo hace falta una ranura/DSP propio sin otras tareas (p. ej. el engine en los slaves con `dsplive`,
  observando por audio) o mapear qué zonas de DT0 usan las demás tareas.

### Contexto limpio: DSP3 del maestro; condiciones C = bits de contexto (2026-10-15) — `re/dspcond.py`
* El DSP3 del maestro está ocioso (programa principal `0001 0001 0b03 0080 7fee 0001`, fin distinto de las
  ranuras). `dspmaster.Session` usa ahí su programa principal (`DSPCTX=dsp3`, por defecto; `efdc` = la tarea del
  DSP4). ~~Corre una vez por bloque de 1024 muestras (46.9/s)~~ **Corrección**: corre a **96 kHz** como todo
  programa; el 46.9/s era 96 000/2048 ≈ un contador que sólo se reencontraba con su valor una vez por vuelta del
  anillo (ver "SAW propio" abajo).
* En el DSP3 la predicación es estable. Palabra C: byte bajo = a qué op (`x10`/`x11` op 1, `x12` op 2), byte alto =
  condición (bit 0 niega, bits 4–1 = una de 16). Las 16 condiciones **no son banderas**: ninguna op A/B las cambia
  y no varían en el tiempo; difieren por contexto (DSP3 `0111111011101110`, DSP4 `0xEFDC` `0111101111100111`).
  Lo de "varía en el tiempo" del DSP4 era ruido de lectura + tareas compartidas.

## SAW propio, primer microcódigo nuestro (2026-10-15) — `re/dspasm.py`, `re/dspown.py`, `re/dspexp/own/`
Documento técnico completo: `CUSTOM_OSC.md`.
* `re/dspasm.py`: ensamblador mínimo del subconjunto establecido (ops A, min/max de B, cargas/guardados, inmediatos
  16/32) que valida cada instrucción con `dspisa`; `saw dco1|dco2` genera el oscilador. `dspown.py`: carga en vivo
  (sólo sirve si el programa registrado tiene el mismo tamaño) y mide notas por audio (pico del espectro).
* Algoritmo: p de `[2201]`/`[220e]` (DCO1) o `[2207]`/`[2214]` (DCO2), f = 440·2^(p − 4.75); 2^x = (e^(x·ln2/16))^16
  con Taylor de grado 4; incremento = 2f/96000; fase en [−1, 1) guardada en el anillo; vuelta sin saltos:
  q = max(fase − 1, 0), 4 × q = min(256·q, 1) (queda 0 ó 1 exacto), fase −= 2q; salida = fase en las dos
  salidas de la voz. 187 palabras por ranura.
* Lo que hubo que descubrir en el camino (todo medido en el DSP3 del maestro, peek exacto):
  1. **El DSP corre a 96 kHz** (no 48): ~96 000 ejecuciones/s.
  2. **La memoria de datos es un anillo**: una referencia R apunta a (R + puntero) y el puntero avanza una palabra por
     muestra; lo guardado en R se lee en R + 1 a la muestra siguiente. Estado persistente: cargar de S + 1, guardar
     en S. (Lo que antes parecía "memoria compartida/no reproducible" y el rastro que llenaba todo el banco era esto.)
  3. **Latencia de carga de una instrucción**: una op justo después de la carga ve el registro viejo.
  4. El poke en vivo de un programa más largo que el registrado no corre entero: hay que instalarlo.
* Resultado en el equipo (C-SAW con `MC_TEST=dco1_saw=…,dco2_saw=…`, patch INIT, DCO-1 SAW, Mod Sens bajo):
  notas 36–96 afinadas dentro de ±1.2 cents, armónicos 1 : 0.50 : 0.33 : 0.25 : 0.20 (diente de sierra ideal).
  Sin anti-aliasing (es un SAW ingenuo) y PULSE (mismo programa con bandera) no está implementado.

### Plugin de laboratorio y condiciones de las ranuras DCO (2026-10-15)
* **Laboratorio** (sin reinstalar): DSPTEST + DSPLIVE + PEEK, DCO2 de DSP2/DSP3 con un programa registrado de 2048
  palabras (`re/dspexp/own/lab_pad.txt`), DCO1 en silencio. `dspown.py load prog.txt` carga cualquier programa
  ≤ 2048 palabras en segundos; probado con el SAW y el FOLD (`CUSTOM_OSC.md` §7.1). Al instalarlo hay que poner
  `work/dsplive/state.json` en el programa de laboratorio (palabras instaladas = `lab_pad`).
* **Bits de condición en la ranura DCO2 de los slaves** (por audio: ruido × suma de pesos 1/2..1/16 de 4
  condiciones por camino, dos instalaciones rotando condiciones entre DSP y voz, `dspasm.cond_probe`): verdaderas
  2 3 5 6 9 10 13 14 (`0011011001100110`), iguales en DSP2/DSP3 y voz A/B. Verificado antes el método en el DSP3 del
  maestro (0.4375 / 0.875 exactos).
* Plantillas: corregida la regla de ranuras (todo lo que sigue a las transferencias es arreglo de ranuras; las w2 de
  MAC primero; referencias = las ranuras que apunta una transferencia `fe`; el resto = inmediatos; las
  transferencias con otro modo no usan ranura). El SAW de Roland pasa de 225 a 239 instrucciones decodificadas; quedan
  C (32 + 6 con modo de transferencia), B (24) y modos de transferencia (20).

### SAW de Roland al 90 % (2026-10-15) — `re/dspcsweep.py`, `re/dspcondsnap.py`, `re/dspisa.py`
* Palabras C decodificadas (barridos completos de 256 valores en 3 clases): byte alto = condición + negación; bits
  7–4 = unidad que se saltea si la condición es verdadera, bits 3–0 = si es falsa; unidades = ops y luego
  transferencias, numeradas desde 1. `dspisa` ya aplica la predicación con los bits de contexto (DCO por defecto).
* Unidad B: con B impar X va de r4 a r7 (las clamp 0..1 de Roland); unidad de funciones especiales (D ≥ 8, nibble
  f): semilla de recíproco (`cf06`, luego Goldschmidt), separación exponencial (`8f19`: e^⌊x⌋ y frac), signo,
  máscara de negativo, techo.
* Resultado: el SAW de Roland pasa de 225 a **291 de 325 instrucciones decodificadas**. Faltan las transferencias
  `X9bY` (26 instrucciones) y las formas de inicio `ef75`/`ef74`. Las `X9bY` no cambian r0–r7, ni los bits de
  condición, ni lo que leen las cargas `fe` después, y `Xa9Y` no lee lo que escribieron: hablan con algo que el
  programa no ve (memoria de retardo, bus de audio). Próximo método: observarlas por audio en los slaves (laboratorio)
  o buscar en el código ARM (`s_boot`/`m1_tcm`) qué registros de hardware se configuran para esos puertos.

### Puertos X9bY y entradas de sincronismo (2026-10-15, por audio en el laboratorio)
* `X9bY` escribiendo ruido a los puertos 0–7 no llega a la salida; `Xa9Y` no carga nada (lo "leído" era el registro
  que dejó la ranura DCO1: **los registros se comparten entre ranuras del mismo DSP**). `X9bY` no redirige el
  resultado de la op B de la misma instrucción ni carga los valores ocultos h4/h5 (que ahora valen 0 en el DSP3).
  Sigue sin decodificar.
* Entradas del DCO2 según Cross Mod: `[2209]` = 1 con SYNC/METAL, `[220a]` = 1 con METAL, `[220b]` = tren de
  impulsos (un pulso por ciclo) con SYNC/METAL, `[220c]` = 0. Las escribe el programa principal (MAIN2/3), no las
  `X9bY`. Detalle en `CUSTOM_OSC.md` §2.

## Próximos pasos del ISA (para seguir)
Referencia limpia de lo establecido: `DSP_ISA.md` (resumen) y `DSP_ISA_SPEC.md` (especificación técnica para
ensamblador/compilador: formatos de bits, 118 plantillas, inmediatos, semántica de ejecución medida — ops de una
instrucción en paralelo, guardado del valor viejo, sin latencia entre instrucciones). Pendiente (2026-10-15):
1. Qué fija los 16 bits de condición de contexto (lado ARM). Ops de la unidad B con resultado `7ffxxxxx`
   (¿conversiones a entero / floor?) en el DSP3.
2. Memoria entre ejecuciones (acumular, retardo de una muestra) en un contexto sin otras tareas.
3. Ops de la unidad B con resultado saturado `7ffxxxxx`, B impares, modos de transferencia ≠ `fe`, segunda palabra
   de C, valores ocultos de la op 0 sub 4/5.
4. Con eso: primer oscilador propio (fase += incremento, vuelta con min/max/condición) en la ranura DCO2.
5. Probar en el equipo el C-SAW normal con la corrección de descriptores (cambios de onda y de efecto).
