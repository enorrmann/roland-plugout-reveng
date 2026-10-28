// The voice functions of the LLVM module as dataflow graphs (s1cc.h, docs/S1CC.md §4).
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <sstream>

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/DemandedBits.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/KnownBits.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#include "s1cc.h"

using namespace llvm;

namespace s1 {

static void printSource();

void fail(const std::string &code, const std::string &msg) {
    fprintf(stderr, "s1cc: error %s: %s\n", code.c_str(), msg.c_str());
    if (debugLevel() >= 1) {
        printSource();
        fprintf(stderr, "s1cc: debug: cause and fix of %s: docs/S1CC.md §9.2\n", code.c_str());
    }
    exit(1);
}

void warn(const std::string &code, const std::string &msg) {
    fprintf(stderr, "s1cc: warning %s: %s\n", code.c_str(), msg.c_str());
}

int debugLevel() {
    const char *e = getenv("S1CC_DEBUG");
    if (!e || !*e)
        return 0;
    int n = atoi(e);
    return n > 0 ? n : (e[0] == '0' ? 0 : 1);
}

// the C / C++ name of a debug-info function: namespaces and classes in front (braids::DigitalOscillator::RenderToy)
static std::string cName(const DISubprogram *sp) {
    std::string n = sp->getName().str();
    for (const DIScope *sc = sp->getScope(); sc && !isa<DIFile>(sc) && !isa<DICompileUnit>(sc); sc = sc->getScope())
        if (!sc->getName().empty())
            n = sc->getName().str() + "::" + n;
    return n;
}

// the C / C++ name of an LLVM function: demangled, without the parameter list
static std::string cName(const Function *F) {
    if (const DISubprogram *sp = F->getSubprogram())
        return cName(sp);
    std::string n = demangle(F->getName().str());
    size_t p = n.find('(');
    return p == std::string::npos ? n : n.substr(0, p);
}

// the file as it opens from the current directory: relative below it, absolute elsewhere
static std::string fileOf(const DILocation *dl) {
    SmallString<256> f(dl->getFilename());
    if (!sys::path::is_absolute(f) && !dl->getDirectory().empty()) {
        f = dl->getDirectory();
        sys::path::append(f, dl->getFilename());
    }
    sys::path::remove_dots(f, true);
    SmallString<256> cwd;
    if (!sys::fs::current_path(cwd)) {
        std::string c = std::string(cwd.str()) + "/", s = f.str().str();
        if (s.compare(0, c.size(), c) == 0)
            return s.substr(c.size());
    }
    return f.str().str();
}

static std::string at(const DILocation *dl) {
    return fileOf(dl) + ":" + std::to_string(dl->getLine()) + " in " + cName(dl->getScope()->getSubprogram());
}

// ---- exact C lines (s1cc build --debug) -------------------------------------------------------------------------
// -O2 moves the code of small ifs in front of them (SimplifyCFG speculation) and gives it the location of the if.
// The same sources at -O0 (S1CC_SRCMAP = the driver's srcmap.ll) still have every instruction at its own line and
// column: an instruction whose -O2 location has no instruction of its opcode in the -O0 code is looked up there,
// among the instructions of that opcode inside the same lexical block.
namespace {

struct SrcInst {
    unsigned line, col;
    std::vector<std::pair<unsigned, unsigned>> blocks;   // (line, column) of its lexical blocks, innermost first
};

struct SrcMap {
    LLVMContext ctx;
    std::unique_ptr<Module> M;
    std::map<std::pair<std::string, std::string>, std::vector<SrcInst>> byFnOp;   // (function, opcode)
};

std::string fnKey(const DISubprogram *sp) {
    return sp->getLinkageName().empty() ? sp->getName().str() : sp->getLinkageName().str();
}

SrcMap *srcMap() {
    static std::unique_ptr<SrcMap> map;
    static bool tried = false;
    if (tried)
        return map.get();
    tried = true;
    const char *path = getenv("S1CC_SRCMAP");
    if (!path || !*path)
        return nullptr;
    auto m = std::make_unique<SrcMap>();
    SMDiagnostic err;
    m->M = parseIRFile(path, err, m->ctx);
    if (!m->M)
        return nullptr;
    for (Function &F : *m->M)
        for (BasicBlock &bb : F)
            for (Instruction &I : bb) {
                const DILocation *dl = I.getDebugLoc();
                if (!dl || !dl->getLine())
                    continue;
                SrcInst si{dl->getLine(), dl->getColumn(), {}};
                for (const DIScope *sc = dl->getScope(); sc && isa<DILexicalBlockBase>(sc); sc = sc->getScope())
                    if (auto *lb = dyn_cast<DILexicalBlock>(sc))
                        si.blocks.push_back({lb->getLine(), lb->getColumn()});
                m->byFnOp[{fnKey(dl->getScope()->getSubprogram()), I.getOpcodeName()}].push_back(si);
            }
    map = std::move(m);
    return map.get();
}

struct Loc {
    std::string file;
    unsigned line = 0;
    std::vector<unsigned> cols;
    std::string note;
};
Loc lastLoc;   // the last location where() printed: fail() shows its source line

// the C line (and columns) of I, whose innermost location is dl
Loc exactLoc(const Instruction *I, const DILocation *dl) {
    Loc l{fileOf(dl), dl->getLine(), {dl->getColumn()}, ""};
    SrcMap *m = srcMap();
    if (!m || isa<CallInst>(I) || isa<PHINode>(I))
        return l;
    auto it = m->byFnOp.find({fnKey(dl->getScope()->getSubprogram()), I->getOpcodeName()});
    if (it == m->byFnOp.end())
        return l;
    std::pair<unsigned, unsigned> blk{0, 0};
    auto *lb = dyn_cast<DILexicalBlock>(dl->getScope());
    if (lb)
        blk = {lb->getLine(), lb->getColumn()};
    std::set<std::pair<unsigned, unsigned>> cands;
    for (auto &si : it->second) {
        if (si.line == dl->getLine() && si.col == dl->getColumn())
            return l;                                     // the location is the C one
        if (!lb || std::find(si.blocks.begin(), si.blocks.end(), blk) != si.blocks.end())
            cands.insert({si.line, si.col});
    }
    std::string op = I->getOpcodeName();
    std::set<unsigned> lines;
    for (auto &c : cands)
        lines.insert(c.first);
    if (lines.size() == 1) {
        l.line = *lines.begin();
        l.cols.clear();
        for (auto &c : cands)
            l.cols.push_back(c.second);
        l.note = "LLVM moved this `" + op + "` to line " + std::to_string(dl->getLine()) + "; in the C it is " +
                 (l.cols.size() == 1 ? "here" : "one of the marked ones");
    } else if (!cands.empty() && cands.size() <= 4) {
        l.note = "LLVM moved this `" + op + "` from one of:";
        for (auto &c : cands)
            l.note += " " + std::to_string(c.first) + ":" + std::to_string(c.second);
    } else {
        l.note = "LLVM moved this `" + op + "`: the line is approximate";
    }
    return l;
}

}  // namespace

static void printSource() {
    if (lastLoc.line == 0)
        return;
    std::ifstream f(lastLoc.file);
    std::string text;
    for (unsigned k = 0; k < lastLoc.line && std::getline(f, text); k++) {
    }
    if (!f)
        return;
    std::string num = std::to_string(lastLoc.line), pad(num.size(), ' '), caret;
    for (unsigned c : lastLoc.cols)
        if (c > 0) {
            if (caret.size() < c)
                caret.resize(c - 1, ' ');
            caret += '^';
        }
    fprintf(stderr, "s1cc: debug: %s | %s\n", num.c_str(), text.c_str());
    if (!caret.empty())
        fprintf(stderr, "s1cc: debug: %s | %s\n", pad.c_str(), caret.c_str());
    if (!lastLoc.note.empty())
        fprintf(stderr, "s1cc: debug: %s   (%s)\n", pad.c_str(), lastLoc.note.c_str());
}

std::string where(const Value *v) {
    if (auto *I = dyn_cast_or_null<Instruction>(v)) {
        if (const DILocation *dl = I->getDebugLoc()) {   // with -g (s1cc build --debug): the C function it came from
            Loc l = exactLoc(I, dl);
            lastLoc = l;
            std::string s = l.file + ":" + std::to_string(l.line) + (l.cols.size() == 1 ? ":" + std::to_string(l.cols[0]) : "") +
                            " in " + cName(dl->getScope()->getSubprogram());
            for (const DILocation *in = dl->getInlinedAt(); in; in = in->getInlinedAt())
                s += ", inlined at " + at(in);
            return s;
        }
        std::string s;
        raw_string_ostream os(s);
        I->print(os);
        return "`" + s.substr(s.find_first_not_of(' ')) + "` in " + I->getFunction()->getName().str() +
               (I->getFunction()->getSubprogram() ? "" : " (s1cc build --debug: the C function and line)");
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

// intrinsics with an operation of their own (the other floating-point ones are math functions, as libm calls)
const std::map<std::string, std::string> kIntrinsicOps = {
    {"fmuladd", "fmuladd"}, {"fma", "fmuladd"}, {"fabs", "fabs"}, {"minnum", "fmin"}, {"maxnum", "fmax"},
    {"minimum", "fmin"}, {"maximum", "fmax"}, {"smin", "smin"}, {"smax", "smax"}, {"umin", "umin"},
    {"umax", "umax"}, {"abs", "abs"}, {"floor", "floor"}};

const std::set<std::string> kS1Calls = {"s1_pitch", "s1_gate", "s1_trig", "s1_increment", "s1_dt", "s1_rdt", "s1_param_get",
                                        "s1_export", "s1_import", "s1_wrap", "s1_clamp", "s1_tri", "s1_exp2", "s1_env",
                                        "s1_color", "s1_lfo_filter", "s1_lfo_amp",
                                        "s1_range", "s1_floor", "s1_ceil", "s1_sign", "s1_abs", "s1_recip", "s1_exp",
                                        "s1_log2", "s1_ldexp", "s1_sum3", "s1_mac2"};

std::string intrinsicBase(const std::string &fn) { return fn.substr(5, fn.find('.', 5) - 5); }   // llvm.<base>.<type>

bool ignoredIntrinsic(const IntrinsicInst *II, const std::string &base) {
    return II->isAssumeLikeIntrinsic() || base == "lifetime" || base == "dbg" || base == "assume" ||
           base == "experimental";
}

// a call or instruction the graph builder rejects: error code (docs/S1CC.md §9.2) and text; code "" = none
struct Problem {
    std::string code, msg;
    bool empty() const { return code.empty(); }
};

// why a call to F is still there after inlining; remarks: opt's missed-inline remarks, mangled callee -> reason
Problem notInlined(const Function *F, const std::map<std::string, std::string> &remarks) {
    std::string why, code = "E106";
    if (F->isDeclaration())
        code = "E101", why = "it has no body in the module: the file that defines it is not among the build's sources (add it), or "
              "it is a library function s1cc does not know";
    else if (F->hasFnAttribute(Attribute::NoInline))
        code = "E102", why = "it is marked noinline";
    else if (F->hasFnAttribute(Attribute::OptimizeNone))
        code = "E103", why = "it is marked optnone";
    else if (F->isVarArg())
        code = "E104", why = "it is variadic (...)";
    else {
        for (const BasicBlock &bb : *F)
            for (const Instruction &I : bb)
                if (auto *CI = dyn_cast<CallInst>(&I))
                    if (CI->getCalledFunction() == F)
                        code = "E105", why = "it is recursive";
        if (why.empty())
            why = "LLVM did not inline it";
    }
    auto it = remarks.find(F->getName().str());
    if (it != remarks.end())
        why += " (LLVM: " + it->second + ")";
    return {code, why};
}

// "" when the graph builder takes the call, else what is wrong with it
Problem callProblem(const CallInst *CI, const std::map<std::string, std::string> &remarks = {}) {
    const Function *F = CI->getCalledFunction();
    if (!F)
        return {"E107", "indirect call (function pointer or virtual method): call the function itself so it can be "
                        "inlined"};
    std::string fn = F->getName().str();
    if (kS1Calls.count(fn) || kLibm.count(fn))
        return {};
    if (auto *II = dyn_cast<IntrinsicInst>(CI)) {
        std::string base = intrinsicBase(fn);
        if (ignoredIntrinsic(II, base) || kIntrinsicOps.count(base) || CI->getType()->isFloatingPointTy())
            return {};
        if ((base == "memset" || base == "memcpy" || base == "memmove") && isa<ConstantInt>(CI->getArgOperand(2)))
            return {};                                   // constant size: one store per word (Builder::memop)
        if (base == "memset" || base == "memcpy" || base == "memmove")
            return {};                                   // variable size / address into a small variable: compares
        return {"E108", "intrinsic " + fn + " is not supported"};
    }
    Problem p = notInlined(F, remarks);
    p.msg = "call to `" + cName(F) + "` not inlined: " + p.msg;
    return p;
}

// "" when the graph builder takes the instruction, else what is wrong with it (loops: s1cc trips / unroll)
Problem instProblem(const Instruction &I, const std::map<std::string, std::string> &remarks = {}) {
    if (auto *CI = dyn_cast<CallInst>(&I))
        return callProblem(CI, remarks);
    if (auto *C = dyn_cast<CastInst>(&I)) {
        std::string op = C->getOpcodeName();
        if (op == "bitcast" || op == "ptrtoint" || op == "inttoptr" || op == "addrspacecast")
            return {"E204", "cast " + op + " is not supported (a pointer used as a number, or a type pun through a "
                            "pointer)"};
        return {};
    }
    if (isa<PHINode>(I) || isa<LoadInst>(I) || isa<StoreInst>(I) || isa<BinaryOperator>(I) || isa<UnaryOperator>(I) ||
        isa<FreezeInst>(I) || isa<CmpInst>(I) || isa<SelectInst>(I) || isa<AllocaInst>(I) ||
        isa<GetElementPtrInst>(I) || isa<ReturnInst>(I) || isa<BranchInst>(I) || isa<UnreachableInst>(I) ||
        isa<SwitchInst>(I))
        return {};
    std::string op = I.getOpcodeName();
    if (op == "invoke" || op == "landingpad" || op == "resume")
        return {"E203", "instruction `" + op + "` is not supported (C++ exceptions)"};
    return {"E201", "instruction `" + op + "` is not supported"};
}

static const int kSmallArray = 16;   // computed indices into arrays up to this many elements: compares and selects

struct Ptr {
    GlobalVariable *gv = nullptr;
    long off = 0;
    Node *var = nullptr;     // computed part (a "gep" node), nullptr: constant offset
    AllocaInst *local = nullptr;   // a local variable of the sample (alloca): forwarded, never in the ring
    Node *idx = nullptr;     // a small computed address: off + cand[k] when idx == key[k] (an index into a small
    std::vector<long> cand, key;   // array, or a phi of pointers into one variable)
};

struct Builder {
    Program &P;
    Dsp &S;
    const DataLayout &DL;
    std::map<const Value *, Node *> vmap;
    std::map<const Value *, Ptr> pmap;
    std::map<int, Node *> cur;           // state index -> current value
    std::map<std::string, Node *> inputs;

    Builder(Program &p, Dsp &s) : P(p), S(s), DL(p.M->getDataLayout()) {}

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
        if (auto *ci = dyn_cast<ConstantInt>(C))          // booleans are 0 / 1 in the lowering (true is -1 signed)
            return cst(ci->getBitWidth() == 1 ? (double)ci->getZExtValue() : (double)ci->getSExtValue(), true,
                       (int)ci->getBitWidth());
        if (auto *cf = dyn_cast<ConstantFP>(C))
            return cst(cf->getValueAPF().convertToDouble());
        if (isa<UndefValue>(C) || isa<ConstantPointerNull>(C) || C->isNullValue())
            return cst(0, C->getType()->isIntegerTy(), C->getType()->isIntegerTy() ? (int)C->getType()->getIntegerBitWidth() : 0);
        fail("E205", "unsupported constant " + where(C));
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
        fail("E206", "value not available (pointer used as a number, or an unsupported instruction): " + where(v));
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
            if (base.idx && gep->accumulateConstantOffset(DL, off)) {   // a constant step from a small computed index
                Ptr p = base;
                p.off += off.getSExtValue();
                pmap[v] = p;
                return p;
            }
            // one computed index into a small array of the state or of the sample (Plaits' `&registration[shift]`,
            // Braids' partials from first_partial): every element, chosen by compares (load / store below)
            if (!base.var && !base.idx && (base.local || base.gv)) {
                MapVector<Value *, APInt> vars;
                APInt coff(64, 0);
                if (gep->collectOffset(DL, 64, vars, coff) && vars.size() == 1) {
                    long stride = vars.front().second.getSExtValue();
                    long start = base.off + coff.getSExtValue();
                    // the variable is a phi / select of constants: those are the only addresses
                    std::vector<long> consts;
                    std::function<bool(Value *)> collect = [&](Value *x) {
                        if (auto *ci = dyn_cast<ConstantInt>(x)) { consts.push_back(ci->getSExtValue()); return true; }
                        if (auto *ph = dyn_cast<PHINode>(x)) {
                            for (Value *in : ph->incoming_values()) if (!collect(in)) return false;
                            return true;
                        }
                        if (auto *sl = dyn_cast<SelectInst>(x))
                            return collect(sl->getTrueValue()) && collect(sl->getFalseValue());
                        if (auto *ce = dyn_cast<CastInst>(x))
                            return collect(ce->getOperand(0));
                        return false;
                    };
                    if (collect(vars.front().first) && consts.size() <= (size_t)kSmallArray) {
                        std::sort(consts.begin(), consts.end());
                        consts.erase(std::unique(consts.begin(), consts.end()), consts.end());
                        Ptr p = base;
                        p.off = start;
                        p.idx = get(vars.front().first);
                        for (long c : consts)
                            p.cand.push_back(c * stride), p.key.push_back(c);
                        pmap[v] = p;
                        return p;
                    }
                    long size = (long)DL.getTypeAllocSize(base.local ? base.local->getAllocatedType()
                                                                     : base.gv->getValueType());
                    long count = stride > 0 ? (size - start) / stride : 0;
                    // an array type indexed by the variable: its length (Braids' partial_phase[11] in a big object)
                    Type *t = gep->getSourceElementType();
                    unsigned nidx = gep->getNumIndices();
                    if (auto *at = dyn_cast<ArrayType>(t))
                        if (nidx == 2 && isa<ConstantInt>(*gep->idx_begin()) && !isa<Constant>(*(gep->idx_begin() + 1)))
                            count = std::min<long>(count, (long)at->getNumElements());
                    if (count >= 1 && count <= kSmallArray) {
                        Ptr p = base;
                        p.off = start;
                        p.idx = get(vars.front().first);
                        for (long k = 0; k < count; k++)
                            p.cand.push_back(k * stride), p.key.push_back(k);
                        pmap[v] = p;
                        return p;
                    }
                }
            }
            if (!base.var && !base.idx && gep->accumulateConstantOffset(DL, off)) {
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
                fail("E304", "computed index into a local array " + where(v) + ": use constant indices");
            Ptr p{base.gv, 0, n};
            pmap[v] = p;
            return p;
        }
        fail("E306", "unsupported pointer " + where(v) + " (only file-scope variables and tables)");
    }

    int stateOf(GlobalVariable *gv, long off, Type *ty, Value *at) {
        for (auto &s : P.states)
            if (s.gv == gv && s.offset == off) {
                if (s.dsp != S.name)
                    fail("E302", "state `" + s.name + "` used by both S1_DSP_A and S1_DSP_B (" + where(at) + ")");
                return s.index;
            }
        StateVar s;
        s.index = (int)P.states.size();
        s.gv = gv;
        s.offset = off;
        s.name = gv->getName().str() + (off ? "+" + std::to_string(off) : "");
        s.isInt = ty->isIntegerTy();
        s.bits = s.isInt ? (int)ty->getIntegerBitWidth() : 0;
        s.dsp = S.name;
        if (gv->hasInitializer()) {
            if (Constant *c = ConstantFoldLoadFromConst(gv->getInitializer(), ty, APInt(64, off), DL)) {
                if (auto *ci = dyn_cast<ConstantInt>(c))
                    s.init = (double)ci->getSExtValue();
                else if (auto *cf = dyn_cast<ConstantFP>(c))
                    s.init = cf->getValueAPF().convertToDouble();
            }
        }
        if (!ty->isIntegerTy() && !ty->isFloatTy() && !ty->isDoubleTy())
            fail("E301", "state `" + s.name + "` is not an int or a float (" + where(at) + ")");
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

    Node *eqIdx(Node *idx, int k) {
        Node *c = mk(NK::Op, "icmp.eq", {idx, cst(k, true, idx->bits)});
        c->isInt = true;
        c->bits = 1;
        return c;
    }
    void load(LoadInst *L) {
        Ptr p = ptrOf(L->getPointerOperand());
        if (p.idx) {                      // a small computed address: the word whose key the index is
            std::vector<Node *> es;
            bool consts = true;
            for (size_t k = 0; k < p.cand.size(); k++) {
                Ptr q = p;
                q.idx = nullptr;
                q.off = p.off + p.cand[k];
                es.push_back(getWord(q, L->getType(), L));
                consts = consts && es.back()->kind == NK::Const;
            }
            // a float table: a sum of steps e0 + sum (e_k - e_k-1) [idx >= key_k] (keys ascending). An integer compare
            // is exact, a select of constants costs nothing, and the compares of one index are the same in every
            // table it reads (the kernel's CSE shares them): ~2 ops per element instead of ~6 for idx == key_k
            // selects (Plaits' chord tables: 290 -> ... instructions per voice). Out of range: the nearest end
            if (consts && es.size() > 2 && !es[0]->isInt) {
                Node *v = es[0];
                for (size_t k = 1; k < es.size(); k++) {
                    double d = es[k]->c - es[k - 1]->c;
                    if (d == 0)
                        continue;
                    Node *ge = mk(NK::Op, "icmp.sge", {p.idx, cst((double)p.key[k], true, p.idx->bits)});
                    ge->isInt = true;
                    ge->bits = 1;
                    v = mk(NK::Op, "fadd", {v, sel(ge, cst(d), cst(0.0), es[0])});
                }
                vmap[L] = v;
                return;
            }
            Node *v = nullptr;
            for (size_t k = 0; k < es.size(); k++)
                v = v ? sel(eqIdx(p.idx, (int)p.key[k]), es[k], v, es[k]) : es[k];
            vmap[L] = v;
            return;
        }
        if (p.local) {
            auto it = locals.find({p.local, p.off});
            vmap[L] = it != locals.end() ? it->second : cst(0, L->getType()->isIntegerTy(),
                                                             L->getType()->isIntegerTy() ? (int)L->getType()->getIntegerBitWidth() : 0);
            return;
        }
        if (!p.gv)
            fail("E305", "load from an unknown address " + where(L));
        if (p.var) {
            if (!p.gv->isConstant())
                fail("E303", "computed index into a state array " + where(L) + ": state arrays need constant indices");
            Node *n = mk(NK::Op, "tload", {p.var}, L);
            n->bad = true;
            vmap[L] = n;
            return;
        }
        if (p.gv->isConstant() && p.gv->hasInitializer()) {
            Constant *c = ConstantFoldLoadFromConst(p.gv->getInitializer(), L->getType(), APInt(64, p.off), DL);
            if (!c)
                fail("E308", "cannot fold the table read " + where(L));
            vmap[L] = constOf(c);
            return;
        }
        if (!p.gv->hasInitializer())
            fail("E307", "`" + p.gv->getName().str() + "` has no definition in the module (" + where(L) +
                 "): compile and link the source that defines it");
        int idx = stateOf(p.gv, p.off, L->getType(), L);
        Node *n = stateValue(idx, L);
        vmap[L] = n;
    }

    void store(StoreInst *St, Node *pred) {
        Ptr p = ptrOf(St->getPointerOperand());
        if (p.idx) {                      // every element, written where the index is its own
            Node *v = get(St->getValueOperand());
            for (size_t k = 0; k < p.cand.size(); k++) {
                Ptr q = p;
                q.idx = nullptr;
                q.off = p.off + p.cand[k];
                Node *e = eqIdx(p.idx, (int)p.key[k]);
                putWord(q, St->getValueOperand()->getType(), v, pred ? band(pred, e) : e, St);
            }
            return;
        }
        if (p.local) {
            Node *v = get(St->getValueOperand());
            auto it = locals.find({p.local, p.off});
            locals[{p.local, p.off}] = pred && it != locals.end() ? sel(pred, v, it->second, v) : v;
            return;
        }
        if (!p.gv || p.var || p.gv->isConstant())
            fail("E309", "store to something that is not a state word " + where(St));
        int idx = stateOf(p.gv, p.off, St->getValueOperand()->getType(), St);
        Node *v = get(St->getValueOperand());
        Node *old = stateValue(idx, St);
        cur[idx] = pred ? sel(pred, v, old, v) : v;
    }

    std::string paramName(Value *desc, long &count) {
        auto *gv = dyn_cast<GlobalVariable>(desc->stripPointerCasts());
        if (!gv || !gv->hasInitializer())
            fail("E112", "s1_param: unknown table " + where(desc));
        auto *cs = dyn_cast<ConstantStruct>(gv->getInitializer());
        if (!cs)
            fail("E113", "s1_param: not an S1_PARAM descriptor " + where(desc));
        auto *ngv = dyn_cast<GlobalVariable>(cs->getOperand(0)->stripPointerCasts());
        auto *str = ngv ? dyn_cast<ConstantDataArray>(ngv->getInitializer()) : nullptr;
        count = cast<ConstantInt>(cs->getOperand(2))->getSExtValue();
        std::string name = str ? str->getAsCString().str() : gv->getName().str();
        P.params[name] = gv;
        P.paramCount[name] = (int)count;
        return name;
    }

    // memset / memcpy of a constant size (Init() of a struct, `state_ = {}`, a struct copy): one store per word the
    // DSP reads or writes in the range (words nothing accesses need nothing); the word's type is that access's
    struct Access { long off; Type *ty; };
    std::vector<Access> accessesIn(Value *base, long lo, long hi) {
        std::vector<Access> out;
        std::set<long> seen;
        for (BasicBlock &bb : *S.fn)
            for (Instruction &I : bb) {
                Value *ptr = nullptr;
                Type *ty = nullptr;
                if (auto *L = dyn_cast<LoadInst>(&I)) ptr = L->getPointerOperand(), ty = L->getType();
                else if (auto *St = dyn_cast<StoreInst>(&I)) ptr = St->getPointerOperand(), ty = St->getValueOperand()->getType();
                if (!ptr || !(ty->isIntegerTy() || ty->isFloatingPointTy()))
                    continue;
                APInt off(64, 0);
                Value *b = ptr->stripAndAccumulateConstantOffsets(DL, off, true);
                long o = off.getSExtValue();
                if (b == base && o >= lo && o + (long)DL.getTypeStoreSize(ty) <= hi && seen.insert(o).second)
                    out.push_back({o, ty});
            }
        return out;
    }
    Ptr constPtr(Value *v, Value *at) {
        Ptr p = ptrOf(v);
        if (p.var || (!p.gv && !p.local))
            fail("E310", "memset / memcpy of something that is not a variable with a constant offset " + where(at));
        return p;
    }
    void putWord(const Ptr &p, Type *ty, Node *v, Node *pred, Value *at) {
        if (p.local) {
            auto it = locals.find({p.local, p.off});
            locals[{p.local, p.off}] = pred && it != locals.end() ? sel(pred, v, it->second, v) : v;
            return;
        }
        if (p.gv->isConstant())
            fail("E309", "store to something that is not a state word " + where(at));
        int idx = stateOf(p.gv, p.off, ty, at);
        Node *old = stateValue(idx, at);
        cur[idx] = pred ? sel(pred, v, old, v) : v;
    }
    Node *getWord(const Ptr &p, Type *ty, Value *at) {
        bool isInt = ty->isIntegerTy();
        int bits = isInt ? (int)ty->getIntegerBitWidth() : 0;
        if (p.local) {
            auto it = locals.find({p.local, p.off});
            return it != locals.end() ? it->second : cst(0, isInt, bits);
        }
        if (p.gv->isConstant() && p.gv->hasInitializer()) {
            Constant *c = ConstantFoldLoadFromConst(p.gv->getInitializer(), ty, APInt(64, p.off), DL);
            if (!c)
                fail("E308", "cannot fold the table read " + where(at));
            return constOf(c);
        }
        return stateValue(stateOf(p.gv, p.off, ty, at), at);
    }
    void memop(CallInst *CI, const std::string &base, Node *pred) {
        long size = cast<ConstantInt>(CI->getArgOperand(2))->getSExtValue();
        Ptr d = constPtr(CI->getArgOperand(0), CI);
        Value *dbase = d.local ? (Value *)d.local : (Value *)d.gv;
        std::vector<Access> words = accessesIn(dbase, d.off, d.off + size);
        if (base == "memset") {
            auto *cv = dyn_cast<ConstantInt>(CI->getArgOperand(1));
            if (!cv)
                fail("E310", "memset with a variable byte " + where(CI));
            uint8_t byte = (uint8_t)cv->getZExtValue();
            for (auto &w : words) {
                unsigned nb = (unsigned)DL.getTypeStoreSize(w.ty);
                APInt bitsv(nb * 8, 0);
                for (unsigned k = 0; k < nb; k++)
                    bitsv |= APInt(nb * 8, byte) << (8 * k);
                Node *v;
                if (w.ty->isIntegerTy())
                    v = cst((double)bitsv.trunc(w.ty->getIntegerBitWidth()).getSExtValue(), true,
                            (int)w.ty->getIntegerBitWidth());
                else
                    v = cst(w.ty->isFloatTy() ? (double)APFloat(APFloat::IEEEsingle(), bitsv).convertToFloat()
                                              : APFloat(APFloat::IEEEdouble(), bitsv).convertToDouble());
                Ptr p = d;
                p.off = w.off;
                putWord(p, w.ty, v, pred, CI);
            }
            return;
        }
        Ptr src = constPtr(CI->getArgOperand(1), CI);
        std::vector<std::pair<Ptr, Node *>> vals;           // read everything first (memmove)
        for (auto &w : words) {
            Ptr sp = src, dp = d;
            sp.off += w.off - d.off;
            dp.off = w.off;
            vals.push_back({dp, getWord(sp, w.ty, CI)});
        }
        for (size_t k = 0; k < words.size(); k++)
            putWord(vals[k].first, words[k].ty, vals[k].second, pred, CI);
    }

    // memcpy with a variable size and / or a small computed destination (Plaits' StringSynthOscillator:
    // std::copy(&unshifted[0], &unshifted[7 - shift], &registration[shift])): for every destination word and every
    // candidate address, the source word at the same distance, when the size reaches it
    void memcpyVar(CallInst *CI, Node *pred) {
        Ptr d = ptrOf(CI->getArgOperand(0));
        Ptr src = constPtr(CI->getArgOperand(1), CI);
        if (d.var || (!d.gv && !d.local))
            fail("E310", "memcpy into something that is not a variable " + where(CI));
        Value *dbase = d.local ? (Value *)d.local : (Value *)d.gv;
        long objSize = (long)DL.getTypeAllocSize(d.local ? d.local->getAllocatedType() : d.gv->getValueType());
        std::vector<Access> words = accessesIn(dbase, 0, objSize);
        if (words.size() > (size_t)kSmallArray)
            fail("E310", "memcpy with a variable size or address: only into a small variable " + where(CI));
        auto *csize = dyn_cast<ConstantInt>(CI->getArgOperand(2));
        Node *size = csize ? nullptr : get(CI->getArgOperand(2));
        std::vector<long> cand = d.idx ? d.cand : std::vector<long>{0}, key = d.idx ? d.key : std::vector<long>{0};
        std::vector<std::tuple<Ptr, Type *, Node *, Node *>> puts;    // read everything first (memmove)
        for (size_t k = 0; k < cand.size(); k++) {
            long start = d.off + cand[k];
            Node *at = d.idx ? eqIdx(d.idx, (int)key[k]) : nullptr;
            for (auto &w : words) {
                long rel = w.off - start;
                if (rel < 0 || (csize && rel >= csize->getSExtValue()))
                    continue;
                Node *c = at;
                if (size) {
                    Node *in = mk(NK::Op, "icmp.sgt", {size, cst((double)rel, true, size->bits)});
                    in->isInt = true;
                    in->bits = 1;
                    c = band(c, in);
                }
                Ptr sp = src;
                sp.off += rel;
                Ptr dp = d;
                dp.idx = nullptr;
                dp.off = w.off;
                puts.push_back({dp, w.ty, getWord(sp, w.ty, CI), c});
            }
        }
        for (auto &t : puts) {
            Node *c = std::get<3>(t);
            putWord(std::get<0>(t), std::get<1>(t), std::get<2>(t), c ? (pred ? band(pred, c) : c) : pred, CI);
        }
    }

    void call(CallInst *CI, Node *pred) {
        Function *F = CI->getCalledFunction();
        if (!F)
            fail(callProblem(CI).code, callProblem(CI).msg + " " + where(CI));
        std::string fn = F->getName().str();
        auto arg = [&](unsigned i) { return get(CI->getArgOperand(i)); };
        auto constArg = [&](unsigned i) -> long {
            auto *c = dyn_cast<ConstantInt>(CI->getArgOperand(i));
            if (!c)
                fail("E110", fn + ": argument " + std::to_string(i) + " must be a constant " + where(CI));
            return c->getSExtValue();
        };
        if (fn == "s1_pitch" || fn == "s1_gate" || fn == "s1_trig" || fn == "s1_increment" || fn == "s1_dt" ||
            fn == "s1_rdt" || fn == "s1_lfo_filter" || fn == "s1_lfo_amp") {
            Node *&n = inputs[fn];
            if (!n) {
                n = mk(fn == "s1_increment" ? NK::Increment : NK::Input, fn, {}, CI);
                n->name = fn.substr(3);
            }
            vmap[CI] = n;
            return;
        }
        if (fn == "s1_env") {                             // the host's envelopes: inputs of DSP B only
            if (S.name != "B")
                fail("E117", "s1_env in S1_DSP_A " + where(CI) + ": the host's envelopes run on DSP B");
            long i = constArg(0);
            if (i < 0 || i > 1)
                fail("E118", "s1_env(" + std::to_string(i) + "): the host has envelopes 0 and 1 " + where(CI));
            std::string name = "env" + std::to_string(i);
            Node *&n = inputs[name];
            if (!n) {
                n = mk(NK::Input, name, {}, CI);
                n->name = name;
            }
            vmap[CI] = n;
            return;
        }
        if (fn == "s1_color") {                           // OSC1 / OSC2 COLOR with its MOD source: DSP A only
            if (S.name != "A")
                fail("E119", "s1_color in S1_DSP_B " + where(CI) + ": the host makes the colors on DSP A");
            long i = constArg(0);
            if (i < 0 || i > 1)
                fail("E120", "s1_color(" + std::to_string(i) + "): the panel has colors 0 (OSC1) and 1 (OSC2) " + where(CI));
            std::string name = "color" + std::to_string(i);
            Node *&n = inputs[name];
            if (!n) {
                n = mk(NK::Input, name, {}, CI);
                n->name = name;
            }
            vmap[CI] = n;
            return;
        }
        if (fn == "s1_param_get") {
            long i = constArg(1);
            // `c ? s1_param(a, i) : s1_param(b, i)`: clang merges the two calls into one on a select of the two
            // descriptors; it becomes the select of the two parameters
            std::function<Node *(Value *, Value *)> param = [&](Value *desc, Value *at) -> Node * {
                if (auto *SI = dyn_cast<SelectInst>(desc->stripPointerCasts())) {
                    Node *t = param(SI->getTrueValue(), nullptr), *f = param(SI->getFalseValue(), nullptr);
                    return sel(get(SI->getCondition()), t, f, t);
                }
                long count = 0;
                std::string name = paramName(desc, count);
                if (i < 0 || i >= count)
                    fail("E111", "s1_param(" + name + ", " + std::to_string(i) + "): index out of the table " +
                                     where(CI));
                Node *n = mk(NK::Param, "param", {}, at);
                n->name = name;
                n->c = (double)i;
                return n;
            };
            vmap[CI] = param(CI->getArgOperand(0), CI);
            return;
        }
        if (fn == "s1_export") {
            if (S.name != "A")
                fail("E114", "s1_export in S1_DSP_B " + where(CI));
            int i = (int)constArg(0);
            Node *v = arg(1);
            auto it = S.exports.find(i);
            S.exports[i] = pred ? sel(pred, v, it != S.exports.end() ? it->second : cst(0), v) : v;
            return;
        }
        if (fn == "s1_import") {
            if (S.name != "B")
                fail("E115", "s1_import in S1_DSP_A " + where(CI));
            Node *n = mk(NK::Import, "import", {}, CI);
            n->c = (double)constArg(0);
            vmap[CI] = n;
            return;
        }
        if (fn == "s1_wrap" || fn == "s1_clamp" || fn == "s1_tri" || fn == "s1_exp2" || fn == "s1_floor" ||
            fn == "s1_ceil" || fn == "s1_sign" || fn == "s1_abs" || fn == "s1_recip" || fn == "s1_exp" ||
            fn == "s1_log2") {
            vmap[CI] = mk(NK::Op, fn, {arg(0)}, CI);
            return;
        }
        if (fn == "s1_ldexp") {
            vmap[CI] = mk(NK::Op, fn, {arg(0), arg(1)}, CI);
            return;
        }
        if (fn == "s1_sum3") {
            vmap[CI] = mk(NK::Op, fn, {arg(0), arg(1), arg(2)}, CI);
            return;
        }
        if (fn == "s1_mac2") {
            vmap[CI] = mk(NK::Op, fn, {arg(0), arg(1), arg(2), arg(3), arg(4)}, CI);
            return;
        }
        if (fn == "s1_range") {
            vmap[CI] = mk(NK::Op, fn, {arg(0), arg(1), arg(2)}, CI);
            return;
        }
        if (auto *II = dyn_cast<IntrinsicInst>(CI)) {
            std::string base = intrinsicBase(fn);
            if (ignoredIntrinsic(II, base))
                return;
            if ((base == "memset" || base == "memcpy" || base == "memmove") && isa<ConstantInt>(CI->getArgOperand(2))) {
                memop(CI, base, pred);
                return;
            }
            if (base == "memcpy" || base == "memmove") {   // std::copy(src, src + n, &dst[shift]): per word
                memcpyVar(CI, pred);
                return;
            }
            if (base == "memset") {               // std::fill(&a[0], &a[n], 0): each word below the size
                Ptr d = constPtr(CI->getArgOperand(0), CI);
                auto *cv = dyn_cast<ConstantInt>(CI->getArgOperand(1));
                Value *dbase = d.local ? (Value *)d.local : (Value *)d.gv;
                long size = (long)DL.getTypeAllocSize(d.local ? d.local->getAllocatedType() : d.gv->getValueType()) - d.off;
                std::vector<Access> words = accessesIn(dbase, d.off, d.off + size);
                if (!cv || !cv->isZero() || words.size() > (size_t)kSmallArray)
                    fail("E310", "memset with a variable size: only zeroing a small variable " + where(CI));
                Node *n = get(CI->getArgOperand(2));
                for (auto &w : words) {
                    Node *c = mk(NK::Op, "icmp.sgt", {n, cst((double)(w.off - d.off), true, n->bits)});
                    c->isInt = true;
                    c->bits = 1;
                    Ptr q = d;
                    q.off = w.off;
                    Node *z = w.ty->isIntegerTy() ? cst(0, true, (int)w.ty->getIntegerBitWidth()) : cst(0);
                    putWord(q, w.ty, z, pred ? band(pred, c) : c, CI);
                }
                return;
            }
            auto it = kIntrinsicOps.find(base);
            if (it != kIntrinsicOps.end()) {
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
            fail(callProblem(CI).code, callProblem(CI).msg + " " + where(CI));
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
        fail(callProblem(CI).code, callProblem(CI).msg + " (" + where(CI) + "); voice code must be inlined (static inline / header code, or "
             "its source in the build) or use the s1_* intrinsics");
    }

    static bool isPow2(long x) { return x > 0 && (x & (x - 1)) == 0; }
    static bool contiguous(unsigned long m) { return m && ((m >> __builtin_ctzl(m)) & ((m >> __builtin_ctzl(m)) + 1)) == 0; }

    // x ^ (c ? K : 0) with every possible bit of x inside K (Braids' triangle `(p << 1) ^ (p & 0x8000 ? 0xffff : 0)`):
    // x ^ K = K - x, so the xor is c ? K - x : x, exact and without an island
    bool xorMask(BinaryOperator *B) {
        for (int i = 0; i < 2; i++) {
            auto *Sl = dyn_cast<SelectInst>(B->getOperand(1 - i));
            if (!Sl)
                continue;
            auto *ct = dyn_cast<ConstantInt>(Sl->getTrueValue()), *cf = dyn_cast<ConstantInt>(Sl->getFalseValue());
            if (!ct || !cf || (!ct->isZero() && !cf->isZero()))
                continue;
            APInt K = ct->isZero() ? cf->getValue() : ct->getValue();
            Value *x = B->getOperand(i);
            KnownBits kb = computeKnownBits(x, DL);
            if ((~kb.Zero & ~K) != 0)                  // a bit of x could fall outside K
                continue;
            Node *xn = get(x), *c = get(Sl->getCondition());
            Node *k = cst((double)K.getZExtValue(), true, xn->bits);
            Node *d = mk(NK::Op, "sub", {k, xn});
            d->isInt = true;
            d->bits = xn->bits;
            Node *n = ct->isZero() ? mk(NK::Op, "select", {c, xn, d}, B) : mk(NK::Op, "select", {c, d, xn}, B);
            vmap[B] = n;
            return true;
        }
        return false;
    }

    void binop(BinaryOperator *B) {
        std::string op = B->getOpcodeName();
        if (op == "xor" && !isa<ConstantInt>(B->getOperand(0)) && !isa<ConstantInt>(B->getOperand(1)) &&
            B->getType()->getIntegerBitWidth() > 1 && xorMask(B))
            return;
        Node *a = get(B->getOperand(0)), *b = get(B->getOperand(1));
        // x / y with y variable: x * recip(y). recip is an island when y depends on one value (a pitch: 1/y is then
        // 2^polynomial of it, exact and without recipseed, which is not proven on the SYSTEM-1); else recipseed
        // (off: the island clone of a recip gave constant samples for a root that is an input, 2026-10-27; to be
        // fixed before it is turned on again)
        if (false && (op == "udiv" || op == "sdiv" || op == "fdiv") && !isa<Constant>(B->getOperand(1))) {
            Node *r = mk(NK::Op, "recip", {b});
            r->bad = r->soft = true;
            b = r;
        }
        Node *n = mk(NK::Op, op, {a, b}, B);
        auto *cb = dyn_cast<ConstantInt>(B->getOperand(1));
        auto *ca = dyn_cast<ConstantInt>(B->getOperand(0));
        if (op == "udiv" || op == "sdiv")
            n->bad = false;                                  // x * 1/y like fdiv, without the floor (lower.cpp recip)
        else if (op == "urem" || op == "srem")
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
                n->bad = false;                      // any constant: runs of bits (lower.cpp)
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
                        fail("E404", "loop in voice code (" + F->getName().str() + "): unroll it or give it a constant "
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
                if (auto *phi = dyn_cast<PHINode>(&I); phi && phi->getType()->isPointerTy()) {
                    // pointers into one variable at constant offsets (SROA's `&registration[shift]`): a small
                    // computed address whose key is the byte offset, chosen like any phi
                    Node *off = nullptr;
                    Ptr base;
                    for (int k = (int)phi->getNumIncomingValues() - 1; k >= 0; k--) {
                        Ptr q = ptrOf(phi->getIncomingValue(k));
                        if (q.var || q.idx || (!q.gv && !q.local) || (k != (int)phi->getNumIncomingValues() - 1 &&
                                                                       (q.gv != base.gv || q.local != base.local)))
                            fail("E306", "unsupported pointer " + where(phi) + " (a phi of pointers into different "
                                 "variables, or with computed offsets)");
                        base = q;
                        BasicBlock *ib = phi->getIncomingBlock(k);
                        Node *e = edgeOf(ib, bb);
                        if (!e)
                            e = pred[ib];
                        Node *c = cst((double)q.off, true, 64);
                        off = !off ? c : sel(e ? e : nullptr, c, off, c);
                    }
                    Ptr p = base;
                    p.off = 0;
                    p.idx = off;
                    std::set<long> seen;
                    for (unsigned k = 0; k < phi->getNumIncomingValues(); k++) {
                        long o = ptrOf(phi->getIncomingValue(k)).off;
                        if (seen.insert(o).second)
                            p.cand.push_back(o), p.key.push_back(o);
                    }
                    pmap[phi] = p;
                } else if (auto *phi = dyn_cast<PHINode>(&I)) {
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
                    if (!instProblem(*C).empty())
                        fail(instProblem(*C).code, instProblem(*C).msg + " " + where(C));
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
                    if (Sel->getType()->isPointerTy())         // of two S1_PARAM descriptors: taken by s1_param
                        continue;
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
                } else if (auto *Sw = dyn_cast<SwitchInst>(&I)) {
                    // a switch is an if / else chain: each case an edge `cond == value`, the default `none of them`;
                    // cases that share a successor are or-ed (exclusive: the sum of their predicates)
                    Node *c = get(Sw->getCondition());
                    int bits = (int)Sw->getCondition()->getType()->getIntegerBitWidth();
                    std::map<BasicBlock *, Node *> to;
                    Node *none = nullptr;
                    for (auto &cs : Sw->cases()) {
                        Node *eq = boolOp("icmp.eq", {c, cst((double)cs.getCaseValue()->getSExtValue(), true, bits)});
                        Node *&e = to[cs.getCaseSuccessor()];
                        e = e ? boolOp("bor", {e, band(p, eq)}) : band(p, eq);
                        none = band(none, bnot(eq));
                    }
                    Node *&d = to[Sw->getDefaultDest()];
                    Node *def = band(p, none);
                    d = d ? boolOp("bor", {d, def}) : def;
                    for (auto &kv : to)
                        edge[{bb, kv.first}] = kv.second;
                } else if (isa<UnreachableInst>(&I)) {
                } else {
                    fail(instProblem(I).code, instProblem(I).msg + " " + where(&I));
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

// ---- affine simplification of integer sums (docs/S1CC.md §4.8) ----
// Braids' BEGIN_INTERPOLATE_PHASE_INCREMENT with size 1 is prev + (prev < cur ? cur - prev : ~(prev - cur)), that is
// cur or cur - 1; LLVM leaves it, and the lowering would compute it with wraps modulo 2^32. A sum whose terms cancel
// (through one select, whose arms are taken apart) is rebuilt from what is left: modulo 2^bits it is the same value.
namespace {
struct Aff {
    std::map<Node *, double> t;
    double c = 0;
    void add(const Aff &o, double k) {
        for (auto &kv : o.t)
            if ((t[kv.first] += k * kv.second) == 0)
                t.erase(kv.first);
        c += k * o.c;
    }
};

// the affine form of n over its leaves; sel: the one select met (its arm is chosen by arm, -1 = none allowed yet)
bool affOf(Node *n, int bits, Aff &out, Node *&sel, int arm, int depth) {
    if (n->kind == NK::Const && n->isInt) {
        out.c = n->c;
        return true;
    }
    if (depth < 8 && n->kind == NK::Op && n->isInt && n->bits == bits) {
        auto sub = [&](Node *x, double k) {
            Aff a;
            if (!affOf(x, bits, a, sel, arm, depth + 1))
                return false;
            out.add(a, k);
            return true;
        };
        if (n->op == "add")
            return sub(n->in[0], 1) && sub(n->in[1], 1);
        if (n->op == "sub")
            return sub(n->in[0], 1) && sub(n->in[1], -1);
        if (n->op == "xor" && n->in[1]->kind == NK::Const && n->in[1]->c == -1) {
            out.c -= 1;
            return sub(n->in[0], -1);
        }
        if ((n->op == "mul" && n->in[1]->kind == NK::Const) || (n->op == "shl" && n->in[1]->kind == NK::Const))
            return sub(n->in[0], n->op == "mul" ? n->in[1]->c : std::ldexp(1.0, (int)n->in[1]->c));
        if (n->op == "select" && arm >= 0 && (!sel || sel == n)) {
            sel = n;
            return sub(n->in[arm ? 1 : 2], 1);
        }
    }
    out.t[n] = 1;
    return true;
}

int leaves(Node *n, int bits, int depth) {        // terms of the expression as written
    if (depth < 8 && n->kind == NK::Op && n->isInt && n->bits == bits &&
        (n->op == "add" || n->op == "sub" || n->op == "select" ||
         (n->op == "xor" && n->in[1]->kind == NK::Const && n->in[1]->c == -1)))
        return leaves(n->in[0], bits, depth + 1) + (n->in.size() > 1 ? leaves(n->in.back(), bits, depth + 1) : 0) +
               (n->op == "select" ? leaves(n->in[1], bits, depth + 1) : 0);
    return n->kind == NK::Const ? 0 : 1;
}
}  // namespace

void affineSimplify(Dsp &S) {
    std::vector<std::unique_ptr<Node>> made;
    auto mk = [&](const std::string &op, std::vector<Node *> in, const Node *like) {
        auto n = std::make_unique<Node>();
        n->kind = NK::Op;
        n->op = op;
        n->in = std::move(in);
        n->isInt = true;
        n->bits = like->bits;
        made.push_back(std::move(n));
        return made.back().get();
    };
    auto cstN = [&](double c, const Node *like) {
        auto n = std::make_unique<Node>();
        n->kind = NK::Const;
        n->op = "const";
        n->c = c;
        n->isInt = true;
        n->bits = like->bits;
        made.push_back(std::move(n));
        return made.back().get();
    };
    auto build = [&](const Aff &a, const Node *like) -> Node * {   // sum of k * leaf + c (k = +-1 only)
        Node *r = nullptr;
        for (auto &kv : a.t) {
            if (kv.second != 1 && kv.second != -1)
                return nullptr;
            r = !r ? (kv.second == 1 ? kv.first : mk("sub", {cstN(0, like), kv.first}, like))
                   : mk(kv.second == 1 ? "add" : "sub", {r, kv.first}, like);
        }
        if (!r)
            return cstN(a.c, like);
        return a.c != 0 ? mk("add", {r, cstN(a.c, like)}, like) : r;
    };
    for (auto &np : S.nodes) {
        Node *n = np.get();
        if (n->kind != NK::Op || !n->isInt || n->bits <= 1 || (n->op != "add" && n->op != "sub"))
            continue;
        int before = leaves(n, n->bits, 0);
        Node *sel = nullptr;
        Aff plain;
        if (!affOf(n, n->bits, plain, sel, -1, 0))
            continue;
        if ((int)plain.t.size() < before - 1 && plain.t.size() <= 2) {
            if (Node *r = build(plain, n)) {
                n->op = "add";
                n->in = {r, cstN(0, n)};
                continue;
            }
        }
        // through one select: both arms
        Aff a, b;
        Node *sa = nullptr, *sb = nullptr;
        if (!affOf(n, n->bits, a, sa, 1, 0) || !sa || !affOf(n, n->bits, b, sb, 0, 0) || sb != sa)
            continue;
        if (a.t.size() + b.t.size() + 2 < (size_t)before && a.t.size() <= 2 && b.t.size() <= 2) {
            Node *ra = build(a, n), *rb = build(b, n);
            if (ra && rb) {
                n->op = "select";
                n->in = {sa->in[0], ra, rb};
            }
        }
    }
    // c ? x + y : x  ->  x + (c ? y : 0): the lowering makes c ? y : 0 one product of c, and the add a mad, where the
    // select of two sums would subtract them first (the phis of Braids' BLEP branches: this_sample += ...)
    for (auto &np : S.nodes) {
        Node *n = np.get();
        if (n->kind != NK::Op || n->op != "select" || n->in.size() != 3)
            continue;
        for (int arm = 1; arm <= 2; arm++) {
            Node *a = n->in[arm], *b = n->in[3 - arm];
            if (a->kind != NK::Op || a->in.size() != 2 || a->users.size() > 1)
                continue;
            bool add = a->op == "add" || a->op == "fadd", sub = a->op == "sub" || a->op == "fsub";
            if (!add && !sub)
                continue;
            int xi = a->in[0] == b ? 0 : (add && a->in[1] == b ? 1 : -1);
            if (xi < 0)
                continue;
            Node *y = a->in[1 - xi];
            if (y->kind == NK::Const)            // x - 1: already one add, and the phase wrap stays recognisable
                break;
            Node *zero = n->isInt ? cstN(0, n) : nullptr;
            if (!zero) {
                auto z = std::make_unique<Node>();
                z->kind = NK::Const;
                z->op = "const";
                made.push_back(std::move(z));
                zero = made.back().get();
            }
            Node *sy = mk("select", arm == 1 ? std::vector<Node *>{n->in[0], y, zero}
                                             : std::vector<Node *>{n->in[0], zero, y}, n);
            sy->isInt = n->isInt;
            sy->bits = n->bits;
            n->op = a->op;
            n->in = {b, sy};
            break;
        }
    }
    if (made.empty())
        return;
    // the new nodes before their users: a topological order again, the old order kept where it allows
    std::vector<std::unique_ptr<Node>> all;
    for (auto &np : S.nodes)
        all.push_back(std::move(np));
    for (auto &np : made)
        all.push_back(std::move(np));
    std::map<Node *, std::unique_ptr<Node> *> own;
    for (auto &np : all)
        own[np.get()] = &np;
    std::set<Node *> done;
    S.nodes.clear();
    std::function<void(Node *)> place = [&](Node *x) {
        if (done.count(x))
            return;
        done.insert(x);
        for (Node *i : x->in)
            place(i);
        S.nodes.push_back(std::move(*own[x]));
    };
    for (auto &np : all)
        if (np)
            place(np.get());
    for (size_t i = 0; i < S.nodes.size(); i++)
        S.nodes[i]->id = (int)i;
}

void markLive(Program &P, Dsp &S) {
    // from the output and the exports; a state word's new value only when its old value is read by something live
    // (a word written and never read, like Braids' strike_ in a shape that never strikes, costs nothing)
    std::vector<Node *> stack;
    if (S.ret)
        stack.push_back(S.ret);
    for (auto &kv : S.exports)
        stack.push_back(kv.second);
    std::set<int> rooted;
    for (bool more = true; more;) {
        while (!stack.empty()) {
            Node *n = stack.back();
            stack.pop_back();
            if (n->live)
                continue;
            n->live = true;
            for (Node *i : n->in)
                stack.push_back(i);
        }
        more = false;
        for (auto &np : S.nodes)
            if (np->live && np->kind == NK::State && !rooted.count(np->state)) {
                rooted.insert(np->state);
                if (P.states[np->state].final) {
                    stack.push_back(P.states[np->state].final);
                    more = true;
                }
            }
    }
    for (int s : S.states)                     // the rest: not stored (lower.cpp stores live finals only)
        if (!rooted.count(s))
            P.states[s].final = nullptr;
    for (auto &n : S.nodes)
        n->users.clear();
    for (auto &n : S.nodes)
        if (n->live)
            for (Node *i : n->in)
                i->users.push_back(n.get());
}

// integer interpretation: unsigned when an operation reads or makes it as unsigned (to a fixed point)
void signedness(Program &P, Dsp &S) {
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

// the C name of the field at byte offset `bits` / 8 inside a value of type t: ".state_.toy.held_sample", "[3]"
static std::string memberPath(const DIType *t, uint64_t bits) {
    while (auto *d = dyn_cast_or_null<DIDerivedType>(t)) {
        if (d->getTag() != dwarf::DW_TAG_typedef && d->getTag() != dwarf::DW_TAG_const_type &&
            d->getTag() != dwarf::DW_TAG_volatile_type)
            break;
        t = d->getBaseType();
    }
    auto *c = dyn_cast_or_null<DICompositeType>(t);
    if (!c)
        return "";
    if (c->getTag() == dwarf::DW_TAG_array_type) {
        const DIType *e = c->getBaseType();
        uint64_t es = e ? e->getSizeInBits() : 0;
        while (auto *d = dyn_cast_or_null<DIDerivedType>(e))
            if (!es && d->getBaseType())
                e = d->getBaseType(), es = e->getSizeInBits();
            else
                break;
        if (!es)
            return "";
        return "[" + std::to_string(bits / es) + "]" + memberPath(c->getBaseType(), bits % es);
    }
    for (const DINode *el : c->getElements()) {
        auto *m = dyn_cast<DIDerivedType>(el);
        if (!m || (m->getTag() != dwarf::DW_TAG_member && m->getTag() != dwarf::DW_TAG_inheritance) ||
            m->isStaticMember())
            continue;
        uint64_t o = m->getOffsetInBits(), sz = m->getSizeInBits();
        if (!sz && m->getBaseType())
            sz = m->getBaseType()->getSizeInBits();
        if (bits >= o && bits < o + std::max<uint64_t>(sz, 1))
            return (m->getTag() == dwarf::DW_TAG_member ? "." + m->getName().str() : "") +
                   memberPath(m->getBaseType(), bits - o);
    }
    return "";
}

// a state word as the C sees it: osc.state_.toy.held_sample (debug info), else its LLVM name (_ZL3osc+40)
static std::string stateCName(const StateVar &sv) {
    SmallVector<DIGlobalVariableExpression *, 1> gves;
    sv.gv->getDebugInfo(gves);
    if (gves.empty())
        return sv.name;
    const DIGlobalVariable *g = gves[0]->getVariable();
    return g->getName().str() + memberPath(g->getType(), (uint64_t)sv.offset * 8);
}

// the sources (inputs, parameters, state words) a node depends on, as the C names them
static std::string sourcesOf(Program &P, Node *N) {
    std::set<std::string> names;
    std::set<Node *> seen;
    std::vector<Node *> work = {N};
    while (!work.empty()) {
        Node *n = work.back();
        work.pop_back();
        if (!seen.insert(n).second)
            continue;
        switch (n->kind) {
        case NK::Input: names.insert("s1_" + n->name + "()"); break;
        case NK::Increment: names.insert("s1_increment()"); break;
        case NK::Param: names.insert("s1_param(" + n->name + ", " + std::to_string((int)n->c) + ")"); break;
        case NK::State: names.insert(stateCName(P.states[n->state])); break;
        case NK::Import: names.insert("s1_import(" + std::to_string((int)n->c) + ")"); break;
        case NK::XIn: names.insert("x (the input of S1_DSP_B)"); break;
        default:
            for (Node *i : n->in)
                work.push_back(i);
        }
    }
    std::string s;
    for (auto &n : names)
        s += (s.empty() ? "" : ", ") + n;
    return s;
}

// islands: for every bad node, the single values that cut all its paths to the sources (dominators of the
// dependency DAG), highest first; members = the nodes that depend on the root only
void islands(Program &P, Dsp &S) {
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
        if (chain.empty() && N->soft) {       // no single value: lowered directly (recipseed)
            N->bad = false;
            continue;
        }
        if (chain.empty()) {
            std::string what = N->v && isa<Instruction>(N->v) ? "`" + std::string(cast<Instruction>(N->v)->getOpcodeName()) + "` " : "";
            std::string at = where(N->v);   // last: fail() shows this line
            fail("E501", "a table read / function / division " + what + "that depends on several values (" +
                 sourcesOf(P, N) + ") at " + at + ": only functions of one value can be tabulated");
        }
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
            // outputs: members used outside the island (or DSP outputs)
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
                        if ((!y->v && y->op != "recip") || (y->v && (!isa<Instruction>(y->v) || isa<PHINode>(y->v))))
                            isl.cloneable = false;
                    }
                if (!R->v)
                    isl.cloneable = false;
                // a state word's new value inside the island would never be stored (covered nodes are not emitted):
                // a root past it is the one to use (Plaits' sine: phase += f, wrap, table; root = the wrapped phase)
                bool holdsFinal = false;
                for (Node *y : isl.members)
                    for (int st : S.states)
                        holdsFinal |= P.states[st].final == y;
                if (holdsFinal)
                    continue;
                S.islands.push_back(isl);
            }
        }
    }
}

// values computed only for one arm of a select (speculated by clang): the cone of each arm whose every user is
// inside the cone (or is the select, through that arm)
void speculation(Dsp &S) {
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

// knob-derived columns (docs/S1CC.md §4.6): a value computed from one S1_PARAM table only (and constants) is a
// function of that knob, known exactly at its 256 positions (`prof sweep`). The heavy ones that feed the rest of the
// voice become tables of their own, written by the ARM with the knob: no DSP work, no fit (Braids' `intervals[p >> 9]`,
// Plaits' per-block coefficients from a knob). Run after islands(), so the island ids stay those of the profile.
void deriveKnobColumns(Program &P) {
    assignProfIds(P);
    for (Dsp *S : {&P.A, &P.B}) {
        if (!S->fn)
            continue;
        std::map<Node *, std::string> dep;      // "" constants only, a table name, "*" anything else
        std::map<Node *, int> ops;              // operations (bad ones count 4) in the knob-only cone, capped
        for (auto &np : S->nodes) {
            Node *n = np.get();
            std::string d;
            int c = 0;
            if (n->kind == NK::Param)
                d = n->name;
            else if (n->kind != NK::Const && n->kind != NK::Op)
                d = "*";
            else
                for (Node *i : n->in) {
                    const std::string &di = dep[i];
                    if (di.empty())
                        continue;
                    d = d.empty() ? di : (d == di ? d : "*");
                    c += ops[i];
                }
            if (n->kind == NK::Op)
                c += n->bad ? 4 : 1;
            dep[n] = d;
            ops[n] = std::min(c, 1000);
        }
        std::set<Node *> roots;
        if (S->ret)
            roots.insert(S->ret);
        for (auto &kv : S->exports)
            roots.insert(kv.second);
        for (int s : S->states)
            if (P.states[s].final)
                roots.insert(P.states[s].final);
        int made = 0;
        for (auto &np : S->nodes) {
            Node *n = np.get();
            const std::string &d = dep[n];
            if (!n->live || n->kind != NK::Op || d.empty() || d == "*" || ops[n] < 2 || n->prof < 0)
                continue;
            bool frontier = roots.count(n) > 0;
            for (Node *u : n->users)
                frontier = frontier || dep[u] != d;
            auto sw = P.sweep.find(n->prof);
            if (!frontier || sw == P.sweep.end() || !sw->second.count(d))
                continue;
            std::vector<double> v = sw->second.at(d);
            for (double &x : v) {
                if (n->isInt && n->bits == 1)
                    x = x != 0;                                  // booleans: 0 / 1 (the profile's signed view: -1)
                else if (n->isInt && n->isUnsigned && x < 0)
                    x += std::ldexp(1.0, n->bits);
            }
            std::string name = d + "~" + std::to_string(n->prof);
            P.derived[name] = v;
            P.derivedBase[name] = d;
            P.params[name] = P.params.at(d);
            P.paramCount[name] = 1;
            n->kind = NK::Param;
            n->op = "param";
            n->name = name;
            n->c = 0;
            n->in.clear();
            n->bad = false;
            n->specSel = nullptr;
            n->specFrontier = false;
            made++;
        }
        if (made) {
            for (auto &np : S->nodes)
                np->live = false;
            markLive(P, *S);
            if (debugLevel() >= 1)
                fprintf(stderr, "s1cc: debug: DSP %s: %d knob-derived columns\n", S->name.c_str(), made);
        }
    }
}

}  // namespace

void relive(Program &P, Dsp &S) {
    for (auto &np : S.nodes)
        np->live = false;
    markLive(P, S);
}

void assignProfIds(Program &P) {
    int id = 0;
    std::map<Value *, int> ids;
    for (Dsp *S : {&P.A, &P.B}) {
        if (!S->fn)
            continue;
        for (auto &n : S->nodes) {
            if (!n->v || n->kind == NK::Const)
                continue;
            Type *t = n->v->getType();
            if (!t->isIntegerTy() && !t->isFloatingPointTy())
                continue;
            auto it = ids.find(n->v);
            n->prof = it != ids.end() ? it->second : (ids[n->v] = id++);
        }
    }
}

void build(Program &P, Module &M) {
    P.M = &M;
    P.A.name = "A";
    P.B.name = "B";
    P.A.fn = M.getFunction("s1_dsp_a");
    P.B.fn = M.getFunction("s1_dsp_b");
    if (!P.A.fn || P.A.fn->isDeclaration())
        fail("E008", "no S1_DSP_A (s1_dsp_a) in the module");
    if (P.B.fn && P.B.fn->isDeclaration())
        P.B.fn = nullptr;
    for (Dsp *S : {&P.A, &P.B}) {
        if (!S->fn)
            continue;
        Builder b(P, *S);
        b.run();
        if (debugLevel() >= 2)
            fprintf(stderr, "graph %s: %zu nodes\n", S->name.c_str(), S->nodes.size());
        if (!S->ret)
            fail("E009", S->fn->getName().str() + " returns nothing");
        affineSimplify(*S);
        {   // demanded bits: LLVM narrows `(int16_t)(t + 32768)` to `sext(t) ^ 0x7fff8000` when only the low bits
            // matter; the lowering computes in reals and needs the small representative (Node::demand)
            DominatorTree DT(*S->fn);
            AssumptionCache AC(*S->fn);
            DemandedBits DB(*S->fn, AC, DT);
            for (auto &np : S->nodes) {
                auto *I = np->v ? dyn_cast<Instruction>(np->v) : nullptr;
                if (!I || !I->getType()->isIntegerTy() || I->getType()->getIntegerBitWidth() <= 1 ||
                    I->getType()->getIntegerBitWidth() > 64)
                    continue;
                unsigned k = DB.getDemandedBits(I).getActiveBits();
                if (k > 0 && k < I->getType()->getIntegerBitWidth())
                    np->demand = (int)k;
            }
        }
        markLive(P, *S);
    }
    for (int k = 0; k < 4; k++)
        for (Dsp *S : {&P.A, &P.B})
            if (S->fn)
                signedness(P, *S);
    int id = 0;
    for (Dsp *S : {&P.A, &P.B}) {
        if (!S->fn)
            continue;
        speculation(*S);
        islands(P, *S);
        for (auto &isl : S->islands)
            isl.id = id++;
    }
    if (!P.sweep.empty())
        deriveKnobColumns(P);
}

std::string diagnose(Module &M, const std::string &remarksText) {
    // "remark: f.cc:3:25: 'callee' not inlined into 'caller' because REASON"
    std::map<std::string, std::string> remarks;
    {
        std::istringstream rs(remarksText);
        std::string line;
        while (std::getline(rs, line)) {
            size_t q = line.find(" not inlined into ");
            size_t b = line.find(" because ");
            size_t q0 = line.rfind('\'', q - 2);
            if (q == std::string::npos || b == std::string::npos || q0 == std::string::npos || line[q - 1] != '\'')
                continue;
            remarks.emplace(line.substr(q0 + 1, q - q0 - 2), line.substr(b + 9));
        }
    }
    std::ostringstream o;
    for (const char *voice : {"s1_dsp_a", "s1_dsp_b"}) {
        Function *V = M.getFunction(voice);
        if (!V || V->isDeclaration())
            continue;
        // C function -> reasons -> where; the C function of a call left in place is its callee (that is what did not
        // compile), the others are where the instruction came from (the innermost function of its debug location)
        std::map<std::string, std::map<std::string, std::vector<std::string>>> bad;
        std::set<std::string> inlined;
        std::vector<Function *> work = {V};
        std::set<Function *> seen = {V};
        while (!work.empty()) {
            Function *F = work.back();
            work.pop_back();
            for (BasicBlock &bb : *F)
                for (Instruction &I : bb) {
                    const DILocation *dl = I.getDebugLoc();
                    if (dl && F == V)
                        for (const DILocation *l = dl; l; l = l->getInlinedAt())
                            inlined.insert(cName(l->getScope()->getSubprogram()));
                    Problem pr = instProblem(I, remarks);
                    if (pr.empty())
                        continue;
                    std::string p = pr.msg;
                    std::string fn = dl ? cName(dl->getScope()->getSubprogram()) : cName(F);
                    if (auto *CI = dyn_cast<CallInst>(&I))
                        if (Function *C = CI->getCalledFunction()) {
                            fn = cName(C);
                            if (!C->isDeclaration() && seen.insert(C).second)
                                work.push_back(C);   // its body: what else would fail once it is inlined
                        }
                    if (p.compare(0, 9, "call to `") == 0)          // the function is the heading already
                        p = p.substr(p.find("` ") + 2);
                    auto &w = bad[fn][pr.code + ": " + p];
                    std::string at = where(&I);
                    if (std::find(w.begin(), w.end(), at) == w.end())
                        w.push_back(at);
                }
        }
        std::string entry = std::string(voice) == "s1_dsp_a" ? "S1_DSP_A" : "S1_DSP_B";
        if (bad.empty())
            o << entry << " (" << voice << "): every call inlined, every instruction supported\n";
        else
            o << entry << " (" << voice << "): " << bad.size() << " C function(s) s1cc cannot compile\n";
        for (auto &f : bad) {
            o << "  " << f.first << "\n";
            for (auto &r : f.second) {
                o << "    " << r.first << "\n";
                for (size_t k = 0; k < r.second.size() && k < 4; k++)
                    o << "      at " << r.second[k] << "\n";
                if (r.second.size() > 4)
                    o << "      ... " << r.second.size() - 4 << " more\n";
            }
        }
        inlined.erase(voice);
        if (!inlined.empty()) {
            o << "  inlined into " << voice << ":";
            for (auto &n : inlined)
                o << " " << n;
            o << "\n";
        } else if (!V->getSubprogram())
            o << "  (no debug info: build with s1cc build --debug for the C functions and lines)\n";
    }
    return o.str();
}

}  // namespace s1
