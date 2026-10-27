// `s1cc lower`: the graph -> kernel file (docs/S1CC.md §7).
//
// Every value V of the program is held as a register r with a scale s and an offset o: V = (r - o) s. Integer
// fixed point (Q15 products, >> n, + constants, the uint32 phase of an oscillator) then costs nothing: a shift or
// a constant factor changes s, a constant term changes o. Operations materialise a representation only when they
// need one (a product of two values: o = 0; a wrap: s = M / 2, o = -1). |r| stays <= LIM (the DSP holds +-512).
//
//   add / sub   r = x + q y (one addxy / subxy / mad, q = s_y / s_x)        mul x y   materialise o = 0, one mul
//   wrap-around (the profile saw the C operation overflow): V mod 2^n = 2^(n-1) (wrap(V / 2^(n-1) - 1) + 1)
//   and with a low mask = mod; xor 2^b = + 2^b mod 2^(b+1); compares = ph_clamp(K (a - b - threshold)) (exact for
//   integers, two sharpening stages for floats); select = b + c (a - b); min / max = one op
//   islands (table reads, libm, divisions: functions of one value) = fitted P(t) or 2^Q(t), t = V / s_t + o_t
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "llvm/IR/Value.h"

#include "s1cc.h"

namespace s1 {

void Profile::read(const std::string &path) {
    std::ifstream f(path);
    if (!f)
        fail("E005", "cannot read the profile " + path);
    std::string tag;
    while (f >> tag) {
        if (tag == "v") {
            int id;
            double a, b, c, d;
            f >> id >> a >> b >> c >> d;
            sr[id].add(a);
            sr[id].add(b);
            ur[id].add(c);
            ur[id].add(d);
        } else if (tag == "c") {
            int id;
            double a, b, c, d;
            f >> id >> a >> b >> c >> d;
            csr[id].add(a);
            csr[id].add(b);
            cur[id].add(c);
            cur[id].add(d);
        } else if (tag == "h") {
            int id;
            f >> id;
            auto &h = hist[id];
            h.resize(257);
            for (auto &c : h)
                f >> c;
        } else if (tag == "s") {
            int id;
            double v;
            f >> id >> v;
            stateInit[id] = v;
        } else if (tag == "o") {
            int id, s, u;
            f >> id >> s >> u;
            sovf[id] = sovf[id] || s;
            uovf[id] = uovf[id] || u;
        } else if (tag == "i") {
            int id, n;
            f >> id >> n;
            auto &v = island[id];
            for (int k = 0; k < n; k++) {
                double x, y;
                f >> x >> y;
                v.push_back({x, y});
            }
        } else {
            std::string rest;
            std::getline(f, rest);
        }
    }
}

namespace {

const double LIM = 256;          // hard limit of |r| (the DSP holds +-512)
const double NORM_AT = 16;       // |r| above this: renormalise to <= 1 (small r keeps the offsets, in r units, small)

struct LV {
    KVal *r = nullptr;           // nullptr: constant k
    double s = 1, o = 0, k = 0;
    Range V;                     // range of V
    bool isBool = false;
    KVal *e2arg = nullptr;       // r = e2mul 2^e2arg (an exponential: its reciprocal is exp2(-e2arg))
    double e2mul = 1;
    int e2extra = 0;
    bool isConst() const { return !r; }
};

double p2ceil(double x) { return x <= 0 ? 1 : std::ldexp(1.0, (int)std::ceil(std::log2(x))); }

Range rangeOf(const LV &x) {
    Range r;
    if (x.isConst()) {
        r.add(x.k);
        return r;
    }
    return x.V;
}

struct Lowerer {
    Program &P;
    const Profile &prof;
    const std::map<std::string, std::vector<double>> &tables;
    const LowerOptions &opt;
    std::ostringstream rep;
    std::map<std::string, double> paramScale;
    std::map<int, std::pair<double, double>> stateRep;     // s, o
    std::map<int, std::pair<double, double>> exportRep;
    std::map<int, double> initR;                           // state index -> register value at the first sample
    std::vector<std::pair<int, double>> traced;            // --trace: record id, scale
    int saturated = 0;                                     // values limited to their 99.99 % range
    std::vector<Node *> unreached;                         // values no profiling scenario computed: 0 (W908)
    std::map<int, std::string> traceWhere;
    KVal *firstFlag = nullptr;                             // 1 at the first sample of the stage, else 0
    std::set<std::string> inputsB;
    std::vector<std::pair<std::string, std::string>> warnings;   // code, text

    Lowerer(Program &p, const Profile &pr, const std::map<std::string, std::vector<double>> &t, const LowerOptions &o)
        : P(p), prof(pr), tables(t), opt(o) {}

    // ---- representation helpers ----
    Kernel *K = nullptr;

    LV cst(double k) { LV x; x.k = k; x.V.add(k); return x; }

    Range rr(const LV &x) const {      // range of r
        Range r;
        r.add(x.V.lo / x.s + x.o);
        r.add(x.V.hi / x.s + x.o);
        return r;
    }

    KVal *materialize(const LV &x, double st, double ot) {      // r' with V = (r' - ot) st
        if (x.isConst())
            return K->mov(KOperand::im(x.k / st + ot));
        // r' = a r + b  (multiply, then add b)  or  (r + b / a) a  (add, then multiply): the smaller immediate
        double a = x.s / st, b = ot - x.o * a;
        if (debugLevel() >= 2 && (std::fabs(a) > 256 || std::fabs(b) > 256))
            fprintf(stderr, "materialize: s %g o %g V [%g, %g] -> st %g ot %g (a %g b %g)\n", x.s, x.o, x.V.lo,
                    x.V.hi, st, ot, a, b);
        KVal *r = x.r;
        bool hasA = std::fabs(a - 1) > 1e-15, hasB = std::fabs(b) > 1e-15 * std::max(1.0, std::fabs(ot));
        if (hasA && hasB && std::fabs(b / a) < std::fabs(b))
            return K->mul(K->add(r, KOperand::im(b / a)), KOperand::im(a));
        if (hasA)
            r = K->mul(r, KOperand::im(a));
        if (hasB)
            r = K->add(r, KOperand::im(b));
        return r;
    }

    LV normalize(LV x) {
        if (x.isConst() || !x.V.seen())
            return x;
        Range r = rr(x);
        double m = std::max(std::fabs(r.lo), std::fabs(r.hi));
        // too large for the DSP's range, or so small that the 2^-32 grid below 0.125 costs precision: to <= 1
        if (m <= NORM_AT && (m >= std::ldexp(1.0, -8) || m == 0))
            return x;
        // r' = (r - o) / f with o folded where it is cheap: keep the offset, scale both
        double f = p2ceil(m);
        x.r = K->mul(x.r, KOperand::im(1 / f));
        x.s *= f;
        x.o /= f;
        x.e2mul /= f;
        return x;
    }

    LV scale(LV x, double c) {
        if (x.isConst())
            return cst(x.k * c);
        if (c == 0)
            return cst(0);
        x.s *= c;
        Range v;
        v.add(x.V.lo * c);
        v.add(x.V.hi * c);
        x.V = v;
        return x;
    }

