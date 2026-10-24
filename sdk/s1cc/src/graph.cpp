// The voice functions of the LLVM module as dataflow graphs (s1cc.h, docs/S1CC.md §4).
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstdlib>
#include <functional>

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/raw_ostream.h"

#include "s1cc.h"

using namespace llvm;

namespace s1 {

void fail(const std::string &msg) {
    fprintf(stderr, "s1cc: error: %s\n", msg.c_str());
    exit(1);
}

void warn(const std::string &msg) { fprintf(stderr, "s1cc: warning: %s\n", msg.c_str()); }

std::string where(const Value *v) {
    if (auto *I = dyn_cast_or_null<Instruction>(v)) {
        if (const DebugLoc &dl = I->getDebugLoc())
            return dl->getFilename().str() + ":" + std::to_string(dl.getLine());
        std::string s;
        raw_string_ostream os(s);
        I->print(os);
        return "`" + s.substr(s.find_first_not_of(' ')) + "` in " + I->getFunction()->getName().str();
    }
    if (!v)
        return "(synthetic)";
    std::string s;
    raw_string_ostream os(s);
    v->print(os);
    return "`" + s + "`";
}

namespace {

const std::set<std::string> kLibm = {
    "sinf", "cosf", "tanf", "tanhf", "sinhf", "coshf", "expf", "exp2f", "logf", "log2f", "log10f", "powf",
    "sqrtf", "atanf", "atan2f", "asinf", "acosf", "fmodf", "cbrtf", "sin", "cos", "tan", "tanh", "exp", "exp2",
    "log", "log2", "log10", "pow", "sqrt", "atan", "atan2", "fmod"};

// math functions lower.cpp computes or fits on the argument's profiled range (no island needed)
bool knownMath(const std::string &f) {
    static const std::set<std::string> k = {"sin", "cos", "tan", "tanh", "sinh", "cosh", "exp", "exp2", "log",
                                            "log2", "log10", "sqrt", "pow", "atan", "asin", "acos", "cbrt"};
    return k.count(f) > 0;
}

struct Ptr {
    GlobalVariable *gv = nullptr;
    long off = 0;
    Node *var = nullptr;     // computed part (a "gep" node), nullptr: constant offset
    AllocaInst *local = nullptr;   // a local variable of the sample (alloca): forwarded, never in the ring
};

struct Builder {
    Program &P;
    Stage &S;
    const DataLayout &DL;
    std::map<const Value *, Node *> vmap;
    std::map<const Value *, Ptr> pmap;
    std::map<int, Node *> cur;           // state index -> current value
    std::map<std::string, Node *> inputs;

    Builder(Program &p, Stage &s) : P(p), S(s), DL(p.M->getDataLayout()) {}

    Node *mk(NK k, const std::string &op, std::vector<Node *> in, Value *v = nullptr) {
        auto n = std::make_unique<Node>();
        n->id = (int)S.nodes.size();
        n->kind = k;
        n->op = op;
        n->in = std::move(in);
        n->v = v;
        if (v && v->getType()->isIntegerTy()) {
            n->isInt = true;
            n->bits = (int)v->getType()->getIntegerBitWidth();
        }
        Node *r = n.get();
        S.nodes.push_back(std::move(n));
        return r;
    }

    Node *cst(double c, bool isInt = false, int bits = 0) {
        Node *n = mk(NK::Const, "const", {});
        n->c = c;
        n->isInt = isInt;
        n->bits = bits;
        return n;
    }

    Node *boolOp(const std::string &op, std::vector<Node *> in) {
        Node *n = mk(NK::Op, op, std::move(in));
        n->isInt = true;
        n->bits = 1;
        return n;
    }
    Node *band(Node *a, Node *b) { return !a ? b : !b ? a : boolOp("band", {a, b}); }
    Node *bnot(Node *a) { return boolOp("bnot", {a}); }
    Node *sel(Node *c, Node *a, Node *b, const Node *like) {   // c ? a : b (c == nullptr: a)
        if (!c)
            return a;
        Node *n = mk(NK::Op, "select", {c, a, b});
        n->isInt = like->isInt;
        n->bits = like->bits;
        return n;
    }

