#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic_analyzer.h"
#include "tac.h"
#include "tac_generator.h"
#include "tac_optimizer.h"

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

static void printSymbolTable(const SymbolTable& table) {
    std::cout << "Name            Type    First Line  Last Line\n";
    std::cout << "--------------  ------  ----------  ---------\n";
    for (const SymbolEntry& entry : table.entries()) {
        std::cout << std::left << std::setw(16) << entry.name
                   << std::setw(8) << entry.type
                   << std::setw(12) << entry.firstAssignLine
                   << entry.lastAssignLine << "\n";
    }
}

static void printTAC(const std::vector<TACInstruction>& instructions) {
    for (const TACInstruction& instr : instructions) {
        std::cout << formatTACInstruction(instr) << "\n";
    }
}

static void printOptimizationLog(const std::vector<OptimizationLogEntry>& log) {
    if (log.empty()) {
        std::cout << "  (no transformations applied)\n";
        return;
    }
    for (const OptimizationLogEntry& entry : log) {
        std::cout << "  Line " << entry.line << ": " << entry.description << "\n";
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

    // ---- Stage 3: Semantic Analysis (AST -> typed AST + Symbol Table) ----
    // Runs on whatever statements the parser did manage to produce, even if
    // there were earlier lexical/syntax errors -- same recover-and-continue
    // philosophy as the lexer and parser, so a single run surfaces as many
    // issues as possible.
    SemanticAnalyzer semanticAnalyzer;
    semanticAnalyzer.analyze(*program);

    std::cout << "\n=== Semantic Analysis ===\n";
    if (semanticAnalyzer.hasErrors()) {
        for (const SemanticError& err : semanticAnalyzer.getErrors()) {
            std::cout << "  Line " << err.line << ": " << err.message << "\n";
        }
    } else {
        std::cout << "  No semantic errors.\n";
    }

    std::cout << "\n=== Symbol Table ===\n";
    printSymbolTable(semanticAnalyzer.getSymbolTable());

    // ---- Stage 4: TAC Generation (typed AST -> unoptimized TAC) ----
    // Only runs on a fully clean pipeline: TAC generation assumes every
    // identifier resolved and every expression's inferredType already set
    // to a real type, which is only guaranteed once lexing, parsing, and
    // semantic analysis all succeeded. There is no meaningful "error TAC"
    // to recover into, unlike the earlier stages.
    bool cleanSoFar = !lexer.hasErrors() && !parser.hasErrors() && !semanticAnalyzer.hasErrors();

    std::cout << "\n=== TAC ===\n";
    std::vector<TACInstruction> tac;
    if (cleanSoFar) {
        TACGenerator tacGenerator;
        tac = tacGenerator.generate(*program);
        printTAC(tac);
    } else {
        std::cout << "  Skipped (fix lexical/syntax/semantic errors above first).\n";
    }

    // ---- Stage 5: TAC Optimization (unoptimized TAC -> optimized TAC) ----
    // Same gating as Stage 4 for the same reason: optimization operates on
    // the TAC Stage 4 produced, so it only makes sense to run once that
    // TAC actually exists. The original (unoptimized) 'tac' vector above
    // is left untouched -- optimize() returns a separate vector -- so both
    // remain available for side-by-side comparison.
    std::cout << "\n=== Optimized TAC ===\n";
    if (cleanSoFar) {
        TACOptimizer optimizer;
        std::vector<TACInstruction> optimizedTac = optimizer.optimize(tac);
        printTAC(optimizedTac);

        std::cout << "\n=== Optimization Log ===\n";
        printOptimizationLog(optimizer.getLog());
    } else {
        std::cout << "  Skipped (fix lexical/syntax/semantic errors above first).\n";
    }

    // ---- Summary ----
    bool ok = cleanSoFar;
    std::cout << "\n=== Result: " << (ok ? "SUCCESS" : "FAILED")
               << " (" << lexer.getErrors().size() << " lexical error(s), "
               << parser.getErrors().size() << " syntax error(s), "
               << semanticAnalyzer.getErrors().size() << " semantic error(s)) ===\n";

    return ok ? 0 : 1;
}
