
#include <math.h>
#include <stdint.h>

/* ============================================================
 * CASIO CZ - PHASE DISTORTION OSCILLATOR
 *
 * Reference implementation based on documented CZ behavior
 * and reverse-engineered waveform observations.
 *
 * Waveforms:
 * 1. Saw-tooth
 * 2. Square
 * 3. Pulse
 * 4. Double Sine
 * 5. Saw-pulse
 * 6. Resonance I   (Saw-tooth window)
 * 7. Resonance II  (Triangle window)
 * 8. Resonance III (Trapezoid window)
 *
 * DCW: 0.0 .. 1.0
 * Pulse width: 0.01 .. 0.99
 * Resonance: 1 .. 16 cycles
 * ============================================================ */

#define CZ_PI       3.14159265358979323846f
#define CZ_TWO_PI   6.28318530717958647692f

typedef enum {
    CZ_WAVE_NONE = 0,
    CZ_WAVE_SAW = 1,
    CZ_WAVE_SQUARE = 2,
    CZ_WAVE_PULSE = 3,
    CZ_WAVE_DOUBLE_SINE = 4,
    CZ_WAVE_SAW_PULSE = 5,
    CZ_WAVE_RESONANCE_I = 6,
    CZ_WAVE_RESONANCE_II = 7,
    CZ_WAVE_RESONANCE_III = 8
} CZWaveform;


/* ============================================================
 * OSCILLATOR
 * ============================================================ */

typedef struct {
    float phase;
    float frequency;
    float sample_rate;

    CZWaveform wave1;
    CZWaveform wave2;

    float dcw;
    float pulse_width;
    float trapezoid_width;

    float gain;
} CZOscillator;


/* ============================================================
 * UTILITIES
 * ============================================================ */

