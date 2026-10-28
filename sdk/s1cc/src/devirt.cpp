// Indirect calls whose possible targets are a finite set of known functions (docs/S1CC.md §4.7): a member-function
// table indexed by a mode that a branch picked (Braids' MacroOscillator::RenderMorph: fn_table_[shape_] with shape_
// one of two constants), a function pointer chosen by a select. The voice must be one function without calls, so
// each such call becomes `if (callee == F1) F1(...) else if ... else Fn(...)` (LLVM's versionCallSite); the driver
// runs O2 again, which folds the compares and inlines the direct calls.
//   s1cc devirt IN.ll -o OUT.ll     prints the number of calls it rewrote
// Targets: the callee traced through phi / select / inttoptr / ptrtoint / casts and loads from constant tables at
// indices that are themselves a finite set of constants (phi / select of constants). The arm of a member-function
// pointer that loads from a vtable (taken only when the pointer's low bit is set) is ignored when the others give
// functions: functions are aligned (-falign-functions=16), so the non-virtual arms are the only ones taken.
#include <set>

#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Transforms/Utils/CallPromotionUtils.h"

#include "s1cc.h"

using namespace llvm;

namespace s1 {

namespace {

const int kMaxTargets = 64;

// the finite set of integer constants v can be, or false
bool constSet(Value *v, std::set<int64_t> &out, int depth = 0) {
    if (depth > 16 || out.size() > (size_t)kMaxTargets)
        return false;
    if (auto *ci = dyn_cast<ConstantInt>(v)) {
        out.insert(ci->getSExtValue());
        return true;
    }
    if (auto *ph = dyn_cast<PHINode>(v)) {
        for (Value *in : ph->incoming_values())
            if (in != ph && !constSet(in, out, depth + 1))
                return false;
        return true;
    }
    if (auto *sl = dyn_cast<SelectInst>(v))
        return constSet(sl->getTrueValue(), out, depth + 1) && constSet(sl->getFalseValue(), out, depth + 1);
    if (auto *c = dyn_cast<CastInst>(v)) {
        std::set<int64_t> in;
        if (!constSet(c->getOperand(0), in, depth + 1))
            return false;
        for (int64_t x : in) {
            Constant *k = ConstantInt::get(c->getOperand(0)->getType(), x, true);
            Constant *r = ConstantFoldCastOperand(c->getOpcode(), k, c->getType(), c->getModule()->getDataLayout());
            auto *ri = dyn_cast_or_null<ConstantInt>(r);
            if (!ri)
                return false;
            out.insert(ri->getSExtValue());
        }
        return true;
    }
    return false;
}

// the functions v (a pointer, or an integer holding one) can be; unknown = something else may reach it
void targets(Value *v, std::set<Function *> &out, bool &unknown, int depth = 0) {
    if (depth > 24 || out.size() > (size_t)kMaxTargets) {
        unknown = true;
        return;
    }
    v = v->stripPointerCasts();
    if (auto *F = dyn_cast<Function>(v)) {
        out.insert(F);
        return;
    }
    if (auto *ce = dyn_cast<ConstantExpr>(v)) {
        if (ce->getOpcode() == Instruction::PtrToInt || ce->getOpcode() == Instruction::IntToPtr ||
            ce->getOpcode() == Instruction::BitCast) {
            targets(ce->getOperand(0), out, unknown, depth + 1);
            return;
        }
    }
    if (auto *c = dyn_cast<CastInst>(v)) {
        if (c->getOpcode() == Instruction::IntToPtr || c->getOpcode() == Instruction::PtrToInt) {
            targets(c->getOperand(0), out, unknown, depth + 1);
            return;
        }
    }
    if (auto *ph = dyn_cast<PHINode>(v)) {
        std::set<Function *> got;
        int lost = 0;
        for (Value *in : ph->incoming_values()) {
            bool u = false;
            std::set<Function *> one;
            targets(in, one, u, depth + 1);
            // a vtable load (load of a load): the virtual arm of a member-function pointer
            auto *L = dyn_cast<LoadInst>(in->stripPointerCasts());
            bool vtable = L && !isa<GlobalVariable>(getUnderlyingObject(L->getPointerOperand()));
            if (u && vtable && one.empty()) {
                lost++;
                continue;
            }
            unknown |= u;
            got.insert(one.begin(), one.end());
        }
        if (got.empty() && lost)
            unknown = true;
        out.insert(got.begin(), got.end());
        return;
    }
    if (auto *sl = dyn_cast<SelectInst>(v)) {
        targets(sl->getTrueValue(), out, unknown, depth + 1);
        targets(sl->getFalseValue(), out, unknown, depth + 1);
        return;
    }
    if (auto *L = dyn_cast<LoadInst>(v)) {
        // a constant table at a finite set of indices
        auto *gep = dyn_cast<GEPOperator>(L->getPointerOperand());
        auto *gv = gep ? dyn_cast<GlobalVariable>(gep->getPointerOperand()->stripPointerCasts()) : nullptr;
        if (gv && gv->isConstant() && gv->hasDefinitiveInitializer()) {
            const DataLayout &DL = L->getModule()->getDataLayout();
            MapVector<Value *, APInt> vars;
            APInt coff(64, 0);
            std::set<int64_t> idx;
            if (gep->collectOffset(DL, 64, vars, coff) && vars.size() == 1 && constSet(vars.front().first, idx)) {
                for (int64_t i : idx) {
                    int64_t off = coff.getSExtValue() + i * vars.front().second.getSExtValue();
                    Constant *c = ConstantFoldLoadFromConst(gv->getInitializer(), L->getType(), APInt(64, off), DL);
                    if (!c) {
                        unknown = true;
                        return;
                    }
                    targets(c, out, unknown, depth + 1);
                }
                return;
            }
        }
    }
    unknown = true;
}

// the callee rebuilt from function constants, mirroring targets(): phi -> phi, select -> select, a constant table at
// a finite set of indices -> a select chain on the index. Compares of it against a function then fold (O2).
struct Rebuild {
    PointerType *pt;
    std::map<Value *, Value *> memo;
    Value *any = nullptr;            // for the vtable arm of a member pointer: never taken, any function will do