    LV addLV(LV a, LV b, const Range *res = nullptr, double preferS = 0) {
        Range sum;
        sum.add(rangeOf(a).lo + rangeOf(b).lo);
        sum.add(rangeOf(a).hi + rangeOf(b).hi);
        if (a.isConst() && b.isConst())
            return cst(a.k + b.k);
        if (b.isConst())
            std::swap(a, b);
        if (a.isConst()) {                                   // b + k: offset only
            b.o -= a.k / b.s;
            b.V = res ? *res : sum;
            return b;
        }
        if (preferS != 0 && (a.s == preferS || b.s == preferS)) {
            if (b.s == preferS)
                std::swap(a, b);
        } else if (std::fabs(b.s) > std::fabs(a.s))
            std::swap(a, b);
        double q = b.s / a.s;
        LV r;
        if (q == 1)
            r.r = K->add(a.r, KOperand::of(b.r));
        else if (q == -1)
            r.r = K->sub(a.r, KOperand::of(b.r));
        else
            r.r = K->mad(a.r, b.r, KOperand::im(q));
        r.s = a.s;
        r.o = a.o + q * b.o;
        r.V = res ? *res : sum;
        return normalize(r);
    }

    LV neg(LV x) { return scale(x, -1); }

    LV mulLV(LV a, LV b, const Range *res) {
        if (a.isConst())
            return scale(b, a.k);
        if (b.isConst())
            return scale(a, b.k);
        KVal *x = materialize(a, a.s, 0), *y = materialize(b, b.s, 0);
        LV r;
        r.r = K->mul(x, KOperand::of(y));
        r.s = a.s * b.s;
        r.o = 0;
        Range v;
        Range ra = rangeOf(a), rb = rangeOf(b);
        for (double p : {ra.lo * rb.lo, ra.lo * rb.hi, ra.hi * rb.lo, ra.hi * rb.hi})
            v.add(p);
        r.V = res ? *res : v;
        return normalize(r);
    }

    LV modLV(LV x, double M, bool signedWrap) {      // V mod M (M = 2^n): [0, M) or [-M/2, M/2)
        LV r;
        if (signedWrap) {
            r.r = K->wrap(materialize(x, M / 2, 0));
            r.s = M / 2;
            r.o = 0;
            r.V.add(-M / 2);
            r.V.add(M / 2);
        } else {
            r.r = K->wrap(materialize(x, M / 2, -1));
            r.s = M / 2;
            r.o = -1;
            r.V.add(0);
            r.V.add(M);
        }
        return r;
    }

    LV minmax(LV a, LV b, bool isMin) {
        if (a.isConst() && b.isConst())
            return cst(isMin ? std::min(a.k, b.k) : std::max(a.k, b.k));
        if (a.isConst())
            std::swap(a, b);
        bool m = isMin == (a.s > 0);             // in r units, a negative scale flips min / max
        KOperand y = b.isConst() ? KOperand::im(b.k / a.s + a.o) : KOperand::of(materialize(b, a.s, a.o));
        LV r = a;
        r.r = m ? K->min(a.r, y) : K->max(a.r, y);
        Range v, ra = rangeOf(a), rb = rangeOf(b);
        v.add(isMin ? std::min(ra.lo, rb.lo) : std::max(ra.lo, rb.lo));
        v.add(isMin ? std::min(ra.hi, rb.hi) : std::max(ra.hi, rb.hi));
        r.V = v;
        return r;
    }

    LV absLV(LV x) {
        if (x.isConst())
            return cst(std::fabs(x.k));
        KVal *y = materialize(x, std::fabs(x.s), 0);
        LV r;
        r.r = K->max(y, KOperand::negOf(y));
        r.s = std::fabs(x.s);
        Range v = rangeOf(x);
        r.V.add(0);
        r.V.add(std::max(std::fabs(v.lo), std::fabs(v.hi)));
        return r;
    }

    LV boolOf(KVal *pm1) {                      // a register at -1 / +1 -> 0 / 1
        LV r;
        r.r = pm1;
        r.s = 0.5;
        r.o = -1;
        r.V.add(0);
        r.V.add(1);
        r.isBool = true;
        return r;
    }

    // X > theta (strict = false: X >= theta), as a bool
    LV positive(LV X, bool isInt, bool strict) {
        Range xr = rangeOf(X);
        if (X.isConst())
            return cst(strict ? X.k > 0 : X.k >= 0);
        double theta = isInt ? (strict ? 0.5 : -0.5) : 0;
        if (xr.lo > theta)
            return cst(1);
        if (xr.hi < theta || (!strict && !isInt && xr.hi < 0) || (strict && xr.hi <= theta && !isInt))
            return cst(0);
        // ph_clamp(K (X - theta)) in stages: |K (X - theta)| stays <= 256 before each clamp, and the product of
        // the stage gains reaches what separates the two outcomes (integers: |X - theta| >= 0.5, so 2; floats:
        // 2^24 / range: a step 2^-24 of the range wide, three stages)
        double m = std::max(std::fabs(xr.lo - theta), std::fabs(xr.hi - theta));
        double need = isInt ? 2 : std::ldexp(1.0, 24) / m;     // floats: a step 2^-24 of the range (float32)
        double k1 = std::min(need, LIM / m);
        KVal *t = K->clamp(materialize(X, 1 / k1, -k1 * theta));
        for (double got = k1; got < need; ) {                     // after a clamp |t| <= 1: gains up to 256
            double k = std::min(need / got, LIM);
            t = K->clamp(K->mul(t, KOperand::im(k)));
            got *= k;
        }
        return boolOf(t);
    }

    LV compare(const std::string &op, LV a, LV b, bool isInt) {
        std::string p = op.substr(op.find('.') + 1);
        auto diff = [&](LV x, LV y) { return addLV(x, neg(y)); };
        if (p == "true")
            return cst(1);
        if (p == "false")
            return cst(0);
        if (p == "ord")
            return cst(1);
        if (p == "uno")
            return cst(0);
        bool lt = p == "slt" || p == "ult" || p == "olt";
        bool le = p == "sle" || p == "ule" || p == "ole";
        bool gt = p == "sgt" || p == "ugt" || p == "ogt";
        bool ge = p == "sge" || p == "uge" || p == "oge";
        if (lt) return positive(diff(b, a), isInt, true);
        if (le) return positive(diff(b, a), isInt, false);
        if (gt) return positive(diff(a, b), isInt, true);
        if (ge) return positive(diff(a, b), isInt, false);
        if (p == "eq" || p == "oeq" || p == "ne" || p == "one" || p == "une" || p == "ueq") {
            LV d = absLV(diff(a, b));
            LV eq = positive(addLV(cst(0.5), neg(d)), false, true);
            if (isInt)
                eq = positive(addLV(cst(0.5), neg(d)), true, false);
            bool ne = p == "ne" || p == "one" || p == "une";
            return ne ? bnot(eq) : eq;
        }
        if (p == "ult" || p == "ole") {}
        fail("E604", "compare `" + op + "` not supported");
    }

    LV bnot(LV c) { LV r = addLV(cst(1), neg(c)); r.isBool = true; return r; }

