/* s1cc runtime (docs/S1CC.md §5): the s1_* intrinsics on the PC, for the reference build and the profiling build
 * of a voice program (same LLVM IR; s1cc prof adds the __s1p_* calls to the profiling one).
 *
 *   PROG tables FILE                     every S1_PARAM table at knob 0..255 ("p name panel count" + 256 rows)
 *   PROG ref OUT NOTE N [name=knob ...] [off=T]
 *                                        N samples of one voice: float32 pairs (S1_DSP_A, S1_DSP_B) in OUT;
 *                                        gate on from 0 (one note start), off at T (default never)
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
extern const s1_param_desc __stop_s1_params[] __attribute__((weak));
float s1_dsp_a(void);
float s1_dsp_b(float x) __attribute__((weak));
void __s1p_reset(void) __attribute__((weak));
void __s1p_dspstart(void) __attribute__((weak));
void s1_init(void) __attribute__((weak));
void __s1p_state(double *out) __attribute__((weak));
extern const int __s1p_nstate __attribute__((weak));
typedef struct { int id, root; double (*fn)(double); } isl_row;
extern const isl_row __s1p_isl_tab[] __attribute__((weak));
extern const int __s1p_isl_n __attribute__((weak));

#define MAXP 64
static int np;
static const s1_param_desc *pd[MAXP];
static float *ptab[MAXP];             /* 256 x count */
static int knob[MAXP];
static float g_pitch, g_gate, g_trig, g_export[16];

static void params_init(void) {
    if (np)
        return;
    for (const s1_param_desc *d = __start_s1_params; d && d < __stop_s1_params; d++) {
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
float s1_gate(void) { return g_gate; }
float s1_trig(void) { return g_trig; }
float s1_env(int i) { (void)i; return g_gate; }     /* the host's envelopes: the gate stands in (check too) */
float s1_param_get(const s1_param_desc *d, int i) {
    params_init();
    for (int k = 0; k < np; k++)
        if (pd[k] == d || !strcmp(pd[k]->name, d->name))
            return ptab[k][knob[k] * d->count + i];
    return 0;
}
void s1_export(int i, float v) { g_export[i & 15] = v; }
float s1_import(int i) { return g_export[i & 15]; }
float s1_wrap(float x) { return wrapf(x); }
float s1_clamp(float x) { return x < -1 ? -1 : x > 1 ? 1 : x; }
float s1_tri(float x) { return 1.0f - 2.0f * fabsf(wrapf(-x - 1.5f)); }
float s1_exp2(float x) { return exp2f(x); }
float s1_range(float x, float lo, float hi) { (void)lo; (void)hi; return x; }

/* ---- profiling records ---- */
#define MAXR 65536
static double slo[MAXR], shi[MAXR], ulo[MAXR], uhi[MAXR];
static unsigned char seen[MAXR], sov[MAXR], uov[MAXR];
static int maxid = -1;
void __s1p_h(int id, double v);
static FILE *dumpf;                    /* dump mode: "id signed unsigned" per value, "n" per sample */
void __s1p_v(int id, double s, double u) {
    if (id < 0 || id >= MAXR)
        return;
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
    if (s1_init)
        s1_init();
}

static float note_pitch(double note) { return (float)((note - 12.0) / 12.0); }

/* profiling: the range of every state word at the start of a sample (a state whose first access in the sample is a
   store before any load has no load record of its own: docs/S1CC.md §5) */
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
static void run(double note, long n, long off, FILE *out) {
    g_pitch = note_pitch(note);
    g_trig = 1.0f - g_trig;
    for (long t = 0; t < n; t++) {
        g_gate = t < off ? 1.0f : 0.0f;
        state_track();
        float a = s1_dsp_a(), b = a;
        if (s1_dsp_b)
            b = s1_dsp_b(a);
        if (out) {
            float ab[2] = {a, b};
            fwrite(ab, sizeof ab, 1, out);
        }
    }
}

static unsigned rng = 12345;
static int rnd(int n) { rng = rng * 1664525u + 1013904223u; return (int)((rng >> 8) % (unsigned)n); }

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s tables FILE | ref OUT NOTE N [name=knob...] [off=T] | prof OUT\n", argv[0]);
        return 2;
    }
    params_init();
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
                int found = 0;
                for (int k = 0; k < np; k++)
                    if (!strcmp(pd[k]->name, name)) { knob[k] = v; found = 1; }
                if (!found) { fprintf(stderr, "no S1_PARAM %s\n", name); return 2; }
            }
        }
        FILE *f = fopen(argv[2], "wb");
        reset();
        if (__s1p_dspstart)          /* ref built by `s1cc init`: start from the DSP's state (ring words = 0) */
            __s1p_dspstart();
        run(note, n, off, f);
        fclose(f);
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
            if (sscanf(argv[a], "%127[^=]=%d", name, &v) == 2)
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
                s1_dsp_b(a);
            fprintf(dumpf, "n\n");
        }
        fclose(dumpf);
        return 0;
    }
    if (!strcmp(argv[1], "prof")) {
        /* scenarios: every knob over its range (the others at 128), then random knob sets; notes 0..127;
           a gate that closes half way (envelopes, releases) */
        static const int notes[] = {0, 12, 24, 36, 48, 57, 60, 69, 72, 84, 96, 108, 120, 127};
        for (int k = 0; k < 4096; k++) { st_lo[k] = 1e300; st_hi[k] = -1e300; }
        st_track = 1;
        int nn = (int)(sizeof notes / sizeof notes[0]);
        long n = 2400;
        for (int p = 0; p < np || p == 0; p++) {
            for (int v = 0; v < 256; v += np ? 17 : 256) {
                for (int k = 0; k < np; k++) knob[k] = 128;
                if (np) knob[p] = v == 255 ? 255 : v;
                for (int i = 0; i < nn; i++) { reset(); run(notes[i], n, n / 2, 0); }
            }
            if (!np) break;
        }
        for (int r = 0; r < 24; r++) {
            for (int k = 0; k < np; k++) knob[k] = rnd(4) == 0 ? (rnd(2) ? 255 : 0) : rnd(256);
            for (int i = 0; i < nn; i++) { reset(); run(notes[i] + rnd(100) / 100.0, n, n / 2, 0); }
        }
        for (int k = 0; k < np; k++) knob[k] = 255;
        for (int i = 0; i < nn; i++) { reset(); run(notes[i], n, n / 2, 0); }
        for (int k = 0; k < np; k++) knob[k] = 0;
        for (int i = 0; i < nn; i++) { reset(); run(notes[i], n, n / 2, 0); }
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
                if (floor(lo) == lo && floor(hi) == hi)
                    x = floor(x);
                fprintf(f, "%.17g %.17g\n", x, row->fn(x));
            }
        }
        fclose(f);
        return 0;
    }
    return 2;
}
