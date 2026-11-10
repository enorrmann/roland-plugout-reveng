# Implementación de ensamblador y desensamblador para el DSP BMC de Roland
## Informe de faltantes, ambigüedades y decisiones necesarias

**Base revisada:** especificación técnica aportada por el usuario, estado indicado 2026-10-16/17.  
**Objetivo:** identificar qué falta confirmar o definir antes de implementar un ensamblador y un desensamblador fiables.  
**Criterio:** no se asume que una inferencia sea un hecho. Los elementos descritos como `[unknown]`, `[inferred]`, hipótesis o reglas pendientes deben conservar ese estado hasta verificarse.

---

## 1. Resumen ejecutivo

La especificación permite comenzar un **desensamblador estructural y un ensamblador de un subconjunto documentado**, pero no basta por sí sola para garantizar una implementación completa, reversible y segura para todo el corpus.

Los principales bloqueos no son la codificación básica de las palabras, sino:

1. **Faltan los artefactos de referencia citados**: `re/dspisa.py`, `re/dsp_templates.txt`, `re/dsp_units_ab.txt`, [Vol. II ch. 1](../../docs/vol2_dsp_core_isa/core_architecture.md), [Vol. II ch. 7](../../docs/vol2_dsp_core_isa/isa_overview.md), corpus y scripts de verificación. Sin ellos no se puede reproducir la tabla completa de clases, contrastar las reglas ni establecer una suite de regresión.
2. **La gramática de fuente no está definida**: sintaxis de instrucciones, directivas, etiquetas, expresiones, literales RAW, comentarios, selección de DSP/slot y formatos de salida.
3. **La tabla de plantillas no equivale todavía a una especificación formal de todas las clases**: varias plantillas proceden de un resolver de fallback; hay campos `C` dentro del área de slots, varios `C`, longitudes poco frecuentes y clases marcadas como inferidas.
4. **Hay reglas de inmediatos que requieren una definición única y ejecutable**, en especial la interacción entre múltiples operaciones A/B, MAC, slots `N`, referencias `R`, ancho de inmediatos y lecturas que pueden invadir la instrucción siguiente.
5. **No está especificada la política de pérdida de información** para clases, palabras, modos o instrucciones desconocidos. Esto es esencial para que el desensamblador pueda modificar y volver a emitir firmware sin corromper partes que no entiende.
6. **Faltan decisiones de producto**: si el objetivo es round-trip binario exacto, código legible, ensamblado nuevo, análisis del corpus o las cuatro cosas; también faltan los formatos de entrada/salida y las garantías de compatibilidad.

**Recomendación:** separar el proyecto en dos niveles: (a) un desensamblador lossless que siempre pueda preservar las palabras originales y (b) un ensamblador simbólico estricto que solo genere construcciones con semántica y plantillas verificadas. No emitir codificaciones desconocidas por conjetura.

---

## 2. Preguntas bloqueantes que hay que responder

Estas decisiones deben cerrarse antes de considerar estable la interfaz pública.

### 2.1 Alcance funcional

- [ ] ¿Se necesita solo ensamblar/desensamblar secuencias de palabras de 16 bits, o también extraer programas desde imágenes del SYSTEM-8/SYSTEM-1?
- [ ] ¿La herramienta debe procesar programas de slots, programas principales, programas de efectos y todos los contextos, o solo slots de oscilador?
- [ ] ¿Se exige reconstrucción binaria exacta (`assemble(disassemble(bytes)) == bytes`) para programas completos, incluidos encabezados, marcadores y palabras desconocidas?
- [ ] ¿Se necesita también una representación canónica, donde dos secuencias equivalentes puedan producir el mismo fuente aunque no sean textualmente idénticas?
- [ ] ¿Se requiere edición y reensamblado de programas Roland existentes, o también generación de programas nuevos desde cero?
- [ ] ¿Debe la herramienta conocer el modelo de DSP, número de DSP, slot y contexto para interpretar bancos, marcadores y direcciones?
- [ ] ¿Qué versiones concretas de SYSTEM-8, SYSTEM-1 y PLUG-OUTs deben considerarse compatibles?

### 2.2 Entorno e interfaz

