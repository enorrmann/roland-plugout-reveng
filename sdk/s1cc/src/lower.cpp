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
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/KnownBits.h"

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
        } else if (tag == "r") {
            int id;
            double lo, hi;
            f >> id >> lo >> hi;
            stateRange[id].add(lo);
            stateRange[id].add(hi);
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
    KVal *firstFlag = nullptr;                             // 1 at the first sample of the DSP's program, else 0
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
        // an offset below 2^-26 of the register's scale is representation noise (a phase offset of 2^-29): no add
        bool hasA = std::fabs(a - 1) > 1e-15, hasB = std::fabs(b) > std::ldexp(1.0, -26) * std::max(1.0, std::fabs(ot));
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
        if (a.r == b.r) {        // one register: (r - oa) sa + (r - ob) sb = (r - o) (sa + sb), no op (x + 1 - x)
            double s2 = a.s + b.s, k = a.o * a.s + b.o * b.s;
            if (s2 == 0)
                return cst(-k);
            LV r = a;
            r.s = s2;
            r.o = k / s2;
            r.V = res ? *res : sum;
            return normalize(r);
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
        return normalize(r);
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
        // integers: exact (2) up to a range of 2^19; wider (a 32-bit phase against a pulse width), a step 2^-16 of
        // the range: two clamps (exact there would take two more, for a band 2^-32 of a cycle wide)
        double need = isInt ? std::min(2.0, std::max(std::ldexp(1.0, 16) / m, 0.0)) : std::ldexp(1.0, 24) / m;
        if (isInt && need < 2 && m < std::ldexp(1.0, 19))
            need = 2;
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
            // integers: -|a - b| >= 0 (an integer against the threshold -0.5: always 0.5 away from it); floats:
            // 0.5 - |a - b| > 0. (2026-10-27: integers used 0.5 - |a - b| too, a half-integer: |a - b| = 1 landed
            // on the threshold and gave 0.5, true and false at once)
            LV eq = isInt ? positive(neg(d), true, false) : positive(addLV(cst(0.5), neg(d)), false, true);
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
        Range res;
        Range ra = rangeOf(a), rb = rangeOf(b);
        res.add(std::min(ra.lo, rb.lo));
        res.add(std::max(ra.hi, rb.hi));
        // V = b + c (a - b) with c = (r_c - o_c) s_c, d = a - b:  (b - o_c s_c d) + r_c (s_c d)
        // the condition's own register in one mad (acc + x y): no 0 / 1 made of it (2026-10-27: that took 0-2 ops)
        LV d = addLV(a, neg(b));
        if (d.isConst())                                   // b + c d: c scaled (free), one add
            return addLV(b, scale(c, d.k), &res);
        // both forms are emitted; the one with fewer ops is kept, the other's ops are unused and dropped (lines())
        size_t n0 = K->opCount();
        LV e = scale(d, c.s);                              // s_c d
        // b - o_c s_c d = (1 + k) b - k a, k = o_c s_c: from a and b (a negated condition, k = -1: a itself)
        double kk = c.o * c.s;
        LV f = kk == 0 ? b : kk == -1 ? a : addLV(scale(b, 1 + kk), scale(a, -kk));
        double S = std::fabs(e.s);
        KVal *er = materialize(e, S, 0), *fr = materialize(f, S, 0);
        LV r;
        r.r = K->mad(fr, c.r, KOperand::of(er));
        r.s = S;
        r.o = 0;
        r.V = res;
        r = normalize(r);
        size_t n1 = K->opCount();
        KVal *c01 = materialize(c, 1, 0);                  // the earlier form: c as 0 / 1, c d, + b
        LV m;
        m.r = K->mul(c01, KOperand::of(materialize(d, d.s, 0)));
        m.s = d.s;
        m.o = 0;
        m.V.add(0);
        m.V.add(rangeOf(d).lo);
        m.V.add(rangeOf(d).hi);
        LV q = addLV(b, m, &res);
        return K->opCount() - n1 < n1 - n0 ? q : r;
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
    // 1/y once per divisor: two divisions by the same value (Braids' BLEP times, / (phase_increment >> 16) on
    // both edges) share it
    std::map<std::pair<const Node *, KVal *>, LV> recipCache;
    bool warnedSeed = false;
    LV recip(LV y, const Node *yn = nullptr) {
        if (!yn || y.isConst())
            return recip1(y, yn);
        auto key = std::make_pair(yn, y.r);
        auto it = recipCache.find(key);
        if (it != recipCache.end())
            return it->second;
        return recipCache[key] = recip1(y, yn);
    }
    LV recip1(LV y, const Node *yn) {
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
        if (!opt.recipseed)
            return recipNorm(y, alo, ahi, v.hi >= -v.lo);
        if (!warnedSeed) {
            warnings.push_back({"W910", "a division by a variable uses recipseed (the SFU seed), proven on the SYSTEM-8 "
                                "but not on the SYSTEM-1; the first at " + (yn ? where(yn->v) : std::string("?")) +
                                ". On the SYSTEM-1 keep divisions out of the voice (S1_PARAM tables, s1_exp2)"});
            warnedSeed = true;
        }
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

    // 1/y with the ops proven on the SYSTEM-1 (recipseed, the SFU seed, is decoded on the SYSTEM-8 only): y on one
    // side of 0, |y| in [alo, ahi]. m = |y| / ahi in [alo / ahi, 1] is brought into [1/2, 1] octave groups at a
    // time (8, 4, 2, 1): t = clamp(G (2^-j - m)) is +1 below the threshold, -1 above, and m and the factor E get the
    // same f = 1 + (t + 1) / 2 (2^j - 1). Near a threshold t is in between, but m E^-1 stays |y| / ahi, so the result
    // is exact all the same; m only ends a little outside [1/2, 1]. Then a cubic seed for 1/m and Newton steps
    // F (2 - m F), and 1/y = sign F E / ahi. About 20 ops for 8 octaves
    LV recipNorm(LV y, double alo, double ahi, bool pos) {
        alo = std::max(alo, ahi / 4096);
        LV m = scale(y, (pos ? 1 : -1) / ahi);         // in [alo / ahi, 1]
        m.V = Range();
        m.V.add(alo / ahi);
        m.V.add(1);
        LV E = cst(1);
        int oct = (int)std::ceil(std::log2(ahi / alo) - 1e-9);
        double lo = alo / ahi;                         // the smallest m so far
        for (int j : {8, 4, 2, 1}) {
            if (oct <= j && !(j == 1 && oct >= 1))
                continue;
            double th = std::ldexp(1.0, -j), mhi = rangeOf(m).hi;
            // t: G1 keeps |G1 (th - m)| <= LIM; a second stage when the band 1 / (G1 th) is wider than 1/32 of th
            double G1 = LIM / std::max(mhi, 1.0);
            KVal *t = K->clamp(materialize(addLV(neg(m), cst(th)), 1 / G1, 0));
            double w = 1 / (G1 * th);
            if (w > 1.0 / 32) {
                double G2 = std::min(LIM, w * 32);
                t = K->clamp(K->mul(t, KOperand::im(G2)));
            }
            LV T;
            T.r = t;
            T.s = 1;
            T.o = 0;
            T.V.add(-1);
            T.V.add(1);
            double A = (std::ldexp(1.0, j) + 1) / 2, B = (std::ldexp(1.0, j) - 1) / 2;
            Range mr = rangeOf(m);
            LV mt = mulLV(m, T, nullptr);
            m = addLV(scale(m, A), scale(mt, B));
            m.V = Range();
            m.V.add(std::max(lo * std::ldexp(1.0, j), th) * 0.9);
            m.V.add(std::max(mr.hi, th * std::ldexp(1.0, j) * (1 + 1.0 / 32)));
            if (E.isConst())
                E = addLV(scale(T, B), cst(A));
            else {
                Range er = rangeOf(E);
                LV et = mulLV(E, T, nullptr);
                E = addLV(scale(E, A), scale(et, B));
                E.V = Range();
                E.V.add(er.lo);
                E.V.add(er.hi * std::ldexp(1.0, j));
            }
            E.V = Range();
            E.V.add(1);
            E.V.add(ahi / alo * 1.1);
            lo = std::max(lo * std::ldexp(1.0, j), th * 0.9);
            oct = j;
        }
        // m in [lo, hi]: a cubic for 1/m (least squares, relative), two Newton steps
        double mlo = std::min(rangeOf(m).lo, 0.45), mhi = std::max(rangeOf(m).hi, 1.1);
        std::vector<double> c(4, 0);
        {
            double A[4][4] = {}, bb[4] = {};
            for (int i = 0; i <= 400; i++) {
                double x = mlo + (mhi - mlo) * i / 400.0, wgt = x * x, p[4] = {1, x, x * x, x * x * x};
                for (int a = 0; a < 4; a++) {
                    bb[a] += wgt * p[a] / x;
                    for (int b2 = 0; b2 < 4; b2++)
                        A[a][b2] += wgt * p[a] * p[b2];
                }
            }
            for (int col = 0; col < 4; col++)          // Gauss-Jordan
                for (int row = 0; row < 4; row++)
                    if (row != col) {
                        double f = A[row][col] / A[col][col];
                        for (int k = 0; k < 4; k++)
                            A[row][k] -= f * A[col][k];
                        bb[row] -= f * bb[col];
                    }
            for (int a = 0; a < 4; a++)
                c[a] = bb[a] / A[a][a];
        }
        KVal *mr = materialize(m, 1, 0);
        KVal *F = K->add(K->mul(mr, KOperand::im(c[3])), KOperand::im(c[2]));
        F = K->ymac(F, mr, KOperand::im(c[1]));
        F = K->ymac(F, mr, KOperand::im(c[0]));
        for (int it = 0; it < 2; it++)
            F = K->mul(F, KOperand::of(K->twoMinus(K->mul(mr, KOperand::of(F)))));
        LV Fv;
        Fv.r = F;
        Fv.s = 1;
        Fv.o = 0;
        Fv.V.add(1 / mhi);
        Fv.V.add(1 / mlo);
        LV r = mulLV(Fv, E, nullptr);
        r = scale(r, (pos ? 1 : -1) / ahi);
        r.V = Range();
        r.V.add((pos ? 1 : -1) / alo);
        r.V.add((pos ? 1 : -1) / ahi);
        return normalize(r);
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

    void chooseIslands(Dsp &S) {
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
                    if (!prof.island.count(isl.id)) { ok = false; break; }
                    Fit f = fitIsland(islandSamples(isl), opt.fitTol, opt.fitTolLog, opt.maxDeg, opt.maxKnots);
                    fits[&isl] = f;
                    rep << "  candidate island " << isl.id << " (root " << where(R->v) << "): " << f.model
                        << (f.model == "hinge" ? " knots " : " degree ") << (f.model == "hinge" ? f.knots.size() : f.c.size() - 1) << ", error " << f.err << " " << f.metric << " (max " << f.errMax << "), "
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
            bool bestGood = false;
            if (bestRoot) {
                bestGood = true;
                for (auto &isl : S.islands)
                    if (isl.root == bestRoot && isl.cloneable && fits.count(&isl))
                        bestGood = bestGood && fits[&isl].err <= (fits[&isl].metric == "log2" ? opt.fitTolLog : opt.fitTol);
            }
            if (N->soft && (!bestRoot || !bestGood)) {     // a recip no fit follows: computed (recipNorm)
                N->bad = false;
                continue;
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

    // an island's samples, the output in the narrower integer view: the profiling clone converts it as the graph
    // reads it (unsigned when a later lshr reads it), and a small negative value is then near 2^bits, a jump no fit
    // follows; the lowering reads it in whatever view its users want (inV)
    std::map<int, std::vector<std::pair<double, double>>> islandXY;
    const std::vector<std::pair<double, double>> &islandSamples(const Island &isl) {
        auto it = islandXY.find(isl.id);
        if (it != islandXY.end())
            return it->second;
        std::vector<std::pair<double, double>> xy = prof.island.at(isl.id);
        for (auto &p : xy) {                              // booleans: 0 / 1 here (the clone converts i1 signed: -1)
            if (isl.out->isInt && isl.out->bits == 1)
                p.second = p.second != 0;
            if (isl.root->isInt && isl.root->bits == 1)
                p.first = p.first != 0;
        }
        if (isl.root->isInt && isl.root->demand > 1) {    // the root is kept modulo 2^demand (lowerNode): so are x
            double M = std::ldexp(1.0, isl.root->demand);
            for (auto &p : xy) {
                double m = std::fmod(p.first, M);
                if (m < -M / 2) m += M;
                if (m >= M / 2) m -= M;
                p.first = m;
            }
            std::sort(xy.begin(), xy.end());
        }
        if (isl.out->isInt && isl.out->bits > 1 && isl.out->bits <= 32) {
            double M = std::ldexp(1.0, isl.out->bits);
            double ulo = 1e300, uhi = -1e300, slo = 1e300, shi = -1e300;
            for (auto &p : xy) {
                double u = p.second < 0 ? p.second + M : p.second, sv = p.second >= M / 2 ? p.second - M : p.second;
                ulo = std::min(ulo, u), uhi = std::max(uhi, u), slo = std::min(slo, sv), shi = std::max(shi, sv);
            }
            bool toSigned = shi - slo < uhi - ulo;
            for (auto &p : xy)
                p.second = toSigned ? (p.second >= M / 2 ? p.second - M : p.second) : (p.second < 0 ? p.second + M : p.second);
        }
        return islandXY[isl.id] = xy;
    }

    LV emitIsland(const Island &isl, const LV &root) {
        const Fit &f = fits.at(&isl);
        rep << "  island " << isl.id << " " << where(isl.out->v) << ": root " << where(isl.root->v) << ", "
            << f.model << (f.model == "hinge" ? " knots " : " degree ") << (f.model == "hinge" ? f.knots.size() : f.c.size() - 1) << (f.clampLo || f.clampHi ? ", saturated" : "")
            << ", error " << f.err << " (" << f.metric << ", max " << f.errMax << "), " << f.cost << " ops\n";
        // the samples are in the view the profiling run chose for the root (signed when its range is not narrower);
        // the root's value here may be in the other one (a uint32 phase): convert it, or the fit leaves its domain
        const auto &xy = islandSamples(isl);
        LV r = root;
        if (isl.root->isInt && isl.root->bits > 1 && !root.isConst() && !xy.empty()) {
            double M = std::ldexp(1.0, isl.root->demand > 1 ? isl.root->demand : isl.root->bits), lo = 1e300, hi = -1e300;
            for (auto &p : xy)
                lo = std::min(lo, p.first), hi = std::max(hi, p.first);
            Range v = rangeOf(root);
            if (lo < 0 && v.hi >= M / 2)
                r = modLV(root, M, true);
            else if (hi >= M / 2 && v.lo < 0)
                r = modLV(root, M, false);
        }
        return emitFit(f, xy, r);
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
        if (f.model == "hinge") {                          // c0 + c1 t + sum w_j max(t, k_j) - w_j k_j: max, mad
            double sy = p2ceil(std::max(std::fabs(out.lo), std::fabs(out.hi)));
            double c0 = f.c[0];
            for (size_t j = 0; j < f.knots.size(); j++)
                c0 -= f.w[j] * f.knots[j];
            KVal *acc = K->add(K->mul(t, KOperand::im(f.c[1] / sy)), KOperand::im(c0 / sy));
            for (size_t j = 0; j < f.knots.size(); j++)
                acc = K->mad(acc, K->max(t, KOperand::im(f.knots[j])), KOperand::im(f.w[j] / sy));
            r.r = acc;
            r.s = sy;
        } else if (f.model == "poly") {
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
    // always exactly 0 or 1 at run time: s1_gate / s1_trig, the constants 0 and 1, booleans, selects of such
    // values, their products / min / max, and a state whose initial and stored values are such (a flag)
    std::map<const Node *, bool> binaryMemo;
    bool binary(const Node *n) {
        auto it = binaryMemo.find(n);
        if (it != binaryMemo.end())
            return it->second;
        binaryMemo[n] = true;                                  // a state's cycle: assumed, checked by the rest
        bool b = false;
        if (n->kind == NK::Const)
            b = n->c == 0 || n->c == 1;
        else if (n->kind == NK::Input)
            b = n->name == "gate" || n->name == "trig";
        else if (n->kind == NK::State) {
            const StateVar &sv = P.states[n->state];
            b = sv.final && (sv.init == 0 || sv.init == 1) && binary(sv.final);
        } else if (n->kind == NK::Op) {
            const std::string &o = n->op;
            if (n->isInt && n->bits == 1)
                b = true;                                      // booleans: 0 / 1 here
            else if (o == "select")
                b = binary(n->in[1]) && binary(n->in[2]);
            else if (o == "fmul" || o == "mul" || o == "fmax" || o == "fmin" || o == "smax" || o == "smin" ||
                     o == "umax" || o == "umin")
                b = binary(n->in[0]) && binary(n->in[1]);
            else if (o == "fsub" || o == "sub")                // 1 - b
                b = n->in[0]->kind == NK::Const && n->in[0]->c == 1 && binary(n->in[1]);
            else if (o == "uitofp" || o == "zext")
                b = n->in[0]->isInt && n->in[0]->bits == 1;
        }
        binaryMemo[n] = b;
        return b;
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

    std::vector<std::pair<Node *, LV>> viewGuard;      // operands re-read in another view by the node being lowered

    // a computed integer range that leaves the range of the node's view (bits <= 32): the value needs a wrap
    bool outOfView(const Range &v, const Node *n) const {
        if (!n->isInt || n->bits <= 1 || n->bits > 32 || !v.seen())
            return false;
        double M = std::ldexp(1.0, n->bits);
        return unsignedView(n) ? (v.lo < -0.5 || v.hi >= M) : (v.lo < -M / 2 - 0.5 || v.hi >= M / 2);
    }

    // operand o read in the view wantUnsigned: does its value (computed range) leave that view's range?
    bool needsView(const Node *o, bool wantUnsigned, int bits) const {
        auto it = val.find(const_cast<Node *>(o));
        if (!o->isInt || o->bits <= 1 || it == val.end() || it->second.isConst())
            return false;
        double M = std::ldexp(1.0, std::min(bits, o->bits));
        Range r = rangeOf(it->second);
        return wantUnsigned ? r.lo < -0.5 : r.hi >= M / 2;
    }
    // x (the value of o) in the view wantUnsigned, modulo 2^bits. A wrap works at scale M/2: for 32 bits the DSP's
    // 29-bit mantissa keeps steps of 8 there (a negative int32 read unsigned is near 2^32: those steps are inherent).
    // (Tried: x -+ M by a compare, keeping x's scale: -M is no immediate at a narrow scale, E701.)
    LV toView(const LV &x, const Node *o, bool wantUnsigned, int bits) {
        return modLV(x, std::ldexp(1.0, std::min(bits, o->bits)), !wantUnsigned);
    }

    LV floorLV(LV x) { return addLV(x, neg(modLV(x, 1, false))); }
    // toward zero, for a value whose profiled range is r: floor when >= 0, ceil when <= 0; across 0, floor plus 1
    // for a negative value with a fraction (2 compares more)
    LV truncLV(LV x, const Range &r) {
        if (r.lo >= 0)
            return floorLV(x);
        if (r.hi <= 0)
            return neg(floorLV(neg(x)));
        LV fr = modLV(x, 1, false);
        LV f = addLV(x, neg(fr));
        LV fix = mulLV(positive(neg(x), false, true), positive(fr, false, true), nullptr);
        return addLV(f, fix);
    }

    LV lowerNode(Dsp &S, Node *n) {
        {   // the tag of the ops this node makes: its C line (a synthetic node: the line of an operand)
            const Node *src = n;
            for (int d = 0; d < 3 && !src->v && !src->in.empty(); d++) {
                const Node *nx = src->in.back();
                for (const Node *i : src->in)
                    if (i->v) { nx = i; break; }
                src = nx;
            }
            std::string w = src->v ? where(src->v) : "(" + n->op + ")";
            size_t c = w.find(", inlined");
            if (c != std::string::npos) w = w.substr(0, c);
            size_t sl = w.rfind('/');
            if (sl != std::string::npos) w = w.substr(sl + 1);
            K->tag = n->v ? w : "(" + n->op + ") " + w;
        }
        LV r = lowerNode1(S, n);
        for (auto it = viewGuard.rbegin(); it != viewGuard.rend(); ++it)
            val[it->first] = it->second;
        viewGuard.clear();
        // only the low `demand` bits are used: a value outside the signed range of those bits is kept modulo
        // 2^demand (LLVM's narrowed forms, products that overflow in C but whose low bits are what is read)
        // Only where a user overflows in C (that product is right modulo 2^bits only, which the DSP cannot keep
        // for a huge operand): elsewhere the users read the value as the integer it is (a trunc then shl).
        bool userWraps = false;
        for (Node *u : n->users)
            userWraps = userWraps || (u->kind == NK::Op && (u->op == "mul" || u->op == "shl" || u->op == "add" ||
                                                             u->op == "sub") && wrapped(u));
        if (n->isInt && n->demand > 1 && n->kind == NK::Op && !r.isConst() && n->prof >= 0 && userWraps) {
            double M = std::ldexp(1.0, n->demand);
            Range v = rawRange(n);                     // the C values (not the loose computed range)
            if (v.seen() && (v.lo < -M / 2 || v.hi >= M / 2))
                r = modLV(r, M, true);
        }
        return r;
    }

    LV lowerNode1(Dsp &S, Node *n) {
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
            if (n->name == "rdt")                     // 1/dt up to 12000: the host writes it / 16384
                x.s = 16384;
            if (n->name == "color0" || n->name == "color1")   // 0..1 whatever the profile saw (the host modulates it)
                x.V = Range{0, 1};
            if (n->name == "lfo_filter")              // the host's LFO -> filter: -4 .. +4 octaves
                x.V = Range{-4, 4};
            if (n->name == "lfo_amp")                 // the host's LFO -> amp: a gain 0 .. 1
                x.V = Range{0, 1};
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
            if (!x.V.seen()) {                            // no record (one side of a select of two tables): the table
                const auto &t = tables.at(n->name);        // column itself, every knob position
                int cnt = P.paramCount.at(n->name);
                for (size_t k = (size_t)n->c; k < t.size(); k += cnt)
                    x.V.add(t[k]);
            }
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
            if (!x.V.seen()) {                               // no load record (a store came first in the sample):
                auto it = prof.stateRange.find(n->state);     // the state's range at the start of a sample
                if (it != prof.stateRange.end())
                    x.V = it->second;
            }
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
        if (op == "recip")                      // no island chose it: recipseed and Newton
            return recip(in(0), n->in[0]);
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
        // operand i as the operation reads it (ashr / sitofp / icmp.slt: signed; lshr / uitofp / icmp.ult: unsigned):
        // a value lowered in the other view, out of this one's range, is reduced modulo 2^bits (the identity when
        // it is in range, so a loose range only costs the wrap)
        auto inV = [&](int i, bool wantSigned) -> LV {
            LV x = in(i);
            const Node *o = n->in[i];
            if (!o->isInt || o->bits <= 1 || o->bits > 32 || x.isConst())
                return x;
            if (needsView(o, !wantSigned, o->bits))
                return toView(x, o, !wantSigned, o->bits);
            return x;
        };
        static const std::set<std::string> readsSigned = {"ashr", "sext", "sitofp", "sdiv", "srem", "smin", "smax",
            "icmp.slt", "icmp.sle", "icmp.sgt", "icmp.sge"};
        static const std::set<std::string> readsUnsigned = {"lshr", "zext", "uitofp", "udiv", "urem", "umin", "umax",
            "icmp.ult", "icmp.ule", "icmp.ugt", "icmp.uge"};
        if (!allConst && (readsSigned.count(op) || readsUnsigned.count(op))) {
            bool sg = readsSigned.count(op) > 0;
            size_t nv = (op == "ashr" || op == "lshr" || op == "sdiv" || op == "udiv" || op == "srem" || op == "urem") ? 1 : n->in.size();
            for (size_t i = 0; i < nv; i++) {
                LV x = inV((int)i, sg);
                if (x.r != val.at(n->in[i]).r) {   // this operation reads the operand in its view; others keep theirs
                    viewGuard.push_back({n->in[i], val.at(n->in[i])});
                    val[n->in[i]] = x;
                }
            }
        }
        if (op == "add" || op == "fadd" || op == "sub" || op == "fsub") {
            bool sub = op == "sub" || op == "fsub";
            Range pre;
            LV a = in(0), b = sub ? neg(in(1)) : in(1);
            if (allConst)
                return cst(a.k + b.k);
            bool w = wrapped(n);
            double M = std::ldexp(1.0, n->bits);
            // an operand in the other integer view, outside this one's range (`a + ~(b - a)`: the add does not wrap
            // as signed, its operand is a huge unsigned): the sum is right modulo 2^bits only
            if (n->isInt && !w)
                for (int i = 0; i < 2; i++) {
                    Node *o = n->in[i];
                    if (!o->isInt || val.at(o).isConst() || o->bits != n->bits || !needsView(o, unsignedView(n), n->bits))
                        continue;
                    LV c = toView(val.at(o), o, unsignedView(n), n->bits);
                    if (i == 0) a = c; else b = sub ? neg(c) : c;
                }
            LV r = addLV(a, b, w ? nullptr : res, w ? M / 2 : 0);      // a wrap wants s = M / 2: no rescale
            if (w)
                return modLV(r, M, !unsignedView(n));
            return r;
        }
        if (op == "mul" || op == "fmul") {
            if (allConst)
                return cst(in(0).k * in(1).k);
            if (n->isInt && !wrapped(n)) {             // operands in the other view: the product, modulo 2^bits
                LV a = in(0), b = in(1);
                bool cv = false;
                if (needsView(n->in[0], unsignedView(n), n->bits)) a = toView(a, n->in[0], unsignedView(n), n->bits), cv = true;
                if (needsView(n->in[1], unsignedView(n), n->bits)) b = toView(b, n->in[1], unsignedView(n), n->bits), cv = true;
                if (cv)
                    return mulLV(a, b, res);
            }
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
        // a variable shift: x * 2^(+-n) (`phase_increment >>= num_shifts`); the DSP computes in reals, no floor (§8)
        if ((op == "shl" || op == "lshr" || op == "ashr") && !val.at(n->in[1]).isConst()) {
            LV e = exp2LV(op == "shl" ? in(1) : neg(in(1)));
            LV r = mulLV(in(0), e, wrapped(n) ? nullptr : res);
            if (op == "shl" && wrapped(n))
                return modLV(r, std::ldexp(1.0, n->bits), !unsignedView(n));
            return r;
        }
        if (op == "shl") {
            LV x0 = in(0);
            if (!wrapped(n) && needsView(n->in[0], unsignedView(n), n->bits))   // an operand in the other view
                x0 = toView(x0, n->in[0], unsignedView(n), n->bits);           // (a negative int16 << 1, read unsigned)
            LV r = scale(x0, std::ldexp(1.0, (int)constIn(1)));
            if (wrapped(n))
                return modLV(r, std::ldexp(1.0, n->bits), !unsignedView(n));
            return withRange(r);
        }
        if (op == "lshr" || op == "ashr") {
            int k = (int)constIn(1);
            LV r = scale(in(0), std::ldexp(1.0, -k));
            return withRange(r);
        }
        if (op == "fdiv") {
            if (val.at(n->in[1]).isConst())
                return withRange(scale(in(0), 1 / constIn(1)));
            if (n->in[1]->op == "recip")
                return mulLV(in(0), in(1), res);
            return mulLV(in(0), recip(in(1), n->in[1]), res);
        }
        if (op == "udiv" || op == "sdiv") {           // the DSP computes in reals: no floor (≤ 1 LSB, §8)
            if (val.at(n->in[1]).isConst()) {
                LV q = scale(in(0), 1 / constIn(1));
                // a small integer quotient (a count of octaves from a `while (x < 0) { x += 1536; n++; }` that LLVM
                // made a division): 1 LSB is everything there, so it is floored (toward 0 for sdiv: ceil if <= 0)
                Range qr = rec.seen() ? rec : rangeOf(q);       // the profile: a loose computed range hides it
                if (qr.hi - qr.lo <= 4096 && qr.hi - qr.lo >= 1)
                    return withRange(op == "sdiv" ? truncLV(q, qr) : floorLV(q));
                return withRange(q);
            }
            if (n->in[1]->op == "recip")
                return mulLV(in(0), in(1), res);
            return mulLV(in(0), recip(in(1), n->in[1]), res);
        }
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
                // per run of contiguous bits k..j of the constant: x ^ c = x + c - 2 (x mod 2^(j+1)) + 2 (x mod 2^k)
                // (floor modulo: right for negative x too, as two's complement); a run up to the top bit wraps
                LV r = x;
                double M = std::ldexp(1.0, n->bits);
                for (int k = 0; k < n->bits;) {
                    if (!((m >> k) & 1)) { k++; continue; }
                    int j = k;
                    while (j + 1 < n->bits && ((m >> (j + 1)) & 1))
                        j++;
                    double hi = std::ldexp(1.0, j + 1), lo = std::ldexp(1.0, k), c = hi - lo;
                    Range rr = rangeOf(r);
                    LV mh = (rr.lo >= 0 && rr.hi < hi) ? r : modLV(r, hi, false);
                    LV t = addLV(addLV(r, cst(c)), scale(mh, -2));
                    if (k > 0)
                        t = addLV(t, scale(modLV(r, lo, false), 2));
                    Range tr;
                    tr.add(rr.lo - hi);
                    tr.add(rr.hi + hi);
                    t.V = tr;
                    if (j == n->bits - 1)
                        t = modLV(t, M, !unsignedView(n));
                    else if (rr.lo >= 0 && rr.hi < hi) {
                        Range e;
                        e.add(0);
                        e.add(hi - 1);
                        t.V = e;
                    }
                    r = t;
                    k = j + 1;
                }
                return withRange(r);
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
            if ((op == "fptosi" || op == "fptoui") && rec.seen() && rec.hi - rec.lo <= 4096)
                return withRange(truncLV(x, rec));     // MAKE_INTEGRAL_FRACTIONAL: the integer part is everything
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
        if (op.rfind("icmp.", 0) == 0 || op.rfind("fcmp.", 0) == 0) {
            // a value that is always 0 or 1 (binary(): s1_gate, s1_trig, flags built from them): against a threshold
            // strictly between, the value is the answer; two of them are equal when (a - b)^2 is 0
            if (binary(n->in[0]) && binary(n->in[1]) && !in(0).isConst() && !in(1).isConst()) {
                std::string p = op.substr(op.find('.') + 1);
                LV d = addLV(in(0), neg(in(1)));
                LV ne = mulLV(d, d, nullptr);
                ne.V = Range();
                ne.V.add(0);
                ne.V.add(1);
                if (p == "one" || p == "une" || p == "ne")
                    return ne;
                if (p == "oeq" || p == "ueq" || p == "eq")
                    return bnot(ne);
            }
            for (int side = 0; side < 2; side++) {
                Node *x = n->in[side];
                LV c = in(1 - side);
                if (!binary(x) || in(side).isConst() || !c.isConst() || c.k <= 0 || c.k >= 1)
                    continue;
                std::string p = op.substr(op.find('.') + 1);
                bool gt = p == "ogt" || p == "ugt" || p == "oge" || p == "uge" || p == "sgt" || p == "sge";
                bool lt = p == "olt" || p == "ult" || p == "ole" || p == "ule" || p == "slt" || p == "sle";
                if (!gt && !lt)
                    break;
                bool above = side == 0 ? gt : lt;                 // true when the input is 1
                return above ? in(side) : bnot(in(side));
            }
            return compare(op, in(0), in(1), op[0] == 'i');
        }
        if (op == "select") {
            LV w;
            if (wrapSelect(n, w))
                return w;
            LV a = in(1), b = in(2);
            if (n->isInt && n->bits > 1 && n->bits <= 32) {   // both arms in the select's own view
                double M = std::ldexp(1.0, n->bits);
                (void)M;
                if (needsView(n->in[1], unsignedView(n), n->bits))
                    a = toView(a, n->in[1], unsignedView(n), n->bits);
                if (needsView(n->in[2], unsignedView(n), n->bits))
                    b = toView(b, n->in[2], unsignedView(n), n->bits);
            }
            LV r = withRange(select(in(0), a, b));
            return r;
        }
        if (op == "band") {
            // two booleans in registers with the same false level o and the same orientation (true above o, or
            // both below): AND is their min (max), one op; the product would first make both 0 / 1. The true level
            // of the result is the nearer of the two
            LV a = in(0), b = in(1);
            if (!a.isConst() && !b.isConst() && a.r && b.r && a.o == b.o && (a.s > 0) == (b.s > 0)) {
                LV r = a;
                r.r = a.s > 0 ? K->min(a.r, KOperand::of(b.r)) : K->max(a.r, KOperand::of(b.r));
                r.s = a.s > 0 ? std::max(a.s, b.s) : std::min(a.s, b.s);
                r.V = Range();
                r.V.add(0);
                r.V.add(1);
                r.isBool = true;
                return r;
            }
            return mulLV(a, b, nullptr);
        }
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

    // c ? x : x - 1 (or c ? x - 1 : x, x + -1): the wrap of a phase in [0, 1); not an envelope's stage select
    static bool isPhaseWrap(const Node *f) {
        if (!f || f->kind != NK::Op || f->op != "select" || f->in.size() != 3)
            return false;
        for (int arm = 1; arm <= 2; arm++) {
            const Node *a = f->in[arm], *b = f->in[3 - arm];
            if (a->kind != NK::Op || a->in.size() != 2 || a->in[0] != b || a->in[1]->kind != NK::Const)
                continue;
            if ((a->op == "fsub" && a->in[1]->c == 1) || (a->op == "fadd" && a->in[1]->c == -1))
                return true;
        }
        return false;
    }

    void chooseStateReps() {
        for (auto &s : P.states) {
            Range r;
            for (Dsp *S : {&P.A, &P.B})
                for (auto &np : S->nodes)
                    if (np->live && np->kind == NK::State && np->state == s.index)
                        r = recRange(np.get());
            if (s.final)
                if (Range f = recRange(s.final); f.seen()) { r.add(f.lo); r.add(f.hi); }
            if (auto it = prof.stateRange.find(s.index); it != prof.stateRange.end() && !r.seen())
                r = it->second;                                // only the start-of-sample range is known
            double M = std::ldexp(1.0, s.bits);
            if (s.isInt && s.isUnsigned && r.seen() && r.hi > M / 4) {
                stateRep[s.index] = {M / 2, -1};        // modular (a phase): r in [-1, 1)
            } else if (s.isInt && !s.isUnsigned && r.seen() && (r.hi > M / 4 || r.lo < -M / 4) && s.bits >= 16) {
                stateRep[s.index] = {M / 2, 0};
            } else if (!s.isInt && r.seen() && r.lo >= 0 && r.hi <= 1.0 + 1e-6 && isPhaseWrap(s.final)) {
                // a float phase in [0, 1) that ends in its wrap (p += dt; if (p >= 1) p -= 1): r in [-1, 1), the
                // form ph_wrap works in, so the wrap is one op and the load and the store need no conversion
                stateRep[s.index] = {0.5, -1};
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

    std::string run(Dsp &S, std::string &report) {
        Kernel k(opt.regs);
        K = &k;
        val.clear();
        recipCache.clear();
        warnedSeed = false;
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
            bool boolean = n->isInt && n->bits == 1;           // 0 / 1 here, -1 / 0 in the profile's signed view: no limits
            if (outliers(n) && !val[n].isConst() && n->kind == NK::Op && !n->specSel && !boolean) {   // rare outliers
                Range c = recRange(n);
                LV x = minmax(minmax(val[n], cst(c.lo), false), cst(c.hi), true);
                x.V = c;
                val[n] = normalize(x);
                saturated++;
            }
            bool repOnly = false;                               // only a scale / offset of an operand: nothing
            for (Node *i : n->in)                               // to limit (and a phase wrap stays recognisable)
                repOnly = repOnly || (val.count(i) && val[i].r && val[i].r == val[n].r);
            if (n->specFrontier && !val[n].isConst() && !repOnly && !boolean) {   // speculated: limited to its arm's range
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
            warnings.push_back({"W906", "DSP " + S.name + ": " + std::to_string(saturated) + " values limited to the range "
                               "of 99.99 % of their profiled samples (rare outliers would ruin their scale)"});
        saturated = 0;
        if (!unreached.empty())
            warnings.push_back({"W908", "DSP " + S.name + ": " + std::to_string(unreached.size()) + " values no "
                                "profiling scenario computed (a branch none of them took) are 0 on the DSP; the "
                                "first at " + where(unreached.front()->v)});
        unreached.clear();
        std::vector<std::string> lines = k.lines();
        report += "DSP " + S.name + ": " + std::to_string(k.units()) + " units" +
                  (k.spills() ? ", " + std::to_string(k.spills()) + " scratch words (spills)" : "") + "\n";
        {   // ops kept per C line (before scheduling: an instruction holds 1-3 of them; s1cc build --debug for
            // file:line)
            std::vector<std::pair<int, std::string>> byCost;
            for (auto &kv : k.liveByTag)
                byCost.push_back({kv.second, kv.first.empty() ? "(kernel: stores, inputs)" : kv.first});
            std::sort(byCost.rbegin(), byCost.rend());
            rep << "  cost of DSP " << S.name << " by C line (ops):\n";
            for (auto &c : byCost)
                rep << "    " << c.first << "  " << c.second << "\n";
        }
        std::ostringstream o;
        o << ".dsp " << S.name << " " << k.units() << "\n";
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
    std::map<std::string, std::vector<double>> all = tables;      // + the knob-derived columns (§4.6)
    for (auto &kv : P.derived)
        all[kv.first] = kv.second;
    Lowerer L(P, prof, all, opt);
    for (auto &kv : P.paramCount) {
        auto it = all.find(kv.first);
        if (it == all.end())
            fail("E006", "no table for S1_PARAM `" + kv.first + "` (tables file)");
        double m = 0;
        for (double v : it->second)
            m = std::max(m, std::fabs(v));
        L.paramScale[kv.first] = p2ceil(m);
    }
    // a compare that gave the same answer in every profiling scenario (a mode change that S1_INIT already made, a
    // clamp no note reaches) is that answer: a constant node, and liveness again, so the side it never takes and
    // the state words only that side kept cost nothing. The profile sweeps every knob and notes 0..127; W909 lists
    // them, --no-fold-branches keeps them
    if (opt.foldBranches)
        for (Dsp *S : {&P.A, &P.B}) {
            if (!S->fn)
                continue;
            std::vector<Node *> folded;
            for (auto &np : S->nodes) {
                Node *n = np.get();
                if (!n->live || n->kind != NK::Op || n->prof < 0 ||
                    (n->op.rfind("icmp.", 0) != 0 && n->op.rfind("fcmp.", 0) != 0))
                    continue;
                auto it = prof.sr.find(n->prof);
                if (it == prof.sr.end() || !it->second.seen() || it->second.lo != it->second.hi)
                    continue;
                L.rep << "  folded compare (always " << (it->second.lo != 0) << "): " << where(n->v) << "\n";
                n->kind = NK::Const;
                n->c = it->second.lo != 0;
                n->in.clear();
                n->bad = false;
                folded.push_back(n);
            }
            if (folded.empty())
                continue;
            relive(P, *S);
            L.warnings.push_back({"W909", "DSP " + S->name + ": " + std::to_string(folded.size()) + " compares gave "
                                  "one answer in every profiling scenario and are that constant (--no-fold-branches "
                                  "keeps them); the first at " + where(folded.front()->v)});
        }
    L.chooseStateReps();
    if (P.B.fn)
        for (auto &np : P.B.nodes)
            if (np->live && (np->kind == NK::Input || np->kind == NK::Increment) && np->name.rfind("env", 0) != 0)
                L.inputsB.insert(np->kind == NK::Increment ? "pitch" : np->name);   // s1_env: made on DSP B
    std::string a = L.run(P.A, report);
    std::string b = P.B.fn ? L.run(P.B, report) : "";
    std::ostringstream o;
    o << "; s1cc kernel (docs/S1CC.md §8): V = (r - o) s for every scaled word\n";
    for (auto &s : P.states) {
        auto sr = L.stateRep.at(s.index);
        o << ".state " << s.index << " " << s.dsp << " " << fmtImm(sr.first).substr(1) << " "
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
