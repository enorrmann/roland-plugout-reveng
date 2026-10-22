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
* Slave (s_boot, UpdateSynthPlugout 0x01104226): D=init(); D[1](); D[5]->vt[0](../examples/from_scratch/obj); D[0]=slot.
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

## Microcódigo DSP (2026-10-14, `re/dsp_progs.py`; detalle en docs/CUSTOM_PLUGOUTS.md "DSP microcode in the engine")
* El sonido lo hacen programas DSP que vienen en la región 1 y el slave los baja a sus 6 DSPs.
  `vt+8` (0x60A0FA98) los registra: OS `+0x28+4n` ranura, `+0x40+4n` programa, `+0x58+4n` datos
  (n = DSP 0–5; descriptor `{dir DSP, bytes, bytes, src}`; los tres = `0x01105808`, agregar a lista).
* DSP2/3 (osciladores): ranura `0x6000` = DCO1, `0xB000` = DCO2; programa elegido por 0x6D4/0x6D9.
  SAW y PULSE = mismo programa (0x1008 B) + flag `+0xB0` 0.0/1.0; **SQR = programa propio (0xF88 B,
  DCO1 src 0x60A1D5F4, DCO2 0x60A1E57C)**; NOISE = 0x180 B. DSP4 ranura `0xB000` = efecto (0x748, 6 programas).
* Cambio en vivo: OS `+0x70(dsp, &prog, &datos, 0x60A44E18, ranura, 2, 0, 0)` con nivel en rampa a 0.
* Palabras de 16 bits LE; DCO1 vs DCO2 = mismo código con operandos +0x200 (banco de memoria). ISA desconocido.
* Para el PLUG-OUT propio: copiar los blobs de Roland (principal de cada DSP + SQR) y registrarlos igual.
* Formato (detalle en docs/CUSTOM_PLUGOUTS.md "Microcode format"): NO son tablas de ondas, es código
  (383 valores distintos en 1988 palabras, sin suavidad; va a memoria de programa 0x41480000+n·0x100000,
  copiado tal cual). Palabras de 16 bits LE, instrucción + palabras operando (direcciones 0x04nn/0x06nn).
  Corpus: `re/dsp_corpus.py` -> 199 programas únicos en todas las imágenes. Mapa de opcodes: pendiente.
