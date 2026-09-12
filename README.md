# 🚀 AMM Compiler

**AMM** (AKA **A--**) is a custom low-level programming language and compiler written in C++. This project serves as an educational deep dive into compiler construction, covering the full pipeline from lexical analysis and AST generation to x86_64 assembly code generation for Linux environments.

---

## 🛠 Current Progress (Stage 2)
The compiler supports core language constructs, custom keywords, and expressions:

* **Lexical Analysis:** Custom hand-written lexer for clean tokenization of identifiers, keywords, and operators.
* **Expression Parsing:** AST construction using the **Precedence Climbing Algorithm** to handle operator precedence and associativity (`+`, `-`, `*`, `/`).
* **Variable Declaration & Assignment:** Variable declaration using `pls_make` with initial value binding.
* **Output / Exit Statements:** Expressive exit statement via `gimme()`, mapping directly to Linux **syscall 60** with calculated exit status codes.
* **Backend Code Generation:** Emits valid **x86_64 Assembly** (NASM syntax) targeted for Linux (Ubuntu / WSL2).

---

## 💻 Code Example
Here is a valid `.amm \ .a--` program featuring variable declarations and arithmetic evaluation:

```amm
pls_make x => 8 / 2;
pls_make y => 10 - 5;

gimme(x * y);