    LV select(LV c, LV a, LV b) {
        if (c.isConst())
            return c.k ? a : b;
        if (a.isConst() && b.isConst()) {                   // affine in c
            if (a.k == b.k)
                return a;
            LV r = addLV(scale(c, a.k - b.k), cst(b.k));
            r.V = Range();
            r.V.add(a.k);
            r.V.add(b.k);
            return r;
        }
        LV d = addLV(a, neg(b));
        KVal *c01 = materialize(c, 1, 0);
        LV dd = d;
        LV m;
        if (d.isConst()) {
            m.r = K->mul(c01, KOperand::im(d.k));
            m.s = 1;
        } else {
            KVal *dm = materialize(d, d.s, 0);
            m.r = K->mul(c01, KOperand::of(dm));
            m.s = d.s;
        }
        m.o = 0;
        Range v;
        v.add(0);
        v.add(rangeOf(dd).lo);
        v.add(rangeOf(dd).hi);
        m.V = v;
        Range res;
        Range ra = rangeOf(a), rb = rangeOf(b);
        res.add(std::min(ra.lo, rb.lo));
        res.add(std::max(ra.hi, rb.hi));
        return addLV(b, m, &res);
    }

    // `x < T ? x : x - K` (and the >= form): the wrap of a phase, written with a compare. Exact as a modulo:
    // (T - K) + mod(x - (T - K), K), when x stays in [T - K, T + K) (the profile says so)
    bool wrapSelect(Node *n, LV &out) {
        Node *c = n->in[0];
        if (c->kind != NK::Op || (c->op.rfind("fcmp.", 0) != 0 && c->op.rfind("icmp.", 0) != 0))
            return false;
        std::string p = c->op.substr(5);
        bool lt = p == "olt" || p == "ult" || p == "slt" || p == "ole" || p == "ule" || p == "sle";
        bool ge = p == "oge" || p == "uge" || p == "sge" || p == "ogt" || p == "ugt" || p == "sgt";
        if (!lt && !ge)
            return false;
        LV X = val.at(c->in[0]), Tv = val.at(c->in[1]), A = val.at(n->in[1]), B = val.at(n->in[2]);
        if (X.isConst() || !Tv.isConst() || A.isConst() || B.isConst())
            return false;
        if (A.r != X.r || B.r != X.r || A.s != X.s || B.s != X.s)
            return false;
        double T = Tv.k;
        if (c->op[0] == 'i' && (p == "sle" || p == "ule"))
            T += 1;
        if (c->op[0] == 'i' && (p == "sgt" || p == "ugt"))
            T += 1;
        // arms as x + a (taken when x < T) and x + b (when x >= T)
        double offA = (X.o - A.o) * X.s, offB = (X.o - B.o) * X.s;
        double a = lt ? offA : offB, b = lt ? offB : offA;
        double K = a - b;
        if (K <= 0)
            return false;
        Range xr = rangeOf(X);
        if (!(xr.lo >= T - K - 1e-12 * K && xr.hi < T + K))
            return false;
        LV m = modLV(addLV(X, cst(-(T - K))), K, false);
        out = addLV(m, cst(T - K + a));
        Range r = recRange(n);
        if (r.seen())
            out.V = r;
        return true;
    }

    LV exp2LV(LV x) {                                     // 2^V
        if (x.isConst())
            return cst(std::exp2(x.k));
        Range v = rangeOf(x);
        double c = std::round((v.lo + v.hi) / 2);
        double half = std::max(std::fabs(v.lo - c), std::fabs(v.hi - c));
        int extra = half <= 8 ? 0 : (int)std::ceil(std::log2(half / 8));
        KVal *t = materialize(x, 1, -c);
        LV r;
        r.r = K->exp2(t, extra);
        r.s = std::exp2(c);
        r.o = 0;
        r.V.add(std::exp2(v.lo));
        r.V.add(std::exp2(v.hi));
        r.e2arg = t;
        r.e2extra = extra;
        return normalize(r);
    }

    // 1 / V: an exponential -> exp2 of the negated exponent; otherwise a fitted seed and Newton steps
    LV recip(LV y, const Node *yn = nullptr) {
        if (y.isConst())
            return cst(1 / y.k);
        Range v = rangeOf(y);
        // the profile's histogram of the divisor: the side it is on, and its usual smallest magnitude (0.05 %)
        if (yn && yn->prof >= 0 && prof.hist.count(yn->prof)) {
            const auto &h = prof.hist.at(yn->prof);
            unsigned long np = 0, nn = 0;
            for (int k = 0; k < 128; k++) {
                np += h[k];
                nn += h[128 + k];
            }
            bool pos = np >= nn;
            unsigned long tot = pos ? np : nn, acc = 0;
            int k0 = 0;
            for (int k = 0; k < 128; k++) {
                acc += h[(pos ? 0 : 128) + k];
                if (acc * 2000 >= tot) {
                    k0 = k;
                    break;
                }
            }
            double small = std::ldexp(1.0, k0 - 64);       // the octave's lower edge
            double big = pos ? v.hi : -v.lo;
            if (big > 0 && (pos ? v.lo < small : v.hi > -small)) {
                double rare = 100.0 * (tot - (double)tot) + 100.0 * (pos ? nn : np) / std::max(1.0, (double)(np + nn));
                warnings.push_back({"W903", "division by a value in [" + std::to_string(v.lo) + ", " + std::to_string(v.hi) +
                                   "] " + where(yn->v) + ": the port divides by " + (pos ? "max(y, " : "min(y, ") +
                                   std::to_string(pos ? small : -small) + ") (" + std::to_string(rare) +
                                   " % of the profiled values on the other side)"});
                y = minmax(y, cst(pos ? small : -small), !pos);
                y.V = Range();
                y.V.add(pos ? small : -big);
                y.V.add(pos ? big : -small);
                v = y.V;
            }
        }
        // a divisor that reaches 0 (a transient, a branch taken with odd values) or spans more than 6 octaves:
        // limited to the side its range mostly covers, at least max / 64 (the seed and Newton steps stay exact)
        double big = std::max(std::fabs(v.lo), std::fabs(v.hi));
        bool pos = v.hi >= -v.lo;
        double lim = big / 4096;
        if ((pos && v.lo < lim) || (!pos && v.hi > -lim)) {
            warnings.push_back({"W903", "division by a value in [" + std::to_string(v.lo) + ", " + std::to_string(v.hi) +
                               "]: the port divides by " + (pos ? "max(y, " : "min(y, ") +
                               std::to_string(pos ? lim : -lim) + ")"});
            y = minmax(y, cst(pos ? lim : -lim), !pos);
            y.V = Range();
            y.V.add(pos ? lim : v.lo);
            y.V.add(pos ? v.hi : -lim);
            v = y.V;
        }
        LV r;
        r.V.add(1 / v.lo);
        r.V.add(1 / v.hi);
        if (y.e2arg && y.o == 0) {
            KVal *nt = K->mul(y.e2arg, KOperand::im(-1));
            r.r = K->exp2(nt, y.e2extra);
            r.s = 1 / (y.s * y.e2mul);
            r.e2arg = nt;
            r.e2extra = y.e2extra;
            return normalize(r);
        }
        // the SFU seed (Roland's recipseed): |y'| = m 2^e -> N = sign 2^-e, F ~ 1/m (1 %); m = y' N; two Newton
        // steps F <- F (2 - m F) (1e-4, then 1e-8); 1/y' = F N. y' = y / 2^k keeps N and 1/y' inside +-256
        double alo = std::min(std::fabs(v.lo), std::fabs(v.hi)), ahi = std::max(std::fabs(v.lo), std::fabs(v.hi));
        if (v.lo < 0 && v.hi > 0)
            alo = std::min(alo, std::ldexp(1.0, -30));
        double kexp = std::round(std::log2(std::sqrt(std::max(alo, 1e-30) * ahi)));   // centre the range in octaves
        double pk = std::ldexp(1.0, (int)kexp);
        if (ahi / pk > LIM || pk / std::max(alo, 1e-30) > LIM)
            warnings.push_back({"W904", "division by a value over [" + std::to_string(alo) + ", " + std::to_string(ahi) +
                               "]: more than 16 octaves, the ends saturate"});
        KVal *yy = materialize(y, pk, 0);                   // y' = y / pk
        auto nf = K->recipseed(yy);
        KVal *m = K->mul(yy, KOperand::of(nf.first));
        KVal *F = nf.second;
        for (int it = 0; it < 2; it++)
            F = K->mul(F, KOperand::of(K->twoMinus(K->mul(m, KOperand::of(F)))));
        r.r = K->mul(F, KOperand::of(nf.first));
        r.s = 1 / pk;
        r.o = 0;
        return r;
    }

