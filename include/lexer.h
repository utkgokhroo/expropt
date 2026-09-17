#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "token.h"

// A single lexical error: an invalid character or a malformed numeric
// literal, recorded with the line it occurred on. Recording an error does
// not stop scanning -- see Lexer::tokenize().
struct LexError {
    std::string message;
    int line;
};

// Converts ExprOpt source text into a flat stream of tokens.
// Single-pass, one character of lookahead beyond the current position.
//
// Error handling: per the Phase 1 design, the lexer does NOT stop at the
// first bad character or malformed literal. It records each problem in
// errors_ and keeps scanning, so a single run can report every lexical
// error in the file and still return the tokens for everything that was
// valid. Call tokenize() first, then check hasErrors()/getErrors().
class Lexer {
public:
    explicit Lexer(std::string source);

    // Scans the entire source and returns the token stream (valid tokens
    // only), terminated with a single END_OF_FILE token. Never throws;
    // any lexical problems are collected in errors_ instead.
    std::vector<Token> tokenize();

    bool hasErrors() const;
    const std::vector<LexError>& getErrors() const;

private:
    std::string source_;
    size_t pos_;     // index of the next unconsumed character
    int line_;       // current line number (1-based)
    std::vector<LexError> errors_;

    bool isAtEnd() const;
    char peek() const;       // current character, '\0' if at end
    char peekNext() const;   // lookahead one past current, '\0' if at end
    char advance();          // consume and return current character

    void skipWhitespaceAndNewlines();
    void recordError(const std::string& message, int line);

    Token scanIdentifier();
    // Returns true and fills 'out' with a valid token, or returns false
    // if the numeric literal was malformed (an error has already been
    // recorded and the bad text has been consumed).
    bool scanNumber(Token& out);
};

#endif // LEXER_H
