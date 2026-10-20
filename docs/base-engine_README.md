# base-engine

Output of `tools/sys8fw.py` on `firmware/SYSTEM8_UP.BIN` (OS 1.32).
Start with [`BASE_ENGINE.md`](BASE_ENGINE.md).

| Path | Content |
|---|---|
| `BASE_ENGINE.md` | what was obtained and what it contains |
| `info.txt` | `sys8fw info` |
| `up/` | unpacked updater: manifest + decompressed images (+ encrypted OS) |
| `elf-asm/` | ELF per image with symbols, `objdump` listings |
| `report/` | per-image class / preset / label tables, `symbols.json` |

Regenerate: see "Reproduce" in `BASE_ENGINE.md`. The files are derived from
Roland's firmware and are kept for analysis only.