    // a known math function of one argument, on the argument's range: exp / exp2 / pow(c, x) exactly through
    // exp2, the others fitted (polynomial or exp2 of a polynomial, with the island fitter)
    LV mathFn(const std::string &fn, LV x, Node *n) {
        if (x.isConst()) {
            double v = x.k;
            static const std::map<std::string, double (*)(double)> f1 = {
                {"sin", std::sin}, {"cos", std::cos}, {"tan", std::tan}, {"tanh", std::tanh}, {"sinh", std::sinh},
                {"cosh", std::cosh}, {"exp", std::exp}, {"exp2", std::exp2}, {"log", std::log}, {"log2", std::log2},
                {"log10", std::log10}, {"sqrt", std::sqrt}, {"atan", std::atan}, {"asin", std::asin},
                {"acos", std::acos}, {"cbrt", std::cbrt}};
            return cst(f1.at(fn)(v));
        }
        if (fn == "exp2")
            return exp2LV(x);
        if (fn == "exp")
            return exp2LV(scale(x, 1 / std::log(2.0)));
        static const std::map<std::string, double (*)(double)> f1 = {
            {"sin", std::sin}, {"cos", std::cos}, {"tan", std::tan}, {"tanh", std::tanh}, {"sinh", std::sinh},
            {"cosh", std::cosh}, {"log", std::log}, {"log2", std::log2}, {"log10", std::log10}, {"sqrt", std::sqrt},
            {"atan", std::atan}, {"asin", std::asin}, {"acos", std::acos}, {"cbrt", std::cbrt}};
        Range v = rangeOf(x);
        if (!v.seen())
            fail("E505", fn + ": the argument's range is unknown " + where(n->v) + " (use s1_range)");
        std::vector<std::pair<double, double>> xy;
        for (int i = 0; i <= 2048; i++) {
            double a = v.lo + (v.hi - v.lo) * i / 2048.0;
            xy.push_back({a, f1.at(fn)(a)});
        }
        Fit f = fitIsland(xy, opt.fitTol, opt.fitTolLog, opt.maxDeg);
        rep << "  " << fn << " " << where(n->v) << " on [" << v.lo << ", " << v.hi << "]: " << f.model << " degree "
            << f.c.size() - 1 << ", error " << f.err << " (" << f.metric << ")\n";
        if (f.err > (f.metric == "log2" ? opt.fitTolLog : opt.fitTol))
            warnings.push_back({"W905", fn + " at " + where(n->v) + ": the fit misses the tolerance (error " +
                               std::to_string(f.err) + ")"});
        return emitFit(f, xy, x);
    }

    LV unitOp(const std::string &op, LV x) {             // s1_wrap / s1_clamp / s1_tri on V
        if (x.isConst()) {
            double v = x.k;
            if (op == "s1_wrap") v = std::fmod(std::fmod(v + 1, 2) + 2, 2) - 1;
            if (op == "s1_clamp") v = std::max(-1.0, std::min(1.0, v));
            if (op == "s1_tri") { double w = std::fmod(std::fmod(-v - 1.5 + 1, 2) + 2, 2) - 1; v = 1 - 2 * std::fabs(w); }
            return cst(v);
        }
        KVal *t = materialize(x, 1, 0);
        LV r;
        r.r = op == "s1_wrap" ? K->wrap(t) : op == "s1_clamp" ? K->clamp(t) : K->tri(t);
        r.V.add(-1);
        r.V.add(1);
        return r;
    }

    // ---- islands ----
    std::map<Node *, const Island *> chosen;              // island output -> island
    std::set<Node *> covered;
    std::map<const Island *, Fit> fits;

    void chooseIslands(Stage &S) {
        for (auto &np : S.nodes) {
            Node *N = np.get();
            if (!N->live || !N->bad || covered.count(N) || unreachedValue(N))
                continue;
            std::vector<Node *> roots;
            for (auto &isl : S.islands)
                if (isl.cloneable && std::find(isl.members.begin(), isl.members.end(), N) != isl.members.end() &&
                    std::find(roots.begin(), roots.end(), isl.root) == roots.end())
                    roots.push_back(isl.root);
            const Island *bestI = nullptr;
            Node *bestRoot = nullptr;
            double bestErr = 1e300;
            for (Node *R : roots) {
                double worst = 0;
                bool ok = true;
                for (auto &isl : S.islands) {
                    if (isl.root != R || !isl.cloneable)
                        continue;
                    auto it = prof.island.find(isl.id);
                    if (it == prof.island.end()) { ok = false; break; }
                    Fit f = fitIsland(it->second, opt.fitTol, opt.fitTolLog, opt.maxDeg);
                    fits[&isl] = f;
                    rep << "  candidate island " << isl.id << " (root " << where(R->v) << "): " << f.model
                        << " degree " << f.c.size() - 1 << ", error " << f.err << " " << f.metric << " (max " << f.errMax << "), "
                        << f.cost << " ops\n";
                    worst = std::max(worst, f.err);
                }
                if (!ok)
                    continue;
                bool good = true;
                for (auto &isl : S.islands)
                    if (isl.root == R && isl.cloneable && fits.count(&isl))
                        good = good && fits[&isl].err <= (fits[&isl].metric == "log2" ? opt.fitTolLog : opt.fitTol);
                if (good) {
                    bestRoot = R;
                    bestErr = worst;
                    break;
                }
                if (worst < bestErr) {
                    bestErr = worst;
                    bestRoot = R;
                }
            }
            if (!bestRoot)
                fail("E502", "cannot tabulate " + where(N->v) + ": no single-value island could be sampled");
            {
                bool good = true;
                for (auto &isl : S.islands)
                    if (isl.root == bestRoot && isl.cloneable && fits.count(&isl))
                        good = good && fits[&isl].err <= (fits[&isl].metric == "log2" ? opt.fitTolLog : opt.fitTol);
                if (!good)
                    warnings.push_back({"W905", "island at " + where(N->v) + ": the fit misses the tolerance (error " +
                                       std::to_string(bestErr) + "; s1cc build --tol / --tollog)"});
            }
            for (auto &isl : S.islands)
                if (isl.root == bestRoot && isl.cloneable) {
                    bool contains = false;
                    for (Node *m : isl.members)
                        contains |= m == N;
                    // every island of this root is used: its members are covered, its output is fitted
                    chosen[isl.out] = &isl;
                    for (Node *m : isl.members)
                        covered.insert(m);
                    (void)contains;
                    bestI = &isl;
                }
            (void)bestI;
        }
    }

