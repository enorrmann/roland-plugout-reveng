/* Decoder of DSP instructions (the reference; it replaces re/dspisa.py's template / Layout / Imm, archived
 * 2026-10-29 in archive/dspisa.py): class templates, the slot area, the immediate bit stream, units, conditions,
 * jumps, and the decoded form (Op / Cond) that dspcore.c executes. docs/DSP_ISA_SPEC.md, docs/DSP_HARDWARE.md.
 *
 * An instruction = header (class << 5 | length) + body words. The class is a template: one letter per body word
 *     A  ALU op            B  unit-B op         M  MAC first word (w1)     N  MAC second word (w2)
 *     T  transfer          R  reference (slot)  I  immediate bits          C  condition / hidden-register word
 *     J  jump (a C word 009Y)                   S  slot word after the transfers (not N)
 * Templates: the built-in table below, then re/dsp_templates.txt with the B units of re/dsp_units_ab.txt
 * (dsp_load_templates). */
#ifndef DSPDEC_H
#define DSPDEC_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { K_A = 1, K_B, K_M, K_J, K_HID, K_X28, K_XB5, K_XFLAG, K_XMEM, K_XTAP };

typedef struct Op {     /* one unit of a decoded instruction, in body position order */
    int32_t kind, unit; /* unit: 1-based unit number of an A/B/M op (for X9bY), else 0 */
    int32_t w, w2;      /* op word(s) */
    int32_t a, b, c, has;
    double imm[3];      /* A/B: Y; M: Y, C, W; HID: static value */
} Op;

typedef struct Cond { int32_t cc, neg, op_t, op_f; } Cond;    /* skip op_t (cond true) / op_f (false); -1 = none */
typedef struct Insn { int32_t op0, nop, cond0, ncond, err; } Insn;

#define DMAX 64                 /* body words of one instruction (the header holds 5 bits of length) */

typedef struct DLayout {        /* what the words alone determine */
    int cls, n;                 /* class, body length */
    char t[DMAX + 1];           /* one letter per body word */
    uint16_t body[DMAX];
    uint16_t slots[DMAX];       /* the words transfers address (slot k = slots[k]) */
    int nslots;
    uint16_t imm[DMAX + 64];    /* immediate stream, then zeros */
    int nimm;
    uint64_t refs;              /* stream words that are references: sequential reads skip them */
    uint16_t nlist[DMAX];       /* MAC second words in order */
    int nn;
    int units[DMAX];            /* body positions of the units: ops (A B M J) first, then transfers */
    int nunits;
    int mac_off;                /* class 0x080 with a P2 term: neither MAC writes (2026-10-29) */
} DLayout;

/* ------------------------------------------------------------------ templates */
static char *g_tmpl[0x800];     /* class -> template ('*' = repeat the letter before it) */
static int g_tmpl_loaded;

static void tmpl_set(int cls, const char *t)
{
    if (cls < 0 || cls >= 0x800 || g_tmpl[cls]) return;   /* the first definition wins */
    g_tmpl[cls] = strdup(t);
}

static void tmpl_builtin(void)
{
    static const struct { int cls; const char *t; } B[] = {
        {0x000, ""}, {0x048, "AI*"}, {0x0A8, "AAI*"}, {0x040, "MNI*"}, {0x060, "TR"}, {0x0F0, "TTRR"},
        {0x1F0, "TTTRRR"}, {0x3C0, "TTTTRRRR"}, {0x720, "TTTTTRRRRR"}, {0x722, "TTTTTTRRRRRR"},
        {0x0A0, "MTNRI*"}, {0x088, "MANI*"}, {0x158, "MTTNRRI*"}};
    for (size_t k = 0; k < sizeof B / sizeof *B; k++) tmpl_set(B[k].cls, B[k].t);
}

/* re/dsp_templates.txt (dsptemplates.py: "0xCLS len raw template ...") with the B positions of re/dsp_units_ab.txt
 * ("0xCLS len template p:U ..."); the trailing I letters become "I*". Returns the number of classes read, -1 if the
 * templates file is missing. */