    Node *constOf(Constant *C) {
        if (auto *ci = dyn_cast<ConstantInt>(C))
            return cst((double)ci->getSExtValue(), true, (int)ci->getBitWidth());
        if (auto *cf = dyn_cast<ConstantFP>(C))
            return cst(cf->getValueAPF().convertToDouble());
        if (isa<UndefValue>(C) || isa<ConstantPointerNull>(C) || C->isNullValue())
            return cst(0, C->getType()->isIntegerTy(), C->getType()->isIntegerTy() ? (int)C->getType()->getIntegerBitWidth() : 0);
        fail("unsupported constant " + where(C));
    }

    Node *get(Value *v) {
        auto it = vmap.find(v);
        if (it != vmap.end())
            return it->second;
        if (auto *C = dyn_cast<Constant>(v)) {
            if (!C->getType()->isPointerTy()) {
                Node *n = constOf(C);
                vmap[v] = n;
                return n;
            }
        }
        fail("value not available (pointer used as a number, or an unsupported instruction): " + where(v));
    }

    Ptr ptrOf(Value *v) {
        v = v->stripPointerCasts();
        auto it = pmap.find(v);
        if (it != pmap.end())
            return it->second;
        if (auto *gv = dyn_cast<GlobalVariable>(v))
            return Ptr{gv, 0, nullptr};
        if (auto *al = dyn_cast<AllocaInst>(v)) {
            Ptr p;
            p.local = al;
            return p;
        }
        if (auto *gep = dyn_cast<GEPOperator>(v)) {
            Ptr base = ptrOf(gep->getPointerOperand());
            APInt off(64, 0);
            if (!base.var && gep->accumulateConstantOffset(DL, off)) {
                Ptr p = base;
                p.off += off.getSExtValue();
                pmap[v] = p;
                return p;
            }
            std::vector<Node *> in;
            if (base.var)
                in.push_back(base.var);
            for (auto &idx : gep->indices())
                if (!isa<Constant>(idx))
                    in.push_back(get(idx));
            Node *n = mk(NK::Op, "gep", in, isa<Instruction>(v) ? v : nullptr);
            n->bad = true;
            n->isInt = true;
            n->bits = 64;
            if (base.local)
                fail("computed index into a local array " + where(v) + ": use constant indices");
            Ptr p{base.gv, 0, n};
            pmap[v] = p;
            return p;
        }
        fail("unsupported pointer " + where(v) + " (only file-scope variables and tables)");
    }

    int stateOf(GlobalVariable *gv, long off, Type *ty, Value *at) {
        for (auto &s : P.states)
            if (s.gv == gv && s.offset == off) {
                if (s.stage != S.name)
                    fail("state `" + s.name + "` used by both S1_VOICE and S1_VOICE_B (" + where(at) + ")");
                return s.index;
            }
        StateVar s;
        s.index = (int)P.states.size();
        s.gv = gv;
        s.offset = off;
        s.name = gv->getName().str() + (off ? "+" + std::to_string(off) : "");
        s.isInt = ty->isIntegerTy();
        s.bits = s.isInt ? (int)ty->getIntegerBitWidth() : 0;
        s.stage = S.name;
        if (gv->hasInitializer()) {
            if (Constant *c = ConstantFoldLoadFromConst(gv->getInitializer(), ty, APInt(64, off), DL)) {
                if (auto *ci = dyn_cast<ConstantInt>(c))
                    s.init = (double)ci->getSExtValue();
                else if (auto *cf = dyn_cast<ConstantFP>(c))
                    s.init = cf->getValueAPF().convertToDouble();
            }
        }
        if (!ty->isIntegerTy() && !ty->isFloatTy() && !ty->isDoubleTy())
            fail("state `" + s.name + "` is not an int or a float (" + where(at) + ")");
        P.states.push_back(s);
        S.states.push_back(s.index);
        return s.index;
    }