    LV emitIsland(const Island &isl, const LV &root) {
        const Fit &f = fits.at(&isl);
        rep << "  island " << isl.id << " " << where(isl.out->v) << ": root " << where(isl.root->v) << ", "
            << f.model << " degree " << f.c.size() - 1 << (f.clampLo || f.clampHi ? ", saturated" : "")
            << ", error " << f.err << " (" << f.metric << ", max " << f.errMax << "), " << f.cost << " ops\n";
        return emitFit(f, prof.island.at(isl.id), root);
    }

    LV emitFit(const Fit &f, const std::vector<std::pair<double, double>> &samples, const LV &root) {
        Range out;
        for (auto &p : samples)
            out.add(p.second);
        if (f.c.size() == 1 && f.model == "poly")
            return cst(f.c[0]);
        // t = ta V + tb
        KVal *t = materialize(root, 1 / f.ta, f.tb);
        bool odd = f.model == "poly";
        for (size_t k = 0; k < f.c.size(); k += 2)
            odd = odd && std::fabs(f.c[k]) < 1e-300;
        auto horner = [&](const std::vector<double> &c, KVal *x, double div) -> KVal * {
            int d = (int)c.size() - 1;
            while (d > 0 && c[d] == 0)
                d--;
            if (d == 0)
                return K->mov(KOperand::im(c[0] / div));
            KVal *h = K->add(K->mul(x, KOperand::im(c[d] / div)), KOperand::im(c[d - 1] / div));
            for (int k = d - 2; k >= 0; k--)
                h = K->ymac(h, x, KOperand::im(c[k] / div));
            return h;
        };
        LV r;
        if (f.model == "poly") {
            double sy = p2ceil(std::max(std::fabs(out.lo), std::fabs(out.hi)));
            if (odd) {
                std::vector<double> oc;
                for (size_t k = 1; k < f.c.size(); k += 2)
                    oc.push_back(f.c[k]);
                KVal *x2 = K->mul(t, KOperand::of(t));
                KVal *h = oc.size() == 1 ? K->mov(KOperand::im(oc[0] / sy)) : horner(oc, x2, sy);
                r.r = K->mul(h, KOperand::of(t));
            } else {
                r.r = horner(f.c, t, 1 / 1.0 * sy);
            }
            r.s = sy;
        } else {
            double qlo = 1e300, qhi = -1e300;
            for (auto &p : samples) {
                double q = 0, tt = f.ta * p.first + f.tb;
                for (int k = (int)f.c.size() - 1; k >= 0; k--)
                    q = q * tt + f.c[k];
                qlo = std::min(qlo, q);
                qhi = std::max(qhi, q);
            }
            double c = std::round((qlo + qhi) / 2);
            std::vector<double> qc = f.c;
            qc[0] -= c;
            KVal *q = horner(qc, t, 1);
            double half = std::max(std::fabs(qlo - c), std::fabs(qhi - c));
            int extra = half <= 8 ? 0 : (int)std::ceil(std::log2(half / 8));
            r.r = K->exp2(q, extra);
            r.s = f.sign * std::exp2(c);
        }
        r.o = 0;
        r.V = out;
        if (f.clampLo || f.clampHi) {
            double lo = f.lo / r.s, hi = f.hi / r.s;
            if (r.s < 0)
                std::swap(lo, hi);
            if ((f.clampLo && r.s > 0) || (f.clampHi && r.s < 0))
                r.r = K->max(r.r, KOperand::im(lo));
            if ((f.clampHi && r.s > 0) || (f.clampLo && r.s < 0))
                r.r = K->min(r.r, KOperand::im(hi));
        }
        return normalize(r);
    }

    // ---- nodes ----
    // the integer view of a value: unsigned when its uses read it so (graph.cpp) and the profile does not show a
    // narrower signed range (clang turns `(uint16_t)(sample + 32768)` into bit operations on the int32 sample)
    bool unsignedView(const Node *n) const {
        // add / sub / mul / shl: the view in which the C operation did not overflow (both or none: from the uses)
        if (n->isInt && n->prof >= 0 && n->kind == NK::Op &&
            (n->op == "add" || n->op == "sub" || n->op == "mul" || n->op == "shl")) {
            auto so = prof.sovf.find(n->prof), uo = prof.uovf.find(n->prof);
            bool s = so != prof.sovf.end() && so->second, u = uo != prof.uovf.end() && uo->second;
            if (s != u)
                return s;
        }
        if (!n->isInt || !n->isUnsigned || n->prof < 0)
            return n->isInt && n->isUnsigned;
        auto s = prof.sr.find(n->prof), u = prof.ur.find(n->prof);
        if (s == prof.sr.end() || u == prof.ur.end())
            return true;
        return s->second.hi - s->second.lo > 0.5 * (u->second.hi - u->second.lo);   // signed only if much narrower
    }
    // floats: the range that covers 99.99 % of the profiled values (octave histogram); rarer outliers (a knob jump
    // through a division, a transient) would set a scale that loses the usual values: they saturate instead
    Range robust(const Node *n, const Range &full) const {
        if (n->isInt || n->prof < 0 || !prof.hist.count(n->prof) || !full.seen())
            return full;
        const auto &h = prof.hist.at(n->prof);
        unsigned long tot = 0;
        for (unsigned c : h)
            tot += c;
        if (tot < 1000)
            return full;
        auto edge = [&](int base) {                        // magnitude below which 99.99 % of this sign falls
            unsigned long side = 0, acc = 0;
            for (int k = 0; k < 128; k++)
                side += h[base + k];
            if (!side)
                return 0.0;
            for (int k = 0; k < 128; k++) {
                acc += h[base + k];
                if (acc * 10000.0 >= side * 9999.0)
                    return std::ldexp(1.0, k - 64 + 1);    // upper edge of that octave
            }
            return std::ldexp(1.0, 64);
        };
        double hi = std::min(full.hi, std::max(0.0, edge(0))), lo = std::max(full.lo, -edge(128));
        if (full.hi <= 0)
            hi = full.hi;
        if (full.lo >= 0)
            lo = full.lo;
        Range r;
        r.add(lo);
        r.add(hi);
        return r;
    }
    // a value of the program that no profiling scenario computed: it sits in a branch none of them took (a second
    // pass of a peeled loop, a corner case), so it is only used under a predicate the profile never saw true. Its
    // range is unknown and the interval arithmetic of its operands can reach 1e15, which would set the scale of the
    // selects that merge it; it is lowered as 0 instead (W908)
    bool unreachedValue(const Node *n) const {
        return n->kind == NK::Op && n->prof >= 0 && !n->specSel && !prof.sr.empty() && !prof.sr.count(n->prof) &&
               !prof.ur.count(n->prof);
    }
    bool outliers(const Node *n) const {
        Range f = rawRange(n), r = robust(n, f);
        double mf = std::max(std::fabs(f.lo), std::fabs(f.hi)), mr = std::max(std::fabs(r.lo), std::fabs(r.hi));
        return f.seen() && mr > 0 && mf > 4 * mr;
    }
    Range recRange(const Node *n) const {
        Range f = rawRange(n);
        return outliers(n) ? robust(n, f) : f;
    }
    Range rawRange(const Node *n) const {
        if (n->prof < 0)
            return Range();
        if (n->specSel) {                     // speculated: the range when its arm is taken (or none if never)
            auto &cm = unsignedView(n) ? prof.cur : prof.csr;
            auto ct = cm.find(n->prof);
            if (ct != cm.end())
                return ct->second;
        }
        auto &m = unsignedView(n) ? prof.ur : prof.sr;
        auto it = m.find(n->prof);
        return it == m.end() ? Range() : it->second;
    }
    bool wrapped(const Node *n) const {
        if (n->prof < 0 || !n->isInt)
            return false;
        auto &m = unsignedView(n) ? prof.uovf : prof.sovf;
        auto it = m.find(n->prof);
        return it != m.end() && it->second;
    }

