# SYSTEM-1

Trabajo sobre el SYSTEM-1 (el equipo, no un PLUG-OUT del SYSTEM-8): imágenes del sistema operativo y el intento
de llevarlo de 4 a 6 voces. Todo lo de esta carpeta es análisis estático; en el equipo solo se ha leído la
versión (1.30).

## Documentos

| Documento | Qué cubre |
|---|---|
| [DSP_BANKS.md](DSP_BANKS.md) | Bancos de datos de los DSP: semántica, rangos que usan el OS y los PLUG-OUTs oficiales, reglas, medida de la línea de retardo |
| [FX_CHAIN.md](FX_CHAIN.md) | Cadena de efectos (delay + reverb) en modo PLUG-OUT: el programa del OS en el maestro B `0x0` (107 instr., memoria de retardo), su enlace por el banco 1 con la salida del maestro A `0x82`, parámetros `C000`–`C00C`, ids del panel `0x18D`–`0x18F`, por qué no suena con C-S1SY y siguientes pasos |
| [FIRMWARE.md](FIRMWARE.md) | Fuentes del firmware (actualizador 1.30 sin cifrar, `Code1.Dat` 1.20 del plug-in), cómo se extraen, organización del maestro y del slave, direcciones conocidas (asignador, servicios, envío MIDI) |
| [VOCES_6.md](VOCES_6.md) | Análisis para 6 voces: programas DSP de voz y su coste, ids maestro→slave, sitios del asignador, cómo se instalaría y próximos pasos |
| [SH101_POLY.md](SH101_POLY.md) | SH-101 (PLUG-OUT para SYSTEM-1) a 4 voces: sus dos programas DSP, servicios de carga del slave (colas por DSP, ranuras de voz), plan |
| [../docs/examples_system_1_README.md](../docs/examples_system_1_README.md) | C-S1V4 (`examples/system_1/`): el C-V4 propio portado al SYSTEM-1; interfaz de PLUG-OUT del SYSTEM-1, cadena de audio esclavo → maestro, build y prueba |
| [DSP_FIT.md](DSP_FIT.md) | ¿Cabe el microcódigo del JX-3P (u otro PLUG-OUT del SYSTEM-8) en los 4 DSP del SYSTEM-1? Tiempo por programa, enlaces entre DSP, reparto; resultado: 4 voces no, 2 voces sí en el papel; con un solo DCO, 4 voces caben en tiempo; §4 compara SH-101 / SH-2 / PROMARS |
| [../docs/examples_system_1_jx_README.md](../docs/examples_system_1_jx_README.md) | C-S1JX: el microcódigo del JX-3P (un DCO, 4 voces) reubicado en los 4 DSP del SYSTEM-1; puesta en marcha, pruebas, riesgos. Construido y probado sin hardware, sin instalar |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Arquitectura: chips (maestro + 1 esclavo, 2 DSP por BMC), qué código corre dónde, flujos, ventanas de los DSP, herramientas |
| [../docs/DSP_PROGRAM_MEMORY.md](../docs/DSP_PROGRAM_MEMORY.md) | Memoria de programa (principal, secciones, cierre) del SYSTEM-1 y del SYSTEM-8, usos y límites |
| [VOICE_ALLOCATION.md](VOICE_ALLOCATION.md) | Asignación de voces: del teclado a los ids `0x110+v`/`0x120+v`, reparto del sinte de serie por DSP (secciones, banco 1 por voz, modulación en el maestro), PLUG-OUTs y C-S1V4 |
| [WAVE_BUS.md](WAVE_BUS.md) | Bus de audio: esclavo A → B → IOMD `6006`–`600D` → maestro B → `A118`/`A119` → salida del OS `6000`/`6001`; qué corre en el maestro en cada modo; medidas |

## Scripts

Desde la raíz del repo. Necesitan `firmware/sys1_sys_v130.zip` y el plug-in en `plugins/SYSTEM-1/`.

| Script | Qué hace |
|---|---|
| `python3 system-1/extract_fw.py` | descomprime las imágenes 1.30 y 1.20 en `work/sys1_v130/` y `work/sys1_v120/` |
| `python3 system-1/voice_sets.py [IMAGEN] [--entries]` | lista los sets de programas DSP del slave y su coste en instrucciones con 4 y con 6 voces |
| `python3 system-1/fnmatch.py A B 0xADDR:N …` | busca en la imagen B la función de la imagen A (ignora los desplazamientos relativos al PC) |
| `python3 system-1/dsp_fit.py [--voices 2] [--dco1/--dco2/--fx …]` | mide en el emulador el tiempo de DSP y los enlaces del JX-3P y busca un reparto en el SYSTEM-1 ([DSP_FIT.md](DSP_FIT.md)); necesita `examples/from_scratch/engine/build/programs.h` |
| `python3 system-1/s1_progs.py [DIR…] [--listing]` | programas DSP de un PLUG-OUT del SYSTEM-1 (SH-101, SH-2, PROMARS): dónde van, instrucciones ejecutadas, IOMD; comparación de polifonía en [DSP_FIT.md](DSP_FIT.md) §4 |
| `python3 system-1/vfp_count.py [IMAGEN…]` | cuenta instrucciones de coma flotante (VFP) en imágenes ARM: el SYSTEM-8 las usa, el SYSTEM-1 no ([ARCHITECTURE.md](ARCHITECTURE.md) §1.1) |
| `python3 system-1/asgn_sites.py [IMAGEN] [INICIO FIN]` | lista las instrucciones del asignador del maestro con las constantes de "4 voces" |

## Estado (2026-10-22)

* El equipo tiene el firmware 1.30, que coincide con `SYSTEM1_UPD.BIN`. Como el actualizador no está cifrado, no
  hace falta el volcado por SysEx que se usó en el SYSTEM-8.
* El `Code1.Dat` del plug-in SYSTEM-1 es una actualización del OS (1.20), no un PLUG-OUT, y no se puede instalar en
  el slot 1.
* 6 voces: el tiempo de DSP de los programas de voz alcanza (peor caso estimado ≈ 740 de ≈ 925 instrucciones). El
  espacio en la memoria de programa, los datos por voz, los otros DSP y el motor del slave están abiertos
  ([VOCES_6.md](VOCES_6.md) §6).
* SH-101 a 4 voces: el SH-101 es un programa de 575 instrucciones en DSP0 (más 12 en DSP1); no caben dos voces en
  un DSP, así que harían falta copias en DSP2–DSP5 y una mezcla ([SH101_POLY.md](SH101_POLY.md)).
* C-S1V4 (`examples/system_1/`, 2026-10-22): plug-out propio de 4 voces, **funciona en el equipo** (notas, acordes, volumen); la variante **C-S1V8 con 8
  voces también funciona** (el OS reenvía los ids de las voces 4–7). En
  [../docs/examples_system_1_README.md](../docs/examples_system_1_README.md) §2 se corrige lo de los seis DSP: cada
  BMC tiene dos (A = `FC30`, B = `FC34`); `FC38`/`FC3C` son colas de datos.
* Instalado en el slot 1: C-S1V8 (sustituye al SH-2; se recupera con `plugout_tool.py install plugins/SH-2 --slot 1`).
* C-S1JX (`examples/system_1_jx/`, 2026-10-22): JX-3P de Roland con un DCO a 4 voces; construido, 7 + 4 pruebas sin
  hardware pasan; **sin instalar** ([../docs/examples_system_1_jx_README.md](../docs/examples_system_1_jx_README.md)).
