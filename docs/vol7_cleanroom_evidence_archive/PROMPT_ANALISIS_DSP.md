# Prompt: análisis de DSP y de la cadena de voces (SYSTEM-8 / JX-3P)

Tarea: analizar y documentar como es el flujo de datos y comunicacion entre todos los componentes del chip custom de roland mediante el estudio del microcodigo de todos los plugouts presentes en el repo, resolver por qué nuestro plug-out propio (C-V3) no suena en las voces 1–7, documentar en detalle la asignación de
voces de Roland y crear pruebas en el emulador que respalden cada conclusión.

## Reglas
- No copies microcódigo de Roland a ninguna build nueva ni a pruebas nuevas. Puedes desensamblarlo para estudiarlo
  (`dsp/dump_main.py`), pero los programas que construimos son nuestros (`examples/from_scratch/v3/make_v3.py`). Verifica
  con la comprobación de firmas de `docs/VOICE_ALLOCATION.md` que la build no contenga bytes de Roland.
- No hagas commit. No instales nada en el SYSTEM-8 sin pedirlo. Tras cada instalación el usuario debe volver a
  seleccionar PLG3.
- Nunca ejecutes dos herramientas que hablen con el equipo a la vez (MIDI/ALSA).
- Marca cada afirmación como [medido], [listado], [código], [documentado] o [inferido]. Si es inferida, di qué prueba la
  confirmaría.
- Escribe la documentación en `docs/` en español y actualiza `docs/INDEX.md`.

## Qué sabemos (con evidencia, pero validalo nuevamente cotejando con el codigo)
1. Asignación de JX-3P (`docs/VOICE_ALLOCATION.md` §1): pitch de las voces 0,1 en banco 5 de DSP0 (ids 0x21, 0x33);
   voces 2,3 en DSP1. Puerta de voces 0,1 en DSP4 (ids 0x2A, 0x39), voces 2,3 en DSP5 (0x29, 0x38) [código:
   `engine/engine.c` `note_gate`]. Cadena MXM: DSP0 Z 0404.. → DSP2 M 0004..; DSP1 Z 0400.. → DSP3 O 0200..; DSP2 Z
   0404..041A → DSP4 M 0005..; DSP3 Z 0400..0414 → DSP5 O 0201..0214 [listado].
2. Salidas: DSP4 → IOMD 6020/6021; DSP5 → IOMD 60B0/60B1 [listado]. Contradicción abierta: `docs/WAVE_BUS.md` dice que 60B0
   no suena (medido en otro montaje), pero en `C-SAWPOLY` (Roland) las voces 2 y 3 sonaron.
3. Osciladores: DCO1 escribe [221F]/[221B] (voz A) y [2227]/[2223] (voz B); DCO2 escribe [2221]/[221D] y [2229]/[2225]
   [listado de DCO1_SAW, DCO2_SAW]. Entradas de tono: [2207]/[2214] (DCO2) y [2201]/[220E] (DCO1), escritas por MAIN2 y
   MAIN3.
4. MAIN2 lee [2220], [2222], [2224], [2226], [2228], [222A]; ningún programa de ranura ni principal las escribe
   [listado]. Origen desconocido (¿ARM por la API de parámetros?).
5. MAIN4 escribe [221B], [221D] y [2229] él mismo: no son salidas de los osciladores.
6. Medidas en el equipo (`examples/from_scratch/voicelog/voicelog.py`, cruza el registro de voces de región 0 con el audio):
   - `C-SAWPOLY` (cadena de Roland + nuestros SAW): slave 0 4/4 voces; slave 1 3/4 (nota 55 muda en secuencial, sonaba en
     acorde).
   - `C-V3L` (nuestra cadena, poly): solo suena la voz 0. Mono: solo voz 0, 8/8 notas, afinación ±40 cent. Unison: 8/8
     notas, afinación exacta.
   - `C-V3L` poly no reproduce igual entre rondas de modos: la nota que suena es la asignada a la voz 0.
7. `engine/engine.c` (`note_pitch`, `note_gate`, `e_load`) es nuestra reproducción. El orden `main_prog[d]` → DSP d está
   confirmado en hardware por `C-SAWPOLY`, pero no por desensamblado del `e_load` de Roland.
8. Nuestro motor (`examples/from_scratch/v3/engine_v3.c`) apaga la puerta escribiendo 0 en DSP 4+u; `engine.c` original
   solo apaga el id de envolvente en DSP u.

## Preguntas abiertas (por orden de impacto)
- **A.** ¿Por qué la voz 1 (DSP2 voz B) no suena, con las mismas direcciones que Roland? Revisar la puerta 0x39 en DSP4 y
  la fase 0x680/0x681.
- **B.** ¿Por qué DSP3 (voces 2 y 3) no suena con nuestra cadena? ¿Llega DSP5 → 60B0 al audio?
- **C.** ¿Por qué el slave 1 entero no suena, si con la cadena de Roland sí?
- **D.** ¿Quién escribe [2220]–[222A]? Revisar la API del ARM (`engine.c` `p_bank`/`p_ramp`, `docs/DSP_ARM_SIDE.md`) y los
  demás principales.
- **E.** ¿Comparten banco 1 DSP2 y DSP3 (`docs/DSP_ARM_SIDE.md` líneas 20 y 190)?
- **F.** Orden dentro de la muestra: ¿el principal corre antes que la ranura?
- **G.** Desfase de MXM: ¿DSP2 Z 0405 llega a DSP4 M 0006 (la voz 1 de DSP2 depende de ello)?
- **H.** Afinación de mono (±40 cent): ¿glide/portamento del motor o error del tono?

## Herramientas que ya existen
- Simulador de DSP: `examples/from_scratch/re/dspisa.py` (`run(instr, regs, mem)`); desensamblador `dspdis.py`;
  ensamblador `asm/asm.py`.
- Simulación multi-DSP: `examples/from_scratch/v3/make_v3.py` (`simulate`: MXM con 1 muestra de retraso). Solo la voz 2
  está comprobada.
- Escáner de descriptores y referencias de bus: `examples/from_scratch/re/alloc_scan.py` y `alloc_mxm.py`.
- Desensamblado de programas de Roland: `examples/from_scratch/dsp/dump_main.py <NOMBRE>`.
- Medida en el equipo: `examples/from_scratch/voicelog/voicelog.py` (requiere la build con `VOICELOG=1`).
- Build de prueba: `examples/from_scratch/v3/build_v3.sh` (`NAME=...`; `VOICELOG=1` para el registro).

## Entregables
1. Banco de pruebas en el emulador (p. ej. `examples/from_scratch/emu/`) que simule los DSP con el orden de muestra, la
   latencia de MXM y la memoria de banco 1, banco 5 e IOMD. Cada pregunta A–H debe tener al menos una prueba que pueda
   fallar. Incluye regresión para la cadena de MAIN0–5 de v3.
2. Para cada pregunta: hipótesis, prueba en el emulador, resultado, y si requiere medida en el equipo, qué medida y qué
   resultado la confirmaría o descartaría. Las medidas en el equipo solo se proponen; no las ejecutes sin que el usuario
   lo pida.
3. Actualización de `docs/VOICE_ALLOCATION.md` y `docs/DSP_ARM_SIDE.md` con lo verificado, lo corregido y lo abierto.
   Corrige cualquier afirmación previa que el emulador contradiga.
4. Resumen final: qué causa cada fallo, qué parte es segura y qué parte sigue abierta.
