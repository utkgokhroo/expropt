#include "semantic_analyzer.h"
#include <string>

// Type names used throughout this file. "invalid" is the error-propagation
// sentinel: it marks an expression built on an already-reported error, so
// later checks can skip it instead of reporting a confusing follow-on error
// about the same root cause.
namespace {
const std::string TYPE_INT = "int";
const std::string TYPE_FLOAT = "float";
const std::string TYPE_INVALID = "invalid";
}

void SemanticAnalyzer::recordError(const std::string& message, int line) {
    errors_.push_back(SemanticError{message, line});
}

bool SemanticAnalyzer::hasErrors() const {
    return !errors_.empty();
}

const std::vector<SemanticError>& SemanticAnalyzer::getErrors() const {
    return errors_;
}

const SymbolTable& SemanticAnalyzer::getSymbolTable() const {
    return symbolTable_;
}

void SemanticAnalyzer::analyze(ProgramNode& program) {
    for (auto& stmt : program.statements) {
        analyzeStatement(*stmt);
    }
}

// statement -> IDENTIFIER ASSIGN expression SEMICOLON
//
// First occurrence of the identifier establishes its type (insert()).
// Every later occurrence must match that fixed type (recordReassignment()
// always updates lastAssignLine; a mismatch is reported but the stored
// type is never changed).
void SemanticAnalyzer::analyzeStatement(AssignStmtNode& stmt) {
    std::string exprType = analyzeExpr(*stmt.expr);

    const SymbolEntry* existing = symbolTable_.lookup(stmt.identifier);

    if (existing == nullptr) {
        // First assignment: the variable's type is whatever the RHS
        // evaluated to (unless the RHS was already invalid, in which case
        // there's nothing new to report -- the root cause was already
        // flagged inside analyzeExpr).
        symbolTable_.insert(stmt.identifier, exprType, stmt.line);
        return;
    }

    // Reassignment: type must match what was fixed at first assignment.
    if (exprType != TYPE_INVALID && existing->type != exprType) {
        recordError(
            "Type mismatch: cannot assign '" + exprType + "' to '" + stmt.identifier +
                "' (previously declared as '" + existing->type + "' at line " +
                std::to_string(existing->firstAssignLine) + ")",
            stmt.line);
    }
    symbolTable_.recordReassignment(stmt.identifier, stmt.line);
}

// Bottom-up (synthesized-attribute) type inference: each node's type is
// computed strictly from its already-computed children, then stored on
// the node itself via inferredType.
std::string SemanticAnalyzer::analyzeExpr(ExprNode& expr) {
    if (auto* lit = dynamic_cast<IntLiteralNode*>(&expr)) {
        lit->inferredType = TYPE_INT;
        return TYPE_INT;
    }

    if (auto* lit = dynamic_cast<FloatLiteralNode*>(&expr)) {
        lit->inferredType = TYPE_FLOAT;
        return TYPE_FLOAT;
    }

    if (auto* id = dynamic_cast<IdentifierNode*>(&expr)) {
        const SymbolEntry* entry = symbolTable_.lookup(id->name);
        if (entry == nullptr) {
            recordError("Unknown identifier '" + id->name + "'", id->line);
            id->inferredType = TYPE_INVALID;
        } else {
            id->inferredType = entry->type;
        }
        return id->inferredType;
    }

    if (auto* un = dynamic_cast<UnaryExprNode*>(&expr)) {
        // Unary minus doesn't change type: -int is int, -float is float.
        std::string operandType = analyzeExpr(*un->operand);
        un->inferredType = operandType;
        return operandType;
    }

    if (auto* bin = dynamic_cast<BinaryExprNode*>(&expr)) {
        std::string leftType = analyzeExpr(*bin->left);
        std::string rightType = analyzeExpr(*bin->right);

        if (bin->op == '/') {
            checkDivisionByZero(*bin);
        }

        std::string resultType;
        if (leftType == TYPE_INVALID || rightType == TYPE_INVALID) {
            // An operand already carries an error; don't pile on another
            // "type mismatch"-style message for the same root cause.
            resultType = TYPE_INVALID;
        } else if (leftType == TYPE_FLOAT || rightType == TYPE_FLOAT) {
            resultType = TYPE_FLOAT;
        } else {
            resultType = TYPE_INT; // int op int -> int (and int/int truncates)
        }

        bin->inferredType = resultType;
        return resultType;
    }

    // Every ExprNode subtype is handled above; this is unreachable for the
    // current grammar, but keeps the function total.
    expr.inferredType = TYPE_INVALID;
    return TYPE_INVALID;
}

// Only catches a *literal* zero divisor (e.g. "x / 0" or "x / 0.0"), as
// specified -- a variable that might be zero at runtime can't be detected
// without executing the program, which is out of scope for this phase.
void SemanticAnalyzer::checkDivisionByZero(const BinaryExprNode& bin) {
    if (const auto* rlit = dynamic_cast<const IntLiteralNode*>(bin.right.get())) {
        if (std::stoll(rlit->value) == 0) {
            recordError("Division by zero", bin.line);
        }
    } else if (const auto* rlit = dynamic_cast<const FloatLiteralNode*>(bin.right.get())) {
        if (std::stod(rlit->value) == 0.0) {
            recordError("Division by zero", bin.line);
        }
    }
}
