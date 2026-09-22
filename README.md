# ExprOpt — Phase 1 Prototype (Review 1)

## What is ExprOpt?

ExprOpt is a small compiler project for a toy expression language with
integer/float variables and arithmetic assignment statements. The full
project (per the submitted Phase 1 report) is planned as a linear pipeline:

```
Source → Lexer → Parser → AST → Semantic Analyzer → Symbol Table
       → TAC Generator → Optimizer → Optimized TAC
```

## Phase 1 scope (this submission)

Only the first part of the pipeline is implemented for Review 1:

```
Source → Lexer → Token Stream → Recursive-Descent Parser → AST
```

Everything after the AST (semantic analysis, symbol table, three-address
code generation, optimization) is **not implemented** — see "What is NOT
implemented yet" below.

## How to build

Requires a C++17 compiler (g++) and `make`. No external libraries.

```
make
```

This compiles `src/*.cpp` with `g++ -std=c++17 -Wall -Wextra` and produces
the `expropt` binary in the project root. `make clean` removes it.

## How to run

```
./expropt <source-file.exo>
```

Example:

```
./expropt tests/valid/01_basic_assignment.exo
```

The program prints the pipeline stage by stage:

1. **Source** — the input file, echoed back.
2. **Tokens** — the full token stream produced by the lexer (type, lexeme,
   line number), plus any lexical errors found.
3. **AST** — the parsed syntax tree as an indented node dump, plus any
   syntax errors found.
4. **Result** — a one-line summary with the error counts and exit code
   (`0` = clean, `1` = one or more lexical/syntax errors).

## Supported grammar

```
program     → statement* EOF
statement   → IDENTIFIER ASSIGN expression SEMICOLON
expression  → term ((PLUS | MINUS) term)*
term        → factor ((MULTIPLY | DIVIDE) factor)*
factor      → MINUS factor
            | LPAREN expression RPAREN
            | INTEGER_LITERAL
            | FLOAT_LITERAL
            | IDENTIFIER
```

Supported tokens: identifiers, integer literals, float literals (`digit+ '.'
digit+`), `= + - * / ( ) ;`, and whitespace/newlines (skipped). Operator
precedence (`*` `/` bind tighter than `+` `-`) and left-associativity are
encoded directly in the grammar layering, exactly as in the report.

## Example input/output

Input (`w = -x + 3;`):

```
=== Tokens ===
Line  Type               Lexeme
----  -----------------  ------
1     IDENTIFIER         w
1     ASSIGN             =
1     MINUS              -
1     IDENTIFIER         x
1     PLUS               +
1     INTEGER_LITERAL    3
1     SEMICOLON          ;
2     END_OF_FILE

=== AST ===
Program
  AssignStmt w =  (line 1)
    BinaryExpr +  (line 1)
      UnaryExpr -  (line 1)
        Identifier x  (line 1)
      IntLiteral 3  (line 1)

=== Result: SUCCESS (0 lexical error(s), 0 syntax error(s)) ===
```

### Error recovery

Both the lexer and the parser use error recovery instead of stopping at the
first problem, per the report's design:

- **Lexer**: an invalid character or a malformed numeric literal (e.g.
  `3.14.5`) is recorded as an error and skipped; scanning continues and
  later valid tokens are still produced.
- **Parser**: on a syntax error, tokens are discarded (panic mode) until a
  `SEMICOLON` or `EOF` is found, then parsing resumes at the next
  statement — so one bad statement doesn't prevent later valid statements
  from appearing in the AST.

See `tests/invalid/06_lexical_error_recovery.exo` and
`tests/invalid/07_syntax_error_recovery.exo` for worked examples.

## Project structure

```
ExprOpt/
├── include/           # token.h, lexer.h, ast.h, parser.h, symbol_table.h, semantic_analyzer.h
├── src/                # matching .cpp files + main.cpp (CLI driver)
├── tests/
│   ├── valid/          # full pipeline valid: pass lexing, parsing, AND semantic
│   │                    analysis with zero errors (basic assignment, multiple
│   │                    statements with variables declared before use)
│   ├── parser-demo/    # syntax-valid parser/AST demonstrations (operator
│   │                    precedence, parentheses, unary minus) that use
│   │                    undeclared identifiers on purpose, so they correctly
│   │                    fail semantic analysis -- useful for showing the
│   │                    Lexer/Parser/AST stages in isolation
│   ├── semantic/        # semantic-analysis-specific cases: valid program,
│   │                    type mismatch, unknown identifier, division by zero,
│   │                    mixed int/float promotion, multiple errors in one run
│   └── invalid/         # lexical error recovery, syntax error recovery
├── Makefile
└── README.md
```

## What is NOT implemented yet

Per the Phase 1 scope, the following are intentionally left for later
phases and are **not** part of this submission:

- Semantic analysis (type inference/checking)
- Symbol table
- Three-address code (TAC) generation
- Optimization (constant folding, constant propagation, algebraic
  simplification)
- GUI (this is a terminal/CLI tool only)
- Any error recovery beyond what's needed for the Phase 1 parser
