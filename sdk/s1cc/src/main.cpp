// s1cc command line (docs/S1CC.md §9). Normally run by the driver sdk/s1cc/s1cc, not by hand:
//   s1cc prof  IN.ll -o OUT.ll                         profiling module (link with runtime/s1rt.c)
//   s1cc lower IN.ll --profile P --tables T -o K.s1k [--report R] [--regs 0,1,2,3,4,7] [--tol 3e-3] [--tollog 0.005] [--maxdeg 11]
//   s1cc init  IN.ll --kernel K.s1k [--into ORIG.ll] -o OUT.ll   reference module (ORIG, default IN) with
//                                                      __s1p_dspstart(): the state words set to what the DSP starts
//                                                      from (ring = 0 -> V = -o s; S1_INIT's values with .initflag)
//   s1cc trips IN.ll -o OUT.ll                         loop-count profiling module (prints the number of loops)
//   s1cc unroll IN.ll --trips P -o OUT.ll              loops peeled to their profiled maximum (loops.cpp)
//   s1cc graph IN.ll                                   the dataflow graph, for debugging
#include <cstdio>
#include <fstream>
#include <sstream>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include "s1cc.h"

using namespace llvm;

static std::unique_ptr<Module> readIR(const std::string &path, LLVMContext &ctx) {
    SMDiagnostic err;
    auto M = parseIRFile(path, err, ctx);
    if (!M) {
        err.print("s1cc", errs());
        s1::fail("cannot read " + path);
    }
    return M;
}

static std::map<std::string, std::vector<double>> readTables(const std::string &path) {
    std::map<std::string, std::vector<double>> t;
    std::ifstream f(path);
    if (!f)
        s1::fail("cannot read the tables " + path);
    std::string tag, name;
    int panel, count;
    while (f >> tag >> name >> panel >> count) {
        if (tag != "p")
            s1::fail("bad tables file " + path);
        auto &v = t[name];
        for (int k = 0; k < 256 * count; k++) {
            double x;
            f >> x;
            v.push_back(x);
        }
    }
    return t;
}

static void usage() {
    fprintf(stderr, "usage: s1cc prof IN.ll -o OUT.ll\n"
                    "       s1cc lower IN.ll --profile P --tables T -o K.s1k [--report R] [--regs LIST] [--tol X] "
                    "[--maxdeg N]\n"
                    "       s1cc graph IN.ll\n");
    exit(2);
}

