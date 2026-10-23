# ExprOpt: A C++ Compiler Pipeline for Expression Analysis and Optimization

Compiler Design Laboratory project (Review 3, Phase 3 final). Individual project by Utkarsh Gokhroo, 24BCE0919, VIT Vellore.

ExprOpt is a hand-written C++17 compiler pipeline for a small expression-and-assignment language with `int` and `float` variables. Every stage is implemented from first principles: no Lex, Flex, Yacc, Bison or ANTLR.

```
Source -> Lexer -> Parser -> AST -> Semantic Analyzer + Symbol Table
       -> TAC Generator -> TAC Optimizer -> Optimized TAC + Optimization Log
```

## Implemented

| Phase | Stage | Files |
|-------|-------|-------|
| 1 | Lexer (single pass, lexical error recovery) | `lexer`, `token` |
| 1 | Recursive-descent parser (panic-mode recovery) | `parser` |
| 1 | AST (`unique_ptr` nodes, line numbers, `inferredType`) | `ast` |
| 2 | Flat symbol table | `symbol_table` |
| 2 | Semantic analyzer (bottom-up type inference) | `semantic_analyzer` |
| 2 | Three-address code (TAC) generator | `tac`, `tac_generator` |
| 3 | TAC optimizer (single forward pass, with log) | `tac_optimizer` |
| 1-3 | Driver: staged output, error gating, exit code | `src/main.cpp` |

Headers are in `include/`, implementations in `src/`.

### Phase 3 optimizer

Per instruction, in this order:

1. **Constant propagation**: replace an operand with its known constant value.
2. **Constant folding**: if both operands are literals, compute the result (integer or floating-point arithmetic chosen by the instruction's type).
3. **Identity simplification** (right-hand operand only, exactly four forms): `x + 0 -> x`, `x - 0 -> x`, `x * 1 -> x`, `x / 1 -> x`.
4. **Update** the known-constants table: record a copy of a literal, otherwise erase the entry.

`optimize()` returns a new vector, so the unoptimized TAC stays available for side-by-side comparison. Every transformation is written to the optimization log.

## Not implemented

- Dead-code elimination, common-subexpression elimination, SSA, liveness analysis, CFG or loop optimization
- `x * 0 -> 0` (deliberately excluded: unsafe for float NaN/Inf) and left-hand identities (`0 + x`, `1 * x`)
- Multiple optimization passes, strength reduction
- Control flow, functions, blocks, types other than `int` and `float`
- Machine-code generation or program execution (output is TAC text only)
- Column-level diagnostics (line numbers only)
- Detection of non-literal zero divisors: `y = 0; z = 5 / y;` passes semantic analysis and the optimizer leaves `5 / 0` unfolded
- An automated regression-test script

## Build

Requires a C++17 compiler (g++) and `make`. No external libraries.

```
make
```

or the equivalent direct command:

```
g++ -std=c++17 -Wall -Wextra -Iinclude \
    src/main.cpp src/lexer.cpp src/token.cpp src/parser.cpp \
    src/ast.cpp src/symbol_table.cpp src/semantic_analyzer.cpp \
    src/tac.cpp src/tac_generator.cpp src/tac_optimizer.cpp \
    -o expropt
```

Both produce the `expropt` binary with zero warnings. `make clean` removes it.

## Run

```
./expropt <source-file.exo>
```

Example:

```
./expropt tests/optimizer/02_constant_propagation.exo
```

Output stages: Source, Tokens, AST, Semantic Analysis, Symbol Table, TAC, Optimized TAC, Optimization Log, Result. TAC generation and optimization run only when there are no lexical, syntax or semantic errors; otherwise both print `Skipped`.

Exit code: `0` = clean, `1` = one or more errors (or bad usage / unreadable file).

### Example

Input (`tests/optimizer/02_constant_propagation.exo`):

```
x = 5;
y = x + 2;
```

Output (TAC stages):

```
=== TAC ===
x = 5
t1 = x + 2
y = t1

=== Optimized TAC ===
x = 5
t1 = 7
y = 7

=== Optimization Log ===
  Line 2: propagated 'x' -> '5' in computation of 't1'
  Line 2: constant folded '5 + 2' -> '7'
  Line 2: propagated 't1' -> '7' in computation of 'y'

=== Result: SUCCESS (0 lexical error(s), 0 syntax error(s), 0 semantic error(s)) ===
```

## Language

```
program     -> statement* EOF
statement   -> IDENTIFIER ASSIGN expression SEMICOLON
expression  -> term ((PLUS | MINUS) term)*
term        -> factor ((MULTIPLY | DIVIDE) factor)*
factor      -> MINUS factor
             | LPAREN expression RPAREN
             | INTEGER_LITERAL
             | FLOAT_LITERAL
             | IDENTIFIER
```

Identifiers are `[a-zA-Z_][a-zA-Z0-9_]*`; float literals are `digit+ '.' digit+`. A variable's type is fixed at its first assignment. `int / int` truncates; any float operand promotes the result to float.

## Error handling

- **Lexical**: an invalid character or malformed number is recorded and skipped; scanning continues.
- **Syntax**: panic mode, discard tokens to the next `;` and resume at the next statement.
- **Semantic**: unknown identifier, type mismatch on reassignment, division by a literal zero. All are reported in one run; an internal `invalid` type prevents cascading messages.

## Tests

16 programs under `tests/`; run any with `./expropt <file>`.

| Folder | Files | Expected exit | Purpose |
|--------|-------|---------------|---------|
| `valid/` | 2 | 0 | Clean full pipeline |
| `optimizer/` | 3 | 0 | Folding, propagation, identity-form programs |
| `semantic/` | 6 | 0 (01, 05), 1 (02, 03, 04, 06) | Valid, mixed int/float, and each semantic error class |
| `invalid/` | 2 | 1 | Lexical and syntax error recovery |
| `parser-demo/` | 3 | 1 | Precedence, parentheses, unary minus (undeclared identifiers on purpose, so semantic analysis rejects them) |

Results are checked by inspection of the output; there is no automated expected-output comparison yet.

## Project structure

```
expropt/
├── include/      # token.h, lexer.h, ast.h, parser.h, symbol_table.h,
│                 # semantic_analyzer.h, tac.h, tac_generator.h, tac_optimizer.h
├── src/          # matching .cpp files + main.cpp
├── tests/        # valid/ optimizer/ semantic/ invalid/ parser-demo/
├── Makefile
└── README.md
```