- [ ] Lenguaje y versión mínima de runtime.
- [ ] Interfaz requerida: CLI, biblioteca/API, módulo Python compatible con `re/dspisa.py`, o combinación.
- [ ] Formatos de entrada: texto de palabras hexadecimales, binario crudo, dump ARM de 32 bits, imagen de firmware o varios.
- [ ] Formatos de salida: fuente ensamblador, flujo de palabras de 16 bits, imagen de memoria de 32 bits little-endian, JSON u otros.
- [ ] ¿Debe respetarse exactamente el formato de imagen descrito (`mem32[i] = stream[2i] << 16 | stream[2i+1]`) o se necesita además exportación binaria convencional little-endian?
- [ ] ¿Qué códigos de salida, mensajes de error y niveles de diagnóstico espera la integración?
- [ ] ¿Se necesita una opción `--strict`, una opción de conservación de desconocidos y un modo de compatibilidad con firmware original?

### 2.3 Lenguaje ensamblador

- [ ] Sintaxis textual deseada: mnemónicos de alto nivel (`ADD`, `MAC`, `LD`) o sintaxis próxima a campos (`A`, `B`, `M`, `N`, `C`, `T`, `R`, `I`).
- [ ] Convención para registros (`r0`–`r7`, `h4`–`h7`) y para bancos/direcciones.
- [ ] Sintaxis de literales RAW: ¿se acepta exclusivamente hexadecimal exacto o también constantes decimales convertidas al formato RAW?
- [ ] Si se permiten constantes decimales, ¿qué política de redondeo se exige al convertirlas?
- [ ] Sintaxis de etiquetas y expresiones, especialmente para saltos relativos al final de la instrucción.
- [ ] ¿Se permiten directivas de datos, relleno, alineación, cabeceras de programa, marcadores de slot y bytes sin decodificar?
- [ ] ¿Se permite una instrucción en forma de palabras crudas (`.word16`) dentro de una fuente simbólica?
- [ ] ¿Cómo se expresa una plantilla explícita cuando una clase admite varias longitudes o interpretaciones?
- [ ] ¿La herramienta debe conservar el orden original de los campos y las palabras no interpretadas al editar una instrucción?

---

## 3. Datos de referencia que faltan para implementar y validar

La especificación menciona estos recursos, pero no están incluidos en el documento recibido. Deben entregarse o confirmarse como dependencias accesibles.

| Recurso citado | Por qué es necesario |
|---|---|
| `re/dspisa.py` | Referencia ejecutable del decoder/simulador; sirve para comparar la interpretación y las reglas de ejecución. |
| `re/dsp_templates.txt` | Tabla de plantillas por clase y estado de verificación. Debe convertirse en una tabla de datos formal para el ensamblador. |
| `re/dsp_units_ab.txt` | Ayuda a resolver qué palabras son operaciones de unidad A o B. |
| [Vol. II ch. 1](../../docs/vol2_dsp_core_isa/core_architecture.md), especialmente §8 y §11 | Contexto de buses y lista actual de elementos no resueltos. |
| [Vol. II ch. 7](../../docs/vol2_dsp_core_isa/isa_overview.md) | Historial de decisiones, definiciones previas y posibles contradicciones ya conocidas. |
| [Vol. II ch. 6](../../docs/vol2_dsp_core_isa/memory_banks_dma.md) | Mapa de bancos y vistas de memoria desde ARM. |
| [Vol. VI ch. 6](../../docs/vol6_reference_designs_applications/custom_osc.md), [Vol. II ch. 9](../../docs/vol2_dsp_core_isa/operation_uses.md) | Ejemplos reales y usos de las instrucciones. |
| Corpus original y corpus ampliado | Pruebas de cobertura y round-trip; el documento menciona 212 programas SYSTEM-8 y 4 SYSTEM-1. |
| `re/dsp_corpus_all.py`, extracción del corpus | Reproducir cómo se localizaron los programas y verificar límites y cabeceras. |
| `re/dspslots.py`, `re/dsptemplates.py` | Comprender la asignación de posiciones y las plantillas generadas por heurística. |
| `re/dspverify.py` y scripts relacionados | Repetir pruebas contra hardware y separar evidencia medida de inferencias. |
| Dumps binarios originales con hashes | Evitar que los fixtures cambien y demostrar reproducción exacta. |

