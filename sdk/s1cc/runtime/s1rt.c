/* s1cc runtime (docs/vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md §5): the s1_* intrinsics on the PC, for the reference build and the profiling build
 * of a voice program (same LLVM IR; s1cc prof adds the __s1p_* calls to the profiling one).
 *
 *   PROG tables FILE                     every S1_PARAM table at knob 0..255 ("p name panel count" + 256 rows)
 *   PROG ref OUT NOTE N [name=knob ...] [name.a=knob name.b=knob ...] [off=T]
 *                                        N samples of one voice: float32 pairs (S1_DSP_A, S1_DSP_B) in OUT;
 *                                        gate on from 0 (one note start), off at T (default never). With an
 *                                        S1_MASTER also OUT.m: its output, float32 per sample (name.a / name.b:
 *                                        the knobs of an S1_VOICE_PARAM)
 *   PROG prof OUT                        profiling scenarios -> ranges, overflows, island samples in OUT
 *
 * Host semantics (the same as the DSP side): s1_increment = 2 * 440 * 2^(pitch - 4.75) / 96000; pitch word =
 * (note - 12) / 12; s1_wrap = ((x + 1) mod 2) - 1, s1_tri = 1 - 2 |wrap(-x - 1.5)|.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct s1_param_desc {
    const char *name;
    int panel_id;
    int count;
    void (*fn)(int knob, float *out);
} s1_param_desc;

extern const s1_param_desc __start_s1_params[] __attribute__((weak));
extern const int s1_note_range[2] __attribute__((weak));     /* S1_NOTE_RANGE: the notes profiled */
static int note_in(int n) {
    if (!s1_note_range)
        return n;
    return n < s1_note_range[0] ? s1_note_range[0] : n > s1_note_range[1] ? s1_note_range[1] : n;
}
extern const s1_param_desc __stop_s1_params[] __attribute__((weak));
float s1_dsp_a(void);
float s1_dsp_b(float x) __attribute__((weak));
float s1_master(float x) __attribute__((weak));
void __s1p_reset(void) __attribute__((weak));
void __s1p_dspstart(void) __attribute__((weak));
void s1_init(void) __attribute__((weak));
void __s1p_state(double *out) __attribute__((weak));
extern const int __s1p_nstate __attribute__((weak));
typedef struct { int id, root; double (*fn)(double); int isint; } isl_row;   /* isint: sample integers only */
extern const isl_row __s1p_isl_tab[] __attribute__((weak));
extern const int __s1p_isl_n __attribute__((weak));

#define MAXP 512                     /* S1_PARAM tables of one voice */
static int np;
static const s1_param_desc *pd[MAXP];
static float *ptab[MAXP];             /* 256 x count */
static int knob[MAXP];
static float g_pitch, g_gate, g_trig, g_export[16];

/* S1_VOICE_PARAM tables: [note 0..127][step a 0..7][step b 0..7][count], the knob of step k = round(k 255 / 7) */
typedef struct s1_vparam_desc {
    const char *name;
    int panel_a, count;
    int panel_b;
    void (*fn)(int note, int ka, int kb, float *out);
} s1_vparam_desc;
extern const s1_vparam_desc __start_s1_vparams[] __attribute__((weak));
extern const s1_vparam_desc __stop_s1_vparams[] __attribute__((weak));
#define VSTEPS 8
static int nvp;
static const s1_vparam_desc *vpd[MAXP];
static float *vptab[MAXP];
static int vstep[MAXP][2];            /* the knobs' steps (knob >> 5), 4 = knob 128 */
static int g_note;                    /* the voice's note (the row of its S1_VOICE_PARAMs) */
static int vknob(int step) { return (step * 255 + 3) / 7; }

