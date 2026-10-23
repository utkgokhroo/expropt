#ifndef TAC_OPTIMIZER_H
#define TAC_OPTIMIZER_H

#include <map>
#include <string>
#include <vector>
#include "tac.h"

// A single optimization transformation that occurred on one instruction.
struct OptimizationLogEntry {
    int line;
    std::string description; // e.g. "constant folded '2 + 3' -> '5'"
};

// Optimizes unoptimized TAC with a single deterministic forward pass.
//
// Scope (deliberately small, per the project's Phase 3 plan):
//   1. Constant folding       -- both operands literal -> compute directly
//   2. Constant propagation   -- substitute a name with its known constant
//                                 value before folding/identity is attempted
//   3. Identity simplification (right-hand only, exactly these four forms):
//        x + 0 -> x      x - 0 -> x      x * 1 -> x      x / 1 -> x
//
// Explicitly NOT implemented: x * 0 (unsafe for float NaN/Inf), dead code
// elimination, liveness/reaching-definitions analysis, SSA, CFG/loop
// optimization, register allocation, inlining, or anything else beyond the
// three rules above.
//
// Per-instruction order (approved design):
//   1. Propagate any known-constant operands.
//   2. If both (now-resolved) operands are literal -> constant fold.
//   3. Else, if exactly one operand is a literal matching an identity
//      pattern -> simplify to the other operand.
//   4. Update the known-constants table from the instruction's final form.
//
// The optimizer never mutates its input: optimize() takes the original
// TAC by const reference and returns a new, separate vector, so the
// unoptimized TAC remains available for side-by-side comparison. Does not
// touch the AST, lexer, parser, semantic analyzer, or TAC generator.
class TACOptimizer {
public:
    // Optimizes the given TAC and returns the optimized instruction list.
    std::vector<TACInstruction> optimize(const std::vector<TACInstruction>& original);

    const std::vector<OptimizationLogEntry>& getLog() const;

private:
    std::vector<OptimizationLogEntry> log_;
    std::map<std::string, std::string> knownConstants_; // name -> literal text

    void recordLog(int line, const std::string& description);

    // Processes one instruction against the current knownConstants_ state,
    // applying the four-step order described above, and returns the
    // transformed instruction.
    TACInstruction optimizeInstruction(const TACInstruction& instr);
};

#endif // TAC_OPTIMIZER_H
