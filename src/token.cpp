#include "token.h"

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::IDENTIFIER:      return "IDENTIFIER";
        case TokenType::INTEGER_LITERAL: return "INTEGER_LITERAL";
        case TokenType::FLOAT_LITERAL:   return "FLOAT_LITERAL";
        case TokenType::ASSIGN:          return "ASSIGN";
        case TokenType::PLUS:            return "PLUS";
        case TokenType::MINUS:           return "MINUS";
        case TokenType::MULTIPLY:        return "MULTIPLY";
        case TokenType::DIVIDE:          return "DIVIDE";
        case TokenType::LPAREN:          return "LPAREN";
        case TokenType::RPAREN:          return "RPAREN";
        case TokenType::SEMICOLON:       return "SEMICOLON";
        case TokenType::END_OF_FILE:     return "END_OF_FILE";
        case TokenType::UNKNOWN:         return "UNKNOWN";
    }
    return "UNKNOWN";
}