static void vparams_init(void) {
    if (nvp)
        return;
    for (const s1_vparam_desc *d = __start_s1_vparams; d && d < __stop_s1_vparams; d++) {
        if (nvp == MAXP) {
            fprintf(stderr, "s1rt: more than %d S1_VOICE_PARAM tables\n", MAXP);
            exit(1);
        }
        vpd[nvp] = d;
        vptab[nvp] = calloc(128 * VSTEPS * VSTEPS * (size_t)d->count, sizeof(float));
        for (int n = 0; n < 128; n++)
            for (int a = 0; a < VSTEPS; a++)
                for (int b = 0; b < VSTEPS; b++)
                    d->fn(n, d->panel_a ? vknob(a) : 128, d->panel_b ? vknob(b) : 128,
                          vptab[nvp] + ((n * VSTEPS + a) * VSTEPS + b) * d->count);
        vstep[nvp][0] = vstep[nvp][1] = 128 >> 5;
        nvp++;
    }
}
float s1_vparam_get(const s1_vparam_desc *d, int i) {
    vparams_init();
    for (int k = 0; k < nvp; k++)
        if (vpd[k] == d || !strcmp(vpd[k]->name, d->name)) {
            int n = g_note < 0 ? 0 : g_note > 127 ? 127 : g_note;
            int a = vpd[k]->panel_a ? vstep[k][0] : 128 >> 5, b = vpd[k]->panel_b ? vstep[k][1] : 128 >> 5;
            return vptab[k][((n * VSTEPS + a) * VSTEPS + b) * d->count + i];
        }
    return 0;
}

/* S1_MASTER's delay lines (the host's lines in the delay memory, one request per line and sample): line l holds
 * w[n] = what s1_line_write gave at sample n; s1_line_tap(l, d, k) = w[n - d' - 1 - k], d' = floor of the d the
 * line was read with the sample before (the host's request takes the age the main stored then; at the first sample
 * the current one) */
#define LINES 6
#define LINE_N 32768                  /* a power of 2 above the line's 16 384 samples */
static float g_line[LINES][LINE_N];
static float g_ld[LINES], g_ldprev[LINES];
static int g_ldset[LINES];
static long g_n;
float s1_line_tap(int line, float d, int k) {
    line = line < 0 ? 0 : line >= LINES ? LINES - 1 : line;
    float dd = g_ldset[line] ? g_ldprev[line] : d;
    g_ld[line] = d;
    long i = g_n - (long)floorf(dd) - 1 - (k & 1);
    return i < 0 ? 0.0f : g_line[line][i & (LINE_N - 1)];
}
void s1_line_write(int line, float w) {
    if (line >= 0 && line < LINES)
        g_line[line][g_n & (LINE_N - 1)] = w;
}
static void lines_reset(void) {
    memset(g_line, 0, sizeof g_line);
    memset(g_ldset, 0, sizeof g_ldset);
    g_n = 0;
}
static void lines_step(void) {          /* the end of a sample: the ages the requests of the next one take */
    for (int l = 0; l < LINES; l++) {
        g_ldprev[l] = g_ld[l];
        g_ldset[l] = 1;
    }
    g_n++;
}

static void params_init(void) {
    if (np)
        return;
    for (const s1_param_desc *d = __start_s1_params; d && d < __stop_s1_params; d++) {
        if (np == MAXP) {
            fprintf(stderr, "s1rt: more than %d S1_PARAM tables\n", MAXP);
            exit(1);
        }
        pd[np] = d;
        ptab[np] = calloc(256 * (size_t)d->count, sizeof(float));
        for (int k = 0; k < 256; k++)
            d->fn(k, ptab[np] + k * d->count);
        knob[np] = 128;
        np++;
    }
}