int dsp_load_templates(const char *dir)
{
    char path[1024], line[1024];
    static char ab[0x800][DMAX + 1];             /* 'B' at the unit-B positions (1-based), 0 elsewhere */
    tmpl_builtin();
    snprintf(path, sizeof path, "%s/dsp_units_ab.txt", dir);
    FILE *f = fopen(path, "r");
    if (f) {
        while (fgets(line, sizeof line, f)) {
            if (strncmp(line, "0x", 2)) continue;
            int cls = (int)strtol(line, NULL, 16);
            if (cls < 0 || cls >= 0x800) continue;
            char *tok = strtok(line, " \t\n");
            for (int k = 0; tok; k++, tok = strtok(NULL, " \t\n")) {
                char *colon = strchr(tok, ':');
                if (k >= 3 && colon) {
                    int p = atoi(tok);
                    if (p >= 1 && p <= DMAX && colon[1] == 'B') ab[cls][p - 1] = 'B';
                }
            }
        }
        fclose(f);
    }
    snprintf(path, sizeof path, "%s/dsp_templates.txt", dir);
    f = fopen(path, "r");
    if (!f) return -1;
    int count = 0;
    while (fgets(line, sizeof line, f)) {
        char c0[32], c1[32], c2[128], c3[128];
        if (sscanf(line, "%31s %31s %127s %127s", c0, c1, c2, c3) != 4 || strncmp(c0, "0x", 2)) continue;
        int cls = (int)strtol(c0, NULL, 16);
        if (cls < 0 || cls >= 0x800) continue;
        char t[DMAX + 3];
        int n = (int)strlen(c3);
        if (n > DMAX) continue;
        memcpy(t, c3, n + 1);
        for (int p = 0; p < n; p++)
            if (ab[cls][p] == 'B' && t[p] == 'A') t[p] = 'B';
        while (n && t[n - 1] == 'I') t[--n] = 0;
        t[n] = 'I', t[n + 1] = '*', t[n + 2] = 0;
        tmpl_set(cls, t);
        count++;
    }
    fclose(f);
    g_tmpl_loaded = 1;
    return count;
}

/* the template of an instruction of n body words, or 0 (unknown class or length); out holds n letters */
static int dsp_template(int cls, int n, char *out)
{
    if (!g_tmpl_loaded) tmpl_builtin();
    if (cls == 0x060 && n == 1) {               /* `0c02 X28Y` / `0c02 3a90`: a transfer without a reference */
        strcpy(out, "T");
        return 1;
    }
    if (cls < 0 || cls >= 0x800 || !g_tmpl[cls] || n > DMAX) return 0;
    char t[2 * DMAX + 8];
    const char *src = g_tmpl[cls];
    int L = (int)strlen(src);
    if (L && src[L - 1] == '*') {               /* t[:-1] + t[-2] * max(0, n - len(t) + 1), cut to n */
        int k = 0;
        for (int i = 0; i < L - 1; i++) t[k++] = src[i];
        for (int r = n - L + 1; r > 0; r--) t[k++] = src[L - 2];
        t[k] = 0;
        if (k >= n) t[n] = 0;
    } else {
        if (L > (int)sizeof t - 1) return 0;
        strcpy(t, src);
    }
    int len = (int)strlen(t);
    char *lt = strrchr(t, 'T');
    if (len != n && lt) {                       /* the slot area after the last transfer can have any length */
        int core = (int)(lt - t) + 1;
        if (n >= core) {
            for (int i = core; i < n; i++) t[i] = 'R';
            t[n] = 0;
            len = n;
        }
    }
    if (len != n) return 0;
    memcpy(out, t, n + 1);
    return 1;
}

/* ------------------------------------------------------------------ layout */
/* returns 0, or -1 (unknown template), -2 (more MAC first words than second words) */
static int dsp_layout_of(const uint16_t *ins, int nins, DLayout *L)
{
    memset(L, 0, sizeof *L);
    if (nins < 1) return -1;
    L->cls = ins[0] >> 5, L->n = nins - 1;
    if (L->n > DMAX || !dsp_template(L->cls, L->n, L->t)) return -1;
    memcpy(L->body, ins + 1, L->n * sizeof *ins);
    char *t = L->t;
    int n = L->n, last_t = -1, nM = 0;
    for (int i = 0; i < n; i++) {
        if (t[i] == 'T') last_t = i;
        if (t[i] == 'M') nM++;
    }
    if (last_t >= 0)                            /* after the last transfer: the N words, then slot words */
        for (int i = last_t + 1, k = 0; i < n; i++, k++) t[i] = k < nM ? 'N' : 'S';
    for (int i = 0; i < n; i++)                 /* a C word 009Y is a jump */
        if (t[i] == 'C' && (L->body[i] & 0xFFF0) == 0x0090) t[i] = 'J';
    uint64_t used = 0;
    for (int i = 0; i < n; i++)
        if (t[i] == 'T' && ((L->body[i] >> 4) & 0xFF) == 0xFE) used |= 1ull << ((L->body[i] & 15) >> 1);
    for (int i = 0; i < n; i++) {
        char k = t[i];
        if (last_t >= 0 ? (k == 'N' || k == 'S') : (k == 'N' || k == 'I')) L->imm[L->nimm++] = L->body[i];
        if (last_t >= 0 ? (k == 'N' || k == 'S') : (k == 'N' || k == 'R')) L->slots[L->nslots++] = L->body[i];
        if (k == 'N') L->nlist[L->nn++] = L->body[i];
    }
    L->refs = last_t >= 0 ? used : 0;
    for (int i = 0; i < n; i++)
        if (strchr("ABMJ", t[i])) L->units[L->nunits++] = i;
    for (int i = 0; i < n; i++)
        if (t[i] == 'T') L->units[L->nunits++] = i;
    if (nM > L->nn) return -2;
    if (L->cls == 0x080)
        for (int i = 0; i < n; i++)
            if (t[i] == 'M' && (L->body[i] >> 9) & 4) L->mac_off = 1;
    return 0;
}