static float cz_clamp(float x, float lo, float hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static float cz_wrap(float p)
{
    return p - floorf(p);
}

static float cz_cos(float p)
{
    return cosf(CZ_TWO_PI * p);
}

static float cz_sin(float p)
{
    return sinf(CZ_TWO_PI * p);
}

static int cz_is_resonance(CZWaveform wave)
{
    return wave >= CZ_WAVE_RESONANCE_I &&
           wave <= CZ_WAVE_RESONANCE_III;
}


/* ============================================================
 * PHASE DISTORTION MAPS
 *
 * All basic waveforms start from linear phase.
 * DCW progressively introduces the selected phase distortion.
 * ============================================================ */

/* ------------------------------------------------------------
 * 1. SAW-TOOTH
 *
 * Two linear phase segments.
 * The first half-cycle is accelerated and the second slowed.
 * ------------------------------------------------------------ */

static float cz_phase_saw(float p, float dcw)
{
    float xb;
    float pd;

    dcw = cz_clamp(dcw, 0.0f, 0.9999f);

    xb = 0.5f * (1.0f - dcw);

    if (xb < 0.0001f)
        xb = 0.0001f;

    if (p < xb) {
        pd = 0.5f * p / xb;
    } else {
        pd = 0.5f +
             0.5f * (p - xb) / (1.0f - xb);
    }

    return pd;
}


/* ------------------------------------------------------------
 * 2. SQUARE
 *
 * Phase is held around the cosine extrema, with transitions
 * between positive and negative portions.
 * ------------------------------------------------------------ */

static float cz_phase_square(float p, float dcw)
{
    float target;

    if (p < 0.25f) {
        target = 0.0f;
    }
    else if (p < 0.50f) {
        target = 2.0f * (p - 0.25f);
    }
    else if (p < 0.75f) {
        target = 0.5f;
    }
    else {
        target = 0.5f + 2.0f * (p - 0.75f);
    }

    return p + dcw * (target - p);
}


/* ------------------------------------------------------------
 * 3. PULSE
 *
 * Asymmetric phase distortion.
 * Pulse width determines the duration of the positive half.
 * ------------------------------------------------------------ */

static float cz_phase_pulse(float p, float dcw, float width)
{
    float target;

    width = cz_clamp(width, 0.001f, 0.999f);

    if (p < width) {
        target = 0.5f * p / width;
    } else {
        target = 0.5f +
                 0.5f * (p - width) / (1.0f - width);
    }

    return p + dcw * (target - p);
}


/* ------------------------------------------------------------
 * 4. DOUBLE SINE
 *
 * Two sinusoidal components within the same cycle.
 * Interpolation preserves the cycle boundaries.
 * ------------------------------------------------------------ */

static float cz_double_sine(float p, float dcw)
{
    float a;
    float b;

    a = cz_cos(p);
    b = cz_cos(2.0f * p);

    return a + dcw * (b - a);
}


/* ------------------------------------------------------------
 * 5. SAW-PULSE
 *
 * Combines the saw and pulse phase-distortion characteristics.
 * ------------------------------------------------------------ */

static float cz_saw_pulse(float p, float dcw, float width)
{
    float saw_phase;
    float pulse_phase;
    float pd;

    saw_phase = cz_phase_saw(p, dcw);
    pulse_phase = cz_phase_pulse(p, dcw, width);

    pd = 0.5f * (saw_phase + pulse_phase);

    return cz_cos(pd);
}


/* ============================================================
 * RESONANCE GENERATOR
 *
 * The CZ manual documents:
 *
 * - Resonance frequency between 1 and 16 cycles.
 * - A window function applied to the sinusoidal carrier.
 *
 * The exact original ROM and internal coefficients are not
 * published in the documentation consulted.
 * ============================================================ */

/* ------------------------------------------------------------
 * 6. RESONANCE I - SAW-TOOTH WINDOW
 *
 * Descending linear window.
 * ------------------------------------------------------------ */

static float cz_resonance_1(float p, float dcw)
{
    float window;
    float n;
    float carrier;

    window = 1.0f - p;

    n = 1.0f + 15.0f * dcw;

    carrier = cz_sin(n * p);

    return -window * carrier;
}


/* ------------------------------------------------------------
 * 7. RESONANCE II - TRIANGLE WINDOW
 *
 * Symmetrical triangular window.
 * ------------------------------------------------------------ */

static float cz_resonance_2(float p, float dcw)
{
    float window;
    float n;
    float carrier;

    window = 1.0f - 2.0f * fabsf(p - 0.5f);

    n = 1.0f + 15.0f * dcw;

    carrier = cz_sin(n * p);

    return -window * carrier;
}


/* ------------------------------------------------------------
 * 8. RESONANCE III - TRAPEZOID WINDOW
 *
 * Linear attack, plateau and linear release.
 * ------------------------------------------------------------ */

static float cz_resonance_3(
    float p,
    float dcw,
    float plateau_width)
{
    float edge;
    float window;
    float n;
    float carrier;

    plateau_width = cz_clamp(
        plateau_width, 0.0f, 0.95f);

    edge = 0.5f * (1.0f - plateau_width);

    if (p < edge) {
        window = p / edge;
    }
    else if (p > (1.0f - edge)) {
        window = (1.0f - p) / edge;
    }
    else {
        window = 1.0f;
    }

    n = 1.0f + 15.0f * dcw;

    carrier = cz_sin(n * p);

    return -window * carrier;
}


/* ============================================================
 * BASIC WAVEFORM GENERATOR
 * ============================================================ */

static float cz_basic_wave(
    CZWaveform wave,
    float p,
    float dcw,
    float pulse_width)
{
    float pd;

    switch (wave) {

        case CZ_WAVE_SAW:
            pd = cz_phase_saw(p, dcw);
            return cz_cos(pd);

        case CZ_WAVE_SQUARE:
            pd = cz_phase_square(p, dcw);
            return cz_cos(pd);

        case CZ_WAVE_PULSE:
            pd = cz_phase_pulse(p, dcw, pulse_width);
            return cz_cos(pd);

        case CZ_WAVE_DOUBLE_SINE:
            return cz_double_sine(p, dcw);

        case CZ_WAVE_SAW_PULSE:
            return cz_saw_pulse(p, dcw, pulse_width);

        default:
            return 0.0f;
    }
}


/* ============================================================
 * SINGLE WAVEFORM GENERATOR
 * ============================================================ */

static float cz_waveform(
    CZWaveform wave,
    float p,
    float dcw,
    float pulse_width,
    float trapezoid_width)
{
    switch (wave) {

        case CZ_WAVE_SAW:
        case CZ_WAVE_SQUARE:
        case CZ_WAVE_PULSE:
        case CZ_WAVE_DOUBLE_SINE:
        case CZ_WAVE_SAW_PULSE:

            return cz_basic_wave(
                wave, p, dcw, pulse_width);

        case CZ_WAVE_RESONANCE_I:

            return cz_resonance_1(p, dcw);

        case CZ_WAVE_RESONANCE_II:

            return cz_resonance_2(p, dcw);

        case CZ_WAVE_RESONANCE_III:

            return cz_resonance_3(
                p, dcw, trapezoid_width);

        default:
            return 0.0f;
    }
}


/* ============================================================
 * WAVEFORM COMBINATION VALIDATION
 *
 * Two resonant waveforms cannot be combined.
 * A basic waveform can be combined with a resonance waveform.
 * ============================================================ */

static int cz_waveform_pair_valid(
    CZWaveform wave1,
    CZWaveform wave2)
{
    if (wave1 < CZ_WAVE_SAW ||
        wave1 > CZ_WAVE_RESONANCE_III)
        return 0;

    if (wave2 == CZ_WAVE_NONE)
        return 1;

    if (wave2 < CZ_WAVE_SAW ||
        wave2 > CZ_WAVE_RESONANCE_III)
        return 0;

    if (cz_is_resonance(wave1) &&
        cz_is_resonance(wave2))
        return 0;

    return 1;
}


/* ============================================================
 * CZ DCO PROCESSING
 *
 * One selected waveform:
 *   The waveform occupies the complete cycle.
 *
 * Two selected waveforms:
 *   Waveform 1 occupies the first half.
 *   Waveform 2 occupies the second half.
 *
 * Both halves are independently normalized to [0, 1).
 * ============================================================ */

static float cz_dco_process(CZOscillator *osc)
{
    float p;
    float local_phase;
    float output;

    p = cz_wrap(osc->phase);

    if (osc->wave2 == CZ_WAVE_NONE) {

        output = cz_waveform(
            osc->wave1,
            p,
            osc->dcw,
            osc->pulse_width,
            osc->trapezoid_width);

    }
    else if (p < 0.5f) {

        local_phase = p * 2.0f;

        output = cz_waveform(
            osc->wave1,
            local_phase,
            osc->dcw,
            osc->pulse_width,
            osc->trapezoid_width);

    }
    else {

        local_phase = (p - 0.5f) * 2.0f;

        output = cz_waveform(
            osc->wave2,
            local_phase,
            osc->dcw,
            osc->pulse_width,
            osc->trapezoid_width);
    }

    /* Advance oscillator phase. */
    osc->phase += osc->frequency / osc->sample_rate;
    osc->phase -= floorf(osc->phase);

    return output * osc->gain;
}


/* ============================================================
 * EXAMPLE: OSCILLATOR 1
 *
 * Saw-tooth + Resonance II
 * ============================================================ */

static CZOscillator osc1 = {
    .phase = 0.0f,
    .frequency = 440.0f,
    .sample_rate = 96000.0f,

    .wave1 = CZ_WAVE_SAW,
    .wave2 = CZ_WAVE_RESONANCE_II,

    .dcw = 0.8f,
    .pulse_width = 0.25f,
    .trapezoid_width = 0.4f,

    .gain = 0.5f
};


/* ============================================================
 * EXAMPLE: OSCILLATOR 2
 *
 * Square + Pulse
 * ============================================================ */

static CZOscillator osc2 = {
    .phase = 0.0f,
    .frequency = 440.0f,
    .sample_rate = 96000.0f,

    .wave1 = CZ_WAVE_SQUARE,
    .wave2 = CZ_WAVE_PULSE,

    .dcw = 0.8f,
    .pulse_width = 0.25f,
    .trapezoid_width = 0.4f,

    .gain = 0.5f
};


/* ============================================================
 * COMPLETE SYNTH OUTPUT
 * ============================================================ */

static float cz_process(void)
{
    float out1;
    float out2;

    out1 = cz_dco_process(&osc1);
    out2 = cz_dco_process(&osc2);

    return 0.5f * (out1 + out2);
}