static float wrapf(float x) { float y = fmodf(x + 1.0f, 2.0f); if (y < 0) y += 2.0f; return y - 1.0f; }
float s1_pitch(void) { return g_pitch; }
float s1_increment(void) { return 2.0f * 440.0f * exp2f(g_pitch - 4.75f) / 96000.0f; }
float s1_dt(void) { return 440.0f * exp2f(g_pitch - 4.75f) / 96000.0f; }
float s1_rdt(void) { return 1.0f / s1_dt(); }
float s1_gate(void) { return g_gate; }
float s1_trig(void) { return g_trig; }
float s1_env(int i) { (void)i; return g_gate; }     /* the host's envelopes: the gate stands in (check too) */
static float g_color[2] = {128 / 255.0f, 128 / 255.0f};
static int host_sweep;          /* profiling: the colors and the LFO terms ramp over their range in each run */
static int color_arg(const char *name, int v) {     /* color0=K / color1=K: the knob s1_color gives */
    if (strcmp(name, "color0") && strcmp(name, "color1"))
        return 0;
    g_color[name[5] - '0'] = v / 255.0f;
    return 1;
}
float s1_color(int i) { return g_color[i & 1]; }
static float g_lfo_filter = 0.0f, g_lfo_amp = 1.0f;   /* the host's LFO: none in the reference; profiling sweeps */
float s1_lfo_filter(void) { return g_lfo_filter; }
float s1_lfo_amp(void) { return g_lfo_amp; }
static float g_host_mix = 0.0f;                        /* fused: the host's SUB / NOISE (none in the reference) */
float s1_host_mix(void) { return g_host_mix; }   /* COLOR: the knob (color0= / color1=); profiling sweeps 0..1 */
float s1_param_get(const s1_param_desc *d, int i) {
    params_init();
    for (int k = 0; k < np; k++)
        if (pd[k] == d || !strcmp(pd[k]->name, d->name))
            return ptab[k][knob[k] * d->count + i];
    return 0;
}
void s1_export(int i, float v) { g_export[i & 15] = v; }
float s1_import(int i) { return g_export[i & 15]; }
/* s1_ldb5(i): the wavetable frame in bank 5 (A080..). Here (native reference / profiling / check) a band-limited saw
 * stands in for whatever the host / USB loader wrote; on the device the DSP reads the loaded frame. 127 points +
 * a guard copy of point 0, peak 0.8, as make_s1cc's sl_frames() (docs/vol6_reference_designs_applications/mutable_memory.md §4.1, §4.5). */
#define S1_WT_N 127
static float g_ldb5[S1_WT_N + 1];
static int g_ldb5_ready;
float s1_ldb5(int i) {
    if (!g_ldb5_ready) {
        for (int k = 0; k < S1_WT_N; k++) {           /* band-limited saw: sum of sines 1/h, 63 harmonics */
            double y = 0, ph = 2.0 * 3.14159265358979323846 * k / S1_WT_N;
            for (int h = 1; h <= S1_WT_N / 2; h++)
                y += sinf((float)(h * ph)) / h;
            g_ldb5[k] = (float)(y * 0.8 * 2.0 / 3.14159265358979323846);
        }
        g_ldb5[S1_WT_N] = g_ldb5[0];
        g_ldb5_ready = 1;
    }
    if (i < 0) i = 0;
    if (i > S1_WT_N) i = S1_WT_N;
    return g_ldb5[i];
}
float s1_wrap(float x) { return wrapf(x); }
float s1_clamp(float x) { return x < -1 ? -1 : x > 1 ? 1 : x; }
float s1_tri(float x) { return 1.0f - 2.0f * fabsf(wrapf(-x - 1.5f)); }
float s1_exp2(float x) { return exp2f(x); }
float s1_range(float x, float lo, float hi) { (void)lo; (void)hi; return x; }
/* native ops (s1dsp.h "Native ops"): the exact function; the DSP's op is measured exact (re/dspnative.py) or, for
 * s1_recip / s1_exp / s1_exp2 / s1_log2, a seed or split with a polynomial (1e-8 .. 3e-5) */
float s1_floor(float x) { return floorf(x); }
float s1_ceil(float x) { return ceilf(x); }
float s1_sign(float x) { return x > 0 ? 1.0f : x < 0 ? -1.0f : 0.0f; }
float s1_abs(float x) { return fabsf(x); }
float s1_recip(float x) { return 1.0f / x; }
float s1_exp(float x) { return expf(x); }
float s1_log2(float x) { return log2f(x); }
float s1_ldexp(float x, int k) { return ldexpf(x, k); }
float s1_sum3(float a, float b, float c) { return a + b + c; }
float s1_mac2(float a, float b, float c, float d, float e) { return a * b + c * d + e; }
float s1_sqrt(float x) { return sqrtf(x); }
float s1_rsqrt(float x) { return 1.0f / sqrtf(x); }

