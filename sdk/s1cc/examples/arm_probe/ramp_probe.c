/* ramp_probe.c: which DSP B parameter ids the slave ARM's hardware ramps (FCE4) reach (docs/vol1_system_architecture/s1_arm_clock.md).
 * One table p of 160 values = the DSP B parameters C060..C0FF. p[0] (C060) = a sync mark, x a 3000 Hz tone;
 * p[1..159] (C061..C0FF) summed, x a 1000 Hz tone. (224 values, to C13F: engine.c's write of the table at load left
 * the slave silent, even after a power cycle, 2026-11-03: ids >= 0x100 are not plain parameters.) With ARM_RAMPTEST=1 engine.c tests one id per 128 ms slot:
 * FCEC 0.5 at 0 ms, FCE4 to 1.0 (y = 5) at 32 ms, FCEC 0 at 96 ms; the mark is 1 during the first slot of each
 * pass (id C061). The table is 0 at knob 128 (what engine.c writes at load): only the ARM's writes move the sum.
 * Build:  sdk/s1cc/s1cc build sdk/s1cc/examples/arm_probe/ramp_probe.c
 *         NAME=RAMP-PROBE KERNEL=sdk/s1cc/build/ramp_probe sh examples/system_s1cc/build.sh */
#include "s1dsp.h"

S1_COMPILE("--no-fused --no-derive --regs 0,1,2,3,4,5,6,7");   /* every value read on its own DSP B id */
S1_BUILD("ARM_RAMPTEST=1");

S1_PARAM(p, S1_OSC1_COLOR, 160) {
    for (int i = 0; i < 160; i++)
        out[i] = knob == 128 ? 0.0f : knob / 255.0f;
}

static float p1, p2;

S1_DSP_A {
    return s1_gate() * 0.0f;
}

