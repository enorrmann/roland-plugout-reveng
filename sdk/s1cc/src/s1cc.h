// s1cc: C / C++ (through LLVM IR) -> microcode of the Roland BMC DSPs (docs/S1CC.md).
//
// Passes, all on the same linked LLVM module (clang -O2 of the voice program and its sources):
//   graph.cpp   the voice functions as a dataflow graph: if-conversion (no branches on the DSP), state words
//               (file-scope variables, forwarded through the sample), constants folded from const tables,
//               signedness of every integer, and the "islands": sub-graphs that the DSP cannot run as they are
//               (computed-index table reads, libm calls, divisions) and that depend on one value only
//   prof.cpp    `s1cc prof`: the module with a call after every value (range, integer overflow) and one native
//               function per island candidate; linked with runtime/s1rt.c it runs the profiling scenarios
//   fit.cpp     least squares: polynomial, exp2 of a polynomial, with saturation plateaus
//   lower.cpp   `s1cc lower`: every value as (register, scale, offset) - V = (r - o) s - so integer fixed point,
//               shifts and offsets cost nothing; wrapping arithmetic and bit masks become ph_wrap; islands
//               become fitted functions; output: the kernel file (.s1k)
//   kernel.cpp  values -> registers -> one-unit lines with load fillers (asm/dspsched.py packs them)
#pragma once
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace llvm {
class Function;
class Module;
class Value;
class Instruction;
class GlobalVariable;
}  // namespace llvm

