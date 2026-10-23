#include "tac_optimizer.h"
#include <cctype>
#include <sstream>

namespace {

// Recognizes literal operand text: optional leading '-', at least one
// digit, optionally followed by '.' and more digits. Covers both the
// literal text the TAC generator emits (always non-negative, since unary
// minus is always its own instruction) and the negative literals the
// optimizer itself may produce when folding a unary minus.
bool isLiteral(const std::string& s) {
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0] == '-') i = 1;
    if (i >= s.size() || !std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    bool seenDot = false;
    for (; i < s.size(); ++i) {
        if (s[i] == '.') {
            if (seenDot) return false;
            seenDot = true;
        } else if (!std::isdigit(static_cast<unsigned char>(s[i]))) {
            return false;
        }
    }
    return true;
}

// Formats a folded double so it still reads as a float literal (always at
// least one decimal digit), e.g. 7.0 rather than a bare "7".
std::string formatFloat(double v) {
    std::ostringstream out;
    out << v;
    std::string s = out.str();
    if (s.find('.') == std::string::npos &&
        s.find('e') == std::string::npos &&
        s.find("inf") == std::string::npos &&
        s.find("nan") == std::string::npos) {
        s += ".0";
    }
    return s;
}

// Computes 'a op b' at compile time, honoring the instruction's own type
// (reused, not re-derived) to choose integer vs. floating-point
// arithmetic. Division by a literal zero is defensively left unfolded
// (returns false) -- this is unreachable in practice, since literal
// division by zero is already a semantic error and TAC/optimization only
// ever run on a semantically clean program, but the guard costs nothing.
bool foldBinary(const std::string& a, char op, const std::string& b,
                 const std::string& type, std::string& outResult) {
    if (type == "int") {
        long long x = std::stoll(a);
        long long y = std::stoll(b);
        long long r = 0;
        switch (op) {
            case '+': r = x + y; break;
            case '-': r = x - y; break;
            case '*': r = x * y; break;
            case '/':
                if (y == 0) return false;
                r = x / y; // truncating integer division
                break;
            default: return false;
        }
        outResult = std::to_string(r);
        return true;
    }

    // float
    double x = std::stod(a);
    double y = std::stod(b);
    double r = 0.0;
    switch (op) {
        case '+': r = x + y; break;
        case '-': r = x - y; break;
        case '*': r = x * y; break;
        case '/':
            if (y == 0.0) return false;
            r = x / y;
            break;
        default: return false;
    }
    outResult = formatFloat(r);
    return true;
}

std::string foldUnary(const std::string& a, const std::string& type) {
    if (type == "int") {
        return std::to_string(-std::stoll(a));
    }
    return formatFloat(-std::stod(a));
}

// Right-hand identity check only, exactly the four approved forms:
// x+0, x-0, x*1, x/1. 'rhsLiteral' is already confirmed literal by the
// caller.
bool isIdentityRight(char op, const std::string& rhsLiteral) {
    double v = std::stod(rhsLiteral);
    if (op == '+' || op == '-') return v == 0.0;
    if (op == '*' || op == '/') return v == 1.0;
    return false;
}

} // namespace

void TACOptimizer::recordLog(int line, const std::string& description) {
    log_.push_back(OptimizationLogEntry{line, description});
}

const std::vector<OptimizationLogEntry>& TACOptimizer::getLog() const {
    return log_;
}

std::vector<TACInstruction> TACOptimizer::optimize(const std::vector<TACInstruction>& original) {
    log_.clear();
    knownConstants_.clear();

    std::vector<TACInstruction> optimized;
    optimized.reserve(original.size());
    for (const TACInstruction& instr : original) {
        optimized.push_back(optimizeInstruction(instr));
    }
    return optimized;
}

TACInstruction TACOptimizer::optimizeInstruction(const TACInstruction& instr) {
    TACInstruction out = instr;

    if (instr.op.empty()) {
        // ---- Plain copy: result = arg1 ----
        // Step 1: propagate.
        auto it = knownConstants_.find(instr.arg1);
        if (it != knownConstants_.end()) {
            recordLog(instr.line, "propagated '" + instr.arg1 + "' -> '" + it->second +
                                       "' in computation of '" + instr.result + "'");
            out.arg1 = it->second;
        }
        // No folding/identity applies to a plain copy.

    } else if (instr.arg2.empty()) {
        // ---- Unary minus: result = -arg1 ----
        std::string a = instr.arg1;

        // Step 1: propagate.
        auto it = knownConstants_.find(a);
        if (it != knownConstants_.end()) {
            recordLog(instr.line, "propagated '" + a + "' -> '" + it->second +
                                       "' in computation of '" + instr.result + "'");
            a = it->second;
        }

        // Step 2: fold if now literal. (No identity rule applies to unary
        // minus -- it's outside the four approved patterns.)
        if (isLiteral(a)) {
            std::string folded = foldUnary(a, instr.type);
            recordLog(instr.line, "constant folded '-" + a + "' -> '" + folded + "'");
            out = TACInstruction{instr.result, folded, "", "", instr.type, instr.line};
        } else {
            out.arg1 = a;
        }

    } else {
        // ---- Binary: result = arg1 op arg2 ----
        std::string a = instr.arg1;
        std::string b = instr.arg2;
        char op = instr.op[0];

        // Step 1: propagate both operands.
        auto itA = knownConstants_.find(a);
        if (itA != knownConstants_.end()) {
            recordLog(instr.line, "propagated '" + a + "' -> '" + itA->second +
                                       "' in computation of '" + instr.result + "'");
            a = itA->second;
        }
        auto itB = knownConstants_.find(b);
        if (itB != knownConstants_.end()) {
            recordLog(instr.line, "propagated '" + b + "' -> '" + itB->second +
                                       "' in computation of '" + instr.result + "'");
            b = itB->second;
        }

        if (isLiteral(a) && isLiteral(b)) {
            // Step 2: both literal -> constant fold.
            std::string folded;
            if (foldBinary(a, op, b, instr.type, folded)) {
                recordLog(instr.line, "constant folded '" + a + " " + instr.op + " " + b +
                                           "' -> '" + folded + "'");
                out = TACInstruction{instr.result, folded, "", "", instr.type, instr.line};
            } else {
                // Defensive only (literal zero divisor) -- see foldBinary's
                // comment; unreachable given the upstream semantic check.
                out.arg1 = a;
                out.arg2 = b;
            }
        } else if (!isLiteral(a) && isLiteral(b) && isIdentityRight(op, b)) {
            // Step 3: identity simplification (right-hand only).
            recordLog(instr.line, "identity simplification '" + a + " " + instr.op + " " + b +
                                       "' -> '" + a + "'");
            out = TACInstruction{instr.result, a, "", "", instr.type, instr.line};
        } else {
            // Neither fully foldable nor identity-eligible: keep as a
            // binary instruction with whatever operands propagation did
            // resolve.
            out.arg1 = a;
            out.arg2 = b;
        }
    }

    // Step 4: update the known-constants table from the final form.
    if (out.op.empty() && isLiteral(out.arg1)) {
        knownConstants_[out.result] = out.arg1;
    } else {
        knownConstants_.erase(out.result); // handles reassignment too
    }

    return out;
}