    Node *stateValue(int idx, Value *at) {
        auto it = cur.find(idx);
        if (it != cur.end())
            return it->second;
        Node *n = mk(NK::State, "state", {}, at);
        n->state = idx;
        n->isInt = P.states[idx].isInt;
        n->bits = P.states[idx].bits;
        cur[idx] = n;
        return n;
    }

    std::map<std::pair<AllocaInst *, long>, Node *> locals;   // local variable -> current value

    void load(LoadInst *L) {
        Ptr p = ptrOf(L->getPointerOperand());
        if (p.local) {
            auto it = locals.find({p.local, p.off});
            vmap[L] = it != locals.end() ? it->second : cst(0, L->getType()->isIntegerTy(),
                                                             L->getType()->isIntegerTy() ? (int)L->getType()->getIntegerBitWidth() : 0);
            return;
        }
        if (!p.gv)
            fail("load from an unknown address " + where(L));
        if (p.var) {
            if (!p.gv->isConstant())
                fail("computed index into a state array " + where(L) + ": state arrays need constant indices");
            Node *n = mk(NK::Op, "tload", {p.var}, L);
            n->bad = true;
            vmap[L] = n;
            return;
        }
        if (p.gv->isConstant() && p.gv->hasInitializer()) {
            Constant *c = ConstantFoldLoadFromConst(p.gv->getInitializer(), L->getType(), APInt(64, p.off), DL);
            if (!c)
                fail("cannot fold the table read " + where(L));
            vmap[L] = constOf(c);
            return;
        }
        if (!p.gv->hasInitializer())
            fail("`" + p.gv->getName().str() + "` has no definition in the module (" + where(L) +
                 "): compile and link the source that defines it");
        int idx = stateOf(p.gv, p.off, L->getType(), L);
        Node *n = stateValue(idx, L);
        vmap[L] = n;
    }

    void store(StoreInst *St, Node *pred) {
        Ptr p = ptrOf(St->getPointerOperand());
        if (p.local) {
            Node *v = get(St->getValueOperand());
            auto it = locals.find({p.local, p.off});
            locals[{p.local, p.off}] = pred && it != locals.end() ? sel(pred, v, it->second, v) : v;
            return;
        }
        if (!p.gv || p.var || p.gv->isConstant())
            fail("store to something that is not a state word " + where(St));
        int idx = stateOf(p.gv, p.off, St->getValueOperand()->getType(), St);
        Node *v = get(St->getValueOperand());
        Node *old = stateValue(idx, St);
        cur[idx] = pred ? sel(pred, v, old, v) : v;
    }

    std::string paramName(Value *desc, long &count) {
        auto *gv = dyn_cast<GlobalVariable>(desc->stripPointerCasts());
        if (!gv || !gv->hasInitializer())
            fail("s1_param: unknown table " + where(desc));
        auto *cs = dyn_cast<ConstantStruct>(gv->getInitializer());
        if (!cs)
            fail("s1_param: not an S1_PARAM descriptor " + where(desc));
        auto *ngv = dyn_cast<GlobalVariable>(cs->getOperand(0)->stripPointerCasts());
        auto *str = ngv ? dyn_cast<ConstantDataArray>(ngv->getInitializer()) : nullptr;
        count = cast<ConstantInt>(cs->getOperand(2))->getSExtValue();
        std::string name = str ? str->getAsCString().str() : gv->getName().str();
        P.params[name] = gv;
        P.paramCount[name] = (int)count;
        return name;
    }

