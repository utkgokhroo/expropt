#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <ostream>

// Base class for every AST node. Each node stores the source line it came
// from and knows how to print itself (and its children) as an indented
// tree, for debugging and viva demonstration.
struct ASTNode {
    int line;
    explicit ASTNode(int line) : line(line) {}
    virtual ~ASTNode() = default;
    virtual void print(std::ostream& os, int indent) const = 0;

protected:
    static void writeIndent(std::ostream& os, int indent);
};

// ---- Expression nodes ----
// The grammar's expression/term/factor layers only exist to encode
// precedence during parsing; they all collapse into this one ExprNode
// hierarchy in the AST.

struct ExprNode : ASTNode {
    using ASTNode::ASTNode;
};

struct IntLiteralNode : ExprNode {
    std::string value; // kept as the raw lexeme; no evaluation at this phase
    IntLiteralNode(std::string value, int line) : ExprNode(line), value(std::move(value)) {}
    void print(std::ostream& os, int indent) const override;
};

struct FloatLiteralNode : ExprNode {
    std::string value;
    FloatLiteralNode(std::string value, int line) : ExprNode(line), value(std::move(value)) {}
    void print(std::ostream& os, int indent) const override;
};

struct IdentifierNode : ExprNode {
    std::string name;
    IdentifierNode(std::string name, int line) : ExprNode(line), name(std::move(name)) {}
    void print(std::ostream& os, int indent) const override;
};

// factor -> MINUS factor  (unary minus only, per the grammar)
struct UnaryExprNode : ExprNode {
    char op; // always '-'
    std::unique_ptr<ExprNode> operand;
    UnaryExprNode(char op, std::unique_ptr<ExprNode> operand, int line)
        : ExprNode(line), op(op), operand(std::move(operand)) {}
    void print(std::ostream& os, int indent) const override;
};

// Built by the while-loops in parseExpression()/parseTerm(), so these are
// always left-associative, matching the report's grammar note.
struct BinaryExprNode : ExprNode {
    char op; // '+' '-' '*' '/'
    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
    BinaryExprNode(char op, std::unique_ptr<ExprNode> left, std::unique_ptr<ExprNode> right, int line)
        : ExprNode(line), op(op), left(std::move(left)), right(std::move(right)) {}
    void print(std::ostream& os, int indent) const override;
};

// ---- Statement node ----

// statement -> IDENTIFIER ASSIGN expression SEMICOLON
struct AssignStmtNode : ASTNode {
    std::string identifier;
    std::unique_ptr<ExprNode> expr;
    AssignStmtNode(std::string identifier, std::unique_ptr<ExprNode> expr, int line)
        : ASTNode(line), identifier(std::move(identifier)), expr(std::move(expr)) {}
    void print(std::ostream& os, int indent) const override;
};

// ---- Program node (AST root) ----

// program -> statement* EOF
struct ProgramNode : ASTNode {
    std::vector<std::unique_ptr<AssignStmtNode>> statements;
    ProgramNode() : ASTNode(0) {}
    void print(std::ostream& os, int indent) const override;
};

#endif // AST_H
