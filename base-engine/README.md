> [!NOTE]
> **TRM Technical Reference Manual**: This implementation example is documented normatively in [Volume VI: Reference Designs & Applications](../docs/vol4_firmware_container_protocol/plugout_virtual_machine.md).

# base-engine

Output of `tools/sys8fw.py` on `firmware/SYSTEM8_UP.BIN` (OS 1.32).
Start with [Vol. IV ch. 8](../docs/vol4_firmware_container_protocol/base_engine.md).

| Path | Content |
|---|---|
| [Vol. IV ch. 8](../docs/vol4_firmware_container_protocol/base_engine.md) | what was obtained and what it contains |
| `info.txt` | `sys8fw info` |
| `up/` | unpacked updater: manifest + decompressed images (+ encrypted OS) |
| `elf-asm/` | ELF per image with symbols, `objdump` listings |
| `report/` | per-image class / preset / label tables, `symbols.json` |

Regenerate: see "Reproduce" in [Vol. IV ch. 8](../docs/vol4_firmware_container_protocol/base_engine.md). The files are derived from
Roland's firmware and are kept for analysis only.
