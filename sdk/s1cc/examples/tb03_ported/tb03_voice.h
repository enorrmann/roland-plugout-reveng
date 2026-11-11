/* tb03_voice.h: the TB-03 voice program (BQ2:Appli 1.07, voice DSP 0x0022, 751 instructions) as C, in two parts
 * (tb03_voice_a: instructions 0..414 on DSP A, tb03_voice_b: 415..750 on DSP B). Written by sdk/s1cc/tools/dsplift.py
 * --bq2 --kernel from the firmware (tools/make_voice.sh); do not edit: README.md §2. */
#include "dsplift_k.h"

/* tb03_voice: written by sdk/s1cc/tools/dsplift.py --kernel --split 415 (exact semantics without the RAW rounding) */
static float D_000_1;   /* part A */
static float D_001_1;   /* part A */
static float D_002_1;   /* part A */
static float D_003_1;   /* part A */
static float D_004_1;   /* part A */
static float D_005_1;   /* part A */
static float D_006_1;   /* part A */
static float D_007_1;   /* part A */
static float D_008_1;   /* part A */
static float D_009_1;   /* part A */
static float D_00a_1;   /* part A */
static float D_00b_1;   /* part A */
static float D_00c_1;   /* part A */
static float D_00d_1;   /* part A */
static float D_00e_1;   /* part A */
static float D_00f_1;   /* part A */
static float D_010_1;   /* part A */
static float D_011_1;   /* part A */
static float D_012_1;   /* part A */
static float D_013_1;   /* part A */
static float D_014_1;   /* part A */
static float D_015_1;   /* part A */
static float D_016_1;   /* part A */
static float D_017_1;   /* part A */
static float D_018_1;   /* part A */
static float D_019_1;   /* part A */
static float D_01a_1;   /* part A */
static float D_01b_1;   /* part A */
static float D_01c_1;   /* part A */
static float D_01d_1;   /* part A */
static float D_01e_1;   /* part A */
static float D_01f_1;   /* part A */
static float D_020_1;   /* part A */
static float D_021_1;   /* part A */
static float D_022_1;   /* part A */
static float D_023_1;   /* part A */
static float D_024_1;   /* part A */
static float D_025_1;   /* part A */
static float D_026_1;   /* part A */
static float D_027_1;   /* part A */
static float D_028_1;   /* part A */
static float D_029_1;   /* part A */
static float D_02a_1;   /* part A */
static float D_02b_1;   /* part A */
static float D_02c_1;   /* part A */
static float D_02d_1;   /* part A */
static float D_02e_1;   /* part A */
static float D_02f_1;   /* part A */
static float D_030_1;   /* part A */
static float D_031_1;   /* part A */
static float D_032_1;   /* part A */
static float D_033_1;   /* part A */
static float D_034_1;   /* part A */
static float D_035_1, D_035_2;   /* part A */
static float D_037_1;   /* part A */
static float D_038_1;   /* part A */
static float D_039_1;   /* part A */
static float D_03a_1;   /* part A */
static float D_03b_1;   /* part A */
static float D_03c_1;   /* part A */
static float D_03d_1;   /* part A */
static float D_03e_1;   /* part A */
static float D_03f_1;   /* part A */
static float D_040_1;   /* part A */
static float D_041_1;   /* part A */
static float D_042_1;   /* part A */
static float D_043_1;   /* part A */
static float D_044_1;   /* part A */
static float D_045_1;   /* part A */
static float D_046_1;   /* part A */
static float D_047_1;   /* part A */
static float D_048_1;   /* part A */
static float D_049_1;   /* part A */
static float D_04a_1;   /* part A */
static float D_04b_1;   /* part A */
static float D_04c_1;   /* part A */
static float D_04d_1;   /* part A */
static float D_04e_1;   /* part A */
static float D_04f_1;   /* part A */
static float D_050_1;   /* part A */
static float D_051_1;   /* part A */
static float D_052_1;   /* part A */
static float D_053_1;   /* part A */
static float D_054_1;   /* part A */
static float D_055_1;   /* part A */
static float D_056_1;   /* part A */
static float D_057_1;   /* part A */
static float D_058_1;   /* part A */
static float D_059_1;   /* part A */
static float D_05a_1;   /* part A */
static float D_05b_1;   /* part A */
static float D_05c_1;   /* part A */
static float D_05d_1;   /* part A */
static float D_05e_1;   /* part A */
static float D_05f_1;   /* part A */
static float D_060_1;   /* part A */
static float D_061_1;   /* part A */
static float D_062_1;   /* part A */
static float D_063_1;   /* part A */
static float D_064_1;   /* part A */
static float D_065_1;   /* part A */
static float D_066_1;   /* part A */
static float D_067_1;   /* part A */
static float D_068_1;   /* part A */
static float D_069_1;   /* part A */
static float D_06a_1;   /* part A */
static float D_06b_1;   /* part A */
static float D_06c_1;   /* part A */
static float D_06d_1;   /* part A */
static float D_06e_1;   /* part A */
static float D_06f_1;   /* part A */
static float D_070_1;   /* part A */
static float D_071_1;   /* part A */
static float D_072_1;   /* part A */
static float D_073_1;   /* part A */
static float D_074_1;   /* part A */
static float D_075_1;   /* part A */
static float D_076_1;   /* part A */
static float D_077_1;   /* part A */
static float D_078_1;   /* part A */
static float D_079_1;   /* part A */
static float D_07a_1;   /* part A */
static float D_07b_1;   /* part A */
static float D_07c_1;   /* part A */
static float D_07d_1;   /* part A */
static float D_07e_1;   /* part A */
static float D_07f_1;   /* part A */
static float D_080_1;   /* part A */
static float D_081_1;   /* part B */
static float D_082_1;   /* part B */
static float D_083_1;   /* part B */
static float D_084_1;   /* part B */
static float D_085_1;   /* part B */
static float D_086_1;   /* part B */
static float D_087_1;   /* part B */
static float D_088_1;   /* part B */
static float D_089_1;   /* part B */
static float D_08a_1;   /* part B */
static float D_08b_1;   /* part B */
static float D_08c_1;   /* part B */
static float D_08d_1;   /* part B */
static float D_08e_1;   /* part B */
static float D_08f_1;   /* part B */
static float D_090_1;   /* part B */
static float D_091_1;   /* part B */
static float D_092_1;   /* part B */
static float D_093_1;   /* part B */
static float D_094_1;   /* part B */
static float D_095_1;   /* part B */
static float D_096_1;   /* part B */
static float D_097_1;   /* part B */
static float D_098_1;   /* part B */
static float D_099_1;   /* part B */
static float D_09a_1;   /* part B */
static float D_09b_1;   /* part B */
static float D_09c_1;   /* part B */
static float D_09d_1;   /* part B */
static float D_09e_1;   /* part B */
static float D_09f_1;   /* part B */
static float D_0a0_1;   /* part B */
static float D_0a1_1;   /* part B */
static float D_0a2_1;   /* part B */
static float D_0a3_1;   /* part B */
static float D_0a4_1;   /* part B */
static float D_0a5_1;   /* part B */
static float D_0a6_1;   /* part B */
static float D_0a7_1;   /* part B */
static float D_0a8_1;   /* part B */
static float D_0a9_1;   /* part B */
static float D_0aa_1;   /* part B */
static float D_0ab_1;   /* part B */
static float D_0ac_1;   /* part B */
static float D_0ad_1;   /* part B */
static float D_0ae_1;   /* part B */
static float D_0af_1;   /* part B */
static float D_0b0_1;   /* part B */
static float D_0b1_1;   /* part B */
static float D_0b2_1;   /* part B */
static float D_0b3_1;   /* part B */
static float D_0b4_1;   /* part B */
static float D_0b5_1;   /* part B */
static float D_0b6_1;   /* part B */
static float D_0b7_1;   /* part B */
static float D_0b8_1;   /* part B */
static float D_0b9_1;   /* part B */
static float D_0ba_1;   /* part B */
static float D_0bb_1;   /* part B */
static float D_0bc_1;   /* part B */
static float D_0bd_1;   /* part B */
static float D_0be_1;   /* part B */
static float D_0bf_1;   /* part B */
static float D_0c0_1;   /* part B */
static float D_0c1_1;   /* part B */
static float D_0c2_1;   /* part B */
static float D_0c3_1;   /* part B */
static float D_0c4_1;   /* part B */
static float D_0c5_1;   /* part B */
static float D_0c6_1;   /* part B */
static float D_0c7_1;   /* part B */
static float D_0c8_1;   /* part B */
static float D_0c9_1;   /* part B */
static float D_0ca_1;   /* part B */
static float D_0cb_1;   /* part B */
static float D_0cc_1;   /* part B */
static float D_0cd_1;   /* part B */
static float D_0ce_1;   /* part B */
static float D_0cf_1;   /* part B */
static float D_0d0_1;   /* part B */
static float D_0d1_1;   /* part B */
static float D_0d2_1;   /* part B */
static float K5_a000;
static float K5_a100;
static float K5_a101;
static float K5_a102;
static float K5_a103;
/* the parts' arguments: what the including file's KIN / KP / KOUT use (default: none) */
#ifndef KARGS_A
#define KARGS_A void
#endif
#ifndef KARGS_B
#define KARGS_B void
#endif
/* handed from part A to part B: s1_export / s1_import slots 0.. */
#define TB03_VOICE_SLOTS 20
static inline __attribute__((always_inline)) void tb03_voice_a(KARGS_A)
{
    float r0 = 0.0f, r1 = 0.0f, r2 = 0.0f, r3 = 0.0f, r4 = 0.0f, r5 = 0.0f, r6 = 0.0f, r7 = 0.0f;
    float f4 = 0.0f;
    float f5 = 0.0f;
    float f7 = 0.0f;
    float f9 = 0.0f;
    float f11 = 0.0f;
    float f17 = 0.0f;
    float pm1 = 0.0f, pv4 = 0.0f, pv5 = 0.0f, pv7 = 0.0f;
    float pm2 = 0.0f, pv9 = 0.0f, pv11 = 0.0f;
    float pm4 = 0.0f, pv17 = 0.0f;
    float c_000 = 0.0f;
    float c_001 = D_000_1;
    float c_002 = D_001_1;
    float c_003 = D_002_1;
    float c_004 = D_003_1;
    float c_005 = D_004_1;
    float c_006 = D_005_1;
    float c_007 = D_006_1;
    float c_008 = D_007_1;
    float c_009 = D_008_1;
    float c_00a = D_009_1;
    float c_00b = D_00a_1;
    float c_00c = D_00b_1;
    float c_00d = D_00c_1;
    float c_00e = D_00d_1;
    float c_00f = D_00e_1;
    float c_010 = D_00f_1;
    float c_011 = D_010_1;
    float c_012 = D_011_1;
    float c_013 = D_012_1;
    float c_014 = D_013_1;
    float c_015 = D_014_1;
    float c_016 = D_015_1;
    float c_017 = D_016_1;
    float c_018 = D_017_1;
    float c_019 = D_018_1;
    float c_01a = D_019_1;
    float c_01b = D_01a_1;
    float c_01c = D_01b_1;
    float c_01d = D_01c_1;
    float c_01e = D_01d_1;
    float c_01f = D_01e_1;
    float c_020 = D_01f_1;
    float c_021 = D_020_1;
    float c_022 = D_021_1;
    float c_023 = D_022_1;
    float c_024 = D_023_1;
    float c_025 = D_024_1;
    float c_026 = D_025_1;
    float c_027 = D_026_1;
    float c_028 = D_027_1;
    float c_029 = D_028_1;
    float c_02a = D_029_1;
    float c_02b = D_02a_1;
    float c_02c = D_02b_1;
    float c_02d = D_02c_1;
    float c_02e = D_02d_1;
    float c_02f = D_02e_1;
    float c_030 = D_02f_1;
    float c_031 = D_030_1;
    float c_032 = D_031_1;
    float c_033 = D_032_1;
    float c_034 = D_033_1;
    float c_035 = D_034_1;
    float c_037 = D_035_2;
    float c_038 = D_037_1;
    float c_039 = D_038_1;
    float c_03a = D_039_1;
    float c_03b = D_03a_1;
    float c_03c = D_03b_1;
    float c_03d = D_03c_1;
    float c_03e = D_03d_1;
    float c_03f = D_03e_1;
    float c_040 = D_03f_1;
    float c_041 = D_040_1;
    float c_042 = D_041_1;
    float c_043 = D_042_1;
    float c_044 = D_043_1;
    float c_045 = D_044_1;
    float c_046 = D_045_1;
    float c_047 = D_046_1;
    float c_048 = D_047_1;
    float c_049 = D_048_1;
    float c_04a = D_049_1;
    float c_04b = D_04a_1;
    float c_04c = D_04b_1;
    float c_04d = D_04c_1;
    float c_04e = D_04d_1;
    float c_04f = D_04e_1;
    float c_050 = D_04f_1;
    float c_051 = D_050_1;
    float c_052 = D_051_1;
    float c_053 = D_052_1;
    float c_054 = D_053_1;
    float c_055 = D_054_1;
    float c_056 = D_055_1;
    float c_057 = D_056_1;
    float c_058 = D_057_1;
    float c_059 = D_058_1;
    float c_05a = D_059_1;
    float c_05b = D_05a_1;
    float c_05c = D_05b_1;
    float c_05d = D_05c_1;
    float c_05e = D_05d_1;
    float c_05f = D_05e_1;
    float c_060 = D_05f_1;
    float c_061 = D_060_1;
    float c_062 = D_061_1;
    float c_063 = D_062_1;
    float c_064 = D_063_1;
    float c_065 = D_064_1;
    float c_066 = D_065_1;
    float c_067 = D_066_1;
    float c_068 = D_067_1;
    float c_069 = D_068_1;
    float c_06a = D_069_1;
    float c_06b = D_06a_1;
    float c_06c = D_06b_1;
    float c_06d = D_06c_1;
    float c_06e = D_06d_1;
    float c_06f = D_06e_1;
    float c_070 = D_06f_1;
    float c_071 = D_070_1;
    float c_072 = D_071_1;
    float c_073 = D_072_1;
    float c_074 = D_073_1;
    float c_075 = D_074_1;
    float c_076 = D_075_1;
    float c_077 = D_076_1;
    float c_078 = D_077_1;
    float c_079 = D_078_1;
    float c_07a = D_079_1;
    float c_07b = D_07a_1;
    float c_07c = D_07b_1;
    float c_07d = D_07c_1;
    float c_07e = D_07d_1;
    float c_07f = D_07e_1;
    float c_080 = D_07f_1;
    float c_0bf = 0.0f /* zero cell */;
    /* 0: 0a02 ff71 */
    {
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd;  }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 1: 0c03 0fe0 a104 */
    {
        float n0 = r0;
          n0 = KIN(0xa104);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 2: 1e05 0fe0 8fe2 a105 0000 */
    {
        float n0 = r0;
          n0 = KIN(0xa105);
        c_000 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 3: 1e05 0fe0 8fe2 a106 0001 */
    {
        float n0 = r0;
          n0 = KIN(0xa106);
        c_001 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 4: 1e05 0fe0 8fe2 a107 0002 */
    {
        float n0 = r0;
          n0 = KIN(0xa107);
        c_002 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 5: 1e05 0fe0 8fe2 a108 0003 */
    {
        float n0 = r0;
          n0 = KIN(0xa108);
        c_003 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 6: 1e05 0fe0 8fe2 a109 0004 */
    {
        float n0 = r0;
          n0 = KIN(0xa109);
        c_004 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 7: 0c03 8fe0 0005 */
    {
        c_005 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 8: 1e05 0fe0 1fe2 c000 c001 */
    {
        float n0 = r0, n1 = r1;
          n0 = KP(0xc000);
          n1 = KP(0xc001);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 9: 1e05 8fe0 9fe2 0006 0007 */
    {
        c_006 = r0;
        c_007 = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 10: 1e05 0fe0 1fe2 c002 c003 */
    {
        float n0 = r0, n1 = r1;
          n0 = KP(0xc002);
          n1 = KP(0xc003);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 11: 3e07 8fe0 9fe2 2fe4 0008 0009 c004 */
    {
        float n2 = r2;
        c_008 = r0;
        c_009 = r1;
          n2 = KP(0xc004);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 12: 0c03 afe0 000a */
    {
        c_00a = r2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 13: 1e05 0fe0 1fe2 0006 0007 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_006;
          n1 = c_007;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 14: 1e05 2fe0 3fe2 0035 0005 */
    {
        float n2 = r2, n3 = r3;
          n2 = c_035;
          n3 = c_005;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 15: 3e07 afe0 5fe2 7fe4 000b 000c a002 */
    {
        float n5 = r5, n7 = r7;
        c_00b = r2;
          n5 = c_00c;
          n7 = KIN(0xa002);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
        r7 = n7;
    }
    /* 16: 1406 6d40 4fe4 a012 b000 000d */
    {
        float n4 = r4, n6 = r6;
        float u16_0v = 0.0f, u16_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (r5) + (-0x1.0000000000000p+0f) + (-r2); u16_0v = rs ? rvv : n6; u16_0z = rs ? rzd : n6; }
          n4 = c_00d;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u16_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u16_0v)); pv7 = (1.0f - fmaxf(s1_sign(u16_0v), 0.0f) * s1_abs(s1_sign(u16_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r6 = n6;
    }
    /* 17: 1503 6d83 2d4a */
    {
        float n2 = r2, n6 = r6;
        float u17_0v = 0.0f, u17_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r6 + r3); u17_0v = rs ? rvv : n6; u17_0z = rs ? rzd : n6; }
        float u17_1v = 0.0f, u17_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r5 + -r2); u17_1v = rs ? rvv : n2; u17_1z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u17_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u17_0v)); pv7 = (1.0f - fmaxf(s1_sign(u17_0v), 0.0f) * s1_abs(s1_sign(u17_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u17_1v)); pv11 = (1.0f - fmaxf(s1_sign(u17_1v), 0.0f) * s1_abs(s1_sign(u17_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r6 = n6;
    }
    /* 18: 0804 1d09 9012 3000 */
    {
        float n1 = r1;
        float u18_0v = 0.0f, u18_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (r4) + (0x1.0000000000000p+0f) + (-r1); u18_0v = rs ? rvv : n1; u18_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u18_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u18_0v)); pv7 = (1.0f - fmaxf(s1_sign(u18_0v), 0.0f) * s1_abs(s1_sign(u18_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 19: 1704 1050 0a10 0000 */
    {
        float n1 = r1;
        float u19_0v = 0.0f, u19_0z = 0.0f;
        { float _w = (1.0f - f5); float o1 = n1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = 0x0.0p+0f; u19_0v = rs ? rvv : n1; u19_0z = rs ? rzd : n1; }
          n1 = o1 + _w * (n1 - o1); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f5); pv4 = s1_abs(s1_sign(u19_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u19_0v)); pv7 = (1.0f - fmaxf(s1_sign(u19_0v), 0.0f) * s1_abs(s1_sign(u19_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 20: 2d05 6047 6050 1212 3000 */
    {
        float n6 = r6;
        float u20_0v = 0.0f, u20_0z = 0.0f;
        { float _w = (1.0f - f9); float o6 = n6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = r7; u20_0v = rs ? rvv : n6; u20_0z = rs ? rzd : n6; }
          n6 = o6 + _w * (n6 - o6); }
        float u20_1v = 0.0f, u20_1z = 0.0f;
        { float _w = f9; float o6 = n6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = 0x1.0000000000000p+0f; u20_1v = rs ? rvv : n6; u20_1z = rs ? rzd : n6; }
          n6 = o6 + _w * (n6 - o6); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f9); pv4 = s1_abs(s1_sign(u20_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u20_0v)); pv7 = (1.0f - fmaxf(s1_sign(u20_0v), 0.0f) * s1_abs(s1_sign(u20_0z)));
        pm2 = f9; pv9 = fminf(1.0f, 1.0f + s1_sign(u20_1v)); pv11 = (1.0f - fmaxf(s1_sign(u20_1v), 0.0f) * s1_abs(s1_sign(u20_1z)));
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 21: 3a06 ded9 1fe0 4fe2 000f 0011 */
    {
        float n1 = r1, n4 = r4, n5 = r5;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = pymax(-1.0f, pymin(1.0f, r1)); }
          n1 = c_00f;
          n4 = c_011;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r4 = n4;
        r5 = n5;
    }
    /* 22: 2e05 0c40 404c dfe0 000c */
    {
        float n0 = r0, n4 = r4;
        float u22_0v = 0.0f, u22_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r1 + r0); u22_0v = rs ? rvv : n0; u22_0z = rs ? rzd : n0; }
        float u22_1v = 0.0f, u22_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = -r4; u22_1v = rs ? rvv : n4; u22_1z = rs ? rzd : n4; }
        c_00c = r5;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u22_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u22_0v)); pv7 = (1.0f - fmaxf(s1_sign(u22_0v), 0.0f) * s1_abs(s1_sign(u22_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u22_1v)); pv11 = (1.0f - fmaxf(s1_sign(u22_1v), 0.0f) * s1_abs(s1_sign(u22_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r4 = n4;
    }
    /* 23: 0902 0403 */
    {
        float n0 = r0;
        float u23_0v = 0.0f, u23_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r3); u23_0v = rs ? rvv : n0; u23_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u23_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u23_0v)); pv7 = (1.0f - fmaxf(s1_sign(u23_0v), 0.0f) * s1_abs(s1_sign(u23_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 24: 0902 0406 */
    {
        float n0 = r0;
        float u24_0v = 0.0f, u24_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r6); u24_0v = rs ? rvv : n0; u24_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u24_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u24_0v)); pv7 = (1.0f - fmaxf(s1_sign(u24_0v), 0.0f) * s1_abs(s1_sign(u24_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 25: 6207 1c50 8ee0 cef4 5fe2 0000 0013 */
    {
        float n0 = r0, n1 = r1, n4 = r4, n5 = r5;
        float u25_0v = 0.0f, u25_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 + 0x0.0p+0f); u25_0v = rs ? rvv : n1; u25_0z = rs ? rzd : n1; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = wrap1(r0); }
        float u25_2v = 0.0f, u25_2z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = pymod(r4, 2) / 2; u25_2v = rs ? rvv : n4; u25_2z = rs ? rzd : n4; }
          n5 = c_013;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u25_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u25_0v)); pv7 = (1.0f - fmaxf(s1_sign(u25_0v), 0.0f) * s1_abs(s1_sign(u25_0z)));
        pm2 = 0.0f;
        pm4 = 1.0f; pv17 = fminf(1.0f, 1.0f + s1_sign(u25_2v));
        r0 = n0;
        r1 = n1;
        r4 = n4;
        r5 = n5;
    }
    /* 26: 3407 1c10 cfe2 8fe4 b000 0010 000e */
    {
        float n1 = r1;
        float u26_0v = 0.0f, u26_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r0 + -0x1.0000000000000p+0f); u26_0v = rs ? rvv : n1; u26_0z = rs ? rzd : n1; }
        c_010 = r4;
        c_00e = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u26_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u26_0v)); pv7 = (1.0f - fmaxf(s1_sign(u26_0v), 0.0f) * s1_abs(s1_sign(u26_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 27: 3204 5044 2310 0a20 */
    {
        float n5 = r5;
        float u27_0v = 0.0f, u27_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f17)); float o5 = n5;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = r4; u27_0v = rs ? rvv : n5; u27_0z = rs ? rzd : n5; }
          n5 = o5 + _w * (n5 - o5); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f17)); pv4 = s1_abs(s1_sign(u27_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u27_0v)); pv7 = (1.0f - fmaxf(s1_sign(u27_0v), 0.0f) * s1_abs(s1_sign(u27_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
    }
    /* 28: 1b04 9ee1 dfe0 0012 */
    {
        float n1 = r1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = wrap1(r1); }
        c_012 = r5;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 29: 3106 2c10 9f29 9fe2 a800 0014 */
    {
        float n1 = r1, n2 = r2;
        float u29_0v = 0.0f, u29_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r0 + -0x1.0000000000000p-1f); u29_0v = rs ? rvv : n2; u29_0z = rs ? rzd : n2; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = s1_sign(r1); }
        c_014 = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u29_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u29_0v)); pv7 = (1.0f - fmaxf(s1_sign(u29_0v), 0.0f) * s1_abs(s1_sign(u29_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
    }
    /* 30: 1b04 aee2 9fe0 0015 */
    {
        float n2 = r2;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = wrap1(r2); }
        c_015 = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 31: 1605 34a0 aeea 4324 3f00 */
    {
        float n2 = r2, n3 = r3;
        float u31_0v = 0.0f, u31_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r2 * 0x1.921f800000000p+1f); u31_0v = rs ? rvv : n3; u31_0z = rs ? rzd : n3; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = 1 - 2 * fabs(wrap1(-r2 - 1.5f)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u31_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u31_0v)); pv7 = (1.0f - fmaxf(s1_sign(u31_0v), 0.0f) * s1_abs(s1_sign(u31_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 32: 1804 44c3 afe0 0016 */
    {
        float n4 = r4;
        float u32_0v = 0.0f, u32_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r3 * r3); u32_0v = rs ? rvv : n4; u32_0z = rs ? rzd : n4; }
        c_016 = r2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u32_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u32_0v)); pv7 = (1.0f - fmaxf(s1_sign(u32_0v), 0.0f) * s1_abs(s1_sign(u32_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 33: 1107 2522 5503 0025 0000 0000 002e */
    {
        float n2 = r2, n5 = r5;
        float u33_0v = 0.0f, u33_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (0x0.0p+0f * r4) + (0x1.7000000000000p-19f); u33_0v = rs ? rvv : n2; u33_0z = rs ? rzd : n2; }
        float u33_1v = 0.0f, u33_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r4 * r3); u33_1v = rs ? rvv : n5; u33_1z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u33_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u33_0v)); pv7 = (1.0f - fmaxf(s1_sign(u33_0v), 0.0f) * s1_abs(s1_sign(u33_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u33_1v)); pv11 = (1.0f - fmaxf(s1_sign(u33_1v), 0.0f) * s1_abs(s1_sign(u33_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r5 = n5;
    }
    /* 34: 1107 7522 6504 0025 800d 0002 2222 */
    {
        float n6 = r6, n7 = r7;
        float u34_0v = 0.0f, u34_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (-0x1.a000000000000p-13f * r4) + (0x1.1111000000000p-7f); u34_0v = rs ? rvv : n7; u34_0z = rs ? rzd : n7; }
        float u34_1v = 0.0f, u34_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r4 * r4); u34_1v = rs ? rvv : n6; u34_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u34_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u34_0v)); pv7 = (1.0f - fmaxf(s1_sign(u34_0v), 0.0f) * s1_abs(s1_sign(u34_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u34_1v)); pv11 = (1.0f - fmaxf(s1_sign(u34_1v), 0.0f) * s1_abs(s1_sign(u34_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 35: 1503 2987 7505 */
    {
        float n2 = r2, n7 = r7;
        float u35_0v = 0.0f, u35_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r7 + r6 * r2); u35_0v = rs ? rvv : n2; u35_0z = rs ? rzd : n2; }
        float u35_1v = 0.0f, u35_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r4 * r5); u35_1v = rs ? rvv : n7; u35_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u35_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u35_0v)); pv7 = (1.0f - fmaxf(s1_sign(u35_0v), 0.0f) * s1_abs(s1_sign(u35_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u35_1v)); pv11 = (1.0f - fmaxf(s1_sign(u35_1v), 0.0f) * s1_abs(s1_sign(u35_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r7 = n7;
    }
    /* 36: 2b09 2c8f 0fe2 1fe4 5983 0014 0015 a2aa aa00 */
    {
        float n0 = r0, n1 = r1, n2 = r2;
        float u36_0v = 0.0f, u36_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (r7 * r2) + (r3) + (-0x1.5555000000000p-3f * r5); u36_0v = rs ? rvv : n2; u36_0z = rs ? rzd : n2; }
          n0 = c_014;
          n1 = c_015;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u36_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u36_0v)); pv7 = (1.0f - fmaxf(s1_sign(u36_0v), 0.0f) * s1_abs(s1_sign(u36_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
        r2 = n2;
    }
    /* 37: 470d 0414 1458 afec 3fee 0016 001a 2800 2800 2800 2800 0017 0016 */
    {
        float n0 = r0, n1 = r1, n3 = r3;
        float u37_0v = 0.0f, u37_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (0x1.0000000000000p-1f * r0) + (0x1.0000000000000p-1f); u37_0v = rs ? rvv : n0; u37_0z = rs ? rzd : n0; }
        float u37_1v = 0.0f, u37_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (0x1.0000000000000p-1f * r1) + (0x1.0000000000000p-1f); u37_1v = rs ? rvv : n1; u37_1z = rs ? rzd : n1; }
        c_017 = r2;
          n3 = c_016;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u37_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u37_0v)); pv7 = (1.0f - fmaxf(s1_sign(u37_0v), 0.0f) * s1_abs(s1_sign(u37_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u37_1v)); pv11 = (1.0f - fmaxf(s1_sign(u37_1v), 0.0f) * s1_abs(s1_sign(u37_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
        r3 = n3;
    }
    /* 38: 470d 2494 34d8 4fec 5fee 0016 001a 2800 2800 2800 2800 0010 0012 */
    {
        float n2 = r2, n3 = r3, n4 = r4, n5 = r5;
        float u38_0v = 0.0f, u38_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (0x1.0000000000000p-1f * r2) + (0x1.0000000000000p-1f); u38_0v = rs ? rvv : n2; u38_0z = rs ? rzd : n2; }
        float u38_1v = 0.0f, u38_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (0x1.0000000000000p-1f * r3) + (0x1.0000000000000p-1f); u38_1v = rs ? rvv : n3; u38_1z = rs ? rzd : n3; }
          n4 = c_010;
          n5 = c_012;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u38_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u38_0v)); pv7 = (1.0f - fmaxf(s1_sign(u38_0v), 0.0f) * s1_abs(s1_sign(u38_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u38_1v)); pv11 = (1.0f - fmaxf(s1_sign(u38_1v), 0.0f) * s1_abs(s1_sign(u38_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
        r4 = n4;
        r5 = n5;
    }
    /* 39: 470d 4514 5558 6fec 7fee 0016 001a 2800 2800 2800 2800 c005 c006 */
    {
        float n4 = r4, n5 = r5, n6 = r6, n7 = r7;
        float u39_0v = 0.0f, u39_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (0x1.0000000000000p-1f * r4) + (0x1.0000000000000p-1f); u39_0v = rs ? rvv : n4; u39_0z = rs ? rzd : n4; }
        float u39_1v = 0.0f, u39_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (0x1.0000000000000p-1f * r5) + (0x1.0000000000000p-1f); u39_1v = rs ? rvv : n5; u39_1z = rs ? rzd : n5; }
          n6 = KP(0xc005);
          n7 = KP(0xc006);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u39_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u39_0v)); pv7 = (1.0f - fmaxf(s1_sign(u39_0v), 0.0f) * s1_abs(s1_sign(u39_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u39_1v)); pv11 = (1.0f - fmaxf(s1_sign(u39_1v), 0.0f) * s1_abs(s1_sign(u39_1z)));
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 40: 2b07 080e 6fe2 7fe4 13c0 c008 c007 */
    {
        float n0 = r0, n6 = r6, n7 = r7;
        float u40_0v = 0.0f, u40_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r6 * r0) + (r7 * r1); u40_0v = rs ? rvv : n0; u40_0z = rs ? rzd : n0; }
          n6 = KP(0xc008);
          n7 = KP(0xc007);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u40_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u40_0v)); pv7 = (1.0f - fmaxf(s1_sign(u40_0v), 0.0f) * s1_abs(s1_sign(u40_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r6 = n6;
        r7 = n7;
    }
    /* 41: 2b07 0c8e 6fe2 7fe4 33c0 c009 c00a */
    {
        float n0 = r0, n6 = r6, n7 = r7;
        float u41_0v = 0.0f, u41_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r6 * r2) + (r0) + (r7 * r3); u41_0v = rs ? rvv : n0; u41_0z = rs ? rzd : n0; }
          n6 = KP(0xc009);
          n7 = KP(0xc00a);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u41_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u41_0v)); pv7 = (1.0f - fmaxf(s1_sign(u41_0v), 0.0f) * s1_abs(s1_sign(u41_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r6 = n6;
        r7 = n7;
    }
    /* 42: 0803 0d0e 53c0 */
    {
        float n0 = r0;
        float u42_0v = 0.0f, u42_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r6 * r4) + (r0) + (r7 * r5); u42_0v = rs ? rvv : n0; u42_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u42_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u42_0v)); pv7 = (1.0f - fmaxf(s1_sign(u42_0v), 0.0f) * s1_abs(s1_sign(u42_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 43: 1b04 8ed8 1fe0 000c */
    {
        float n0 = r0, n1 = r1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = pymax(-1.0f, pymin(1.0f, r0)); }
          n1 = c_00c;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 44: 0902 0401 */
    {
        float n0 = r0;
        float u44_0v = 0.0f, u44_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r1); u44_0v = rs ? rvv : n0; u44_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u44_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u44_0v)); pv7 = (1.0f - fmaxf(s1_sign(u44_0v), 0.0f) * s1_abs(s1_sign(u44_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 45: 0c03 8fe0 0018 */
    {
        c_018 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 46: 0c03 2fe0 001a */
    {
        float n2 = r2;
          n2 = c_01a;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 47: 0805 0c12 a502 00e3 00e3 */
    {
        float n0 = r0;
        float u47_0v = 0.0f, u47_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (0x1.c600000000000p-9f * r0) + (r2) + (0x1.c600000000000p-9f * -r2); u47_0v = rs ? rvv : n0; u47_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u47_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u47_0v)); pv7 = (1.0f - fmaxf(s1_sign(u47_0v), 0.0f) * s1_abs(s1_sign(u47_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 48: 0a03 8410 0000 */
    {
        float n0 = r0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = pymax(r0, 0x0.0p+0f); RESZ(n0, (r0) - (0x0.0p+0f)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 49: 0c03 8fe0 0019 */
    {
        c_019 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 50: 1e05 0fe0 1fe2 0018 0008 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_018;
          n1 = c_008;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 51: 0902 0401 */
    {
        float n0 = r0;
        float u51_0v = 0.0f, u51_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r1); u51_0v = rs ? rvv : n0; u51_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u51_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u51_0v)); pv7 = (1.0f - fmaxf(s1_sign(u51_0v), 0.0f) * s1_abs(s1_sign(u51_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 52: 0c03 8fe0 001b */
    {
        c_01b = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 53: 1e05 0fe0 1fe2 0018 0009 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_018;
          n1 = c_009;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 54: 0902 0401 */
    {
        float n0 = r0;
        float u54_0v = 0.0f, u54_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r1); u54_0v = rs ? rvv : n0; u54_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u54_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u54_0v)); pv7 = (1.0f - fmaxf(s1_sign(u54_0v), 0.0f) * s1_abs(s1_sign(u54_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 55: 0c03 8fe0 001c */
    {
        c_01c = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 56: 1e05 0fe0 1fe2 0019 000a */
    {
        float n0 = r0, n1 = r1;
          n0 = c_019;
          n1 = c_00a;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 57: 0902 0401 */
    {
        float n0 = r0;
        float u57_0v = 0.0f, u57_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r1); u57_0v = rs ? rvv : n0; u57_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u57_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u57_0v)); pv7 = (1.0f - fmaxf(s1_sign(u57_0v), 0.0f) * s1_abs(s1_sign(u57_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 58: 0c03 8fe0 001d */
    {
        c_01d = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 59: 0c03 0fe0 c00b */
    {
        float n0 = r0;
          n0 = KP(0xc00b);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 60: 0c03 8fe0 001e */
    {
        c_01e = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 61: 1e05 0fe0 1fe2 c00c c00d */
    {
        float n0 = r0, n1 = r1;
          n0 = KP(0xc00c);
          n1 = KP(0xc00d);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 62: 7809 8fe0 9fe2 2fe4 3fe6 001f 0020 c00e c00f */
    {
        float n2 = r2, n3 = r3;
        c_01f = r0;
        c_020 = r1;
          n2 = KP(0xc00e);
          n3 = KP(0xc00f);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 63: 1e05 afe0 bfe2 0021 0022 */
    {
        c_021 = r2;
        c_022 = r3;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 64: 0c03 0fe0 a003 */
    {
        float n0 = r0;
          n0 = KIN(0xa003);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 65: 0c03 8fe0 0023 */
    {
        c_023 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 66: 0c03 0fe0 a004 */
    {
        float n0 = r0;
          n0 = KIN(0xa004);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 67: 1e05 8fe0 1fe2 0024 a005 */
    {
        float n1 = r1;
        c_024 = r0;
          n1 = KIN(0xa005);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 68: 1e05 9fe0 2fe2 0025 a006 */
    {
        float n2 = r2;
        c_025 = r1;
          n2 = KIN(0xa006);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 69: 1e05 afe0 3fe2 0026 a007 */
    {
        float n3 = r3;
        c_026 = r2;
          n3 = KIN(0xa007);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 70: 0c03 bfe0 0027 */
    {
        c_027 = r3;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 71: 0904 0060 22fc 0700 */
    {
        float n0 = r0;
        float u71_0v = 0.0f, u71_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = 0x1.7e03800000000p-3f; u71_0v = rs ? rvv : n0; u71_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u71_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u71_0v)); pv7 = (1.0f - fmaxf(s1_sign(u71_0v), 0.0f) * s1_abs(s1_sign(u71_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 72: 0c03 8fe0 0028 */
    {
        c_028 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 73: 1e05 0fe0 1fe2 0024 001e */
    {
        float n0 = r0, n1 = r1;
          n0 = c_024;
          n1 = c_01e;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 74: 4e0d 3452 7c7c 4fe4 5fe6 0038 7000 002a 002b 0000 0200 0000 0002 */
    {
        float n3 = r3, n4 = r4, n5 = r5, n7 = r7;
        float u74_0v = 0.0f, u74_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (0x1.0000000000000p+8f * r1) + (0x1.0000000000000p-23f); u74_0v = rs ? rvv : n3; u74_0z = rs ? rzd : n3; }
        float u74_1v = 0.0f, u74_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r1 + 0x1.0000000000000p-31f); u74_1v = rs ? rvv : n7; u74_1z = rs ? rzd : n7; }
          n4 = c_02a;
          n5 = c_02b;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u74_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u74_0v)); pv7 = (1.0f - fmaxf(s1_sign(u74_0v), 0.0f) * s1_abs(s1_sign(u74_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u74_1v)); pv11 = (1.0f - fmaxf(s1_sign(u74_1v), 0.0f) * s1_abs(s1_sign(u74_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r4 = n4;
        r5 = n5;
        r7 = n7;
    }
    /* 75: 0a02 9f03 */
    {
        float n1 = r1, n2 = r2, n3 = r3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = s1_sign(r3) * s1_recip(fmaxf(s1_abs(r3), 0x1p-9f)); n2 = 1.0f; n3 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
        r3 = n3;
    }
    /* 76: 0902 1083 */
    {
        float n1 = r1, n2 = r2, n3 = r3;
        float u76_0v = 0.0f, u76_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = r1 * r2; n2 = 2 - r2 * r3; n3 = r2 * r3; u76_0v = rs ? rvv : n1; u76_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u76_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u76_0v)); pv7 = (1.0f - fmaxf(s1_sign(u76_0v), 0.0f) * s1_abs(s1_sign(u76_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
        r3 = n3;
    }
    /* 77: 0902 1083 */
    {
        float n1 = r1, n2 = r2, n3 = r3;
        float u77_0v = 0.0f, u77_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = r1 * r2; n2 = 2 - r2 * r3; n3 = r2 * r3; u77_0v = rs ? rvv : n1; u77_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u77_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u77_0v)); pv7 = (1.0f - fmaxf(s1_sign(u77_0v), 0.0f) * s1_abs(s1_sign(u77_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
        r3 = n3;
    }
    /* 78: 0902 1083 */
    {
        float n1 = r1, n2 = r2, n3 = r3;
        float u78_0v = 0.0f, u78_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = r1 * r2; n2 = 2 - r2 * r3; n3 = r2 * r3; u78_0v = rs ? rvv : n1; u78_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u78_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u78_0v)); pv7 = (1.0f - fmaxf(s1_sign(u78_0v), 0.0f) * s1_abs(s1_sign(u78_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
        r3 = n3;
    }
    /* 79: 0902 5549 */
    {
        float n5 = r5;
        float u79_0v = 0.0f, u79_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r5 * -r1); u79_0v = rs ? rvv : n5; u79_0z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u79_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u79_0v)); pv7 = (1.0f - fmaxf(s1_sign(u79_0v), 0.0f) * s1_abs(s1_sign(u79_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
    }
    /* 80: 1406 5552 6fe4 0004 7000 002c */
    {
        float n5 = r5, n6 = r6;
        float u80_0v = 0.0f, u80_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (0x1.0000000000000p+8f * r5) + (r4); u80_0v = rs ? rvv : n5; u80_0z = rs ? rzd : n5; }
          n6 = c_02c;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u80_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u80_0v)); pv7 = (1.0f - fmaxf(s1_sign(u80_0v), 0.0f) * s1_abs(s1_sign(u80_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
        r6 = n6;
    }
    /* 81: 0902 814e */
    {
        float n0 = r0;
        float u81_0v = 0.0f, u81_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = r5 + -r6 + r0; RES(n0); u81_0v = rs ? rvv : n0; u81_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u81_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u81_0v)); pv7 = (1.0f - fmaxf(s1_sign(u81_0v), 0.0f) * s1_abs(s1_sign(u81_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 82: 1804 0407 8fe0 0029 */
    {
        float n0 = r0;
        float u82_0v = 0.0f, u82_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r7); u82_0v = rs ? rvv : n0; u82_0z = rs ? rzd : n0; }
        c_029 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u82_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u82_0v)); pv7 = (1.0f - fmaxf(s1_sign(u82_0v), 0.0f) * s1_abs(s1_sign(u82_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 83: 1804 0c06 8fe0 002a */
    {
        float n0 = r0;
        float u83_0v = 0.0f, u83_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + r6); u83_0v = rs ? rvv : n0; u83_0z = rs ? rzd : n0; }
        c_02a = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u83_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u83_0v)); pv7 = (1.0f - fmaxf(s1_sign(u83_0v), 0.0f) * s1_abs(s1_sign(u83_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 84: 0c03 8fe0 002b */
    {
        c_02b = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 85: 0c03 0fe0 0000 */
    {
        float n0 = r0;
          n0 = c_000;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 86: 3408 7074 4fe0 5fe2 002e 002f 001f 07c1 */
    {
        float n4 = r4, n5 = r5, n7 = r7;
        float u86_0v = 0.0f, u86_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.f07c100000000p-12f; u86_0v = rs ? rvv : n7; u86_0z = rs ? rzd : n7; }
          n4 = c_02e;
          n5 = c_02f;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u86_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u86_0v)); pv7 = (1.0f - fmaxf(s1_sign(u86_0v), 0.0f) * s1_abs(s1_sign(u86_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r7 = n7;
    }
    /* 87: 0904 5570 c840 0001 */
    {
        float n5 = r5;
        float u87_0v = 0.0f, u87_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r5 * -0x1.0800002000000p+3f); u87_0v = rs ? rvv : n5; u87_0z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u87_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u87_0v)); pv7 = (1.0f - fmaxf(s1_sign(u87_0v), 0.0f) * s1_abs(s1_sign(u87_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
    }
    /* 88: 1407 5574 6fe2 0004 0030 7000 0000 */
    {
        float n5 = r5, n6 = r6;
        float u88_0v = 0.0f, u88_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (0x1.0000000000000p+8f * r5) + (r4); u88_0v = rs ? rvv : n5; u88_0z = rs ? rzd : n5; }
          n6 = c_030;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u88_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u88_0v)); pv7 = (1.0f - fmaxf(s1_sign(u88_0v), 0.0f) * s1_abs(s1_sign(u88_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
        r6 = n6;
    }
    /* 89: 0902 814e */
    {
        float n0 = r0;
        float u89_0v = 0.0f, u89_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = r5 + -r6 + r0; RES(n0); u89_0v = rs ? rvv : n0; u89_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u89_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u89_0v)); pv7 = (1.0f - fmaxf(s1_sign(u89_0v), 0.0f) * s1_abs(s1_sign(u89_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 90: 1804 0407 8fe0 002d */
    {
        float n0 = r0;
        float u90_0v = 0.0f, u90_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r7); u90_0v = rs ? rvv : n0; u90_0z = rs ? rzd : n0; }
        c_02d = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u90_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u90_0v)); pv7 = (1.0f - fmaxf(s1_sign(u90_0v), 0.0f) * s1_abs(s1_sign(u90_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 91: 1804 0c06 8fe0 002e */
    {
        float n0 = r0;
        float u91_0v = 0.0f, u91_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + r6); u91_0v = rs ? rvv : n0; u91_0z = rs ? rzd : n0; }
        c_02e = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u91_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u91_0v)); pv7 = (1.0f - fmaxf(s1_sign(u91_0v), 0.0f) * s1_abs(s1_sign(u91_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 92: 0c03 8fe0 002f */
    {
        c_02f = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 93: 0c03 0fe0 0001 */
    {
        float n0 = r0;
          n0 = c_001;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 94: 1e05 8fe0 2fe2 0031 0032 */
    {
        float n2 = r2;
        c_031 = r0;
          n2 = c_032;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 95: 0902 2c0a */
    {
        float n2 = r2;
        float u95_0v = 0.0f, u95_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r0 + -r2); u95_0v = rs ? rvv : n2; u95_0z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u95_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u95_0v)); pv7 = (1.0f - fmaxf(s1_sign(u95_0v), 0.0f) * s1_abs(s1_sign(u95_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 96: 0001 */
    {
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 97: 1c03 00c1 0e10 */
    {
        /* control word c1: no effect */
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 98: 1c03 00c2 0a10 */
    {
        /* control word c2: no effect */
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 99: 1e05 0fe0 1fe2 002b 0024 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_02b;
          n1 = c_024;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 100: 1e05 2fe0 3fe2 0027 002f */
    {
        float n2 = r2, n3 = r3;
          n2 = c_027;
          n3 = c_02f;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 101: 1e05 4fe0 5fe2 0000 0003 */
    {
        float n4 = r4, n5 = r5;
          n4 = c_000;
          n5 = c_003;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
    }
    /* 102: 0c03 6fe0 0023 */
    {
        float n6 = r6;
          n6 = c_023;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 103: 0803 0c0a 9281 */
    {
        float n0 = r0;
        float u103_0v = 0.0f, u103_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r2 * r0) + (r1) + (r2 * -r1); u103_0v = rs ? rvv : n0; u103_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u103_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u103_0v)); pv7 = (1.0f - fmaxf(s1_sign(u103_0v), 0.0f) * s1_abs(s1_sign(u103_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 104: 0803 3ccd c344 */
    {
        float n3 = r3;
        float u104_0v = 0.0f, u104_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r5 * r3) + (r4) + (r5 * -r4); u104_0v = rs ? rvv : n3; u104_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u104_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u104_0v)); pv7 = (1.0f - fmaxf(s1_sign(u104_0v), 0.0f) * s1_abs(s1_sign(u104_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 105: 2b07 0c0e 1fe2 2fe4 b383 c010 c011 */
    {
        float n0 = r0, n1 = r1, n2 = r2;
        float u105_0v = 0.0f, u105_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r6 * r0) + (r3) + (r6 * -r3); u105_0v = rs ? rvv : n0; u105_0z = rs ? rzd : n0; }
          n1 = KP(0xc010);
          n2 = KP(0xc011);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u105_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u105_0v)); pv7 = (1.0f - fmaxf(s1_sign(u105_0v), 0.0f) * s1_abs(s1_sign(u105_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
        r2 = n2;
    }
    /* 106: 0902 8042 */
    {
        float n0 = r0;
        float u106_0v = 0.0f, u106_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = r1 + r2 + r0; RES(n0); u106_0v = rs ? rvv : n0; u106_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u106_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u106_0v)); pv7 = (1.0f - fmaxf(s1_sign(u106_0v), 0.0f) * s1_abs(s1_sign(u106_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 107: 0c03 8fe0 0033 */
    {
        c_033 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 108: 1e05 0fe0 1fe2 0025 0001 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_025;
          n1 = c_001;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 109: 0c03 2fe0 0023 */
    {
        float n2 = r2;
          n2 = c_023;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 110: 0803 0c0a 9281 */
    {
        float n0 = r0;
        float u110_0v = 0.0f, u110_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r2 * r0) + (r1) + (r2 * -r1); u110_0v = rs ? rvv : n0; u110_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u110_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u110_0v)); pv7 = (1.0f - fmaxf(s1_sign(u110_0v), 0.0f) * s1_abs(s1_sign(u110_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 111: 0c03 8fe0 0034 */
    {
        c_034 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 112: 0c03 8fe0 0035 */
    {
        c_035 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 113: 0c03 5fe0 0040 */
    {
        float n5 = r5;
          n5 = c_040;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
    }
    /* 114: 3406 5c0d 6fe0 2fe2 0038 003e */
    {
        float n2 = r2, n5 = r5, n6 = r6;
        float u114_0v = 0.0f, u114_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r0 + -r5); u114_0v = rs ? rvv : n5; u114_0z = rs ? rzd : n5; }
          n6 = c_038;
          n2 = c_03e;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u114_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u114_0v)); pv7 = (1.0f - fmaxf(s1_sign(u114_0v), 0.0f) * s1_abs(s1_sign(u114_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r5 = n5;
        r6 = n6;
    }
    /* 115: b409 3c0e 204a d550 8fe2 1fe4 0000 003f 003c */
    {
        float n1 = r1, n2 = r2, n3 = r3, n5 = r5;
        float u115_0v = 0.0f, u115_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r0 + -r6); u115_0v = rs ? rvv : n3; u115_0z = rs ? rzd : n3; }
        float u115_1v = 0.0f, u115_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = -r2; u115_1v = rs ? rvv : n2; u115_1z = rs ? rzd : n2; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = pymax(r5, 0x0.0p+0f); RESZ(n5, (r5) - (0x0.0p+0f)); }
        c_03f = r0;
          n1 = c_03c;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u115_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u115_0v)); pv7 = (1.0f - fmaxf(s1_sign(u115_0v), 0.0f) * s1_abs(s1_sign(u115_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u115_1v)); pv11 = (1.0f - fmaxf(s1_sign(u115_1v), 0.0f) * s1_abs(s1_sign(u115_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
        r3 = n3;
        r5 = n5;
    }
    /* 116: 2806 5c4d aef2 1fe2 e346 c012 */
    {
        float n1 = r1, n2 = r2, n5 = r5;
        float u116_0v = 0.0f, u116_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (r5 * r1) + (r6) + (r5 * -r6); u116_0v = rs ? rvv : n5; u116_0z = rs ? rzd : n5; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = pymod(r2, 2) / 2; }
          n1 = KP(0xc012);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u116_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u116_0v)); pv7 = (1.0f - fmaxf(s1_sign(u116_0v), 0.0f) * s1_abs(s1_sign(u116_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
        r5 = n5;
    }
    /* 117: 1304 0d49 0e10 8240 */
    {
        float n0 = r0;
        float u117_0v = 0.0f, u117_0z = 0.0f;
        { float _w = (1.0f - f7); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r1 * r5) + (r0) + (r1 * -r0); u117_0v = rs ? rvv : n0; u117_0z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f7); pv4 = s1_abs(s1_sign(u117_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u117_0v)); pv7 = (1.0f - fmaxf(s1_sign(u117_0v), 0.0f) * s1_abs(s1_sign(u117_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 118: 4e0b 2492 5416 8fe8 afea 0014 2800 2800 3199 0037 003d */
    {
        float n2 = r2, n5 = r5;
        float u118_0v = 0.0f, u118_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (0x1.0000000000000p-1f * r2) + (0x1.0000000000000p-1f); u118_0v = rs ? rvv : n2; u118_0z = rs ? rzd : n2; }
        float u118_1v = 0.0f, u118_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r0 * 0x1.1990000000000p+0f); u118_1v = rs ? rvv : n5; u118_1z = rs ? rzd : n5; }
        c_037 = r0;
        c_03d = r2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u118_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u118_0v)); pv7 = (1.0f - fmaxf(s1_sign(u118_0v), 0.0f) * s1_abs(s1_sign(u118_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u118_1v)); pv11 = (1.0f - fmaxf(s1_sign(u118_1v), 0.0f) * s1_abs(s1_sign(u118_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r5 = n5;
    }
    /* 119: 5f0a 3d64 7d67 dfe0 6fe2 0038 003a b000 0099 9999 */
    {
        float n3 = r3, n6 = r6, n7 = r7;
        float u119_0v = 0.0f, u119_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r5 + -0x1.0000000000000p+0f); u119_0v = rs ? rvv : n3; u119_0z = rs ? rzd : n3; }
        float u119_1v = 0.0f, u119_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r5 + -0x1.9999900000000p-4f); u119_1v = rs ? rvv : n7; u119_1z = rs ? rzd : n7; }
        c_038 = r5;
          n6 = c_03a;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u119_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u119_0v)); pv7 = (1.0f - fmaxf(s1_sign(u119_0v), 0.0f) * s1_abs(s1_sign(u119_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u119_1v)); pv11 = (1.0f - fmaxf(s1_sign(u119_1v), 0.0f) * s1_abs(s1_sign(u119_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r6 = n6;
        r7 = n7;
    }
    /* 120: 1e05 1fe0 afe2 0039 003e */
    {
        float n1 = r1;
          n1 = c_039;
        c_03e = r2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 121: 2d05 3050 3046 0e12 3000 */
    {
        float n3 = r3;
        float u121_0v = 0.0f, u121_0z = 0.0f;
        { float _w = (1.0f - f7); float o3 = n3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = 0x1.0000000000000p+0f; u121_0v = rs ? rvv : n3; u121_0z = rs ? rzd : n3; }
          n3 = o3 + _w * (n3 - o3); }
        float u121_1v = 0.0f, u121_1z = 0.0f;
        { float _w = f7; float o3 = n3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = r6; u121_1v = rs ? rvv : n3; u121_1z = rs ? rzd : n3; }
          n3 = o3 + _w * (n3 - o3); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f7); pv4 = s1_abs(s1_sign(u121_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u121_0v)); pv7 = (1.0f - fmaxf(s1_sign(u121_0v), 0.0f) * s1_abs(s1_sign(u121_0z)));
        pm2 = f7; pv9 = fminf(1.0f, 1.0f + s1_sign(u121_1v)); pv11 = (1.0f - fmaxf(s1_sign(u121_1v), 0.0f) * s1_abs(s1_sign(u121_1z)));
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 122: 5907 1d49 7590 7592 1223 0000 3000 */
    {
        float n1 = r1, n7 = r7;
        float u122_0v = 0.0f, u122_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r5 + -r1); u122_0v = rs ? rvv : n1; u122_0z = rs ? rzd : n1; }
        float u122_1v = 0.0f, u122_1z = 0.0f;
        { float _w = (1.0f - f9); float o7 = n7;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r6 * 0x0.0p+0f); u122_1v = rs ? rvv : n7; u122_1z = rs ? rzd : n7; }
          n7 = o7 + _w * (n7 - o7); }
        { float _w = f9; float o7 = n7;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r6 * 0x1.0000000000000p+0f); }
          n7 = o7 + _w * (n7 - o7); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u122_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u122_0v)); pv7 = (1.0f - fmaxf(s1_sign(u122_0v), 0.0f) * s1_abs(s1_sign(u122_0z)));
        pm2 = (1.0f - f9); pv9 = fminf(1.0f, 1.0f + s1_sign(u122_1v)); pv11 = (1.0f - fmaxf(s1_sign(u122_1v), 0.0f) * s1_abs(s1_sign(u122_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r7 = n7;
    }
    /* 123: 0001 */
    {
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 124: 2d04 1043 1047 0e12 */
    {
        float n1 = r1;
        float u124_0v = 0.0f, u124_0z = 0.0f;
        { float _w = (1.0f - f7); float o1 = n1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = r3; u124_0v = rs ? rvv : n1; u124_0z = rs ? rzd : n1; }
          n1 = o1 + _w * (n1 - o1); }
        float u124_1v = 0.0f, u124_1z = 0.0f;
        { float _w = f7; float o1 = n1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = r7; u124_1v = rs ? rvv : n1; u124_1z = rs ? rzd : n1; }
          n1 = o1 + _w * (n1 - o1); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f7); pv4 = s1_abs(s1_sign(u124_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u124_0v)); pv7 = (1.0f - fmaxf(s1_sign(u124_0v), 0.0f) * s1_abs(s1_sign(u124_0z)));
        pm2 = f7; pv9 = fminf(1.0f, 1.0f + s1_sign(u124_1v)); pv11 = (1.0f - fmaxf(s1_sign(u124_1v), 0.0f) * s1_abs(s1_sign(u124_1z)));
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 125: 3407 1e50 9fe2 7fe4 3000 0039 003b */
    {
        float n1 = r1, n7 = r7;
        float u125_0v = 0.0f, u125_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(0x1.0000000000000p+0f - r1); u125_0v = rs ? rvv : n1; u125_0z = rs ? rzd : n1; }
        c_039 = r1;
          n7 = c_03b;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u125_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u125_0v)); pv7 = (1.0f - fmaxf(s1_sign(u125_0v), 0.0f) * s1_abs(s1_sign(u125_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r7 = n7;
    }
    /* 126: 1104 5448 1440 000f */
    {
        float n1 = r1, n5 = r5;
        float u126_0v = 0.0f, u126_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (r0 * r1) + (-r7); u126_0v = rs ? rvv : n5; u126_0z = rs ? rzd : n5; }
        float u126_1v = 0.0f, u126_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r0); u126_1v = rs ? rvv : n1; u126_1z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u126_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u126_0v)); pv7 = (1.0f - fmaxf(s1_sign(u126_0v), 0.0f) * s1_abs(s1_sign(u126_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u126_1v)); pv11 = (1.0f - fmaxf(s1_sign(u126_1v), 0.0f) * s1_abs(s1_sign(u126_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r5 = n5;
    }
    /* 127: 1e05 5fe0 2fe2 003e c013 */
    {
        float n2 = r2, n5 = r5;
          n5 = c_03e;
          n2 = KP(0xc013);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r5 = n5;
    }
    /* 128: 1306 19ca 0a10 dc80 0000 1316 */
    {
        float n1 = r1;
        float u128_0v = 0.0f, u128_0z = 0.0f;
        { float _w = (1.0f - f5); float o1 = n1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (r2 * r7) + (0x1.3160000000000p-20f * -r5); u128_0v = rs ? rvv : n1; u128_0z = rs ? rzd : n1; }
          n1 = o1 + _w * (n1 - o1); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f5); pv4 = s1_abs(s1_sign(u128_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u128_0v)); pv7 = (1.0f - fmaxf(s1_sign(u128_0v), 0.0f) * s1_abs(s1_sign(u128_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 129: 3a07 9450 5fe2 2fe4 0000 003f c014 */
    {
        float n1 = r1, n2 = r2, n5 = r5;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = pymax(r1, 0x0.0p+0f); RESZ(n1, (r1) - (0x0.0p+0f)); }
          n5 = c_03f;
          n2 = KP(0xc014);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
        r5 = n5;
    }
    /* 130: 0902 1445 */
    {
        float n1 = r1;
        float u130_0v = 0.0f, u130_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r5); u130_0v = rs ? rvv : n1; u130_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u130_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u130_0v)); pv7 = (1.0f - fmaxf(s1_sign(u130_0v), 0.0f) * s1_abs(s1_sign(u130_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 131: 1405 1c0a 9fe2 9281 003a */
    {
        float n1 = r1;
        float u131_0v = 0.0f, u131_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (r2 * r0) + (r1) + (r2 * -r1); u131_0v = rs ? rvv : n1; u131_0z = rs ? rzd : n1; }
        c_03a = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u131_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u131_0v)); pv7 = (1.0f - fmaxf(s1_sign(u131_0v), 0.0f) * s1_abs(s1_sign(u131_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 132: 0903 0c10 0000 */
    {
        float n0 = r0;
        float u132_0v = 0.0f, u132_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + 0x0.0p+0f); u132_0v = rs ? rvv : n0; u132_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u132_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u132_0v)); pv7 = (1.0f - fmaxf(s1_sign(u132_0v), 0.0f) * s1_abs(s1_sign(u132_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 133: 3e07 5fe0 7fe2 2fe4 003e 003c c015 */
    {
        float n2 = r2, n5 = r5, n7 = r7;
          n5 = c_03e;
          n7 = c_03c;
          n2 = KP(0xc015);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r5 = n5;
        r7 = n7;
    }
    /* 134: 1306 19ca 0810 dc80 0000 29f1 */
    {
        float n1 = r1;
        float u134_0v = 0.0f, u134_0z = 0.0f;
        { float _w = (1.0f - f4); float o1 = n1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (r2 * r7) + (0x1.4f88000000000p-19f * -r5); u134_0v = rs ? rvv : n1; u134_0z = rs ? rzd : n1; }
          n1 = o1 + _w * (n1 - o1); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f4); pv4 = s1_abs(s1_sign(u134_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u134_0v)); pv7 = (1.0f - fmaxf(s1_sign(u134_0v), 0.0f) * s1_abs(s1_sign(u134_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 135: 0a03 8450 0000 */
    {
        float n0 = r0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = pymax(r1, 0x0.0p+0f); RESZ(n0, (r1) - (0x0.0p+0f)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 136: 1e05 8fe0 8fe2 003b 0041 */
    {
        c_03b = r0;
        c_041 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 137: 1e05 0fe0 1fe2 0026 0002 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_026;
          n1 = c_002;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 138: 0c03 2fe0 0023 */
    {
        float n2 = r2;
          n2 = c_023;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 139: 0803 0c0a 9281 */
    {
        float n0 = r0;
        float u139_0v = 0.0f, u139_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r2 * r0) + (r1) + (r2 * -r1); u139_0v = rs ? rvv : n0; u139_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u139_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u139_0v)); pv7 = (1.0f - fmaxf(s1_sign(u139_0v), 0.0f) * s1_abs(s1_sign(u139_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 140: 0c03 8fe0 0042 */
    {
        c_042 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 141: 1e05 0fe0 1fe2 0028 0020 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_028;
          n1 = c_020;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 142: 0c03 2fe0 0042 */
    {
        float n2 = r2;
          n2 = c_042;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 143: 0803 0c0a 9281 */
    {
        float n0 = r0;
        float u143_0v = 0.0f, u143_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r2 * r0) + (r1) + (r2 * -r1); u143_0v = rs ? rvv : n0; u143_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u143_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u143_0v)); pv7 = (1.0f - fmaxf(s1_sign(u143_0v), 0.0f) * s1_abs(s1_sign(u143_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 144: 0c03 8fe0 0043 */
    {
        c_043 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 145: 1108 3432 6400 0036 000f 6bdd 2fff 4453 */
    {
        float n3 = r3, n6 = r6;
        float u145_0v = 0.0f, u145_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (0x1.ed7ba00000000p-13f * r0) + (0x1.ffe88a6000000p-1f); u145_0v = rs ? rvv : n3; u145_0z = rs ? rzd : n3; }
        float u145_1v = 0.0f, u145_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r0 * r0); u145_1v = rs ? rvv : n6; u145_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u145_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u145_0v)); pv7 = (1.0f - fmaxf(s1_sign(u145_0v), 0.0f) * s1_abs(s1_sign(u145_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u145_1v)); pv11 = (1.0f - fmaxf(s1_sign(u145_1v), 0.0f) * s1_abs(s1_sign(u145_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r6 = n6;
    }
    /* 146: 1108 35b2 2e36 0003 8004 acc4 3000 0000 */
    {
        float n2 = r2, n3 = r3;
        float u146_0v = 0.0f, u146_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (-0x1.2b31000000000p-14f * r6) + (r3); u146_0v = rs ? rvv : n3; u146_0z = rs ? rzd : n3; }
        float u146_1v = 0.0f, u146_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(0x1.0000000000000p+0f - r0); u146_1v = rs ? rvv : n2; u146_1z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u146_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u146_0v)); pv7 = (1.0f - fmaxf(s1_sign(u146_0v), 0.0f) * s1_abs(s1_sign(u146_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u146_1v)); pv11 = (1.0f - fmaxf(s1_sign(u146_1v), 0.0f) * s1_abs(s1_sign(u146_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 147: 0807 4432 0036 0000 0000 2fff f045 */
    {
        float n4 = r4;
        float u147_0v = 0.0f, u147_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (0x0.0p+0f * r0) + (0x1.fffe08a000000p-1f); u147_0v = rs ? rvv : n4; u147_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u147_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u147_0v)); pv7 = (1.0f - fmaxf(s1_sign(u147_0v), 0.0f) * s1_abs(s1_sign(u147_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 148: 1703 3044 1710 */
    {
        float n3 = r3;
        float u148_0v = 0.0f, u148_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f11)); float o3 = n3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = r4; u148_0v = rs ? rvv : n3; u148_0z = rs ? rzd : n3; }
          n3 = o3 + _w * (n3 - o3); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f11)); pv4 = s1_abs(s1_sign(u148_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u148_0v)); pv7 = (1.0f - fmaxf(s1_sign(u148_0v), 0.0f) * s1_abs(s1_sign(u148_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 149: 0904 2e30 0000 0000 */
    {
        float n2 = r2;
        float u149_0v = 0.0f, u149_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(0x0.0p+0f - r0); u149_0v = rs ? rvv : n2; u149_0z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u149_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u149_0v)); pv7 = (1.0f - fmaxf(s1_sign(u149_0v), 0.0f) * s1_abs(s1_sign(u149_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 150: 0807 4432 0036 0000 0000 2fff 4453 */
    {
        float n4 = r4;
        float u150_0v = 0.0f, u150_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (0x0.0p+0f * r0) + (0x1.ffe88a6000000p-1f); u150_0v = rs ? rvv : n4; u150_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u150_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u150_0v)); pv7 = (1.0f - fmaxf(s1_sign(u150_0v), 0.0f) * s1_abs(s1_sign(u150_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 151: 1703 3044 0b10 */
    {
        float n3 = r3;
        float u151_0v = 0.0f, u151_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o3 = n3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = r4; u151_0v = rs ? rvv : n3; u151_0z = rs ? rzd : n3; }
          n3 = o3 + _w * (n3 - o3); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u151_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u151_0v)); pv7 = (1.0f - fmaxf(s1_sign(u151_0v), 0.0f) * s1_abs(s1_sign(u151_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 152: 1804 0043 bfe0 0044 */
    {
        float n0 = r0;
        float u152_0v = 0.0f, u152_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = r3; u152_0v = rs ? rvv : n0; u152_0z = rs ? rzd : n0; }
        c_044 = r3;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u152_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u152_0v)); pv7 = (1.0f - fmaxf(s1_sign(u152_0v), 0.0f) * s1_abs(s1_sign(u152_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 153: 1e05 0fe0 1fe2 0034 001f */
    {
        float n0 = r0, n1 = r1;
          n0 = c_034;
          n1 = c_01f;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 154: 1e05 2fe0 3fe2 0044 0021 */
    {
        float n2 = r2, n3 = r3;
          n2 = c_044;
          n3 = c_021;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 155: 0c03 4fe0 0022 */
    {
        float n4 = r4;
          n4 = c_022;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 156: 1e05 6fe0 bfe2 0046 004c */
    {
        float n6 = r6;
          n6 = c_046;
        c_04c = r3;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 157: 1804 3c0e 8fe0 004b */
    {
        float n3 = r3;
        float u157_0v = 0.0f, u157_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r0 + -r6); u157_0v = rs ? rvv : n3; u157_0z = rs ? rzd : n3; }
        c_04b = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u157_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u157_0v)); pv7 = (1.0f - fmaxf(s1_sign(u157_0v), 0.0f) * s1_abs(s1_sign(u157_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 158: 0001 */
    {
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 159: 1304 0d89 0e10 8240 */
    {
        float n0 = r0;
        float u159_0v = 0.0f, u159_0z = 0.0f;
        { float _w = (1.0f - f7); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r1 * r6) + (r0) + (r1 * -r0); u159_0v = rs ? rvv : n0; u159_0z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f7); pv4 = s1_abs(s1_sign(u159_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u159_0v)); pv7 = (1.0f - fmaxf(s1_sign(u159_0v), 0.0f) * s1_abs(s1_sign(u159_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 160: 1805 5410 8fe2 3199 0045 */
    {
        float n5 = r5;
        float u160_0v = 0.0f, u160_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r0 * 0x1.1990000000000p+0f); u160_0v = rs ? rvv : n5; u160_0z = rs ? rzd : n5; }
        c_045 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u160_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u160_0v)); pv7 = (1.0f - fmaxf(s1_sign(u160_0v), 0.0f) * s1_abs(s1_sign(u160_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
    }
    /* 161: 5f0a 3d64 7d67 dfe0 6fe2 0046 0048 b000 0099 9999 */
    {
        float n3 = r3, n6 = r6, n7 = r7;
        float u161_0v = 0.0f, u161_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r5 + -0x1.0000000000000p+0f); u161_0v = rs ? rvv : n3; u161_0z = rs ? rzd : n3; }
        float u161_1v = 0.0f, u161_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r5 + -0x1.9999900000000p-4f); u161_1v = rs ? rvv : n7; u161_1z = rs ? rzd : n7; }
        c_046 = r5;
          n6 = c_048;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u161_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u161_0v)); pv7 = (1.0f - fmaxf(s1_sign(u161_0v), 0.0f) * s1_abs(s1_sign(u161_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u161_1v)); pv11 = (1.0f - fmaxf(s1_sign(u161_1v), 0.0f) * s1_abs(s1_sign(u161_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r6 = n6;
        r7 = n7;
    }
    /* 162: 0c03 1fe0 0047 */
    {
        float n1 = r1;
          n1 = c_047;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 163: 2d05 3050 3046 0e12 3199 */
    {
        float n3 = r3;
        float u163_0v = 0.0f, u163_0z = 0.0f;
        { float _w = (1.0f - f7); float o3 = n3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = 0x1.1990000000000p+0f; u163_0v = rs ? rvv : n3; u163_0z = rs ? rzd : n3; }
          n3 = o3 + _w * (n3 - o3); }
        float u163_1v = 0.0f, u163_1z = 0.0f;
        { float _w = f7; float o3 = n3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = r6; u163_1v = rs ? rvv : n3; u163_1z = rs ? rzd : n3; }
          n3 = o3 + _w * (n3 - o3); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f7); pv4 = s1_abs(s1_sign(u163_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u163_0v)); pv7 = (1.0f - fmaxf(s1_sign(u163_0v), 0.0f) * s1_abs(s1_sign(u163_0z)));
        pm2 = f7; pv9 = fminf(1.0f, 1.0f + s1_sign(u163_1v)); pv11 = (1.0f - fmaxf(s1_sign(u163_1v), 0.0f) * s1_abs(s1_sign(u163_1z)));
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 164: 2d05 7050 7046 1212 0000 */
    {
        float n7 = r7;
        float u164_0v = 0.0f, u164_0z = 0.0f;
        { float _w = (1.0f - f9); float o7 = n7;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x0.0p+0f; u164_0v = rs ? rvv : n7; u164_0z = rs ? rzd : n7; }
          n7 = o7 + _w * (n7 - o7); }
        float u164_1v = 0.0f, u164_1z = 0.0f;
        { float _w = f9; float o7 = n7;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = r6; u164_1v = rs ? rvv : n7; u164_1z = rs ? rzd : n7; }
          n7 = o7 + _w * (n7 - o7); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f9); pv4 = s1_abs(s1_sign(u164_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u164_0v)); pv7 = (1.0f - fmaxf(s1_sign(u164_0v), 0.0f) * s1_abs(s1_sign(u164_0z)));
        pm2 = f9; pv9 = fminf(1.0f, 1.0f + s1_sign(u164_1v)); pv11 = (1.0f - fmaxf(s1_sign(u164_1v), 0.0f) * s1_abs(s1_sign(u164_1z)));
        pm4 = 0.0f;
        r7 = n7;
    }
    /* 165: 0902 1d49 */
    {
        float n1 = r1;
        float u165_0v = 0.0f, u165_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r5 + -r1); u165_0v = rs ? rvv : n1; u165_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u165_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u165_0v)); pv7 = (1.0f - fmaxf(s1_sign(u165_0v), 0.0f) * s1_abs(s1_sign(u165_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 166: 0001 */
    {
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 167: 2d04 1043 1047 0e12 */
    {
        float n1 = r1;
        float u167_0v = 0.0f, u167_0z = 0.0f;
        { float _w = (1.0f - f7); float o1 = n1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = r3; u167_0v = rs ? rvv : n1; u167_0z = rs ? rzd : n1; }
          n1 = o1 + _w * (n1 - o1); }
        float u167_1v = 0.0f, u167_1z = 0.0f;
        { float _w = f7; float o1 = n1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = r7; u167_1v = rs ? rvv : n1; u167_1z = rs ? rzd : n1; }
          n1 = o1 + _w * (n1 - o1); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f7); pv4 = s1_abs(s1_sign(u167_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u167_0v)); pv7 = (1.0f - fmaxf(s1_sign(u167_0v), 0.0f) * s1_abs(s1_sign(u167_0z)));
        pm2 = f7; pv9 = fminf(1.0f, 1.0f + s1_sign(u167_1v)); pv11 = (1.0f - fmaxf(s1_sign(u167_1v), 0.0f) * s1_abs(s1_sign(u167_1z)));
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 168: 3407 1e50 9fe2 7fe4 3199 0047 0049 */
    {
        float n1 = r1, n7 = r7;
        float u168_0v = 0.0f, u168_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(0x1.1990000000000p+0f - r1); u168_0v = rs ? rvv : n1; u168_0z = rs ? rzd : n1; }
        c_047 = r1;
          n7 = c_049;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u168_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u168_0v)); pv7 = (1.0f - fmaxf(s1_sign(u168_0v), 0.0f) * s1_abs(s1_sign(u168_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r7 = n7;
    }
    /* 169: 1104 5448 1440 000f */
    {
        float n1 = r1, n5 = r5;
        float u169_0v = 0.0f, u169_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (r0 * r1) + (-r7); u169_0v = rs ? rvv : n5; u169_0z = rs ? rzd : n5; }
        float u169_1v = 0.0f, u169_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r0); u169_1v = rs ? rvv : n1; u169_1z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u169_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u169_0v)); pv7 = (1.0f - fmaxf(s1_sign(u169_0v), 0.0f) * s1_abs(s1_sign(u169_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u169_1v)); pv11 = (1.0f - fmaxf(s1_sign(u169_1v), 0.0f) * s1_abs(s1_sign(u169_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r5 = n5;
    }
    /* 170: 1e05 5fe0 3fe2 004b 004c */
    {
        float n3 = r3, n5 = r5;
          n5 = c_04b;
          n3 = c_04c;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
        r5 = n5;
    }
    /* 171: 1703 15c2 0a10 */
    {
        float n1 = r1;
        float u171_0v = 0.0f, u171_0z = 0.0f;
        { float _w = (1.0f - f5); float o1 = n1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r7 * r2); u171_0v = rs ? rvv : n1; u171_0z = rs ? rzd : n1; }
          n1 = o1 + _w * (n1 - o1); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f5); pv4 = s1_abs(s1_sign(u171_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u171_0v)); pv7 = (1.0f - fmaxf(s1_sign(u171_0v), 0.0f) * s1_abs(s1_sign(u171_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 172: 0902 1445 */
    {
        float n1 = r1;
        float u172_0v = 0.0f, u172_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r5); u172_0v = rs ? rvv : n1; u172_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u172_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u172_0v)); pv7 = (1.0f - fmaxf(s1_sign(u172_0v), 0.0f) * s1_abs(s1_sign(u172_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 173: 1405 1c0b 9fe2 92c1 0048 */
    {
        float n1 = r1;
        float u173_0v = 0.0f, u173_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (r3 * r0) + (r1) + (r3 * -r1); u173_0v = rs ? rvv : n1; u173_0z = rs ? rzd : n1; }
        c_048 = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u173_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u173_0v)); pv7 = (1.0f - fmaxf(s1_sign(u173_0v), 0.0f) * s1_abs(s1_sign(u173_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 174: 0903 0c10 0000 */
    {
        float n0 = r0;
        float u174_0v = 0.0f, u174_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + 0x0.0p+0f); u174_0v = rs ? rvv : n0; u174_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u174_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u174_0v)); pv7 = (1.0f - fmaxf(s1_sign(u174_0v), 0.0f) * s1_abs(s1_sign(u174_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 175: 1804 0041 7fe0 004a */
    {
        float n0 = r0, n7 = r7;
        float u175_0v = 0.0f, u175_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = r1; u175_0v = rs ? rvv : n0; u175_0z = rs ? rzd : n0; }
          n7 = c_04a;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u175_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u175_0v)); pv7 = (1.0f - fmaxf(s1_sign(u175_0v), 0.0f) * s1_abs(s1_sign(u175_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r7 = n7;
    }
    /* 176: 1703 05c4 0810 */
    {
        float n0 = r0;
        float u176_0v = 0.0f, u176_0z = 0.0f;
        { float _w = (1.0f - f4); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r7 * r4); u176_0v = rs ? rvv : n0; u176_0z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f4); pv4 = s1_abs(s1_sign(u176_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u176_0v)); pv7 = (1.0f - fmaxf(s1_sign(u176_0v), 0.0f) * s1_abs(s1_sign(u176_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 177: 1e05 8fe0 8fe2 0049 004d */
    {
        c_049 = r0;
        c_04d = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 178: 1e05 0fe0 1fe2 004d 0042 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_04d;
          n1 = c_042;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 179: 0902 0401 */
    {
        float n0 = r0;
        float u179_0v = 0.0f, u179_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r1); u179_0v = rs ? rvv : n0; u179_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u179_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u179_0v)); pv7 = (1.0f - fmaxf(s1_sign(u179_0v), 0.0f) * s1_abs(s1_sign(u179_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 180: 0c03 8fe0 004e */
    {
        c_04e = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 181: 1e05 0fe0 1fe2 c016 c017 */
    {
        float n0 = r0, n1 = r1;
          n0 = KP(0xc016);
          n1 = KP(0xc017);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 182: 3e07 8fe0 9fe2 2fe4 004f 0050 c018 */
    {
        float n2 = r2;
        c_04f = r0;
        c_050 = r1;
          n2 = KP(0xc018);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 183: 0c03 afe0 0051 */
    {
        c_051 = r2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 184: 1e05 0fe0 1fe2 c019 c01a */
    {
        float n0 = r0, n1 = r1;
          n0 = KP(0xc019);
          n1 = KP(0xc01a);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 185: 7809 8fe0 9fe2 2fe4 3fe6 0052 0053 c01b c01c */
    {
        float n2 = r2, n3 = r3;
        c_052 = r0;
        c_053 = r1;
          n2 = KP(0xc01b);
          n3 = KP(0xc01c);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 186: 7809 afe0 bfe2 4fe4 5fe6 0054 0055 c01d c01e */
    {
        float n4 = r4, n5 = r5;
        c_054 = r2;
        c_055 = r3;
          n4 = KP(0xc01d);
          n5 = KP(0xc01e);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
    }
    /* 187: 1e05 cfe0 dfe2 0056 0057 */
    {
        c_056 = r4;
        c_057 = r5;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 188: 0904 0060 3000 0000 */
    {
        float n0 = r0;
        float u188_0v = 0.0f, u188_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = 0x1.0000000000000p+0f; u188_0v = rs ? rvv : n0; u188_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u188_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u188_0v)); pv7 = (1.0f - fmaxf(s1_sign(u188_0v), 0.0f) * s1_abs(s1_sign(u188_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 189: 0c03 8fe0 0058 */
    {
        c_058 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 190: 1e05 0fe0 1fe2 004d 001c */
    {
        float n0 = r0, n1 = r1;
          n0 = c_04d;
          n1 = c_01c;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 191: 0902 0c01 */
    {
        float n0 = r0;
        float u191_0v = 0.0f, u191_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + r1); u191_0v = rs ? rvv : n0; u191_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u191_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u191_0v)); pv7 = (1.0f - fmaxf(s1_sign(u191_0v), 0.0f) * s1_abs(s1_sign(u191_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 192: 0c03 8fe0 0059 */
    {
        c_059 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 193: 1506 2c20 3c23 b000 0000 0000 */
    {
        float n2 = r2, n3 = r3;
        float u193_0v = 0.0f, u193_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r0 + -0x1.0000000000000p+0f); u193_0v = rs ? rvv : n2; u193_0z = rs ? rzd : n2; }
        float u193_1v = 0.0f, u193_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r0 + 0x0.0p+0f); u193_1v = rs ? rvv : n3; u193_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u193_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u193_0v)); pv7 = (1.0f - fmaxf(s1_sign(u193_0v), 0.0f) * s1_abs(s1_sign(u193_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u193_1v)); pv11 = (1.0f - fmaxf(s1_sign(u193_1v), 0.0f) * s1_abs(s1_sign(u193_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 194: 1506 2060 3063 b000 0000 0000 */
    {
        float n2 = r2, n3 = r3;
        float u194_0v = 0.0f, u194_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = -0x1.0000000000000p+0f; u194_0v = rs ? rvv : n2; u194_0z = rs ? rzd : n2; }
        float u194_1v = 0.0f, u194_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = 0x0.0p+0f; u194_1v = rs ? rvv : n3; u194_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u194_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u194_0v)); pv7 = (1.0f - fmaxf(s1_sign(u194_0v), 0.0f) * s1_abs(s1_sign(u194_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u194_1v)); pv11 = (1.0f - fmaxf(s1_sign(u194_1v), 0.0f) * s1_abs(s1_sign(u194_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 195: 5d05 004a 004b 0e10 1220 */
    {
        float n0 = r0;
        float u195_0v = 0.0f, u195_0z = 0.0f;
        { float _w = (1.0f - f7); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -r2; u195_0v = rs ? rvv : n0; u195_0z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        float u195_1v = 0.0f, u195_1z = 0.0f;
        { float _w = (1.0f - f9); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -r3; u195_1v = rs ? rvv : n0; u195_1z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f7); pv4 = s1_abs(s1_sign(u195_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u195_0v)); pv7 = (1.0f - fmaxf(s1_sign(u195_0v), 0.0f) * s1_abs(s1_sign(u195_0z)));
        pm2 = (1.0f - f9); pv9 = fminf(1.0f, 1.0f + s1_sign(u195_1v)); pv11 = (1.0f - fmaxf(s1_sign(u195_1v), 0.0f) * s1_abs(s1_sign(u195_1z)));
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 196: 0c03 8fe0 005a */
    {
        c_05a = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 197: 1e05 0fe0 1fe2 005a 004e */
    {
        float n0 = r0, n1 = r1;
          n0 = c_05a;
          n1 = c_04e;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 198: 1e05 2fe0 3fe2 0052 0053 */
    {
        float n2 = r2, n3 = r3;
          n2 = c_052;
          n3 = c_053;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 199: 1e05 4fe0 5fe2 0054 0055 */
    {
        float n4 = r4, n5 = r5;
          n4 = c_054;
          n5 = c_055;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
    }
    /* 200: 250a 74b4 6482 9fe2 0038 0062 4294 6e59 0000 0000 */
    {
        float n6 = r6, n7 = r7;
        float u200_0v = 0.0f, u200_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (0x1.4a372c8000000p+1f * r2) + (0x0.0p+0f); u200_0v = rs ? rvv : n7; u200_0z = rs ? rzd : n7; }
        float u200_1v = 0.0f, u200_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r2 * r2); u200_1v = rs ? rvv : n6; u200_1z = rs ? rzd : n6; }
        c_062 = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u200_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u200_0v)); pv7 = (1.0f - fmaxf(s1_sign(u200_0v), 0.0f) * s1_abs(s1_sign(u200_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u200_1v)); pv11 = (1.0f - fmaxf(s1_sign(u200_1v), 0.0f) * s1_abs(s1_sign(u200_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 201: 1106 75b2 6582 0007 c258 f59a */
    {
        float n6 = r6, n7 = r7;
        float u201_0v = 0.0f, u201_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (-0x1.2c7acd0000000p+1f * r6) + (r7); u201_0v = rs ? rvv : n7; u201_0z = rs ? rzd : n7; }
        float u201_1v = 0.0f, u201_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r6 * r2); u201_1v = rs ? rvv : n6; u201_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u201_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u201_0v)); pv7 = (1.0f - fmaxf(s1_sign(u201_0v), 0.0f) * s1_abs(s1_sign(u201_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u201_1v)); pv11 = (1.0f - fmaxf(s1_sign(u201_1v), 0.0f) * s1_abs(s1_sign(u201_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 202: 220b 5554 75ba 1fe8 0016 0007 4200 c200 005d 2c48 740b */
    {
        float n1 = r1, n5 = r5, n7 = r7;
        float u202_0v = 0.0f, u202_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (0x1.0000000000000p+1f * r5) + (-0x1.0000000000000p+1f); u202_0v = rs ? rvv : n5; u202_0z = rs ? rzd : n5; }
        float u202_1v = 0.0f, u202_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (0x1.890e816000000p-1f * r6) + (r7); u202_1v = rs ? rvv : n7; u202_1z = rs ? rzd : n7; }
          n1 = c_05d;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u202_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u202_0v)); pv7 = (1.0f - fmaxf(s1_sign(u202_0v), 0.0f) * s1_abs(s1_sign(u202_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u202_1v)); pv11 = (1.0f - fmaxf(s1_sign(u202_1v), 0.0f) * s1_abs(s1_sign(u202_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r5 = n5;
        r7 = n7;
    }
    /* 203: 2809 1c24 dedd ffe2 99c1 0063 3000 0030 0000 */
    {
        float n1 = r1, n5 = r5;
        float u203_0v = 0.0f, u203_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (0x1.0000000000000p+0f * r0) + (r1) + (0x1.0000000000000p+0f * -r1); u203_0v = rs ? rvv : n1; u203_0z = rs ? rzd : n1; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = pymax(-1.0f, pymin(1.0f, r5)); }
        c_063 = r7;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u203_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u203_0v)); pv7 = (1.0f - fmaxf(s1_sign(u203_0v), 0.0f) * s1_abs(s1_sign(u203_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r5 = n5;
    }
    /* 204: 1805 5d50 9fe2 3000 005c */
    {
        float n5 = r5;
        float u204_0v = 0.0f, u204_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r5 + 0x1.0000000000000p+0f); u204_0v = rs ? rvv : n5; u204_0z = rs ? rzd : n5; }
        c_05c = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u204_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u204_0v)); pv7 = (1.0f - fmaxf(s1_sign(u204_0v), 0.0f) * s1_abs(s1_sign(u204_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
    }
    /* 205: 250a 74b4 6482 8fe2 0038 0060 0620 3883 0f5c 28f5 */
    {
        float n6 = r6, n7 = r7;
        float u205_0v = 0.0f, u205_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (0x1.880e20c000000p-6f * r2) + (0x1.eb851ea000000p-5f); u205_0v = rs ? rvv : n7; u205_0z = rs ? rzd : n7; }
        float u205_1v = 0.0f, u205_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r2 * r2); u205_1v = rs ? rvv : n6; u205_1z = rs ? rzd : n6; }
        c_060 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u205_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u205_0v)); pv7 = (1.0f - fmaxf(s1_sign(u205_0v), 0.0f) * s1_abs(s1_sign(u205_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u205_1v)); pv11 = (1.0f - fmaxf(s1_sign(u205_1v), 0.0f) * s1_abs(s1_sign(u205_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 206: 220c 75bb 06e8 cfe6 0007 0014 3000 0061 2ccc cc83 90dc 5a00 */
    {
        float n0 = r0, n7 = r7;
        float u206_0v = 0.0f, u206_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (-0x1.c86e2d0000000p-7f * r6) + (r7); u206_0v = rs ? rvv : n7; u206_0z = rs ? rzd : n7; }
        float u206_1v = 0.0f, u206_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(0x1.9999800000000p-1f * r3) + (0x1.0000000000000p+0f); u206_1v = rs ? rvv : n0; u206_1z = rs ? rzd : n0; }
        c_061 = r4;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u206_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u206_0v)); pv7 = (1.0f - fmaxf(s1_sign(u206_0v), 0.0f) * s1_abs(s1_sign(u206_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u206_1v)); pv11 = (1.0f - fmaxf(s1_sign(u206_1v), 0.0f) * s1_abs(s1_sign(u206_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r7 = n7;
    }
    /* 207: 250a 74b4 4547 1fe2 0038 0062 1152 5506 10a3 d70a */
    {
        float n1 = r1, n4 = r4, n7 = r7;
        float u207_0v = 0.0f, u207_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (0x1.1525506000000p-4f * r2) + (0x1.0a3d70a000000p-4f); u207_0v = rs ? rvv : n7; u207_0z = rs ? rzd : n7; }
        float u207_1v = 0.0f, u207_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r5 * r7); u207_1v = rs ? rvv : n4; u207_1z = rs ? rzd : n4; }
          n1 = c_062;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u207_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u207_0v)); pv7 = (1.0f - fmaxf(s1_sign(u207_0v), 0.0f) * s1_abs(s1_sign(u207_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u207_1v)); pv11 = (1.0f - fmaxf(s1_sign(u207_1v), 0.0f) * s1_abs(s1_sign(u207_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r4 = n4;
        r7 = n7;
    }
    /* 208: 1106 75b2 0440 0007 0470 3a55 */
    {
        float n0 = r0, n7 = r7;
        float u208_0v = 0.0f, u208_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (0x1.1c0e954000000p-6f * r6) + (r7); u208_0v = rs ? rvv : n7; u208_0z = rs ? rzd : n7; }
        float u208_1v = 0.0f, u208_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r1 * r0); u208_1v = rs ? rvv : n0; u208_1z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u208_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u208_0v)); pv7 = (1.0f - fmaxf(s1_sign(u208_0v), 0.0f) * s1_abs(s1_sign(u208_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u208_1v)); pv11 = (1.0f - fmaxf(s1_sign(u208_1v), 0.0f) * s1_abs(s1_sign(u208_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r7 = n7;
    }
    /* 209: 0902 0404 */
    {
        float n0 = r0;
        float u209_0v = 0.0f, u209_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r4); u209_0v = rs ? rvv : n0; u209_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u209_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u209_0v)); pv7 = (1.0f - fmaxf(s1_sign(u209_0v), 0.0f) * s1_abs(s1_sign(u209_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 210: 0904 4c70 a70a 3d70 */
    {
        float n4 = r4;
        float u210_0v = 0.0f, u210_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r1 + -0x1.c28f5c0000000p-2f); u210_0v = rs ? rvv : n4; u210_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u210_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u210_0v)); pv7 = (1.0f - fmaxf(s1_sign(u210_0v), 0.0f) * s1_abs(s1_sign(u210_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 211: 0904 4c70 a70c 88a4 */
    {
        float n4 = r4;
        float u211_0v = 0.0f, u211_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r1 + -0x1.c322290000000p-2f); u211_0v = rs ? rvv : n4; u211_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u211_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u211_0v)); pv7 = (1.0f - fmaxf(s1_sign(u211_0v), 0.0f) * s1_abs(s1_sign(u211_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 212: 2d06 4b30 4041 0e12 270a 3dc3 */
    {
        float n4 = r4;
        float u212_0v = 0.0f, u212_0z = 0.0f;
        { float _w = (1.0f - f7); float o4 = n4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(0x1.c28f70c000000p-2f - r4 * r4); u212_0v = rs ? rvv : n4; u212_0z = rs ? rzd : n4; }
          n4 = o4 + _w * (n4 - o4); }
        float u212_1v = 0.0f, u212_1z = 0.0f;
        { float _w = f7; float o4 = n4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r1; u212_1v = rs ? rvv : n4; u212_1z = rs ? rzd : n4; }
          n4 = o4 + _w * (n4 - o4); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f7); pv4 = s1_abs(s1_sign(u212_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u212_0v)); pv7 = (1.0f - fmaxf(s1_sign(u212_0v), 0.0f) * s1_abs(s1_sign(u212_0z)));
        pm2 = f7; pv9 = fminf(1.0f, 1.0f + s1_sign(u212_1v)); pv11 = (1.0f - fmaxf(s1_sign(u212_1v), 0.0f) * s1_abs(s1_sign(u212_1z)));
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 213: 1405 4d0b 6fe2 92c1 0062 */
    {
        float n4 = r4, n6 = r6;
        float u213_0v = 0.0f, u213_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (r3 * r4) + (r1) + (r3 * -r1); u213_0v = rs ? rvv : n4; u213_0z = rs ? rzd : n4; }
          n6 = c_062;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u213_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u213_0v)); pv7 = (1.0f - fmaxf(s1_sign(u213_0v), 0.0f) * s1_abs(s1_sign(u213_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r6 = n6;
    }
    /* 214: 2e07 65b2 7547 1fe0 0061 4800 0000 */
    {
        float n1 = r1, n6 = r6, n7 = r7;
        float u214_0v = 0.0f, u214_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r6 * 0x1.0000000000000p+3f); u214_0v = rs ? rvv : n6; u214_0z = rs ? rzd : n6; }
        float u214_1v = 0.0f, u214_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r5 * r7); u214_1v = rs ? rvv : n7; u214_1z = rs ? rzd : n7; }
          n1 = c_061;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u214_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u214_0v)); pv7 = (1.0f - fmaxf(s1_sign(u214_0v), 0.0f) * s1_abs(s1_sign(u214_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u214_1v)); pv11 = (1.0f - fmaxf(s1_sign(u214_1v), 0.0f) * s1_abs(s1_sign(u214_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r6 = n6;
        r7 = n7;
    }
    /* 215: 0a03 9050 2400 */
    {
        float n1 = r1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = pymin(r1, 0x1.0000000000000p-2f); RESZ(n1, (r1) - (0x1.0000000000000p-2f)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 216: 0805 1652 0014 4400 3000 */
    {
        float n1 = r1;
        float u216_0v = 0.0f, u216_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = -(0x1.0000000000000p+2f * r1) + (0x1.0000000000000p+0f); u216_0v = rs ? rvv : n1; u216_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u216_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u216_0v)); pv7 = (1.0f - fmaxf(s1_sign(u216_0v), 0.0f) * s1_abs(s1_sign(u216_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 217: 1503 6585 1441 */
    {
        float n1 = r1, n6 = r6;
        float u217_0v = 0.0f, u217_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r6 * r5); u217_0v = rs ? rvv : n6; u217_0z = rs ? rzd : n6; }
        float u217_1v = 0.0f, u217_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r1); u217_1v = rs ? rvv : n1; u217_1z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u217_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u217_0v)); pv7 = (1.0f - fmaxf(s1_sign(u217_0v), 0.0f) * s1_abs(s1_sign(u217_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u217_1v)); pv11 = (1.0f - fmaxf(s1_sign(u217_1v), 0.0f) * s1_abs(s1_sign(u217_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r6 = n6;
    }
    /* 218: 2e06 6850 1507 7fe2 3000 0061 */
    {
        float n1 = r1, n6 = r6, n7 = r7;
        float u218_0v = 0.0f, u218_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(0x1.0000000000000p+0f + r1 * r6); u218_0v = rs ? rvv : n6; u218_0z = rs ? rzd : n6; }
        float u218_1v = 0.0f, u218_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r4 * r7); u218_1v = rs ? rvv : n1; u218_1z = rs ? rzd : n1; }
          n7 = c_061;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u218_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u218_0v)); pv7 = (1.0f - fmaxf(s1_sign(u218_0v), 0.0f) * s1_abs(s1_sign(u218_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u218_1v)); pv11 = (1.0f - fmaxf(s1_sign(u218_1v), 0.0f) * s1_abs(s1_sign(u218_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r6 = n6;
        r7 = n7;
    }
    /* 219: 1405 1c0f efe2 93c1 0064 */
    {
        float n1 = r1;
        float u219_0v = 0.0f, u219_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (r7 * r0) + (r1) + (r7 * -r1); u219_0v = rs ? rvv : n1; u219_0z = rs ? rzd : n1; }
        c_064 = r6;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u219_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u219_0v)); pv7 = (1.0f - fmaxf(s1_sign(u219_0v), 0.0f) * s1_abs(s1_sign(u219_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 220: 250a 74b4 6482 5fe2 0038 0062 1e76 b534 31eb 851e */
    {
        float n5 = r5, n6 = r6, n7 = r7;
        float u220_0v = 0.0f, u220_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (0x1.e76b534000000p-4f * r2) + (0x1.1eb851e000000p+0f); u220_0v = rs ? rvv : n7; u220_0z = rs ? rzd : n7; }
        float u220_1v = 0.0f, u220_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r2 * r2); u220_1v = rs ? rvv : n6; u220_1z = rs ? rzd : n6; }
          n5 = c_062;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u220_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u220_0v)); pv7 = (1.0f - fmaxf(s1_sign(u220_0v), 0.0f) * s1_abs(s1_sign(u220_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u220_1v)); pv11 = (1.0f - fmaxf(s1_sign(u220_1v), 0.0f) * s1_abs(s1_sign(u220_1z)));
        pm4 = 0.0f;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 221: 1108 75b2 5576 0007 2a85 0837 4800 0000 */
    {
        float n5 = r5, n7 = r7;
        float u221_0v = 0.0f, u221_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (0x1.50a106e000000p-1f * r6) + (r7); u221_0v = rs ? rvv : n7; u221_0z = rs ? rzd : n7; }
        float u221_1v = 0.0f, u221_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r5 * 0x1.0000000000000p+3f); u221_1v = rs ? rvv : n5; u221_1z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u221_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u221_0v)); pv7 = (1.0f - fmaxf(s1_sign(u221_0v), 0.0f) * s1_abs(s1_sign(u221_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u221_1v)); pv11 = (1.0f - fmaxf(s1_sign(u221_1v), 0.0f) * s1_abs(s1_sign(u221_1z)));
        pm4 = 0.0f;
        r5 = n5;
        r7 = n7;
    }
    /* 222: 1208 44b2 dedd 0036 2f29 02a7 2ccc cccc */
    {
        float n4 = r4, n5 = r5;
        float u222_0v = 0.0f, u222_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (0x1.e52054e000000p-1f * r2) + (0x1.9999998000000p-1f); u222_0v = rs ? rvv : n4; u222_0z = rs ? rzd : n4; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = pymax(-1.0f, pymin(1.0f, r5)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u222_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u222_0v)); pv7 = (1.0f - fmaxf(s1_sign(u222_0v), 0.0f) * s1_abs(s1_sign(u222_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
    }
    /* 223: 1407 45b4 dfe2 0004 0062 a847 baf8 */
    {
        float n4 = r4;
        float u223_0v = 0.0f, u223_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (-0x1.08f75f0000000p-1f * r6) + (r4); u223_0v = rs ? rvv : n4; u223_0z = rs ? rzd : n4; }
        c_062 = r5;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u223_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u223_0v)); pv7 = (1.0f - fmaxf(s1_sign(u223_0v), 0.0f) * s1_abs(s1_sign(u223_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 224: 1405 7dcb 5fe2 c2c4 0060 */
    {
        float n5 = r5, n7 = r7;
        float u224_0v = 0.0f, u224_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (r3 * r7) + (r4) + (r3 * -r4); u224_0v = rs ? rvv : n7; u224_0z = rs ? rzd : n7; }
          n5 = c_060;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u224_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u224_0v)); pv7 = (1.0f - fmaxf(s1_sign(u224_0v), 0.0f) * s1_abs(s1_sign(u224_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
        r7 = n7;
    }
    /* 225: 4e0c 74b6 0547 4fe2 5fe4 003a 0064 005c 1608 37fd 2ee1 47ae */
    {
        float n0 = r0, n4 = r4, n5 = r5, n7 = r7;
        float u225_0v = 0.0f, u225_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (0x1.60837fd000000p-4f * r2) + (0x1.dc28f5c000000p-1f); u225_0v = rs ? rvv : n7; u225_0z = rs ? rzd : n7; }
        float u225_1v = 0.0f, u225_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r5 * r7); u225_1v = rs ? rvv : n0; u225_1z = rs ? rzd : n0; }
          n4 = c_064;
          n5 = c_05c;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u225_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u225_0v)); pv7 = (1.0f - fmaxf(s1_sign(u225_0v), 0.0f) * s1_abs(s1_sign(u225_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u225_1v)); pv11 = (1.0f - fmaxf(s1_sign(u225_1v), 0.0f) * s1_abs(s1_sign(u225_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r4 = n4;
        r5 = n5;
        r7 = n7;
    }
    /* 226: 2508 75b4 5544 4fe2 0007 0061 841c b2de */
    {
        float n4 = r4, n5 = r5, n7 = r7;
        float u226_0v = 0.0f, u226_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (-0x1.072cb78000000p-6f * r6) + (r7); u226_0v = rs ? rvv : n7; u226_0z = rs ? rzd : n7; }
        float u226_1v = 0.0f, u226_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r5 * r4); u226_1v = rs ? rvv : n5; u226_1z = rs ? rzd : n5; }
          n4 = c_061;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u226_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u226_0v)); pv7 = (1.0f - fmaxf(s1_sign(u226_0v), 0.0f) * s1_abs(s1_sign(u226_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u226_1v)); pv11 = (1.0f - fmaxf(s1_sign(u226_1v), 0.0f) * s1_abs(s1_sign(u226_1z)));
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r7 = n7;
    }
    /* 227: 1603 4507 dedd */
    {
        float n4 = r4, n5 = r5;
        float u227_0v = 0.0f, u227_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r4 * r7); u227_0v = rs ? rvv : n4; u227_0z = rs ? rzd : n4; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = pymax(-1.0f, pymin(1.0f, r5)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u227_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u227_0v)); pv7 = (1.0f - fmaxf(s1_sign(u227_0v), 0.0f) * s1_abs(s1_sign(u227_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
    }
    /* 228: 250a 7534 6504 dfe2 0038 0064 0817 a9c7 04dd 2f1a */
    {
        float n6 = r6, n7 = r7;
        float u228_0v = 0.0f, u228_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (0x1.02f538e000000p-5f * r4) + (0x1.374bc68000000p-6f); u228_0v = rs ? rvv : n7; u228_0z = rs ? rzd : n7; }
        float u228_1v = 0.0f, u228_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r4 * r4); u228_1v = rs ? rvv : n6; u228_1z = rs ? rzd : n6; }
        c_064 = r5;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u228_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u228_0v)); pv7 = (1.0f - fmaxf(s1_sign(u228_0v), 0.0f) * s1_abs(s1_sign(u228_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u228_1v)); pv11 = (1.0f - fmaxf(s1_sign(u228_1v), 0.0f) * s1_abs(s1_sign(u228_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 229: 1106 75b2 6584 0007 1ca6 e638 */
    {
        float n6 = r6, n7 = r7;
        float u229_0v = 0.0f, u229_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (0x1.ca6e638000000p-4f * r6) + (r7); u229_0v = rs ? rvv : n7; u229_0z = rs ? rzd : n7; }
        float u229_1v = 0.0f, u229_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r6 * r4); u229_1v = rs ? rvv : n6; u229_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u229_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u229_0v)); pv7 = (1.0f - fmaxf(s1_sign(u229_0v), 0.0f) * s1_abs(s1_sign(u229_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u229_1v)); pv11 = (1.0f - fmaxf(s1_sign(u229_1v), 0.0f) * s1_abs(s1_sign(u229_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 230: 1106 75b2 6482 0007 85be eb9f */
    {
        float n6 = r6, n7 = r7;
        float u230_0v = 0.0f, u230_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (-0x1.6fbae7c000000p-6f * r6) + (r7); u230_0v = rs ? rvv : n7; u230_0z = rs ? rzd : n7; }
        float u230_1v = 0.0f, u230_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r2 * r2); u230_1v = rs ? rvv : n6; u230_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u230_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u230_0v)); pv7 = (1.0f - fmaxf(s1_sign(u230_0v), 0.0f) * s1_abs(s1_sign(u230_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u230_1v)); pv11 = (1.0f - fmaxf(s1_sign(u230_1v), 0.0f) * s1_abs(s1_sign(u230_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 231: 1108 74b2 0407 0036 1245 c842 2ccc cccc */
    {
        float n0 = r0, n7 = r7;
        float u231_0v = 0.0f, u231_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (0x1.245c842000000p-4f * r2) + (0x1.9999998000000p-1f); u231_0v = rs ? rvv : n7; u231_0z = rs ? rzd : n7; }
        float u231_1v = 0.0f, u231_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r7); u231_1v = rs ? rvv : n0; u231_1z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u231_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u231_0v)); pv7 = (1.0f - fmaxf(s1_sign(u231_0v), 0.0f) * s1_abs(s1_sign(u231_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u231_1v)); pv11 = (1.0f - fmaxf(s1_sign(u231_1v), 0.0f) * s1_abs(s1_sign(u231_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r7 = n7;
    }
    /* 232: 1407 75b4 4fe2 0007 0061 220e d6af */
    {
        float n4 = r4, n7 = r7;
        float u232_0v = 0.0f, u232_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (0x1.076b578000000p-3f * r6) + (r7); u232_0v = rs ? rvv : n7; u232_0z = rs ? rzd : n7; }
          n4 = c_061;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u232_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u232_0v)); pv7 = (1.0f - fmaxf(s1_sign(u232_0v), 0.0f) * s1_abs(s1_sign(u232_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r7 = n7;
    }
    /* 233: 0902 4507 */
    {
        float n4 = r4;
        float u233_0v = 0.0f, u233_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r4 * r7); u233_0v = rs ? rvv : n4; u233_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u233_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u233_0v)); pv7 = (1.0f - fmaxf(s1_sign(u233_0v), 0.0f) * s1_abs(s1_sign(u233_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 234: 1108 7532 6504 0036 8776 ca85 0000 0000 */
    {
        float n6 = r6, n7 = r7;
        float u234_0v = 0.0f, u234_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (-0x1.ddb2a14000000p-6f * r4) + (0x0.0p+0f); u234_0v = rs ? rvv : n7; u234_0z = rs ? rzd : n7; }
        float u234_1v = 0.0f, u234_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r4 * r4); u234_1v = rs ? rvv : n6; u234_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u234_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u234_0v)); pv7 = (1.0f - fmaxf(s1_sign(u234_0v), 0.0f) * s1_abs(s1_sign(u234_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u234_1v)); pv11 = (1.0f - fmaxf(s1_sign(u234_1v), 0.0f) * s1_abs(s1_sign(u234_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 235: 1106 75b2 6584 0007 83d8 0da8 */
    {
        float n6 = r6, n7 = r7;
        float u235_0v = 0.0f, u235_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (-0x1.ec06d40000000p-7f * r6) + (r7); u235_0v = rs ? rvv : n7; u235_0z = rs ? rzd : n7; }
        float u235_1v = 0.0f, u235_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r6 * r4); u235_1v = rs ? rvv : n6; u235_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u235_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u235_0v)); pv7 = (1.0f - fmaxf(s1_sign(u235_0v), 0.0f) * s1_abs(s1_sign(u235_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u235_1v)); pv11 = (1.0f - fmaxf(s1_sign(u235_1v), 0.0f) * s1_abs(s1_sign(u235_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 236: 1106 75b2 4503 0007 8036 468a */
    {
        float n4 = r4, n7 = r7;
        float u236_0v = 0.0f, u236_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (-0x1.b234500000000p-11f * r6) + (r7); u236_0v = rs ? rvv : n7; u236_0z = rs ? rzd : n7; }
        float u236_1v = 0.0f, u236_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r4 * r3); u236_1v = rs ? rvv : n4; u236_1z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u236_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u236_0v)); pv7 = (1.0f - fmaxf(s1_sign(u236_0v), 0.0f) * s1_abs(s1_sign(u236_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u236_1v)); pv11 = (1.0f - fmaxf(s1_sign(u236_1v), 0.0f) * s1_abs(s1_sign(u236_1z)));
        pm4 = 0.0f;
        r4 = n4;
        r7 = n7;
    }
    /* 237: 2e05 81c1 4503 7fe0 0062 */
    {
        float n0 = r0, n4 = r4, n7 = r7;
        float u237_0v = 0.0f, u237_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = r7 + r1 + r0; RES(n0); u237_0v = rs ? rvv : n0; u237_0z = rs ? rzd : n0; }
        float u237_1v = 0.0f, u237_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r4 * r3); u237_1v = rs ? rvv : n4; u237_1z = rs ? rzd : n4; }
          n7 = c_062;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u237_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u237_0v)); pv7 = (1.0f - fmaxf(s1_sign(u237_0v), 0.0f) * s1_abs(s1_sign(u237_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u237_1v)); pv11 = (1.0f - fmaxf(s1_sign(u237_1v), 0.0f) * s1_abs(s1_sign(u237_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r4 = n4;
        r7 = n7;
    }
    /* 238: 2e09 55f2 75f6 6fe0 0063 0000 0000 4200 0000 */
    {
        float n5 = r5, n6 = r6, n7 = r7;
        float u238_0v = 0.0f, u238_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r7 * 0x0.0p+0f); u238_0v = rs ? rvv : n5; u238_0z = rs ? rzd : n5; }
        float u238_1v = 0.0f, u238_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r7 * 0x1.0000000000000p+1f); u238_1v = rs ? rvv : n7; u238_1z = rs ? rzd : n7; }
          n6 = c_063;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u238_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u238_0v)); pv7 = (1.0f - fmaxf(s1_sign(u238_0v), 0.0f) * s1_abs(s1_sign(u238_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u238_1v)); pv11 = (1.0f - fmaxf(s1_sign(u238_1v), 0.0f) * s1_abs(s1_sign(u238_1z)));
        pm4 = 0.0f;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 239: 0803 5d4e f387 */
    {
        float n5 = r5;
        float u239_0v = 0.0f, u239_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (r6 * r5) + (r7) + (r6 * -r7); u239_0v = rs ? rvv : n5; u239_0z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u239_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u239_0v)); pv7 = (1.0f - fmaxf(s1_sign(u239_0v), 0.0f) * s1_abs(s1_sign(u239_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
    }
    /* 240: 0809 6db2 edba 4500 0000 4700 0000 4700 0000 */
    {
        float n6 = r6;
        float u240_0v = 0.0f, u240_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (0x1.4000000000000p+2f * r6) + (0x1.c000000000000p+2f) + (0x1.c000000000000p+2f * -r6); u240_0v = rs ? rvv : n6; u240_0z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u240_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u240_0v)); pv7 = (1.0f - fmaxf(s1_sign(u240_0v), 0.0f) * s1_abs(s1_sign(u240_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 241: 0807 7d0e ccb6 3aa3 d70a 3aa3 d70a */
    {
        float n7 = r7;
        float u241_0v = 0.0f, u241_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (r6 * r4) + (0x1.aa3d70a000000p+0f) + (0x1.aa3d70a000000p+0f * -r4); u241_0v = rs ? rvv : n7; u241_0z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u241_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u241_0v)); pv7 = (1.0f - fmaxf(s1_sign(u241_0v), 0.0f) * s1_abs(s1_sign(u241_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r7 = n7;
    }
    /* 242: 1804 7dc5 6fe0 0064 */
    {
        float n6 = r6, n7 = r7;
        float u242_0v = 0.0f, u242_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r7 + r5); u242_0v = rs ? rvv : n7; u242_0z = rs ? rzd : n7; }
          n6 = c_064;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u242_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u242_0v)); pv7 = (1.0f - fmaxf(s1_sign(u242_0v), 0.0f) * s1_abs(s1_sign(u242_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 243: 0804 758f 0012 3000 */
    {
        float n7 = r7;
        float u243_0v = 0.0f, u243_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (r7 * r6) + (0x1.0000000000000p+0f); u243_0v = rs ? rvv : n7; u243_0z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u243_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u243_0v)); pv7 = (1.0f - fmaxf(s1_sign(u243_0v), 0.0f) * s1_abs(s1_sign(u243_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r7 = n7;
    }
    /* 244: 1603 44c3 df07 */
    {
        float n4 = r4, n5 = r5, n6 = r6, n7 = r7;
        float u244_0v = 0.0f, u244_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r3 * r3); u244_0v = rs ? rvv : n4; u244_0z = rs ? rzd : n4; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = s1_sign(r7) * s1_recip(fmaxf(s1_abs(r7), 0x1p-9f)); n6 = 1.0f; n7 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u244_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u244_0v)); pv7 = (1.0f - fmaxf(s1_sign(u244_0v), 0.0f) * s1_abs(s1_sign(u244_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 245: 0902 5087 */
    {
        float n5 = r5, n6 = r6, n7 = r7;
        float u245_0v = 0.0f, u245_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = r5 * r6; n6 = 2 - r6 * r7; n7 = r6 * r7; u245_0v = rs ? rvv : n5; u245_0z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u245_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u245_0v)); pv7 = (1.0f - fmaxf(s1_sign(u245_0v), 0.0f) * s1_abs(s1_sign(u245_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 246: 0902 5087 */
    {
        float n5 = r5, n6 = r6, n7 = r7;
        float u246_0v = 0.0f, u246_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = r5 * r6; n6 = 2 - r6 * r7; n7 = r6 * r7; u246_0v = rs ? rvv : n5; u246_0z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u246_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u246_0v)); pv7 = (1.0f - fmaxf(s1_sign(u246_0v), 0.0f) * s1_abs(s1_sign(u246_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 247: 0902 5087 */
    {
        float n5 = r5, n6 = r6, n7 = r7;
        float u247_0v = 0.0f, u247_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = r5 * r6; n6 = 2 - r6 * r7; n7 = r6 * r7; u247_0v = rs ? rvv : n5; u247_0z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u247_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u247_0v)); pv7 = (1.0f - fmaxf(s1_sign(u247_0v), 0.0f) * s1_abs(s1_sign(u247_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 248: 2509 54a4 7503 dfe2 0027 0060 0f81 f143 bb05 */
    {
        float n5 = r5, n7 = r7;
        float u248_0v = 0.0f, u248_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (0x1.f03e200000000p-5f * r2) + (0x1.dd82800000000p+1f); u248_0v = rs ? rvv : n5; u248_0z = rs ? rzd : n5; }
        float u248_1v = 0.0f, u248_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r4 * r3); u248_1v = rs ? rvv : n7; u248_1z = rs ? rzd : n7; }
        c_060 = r5;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u248_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u248_0v)); pv7 = (1.0f - fmaxf(s1_sign(u248_0v), 0.0f) * s1_abs(s1_sign(u248_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u248_1v)); pv11 = (1.0f - fmaxf(s1_sign(u248_1v), 0.0f) * s1_abs(s1_sign(u248_1z)));
        pm4 = 0.0f;
        r5 = n5;
        r7 = n7;
    }
    /* 249: 0902 0c05 */
    {
        float n0 = r0;
        float u249_0v = 0.0f, u249_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + r5); u249_0v = rs ? rvv : n0; u249_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u249_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u249_0v)); pv7 = (1.0f - fmaxf(s1_sign(u249_0v), 0.0f) * s1_abs(s1_sign(u249_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 250: 1108 1432 6400 0036 43c9 1263 c435 586d */
    {
        float n1 = r1, n6 = r6;
        float u250_0v = 0.0f, u250_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (0x1.e489318000000p+1f * r0) + (-0x1.0d561b4000000p+2f); u250_0v = rs ? rvv : n1; u250_0z = rs ? rzd : n1; }
        float u250_1v = 0.0f, u250_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r0 * r0); u250_1v = rs ? rvv : n6; u250_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u250_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u250_0v)); pv7 = (1.0f - fmaxf(s1_sign(u250_0v), 0.0f) * s1_abs(s1_sign(u250_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u250_1v)); pv11 = (1.0f - fmaxf(s1_sign(u250_1v), 0.0f) * s1_abs(s1_sign(u250_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r6 = n6;
    }
    /* 251: 1108 15b2 6c36 0001 a6e4 d26f c466 6666 */
    {
        float n1 = r1, n6 = r6;
        float u251_0v = 0.0f, u251_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (-0x1.b9349bc000000p-2f * r6) + (r1); u251_0v = rs ? rvv : n1; u251_0z = rs ? rzd : n1; }
        float u251_1v = 0.0f, u251_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r0 + -0x1.1999998000000p+2f); u251_1v = rs ? rvv : n6; u251_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u251_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u251_0v)); pv7 = (1.0f - fmaxf(s1_sign(u251_0v), 0.0f) * s1_abs(s1_sign(u251_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u251_1v)); pv11 = (1.0f - fmaxf(s1_sign(u251_1v), 0.0f) * s1_abs(s1_sign(u251_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r6 = n6;
    }
    /* 252: 0806 6412 0034 0000 441b 22d0 */
    {
        float n6 = r6;
        float u252_0v = 0.0f, u252_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (0x0.0p+0f * r0) + (0x1.06c8b40000000p+2f); u252_0v = rs ? rvv : n6; u252_0z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u252_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u252_0v)); pv7 = (1.0f - fmaxf(s1_sign(u252_0v), 0.0f) * s1_abs(s1_sign(u252_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 253: 2d06 1046 6c30 1610 c40e bedf */
    {
        float n1 = r1, n6 = r6;
        float u253_0v = 0.0f, u253_0z = 0.0f;
        { float _w = (1.0f - f11); float o1 = n1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = r6; u253_0v = rs ? rvv : n1; u253_0z = rs ? rzd : n1; }
          n1 = o1 + _w * (n1 - o1); }
        float u253_1v = 0.0f, u253_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r0 + -0x1.03afb7c000000p+2f); u253_1v = rs ? rvv : n6; u253_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f11); pv4 = s1_abs(s1_sign(u253_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u253_0v)); pv7 = (1.0f - fmaxf(s1_sign(u253_0v), 0.0f) * s1_abs(s1_sign(u253_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u253_1v)); pv11 = (1.0f - fmaxf(s1_sign(u253_1v), 0.0f) * s1_abs(s1_sign(u253_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r6 = n6;
    }
    /* 254: 0805 6412 0014 3000 0000 */
    {
        float n6 = r6;
        float u254_0v = 0.0f, u254_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (0x1.0000000000000p+0f * r0) + (0x0.0p+0f); u254_0v = rs ? rvv : n6; u254_0z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u254_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u254_0v)); pv7 = (1.0f - fmaxf(s1_sign(u254_0v), 0.0f) * s1_abs(s1_sign(u254_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 255: 480a 64f2 0046 0041 0a23 0036 02bd 3c36 2fe5 6041 */
    {
        float n0 = r0, n6 = r6;
        float u255_0v = 0.0f, u255_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (0x1.5e9e1b0000000p-7f * r3) + (0x1.fcac082000000p-1f); u255_0v = rs ? rvv : n6; u255_0z = rs ? rzd : n6; }
        float u255_1v = 0.0f, u255_1z = 0.0f;
        { float _w = (1.0f - f5); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = r6; u255_1v = rs ? rvv : n0; u255_1z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        { float _w = f5; float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = r1; }
          n0 = o0 + _w * (n0 - o0); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u255_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u255_0v)); pv7 = (1.0f - fmaxf(s1_sign(u255_0v), 0.0f) * s1_abs(s1_sign(u255_0z)));
        pm2 = (1.0f - f5); pv9 = fminf(1.0f, 1.0f + s1_sign(u255_1v)); pv11 = (1.0f - fmaxf(s1_sign(u255_1v), 0.0f) * s1_abs(s1_sign(u255_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r6 = n6;
    }
    /* 256: 0805 6532 0006 81b0 89a0 */
    {
        float n6 = r6;
        float u256_0v = 0.0f, u256_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (-0x1.b089a00000000p-8f * r4) + (r6); u256_0v = rs ? rvv : n6; u256_0z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u256_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u256_0v)); pv7 = (1.0f - fmaxf(s1_sign(u256_0v), 0.0f) * s1_abs(s1_sign(u256_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 257: 0805 65f2 0006 009d 4951 */
    {
        float n6 = r6;
        float u257_0v = 0.0f, u257_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (0x1.3a92a20000000p-9f * r7) + (r6); u257_0v = rs ? rvv : n6; u257_0z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u257_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u257_0v)); pv7 = (1.0f - fmaxf(s1_sign(u257_0v), 0.0f) * s1_abs(s1_sign(u257_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 258: 100c 54fa 14e4 003e 0027 d90e 3648 0000 0194 237f 2fe5 6041 */
    {
        float n1 = r1, n5 = r5;
        float u258_0v = 0.0f, u258_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (0x1.94237f0000000p-8f * r3) + (0x1.fcac082000000p-1f); u258_0v = rs ? rvv : n5; u258_0z = rs ? rzd : n5; }
        float u258_1v = 0.0f, u258_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (-0x1.90e3600000000p+4f * r3) + (0x1.0000000000000p+3f); u258_1v = rs ? rvv : n1; u258_1z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u258_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u258_0v)); pv7 = (1.0f - fmaxf(s1_sign(u258_0v), 0.0f) * s1_abs(s1_sign(u258_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u258_1v)); pv11 = (1.0f - fmaxf(s1_sign(u258_1v), 0.0f) * s1_abs(s1_sign(u258_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r5 = n5;
    }
    /* 259: 1009 5537 1524 0005 0001 5b1c 6980 c49b a500 */
    {
        float n1 = r1, n5 = r5;
        float u259_0v = 0.0f, u259_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (-0x1.89374a0000000p-9f * r4) + (r5); u259_0v = rs ? rvv : n5; u259_0z = rs ? rzd : n5; }
        float u259_1v = 0.0f, u259_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (0x1.b1c6900000000p+4f * r4) + (r1); u259_1v = rs ? rvv : n1; u259_1z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u259_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u259_0v)); pv7 = (1.0f - fmaxf(s1_sign(u259_0v), 0.0f) * s1_abs(s1_sign(u259_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u259_1v)); pv11 = (1.0f - fmaxf(s1_sign(u259_1v), 0.0f) * s1_abs(s1_sign(u259_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r5 = n5;
    }
    /* 260: 1009 55f7 15e4 0005 0001 c9ff fa00 5761 9f00 */
    {
        float n1 = r1, n5 = r5;
        float u260_0v = 0.0f, u260_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (0x1.5d867c0000000p-10f * r7) + (r5); u260_0v = rs ? rvv : n5; u260_0z = rs ? rzd : n5; }
        float u260_1v = 0.0f, u260_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (-0x1.3fff400000000p+3f * r7) + (r1); u260_1v = rs ? rvv : n1; u260_1z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u260_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u260_0v)); pv7 = (1.0f - fmaxf(s1_sign(u260_0v), 0.0f) * s1_abs(s1_sign(u260_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u260_1v)); pv11 = (1.0f - fmaxf(s1_sign(u260_1v), 0.0f) * s1_abs(s1_sign(u260_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r5 = n5;
    }
    /* 261: 0803 5d8a d285 */
    {
        float n5 = r5;
        float u261_0v = 0.0f, u261_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (r2 * r6) + (r5) + (r2 * -r5); u261_0v = rs ? rvv : n5; u261_0z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u261_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u261_0v)); pv7 = (1.0f - fmaxf(s1_sign(u261_0v), 0.0f) * s1_abs(s1_sign(u261_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
    }
    /* 262: 0806 64e2 0025 d595 5548 0000 */
    {
        float n6 = r6;
        float u262_0v = 0.0f, u262_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (-0x1.5955500000000p+4f * r3) + (0x1.0000000000000p+3f); u262_0v = rs ? rvv : n6; u262_0z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u262_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u262_0v)); pv7 = (1.0f - fmaxf(s1_sign(u262_0v), 0.0f) * s1_abs(s1_sign(u262_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 263: 1106 6522 0405 0006 5519 9900 */
    {
        float n0 = r0, n6 = r6;
        float u263_0v = 0.0f, u263_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (0x1.5199900000000p+4f * r4) + (r6); u263_0v = rs ? rvv : n6; u263_0z = rs ? rzd : n6; }
        float u263_1v = 0.0f, u263_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r5); u263_1v = rs ? rvv : n0; u263_1z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u263_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u263_0v)); pv7 = (1.0f - fmaxf(s1_sign(u263_0v), 0.0f) * s1_abs(s1_sign(u263_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u263_1v)); pv11 = (1.0f - fmaxf(s1_sign(u263_1v), 0.0f) * s1_abs(s1_sign(u263_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r6 = n6;
    }
    /* 264: 1407 65e4 5fe2 0006 0063 c711 1100 */
    {
        float n5 = r5, n6 = r6;
        float u264_0v = 0.0f, u264_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (-0x1.c444400000000p+2f * r7) + (r6); u264_0v = rs ? rvv : n6; u264_0z = rs ? rzd : n6; }
          n5 = c_063;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u264_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u264_0v)); pv7 = (1.0f - fmaxf(s1_sign(u264_0v), 0.0f) * s1_abs(s1_sign(u264_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
        r6 = n6;
    }
    /* 265: 1405 1c4d 5fe2 e346 0060 */
    {
        float n1 = r1, n5 = r5;
        float u265_0v = 0.0f, u265_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (r5 * r1) + (r6) + (r5 * -r6); u265_0v = rs ? rvv : n1; u265_0z = rs ? rzd : n1; }
          n5 = c_060;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u265_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u265_0v)); pv7 = (1.0f - fmaxf(s1_sign(u265_0v), 0.0f) * s1_abs(s1_sign(u265_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r5 = n5;
    }
    /* 266: 1108 3432 6400 0036 cced 9029 5801 dac1 */
    {
        float n3 = r3, n6 = r6;
        float u266_0v = 0.0f, u266_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (-0x1.9db2052000000p+3f * r0) + (0x1.801dac1000000p+4f); u266_0v = rs ? rvv : n3; u266_0z = rs ? rzd : n3; }
        float u266_1v = 0.0f, u266_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r0 * r0); u266_1v = rs ? rvv : n6; u266_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u266_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u266_0v)); pv7 = (1.0f - fmaxf(s1_sign(u266_0v), 0.0f) * s1_abs(s1_sign(u266_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u266_1v)); pv11 = (1.0f - fmaxf(s1_sign(u266_1v), 0.0f) * s1_abs(s1_sign(u266_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r6 = n6;
    }
    /* 267: 1108 35b2 2c36 0003 3bd9 415e c417 79a6 */
    {
        float n2 = r2, n3 = r3;
        float u267_0v = 0.0f, u267_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (0x1.bd9415e000000p+0f * r6) + (r3); u267_0v = rs ? rvv : n3; u267_0z = rs ? rzd : n3; }
        float u267_1v = 0.0f, u267_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r0 + -0x1.05de698000000p+2f); u267_1v = rs ? rvv : n2; u267_1z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u267_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u267_0v)); pv7 = (1.0f - fmaxf(s1_sign(u267_0v), 0.0f) * s1_abs(s1_sign(u267_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u267_1v)); pv11 = (1.0f - fmaxf(s1_sign(u267_1v), 0.0f) * s1_abs(s1_sign(u267_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 268: 1109 4434 34d2 0038 4800 0000 0000 3fff 2d2f */
    {
        float n3 = r3, n4 = r4;
        float u268_0v = 0.0f, u268_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (0x0.0p+0f * r0) + (0x1.fff2d2f000000p+0f); u268_0v = rs ? rvv : n4; u268_0z = rs ? rzd : n4; }
        float u268_1v = 0.0f, u268_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p+3f); u268_1v = rs ? rvv : n3; u268_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u268_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u268_0v)); pv7 = (1.0f - fmaxf(s1_sign(u268_0v), 0.0f) * s1_abs(s1_sign(u268_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u268_1v)); pv11 = (1.0f - fmaxf(s1_sign(u268_1v), 0.0f) * s1_abs(s1_sign(u268_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r4 = n4;
    }
    /* 269: 5e08 3044 2c32 1610 6fe0 005f c3b6 c8b4 */
    {
        float n2 = r2, n3 = r3, n6 = r6;
        float u269_0v = 0.0f, u269_0z = 0.0f;
        { float _w = (1.0f - f11); float o3 = n3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = r4; u269_0v = rs ? rvv : n3; u269_0z = rs ? rzd : n3; }
          n3 = o3 + _w * (n3 - o3); }
        float u269_1v = 0.0f, u269_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r0 + -0x1.db645a0000000p+1f); u269_1v = rs ? rvv : n2; u269_1z = rs ? rzd : n2; }
          n6 = c_05f;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f11); pv4 = s1_abs(s1_sign(u269_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u269_0v)); pv7 = (1.0f - fmaxf(s1_sign(u269_0v), 0.0f) * s1_abs(s1_sign(u269_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u269_1v)); pv11 = (1.0f - fmaxf(s1_sign(u269_1v), 0.0f) * s1_abs(s1_sign(u269_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
        r6 = n6;
    }
    /* 270: 1108 4432 1445 0036 05a1 19db 9210 60a0 */
    {
        float n1 = r1, n4 = r4;
        float u270_0v = 0.0f, u270_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (0x1.684676c000000p-6f * r0) + (-0x1.21060a0000000p-4f); u270_0v = rs ? rvv : n4; u270_0z = rs ? rzd : n4; }
        float u270_1v = 0.0f, u270_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r5); u270_1v = rs ? rvv : n1; u270_1z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u270_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u270_0v)); pv7 = (1.0f - fmaxf(s1_sign(u270_0v), 0.0f) * s1_abs(s1_sign(u270_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u270_1v)); pv11 = (1.0f - fmaxf(s1_sign(u270_1v), 0.0f) * s1_abs(s1_sign(u270_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r4 = n4;
    }
    /* 271: 5905 0044 0043 1041 0a12 */
    {
        float n0 = r0, n1 = r1;
        float u271_0v = 0.0f, u271_0z = 0.0f;
        { float _w = (1.0f - f5); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = r4; u271_0v = rs ? rvv : n0; u271_0z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        float u271_1v = 0.0f, u271_1z = 0.0f;
        { float _w = f5; float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = r3; u271_1v = rs ? rvv : n0; u271_1z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = r1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f5); pv4 = s1_abs(s1_sign(u271_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u271_0v)); pv7 = (1.0f - fmaxf(s1_sign(u271_0v), 0.0f) * s1_abs(s1_sign(u271_0z)));
        pm2 = f5; pv9 = fminf(1.0f, 1.0f + s1_sign(u271_1v)); pv11 = (1.0f - fmaxf(s1_sign(u271_1v), 0.0f) * s1_abs(s1_sign(u271_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 272: 1e05 8fe0 9fe2 005b 005e */
    {
        c_05b = r0;
        c_05e = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 273: 0c03 0fe0 0033 */
    {
        float n0 = r0;
          n0 = c_033;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 274: 0904 1430 2b17 217f */
    {
        float n1 = r1;
        float u274_0v = 0.0f, u274_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r0 * 0x1.62e42fe000000p-1f); u274_0v = rs ? rvv : n1; u274_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u274_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u274_0v)); pv7 = (1.0f - fmaxf(s1_sign(u274_0v), 0.0f) * s1_abs(s1_sign(u274_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 275: 0a02 8f19 */
    {
        float n0 = r0, n1 = r1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; { float _f = s1_floor(r1); n0 = s1_exp(_f); n1 = r1 - _f; } }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 276: 1108 3472 20a1 0036 0001 ae64 0012 7e4f */
    {
        float n2 = r2, n3 = r3;
        float u276_0v = 0.0f, u276_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (0x1.ae64000000000p-16f * r1) + (0x1.27e4f00000000p-12f); u276_0v = rs ? rvv : n3; u276_0z = rs ? rzd : n3; }
        float u276_1v = 0.0f, u276_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = 0.25f * r1 * r1; u276_1v = rs ? rvv : n2; u276_1z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u276_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u276_0v)); pv7 = (1.0f - fmaxf(s1_sign(u276_0v), 0.0f) * s1_abs(s1_sign(u276_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u276_1v)); pv11 = (1.0f - fmaxf(s1_sign(u276_1v), 0.0f) * s1_abs(s1_sign(u276_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 277: 0807 3cca 1cb6 002e 3bc7 01a0 1a01 */
    {
        float n3 = r3;
        float u277_0v = 0.0f, u277_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.a01a010000000p-8f) + (0x1.71de380000000p-11f * r1); u277_0v = rs ? rvv : n3; u277_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u277_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u277_0v)); pv7 = (1.0f - fmaxf(s1_sign(u277_0v), 0.0f) * s1_abs(s1_sign(u277_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 278: 0807 3cca 1cb6 0340 3403 16c1 6c16 */
    {
        float n3 = r3;
        float u278_0v = 0.0f, u278_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.6c16c16000000p-4f) + (0x1.a01a018000000p-7f * r1); u278_0v = rs ? rvv : n3; u278_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u278_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u278_0v)); pv7 = (1.0f - fmaxf(s1_sign(u278_0v), 0.0f) * s1_abs(s1_sign(u278_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 279: 0807 3cca 1cb6 2222 2222 2aaa aaaa */
    {
        float n3 = r3;
        float u279_0v = 0.0f, u279_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.5555554000000p-1f) + (0x1.1111110000000p-3f * r1); u279_0v = rs ? rvv : n3; u279_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u279_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u279_0v)); pv7 = (1.0f - fmaxf(s1_sign(u279_0v), 0.0f) * s1_abs(s1_sign(u279_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 280: 0807 3cca 1d62 4200 002a aaaa aa00 */
    {
        float n3 = r3;
        float u280_0v = 0.0f, u280_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.0000000000000p+1f) + (0x1.5555554000000p-1f * r1); u280_0v = rs ? rvv : n3; u280_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u280_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u280_0v)); pv7 = (1.0f - fmaxf(s1_sign(u280_0v), 0.0f) * s1_abs(s1_sign(u280_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 281: 0806 3cca 1912 3000 3000 0000 */
    {
        float n3 = r3;
        float u281_0v = 0.0f, u281_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.0000000000000p+0f) + (0x1.0000000000000p+0f * r1); u281_0v = rs ? rvv : n3; u281_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u281_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u281_0v)); pv7 = (1.0f - fmaxf(s1_sign(u281_0v), 0.0f) * s1_abs(s1_sign(u281_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 282: 0902 0403 */
    {
        float n0 = r0;
        float u282_0v = 0.0f, u282_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r3); u282_0v = rs ? rvv : n0; u282_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u282_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u282_0v)); pv7 = (1.0f - fmaxf(s1_sign(u282_0v), 0.0f) * s1_abs(s1_sign(u282_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 283: 0904 0420 0231 1900 */
    {
        float n0 = r0;
        float u283_0v = 0.0f, u283_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * 0x1.188c800000000p-7f); u283_0v = rs ? rvv : n0; u283_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u283_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u283_0v)); pv7 = (1.0f - fmaxf(s1_sign(u283_0v), 0.0f) * s1_abs(s1_sign(u283_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 284: 0902 1400 */
    {
        float n1 = r1;
        float u284_0v = 0.0f, u284_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r0 * r0); u284_0v = rs ? rvv : n1; u284_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u284_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u284_0v)); pv7 = (1.0f - fmaxf(s1_sign(u284_0v), 0.0f) * s1_abs(s1_sign(u284_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 285: 1107 5462 2440 0025 0000 0000 002e */
    {
        float n2 = r2, n5 = r5;
        float u285_0v = 0.0f, u285_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (0x0.0p+0f * r1) + (0x1.7000000000000p-19f); u285_0v = rs ? rvv : n5; u285_0z = rs ? rzd : n5; }
        float u285_1v = 0.0f, u285_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r1 * r0); u285_1v = rs ? rvv : n2; u285_1z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u285_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u285_0v)); pv7 = (1.0f - fmaxf(s1_sign(u285_0v), 0.0f) * s1_abs(s1_sign(u285_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u285_1v)); pv11 = (1.0f - fmaxf(s1_sign(u285_1v), 0.0f) * s1_abs(s1_sign(u285_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r5 = n5;
    }
    /* 286: 1107 4462 3441 0025 800d 0002 2222 */
    {
        float n3 = r3, n4 = r4;
        float u286_0v = 0.0f, u286_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (-0x1.a000000000000p-13f * r1) + (0x1.1111000000000p-7f); u286_0v = rs ? rvv : n4; u286_0z = rs ? rzd : n4; }
        float u286_1v = 0.0f, u286_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r1 * r1); u286_1v = rs ? rvv : n3; u286_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u286_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u286_0v)); pv7 = (1.0f - fmaxf(s1_sign(u286_0v), 0.0f) * s1_abs(s1_sign(u286_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u286_1v)); pv11 = (1.0f - fmaxf(s1_sign(u286_1v), 0.0f) * s1_abs(s1_sign(u286_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r4 = n4;
    }
    /* 287: 1503 58c4 4442 */
    {
        float n4 = r4, n5 = r5;
        float u287_0v = 0.0f, u287_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r4 + r3 * r5); u287_0v = rs ? rvv : n5; u287_0z = rs ? rzd : n5; }
        float u287_1v = 0.0f, u287_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r1 * r2); u287_1v = rs ? rvv : n4; u287_1z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u287_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u287_0v)); pv7 = (1.0f - fmaxf(s1_sign(u287_0v), 0.0f) * s1_abs(s1_sign(u287_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u287_1v)); pv11 = (1.0f - fmaxf(s1_sign(u287_1v), 0.0f) * s1_abs(s1_sign(u287_1z)));
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
    }
    /* 288: 0805 5d4c 2880 a2aa aa00 */
    {
        float n5 = r5;
        float u288_0v = 0.0f, u288_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (r4 * r5) + (r0) + (-0x1.5555000000000p-3f * r2); u288_0v = rs ? rvv : n5; u288_0z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u288_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u288_0v)); pv7 = (1.0f - fmaxf(s1_sign(u288_0v), 0.0f) * s1_abs(s1_sign(u288_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
    }
    /* 289: 0903 0550 4200 */
    {
        float n0 = r0;
        float u289_0v = 0.0f, u289_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r5 * 0x1.0000000000000p+1f); u289_0v = rs ? rvv : n0; u289_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u289_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u289_0v)); pv7 = (1.0f - fmaxf(s1_sign(u289_0v), 0.0f) * s1_abs(s1_sign(u289_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 290: 0c03 8fe0 0065 */
    {
        c_065 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 291: 1e05 0fe0 1fe2 0065 005b */
    {
        float n0 = r0, n1 = r1;
          n0 = c_065;
          n1 = c_05b;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 292: 0c03 2fe0 0056 */
    {
        float n2 = r2;
          n2 = c_056;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 293: 0803 0c0a 9281 */
    {
        float n0 = r0;
        float u293_0v = 0.0f, u293_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r2 * r0) + (r1) + (r2 * -r1); u293_0v = rs ? rvv : n0; u293_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u293_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u293_0v)); pv7 = (1.0f - fmaxf(s1_sign(u293_0v), 0.0f) * s1_abs(s1_sign(u293_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 294: 0c03 8fe0 0066 */
    {
        c_066 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 295: 1e05 0fe0 1fe2 0066 005e */
    {
        float n0 = r0, n1 = r1;
          n0 = c_066;
          n1 = c_05e;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 296: 1506 4c32 7e10 4200 0000 0002 */
    {
        float n4 = r4, n7 = r7;
        float u296_0v = 0.0f, u296_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r0 + 0x1.0000000000000p-31f); u296_0v = rs ? rvv : n4; u296_0z = rs ? rzd : n4; }
        float u296_1v = 0.0f, u296_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(0x1.0000000000000p+1f - r0); u296_1v = rs ? rvv : n7; u296_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u296_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u296_0v)); pv7 = (1.0f - fmaxf(s1_sign(u296_0v), 0.0f) * s1_abs(s1_sign(u296_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u296_1v)); pv11 = (1.0f - fmaxf(s1_sign(u296_1v), 0.0f) * s1_abs(s1_sign(u296_1z)));
        pm4 = 0.0f;
        r4 = n4;
        r7 = n7;
    }
    /* 297: 0a02 af04 */
    {
        float n2 = r2, n3 = r3, n4 = r4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = s1_sign(r4) * s1_recip(fmaxf(s1_abs(r4), 0x1p-9f)); n3 = 1.0f; n4 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
        r4 = n4;
    }
    /* 298: 0902 2084 */
    {
        float n2 = r2, n3 = r3, n4 = r4;
        float u298_0v = 0.0f, u298_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = r2 * r3; n3 = 2 - r3 * r4; n4 = r3 * r4; u298_0v = rs ? rvv : n2; u298_0z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u298_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u298_0v)); pv7 = (1.0f - fmaxf(s1_sign(u298_0v), 0.0f) * s1_abs(s1_sign(u298_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
        r4 = n4;
    }
    /* 299: 0902 2084 */
    {
        float n2 = r2, n3 = r3, n4 = r4;
        float u299_0v = 0.0f, u299_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = r2 * r3; n3 = 2 - r3 * r4; n4 = r3 * r4; u299_0v = rs ? rvv : n2; u299_0z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u299_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u299_0v)); pv7 = (1.0f - fmaxf(s1_sign(u299_0v), 0.0f) * s1_abs(s1_sign(u299_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
        r4 = n4;
    }
    /* 300: 0902 2084 */
    {
        float n2 = r2, n3 = r3, n4 = r4;
        float u300_0v = 0.0f, u300_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = r2 * r3; n3 = 2 - r3 * r4; n4 = r3 * r4; u300_0v = rs ? rvv : n2; u300_0z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u300_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u300_0v)); pv7 = (1.0f - fmaxf(s1_sign(u300_0v), 0.0f) * s1_abs(s1_sign(u300_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
        r4 = n4;
    }
    /* 301: 1505 4c10 5c12 b000 c200 */
    {
        float n4 = r4, n5 = r5;
        float u301_0v = 0.0f, u301_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r0 + -0x1.0000000000000p+0f); u301_0v = rs ? rvv : n4; u301_0z = rs ? rzd : n4; }
        float u301_1v = 0.0f, u301_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r0 + -0x1.0000000000000p+1f); u301_1v = rs ? rvv : n5; u301_1z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u301_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u301_0v)); pv7 = (1.0f - fmaxf(s1_sign(u301_0v), 0.0f) * s1_abs(s1_sign(u301_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u301_1v)); pv11 = (1.0f - fmaxf(s1_sign(u301_1v), 0.0f) * s1_abs(s1_sign(u301_1z)));
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
    }
    /* 302: 0001 */
    {
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 303: 1703 7042 0a10 */
    {
        float n7 = r7;
        float u303_0v = 0.0f, u303_0z = 0.0f;
        { float _w = (1.0f - f5); float o7 = n7;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = r2; u303_0v = rs ? rvv : n7; u303_0z = rs ? rzd : n7; }
          n7 = o7 + _w * (n7 - o7); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f5); pv4 = s1_abs(s1_sign(u303_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u303_0v)); pv7 = (1.0f - fmaxf(s1_sign(u303_0v), 0.0f) * s1_abs(s1_sign(u303_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r7 = n7;
    }
    /* 304: 1704 7050 1310 0000 */
    {
        float n7 = r7;
        float u304_0v = 0.0f, u304_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f9)); float o7 = n7;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x0.0p+0f; u304_0v = rs ? rvv : n7; u304_0z = rs ? rzd : n7; }
          n7 = o7 + _w * (n7 - o7); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f9)); pv4 = s1_abs(s1_sign(u304_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u304_0v)); pv7 = (1.0f - fmaxf(s1_sign(u304_0v), 0.0f) * s1_abs(s1_sign(u304_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r7 = n7;
    }
    /* 305: 0902 3c4f */
    {
        float n3 = r3;
        float u305_0v = 0.0f, u305_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r1 + -r7); u305_0v = rs ? rvv : n3; u305_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u305_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u305_0v)); pv7 = (1.0f - fmaxf(s1_sign(u305_0v), 0.0f) * s1_abs(s1_sign(u305_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 306: 0001 */
    {
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 307: 1703 1047 0b10 */
    {
        float n1 = r1;
        float u307_0v = 0.0f, u307_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o1 = n1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = r7; u307_0v = rs ? rvv : n1; u307_0z = rs ? rzd : n1; }
          n1 = o1 + _w * (n1 - o1); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u307_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u307_0v)); pv7 = (1.0f - fmaxf(s1_sign(u307_0v), 0.0f) * s1_abs(s1_sign(u307_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 308: 1e05 8fe0 9fe2 0067 0068 */
    {
        c_067 = r0;
        c_068 = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 309: 0a02 ef75 */
    {
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd;  }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 310: 1e05 0fe0 1fe2 0033 004f */
    {
        float n0 = r0, n1 = r1;
          n0 = c_033;
          n1 = c_04f;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 311: 1e05 2fe0 3fe2 001b 00bf */
    {
        float n2 = r2, n3 = r3;
          n2 = c_01b;
          n3 = c_0bf;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 312: 0902 8042 */
    {
        float n0 = r0;
        float u312_0v = 0.0f, u312_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = r1 + r2 + r0; RES(n0); u312_0v = rs ? rvv : n0; u312_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u312_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u312_0v)); pv7 = (1.0f - fmaxf(s1_sign(u312_0v), 0.0f) * s1_abs(s1_sign(u312_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 313: 0902 0c03 */
    {
        float n0 = r0;
        float u313_0v = 0.0f, u313_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + r3); u313_0v = rs ? rvv : n0; u313_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u313_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u313_0v)); pv7 = (1.0f - fmaxf(s1_sign(u313_0v), 0.0f) * s1_abs(s1_sign(u313_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 314: 0c03 8fe0 0069 */
    {
        c_069 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 315: 1506 2c20 3c23 ca00 0042 0000 */
    {
        float n2 = r2, n3 = r3;
        float u315_0v = 0.0f, u315_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r0 + -0x1.4000000000000p+3f); u315_0v = rs ? rvv : n2; u315_0z = rs ? rzd : n2; }
        float u315_1v = 0.0f, u315_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r0 + 0x1.0000000000000p+1f); u315_1v = rs ? rvv : n3; u315_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u315_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u315_0v)); pv7 = (1.0f - fmaxf(s1_sign(u315_0v), 0.0f) * s1_abs(s1_sign(u315_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u315_1v)); pv11 = (1.0f - fmaxf(s1_sign(u315_1v), 0.0f) * s1_abs(s1_sign(u315_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 316: 1506 2060 3063 ca00 0042 0000 */
    {
        float n2 = r2, n3 = r3;
        float u316_0v = 0.0f, u316_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = -0x1.4000000000000p+3f; u316_0v = rs ? rvv : n2; u316_0z = rs ? rzd : n2; }
        float u316_1v = 0.0f, u316_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = 0x1.0000000000000p+1f; u316_1v = rs ? rvv : n3; u316_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u316_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u316_0v)); pv7 = (1.0f - fmaxf(s1_sign(u316_0v), 0.0f) * s1_abs(s1_sign(u316_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u316_1v)); pv11 = (1.0f - fmaxf(s1_sign(u316_1v), 0.0f) * s1_abs(s1_sign(u316_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 317: 5d05 004a 004b 0e10 1220 */
    {
        float n0 = r0;
        float u317_0v = 0.0f, u317_0z = 0.0f;
        { float _w = (1.0f - f7); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -r2; u317_0v = rs ? rvv : n0; u317_0z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        float u317_1v = 0.0f, u317_1z = 0.0f;
        { float _w = (1.0f - f9); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -r3; u317_1v = rs ? rvv : n0; u317_1z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f7); pv4 = s1_abs(s1_sign(u317_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u317_0v)); pv7 = (1.0f - fmaxf(s1_sign(u317_0v), 0.0f) * s1_abs(s1_sign(u317_0z)));
        pm2 = (1.0f - f9); pv9 = fminf(1.0f, 1.0f + s1_sign(u317_1v)); pv11 = (1.0f - fmaxf(s1_sign(u317_1v), 0.0f) * s1_abs(s1_sign(u317_1z)));
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 318: 0c03 8fe0 006a */
    {
        c_06a = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 319: 0904 0c20 c4c0 0000 */
    {
        float n0 = r0;
        float u319_0v = 0.0f, u319_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + -0x1.3000000000000p+2f); u319_0v = rs ? rvv : n0; u319_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u319_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u319_0v)); pv7 = (1.0f - fmaxf(s1_sign(u319_0v), 0.0f) * s1_abs(s1_sign(u319_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 320: 0904 1430 2b17 217f */
    {
        float n1 = r1;
        float u320_0v = 0.0f, u320_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r0 * 0x1.62e42fe000000p-1f); u320_0v = rs ? rvv : n1; u320_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u320_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u320_0v)); pv7 = (1.0f - fmaxf(s1_sign(u320_0v), 0.0f) * s1_abs(s1_sign(u320_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 321: 0a02 8f19 */
    {
        float n0 = r0, n1 = r1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; { float _f = s1_floor(r1); n0 = s1_exp(_f); n1 = r1 - _f; } }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 322: 1108 3472 20a1 0036 0001 ae64 0012 7e4f */
    {
        float n2 = r2, n3 = r3;
        float u322_0v = 0.0f, u322_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (0x1.ae64000000000p-16f * r1) + (0x1.27e4f00000000p-12f); u322_0v = rs ? rvv : n3; u322_0z = rs ? rzd : n3; }
        float u322_1v = 0.0f, u322_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = 0.25f * r1 * r1; u322_1v = rs ? rvv : n2; u322_1z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u322_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u322_0v)); pv7 = (1.0f - fmaxf(s1_sign(u322_0v), 0.0f) * s1_abs(s1_sign(u322_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u322_1v)); pv11 = (1.0f - fmaxf(s1_sign(u322_1v), 0.0f) * s1_abs(s1_sign(u322_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 323: 0807 3cca 1cb6 002e 3bc7 01a0 1a01 */
    {
        float n3 = r3;
        float u323_0v = 0.0f, u323_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.a01a010000000p-8f) + (0x1.71de380000000p-11f * r1); u323_0v = rs ? rvv : n3; u323_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u323_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u323_0v)); pv7 = (1.0f - fmaxf(s1_sign(u323_0v), 0.0f) * s1_abs(s1_sign(u323_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 324: 0807 3cca 1cb6 0340 3403 16c1 6c16 */
    {
        float n3 = r3;
        float u324_0v = 0.0f, u324_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.6c16c16000000p-4f) + (0x1.a01a018000000p-7f * r1); u324_0v = rs ? rvv : n3; u324_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u324_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u324_0v)); pv7 = (1.0f - fmaxf(s1_sign(u324_0v), 0.0f) * s1_abs(s1_sign(u324_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 325: 0807 3cca 1cb6 2222 2222 2aaa aaaa */
    {
        float n3 = r3;
        float u325_0v = 0.0f, u325_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.5555554000000p-1f) + (0x1.1111110000000p-3f * r1); u325_0v = rs ? rvv : n3; u325_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u325_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u325_0v)); pv7 = (1.0f - fmaxf(s1_sign(u325_0v), 0.0f) * s1_abs(s1_sign(u325_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 326: 0807 3cca 1d62 4200 002a aaaa aa00 */
    {
        float n3 = r3;
        float u326_0v = 0.0f, u326_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.0000000000000p+1f) + (0x1.5555554000000p-1f * r1); u326_0v = rs ? rvv : n3; u326_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u326_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u326_0v)); pv7 = (1.0f - fmaxf(s1_sign(u326_0v), 0.0f) * s1_abs(s1_sign(u326_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 327: 0806 3cca 1912 3000 3000 0000 */
    {
        float n3 = r3;
        float u327_0v = 0.0f, u327_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.0000000000000p+0f) + (0x1.0000000000000p+0f * r1); u327_0v = rs ? rvv : n3; u327_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u327_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u327_0v)); pv7 = (1.0f - fmaxf(s1_sign(u327_0v), 0.0f) * s1_abs(s1_sign(u327_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 328: 0902 0403 */
    {
        float n0 = r0;
        float u328_0v = 0.0f, u328_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r3); u328_0v = rs ? rvv : n0; u328_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u328_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u328_0v)); pv7 = (1.0f - fmaxf(s1_sign(u328_0v), 0.0f) * s1_abs(s1_sign(u328_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 329: 0c03 8fe0 006b */
    {
        c_06b = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 330: 1e05 0fe0 1fe2 006b 0058 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_06b;
          n1 = c_058;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 331: 0904 0430 0096 2fc9 */
    {
        float n0 = r0;
        float u331_0v = 0.0f, u331_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * 0x1.2c5f920000000p-9f); u331_0v = rs ? rvv : n0; u331_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u331_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u331_0v)); pv7 = (1.0f - fmaxf(s1_sign(u331_0v), 0.0f) * s1_abs(s1_sign(u331_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 332: 0001 */
    {
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 333: 1705 0070 0810 0000 0001 */
    {
        float n0 = r0;
        float u333_0v = 0.0f, u333_0z = 0.0f;
        { float _w = (1.0f - f4); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = 0x1.0000000000000p-32f; u333_0v = rs ? rvv : n0; u333_0z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f4); pv4 = s1_abs(s1_sign(u333_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u333_0v)); pv7 = (1.0f - fmaxf(s1_sign(u333_0v), 0.0f) * s1_abs(s1_sign(u333_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 334: 3407 6c50 9fe2 1fe4 b000 007e 006d */
    {
        float n1 = r1, n6 = r6;
        float u334_0v = 0.0f, u334_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r1 + -0x1.0000000000000p+0f); u334_0v = rs ? rvv : n6; u334_0z = rs ? rzd : n6; }
        c_07e = r1;
          n1 = c_06d;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u334_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u334_0v)); pv7 = (1.0f - fmaxf(s1_sign(u334_0v), 0.0f) * s1_abs(s1_sign(u334_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r6 = n6;
    }
    /* 335: 1804 1c40 efe0 007f */
    {
        float n1 = r1;
        float u335_0v = 0.0f, u335_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 + r0); u335_0v = rs ? rvv : n1; u335_0z = rs ? rzd : n1; }
        c_07f = r6;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u335_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u335_0v)); pv7 = (1.0f - fmaxf(s1_sign(u335_0v), 0.0f) * s1_abs(s1_sign(u335_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 336: 0a02 9ee1 */
    {
        float n1 = r1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = wrap1(r1); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 337: 1107 3452 6416 0014 2800 2800 7000 */
    {
        float n3 = r3, n6 = r6;
        float u337_0v = 0.0f, u337_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (0x1.0000000000000p-1f * r1) + (0x1.0000000000000p-1f); u337_0v = rs ? rvv : n3; u337_0z = rs ? rzd : n3; }
        float u337_1v = 0.0f, u337_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r0 * 0x1.0000000000000p+8f); u337_1v = rs ? rvv : n6; u337_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u337_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u337_0v)); pv7 = (1.0f - fmaxf(s1_sign(u337_0v), 0.0f) * s1_abs(s1_sign(u337_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u337_1v)); pv11 = (1.0f - fmaxf(s1_sign(u337_1v), 0.0f) * s1_abs(s1_sign(u337_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r6 = n6;
    }
    /* 338: 0a02 cf06 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = s1_sign(r6) * s1_recip(fmaxf(s1_abs(r6), 0x1p-9f)); n5 = 1.0f; n6 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 339: 1603 4086 beeb */
    {
        float n3 = r3, n4 = r4, n5 = r5, n6 = r6;
        float u339_0v = 0.0f, u339_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u339_0v = rs ? rvv : n4; u339_0z = rs ? rzd : n4; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = 1 - 2 * fabs(wrap1(-r3 - 1.5f)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u339_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u339_0v)); pv7 = (1.0f - fmaxf(s1_sign(u339_0v), 0.0f) * s1_abs(s1_sign(u339_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 340: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u340_0v = 0.0f, u340_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u340_0v = rs ? rvv : n4; u340_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u340_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u340_0v)); pv7 = (1.0f - fmaxf(s1_sign(u340_0v), 0.0f) * s1_abs(s1_sign(u340_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 341: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u341_0v = 0.0f, u341_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u341_0v = rs ? rvv : n4; u341_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u341_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u341_0v)); pv7 = (1.0f - fmaxf(s1_sign(u341_0v), 0.0f) * s1_abs(s1_sign(u341_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 342: 0903 34d0 7000 */
    {
        float n3 = r3;
        float u342_0v = 0.0f, u342_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p+8f); u342_0v = rs ? rvv : n3; u342_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u342_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u342_0v)); pv7 = (1.0f - fmaxf(s1_sign(u342_0v), 0.0f) * s1_abs(s1_sign(u342_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 343: 0902 34c4 */
    {
        float n3 = r3;
        float u343_0v = 0.0f, u343_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * r4); u343_0v = rs ? rvv : n3; u343_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u343_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u343_0v)); pv7 = (1.0f - fmaxf(s1_sign(u343_0v), 0.0f) * s1_abs(s1_sign(u343_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 344: 0904 34e0 3000 0000 */
    {
        float n3 = r3;
        float u344_0v = 0.0f, u344_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p+0f); u344_0v = rs ? rvv : n3; u344_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u344_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u344_0v)); pv7 = (1.0f - fmaxf(s1_sign(u344_0v), 0.0f) * s1_abs(s1_sign(u344_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 345: 0a02 aedb */
    {
        float n2 = r2;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = pymax(-1.0f, pymin(1.0f, r3)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 346: 0904 34a0 3921 fb00 */
    {
        float n3 = r3;
        float u346_0v = 0.0f, u346_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r2 * 0x1.921fb00000000p+0f); u346_0v = rs ? rvv : n3; u346_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u346_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u346_0v)); pv7 = (1.0f - fmaxf(s1_sign(u346_0v), 0.0f) * s1_abs(s1_sign(u346_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 347: 0902 44c3 */
    {
        float n4 = r4;
        float u347_0v = 0.0f, u347_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r3 * r3); u347_0v = rs ? rvv : n4; u347_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u347_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u347_0v)); pv7 = (1.0f - fmaxf(s1_sign(u347_0v), 0.0f) * s1_abs(s1_sign(u347_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 348: 1107 2522 5503 0025 0000 0000 002e */
    {
        float n2 = r2, n5 = r5;
        float u348_0v = 0.0f, u348_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (0x0.0p+0f * r4) + (0x1.7000000000000p-19f); u348_0v = rs ? rvv : n2; u348_0z = rs ? rzd : n2; }
        float u348_1v = 0.0f, u348_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r4 * r3); u348_1v = rs ? rvv : n5; u348_1z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u348_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u348_0v)); pv7 = (1.0f - fmaxf(s1_sign(u348_0v), 0.0f) * s1_abs(s1_sign(u348_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u348_1v)); pv11 = (1.0f - fmaxf(s1_sign(u348_1v), 0.0f) * s1_abs(s1_sign(u348_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r5 = n5;
    }
    /* 349: 1107 7522 6504 0025 800d 0002 2222 */
    {
        float n6 = r6, n7 = r7;
        float u349_0v = 0.0f, u349_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (-0x1.a000000000000p-13f * r4) + (0x1.1111000000000p-7f); u349_0v = rs ? rvv : n7; u349_0z = rs ? rzd : n7; }
        float u349_1v = 0.0f, u349_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r4 * r4); u349_1v = rs ? rvv : n6; u349_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u349_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u349_0v)); pv7 = (1.0f - fmaxf(s1_sign(u349_0v), 0.0f) * s1_abs(s1_sign(u349_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u349_1v)); pv11 = (1.0f - fmaxf(s1_sign(u349_1v), 0.0f) * s1_abs(s1_sign(u349_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 350: 1503 2987 7505 */
    {
        float n2 = r2, n7 = r7;
        float u350_0v = 0.0f, u350_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r7 + r6 * r2); u350_0v = rs ? rvv : n2; u350_0z = rs ? rzd : n2; }
        float u350_1v = 0.0f, u350_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r4 * r5); u350_1v = rs ? rvv : n7; u350_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u350_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u350_0v)); pv7 = (1.0f - fmaxf(s1_sign(u350_0v), 0.0f) * s1_abs(s1_sign(u350_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u350_1v)); pv11 = (1.0f - fmaxf(s1_sign(u350_1v), 0.0f) * s1_abs(s1_sign(u350_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r7 = n7;
    }
    /* 351: 0805 2c8f 5883 a2aa aa00 */
    {
        float n2 = r2;
        float u351_0v = 0.0f, u351_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (r7 * r2) + (r3) + (-0x1.5555000000000p-3f * r5); u351_0v = rs ? rvv : n2; u351_0z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u351_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u351_0v)); pv7 = (1.0f - fmaxf(s1_sign(u351_0v), 0.0f) * s1_abs(s1_sign(u351_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 352: 1503 2442 1c40 */
    {
        float n1 = r1, n2 = r2;
        float u352_0v = 0.0f, u352_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r1 * r2); u352_0v = rs ? rvv : n2; u352_0z = rs ? rzd : n2; }
        float u352_1v = 0.0f, u352_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 + r0); u352_1v = rs ? rvv : n1; u352_1z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u352_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u352_0v)); pv7 = (1.0f - fmaxf(s1_sign(u352_0v), 0.0f) * s1_abs(s1_sign(u352_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u352_1v)); pv11 = (1.0f - fmaxf(s1_sign(u352_1v), 0.0f) * s1_abs(s1_sign(u352_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
    }
    /* 353: 1b04 9ee1 afe0 006e */
    {
        float n1 = r1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = wrap1(r1); }
        c_06e = r2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 354: 1107 3452 6416 0014 2800 2800 7000 */
    {
        float n3 = r3, n6 = r6;
        float u354_0v = 0.0f, u354_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (0x1.0000000000000p-1f * r1) + (0x1.0000000000000p-1f); u354_0v = rs ? rvv : n3; u354_0z = rs ? rzd : n3; }
        float u354_1v = 0.0f, u354_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r0 * 0x1.0000000000000p+8f); u354_1v = rs ? rvv : n6; u354_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u354_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u354_0v)); pv7 = (1.0f - fmaxf(s1_sign(u354_0v), 0.0f) * s1_abs(s1_sign(u354_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u354_1v)); pv11 = (1.0f - fmaxf(s1_sign(u354_1v), 0.0f) * s1_abs(s1_sign(u354_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r6 = n6;
    }
    /* 355: 0a02 cf06 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = s1_sign(r6) * s1_recip(fmaxf(s1_abs(r6), 0x1p-9f)); n5 = 1.0f; n6 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 356: 1603 4086 beeb */
    {
        float n3 = r3, n4 = r4, n5 = r5, n6 = r6;
        float u356_0v = 0.0f, u356_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u356_0v = rs ? rvv : n4; u356_0z = rs ? rzd : n4; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = 1 - 2 * fabs(wrap1(-r3 - 1.5f)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u356_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u356_0v)); pv7 = (1.0f - fmaxf(s1_sign(u356_0v), 0.0f) * s1_abs(s1_sign(u356_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 357: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u357_0v = 0.0f, u357_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u357_0v = rs ? rvv : n4; u357_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u357_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u357_0v)); pv7 = (1.0f - fmaxf(s1_sign(u357_0v), 0.0f) * s1_abs(s1_sign(u357_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 358: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u358_0v = 0.0f, u358_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u358_0v = rs ? rvv : n4; u358_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u358_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u358_0v)); pv7 = (1.0f - fmaxf(s1_sign(u358_0v), 0.0f) * s1_abs(s1_sign(u358_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 359: 0903 34d0 7000 */
    {
        float n3 = r3;
        float u359_0v = 0.0f, u359_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p+8f); u359_0v = rs ? rvv : n3; u359_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u359_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u359_0v)); pv7 = (1.0f - fmaxf(s1_sign(u359_0v), 0.0f) * s1_abs(s1_sign(u359_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 360: 0902 34c4 */
    {
        float n3 = r3;
        float u360_0v = 0.0f, u360_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * r4); u360_0v = rs ? rvv : n3; u360_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u360_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u360_0v)); pv7 = (1.0f - fmaxf(s1_sign(u360_0v), 0.0f) * s1_abs(s1_sign(u360_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 361: 0904 34e0 3000 0000 */
    {
        float n3 = r3;
        float u361_0v = 0.0f, u361_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p+0f); u361_0v = rs ? rvv : n3; u361_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u361_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u361_0v)); pv7 = (1.0f - fmaxf(s1_sign(u361_0v), 0.0f) * s1_abs(s1_sign(u361_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 362: 0a02 aedb */
    {
        float n2 = r2;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = pymax(-1.0f, pymin(1.0f, r3)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 363: 0904 34a0 3921 fb00 */
    {
        float n3 = r3;
        float u363_0v = 0.0f, u363_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r2 * 0x1.921fb00000000p+0f); u363_0v = rs ? rvv : n3; u363_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u363_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u363_0v)); pv7 = (1.0f - fmaxf(s1_sign(u363_0v), 0.0f) * s1_abs(s1_sign(u363_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 364: 0902 44c3 */
    {
        float n4 = r4;
        float u364_0v = 0.0f, u364_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r3 * r3); u364_0v = rs ? rvv : n4; u364_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u364_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u364_0v)); pv7 = (1.0f - fmaxf(s1_sign(u364_0v), 0.0f) * s1_abs(s1_sign(u364_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 365: 1107 2522 5503 0025 0000 0000 002e */
    {
        float n2 = r2, n5 = r5;
        float u365_0v = 0.0f, u365_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (0x0.0p+0f * r4) + (0x1.7000000000000p-19f); u365_0v = rs ? rvv : n2; u365_0z = rs ? rzd : n2; }
        float u365_1v = 0.0f, u365_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r4 * r3); u365_1v = rs ? rvv : n5; u365_1z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u365_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u365_0v)); pv7 = (1.0f - fmaxf(s1_sign(u365_0v), 0.0f) * s1_abs(s1_sign(u365_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u365_1v)); pv11 = (1.0f - fmaxf(s1_sign(u365_1v), 0.0f) * s1_abs(s1_sign(u365_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r5 = n5;
    }
    /* 366: 1107 7522 6504 0025 800d 0002 2222 */
    {
        float n6 = r6, n7 = r7;
        float u366_0v = 0.0f, u366_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (-0x1.a000000000000p-13f * r4) + (0x1.1111000000000p-7f); u366_0v = rs ? rvv : n7; u366_0z = rs ? rzd : n7; }
        float u366_1v = 0.0f, u366_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r4 * r4); u366_1v = rs ? rvv : n6; u366_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u366_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u366_0v)); pv7 = (1.0f - fmaxf(s1_sign(u366_0v), 0.0f) * s1_abs(s1_sign(u366_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u366_1v)); pv11 = (1.0f - fmaxf(s1_sign(u366_1v), 0.0f) * s1_abs(s1_sign(u366_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 367: 1503 2987 7505 */
    {
        float n2 = r2, n7 = r7;
        float u367_0v = 0.0f, u367_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r7 + r6 * r2); u367_0v = rs ? rvv : n2; u367_0z = rs ? rzd : n2; }
        float u367_1v = 0.0f, u367_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r4 * r5); u367_1v = rs ? rvv : n7; u367_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u367_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u367_0v)); pv7 = (1.0f - fmaxf(s1_sign(u367_0v), 0.0f) * s1_abs(s1_sign(u367_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u367_1v)); pv11 = (1.0f - fmaxf(s1_sign(u367_1v), 0.0f) * s1_abs(s1_sign(u367_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r7 = n7;
    }
    /* 368: 0805 2c8f 5883 a2aa aa00 */
    {
        float n2 = r2;
        float u368_0v = 0.0f, u368_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (r7 * r2) + (r3) + (-0x1.5555000000000p-3f * r5); u368_0v = rs ? rvv : n2; u368_0z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u368_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u368_0v)); pv7 = (1.0f - fmaxf(s1_sign(u368_0v), 0.0f) * s1_abs(s1_sign(u368_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 369: 1503 2442 1c40 */
    {
        float n1 = r1, n2 = r2;
        float u369_0v = 0.0f, u369_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r1 * r2); u369_0v = rs ? rvv : n2; u369_0z = rs ? rzd : n2; }
        float u369_1v = 0.0f, u369_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 + r0); u369_1v = rs ? rvv : n1; u369_1z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u369_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u369_0v)); pv7 = (1.0f - fmaxf(s1_sign(u369_0v), 0.0f) * s1_abs(s1_sign(u369_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u369_1v)); pv11 = (1.0f - fmaxf(s1_sign(u369_1v), 0.0f) * s1_abs(s1_sign(u369_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
    }
    /* 370: 1b04 9ee1 afe0 0072 */
    {
        float n1 = r1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = wrap1(r1); }
        c_072 = r2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 371: 1107 3452 6416 0014 2800 2800 7000 */
    {
        float n3 = r3, n6 = r6;
        float u371_0v = 0.0f, u371_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (0x1.0000000000000p-1f * r1) + (0x1.0000000000000p-1f); u371_0v = rs ? rvv : n3; u371_0z = rs ? rzd : n3; }
        float u371_1v = 0.0f, u371_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r0 * 0x1.0000000000000p+8f); u371_1v = rs ? rvv : n6; u371_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u371_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u371_0v)); pv7 = (1.0f - fmaxf(s1_sign(u371_0v), 0.0f) * s1_abs(s1_sign(u371_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u371_1v)); pv11 = (1.0f - fmaxf(s1_sign(u371_1v), 0.0f) * s1_abs(s1_sign(u371_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r6 = n6;
    }
    /* 372: 0a02 cf06 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = s1_sign(r6) * s1_recip(fmaxf(s1_abs(r6), 0x1p-9f)); n5 = 1.0f; n6 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 373: 1603 4086 beeb */
    {
        float n3 = r3, n4 = r4, n5 = r5, n6 = r6;
        float u373_0v = 0.0f, u373_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u373_0v = rs ? rvv : n4; u373_0z = rs ? rzd : n4; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = 1 - 2 * fabs(wrap1(-r3 - 1.5f)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u373_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u373_0v)); pv7 = (1.0f - fmaxf(s1_sign(u373_0v), 0.0f) * s1_abs(s1_sign(u373_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 374: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u374_0v = 0.0f, u374_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u374_0v = rs ? rvv : n4; u374_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u374_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u374_0v)); pv7 = (1.0f - fmaxf(s1_sign(u374_0v), 0.0f) * s1_abs(s1_sign(u374_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 375: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u375_0v = 0.0f, u375_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u375_0v = rs ? rvv : n4; u375_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u375_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u375_0v)); pv7 = (1.0f - fmaxf(s1_sign(u375_0v), 0.0f) * s1_abs(s1_sign(u375_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 376: 0903 34d0 7000 */
    {
        float n3 = r3;
        float u376_0v = 0.0f, u376_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p+8f); u376_0v = rs ? rvv : n3; u376_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u376_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u376_0v)); pv7 = (1.0f - fmaxf(s1_sign(u376_0v), 0.0f) * s1_abs(s1_sign(u376_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 377: 0902 34c4 */
    {
        float n3 = r3;
        float u377_0v = 0.0f, u377_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * r4); u377_0v = rs ? rvv : n3; u377_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u377_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u377_0v)); pv7 = (1.0f - fmaxf(s1_sign(u377_0v), 0.0f) * s1_abs(s1_sign(u377_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 378: 0904 34e0 3000 0000 */
    {
        float n3 = r3;
        float u378_0v = 0.0f, u378_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p+0f); u378_0v = rs ? rvv : n3; u378_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u378_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u378_0v)); pv7 = (1.0f - fmaxf(s1_sign(u378_0v), 0.0f) * s1_abs(s1_sign(u378_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 379: 0a02 aedb */
    {
        float n2 = r2;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = pymax(-1.0f, pymin(1.0f, r3)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 380: 0904 34a0 3921 fb00 */
    {
        float n3 = r3;
        float u380_0v = 0.0f, u380_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r2 * 0x1.921fb00000000p+0f); u380_0v = rs ? rvv : n3; u380_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u380_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u380_0v)); pv7 = (1.0f - fmaxf(s1_sign(u380_0v), 0.0f) * s1_abs(s1_sign(u380_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 381: 0902 44c3 */
    {
        float n4 = r4;
        float u381_0v = 0.0f, u381_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r3 * r3); u381_0v = rs ? rvv : n4; u381_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u381_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u381_0v)); pv7 = (1.0f - fmaxf(s1_sign(u381_0v), 0.0f) * s1_abs(s1_sign(u381_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 382: 1107 2522 5503 0025 0000 0000 002e */
    {
        float n2 = r2, n5 = r5;
        float u382_0v = 0.0f, u382_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (0x0.0p+0f * r4) + (0x1.7000000000000p-19f); u382_0v = rs ? rvv : n2; u382_0z = rs ? rzd : n2; }
        float u382_1v = 0.0f, u382_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r4 * r3); u382_1v = rs ? rvv : n5; u382_1z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u382_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u382_0v)); pv7 = (1.0f - fmaxf(s1_sign(u382_0v), 0.0f) * s1_abs(s1_sign(u382_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u382_1v)); pv11 = (1.0f - fmaxf(s1_sign(u382_1v), 0.0f) * s1_abs(s1_sign(u382_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r5 = n5;
    }
    /* 383: 1107 7522 6504 0025 800d 0002 2222 */
    {
        float n6 = r6, n7 = r7;
        float u383_0v = 0.0f, u383_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (-0x1.a000000000000p-13f * r4) + (0x1.1111000000000p-7f); u383_0v = rs ? rvv : n7; u383_0z = rs ? rzd : n7; }
        float u383_1v = 0.0f, u383_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r4 * r4); u383_1v = rs ? rvv : n6; u383_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u383_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u383_0v)); pv7 = (1.0f - fmaxf(s1_sign(u383_0v), 0.0f) * s1_abs(s1_sign(u383_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u383_1v)); pv11 = (1.0f - fmaxf(s1_sign(u383_1v), 0.0f) * s1_abs(s1_sign(u383_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 384: 1503 2987 7505 */
    {
        float n2 = r2, n7 = r7;
        float u384_0v = 0.0f, u384_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r7 + r6 * r2); u384_0v = rs ? rvv : n2; u384_0z = rs ? rzd : n2; }
        float u384_1v = 0.0f, u384_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r4 * r5); u384_1v = rs ? rvv : n7; u384_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u384_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u384_0v)); pv7 = (1.0f - fmaxf(s1_sign(u384_0v), 0.0f) * s1_abs(s1_sign(u384_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u384_1v)); pv11 = (1.0f - fmaxf(s1_sign(u384_1v), 0.0f) * s1_abs(s1_sign(u384_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r7 = n7;
    }
    /* 385: 0805 2c8f 5883 a2aa aa00 */
    {
        float n2 = r2;
        float u385_0v = 0.0f, u385_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (r7 * r2) + (r3) + (-0x1.5555000000000p-3f * r5); u385_0v = rs ? rvv : n2; u385_0z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u385_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u385_0v)); pv7 = (1.0f - fmaxf(s1_sign(u385_0v), 0.0f) * s1_abs(s1_sign(u385_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 386: 1503 2442 1c40 */
    {
        float n1 = r1, n2 = r2;
        float u386_0v = 0.0f, u386_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r1 * r2); u386_0v = rs ? rvv : n2; u386_0z = rs ? rzd : n2; }
        float u386_1v = 0.0f, u386_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 + r0); u386_1v = rs ? rvv : n1; u386_1z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u386_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u386_0v)); pv7 = (1.0f - fmaxf(s1_sign(u386_0v), 0.0f) * s1_abs(s1_sign(u386_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u386_1v)); pv11 = (1.0f - fmaxf(s1_sign(u386_1v), 0.0f) * s1_abs(s1_sign(u386_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
    }
    /* 387: 1b04 9ee1 afe0 0076 */
    {
        float n1 = r1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = wrap1(r1); }
        c_076 = r2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 388: 1107 3452 6416 0014 2800 2800 7000 */
    {
        float n3 = r3, n6 = r6;
        float u388_0v = 0.0f, u388_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (0x1.0000000000000p-1f * r1) + (0x1.0000000000000p-1f); u388_0v = rs ? rvv : n3; u388_0z = rs ? rzd : n3; }
        float u388_1v = 0.0f, u388_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r0 * 0x1.0000000000000p+8f); u388_1v = rs ? rvv : n6; u388_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u388_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u388_0v)); pv7 = (1.0f - fmaxf(s1_sign(u388_0v), 0.0f) * s1_abs(s1_sign(u388_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u388_1v)); pv11 = (1.0f - fmaxf(s1_sign(u388_1v), 0.0f) * s1_abs(s1_sign(u388_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r6 = n6;
    }
    /* 389: 0a02 cf06 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = s1_sign(r6) * s1_recip(fmaxf(s1_abs(r6), 0x1p-9f)); n5 = 1.0f; n6 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 390: 1603 4086 beeb */
    {
        float n3 = r3, n4 = r4, n5 = r5, n6 = r6;
        float u390_0v = 0.0f, u390_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u390_0v = rs ? rvv : n4; u390_0z = rs ? rzd : n4; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = 1 - 2 * fabs(wrap1(-r3 - 1.5f)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u390_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u390_0v)); pv7 = (1.0f - fmaxf(s1_sign(u390_0v), 0.0f) * s1_abs(s1_sign(u390_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 391: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u391_0v = 0.0f, u391_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u391_0v = rs ? rvv : n4; u391_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u391_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u391_0v)); pv7 = (1.0f - fmaxf(s1_sign(u391_0v), 0.0f) * s1_abs(s1_sign(u391_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 392: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u392_0v = 0.0f, u392_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u392_0v = rs ? rvv : n4; u392_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u392_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u392_0v)); pv7 = (1.0f - fmaxf(s1_sign(u392_0v), 0.0f) * s1_abs(s1_sign(u392_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 393: 0903 34d0 7000 */
    {
        float n3 = r3;
        float u393_0v = 0.0f, u393_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p+8f); u393_0v = rs ? rvv : n3; u393_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u393_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u393_0v)); pv7 = (1.0f - fmaxf(s1_sign(u393_0v), 0.0f) * s1_abs(s1_sign(u393_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 394: 0902 34c4 */
    {
        float n3 = r3;
        float u394_0v = 0.0f, u394_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * r4); u394_0v = rs ? rvv : n3; u394_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u394_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u394_0v)); pv7 = (1.0f - fmaxf(s1_sign(u394_0v), 0.0f) * s1_abs(s1_sign(u394_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 395: 0904 34e0 3000 0000 */
    {
        float n3 = r3;
        float u395_0v = 0.0f, u395_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p+0f); u395_0v = rs ? rvv : n3; u395_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u395_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u395_0v)); pv7 = (1.0f - fmaxf(s1_sign(u395_0v), 0.0f) * s1_abs(s1_sign(u395_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 396: 0a02 aedb */
    {
        float n2 = r2;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = pymax(-1.0f, pymin(1.0f, r3)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 397: 0904 34a0 3921 fb00 */
    {
        float n3 = r3;
        float u397_0v = 0.0f, u397_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r2 * 0x1.921fb00000000p+0f); u397_0v = rs ? rvv : n3; u397_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u397_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u397_0v)); pv7 = (1.0f - fmaxf(s1_sign(u397_0v), 0.0f) * s1_abs(s1_sign(u397_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 398: 0902 44c3 */
    {
        float n4 = r4;
        float u398_0v = 0.0f, u398_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r3 * r3); u398_0v = rs ? rvv : n4; u398_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u398_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u398_0v)); pv7 = (1.0f - fmaxf(s1_sign(u398_0v), 0.0f) * s1_abs(s1_sign(u398_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 399: 1107 2522 5503 0025 0000 0000 002e */
    {
        float n2 = r2, n5 = r5;
        float u399_0v = 0.0f, u399_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (0x0.0p+0f * r4) + (0x1.7000000000000p-19f); u399_0v = rs ? rvv : n2; u399_0z = rs ? rzd : n2; }
        float u399_1v = 0.0f, u399_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r4 * r3); u399_1v = rs ? rvv : n5; u399_1z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u399_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u399_0v)); pv7 = (1.0f - fmaxf(s1_sign(u399_0v), 0.0f) * s1_abs(s1_sign(u399_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u399_1v)); pv11 = (1.0f - fmaxf(s1_sign(u399_1v), 0.0f) * s1_abs(s1_sign(u399_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r5 = n5;
    }
    /* 400: 1107 7522 6504 0025 800d 0002 2222 */
    {
        float n6 = r6, n7 = r7;
        float u400_0v = 0.0f, u400_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (-0x1.a000000000000p-13f * r4) + (0x1.1111000000000p-7f); u400_0v = rs ? rvv : n7; u400_0z = rs ? rzd : n7; }
        float u400_1v = 0.0f, u400_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r4 * r4); u400_1v = rs ? rvv : n6; u400_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u400_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u400_0v)); pv7 = (1.0f - fmaxf(s1_sign(u400_0v), 0.0f) * s1_abs(s1_sign(u400_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u400_1v)); pv11 = (1.0f - fmaxf(s1_sign(u400_1v), 0.0f) * s1_abs(s1_sign(u400_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 401: 1503 2987 7505 */
    {
        float n2 = r2, n7 = r7;
        float u401_0v = 0.0f, u401_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r7 + r6 * r2); u401_0v = rs ? rvv : n2; u401_0z = rs ? rzd : n2; }
        float u401_1v = 0.0f, u401_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r4 * r5); u401_1v = rs ? rvv : n7; u401_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u401_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u401_0v)); pv7 = (1.0f - fmaxf(s1_sign(u401_0v), 0.0f) * s1_abs(s1_sign(u401_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u401_1v)); pv11 = (1.0f - fmaxf(s1_sign(u401_1v), 0.0f) * s1_abs(s1_sign(u401_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r7 = n7;
    }
    /* 402: 2b09 2c8f 7fe2 6fe4 5983 007e 007f a2aa aa00 */
    {
        float n2 = r2, n6 = r6, n7 = r7;
        float u402_0v = 0.0f, u402_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (r7 * r2) + (r3) + (-0x1.5555000000000p-3f * r5); u402_0v = rs ? rvv : n2; u402_0z = rs ? rzd : n2; }
          n7 = c_07e;
          n6 = c_07f;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u402_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u402_0v)); pv7 = (1.0f - fmaxf(s1_sign(u402_0v), 0.0f) * s1_abs(s1_sign(u402_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r6 = n6;
        r7 = n7;
    }
    /* 403: b40a 2442 19c6 fe64 3fe0 4fe2 006e 007d 0251 8c00 */
    {
        float n1 = r1, n2 = r2, n3 = r3, n4 = r4, n7 = r7;
        float u403_0v = 0.0f, u403_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r1 * r2); u403_0v = rs ? rvv : n2; u403_0z = rs ? rzd : n2; }
        float u403_1v = 0.0f, u403_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r6 + r7 * r1); u403_1v = rs ? rvv : n1; u403_1z = rs ? rzd : n1; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.28c6000000000p-7f; }
          n3 = c_06e;
          n4 = c_07d;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u403_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u403_0v)); pv7 = (1.0f - fmaxf(s1_sign(u403_0v), 0.0f) * s1_abs(s1_sign(u403_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u403_1v)); pv11 = (1.0f - fmaxf(s1_sign(u403_1v), 0.0f) * s1_abs(s1_sign(u403_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
        r3 = n3;
        r4 = n4;
        r7 = n7;
    }
    /* 404: 9c0c 0acf 7068 afe2 3fe4 4fe6 c3c0 007a 0072 0079 038d 7f00 */
    {
        float n0 = r0, n3 = r3, n4 = r4, n7 = r7;
        float u404_0v = 0.0f, u404_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r3) + (r7 * -r4); u404_0v = rs ? rvv : n0; u404_0z = rs ? rzd : n0; }
        float u404_1v = 0.0f, u404_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.c6bf800000000p-7f; u404_1v = rs ? rvv : n7; u404_1z = rs ? rzd : n7; }
        c_07a = r2;
          n3 = c_072;
          n4 = c_079;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u404_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u404_0v)); pv7 = (1.0f - fmaxf(s1_sign(u404_0v), 0.0f) * s1_abs(s1_sign(u404_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u404_1v)); pv11 = (1.0f - fmaxf(s1_sign(u404_1v), 0.0f) * s1_abs(s1_sign(u404_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r3 = n3;
        r4 = n4;
        r7 = n7;
    }
    /* 405: 4e0a 0ecf 7066 3fe2 4fe4 c3c0 0076 0075 0706 6c00 */
    {
        float n0 = r0, n3 = r3, n4 = r4, n7 = r7;
        float u405_0v = 0.0f, u405_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r3) + (r0) + (r7 * -r4); u405_0v = rs ? rvv : n0; u405_0z = rs ? rzd : n0; }
        float u405_1v = 0.0f, u405_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.c19b000000000p-6f; u405_1v = rs ? rvv : n7; u405_1z = rs ? rzd : n7; }
          n3 = c_076;
          n4 = c_075;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u405_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u405_0v)); pv7 = (1.0f - fmaxf(s1_sign(u405_0v), 0.0f) * s1_abs(s1_sign(u405_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u405_1v)); pv11 = (1.0f - fmaxf(s1_sign(u405_1v), 0.0f) * s1_abs(s1_sign(u405_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r3 = n3;
        r4 = n4;
        r7 = n7;
    }
    /* 406: 4e0a 0ecf 7066 3fe2 4fe4 c3c0 007a 0071 0c3c 2900 */
    {
        float n0 = r0, n3 = r3, n4 = r4, n7 = r7;
        float u406_0v = 0.0f, u406_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r3) + (r0) + (r7 * -r4); u406_0v = rs ? rvv : n0; u406_0z = rs ? rzd : n0; }
        float u406_1v = 0.0f, u406_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.8785200000000p-5f; u406_1v = rs ? rvv : n7; u406_1z = rs ? rzd : n7; }
          n3 = c_07a;
          n4 = c_071;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u406_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u406_0v)); pv7 = (1.0f - fmaxf(s1_sign(u406_0v), 0.0f) * s1_abs(s1_sign(u406_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u406_1v)); pv11 = (1.0f - fmaxf(s1_sign(u406_1v), 0.0f) * s1_abs(s1_sign(u406_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r3 = n3;
        r4 = n4;
        r7 = n7;
    }
    /* 407: 4e0a 0ecf 7066 3fe2 4fe4 c3c0 006f 007c 1252 4f00 */
    {
        float n0 = r0, n3 = r3, n4 = r4, n7 = r7;
        float u407_0v = 0.0f, u407_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r3) + (r0) + (r7 * -r4); u407_0v = rs ? rvv : n0; u407_0z = rs ? rzd : n0; }
        float u407_1v = 0.0f, u407_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.2524f00000000p-4f; u407_1v = rs ? rvv : n7; u407_1z = rs ? rzd : n7; }
          n3 = c_06f;
          n4 = c_07c;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u407_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u407_0v)); pv7 = (1.0f - fmaxf(s1_sign(u407_0v), 0.0f) * s1_abs(s1_sign(u407_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u407_1v)); pv11 = (1.0f - fmaxf(s1_sign(u407_1v), 0.0f) * s1_abs(s1_sign(u407_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r3 = n3;
        r4 = n4;
        r7 = n7;
    }
    /* 408: 4e0a 0ecf 7066 3fe2 4fe4 c3c0 0073 0078 1839 0b00 */
    {
        float n0 = r0, n3 = r3, n4 = r4, n7 = r7;
        float u408_0v = 0.0f, u408_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r3) + (r0) + (r7 * -r4); u408_0v = rs ? rvv : n0; u408_0z = rs ? rzd : n0; }
        float u408_1v = 0.0f, u408_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.8390b00000000p-4f; u408_1v = rs ? rvv : n7; u408_1z = rs ? rzd : n7; }
          n3 = c_073;
          n4 = c_078;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u408_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u408_0v)); pv7 = (1.0f - fmaxf(s1_sign(u408_0v), 0.0f) * s1_abs(s1_sign(u408_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u408_1v)); pv11 = (1.0f - fmaxf(s1_sign(u408_1v), 0.0f) * s1_abs(s1_sign(u408_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r3 = n3;
        r4 = n4;
        r7 = n7;
    }
    /* 409: 4e0a 0ecf 7066 3fe2 4fe4 c3c0 0077 0074 1ce1 4900 */
    {
        float n0 = r0, n3 = r3, n4 = r4, n7 = r7;
        float u409_0v = 0.0f, u409_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r3) + (r0) + (r7 * -r4); u409_0v = rs ? rvv : n0; u409_0z = rs ? rzd : n0; }
        float u409_1v = 0.0f, u409_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.ce14900000000p-4f; u409_1v = rs ? rvv : n7; u409_1z = rs ? rzd : n7; }
          n3 = c_077;
          n4 = c_074;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u409_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u409_0v)); pv7 = (1.0f - fmaxf(s1_sign(u409_0v), 0.0f) * s1_abs(s1_sign(u409_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u409_1v)); pv11 = (1.0f - fmaxf(s1_sign(u409_1v), 0.0f) * s1_abs(s1_sign(u409_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r3 = n3;
        r4 = n4;
        r7 = n7;
    }
    /* 410: 4e0a 0ecf 7066 3fe2 4fe4 c3c0 007b 0070 1f71 b900 */
    {
        float n0 = r0, n3 = r3, n4 = r4, n7 = r7;
        float u410_0v = 0.0f, u410_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r3) + (r0) + (r7 * -r4); u410_0v = rs ? rvv : n0; u410_0z = rs ? rzd : n0; }
        float u410_1v = 0.0f, u410_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.f71b900000000p-4f; u410_1v = rs ? rvv : n7; u410_1z = rs ? rzd : n7; }
          n3 = c_07b;
          n4 = c_070;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u410_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u410_0v)); pv7 = (1.0f - fmaxf(s1_sign(u410_0v), 0.0f) * s1_abs(s1_sign(u410_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u410_1v)); pv11 = (1.0f - fmaxf(s1_sign(u410_1v), 0.0f) * s1_abs(s1_sign(u410_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r3 = n3;
        r4 = n4;
        r7 = n7;
    }
    /* 411: 1405 0ecf 9fe2 c3c0 006c */
    {
        float n0 = r0;
        float u411_0v = 0.0f, u411_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r3) + (r0) + (r7 * -r4); u411_0v = rs ? rvv : n0; u411_0z = rs ? rzd : n0; }
        c_06c = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u411_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u411_0v)); pv7 = (1.0f - fmaxf(s1_sign(u411_0v), 0.0f) * s1_abs(s1_sign(u411_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 412: 1e05 8fe0 2fe2 0080 006d */
    {
        float n2 = r2;
        c_080 = r0;
          n2 = c_06d;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 413: 1e05 0fe0 1fe2 0080 0052 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_080;
          n1 = c_052;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 414: 0c03 2fe0 0053 */
    {
        float n2 = r2;
          n2 = c_053;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    s1_export(0, r4);
    s1_export(1, r1);
    s1_export(2, f5);
    s1_export(3, pv5);
    s1_export(4, f7);
    s1_export(5, pv7);
    s1_export(6, r3);
    s1_export(7, r0);
    s1_export(8, r5);
    s1_export(9, r2);
    s1_export(10, c_033);
    s1_export(11, c_052);
    s1_export(12, c_053);
    s1_export(13, c_067);
    s1_export(14, c_068);
    s1_export(15, c_041);
    s1_export(16, c_04e);
    s1_export(17, c_055);
    s1_export(18, c_054);
    s1_export(19, c_06b);
    D_000_1 = c_000;
    D_001_1 = c_001;
    D_002_1 = c_002;
    D_003_1 = c_003;
    D_004_1 = c_004;
    D_005_1 = c_005;
    D_006_1 = c_006;
    D_007_1 = c_007;
    D_008_1 = c_008;
    D_009_1 = c_009;
    D_00a_1 = c_00a;
    D_00b_1 = c_00b;
    D_00c_1 = c_00c;
    D_00d_1 = c_00d;
    D_00e_1 = c_00e;
    D_00f_1 = c_00f;
    D_010_1 = c_010;
    D_011_1 = c_011;
    D_012_1 = c_012;
    D_013_1 = c_013;
    D_014_1 = c_014;
    D_015_1 = c_015;
    D_016_1 = c_016;
    D_017_1 = c_017;
    D_018_1 = c_018;
    D_019_1 = c_019;
    D_01a_1 = c_01a;
    D_01b_1 = c_01b;
    D_01c_1 = c_01c;
    D_01d_1 = c_01d;
    D_01e_1 = c_01e;
    D_01f_1 = c_01f;
    D_020_1 = c_020;
    D_021_1 = c_021;
    D_022_1 = c_022;
    D_023_1 = c_023;
    D_024_1 = c_024;
    D_025_1 = c_025;
    D_026_1 = c_026;
    D_027_1 = c_027;
    D_028_1 = c_028;
    D_029_1 = c_029;
    D_02a_1 = c_02a;
    D_02b_1 = c_02b;
    D_02c_1 = c_02c;
    D_02d_1 = c_02d;
    D_02e_1 = c_02e;
    D_02f_1 = c_02f;
    D_030_1 = c_030;
    D_031_1 = c_031;
    D_032_1 = c_032;
    D_033_1 = c_033;
    D_034_1 = c_034;
    D_035_2 = D_035_1;
    D_035_1 = c_035;
    D_037_1 = c_037;
    D_038_1 = c_038;
    D_039_1 = c_039;
    D_03a_1 = c_03a;
    D_03b_1 = c_03b;
    D_03c_1 = c_03c;
    D_03d_1 = c_03d;
    D_03e_1 = c_03e;
    D_03f_1 = c_03f;
    D_040_1 = c_040;
    D_041_1 = c_041;
    D_042_1 = c_042;
    D_043_1 = c_043;
    D_044_1 = c_044;
    D_045_1 = c_045;
    D_046_1 = c_046;
    D_047_1 = c_047;
    D_048_1 = c_048;
    D_049_1 = c_049;
    D_04a_1 = c_04a;
    D_04b_1 = c_04b;
    D_04c_1 = c_04c;
    D_04d_1 = c_04d;
    D_04e_1 = c_04e;
    D_04f_1 = c_04f;
    D_050_1 = c_050;
    D_051_1 = c_051;
    D_052_1 = c_052;
    D_053_1 = c_053;
    D_054_1 = c_054;
    D_055_1 = c_055;
    D_056_1 = c_056;
    D_057_1 = c_057;
    D_058_1 = c_058;
    D_059_1 = c_059;
    D_05a_1 = c_05a;
    D_05b_1 = c_05b;
    D_05c_1 = c_05c;
    D_05d_1 = c_05d;
    D_05e_1 = c_05e;
    D_05f_1 = c_05f;
    D_060_1 = c_060;
    D_061_1 = c_061;
    D_062_1 = c_062;
    D_063_1 = c_063;
    D_064_1 = c_064;
    D_065_1 = c_065;
    D_066_1 = c_066;
    D_067_1 = c_067;
    D_068_1 = c_068;
    D_069_1 = c_069;
    D_06a_1 = c_06a;
    D_06b_1 = c_06b;
    D_06c_1 = c_06c;
    D_06d_1 = c_06d;
    D_06e_1 = c_06e;
    D_06f_1 = c_06f;
    D_070_1 = c_070;
    D_071_1 = c_071;
    D_072_1 = c_072;
    D_073_1 = c_073;
    D_074_1 = c_074;
    D_075_1 = c_075;
    D_076_1 = c_076;
    D_077_1 = c_077;
    D_078_1 = c_078;
    D_079_1 = c_079;
    D_07a_1 = c_07a;
    D_07b_1 = c_07b;
    D_07c_1 = c_07c;
    D_07d_1 = c_07d;
    D_07e_1 = c_07e;
    D_07f_1 = c_07f;
    D_080_1 = c_080;
}
static inline __attribute__((always_inline)) void tb03_voice_b(KARGS_B)
{
    float r0 = s1_range(s1_import(7), -1.4996847185f, 1.4998216305f);
    float r1 = s1_range(s1_import(1), -0.251f, 1.251f);
    float r2 = s1_range(s1_import(9), -0.10394117624999999f, 1.22158823525f);
    float r3 = s1_range(s1_import(6), -1.50084157075f, 1.50083119975f);
    float r4 = s1_range(s1_import(0), -1.5008301265f, 1.5008301265f);
    float r5 = s1_range(s1_import(8), -0.9699455625f, 4.8457278125f);
    float r6 = 0.0f;
    float r7 = 0.12282902f;
    float f4 = 1.0f;
    float f5 = s1_range(s1_import(2), -0.251f, 1.251f);
    float f7 = s1_range(s1_import(4), -0.251f, 1.251f);
    float f9 = 1.0f;
    float f11 = 0.0f;
    float f17 = 1.0f;
    float pm1 = 0.0f, pv4 = 1.0f, pv5 = s1_range(s1_import(3), -0.251f, 1.251f), pv7 = s1_range(s1_import(5), -0.251f, 1.251f);
    float pm2 = 0.0f, pv9 = 1.0f, pv11 = 0.0f;
    float pm4 = 0.0f, pv17 = 1.0f;
    float c_004 = 0.0f;
    float c_01b = 0.0f;
    float c_01d = 0.0f;
    float c_033 = s1_range(s1_import(10), -1.6987500326249998f, 8.627750174525f);
    float c_041 = s1_range(s1_import(15), -0.228223441f, 1.137117205f);
    float c_04e = s1_range(s1_import(16), -0.2506477365f, 1.2492386825f);
    float c_050 = 0.0f;
    float c_051 = 0.0f;
    float c_052 = s1_range(s1_import(11), -0.251f, 1.251f);
    float c_053 = s1_range(s1_import(12), -0.10394117624999999f, 1.22158823525f);
    float c_054 = s1_range(s1_import(18), -0.251f, 1.251f);
    float c_055 = s1_range(s1_import(17), -0.251f, 1.251f);
    float c_057 = 0.477958947f;
    float c_058 = 1.0f;
    float c_067 = s1_range(s1_import(13), 0.16130773999999998f, 1.5369649600000002f);
    float c_068 = s1_range(s1_import(14), -0.610039037875f, 3.1898956435750003f);
    float c_06b = s1_range(s1_import(19), -1.06799388975f, 5.56252900595f);
    float c_081 = 0.0f /* written before read */;
    float c_082 = D_081_1;
    float c_083 = D_082_1;
    float c_084 = D_083_1;
    float c_085 = D_084_1;
    float c_086 = D_085_1;
    float c_087 = D_086_1;
    float c_088 = D_087_1;
    float c_089 = D_088_1;
    float c_08a = D_089_1;
    float c_08b = D_08a_1;
    float c_08c = D_08b_1;
    float c_08d = D_08c_1;
    float c_08e = D_08d_1;
    float c_08f = D_08e_1;
    float c_090 = D_08f_1;
    float c_091 = D_090_1;
    float c_092 = D_091_1;
    float c_093 = D_092_1;
    float c_094 = D_093_1;
    float c_095 = D_094_1;
    float c_096 = D_095_1;
    float c_097 = D_096_1;
    float c_098 = D_097_1;
    float c_099 = D_098_1;
    float c_09a = D_099_1;
    float c_09b = D_09a_1;
    float c_09c = D_09b_1;
    float c_09d = D_09c_1;
    float c_09e = D_09d_1;
    float c_09f = D_09e_1;
    float c_0a0 = D_09f_1;
    float c_0a1 = D_0a0_1;
    float c_0a2 = D_0a1_1;
    float c_0a3 = D_0a2_1;
    float c_0a4 = D_0a3_1;
    float c_0a5 = D_0a4_1;
    float c_0a6 = D_0a5_1;
    float c_0a7 = D_0a6_1;
    float c_0a8 = D_0a7_1;
    float c_0a9 = D_0a8_1;
    float c_0aa = D_0a9_1;
    float c_0ab = D_0aa_1;
    float c_0ac = D_0ab_1;
    float c_0ad = D_0ac_1;
    float c_0ae = D_0ad_1;
    float c_0af = D_0ae_1;
    float c_0b0 = D_0af_1;
    float c_0b1 = D_0b0_1;
    float c_0b2 = D_0b1_1;
    float c_0b3 = D_0b2_1;
    float c_0b4 = D_0b3_1;
    float c_0b5 = D_0b4_1;
    float c_0b6 = D_0b5_1;
    float c_0b7 = D_0b6_1;
    float c_0b8 = D_0b7_1;
    float c_0b9 = D_0b8_1;
    float c_0ba = D_0b9_1;
    float c_0bb = D_0ba_1;
    float c_0bc = D_0bb_1;
    float c_0bd = D_0bc_1;
    float c_0be = D_0bd_1;
    float c_0bf = D_0be_1;
    float c_0c0 = D_0bf_1;
    float c_0c1 = D_0c0_1;
    float c_0c2 = D_0c1_1;
    float c_0c3 = D_0c2_1;
    float c_0c4 = D_0c3_1;
    float c_0c5 = D_0c4_1;
    float c_0c6 = D_0c5_1;
    float c_0c7 = D_0c6_1;
    float c_0c8 = D_0c7_1;
    float c_0c9 = D_0c8_1;
    float c_0ca = D_0c9_1;
    float c_0cb = D_0ca_1;
    float c_0cc = D_0cb_1;
    float c_0cd = D_0cc_1;
    float c_0ce = D_0cd_1;
    float c_0cf = D_0ce_1;
    float c_0d0 = D_0cf_1;
    float c_0d1 = D_0d0_1;
    float c_0d2 = D_0d1_1;
    float c_0d3 = D_0d2_1;
    /* 415: 250a 4474 7441 6fe2 0038 0082 2bd5 5144 26e1 47ae */
    {
        float n4 = r4, n6 = r6, n7 = r7;
        float u415_0v = 0.0f, u415_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (0x1.7aaa288000000p-1f * r1) + (0x1.b851eb8000000p-2f); u415_0v = rs ? rvv : n4; u415_0z = rs ? rzd : n4; }
        float u415_1v = 0.0f, u415_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r1 * r1); u415_1v = rs ? rvv : n7; u415_1z = rs ? rzd : n7; }
          n6 = c_082;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u415_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u415_0v)); pv7 = (1.0f - fmaxf(s1_sign(u415_0v), 0.0f) * s1_abs(s1_sign(u415_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u415_1v)); pv11 = (1.0f - fmaxf(s1_sign(u415_1v), 0.0f) * s1_abs(s1_sign(u415_1z)));
        pm4 = 0.0f;
        r4 = n4;
        r6 = n6;
        r7 = n7;
    }
    /* 416: 1106 45f2 3c0e 0004 a2b6 98f2 */
    {
        float n3 = r3, n4 = r4;
        float u416_0v = 0.0f, u416_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (-0x1.5b4c790000000p-3f * r7) + (r4); u416_0v = rs ? rvv : n4; u416_0z = rs ? rzd : n4; }
        float u416_1v = 0.0f, u416_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r0 + -r6); u416_1v = rs ? rvv : n3; u416_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u416_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u416_0v)); pv7 = (1.0f - fmaxf(s1_sign(u416_0v), 0.0f) * s1_abs(s1_sign(u416_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u416_1v)); pv11 = (1.0f - fmaxf(s1_sign(u416_1v), 0.0f) * s1_abs(s1_sign(u416_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r4 = n4;
    }
    /* 417: 0904 4520 00b8 7000 */
    {
        float n4 = r4;
        float u417_0v = 0.0f, u417_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r4 * 0x1.70e0000000000p-9f); u417_0v = rs ? rvv : n4; u417_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u417_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u417_0v)); pv7 = (1.0f - fmaxf(s1_sign(u417_0v), 0.0f) * s1_abs(s1_sign(u417_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 418: 1108 54b2 7482 0036 4340 0000 0000 0000 */
    {
        float n5 = r5, n7 = r7;
        float u418_0v = 0.0f, u418_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (0x1.a000000000000p+1f * r2) + (0x0.0p+0f); u418_0v = rs ? rvv : n5; u418_0z = rs ? rzd : n5; }
        float u418_1v = 0.0f, u418_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r2 * r2); u418_1v = rs ? rvv : n7; u418_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u418_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u418_0v)); pv7 = (1.0f - fmaxf(s1_sign(u418_0v), 0.0f) * s1_abs(s1_sign(u418_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u418_1v)); pv11 = (1.0f - fmaxf(s1_sign(u418_1v), 0.0f) * s1_abs(s1_sign(u418_1z)));
        pm4 = 0.0f;
        r5 = n5;
        r7 = n7;
    }
    /* 419: 1007 64cc 55f4 0006 0005 bb33 3333 */
    {
        float n5 = r5, n6 = r6;
        float u419_0v = 0.0f, u419_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (r4 * r3) + (r6); u419_0v = rs ? rvv : n6; u419_0z = rs ? rzd : n6; }
        float u419_1v = 0.0f, u419_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (-0x1.b333333000000p+0f * r7) + (r5); u419_1v = rs ? rvv : n5; u419_1z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u419_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u419_0v)); pv7 = (1.0f - fmaxf(s1_sign(u419_0v), 0.0f) * s1_abs(s1_sign(u419_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u419_1v)); pv11 = (1.0f - fmaxf(s1_sign(u419_1v), 0.0f) * s1_abs(s1_sign(u419_1z)));
        pm4 = 0.0f;
        r5 = n5;
        r6 = n6;
    }
    /* 420: 1405 058d efe2 0003 0081 */
    {
        float n0 = r0;
        float u420_0v = 0.0f, u420_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r5 * r6) + (r3); u420_0v = rs ? rvv : n0; u420_0z = rs ? rzd : n0; }
        c_081 = r6;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u420_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u420_0v)); pv7 = (1.0f - fmaxf(s1_sign(u420_0v), 0.0f) * s1_abs(s1_sign(u420_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 421: 0807 64b2 0036 b5f2 58bf 26cc cccc */
    {
        float n6 = r6;
        float u421_0v = 0.0f, u421_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (-0x1.5f258bf000000p+0f * r2) + (0x1.b333330000000p-2f); u421_0v = rs ? rvv : n6; u421_0z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u421_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u421_0v)); pv7 = (1.0f - fmaxf(s1_sign(u421_0v), 0.0f) * s1_abs(s1_sign(u421_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 422: 1106 65f2 75c2 0006 39c2 8f5c */
    {
        float n6 = r6, n7 = r7;
        float u422_0v = 0.0f, u422_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (0x1.9c28f5c000000p+0f * r7) + (r6); u422_0v = rs ? rvv : n6; u422_0z = rs ? rzd : n6; }
        float u422_1v = 0.0f, u422_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r7 * r2); u422_1v = rs ? rvv : n7; u422_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u422_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u422_0v)); pv7 = (1.0f - fmaxf(s1_sign(u422_0v), 0.0f) * s1_abs(s1_sign(u422_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u422_1v)); pv11 = (1.0f - fmaxf(s1_sign(u422_1v), 0.0f) * s1_abs(s1_sign(u422_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 423: 1106 65f2 7441 0006 aa22 2222 */
    {
        float n6 = r6, n7 = r7;
        float u423_0v = 0.0f, u423_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (-0x1.4444444000000p-1f * r7) + (r6); u423_0v = rs ? rvv : n6; u423_0z = rs ? rzd : n6; }
        float u423_1v = 0.0f, u423_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r1 * r1); u423_1v = rs ? rvv : n7; u423_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u423_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u423_0v)); pv7 = (1.0f - fmaxf(s1_sign(u423_0v), 0.0f) * s1_abs(s1_sign(u423_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u423_1v)); pv11 = (1.0f - fmaxf(s1_sign(u423_1v), 0.0f) * s1_abs(s1_sign(u423_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 424: 1108 6472 0406 0036 243c 45c1 2e66 6666 */
    {
        float n0 = r0, n6 = r6;
        float u424_0v = 0.0f, u424_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (0x1.0f11704000000p-2f * r1) + (0x1.ccccccc000000p-1f); u424_0v = rs ? rvv : n6; u424_0z = rs ? rzd : n6; }
        float u424_1v = 0.0f, u424_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r6); u424_1v = rs ? rvv : n0; u424_1z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u424_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u424_0v)); pv7 = (1.0f - fmaxf(s1_sign(u424_0v), 0.0f) * s1_abs(s1_sign(u424_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u424_1v)); pv11 = (1.0f - fmaxf(s1_sign(u424_1v), 0.0f) * s1_abs(s1_sign(u424_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r6 = n6;
    }
    /* 425: 1106 65f2 75c1 0006 a44e 6add */
    {
        float n6 = r6, n7 = r7;
        float u425_0v = 0.0f, u425_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (-0x1.139ab74000000p-2f * r7) + (r6); u425_0v = rs ? rvv : n6; u425_0z = rs ? rzd : n6; }
        float u425_1v = 0.0f, u425_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r7 * r1); u425_1v = rs ? rvv : n7; u425_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u425_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u425_0v)); pv7 = (1.0f - fmaxf(s1_sign(u425_0v), 0.0f) * s1_abs(s1_sign(u425_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u425_1v)); pv11 = (1.0f - fmaxf(s1_sign(u425_1v), 0.0f) * s1_abs(s1_sign(u425_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 426: 0805 65f2 0006 1abb eb5c */
    {
        float n6 = r6;
        float u426_0v = 0.0f, u426_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (0x1.abbeb5c000000p-4f * r7) + (r6); u426_0v = rs ? rvv : n6; u426_0z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u426_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u426_0v)); pv7 = (1.0f - fmaxf(s1_sign(u426_0v), 0.0f) * s1_abs(s1_sign(u426_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 427: 0902 0406 */
    {
        float n0 = r0;
        float u427_0v = 0.0f, u427_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r6); u427_0v = rs ? rvv : n0; u427_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u427_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u427_0v)); pv7 = (1.0f - fmaxf(s1_sign(u427_0v), 0.0f) * s1_abs(s1_sign(u427_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 428: 0c03 8fe0 0083 */
    {
        c_083 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 429: 1e05 0fe0 1fe2 0033 0050 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_033;
          n1 = c_050;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 430: 1e05 2fe0 3fe2 001b 00bd */
    {
        float n2 = r2, n3 = r3;
          n2 = c_01b;
          n3 = c_0bd;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 431: 0902 8042 */
    {
        float n0 = r0;
        float u431_0v = 0.0f, u431_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = r1 + r2 + r0; RES(n0); u431_0v = rs ? rvv : n0; u431_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u431_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u431_0v)); pv7 = (1.0f - fmaxf(s1_sign(u431_0v), 0.0f) * s1_abs(s1_sign(u431_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 432: 0902 0c03 */
    {
        float n0 = r0;
        float u432_0v = 0.0f, u432_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + r3); u432_0v = rs ? rvv : n0; u432_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u432_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u432_0v)); pv7 = (1.0f - fmaxf(s1_sign(u432_0v), 0.0f) * s1_abs(s1_sign(u432_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 433: 0c03 8fe0 0084 */
    {
        c_084 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 434: 1506 2c20 3c23 ca00 0042 0000 */
    {
        float n2 = r2, n3 = r3;
        float u434_0v = 0.0f, u434_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r0 + -0x1.4000000000000p+3f); u434_0v = rs ? rvv : n2; u434_0z = rs ? rzd : n2; }
        float u434_1v = 0.0f, u434_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r0 + 0x1.0000000000000p+1f); u434_1v = rs ? rvv : n3; u434_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u434_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u434_0v)); pv7 = (1.0f - fmaxf(s1_sign(u434_0v), 0.0f) * s1_abs(s1_sign(u434_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u434_1v)); pv11 = (1.0f - fmaxf(s1_sign(u434_1v), 0.0f) * s1_abs(s1_sign(u434_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 435: 1506 2060 3063 ca00 0042 0000 */
    {
        float n2 = r2, n3 = r3;
        float u435_0v = 0.0f, u435_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = -0x1.4000000000000p+3f; u435_0v = rs ? rvv : n2; u435_0z = rs ? rzd : n2; }
        float u435_1v = 0.0f, u435_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = 0x1.0000000000000p+1f; u435_1v = rs ? rvv : n3; u435_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u435_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u435_0v)); pv7 = (1.0f - fmaxf(s1_sign(u435_0v), 0.0f) * s1_abs(s1_sign(u435_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u435_1v)); pv11 = (1.0f - fmaxf(s1_sign(u435_1v), 0.0f) * s1_abs(s1_sign(u435_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 436: 5d05 004a 004b 0e10 1220 */
    {
        float n0 = r0;
        float u436_0v = 0.0f, u436_0z = 0.0f;
        { float _w = (1.0f - f7); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -r2; u436_0v = rs ? rvv : n0; u436_0z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        float u436_1v = 0.0f, u436_1z = 0.0f;
        { float _w = (1.0f - f9); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -r3; u436_1v = rs ? rvv : n0; u436_1z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f7); pv4 = s1_abs(s1_sign(u436_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u436_0v)); pv7 = (1.0f - fmaxf(s1_sign(u436_0v), 0.0f) * s1_abs(s1_sign(u436_0z)));
        pm2 = (1.0f - f9); pv9 = fminf(1.0f, 1.0f + s1_sign(u436_1v)); pv11 = (1.0f - fmaxf(s1_sign(u436_1v), 0.0f) * s1_abs(s1_sign(u436_1z)));
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 437: 0c03 8fe0 0085 */
    {
        c_085 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 438: 0904 0c20 c4c0 0000 */
    {
        float n0 = r0;
        float u438_0v = 0.0f, u438_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + -0x1.3000000000000p+2f); u438_0v = rs ? rvv : n0; u438_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u438_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u438_0v)); pv7 = (1.0f - fmaxf(s1_sign(u438_0v), 0.0f) * s1_abs(s1_sign(u438_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 439: 0904 1430 2b17 217f */
    {
        float n1 = r1;
        float u439_0v = 0.0f, u439_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r0 * 0x1.62e42fe000000p-1f); u439_0v = rs ? rvv : n1; u439_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u439_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u439_0v)); pv7 = (1.0f - fmaxf(s1_sign(u439_0v), 0.0f) * s1_abs(s1_sign(u439_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 440: 0a02 8f19 */
    {
        float n0 = r0, n1 = r1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; { float _f = s1_floor(r1); n0 = s1_exp(_f); n1 = r1 - _f; } }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 441: 1108 3472 20a1 0036 0001 ae64 0012 7e4f */
    {
        float n2 = r2, n3 = r3;
        float u441_0v = 0.0f, u441_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (0x1.ae64000000000p-16f * r1) + (0x1.27e4f00000000p-12f); u441_0v = rs ? rvv : n3; u441_0z = rs ? rzd : n3; }
        float u441_1v = 0.0f, u441_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = 0.25f * r1 * r1; u441_1v = rs ? rvv : n2; u441_1z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u441_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u441_0v)); pv7 = (1.0f - fmaxf(s1_sign(u441_0v), 0.0f) * s1_abs(s1_sign(u441_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u441_1v)); pv11 = (1.0f - fmaxf(s1_sign(u441_1v), 0.0f) * s1_abs(s1_sign(u441_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 442: 0807 3cca 1cb6 002e 3bc7 01a0 1a01 */
    {
        float n3 = r3;
        float u442_0v = 0.0f, u442_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.a01a010000000p-8f) + (0x1.71de380000000p-11f * r1); u442_0v = rs ? rvv : n3; u442_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u442_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u442_0v)); pv7 = (1.0f - fmaxf(s1_sign(u442_0v), 0.0f) * s1_abs(s1_sign(u442_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 443: 0807 3cca 1cb6 0340 3403 16c1 6c16 */
    {
        float n3 = r3;
        float u443_0v = 0.0f, u443_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.6c16c16000000p-4f) + (0x1.a01a018000000p-7f * r1); u443_0v = rs ? rvv : n3; u443_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u443_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u443_0v)); pv7 = (1.0f - fmaxf(s1_sign(u443_0v), 0.0f) * s1_abs(s1_sign(u443_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 444: 0807 3cca 1cb6 2222 2222 2aaa aaaa */
    {
        float n3 = r3;
        float u444_0v = 0.0f, u444_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.5555554000000p-1f) + (0x1.1111110000000p-3f * r1); u444_0v = rs ? rvv : n3; u444_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u444_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u444_0v)); pv7 = (1.0f - fmaxf(s1_sign(u444_0v), 0.0f) * s1_abs(s1_sign(u444_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 445: 0807 3cca 1d62 4200 002a aaaa aa00 */
    {
        float n3 = r3;
        float u445_0v = 0.0f, u445_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.0000000000000p+1f) + (0x1.5555554000000p-1f * r1); u445_0v = rs ? rvv : n3; u445_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u445_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u445_0v)); pv7 = (1.0f - fmaxf(s1_sign(u445_0v), 0.0f) * s1_abs(s1_sign(u445_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 446: 0806 3cca 1912 3000 3000 0000 */
    {
        float n3 = r3;
        float u446_0v = 0.0f, u446_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.0000000000000p+0f) + (0x1.0000000000000p+0f * r1); u446_0v = rs ? rvv : n3; u446_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u446_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u446_0v)); pv7 = (1.0f - fmaxf(s1_sign(u446_0v), 0.0f) * s1_abs(s1_sign(u446_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 447: 0902 0403 */
    {
        float n0 = r0;
        float u447_0v = 0.0f, u447_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r3); u447_0v = rs ? rvv : n0; u447_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u447_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u447_0v)); pv7 = (1.0f - fmaxf(s1_sign(u447_0v), 0.0f) * s1_abs(s1_sign(u447_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 448: 0c03 8fe0 0086 */
    {
        c_086 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 449: 0c03 0fe0 0085 */
    {
        float n0 = r0;
          n0 = c_085;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 450: 1108 3432 6400 0036 22b4 3958 2410 624d */
    {
        float n3 = r3, n6 = r6;
        float u450_0v = 0.0f, u450_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (0x1.5a1cac0000000p-3f * r0) + (0x1.0418934000000p-2f); u450_0v = rs ? rvv : n3; u450_0z = rs ? rzd : n3; }
        float u450_1v = 0.0f, u450_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r0 * r0); u450_1v = rs ? rvv : n6; u450_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u450_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u450_0v)); pv7 = (1.0f - fmaxf(s1_sign(u450_0v), 0.0f) * s1_abs(s1_sign(u450_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u450_1v)); pv11 = (1.0f - fmaxf(s1_sign(u450_1v), 0.0f) * s1_abs(s1_sign(u450_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r6 = n6;
    }
    /* 451: 1108 35b2 2e36 0003 85e3 53f7 43aa aae2 */
    {
        float n2 = r2, n3 = r3;
        float u451_0v = 0.0f, u451_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (-0x1.78d4fdc000000p-6f * r6) + (r3); u451_0v = rs ? rvv : n3; u451_0z = rs ? rzd : n3; }
        float u451_1v = 0.0f, u451_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(0x1.d555710000000p+1f - r0); u451_1v = rs ? rvv : n2; u451_1z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u451_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u451_0v)); pv7 = (1.0f - fmaxf(s1_sign(u451_0v), 0.0f) * s1_abs(s1_sign(u451_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u451_1v)); pv11 = (1.0f - fmaxf(s1_sign(u451_1v), 0.0f) * s1_abs(s1_sign(u451_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 452: 0807 4432 0036 0000 0000 2907 f23c */
    {
        float n4 = r4;
        float u452_0v = 0.0f, u452_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (0x0.0p+0f * r0) + (0x1.20fe478000000p-1f); u452_0v = rs ? rvv : n4; u452_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u452_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u452_0v)); pv7 = (1.0f - fmaxf(s1_sign(u452_0v), 0.0f) * s1_abs(s1_sign(u452_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 453: 1703 3044 1710 */
    {
        float n3 = r3;
        float u453_0v = 0.0f, u453_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f11)); float o3 = n3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = r4; u453_0v = rs ? rvv : n3; u453_0z = rs ? rzd : n3; }
          n3 = o3 + _w * (n3 - o3); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f11)); pv4 = s1_abs(s1_sign(u453_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u453_0v)); pv7 = (1.0f - fmaxf(s1_sign(u453_0v), 0.0f) * s1_abs(s1_sign(u453_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 454: 0904 2e30 0000 0000 */
    {
        float n2 = r2;
        float u454_0v = 0.0f, u454_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(0x0.0p+0f - r0); u454_0v = rs ? rvv : n2; u454_0z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u454_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u454_0v)); pv7 = (1.0f - fmaxf(s1_sign(u454_0v), 0.0f) * s1_abs(s1_sign(u454_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 455: 0807 4432 0036 0000 0000 2410 624d */
    {
        float n4 = r4;
        float u455_0v = 0.0f, u455_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (0x0.0p+0f * r0) + (0x1.0418934000000p-2f); u455_0v = rs ? rvv : n4; u455_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u455_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u455_0v)); pv7 = (1.0f - fmaxf(s1_sign(u455_0v), 0.0f) * s1_abs(s1_sign(u455_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 456: 1703 3044 0b10 */
    {
        float n3 = r3;
        float u456_0v = 0.0f, u456_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o3 = n3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = r4; u456_0v = rs ? rvv : n3; u456_0z = rs ? rzd : n3; }
          n3 = o3 + _w * (n3 - o3); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u456_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u456_0v)); pv7 = (1.0f - fmaxf(s1_sign(u456_0v), 0.0f) * s1_abs(s1_sign(u456_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 457: 1804 0043 bfe0 0087 */
    {
        float n0 = r0;
        float u457_0v = 0.0f, u457_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = r3; u457_0v = rs ? rvv : n0; u457_0z = rs ? rzd : n0; }
        c_087 = r3;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u457_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u457_0v)); pv7 = (1.0f - fmaxf(s1_sign(u457_0v), 0.0f) * s1_abs(s1_sign(u457_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 458: 1e05 0fe0 1fe2 0086 0087 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_086;
          n1 = c_087;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 459: 0c03 2fe0 0058 */
    {
        float n2 = r2;
          n2 = c_058;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 460: 0904 0430 0096 2fc9 */
    {
        float n0 = r0;
        float u460_0v = 0.0f, u460_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * 0x1.2c5f920000000p-9f); u460_0v = rs ? rvv : n0; u460_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u460_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u460_0v)); pv7 = (1.0f - fmaxf(s1_sign(u460_0v), 0.0f) * s1_abs(s1_sign(u460_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 461: 0001 */
    {
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 462: 1705 0070 0810 0000 0001 */
    {
        float n0 = r0;
        float u462_0v = 0.0f, u462_0z = 0.0f;
        { float _w = (1.0f - f4); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = 0x1.0000000000000p-32f; u462_0v = rs ? rvv : n0; u462_0z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f4); pv4 = s1_abs(s1_sign(u462_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u462_0v)); pv7 = (1.0f - fmaxf(s1_sign(u462_0v), 0.0f) * s1_abs(s1_sign(u462_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 463: 2b09 1652 afe6 3fe8 0014 4200 3000 00a0 0089 */
    {
        float n1 = r1, n3 = r3;
        float u463_0v = 0.0f, u463_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = -(0x1.0000000000000p+1f * r1) + (0x1.0000000000000p+0f); u463_0v = rs ? rvv : n1; u463_0z = rs ? rzd : n1; }
        c_0a0 = r2;
          n3 = c_089;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u463_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u463_0v)); pv7 = (1.0f - fmaxf(s1_sign(u463_0v), 0.0f) * s1_abs(s1_sign(u463_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r3 = n3;
    }
    /* 464: 1804 3cc0 9fe0 009e */
    {
        float n3 = r3;
        float u464_0v = 0.0f, u464_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 + r0); u464_0v = rs ? rvv : n3; u464_0z = rs ? rzd : n3; }
        c_09e = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u464_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u464_0v)); pv7 = (1.0f - fmaxf(s1_sign(u464_0v), 0.0f) * s1_abs(s1_sign(u464_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 465: 1206 1452 bee3 0014 a6f3 2803 */
    {
        float n1 = r1, n3 = r3;
        float u465_0v = 0.0f, u465_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (-0x1.bcc0000000000p-2f * r1) + (0x1.0060000000000p-1f); u465_0v = rs ? rvv : n1; u465_0z = rs ? rzd : n1; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = wrap1(r3); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u465_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u465_0v)); pv7 = (1.0f - fmaxf(s1_sign(u465_0v), 0.0f) * s1_abs(s1_sign(u465_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r3 = n3;
    }
    /* 466: 0a02 9ed9 */
    {
        float n1 = r1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = pymax(-1.0f, pymin(1.0f, r1)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 467: 0c03 9fe0 009f */
    {
        c_09f = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 468: 1e05 bfe0 1fe2 009d 009e */
    {
        float n1 = r1;
        c_09d = r3;
          n1 = c_09e;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 469: 0902 3cc1 */
    {
        float n3 = r3;
        float u469_0v = 0.0f, u469_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 + r1); u469_0v = rs ? rvv : n3; u469_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u469_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u469_0v)); pv7 = (1.0f - fmaxf(s1_sign(u469_0v), 0.0f) * s1_abs(s1_sign(u469_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 470: 0c03 2fe0 009b */
    {
        float n2 = r2;
          n2 = c_09b;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 471: 2d06 4050 4052 0b12 3000 0000 */
    {
        float n4 = r4;
        float u471_0v = 0.0f, u471_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o4 = n4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = 0x1.0000000000000p+0f; u471_0v = rs ? rvv : n4; u471_0z = rs ? rzd : n4; }
          n4 = o4 + _w * (n4 - o4); }
        float u471_1v = 0.0f, u471_1z = 0.0f;
        { float _w = (1.0f - f5); float o4 = n4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = 0x0.0p+0f; u471_1v = rs ? rvv : n4; u471_1z = rs ? rzd : n4; }
          n4 = o4 + _w * (n4 - o4); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u471_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u471_0v)); pv7 = (1.0f - fmaxf(s1_sign(u471_0v), 0.0f) * s1_abs(s1_sign(u471_0z)));
        pm2 = (1.0f - f5); pv9 = fminf(1.0f, 1.0f + s1_sign(u471_1v)); pv11 = (1.0f - fmaxf(s1_sign(u471_1v), 0.0f) * s1_abs(s1_sign(u471_1z)));
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 472: 2d06 5050 5052 0b12 b000 3000 */
    {
        float n5 = r5;
        float u472_0v = 0.0f, u472_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o5 = n5;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = -0x1.0000000000000p+0f; u472_0v = rs ? rvv : n5; u472_0z = rs ? rzd : n5; }
          n5 = o5 + _w * (n5 - o5); }
        float u472_1v = 0.0f, u472_1z = 0.0f;
        { float _w = (1.0f - f5); float o5 = n5;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = 0x1.0000000000000p+0f; u472_1v = rs ? rvv : n5; u472_1z = rs ? rzd : n5; }
          n5 = o5 + _w * (n5 - o5); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u472_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u472_0v)); pv7 = (1.0f - fmaxf(s1_sign(u472_0v), 0.0f) * s1_abs(s1_sign(u472_0z)));
        pm2 = (1.0f - f5); pv9 = fminf(1.0f, 1.0f + s1_sign(u472_1v)); pv11 = (1.0f - fmaxf(s1_sign(u472_1v), 0.0f) * s1_abs(s1_sign(u472_1z)));
        pm4 = 0.0f;
        r5 = n5;
    }
    /* 473: 2d06 6050 6052 0b12 3000 b000 */
    {
        float n6 = r6;
        float u473_0v = 0.0f, u473_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o6 = n6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = 0x1.0000000000000p+0f; u473_0v = rs ? rvv : n6; u473_0z = rs ? rzd : n6; }
          n6 = o6 + _w * (n6 - o6); }
        float u473_1v = 0.0f, u473_1z = 0.0f;
        { float _w = (1.0f - f5); float o6 = n6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = -0x1.0000000000000p+0f; u473_1v = rs ? rvv : n6; u473_1z = rs ? rzd : n6; }
          n6 = o6 + _w * (n6 - o6); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u473_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u473_0v)); pv7 = (1.0f - fmaxf(s1_sign(u473_0v), 0.0f) * s1_abs(s1_sign(u473_0z)));
        pm2 = (1.0f - f5); pv9 = fminf(1.0f, 1.0f + s1_sign(u473_1v)); pv11 = (1.0f - fmaxf(s1_sign(u473_1v), 0.0f) * s1_abs(s1_sign(u473_1z)));
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 474: 2509 1964 6c46 5fe2 29c0 009c 0024 752f fdb8 */
    {
        float n1 = r1, n5 = r5, n6 = r6;
        float u474_0v = 0.0f, u474_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (0x1.23a8000000000p-11f * r5) + (0x1.ffb7000000000p-1f * r2); u474_0v = rs ? rvv : n1; u474_0z = rs ? rzd : n1; }
        float u474_1v = 0.0f, u474_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r1 + r6); u474_1v = rs ? rvv : n6; u474_1z = rs ? rzd : n6; }
          n5 = c_09c;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u474_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u474_0v)); pv7 = (1.0f - fmaxf(s1_sign(u474_0v), 0.0f) * s1_abs(s1_sign(u474_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u474_1v)); pv11 = (1.0f - fmaxf(s1_sign(u474_1v), 0.0f) * s1_abs(s1_sign(u474_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r5 = n5;
        r6 = n6;
    }
    /* 475: 2509 1464 2d0d 9fe2 0027 009a a3ce d9ac 3126 */
    {
        float n1 = r1, n2 = r2;
        float u475_0v = 0.0f, u475_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (-0x1.e76c800000000p-3f * r1) + (-0x1.8624c00000000p-1f); u475_0v = rs ? rvv : n1; u475_0z = rs ? rzd : n1; }
        float u475_1v = 0.0f, u475_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r4 + -r5); u475_1v = rs ? rvv : n2; u475_1z = rs ? rzd : n2; }
        c_09a = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u475_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u475_0v)); pv7 = (1.0f - fmaxf(s1_sign(u475_0v), 0.0f) * s1_abs(s1_sign(u475_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u475_1v)); pv11 = (1.0f - fmaxf(s1_sign(u475_1v), 0.0f) * s1_abs(s1_sign(u475_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
    }
    /* 476: 1806 2422 7fe0 009d 6410 0000 */
    {
        float n2 = r2, n7 = r7;
        float u476_0v = 0.0f, u476_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r0 * 0x1.0400000000000p+6f); u476_0v = rs ? rvv : n2; u476_0z = rs ? rzd : n2; }
          n7 = c_09d;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u476_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u476_0v)); pv7 = (1.0f - fmaxf(s1_sign(u476_0v), 0.0f) * s1_abs(s1_sign(u476_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r7 = n7;
    }
    /* 477: 1304 4d0a 1610 d285 */
    {
        float n4 = r4;
        float u477_0v = 0.0f, u477_0z = 0.0f;
        { float _w = (1.0f - f11); float o4 = n4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (r2 * r4) + (r5) + (r2 * -r5); u477_0v = rs ? rvv : n4; u477_0z = rs ? rzd : n4; }
          n4 = o4 + _w * (n4 - o4); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f11); pv4 = s1_abs(s1_sign(u477_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u477_0v)); pv7 = (1.0f - fmaxf(s1_sign(u477_0v), 0.0f) * s1_abs(s1_sign(u477_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 478: 4e0a 47d2 1444 cfe6 2fe8 0014 2800 2800 009b 009f */
    {
        float n1 = r1, n2 = r2, n4 = r4;
        float u478_0v = 0.0f, u478_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = -(0x1.0000000000000p-1f * r7) + (0x1.0000000000000p-1f); u478_0v = rs ? rvv : n4; u478_0z = rs ? rzd : n4; }
        float u478_1v = 0.0f, u478_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r4); u478_1v = rs ? rvv : n1; u478_1z = rs ? rzd : n1; }
        c_09b = r4;
          n2 = c_09f;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u478_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u478_0v)); pv7 = (1.0f - fmaxf(s1_sign(u478_0v), 0.0f) * s1_abs(s1_sign(u478_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u478_1v)); pv11 = (1.0f - fmaxf(s1_sign(u478_1v), 0.0f) * s1_abs(s1_sign(u478_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
        r4 = n4;
    }
    /* 479: 0a02 c502 */
    {
        float n4 = r4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = pymax(r4, r2); RESZ(n4, (r4) - (r2)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 480: 0806 4f12 2516 4300 4300 3000 */
    {
        float n4 = r4;
        float u480_0v = 0.0f, u480_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = -(0x1.8000000000000p+1f * r4) + (0x1.0000000000000p+0f) + (0x1.8000000000000p+1f * r2); u480_0v = rs ? rvv : n4; u480_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u480_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u480_0v)); pv7 = (1.0f - fmaxf(s1_sign(u480_0v), 0.0f) * s1_abs(s1_sign(u480_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 481: 0902 1444 */
    {
        float n1 = r1;
        float u481_0v = 0.0f, u481_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r4); u481_0v = rs ? rvv : n1; u481_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u481_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u481_0v)); pv7 = (1.0f - fmaxf(s1_sign(u481_0v), 0.0f) * s1_abs(s1_sign(u481_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 482: 0a02 cf06 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = s1_sign(r6) * s1_recip(fmaxf(s1_abs(r6), 0x1p-9f)); n5 = 1.0f; n6 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 483: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u483_0v = 0.0f, u483_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u483_0v = rs ? rvv : n4; u483_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u483_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u483_0v)); pv7 = (1.0f - fmaxf(s1_sign(u483_0v), 0.0f) * s1_abs(s1_sign(u483_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 484: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u484_0v = 0.0f, u484_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u484_0v = rs ? rvv : n4; u484_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u484_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u484_0v)); pv7 = (1.0f - fmaxf(s1_sign(u484_0v), 0.0f) * s1_abs(s1_sign(u484_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 485: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u485_0v = 0.0f, u485_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u485_0v = rs ? rvv : n4; u485_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u485_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u485_0v)); pv7 = (1.0f - fmaxf(s1_sign(u485_0v), 0.0f) * s1_abs(s1_sign(u485_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 486: 1504 3503 6410 7000 */
    {
        float n3 = r3, n6 = r6;
        float u486_0v = 0.0f, u486_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r4 * r3); u486_0v = rs ? rvv : n3; u486_0z = rs ? rzd : n3; }
        float u486_1v = 0.0f, u486_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r0 * 0x1.0000000000000p+8f); u486_1v = rs ? rvv : n6; u486_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u486_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u486_0v)); pv7 = (1.0f - fmaxf(s1_sign(u486_0v), 0.0f) * s1_abs(s1_sign(u486_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u486_1v)); pv11 = (1.0f - fmaxf(s1_sign(u486_1v), 0.0f) * s1_abs(s1_sign(u486_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r6 = n6;
    }
    /* 487: 0a02 cf06 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = s1_sign(r6) * s1_recip(fmaxf(s1_abs(r6), 0x1p-9f)); n5 = 1.0f; n6 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 488: 1603 4086 beeb */
    {
        float n3 = r3, n4 = r4, n5 = r5, n6 = r6;
        float u488_0v = 0.0f, u488_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u488_0v = rs ? rvv : n4; u488_0z = rs ? rzd : n4; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = 1 - 2 * fabs(wrap1(-r3 - 1.5f)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u488_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u488_0v)); pv7 = (1.0f - fmaxf(s1_sign(u488_0v), 0.0f) * s1_abs(s1_sign(u488_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 489: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u489_0v = 0.0f, u489_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u489_0v = rs ? rvv : n4; u489_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u489_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u489_0v)); pv7 = (1.0f - fmaxf(s1_sign(u489_0v), 0.0f) * s1_abs(s1_sign(u489_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 490: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u490_0v = 0.0f, u490_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u490_0v = rs ? rvv : n4; u490_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u490_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u490_0v)); pv7 = (1.0f - fmaxf(s1_sign(u490_0v), 0.0f) * s1_abs(s1_sign(u490_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 491: 0903 34d0 7000 */
    {
        float n3 = r3;
        float u491_0v = 0.0f, u491_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p+8f); u491_0v = rs ? rvv : n3; u491_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u491_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u491_0v)); pv7 = (1.0f - fmaxf(s1_sign(u491_0v), 0.0f) * s1_abs(s1_sign(u491_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 492: 0902 34c4 */
    {
        float n3 = r3;
        float u492_0v = 0.0f, u492_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * r4); u492_0v = rs ? rvv : n3; u492_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u492_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u492_0v)); pv7 = (1.0f - fmaxf(s1_sign(u492_0v), 0.0f) * s1_abs(s1_sign(u492_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 493: 0904 34e0 1000 0000 */
    {
        float n3 = r3;
        float u493_0v = 0.0f, u493_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p-4f); u493_0v = rs ? rvv : n3; u493_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u493_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u493_0v)); pv7 = (1.0f - fmaxf(s1_sign(u493_0v), 0.0f) * s1_abs(s1_sign(u493_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 494: 1b04 aedb 3fe0 009d */
    {
        float n2 = r2, n3 = r3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = pymax(-1.0f, pymin(1.0f, r3)); }
          n3 = c_09d;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 495: 1503 2442 3cc0 */
    {
        float n2 = r2, n3 = r3;
        float u495_0v = 0.0f, u495_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r1 * r2); u495_0v = rs ? rvv : n2; u495_0z = rs ? rzd : n2; }
        float u495_1v = 0.0f, u495_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 + r0); u495_1v = rs ? rvv : n3; u495_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u495_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u495_0v)); pv7 = (1.0f - fmaxf(s1_sign(u495_0v), 0.0f) * s1_abs(s1_sign(u495_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u495_1v)); pv11 = (1.0f - fmaxf(s1_sign(u495_1v), 0.0f) * s1_abs(s1_sign(u495_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 496: 1b04 bee3 afe0 008a */
    {
        float n3 = r3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = wrap1(r3); }
        c_08a = r2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 497: 1e05 bfe0 1fe2 009d 009e */
    {
        float n1 = r1;
        c_09d = r3;
          n1 = c_09e;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 498: 0902 3cc1 */
    {
        float n3 = r3;
        float u498_0v = 0.0f, u498_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 + r1); u498_0v = rs ? rvv : n3; u498_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u498_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u498_0v)); pv7 = (1.0f - fmaxf(s1_sign(u498_0v), 0.0f) * s1_abs(s1_sign(u498_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 499: 0c03 2fe0 009a */
    {
        float n2 = r2;
          n2 = c_09a;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 500: 2d06 4050 4052 0b12 3000 0000 */
    {
        float n4 = r4;
        float u500_0v = 0.0f, u500_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o4 = n4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = 0x1.0000000000000p+0f; u500_0v = rs ? rvv : n4; u500_0z = rs ? rzd : n4; }
          n4 = o4 + _w * (n4 - o4); }
        float u500_1v = 0.0f, u500_1z = 0.0f;
        { float _w = (1.0f - f5); float o4 = n4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = 0x0.0p+0f; u500_1v = rs ? rvv : n4; u500_1z = rs ? rzd : n4; }
          n4 = o4 + _w * (n4 - o4); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u500_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u500_0v)); pv7 = (1.0f - fmaxf(s1_sign(u500_0v), 0.0f) * s1_abs(s1_sign(u500_0z)));
        pm2 = (1.0f - f5); pv9 = fminf(1.0f, 1.0f + s1_sign(u500_1v)); pv11 = (1.0f - fmaxf(s1_sign(u500_1v), 0.0f) * s1_abs(s1_sign(u500_1z)));
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 501: 2d06 5050 5052 0b12 b000 3000 */
    {
        float n5 = r5;
        float u501_0v = 0.0f, u501_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o5 = n5;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = -0x1.0000000000000p+0f; u501_0v = rs ? rvv : n5; u501_0z = rs ? rzd : n5; }
          n5 = o5 + _w * (n5 - o5); }
        float u501_1v = 0.0f, u501_1z = 0.0f;
        { float _w = (1.0f - f5); float o5 = n5;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = 0x1.0000000000000p+0f; u501_1v = rs ? rvv : n5; u501_1z = rs ? rzd : n5; }
          n5 = o5 + _w * (n5 - o5); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u501_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u501_0v)); pv7 = (1.0f - fmaxf(s1_sign(u501_0v), 0.0f) * s1_abs(s1_sign(u501_0z)));
        pm2 = (1.0f - f5); pv9 = fminf(1.0f, 1.0f + s1_sign(u501_1v)); pv11 = (1.0f - fmaxf(s1_sign(u501_1v), 0.0f) * s1_abs(s1_sign(u501_1z)));
        pm4 = 0.0f;
        r5 = n5;
    }
    /* 502: 2d06 6050 6052 0b12 3000 b000 */
    {
        float n6 = r6;
        float u502_0v = 0.0f, u502_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o6 = n6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = 0x1.0000000000000p+0f; u502_0v = rs ? rvv : n6; u502_0z = rs ? rzd : n6; }
          n6 = o6 + _w * (n6 - o6); }
        float u502_1v = 0.0f, u502_1z = 0.0f;
        { float _w = (1.0f - f5); float o6 = n6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = -0x1.0000000000000p+0f; u502_1v = rs ? rvv : n6; u502_1z = rs ? rzd : n6; }
          n6 = o6 + _w * (n6 - o6); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u502_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u502_0v)); pv7 = (1.0f - fmaxf(s1_sign(u502_0v), 0.0f) * s1_abs(s1_sign(u502_0z)));
        pm2 = (1.0f - f5); pv9 = fminf(1.0f, 1.0f + s1_sign(u502_1v)); pv11 = (1.0f - fmaxf(s1_sign(u502_1v), 0.0f) * s1_abs(s1_sign(u502_1z)));
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 503: 2509 1964 6c46 5fe2 29c0 009b 0024 752f fdb8 */
    {
        float n1 = r1, n5 = r5, n6 = r6;
        float u503_0v = 0.0f, u503_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (0x1.23a8000000000p-11f * r5) + (0x1.ffb7000000000p-1f * r2); u503_0v = rs ? rvv : n1; u503_0z = rs ? rzd : n1; }
        float u503_1v = 0.0f, u503_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r1 + r6); u503_1v = rs ? rvv : n6; u503_1z = rs ? rzd : n6; }
          n5 = c_09b;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u503_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u503_0v)); pv7 = (1.0f - fmaxf(s1_sign(u503_0v), 0.0f) * s1_abs(s1_sign(u503_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u503_1v)); pv11 = (1.0f - fmaxf(s1_sign(u503_1v), 0.0f) * s1_abs(s1_sign(u503_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r5 = n5;
        r6 = n6;
    }
    /* 504: 2509 1464 2d0d 9fe2 0027 009a a3ce d9ac 3126 */
    {
        float n1 = r1, n2 = r2;
        float u504_0v = 0.0f, u504_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (-0x1.e76c800000000p-3f * r1) + (-0x1.8624c00000000p-1f); u504_0v = rs ? rvv : n1; u504_0z = rs ? rzd : n1; }
        float u504_1v = 0.0f, u504_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r4 + -r5); u504_1v = rs ? rvv : n2; u504_1z = rs ? rzd : n2; }
        c_09a = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u504_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u504_0v)); pv7 = (1.0f - fmaxf(s1_sign(u504_0v), 0.0f) * s1_abs(s1_sign(u504_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u504_1v)); pv11 = (1.0f - fmaxf(s1_sign(u504_1v), 0.0f) * s1_abs(s1_sign(u504_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
    }
    /* 505: 1806 2422 7fe0 009d 6410 0000 */
    {
        float n2 = r2, n7 = r7;
        float u505_0v = 0.0f, u505_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r0 * 0x1.0400000000000p+6f); u505_0v = rs ? rvv : n2; u505_0z = rs ? rzd : n2; }
          n7 = c_09d;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u505_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u505_0v)); pv7 = (1.0f - fmaxf(s1_sign(u505_0v), 0.0f) * s1_abs(s1_sign(u505_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r7 = n7;
    }
    /* 506: 1304 4d0a 1610 d285 */
    {
        float n4 = r4;
        float u506_0v = 0.0f, u506_0z = 0.0f;
        { float _w = (1.0f - f11); float o4 = n4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (r2 * r4) + (r5) + (r2 * -r5); u506_0v = rs ? rvv : n4; u506_0z = rs ? rzd : n4; }
          n4 = o4 + _w * (n4 - o4); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f11); pv4 = s1_abs(s1_sign(u506_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u506_0v)); pv7 = (1.0f - fmaxf(s1_sign(u506_0v), 0.0f) * s1_abs(s1_sign(u506_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 507: 4e0a 47d2 1444 cfe6 2fe8 0014 2800 2800 009b 009f */
    {
        float n1 = r1, n2 = r2, n4 = r4;
        float u507_0v = 0.0f, u507_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = -(0x1.0000000000000p-1f * r7) + (0x1.0000000000000p-1f); u507_0v = rs ? rvv : n4; u507_0z = rs ? rzd : n4; }
        float u507_1v = 0.0f, u507_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r4); u507_1v = rs ? rvv : n1; u507_1z = rs ? rzd : n1; }
        c_09b = r4;
          n2 = c_09f;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u507_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u507_0v)); pv7 = (1.0f - fmaxf(s1_sign(u507_0v), 0.0f) * s1_abs(s1_sign(u507_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u507_1v)); pv11 = (1.0f - fmaxf(s1_sign(u507_1v), 0.0f) * s1_abs(s1_sign(u507_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
        r4 = n4;
    }
    /* 508: 0a02 c502 */
    {
        float n4 = r4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = pymax(r4, r2); RESZ(n4, (r4) - (r2)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 509: 0806 4f12 2516 4300 4300 3000 */
    {
        float n4 = r4;
        float u509_0v = 0.0f, u509_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = -(0x1.8000000000000p+1f * r4) + (0x1.0000000000000p+0f) + (0x1.8000000000000p+1f * r2); u509_0v = rs ? rvv : n4; u509_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u509_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u509_0v)); pv7 = (1.0f - fmaxf(s1_sign(u509_0v), 0.0f) * s1_abs(s1_sign(u509_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 510: 0902 1444 */
    {
        float n1 = r1;
        float u510_0v = 0.0f, u510_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r4); u510_0v = rs ? rvv : n1; u510_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u510_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u510_0v)); pv7 = (1.0f - fmaxf(s1_sign(u510_0v), 0.0f) * s1_abs(s1_sign(u510_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 511: 0a02 cf06 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = s1_sign(r6) * s1_recip(fmaxf(s1_abs(r6), 0x1p-9f)); n5 = 1.0f; n6 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 512: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u512_0v = 0.0f, u512_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u512_0v = rs ? rvv : n4; u512_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u512_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u512_0v)); pv7 = (1.0f - fmaxf(s1_sign(u512_0v), 0.0f) * s1_abs(s1_sign(u512_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 513: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u513_0v = 0.0f, u513_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u513_0v = rs ? rvv : n4; u513_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u513_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u513_0v)); pv7 = (1.0f - fmaxf(s1_sign(u513_0v), 0.0f) * s1_abs(s1_sign(u513_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 514: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u514_0v = 0.0f, u514_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u514_0v = rs ? rvv : n4; u514_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u514_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u514_0v)); pv7 = (1.0f - fmaxf(s1_sign(u514_0v), 0.0f) * s1_abs(s1_sign(u514_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 515: 1504 3503 6410 7000 */
    {
        float n3 = r3, n6 = r6;
        float u515_0v = 0.0f, u515_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r4 * r3); u515_0v = rs ? rvv : n3; u515_0z = rs ? rzd : n3; }
        float u515_1v = 0.0f, u515_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r0 * 0x1.0000000000000p+8f); u515_1v = rs ? rvv : n6; u515_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u515_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u515_0v)); pv7 = (1.0f - fmaxf(s1_sign(u515_0v), 0.0f) * s1_abs(s1_sign(u515_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u515_1v)); pv11 = (1.0f - fmaxf(s1_sign(u515_1v), 0.0f) * s1_abs(s1_sign(u515_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r6 = n6;
    }
    /* 516: 0a02 cf06 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = s1_sign(r6) * s1_recip(fmaxf(s1_abs(r6), 0x1p-9f)); n5 = 1.0f; n6 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 517: 1603 4086 beeb */
    {
        float n3 = r3, n4 = r4, n5 = r5, n6 = r6;
        float u517_0v = 0.0f, u517_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u517_0v = rs ? rvv : n4; u517_0z = rs ? rzd : n4; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = 1 - 2 * fabs(wrap1(-r3 - 1.5f)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u517_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u517_0v)); pv7 = (1.0f - fmaxf(s1_sign(u517_0v), 0.0f) * s1_abs(s1_sign(u517_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 518: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u518_0v = 0.0f, u518_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u518_0v = rs ? rvv : n4; u518_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u518_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u518_0v)); pv7 = (1.0f - fmaxf(s1_sign(u518_0v), 0.0f) * s1_abs(s1_sign(u518_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 519: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u519_0v = 0.0f, u519_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u519_0v = rs ? rvv : n4; u519_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u519_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u519_0v)); pv7 = (1.0f - fmaxf(s1_sign(u519_0v), 0.0f) * s1_abs(s1_sign(u519_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 520: 0903 34d0 7000 */
    {
        float n3 = r3;
        float u520_0v = 0.0f, u520_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p+8f); u520_0v = rs ? rvv : n3; u520_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u520_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u520_0v)); pv7 = (1.0f - fmaxf(s1_sign(u520_0v), 0.0f) * s1_abs(s1_sign(u520_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 521: 0902 34c4 */
    {
        float n3 = r3;
        float u521_0v = 0.0f, u521_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * r4); u521_0v = rs ? rvv : n3; u521_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u521_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u521_0v)); pv7 = (1.0f - fmaxf(s1_sign(u521_0v), 0.0f) * s1_abs(s1_sign(u521_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 522: 0904 34e0 1000 0000 */
    {
        float n3 = r3;
        float u522_0v = 0.0f, u522_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p-4f); u522_0v = rs ? rvv : n3; u522_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u522_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u522_0v)); pv7 = (1.0f - fmaxf(s1_sign(u522_0v), 0.0f) * s1_abs(s1_sign(u522_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 523: 1b04 aedb 3fe0 009d */
    {
        float n2 = r2, n3 = r3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = pymax(-1.0f, pymin(1.0f, r3)); }
          n3 = c_09d;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 524: 1503 2442 3cc0 */
    {
        float n2 = r2, n3 = r3;
        float u524_0v = 0.0f, u524_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r1 * r2); u524_0v = rs ? rvv : n2; u524_0z = rs ? rzd : n2; }
        float u524_1v = 0.0f, u524_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 + r0); u524_1v = rs ? rvv : n3; u524_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u524_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u524_0v)); pv7 = (1.0f - fmaxf(s1_sign(u524_0v), 0.0f) * s1_abs(s1_sign(u524_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u524_1v)); pv11 = (1.0f - fmaxf(s1_sign(u524_1v), 0.0f) * s1_abs(s1_sign(u524_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 525: 1b04 bee3 afe0 008e */
    {
        float n3 = r3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = wrap1(r3); }
        c_08e = r2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 526: 1e05 bfe0 1fe2 009d 009e */
    {
        float n1 = r1;
        c_09d = r3;
          n1 = c_09e;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 527: 0902 3cc1 */
    {
        float n3 = r3;
        float u527_0v = 0.0f, u527_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 + r1); u527_0v = rs ? rvv : n3; u527_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u527_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u527_0v)); pv7 = (1.0f - fmaxf(s1_sign(u527_0v), 0.0f) * s1_abs(s1_sign(u527_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 528: 0c03 2fe0 009a */
    {
        float n2 = r2;
          n2 = c_09a;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 529: 2d06 4050 4052 0b12 3000 0000 */
    {
        float n4 = r4;
        float u529_0v = 0.0f, u529_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o4 = n4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = 0x1.0000000000000p+0f; u529_0v = rs ? rvv : n4; u529_0z = rs ? rzd : n4; }
          n4 = o4 + _w * (n4 - o4); }
        float u529_1v = 0.0f, u529_1z = 0.0f;
        { float _w = (1.0f - f5); float o4 = n4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = 0x0.0p+0f; u529_1v = rs ? rvv : n4; u529_1z = rs ? rzd : n4; }
          n4 = o4 + _w * (n4 - o4); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u529_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u529_0v)); pv7 = (1.0f - fmaxf(s1_sign(u529_0v), 0.0f) * s1_abs(s1_sign(u529_0z)));
        pm2 = (1.0f - f5); pv9 = fminf(1.0f, 1.0f + s1_sign(u529_1v)); pv11 = (1.0f - fmaxf(s1_sign(u529_1v), 0.0f) * s1_abs(s1_sign(u529_1z)));
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 530: 2d06 5050 5052 0b12 b000 3000 */
    {
        float n5 = r5;
        float u530_0v = 0.0f, u530_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o5 = n5;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = -0x1.0000000000000p+0f; u530_0v = rs ? rvv : n5; u530_0z = rs ? rzd : n5; }
          n5 = o5 + _w * (n5 - o5); }
        float u530_1v = 0.0f, u530_1z = 0.0f;
        { float _w = (1.0f - f5); float o5 = n5;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = 0x1.0000000000000p+0f; u530_1v = rs ? rvv : n5; u530_1z = rs ? rzd : n5; }
          n5 = o5 + _w * (n5 - o5); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u530_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u530_0v)); pv7 = (1.0f - fmaxf(s1_sign(u530_0v), 0.0f) * s1_abs(s1_sign(u530_0z)));
        pm2 = (1.0f - f5); pv9 = fminf(1.0f, 1.0f + s1_sign(u530_1v)); pv11 = (1.0f - fmaxf(s1_sign(u530_1v), 0.0f) * s1_abs(s1_sign(u530_1z)));
        pm4 = 0.0f;
        r5 = n5;
    }
    /* 531: 2d06 6050 6052 0b12 3000 b000 */
    {
        float n6 = r6;
        float u531_0v = 0.0f, u531_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o6 = n6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = 0x1.0000000000000p+0f; u531_0v = rs ? rvv : n6; u531_0z = rs ? rzd : n6; }
          n6 = o6 + _w * (n6 - o6); }
        float u531_1v = 0.0f, u531_1z = 0.0f;
        { float _w = (1.0f - f5); float o6 = n6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = -0x1.0000000000000p+0f; u531_1v = rs ? rvv : n6; u531_1z = rs ? rzd : n6; }
          n6 = o6 + _w * (n6 - o6); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u531_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u531_0v)); pv7 = (1.0f - fmaxf(s1_sign(u531_0v), 0.0f) * s1_abs(s1_sign(u531_0z)));
        pm2 = (1.0f - f5); pv9 = fminf(1.0f, 1.0f + s1_sign(u531_1v)); pv11 = (1.0f - fmaxf(s1_sign(u531_1v), 0.0f) * s1_abs(s1_sign(u531_1z)));
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 532: 2509 1964 6c46 5fe2 29c0 009b 0024 752f fdb8 */
    {
        float n1 = r1, n5 = r5, n6 = r6;
        float u532_0v = 0.0f, u532_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (0x1.23a8000000000p-11f * r5) + (0x1.ffb7000000000p-1f * r2); u532_0v = rs ? rvv : n1; u532_0z = rs ? rzd : n1; }
        float u532_1v = 0.0f, u532_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r1 + r6); u532_1v = rs ? rvv : n6; u532_1z = rs ? rzd : n6; }
          n5 = c_09b;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u532_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u532_0v)); pv7 = (1.0f - fmaxf(s1_sign(u532_0v), 0.0f) * s1_abs(s1_sign(u532_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u532_1v)); pv11 = (1.0f - fmaxf(s1_sign(u532_1v), 0.0f) * s1_abs(s1_sign(u532_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r5 = n5;
        r6 = n6;
    }
    /* 533: 2509 1464 2d0d 9fe2 0027 009a a3ce d9ac 3126 */
    {
        float n1 = r1, n2 = r2;
        float u533_0v = 0.0f, u533_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (-0x1.e76c800000000p-3f * r1) + (-0x1.8624c00000000p-1f); u533_0v = rs ? rvv : n1; u533_0z = rs ? rzd : n1; }
        float u533_1v = 0.0f, u533_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r4 + -r5); u533_1v = rs ? rvv : n2; u533_1z = rs ? rzd : n2; }
        c_09a = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u533_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u533_0v)); pv7 = (1.0f - fmaxf(s1_sign(u533_0v), 0.0f) * s1_abs(s1_sign(u533_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u533_1v)); pv11 = (1.0f - fmaxf(s1_sign(u533_1v), 0.0f) * s1_abs(s1_sign(u533_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
    }
    /* 534: 1806 2422 7fe0 009d 6410 0000 */
    {
        float n2 = r2, n7 = r7;
        float u534_0v = 0.0f, u534_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r0 * 0x1.0400000000000p+6f); u534_0v = rs ? rvv : n2; u534_0z = rs ? rzd : n2; }
          n7 = c_09d;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u534_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u534_0v)); pv7 = (1.0f - fmaxf(s1_sign(u534_0v), 0.0f) * s1_abs(s1_sign(u534_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r7 = n7;
    }
    /* 535: 1304 4d0a 1610 d285 */
    {
        float n4 = r4;
        float u535_0v = 0.0f, u535_0z = 0.0f;
        { float _w = (1.0f - f11); float o4 = n4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (r2 * r4) + (r5) + (r2 * -r5); u535_0v = rs ? rvv : n4; u535_0z = rs ? rzd : n4; }
          n4 = o4 + _w * (n4 - o4); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f11); pv4 = s1_abs(s1_sign(u535_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u535_0v)); pv7 = (1.0f - fmaxf(s1_sign(u535_0v), 0.0f) * s1_abs(s1_sign(u535_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 536: 4e0a 47d2 1444 cfe6 2fe8 0014 2800 2800 009b 009f */
    {
        float n1 = r1, n2 = r2, n4 = r4;
        float u536_0v = 0.0f, u536_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = -(0x1.0000000000000p-1f * r7) + (0x1.0000000000000p-1f); u536_0v = rs ? rvv : n4; u536_0z = rs ? rzd : n4; }
        float u536_1v = 0.0f, u536_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r4); u536_1v = rs ? rvv : n1; u536_1z = rs ? rzd : n1; }
        c_09b = r4;
          n2 = c_09f;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u536_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u536_0v)); pv7 = (1.0f - fmaxf(s1_sign(u536_0v), 0.0f) * s1_abs(s1_sign(u536_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u536_1v)); pv11 = (1.0f - fmaxf(s1_sign(u536_1v), 0.0f) * s1_abs(s1_sign(u536_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
        r4 = n4;
    }
    /* 537: 0a02 c502 */
    {
        float n4 = r4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = pymax(r4, r2); RESZ(n4, (r4) - (r2)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 538: 0806 4f12 2516 4300 4300 3000 */
    {
        float n4 = r4;
        float u538_0v = 0.0f, u538_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = -(0x1.8000000000000p+1f * r4) + (0x1.0000000000000p+0f) + (0x1.8000000000000p+1f * r2); u538_0v = rs ? rvv : n4; u538_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u538_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u538_0v)); pv7 = (1.0f - fmaxf(s1_sign(u538_0v), 0.0f) * s1_abs(s1_sign(u538_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 539: 0902 1444 */
    {
        float n1 = r1;
        float u539_0v = 0.0f, u539_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r4); u539_0v = rs ? rvv : n1; u539_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u539_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u539_0v)); pv7 = (1.0f - fmaxf(s1_sign(u539_0v), 0.0f) * s1_abs(s1_sign(u539_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 540: 0a02 cf06 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = s1_sign(r6) * s1_recip(fmaxf(s1_abs(r6), 0x1p-9f)); n5 = 1.0f; n6 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 541: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u541_0v = 0.0f, u541_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u541_0v = rs ? rvv : n4; u541_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u541_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u541_0v)); pv7 = (1.0f - fmaxf(s1_sign(u541_0v), 0.0f) * s1_abs(s1_sign(u541_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 542: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u542_0v = 0.0f, u542_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u542_0v = rs ? rvv : n4; u542_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u542_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u542_0v)); pv7 = (1.0f - fmaxf(s1_sign(u542_0v), 0.0f) * s1_abs(s1_sign(u542_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 543: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u543_0v = 0.0f, u543_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u543_0v = rs ? rvv : n4; u543_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u543_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u543_0v)); pv7 = (1.0f - fmaxf(s1_sign(u543_0v), 0.0f) * s1_abs(s1_sign(u543_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 544: 1504 3503 6410 7000 */
    {
        float n3 = r3, n6 = r6;
        float u544_0v = 0.0f, u544_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r4 * r3); u544_0v = rs ? rvv : n3; u544_0z = rs ? rzd : n3; }
        float u544_1v = 0.0f, u544_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r0 * 0x1.0000000000000p+8f); u544_1v = rs ? rvv : n6; u544_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u544_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u544_0v)); pv7 = (1.0f - fmaxf(s1_sign(u544_0v), 0.0f) * s1_abs(s1_sign(u544_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u544_1v)); pv11 = (1.0f - fmaxf(s1_sign(u544_1v), 0.0f) * s1_abs(s1_sign(u544_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r6 = n6;
    }
    /* 545: 0a02 cf06 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = s1_sign(r6) * s1_recip(fmaxf(s1_abs(r6), 0x1p-9f)); n5 = 1.0f; n6 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 546: 1603 4086 beeb */
    {
        float n3 = r3, n4 = r4, n5 = r5, n6 = r6;
        float u546_0v = 0.0f, u546_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u546_0v = rs ? rvv : n4; u546_0z = rs ? rzd : n4; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = 1 - 2 * fabs(wrap1(-r3 - 1.5f)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u546_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u546_0v)); pv7 = (1.0f - fmaxf(s1_sign(u546_0v), 0.0f) * s1_abs(s1_sign(u546_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 547: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u547_0v = 0.0f, u547_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u547_0v = rs ? rvv : n4; u547_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u547_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u547_0v)); pv7 = (1.0f - fmaxf(s1_sign(u547_0v), 0.0f) * s1_abs(s1_sign(u547_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 548: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u548_0v = 0.0f, u548_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u548_0v = rs ? rvv : n4; u548_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u548_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u548_0v)); pv7 = (1.0f - fmaxf(s1_sign(u548_0v), 0.0f) * s1_abs(s1_sign(u548_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 549: 0903 34d0 7000 */
    {
        float n3 = r3;
        float u549_0v = 0.0f, u549_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p+8f); u549_0v = rs ? rvv : n3; u549_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u549_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u549_0v)); pv7 = (1.0f - fmaxf(s1_sign(u549_0v), 0.0f) * s1_abs(s1_sign(u549_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 550: 0902 34c4 */
    {
        float n3 = r3;
        float u550_0v = 0.0f, u550_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * r4); u550_0v = rs ? rvv : n3; u550_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u550_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u550_0v)); pv7 = (1.0f - fmaxf(s1_sign(u550_0v), 0.0f) * s1_abs(s1_sign(u550_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 551: 0904 34e0 1000 0000 */
    {
        float n3 = r3;
        float u551_0v = 0.0f, u551_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p-4f); u551_0v = rs ? rvv : n3; u551_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u551_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u551_0v)); pv7 = (1.0f - fmaxf(s1_sign(u551_0v), 0.0f) * s1_abs(s1_sign(u551_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 552: 1b04 aedb 3fe0 009d */
    {
        float n2 = r2, n3 = r3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = pymax(-1.0f, pymin(1.0f, r3)); }
          n3 = c_09d;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 553: 1503 2442 3cc0 */
    {
        float n2 = r2, n3 = r3;
        float u553_0v = 0.0f, u553_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r1 * r2); u553_0v = rs ? rvv : n2; u553_0z = rs ? rzd : n2; }
        float u553_1v = 0.0f, u553_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 + r0); u553_1v = rs ? rvv : n3; u553_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u553_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u553_0v)); pv7 = (1.0f - fmaxf(s1_sign(u553_0v), 0.0f) * s1_abs(s1_sign(u553_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u553_1v)); pv11 = (1.0f - fmaxf(s1_sign(u553_1v), 0.0f) * s1_abs(s1_sign(u553_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 554: 1b04 bee3 afe0 0092 */
    {
        float n3 = r3;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = wrap1(r3); }
        c_092 = r2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 555: 1e05 bfe0 1fe2 009d 009e */
    {
        float n1 = r1;
        c_09d = r3;
          n1 = c_09e;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 556: 0902 3cc1 */
    {
        float n3 = r3;
        float u556_0v = 0.0f, u556_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 + r1); u556_0v = rs ? rvv : n3; u556_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u556_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u556_0v)); pv7 = (1.0f - fmaxf(s1_sign(u556_0v), 0.0f) * s1_abs(s1_sign(u556_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 557: 0c03 2fe0 009a */
    {
        float n2 = r2;
          n2 = c_09a;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 558: 2d06 4050 4052 0b12 3000 0000 */
    {
        float n4 = r4;
        float u558_0v = 0.0f, u558_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o4 = n4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = 0x1.0000000000000p+0f; u558_0v = rs ? rvv : n4; u558_0z = rs ? rzd : n4; }
          n4 = o4 + _w * (n4 - o4); }
        float u558_1v = 0.0f, u558_1z = 0.0f;
        { float _w = (1.0f - f5); float o4 = n4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = 0x0.0p+0f; u558_1v = rs ? rvv : n4; u558_1z = rs ? rzd : n4; }
          n4 = o4 + _w * (n4 - o4); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u558_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u558_0v)); pv7 = (1.0f - fmaxf(s1_sign(u558_0v), 0.0f) * s1_abs(s1_sign(u558_0z)));
        pm2 = (1.0f - f5); pv9 = fminf(1.0f, 1.0f + s1_sign(u558_1v)); pv11 = (1.0f - fmaxf(s1_sign(u558_1v), 0.0f) * s1_abs(s1_sign(u558_1z)));
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 559: 2d06 5050 5052 0b12 b000 3000 */
    {
        float n5 = r5;
        float u559_0v = 0.0f, u559_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o5 = n5;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = -0x1.0000000000000p+0f; u559_0v = rs ? rvv : n5; u559_0z = rs ? rzd : n5; }
          n5 = o5 + _w * (n5 - o5); }
        float u559_1v = 0.0f, u559_1z = 0.0f;
        { float _w = (1.0f - f5); float o5 = n5;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = 0x1.0000000000000p+0f; u559_1v = rs ? rvv : n5; u559_1z = rs ? rzd : n5; }
          n5 = o5 + _w * (n5 - o5); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u559_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u559_0v)); pv7 = (1.0f - fmaxf(s1_sign(u559_0v), 0.0f) * s1_abs(s1_sign(u559_0z)));
        pm2 = (1.0f - f5); pv9 = fminf(1.0f, 1.0f + s1_sign(u559_1v)); pv11 = (1.0f - fmaxf(s1_sign(u559_1v), 0.0f) * s1_abs(s1_sign(u559_1z)));
        pm4 = 0.0f;
        r5 = n5;
    }
    /* 560: 2d06 6050 6052 0b12 3000 b000 */
    {
        float n6 = r6;
        float u560_0v = 0.0f, u560_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o6 = n6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = 0x1.0000000000000p+0f; u560_0v = rs ? rvv : n6; u560_0z = rs ? rzd : n6; }
          n6 = o6 + _w * (n6 - o6); }
        float u560_1v = 0.0f, u560_1z = 0.0f;
        { float _w = (1.0f - f5); float o6 = n6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = -0x1.0000000000000p+0f; u560_1v = rs ? rvv : n6; u560_1z = rs ? rzd : n6; }
          n6 = o6 + _w * (n6 - o6); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u560_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u560_0v)); pv7 = (1.0f - fmaxf(s1_sign(u560_0v), 0.0f) * s1_abs(s1_sign(u560_0z)));
        pm2 = (1.0f - f5); pv9 = fminf(1.0f, 1.0f + s1_sign(u560_1v)); pv11 = (1.0f - fmaxf(s1_sign(u560_1v), 0.0f) * s1_abs(s1_sign(u560_1z)));
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 561: 2509 1964 6c46 5fe2 29c0 009b 0024 752f fdb8 */
    {
        float n1 = r1, n5 = r5, n6 = r6;
        float u561_0v = 0.0f, u561_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (0x1.23a8000000000p-11f * r5) + (0x1.ffb7000000000p-1f * r2); u561_0v = rs ? rvv : n1; u561_0z = rs ? rzd : n1; }
        float u561_1v = 0.0f, u561_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r1 + r6); u561_1v = rs ? rvv : n6; u561_1z = rs ? rzd : n6; }
          n5 = c_09b;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u561_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u561_0v)); pv7 = (1.0f - fmaxf(s1_sign(u561_0v), 0.0f) * s1_abs(s1_sign(u561_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u561_1v)); pv11 = (1.0f - fmaxf(s1_sign(u561_1v), 0.0f) * s1_abs(s1_sign(u561_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r5 = n5;
        r6 = n6;
    }
    /* 562: 2509 1464 2d0d 9fe2 0027 009a a3ce d9ac 3126 */
    {
        float n1 = r1, n2 = r2;
        float u562_0v = 0.0f, u562_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (-0x1.e76c800000000p-3f * r1) + (-0x1.8624c00000000p-1f); u562_0v = rs ? rvv : n1; u562_0z = rs ? rzd : n1; }
        float u562_1v = 0.0f, u562_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r4 + -r5); u562_1v = rs ? rvv : n2; u562_1z = rs ? rzd : n2; }
        c_09a = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u562_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u562_0v)); pv7 = (1.0f - fmaxf(s1_sign(u562_0v), 0.0f) * s1_abs(s1_sign(u562_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u562_1v)); pv11 = (1.0f - fmaxf(s1_sign(u562_1v), 0.0f) * s1_abs(s1_sign(u562_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
    }
    /* 563: 1806 2422 7fe0 009d 6410 0000 */
    {
        float n2 = r2, n7 = r7;
        float u563_0v = 0.0f, u563_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r0 * 0x1.0400000000000p+6f); u563_0v = rs ? rvv : n2; u563_0z = rs ? rzd : n2; }
          n7 = c_09d;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u563_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u563_0v)); pv7 = (1.0f - fmaxf(s1_sign(u563_0v), 0.0f) * s1_abs(s1_sign(u563_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r7 = n7;
    }
    /* 564: 1304 4d0a 1610 d285 */
    {
        float n4 = r4;
        float u564_0v = 0.0f, u564_0z = 0.0f;
        { float _w = (1.0f - f11); float o4 = n4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (r2 * r4) + (r5) + (r2 * -r5); u564_0v = rs ? rvv : n4; u564_0z = rs ? rzd : n4; }
          n4 = o4 + _w * (n4 - o4); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f11); pv4 = s1_abs(s1_sign(u564_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u564_0v)); pv7 = (1.0f - fmaxf(s1_sign(u564_0v), 0.0f) * s1_abs(s1_sign(u564_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 565: 4e0a 47d2 1444 cfe6 2fe8 0014 2800 2800 009b 009f */
    {
        float n1 = r1, n2 = r2, n4 = r4;
        float u565_0v = 0.0f, u565_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = -(0x1.0000000000000p-1f * r7) + (0x1.0000000000000p-1f); u565_0v = rs ? rvv : n4; u565_0z = rs ? rzd : n4; }
        float u565_1v = 0.0f, u565_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r4); u565_1v = rs ? rvv : n1; u565_1z = rs ? rzd : n1; }
        c_09b = r4;
          n2 = c_09f;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u565_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u565_0v)); pv7 = (1.0f - fmaxf(s1_sign(u565_0v), 0.0f) * s1_abs(s1_sign(u565_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u565_1v)); pv11 = (1.0f - fmaxf(s1_sign(u565_1v), 0.0f) * s1_abs(s1_sign(u565_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
        r4 = n4;
    }
    /* 566: 0a02 c502 */
    {
        float n4 = r4;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = pymax(r4, r2); RESZ(n4, (r4) - (r2)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 567: 0806 4f12 2516 4300 4300 3000 */
    {
        float n4 = r4;
        float u567_0v = 0.0f, u567_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = -(0x1.8000000000000p+1f * r4) + (0x1.0000000000000p+0f) + (0x1.8000000000000p+1f * r2); u567_0v = rs ? rvv : n4; u567_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u567_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u567_0v)); pv7 = (1.0f - fmaxf(s1_sign(u567_0v), 0.0f) * s1_abs(s1_sign(u567_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 568: 0902 1444 */
    {
        float n1 = r1;
        float u568_0v = 0.0f, u568_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r4); u568_0v = rs ? rvv : n1; u568_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u568_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u568_0v)); pv7 = (1.0f - fmaxf(s1_sign(u568_0v), 0.0f) * s1_abs(s1_sign(u568_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 569: 0a02 cf06 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = s1_sign(r6) * s1_recip(fmaxf(s1_abs(r6), 0x1p-9f)); n5 = 1.0f; n6 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 570: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u570_0v = 0.0f, u570_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u570_0v = rs ? rvv : n4; u570_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u570_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u570_0v)); pv7 = (1.0f - fmaxf(s1_sign(u570_0v), 0.0f) * s1_abs(s1_sign(u570_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 571: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u571_0v = 0.0f, u571_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u571_0v = rs ? rvv : n4; u571_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u571_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u571_0v)); pv7 = (1.0f - fmaxf(s1_sign(u571_0v), 0.0f) * s1_abs(s1_sign(u571_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 572: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u572_0v = 0.0f, u572_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u572_0v = rs ? rvv : n4; u572_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u572_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u572_0v)); pv7 = (1.0f - fmaxf(s1_sign(u572_0v), 0.0f) * s1_abs(s1_sign(u572_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 573: 1504 3503 6410 7000 */
    {
        float n3 = r3, n6 = r6;
        float u573_0v = 0.0f, u573_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r4 * r3); u573_0v = rs ? rvv : n3; u573_0z = rs ? rzd : n3; }
        float u573_1v = 0.0f, u573_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r0 * 0x1.0000000000000p+8f); u573_1v = rs ? rvv : n6; u573_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u573_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u573_0v)); pv7 = (1.0f - fmaxf(s1_sign(u573_0v), 0.0f) * s1_abs(s1_sign(u573_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u573_1v)); pv11 = (1.0f - fmaxf(s1_sign(u573_1v), 0.0f) * s1_abs(s1_sign(u573_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r6 = n6;
    }
    /* 574: 0a02 cf06 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = s1_sign(r6) * s1_recip(fmaxf(s1_abs(r6), 0x1p-9f)); n5 = 1.0f; n6 = 1.0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 575: 1603 4086 beeb */
    {
        float n3 = r3, n4 = r4, n5 = r5, n6 = r6;
        float u575_0v = 0.0f, u575_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u575_0v = rs ? rvv : n4; u575_0z = rs ? rzd : n4; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = 1 - 2 * fabs(wrap1(-r3 - 1.5f)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u575_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u575_0v)); pv7 = (1.0f - fmaxf(s1_sign(u575_0v), 0.0f) * s1_abs(s1_sign(u575_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 576: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u576_0v = 0.0f, u576_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u576_0v = rs ? rvv : n4; u576_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u576_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u576_0v)); pv7 = (1.0f - fmaxf(s1_sign(u576_0v), 0.0f) * s1_abs(s1_sign(u576_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 577: 0902 4086 */
    {
        float n4 = r4, n5 = r5, n6 = r6;
        float u577_0v = 0.0f, u577_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = r4 * r5; n5 = 2 - r5 * r6; n6 = r5 * r6; u577_0v = rs ? rvv : n4; u577_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u577_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u577_0v)); pv7 = (1.0f - fmaxf(s1_sign(u577_0v), 0.0f) * s1_abs(s1_sign(u577_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
        r6 = n6;
    }
    /* 578: 0903 34d0 7000 */
    {
        float n3 = r3;
        float u578_0v = 0.0f, u578_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p+8f); u578_0v = rs ? rvv : n3; u578_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u578_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u578_0v)); pv7 = (1.0f - fmaxf(s1_sign(u578_0v), 0.0f) * s1_abs(s1_sign(u578_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 579: 0902 34c4 */
    {
        float n3 = r3;
        float u579_0v = 0.0f, u579_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * r4); u579_0v = rs ? rvv : n3; u579_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u579_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u579_0v)); pv7 = (1.0f - fmaxf(s1_sign(u579_0v), 0.0f) * s1_abs(s1_sign(u579_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 580: 0904 34e0 1000 0000 */
    {
        float n3 = r3;
        float u580_0v = 0.0f, u580_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * 0x1.0000000000000p-4f); u580_0v = rs ? rvv : n3; u580_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u580_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u580_0v)); pv7 = (1.0f - fmaxf(s1_sign(u580_0v), 0.0f) * s1_abs(s1_sign(u580_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 581: 3a06 aedb 3fe0 7fe2 009d 00a0 */
    {
        float n2 = r2, n3 = r3, n7 = r7;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = pymax(-1.0f, pymin(1.0f, r3)); }
          n3 = c_09d;
          n7 = c_0a0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
        r7 = n7;
    }
    /* 582: b40a 2442 34c7 fe64 5fe0 6fe2 008a 0099 0251 8c00 */
    {
        float n2 = r2, n3 = r3, n5 = r5, n6 = r6, n7 = r7;
        float u582_0v = 0.0f, u582_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r1 * r2); u582_0v = rs ? rvv : n2; u582_0z = rs ? rzd : n2; }
        float u582_1v = 0.0f, u582_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * r7); u582_1v = rs ? rvv : n3; u582_1z = rs ? rzd : n3; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.28c6000000000p-7f; }
          n5 = c_08a;
          n6 = c_099;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u582_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u582_0v)); pv7 = (1.0f - fmaxf(s1_sign(u582_0v), 0.0f) * s1_abs(s1_sign(u582_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u582_1v)); pv11 = (1.0f - fmaxf(s1_sign(u582_1v), 0.0f) * s1_abs(s1_sign(u582_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 583: 9c0c 0b4f 7068 afe2 5fe4 6fe6 e3c0 0096 008e 0095 038d 7f00 */
    {
        float n0 = r0, n5 = r5, n6 = r6, n7 = r7;
        float u583_0v = 0.0f, u583_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r5) + (r7 * -r6); u583_0v = rs ? rvv : n0; u583_0z = rs ? rzd : n0; }
        float u583_1v = 0.0f, u583_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.c6bf800000000p-7f; u583_1v = rs ? rvv : n7; u583_1z = rs ? rzd : n7; }
        c_096 = r2;
          n5 = c_08e;
          n6 = c_095;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u583_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u583_0v)); pv7 = (1.0f - fmaxf(s1_sign(u583_0v), 0.0f) * s1_abs(s1_sign(u583_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u583_1v)); pv11 = (1.0f - fmaxf(s1_sign(u583_1v), 0.0f) * s1_abs(s1_sign(u583_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 584: 4e0a 0f4f 7066 5fe2 6fe4 e3c0 0092 0091 0706 6c00 */
    {
        float n0 = r0, n5 = r5, n6 = r6, n7 = r7;
        float u584_0v = 0.0f, u584_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r5) + (r0) + (r7 * -r6); u584_0v = rs ? rvv : n0; u584_0z = rs ? rzd : n0; }
        float u584_1v = 0.0f, u584_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.c19b000000000p-6f; u584_1v = rs ? rvv : n7; u584_1z = rs ? rzd : n7; }
          n5 = c_092;
          n6 = c_091;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u584_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u584_0v)); pv7 = (1.0f - fmaxf(s1_sign(u584_0v), 0.0f) * s1_abs(s1_sign(u584_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u584_1v)); pv11 = (1.0f - fmaxf(s1_sign(u584_1v), 0.0f) * s1_abs(s1_sign(u584_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 585: 4e0a 0f4f 7066 5fe2 6fe4 e3c0 0096 008d 0c3c 2900 */
    {
        float n0 = r0, n5 = r5, n6 = r6, n7 = r7;
        float u585_0v = 0.0f, u585_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r5) + (r0) + (r7 * -r6); u585_0v = rs ? rvv : n0; u585_0z = rs ? rzd : n0; }
        float u585_1v = 0.0f, u585_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.8785200000000p-5f; u585_1v = rs ? rvv : n7; u585_1z = rs ? rzd : n7; }
          n5 = c_096;
          n6 = c_08d;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u585_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u585_0v)); pv7 = (1.0f - fmaxf(s1_sign(u585_0v), 0.0f) * s1_abs(s1_sign(u585_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u585_1v)); pv11 = (1.0f - fmaxf(s1_sign(u585_1v), 0.0f) * s1_abs(s1_sign(u585_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 586: 4e0a 0f4f 7066 5fe2 6fe4 e3c0 008b 0098 1252 4f00 */
    {
        float n0 = r0, n5 = r5, n6 = r6, n7 = r7;
        float u586_0v = 0.0f, u586_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r5) + (r0) + (r7 * -r6); u586_0v = rs ? rvv : n0; u586_0z = rs ? rzd : n0; }
        float u586_1v = 0.0f, u586_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.2524f00000000p-4f; u586_1v = rs ? rvv : n7; u586_1z = rs ? rzd : n7; }
          n5 = c_08b;
          n6 = c_098;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u586_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u586_0v)); pv7 = (1.0f - fmaxf(s1_sign(u586_0v), 0.0f) * s1_abs(s1_sign(u586_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u586_1v)); pv11 = (1.0f - fmaxf(s1_sign(u586_1v), 0.0f) * s1_abs(s1_sign(u586_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 587: 4e0a 0f4f 7066 5fe2 6fe4 e3c0 008f 0094 1839 0b00 */
    {
        float n0 = r0, n5 = r5, n6 = r6, n7 = r7;
        float u587_0v = 0.0f, u587_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r5) + (r0) + (r7 * -r6); u587_0v = rs ? rvv : n0; u587_0z = rs ? rzd : n0; }
        float u587_1v = 0.0f, u587_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.8390b00000000p-4f; u587_1v = rs ? rvv : n7; u587_1z = rs ? rzd : n7; }
          n5 = c_08f;
          n6 = c_094;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u587_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u587_0v)); pv7 = (1.0f - fmaxf(s1_sign(u587_0v), 0.0f) * s1_abs(s1_sign(u587_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u587_1v)); pv11 = (1.0f - fmaxf(s1_sign(u587_1v), 0.0f) * s1_abs(s1_sign(u587_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 588: 4e0a 0f4f 7066 5fe2 6fe4 e3c0 0093 0090 1ce1 4900 */
    {
        float n0 = r0, n5 = r5, n6 = r6, n7 = r7;
        float u588_0v = 0.0f, u588_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r5) + (r0) + (r7 * -r6); u588_0v = rs ? rvv : n0; u588_0z = rs ? rzd : n0; }
        float u588_1v = 0.0f, u588_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.ce14900000000p-4f; u588_1v = rs ? rvv : n7; u588_1z = rs ? rzd : n7; }
          n5 = c_093;
          n6 = c_090;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u588_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u588_0v)); pv7 = (1.0f - fmaxf(s1_sign(u588_0v), 0.0f) * s1_abs(s1_sign(u588_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u588_1v)); pv11 = (1.0f - fmaxf(s1_sign(u588_1v), 0.0f) * s1_abs(s1_sign(u588_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 589: 4e0a 0f4f 7066 5fe2 6fe4 e3c0 0097 008c 1f71 b900 */
    {
        float n0 = r0, n5 = r5, n6 = r6, n7 = r7;
        float u589_0v = 0.0f, u589_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r5) + (r0) + (r7 * -r6); u589_0v = rs ? rvv : n0; u589_0z = rs ? rzd : n0; }
        float u589_1v = 0.0f, u589_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.f71b900000000p-4f; u589_1v = rs ? rvv : n7; u589_1z = rs ? rzd : n7; }
          n5 = c_097;
          n6 = c_08c;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u589_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u589_0v)); pv7 = (1.0f - fmaxf(s1_sign(u589_0v), 0.0f) * s1_abs(s1_sign(u589_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u589_1v)); pv11 = (1.0f - fmaxf(s1_sign(u589_1v), 0.0f) * s1_abs(s1_sign(u589_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 590: 1405 0f4f bfe2 e3c0 0088 */
    {
        float n0 = r0;
        float u590_0v = 0.0f, u590_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r7 * r5) + (r0) + (r7 * -r6); u590_0v = rs ? rvv : n0; u590_0z = rs ? rzd : n0; }
        c_088 = r3;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u590_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u590_0v)); pv7 = (1.0f - fmaxf(s1_sign(u590_0v), 0.0f) * s1_abs(s1_sign(u590_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 591: 3406 1043 8fe0 2fe2 00a1 0089 */
    {
        float n1 = r1, n2 = r2;
        float u591_0v = 0.0f, u591_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = r3; u591_0v = rs ? rvv : n1; u591_0z = rs ? rzd : n1; }
        c_0a1 = r0;
          n2 = c_089;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u591_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u591_0v)); pv7 = (1.0f - fmaxf(s1_sign(u591_0v), 0.0f) * s1_abs(s1_sign(u591_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
    }
    /* 592: 1e05 0fe0 1fe2 00a1 0052 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_0a1;
          n1 = c_052;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 593: 0c03 2fe0 0053 */
    {
        float n2 = r2;
          n2 = c_053;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 594: 250a 4474 7441 6fe2 0038 00a3 2e8a d823 2733 3333 */
    {
        float n4 = r4, n6 = r6, n7 = r7;
        float u594_0v = 0.0f, u594_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (0x1.d15b046000000p-1f * r1) + (0x1.ccccccc000000p-2f); u594_0v = rs ? rvv : n4; u594_0z = rs ? rzd : n4; }
        float u594_1v = 0.0f, u594_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r1 * r1); u594_1v = rs ? rvv : n7; u594_1z = rs ? rzd : n7; }
          n6 = c_0a3;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u594_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u594_0v)); pv7 = (1.0f - fmaxf(s1_sign(u594_0v), 0.0f) * s1_abs(s1_sign(u594_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u594_1v)); pv11 = (1.0f - fmaxf(s1_sign(u594_1v), 0.0f) * s1_abs(s1_sign(u594_1z)));
        pm4 = 0.0f;
        r4 = n4;
        r6 = n6;
        r7 = n7;
    }
    /* 595: 1106 45f2 3c0e 0004 a5be 0b56 */
    {
        float n3 = r3, n4 = r4;
        float u595_0v = 0.0f, u595_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (-0x1.6f82d58000000p-2f * r7) + (r4); u595_0v = rs ? rvv : n4; u595_0z = rs ? rzd : n4; }
        float u595_1v = 0.0f, u595_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r0 + -r6); u595_1v = rs ? rvv : n3; u595_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u595_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u595_0v)); pv7 = (1.0f - fmaxf(s1_sign(u595_0v), 0.0f) * s1_abs(s1_sign(u595_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u595_1v)); pv11 = (1.0f - fmaxf(s1_sign(u595_1v), 0.0f) * s1_abs(s1_sign(u595_1z)));
        pm4 = 0.0f;
        r3 = n3;
        r4 = n4;
    }
    /* 596: 0904 4520 00b8 7000 */
    {
        float n4 = r4;
        float u596_0v = 0.0f, u596_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r4 * 0x1.70e0000000000p-9f); u596_0v = rs ? rvv : n4; u596_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u596_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u596_0v)); pv7 = (1.0f - fmaxf(s1_sign(u596_0v), 0.0f) * s1_abs(s1_sign(u596_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 597: 1108 54b2 7482 0036 4340 0000 0000 0000 */
    {
        float n5 = r5, n7 = r7;
        float u597_0v = 0.0f, u597_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (0x1.a000000000000p+1f * r2) + (0x0.0p+0f); u597_0v = rs ? rvv : n5; u597_0z = rs ? rzd : n5; }
        float u597_1v = 0.0f, u597_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r2 * r2); u597_1v = rs ? rvv : n7; u597_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u597_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u597_0v)); pv7 = (1.0f - fmaxf(s1_sign(u597_0v), 0.0f) * s1_abs(s1_sign(u597_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u597_1v)); pv11 = (1.0f - fmaxf(s1_sign(u597_1v), 0.0f) * s1_abs(s1_sign(u597_1z)));
        pm4 = 0.0f;
        r5 = n5;
        r7 = n7;
    }
    /* 598: 1007 64cc 55f4 0006 0005 bb33 3333 */
    {
        float n5 = r5, n6 = r6;
        float u598_0v = 0.0f, u598_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (r4 * r3) + (r6); u598_0v = rs ? rvv : n6; u598_0z = rs ? rzd : n6; }
        float u598_1v = 0.0f, u598_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (-0x1.b333333000000p+0f * r7) + (r5); u598_1v = rs ? rvv : n5; u598_1z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u598_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u598_0v)); pv7 = (1.0f - fmaxf(s1_sign(u598_0v), 0.0f) * s1_abs(s1_sign(u598_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u598_1v)); pv11 = (1.0f - fmaxf(s1_sign(u598_1v), 0.0f) * s1_abs(s1_sign(u598_1z)));
        pm4 = 0.0f;
        r5 = n5;
        r6 = n6;
    }
    /* 599: 1405 058d efe2 0003 00a2 */
    {
        float n0 = r0;
        float u599_0v = 0.0f, u599_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r5 * r6) + (r3); u599_0v = rs ? rvv : n0; u599_0z = rs ? rzd : n0; }
        c_0a2 = r6;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u599_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u599_0v)); pv7 = (1.0f - fmaxf(s1_sign(u599_0v), 0.0f) * s1_abs(s1_sign(u599_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 600: 0807 64b2 0036 c291 eb85 2b99 9999 */
    {
        float n6 = r6;
        float u600_0v = 0.0f, u600_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (-0x1.48f5c28000000p+1f * r2) + (0x1.7333332000000p-1f); u600_0v = rs ? rvv : n6; u600_0z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u600_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u600_0v)); pv7 = (1.0f - fmaxf(s1_sign(u600_0v), 0.0f) * s1_abs(s1_sign(u600_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 601: 1106 65f2 75c2 0006 4352 8f5c */
    {
        float n6 = r6, n7 = r7;
        float u601_0v = 0.0f, u601_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (0x1.a947ae0000000p+1f * r7) + (r6); u601_0v = rs ? rvv : n6; u601_0z = rs ? rzd : n6; }
        float u601_1v = 0.0f, u601_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r7 * r2); u601_1v = rs ? rvv : n7; u601_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u601_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u601_0v)); pv7 = (1.0f - fmaxf(s1_sign(u601_0v), 0.0f) * s1_abs(s1_sign(u601_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u601_1v)); pv11 = (1.0f - fmaxf(s1_sign(u601_1v), 0.0f) * s1_abs(s1_sign(u601_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 602: 1106 65f2 7441 0006 b6cc cccc */
    {
        float n6 = r6, n7 = r7;
        float u602_0v = 0.0f, u602_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (-0x1.6cccccc000000p+0f * r7) + (r6); u602_0v = rs ? rvv : n6; u602_0z = rs ? rzd : n6; }
        float u602_1v = 0.0f, u602_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r1 * r1); u602_1v = rs ? rvv : n7; u602_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u602_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u602_0v)); pv7 = (1.0f - fmaxf(s1_sign(u602_0v), 0.0f) * s1_abs(s1_sign(u602_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u602_1v)); pv11 = (1.0f - fmaxf(s1_sign(u602_1v), 0.0f) * s1_abs(s1_sign(u602_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 603: 1108 6472 0406 0036 2581 74bc 2c66 6666 */
    {
        float n0 = r0, n6 = r6;
        float u603_0v = 0.0f, u603_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (0x1.605d2f0000000p-2f * r1) + (0x1.8cccccc000000p-1f); u603_0v = rs ? rvv : n6; u603_0z = rs ? rzd : n6; }
        float u603_1v = 0.0f, u603_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r6); u603_1v = rs ? rvv : n0; u603_1z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u603_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u603_0v)); pv7 = (1.0f - fmaxf(s1_sign(u603_0v), 0.0f) * s1_abs(s1_sign(u603_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u603_1v)); pv11 = (1.0f - fmaxf(s1_sign(u603_1v), 0.0f) * s1_abs(s1_sign(u603_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r6 = n6;
    }
    /* 604: 1106 65f2 75c1 0006 a284 5e36 */
    {
        float n6 = r6, n7 = r7;
        float u604_0v = 0.0f, u604_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (-0x1.422f1b0000000p-3f * r7) + (r6); u604_0v = rs ? rvv : n6; u604_0z = rs ? rzd : n6; }
        float u604_1v = 0.0f, u604_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r7 * r1); u604_1v = rs ? rvv : n7; u604_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u604_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u604_0v)); pv7 = (1.0f - fmaxf(s1_sign(u604_0v), 0.0f) * s1_abs(s1_sign(u604_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u604_1v)); pv11 = (1.0f - fmaxf(s1_sign(u604_1v), 0.0f) * s1_abs(s1_sign(u604_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 605: 0805 65f2 0006 09c8 3138 */
    {
        float n6 = r6;
        float u605_0v = 0.0f, u605_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (0x1.3906270000000p-5f * r7) + (r6); u605_0v = rs ? rvv : n6; u605_0z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u605_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u605_0v)); pv7 = (1.0f - fmaxf(s1_sign(u605_0v), 0.0f) * s1_abs(s1_sign(u605_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 606: 0902 0406 */
    {
        float n0 = r0;
        float u606_0v = 0.0f, u606_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r6); u606_0v = rs ? rvv : n0; u606_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u606_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u606_0v)); pv7 = (1.0f - fmaxf(s1_sign(u606_0v), 0.0f) * s1_abs(s1_sign(u606_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 607: 0c03 8fe0 00a4 */
    {
        c_0a4 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 608: 1e05 0fe0 1fe2 0033 0051 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_033;
          n1 = c_051;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 609: 0c03 2fe0 001b */
    {
        float n2 = r2;
          n2 = c_01b;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 610: 0902 8042 */
    {
        float n0 = r0;
        float u610_0v = 0.0f, u610_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = r1 + r2 + r0; RES(n0); u610_0v = rs ? rvv : n0; u610_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u610_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u610_0v)); pv7 = (1.0f - fmaxf(s1_sign(u610_0v), 0.0f) * s1_abs(s1_sign(u610_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 611: 0c03 8fe0 00a5 */
    {
        c_0a5 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 612: 1506 2c20 3c23 ca00 0042 0000 */
    {
        float n2 = r2, n3 = r3;
        float u612_0v = 0.0f, u612_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r0 + -0x1.4000000000000p+3f); u612_0v = rs ? rvv : n2; u612_0z = rs ? rzd : n2; }
        float u612_1v = 0.0f, u612_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r0 + 0x1.0000000000000p+1f); u612_1v = rs ? rvv : n3; u612_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u612_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u612_0v)); pv7 = (1.0f - fmaxf(s1_sign(u612_0v), 0.0f) * s1_abs(s1_sign(u612_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u612_1v)); pv11 = (1.0f - fmaxf(s1_sign(u612_1v), 0.0f) * s1_abs(s1_sign(u612_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 613: 1506 2060 3063 ca00 0042 0000 */
    {
        float n2 = r2, n3 = r3;
        float u613_0v = 0.0f, u613_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = -0x1.4000000000000p+3f; u613_0v = rs ? rvv : n2; u613_0z = rs ? rzd : n2; }
        float u613_1v = 0.0f, u613_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = 0x1.0000000000000p+1f; u613_1v = rs ? rvv : n3; u613_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u613_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u613_0v)); pv7 = (1.0f - fmaxf(s1_sign(u613_0v), 0.0f) * s1_abs(s1_sign(u613_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u613_1v)); pv11 = (1.0f - fmaxf(s1_sign(u613_1v), 0.0f) * s1_abs(s1_sign(u613_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 614: 5d05 004a 004b 0e10 1220 */
    {
        float n0 = r0;
        float u614_0v = 0.0f, u614_0z = 0.0f;
        { float _w = (1.0f - f7); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -r2; u614_0v = rs ? rvv : n0; u614_0z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        float u614_1v = 0.0f, u614_1z = 0.0f;
        { float _w = (1.0f - f9); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -r3; u614_1v = rs ? rvv : n0; u614_1z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f7); pv4 = s1_abs(s1_sign(u614_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u614_0v)); pv7 = (1.0f - fmaxf(s1_sign(u614_0v), 0.0f) * s1_abs(s1_sign(u614_0z)));
        pm2 = (1.0f - f9); pv9 = fminf(1.0f, 1.0f + s1_sign(u614_1v)); pv11 = (1.0f - fmaxf(s1_sign(u614_1v), 0.0f) * s1_abs(s1_sign(u614_1z)));
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 615: 0c03 8fe0 00a6 */
    {
        c_0a6 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 616: 0904 0c20 c4c0 0000 */
    {
        float n0 = r0;
        float u616_0v = 0.0f, u616_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + -0x1.3000000000000p+2f); u616_0v = rs ? rvv : n0; u616_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u616_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u616_0v)); pv7 = (1.0f - fmaxf(s1_sign(u616_0v), 0.0f) * s1_abs(s1_sign(u616_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 617: 0904 1430 2b17 217f */
    {
        float n1 = r1;
        float u617_0v = 0.0f, u617_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r0 * 0x1.62e42fe000000p-1f); u617_0v = rs ? rvv : n1; u617_0z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u617_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u617_0v)); pv7 = (1.0f - fmaxf(s1_sign(u617_0v), 0.0f) * s1_abs(s1_sign(u617_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 618: 0a02 8f19 */
    {
        float n0 = r0, n1 = r1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; { float _f = s1_floor(r1); n0 = s1_exp(_f); n1 = r1 - _f; } }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 619: 1108 3472 20a1 0036 0001 ae64 0012 7e4f */
    {
        float n2 = r2, n3 = r3;
        float u619_0v = 0.0f, u619_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (0x1.ae64000000000p-16f * r1) + (0x1.27e4f00000000p-12f); u619_0v = rs ? rvv : n3; u619_0z = rs ? rzd : n3; }
        float u619_1v = 0.0f, u619_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = 0.25f * r1 * r1; u619_1v = rs ? rvv : n2; u619_1z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u619_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u619_0v)); pv7 = (1.0f - fmaxf(s1_sign(u619_0v), 0.0f) * s1_abs(s1_sign(u619_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u619_1v)); pv11 = (1.0f - fmaxf(s1_sign(u619_1v), 0.0f) * s1_abs(s1_sign(u619_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 620: 0807 3cca 1cb6 002e 3bc7 01a0 1a01 */
    {
        float n3 = r3;
        float u620_0v = 0.0f, u620_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.a01a010000000p-8f) + (0x1.71de380000000p-11f * r1); u620_0v = rs ? rvv : n3; u620_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u620_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u620_0v)); pv7 = (1.0f - fmaxf(s1_sign(u620_0v), 0.0f) * s1_abs(s1_sign(u620_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 621: 0807 3cca 1cb6 0340 3403 16c1 6c16 */
    {
        float n3 = r3;
        float u621_0v = 0.0f, u621_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.6c16c16000000p-4f) + (0x1.a01a018000000p-7f * r1); u621_0v = rs ? rvv : n3; u621_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u621_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u621_0v)); pv7 = (1.0f - fmaxf(s1_sign(u621_0v), 0.0f) * s1_abs(s1_sign(u621_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 622: 0807 3cca 1cb6 2222 2222 2aaa aaaa */
    {
        float n3 = r3;
        float u622_0v = 0.0f, u622_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.5555554000000p-1f) + (0x1.1111110000000p-3f * r1); u622_0v = rs ? rvv : n3; u622_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u622_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u622_0v)); pv7 = (1.0f - fmaxf(s1_sign(u622_0v), 0.0f) * s1_abs(s1_sign(u622_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 623: 0807 3cca 1d62 4200 002a aaaa aa00 */
    {
        float n3 = r3;
        float u623_0v = 0.0f, u623_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.0000000000000p+1f) + (0x1.5555554000000p-1f * r1); u623_0v = rs ? rvv : n3; u623_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u623_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u623_0v)); pv7 = (1.0f - fmaxf(s1_sign(u623_0v), 0.0f) * s1_abs(s1_sign(u623_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 624: 0806 3cca 1912 3000 3000 0000 */
    {
        float n3 = r3;
        float u624_0v = 0.0f, u624_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = (r2 * r3) + (0x1.0000000000000p+0f) + (0x1.0000000000000p+0f * r1); u624_0v = rs ? rvv : n3; u624_0z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u624_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u624_0v)); pv7 = (1.0f - fmaxf(s1_sign(u624_0v), 0.0f) * s1_abs(s1_sign(u624_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
    }
    /* 625: 0902 0403 */
    {
        float n0 = r0;
        float u625_0v = 0.0f, u625_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r3); u625_0v = rs ? rvv : n0; u625_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u625_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u625_0v)); pv7 = (1.0f - fmaxf(s1_sign(u625_0v), 0.0f) * s1_abs(s1_sign(u625_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 626: 0c03 8fe0 00a7 */
    {
        c_0a7 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 627: 1e05 0fe0 1fe2 00a7 0058 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_0a7;
          n1 = c_058;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 628: 0904 0430 0258 bf25 */
    {
        float n0 = r0;
        float u628_0v = 0.0f, u628_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * 0x1.2c5f928000000p-7f); u628_0v = rs ? rvv : n0; u628_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u628_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u628_0v)); pv7 = (1.0f - fmaxf(s1_sign(u628_0v), 0.0f) * s1_abs(s1_sign(u628_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 629: 0001 */
    {
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 630: 1705 0070 0810 0000 0001 */
    {
        float n0 = r0;
        float u630_0v = 0.0f, u630_0z = 0.0f;
        { float _w = (1.0f - f4); float o0 = n0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = 0x1.0000000000000p-32f; u630_0v = rs ? rvv : n0; u630_0z = rs ? rzd : n0; }
          n0 = o0 + _w * (n0 - o0); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - f4); pv4 = s1_abs(s1_sign(u630_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u630_0v)); pv7 = (1.0f - fmaxf(s1_sign(u630_0v), 0.0f) * s1_abs(s1_sign(u630_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 631: 1805 6c50 2fe2 b000 00a9 */
    {
        float n2 = r2, n6 = r6;
        float u631_0v = 0.0f, u631_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r1 + -0x1.0000000000000p+0f); u631_0v = rs ? rvv : n6; u631_0z = rs ? rzd : n6; }
          n2 = c_0a9;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u631_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u631_0v)); pv7 = (1.0f - fmaxf(s1_sign(u631_0v), 0.0f) * s1_abs(s1_sign(u631_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r6 = n6;
    }
    /* 632: 0902 2c80 */
    {
        float n2 = r2;
        float u632_0v = 0.0f, u632_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r2 + r0); u632_0v = rs ? rvv : n2; u632_0z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u632_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u632_0v)); pv7 = (1.0f - fmaxf(s1_sign(u632_0v), 0.0f) * s1_abs(s1_sign(u632_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 633: 0a02 aee2 */
    {
        float n2 = r2;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = wrap1(r2); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 634: 1505 2846 34a0 4324 3f00 */
    {
        float n2 = r2, n3 = r3;
        float u634_0v = 0.0f, u634_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r6 + r1 * r2); u634_0v = rs ? rvv : n2; u634_0z = rs ? rzd : n2; }
        float u634_1v = 0.0f, u634_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r2 * 0x1.921f800000000p+1f); u634_1v = rs ? rvv : n3; u634_1z = rs ? rzd : n3; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u634_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u634_0v)); pv7 = (1.0f - fmaxf(s1_sign(u634_0v), 0.0f) * s1_abs(s1_sign(u634_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u634_1v)); pv11 = (1.0f - fmaxf(s1_sign(u634_1v), 0.0f) * s1_abs(s1_sign(u634_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 635: 1804 44c3 afe0 00a8 */
    {
        float n4 = r4;
        float u635_0v = 0.0f, u635_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r3 * r3); u635_0v = rs ? rvv : n4; u635_0z = rs ? rzd : n4; }
        c_0a8 = r2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u635_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u635_0v)); pv7 = (1.0f - fmaxf(s1_sign(u635_0v), 0.0f) * s1_abs(s1_sign(u635_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 636: 1107 0522 5503 0025 0000 0000 002e */
    {
        float n0 = r0, n5 = r5;
        float u636_0v = 0.0f, u636_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (0x0.0p+0f * r4) + (0x1.7000000000000p-19f); u636_0v = rs ? rvv : n0; u636_0z = rs ? rzd : n0; }
        float u636_1v = 0.0f, u636_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r4 * r3); u636_1v = rs ? rvv : n5; u636_1z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u636_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u636_0v)); pv7 = (1.0f - fmaxf(s1_sign(u636_0v), 0.0f) * s1_abs(s1_sign(u636_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u636_1v)); pv11 = (1.0f - fmaxf(s1_sign(u636_1v), 0.0f) * s1_abs(s1_sign(u636_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r5 = n5;
    }
    /* 637: 1107 7522 6504 0025 800d 0002 2222 */
    {
        float n6 = r6, n7 = r7;
        float u637_0v = 0.0f, u637_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = (-0x1.a000000000000p-13f * r4) + (0x1.1111000000000p-7f); u637_0v = rs ? rvv : n7; u637_0z = rs ? rzd : n7; }
        float u637_1v = 0.0f, u637_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = AWRAP(r4 * r4); u637_1v = rs ? rvv : n6; u637_1z = rs ? rzd : n6; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u637_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u637_0v)); pv7 = (1.0f - fmaxf(s1_sign(u637_0v), 0.0f) * s1_abs(s1_sign(u637_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u637_1v)); pv11 = (1.0f - fmaxf(s1_sign(u637_1v), 0.0f) * s1_abs(s1_sign(u637_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 638: 1503 0987 7505 */
    {
        float n0 = r0, n7 = r7;
        float u638_0v = 0.0f, u638_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r7 + r6 * r0); u638_0v = rs ? rvv : n0; u638_0z = rs ? rzd : n0; }
        float u638_1v = 0.0f, u638_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r4 * r5); u638_1v = rs ? rvv : n7; u638_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u638_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u638_0v)); pv7 = (1.0f - fmaxf(s1_sign(u638_0v), 0.0f) * s1_abs(s1_sign(u638_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u638_1v)); pv11 = (1.0f - fmaxf(s1_sign(u638_1v), 0.0f) * s1_abs(s1_sign(u638_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r7 = n7;
    }
    /* 639: 1407 0c0f 1fe2 5903 00a8 a2aa aa00 */
    {
        float n0 = r0, n1 = r1;
        float u639_0v = 0.0f, u639_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r7 * r0) + (r3) + (-0x1.5555000000000p-3f * r5); u639_0v = rs ? rvv : n0; u639_0z = rs ? rzd : n0; }
          n1 = c_0a8;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u639_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u639_0v)); pv7 = (1.0f - fmaxf(s1_sign(u639_0v), 0.0f) * s1_abs(s1_sign(u639_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 640: 0904 0430 4300 0000 */
    {
        float n0 = r0;
        float u640_0v = 0.0f, u640_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * 0x1.8000000000000p+1f); u640_0v = rs ? rvv : n0; u640_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u640_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u640_0v)); pv7 = (1.0f - fmaxf(s1_sign(u640_0v), 0.0f) * s1_abs(s1_sign(u640_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 641: 1e05 8fe0 2fe2 00aa 00a9 */
    {
        float n2 = r2;
        c_0aa = r0;
          n2 = c_0a9;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 642: 0c03 0fe0 00ac */
    {
        float n0 = r0;
          n0 = c_0ac;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 643: 0902 0048 */
    {
        float n0 = r0;
        float u643_0v = 0.0f, u643_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -r0; u643_0v = rs ? rvv : n0; u643_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u643_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u643_0v)); pv7 = (1.0f - fmaxf(s1_sign(u643_0v), 0.0f) * s1_abs(s1_sign(u643_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 644: 0a02 8ef0 */
    {
        float n0 = r0;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = pymod(r0, 2) / 2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 645: 0c03 8fe0 00ab */
    {
        c_0ab = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 646: 1e05 0fe0 1fe2 00aa 00ab */
    {
        float n0 = r0, n1 = r1;
          n0 = c_0aa;
          n1 = c_0ab;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 647: 100b 0424 146a 0027 002d b000 0000 0000 b000 0000 0000 */
    {
        float n0 = r0, n1 = r1;
        float u647_0v = 0.0f, u647_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (-0x1.0000000000000p+0f * r0) + (0x0.0p+0f); u647_0v = rs ? rvv : n0; u647_0z = rs ? rzd : n0; }
        float u647_1v = 0.0f, u647_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (-0x1.0000000000000p+0f * r1) + (0x0.0p+0f); u647_1v = rs ? rvv : n1; u647_1z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u647_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u647_0v)); pv7 = (1.0f - fmaxf(s1_sign(u647_0v), 0.0f) * s1_abs(s1_sign(u647_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u647_1v)); pv11 = (1.0f - fmaxf(s1_sign(u647_1v), 0.0f) * s1_abs(s1_sign(u647_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 648: 1e05 8fe0 9fe2 00ad 00ae */
    {
        c_0ad = r0;
        c_0ae = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 649: 0c03 0fe0 00ae */
    {
        float n0 = r0;
          n0 = c_0ae;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 650: 1806 6062 2fe0 00b0 11c6 3200 */
    {
        float n2 = r2, n6 = r6;
        float u650_0v = 0.0f, u650_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = 0x1.1c63200000000p-4f; u650_0v = rs ? rvv : n6; u650_0z = rs ? rzd : n6; }
          n2 = c_0b0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u650_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u650_0v)); pv7 = (1.0f - fmaxf(s1_sign(u650_0v), 0.0f) * s1_abs(s1_sign(u650_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r6 = n6;
    }
    /* 651: 0902 4c0a */
    {
        float n4 = r4;
        float u651_0v = 0.0f, u651_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r0 + -r2); u651_0v = rs ? rvv : n4; u651_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u651_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u651_0v)); pv7 = (1.0f - fmaxf(s1_sign(u651_0v), 0.0f) * s1_abs(s1_sign(u651_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 652: 0803 050e 0002 */
    {
        float n0 = r0;
        float u652_0v = 0.0f, u652_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r6 * r4) + (r2); u652_0v = rs ? rvv : n0; u652_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u652_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u652_0v)); pv7 = (1.0f - fmaxf(s1_sign(u652_0v), 0.0f) * s1_abs(s1_sign(u652_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 653: 0c03 8fe0 00af */
    {
        c_0af = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 654: 0a02 ef74 */
    {
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd;  }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 655: 1e05 0fe0 1fe2 0083 00a4 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_083;
          n1 = c_0a4;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 656: 1e05 2fe0 3fe2 00ad 00ae */
    {
        float n2 = r2, n3 = r3;
          n2 = c_0ad;
          n3 = c_0ae;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 657: 0c03 4fe0 00af */
    {
        float n4 = r4;
          n4 = c_0af;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 658: 3e07 6fe0 7fe2 cfe4 c01f c020 00b2 */
    {
        float n6 = r6, n7 = r7;
          n6 = KP(0xc01f);
          n7 = KP(0xc020);
        c_0b2 = r4;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 659: 5809 480e 6fe2 7fe4 afe6 13c0 c021 c022 00b1 */
    {
        float n4 = r4, n6 = r6, n7 = r7;
        float u659_0v = 0.0f, u659_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (r6 * r0) + (r7 * r1); u659_0v = rs ? rvv : n4; u659_0z = rs ? rzd : n4; }
          n6 = KP(0xc021);
          n7 = KP(0xc022);
        c_0b1 = r2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u659_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u659_0v)); pv7 = (1.0f - fmaxf(s1_sign(u659_0v), 0.0f) * s1_abs(s1_sign(u659_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r6 = n6;
        r7 = n7;
    }
    /* 660: 2b07 4c8e 6fe2 2fe4 33c4 c023 00b2 */
    {
        float n2 = r2, n4 = r4, n6 = r6;
        float u660_0v = 0.0f, u660_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (r6 * r2) + (r4) + (r7 * r3); u660_0v = rs ? rvv : n4; u660_0z = rs ? rzd : n4; }
          n6 = KP(0xc023);
          n2 = c_0b2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u660_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u660_0v)); pv7 = (1.0f - fmaxf(s1_sign(u660_0v), 0.0f) * s1_abs(s1_sign(u660_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r4 = n4;
        r6 = n6;
    }
    /* 661: 2b07 448e 6fe2 7fe4 0004 c024 c025 */
    {
        float n4 = r4, n6 = r6, n7 = r7;
        float u661_0v = 0.0f, u661_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (r6 * r2) + (r4); u661_0v = rs ? rvv : n4; u661_0z = rs ? rzd : n4; }
          n6 = KP(0xc024);
          n7 = KP(0xc025);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u661_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u661_0v)); pv7 = (1.0f - fmaxf(s1_sign(u661_0v), 0.0f) * s1_abs(s1_sign(u661_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r6 = n6;
        r7 = n7;
    }
    /* 662: 5809 580e 6fe2 7fe4 2fe6 13c0 c026 c027 00b1 */
    {
        float n2 = r2, n5 = r5, n6 = r6, n7 = r7;
        float u662_0v = 0.0f, u662_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (r6 * r0) + (r7 * r1); u662_0v = rs ? rvv : n5; u662_0z = rs ? rzd : n5; }
          n6 = KP(0xc026);
          n7 = KP(0xc027);
          n2 = c_0b1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u662_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u662_0v)); pv7 = (1.0f - fmaxf(s1_sign(u662_0v), 0.0f) * s1_abs(s1_sign(u662_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r5 = n5;
        r6 = n6;
        r7 = n7;
    }
    /* 663: 2b07 5c8e 6fe2 2fe4 33c5 c028 00b2 */
    {
        float n2 = r2, n5 = r5, n6 = r6;
        float u663_0v = 0.0f, u663_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (r6 * r2) + (r5) + (r7 * r3); u663_0v = rs ? rvv : n5; u663_0z = rs ? rzd : n5; }
          n6 = KP(0xc028);
          n2 = c_0b2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u663_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u663_0v)); pv7 = (1.0f - fmaxf(s1_sign(u663_0v), 0.0f) * s1_abs(s1_sign(u663_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r5 = n5;
        r6 = n6;
    }
    /* 664: 0803 548e 0005 */
    {
        float n5 = r5;
        float u664_0v = 0.0f, u664_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (r6 * r2) + (r5); u664_0v = rs ? rvv : n5; u664_0z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u664_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u664_0v)); pv7 = (1.0f - fmaxf(s1_sign(u664_0v), 0.0f) * s1_abs(s1_sign(u664_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r5 = n5;
    }
    /* 665: 1505 4510 5552 5000 5000 */
    {
        float n4 = r4, n5 = r5;
        float u665_0v = 0.0f, u665_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r4 * 0x1.0000000000000p+4f); u665_0v = rs ? rvv : n4; u665_0z = rs ? rzd : n4; }
        float u665_1v = 0.0f, u665_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r5 * 0x1.0000000000000p+4f); u665_1v = rs ? rvv : n5; u665_1z = rs ? rzd : n5; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u665_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u665_0v)); pv7 = (1.0f - fmaxf(s1_sign(u665_0v), 0.0f) * s1_abs(s1_sign(u665_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u665_1v)); pv11 = (1.0f - fmaxf(s1_sign(u665_1v), 0.0f) * s1_abs(s1_sign(u665_1z)));
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
    }
    /* 666: 0902 0505 */
    {
        float n0 = r0;
        float u666_0v = 0.0f, u666_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r4 * r5); u666_0v = rs ? rvv : n0; u666_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u666_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u666_0v)); pv7 = (1.0f - fmaxf(s1_sign(u666_0v), 0.0f) * s1_abs(s1_sign(u666_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 667: 0903 0410 1000 */
    {
        float n0 = r0;
        float u667_0v = 0.0f, u667_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * 0x1.0000000000000p-4f); u667_0v = rs ? rvv : n0; u667_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u667_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u667_0v)); pv7 = (1.0f - fmaxf(s1_sign(u667_0v), 0.0f) * s1_abs(s1_sign(u667_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 668: 0c03 8fe0 00b3 */
    {
        c_0b3 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 669: 1e05 0fe0 1fe2 0083 00a4 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_083;
          n1 = c_0a4;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 670: 1e05 2fe0 3fe2 00ad 00ae */
    {
        float n2 = r2, n3 = r3;
          n2 = c_0ad;
          n3 = c_0ae;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 671: 1e05 4fe0 5fe2 00af 00b3 */
    {
        float n4 = r4, n5 = r5;
          n4 = c_0af;
          n5 = c_0b3;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
    }
    /* 672: 1e05 6fe0 7fe2 c029 c02a */
    {
        float n6 = r6, n7 = r7;
          n6 = KP(0xc029);
          n7 = KP(0xc02a);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 673: 5f07 0406 1447 6fe0 7fe2 c02b c02c */
    {
        float n0 = r0, n1 = r1, n6 = r6, n7 = r7;
        float u673_0v = 0.0f, u673_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r6); u673_0v = rs ? rvv : n0; u673_0z = rs ? rzd : n0; }
        float u673_1v = 0.0f, u673_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r7); u673_1v = rs ? rvv : n1; u673_1z = rs ? rzd : n1; }
          n6 = KP(0xc02b);
          n7 = KP(0xc02c);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u673_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u673_0v)); pv7 = (1.0f - fmaxf(s1_sign(u673_0v), 0.0f) * s1_abs(s1_sign(u673_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u673_1v)); pv11 = (1.0f - fmaxf(s1_sign(u673_1v), 0.0f) * s1_abs(s1_sign(u673_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
        r6 = n6;
        r7 = n7;
    }
    /* 674: b84b 2486 34c7 8fe0 9fe2 6fe4 7fe6 00b4 00b5 c02d c02e */
    {
        float n2 = r2, n3 = r3, n6 = r6, n7 = r7;
        float u674_0v = 0.0f, u674_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(r2 * r6); u674_0v = rs ? rvv : n2; u674_0z = rs ? rzd : n2; }
        float u674_1v = 0.0f, u674_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n3 = AWRAP(r3 * r7); u674_1v = rs ? rvv : n3; u674_1z = rs ? rzd : n3; }
        c_0b4 = r0;
        c_0b5 = r1;
          n6 = KP(0xc02d);
          n7 = KP(0xc02e);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u674_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u674_0v)); pv7 = (1.0f - fmaxf(s1_sign(u674_0v), 0.0f) * s1_abs(s1_sign(u674_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u674_1v)); pv11 = (1.0f - fmaxf(s1_sign(u674_1v), 0.0f) * s1_abs(s1_sign(u674_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
        r6 = n6;
        r7 = n7;
    }
    /* 675: 5f07 4506 5547 afe0 bfe2 00b6 00b7 */
    {
        float n4 = r4, n5 = r5;
        float u675_0v = 0.0f, u675_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r4 * r6); u675_0v = rs ? rvv : n4; u675_0z = rs ? rzd : n4; }
        float u675_1v = 0.0f, u675_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r5 * r7); u675_1v = rs ? rvv : n5; u675_1z = rs ? rzd : n5; }
        c_0b6 = r2;
        c_0b7 = r3;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u675_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u675_0v)); pv7 = (1.0f - fmaxf(s1_sign(u675_0v), 0.0f) * s1_abs(s1_sign(u675_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u675_1v)); pv11 = (1.0f - fmaxf(s1_sign(u675_1v), 0.0f) * s1_abs(s1_sign(u675_1z)));
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
    }
    /* 676: 1e05 cfe0 dfe2 00b8 00b9 */
    {
        c_0b8 = r4;
        c_0b9 = r5;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 677: 1e05 6fe0 7fe2 c02f c030 */
    {
        float n6 = r6, n7 = r7;
          n6 = KP(0xc02f);
          n7 = KP(0xc030);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 678: 2b07 080e 6fe2 7fe4 13c0 c031 c032 */
    {
        float n0 = r0, n6 = r6, n7 = r7;
        float u678_0v = 0.0f, u678_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r6 * r0) + (r7 * r1); u678_0v = rs ? rvv : n0; u678_0z = rs ? rzd : n0; }
          n6 = KP(0xc031);
          n7 = KP(0xc032);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u678_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u678_0v)); pv7 = (1.0f - fmaxf(s1_sign(u678_0v), 0.0f) * s1_abs(s1_sign(u678_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r6 = n6;
        r7 = n7;
    }
    /* 679: 2b07 0c8e 6fe2 7fe4 33c0 c033 c034 */
    {
        float n0 = r0, n6 = r6, n7 = r7;
        float u679_0v = 0.0f, u679_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r6 * r2) + (r0) + (r7 * r3); u679_0v = rs ? rvv : n0; u679_0z = rs ? rzd : n0; }
          n6 = KP(0xc033);
          n7 = KP(0xc034);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u679_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u679_0v)); pv7 = (1.0f - fmaxf(s1_sign(u679_0v), 0.0f) * s1_abs(s1_sign(u679_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r6 = n6;
        r7 = n7;
    }
    /* 680: 0803 0d0e 53c0 */
    {
        float n0 = r0;
        float u680_0v = 0.0f, u680_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r6 * r4) + (r0) + (r7 * r5); u680_0v = rs ? rvv : n0; u680_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u680_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u680_0v)); pv7 = (1.0f - fmaxf(s1_sign(u680_0v), 0.0f) * s1_abs(s1_sign(u680_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 681: 1e05 8fe0 1fe2 00ba 00bb */
    {
        float n1 = r1;
        c_0ba = r0;
          n1 = c_0bb;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 682: 1e05 0fe0 1fe2 0083 00ad */
    {
        float n0 = r0, n1 = r1;
          n0 = c_083;
          n1 = c_0ad;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 683: 1e05 2fe0 3fe2 00ae 00b3 */
    {
        float n2 = r2, n3 = r3;
          n2 = c_0ae;
          n3 = c_0b3;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 684: 1e05 6fe0 7fe2 c035 c036 */
    {
        float n6 = r6, n7 = r7;
          n6 = KP(0xc035);
          n7 = KP(0xc036);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 685: 2b07 080e 6fe2 7fe4 13c0 c037 c038 */
    {
        float n0 = r0, n6 = r6, n7 = r7;
        float u685_0v = 0.0f, u685_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r6 * r0) + (r7 * r1); u685_0v = rs ? rvv : n0; u685_0z = rs ? rzd : n0; }
          n6 = KP(0xc037);
          n7 = KP(0xc038);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u685_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u685_0v)); pv7 = (1.0f - fmaxf(s1_sign(u685_0v), 0.0f) * s1_abs(s1_sign(u685_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r6 = n6;
        r7 = n7;
    }
    /* 686: 0803 0c8e 33c0 */
    {
        float n0 = r0;
        float u686_0v = 0.0f, u686_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r6 * r2) + (r0) + (r7 * r3); u686_0v = rs ? rvv : n0; u686_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u686_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u686_0v)); pv7 = (1.0f - fmaxf(s1_sign(u686_0v), 0.0f) * s1_abs(s1_sign(u686_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 687: 1e05 8fe0 1fe2 00bc 00bd */
    {
        float n1 = r1;
        c_0bc = r0;
          n1 = c_0bd;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 688: 1e05 0fe0 1fe2 00a4 00ad */
    {
        float n0 = r0, n1 = r1;
          n0 = c_0a4;
          n1 = c_0ad;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 689: 1e05 2fe0 3fe2 00ae 00b3 */
    {
        float n2 = r2, n3 = r3;
          n2 = c_0ae;
          n3 = c_0b3;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 690: 1e05 6fe0 7fe2 c039 c03a */
    {
        float n6 = r6, n7 = r7;
          n6 = KP(0xc039);
          n7 = KP(0xc03a);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 691: 2b07 080e 6fe2 7fe4 13c0 c03b c03c */
    {
        float n0 = r0, n6 = r6, n7 = r7;
        float u691_0v = 0.0f, u691_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r6 * r0) + (r7 * r1); u691_0v = rs ? rvv : n0; u691_0z = rs ? rzd : n0; }
          n6 = KP(0xc03b);
          n7 = KP(0xc03c);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u691_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u691_0v)); pv7 = (1.0f - fmaxf(s1_sign(u691_0v), 0.0f) * s1_abs(s1_sign(u691_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r6 = n6;
        r7 = n7;
    }
    /* 692: 0803 0c8e 33c0 */
    {
        float n0 = r0;
        float u692_0v = 0.0f, u692_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r6 * r2) + (r0) + (r7 * r3); u692_0v = rs ? rvv : n0; u692_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u692_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u692_0v)); pv7 = (1.0f - fmaxf(s1_sign(u692_0v), 0.0f) * s1_abs(s1_sign(u692_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 693: 1e05 8fe0 1fe2 00be 00bf */
    {
        float n1 = r1;
        c_0be = r0;
          n1 = c_0bf;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 694: 1e05 0fe0 1fe2 00ba 0067 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_0ba;
          n1 = c_067;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 695: 0c03 2fe0 0068 */
    {
        float n2 = r2;
          n2 = c_068;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 696: 0c03 6fe0 00c1 */
    {
        float n6 = r6;
          n6 = c_0c1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r6 = n6;
    }
    /* 697: 1405 078a 3fe2 0000 00c2 */
    {
        float n0 = r0, n3 = r3;
        float u697_0v = 0.0f, u697_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = -(r2 * r6) + (r0); u697_0v = rs ? rvv : n0; u697_0z = rs ? rzd : n0; }
          n3 = c_0c2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u697_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u697_0v)); pv7 = (1.0f - fmaxf(s1_sign(u697_0v), 0.0f) * s1_abs(s1_sign(u697_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r3 = n3;
    }
    /* 698: 1505 4c0b 7060 0000 0000 */
    {
        float n4 = r4, n7 = r7;
        float u698_0v = 0.0f, u698_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = AWRAP(r0 + -r3); u698_0v = rs ? rvv : n4; u698_0z = rs ? rzd : n4; }
        float u698_1v = 0.0f, u698_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x0.0p+0f; u698_1v = rs ? rvv : n7; u698_1z = rs ? rzd : n7; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u698_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u698_0v)); pv7 = (1.0f - fmaxf(s1_sign(u698_0v), 0.0f) * s1_abs(s1_sign(u698_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u698_1v)); pv11 = (1.0f - fmaxf(s1_sign(u698_1v), 0.0f) * s1_abs(s1_sign(u698_1z)));
        pm4 = 0.0f;
        r4 = n4;
        r7 = n7;
    }
    /* 699: 4b09 5509 0507 fe64 4fe2 0006 00c3 0000 0000 */
    {
        float n0 = r0, n4 = r4, n5 = r5, n7 = r7;
        float u699_0v = 0.0f, u699_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (r1 * r4) + (r6); u699_0v = rs ? rvv : n5; u699_0z = rs ? rzd : n5; }
        float u699_1v = 0.0f, u699_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r4 * r7); u699_1v = rs ? rvv : n0; u699_1z = rs ? rzd : n0; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x0.0p+0f; }
          n4 = c_0c3;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u699_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u699_0v)); pv7 = (1.0f - fmaxf(s1_sign(u699_0v), 0.0f) * s1_abs(s1_sign(u699_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u699_1v)); pv11 = (1.0f - fmaxf(s1_sign(u699_1v), 0.0f) * s1_abs(s1_sign(u699_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r4 = n4;
        r5 = n5;
        r7 = n7;
    }
    /* 700: 440a 6549 054f fe66 dfe4 0003 0000 00c0 3000 0000 */
    {
        float n0 = r0, n6 = r6, n7 = r7;
        float u700_0v = 0.0f, u700_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (r1 * r5) + (r3); u700_0v = rs ? rvv : n6; u700_0z = rs ? rzd : n6; }
        float u700_1v = 0.0f, u700_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r7 * r5) + (r0); u700_1v = rs ? rvv : n0; u700_1z = rs ? rzd : n0; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.0000000000000p+0f; }
        c_0c0 = r5;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u700_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u700_0v)); pv7 = (1.0f - fmaxf(s1_sign(u700_0v), 0.0f) * s1_abs(s1_sign(u700_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u700_1v)); pv11 = (1.0f - fmaxf(s1_sign(u700_1v), 0.0f) * s1_abs(s1_sign(u700_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r6 = n6;
        r7 = n7;
    }
    /* 701: 2508 5d8f 7064 efe2 000c 00c1 0000 0000 */
    {
        float n5 = r5, n7 = r7;
        float u701_0v = 0.0f, u701_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (r6) + (-r4) + (r0); u701_0v = rs ? rvv : n5; u701_0z = rs ? rzd : n5; }
        float u701_1v = 0.0f, u701_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x0.0p+0f; u701_1v = rs ? rvv : n7; u701_1z = rs ? rzd : n7; }
        c_0c1 = r6;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u701_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u701_0v)); pv7 = (1.0f - fmaxf(s1_sign(u701_0v), 0.0f) * s1_abs(s1_sign(u701_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u701_1v)); pv11 = (1.0f - fmaxf(s1_sign(u701_1v), 0.0f) * s1_abs(s1_sign(u701_1z)));
        pm4 = 0.0f;
        r5 = n5;
        r7 = n7;
    }
    /* 702: 2307 6549 0547 fe62 0004 3000 0000 */
    {
        float n0 = r0, n6 = r6, n7 = r7;
        float u702_0v = 0.0f, u702_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (r1 * r5) + (r4); u702_0v = rs ? rvv : n6; u702_0z = rs ? rzd : n6; }
        float u702_1v = 0.0f, u702_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r5 * r7); u702_1v = rs ? rvv : n0; u702_1z = rs ? rzd : n0; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = 0x1.0000000000000p+0f; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u702_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u702_0v)); pv7 = (1.0f - fmaxf(s1_sign(u702_0v), 0.0f) * s1_abs(s1_sign(u702_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u702_1v)); pv11 = (1.0f - fmaxf(s1_sign(u702_1v), 0.0f) * s1_abs(s1_sign(u702_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r6 = n6;
        r7 = n7;
    }
    /* 703: 1405 058f efe2 0000 00c2 */
    {
        float n0 = r0;
        float u703_0v = 0.0f, u703_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r7 * r6) + (r0); u703_0v = rs ? rvv : n0; u703_0z = rs ? rzd : n0; }
        c_0c2 = r6;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u703_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u703_0v)); pv7 = (1.0f - fmaxf(s1_sign(u703_0v), 0.0f) * s1_abs(s1_sign(u703_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 704: 0c03 8fe0 00c3 */
    {
        c_0c3 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 705: 1806 6062 2fe0 00c6 0013 bb00 */
    {
        float n2 = r2, n6 = r6;
        float u705_0v = 0.0f, u705_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = 0x1.3bb0000000000p-12f; u705_0v = rs ? rvv : n6; u705_0z = rs ? rzd : n6; }
          n2 = c_0c6;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u705_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u705_0v)); pv7 = (1.0f - fmaxf(s1_sign(u705_0v), 0.0f) * s1_abs(s1_sign(u705_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r6 = n6;
    }
    /* 706: 0902 0c0a */
    {
        float n0 = r0;
        float u706_0v = 0.0f, u706_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + -r2); u706_0v = rs ? rvv : n0; u706_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u706_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u706_0v)); pv7 = (1.0f - fmaxf(s1_sign(u706_0v), 0.0f) * s1_abs(s1_sign(u706_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 707: 1405 440e 8fe2 0002 00c4 */
    {
        float n4 = r4;
        float u707_0v = 0.0f, u707_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (r6 * r0) + (r2); u707_0v = rs ? rvv : n4; u707_0z = rs ? rzd : n4; }
        c_0c4 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u707_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u707_0v)); pv7 = (1.0f - fmaxf(s1_sign(u707_0v), 0.0f) * s1_abs(s1_sign(u707_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 708: 0c03 cfe0 00c5 */
    {
        c_0c5 = r4;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 709: 1e05 0fe0 1fe2 00c4 0041 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_0c4;
          n1 = c_041;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 710: 1e05 2fe0 3fe2 004e 0053 */
    {
        float n2 = r2, n3 = r3;
          n2 = c_04e;
          n3 = c_053;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r2 = n2;
        r3 = n3;
    }
    /* 711: 1e05 4fe0 5fe2 0055 0054 */
    {
        float n4 = r4, n5 = r5;
          n4 = c_055;
          n5 = c_054;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
        r5 = n5;
    }
    /* 712: 250a 64f4 74c3 dfe2 0038 00ce 3178 f0c3 3b1b c415 */
    {
        float n6 = r6, n7 = r7;
        float u712_0v = 0.0f, u712_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (0x1.178f0c3000000p+0f * r3) + (0x1.b1bc415000000p+0f); u712_0v = rs ? rvv : n6; u712_0z = rs ? rzd : n6; }
        float u712_1v = 0.0f, u712_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n7 = AWRAP(r3 * r3); u712_1v = rs ? rvv : n7; u712_1z = rs ? rzd : n7; }
        c_0ce = r5;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u712_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u712_0v)); pv7 = (1.0f - fmaxf(s1_sign(u712_0v), 0.0f) * s1_abs(s1_sign(u712_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u712_1v)); pv11 = (1.0f - fmaxf(s1_sign(u712_1v), 0.0f) * s1_abs(s1_sign(u712_1z)));
        pm4 = 0.0f;
        r6 = n6;
        r7 = n7;
    }
    /* 713: 1108 65f2 2936 0006 299f 574e 9050 6ba7 */
    {
        float n2 = r2, n6 = r6;
        float u713_0v = 0.0f, u713_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n6 = (0x1.33eae9c000000p-1f * r7) + (r6); u713_0v = rs ? rvv : n6; u713_0z = rs ? rzd : n6; }
        float u713_1v = 0.0f, u713_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = AWRAP(-0x1.0506ba7000000p-4f + r4 * r2); u713_1v = rs ? rvv : n2; u713_1z = rs ? rzd : n2; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u713_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u713_0v)); pv7 = (1.0f - fmaxf(s1_sign(u713_0v), 0.0f) * s1_abs(s1_sign(u713_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u713_1v)); pv11 = (1.0f - fmaxf(s1_sign(u713_1v), 0.0f) * s1_abs(s1_sign(u713_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r6 = n6;
    }
    /* 714: 230c 44f7 54e4 a492 003b 0000 3cf4 932f ce96 143c 2b9d 0b00 */
    {
        float n2 = r2, n4 = r4, n5 = r5;
        float u714_0v = 0.0f, u714_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (0x1.f9d2c28000000p-1f * r3) + (0x1.c2b9d0b000000p+0f); u714_0v = rs ? rvv : n4; u714_0z = rs ? rzd : n4; }
        float u714_1v = 0.0f, u714_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = AWRAP(r3 * 0x1.cf49300000000p+0f); u714_1v = rs ? rvv : n5; u714_1z = rs ? rzd : n5; }
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = pymax(r2, 0x0.0p+0f); RESZ(n2, (r2) - (0x0.0p+0f)); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u714_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u714_0v)); pv7 = (1.0f - fmaxf(s1_sign(u714_0v), 0.0f) * s1_abs(s1_sign(u714_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u714_1v)); pv11 = (1.0f - fmaxf(s1_sign(u714_1v), 0.0f) * s1_abs(s1_sign(u714_1z)));
        pm4 = 0.0f;
        r2 = n2;
        r4 = n4;
        r5 = n5;
    }
    /* 715: 2508 45f4 0940 5fe2 0004 00ce a2b0 16ab */
    {
        float n0 = r0, n4 = r4, n5 = r5;
        float u715_0v = 0.0f, u715_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (-0x1.580b558000000p-3f * r7) + (r4); u715_0v = rs ? rvv : n4; u715_0z = rs ? rzd : n4; }
        float u715_1v = 0.0f, u715_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + r5 * r0); u715_1v = rs ? rvv : n0; u715_1z = rs ? rzd : n0; }
          n5 = c_0ce;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u715_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u715_0v)); pv7 = (1.0f - fmaxf(s1_sign(u715_0v), 0.0f) * s1_abs(s1_sign(u715_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u715_1v)); pv11 = (1.0f - fmaxf(s1_sign(u715_1v), 0.0f) * s1_abs(s1_sign(u715_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r4 = n4;
        r5 = n5;
    }
    /* 716: 2b07 5d8d 6fe2 3fe4 c344 00c8 00c9 */
    {
        float n3 = r3, n5 = r5, n6 = r6;
        float u716_0v = 0.0f, u716_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n5 = (r5 * r6) + (r4) + (r5 * -r4); u716_0v = rs ? rvv : n5; u716_0z = rs ? rzd : n5; }
          n6 = c_0c8;
          n3 = c_0c9;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u716_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u716_0v)); pv7 = (1.0f - fmaxf(s1_sign(u716_0v), 0.0f) * s1_abs(s1_sign(u716_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r3 = n3;
        r5 = n5;
        r6 = n6;
    }
    /* 717: 0803 4f8d 000b */
    {
        float n4 = r4;
        float u717_0v = 0.0f, u717_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = -(r6) + (-r3) + (r0); u717_0v = rs ? rvv : n4; u717_0z = rs ? rzd : n4; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u717_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u717_0v)); pv7 = (1.0f - fmaxf(s1_sign(u717_0v), 0.0f) * s1_abs(s1_sign(u717_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r4 = n4;
    }
    /* 718: 1107 4522 0525 0006 0130 ddad 3333 */
    {
        float n0 = r0, n4 = r4;
        float u718_0v = 0.0f, u718_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (0x1.30dd000000000p-8f * r4) + (r6); u718_0v = rs ? rvv : n4; u718_0z = rs ? rzd : n4; }
        float u718_1v = 0.0f, u718_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r4 * -0x1.a666600000000p-1f); u718_1v = rs ? rvv : n0; u718_1z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u718_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u718_0v)); pv7 = (1.0f - fmaxf(s1_sign(u718_0v), 0.0f) * s1_abs(s1_sign(u718_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u718_1v)); pv11 = (1.0f - fmaxf(s1_sign(u718_1v), 0.0f) * s1_abs(s1_sign(u718_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r4 = n4;
    }
    /* 719: 220a 0526 4529 cfe4 0000 0003 00c7 9ea5 7f01 30dd */
    {
        float n0 = r0, n4 = r4;
        float u719_0v = 0.0f, u719_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (-0x1.ea57f00000000p-4f * r4) + (r0); u719_0v = rs ? rvv : n0; u719_0z = rs ? rzd : n0; }
        float u719_1v = 0.0f, u719_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n4 = (0x1.30dd000000000p-8f * r4) + (r3); u719_1v = rs ? rvv : n4; u719_1z = rs ? rzd : n4; }
        c_0c7 = r4;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u719_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u719_0v)); pv7 = (1.0f - fmaxf(s1_sign(u719_0v), 0.0f) * s1_abs(s1_sign(u719_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u719_1v)); pv11 = (1.0f - fmaxf(s1_sign(u719_1v), 0.0f) * s1_abs(s1_sign(u719_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r4 = n4;
    }
    /* 720: 580b 0528 cfe2 6fe4 7fe6 0000 00c8 00cd 00cb 2739 6500 */
    {
        float n0 = r0, n6 = r6, n7 = r7;
        float u720_0v = 0.0f, u720_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (0x1.ce59400000000p-2f * r4) + (r0); u720_0v = rs ? rvv : n0; u720_0z = rs ? rzd : n0; }
        c_0c8 = r4;
          n6 = c_0cd;
          n7 = c_0cb;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u720_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u720_0v)); pv7 = (1.0f - fmaxf(s1_sign(u720_0v), 0.0f) * s1_abs(s1_sign(u720_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r6 = n6;
        r7 = n7;
    }
    /* 721: 1106 24a2 1c4f 000e 420a a900 */
    {
        float n1 = r1, n2 = r2;
        float u721_0v = 0.0f, u721_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (0x1.0554800000000p+1f * r2) + (-r6); u721_0v = rs ? rvv : n2; u721_0z = rs ? rzd : n2; }
        float u721_1v = 0.0f, u721_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 + -r7); u721_1v = rs ? rvv : n1; u721_1z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u721_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u721_0v)); pv7 = (1.0f - fmaxf(s1_sign(u721_0v), 0.0f) * s1_abs(s1_sign(u721_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u721_1v)); pv11 = (1.0f - fmaxf(s1_sign(u721_1v), 0.0f) * s1_abs(s1_sign(u721_1z)));
        pm4 = 0.0f;
        r1 = n1;
        r2 = n2;
    }
    /* 722: 0001 */
    {
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 723: 2109 24a4 24a7 0b12 0006 0006 0333 4100 a3da */
    {
        float n2 = r2;
        float u723_0v = 0.0f, u723_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f5)); float o2 = n2;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (0x1.99a0800000000p-7f * r2) + (r6); u723_0v = rs ? rvv : n2; u723_0z = rs ? rzd : n2; }
          n2 = o2 + _w * (n2 - o2); }
        float u723_1v = 0.0f, u723_1z = 0.0f;
        { float _w = (1.0f - f5); float o2 = n2;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n2 = (0x1.47b4000000000p-9f * r2) + (r6); u723_1v = rs ? rvv : n2; u723_1z = rs ? rzd : n2; }
          n2 = o2 + _w * (n2 - o2); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f5)); pv4 = s1_abs(s1_sign(u723_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u723_0v)); pv7 = (1.0f - fmaxf(s1_sign(u723_0v), 0.0f) * s1_abs(s1_sign(u723_0z)));
        pm2 = (1.0f - f5); pv9 = fminf(1.0f, 1.0f + s1_sign(u723_1v)); pv11 = (1.0f - fmaxf(s1_sign(u723_1v), 0.0f) * s1_abs(s1_sign(u723_1z)));
        pm4 = 0.0f;
        r2 = n2;
    }
    /* 724: 2109 1464 1467 1312 0007 0007 2665 9926 6599 */
    {
        float n1 = r1;
        float u724_0v = 0.0f, u724_0z = 0.0f;
        { float _w = (1.0f - (1.0f - f9)); float o1 = n1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (0x1.9966400000000p-2f * r1) + (r7); u724_0v = rs ? rvv : n1; u724_0z = rs ? rzd : n1; }
          n1 = o1 + _w * (n1 - o1); }
        float u724_1v = 0.0f, u724_1z = 0.0f;
        { float _w = (1.0f - f9); float o1 = n1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = (0x1.9966400000000p-2f * r1) + (r7); u724_1v = rs ? rvv : n1; u724_1z = rs ? rzd : n1; }
          n1 = o1 + _w * (n1 - o1); }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = (1.0f - (1.0f - f9)); pv4 = s1_abs(s1_sign(u724_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u724_0v)); pv7 = (1.0f - fmaxf(s1_sign(u724_0v), 0.0f) * s1_abs(s1_sign(u724_0z)));
        pm2 = (1.0f - f9); pv9 = fminf(1.0f, 1.0f + s1_sign(u724_1v)); pv11 = (1.0f - fmaxf(s1_sign(u724_1v), 0.0f) * s1_abs(s1_sign(u724_1z)));
        pm4 = 0.0f;
        r1 = n1;
    }
    /* 725: 2b07 080a afe2 9fe4 0240 00cc 00ca */
    {
        float n0 = r0;
        float u725_0v = 0.0f, u725_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (r2 * r0) + (r1 * r0); u725_0v = rs ? rvv : n0; u725_0z = rs ? rzd : n0; }
        c_0cc = r2;
        c_0ca = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u725_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u725_0v)); pv7 = (1.0f - fmaxf(s1_sign(u725_0v), 0.0f) * s1_abs(s1_sign(u725_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 726: 0c03 8fe0 00c9 */
    {
        c_0c9 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 727: 1e05 0fe0 1fe2 0057 001d */
    {
        float n0 = r0, n1 = r1;
          n0 = c_057;
          n1 = c_01d;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 728: 0902 0c01 */
    {
        float n0 = r0;
        float u728_0v = 0.0f, u728_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + r1); u728_0v = rs ? rvv : n0; u728_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u728_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u728_0v)); pv7 = (1.0f - fmaxf(s1_sign(u728_0v), 0.0f) * s1_abs(s1_sign(u728_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 729: 0c03 8fe0 00cf */
    {
        c_0cf = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 730: 0806 0422 0025 2400 0000 0000 */
    {
        float n0 = r0;
        float u730_0v = 0.0f, u730_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = (0x1.0000000000000p-2f * r0) + (0x0.0p+0f); u730_0v = rs ? rvv : n0; u730_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u730_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u730_0v)); pv7 = (1.0f - fmaxf(s1_sign(u730_0v), 0.0f) * s1_abs(s1_sign(u730_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 731: 0c03 8fe0 00d0 */
    {
        c_0d0 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 732: 1e05 0fe0 1fe2 00c9 00d0 */
    {
        float n0 = r0, n1 = r1;
          n0 = c_0c9;
          n1 = c_0d0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 733: 0902 0401 */
    {
        float n0 = r0;
        float u733_0v = 0.0f, u733_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r1); u733_0v = rs ? rvv : n0; u733_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u733_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u733_0v)); pv7 = (1.0f - fmaxf(s1_sign(u733_0v), 0.0f) * s1_abs(s1_sign(u733_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 734: 0c03 8fe0 00d1 */
    {
        c_0d1 = r0;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 735: 7809 0fe0 1fe2 6fe4 7fe6 00d1 00d1 c03d c03e */
    {
        float n0 = r0, n1 = r1, n6 = r6, n7 = r7;
          n0 = c_0d1;
          n1 = c_0d1;
          n6 = KP(0xc03d);
          n7 = KP(0xc03e);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
        r6 = n6;
        r7 = n7;
    }
    /* 736: 1503 0406 1447 */
    {
        float n0 = r0, n1 = r1;
        float u736_0v = 0.0f, u736_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 * r6); u736_0v = rs ? rvv : n0; u736_0z = rs ? rzd : n0; }
        float u736_1v = 0.0f, u736_1z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = AWRAP(r1 * r7); u736_1v = rs ? rvv : n1; u736_1z = rs ? rzd : n1; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u736_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u736_0v)); pv7 = (1.0f - fmaxf(s1_sign(u736_0v), 0.0f) * s1_abs(s1_sign(u736_0z)));
        pm2 = 1.0f; pv9 = fminf(1.0f, 1.0f + s1_sign(u736_1v)); pv11 = (1.0f - fmaxf(s1_sign(u736_1v), 0.0f) * s1_abs(s1_sign(u736_1z)));
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 737: 1e05 8fe0 9fe2 00d2 00d3 */
    {
        c_0d2 = r0;
        c_0d3 = r1;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 738: 0c03 0fe0 0004 */
    {
        float n0 = r0;
          n0 = c_004;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 739: 0903 0c10 0000 */
    {
        float n0 = r0;
        float u739_0v = 0.0f, u739_0z = 0.0f;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n0 = AWRAP(r0 + 0x0.0p+0f); u739_0v = rs ? rvv : n0; u739_0z = rs ? rzd : n0; }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 1.0f; pv4 = s1_abs(s1_sign(u739_0z)); pv5 = fminf(1.0f, 1.0f + s1_sign(u739_0v)); pv7 = (1.0f - fmaxf(s1_sign(u739_0v), 0.0f) * s1_abs(s1_sign(u739_0z)));
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 740: 0001 */
    {
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 741: 1c03 00c3 0910 */
    {
        /* control word c3: no effect */
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 742: 0c03 0fe0 00d2 */
    {
        float n0 = r0;
          n0 = c_0d2;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
    }
    /* 743: 3e07 8fe0 0fe2 1fe4 a100 006b 0033 */
    {
        float n0 = r0, n1 = r1;
        K5_a100 = r0; KOUT(0xa100, K5_a100);
          n0 = c_06b;
          n1 = c_033;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 744: 3a07 9450 8fe2 0fe4 b000 a101 0086 */
    {
        float n0 = r0, n1 = r1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = pymax(r1, -0x1.0000000000000p+0f); RESZ(n1, (r1) - (-0x1.0000000000000p+0f)); }
        K5_a101 = r0; KOUT(0xa101, K5_a101);
          n0 = c_086;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 745: 3a07 9050 8fe2 0fe4 4700 a102 00a7 */
    {
        float n0 = r0, n1 = r1;
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd; n1 = pymin(r1, 0x1.c000000000000p+2f); RESZ(n1, (r1) - (0x1.c000000000000p+2f)); }
        K5_a102 = r0; KOUT(0xa102, K5_a102);
          n0 = c_0a7;
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
        r0 = n0;
        r1 = n1;
    }
    /* 746: 0c03 8fe0 a103 */
    {
        K5_a103 = r0; KOUT(0xa103, K5_a103);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 747: 0c03 9fe0 a000 */
    {
        K5_a000 = r1; KOUT(0xa000, K5_a000);
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 748: 0a02 ff70 */
    {
        { int rs = 0; float rvv = 0, rzd = 0; (void)rs, (void)rvv, (void)rzd;  }
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 749: 0001 */
    {
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    /* 750: 0001 */
    {
        f4 = f4 + pm1 * (pv4 - f4);
        f5 = f5 + pm1 * (pv5 - f5);
        f7 = f7 + pm1 * (pv7 - f7);
        f9 = f9 + pm2 * (pv9 - f9);
        f11 = f11 + pm2 * (pv11 - f11);
        f17 = f17 + pm4 * (pv17 - f17);
        pm1 = 0.0f;
        pm2 = 0.0f;
        pm4 = 0.0f;
    }
    D_081_1 = c_081;
    D_082_1 = c_082;
    D_083_1 = c_083;
    D_084_1 = c_084;
    D_085_1 = c_085;
    D_086_1 = c_086;
    D_087_1 = c_087;
    D_088_1 = c_088;
    D_089_1 = c_089;
    D_08a_1 = c_08a;
    D_08b_1 = c_08b;
    D_08c_1 = c_08c;
    D_08d_1 = c_08d;
    D_08e_1 = c_08e;
    D_08f_1 = c_08f;
    D_090_1 = c_090;
    D_091_1 = c_091;
    D_092_1 = c_092;
    D_093_1 = c_093;
    D_094_1 = c_094;
    D_095_1 = c_095;
    D_096_1 = c_096;
    D_097_1 = c_097;
    D_098_1 = c_098;
    D_099_1 = c_099;
    D_09a_1 = c_09a;
    D_09b_1 = c_09b;
    D_09c_1 = c_09c;
    D_09d_1 = c_09d;
    D_09e_1 = c_09e;
    D_09f_1 = c_09f;
    D_0a0_1 = c_0a0;
    D_0a1_1 = c_0a1;
    D_0a2_1 = c_0a2;
    D_0a3_1 = c_0a3;
    D_0a4_1 = c_0a4;
    D_0a5_1 = c_0a5;
    D_0a6_1 = c_0a6;
    D_0a7_1 = c_0a7;
    D_0a8_1 = c_0a8;
    D_0a9_1 = c_0a9;
    D_0aa_1 = c_0aa;
    D_0ab_1 = c_0ab;
    D_0ac_1 = c_0ac;
    D_0ad_1 = c_0ad;
    D_0ae_1 = c_0ae;
    D_0af_1 = c_0af;
    D_0b0_1 = c_0b0;
    D_0b1_1 = c_0b1;
    D_0b2_1 = c_0b2;
    D_0b3_1 = c_0b3;
    D_0b4_1 = c_0b4;
    D_0b5_1 = c_0b5;
    D_0b6_1 = c_0b6;
    D_0b7_1 = c_0b7;
    D_0b8_1 = c_0b8;
    D_0b9_1 = c_0b9;
    D_0ba_1 = c_0ba;
    D_0bb_1 = c_0bb;
    D_0bc_1 = c_0bc;
    D_0bd_1 = c_0bd;
    D_0be_1 = c_0be;
    D_0bf_1 = c_0bf;
    D_0c0_1 = c_0c0;
    D_0c1_1 = c_0c1;
    D_0c2_1 = c_0c2;
    D_0c3_1 = c_0c3;
    D_0c4_1 = c_0c4;
    D_0c5_1 = c_0c5;
    D_0c6_1 = c_0c6;
    D_0c7_1 = c_0c7;
    D_0c8_1 = c_0c8;
    D_0c9_1 = c_0c9;
    D_0ca_1 = c_0ca;
    D_0cb_1 = c_0cb;
    D_0cc_1 = c_0cc;
    D_0cd_1 = c_0cd;
    D_0ce_1 = c_0ce;
    D_0cf_1 = c_0cf;
    D_0d0_1 = c_0d0;
    D_0d1_1 = c_0d1;
    D_0d2_1 = c_0d2;
}

