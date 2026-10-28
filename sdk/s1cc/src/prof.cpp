// `s1cc prof`: the module with profiling calls (docs/S1CC.md §5). After every value of the voice functions:
//   __s1p_v(id, signed view, unsigned view)                       range
//   __s1p_o(id, wrapped as signed, wrapped as unsigned)           integer add / sub / mul / shl overflow
// plus, per island candidate, `double __s1p_isl_<k>(double x)`: a clone of the island with x as its root, listed
// in __s1p_isl_tab[] = { island id, root record id, function }, and __s1p_reset(): the state globals back to their
// initial values (runtime/s1rt.c calls it before every scenario).
#include <functional>

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#include "s1cc.h"

using namespace llvm;

namespace s1 {

namespace {

void recordAfter(IRBuilder<> &b, Module &M, int id, Value *v) {
    LLVMContext &C = M.getContext();
    Type *d = Type::getDoubleTy(C), *i32 = Type::getInt32Ty(C);
    FunctionCallee rec = M.getOrInsertFunction("__s1p_v", Type::getVoidTy(C), i32, d, d);
    Value *s, *u;
    if (v->getType()->isIntegerTy()) {
        s = b.CreateSIToFP(v, d);
        u = b.CreateUIToFP(v, d);
    } else {
        s = u = b.CreateFPExt(v, d);
    }
    b.CreateCall(rec, {ConstantInt::get(i32, id), s, u});
}

void overflowAfter(IRBuilder<> &b, Module &M, int id, BinaryOperator *B) {
    LLVMContext &C = M.getContext();
    Type *i32 = Type::getInt32Ty(C), *i64 = Type::getInt64Ty(C), *i128 = Type::getInt128Ty(C);
    unsigned w = B->getType()->getIntegerBitWidth();
    Type *wide = w <= 32 ? i64 : i128;
    FunctionCallee rec = M.getOrInsertFunction("__s1p_o", Type::getVoidTy(C), i32, i32, i32);
    auto exact = [&](bool sgn) {
        Value *a = sgn ? b.CreateSExt(B->getOperand(0), wide) : b.CreateZExt(B->getOperand(0), wide);
        Value *c = sgn ? b.CreateSExt(B->getOperand(1), wide) : b.CreateZExt(B->getOperand(1), wide);
        Value *e = b.CreateBinOp(B->getOpcode(), a, c);
        Value *r = sgn ? b.CreateSExt(B, wide) : b.CreateZExt(B, wide);
        return b.CreateZExt(b.CreateICmpNE(e, r), i32);
    };
    b.CreateCall(rec, {ConstantInt::get(i32, id), exact(true), exact(false)});
}

}  // namespace

void instrument(Program &P) {
    Module &M = *P.M;
    LLVMContext &C = M.getContext();
    Type *d = Type::getDoubleTy(C), *i32 = Type::getInt32Ty(C);
    int id = 0;
    std::map<Value *, int> recId;
    for (Stage *S : {&P.A, &P.B}) {
        if (!S->fn)
            continue;
        for (auto &np : S->nodes) {
            Node *n = np.get();
            if (!n->v || n->kind == NK::Const)
                continue;
            Type *t = n->v->getType();
            if (!t->isIntegerTy() && !t->isFloatingPointTy())
                continue;
            auto it = recId.find(n->v);
            if (it != recId.end()) {
                n->prof = it->second;
                continue;
            }
            n->prof = recId[n->v] = id++;
        }
    }
    // insert the calls (values first, so the ids stay those of the graph)
    for (auto &kv : recId) {
        Value *v = kv.first;
        if (auto *A = dyn_cast<Argument>(v)) {
            IRBuilder<> b(&*A->getParent()->getEntryBlock().getFirstInsertionPt());
            recordAfter(b, M, kv.second, A);
            continue;
        }
        auto *I = cast<Instruction>(v);
        IRBuilder<> b(I->getParent(), isa<PHINode>(I) ? I->getParent()->getFirstInsertionPt()
                                                       : std::next(I->getIterator()));
        recordAfter(b, M, kv.second, I);
        if (auto *B = dyn_cast<BinaryOperator>(I)) {
            auto op = B->getOpcode();
            if (B->getType()->isIntegerTy() && B->getType()->getIntegerBitWidth() > 1 &&
                (op == Instruction::Add || op == Instruction::Sub || op == Instruction::Mul || op == Instruction::Shl))
                overflowAfter(b, M, kv.second, B);
        }
    }
    // divisors: a histogram of sign and octave (__s1p_h), so the reciprocal knows its usual range
    {
        FunctionCallee rech = M.getOrInsertFunction("__s1p_h", Type::getVoidTy(C), i32, d);
        for (Stage *S : {&P.A, &P.B}) {
            if (!S->fn)
                continue;
            for (auto &np : S->nodes) {
                Node *n = np.get();
                if (n->op != "fdiv" || !n->v || n->in.size() < 2 || n->in[1]->prof < 0 || !n->in[1]->v)
                    continue;
                auto *I = cast<Instruction>(n->v);
                IRBuilder<> b(I);
                Value *y = cast<Instruction>(n->v)->getOperand(1);
                b.CreateCall(rech, {ConstantInt::get(i32, n->in[1]->prof), b.CreateFPExt(y, d)});
            }
        }
    }
    // speculated values: also recorded at their select, when the arm they feed is taken
    {
        FunctionCallee recc = M.getOrInsertFunction("__s1p_c", Type::getVoidTy(C), i32, i32, d, d);
        for (Stage *S : {&P.A, &P.B}) {
            if (!S->fn)
                continue;
            for (auto &np : S->nodes) {
                Node *n = np.get();
                if (!n->specSel || n->prof < 0 || !n->v)
                    continue;
                auto *SI = cast<SelectInst>(n->specSel->v);
                IRBuilder<> b(SI);
                Value *c = SI->getCondition();
                if (!n->specArmTrue)
                    c = b.CreateNot(c);
                Value *s, *u;
                if (n->v->getType()->isIntegerTy()) {
                    s = b.CreateSIToFP(n->v, d);
                    u = b.CreateUIToFP(n->v, d);
                } else
                    s = u = b.CreateFPExt(n->v, d);
                b.CreateCall(recc, {ConstantInt::get(i32, n->prof), b.CreateZExt(c, i32), s, u});
            }
        }
    }
    // islands: one native function per candidate
    std::vector<Constant *> rows;
    StructType *rowTy = StructType::get(C, {i32, i32, PointerType::getUnqual(C), i32});   // + root is an integer
    for (Stage *S : {&P.A, &P.B}) {
        if (!S->fn)
            continue;
        for (Island &isl : S->islands) {
            if (!isl.cloneable || isl.root->prof < 0)
                continue;
            Function *F = Function::Create(FunctionType::get(d, {d}, false), GlobalValue::ExternalLinkage,
                                           "__s1p_isl_" + std::to_string(isl.id), M);
            BasicBlock *bb = BasicBlock::Create(C, "entry", F);
            IRBuilder<> b(bb);
            Value *x = F->getArg(0);
            Type *rt = isl.root->v->getType();
            // integers through int64: the runtime samples the root in its signed or unsigned view
            Value *root = rt->isIntegerTy()
                              ? b.CreateTrunc(b.CreateFPToSI(x, Type::getInt64Ty(C)), rt)
                              : b.CreateFPTrunc(x, rt);
            std::map<Value *, Value *> vm;
            vm[isl.root->v] = root;
            std::set<Value *> members;
            for (Node *m : isl.members)
                members.insert(m->v);
            bool ok = true;
            std::function<Value *(Value *)> remap = [&](Value *v) -> Value * {
                auto it = vm.find(v);
                if (it != vm.end())
                    return it->second;
                if (isa<Constant>(v))
                    return v;
                auto *I = dyn_cast<Instruction>(v);
                if (!I || isa<PHINode>(I) || I->mayHaveSideEffects()) {
                    ok = false;
                    return UndefValue::get(v->getType());
                }
                // a constant-only instruction outside the island (zext of a folded table entry...): clone it too
                Instruction *c = I->clone();
                for (unsigned k = 0; k < c->getNumOperands(); k++)
                    c->setOperand(k, remap(I->getOperand(k)));
                b.Insert(c);
                vm[v] = c;
                return c;
            };
            for (Node *m : isl.members)
                remap(m->v);
            if (!ok) {
                F->eraseFromParent();
                isl.cloneable = false;
                continue;
            }
            Value *out = vm[isl.out->v];
            Value *r = out->getType()->isIntegerTy()
                           ? (isl.out->isUnsigned ? b.CreateUIToFP(out, d) : b.CreateSIToFP(out, d))
                           : b.CreateFPExt(out, d);
            b.CreateRet(r);
            rows.push_back(ConstantStruct::get(rowTy, {ConstantInt::get(i32, isl.id),
                                                       ConstantInt::get(i32, isl.root->prof), F,
                                                       ConstantInt::get(i32, isl.root->isInt ? 1 : 0)}));
        }
    }
    ArrayType *at = ArrayType::get(rowTy, rows.size());
    new GlobalVariable(M, at, true, GlobalValue::ExternalLinkage, ConstantArray::get(at, rows), "__s1p_isl_tab");
    new GlobalVariable(M, i32, true, GlobalValue::ExternalLinkage, ConstantInt::get(i32, rows.size()), "__s1p_isl_n");
    // __s1p_state(double *out): every state word, as a double (the runtime writes them after s1_init)
    {
        Function *D = Function::Create(FunctionType::get(Type::getVoidTy(C), {PointerType::getUnqual(C)}, false),
                                       GlobalValue::ExternalLinkage, "__s1p_state", M);
        IRBuilder<> b(BasicBlock::Create(C, "entry", D));
        for (auto &s : P.states) {
            Type *t = s.isInt ? (Type *)Type::getIntNTy(C, s.bits) : (s.bits == 64 ? Type::getDoubleTy(C) : Type::getFloatTy(C));
            Value *ptr = b.CreateConstGEP1_64(Type::getInt8Ty(C), s.gv, s.offset);
            Value *v = b.CreateLoad(t, ptr);
            Value *dv = s.isInt ? (s.isUnsigned ? b.CreateUIToFP(v, d) : b.CreateSIToFP(v, d)) : b.CreateFPExt(v, d);
            b.CreateStore(dv, b.CreateConstGEP1_64(d, D->getArg(0), s.index));
        }
        b.CreateRetVoid();
        new GlobalVariable(M, i32, true, GlobalValue::ExternalLinkage, ConstantInt::get(i32, P.states.size()),
                           "__s1p_nstate");
    }
    // __s1p_reset: every state global back to its initializer
    Function *R = Function::Create(FunctionType::get(Type::getVoidTy(C), false), GlobalValue::ExternalLinkage,
                                   "__s1p_reset", M);
    IRBuilder<> b(BasicBlock::Create(C, "entry", R));
    std::set<GlobalVariable *> done;
    for (auto &s : P.states)
        if (done.insert(s.gv).second && s.gv->hasInitializer())
            b.CreateStore(s.gv->getInitializer(), s.gv);
    b.CreateRetVoid();
}

}  // namespace s1
