# SYSTEM-1

Trabajo sobre el SYSTEM-1 (el equipo, no un PLUG-OUT del SYSTEM-8): imágenes del sistema operativo y el intento
de llevarlo de 4 a 6 voces. Todo lo de esta carpeta es análisis estático; en el equipo solo se ha leído la
versión (1.30).

## Documentos

| Documento | Qué cubre |
|---|---|
| [FIRMWARE.md](FIRMWARE.md) | Fuentes del firmware (actualizador 1.30 sin cifrar, `Code1.Dat` 1.20 del plug-in), cómo se extraen, organización del maestro y del slave, direcciones conocidas (asignador, servicios, envío MIDI) |
| [VOCES_6.md](VOCES_6.md) | Análisis para 6 voces: programas DSP de voz y su coste, ids maestro→slave, sitios del asignador, cómo se instalaría y próximos pasos |
| [SH101_POLY.md](SH101_POLY.md) | SH-101 (PLUG-OUT para SYSTEM-1) a 4 voces: sus dos programas DSP, servicios de carga del slave (colas por DSP, ranuras de voz), plan |
| [../docs/examples_system_1_README.md](../docs/examples_system_1_README.md) | C-S1V4 (`examples/system_1/`): el C-V4 propio portado al SYSTEM-1; interfaz de PLUG-OUT del SYSTEM-1, cadena de audio esclavo → maestro, build y prueba |

## Scripts

Desde la raíz del repo. Necesitan `firmware/sys1_sys_v130.zip` y el plug-in en `plugins/SYSTEM-1/`.

| Script | Qué hace |
|---|---|
| `python3 system-1/extract_fw.py` | descomprime las imágenes 1.30 y 1.20 en `work/sys1_v130/` y `work/sys1_v120/` |
| `python3 system-1/voice_sets.py [IMAGEN] [--entries]` | lista los sets de programas DSP del slave y su coste en instrucciones con 4 y con 6 voces |
| `python3 system-1/fnmatch.py A B 0xADDR:N …` | busca en la imagen B la función de la imagen A (ignora los desplazamientos relativos al PC) |
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
* C-S1V4 (`examples/system_1/`, 2026-10-22): plug-out propio de 4 voces construido y simulado, sin instalar. En
  [../docs/examples_system_1_README.md](../docs/examples_system_1_README.md) §2 se corrige lo de los seis DSP: cada
  BMC tiene dos (A = `FC30`, B = `FC34`); `FC38`/`FC3C` son colas de datos.
* No se ha instalado nada en el equipo.
