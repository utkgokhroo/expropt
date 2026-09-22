#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <string>
#include <vector>
#include "ast.h"
#include "symbol_table.h"

// A single semantic error: what went wrong and on which line.
struct SemanticError {
    std::string message;
    int line;
};

// Walks the existing Phase 1 AST bottom-up, computing each expression
// node's type as a synthesized attribute (ExprNode::inferredType) and
// building the flat symbol table as it goes. It never changes the AST's
// shape -- only fills in inferredType on nodes that already exist.
//
// Checks performed (per the Phase 1 report's semantic rules):
//   1. Unknown identifier            -- identifier used before any assignment
//   2. Type mismatch on reassignment -- a variable's type is fixed by its
//                                        first assignment and can't change
//   3. Literal division by zero      -- divisor is the literal 0 or 0.0
//   4. Type promotion (bottom-up):
//        int  op int  -> int
//        int  /  int  -> int, with truncating integer division
//        either operand float -> float
//
// Error recovery: like the lexer and parser, this does not stop at the
// first problem. An identifier used before assignment or an expression
// built on one is given the sentinel type "invalid" so that later
// promotion/reassignment checks can silently skip it (the error was
// already reported once) instead of producing a cascade of follow-on
// errors from the same root cause.
class SemanticAnalyzer {
public:
    // Analyzes the whole program, filling in inferredType on every
    // expression node and populating the symbol table. Never throws;
    // check hasErrors()/getErrors() afterwards.
    void analyze(ProgramNode& program);

    bool hasErrors() const;
    const std::vector<SemanticError>& getErrors() const;
    const SymbolTable& getSymbolTable() const;

private:
    SymbolTable symbolTable_;
    std::vector<SemanticError> errors_;

    void analyzeStatement(AssignStmtNode& stmt);

    // Computes and stores the inferred type on 'expr' (and, recursively, on
    // every descendant expression), then returns that type. This is the
    // bottom-up / synthesized-attribute pass: each node's type is computed
    // strictly from its already-computed children.
    std::string analyzeExpr(ExprNode& expr);

    void checkDivisionByZero(const BinaryExprNode& bin);
    void recordError(const std::string& message, int line);
};

#endif // SEMANTIC_ANALYZER_H
