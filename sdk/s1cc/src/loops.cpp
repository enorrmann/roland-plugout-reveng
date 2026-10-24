// Loops whose trip count depends on the data (docs/S1CC.md §4.5): the DSP runs straight-line code, so a loop is
// peeled as many times as the profiling run saw it turn, and what is left of it is dropped.
//   s1cc trips  IN.ll -o OUT.ll      a call __s1p_trip(loop id, header count) in every loop header of the voice
//                                    functions (runtime/s1rt.c records the maximum per invocation)
//   s1cc unroll IN.ll --trips P -o OUT.ll   every loop peeled max-trip times, innermost first, then cleaned up
// Loop ids: the preorder of LoopInfo in s1_voice, then s1_voice_b, on the module as clang left it.
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/LoopPeel.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#include "s1cc.h"

using namespace llvm;

namespace s1 {

namespace {

std::vector<Function *> voiceFunctions(Module &M) {
    std::vector<Function *> fs;
    for (const char *n : {"s1_voice", "s1_voice_b"})
        if (Function *F = M.getFunction(n))
            if (!F->isDeclaration())
                fs.push_back(F);
    return fs;
}

void runPipeline(Function &F, const char *pipeline);

void cleanup(Function &F) { runPipeline(F, "simplifycfg,sroa,early-cse,instcombine,simplifycfg,adce"); }

// canonical loops (one preheader, one latch): the same in `trips` and `unroll`, so the loop ids agree
void simplifyLoops(Module &M) {
    for (Function *F : voiceFunctions(M))
        runPipeline(*F, "loop-simplify");
}

void runPipeline(Function &F, const char *pipeline) {
    PassBuilder PB;
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);
    FunctionPassManager FPM;
    if (auto err = PB.parsePassPipeline(FPM, pipeline))
        fail("pass pipeline: " + toString(std::move(err)));
    FPM.run(F, FAM);
}

}  // namespace

int instrumentTrips(Module &M) {
    LLVMContext &C = M.getContext();
    Type *i32 = Type::getInt32Ty(C);
    FunctionCallee rec = M.getOrInsertFunction("__s1p_trip", Type::getVoidTy(C), i32, i32);
    simplifyLoops(M);
    int id = 0;
    for (Function *F : voiceFunctions(M)) {
        DominatorTree DT(*F);
        LoopInfo LI(DT);
        for (Loop *L : LI.getLoopsInPreorder()) {
            BasicBlock *pre = L->getLoopPreheader();
            if (!pre)
                fail("a loop of " + F->getName().str() + " without a preheader");
            auto *cnt = new GlobalVariable(M, i32, false, GlobalValue::InternalLinkage, ConstantInt::get(i32, 0),
                                           "__s1t_c" + std::to_string(id));
            IRBuilder<> b(pre->getTerminator());
            b.CreateStore(ConstantInt::get(i32, 0), cnt);
            IRBuilder<> h(&*L->getHeader()->getFirstInsertionPt());
            Value *n = h.CreateAdd(h.CreateLoad(i32, cnt), ConstantInt::get(i32, 1));
            h.CreateStore(n, cnt);
            h.CreateCall(rec, {ConstantInt::get(i32, id), n});
            id++;
        }
    }
    return id;
}

void unrollBounded(Module &M, const std::map<int, int> &trips, std::string &report) {
    LLVMContext &C = M.getContext();
    simplifyLoops(M);
    // tag every loop with its profiled maximum (clones made by peeling an outer loop keep the tag)
    int id = 0;
    for (Function *F : voiceFunctions(M)) {
        DominatorTree DT(*F);
        LoopInfo LI(DT);
        for (Loop *L : LI.getLoopsInPreorder()) {
            auto it = trips.find(id);
            int n = it == trips.end() ? -1 : it->second;
            L->getHeader()->getTerminator()->setMetadata(
                "s1.trip", MDNode::get(C, {ConstantAsMetadata::get(ConstantInt::get(Type::getInt32Ty(C), n))}));
            report += "  loop " + std::to_string(id) + " in " + F->getName().str() + ": up to " +
                      std::to_string(n) + " passes per sample (profiled)\n";
            id++;
        }
    }
    for (Function *F : voiceFunctions(M)) {
        for (int guard = 0; guard < 1000; guard++) {
            DominatorTree DT(*F);
            LoopInfo LI(DT);
            Loop *inner = nullptr;
            for (Loop *L : LI.getLoopsInPreorder())
                if (L->getSubLoops().empty()) {
                    inner = L;
                    break;
                }
            if (!inner)
                break;
            int n = -1;
            if (MDNode *md = inner->getHeader()->getTerminator()->getMetadata("s1.trip"))
                n = (int)mdconst::extract<ConstantInt>(md->getOperand(0))->getSExtValue();
            if (n < 0)
                fail("a loop in " + F->getName().str() + " never ran in the profiling scenarios: its trip count is "
                     "unknown");
            if (n > 64)
                fail("a loop in " + F->getName().str() + " turns up to " + std::to_string(n) +
                     " times per sample: too long for straight-line DSP code");
            if (n > 0) {
                TargetLibraryInfoImpl TLII(Triple(M.getTargetTriple()));
                TargetLibraryInfo TLI(TLII);
                AssumptionCache AC(*F);
                ScalarEvolution SE(*F, TLI, AC, DT, LI);
                ValueToValueMapTy VMap;
                if (!peelLoop(inner, (unsigned)n, &LI, &SE, DT, &AC, false, VMap))
                    fail("cannot peel a loop of " + F->getName().str());
            }
            // the rest of the loop never runs (the profile's maximum): the header becomes unreachable
            changeToUnreachable(&*inner->getHeader()->getFirstNonPHIIt());
            cleanup(*F);
        }
        cleanup(*F);
    }
}

}  // namespace s1