    std::map<Node *, LV> val;

    LV lowerNode(Stage &S, Node *n) {
        auto in = [&](int i) { return val.at(n->in[i]); };
        Range rec = recRange(n);
        const Range *res = rec.seen() ? &rec : nullptr;
        auto withRange = [&](LV x) { if (res && !x.isConst()) x.V = *res; return x; };
        switch (n->kind) {
        case NK::Const:
            return cst(n->c);
        case NK::Input: {
            LV x;
            x.r = K->load("{IN:" + n->name + "}", n->name);
            x.V = rec.seen() ? rec : Range{0, 1};
            return x;
        }
        case NK::Increment: {
            LV p;
            p.r = K->load("{IN:pitch}", "pitch");
            p.V.add(-1);
            p.V.add(11);
            LV x = exp2LV(addLV(p, cst(-4.75)));
            return withRange(scale(x, 2 * 440.0 / 96000));
        }
        case NK::Param: {
            LV x;
            x.r = K->load("{P:" + n->name + ":" + std::to_string((int)n->c) + "}", n->name);
            x.s = paramScale.at(n->name);
            x.V = rec;
            return x;
        }
        case NK::State: {
            auto sr = stateRep.at(n->state);
            LV x;
            x.r = K->load("{S" + std::to_string(n->state) + "+1}", P.states[n->state].name);
            auto ii = initR.find(n->state);
            if (ii != initR.end()) {                          // the first sample reads 0: r + first r0
                if (!firstFlag) {                             // first = 1 - flag; the flag word holds 1 after it
                    KVal *f = K->load("{I+1}", "inited");
                    firstFlag = K->sub(K->mov(KOperand::im(1.0)), KOperand::of(f));
                    K->store(K->mov(KOperand::im(1.0)), "{I}");
                }
                x.r = K->mad(x.r, firstFlag, KOperand::im(ii->second));
            }
            x.s = sr.first;
            x.o = sr.second;
            x.V = rec;
            return x;
        }
        case NK::Import: {
            int i = (int)n->c;
            auto it = exportRep.find(i);
            if (it == exportRep.end())
                fail("E116", "s1_import(" + std::to_string(i) + ") without an s1_export(" + std::to_string(i) + ")");
            LV x;
            x.r = K->load("{X:" + std::to_string(i) + "}", "import");
            x.s = it->second.first;
            x.o = it->second.second;
            x.V = rec;
            return x;
        }
        case NK::XIn: {
            LV x;
            x.r = K->load("{XIN}", "x");
            x.V = rec;
            return x;
        }
        case NK::Op:
            break;
        }
        const std::string &op = n->op;
        if (chosen.count(n))
            return emitIsland(*chosen[n], val.at(chosen[n]->root));
        if (n->bad)
            fail("E504", "not translatable and not tabulated: " + where(n->v));
        auto constIn = [&](int i) -> double {
            LV x = in(i);
            if (!x.isConst())
                fail("E601", op + ": operand " + std::to_string(i) + " must be a constant " + where(n->v));
            return x.k;
        };
        bool allConst = true;
        for (Node *i : n->in)
            allConst = allConst && val.at(i).isConst();
        if (op == "add" || op == "fadd" || op == "sub" || op == "fsub") {
            bool sub = op == "sub" || op == "fsub";
            Range pre;
            LV a = in(0), b = sub ? neg(in(1)) : in(1);
            if (allConst)
                return cst(a.k + b.k);
            bool w = wrapped(n);
            double M = std::ldexp(1.0, n->bits);
            LV r = addLV(a, b, w ? nullptr : res, w ? M / 2 : 0);      // a wrap wants s = M / 2: no rescale
            if (w)
                return modLV(r, M, !unsignedView(n));
            return r;
        }
        if (op == "mul" || op == "fmul") {
            if (allConst)
                return cst(in(0).k * in(1).k);
            if (wrapped(n))
                warnings.push_back({"W901", "the original overflows at " + where(n->v) +
                                   " (integer product); the port computes it without wrapping"});
            return mulLV(in(0), in(1), wrapped(n) ? nullptr : res);
        }
        if (op == "fmuladd") {
            LV m = mulLV(in(0), in(1), nullptr);
            return addLV(m, in(2), res);
        }
        if (op == "fneg")
            return neg(in(0));
        if (op == "shl") {
            LV r = scale(in(0), std::ldexp(1.0, (int)constIn(1)));
            if (wrapped(n))
                return modLV(r, std::ldexp(1.0, n->bits), !unsignedView(n));
            return withRange(r);
        }
        if (op == "lshr" || op == "ashr")
            return withRange(scale(in(0), std::ldexp(1.0, -(int)constIn(1))));
        if (op == "fdiv") {
            if (val.at(n->in[1]).isConst())
                return withRange(scale(in(0), 1 / constIn(1)));
            return mulLV(in(0), recip(in(1), n->in[1]), res);
        }
        if (op == "udiv" || op == "sdiv")
            return withRange(scale(in(0), 1 / constIn(1)));
        if (op.rfind("call.", 0) == 0) {
            std::string fn = op.substr(5);
            if (fn == "pow") {
                LV b = in(0), e = in(1);
                if (b.isConst() && b.k > 0)
                    return withRange(exp2LV(scale(e, std::log2(b.k))));
                if (e.isConst()) {
                    if (e.k == 2) return mulLV(b, b, res);
                    if (e.k == -1) return recip(b);
                    if (e.k == 0.5) return mathFn("sqrt", b, n);
                }
                fail("E506", "pow with two variables " + where(n->v));
            }
            return withRange(mathFn(fn, in(0), n));
        }
        if (op == "urem" || op == "srem")
            return modLV(in(0), constIn(1), op == "srem");
        if (op == "and" || op == "or" || op == "xor") {
            if (n->bits == 1) {
                LV a = in(0), b = in(1);
                if (op == "and") return withRange(mulLV(a, b, nullptr));
                if (op == "or") return addLV(addLV(a, b), neg(mulLV(a, b, nullptr)));
                LV d = addLV(a, neg(b));
                return mulLV(d, d, nullptr);
            }
            int ci = val.at(n->in[1]).isConst() ? 1 : 0;
            LV x = in(1 - ci);
            long c = (long)constIn(ci);
            unsigned long m = n->bits >= 64 ? (unsigned long)c : ((unsigned long)c & ((1UL << n->bits) - 1));
            Range xr = rangeOf(x);
            if (op == "xor" && (c == -1 || m == (n->bits >= 64 ? ~0UL : (1UL << n->bits) - 1))) {
                LV r = addLV(neg(x), cst(-1));
                return unsignedView(n) ? addLV(r, cst(std::ldexp(1.0, n->bits))) : r;
            }
            if (op == "xor") {
                double b = (double)m;
                if (xr.lo < 0 || xr.hi >= 2 * b)
                    fail("E602", "xor with a bit outside the value's range " + where(n->v));
                return modLV(addLV(x, cst(b)), 2 * b, false);
            }
            if (op == "and") {
                int k = __builtin_ctzl(m), top = 64 - __builtin_clzl(m);
                double M = std::ldexp(1.0, top), L = std::ldexp(1.0, k);
                bool below = xr.lo >= 0 && xr.hi < M;
                LV hi = below ? x : modLV(x, M, false);
                if (k == 0)
                    return withRange(hi);
                LV lo = modLV(x, L, false);
                return withRange(addLV(hi, neg(lo)));
            }
            fail("E603", "`or` with a constant is not supported outside a tabulated function " + where(n->v));
        }
        if (op == "sext" || op == "zext" || op == "fpext" || op == "fptrunc" || op == "sitofp" || op == "uitofp" ||
            op == "fptosi" || op == "fptoui") {
            LV x = in(0);
            if (x.isConst()) {
                int b = n->in[0]->bits;
                if ((op == "zext" || op == "uitofp") && b > 0 && b < 64 && x.k < 0)
                    return cst(x.k + std::ldexp(1.0, b));      // constants are read signed: the unsigned value
                if (op == "fptosi" || op == "fptoui")
                    return cst(std::trunc(x.k));
                return x;
            }
            return withRange(x);
        }
        if (op == "trunc") {
            LV x = in(0);
            if (x.isConst()) {
                double M = std::ldexp(1.0, n->bits), v = std::fmod(x.k, M);
                if (v < 0) v += M;
                return cst(v >= M / 2 ? v - M : v);
            }
            Range xr = rangeOf(x);
            double M = std::ldexp(1.0, n->bits);
            bool fits = unsignedView(n) ? (xr.lo >= 0 && xr.hi < M) : (xr.lo >= -M / 2 && xr.hi < M / 2);
            if (fits || n->bits == 1)
                return withRange(x);
            warnings.push_back({"W902", "truncation wraps at " + where(n->v)});
            return modLV(x, M, !unsignedView(n));
        }
        if (op.rfind("icmp.", 0) == 0 || op.rfind("fcmp.", 0) == 0)
            return compare(op, in(0), in(1), op[0] == 'i');
        if (op == "select") {
            LV w;
            if (wrapSelect(n, w))
                return w;
            return withRange(select(in(0), in(1), in(2)));
        }
        if (op == "band")
            return mulLV(in(0), in(1), nullptr);
        if (op == "bor")
            return addLV(in(0), in(1));
        if (op == "bnot")
            return bnot(in(0));
        if (op == "smin" || op == "umin" || op == "fmin")
            return minmax(in(0), in(1), true);
        if (op == "smax" || op == "umax" || op == "fmax")
            return minmax(in(0), in(1), false);
        if (op == "abs" || op == "fabs")
            return absLV(in(0));
        if (op == "floor") {
            LV x = in(0);
            return addLV(x, neg(modLV(x, 1, false)), res);
        }
        if (op == "s1_wrap" || op == "s1_clamp" || op == "s1_tri")
            return unitOp(op, in(0));
        if (op == "s1_exp2")
            return exp2LV(in(0));
        if (op == "s1_range") {
            LV x = in(0);
            x.V = Range();
            x.V.add(in(1).isConst() ? in(1).k : 0.0);
            x.V.add(in(2).isConst() ? in(2).k : 0.0);
            return x;
        }
        fail("E605", "operation `" + op + "` not supported " + where(n->v));
    }

