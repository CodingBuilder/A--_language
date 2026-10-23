# AMM Compiler

**AMM** (AKA **A--**) is a custom low-level programming language and compiler written in C++. This project serves as an educational deep dive into compiler construction, covering the full pipeline from lexical analysis and AST generation to x86_64 assembly code generation for Linux environments.

---

## Current Progress (Stage 3)
The compiler supports core language constructs, custom control flow, and expression evaluation:

- **Lexical Analysis:** Custom hand-written lexer for clean tokenization of identifiers, keywords (`pls_make`, `check`, `otherwise`, `gimme`), multi-character operators, and custom comment syntax `(: ... :)`.
- **Expression & AST Parsing:** Parser using the **Precedence Climbing Algorithm** for arithmetic precedence (`+`, `-`, `*`, `/`) alongside comparison and assignment node construction.
- **Variable Binding & Re-assignment:** Declaration via `pls_make` and mutable re-assignments (`x => ...`).
- **Unary Operators:** Support for post-increment (`++`) and post-decrement (`--`) operations.
- **Control Flow:** Conditional branching with `check(...)`, `otherwise check(...)`, and `otherwise` blocks.
- **Program Exit:** Expressive termination via `gimme(...)`, mapping directly to Linux **syscall 60** with returned status codes.
- **Backend Code Generation:** Emits valid **x86_64 Assembly** (NASM syntax) targeted for Linux (Ubuntu / WSL2).

---

## Code Example
Here is a valid `.amm` / `.a--` program demonstrating variable declarations, control flow, post-operators, comments, and syscall execution:

```amm
pls_make x => 10;
pls_make y => 20;

check(x < y)
{
    x => x + y;
    y--;
}
otherwise check(x <=> 30)
{
    x++;
    y => y - 5;
}
otherwise
{
    x => 0;
    y => 0;
}

(: Exit the program with exit code << x >> :)
gimme(x);