namespace s1 {

// every error and warning has a code (E101, W905): its cause and fix are in docs/S1CC.md §9.2 under that code
[[noreturn]] void fail(const std::string &code, const std::string &msg);
void warn(const std::string &code, const std::string &msg);
std::string where(const llvm::Value *v);   // "file:line in f, inlined at file:line in g" (debug info) or its IR text
int debugLevel();                           // S1CC_DEBUG: 0 off, 1 diagnostics (s1cc build --debug), 2 + every node
// s1cc diag: the calls and instructions of the voice functions s1cc cannot compile, by C function, with the reason;
// remarks = opt's -pass-remarks-missed=inline output (why LLVM kept a call), "" for none
std::string diagnose(llvm::Module &M, const std::string &remarks);

// ---- dataflow graph ----------------------------------------------------------------------------------------
enum class NK {
    Const,      // c
    Input,      // s1_pitch / s1_gate / s1_trig (name)
    Increment,  // s1_increment (computed from the pitch)
    Param,      // s1_param(name, idx)
    State,      // value of a state word at the start of the sample
    Import,     // s1_import(idx)
    XIn,        // the argument of S1_DSP_B
    Op,         // an operation (op = LLVM opcode / intrinsic / synthetic name)
};

struct Node {
    int id = -1;
    NK kind = NK::Op;
    std::string op;              // Op: "add", "fmul", "icmp.slt", "smin", "s1_wrap", "sel", "band", ...
    std::vector<Node *> in;
    double c = 0;                // Const value; for Param / Import / Input: index
    std::string name;            // Input / Param name
    int state = -1;              // State: state index
    bool isInt = false;
    int bits = 0;                // integer width (1 = bool)
    bool isUnsigned = false;     // integer interpretation (from its uses; graph.cpp)
    bool bad = false;            // not translatable as it is (table read, libm, division...): must be in an island
    llvm::Value *v = nullptr;    // origin in the IR (nullptr for synthetic nodes)
    int prof = -1;               // profile record id (-1: none)
    std::vector<Node *> users;
    bool live = false;
    // speculated: this value only feeds one arm of a select (clang computed it for every sample); the profile also
    // records it when that arm is taken (cond, condTrue), and the lowering limits it to that range
    Node *specSel = nullptr;
    bool specArmTrue = false;
    bool specFrontier = false;   // its operands are not speculated with it: the lowering clamps here
    bool soft = false;           // bad, but lowerable directly when no island fits it (recip: 1/y by recipseed)
    int demand = 0;              // only the low `demand` bits are used (LLVM DemandedBits; 0 = all): the lowering may
                                 // keep the value modulo 2^demand, its smallest representative (§7.2)
};

struct StateVar {
    int index = 0;
    std::string name;
    llvm::GlobalVariable *gv = nullptr;
    long offset = 0;
    bool isInt = false;
    int bits = 0;
    bool isUnsigned = false;
    double init = 0;
    std::string dsp;             // "A" / "B"
    Node *final = nullptr;       // value at the end of the sample (stored back to the ring)
};

struct Island {
    int id = 0;
    Node *root = nullptr;
    Node *out = nullptr;
    std::vector<Node *> members; // in order (root excluded)
    bool cloneable = true;
};

struct Dsp {
    std::string name;            // "A" / "B"
    llvm::Function *fn = nullptr;
    std::vector<std::unique_ptr<Node>> nodes;   // topological order
    Node *ret = nullptr;
    std::map<int, Node *> exports;
    std::vector<int> states;     // indices of the state words this DSP writes
    std::vector<Island> islands; // candidates (several roots per bad node)
};

struct Program {
    llvm::Module *M = nullptr;
    std::vector<StateVar> states;
    Dsp A, B;                    // B.fn == nullptr: no S1_DSP_B
    std::map<std::string, llvm::GlobalVariable *> params;   // name -> descriptor
    std::map<std::string, int> paramCount;
    // knob-derived columns (§4.6): `prof sweep` values, record id -> knob table -> value at knob 0..255; build() turns
    // every value that depends on one knob only into a table of its own (derived: name -> 256 values, base: its knob)
    std::map<int, std::map<std::string, std::vector<double>>> sweep;
    std::map<std::string, std::vector<double>> derived;
    std::map<std::string, std::string> derivedBase;
};
void assignProfIds(Program &P);   // graph.cpp: the record ids `s1cc prof` gives (and lower reads)

void build(Program &P, llvm::Module &M);    // graph.cpp
void relive(Program &P, Dsp &S);            // graph.cpp: live nodes and stored state words again, from the outputs

// ---- profile (written by runtime/s1rt.c) ----------------------------------------------------------------------
struct Range {
    double lo = 1e300, hi = -1e300;
    bool seen() const { return lo <= hi; }
    void add(double x) { if (x < lo) lo = x; if (x > hi) hi = x; }
};

struct Profile {
    std::map<int, Range> sr, ur;            // signed / unsigned view of every record id
    std::map<int, Range> csr, cur;          // the same, only when the select arm it feeds is taken
    std::map<int, bool> sovf, uovf;         // the operation wrapped (signed / unsigned view)
    std::map<int, std::vector<std::pair<double, double>>> island;   // island id -> samples (x, y)
    std::map<int, double> stateInit;        // state index -> value after S1_INIT
    std::map<int, Range> stateRange;        // state index -> range at the start of a sample (all scenarios)
    std::map<int, std::vector<unsigned>> hist;   // divisor record id -> counts: [0..127] positive octaves
                                                 // -64..63, [128..255] negative, [256] zero
    void read(const std::string &path);
};

// ---- fitting ----------------------------------------------------------------------------------------------------
struct Fit {
    std::string model;           // "poly" / "exp2" / "hinge"
    std::vector<double> c;       // coefficients in t (power basis, c[0] constant); hinge: c0 + c1 t
    std::vector<double> knots, w;   // hinge: + sum w_j max(0, t - knots_j) (a linear spline: folders, zigzags)
    double ta = 1, tb = 0;       // t = ta r + tb (r = the root's register value)
    double lo = -1e300, hi = 1e300;   // output clamp (saturation plateaus), in output units
    bool clampLo = false, clampHi = false;
    double expShift = 0;         // exp2 model: y = 2^(P(t) - shift) * 2^shift * sign
    double sign = 1;
    double err = 0;              // 99.5th percentile of |error| / output span
    double errMax = 0;           // max |error| / output span
    std::string metric = "span"; // "span": |error| / span; "log2": |log2(fit / y)| (octaves), for y > 0 over > 3 octaves
    int cost = 0;                // ops
};
Fit fitIsland(const std::vector<std::pair<double, double>> &xy, double tolSpan, double tolLog, int maxDeg,
              int maxKnots = 32);   // x, y in V units

// ---- kernel (register allocation, one-unit lines) ------------------------------------------------------------
struct KVal;
struct KOperand {
    KVal *v = nullptr;           // value (nullptr: immediate)
    bool neg = false;
    double imm = 0;
    static KOperand of(KVal *x) { KOperand o; o.v = x; return o; }
    static KOperand negOf(KVal *x) { KOperand o; o.v = x; o.neg = true; return o; }
    static KOperand im(double x) { KOperand o; o.imm = x; return o; }
};

struct KVal { int n = 0; int reg = -1; std::string name; };

struct KOp {
    std::string kind;            // "op" / "ld" / "st" / "comment" / "multi" (a native op that writes more than its
                                 // destination: fmt = its lines, {d} / {d1} = r(D) / r(D+1), {0} = the argument)
    KVal *d2 = nullptr;          // multi + clobber: the second result, in the argument's register (expsplit: frac)
    bool pairD = false;          // multi: r(D + 1) is written too (a scratch register next to D)
    bool clobber = false;        // multi: the argument's register is written (the argument dies here; a copy first
                                 // when it is read later)
    std::string fmt;             // "mul {d}, {0}, {1}" ...
    KVal *d = nullptr;
    std::vector<KOperand> args;
    KVal *acc = nullptr;
    std::string addr;            // ld / st: address text (may hold placeholders)
    std::string tag;             // the C line that made it (report: cost by C line)
};

class Kernel {
public:
    explicit Kernel(std::vector<int> regs) : regs_(std::move(regs)) {}
    KVal *load(const std::string &addr, const std::string &name = "");
    void store(KVal *v, const std::string &addr);
    void comment(const std::string &text);
    KVal *mov(KOperand y);
    KVal *mul(KVal *x, KOperand y);
    KVal *add(KVal *x, KOperand y);
    KVal *sub(KVal *x, KOperand y);       // x - y
    KVal *mad(KVal *acc, KVal *x, KOperand y);
    KVal *ymac(KVal *acc, KVal *x, KOperand y);
    KVal *min(KVal *x, KOperand y);
    KVal *max(KVal *x, KOperand y);
    KVal *wrap(KVal *x);
    KVal *tri(KVal *x);
    KVal *clamp(KVal *x);
    KVal *exp2(KVal *x, int extraSquarings = 0);
    KVal *twoMinus(KVal *x);                        // 2 - x (subxy d, x, #2.0)
    // the DSP's native ops (DSP_HARDWARE.md §5, measured exact on the SYSTEM-8 master: re/dspnative.py)
    KVal *recip(KVal *y);                           // 1/y: recipseed + 3 newton (1e-8; |y| in [2^-9, 512))
    std::pair<KVal *, KVal *> expsplit(KVal *z);   // (e^floor(z), frac(z)), z in [-19, 7)
    std::pair<KVal *, KVal *> log2split(KVal *y);  // (e, m) with y = m 2^e, m in [0.75, 1.5), y > 0
    KVal *exp2split(KVal *t, double shift);         // 2^(t + shift) for t + shift in [-27, 8]: expsplit + degree 4
    KVal *floor(KVal *x);                           // SFU floor / ceil / sign (+-1, 0)
    KVal *ceil(KVal *x);
    KVal *sign(KVal *x);
    KVal *shift(KVal *x, int k);                    // x 2^k (shl / shr / shrn: unit B, two's complement)
    KVal *mac2(KVal *a, KVal *b, KVal *c, KVal *d, KVal *e = nullptr);   // a b + c d (+ e): one MAC
    KVal *sum3(KVal *a, KVal *b, KVal *c);          // a + b + c: sum8
    std::vector<std::string> lines();
    int units() const;
    int spills() const { return spills_; }
    size_t opCount() const { return ops_.size(); }
    std::string tag;                                   // set by the lowering: the C line of the ops that follow
    std::map<std::string, int> liveByTag;              // after lines(): ops kept per tag   // ops emitted so far (unused ones are dropped by lines())
private:
    KVal *val(const std::string &name = "");
    KVal *op(const std::string &fmt, std::vector<KOperand> args, KVal *acc = nullptr);
    KVal *multi(const std::string &fmt, KVal *arg, bool pairD, bool clobber, bool second);
    std::vector<int> regs_;
    std::vector<KOp> ops_;
    std::vector<std::unique_ptr<KVal>> vals_;
    int spills_ = 0;
};

std::string fmtImm(double x);

// ---- lowering ---------------------------------------------------------------------------------------------------
struct LowerOptions {
    std::vector<int> regs = {0, 1, 2, 3, 4, 7};
    std::vector<int> regsA, regsB;   // --regs-a / --regs-b: for one DSP only (SYSTEM-S1CC PAIRS=1: DSP A in r0..r3)
    double fitTol = 3e-3;        // span metric
    int maxKnots = 32;           // hinge fits: at most this many knots (2 ops each)
    double fitTolLog = 0.0005;   // log2 metric (octaves; 0.0005 = 0.6 cent: increments are pitches)
    bool recipseed = true;       // 1/y by the SFU seed (proven on the SYSTEM-8 only); false (--no-recipseed): octave
                                 // normalisation with proven ops (experimental: less exact, more ops)
    bool native = true;          // the DSP's native ops where they are shorter: SFU exp split for exp2, floor / ceil,
                                 // the MAC for s1_mac2 (--no-native: the older sequences of mul / mad / addxy)
    bool foldBranches = true;    // a compare with one value in every profiling scenario is that constant (W909)
    bool exactInit = true;       // state words with a non-zero start: first-sample flag + one mad each
    bool trace = false;          // debugging: every lowered value also stored to {D:<record id>} (s1cc check --trace)
    int maxDeg = 11;
};

struct ParamInfo { std::string name; int count = 0; double scale = 1; };

std::string lower(Program &P, const Profile &prof, const std::map<std::string, std::vector<double>> &tables,
                  const LowerOptions &opt, std::string &report);

// ---- profiling instrumentation -----------------------------------------------------------------------------------
void instrument(Program &P);    // prof.cpp: modifies P.M

// ---- loops (loops.cpp) ----------------------------------------------------------------------------------------------
int devirtualize(llvm::Module &M);   // devirt.cpp: indirect calls with known targets -> direct; returns how many
int instrumentTrips(llvm::Module &M);                                               // returns the number of loops
void unrollBounded(llvm::Module &M, const std::map<int, int> &trips, std::string &report);

}  // namespace s1