int main(int argc, char **argv) {
    if (argc < 3)
        usage();
    std::string cmd = argv[1], in = argv[2], out, profile, tables, report, kernelPath, tripsPath, intoPath;
    s1::LowerOptions opt;
    for (int i = 3; i < argc; i++) {
        std::string a = argv[i];
        auto next = [&]() -> std::string { if (i + 1 >= argc) usage(); return argv[++i]; };
        if (a == "-o") out = next();
        else if (a == "--profile") profile = next();
        else if (a == "--tables") tables = next();
        else if (a == "--report") report = next();
        else if (a == "--kernel") kernelPath = next();
        else if (a == "--trips") tripsPath = next();
        else if (a == "--into") intoPath = next();
        else if (a == "--tol") opt.fitTol = std::stod(next());
        else if (a == "--tollog") opt.fitTolLog = std::stod(next());
        else if (a == "--no-exact-init") opt.exactInit = false;
        else if (a == "--trace") opt.trace = true;
        else if (a == "--maxdeg") opt.maxDeg = std::stoi(next());
        else if (a == "--regs") {
            opt.regs.clear();
            std::stringstream ss(next());
            std::string r;
            while (std::getline(ss, r, ','))
                opt.regs.push_back(std::stoi(r));
        } else usage();
    }
    LLVMContext ctx;
    auto M = readIR(in, ctx);
    auto writeIR = [&](const std::string &path) {
        std::error_code ec;
        raw_fd_ostream os(path, ec);
        if (ec)
            s1::fail("cannot write " + path);
        M->print(os, nullptr);
    };
    if (cmd == "trips") {
        int n = s1::instrumentTrips(*M);
        writeIR(out);
        printf("%d\n", n);
        return 0;
    }
    if (cmd == "unroll") {
        std::map<int, int> trips;
        std::ifstream tf(tripsPath);
        std::string tag;
        while (tf >> tag) {
            if (tag == "t") {
                int id, n;
                tf >> id >> n;
                trips[id] = std::max(trips[id], n);
            } else {
                std::string rest;
                std::getline(tf, rest);
            }
        }
        std::string rep;
        s1::unrollBounded(*M, trips, rep);
        writeIR(out);
        fputs(rep.c_str(), stdout);
        return 0;
    }
    s1::Program P;
    s1::build(P, *M);
    if (cmd == "graph") {
        for (s1::Stage *S : {&P.A, &P.B}) {
            if (!S->fn)
                continue;
            printf("stage %s\n", S->name.c_str());
            for (auto &n : S->nodes) {
                if (!n->live)
                    continue;
                printf("  n%d %s%s%s%s", n->id, n->op.c_str(), n->bad ? " BAD" : "", n->isUnsigned ? " u" : "",
                       n->specSel ? (n->specFrontier ? " SPEC*" : " SPEC") : "");
                for (auto *i : n->in)
                    printf(" n%d", i->id);
                if (n->kind == s1::NK::Const)
                    printf(" = %g", n->c);
                if (!n->name.empty())
                    printf(" %s[%g]", n->name.c_str(), n->c);
                printf("\n");
            }
            for (auto &isl : S->islands)
                printf("  island %d: root n%d out n%d, %zu members%s\n", isl.id, isl.root->id, isl.out->id,
                       isl.members.size(), isl.cloneable ? "" : " (not cloneable)");
        }
        for (auto &s : P.states)
            printf("state %d %s stage %s %s%d%s\n", s.index, s.name.c_str(), s.stage.c_str(), s.isInt ? "i" : "f",
                   s.bits, s.isUnsigned ? " unsigned" : "");
        return 0;
    }
    if (out.empty())
        usage();
    if (cmd == "init") {
        if (kernelPath.empty())
            usage();
        // the states come from this module's graph; the function goes into --into (the original module)
        std::unique_ptr<Module> target;
        if (!intoPath.empty())
            target = readIR(intoPath, ctx);
        Module *TM = target ? target.get() : M.get();
        std::ifstream kf(kernelPath);
        std::string line;
        std::map<int, std::pair<double, double>> rep;
        bool flag = false;
        while (std::getline(kf, line)) {
            std::istringstream ls(line);
            std::string tag;
            ls >> tag;
            if (tag == ".initflag")
                flag = true;
            if (tag == ".state") {
                int idx;
                std::string stage;
                double s, o;
                ls >> idx >> stage >> s >> o;
                rep[idx] = {s, o};
            }
        }
        LLVMContext &C = M->getContext();
        Function *F = Function::Create(FunctionType::get(Type::getVoidTy(C), false), GlobalValue::ExternalLinkage,
                                       "__s1p_dspstart", *TM);
        IRBuilder<> b(BasicBlock::Create(C, "entry", F));
        for (auto &s : P.states) {
            auto it = rep.find(s.index);
            if (it == rep.end() || flag)          // exact init: the reference keeps S1_INIT's values
                continue;
            double v = -it->second.second * it->second.first;
            Type *t = s.isInt ? (Type *)Type::getIntNTy(C, s.bits)
                              : (s.bits == 64 ? Type::getDoubleTy(C) : Type::getFloatTy(C));
            Value *c = s.isInt ? (Value *)ConstantInt::get(t, (uint64_t)(int64_t)(v >= 9.2e18 ? 0 : (long long)(unsigned long long)v))
                               : (Value *)ConstantFP::get(t, v);
            GlobalVariable *gv = TM == M.get() ? s.gv : TM->getGlobalVariable(s.gv->getName(), true);
            if (!gv)
                s1::fail("init: state " + s.name + " not in " + intoPath);
            Value *ptr = b.CreateConstGEP1_64(Type::getInt8Ty(C), gv, s.offset);
            b.CreateStore(c, ptr);
        }
        b.CreateRetVoid();
        std::error_code ec;
        raw_fd_ostream os(out, ec);
        TM->print(os, nullptr);
        return 0;
    }
    if (cmd == "prof") {
        s1::instrument(P);
        std::error_code ec;
        raw_fd_ostream os(out, ec);
        if (ec)
            s1::fail("cannot write " + out);
        M->print(os, nullptr);
        return 0;
    }
    if (cmd == "lower") {
        if (profile.empty() || tables.empty())
            usage();
        s1::Profile prof;
        // the record ids of the profile are those `s1cc prof` gave: number the graph the same way
        {
            int id = 0;
            std::map<llvm::Value *, int> ids;
            for (s1::Stage *S : {&P.A, &P.B}) {
                if (!S->fn)
                    continue;
                for (auto &n : S->nodes) {
                    if (!n->v || n->kind == s1::NK::Const)
                        continue;
                    Type *t = n->v->getType();
                    if (!t->isIntegerTy() && !t->isFloatingPointTy())
                        continue;
                    auto it = ids.find(n->v);
                    n->prof = it != ids.end() ? it->second : (ids[n->v] = id++);
                }
            }
        }
        prof.read(profile);
        auto tab = readTables(tables);
        std::string rep;
        std::string k = s1::lower(P, prof, tab, opt, rep);
        std::ofstream(out) << k;
        if (!report.empty())
            std::ofstream(report) << rep;
        fputs(rep.c_str(), stdout);
        return 0;
    }
    usage();
}