/* ---- profiling records ---- */
#define MAXR 65536
static double slo[MAXR], shi[MAXR], ulo[MAXR], uhi[MAXR];
static unsigned char seen[MAXR], sov[MAXR], uov[MAXR];
static int maxid = -1;
void __s1p_h(int id, double v);
static FILE *dumpf;                    /* dump mode: "id signed unsigned" per value, "n" per sample */
static double *swv;                    /* sweep mode: the first value of every record in this knob position */
static unsigned char *swseen;
void __s1p_v(int id, double s, double u) {
    if (id < 0 || id >= MAXR)
        return;
    if (swv) {
        if (!swseen[id]) { swv[id] = s; swseen[id] = 1; }
        if (id > maxid) maxid = id;
        return;
    }
    if (dumpf)
        fprintf(dumpf, "%d %.17g %.17g\n", id, s, u);
    if (s == u)                            /* floats (and non-negative ints): the octave histogram */
        __s1p_h(id, s);
    if (!seen[id]) { slo[id] = shi[id] = s; ulo[id] = uhi[id] = u; seen[id] = 1; }
    if (s < slo[id]) slo[id] = s;
    if (s > shi[id]) shi[id] = s;
    if (u < ulo[id]) ulo[id] = u;
    if (u > uhi[id]) uhi[id] = u;
    if (id > maxid) maxid = id;
}
static double cslo[MAXR], cshi[MAXR], culo[MAXR], cuhi[MAXR];
static unsigned char cseen[MAXR];
void __s1p_c(int id, int taken, double s, double u) {
    if (!taken || id < 0 || id >= MAXR)
        return;
    if (!cseen[id]) { cslo[id] = cshi[id] = s; culo[id] = cuhi[id] = u; cseen[id] = 1; }
    if (s < cslo[id]) cslo[id] = s;
    if (s > cshi[id]) cshi[id] = s;
    if (u < culo[id]) culo[id] = u;
    if (u > cuhi[id]) cuhi[id] = u;
}
/* divisor histograms: sign, octave bins -64..+63 */
#define HB 128
static unsigned *hist[MAXR];
void __s1p_h(int id, double v) {
    if (id < 0 || id >= MAXR)
        return;
    if (!hist[id])
        hist[id] = calloc(2 * HB + 1, sizeof(unsigned));
    if (v == 0) { hist[id][2 * HB]++; return; }
    int e = (int)floor(log2(fabs(v))) + 64;
    if (e < 0) e = 0;
    if (e >= HB) e = HB - 1;
    hist[id][(v < 0 ? HB : 0) + e]++;
}
static int tripmax[4096];
static int ntrip = -1;
void __s1p_trip(int id, int n) {
    if (id >= 0 && id < 4096) {
        if (n > tripmax[id]) tripmax[id] = n;
        if (id > ntrip) ntrip = id;
    }
}
void __s1p_o(int id, int s, int u) {
    if (id < 0 || id >= MAXR)
        return;
    sov[id] |= (unsigned char)s;
    uov[id] |= (unsigned char)u;
}

static void reset(void) {
    if (__s1p_reset)
        __s1p_reset();
    memset(g_export, 0, sizeof g_export);
    lines_reset();
    if (s1_init)
        s1_init();
}

static float note_pitch(double note) { return (float)((note - 12.0) / 12.0); }

/* profiling: the range of every state word at the start of a sample (a state whose first access in the sample is a
   store before any load has no load record of its own: docs/vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md §5) */
static int st_track;
static double st_lo[4096], st_hi[4096], st_buf[4096];

static void state_track(void) {
    if (!st_track || !__s1p_state || !&__s1p_nstate)
        return;
    __s1p_state(st_buf);
    for (int k = 0; k < __s1p_nstate && k < 4096; k++) {
        if (st_buf[k] < st_lo[k]) st_lo[k] = st_buf[k];
        if (st_buf[k] > st_hi[k]) st_hi[k] = st_buf[k];
    }
}