/* the immediate of `bits` (16/24/32) starting `byte` bytes into the stream, reference words after the first one
 * skipped (2026-10-17); the RAW word, left-aligned */
static uint32_t dsp_take(const DLayout *L, int byte, int bits)
{
    uint64_t out = 0;
    int got = 0, pos = byte * 8, start = pos / 16;
    while (got < bits) {
        int wi = pos / 16;
        if (wi != start && wi < 64 && (L->refs >> wi & 1)) {
            pos = (wi + 1) * 16;
            continue;
        }
        int k = 16 - pos % 16;
        if (k > bits - got) k = bits - got;
        uint32_t w = wi < L->nimm ? L->imm[wi] : 0;
        out = out << k | ((w >> (16 - pos % 16 - k)) & ((1u << k) - 1));
        got += k, pos += k;
    }
    return (uint32_t)(out << (32 - bits));
}

double dsp_raw_value(uint32_t w)
{
    int s = (w >> 29) & 3;
    double v = (double)(w & 0x1FFFFFFF) / (double)(1ull << (32 - 4 * s));
    return w >> 31 ? -v : v;
}

static const int IMMBITS[4] = {0, 16, 24, 32};

static double take_value(const DLayout *L, int mode, int byte) { return dsp_raw_value(dsp_take(L, byte, IMMBITS[mode])); }

/* the unit a C word skips when its condition is `cond` (0 = none) */
static int dsp_skipped_unit(const DLayout *L, int w, int cond)
{
    int ua = (w >> 4) & 15, ub = w & 15;
    int u = cond ? ua : ub;
    if ((cond ? ub : ua) & 8) u = 0;            /* the chosen field only acts if bit 3 of the other field is 0 */
    if (L->cls == 0x190 && u == 2) u = 1;       /* class 0x190 (A C C): field 2 also skips the op */
    return u >= 1 && u <= L->nunits ? u : 0;
}

/* offset of a jump word 009Y, in words from the instruction end */
static int dsp_jump_of(const DLayout *L, int w)
{
    int i = (w & 15) >> 1;
    int n = i < L->nimm ? L->imm[i] : 0;
    return n & 0x8000 ? n - 0x10000 : n;
}

static double hidden_static(const DLayout *L, int pos, int w)
{
    int m = (w >> 4) & 3, y = w & 15, i = pos + 1 + y;
    uint32_t r0 = i < L->n ? L->body[i] : 0, r1 = i + 1 < L->n ? L->body[i + 1] : 0;
    return (double)((r0 << 16 | r1) >> (32 - IMMBITS[m])) / 16384.0;
}

static const int FLAG_GROUP[16] = {[0x9] = 1, [0xA] = 2, [0xB] = 3, [0xC] = 4, [0xD] = 5};

/* the MAC coefficient's byte in the slot array (2026-10-29, re/dspnative.py and the class sweeps of
 * DSP_ISA_DISCREPANCIES.md D8): byte C, plus 8 when bit 9 of the second word is set; Y register or immediate,
 * with or without transfers */
static int mac_coef_byte(int w2) { return ((w2 >> 6) & 7) + ((w2 >> 9) & 1 ? 8 : 0); }

/* decode one instruction: ops (body order), conds, and for jump ops the offset in jump_off[op] (words from the
 * instruction end). Returns 0, or <0 where the instruction cannot run (unknown template, missing N word). */
