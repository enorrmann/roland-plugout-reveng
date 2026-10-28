// Values -> registers -> one-unit lines (the form asm/dspsched.py packs; docs/S1CC.md §7). Same rules as
// sdk/mport/kernel.py: linear scan in program order, a dying operand's register may take the result (all units of
// an instruction read before they write), least recently freed register first, an accumulator read again later is
// copied first, a "mov rS, rS" filler after a load whose register the next line uses.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <deque>
#include <functional>
#include <regex>
#include <set>

#include "s1cc.h"

namespace s1 {

std::string fmtImm(double x) {
    char buf[64];
    snprintf(buf, sizeof buf, "#%.17g", x);
    std::string s = buf;
    if (s.find_first_of(".e") == std::string::npos)
        s += ".0";
    return s;
}

KVal *Kernel::val(const std::string &name) {
    vals_.push_back(std::make_unique<KVal>());
    vals_.back()->n = (int)vals_.size();
    vals_.back()->name = name;
    return vals_.back().get();
}

KVal *Kernel::op(const std::string &fmt, std::vector<KOperand> args, KVal *acc) {
    // the same op on the same values again (two C expressions that lower alike): its first result (values are
    // single-assignment, ops have no side effect; loads and stores are not ops)
    for (auto it = ops_.rbegin(); it != ops_.rend(); ++it) {
        if (it->kind != "op" || it->fmt != fmt || it->acc != acc || it->args.size() != args.size())
            continue;
        bool same = true;
        for (size_t i = 0; i < args.size() && same; i++)
            same = args[i].v == it->args[i].v && args[i].neg == it->args[i].neg &&
                   (args[i].v || args[i].imm == it->args[i].imm);
        if (same)
            return it->d;
    }
    KOp o;
    o.kind = "op";
    o.fmt = fmt;
    o.d = val();
    o.args = std::move(args);
    o.acc = acc;
    o.tag = tag;
    ops_.push_back(o);
    return o.d;
}

KVal *Kernel::load(const std::string &addr, const std::string &name) {
    KOp o;
    o.kind = "ld";
    o.d = val(name);
    o.addr = addr;
    o.tag = tag;
    ops_.push_back(o);
    return o.d;
}

void Kernel::store(KVal *v, const std::string &addr) {
    KOp o;
    o.kind = "st";
    o.args = {KOperand::of(v)};
    o.addr = addr;
    o.tag = tag;
    ops_.push_back(o);
}

void Kernel::comment(const std::string &text) {
    KOp o;
    o.kind = "comment";
    o.fmt = "; " + text;
    o.tag = tag;
    ops_.push_back(o);
}

KVal *Kernel::mov(KOperand y) {
    if (!y.v && std::fabs(y.imm) >= 512)
        fail("E701", "kernel: constant " + std::to_string(y.imm) + " outside the DSP range (+-512)");
    return op("mov {d}, {0}", {y});
}
// an immediate the DSP holds well: |a| <= 256, and below 2^-12 only a power of two (the grid is 2^-32 there)
static bool goodImm(double a) {
    if (a == 0 || std::fabs(a) > 256)
        return a == 0;
    if (std::fabs(a) >= std::ldexp(1.0, -12))
        return true;
    int e;
    return std::frexp(std::fabs(a), &e) == 0.5 && e >= -30;
}

KVal *Kernel::mul(KVal *x, KOperand y) {
    if (!y.v) {
        double a = y.imm;
        if (!std::isfinite(a))
            fail("E702", "kernel: a multiply by a non-finite constant (a range the profile left unbounded)");
        if (a == 1)
            return x;
        // fold into the multiply that made x (the peephole pass in lines() drops the old one if unused)
        for (auto it = ops_.rbegin(); it != ops_.rend(); ++it)
            if (it->d == x) {
                if (it->fmt == "mul {d}, {0}, {1}" && !it->args[1].v && goodImm(it->args[1].imm * a))
                    return mul(it->args[0].v, KOperand::im(it->args[1].imm * a));
                break;
            }
        if (std::fabs(a) > 256) {                // the power of two by shl (one op on unit B, any size up to 2^15)
            int e;
            std::frexp(std::fabs(a), &e);
            int k = e - 1;                          // |a| = 2^k b, b in [1, 2)
            if (std::fabs(a) == std::ldexp(1.0, k) && a > 0)
                return shift(x, k);
            x = shift(x, k);
            a = std::ldexp(a, -k);
        }
        if (!goodImm(a)) {
            int e;
            std::frexp(a, &e);
            double p = std::ldexp(1.0, e - 1 + 12 > 0 ? -(12 - 0) : 0);   // power of two that brings a above 2^-12
            p = std::ldexp(1.0, std::max(e - 1, -30) - (e - 1 < -12 ? 0 : 0));
            double big = a / std::ldexp(1.0, e - 1);                      // in [1, 2)
            x = op("mul {d}, {0}, {1}", {KOperand::of(x), KOperand::im(std::ldexp(1.0, e - 1))});
            a = big;
            (void)p;
        }
        y.imm = a;
    }
    return op("mul {d}, {0}, {1}", {KOperand::of(x), y});
}
KVal *Kernel::add(KVal *x, KOperand y) {
    if (!y.v && !std::isfinite(y.imm))
        fail("E702", "kernel: a non-finite constant");
    // a constant beyond the immediate range: in steps (x and x + c are registers, so every step between is too)
    while (!y.v && std::fabs(y.imm) >= 512) {
        double step = y.imm > 0 ? 511.0 : -511.0;
        x = op("addxy {d}, {0}, {1}", {KOperand::of(x), KOperand::im(step)});
        y.imm -= step;
        if (std::fabs(y.imm) > 1e6)
            fail("E701", "kernel: constant " + std::to_string(y.imm) + " outside the DSP range (+-512)");
    }
    return op("addxy {d}, {0}, {1}", {KOperand::of(x), y});
}
KVal *Kernel::sub(KVal *x, KOperand y) {
    if (!y.v)
        return add(x, KOperand::im(-y.imm));
    if (y.neg)
        return add(x, KOperand::of(y.v));
    return op("subxy {d}, {0}, {1}", {y, KOperand::of(x)});      // subxy d, X, Y = Y - X
}
KVal *Kernel::mad(KVal *acc, KVal *x, KOperand y) {
    if (!y.v && !goodImm(y.imm))
        return add(acc, KOperand::of(mul(x, y)));
    return op("mad {d}, {0}, {1}", {KOperand::of(x), y}, acc);
}
KVal *Kernel::ymac(KVal *acc, KVal *x, KOperand y) { return op("ymacd {d}, {0}, {1}", {KOperand::of(x), y}, acc); }
KVal *Kernel::min(KVal *x, KOperand y) { return op("min {d}, {0}, {1}", {KOperand::of(x), y}); }
KVal *Kernel::max(KVal *x, KOperand y) { return op("max {d}, {0}, {1}", {KOperand::of(x), y}); }
namespace {
// e^f on [0, 1): least squares of the relative error, degree 6 (5.7e-8)
const double kExpPoly[7] = {1.00000005659, 0.999996450303, 0.500051973938, 0.166360755989, 0.0425387636987,
                            0.00706573356093, 0.00226793999827};
// the shift field of shl / shr / shrn: 4 bits written as a register operand (bit 3 = its negation sign)
std::string shiftField(int k) { return k >= 8 ? "-r" + std::to_string(k - 8) : "r" + std::to_string(k); }
}

KVal *Kernel::multi(const std::string &fmt, KVal *arg, bool pairD, bool clobber, bool second) {
    KOp o;
    o.kind = "multi";
    o.fmt = fmt;
    o.d = val();
    o.d2 = second ? val() : nullptr;
    o.args = {KOperand::of(arg)};
    o.pairD = pairD;
    o.clobber = clobber;
    o.tag = tag;
    ops_.push_back(o);
    return o.d;
}

KVal *Kernel::recip(KVal *y) {
    // Goldschmidt with the SFU seed (Roland's division): recipseed leaves N = sign 2^-e in r(D), F ~ 1/m (ROM, 1 %)
    // in r(D+1) and m in rY; each newton: q *= f, f = 2 - f m, m *= f. Three steps: q = N F f1 f2 (1e-8)
    return multi("recipseed {d}, {0}\nnewton {d}, {0}\nnewton {d}, {0}\nnewton {d}, {0}", y, true, true, false);
}

std::pair<KVal *, KVal *> Kernel::expsplit(KVal *z) {
    KVal *d = multi("expsplit {d}, {0}", z, false, true, true);
    return {d, ops_.back().d2};
}

std::pair<KVal *, KVal *> Kernel::log2split(KVal *y) {
    KVal *d = multi("log2split {d}, {0}", y, false, true, true);
    return {d, ops_.back().d2};
}

KVal *Kernel::exp2split(KVal *t, double shift) {
    KVal *z = mul(t, KOperand::im(std::log(2.0)));
    if (shift != 0)
        z = add(z, KOperand::im(shift * std::log(2.0)));
    auto ef = expsplit(z);
    KVal *p = mov(KOperand::im(kExpPoly[6]));
    for (int k = 5; k >= 0; k--)
        p = ymac(p, ef.second, KOperand::im(kExpPoly[k]));
    return mul(p, KOperand::of(ef.first));
}

KVal *Kernel::floor(KVal *x) { return op("floor {d}, {0}", {KOperand::of(x)}); }
KVal *Kernel::ceil(KVal *x) { return op("ceil {d}, {0}", {KOperand::of(x)}); }
KVal *Kernel::sign(KVal *x) { return op("sign {d}, {0}", {KOperand::of(x)}); }

KVal *Kernel::shift(KVal *x, int k) {
    // shl / shr: 2^k with |k| <= 15; shrn: 2^-(16 + j), j <= 15
    while (k > 15) {
        x = op("shl {d}, {0}, " + shiftField(15), {KOperand::of(x)});
        k -= 15;
    }
    if (k > 0)
        return op("shl {d}, {0}, " + shiftField(k), {KOperand::of(x)});
    if (k == 0)
        return x;
    if (k >= -15)
        return op("shr {d}, {0}, " + shiftField(-k), {KOperand::of(x)});
    if (k >= -31)
        return op("shrn {d}, {0}, " + shiftField(-k - 16), {KOperand::of(x)});
    return shift(op("shrn {d}, {0}, " + shiftField(15), {KOperand::of(x)}), k + 31);
}

KVal *Kernel::mac2(KVal *a, KVal *b, KVal *c, KVal *d, KVal *e) {
    // MAC unit, register coefficient: rD <- rX rY + rC rZ (+ rW); text order D, X, Y, Z, C, W
    if (e)
        return op("mac_p1p2 {d}, {0}, {1}, {3}, {2}, {4}",
                  {KOperand::of(a), KOperand::of(b), KOperand::of(c), KOperand::of(d), KOperand::of(e)});
    return op("mac_p1w {d}, {0}, {1}, {3}, {2}", {KOperand::of(a), KOperand::of(b), KOperand::of(c), KOperand::of(d)});
}

KVal *Kernel::sum3(KVal *a, KVal *b, KVal *c) {
    return op("sum8 {d}, {0}, {1}, {2}", {KOperand::of(a), KOperand::of(b), KOperand::of(c)});
}

KVal *Kernel::twoMinus(KVal *x) { return op("subxy {d}, {0}, #2.0", {KOperand::of(x)}); }

KVal *Kernel::wrap(KVal *x) { return op("ph_wrap {d}, r3, {0}", {KOperand::of(x)}); }
KVal *Kernel::tri(KVal *x) { return op("ph_tri {d}, r3, -{0}", {KOperand::of(x)}); }
KVal *Kernel::clamp(KVal *x) { return op("ph_clamp {d}, r3, -{0}", {KOperand::of(x)}); }

KVal *Kernel::exp2(KVal *x, int extra) {
    if (extra > 8)
        fail("E703", "kernel: exp2 of a value spanning more than 2^11 (the profile's range is unbounded)");
    // 2^x = (e^(x ln2 / 2^(4 + extra)))^(2^(4 + extra)), e^y by 4 Taylor terms (make_synth.py exp2_to)
    double k = std::log(2.0) / std::ldexp(1.0, 4 + extra);
    KVal *y = mul(x, KOperand::im(k));
    KVal *t = mov(KOperand::im(1.0 / 24));
    for (double c : {1.0 / 6, 0.5, 1.0, 1.0})
        t = ymac(t, y, KOperand::im(c));
    for (int i = 0; i < 4 + extra; i++)
        t = mul(t, KOperand::of(t));
    return t;
}

int Kernel::units() const {
    int n = 0;
    for (auto &o : ops_)
        n += o.kind != "comment";
    return n;
}

std::vector<std::string> Kernel::lines() {
    // accumulators read again later: work on a copy
    std::map<int, int> lastRead;
    auto reads = [](const KOp &o) {
        std::vector<KVal *> r;
        for (auto &a : o.args)
            if (a.v)
                r.push_back(a.v);
        if (o.acc)
            r.push_back(o.acc);
        return r;
    };
    // dead results (a multiply folded into a later one): drop, repeatedly
    for (bool changed = true; changed;) {
        changed = false;
        std::set<int> used;
        for (auto &o : ops_)
            for (KVal *v : reads(o))
                used.insert(v->n);
        std::vector<KOp> keep;
        for (auto &o : ops_) {
            if ((o.kind == "op" && o.d && !used.count(o.d->n)) ||
                (o.kind == "multi" && !used.count(o.d->n) && !(o.d2 && used.count(o.d2->n)))) {
                changed = true;
                continue;
            }
            keep.push_back(o);
        }
        ops_.swap(keep);
    }
    liveByTag.clear();
    for (auto &o : ops_)
        if (o.kind != "comment")
            liveByTag[o.tag]++;
    if (getenv("S1CC_OPS"))               // debugging: every kept op with the C line that made it
        for (auto &o : ops_)
        {
            std::string a;
            for (auto &x : o.args)
                a += x.v ? (x.neg ? " -v" : " v") + std::to_string(x.v->n) : " #" + std::to_string(x.imm);
            if (o.acc)
                a += " acc v" + std::to_string(o.acc->n);
            fprintf(stderr, "op %-6s %-22s v%-4d <-%-34s | %s\n", o.kind.c_str(), o.fmt.substr(0, o.fmt.find(' ')).c_str(),
                    o.d ? o.d->n : -1, a.c_str(), o.tag.c_str());
        }
    std::vector<KOp> ops;
    {
        std::map<int, int> last;
        for (int i = 0; i < (int)ops_.size(); i++)
            for (KVal *v : reads(ops_[i]))
                last[v->n] = i;
        for (int i = 0; i < (int)ops_.size(); i++) {
            KOp o = ops_[i];
            if (o.kind == "multi" && o.clobber && last[o.args[0].v->n] > i) {
                KOp c;
                c.kind = "op";
                c.fmt = "mov {d}, {0}";
                c.d = val("copy");
                c.args = {o.args[0]};
                ops.push_back(c);
                o.args[0] = KOperand::of(c.d);
            }
            if (o.acc && last[o.acc->n] > i) {
                KOp c;
                c.kind = "op";
                c.fmt = "mov {d}, {0}";
                c.d = val("copy");
                c.args = {KOperand::of(o.acc)};
                ops.push_back(c);
                o.acc = c.d;
            }
            ops.push_back(o);
        }
    }
    // linear scan with spilling: a value that does not fit is stored to a scratch word {T<k>} (bank 0, written
    // and read in the same sample) and loaded back before its next use; the victim is the value used furthest away
    std::map<int, std::vector<int>> uses;
    for (int i = 0; i < (int)ops.size(); i++)
        for (KVal *v : reads(ops[i]))
            uses[v->n].push_back(i);
    auto nextUse = [&](int n, int i) {
        auto &u = uses[n];
        auto it = std::lower_bound(u.begin(), u.end(), i);
        return it == u.end() ? 1 << 30 : *it;
    };
    std::deque<int> freeRegs(regs_.begin(), regs_.end());
    std::map<int, KVal *> live;            // value number -> value, in a register
    std::map<int, std::string> spilled;    // value number -> scratch word
    // a value reloaded from its scratch word keeps it until its last use: values do not change, so spilling it again
    // needs no store (2026-10-28: mp_csaw in four registers stored T3 back unchanged)
    std::map<int, std::string> home;
    std::vector<KOp> out2;
    int nslot = 0;
    std::vector<std::string> freeSlots;
    std::function<int(int, const std::set<int> &, bool)> takeReg = [&](int i, const std::set<int> &keep,
                                                                        bool forceSpill) -> int {
        if (!freeRegs.empty() && !forceSpill) {
            int r = freeRegs.front();
            freeRegs.pop_front();
            return r;
        }
        KVal *victim = nullptr;
        int far = -1;
        for (auto &kv : live)
            if (!keep.count(kv.first) && nextUse(kv.first, i) > far) {
                far = nextUse(kv.first, i);
                victim = kv.second;
            }
        if (!victim)
            fail("E704", "kernel: out of registers (" + std::to_string(regs_.size()) + " usable) even with spilling");
        auto hm = home.find(victim->n);
        if (hm != home.end()) {                  // still in its scratch word: no store
            spilled[victim->n] = hm->second;
            home.erase(hm);
            live.erase(victim->n);
            return victim->reg;
        }
        std::string slot;
        if (!freeSlots.empty()) {
            slot = freeSlots.back();
            freeSlots.pop_back();
        } else
            slot = "{T" + std::to_string(nslot++) + "}";
        KOp st;
        st.kind = "st";
        st.args = {KOperand::of(victim)};
        st.addr = slot;
        out2.push_back(st);
        spilled[victim->n] = slot;
        live.erase(victim->n);
        return victim->reg;
    };
    for (int i = 0; i < (int)ops.size(); i++) {
        KOp o = ops[i];
        auto rs = reads(o);
        std::set<int> keep;
        for (KVal *v : rs)
            keep.insert(v->n);
        // reload spilled operands
        for (KVal *v : rs) {
            auto it = spilled.find(v->n);
            if (it == spilled.end())
                continue;
            KVal *nv = val(v->name);
            nv->reg = takeReg(i, keep, false);
            KOp ld;
            ld.kind = "ld";
            ld.d = nv;
            ld.addr = it->second;
            out2.push_back(ld);
            std::string slot = it->second;
            spilled.erase(it);
            // later reads of v read nv
            for (int j = i; j < (int)ops.size(); j++) {
                for (auto &a : ops[j].args)
                    if (a.v == v)
                        a.v = nv;
                if (ops[j].acc == v)
                    ops[j].acc = nv;
            }
            uses[nv->n] = uses[v->n];
            home[nv->n] = slot;
            keep.insert(nv->n);
            live[nv->n] = nv;
            o = ops[i];
        }
        rs = reads(o);
        for (KVal *v : rs)
            if (v->reg < 0)
                fail("E705", "kernel: value read before it is defined");
        for (KVal *v : rs) {
            bool dies = nextUse(v->n, i + 1) == 1 << 30;
            if (dies && live.erase(v->n))
                freeRegs.push_back(v->reg);
            auto hm = home.find(v->n);
            if (dies && hm != home.end()) {        // its scratch word is free again
                freeSlots.push_back(hm->second);
                home.erase(hm);
            }
        }
        if (o.kind == "multi") {                     // native op writing more than D (Kernel::multi)
            int yreg = o.args[0].v->reg;
            if (o.clobber) {                         // the argument died here (a copy was made otherwise)
                auto it = std::find(freeRegs.begin(), freeRegs.end(), yreg);
                if (it == freeRegs.end())
                    fail("E707", "kernel: the argument of " + o.fmt.substr(0, o.fmt.find(' ')) + " is still live");
                freeRegs.erase(it);
            }
            int got = -1;
            if (o.pairD) {                           // r(D), r(D+1)
                for (int tries = 0; tries < 16 && got < 0; tries++) {
                    for (int r : freeRegs)
                        if (r < 7 && std::find(freeRegs.begin(), freeRegs.end(), r + 1) != freeRegs.end()) {
                            got = r;
                            break;
                        }
                    if (got < 0) {                      // spill until a pair is free
                        int r = takeReg(i, keep, true);
                        freeRegs.push_back(r);
                    }
                }
                if (got < 0)
                    fail("E706", "kernel: no free register pair for " + o.fmt.substr(0, o.fmt.find(' ')));
                freeRegs.erase(std::find(freeRegs.begin(), freeRegs.end(), got));
                freeRegs.erase(std::find(freeRegs.begin(), freeRegs.end(), got + 1));
            } else {
                got = takeReg(i, keep, false);
            }
            o.d->reg = got;
            if (o.d2)
                o.d2->reg = yreg;
            for (KVal *v : {o.d, o.d2}) {
                if (!v)
                    continue;
                if (nextUse(v->n, i + 1) != 1 << 30)
                    live[v->n] = v;
                else
                    freeRegs.push_back(v->reg);
            }
            if (o.pairD)
                freeRegs.push_back(got + 1);
            if (o.clobber && !o.d2)
                freeRegs.push_back(yreg);
            out2.push_back(o);
            continue;
        }
        if (o.d) {
            if (o.acc) {
                auto it = std::find(freeRegs.begin(), freeRegs.end(), o.acc->reg);
                if (it != freeRegs.end())
                    freeRegs.erase(it);
                o.d->reg = o.acc->reg;
            } else {
                o.d->reg = takeReg(i, keep, false);
            }
            if (nextUse(o.d->n, i + 1) != 1 << 30)
                live[o.d->n] = o.d;
            else
                freeRegs.push_back(o.d->reg);
        }
        out2.push_back(o);
    }
    ops.swap(out2);
    spills_ = nslot;
    auto operand = [](const KOperand &a) {
        if (!a.v)
            return fmtImm(a.imm);
        return std::string(a.neg ? "-r" : "r") + std::to_string(a.v->reg);
    };
    std::vector<std::string> out;
    for (auto &o : ops) {
        std::string s;
        if (o.kind == "comment")
            s = o.fmt;
        else if (o.kind == "ld")
            s = "ld r" + std::to_string(o.d->reg) + ", [" + o.addr + "]";
        else if (o.kind == "st")
            s = "st r" + std::to_string(o.args[0].v->reg) + ", [" + o.addr + "]";
        else if (o.kind == "multi") {
            s = o.fmt;
            auto all = [&](const std::string &key, const std::string &val) {
                for (size_t p; (p = s.find(key)) != std::string::npos;)
                    s.replace(p, key.size(), val);
            };
            all("{d}", "r" + std::to_string(o.d->reg));
            all("{d1}", "r" + std::to_string(o.d->reg + 1));
            all("{0}", "r" + std::to_string(o.args[0].v->reg));
            for (size_t p = 0, q; (q = s.find('\n', p)) != std::string::npos; p = 0) {
                out.push_back(s.substr(0, q));
                s = s.substr(q + 1);
            }
        } else {
            s = o.fmt;
            auto rep = [&](const std::string &key, const std::string &val) {
                size_t p = s.find(key);
                if (p != std::string::npos)
                    s.replace(p, key.size(), val);
            };
            rep("{d}", "r" + std::to_string(o.d->reg));
            for (size_t k = 0; k < o.args.size(); k++) {
                std::string a = operand(o.args[k]);
                std::string key = "{" + std::to_string(k) + "}";
                size_t p = s.find(key);
                if (p != std::string::npos) {
                    if (p > 0 && s[p - 1] == '-' && a[0] == 'r') {         // ph_* -rY form
                        s.replace(p, key.size(), a);
                    } else
                        s.replace(p, key.size(), a);
                }
            }
        }
        out.push_back(s);
    }
    // load fillers
    std::regex reg("\\br(\\d)\\b");
    std::vector<std::string> res;
    for (size_t i = 0; i < out.size(); i++) {
        res.push_back(out[i]);
        if (out[i].rfind("ld r", 0) != 0)
            continue;
        int dst = out[i][4] - '0';
        size_t j = i + 1;
        while (j < out.size() && out[j][0] == ';')
            j++;
        if (j >= out.size())
            continue;
        std::string body = out[j];
        bool uses = false;
        for (std::sregex_iterator it(body.begin(), body.end(), reg), e; it != e; ++it)
            uses |= (*it)[1].str()[0] - '0' == dst;
        if (uses) {
            int spare = regs_[0] != dst ? regs_[0] : regs_[1];
            res.push_back("mov r" + std::to_string(spare) + ", r" + std::to_string(spare));
        }
    }
    return res;
}

}  // namespace s1
