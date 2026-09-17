#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "lexer.h"
#include "parser.h"
#include "ast.h"

// Reads the whole file into a string. Returns false if it couldn't be opened.
static bool readFile(const std::string& path, std::string& out) {
    std::ifstream file(path);
    if (!file) return false;
    std::stringstream buffer;
    buffer << file.rdbuf();
    out = buffer.str();
    return true;
}

static void printTokens(const std::vector<Token>& tokens) {
    std::cout << "Line  Type               Lexeme\n";
    std::cout << "----  -----------------  ------\n";
    for (const Token& tok : tokens) {
        std::cout << std::left << std::setw(6) << tok.line
                   << std::setw(19) << tokenTypeToString(tok.type)
                   << tok.lexeme << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source-file.exo>\n";
        return 1;
    }

    std::string source;
    if (!readFile(argv[1], source)) {
        std::cerr << "Error: could not open file '" << argv[1] << "'\n";
        return 1;
    }

    // ---- Stage 0: Source ----
    std::cout << "=== Source ===\n" << source;
    if (!source.empty() && source.back() != '\n') std::cout << "\n";
    std::cout << "\n";

    // ---- Stage 1: Lexing (Source -> Tokens) ----
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    std::cout << "=== Tokens ===\n";
    printTokens(tokens);
    if (lexer.hasErrors()) {
        std::cout << "\nLexical Errors:\n";
        for (const LexError& err : lexer.getErrors()) {
            std::cout << "  Line " << err.line << ": " << err.message << "\n";
        }
    }
    std::cout << "\n";

    // ---- Stage 2: Parsing (Tokens -> AST) ----
    Parser parser(std::move(tokens));
    std::unique_ptr<ProgramNode> program = parser.parseProgram();

    std::cout << "=== AST ===\n";
    program->print(std::cout, 0);
    if (parser.hasErrors()) {
        std::cout << "\nSyntax Errors:\n";
        for (const SyntaxError& err : parser.getErrors()) {
            std::cout << "  Line " << err.line << ": " << err.message << "\n";
        }
    }

    // ---- Summary ----
    bool ok = !lexer.hasErrors() && !parser.hasErrors();
    std::cout << "\n=== Result: " << (ok ? "SUCCESS" : "FAILED")
               << " (" << lexer.getErrors().size() << " lexical error(s), "
               << parser.getErrors().size() << " syntax error(s)) ===\n";

    return ok ? 0 : 1;
}