    void chooseStateReps() {
        for (auto &s : P.states) {
            Range r;
            for (Stage *S : {&P.A, &P.B})
                for (auto &np : S->nodes)
                    if (np->live && np->kind == NK::State && np->state == s.index)
                        r = recRange(np.get());
            if (s.final)
                if (Range f = recRange(s.final); f.seen()) { r.add(f.lo); r.add(f.hi); }
            double M = std::ldexp(1.0, s.bits);
            if (s.isInt && s.isUnsigned && r.seen() && r.hi > M / 4) {
                stateRep[s.index] = {M / 2, -1};        // modular (a phase): r in [-1, 1)
            } else if (s.isInt && !s.isUnsigned && r.seen() && (r.hi > M / 4 || r.lo < -M / 4) && s.bits >= 16) {
                stateRep[s.index] = {M / 2, 0};
            } else {
                double m = r.seen() ? std::max(std::fabs(r.lo), std::fabs(r.hi)) : 1;
                stateRep[s.index] = {p2ceil(m), 0};
            }
            auto sr = stateRep[s.index];
            auto it = prof.stateInit.find(s.index);
            if (it != prof.stateInit.end())
                s.init = it->second;                  // after S1_INIT (or the C initializer)
            double r0 = s.init / sr.first + sr.second;
            // a phase (modular word) that starts at 0 in the C starts at half a cycle in the ring: a fixed offset of
            // a free-running oscillator, not worth a first-sample mad in every sample (docs/S1CC.md §11.1)
            bool phaseAtZero = sr.second == -1 && s.init == 0;
            if (phaseAtZero && std::fabs(r0) > 1e-12)
                rep << "  state " << s.name << ": phase starting at 0, left at half a cycle (no first-sample "
                       "correction)\n";
            else if (std::fabs(r0) > 1e-12) {
                if (opt.exactInit)
                    initR[s.index] = r0;              // first sample: r = r0 (ring words start at 0)
                else
                    warnings.push_back({"W907", "state `" + s.name + "` starts at 0 in the ring (V = " +
                                       std::to_string(-sr.second * sr.first) + "), not at its initial value " +
                                       std::to_string(s.init) + " (--exact-init)"});
            }
        }
    }