**Pregunta:** ¿Estos recursos forman parte del repositorio que debe usar la implementación y pueden considerarse la fuente de verdad? Si sí, hay que fijar commit/hash y versiones.

---

## 4. Codificación de instrucciones: definiciones que deben formalizarse

### 4.1 Decodificación del encabezado

La regla indicada es `class = header >> 5`, `length = header & 0x1f`, con longitud de 1 a 31 palabras de 16 bits.

Falta precisar el comportamiento de la herramienta cuando:

- la longitud es cero;
- la longitud excede las palabras restantes del archivo;
- una clase no está en la tabla;
- una instrucción tiene una longitud no observada para esa clase;
- una palabra de encabezado cae dentro de una región que debería conservarse como datos o marcador;
- una secuencia es truncada a mitad de instrucción.

**Decisión propuesta:** el desensamblador debe reportar el problema y preservar el tramo original como datos crudos, sin intentar resincronizar silenciosamente. La resincronización heurística, si se implementa, debe ser opcional y anotada como no fiable.

### 4.2 Catálogo de clases y plantillas

La especificación presenta una tabla amplia de clases, pero advierte que:

- las plantillas se resolvieron a partir de la longitud más frecuente;
- otras longitudes suelen diferir en el número de palabras `I`, algo inferido en general;
- las plantillas con `C` dentro del área de slots o con varios `C` requieren revisión;
- parte de las clases añadidas desde el motor base fueron inicialmente inferidas;
- en una prueba de 20 clases inferidas, la clase `0x56a` no coincidió en el campo `C` fuera de rango.

Para cada clase, falta una definición de máquina que indique:

- plantilla base exacta y posiciones de cada campo;
- número y orden de unidades de ejecución;
- reglas de longitud mínima/máxima y longitudes válidas;
- si las palabras finales son `I`, slots `N`, referencias `R`, o posiciones ambiguas;
- reglas específicas de interpretación de los `C` y transferencias;
- estado de confianza por regla, no solo por clase;
- ejemplos positivos y negativos;
- si se permite ensamblar la clase en modo estricto.

**Pregunta:** ¿La tabla final autoritativa será la de la especificación o la que produzca `re/dsp_templates.txt` en un commit fijado? No deben coexistir dos fuentes de verdad.

### 4.3 Orden de campos y slots

La regla general dice que el cuerpo contiene operaciones A/B/M/C, luego transferencias T, luego el área de slots y finalmente inmediatos; pero también documenta excepciones y una regla corregida: el área de slots incluye primero las palabras `N`, luego las referencias a las que apuntan las transferencias `fe`, y después los bits de inmediato que no estén referenciados.

Falta una especificación formal para resolver todas las posiciones, incluyendo:

- varias transferencias que apuntan al mismo slot;
- slots sin referencia;
- referencias que apuntan a posiciones fuera de la instrucción;
- transferencia con modo distinto de `fe`, que no consume slot;
- transferencia con `Y` impar, caso raro marcado como desconocido;
- instrucciones con `C` dentro del área de slots;
- instrucciones con varios `C` y/o palabras que el resolver clasificó de forma ambigua;
- slots `N` asociados a cada `M` cuando hay varios MAC;
- relación entre los slots `N`, las referencias y el stream de inmediatos.

**Pregunta:** ¿Existe una rutina de clasificación de slots que pueda usarse como especificación normativa, o debe escribirse una nueva a partir de los datos medidos?

---

## 5. Inmediatos: ambigüedades que afectan directamente al ensamblador

Este apartado es de prioridad alta. La especificación documenta mediciones nuevas que reemplazan reglas anteriores, pero quedan combinaciones sin caracterizar.

### 5.1 Stream y anchuras

Reglas documentadas:

- modo 1 = 16 bits, modo 2 = 24 bits, modo 3 = 32 bits;
- el inmediato es el prefijo de 16/24/32 bits del valor RAW de 32 bits;
- los bits se leen en orden MSB-first y sin alineación por palabra;
- las palabras `I` se rellenan con ceros al final hasta completar palabras de 16 bits;
- una lectura puede continuar dentro de las palabras de la siguiente instrucción si el programa está mal formado.

