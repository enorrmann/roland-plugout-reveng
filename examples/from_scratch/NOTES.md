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
* Callbacks del OS en 0x01000400 (kernel): [0]=get(parte,grupo,id,&v)->1,
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