    std::string run(Stage &S, std::string &report) {
        Kernel k(opt.regs);
        K = &k;
        val.clear();
        firstFlag = nullptr;
        chooseIslands(S);
        std::set<std::string> inputsUsed;
        for (auto &np : S.nodes) {
            Node *n = np.get();
            if (!n->live || (covered.count(n) && !chosen.count(n)))
                continue;
            if (n->kind == NK::Input || n->kind == NK::Increment)
                inputsUsed.insert(n->kind == NK::Increment ? "pitch" : n->name);
            if (unreachedValue(n)) {
                val[n] = cst(0);
                unreached.push_back(n);
            } else {
                val[n] = lowerNode(S, n);
            }
            if (outliers(n) && !val[n].isConst() && n->kind == NK::Op && !n->specSel) {   // rare outliers: saturate
                Range c = recRange(n);
                LV x = minmax(minmax(val[n], cst(c.lo), false), cst(c.hi), true);
                x.V = c;
                val[n] = normalize(x);
                saturated++;
            }
            bool repOnly = false;                               // only a scale / offset of an operand: nothing
            for (Node *i : n->in)                               // to limit (and a phase wrap stays recognisable)
                repOnly = repOnly || (val.count(i) && val[i].r && val[i].r == val[n].r);
            if (n->specFrontier && !val[n].isConst() && !repOnly) {   // speculated: limited to its arm's range
                Range c = recRange(n);
                if (c.seen()) {
                    LV x = minmax(minmax(val[n], cst(c.lo), false), cst(c.hi), true);
                    x.V = c;
                    val[n] = normalize(x);
                }
            }
            if (opt.trace && n->prof >= 0 && !val[n].isConst() && n->kind != NK::Const) {
                const LV &x = val[n];
                // V in a fixed scale (a power of 2 near its range) so |r| stays small: .trace <id> <scale>
                Range vr = rangeOf(x);
                double ts = vr.seen() ? p2ceil(std::max(std::fabs(vr.lo), std::fabs(vr.hi))) : 1;
                k.store(materialize(x, ts, 0), "{D:" + std::to_string(n->prof) + "}");
                traced.push_back({n->prof, ts});
                traceWhere[n->prof] = where(n->v) + " (" + n->op + ")";
            }
            if (debugLevel() >= 2) {
                const LV &x = val[n];
                fprintf(stderr, "  n%d %s: %s s %g o %g V [%g, %g]\n", n->id, n->op.c_str(),
                        x.isConst() ? ("const " + std::to_string(x.k)).c_str() : ("%" + std::to_string(x.r->n)).c_str(),
                        x.s, x.o, x.V.lo, x.V.hi);
            }
            // outputs as soon as they exist (fewer live registers): state words, exports, the result
            for (int s : S.states)
                if (P.states[s].final == n) {
                    auto sr = stateRep.at(s);
                    k.store(materialize(val[n], sr.first, sr.second), "{S" + std::to_string(s) + "}");
                }
            for (auto &kv : S.exports)
                if (kv.second == n) {
                    LV x = val[n];
                    if (x.isConst()) {
                        k.store(k.mov(KOperand::im(x.k)), "{X:" + std::to_string(kv.first) + "}");
                        exportRep[kv.first] = {1, 0};
                    } else {
                        k.store(x.r, "{X:" + std::to_string(kv.first) + "}");
                        exportRep[kv.first] = {x.s, x.o};
                    }
                }
            if (S.ret == n)
                k.store(materialize(val[n], 1, 0), "{OUT}");
        }
        if (S.name == "A")
            for (const std::string &in : inputsB) {          // inputs DSP B needs, passed through A
                KVal *v = k.load("{IN:" + in + "}", in);
                k.store(v, "{INB:" + in + "}");
            }
        if (saturated)
            warnings.push_back({"W906", "stage " + S.name + ": " + std::to_string(saturated) + " values limited to the range "
                               "of 99.99 % of their profiled samples (rare outliers would ruin their scale)"});
        saturated = 0;
        if (!unreached.empty())
            warnings.push_back({"W908", "stage " + S.name + ": " + std::to_string(unreached.size()) + " values no "
                                "profiling scenario computed (a branch none of them took) are 0 on the DSP; the "
                                "first at " + where(unreached.front()->v)});
        unreached.clear();
        std::vector<std::string> lines = k.lines();
        report += "stage " + S.name + ": " + std::to_string(k.units()) + " units" +
                  (k.spills() ? ", " + std::to_string(k.spills()) + " scratch words (spills)" : "") + "\n";
        std::ostringstream o;
        o << ".stage " << S.name << " " << k.units() << "\n";
        if (k.spills())
            o << ".scratch " << k.spills() << "\n";
        if (firstFlag) {
            o << ".initflag\n";
            for (int st : S.states)                     // the words the flag corrects: the reference keeps their C start
                if (initR.count(st))
                    o << ".init " << st << "\n";
        }
        for (auto &t : traced)
            o << ".trace " << t.first << " " << fmtImm(t.second).substr(1) << " " << traceWhere[t.first] << "\n";
        traced.clear();
        for (auto &l : lines)
            o << l << "\n";
        K = nullptr;
        return o.str();
    }
};

}  // namespace

std::string lower(Program &P, const Profile &prof, const std::map<std::string, std::vector<double>> &tables,
                  const LowerOptions &opt, std::string &report) {
    Lowerer L(P, prof, tables, opt);
    for (auto &kv : P.paramCount) {
        auto it = tables.find(kv.first);
        if (it == tables.end())
            fail("E006", "no table for S1_PARAM `" + kv.first + "` (tables file)");
        double m = 0;
        for (double v : it->second)
            m = std::max(m, std::fabs(v));
        L.paramScale[kv.first] = p2ceil(m);
    }
    L.chooseStateReps();
    if (P.B.fn)
        for (auto &np : P.B.nodes)
            if (np->live && (np->kind == NK::Input || np->kind == NK::Increment))
                L.inputsB.insert(np->kind == NK::Increment ? "pitch" : np->name);
    std::string a = L.run(P.A, report);
    std::string b = P.B.fn ? L.run(P.B, report) : "";
    std::ostringstream o;
    o << "; s1cc kernel (docs/S1CC.md §8): V = (r - o) s for every scaled word\n";
    for (auto &s : P.states) {
        auto sr = L.stateRep.at(s.index);
        o << ".state " << s.index << " " << s.stage << " " << fmtImm(sr.first).substr(1) << " "
          << fmtImm(sr.second).substr(1) << " " << s.name << "\n";
    }
    for (auto &kv : P.paramCount)
        o << ".param " << kv.first << " " << kv.second << " " << fmtImm(L.paramScale[kv.first]).substr(1) << "\n";
    for (auto &kv : L.exportRep)
        o << ".export " << kv.first << " " << fmtImm(kv.second.first).substr(1) << " "
          << fmtImm(kv.second.second).substr(1) << "\n";
    for (auto &in : L.inputsB)
        o << ".inputb " << in << "\n";
    o << a << b << ".end\n";
    report += L.rep.str();
    for (auto &w : L.warnings) {
        warn(w.first, w.second);
        report += "warning " + w.first + ": " + w.second + "\n";
    }
    return o.str();
}

}  // namespace s1
