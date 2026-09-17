#include "parser.h"

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)), pos_(0) {}

const Token& Parser::peek() const {
    return tokens_[pos_];
}

const Token& Parser::previous() const {
    return tokens_[pos_ - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

bool Parser::check(TokenType type) const {
    return peek().type == type;
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        pos_++;
    }
    return previous();
}

void Parser::recordError(const std::string& message, int line) {
    errors_.push_back(SyntaxError{message, line});
}

bool Parser::hasErrors() const {
    return !errors_.empty();
}

const std::vector<SyntaxError>& Parser::getErrors() const {
    return errors_;
}

const Token& Parser::expect(TokenType type, const std::string& what) {
    if (check(type)) {
        return advance();
    }
    std::string got = isAtEnd() ? "end of file" : ("'" + peek().lexeme + "'");
    recordError("Expected " + what + " but found " + got, peek().line);
    throw ParseAbort{};
}

// Panic-mode recovery: discard tokens until SEMICOLON or EOF. A SEMICOLON,
// if found, is consumed (it closes off the broken statement); EOF is left
// alone so the main parseProgram() loop can stop cleanly.
void Parser::synchronize() {
    while (!check(TokenType::SEMICOLON) && !isAtEnd()) {
        advance();
    }
    if (check(TokenType::SEMICOLON)) {
        advance();
    }
}

// program -> statement* EOF
std::unique_ptr<ProgramNode> Parser::parseProgram() {
    auto program = std::make_unique<ProgramNode>();
    while (!isAtEnd()) {
        try {
            program->statements.push_back(parseStatement());
        } catch (const ParseAbort&) {
            synchronize();
        }
    }
    return program;
}

// statement -> IDENTIFIER ASSIGN expression SEMICOLON
std::unique_ptr<AssignStmtNode> Parser::parseStatement() {
    const Token& nameTok = expect(TokenType::IDENTIFIER, "identifier at start of statement");
    std::string name = nameTok.lexeme;
    int line = nameTok.line;

    expect(TokenType::ASSIGN, "'='");
    auto expr = parseExpression();
    expect(TokenType::SEMICOLON, "';'");

    return std::make_unique<AssignStmtNode>(name, std::move(expr), line);
}

// expression -> term ((PLUS | MINUS) term)*
std::unique_ptr<ExprNode> Parser::parseExpression() {
    auto left = parseTerm();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        const Token& opTok = advance();
        auto right = parseTerm();
        left = std::make_unique<BinaryExprNode>(opTok.lexeme[0], std::move(left), std::move(right), opTok.line);
    }
    return left;
}

// term -> factor ((MULTIPLY | DIVIDE) factor)*
std::unique_ptr<ExprNode> Parser::parseTerm() {
    auto left = parseFactor();
    while (check(TokenType::MULTIPLY) || check(TokenType::DIVIDE)) {
        const Token& opTok = advance();
        auto right = parseFactor();
        left = std::make_unique<BinaryExprNode>(opTok.lexeme[0], std::move(left), std::move(right), opTok.line);
    }
    return left;
}

// factor -> MINUS factor
//         | LPAREN expression RPAREN
//         | INTEGER_LITERAL
//         | FLOAT_LITERAL
//         | IDENTIFIER
std::unique_ptr<ExprNode> Parser::parseFactor() {
    if (check(TokenType::MINUS)) {
        const Token& opTok = advance();
        auto operand = parseFactor(); // right-recursive: handles --x, -(x), etc.
        return std::make_unique<UnaryExprNode>(opTok.lexeme[0], std::move(operand), opTok.line);
    }

    if (check(TokenType::LPAREN)) {
        advance();
        auto expr = parseExpression();
        expect(TokenType::RPAREN, "')'");
        return expr; // parentheses only group; no separate AST node needed
    }

    if (check(TokenType::INTEGER_LITERAL)) {
        const Token& tok = advance();
        return std::make_unique<IntLiteralNode>(tok.lexeme, tok.line);
    }

    if (check(TokenType::FLOAT_LITERAL)) {
        const Token& tok = advance();
        return std::make_unique<FloatLiteralNode>(tok.lexeme, tok.line);
    }

    if (check(TokenType::IDENTIFIER)) {
        const Token& tok = advance();
        return std::make_unique<IdentifierNode>(tok.lexeme, tok.line);
    }

    std::string got = isAtEnd() ? "end of file" : ("'" + peek().lexeme + "'");
    recordError("Expected expression but found " + got, peek().line);
    throw ParseAbort{};
}