Hay que definir y probar explícitamente:

- cómo se codifica un inmediato negativo en cada ancho;
- si los bits bajos descartados al reducir 32 a 24 o 16 se descartan siempre sin redondeo;
- qué sucede si el ancho solicitado no está disponible en el área `I`;
- cómo debe actuar el ensamblador ante un inmediato que necesita más bits de los disponibles;
- si el desensamblador debe mostrar el valor RAW reconstruido, el prefijo leído, o ambos;
- si el relleno final debe considerarse parte de la instrucción para round-trip exacto.

### 5.2 Inicio del inmediato por desplazamiento Y

La especificación indica que A/B comienza a leer el stream de inmediato a `Y` bytes dentro del área de slots y que MAC usa desplazamientos de Y y W, con reglas adicionales para C y para referencias que se saltan. Falta una formulación algorítmica completa que defina:

- qué significa exactamente “byte Y” respecto a un stream formado por palabras de 16 bits;
- si Y siempre representa un desplazamiento en bytes o si hay excepciones por clase;
- el comportamiento cuando el desplazamiento apunta dentro de una palabra de 16 bits;
- el orden de prioridad cuando el desplazamiento cruza slots `N` y referencias `R`;
- la regla precisa para el desplazamiento de `C` del MAC cuando Y es registro frente a inmediato;
- qué pasa si dos operaciones leen regiones solapadas del stream;
- si las palabras de inmediato deben considerarse un único stream global o un stream por instrucción con ventanas desde distintas posiciones.

### 5.3 Varias operaciones A/B y MAC

La especificación indica que varias operaciones A/B de una instrucción comparten el inicio del stream, cada una con su ancho propio; también dice que el ensamblador debe asignar como máximo un valor inmediato a una instrucción, y que la combinación de MAC más A/B con inmediatos es desconocida.

Hay que resolver:

- ¿Puede una instrucción tener simultáneamente inmediato de A, inmediato de B e inmediato de MAC?
- Si hay dos operaciones con inmediatos de diferente anchura, ¿ambas leen desde el mismo byte inicial, como en los ejemplos medidos?
- ¿Cómo se expresan varios valores deseados que no pueden coexistir por compartir el mismo stream?
- ¿Qué clases concretas están permitidas para el ensamblado simbólico de esta combinación?
- ¿Debe rechazarse cualquier combinación no medida, aunque el formato físico permita codificarla?
- ¿Cómo se codifican los operandos inmediatos de MAC cuando el `N` correspondiente ocupa la primera posición del área de slots?
- En clase `0x088`, una operación A toma su inmediato de la palabra `N` del MAC. ¿Hay más clases con una interacción equivalente?

**Decisión propuesta:** no emitir combinaciones MAC + A/B con inmediatos salvo que haya una prueba de referencia específica. El desensamblador puede conservarlas en representación cruda o anotarlas como interpretación parcial.

---

## 6. Semántica de operaciones que requiere tratamiento especial

### 6.1 RAW numérico

La representación RAW se describe como signo-magnitud con exponente de dos bits y mantisa de 29 bits. La aritmética se describe como exacta y truncada hacia cero, con envoltura para resultados A fuera del rango representable.

Falta definir para la API:

- si la conversión de constantes de usuario a RAW forma parte del ensamblador o queda fuera;
- política para NaN, infinito, valores fuera del rango y redondeo de mantisa;
- si `-0` debe normalizarse a `0x00000000` o preservarse como patrón de bits si aparece en fuente cruda;
- representación textual de patrones especiales como `7ffxxxxx`;
- si la herramienta necesita evaluación de expresiones RAW en tiempo de ensamblado;
- tratamiento uniforme de los casos límite en A, B, MAC y SFU.

La implementación debe operar sobre enteros y reglas exactas, no sobre `float` de host, si se exige equivalencia bit a bit.

### 6.2 Unidad A y variantes especiales

Hay semántica detallada para operaciones A, subcódigos de op 0 y destinos `D >= 8` en clase `0x048`, pero la forma `D >= 8` no está medida en clases con varias operaciones.