* Espacios de operandos (docs/CUSTOM_PLUGOUTS.md "Memory map of a slave DSP"): `C0nn` = parámetro `+0xA0` id nn,
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
| Microcódigo DSP (6 principales, SAW/SQR/NOISE, 6 efectos; 48 KB) | copiado del JX-3P al compilar, **estático** (`engine/programs.py`, offsets + SHA-256, sin ejecutar nada) | **pendiente**: requiere el ISA del DSP (assembler propio); el programa vacío y los bloques de datos ya son nuestros. **2026-10-22:** `C-V4` ya no usa microcódigo de Roland (`FROM_SCRATCH.md`) |
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
Ver `docs/ARCHITECTURE.md` (raíz) para qué corre dónde. La región 0 (`model.c -DPEEK`, se instala con `dsplive.py
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
* Hipótesis del análisis estático (agente, `DSP_ARM_SIDE.md`): puerto 1/2/3 de `X9bY` → grupos de condición
  4–7 / 8–11 / 12–15, leídos por una C dos instrucciones después (el 90 % de las `X9bY` del corpus tienen una C a
  distancia 2). **No se confirma**: en el DSP3 del maestro (`re/dspx9b.py`; valores ±0.5, 0, ±2, ±1/16, patrones
  crudos 1/2/4/8/F, 0x000F0000, 0x7fffffff, 0xF0000000, distancia 1–3, dentro de una instrucción con op B, y
  escribiendo al final de una muestra y leyendo al principio de la siguiente) los grupos siguen en `1110`; en los
  slaves por audio (`re/dspexp/own/x9b_pos_neg.txt`, `x9b_zero_big.txt` contra `x9b_none.txt`) la ganancia queda en
  0.875/0.876/0.874. La correlación parece patrón del compilador de Roland, no causa.
* Del mismo análisis (sin verificar en el equipo): bits 15–13 de una referencia = banco (0 DT0, 1 DT1, 2 DT2,
  3 IOMD, 4 MXM, 5 DPC, 6 ENV); el audio entre DSP va por MXM (zona Z → zonas M/O del par siguiente); `Xa9Y` sólo
  en programas de efecto tras `{8+d}f6c`, seguido de cargas del banco 2 `4001…` e interpolación lineal → toma de
  línea de retardo (WMEM). Próximos experimentos: `Xa9Y` como toma de retardo y referencias IOMD/MXM en el DSP3.

## Wavetables desde WAV y presupuesto de tiempo del DSP (2026-10-16) — `re/wav2dco.py`, `CUSTOM_OSC.md`
* **Conversor**: `re/wav2dco.py ciclo.wav -n N` (PCM o float, `--cycle 2048 --frame K` para tablas de varios
  cuadros) → programa para DCO1/DCO2; la tabla va como inmediatos (no hay lecturas indexadas). `re/wtcheck.py`
  toca una escala y compara los armónicos con la tabla (`--harmonics 5`: el filtro del patch apaga los altos).
* **El límite es de tiempo, no de memoria**: con la primera versión (5 instrucciones por punto) fallaba la **voz B**
  (segunda mitad del programa → notas alternas de una escala; el usuario oía "un saw en notas fijas"). Medido en el
  laboratorio con relleno entre las voces: la ranura DCO2 corre 733 instrucciones y no 753 (DCO1 = 6, main = 189)
  → DSP2/3 ≈ 930–950 instrucciones simples por muestra.
* **Plugout C-WAVE** (`NAME=C-WAVE WAVETABLE=1 OWN_PROGS=… engine/build.sh`, `engine.c -DWAVETABLE`): 6 programas
  propios por DCO, uno por posición de forma de onda; DCO2 = 6 AKWF, DCO1 = 6 cuadros de Echo Sound Works de 32 a
  120 puntos. Con la versión de 2 instrucciones por punto todo sonó bien (verificado por audio y por el usuario).
  Ahora la ranura 3 tiene de nuevo el laboratorio.
* **Decodificado (DSP3 del maestro, exacto)**: las clases 0x0B0 (A, B) y 0x160 (A, A, B) leen todos los operandos
  antes de escribir; **varios ops A/B de una instrucción comparten el inmediato** (cada uno lo lee desde el
  principio del flujo de bits). Corregido `re/dspisa.py`; nuevo `dspasm.aab()`.
* **Una instrucción por punto**: `r1 += r7 | r3 += r2·s | r2 = max(r1, r5)` en cadena sobre relu(w − i), bloques
  de 64 puntos. En vivo en el laboratorio: 32 y 144 puntos (2022 palabras) → 6/6 notas bien.
* **Cuidado**: tras cargar un programa que se pasa del presupuesto el equipo puede quedar inestable (silencio,
  niveles erráticos, ruido tipo LFO, el SAW de referencia roto; el reset MIDI no alcanza) hasta **apagarlo y
  prenderlo**. Pasó dos veces (relleno con `0001` y la prueba de 705 instrucciones con relleno). Antes de medir:
  comprobar el SAW de referencia (`dspown.py load dspexp/own/saw_dco2.txt` → armónicos 1/n).
* **Costo de A/A/B medido sin audio** (`re/dspbudget.py`, DSP3 del maestro: K rellenos antes de un contador en el
  anillo; el contador sube 1,46/s mientras el programa entra en la muestra): con relleno simple corre K = 900 y se
  detiene en 920–950; con relleno A/A/B lo mismo (3654 medias palabras, no es memoria) → **una A/A/B cuesta un
  ciclo** y el presupuesto es ≈ 925–945 instrucciones por muestra también en el maestro.
* **Máximo**: en el borde, una DCO sola 310 puntos (733 instrucciones) y las dos 136 cada una; **recomendado con
  margen: una DCO sola ≤ 290 puntos (693 instrucciones, 3830 palabras), las dos ≤ 128 cada una**.
* **C-WAVE v2** (`re/dspexp/own/wt2/`, DCO1 64/96/128/160/208/224 puntos, DCO2 AKWF 32): instalado tras apagar y
  prender el equipo; las 6 posiciones de DCO1 dan 6/6 notas (error ≤ 0,021), hasta 224 puntos con el DSP en 887
  instrucciones. `wtcheck.py` normaliza ahora sobre los armónicos comparados (los "WRONG" de Ahhh en vivo eran del
  script, no del oscilador).
* **Laboratorio de 4096 palabras instalado** (`lab_pad4k.txt`: silencio y fin en 7 instrucciones, ceros después):
  en vivo una tabla de **290 puntos** (3830 palabras, 693 instrucciones) suena igual a la tabla (armónicos 1–6
  dentro del 3 %; arriba de ~500 Hz corta el filtro del patch). La carga tarda ~2 min.
* (Hecho) un `lab_pad` que no se pase del presupuesto (programa corto + relleno **después** del final) y
  que reserve 4096 palabras (el máximo de la sombra de DSPLIVE) para probar tablas más grandes en vivo.

## Posibles próximos pasos: emulador (2026-10-17, decidido dejarlo para otra sesión)

> 2026-10-22: plan actualizado en `EMULATOR.md`. Hecho en parte, sin Unicorn: `examples/from_scratch/emu/` emula los seis DSP de un slave (y dos slaves)
> con nuestros programas, en Python (`VOICE_ALLOCATION.md` §6).
> 2026-10-22 (más tarde): núcleo del DSP en C, código nativo e hilos: un slave del JX-3P en tiempo real, bit a bit igual
> a `chain.py`. Estado y pendientes en `EMULATOR.md` §5.
Idea del usuario: correr los binarios ARM originales en un emulador de CPU y escribir en C sólo el DSP.
* **Ya resuelto**: núcleos Cortex-M Thumb-2 → Unicorn (ya usado: `re/emu0.py` región 0, `re/emu1.py` región 1,
  `tools/engine_emu.py` motor base, con los servicios del OS atrapados); binarios descifrados (OS maestro núcleos
  0/1, `s_boot`/`s_data`, regiones de todos los plugouts, STM32); ISA del DSP documentado (`DSP_HARDWARE.md`).
* **Lo difícil son los periféricos del BMC** (`docs/BASE_FIRMWARE.md` §8.8): bloques de control DSP `0x411xxxxx` (media,
  es la interfaz con el emulador de DSP), NVIC/SysTick e interrupciones (Unicorn no los tiene), DMA, mailbox entre
  núcleos `0x40054000`, **enlace maestro↔slaves por USB host interno** (EHCI/OHCI "BMCBulk": interceptar el driver
  en vez de emularlo), `0x40026000`/`0x40080000` sin identificar, SD, QSPI, UART al panel, USB al PC, códec, IOMD /
  bus de onda. El cargador de arranque está cifrado: arrancar el OS en el estado que deja el cargador.
* **Niveles**: (1) plugout = región 1 (+ región 0) en Unicorn + emulador de DSP en C → audio del plugout con el
  código y microcódigo reales de Roland, validable contra la captura USB del equipo (`dspaudio.py`); (2) slave
  completo (`s_boot`/`s_data` + periféricos, enlace atrapado a nivel de driver); (3) SYSTEM-8 entero (mucho
  trabajo, poco aporte al sonido). Estrategia híbrida: código de Roland donde está el sonido, OS/periféricos
  reemplazados respetando su interfaz.
* **Rendimiento**: 18 DSP × ~930 instr × 96 kHz ≈ 1.6 G instr/s en el peor caso → no alcanza un intérprete en un
  núcleo: traducir cada programa a código nativo una vez (JIT o generación de C; casi no hay saltos) y paralelizar
  (MXM se congela por muestra → los DSP de una muestra son independientes, un hilo por DSP); saltear DSP sin voces.
* **Prototipo propuesto (nivel 1)**: emulador de DSP en C (intérprete, después JIT) como biblioteca llamable desde
  Python; conectarlo al motor del JX-3P en Unicorn; render de una nota a WAV y comparación con la captura del equipo.
* Pendiente antes de pruebas de audio: el usuario puso el fine tune en 0 (verificar en la próxima tanda; la
  referencia ya dio 440.15 Hz).

## Verificación de lo pendiente de `DSP_USOS.md` (2026-10-17, tarde) — tablas fijas resueltas
* **Banco 5 indexado** (`re/dspmodescan.py`: los 256 modos de transferencia con el banco 5 lleno de marcas, DSP3
  del maestro): `XdaY`/`XdeY` = banco5[⌊h6⌋ + Y], `XdbY`/`XdfY` = banco5[⌊h7⌋ + Y] (h7 ← r0 con `40c0`: **h7 =
  segundo índice**), `Xd0Y`…`Xd7Y` = banco5[N·16 + Y]. Carga y store (`9daY`; el ARM ve lo guardado). Truncado, una
  instrucción entre `4080` y la carga alcanza (`re/dspb5idx.py`). 1024 palabras persistentes, índice mod 0x400
  (0x800/0xC00/0x1000 alias); 0x400–0x7FF sólo dura la muestra (`re/dspb5alias.py`, `re/dspb5map.py`).
  `d8`/`d9`/`dc`/`dd`: dirección que cambia con el tiempo (sin decodificar).
* **Slaves por audio** (`re/dspb5osc.py` + `re/dspharm.py`, ranura DCO2): tabla de 16 puntos en A100 → triángulo
  (1, 0, 0.111, 0, 0.042…) y cuadrada (1, 0, 0.315, 0, 0.180…) exactos; el programa que sólo lee sigue sonando igual
  > 10 s después de sacar el llenado. **Wavetables verdaderas posibles**: ~10 instrucciones por voz, sin importar N.
* Banco 5 de Roland (corpus): A000–A047, A200–A205, A300–A303, A4xx → libre 0x048–0x1FF y 0x306–0x3FF.
* **Banco 0**: tabla fija posible sólo sin ningún store al banco 0 en el DSP (h6 = T + n, n = contador en el banco
  5; 3 s intacta en el DSP3; un store común la borra en < 10 ms; `re/dspb0fix.py`). La vista RAW del ARM del banco
  0 también es relativa al anillo (lecturas seguidas del ARM ≈ 1 muestra entre sí repiten la misma palabra física).
* **Retardo indexado del banco 0 oído en los slaves** (`re/dspb0dly.py`): SAW + SAW retrasada 92 muestras → nota 72
  a 1045.9 Hz (fundamental cancelado); k = 0 → 523.6 Hz.
* **Línea de retardo** (DSP4 `f7dc`, `re/dspmtap.py`): **24 pedidos por muestra** funcionan (límite no hallado);
  **fracción de 6 bits (1/64), truncada**.
* SAW de referencia comprobado al empezar (1/n). Al cerrar: `dspown.py restore` (ranura DCO2 = silencio de `lab_pad4k`); el maestro quedó con sus programas originales (las sesiones restauran).
* Simulador: `dspisa.py` ejecuta `da`/`de`/`db`/`df`/`d0`–`d7` (banco 5, mod 0x400).

## Muestras largas: dónde guardarlas y cómo llegan al DSP (2026-10-17, noche)
* **Lado ARM (plugout)**: 2 MiB de flash por ranura = región 0 (maestro) ≤ 1 MiB comprimida + región 1 (slaves) ≤ 1 MiB
  comprimida; cada una se descomprime en una ventana de 2 MiB de SDRAM. El LZSS del contenedor **agranda** el audio
  (PCM 16 bits: ratio 1.114) → ≈ 0.94 MB de PCM por región (≈ 470 K muestras de 16 bits). Con códec propio en el
  ARM (8 bits / ADPCM 4 bits) ×2 / ×4. Región 1 ya ocupa 200–280 KiB de código.
* **Lado DSP**: banco 5 = 1024 palabras por DSP (ciclos, LUT). **EDM = 0x200000 palabras por BMC** (el driver la
  borra entera al iniciar, también `s_boot` en los slaves). La línea de `f7dc` ocupa **EDM 0x110000–0x1FFFFF =
  983 040 palabras = 60 unidades = 10.24 s a 96 kHz** (por eso u = 100 no devolvía nada); 0x000000–0x10FFFF en 0
  (¿otras ventanas?). WMEM 0x41300000 (IDM, 128 K) queda en 0 con nuestra línea: no es la memoria de retardo.
* **ARM ↔ EDM** (`re/dspedm.py`): lectura por puerto indirecto OK desde el maestro (dir → sys+0x40, `0xA0000000` →
  sys+0x44, dato en 0x41158000 IEEE / +8 RAW; driver `s_boot` 0x01112150). **Escritura con la secuencia del driver
  (dir, dato a +0/+4/+8; 0x01111D8C / 0x01111DE0 / 0x0111210A) no queda**, tampoco con comandos `8/9/B/C/D/E0000001`
  en sys+0x44. Ojo: tras `D0000001` el bit 28 de sys+0x44 quedó en 1 (`0x18000000`), el delay siguió andando.
  Hay una interfaz de objeto en `s_boot` 0x01114560.. (cfg, wr float/int/raw, rd) que llama a esas funciones.
* **El DMA escribe `[4300]` en la línea cada muestra aunque no haya store** (repite el último valor): una tabla en
  la EDM hay que refrescarla. **Loop**: escribir de vuelta la toma de edad a → la línea guarda un loop de ≈ a
  muestras (`re/dspedmloop.py`: toma(a) = toma(2a) 45 s, a = 409 602; diferencia estable de pocas muestras = latencia).
* Tomas de retardo en los slaves: programas del motor del SYSTEM-8 `p0s_004a404`, `p0s_004abd8`, `p0s_005085c`,
  `p0s_0051140` (4 pedidos cada uno) → los slaves tienen ventana de retardo en alguna ranura (sin ubicar).

## Wavetable verdadera en el banco 5: `re/wt5.py` (2026-10-17, noche)
* Script todo en uno: `python3 re/wt5.py send TABLA.wav [--frames K] [--points N] [--frame F | --scan HZ]`
  (conversión + envío en vivo + reproductor). Detalle y línea de comandos: `CUSTOM_OSC.md` "Wavetable verdadera en el
  banco 5".
* **Banco 5: un solo store por instrucción** (con 2 o 6 no queda ninguno; 1 al banco 5 + 1 al banco 0 sí): el
  primer intento con stores de a 6 (clase 0x722) dejó la tabla vacía (sonó basura). El simulador no lo modela.
* Oído en los slaves: seno de 437 puntos limpio; sierra (2 × 218) bien. `--scan` sin validar. Al cerrar quedó
  sonando la tabla de 8 cuadros con `--scan 0.25` en la ranura DCO2.

## Envío rápido de tablas: comandos DSPLIVE nuevos (2026-10-17, noche)
* `engine.c` (DSPLIVE): ADDR ext bit 1 = escribir DSP2 y DSP3 con un comando; ext bit 2 = destino banco 5: DATA =
  18 bits (valor·2¹⁷, complemento a 2) → RAW (`live_raw18`) → DPC RAW del slave (0x41114000 + 0x4000·n + 0x2000).
  `re/dsplive.py`: `Link.write(..., both=True)`, `Link.write_b5(start, values)`. `re/dspown.py load`: los dos DSP a
  la vez y sólo las palabras cambiadas (`OWN_FULL=1`, `OWN_BOTH=0`). `re/wt5.py`: tabla como datos por defecto
  (`--code` = vía vieja). Antes ≈ 2 min por tabla máxima, ahora ≈ 11 s (estimado: 15 ms por comando).
* Test emulado `engine/test_live.py --modes` OK (escritura doble y banco 5, valores RAW exactos). Lab reinstalado
  en el slot 3 (DSPLIVE + PEEK, `lab_pad4k`, `dco1_=z_dco1.txt`), `state.json` = lab instalado.
* Que el ARM del slave pueda escribir su banco 5 está supuesto por el mapa del maestro: verificar en el equipo.

## Próximos pasos del ISA (para seguir)
Referencia: `DSP_ISA_SPEC.md` (especificación, manda), `DSP_ISA.md` (resumen), `DSP_ARM_SIDE.md` (mapa de bancos
del lado ARM, análisis estático), `CUSTOM_OSC.md` (SAW/FOLD/wavetables propios, laboratorio, presupuesto de tiempo).
`DSP_USOS.md`: para qué sirven las operaciones halladas (usos, secuencias mínimas, qué falta verificar).
Estado al 2026-10-16 (noche): corpus 93.6 % (`dspisa.py cov`); antes: SAW de Roland 90 % decodificado, corpus 90,4 % (medido el 15/10; lo del 16/10 agrega
semántica: inmediatos compartidos entre ops A/B, clases A+B y A+A+B, costo de 1 ciclo, presupuesto ≈ 940
instrucciones por muestra).

Estado del equipo al cerrar (2026-10-16, noche): reiniciado una vez a la tarde (el volcado de WMEM colgó el
maestro); después todas las sesiones del maestro (DSP3 y DSP4 `f7dc`) cerraron restaurando el original
(verificado: delay `f7dc` = `m1_tcm_f7dc_01134238` palabra por palabra, DSP3 main = `0001 0001 0b03 0080 7fee 0001`).
El laboratorio de los slaves no se tocó hoy. Estado anterior: slot 3 = laboratorio de 4096 palabras (DSPTEST + DSPLIVE + PEEK, nombre
C-SAW, `lab_pad4k.txt`; `work/dsplive/state.json` = lab_pad4k). En la ranura DCO2 quedó la Fuzz Saw de 290 puntos
(`re/dspexp/own/wt2/live_fuzz290.txt`). `python3 re/dspown.py restore` vuelve al silencio de `lab_pad4k`.
C-WAVE v2 se reinstala con `NAME=C-WAVE WAVETABLE=1 OWN_PROGS=dco1_w0=…/wt2/dco1_w0.txt,… engine/build.sh` +
`tools/plugout_tool.py install plugins/C-WAVE --slot 3`.

Herramientas del 16/10 (ISA, todas por el peek, salidas en el banco 5 `A3C0..`): `re/dsptap.py` (línea de retardo
propia en `f7dc`), `re/dsprdly.py` (mediciones dentro del delay de Roland, `--d3`, `--drop`/`--remove`),
`re/dsphreg.py` (palabras `40TY` → h4/h5), `re/dspidx.py` (indexado `X28Y`), `re/dspidxring.py` (indexado y
anillo), `re/dspbus.py` (stores MXM/IOMD vistos por el ARM), `re/dspbus2.py` (DSP3 → DSP4), `re/dspx9bscan.py`
(busca lo escrito con `X9bY`), `re/dspmacfit.py` (fórmula de una instrucción con registros aleatorios, `--dump`),
`re/dspisa.py cov` (cobertura del corpus).

Herramientas anteriores: `re/wav2dco.py` (WAV → programa, imprime instrucciones y lo que queda para la otra DCO),
`re/wtcheck.py` (escala + armónicos contra la tabla), `re/dspbudget.py` (presupuesto de tiempo en el DSP3 del
maestro, sin audio), `dspasm.aab()` (instrucción A/A/B).

Reglas aprendidas (antes de cada sesión de medición):
* Programas de prueba en el maestro: **≤ ~900 instrucciones** (uno de ~1200 en `f7dc` dio todo 0); medir en tandas.
* No volcar la WMEM (0x41300000) entera con un programa propio en `f7dc` (colgó el maestro).
* Las salidas de prueba van al banco 5 (`A3C0..`, DPC*n* RAW 0x4111E000 + 0x4000·(n−2)): las lecturas sueltas del
  banco 0 caen en otras referencias por el anillo. Marcas únicas por referencia (no reutilizar valores entre corridas).
* Comprobar el SAW de referencia (`dspown.py load dspexp/own/saw_dco2.txt` + escala → armónicos 1/n). Si sale mal
  (silencio, ruido, tono fijo) apagar y prender el equipo: un programa que se pasa del presupuesto deja los slaves
  inestables y el reset MIDI no alcanza.
* Contar instrucciones, no palabras: DSP2/3 ≈ 940 por muestra con main2/3 = 189 → las dos ranuras DCO ≈ 750.
* Una sola herramienta MIDI a la vez; `dspmaster.py --restore` si se corta una sesión del maestro.

**Plan para la próxima sesión (2026-10-17)**: probar por audio en el laboratorio de los slaves (PLG3, ranura DCO2)
lo hallado hoy, empezando por el **retardo indexado del banco 0** (vibrato/chorus propio: `[S] ← x`, h6 = (S + k)/16384,
`X28Y` + lerp; ver `DSP_USOS.md` §2) y el indexado/`40TY` en los slaves; después seguir con la lista de abajo
(`X9bY`, h7, latencia MXM). Antes: comprobar el SAW de referencia.

Pendiente (ISA):
1. (Hecho el 16/10, ver "Tomas de retardo") `Xa9Y` = pedido de toma. Falta: palabras `40xx` con otros bytes
   bajos, tamaño de la línea, cómo se configura una línea en otra ranura/DSP (¿plugout propio en los slaves, donde
   corren chorus de J60/JX?), y probarlo por audio.
2. (Hecho el 17/10, ver "Banderas `X9bY`") `X9bY` = fijar banderas. Falta: grupo 0 / `c9bY`, unidades Y = 3–4,
   banderas de resultados MAC/SFU y de min, probarlo en los slaves.
3. (Hecho el 16/10: IOMD directo; MXM Z/S del DSP3 → zona O del DSP4.) Falta: latencia del bus, escritura desde el
   ARM a MXM y lectura en el DSP, mapa completo en los slaves.
4. (Hecho el 16/10: D ≥ 8 = mitad, op2 010 = imm16, A en clase MAC lee N.) Falta op2 = 011 y bit 9 de op2.
5. (Bits de condición: resuelto el 17/10, son banderas.) Ops B/SFU sin decodificar (`cf16`/`cf26` resueltos el 16/10; `8f09` parcial; resultados
   `7ffxxxxx`); (h4/h5 resueltos: registros del retardo) palabras de inicio/fin de programa; destino 3 (h7) de `40TY`.
6. Por qué un programa que se pasa del presupuesto deja el equipo inestable (¿se puede detectar o evitar?).
7. (Hecho el 16/10: 93.6 %, `dspisa.py cov`.)

### Banderas `X9bY` (2026-10-17, maestro DSP3 y DSP4 `efdc`, `re/dspx9bflags.py`)
* Pista del corpus: cada `X9bY` va seguido (a 2 instrucciones) de C words sobre el grupo X−8, y su instrucción
  tiene una op en la unidad Y. Las pruebas viejas fallaban porque usaban `1803 0040 b9bP` (unidad 1 = `r0 = r0` con
  r0 = 0: resultado siempre 0) y leían grupos que no correspondían a X.
* **`X9bY`: grupo X−8 (9/a/b → cond 4–7 / 8–11 / 12–15) ← banderas del resultado de la unidad Y de la misma
  instrucción.** Por grupo: c0 = ≠0, c1 = ≥0, c2 = sin overflow (|x| < 512), c3 = ≤0. Patrones: positivo `1110`,
  negativo `1011`, cero `0111`, overflow positivo `1100`, negativo `1001`, 512 → 0 `0101`.
* X = 0–8, d–f: nada. X = c (Roland lo usa antes de cond 0–3): en el maestro no cambió el grupo 0, que siempre lee
  `0111` (banderas de un cero).
* Y = unidad (1 = A, 2 = B medido en la clase 0x328 `6505 7c0f be0c 99b1 b9b2`).
* **Latencia de una instrucción** (como las cargas); **persisten** entre muestras, programas y sesiones: los "bits
  fijos por contexto" eran las últimas banderas que dejó el código de Roland.
* Op B max (`b4c0`): N = signo del resultado, pero Z = operandos iguales.
* **Op B nibble e con D ≥ 8** (bits 7–6): 00 = **compara** (banderas de r(d) + Y, no escribe nada: `be0c` = r3 − r4,
  `8e09` = r0 − r1, `ae10` = r2 vs inmediato), 01 = r(d) ← Y, 11 = r(d) ← r(d) + Y, 10 = escribe r(d) y r(d+1)
  raro, nunca usado. El desensamblador antes mostraba todas como `r(d) = Y` (mal para 00 y 11).
* Las sumas A **se envuelven**: magnitud mod 512, se conserva el signo (300 + 300 → 88; 511·2 → 510) y fija V.
* `dspisa.py` simula banderas (estado persistente, latencia 1, `sim` las imprime). Cobertura del corpus:
  **98.4 %** (antes 93.6 %); queda sobre todo `ff70`/`ef75`/`ef74` (262).
* Consecuencia: **selección dependiente de los datos** posible (if/else con C `x12`), ver `DSP_USOS.md` §5c.
* Más (tarde del 17/10): Y = 3 y 4 medidos (Y = Y-ésima op, MAC incluida); banderas de MAC, SFU y min medidas.
* **Saltos** `009Y` (en posición de C, cuenta como unidad para las C): desplazamiento con signo = palabra Y>>1 del
  flujo de inmediatos/slots, en palabras desde el final de la instrucción. Hallados en el corpus (JX: if/else y un
  `switch` de 4 ramas en `JX_b000_60a24bc8`), medidos en el maestro (`re/dspjump.py`) y **bucles** con saltos
  hacia atrás (3/8/16 vueltas exactas). **Audio en los slaves** (`re/dspflagsaw.py` → `dspexp/own/fsaw_*.txt`):
  SAW con wrap por banderas + C (`sel`), por salto (`jmp`) y con bucle de 4 vueltas (`loop4`) = idénticas a la
  referencia (65.897 / 221.677 Hz, armónicos 1/n). Ojo: el panel estaba una octava abajo +13 cents en todas.
* **Cuelgue de los slaves** por cargar en vivo un programa con salto hacia atrás: mientras se escribe, el DSP
  corre un programa mezclado y el salto cae dentro de código viejo → bucle infinito → hubo que apagar y prender.
  `dspown.load` ahora escribe primero un programa vacío y la cabeza al final. Los programas de Roland no tienen
  saltos hacia atrás.
* Familia nibble f, bits 7–6 = 01 (`re/dsphops.py`, `re/dspf5.py`): `f4Y` = mover r(Y&7), `f5c` = posición de
  escritura del retardo (−1/16384 por muestra), `f6Y` = rd + h(Y−8) mod 512 (Y = c–f), `f7Y` = marcadores.
* Costo (`re/dspbudget.py skip|loop`, maestro DSP3): lo salteado por un salto no cuesta (1500 instrucciones
  salteadas y el contador sigue); bucle de 4 instrucciones: 180 vueltas sí, 185 no → ≈ 5 ciclos por vuelta (un
  salto tomado ≈ 1 ciclo extra).
* **El campo de condición de la C tiene 5 bits** (cond 0–31; el desensamblador enmascaraba con & 15): `c9bY` →
  grupo 4 (cond 16–19), `d9bY` → grupo 5 (cond 20–23); medido en el maestro (`dspx9bflags.py --high`) y por
  audio en los slaves (`dspflagsaw.build_group`: nivel de la SAW según la condición; `grp_*.txt`). Grupo 0 =
  `0111` constante, cond 24–31 = `1111`. X = 8, e, f: nada. Con & 15 los `2310`/`2a10` de Roland parecían cond 1/5.
* `dspown.freq` elegía a veces dos períodos (autocorrelación): ahora toma el primer pico ≥ 90 % del máximo.
  El panel tenía DCO2 en 16' (el usuario lo pasó a 8'); queda un +13 cents uniforme (fine/master tune).
* `f5Y` en los slaves por audio (SAW × valor, `dspexp/own/f5s_*.txt`): `cf54`/`cf55` = 0 (silencio), `cf5c` ≠ 0
  (posición del retardo). La salida bloquea la continua: medir siempre con una señal multiplicada.
* **Todos los binarios DSP de Roland** (`re/dsp_corpus_all.py`, `re/dsp_s1.py`): 212 programas únicos del SYSTEM-8
  (motor base 116 nuevos, plugouts nuevos JUPITER-8/PROMARS/SH-2 12 nuevos; región 0 de los plugouts sin DSP) +
  4 del SYSTEM-1 (mismo ISA, marcador `ff71`). 20 clases nuevas: plantillas inferidas (`inferred` en
  `dsp_templates.txt`). Decodificado todo salvo los marcadores de sección (`ef75` abre / `ef74` cierra la sección
  de cada voz). Nuevo: **ops lógicas** B con D < 8 (`0` AND, `2` ANDN, `4` OR, `6` ORN, `8` XOR, `a` XNOR,
  complemento a 2, resultado mod 512 = el formato `7ffxxxxx`; `re/dspbits.py`), **`9f0d`/`8f09` = separación
  atanh** para logaritmos ((x−1)/(x+1) con semilla de 1/x).
* **Verificación en el equipo de lo inferido (17/10, tarde)** — `re/dspverify.py` ejecuta una instrucción real en el
  DSP3 del maestro (referencias movidas a palabras libres del mismo banco y precargadas, registros y banderas al
  azar) y compara registros y memoria con `dspisa.run`:
  * Las 20 plantillas inferidas + las corregidas a mano: todas coinciden salvo 0x56a en un caso de C fuera de
    rango. Marcadas `verified` en `dsp_templates.txt`. Mapas de posiciones (`dspslots.py`) de 0x240/0x248/0x250/
    0x478/0x47a/0x4aa/0x4da coinciden con las plantillas.
  * Reglas nuevas que salieron de las diferencias: **inmediato = Y bytes dentro del área de slots** (no "desde el
    principio"; vale para A/B; en la MAC Y y W por su campo, C a continuación de Y salteando referencias);
    **puertos de lectura**: banco 0 = 2 cargas por instrucción (con más, sólo la última), banco 5 = 1 (con más,
    ninguna); **C**: el campo elegido actúa sólo si el bit 3 del otro campo es 0; campo 0/fuera de rango anula
    todas las ops sólo en clases de 3 ops A con una sola C (Roland no lo usa); clase 0x190: campo 2 = la op;
    **B nibble e bits 7–6 = 11**: funciones de fase (± / abs / clamp / wrap / triángulo / fase 0–1 / −Y,
    `re/dspshape.py`); **B nibble c modos 1–3**: corrimientos X·2^±Y; **MAC op2 bit 9**: habilita productos en
    modo W.
  * Regresión al azar sobre todo el corpus (100 instrucciones sin saltos ni transferencias especiales, 2 casos al
    azar cada una): **100 / 100 coinciden** con el simulador final; plantillas inferidas 19 / 20 (0x56a: sólo su
    C fuera de rango).
* **Marcadores `ef75`/`ef74` medidos**: no cambian registros, banderas ni direccionamiento (bancos 0/5); una SAW
  con secciones de voz al estilo Roland suena idéntica en los slaves (`dspexp/own/saw_sections.txt`); cuestan 1
  ciclo cada uno (`dspbudget.py marker`); ningún código ARM los busca. Son opcionales para un programa propio.
* **Referencia consolidada del hardware: `DSP_HARDWARE.md`** (17/10, noche). Mediciones nuevas para el
  emulador:
  * Aritmética **exacta y truncada hacia cero** una sola vez (`re/dspexact.py`, 48/48 en suma, resta, producto,
    D + X·Y, MAC, MAC/2); desborde = magnitud mod 512.
  * ROM de semillas (`re/dspseed.py`): 1/x = 64 entradas, R[k] = round(4096/(0.5 + (2k+1)/256))/4096 (64/64);
    1/√x = 48 entradas, tabla literal (en la spec §6.4). La separación atanh usa la ROM de 1/x.
  * **Costo: 1 ciclo por instrucción siempre** (1 o 2 MAC, 4 o 6 transferencias; `dspbudget.py mac|mac2|xfer4|
    xfer6`); salto tomado +1.
  * VST3 (los 9): `CDspSim` + modelo float por sintetizador; **sin microcódigo ni intérprete del ISA** → no sirve
    de referencia bit a bit.
  * **Buses** (maestro, DSP3 ↔ DSP4): MXM congelado por muestra, **latencia 1 muestra** (reloj común = posición
    del retardo), punto a punto (DSP3 Z/S → DSP4 O, DSP4 O → DSP3 S), gana la última escritura, no suma, no
    guarda (`dspbussync.py`, `dspbusclk.py`, `dspmxm.py`, `dspmxsum.py`). IOMD = palabras de E/S con DSP dueño,
    persistentes, con datos vivos en el maestro (`dspiomd.py`). Los efectos del maestro no usan MXM/IOMD: banco 5
    `a1fe`/`a2fe` con su programa principal.
* Plugouts nuevos desempaquetados en `work/unpack/{JUPITER-8,SH-2,PROMARS}` (+ `*_S1_Code*` del SYSTEM-1).
* Clase 0x098: la 2.ª palabra es una C (`1610` = if cond11), no N: las "MAC op2 = 011" del corpus eran eso.
* Plantillas corregidas (C que eran inmediatos/N o una T que era B): 0x098, 0x0e0, 0x130, 0x178, 0x1d8, 0x208, 0x240, 0x572,
  0x592. Cobertura 98.6 %; queda casi sólo `ff70`/`ef75`/`ef74`.

### Tomas de retardo `{8+d}f6c` / `Xa9Y` y bancos 2/4/5 (2026-10-16, maestro, `re/dsptap.py`)
* **Persistencia por banco** (contador load W / store W en el DSP3): banco 0 anillo (load S+1, 96 000/s); **banco 2
  (`4xxx`) = dos páginas que alternan cada muestra** (load W lee lo guardado hace 2 muestras: 48 000/s; lo guardado se
  relee en la misma muestra; W+1 no); banco 4 (`80xx`, zona M) no persiste entre muestras (0/s; en la misma muestra
  sí se relee); banco 5 (`Axxx`) memoria simple persistente (96 000/s); IOMD (`6010`) no se relee; `4800` lee 0.
  Las lecturas del ARM del banco 2 del DSP3 (0x41720000, IEEE) titilan entre las dos páginas; las escrituras del ARM
  caen en una sola.
* **Línea de retardo propia funcionando** (ranura `f7dc` del DSP4 del maestro, la única con el banco 2 conectado a
  la memoria de retardo; en el DSP3 nada de esto tiene efecto). Secuencia mínima por muestra (`re/dsptap.py`):
  1. `[4300] ← x` (puerto de escritura de esta ranura: el ARM lo ubica en la **posición 324** de un anillo que
     avanza una muestra por muestra, como el banco 0 pero en la memoria de retardo; unidad = 16384 muestras).
  2. `1d06 4022 7fe0 c1fc <imm32>`: fija la **base de `f6c`** = bits crudos del inmediato >> 22 (`5100 0000` → 324,
     `4800 0000` → 288, `3000 0000` → 192, `0` → 0). El valor que deja otra tarea del DSP4 es 288 (los `efdc`
     usan `1d05 4020 3a90 4800 0000`). La palabra `40xx` tiene el bit 6 del byte alto, no es una C común; otros
     bytes bajos (`4003`, `4010`, `4020`…) dan efectos raros: **sin decodificar**.
  3. `{8+d}f6c`: **rd = (rd + base) mod 512** (dirección de lectura; aplicado dos veces: 302.648 → 78.648).
  4. `0b03 4050 <addr>`: **destino de los resultados en el banco 2** (`0301` → `[4301..4303]`; Roland `0304` para
     la toma 2, J60 `0001`/`0004`). **Sin esta palabra el pedido se ignora** (por eso mis primeras pruebas daban 0).
  5. `da90` (`0c02 3a90` o el `1b04 c510 3a90 0008` de Roland): el pedido con la dirección en rd.
  6. Cargar `[W+1]` = x[n − L − ⌊16384·(A − 324)⌋], `[W+2]` = una muestra más vieja, `[W+3]` = fracción de
     16384·(A − 324) (0.25/0.5/0.75 medidos exactos), con A = dirección de 3. → `r0 + frac·(r1 − r0)`.
     L = 2 muestras en mi programa corto, 3 en el de Roland (posición en el programa). Con base = 324 el retardo
     es directamente 16384·u + L. Medido: u = 0 → 3, 0.5 → 8196, 1 → 16388, 10 → 163844, 50.648 → 829826 (8.6 s),
     100 → nada (fuera de la línea); base 330 → edad +6 unidades; base 300 → zona "futura" (0).
* El delay global que corre (`m1_tcm_f7dc_01134238`): r3 = 14.648 (≈ 2.5 s) antes de `bf6c`, base 324.
  `[4303]` = 1/64 y `[4306]` = 3/16 son sus fracciones reales. `X9bY` no cambia la base (probado puertos 1–3).
* Herramienta para medir dentro del programa de Roland: `re/dsprdly.py` (contador como entrada del delay, salida de
  audio en 0, `--d3` fuerza el retardo, `--drop`/`--remove` bisecan instrucciones). Salidas por el **banco 5**
  (`A3C0..`, DPC4 RAW 0x41126000): el ARM lo lee coherente (las lecturas sueltas del banco 0 caen en otras
  referencias por el anillo).
* **Registros ocultos h4–h7 y palabras `40TY`** (DSP4 `f7dc` y DSP3, salidas por el banco 5): una palabra en
  posición de C con byte alto `0x40` **carga un registro oculto**: bits 7–6 = destino (0 → h4, 1 → h5, 2 → h6,
  3 → h7?), bits 5–4 = modo (0: rY, guardado mod 512; 1: imm16; 2: imm24; 3: imm32 → 0), bits 3–0 = Y (registro, o
  **cuántas palabras del flujo de inmediatos saltear**: `4022` en `1d06 4022 7fe0 c1fc 5100 0000` saltea `7fe0 c1fc`).
  Los inmediatos son enteros = valor × 16384 (direcciones: `0x510000` → 324, `0x0301` → 0x301).
  * **h4 = base de `f6c`** y lo que suma la op A 0 sub 4 (los "valores ocultos" de antes): 288 en el DSP4.
  * **h5 = destino de los resultados de la toma** en el banco 2 (`4050 0301`), lo que suma la op A 0 sub 5.
  * **h6 = índice de la carga indexada** (`4080` = h6 ← r0, como el S-100 antes de `0c02 0280`).
* **Direccionamiento indexado (DSP3 del maestro, exacto)**: modo de transferencia `28`: `X28Y` con X 0–7 = rX ←
  banco0[⌊h6⌋ + Y], X 8–15 = banco0[⌊h6⌋ + Y] ← r(X−8) (`8280` escribió 7 y se releyó). Índice en unidades de
  r·16384, truncado, módulo 0x800 (sólo banco 0, relativo al anillo como cualquier referencia); la palabra R no se
  suma; varias por instrucción (`1e04 0281 1282` → r0 = [h6+1], r1 = [h6+2]). **Sirve para wavetables en memoria**
  (tabla en el banco 0 + índice de la fase) en vez de inmediatos.
* `X9bY` sigue sin decodificar: en `f7dc` tampoco fija condiciones (grupos `TTTF` con o sin escritura; la primera
  línea base distinta fue de la primera carga), no cambia h4/h5/h6, no escribe la línea de retardo, no es un store
  indexado del banco 0; el delay de Roland funciona sin sus `X9bY`.
* Cuidado: un programa de prueba de ~1200 instrucciones (más que el presupuesto) en `f7dc` dio todo 0; el maestro
  siguió bien. Medir en tandas de ≤ 8 casos (`re/dsphreg.py`; indexado: `re/dspidx.py`).
* **Indexado relativo al anillo** (`re/dspidxring.py`): contador con load `[h6+1]` / store `[h6]` cuenta 96 000/s,
  con `[h6]`/`[h6]` no. Una tabla fija no puede vivir en el banco 0; en cambio `[S + k]` indexado = lo guardado en
  S hace k muestras → **retardo variable < 2048 muestras en cualquier DSP** (chorus/flanger sin DMA). Usos en
  `DSP_USOS.md`.
* **Bancos 3 y 4 vistos desde el ARM y desde otro DSP** (`re/dspbus.py`, `re/dspbus2.py`, DSP3 del maestro):
  * IOMD: `[60nn]` = palabra nn del bloque IOMD (0x4113C000 IEEE / 0x4113E000 RAW): confirmado.
  * MXM, stores del DSP3: zona M (`81F0`) → ventana MXM2 (0x41430000), O (`83F0`) → MXM3, Z (`85F0`) y S (`87F0`)
    → MXM5; cada una aparece en las palabras 0x1F0 y 0x3F0 de la ventana.
  * **Entre DSP**: lo que el DSP3 guarda en Z o S lo lee el DSP4 (`f7dc`) en su zona O (`83F0`, misma palabra);
    lo de M y O no le llega. Bus de audio propio entre DSP del maestro confirmado (latencia sin medir).
* `X9bY` (puertos 1–3) con una marca única: **no aparece** en las 6 ventanas MXM, IOMD (3 vistas), bancos 0/1/2/5
  del DSP3 ni banco 5 del DSP2 (`re/dspx9bscan.py`): su destino no está mapeado para el ARM.
* **MAC, casos pendientes** (DSP3 del maestro, `re/dspprobe.py`, `re/dspmacfit.py` con registros aleatorios, exacto):
  * **MAC con D ≥ 8: r(D−8) = resultado / 2** (`0803 8c0a 9281` = (r1 + r2·(r0 − r1))/2; `cc0a` → r4).
  * **op2 = 010 → c = imm16** (`0804 0809 5480 3000` = r0·r1 + 1.0·r5): los modos de c son 00 rC, 01 imm16, 10 imm24,
    11 imm32, como en las ops A. op2 = 011 da 0.0898 fijo con cualquier inmediato: sin decodificar.
  * **MAC + op A con inmediato** (clase 0x088): la A **lee como inmediato la palabra N** de la MAC
    (`1108 7522 6d10 0025 800d 0002 2222 3000` → r6 = r4 + RAW `00250000`); la MAC no cambia. No combinar una A con
    constante propia en una instrucción MAC (en las clases con transferencias el inmediato sí saltea las R).
* **SFU** (`re/dspmacfit.py --dump`, registros aleatorios y 2, 9, 1/64, 100, −2…): `cf16` = separación para raíz
  (rY ← m ∈ [1,4) con |x| = m·4^−k, r5 ≈ 1/√m, r7 = signo·2^−k → √|x| = r7·m·r5); `cf26` = separación log2
  (rY ← m ∈ [0.75,1.5), r4 ← e; negativo → r4 = 9, rY = x/512); `8f09` parcial (x > 0: r0 = (x−1)/2).
* **Ops B con D ≥ 8** (`re/dspmacfit.py --dump`): `b242` = min(−X, Y), `b642` = max(−X, Y) (bit 1 del nibble niega
  X); `bc42` = **frexp**: X = m·2^e, m ∈ [0.5, 1) → r(d) = m, rY = e (0 → 0, −31).
* **Cobertura** (`python3 re/dspisa.py cov work/corpus`, con todo lo del 16/10 en `dspisa.py`): **17877 / 19095 =
  93.6 %** (antes 90.4 %). Resto: `X9bY` ≈ 650 (3.4 %), marcadores `ff70`/`ef75`/`ef74` 262 (1.4 %, sin efecto
  medible), ops B sueltas (`eda8`, `e7d2` con inmediato o X de r4–r7, `8c0f`…), `xfer.e0`.
* WMEM (0x41300000, 128 K palabras, el tamaño del IDM) leída entera con el delay de Roland en silencio: todo 0.
  **Un segundo volcado con mi programa en `f7dc` colgó el maestro** (el peek dejó de responder; USB seguía): no
  volver a leer la WMEM con un programa propio en esa ranura (o leer de a poco).

Pendiente (osciladores, cuando se retome):
* `wav2dco.py` → plugout completo con N WAV elegidos, repartiendo puntos según el presupuesto de cada DSP.
* Antialiasing por nota (apagar puntos / armónicos según el pitch) y morphing entre tablas.
* Decisión del usuario: empezar el compilador / SDK (DSL embebido en Python: grafo, registros, estado, voces),
  validado con el laboratorio.

## C-V4: plug-out propio de ocho voces (2026-10-22)

Detalle en `FROM_SCRATCH.md` (cómo se construye) y `VOICE_ALLOCATION.md` §6–§8 (análisis, emulador, medida, límite de
voces). Resumen:
* `C-V3` sólo sonaba la voz 0. El emulador nuevo (`examples/from_scratch/emu/`, 20 pruebas) encontró tres errores en
  nuestra cadena: el desfase de MXM que no existe, las voces de DSP5 hacia `60B0` (no se oye) y la falta de la suma de
  `6000`–`6003` (la entrada del slave 1). Además, el banco 1 es un anillo.
* `C-V4` (`examples/from_scratch/v4/`) corrige los tres. Medido con `voicelog.py`: 8/8 voces en los dos slaves,
  sueltas y en acordes, ±3 cent. `emu/sigcheck.py`: sin bytes de Roland.
* Pendiente: envolvente, filtro y LFO propios; palabras exactas del enlace entre slaves; afinación en mono; si el OS
  admite más de 8 voces.