/* one note: gate on with a note start at 0, off at `off`; returns the B output of the last sample */
static FILE *outm;                      /* ref: S1_MASTER's output */
static void run(double note, long n, long off, FILE *out) {
    g_pitch = note_pitch(note);
    g_note = (int)note;
    g_trig = 1.0f - g_trig;
    for (long t = 0; t < n; t++) {
        g_gate = t < off ? 1.0f : 0.0f;
        if (host_sweep) {
            g_color[0] = (float)t / (float)(n > 1 ? n - 1 : 1);
            g_color[1] = 1.0f - g_color[0];
            g_lfo_filter = 8.0f * g_color[0] - 4.0f;
            g_lfo_amp = g_color[1];
            g_host_mix = 4.0f * g_color[0] - 2.0f;
        }
        state_track();
        float a = s1_dsp_a(), b = a;
        if (s1_dsp_b)
            b = s1_dsp_b(a);
        if (out) {
            float ab[2] = {a, b};
            fwrite(ab, sizeof ab, 1, out);
        }
        if (s1_master) {
            float m = s1_master(b);
            lines_step();
            if (outm)
                fwrite(&m, sizeof m, 1, outm);
        }
    }
}

static unsigned rng = 12345;
static int rnd(int n) { rng = rng * 1664525u + 1013904223u; return (int)((rng >> 8) % (unsigned)n); }

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s tables FILE | ref OUT NOTE N [name=knob...] [off=T] | prof OUT | sweep OUT\n", argv[0]);
        return 2;
    }
    params_init();
    vparams_init();
    if (!strcmp(argv[1], "tables")) {
        FILE *f = fopen(argv[2], "w");
        for (int k = 0; k < np; k++) {
            fprintf(f, "p %s %d %d\n", pd[k]->name, pd[k]->panel_id, pd[k]->count);
            for (int v = 0; v < 256; v++) {
                for (int i = 0; i < pd[k]->count; i++)
                    fprintf(f, "%.9g ", ptab[k][v * pd[k]->count + i]);
                fprintf(f, "\n");
            }
        }
        for (int k = 0; k < nvp; k++) {        /* "v name panel_a panel_b count", rows (note, step a, step b) */
            fprintf(f, "v %s %d %d %d\n", vpd[k]->name, vpd[k]->panel_a, vpd[k]->panel_b, vpd[k]->count);
            for (int r = 0; r < 128 * VSTEPS * VSTEPS; r++) {
                for (int i = 0; i < vpd[k]->count; i++)
                    fprintf(f, "%.9g ", vptab[k][r * vpd[k]->count + i]);
                fprintf(f, "\n");
            }
        }
        fclose(f);
        return 0;
    }
    if (!strcmp(argv[1], "ref")) {
        if (argc < 5)
            return 2;
        double note = atof(argv[3]);
        long n = atol(argv[4]), off = 1L << 60;
        for (int a = 5; a < argc; a++) {
            char name[128];
            int v;
            if (sscanf(argv[a], "off=%ld", &off) == 1)
                continue;
            if (sscanf(argv[a], "%127[^=]=%d", name, &v) == 2) {
                int found = color_arg(name, v);
                for (int k = 0; k < np; k++)
                    if (!strcmp(pd[k]->name, name)) { knob[k] = v; found = 1; }
                size_t L = strlen(name);
                for (int k = 0; L > 2 && k < nvp; k++)
                    if (name[L - 2] == '.' && (name[L - 1] == 'a' || name[L - 1] == 'b') &&
                        strlen(vpd[k]->name) == L - 2 && !strncmp(vpd[k]->name, name, L - 2)) {
                        vstep[k][name[L - 1] - 'a'] = (v < 0 ? 0 : v > 255 ? 255 : v) >> 5;
                        found = 1;
                    }
                if (!found) { fprintf(stderr, "no S1_PARAM %s\n", name); return 2; }
            }
        }
        FILE *f = fopen(argv[2], "wb");
        if (s1_master) {
            char mpath[4096];
            snprintf(mpath, sizeof mpath, "%s.m", argv[2]);
            outm = fopen(mpath, "wb");
        }
        reset();
        if (__s1p_dspstart)          /* ref built by `s1cc init`: start from the DSP's state (ring words = 0) */
            __s1p_dspstart();
        run(note, n, off, f);
        fclose(f);
        if (outm)
            fclose(outm);
        return 0;
    }
    if (!strcmp(argv[1], "dump")) {          /* PROG dump OUT NOTE N [name=knob ...]: every value, every sample */
        if (argc < 5)
            return 2;
        double note = atof(argv[3]);
        long n = atol(argv[4]);
        for (int a = 5; a < argc; a++) {
            char name[128];
            int v;
            if (sscanf(argv[a], "%127[^=]=%d", name, &v) == 2 && !color_arg(name, v))
                for (int k = 0; k < np; k++)
                    if (!strcmp(pd[k]->name, name)) knob[k] = v;
        }
        reset();
        dumpf = fopen(argv[2], "w");
        g_pitch = note_pitch(note);
        g_trig = 1.0f;
        for (long t = 0; t < n; t++) {
            g_gate = 1.0f;
            float a = s1_dsp_a();
            if (s1_dsp_b)
                a = s1_dsp_b(a);
            if (s1_master) {
                s1_master(a);
                lines_step();
            }
            fprintf(dumpf, "n\n");
        }
        fclose(dumpf);
        return 0;
    }
    if (!strcmp(argv[1], "sweep")) {
        /* PROG sweep OUT: knob-derived columns (docs/vol5_toolchain_compiler_sdk/s1cc_compiler_internals.md §4.6). Every knob over 0..255, the others at 128, a few
           samples of 3 notes each; per knob and record "w KNOB ID v0 .. v255" (signed view), when the record was seen
           at every position and changes with the knob. s1cc lower keeps those of values that depend on that knob only */
        static const int notes[] = {57, 0, 127};
        swv = calloc(MAXR, sizeof(double));
        swseen = calloc(MAXR, 1);
        double *all = NULL;
        unsigned char *full = NULL;
        int amax = -1;                 /* records seen at knob 0: only those can be seen at every position */
        FILE *f = fopen(argv[2], "w");
        for (int p = 0; p < np; p++) {
            for (int v = 0; v < 256; v++) {
                for (int k = 0; k < np; k++) knob[k] = 128;
                knob[p] = v;
                memset(swseen, 0, MAXR);
                for (int i = 0; i < 3; i++) { reset(); run(note_in(notes[i]), i ? 1 : 4, 1L << 60, 0); }
                if (!all) {
                    amax = maxid;
                    all = calloc((size_t)(amax + 1) * 256, sizeof(double));
                    full = malloc((size_t)amax + 1);
                    memset(full, 1, (size_t)amax + 1);
                }
                for (int id = 0; id <= amax; id++) {
                    if (!swseen[id]) full[id] = 0;
                    else if (full[id]) all[(size_t)id * 256 + v] = swv[id];
                }
            }
            for (int id = 0; all && id <= amax; id++) {
                if (!full[id]) continue;
                int varies = 0;
                for (int v = 1; v < 256; v++) varies |= all[(size_t)id * 256 + v] != all[(size_t)id * 256];
                if (!varies) continue;
                fprintf(f, "w %s %d", pd[p]->name, id);
                for (int v = 0; v < 256; v++) fprintf(f, " %.17g", all[(size_t)id * 256 + v]);
                fprintf(f, "\n");
            }
            free(all); free(full); all = NULL; full = NULL;
        }
        fclose(f);
        return 0;
    }
    if (!strcmp(argv[1], "prof")) {
        /* scenarios: every knob over its range (the others at 128), then random knob sets; notes 0..127;
           a gate that closes half way (envelopes, releases) */
        static const int notes[] = {0, 12, 24, 36, 48, 57, 60, 69, 72, 84, 96, 108, 120, 127};
        for (int k = 0; k < 4096; k++) { st_lo[k] = 1e300; st_hi[k] = -1e300; }
        st_track = 1;
        host_sweep = 1;
        int nn = (int)(sizeof notes / sizeof notes[0]);
        long n = 2400;
        for (int p = 0; p < np || p == 0; p++) {
            for (int v = 0; v < 256; v += np ? 17 : 256) {
                for (int k = 0; k < np; k++) knob[k] = 128;
                if (np) knob[p] = v == 255 ? 255 : v;
                for (int i = 0; i < nn; i++) { reset(); run(note_in(notes[i]), n, n / 2, 0); }
            }
            if (!np) break;
        }
        for (int k = 0; k < nvp; k++)          /* every step of each S1_VOICE_PARAM knob (the others at 128) */
            for (int ab = 0; ab < 2; ab++)
                for (int st = 0; st < VSTEPS; st++) {
                    if (!(ab ? vpd[k]->panel_b : vpd[k]->panel_a))
                        continue;
                    for (int j = 0; j < np; j++) knob[j] = 128;
                    for (int j = 0; j < nvp; j++) vstep[j][0] = vstep[j][1] = 128 >> 5;
                    vstep[k][ab] = st;
                    for (int i = 0; i < nn; i++) { reset(); run(note_in(notes[i]), n, n / 2, 0); }
                }
        for (int r = 0; r < 24; r++) {
            for (int k = 0; k < np; k++) knob[k] = rnd(4) == 0 ? (rnd(2) ? 255 : 0) : rnd(256);
            for (int k = 0; k < nvp; k++) { vstep[k][0] = rnd(VSTEPS); vstep[k][1] = rnd(VSTEPS); }
            for (int i = 0; i < nn; i++) { reset(); run(note_in(notes[i]) + (note_in(notes[i]) < (s1_note_range ? s1_note_range[1] : 127) ? rnd(100) / 100.0 : 0), n, n / 2, 0); }
        }
        for (int k = 0; k < np; k++) knob[k] = 255;
        for (int k = 0; k < nvp; k++) vstep[k][0] = vstep[k][1] = VSTEPS - 1;
        for (int i = 0; i < nn; i++) { reset(); run(note_in(notes[i]), n, n / 2, 0); }
        for (int k = 0; k < nvp; k++) vstep[k][0] = vstep[k][1] = 0;
        for (int k = 0; k < np; k++) knob[k] = 0;
        for (int i = 0; i < nn; i++) { reset(); run(note_in(notes[i]), n, n / 2, 0); }
        FILE *f = fopen(argv[2], "w");
        if (__s1p_state && &__s1p_nstate) {           /* the initial state (after s1_init) */
            static double st[4096];
            reset();
            __s1p_state(st);
            for (int k = 0; k < __s1p_nstate && k < 4096; k++)
                fprintf(f, "s %d %.17g\n", k, st[k]);
            for (int k = 0; k < __s1p_nstate && k < 4096; k++)     /* the range at the start of a sample */
                if (st_lo[k] <= st_hi[k])
                    fprintf(f, "r %d %.17g %.17g\n", k, st_lo[k], st_hi[k]);
        }
        for (int id = 0; id <= ntrip; id++)
            fprintf(f, "t %d %d\n", id, tripmax[id]);
        for (int id = 0; id <= maxid; id++)
            if (seen[id])
                fprintf(f, "v %d %.17g %.17g %.17g %.17g\n", id, slo[id], shi[id], ulo[id], uhi[id]);
        for (int id = 0; id < MAXR; id++)
            if (hist[id]) {
                fprintf(f, "h %d", id);
                for (int k = 0; k <= 2 * HB; k++)
                    fprintf(f, " %u", hist[id][k]);
                fprintf(f, "\n");
            }
        for (int id = 0; id <= maxid; id++)
            if (cseen[id])
                fprintf(f, "c %d %.17g %.17g %.17g %.17g\n", id, cslo[id], cshi[id], culo[id], cuhi[id]);
        for (int id = 0; id <= maxid; id++)
            if (sov[id] || uov[id])
                fprintf(f, "o %d %d %d\n", id, sov[id], uov[id]);
        int ni = &__s1p_isl_n ? __s1p_isl_n : 0;
        for (int k = 0; k < ni; k++) {
            const isl_row *row = &__s1p_isl_tab[k];
            if (row->root < 0 || row->root >= MAXR || !seen[row->root])
                continue;
            /* the root's range in the view the island converts from: try the signed one, else unsigned */
            double lo = slo[row->root], hi = shi[row->root];
            if (ulo[row->root] >= 0 && slo[row->root] < 0 && uhi[row->root] < 4294967296.0 &&
                (uhi[row->root] - ulo[row->root]) < (shi[row->root] - slo[row->root]))
                lo = ulo[row->root], hi = uhi[row->root];
            int m = 2049;
            fprintf(f, "i %d %d\n", row->id, m);
            for (int j = 0; j < m; j++) {
                double x = lo + (hi - lo) * j / (m - 1);
                if (row->isint)
                    x = floor(x);
                fprintf(f, "%.17g %.17g\n", x, row->fn(x));
            }
        }
        fclose(f);
        return 0;
    }
    return 2;
}
