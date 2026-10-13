# Updater versions 1.30 / 1.31 / 1.32

The three updaters are in `firmware/sys8_sys_v130|131|132/SYSTEM8_UP.BIN`
(same tar layout, same six members). Roland's update history (supplied):

| Version | Date | Changes (Roland) |
|---|---|---|
| 1.32 | JUN 2022 | JUPITER-4 PLUG-OUT supported; fixed a freeze with a specific operation |
| 1.31 | FEB 2021 | JUNO-60 PLUG-OUT supported; JUNO-106 POLY→MONO/UNISON stuck note fixed; JUPITER-8 model-switch arpeggio fixed |
| 1.30 | AUG 2018 | JX-3P PLUG-OUT preloaded on PLUG-OUT 3 (factory reset needed); patch-import fix; minor fixes |

## What the unencrypted members show (sizes of the decompressed images)

| Member | v1.30 | v1.31 | v1.32 |
|---|---|---|---|
| `p0m` slot-0 model, SYSTEM-8 v1.20 | 1 166 152 | 1 166 152 (identical) | identical |
| `p0s` slot-0 engine | 536 220 | 536 308 | identical to 1.31 |
| `pl1m` JUPITER-8 model v1.06 / engine | 1 160 460 / 342 404 | identical / 342 424 | identical to 1.31 |
| `pl1s` JUNO-106 model / engine | v1.**05** 1 157 772 / 262 160 | v1.**06** 1 157 780 / 262 192 | identical to 1.31 |
| `pl2m` JX-3P model v1.00 / engine | 1 159 484 / 275 492 | content changed / 275 508 | identical to 1.31 |

Reading, with the history:

* **1.30 → 1.31**: the JUNO-106 model version moves 1.05 → 1.06 and every
  engine grows by 16–88 bytes: these are the small fixes (JUNO-106 mono/unison
  note-off, JUPITER-8 arpeggio) implemented in the engines. The JX-3P model
  image changes with the same size.
* **1.31 → 1.32**: all decoded images are byte-identical apart from the build
  stamp. The 1.32 changes (JUPITER-4 support, the freeze fix) are therefore
  in the OS (`App1_Main`) and not in any decodable image. JUPITER-4 is not
  one of the preloaded members.
* The OS file differs in size in every version
  (2 938 080 → 2 938 640 → 2 939 664 bytes) and is encrypted in all three.