int dsp_decode(const uint16_t *ins, int nins, Op *ops, int maxops, int *nops, Cond *conds, int maxc, int *nconds,
               int32_t *jump_off)
{
    DLayout L;
    int e = dsp_layout_of(ins, nins, &L);
    if (e) return e;
    int pos_op[DMAX], no = 0, nc = 0, nn = 0;
    for (int i = 0; i < DMAX; i++) pos_op[i] = -1;
    for (int pos = 0; pos < L.n; pos++) {
        int w = L.body[pos];
        char k = L.t[pos];
        int unit = 0;
        for (int u = 0; u < L.nunits; u++)
            if (L.units[u] == pos) { unit = u + 1; break; }
        if (!strchr("ABMJCT", k)) continue;
        if (no >= maxops) return -3;
        Op o;
        memset(&o, 0, sizeof o);
        o.unit = unit, o.w = w;
        int add = 1;
        if (k == 'A') {
            o.kind = K_A;
            if ((w >> 4) & 3) o.imm[0] = take_value(&L, (w >> 4) & 3, w & 15), o.has = 1;
        } else if (k == 'B') {
            o.kind = K_B;
            if ((w >> 4) & 3 && !(w >> 12 >= 8 && ((w >> 8) & 15) == 0xF))
                o.imm[0] = take_value(&L, (w >> 4) & 3, w & 15), o.has = 1;
        } else if (k == 'M') {
            int w2 = L.nlist[nn++];
            o.kind = K_M, o.w2 = w2;
            if (L.mac_off) o.w = w & ~0x0E00;
            int ym = (w >> 4) & 3, cm = (w2 >> 10) & 3, wm = (w2 >> 4) & 3;
            if (ym) o.imm[0] = take_value(&L, ym, w & 15);
            if (cm) o.imm[1] = take_value(&L, cm, mac_coef_byte(w2));
            if (wm) o.imm[2] = take_value(&L, wm, w2 & 15);
        } else if (k == 'J') {
            o.kind = K_J;
            if (jump_off) jump_off[no] = dsp_jump_of(&L, w);
        } else if (k == 'C') {
            add = 0;
            if (w >> 8 == 0x40) {               /* 40TY: hidden register load */
                int tg = 4 + ((w >> 6) & 3), m = (w >> 4) & 3;
                if (m == 0) o.kind = K_HID, o.a = tg, o.b = 0, o.c = w & 15, add = 1;
                else if (m != 3) o.kind = K_HID, o.a = tg, o.b = 1, o.imm[0] = hidden_static(&L, pos, w), add = 1;
            }
        } else {                                /* T */
            int reg = w >> 12, slot = (w & 15) >> 1, mode = (w >> 4) & 0xFF;
            add = 1;
            if (mode == 0x28) o.kind = K_X28, o.a = reg, o.b = w & 15;
            else if (mode == 0xDA || mode == 0xDE || mode == 0xDB || mode == 0xDF)
                o.kind = K_XB5, o.a = reg, o.b = w & 15, o.c = (mode == 0xDA || mode == 0xDE) ? 6 : 7;
            else if (mode >= 0xD0 && mode <= 0xD7) o.kind = K_XB5, o.a = reg, o.b = (mode & 7) << 4 | (w & 15), o.c = 0;
            else if (mode == 0x9B) o.kind = K_XFLAG, o.a = FLAG_GROUP[reg & 15], o.b = w & 15;
            else if (mode == 0xFE && slot < L.nslots) o.kind = K_XMEM, o.a = reg, o.b = L.slots[slot];
            else if (mode == 0xA9 && reg < 8) o.kind = K_XTAP, o.a = reg;        /* delay tap, address in rX */
            else add = 0;
        }
        if (add) {
            if (o.kind != K_A && o.kind != K_B && o.kind != K_M) o.unit = 0, o.w = 0;   /* only ops carry them */
            if (k != 'J' && jump_off) jump_off[no] = 0;
            pos_op[pos] = no;
            ops[no++] = o;
        }
    }
    for (int pos = 0; pos < L.n; pos++) {
        if (L.t[pos] != 'C') continue;
        if (nc >= maxc) return -3;
        int w = L.body[pos], sk[2];
        for (int c = 0; c < 2; c++) {
            int u = dsp_skipped_unit(&L, w, c == 0);
            sk[c] = u ? pos_op[L.units[u - 1]] : -1;
        }
        conds[nc++] = (Cond){(w >> 9) & 31, (w >> 8) & 1, sk[0], sk[1]};
    }
    *nops = no, *nconds = nc;
    return 0;
}

/* the first unit whose effect is not measured (the reference performs it as best it can, a strict simulation
 * refuses it): writes a reason to why and returns its body position + 1, or 0 when every unit is measured.
 * noop_w: a unit-B word measured to have no visible effect (the program-start forms), or -1. */
