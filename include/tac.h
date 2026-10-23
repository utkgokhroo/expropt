#ifndef TAC_H
#define TAC_H

#include <string>
#include <vector>

// One three-address-code instruction: result = arg1 op arg2
//
//   op == ""            -> plain copy:   result = arg1          (arg2 unused)
//   op != "" && arg2==""-> unary minus:  result = -arg1
//   op != "" && arg2!=""-> binary op:    result = arg1 op arg2
//
// 'type' is the result's type ("int"/"float"), reused directly from the
// AST's ExprNode::inferredType (set earlier by the semantic analyzer).
// It is kept here for traceability but is not part of the printed TAC
// text -- see formatTACInstruction().
struct TACInstruction {
    std::string result;
    std::string arg1;
    std::string op;
    std::string arg2;
    std::string type;
    int line;
};

// Formats one instruction as plain TAC text, e.g. "t1 = y * 2",
// "t2 = -x", or "z = t2". Deliberately leaves out 'type' and 'line' to
// keep the printed TAC matching the report's plain conceptual examples.
std::string formatTACInstruction(const TACInstruction& instr);

#endif // TAC_H