- [ ] Confirmar si el ensamblador debe rechazar esas formas fuera de la clase donde fueron medidas.
- [ ] Precisar el comportamiento de la suboperación Newton cuando los registros destino/origen se solapan.
- [ ] Confirmar los casos de overflow y la representación exacta del resultado cuando se alcanza el límite de 512.
- [ ] Identificar cualquier patrón de bits reservado que deba rechazarse.

### 6.3 Unidad B y funciones especiales

La tabla de unidad B incluye operaciones bit a bit, min/max, compare, shifts, `frexp`, envoltura, funciones de forma y SFU. Algunas variantes se describen como medidas; otras son inferidas o desconocidas.

Falta una tabla normalizada por patrón de bits con:

- máscara y valor de selección exactos;
- operandos leídos y registros escritos;
- efectos colaterales adicionales;
- banderas que genera cada variante;
- estado de verificación y conjunto de pruebas;
- patrones reservados/ilegales;
- diferencias entre comportamiento de master DSP y DSP esclavos, si existen.

### 6.4 MAC

La fórmula general está descrita, pero hay casos especiales en función de `op1`, `op2`, modo del coeficiente y destinos `D >= 8`.

- [ ] Resolver o prohibir `op2 = 011`, descrito como resultado constante desconocido.
- [ ] Confirmar si el modo de coeficiente `01` (inmediato de 16 bits) puede considerarse estable en todos los contextos; actualmente se marca como inferido.
- [ ] Definir validación para combinaciones de `op1`/`op2` no observadas.
- [ ] Confirmar los destinos MAC `D >= 8` y su división por dos, incluidas banderas y overflow.
- [ ] Especificar si se puede emitir MAC con operandos que se solapan con otros destinos de la misma instrucción.
- [ ] Definir la sintaxis de dos MAC en una misma instrucción y cómo se relaciona cada `M` con su palabra `N`.

### 6.5 Banderas y predicación

Las banderas son persistentes y tienen cinco grupos escribibles documentados, latencia de una instrucción y condiciones constantes. El comportamiento de algunos campos fuera de rango es específico de clase y no intuitivo.

Falta definir:

- una tabla completa de las 32 condiciones, su origen y el valor de verdad;
- validación formal de los campos `skipT` y `skipF`, incluidos valores fuera de rango;
- comportamiento normativo para clases con varios `C`;
- regla exacta de unidades para cada clase, incluyendo MAC, saltos y transferencias;
- qué debe hacer el ensamblador con combinaciones de predicación que el firmware Roland no usa;
- si las banderas del simulador deben ser parte del contrato público de la herramienta.

### 6.6 Saltos

Los saltos `009Y` usan un desplazamiento con signo relativo al final de la instrucción y cuentan en palabras de 16 bits. Se indica que los saltos hacia atrás funcionan y que los destinos del corpus caen en límites de instrucción.

Falta cerrar:

- si el desplazamiento se interpreta siempre como entero con signo de 16 bits tomado de las palabras del stream de inmediato, y qué pasa con desplazamientos impares o destinos no alineados;
- si las etiquetas simbólicas se resuelven sobre palabras de 16 bits o sobre instrucciones;
- cómo se calcula el desplazamiento cuando las longitudes cambian por la inserción de inmediatos;
- si se requiere una segunda pasada o relajación de saltos;
- comportamiento para destino fuera del programa;
- si un salto puede apuntar deliberadamente a datos o a un marcador;
- reglas para programas con saltos sobre palabras desconocidas.

**Decisión propuesta:** la sintaxis de etiqueta debe ser relativa a límites de instrucción y el ensamblador debe rechazar destinos no alineados o fuera de rango, salvo modo raw explícito.

---

## 7. Transferencias, bancos y estado oculto

### 7.1 Modos de transferencia

Se conocen `fe` y alias observados, `28`, modos de banco 5 (`da`–`df`, `d0`–`d7`), `a9` y `9b`. Otros modos están marcados como desconocidos.