    void call(CallInst *CI, Node *pred) {
        Function *F = CI->getCalledFunction();
        if (!F)
            fail("indirect call " + where(CI));
        std::string fn = F->getName().str();
        auto arg = [&](unsigned i) { return get(CI->getArgOperand(i)); };
        auto constArg = [&](unsigned i) -> long {
            auto *c = dyn_cast<ConstantInt>(CI->getArgOperand(i));
            if (!c)
                fail(fn + ": argument " + std::to_string(i) + " must be a constant " + where(CI));
            return c->getSExtValue();
        };
        if (fn == "s1_pitch" || fn == "s1_gate" || fn == "s1_trig" || fn == "s1_increment") {
            Node *&n = inputs[fn];
            if (!n) {
                n = mk(fn == "s1_increment" ? NK::Increment : NK::Input, fn, {}, CI);
                n->name = fn.substr(3);
            }
            vmap[CI] = n;
            return;
        }
        if (fn == "s1_param_get") {
            long count = 0;
            std::string name = paramName(CI->getArgOperand(0), count);
            long i = constArg(1);
            if (i < 0 || i >= count)
                fail("s1_param(" + name + ", " + std::to_string(i) + "): index out of the table " + where(CI));
            Node *n = mk(NK::Param, "param", {}, CI);
            n->name = name;
            n->c = (double)i;
            vmap[CI] = n;
            return;
        }
        if (fn == "s1_export") {
            if (S.name != "A")
                fail("s1_export in S1_VOICE_B " + where(CI));
            int i = (int)constArg(0);
            Node *v = arg(1);
            auto it = S.exports.find(i);
            S.exports[i] = pred ? sel(pred, v, it != S.exports.end() ? it->second : cst(0), v) : v;
            return;
        }
        if (fn == "s1_import") {
            if (S.name != "B")
                fail("s1_import in S1_VOICE " + where(CI));
            Node *n = mk(NK::Import, "import", {}, CI);
            n->c = (double)constArg(0);
            vmap[CI] = n;
            return;
        }
        if (fn == "s1_wrap" || fn == "s1_clamp" || fn == "s1_tri" || fn == "s1_exp2") {
            vmap[CI] = mk(NK::Op, fn, {arg(0)}, CI);
            return;
        }
        if (fn == "s1_range") {
            vmap[CI] = mk(NK::Op, fn, {arg(0), arg(1), arg(2)}, CI);
            return;
        }
        if (auto *II = dyn_cast<IntrinsicInst>(CI)) {
            std::string base = fn.substr(5, fn.find('.', 5) - 5);   // llvm.<base>.<type>
            if (II->isAssumeLikeIntrinsic() || base == "lifetime" || base == "dbg" || base == "assume" ||
                base == "experimental")
                return;
            static const std::map<std::string, std::string> ops = {
                {"fmuladd", "fmuladd"}, {"fma", "fmuladd"}, {"fabs", "fabs"}, {"minnum", "fmin"},
                {"maxnum", "fmax"}, {"minimum", "fmin"}, {"maximum", "fmax"}, {"smin", "smin"},
                {"smax", "smax"}, {"umin", "umin"}, {"umax", "umax"}, {"abs", "abs"}, {"floor", "floor"}};
            auto it = ops.find(base);
            if (it != ops.end()) {
                std::vector<Node *> in;
                unsigned n = base == "abs" ? 1 : CI->arg_size();
                for (unsigned i = 0; i < n; i++)
                    in.push_back(arg(i));
                vmap[CI] = mk(NK::Op, it->second, in, CI);
                return;
            }
            if (CI->getType()->isFloatingPointTy()) {           // math intrinsics (sqrt, sin, tanh, pow...)
                std::vector<Node *> in;
                for (unsigned i = 0; i < CI->arg_size(); i++)
                    in.push_back(arg(i));
                Node *n = mk(NK::Op, "call." + base, in, CI);
                n->bad = !knownMath(base);
                vmap[CI] = n;
                return;
            }
            fail("intrinsic " + fn + " is not supported " + where(CI));
        }
        if (kLibm.count(fn)) {
            std::vector<Node *> in;
            for (unsigned i = 0; i < CI->arg_size(); i++)
                in.push_back(arg(i));
            std::string base = fn.back() == 'f' && fn != "erf" ? fn.substr(0, fn.size() - 1) : fn;
            Node *n = mk(NK::Op, "call." + base, in, CI);
            n->bad = !knownMath(base);
            vmap[CI] = n;
            return;
        }
        fail("call to `" + fn + "` " + where(CI) +
             ": voice code must be inlined (static inline / header code) or use the s1_* intrinsics");
    }

    static bool isPow2(long x) { return x > 0 && (x & (x - 1)) == 0; }
    static bool contiguous(unsigned long m) { return m && ((m >> __builtin_ctzl(m)) & ((m >> __builtin_ctzl(m)) + 1)) == 0; }