S1_DSP_B {
    p1 = s1_wrap(p1 + 2.0f * 1000.0f / 96000.0f);
    p2 = s1_wrap(p2 + 2.0f * 3000.0f / 96000.0f);
    float s = 0.0f;
    s += s1_param(p, 1);
    s += s1_param(p, 2);
    s += s1_param(p, 3);
    s += s1_param(p, 4);
    s += s1_param(p, 5);
    s += s1_param(p, 6);
    s += s1_param(p, 7);
    s += s1_param(p, 8);
    s += s1_param(p, 9);
    s += s1_param(p, 10);
    s += s1_param(p, 11);
    s += s1_param(p, 12);
    s += s1_param(p, 13);
    s += s1_param(p, 14);
    s += s1_param(p, 15);
    s += s1_param(p, 16);
    s += s1_param(p, 17);
    s += s1_param(p, 18);
    s += s1_param(p, 19);
    s += s1_param(p, 20);
    s += s1_param(p, 21);
    s += s1_param(p, 22);
    s += s1_param(p, 23);
    s += s1_param(p, 24);
    s += s1_param(p, 25);
    s += s1_param(p, 26);
    s += s1_param(p, 27);
    s += s1_param(p, 28);
    s += s1_param(p, 29);
    s += s1_param(p, 30);
    s += s1_param(p, 31);
    s += s1_param(p, 32);
    s += s1_param(p, 33);
    s += s1_param(p, 34);
    s += s1_param(p, 35);
    s += s1_param(p, 36);
    s += s1_param(p, 37);
    s += s1_param(p, 38);
    s += s1_param(p, 39);
    s += s1_param(p, 40);
    s += s1_param(p, 41);
    s += s1_param(p, 42);
    s += s1_param(p, 43);
    s += s1_param(p, 44);
    s += s1_param(p, 45);
    s += s1_param(p, 46);
    s += s1_param(p, 47);
    s += s1_param(p, 48);
    s += s1_param(p, 49);
    s += s1_param(p, 50);
    s += s1_param(p, 51);
    s += s1_param(p, 52);
    s += s1_param(p, 53);
    s += s1_param(p, 54);
    s += s1_param(p, 55);
    s += s1_param(p, 56);
    s += s1_param(p, 57);
    s += s1_param(p, 58);
    s += s1_param(p, 59);
    s += s1_param(p, 60);
    s += s1_param(p, 61);
    s += s1_param(p, 62);
    s += s1_param(p, 63);
    s += s1_param(p, 64);
    s += s1_param(p, 65);
    s += s1_param(p, 66);
    s += s1_param(p, 67);
    s += s1_param(p, 68);
    s += s1_param(p, 69);
    s += s1_param(p, 70);
    s += s1_param(p, 71);
    s += s1_param(p, 72);
    s += s1_param(p, 73);
    s += s1_param(p, 74);
    s += s1_param(p, 75);
    s += s1_param(p, 76);
    s += s1_param(p, 77);
    s += s1_param(p, 78);
    s += s1_param(p, 79);
    s += s1_param(p, 80);
    s += s1_param(p, 81);
    s += s1_param(p, 82);
    s += s1_param(p, 83);
    s += s1_param(p, 84);
    s += s1_param(p, 85);
    s += s1_param(p, 86);
    s += s1_param(p, 87);
    s += s1_param(p, 88);
    s += s1_param(p, 89);
    s += s1_param(p, 90);
    s += s1_param(p, 91);
    s += s1_param(p, 92);
    s += s1_param(p, 93);
    s += s1_param(p, 94);
    s += s1_param(p, 95);
    s += s1_param(p, 96);
    s += s1_param(p, 97);
    s += s1_param(p, 98);
    s += s1_param(p, 99);
    s += s1_param(p, 100);
    s += s1_param(p, 101);
    s += s1_param(p, 102);
    s += s1_param(p, 103);
    s += s1_param(p, 104);
    s += s1_param(p, 105);
    s += s1_param(p, 106);
    s += s1_param(p, 107);
    s += s1_param(p, 108);
    s += s1_param(p, 109);
    s += s1_param(p, 110);
    s += s1_param(p, 111);
    s += s1_param(p, 112);
    s += s1_param(p, 113);
    s += s1_param(p, 114);
    s += s1_param(p, 115);
    s += s1_param(p, 116);
    s += s1_param(p, 117);
    s += s1_param(p, 118);
    s += s1_param(p, 119);
    s += s1_param(p, 120);
    s += s1_param(p, 121);
    s += s1_param(p, 122);
    s += s1_param(p, 123);
    s += s1_param(p, 124);
    s += s1_param(p, 125);
    s += s1_param(p, 126);
    s += s1_param(p, 127);
    s += s1_param(p, 128);
    s += s1_param(p, 129);
    s += s1_param(p, 130);
    s += s1_param(p, 131);
    s += s1_param(p, 132);
    s += s1_param(p, 133);
    s += s1_param(p, 134);
    s += s1_param(p, 135);
    s += s1_param(p, 136);
    s += s1_param(p, 137);
    s += s1_param(p, 138);
    s += s1_param(p, 139);
    s += s1_param(p, 140);
    s += s1_param(p, 141);
    s += s1_param(p, 142);
    s += s1_param(p, 143);
    s += s1_param(p, 144);
    s += s1_param(p, 145);
    s += s1_param(p, 146);
    s += s1_param(p, 147);
    s += s1_param(p, 148);
    s += s1_param(p, 149);
    s += s1_param(p, 150);
    s += s1_param(p, 151);
    s += s1_param(p, 152);
    s += s1_param(p, 153);
    s += s1_param(p, 154);
    s += s1_param(p, 155);
    s += s1_param(p, 156);
    s += s1_param(p, 157);
    s += s1_param(p, 158);
    s += s1_param(p, 159);
    return 0.25f * (s1_tri(p1) * s + s1_tri(p2) * s1_param(p, 0)) + 0.0f * x;
}
