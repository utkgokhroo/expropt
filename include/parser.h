#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <memory>
#include "token.h"
#include "ast.h"

// A single syntax error: what went wrong and on which line.
struct SyntaxError {
    std::string message;
    int line;
};

// Recursive-descent parser for the Phase 1 ExprOpt grammar:
//
//   program     -> statement* EOF
//   statement   -> IDENTIFIER ASSIGN expression SEMICOLON
//   expression  -> term ((PLUS | MINUS) term)*
//   term        -> factor ((MULTIPLY | DIVIDE) factor)*
//   factor      -> MINUS factor
//                | LPAREN expression RPAREN
//                | INTEGER_LITERAL
//                | FLOAT_LITERAL
//                | IDENTIFIER
//
// One function per grammar rule, matching the report's methodology.
//
// Error handling (panic-mode recovery, per the report): when a statement
// can't be parsed, the error is recorded and the parser discards tokens
// until it finds a SEMICOLON (which it consumes) or EOF, then resumes
// parsing at the next statement. This lets a single run report several
// syntax errors instead of stopping at the first one.
class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    // Parses the whole token stream and returns the Program AST node.
    // Never throws; check hasErrors()/getErrors() afterwards.
    std::unique_ptr<ProgramNode> parseProgram();

    bool hasErrors() const;
    const std::vector<SyntaxError>& getErrors() const;

private:
    std::vector<Token> tokens_;
    size_t pos_;
    std::vector<SyntaxError> errors_;

    // Thrown internally to unwind out of a broken statement so that
    // parseProgram() can synchronize() and move on. Never escapes
    // parseProgram() itself.
    struct ParseAbort {};

    const Token& peek() const;
    const Token& previous() const;
    bool isAtEnd() const;
    bool check(TokenType type) const;
    const Token& advance();

    // If the current token matches 'type', consumes and returns it.
    // Otherwise records a syntax error and throws ParseAbort.
    const Token& expect(TokenType type, const std::string& what);

    void recordError(const std::string& message, int line);
    void synchronize();

    std::unique_ptr<AssignStmtNode> parseStatement();
    std::unique_ptr<ExprNode> parseExpression();
    std::unique_ptr<ExprNode> parseTerm();
    std::unique_ptr<ExprNode> parseFactor();
};

#endif // PARSER_H
