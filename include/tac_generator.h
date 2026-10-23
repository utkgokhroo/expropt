#ifndef TAC_GENERATOR_H
#define TAC_GENERATOR_H

#include <vector>
#include "ast.h"
#include "tac.h"

// Generates unoptimized three-address code from an already-typed AST.
//
// Precondition: the AST has already been through a clean semantic
// analysis pass -- every ExprNode::inferredType is set to a real type
// ("int"/"float"), not "invalid" or empty. The caller (main.cpp) is
// responsible for only invoking this when there are zero lexical,
// syntax, and semantic errors; this class does no error checking of its
// own and does not modify the AST.
//
// Traversal: genExpr() visits an expression's children before emitting
// the instruction for the expression itself (post-order). This alone is
// what gives:
//   - operator precedence/associativity preserved (already encoded in
//     the AST's shape by the parser -- no extra logic needed here)
//   - definitions before uses (a temp's defining instruction is always
//     pushed before any instruction that reads it)
//   - fresh, never-reused temporaries (one counter, only ever incremented)
class TACGenerator {
public:
    // Generates TAC for the whole program and returns the instruction list.
    std::vector<TACInstruction> generate(const ProgramNode& program);

private:
    std::vector<TACInstruction> instructions_;
    int tempCounter_ = 0;

    std::string newTemp();

    void genStatement(const AssignStmtNode& stmt);

    // Returns the operand text holding this expression's value. For a
    // literal or identifier, that's the literal/identifier text itself
    // and no instruction is emitted (there's nothing to compute). For a
    // unary or binary expression, a fresh temporary is allocated, one
    // instruction is emitted, and the temp's name is returned.
    std::string genExpr(const ExprNode& expr);
};

#endif // TAC_GENERATOR_H
