#ifndef TOKEN_H
#define TOKEN_H

#include <string>

// All token kinds recognized by the ExprOpt lexer (Phase 1 grammar).
enum class TokenType {
    IDENTIFIER,
    INTEGER_LITERAL,
    FLOAT_LITERAL,

    ASSIGN,      // =
    PLUS,        // +
    MINUS,       // -
    MULTIPLY,    // *
    DIVIDE,      // /

    LPAREN,      // (
    RPAREN,      // )
    SEMICOLON,   // ;

    END_OF_FILE,
    UNKNOWN      // invalid/unrecognized character or malformed literal
};

// Human-readable name for a TokenType, used for debug printing and error messages.
std::string tokenTypeToString(TokenType type);

struct Token {
    TokenType type;
    std::string lexeme;  // raw text as it appeared in the source
    int line;            // 1-based line number where the token starts

    Token(TokenType type, std::string lexeme, int line)
        : type(type), lexeme(std::move(lexeme)), line(line) {}
};

#endif // TOKEN_H