- [ ] Definir si alias como `ff`, `3e`, `0e`, `10`, `12` deben conservarse literalmente o normalizarse a `fe`.
- [ ] Para desensamblado, ¿se muestra el modo exacto original aunque sea equivalente a `fe`?
- [ ] El ensamblador estricto debe rechazar modos desconocidos; falta decidir si el modo raw permite preservarlos.
- [ ] Precisar las restricciones de ancho de banda por banco como validaciones opcionales o solo como advertencias.
- [ ] Confirmar cómo se representa la transferencia `a9` y la secuencia de configuración necesaria para que tenga efecto.
- [ ] Confirmar la semántica de modos temporales/desconocidos (`d8`, `d9`, `dc`, `dd`, etc.) antes de permitir su emisión simbólica.

### 7.2 Direccionamiento y bancos

Los prefijos de direcciones se describen parcialmente como referencias de banco, pero varios se basan en análisis estático o hipótesis.

- [ ] Definir la representación de una dirección: valor raw de 16 bits, banco + offset, o ambas.
- [ ] Establecer si la herramienta valida límites de banco o permite cualquier referencia de 16 bits.
- [ ] Formalizar la relación entre dirección de referencia, banco físico y vista ARM.
- [ ] Precisar cómo se muestran las direcciones de mailbox, parámetros, IOMD, MXM y delay memory.
- [ ] Determinar si la herramienta es solo de ISA o también debe conocer el mapa de memoria del hardware y el contexto del programa.

### 7.3 Registros ocultos

`h4`–`h7` no son registros ordinarios; se cargan mediante palabras `40TY` en posición C. `h7` no se puede leer directamente y los valores pueden depender de tareas anteriores.

- [ ] Definir sintaxis y representación de `40TY` en la fuente.
- [ ] Precisar validación de los modos de inmediato y del salto de palabras de stream que usa la carga.
- [ ] Determinar si el desensamblador debe mostrar los valores como registros ocultos o como palabra `C` cruda.
- [ ] Definir si la herramienta hace análisis de estado para avisar sobre uso de `h4`–`h7` sin inicialización.
- [ ] Confirmar si se requiere simulación de su estado entre programas y muestras, o si queda fuera del alcance del ensamblador/desensamblador.

### 7.4 Memoria de delay y buses

La semántica de delay depende de la conexión física que el ARM haya configurado para un slot; MXM e IOMD también dependen del DSP propietario y del calendario de ejecución.

- [ ] Confirmar si la herramienta debe generar mnemónicos de alto nivel para las secuencias de delay o solo decodificar las palabras.
- [ ] Determinar cómo representar el contexto externo requerido para saber si una transferencia realmente alcanza la delay memory.
- [ ] Definir si las advertencias sobre latencia de buses y persistencia de memoria pertenecen al ensamblador, al simulador o a una herramienta de análisis separada.

---

## 8. Marcadores, cabeceras y formato de programa

La especificación indica patrones de inicio y fin para slots y un patrón medido para el programa principal. Los marcadores tienen semántica interna desconocida o no visible, aunque algunos se han observado como marcadores de sección.

- [ ] Definir perfiles de imagen: `slot`, `main`, `SYSTEM-1` u otro.
- [ ] Precisar si los marcadores se escriben como directivas (`.slot_start`, `.voice_begin`) o como palabras crudas.
- [ ] Determinar si el ensamblador inserta automáticamente cabeceras/finales o si la fuente debe contenerlos explícitamente.
- [ ] Definir si los `0001` de relleno/no-op se preservan exactamente.
- [ ] Especificar cómo manejar la nota de blobs de `4n+2` bytes y la palabra final `0001`, para evitar que una conversión a palabras de 32 bits añada o pierda `0000`.
- [ ] Definir el formato exacto de serialización a memoria ARM de 32 bits y cómo tratar un número impar de palabras.
- [ ] Confirmar el comportamiento de `ff70`, `ff71`, `ef75` y `ef74`: deben preservarse incluso si se muestran como marcadores sin efecto visible.

---

## 9. Política de desconocidos y reversibilidad

Este requisito debe fijarse antes de implementar el parser.

### Modos recomendados

