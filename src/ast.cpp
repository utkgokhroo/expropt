#include "ast.h"

void ASTNode::writeIndent(std::ostream& os, int indent) {
    for (int i = 0; i < indent; ++i) {
        os << "  ";
    }
}

void IntLiteralNode::print(std::ostream& os, int indent) const {
    writeIndent(os, indent);
    os << "IntLiteral " << value << "  (line " << line << ")\n";
}

void FloatLiteralNode::print(std::ostream& os, int indent) const {
    writeIndent(os, indent);
    os << "FloatLiteral " << value << "  (line " << line << ")\n";
}

void IdentifierNode::print(std::ostream& os, int indent) const {
    writeIndent(os, indent);
    os << "Identifier " << name << "  (line " << line << ")\n";
}

void UnaryExprNode::print(std::ostream& os, int indent) const {
    writeIndent(os, indent);
    os << "UnaryExpr " << op << "  (line " << line << ")\n";
    operand->print(os, indent + 1);
}

void BinaryExprNode::print(std::ostream& os, int indent) const {
    writeIndent(os, indent);
    os << "BinaryExpr " << op << "  (line " << line << ")\n";
    left->print(os, indent + 1);
    right->print(os, indent + 1);
}

void AssignStmtNode::print(std::ostream& os, int indent) const {
    writeIndent(os, indent);
    os << "AssignStmt " << identifier << " =  (line " << line << ")\n";
    expr->print(os, indent + 1);
}

void ProgramNode::print(std::ostream& os, int indent) const {
    writeIndent(os, indent);
    os << "Program\n";
    for (const auto& stmt : statements) {
        stmt->print(os, indent + 1);
    }
}