    void binop(BinaryOperator *B) {
        std::string op = B->getOpcodeName();
        Node *a = get(B->getOperand(0)), *b = get(B->getOperand(1));
        Node *n = mk(NK::Op, op, {a, b}, B);
        auto *cb = dyn_cast<ConstantInt>(B->getOperand(1));
        auto *ca = dyn_cast<ConstantInt>(B->getOperand(0));
        if (op == "udiv" || op == "sdiv" || op == "urem" || op == "srem")
            n->bad = !(cb && isPow2(cb->getSExtValue()));
        else if (op == "frem")
            n->bad = true;                                   // fdiv by a variable: x * 1/y (lower.cpp recip)
        else if (op == "and" || op == "or" || op == "xor") {
            ConstantInt *c = cb ? cb : ca;
            if (!c)
                n->bad = !(n->bits == 1);              // bool logic is fine
            else if (op == "and")
                n->bad = n->bits > 1 && !contiguous(c->getZExtValue() & ((n->bits >= 64) ? ~0UL : ((1UL << n->bits) - 1)));
            else if (op == "xor")
                n->bad = n->bits > 1 && !(c->isMinusOne() || isPow2(c->getSExtValue()));
            else
                n->bad = n->bits > 1;
        }
        vmap[B] = n;
    }

