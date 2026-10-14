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
* (stub de emu0, NO es el OS real) callbacks en 0x01000400: [0]=get(parte,grupo,id,&v)->1,
  [1]=set(parte,grupo,id,v), [2]=flush (nop), [3]=set2(parte,grupo,id).
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
2. **Región 0 en C**: modelo propio (SYNINFO, descriptor, lista de parámetros terminada en -1, objeto asignador de
   17 entradas). El `get/set` del maestro es `obj->vt[0x28](obj, parte, id, &v)` / `+0x24`, `+0x2C`, `+0x30`
   (ver región 0 del JX-3P en `0x6191815A`); revisar la sección "Región 0" de arriba contra eso.
3. Opcional: leer los valores reales de los ids fuera del patch desde la región 0 (arreglar `params_dump.py` /
   `init_params.s` con el objeto del OS) para validar las curvas.
