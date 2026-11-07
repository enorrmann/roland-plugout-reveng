# Volume VII: Clean-Room Archive & Evidence Logs

# Chapter 4: Firmware Reverse Engineering Synthesis & Binary Analysis

> **Document ID**: TRM-BMC-VOL7-004  
> **Classification**: Forensic & Historical Reverse Engineering Report  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 4.1 Master Firmware Image Decomposition (`sys8fw.py`)

Static analysis and extraction of the official Roland firmware update archive (`firmware/sys8_sys_v132/SYSTEM8_UP.BIN`, size 42.4 MB) performed using `tools/sys8fw.py` yields three primary LZSS-compressed executable partitions:

```
+---------------------------------------------------------------------------------------------------+
|                                  SYSTEM8_UP.BIN EXECUTABLE PARTITIONS                             |
|                                                                                                   |
|  Partition Name     Compressed Size    Expanded Size      Target Processor Core & Memory Mapping  |
|  p1y01_p0m.bin      149,902 Bytes      1,166,152 Bytes    Master BMC ARM Core 0 (SDRAM 0x60000000)|
|  p1y01_p0s.bin      314,736 Bytes      536,308 Bytes      Slave BMC ARM Cores (SDRAM 0x60000000)  |
|  p1y01_m0c0a_up.bin 69,248 Bytes       128,000 Bytes      STM32F103 Panel Micro (Flash 0x08000000)|
+---------------------------------------------------------------------------------------------------+
```

---

## 4.2 Analysis of Master OS Partition (`p1y01_p0m.bin`) `[STATIC_ANALYSIS]`

The Master OS executable is mapped into external SDRAM at base physical address **`0x60000000`**:
- **Operating System Kernel**: Standard μT-Kernel 2.0 real-time multi-tasking executive.
- **Task Hierarchy**:
  - `t_gui`: High-level menu navigation and OLED graphics rendering.
  - `t_midi`: Low-jitter MIDI input processing, SysEx message parser, and USB-MIDI router (`0x6000345C`).
  - `t_param`: Real-time synthesizer parameter engine (`0x01000400`), handling group translation, continuous knob smoothing, and patch recall.
  - `t_usb_host`: Low-level USB 2.0 host controller managing communication with the Slave BMC ASICs ("`BMCBulk%d Attached`" endpoints).
- **Floating-Point Utilization**: Compiled for hardware single-precision VFP (`-mcpu=cortex-m4 -mfloat-abi=hard`). Contains 3,054 hardware VFP instructions across the image.

---

## 4.3 Analysis of Slave OS Partition (`p1y01_p0s.bin`) `[STATIC_ANALYSIS]`

During system cold boot, the Master ARM streams the expanded `p1y01_p0s.bin` image across the internal USB host link into each physical Slave BMC:
- **`s_boot.bin` (TCM Base `0x01100000`)**: Low-level hardware initialization code, interrupt vector table, and fixed syscall jump table (`0x0113FC00`).
- **`s_data.bin` (SDRAM Base `0x60000000`)**: Slave runtime voice engine, parameter receiver (`UpdateSynthParam` at `0x01103F2C`), and DSP microcode loader.
- **Diagnostic Telemetry Strings**: Contains strings `BMCInitWaveBus`, `USB Access (DSP %c%c%c%c%c%c)`, `RXDT`, `TXDT`, `RXMIDI`, and `TXMIDI`. Confirms internal USB-MIDI packet reassembly and diagnostic loopback queues.

---

## 4.4 C++ Runtime Type Information (RTTI) Class Catalog

Static analysis of GCC RTTI tables across Master and Slave binaries reveals the internal object-oriented architecture of the synthesizer engine:

| Class Symbol / Type Descriptor | Target Processor | Functional Role |
|---|---|---|
| `CAssignPlugout` | Master ARM Core 0 | Polyphonic note allocator, voice stealing manager, and MIDI CC router. |
| `CDSPSystem8Rev` | Master ARM Core 1 | Global Reverb DSP manager (Controls Master DSP4 slots `0xEFDC`/`0xF7DC`). |
| `CDSPSystem8Dly` | Master ARM Core 1 | Global Stereo Delay and Modulation Chorus DSP manager. |
| `CDSPSystem8Mst` | Master ARM Core 1 | Master Output Limiter, Multi-Band Compressor, and Output Clipping Stage. |
| `CEZUtilDSPDump` | Master ARM Core 0 | Hardware diagnostic tool providing internal DSP register and memory hex dumps. |