    void run() {
        Function *F = S.fn;
        if (F->arg_size() == 1)
            vmap[F->getArg(0)] = [&] {
                Node *n = mk(NK::XIn, "xin", {}, F->getArg(0));
                return n;
            }();
        ReversePostOrderTraversal<Function *> rpot(F);
        std::map<BasicBlock *, int> order;
        for (BasicBlock *bb : rpot)
            order[bb] = (int)order.size();
        std::map<BasicBlock *, Node *> pred;                       // nullptr = always
        std::map<std::pair<BasicBlock *, BasicBlock *>, Node *> edge;
        std::set<BasicBlock *> always;
        auto edgeOf = [&](BasicBlock *from, BasicBlock *to) -> Node * {
            auto it = edge.find({from, to});
            return it == edge.end() ? nullptr : it->second;
        };
        for (BasicBlock *bb : rpot) {
            Node *p = nullptr;
            if (bb != &F->getEntryBlock()) {
                std::vector<Node *> es;
                bool unconditional = false;
                for (BasicBlock *pb : predecessors(bb)) {
                    if (!order.count(pb))
                        continue;
                    if (order[pb] >= order[bb])
                        fail("loop in voice code (" + F->getName().str() + "): unroll it or give it a constant "
                             "trip count");
                    Node *e = edgeOf(pb, bb);
                    if (!e && !pred[pb])
                        unconditional = true;
                    es.push_back(e ? e : pred[pb]);
                }
                if (!unconditional) {
                    for (Node *e : es)
                        p = p ? boolOp("bor", {p, e}) : e;
                }
            }
            pred[bb] = p;
            for (Instruction &I : *bb) {
                if (auto *phi = dyn_cast<PHINode>(&I)) {
                    Node *v = nullptr;
                    for (int k = (int)phi->getNumIncomingValues() - 1; k >= 0; k--) {
                        BasicBlock *ib = phi->getIncomingBlock(k);
                        Node *e = edgeOf(ib, bb);
                        if (!e)
                            e = pred[ib];
                        Node *iv = get(phi->getIncomingValue(k));
                        v = !v ? iv : sel(e ? e : nullptr, iv, v, iv);
                    }
                    vmap[phi] = v;
                } else if (auto *L = dyn_cast<LoadInst>(&I)) {
                    load(L);
                } else if (auto *St = dyn_cast<StoreInst>(&I)) {
                    store(St, p);
                } else if (auto *CI = dyn_cast<CallInst>(&I)) {
                    call(CI, p);
                } else if (auto *B = dyn_cast<BinaryOperator>(&I)) {
                    binop(B);
                } else if (auto *U = dyn_cast<UnaryOperator>(&I)) {
                    vmap[U] = mk(NK::Op, "fneg", {get(U->getOperand(0))}, U);
                } else if (auto *C = dyn_cast<CastInst>(&I)) {
                    std::string op = C->getOpcodeName();
                    if (op == "bitcast" || op == "ptrtoint" || op == "inttoptr" || op == "addrspacecast")
                        fail("cast " + op + " is not supported " + where(C));
                    vmap[C] = mk(NK::Op, op, {get(C->getOperand(0))}, C);
                } else if (auto *Fz = dyn_cast<FreezeInst>(&I)) {
                    vmap[Fz] = get(Fz->getOperand(0));
                } else if (auto *Cmp = dyn_cast<CmpInst>(&I)) {
                    std::string pn = CmpInst::getPredicateName(Cmp->getPredicate()).str();
                    Node *n = mk(NK::Op, std::string(isa<ICmpInst>(Cmp) ? "icmp." : "fcmp.") + pn,
                                 {get(Cmp->getOperand(0)), get(Cmp->getOperand(1))}, Cmp);
                    n->isInt = true;
                    n->bits = 1;
                    vmap[Cmp] = n;
                } else if (auto *Sel = dyn_cast<SelectInst>(&I)) {
                    vmap[Sel] = mk(NK::Op, "select", {get(Sel->getCondition()), get(Sel->getTrueValue()),
                                                      get(Sel->getFalseValue())}, Sel);
                } else if (isa<AllocaInst>(&I)) {
                    ptrOf(&I);
                } else if (isa<GetElementPtrInst>(&I)) {
                    ptrOf(&I);
                } else if (auto *R = dyn_cast<ReturnInst>(&I)) {
                    if (R->getNumOperands()) {
                        Node *v = get(R->getOperand(0));
                        S.ret = (p && S.ret) ? sel(p, v, S.ret, v) : v;
                    }
                } else if (auto *Br = dyn_cast<BranchInst>(&I)) {
                    if (Br->isConditional()) {
                        Node *c = get(Br->getCondition());
                        BasicBlock *t = Br->getSuccessor(0), *f = Br->getSuccessor(1);
                        if (t != f) {
                            edge[{bb, t}] = band(p, c);
                            edge[{bb, f}] = band(p, bnot(c));
                        }
                    }
                } else if (isa<UnreachableInst>(&I)) {
                } else {
                    fail(std::string("instruction `") + I.getOpcodeName() + "` is not supported " + where(&I));
                }
            }
        }
        for (auto &kv : cur)
            P.states[kv.first].final = kv.second;
    }
};

std::map<const Node *, bool> constMemo;

bool isConstExpr(const Node *n) {
    if (n->kind == NK::Const)
        return true;
    if (n->kind != NK::Op || n->bad || n->in.empty())
        return false;
    auto it = constMemo.find(n);
    if (it != constMemo.end())
        return it->second;
    bool r = true;
    for (Node *i : n->in)
        if (!isConstExpr(i)) {
            r = false;
            break;
        }
    constMemo[n] = r;
    return r;
}

void markLive(Program &P, Stage &S) {
    std::vector<Node *> stack;
    if (S.ret)
        stack.push_back(S.ret);
    for (auto &kv : S.exports)
        stack.push_back(kv.second);
    for (int s : S.states)
        if (P.states[s].final)
            stack.push_back(P.states[s].final);
    while (!stack.empty()) {
        Node *n = stack.back();
        stack.pop_back();
        if (n->live)
            continue;
        n->live = true;
        for (Node *i : n->in)
            stack.push_back(i);
    }
    for (auto &n : S.nodes)
        n->users.clear();
    for (auto &n : S.nodes)
        if (n->live)
            for (Node *i : n->in)
                i->users.push_back(n.get());
}

// integer interpretation: unsigned when an operation reads or makes it as unsigned (to a fixed point)
void signedness(Program &P, Stage &S) {
    static const std::set<std::string> readsU = {"lshr", "zext", "uitofp", "udiv", "urem", "umin", "umax",
                                                 "icmp.ugt", "icmp.uge", "icmp.ult", "icmp.ule"};
    static const std::set<std::string> makesU = {"lshr", "zext", "fptoui", "udiv", "urem", "umin", "umax"};
    bool changed = true;
    while (changed) {
        changed = false;
        auto setU = [&](Node *n) {
            if (n->isInt && n->bits > 1 && !n->isUnsigned) {
                n->isUnsigned = true;
                changed = true;
            }
        };
        for (auto &np : S.nodes) {
            Node *n = np.get();
            if (!n->live)
                continue;
            if (n->kind == NK::Op && makesU.count(n->op))
                setU(n);
            if (n->kind == NK::Op && readsU.count(n->op))
                for (Node *i : n->in)
                    if (i->kind != NK::Const && !(n->op == "lshr" && i == n->in[1]))
                        setU(i);
            if (n->kind == NK::State && P.states[n->state].isUnsigned)
                setU(n);
            // an unsigned result of a bit-preserving operation reads its operands as unsigned too (the profile
            // then decides the view per value: lower.cpp unsignedView)
            if (n->isUnsigned && n->kind == NK::Op) {
                static const std::set<std::string> through = {"select", "shl", "add", "sub", "and", "or", "xor",
                                                              "trunc", "mul"};
                if (through.count(n->op))
                    for (size_t k = n->op == "select" ? 1 : 0; k < n->in.size(); k++)
                        if (n->in[k]->kind != NK::Const && !(n->op == "shl" && k == 1))
                            setU(n->in[k]);
            }
        }
        for (int s : S.states) {
            StateVar &sv = P.states[s];
            if (sv.final && sv.final->isUnsigned && !sv.isUnsigned) {
                sv.isUnsigned = true;
                changed = true;
            }
        }
    }
}

// islands: for every bad node, the single values that cut all its paths to the sources (dominators of the
// dependency DAG), highest first; members = the nodes that depend on the root only
void islands(Stage &S) {
    std::map<Node *, Node *> idom;
    std::map<Node *, int> depth;
    Node *VR = nullptr;                                  // virtual root
    auto lca = [&](Node *a, Node *b) {
        while (a != b) {
            if (depth[a] < depth[b])
                std::swap(a, b);
            a = idom[a];
        }
        return a;
    };
    for (auto &np : S.nodes) {
        Node *n = np.get();
        if (!n->live || isConstExpr(n))
            continue;
        Node *d = VR;
        bool first = true;
        if (n->kind == NK::Op)
            for (Node *i : n->in) {
                if (isConstExpr(i))
                    continue;
                d = first ? i : lca(d, i);
                first = false;
            }
        idom[n] = d;
        depth[n] = d ? depth[d] + 1 : 1;
    }
    std::set<std::pair<Node *, Node *>> made;
    for (auto &np : S.nodes) {
        Node *N = np.get();
        if (!N->live || !N->bad)
            continue;
        std::vector<Node *> chain;
        for (Node *r = idom[N]; r; r = idom[r])
            chain.push_back(r);
        if (chain.empty())
            fail("a table read / function / division that depends on several values " + where(N->v) +
                 ": only functions of one value can be tabulated");
        // candidates: the three highest roots and the immediate one (the rest only cost memory and time)
        std::vector<Node *> cands;
        for (auto it = chain.rbegin(); it != chain.rend() && cands.size() < 3; ++it)
            cands.push_back(*it);
        if (std::find(cands.begin(), cands.end(), chain.front()) == cands.end())
            cands.push_back(chain.front());
        for (Node *R : cands) {
            std::set<Node *> pure;
            std::vector<Node *> members;
            for (auto &mp : S.nodes) {
                Node *x = mp.get();
                if (!x->live || x->id <= R->id || isConstExpr(x) || x->kind != NK::Op)
                    continue;
                bool dep = false, ok = true;
                for (Node *i : x->in) {
                    if (isConstExpr(i))
                        continue;
                    if (i == R || pure.count(i))
                        dep = true;
                    else
                        ok = false;
                }
                if (ok && dep) {
                    pure.insert(x);
                    members.push_back(x);
                }
            }
            if (!pure.count(N))
                continue;
            // outputs: members used outside the island (or stage outputs)
            for (Node *x : members) {
                bool outside = x == S.ret;
                for (auto &kv : S.exports)
                    outside |= kv.second == x;
                for (Node *u : x->users)
                    outside |= !pure.count(u);
                if (!outside)
                    continue;
                if (made.count({R, x}))
                    continue;
                made.insert({R, x});
                Island isl;
                isl.id = 0;
                isl.root = R;
                isl.out = x;
                std::set<Node *> cone;
                std::function<void(Node *)> back = [&](Node *y) {
                    if (cone.count(y) || !pure.count(y))
                        return;
                    cone.insert(y);
                    for (Node *i : y->in)
                        back(i);
                };
                back(x);
                for (Node *y : members)
                    if (cone.count(y)) {
                        isl.members.push_back(y);
                        if (!y->v || !isa<Instruction>(y->v) || isa<PHINode>(y->v))
                            isl.cloneable = false;
                    }
                if (!R->v)
                    isl.cloneable = false;
                S.islands.push_back(isl);
            }
        }
    }
}

// values computed only for one arm of a select (speculated by clang): the cone of each arm whose every user is
// inside the cone (or is the select, through that arm)
void speculation(Stage &S) {
    for (auto it = S.nodes.rbegin(); it != S.nodes.rend(); ++it) {
        Node *sel = it->get();
        if (!sel->live || sel->op != "select" || !sel->v || !isa<SelectInst>(sel->v))
            continue;
        for (int arm = 1; arm <= 2; arm++) {
            Node *A = sel->in[arm];
            if (A->kind != NK::Op || A->specSel || isConstExpr(A))
                continue;
            std::set<Node *> cone;
            bool onlyHere = true;
            for (Node *u : A->users)
                onlyHere = onlyHere && u == sel && sel->in[3 - arm] != A && sel->in[0] != A;
            if (!onlyHere)
                continue;
            cone.insert(A);
            for (bool grew = true; grew;) {
                grew = false;
                for (Node *x : std::vector<Node *>(cone.begin(), cone.end()))
                    for (Node *i : x->in) {
                        if (cone.count(i) || i->kind != NK::Op || isConstExpr(i) || i->specSel || !i->v)
                            continue;
                        bool inside = true;
                        for (Node *u : i->users)
                            inside = inside && cone.count(u);
                        if (inside) {
                            cone.insert(i);
                            grew = true;
                        }
                    }
            }
            for (Node *x : cone) {
                x->specSel = sel;
                x->specArmTrue = arm == 1;
            }
            for (Node *x : cone) {
                bool frontier = true;
                for (Node *i : x->in)
                    frontier = frontier && !cone.count(i);
                x->specFrontier = frontier;
            }
        }
    }
}

}  // namespace

void build(Program &P, Module &M) {
    P.M = &M;
    P.A.name = "A";
    P.B.name = "B";
    P.A.fn = M.getFunction("s1_voice");
    P.B.fn = M.getFunction("s1_voice_b");
    if (!P.A.fn || P.A.fn->isDeclaration())
        fail("no S1_VOICE (s1_voice) in the module");
    if (P.B.fn && P.B.fn->isDeclaration())
        P.B.fn = nullptr;
    for (Stage *S : {&P.A, &P.B}) {
        if (!S->fn)
            continue;
        Builder b(P, *S);
        b.run();
        if (getenv("S1CC_DEBUG"))
            fprintf(stderr, "graph %s: %zu nodes\n", S->name.c_str(), S->nodes.size());
        if (!S->ret)
            fail(S->fn->getName().str() + " returns nothing");
        markLive(P, *S);
    }
    for (int k = 0; k < 4; k++)
        for (Stage *S : {&P.A, &P.B})
            if (S->fn)
                signedness(P, *S);
    int id = 0;
    for (Stage *S : {&P.A, &P.B}) {
        if (!S->fn)
            continue;
        speculation(*S);
        islands(*S);
        for (auto &isl : S->islands)
            isl.id = id++;
    }
}

}  // namespace s1