    Value *run(Value *v, int depth = 0) {
        auto m = memo.find(v);
        if (m != memo.end())
            return m->second;
        Value *r = make(v, depth);
        memo[v] = r;
        return r;
    }
    Value *make(Value *v, int depth) {
        Value *s = v->stripPointerCasts();
        if (auto *F = dyn_cast<Function>(s))
            return any = F;
        if (auto *ce = dyn_cast<ConstantExpr>(s))
            if (ce->getOpcode() == Instruction::PtrToInt || ce->getOpcode() == Instruction::IntToPtr ||
                ce->getOpcode() == Instruction::BitCast)
                return run(ce->getOperand(0), depth + 1);
        if (auto *c = dyn_cast<CastInst>(s))
            if (c->getOpcode() == Instruction::IntToPtr || c->getOpcode() == Instruction::PtrToInt)
                return run(c->getOperand(0), depth + 1);
        if (auto *ph = dyn_cast<PHINode>(s)) {
            PHINode *np = PHINode::Create(pt, ph->getNumIncomingValues(), "s1.callee", ph->getParent()->getFirstNonPHI());
            memo[v] = np;
            std::vector<std::pair<int, Value *>> got;
            for (unsigned k = 0; k < ph->getNumIncomingValues(); k++) {
                bool u = false;
                std::set<Function *> one;
                targets(ph->getIncomingValue(k), one, u);
                got.push_back({k, one.empty() ? nullptr : run(ph->getIncomingValue(k), depth + 1)});
            }
            for (auto &g : got)
                np->addIncoming(g.second ? g.second : any, ph->getIncomingBlock(g.first));
            return np;
        }
        if (auto *sl = dyn_cast<SelectInst>(s)) {
            Value *t = run(sl->getTrueValue(), depth + 1), *f = run(sl->getFalseValue(), depth + 1);
            return SelectInst::Create(sl->getCondition(), t, f, "s1.callee", sl->getNextNode());
        }
        if (auto *L = dyn_cast<LoadInst>(s)) {
            auto *gep = cast<GEPOperator>(L->getPointerOperand());
            auto *gv = cast<GlobalVariable>(gep->getPointerOperand()->stripPointerCasts());
            const DataLayout &DL = L->getModule()->getDataLayout();
            MapVector<Value *, APInt> vars;
            APInt coff(64, 0);
            std::set<int64_t> idx;
            gep->collectOffset(DL, 64, vars, coff);
            constSet(vars.front().first, idx);
            Value *iv = vars.front().first;
            IRBuilder<> b(L->getNextNode());
            Value *acc = nullptr;
            for (int64_t i : idx) {
                int64_t off = coff.getSExtValue() + i * vars.front().second.getSExtValue();
                Constant *c = ConstantFoldLoadFromConst(gv->getInitializer(), L->getType(), APInt(64, off), DL);
                Value *f = run(c, depth + 1);
                acc = !acc ? f : b.CreateSelect(b.CreateICmpEQ(iv, ConstantInt::get(iv->getType(), i, true)), f, acc,
                                               "s1.callee");
            }
            return acc;
        }
        return any;
    }
};

}  // namespace

int devirtualize(Module &M) {
    int n = 0;
    for (const char *name : {"s1_dsp_a", "s1_dsp_b"}) {
        Function *F = M.getFunction(name);
        if (!F || F->isDeclaration())
            continue;
        std::vector<CallBase *> calls;
        for (BasicBlock &bb : *F)
            for (Instruction &I : bb)
                if (auto *CB = dyn_cast<CallBase>(&I))
                    if (!CB->getCalledFunction() && !CB->isInlineAsm())
                        calls.push_back(CB);
        for (CallBase *CB : calls) {
            std::set<Function *> fs;
            bool unknown = false;
            targets(CB->getCalledOperand(), fs, unknown);
            if (unknown || fs.empty())
                continue;
            std::vector<Function *> list(fs.begin(), fs.end());
            Rebuild rb{cast<PointerType>(CB->getCalledOperand()->getType())};
            rb.any = list.front();
            CB->setCalledOperand(rb.run(CB->getCalledOperand()));
            for (size_t k = 0; k + 1 < list.size(); k++)
                versionCallSite(*CB, list[k], nullptr);      // CB stays as the else branch
            promoteCall(*CB, list.back());                   // the last one: no other target is possible
            n++;
        }
    }
    return n;
}

}  // namespace s1