int dsp_check(const uint16_t *ins, int nins, int noop_w, char *why, int len)
{
    DLayout L;
    if (dsp_layout_of(ins, nins, &L)) {
        snprintf(why, len, "undecodable instruction");
        return 1;
    }
    for (int pos = 0; pos < L.n; pos++) {
        int w = L.body[pos], d = w >> 12, b = (w >> 8) & 15, sub = (w >> 6) & 3, mode = (w >> 4) & 3, y = w & 15;
        const char *r = NULL;
        switch (L.t[pos]) {
        case 'B':
            if (d >= 8 && b == 0xF && sub == 1) {
                if (mode == 1 && y == 12) r = NULL;                       /* f5c: modelled (write position) */
                else if (mode == 1) r = "f5Y counter (reads 0 on the master)";
                else if ((mode == 3 || (mode == 2 && y < 12)) && w != noop_w) r = "unit-B form with no visible effect";
            } else if (d >= 8 && b == 0xF && sub != 0 && w != noop_w) r = "unit-B form with no visible effect";
            else if (d >= 8 && b == 0xE && sub == 2) r = "unit-B op e, bits 7-6 = 10 (unknown)";
            else if (d >= 8 && b == 0xF && sub == 0 && mode == 1 && !(y & 8) &&
                     ((y & 7) == ((d - 7) & 7) || (y & 7) == ((d - 5) & 7)))
                r = "sqrt split with Y on r(d+1) or r(d+3): the writes mix their bits";
            break;
        case 'C':
            if (w >> 8 == 0x40 && mode == 3) r = "40TY with an imm32 (reads 0)";
            break;
        case 'T': {
            int tm = (w >> 4) & 0xFF;
            int skippable = 0;                  /* X9bY from a unit a condition word can skip: not measured */
            for (int i = 0; i < L.n; i++)
                if (L.t[i] == 'C' && y)
                    skippable |= dsp_skipped_unit(&L, L.body[i], 1) == y || dsp_skipped_unit(&L, L.body[i], 0) == y;
            if (tm == 0x9B && !FLAG_GROUP[(w >> 12) & 15]) r = "X9bY on a flag group with no measured effect";
            else if (tm == 0x9B && skippable) r = "X9bY from a unit a condition can skip (no effect measured)";
            else if (tm == 0xA9 && (w >> 12) >= 8) r = "delay-tap request with a store register (not measured)";
            else if (!(tm == 0x28 || tm == 0x9B || tm == 0xA9 || (tm >= 0xD0 && tm <= 0xD7) || tm == 0xDA || tm == 0xDE ||
                       tm == 0xDB || tm == 0xDF || (tm == 0xFE && ((w & 15) >> 1) < L.nslots)))
                r = "transfer mode not measured";
            break;
        }
        default: break;
        }
        if (r) {
            snprintf(why, len, "%s (word %04x)", r, w);
            return pos + 1;
        }
    }
    return 0;
}

/* hidden registers an instruction reads (*reads) and loads (*loads), as bit masks of h0..h7 (h4..h7 exist) */
int dsp_hidden_use(const uint16_t *ins, int nins, int *reads, int *loads)
{
    DLayout L;
    *reads = *loads = 0;
    if (dsp_layout_of(ins, nins, &L)) return -1;
    for (int pos = 0; pos < L.n; pos++) {
        int w = L.body[pos], d = w >> 12, x = (w >> 6) & 7, mode = (w >> 4) & 3, y = w & 15;
        switch (L.t[pos]) {
        case 'A':
            if (d < 8 && ((w >> 9) & 7) == 0 && (x == 4 || x == 5)) *reads |= 1 << x;     /* rD + Y + h4 / h5 */
            break;
        case 'B':
            if (d >= 8 && ((w >> 8) & 15) == 0xF && ((w >> 6) & 3) == 1 && mode == 2 && y >= 12)
                *reads |= 1 << (y - 8);                                                     /* f6Y */
            break;
        case 'C':
            if (w >> 8 == 0x40 && mode != 3) *loads |= 1 << (4 + ((w >> 6) & 3));
            break;
        case 'T': {
            int tm = (w >> 4) & 0xFF;
            if (tm == 0x28 || tm == 0xDA || tm == 0xDE) *reads |= 1 << 6;
            if (tm == 0xDB || tm == 0xDF) *reads |= 1 << 7;
            break;
        }
        default: break;
        }
    }
    return 0;
}

/* the layout for Python tools (dspref.Layout): t, slots, units, refs of one instruction */
int dsp_layout(const uint16_t *ins, int nins, DLayout *out) { return dsp_layout_of(ins, nins, out); }
int dsp_layout_size(void) { return (int)sizeof(DLayout); }

#endif
