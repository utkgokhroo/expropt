#include "tac.h"

std::string formatTACInstruction(const TACInstruction& instr) {
    if (instr.op.empty()) {
        // Plain copy: result = arg1
        return instr.result + " = " + instr.arg1;
    }
    if (instr.arg2.empty()) {
        // Unary: result = op arg1
        return instr.result + " = " + instr.op + instr.arg1;
    }
    // Binary: result = arg1 op arg2
    return instr.result + " = " + instr.arg1 + " " + instr.op + " " + instr.arg2;
}