1. **Lossless/raw:** cada palabra se conserva exactamente; las instrucciones conocidas pueden anotarse, pero el formato fuente siempre permite reconstruir el binario original.
2. **Estricto:** solo se ensambla código con clase, plantilla, campos y semántica permitidos explícitamente.
3. **Exploratorio:** el desensamblador muestra hipótesis y advertencias, pero estas no habilitan automáticamente al ensamblador a emitir la codificación.
4. **Canónico:** opcional, para normalizar alias y constantes; no garantiza identidad textual con el original.

### Preguntas que faltan

- [ ] ¿Se exige identidad binaria exacta por defecto?
- [ ] ¿Los bytes/palabras desconocidos se representan con `.word16`, `.raw` o bloques opacos?
- [ ] ¿Puede editarse una instrucción conocida adyacente a una desconocida sin moverla o reinterpretarla?
- [ ] ¿Qué pasa si el tamaño de una instrucción cambia y afecta a los destinos de saltos?
- [ ] ¿Se debe guardar metadato de confianza (`exact`, `measured`, `inferred`, `unknown`) dentro de comentarios o estructura de datos?
- [ ] ¿Qué comportamiento debe tener el ensamblador si una instrucción está parcialmente decodificada?

**Requisito mínimo recomendado:** el desensamblador nunca debe descartar bits ni transformar silenciosamente un modo desconocido en otro equivalente.

---

## 10. Pruebas de aceptación que todavía deben especificarse

La documentación cita mediciones y regresiones, pero no define una suite de aceptación independiente para el nuevo ensamblador/desensamblador.

### 10.1 Pruebas de formato y round-trip

- [ ] Para cada programa del corpus, desensamblar y reensamblar debe producir exactamente el mismo flujo de palabras.
- [ ] Convertir flujo de 16 bits a imagen ARM de 32 bits y viceversa debe preservar orden y cantidad de palabras.
- [ ] Archivos truncados, longitudes inválidas y clases desconocidas deben producir errores/diagnósticos deterministas.
- [ ] Probar instrucciones de longitudes mínima, máxima y todas las longitudes registradas por clase.
- [ ] Probar preservación de marcadores y de la última palabra de programa.

### 10.2 Pruebas de codificación

- [ ] Casos de referencia de las secciones 11 y 12 deben ensamblar a las palabras exactas indicadas.
- [ ] Probar campos extremos: `D`, `op`, `X`, modos `mY`, registros, transferencias, referencias y desplazamientos de salto.
- [ ] Probar inmediatos de 16, 24 y 32 bits, incluidos negativos, ceros, patrones RAW y anchuras mezcladas.
- [ ] Probar solapamientos entre ventanas de inmediato y slots `N`/referencias `R`.
- [ ] Probar saltos hacia delante y hacia atrás, desplazamiento cero, límite de rango y destino no alineado.
- [ ] Probar todas las combinaciones de campos prohibidas o no medidas para comprobar que se rechazan en modo estricto.

### 10.3 Pruebas semánticas

- [ ] Comparación diferencial con `re/dspisa.py` para todas las instrucciones conocidas y casos de operandos.
- [ ] Repetir el conjunto de pruebas de hardware descrito por `re/dspverify.py`, donde sea posible.
- [ ] Separar pruebas del ensamblador/desensamblador de las pruebas del simulador: el éxito de round-trip no demuestra semántica correcta.
- [ ] Pruebas específicas para las anomalías documentadas: carga con latencia, banderas con latencia, ring de bank 0, accesos bank 5, MAC, SFU, saltos y transferencias de delay.
- [ ] Registrar el modelo/contexto DSP en las pruebas donde el resultado dependa de él.

### 10.4 Métricas de cobertura

- [ ] Definir qué significa “cobertura”: palabras decodificadas, instrucciones decodificadas, clases cubiertas, semántica validada o programas completos.
- [ ] Establecer umbrales de cobertura para el corpus antiguo, corpus SYSTEM-8 ampliado y SYSTEM-1.
- [ ] Asegurar que las palabras que quedan sin decodificar son únicamente las conocidas (por ejemplo, marcadores de sección), o documentar cada excepción.

---

## 11. Arquitectura de implementación propuesta

La arquitectura puede avanzar sin resolver todas las incógnitas, siempre que los límites entre capas sean explícitos.

