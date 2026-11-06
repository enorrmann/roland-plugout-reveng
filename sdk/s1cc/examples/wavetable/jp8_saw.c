/* jp8_saw.c: a SINGLE-CYCLE wavetable oscillator for SYSTEM-S1CC (SYSTEM-1).
 *
 * Which wavetable plug-out to use (docs/MUTABLE_MEMORY.md §4.0):
 *   - SINGLE-CYCLE / few-frame oscillator  -> the SLAVE, frame FIXED in bank 5 (SL_WT=b5, this file):
 *       one frame (127 points, band-limited to 63 harmonics) written once into bank 5 A080..A0FF, read by every
 *       voice with ldb5 + linear interpolation; OSC1 COLOR picks the frame. 8 voices, no EDM, no note-start fill.
 *       MEASURED 2026-11-06 the most faithful: vs the Jupiter 8 Saw, harmonics within 0.5 dB of an ideal saw and
 *       inter-harmonic floor ~-161 dB (A/B below). Because it is static it has a perfect period.
 *   - LONG / multi-frame morphing wavetable -> the EDM path (EDM_WT, wt_edm_cmod.c, e.g. WT-LINES8):
 *       the whole table (up to ~region-0 size) in the master's external delay memory, morph across all frames.
 *
 * A/B measured on the device (note A3 = 220 Hz, same WAV), fixed bank 5 vs the recirculating bank-0 loop
 * (SL_WT=b0, below): bank 5 = 0.5 dB mean harmonic error, floor -161 dB; bank 0 = 3.7 dB error, floor -91 dB.
 * The bank-0 loop is brighter (768 pts / 384 harmonics) but the ARM refills it in 64-pt blocks every 12 ms, so its
 * period is never stable -> it colors the harmonics and adds broadband noise. Prefer bank 5 for single-cycle.
 *
 * Loaded here with Jupiter 8 Saw.wav (Echo Sound Works Core Tables, 2048/frame, 2 frames). DSP B = filter + AMP
 * envelope of mp_dsp_b.h. Kernel body gives nothing: the SL_WT host code is the oscillator (microcode/sl_wt.s).
 *
 * Build (the WAV path has spaces, so it stays on the command line, not in S1_BUILD):
 *   sdk/s1cc/s1cc build sdk/s1cc/examples/wavetable/jp8_saw.c
 *   WT_WAV="/home/emilio/PRODUCCION/samples/THE_REST/wavetables/Echo Sound Works Core Tables/Analog/Jupiter 8 Saw.wav" \
 *     KERNEL=sdk/s1cc/build/jp8_saw sh examples/system_s1cc/build.sh
 * Install (slot 1; reboot + select the PLUG-OUT on the panel):
 *   python3 tools/plugout_tool.py install plugins/JP8-SAW5 --slot 1 --force --no-patches -y
 *
 * Brighter but measured-worse alternative (recirculating bank-0 loop, 768 pts / 384 harmonics):
 *   add on the build.sh line, before KERNEL=:
 *     NAME=JP8-SAW SL_WT=b0 SL_MORPH=0 SL_L=768 SL_S=0x0040 A_BLOCK0=0x0360 SL_KOUT=0x0390 SL_P=0x03A0 SL_C=0x03B0
 */
#include "../mutable_ported/include/mp_dsp_a.h"
#include "../mutable_ported/include/mp_dsp_b.h"

S1_DSP_A {
  return 0.0f * s1_gate();
}

S1_BUILD("NAME=JP8-SAW5 SL_WT=b5 MIX=0");
S1_COMPILE("--regs 0,1,2,3 --no-fused");
