# Volume VII: Clean-Room Archive & Evidence Logs

# Chapter 3: Clean-Room Test Vectors & System Verification Plan

> **Document ID**: TRM-BMC-VOL7-003  
> **Classification**: Normative Quality Assurance & Verification Plan  
> **Release Target**: Texas Instruments / Analog Devices TRM Standard  

---

## 3.1 Verification Methodology & Golden Vectors

To ensure that independent clean-room implementations (Team B) conform bit-for-bit to Roland hardware protocols without inspecting proprietary binaries, the repository maintains an exhaustive suite of deterministic JSON test vectors located in `cleanroom/test-vectors/`:

```
+---------------------------------------------------------------------------------------------------+
|                                  TEST VECTOR VERIFICATION SUITE                                   |
|                                                                                                   |
|  [ cleanroom/test-vectors/ ]                                                                      |
|  - container-vectors.json       : Container decryption, 128-byte headers, record counts & hashes   |
|  - protocol-vectors.json        : SysEx packet framing, address decoding, Roland checksum vectors|
|  - slot-and-preset-vectors.json : Slot descriptors, parameter walker offsets, patch memory dumps  |
|                                                                                                   |
|  [ Automated Test Runners ]                                                                       |
|  - python3 cleanroom/test-vectors/check_vectors.py                                                |
|  - python3 tools/doccheck.py docs/vol*/*.md                                                       |
+---------------------------------------------------------------------------------------------------+
```

---

## 3.2 Automated Test Execution Suite

### 3.2.1 Container & Protocol Hash Verification (`check_vectors.py`)
Executes standalone verification of container decryptors, checksum generators, and parameter record parsers without requiring physical hardware:

```bash
python3 cleanroom/test-vectors/check_vectors.py
```

- Verifies chained XOR decryption against golden hashes across 4,096-record datasets.
- Tests 7-in-8 packet encoding against known-good MIDI byte arrays.
- Validates Roland 7-bit checksum generation across edge cases ($0, 127, 128, 255$).

### 3.2.2 Microcode Assembler Verification (`doccheck.py`)
Ensures that every assembly listing across all volumes of the TRM compiles cleanly with zero errors:

```bash
python3 tools/doccheck.py docs/vol*/*.md
```

---

## 3.3 Golden Cryptographic Hashes & Reference Constants

| Asset / Vector Name | SHA-256 Checksum | Invariant Characteristic | Validation Rule |
|---|---|---|---|
| `JX-3P Decrypted Header` | `a3f9e...` | Magic `"PLUGOUT_SYS_INF"`, Model `0x0008` | Byte-for-byte exact |
| `JUNO-60 Decrypted Header` | `c108b...` | Magic `"PLUGOUT_SYS_INF"`, Model `0x000A` | Byte-for-byte exact |
| `SH-101 Decrypted Header` | `7e8d2...` | Magic `"PLUGOUT_SYS_INF"`, Model `0x0001` | Byte-for-byte exact |
| `XOR Initial Decryption Key`| — | Constant `0xBACADABE` (32-bit LE) | Exact initial seed |
| `Roland SysEx Model Header` | — | Constant `0x00 0x00 0x7E` | Exact 3-byte model tag |
| `Roland Device Unit ID` | — | Constant `0x10` (Device ID 17) | Default system address |

---

## 3.4 Hardware Verification Plan (Physical Benchmarking)

For live hardware verification sessions on the Roland SYSTEM-8 or SYSTEM-1:

1. **Phase 1: Diagnostic Query Handshake**:
   Send query packet `F0 41 10 00 00 7E 12 70 00 02 00 00 0E F7`. Verify instrument responds with `71 00 02 00` and model byte.
2. **Phase 2: Slot Installation Dry-Run**:
   Install target test package with `--dry-run` to verify record stride and ACK handshake timings without committing flash memory writes.
3. **Phase 3: 96 kHz Audio Telemetry Loopback**:
   Execute diagnostic test tone kernels in target slots, capture 96 kHz stereo audio over USB, and evaluate signal-to-noise ratio and harmonic distribution against host reference models.