1. **Capa de transporte/formato:** leer y escribir palabras de 16 bits y la imagen de memoria de 32 bits con el orden especificado.
2. **Decoder estructural:** leer encabezado y longitud; nunca inferir una clase desconocida a partir de palabras posteriores.
3. **Catálogo de plantillas versionado:** una entrada por clase, con posiciones, restricciones de longitud, estado de confianza y procedencia de la evidencia.
4. **Decoder de campos:** interpretar A/B/M/N/C/T/R/I solo cuando la plantilla lo permita.
5. **Representación intermedia (IR):** preservar palabras originales, offsets, longitud, campos interpretados, warnings y estado de confianza.
6. **Desensamblador lossless:** producir texto legible más directivas crudas suficientes para reconstrucción exacta.
7. **Parser de fuente simbólica:** procesar mnemónicos/directivas/etiquetas una vez acordada la gramática.
8. **Ensamblador de dos pasadas:** resolver etiquetas y longitudes; emitir solo combinaciones permitidas.
9. **Validador estático:** verificar longitud, posiciones, índices de slots, destinos de salto y restricciones conocidas.
10. **Suite de regresión diferencial:** comparar salida binaria con el corpus y semántica con el simulador de referencia.

No conviene mezclar la lógica de decodificación con la de simulación: la ISA puede decodificarse aunque una operación todavía tenga semántica desconocida.

---

## 12. Criterios mínimos para considerar la implementación lista

- [ ] Round-trip exacto del corpus disponible, sin pérdida de palabras.
- [ ] Separación clara entre semántica exacta, medida, inferida y desconocida.
- [ ] El ensamblador estricto no emite patrones `unknown` ni combinaciones no caracterizadas.
- [ ] Cada clase ensamblable tiene una plantilla formal, longitudes permitidas y pruebas.
- [ ] Las etiquetas de salto se resuelven correctamente con desplazamiento relativo al final de la instrucción.
- [ ] Los inmediatos se codifican según una única regla documentada y testeada.
- [ ] Los marcadores y el formato de imagen no cambian durante round-trip.
- [ ] Los errores de entrada malformada no provocan pérdida silenciosa de datos ni resincronización opaca.
- [ ] Se documenta qué no cubre el proyecto: simulación completa, extracción del firmware, mapas de memoria o análisis contextual, según el alcance acordado.

---

## 13. Lista de decisiones para el responsable del proyecto

Responder estas preguntas permite convertir este informe en una especificación de implementación cerrada:

1. ¿El objetivo prioritario es round-trip exacto, ensamblar código nuevo, o ambos?
2. ¿Qué lenguaje y CLI/API se requieren?
3. ¿Qué archivos y commit del repositorio contienen la referencia ejecutable y el corpus?
4. ¿Qué formatos de entrada/salida deben soportarse?
5. ¿Qué sintaxis de ensamblador se prefiere: mnemónica, cercana a campos, o ambas?
6. ¿Debe haber un modo raw que permita preservar cualquier palabra desconocida?
7. ¿Qué clases inferidas se pueden ensamblar y cuáles quedan deshabilitadas hasta verificar?
8. ¿Se exige soporte simbólico para saltos y resolución automática de etiquetas?
9. ¿La conversión de constantes decimales a RAW es parte del alcance?
10. ¿La semántica de ejecución/simulación forma parte del proyecto o solo el ensamblado y desensamblado?
11. ¿Se debe soportar extracción de programas desde imágenes SYSTEM-8/SYSTEM-1?
12. ¿Qué modelos/firmwares y qué conjunto de corpus definen la aceptación?

---

## 14. Conclusión

La especificación ya contiene una cantidad considerable de conocimiento medido y ejemplos útiles. El riesgo principal es tratar como cerradas reglas que el propio documento identifica como inferidas, específicas de clase o desconocidas. Para avanzar de forma segura, primero hay que fijar los artefactos de referencia y el objetivo de reversibilidad; después formalizar la tabla de plantillas y las reglas de inmediatos; finalmente definir la gramática del ensamblador y la suite de aceptación.

Hasta que se resuelvan esos puntos, es viable implementar un decoder estructural y una representación raw lossless, pero no es recomendable prometer un ensamblador general para toda la ISA.
