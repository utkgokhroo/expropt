#include "tac_generator.h"

std::string TACGenerator::newTemp() {
    tempCounter_++;
    return "t" + std::to_string(tempCounter_);
}

std::vector<TACInstruction> TACGenerator::generate(const ProgramNode& program) {
    instructions_.clear();
    tempCounter_ = 0;
    for (const auto& stmt : program.statements) {
        genStatement(*stmt);
    }
    return instructions_;
}

// statement -> IDENTIFIER ASSIGN expression SEMICOLON
// Generate the RHS first (post-order), then emit the final copy into the
// real variable name.
void TACGenerator::genStatement(const AssignStmtNode& stmt) {
    std::string rhs = genExpr(*stmt.expr);
    instructions_.push_back(TACInstruction{
        stmt.identifier, rhs, "", "", stmt.expr->inferredType, stmt.line});
}

std::string TACGenerator::genExpr(const ExprNode& expr) {
    if (const auto* lit = dynamic_cast<const IntLiteralNode*>(&expr)) {
        return lit->value; // literal text used directly; no instruction needed
    }

    if (const auto* lit = dynamic_cast<const FloatLiteralNode*>(&expr)) {
        return lit->value;
    }

    if (const auto* id = dynamic_cast<const IdentifierNode*>(&expr)) {
        // Semantic analysis already guarantees this identifier was assigned
        // earlier, so its TAC-level definition already precedes this use.
        return id->name;
    }

    if (const auto* un = dynamic_cast<const UnaryExprNode*>(&expr)) {
        std::string operand = genExpr(*un->operand); // child first (post-order)
        std::string temp = newTemp();
        instructions_.push_back(TACInstruction{
            temp, operand, std::string(1, un->op), "", un->inferredType, un->line});
        return temp;
    }

    if (const auto* bin = dynamic_cast<const BinaryExprNode*>(&expr)) {
        std::string left = genExpr(*bin->left);   // left child first (post-order)
        std::string right = genExpr(*bin->right); // then right child
        std::string temp = newTemp();
        instructions_.push_back(TACInstruction{
            temp, left, std::string(1, bin->op), right, bin->inferredType, bin->line});
        return temp;
    }

    // Every ExprNode subtype is handled above; unreachable for the current
    // grammar, but keeps the function total.
    return "";
}
