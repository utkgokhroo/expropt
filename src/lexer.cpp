#include "lexer.h"
#include <cctype>

Lexer::Lexer(std::string source)
    : source_(std::move(source)), pos_(0), line_(1) {}

bool Lexer::isAtEnd() const {
    return pos_ >= source_.size();
}

char Lexer::peek() const {
    return isAtEnd() ? '\0' : source_[pos_];
}

char Lexer::peekNext() const {
    return (pos_ + 1 >= source_.size()) ? '\0' : source_[pos_ + 1];
}

char Lexer::advance() {
    return source_[pos_++];
}

void Lexer::skipWhitespaceAndNewlines() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n') {
            advance();
            line_++;
        } else {
            break;
        }
    }
}

void Lexer::recordError(const std::string& message, int line) {
    errors_.push_back(LexError{message, line});
}

bool Lexer::hasErrors() const {
    return !errors_.empty();
}

const std::vector<LexError>& Lexer::getErrors() const {
    return errors_;
}

// identifier -> [a-zA-Z_][a-zA-Z0-9_]*
Token Lexer::scanIdentifier() {
    size_t start = pos_;
    int startLine = line_;
    while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
        advance();
    }
    std::string lexeme = source_.substr(start, pos_ - start);
    return Token(TokenType::IDENTIFIER, lexeme, startLine);
}

// INTEGER_LITERAL -> digit+
// FLOAT_LITERAL   -> digit+ '.' digit+
// Anything with a second '.' (e.g. 3.14.5) is malformed: the bad text is
// consumed and an error is recorded, but scanning continues afterwards.
bool Lexer::scanNumber(Token& out) {
    size_t start = pos_;
    int startLine = line_;
    bool isFloat = false;

    while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }

    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
        isFloat = true;
        advance(); // consume '.'
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }

        // A further '.' right after a valid float means the literal is malformed,
        // e.g. "3.14.5". Consume the rest of it so the error message is complete,
        // record the error, and let the caller resume scanning after it.
        if (peek() == '.') {
            while (!isAtEnd() &&
                   (peek() == '.' || std::isdigit(static_cast<unsigned char>(peek())))) {
                advance();
            }
            std::string bad = source_.substr(start, pos_ - start);
            recordError("Malformed numeric literal '" + bad + "'", startLine);
            return false;
        }
    }

    std::string lexeme = source_.substr(start, pos_ - start);
    out = Token(isFloat ? TokenType::FLOAT_LITERAL : TokenType::INTEGER_LITERAL, lexeme, startLine);
    return true;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        skipWhitespaceAndNewlines();
        if (isAtEnd()) break;

        char c = peek();

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            tokens.push_back(scanIdentifier());
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            Token numberToken(TokenType::UNKNOWN, "", line_);
            if (scanNumber(numberToken)) {
                tokens.push_back(numberToken);
            }
            // On failure, the error was already recorded and the bad text
            // already consumed by scanNumber(); just resume scanning.
            continue;
        }

        // Any other single character that isn't a recognized operator or
        // punctuation is invalid: record it and skip past just that one
        // character so scanning can continue.
        int startLine = line_;
        char ch = advance();

        switch (ch) {
            case '=': tokens.emplace_back(TokenType::ASSIGN, "=", startLine); break;
            case '+': tokens.emplace_back(TokenType::PLUS, "+", startLine); break;
            case '-': tokens.emplace_back(TokenType::MINUS, "-", startLine); break;
            case '*': tokens.emplace_back(TokenType::MULTIPLY, "*", startLine); break;
            case '/': tokens.emplace_back(TokenType::DIVIDE, "/", startLine); break;
            case '(': tokens.emplace_back(TokenType::LPAREN, "(", startLine); break;
            case ')': tokens.emplace_back(TokenType::RPAREN, ")", startLine); break;
            case ';': tokens.emplace_back(TokenType::SEMICOLON, ";", startLine); break;
            default:
                recordError("Invalid character '" + std::string(1, ch) + "'", startLine);
                // don't add a token; move on to the next character
                break;
        }
    }

    tokens.emplace_back(TokenType::END_OF_FILE, "", line_);
    return tokens;
}
